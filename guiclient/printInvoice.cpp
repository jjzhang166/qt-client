/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printInvoice.h"

#include <QSqlRecord>
#include <QVariant>

printInvoice::printInvoice(QWidget *parent, const char *name, bool modal, Qt::WindowFlags fl)
    : printMulticopyDocument("InvoiceCopies",     "InvoiceWatermark",
                             "InvoiceShowPrices", "PostMiscInvoices",
                             parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  setDoctype("IN");
  setReportKey("invchead_id");
  _distributeInventory = true;

  _docinfoQueryString =
         "SELECT invchead_id         AS docid,"
         "       invchead_invcnumber AS docnumber,"
         "       invchead_printed    AS printed,"
         "       invchead_posted     AS posted,"
         "       invchead_cust_id,"
         "       findCustomerForm(invchead_cust_id, 'I') AS reportname "
         "FROM invchead "
         "WHERE (invchead_id=<? value('docid') ?>);" ;

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

  _markOnePrintedQry = "UPDATE invchead"
                       "   SET invchead_printed=true"
                       " WHERE (invchead_id=<? value('docid') ?>);" ;

  _postFunction = "postInvoice";
  _postQuery = "SELECT postInvoice(<? value('docid') ?>) AS result;" ;

  connect(this, SIGNAL(docUpdated(int)),       this, SLOT(sHandleDocUpdated(int)));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sHandlePopulated(XSqlQuery*)));
}

printInvoice::~printInvoice()
{
}

void printInvoice::languageChange()
{
  retranslateUi(this);
}

void printInvoice::sHandlePopulated(XSqlQuery *qry)
{
  if (qry)
  {
    _invoiceNum->setText(qry->value("docnumber").toString());
    _cust->setId(qry->value("invchead_cust_id").toInt());
  }
}

void printInvoice::sHandleDocUpdated(int docid)
{
  omfgThis->sInvoicesUpdated(docid, true);
}
