/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printInvoices.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <mqlutil.h>

#include "errorReporter.h"

printInvoices::printInvoices(QWidget *parent, const char *name, bool modal, Qt::WFlags fl)
    : printMulticopyDocument("InvoiceCopies",     "InvoiceWatermark",
                             "InvoiceShowPrices", "PostMiscInvoices",
                             parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  setDoctype("IN");
  setReportKey("invchead_id");
  _distributeInventory = true;

  _shipvia->populate( "SELECT MIN(invchead_id), invchead_shipvia "
                      "  FROM invchead "
                      " WHERE ( (NOT invchead_printed)"
                      "   AND   (NOT invchead_posted) )"
                      " GROUP BY invchead_shipvia"
                      " ORDER BY invchead_shipvia;" );

  QString      errmsg;
  bool         ok  = false;
  MetaSQLQuery mql = MQLUtil::mqlLoad("invoices", "print", errmsg, &ok);
  if (ok)
    _docinfoQueryString = mql.getSource();
  else
    ErrorReporter::error(QtCriticalMsg, this, tr("Getting Invoices to Print"),
                         errmsg, __FILE__, __LINE__);

  _markAllPrintedQry = "UPDATE invchead"
                       "   SET invchead_printed=TRUE "
                       " WHERE invchead_id IN ("
                       "<? foreach('printedDocs') ?>"
                       "  <? if not isfirst('printedDocs') ?>, <? endif ?>"
                       "  <? value('printedDocs') ?>"
                       "<? endforeach ?>"
                       ");" ;
              
  _postFunction = "postInvoice";
  _postQuery    = "SELECT postInvoice(<? value('docid') ?>) AS result;" ;

  _askBeforePostingQry = "SELECT invoiceTotal(<? value('docid') ?>) = 0 AS ask;" ;
  _askBeforePostingMsg = tr("<p>Invoice %1 has a total value of 0.<br/>"
                            "Would you like to post it anyway?</p>");

  _errCheckBeforePostingQry =
         "SELECT EXISTS(SELECT 1"
         "                FROM curr_rate"
         "                JOIN invchead ON (curr_id=invchead_curr_id)"
         "               WHERE ((invchead_invcdate BETWEEN curr_effective AND curr_expires)"
         "                 AND  (invchead_id=<? value('docid') ?>))) AS ok;" ;
  _errCheckBeforePostingMsg =
          tr("Could not post Invoice %1 because of a missing exchange rate.");

  connect(this, SIGNAL(aboutToStart(XSqlQuery*)), this, SLOT(sHandleAboutToStart(XSqlQuery*)));
  connect(this, SIGNAL(finishedWithAll()),        this, SLOT(sHandleFinishedWithAll()));
}

printInvoices::~printInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void printInvoices::languageChange()
{
  retranslateUi(this);
}

ParameterList printInvoices::getParamsDocList()
{
  ParameterList params = printMulticopyDocument::getParamsDocList();
  if (_selectedShipvia->isChecked())
    params.append("shipvia", _shipvia->currentText());

  return params;
}

ParameterList printInvoices::getParamsOneCopy(const int row, XSqlQuery *qry)
{
  ParameterList params = printMulticopyDocument::getParamsOneCopy(row, qry);
  if (_selectedShipvia->isChecked())
    params.append("shipvia", _shipvia->currentText());

  return params;
}

void printInvoices::sHandleAboutToStart(XSqlQuery *qry)
{
  int invoiceNumber = qry->value("invchead_invcnumber").toInt();
  if (invoiceNumber == 0)
  {
    XSqlQuery local;
    local.prepare("UPDATE invchead"
                  "   SET invchead_invcnumber=text(fetchInvcNumber())"
                  " WHERE (invchead_id=:invchead_id)"
                  " RETURNING invchead_invcnumber;" );
    local.bindValue(":invchead_id", qry->value("invchead_id"));
    local.exec();
    if (local.first())
      ; // TODO: what should we should do with the new invchead_invcnumber?
    else
      ErrorReporter::error(QtCriticalMsg, this, tr("Updating Invoice"),
                           local, __FILE__, __LINE__);
  }
}

void printInvoices::sHandleFinishedWithAll()
{
  omfgThis->sInvoicesUpdated(-1, TRUE);
}

