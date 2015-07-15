/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printCreditMemo.h"

#include <QDebug>
#include <QSqlRecord>
#include <QVariant>

printCreditMemo::printCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : printMulticopyDocument("CreditMemoCopies",     "CreditMemoWatermark",
                             "CreditMemoShowPrices", "PostARDocuments",
                             parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  setDoctype("CM");
  setReportKey("cmhead_id");
  _distributeInventory = true;

  _docinfoQueryString =
             "SELECT cmhead_id      AS docid,"
             "       cmhead_number  AS docnumber,"
             "       cmhead_printed AS printed,"
             "       cmhead_posted  AS posted,"
             "       cmhead_cust_id,"
             "       findCustomerForm(cmhead_cust_id, 'C') AS reportname "
             "FROM cmhead "
             "WHERE (cmhead_id=<? value('docid') ?>);" ;

  _markOnePrintedQry = "UPDATE cmhead "
                       "   SET cmhead_printed=true "
                       " WHERE (cmhead_id=<? value('docid') ?>);" ;
  _postFunction = "postCreditMemo";
  _postQuery    = "SELECT postCreditMemo(<? value('docid') ?>, 0) AS result;" ;

  connect(this, SIGNAL(docUpdated(int)),       this, SLOT(sHandleDocUpdated(int)));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sHandlePopulated(XSqlQuery*)));
}

printCreditMemo::~printCreditMemo()
{
}

void printCreditMemo::languageChange()
{
  retranslateUi(this);
}

void printCreditMemo::sHandlePopulated(XSqlQuery *qry)
{
  if (qry)
  {
    _number->setText(qry->value("docnumber").toString());
    _cust->setId(qry->value("cmhead_cust_id").toInt());
  }
}

void printCreditMemo::sHandleDocUpdated(int docid)
{
  Q_UNUSED(docid);
  omfgThis->sCreditMemosUpdated();
}
