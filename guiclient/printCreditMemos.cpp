/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printCreditMemos.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "storedProcErrorLookup.h"
#include "distributeInventory.h"

printCreditMemos::printCreditMemos(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
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
             "       findCustomerForm(cmhead_cust_id, 'C') AS reportname"
             "  FROM cmhead "
             " WHERE (NOT cmhead_hold"
             "    AND NOT COALESCE(cmhead_printed, false)"
             "    AND checkCreditMemoSitePrivs(cmhead_id));" ;

  _markAllPrintedQry = "UPDATE cmhead"
                       "   SET cmhead_printed=true "
                       " WHERE cmhead_id IN ("
                       "<? foreach('printedDocs') ?>"
                       "  <? if not isfirst('printedDocs') ?>, <? endif ?>"
                       "  <? value('printedDocs') ?>"
                       "<? endforeach ?>"
                       ");" ;
              
  _postFunction = "postCreditMemo";
  _postQuery    = "SELECT postCreditMemo(<? value('docid') ?>, 0) AS result;" ;

  connect(this, SIGNAL(finishedWithAll()), this, SLOT(sHandleFinishedWithAll()));
}

printCreditMemos::~printCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

void printCreditMemos::languageChange()
{
  retranslateUi(this);
}

void printCreditMemos::sHandleFinishedWithAll()
{
  omfgThis->sCreditMemosUpdated();
  this->close();
}
