/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printPurchaseOrder.h"

#include <QMessageBox>

#include <metasql.h>

#include "errorReporter.h"

printPurchaseOrder::printPurchaseOrder(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : printMulticopyDocument("POCopies",     "POWatermark",
                             "POShowPrices", "",
                             parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  setDoctype("PO");
  setReportKey("pohead_id");

  _docinfoQueryString =
         "SELECT pohead_id       AS docid, pohead_id,"
         "       pohead_number   AS docnumber,"
         "       pohead_printed  AS printed,"
         "       false           AS posted,"
         "       'PurchaseOrder' AS reportname,"
         "       pohead_saved"
         "  FROM pohead"
         " WHERE (pohead_id=<? value('docid') ?>);" ;

  _markOnePrintedQry = "UPDATE pohead"
                       "   SET pohead_printed=true "
                       " WHERE (pohead_id=<? value('docid') ?>);" ;

  connect(_po,  SIGNAL(newId(int, QString)),   this, SLOT(setId(int)));
  connect(this, SIGNAL(docUpdated(int)),       this, SLOT(sHandleDocUpdated(int)));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sHandlePopulated(XSqlQuery*)));
  connect(this, SIGNAL(finishedWithAll()),     this, SLOT(sFinishedWithAll()));
}

printPurchaseOrder::~printPurchaseOrder()
{
}

void printPurchaseOrder::languageChange()
{
  retranslateUi(this);
}

ParameterList printPurchaseOrder::getParamsOneCopy(const int row,
                                                   XSqlQuery *qry)
{
  ParameterList params = printMulticopyDocument::getParamsOneCopy(row, qry);
  params.append("title", copies()->watermark(row));
  return params;
}

bool printPurchaseOrder::isOkToPrint()
{
  MetaSQLQuery mql(_docinfoQueryString);
  ParameterList params;
  params.append("docid", id());
  XSqlQuery okq = mql.toQuery(params);
  if (okq.first() && ! okq.value("pohead_saved").toBool())
  {
    QMessageBox::warning( this, tr("Cannot Print P/O"),
                         tr("<p>The Purchase Order you are trying to print has "
                            "not been completed. Please wait until the "
                            "Purchase Order has been completely saved.") );
    return false;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Has P/O Been Saved?"),
                                okq, __FILE__, __LINE__))
    return false;

  return true;
}

void printPurchaseOrder::sHandleDocUpdated(int docid)
{
  omfgThis->sPurchaseOrdersUpdated(docid, false);
}

void printPurchaseOrder::sHandlePopulated(XSqlQuery *docq)
{
  if (docq && _po->id() != docq->value("docid").toInt())
    _po->setId(docq->value("docid").toInt());
}

void printPurchaseOrder::sFinishedWithAll()
{
  _po->setId(-1);
}

