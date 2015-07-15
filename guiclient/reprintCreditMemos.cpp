/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reprintCreditMemos.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

reprintCreditMemos::reprintCreditMemos(QWidget    *parent,
                                       const char *name,
                                       bool        modal,
                                       Qt::WindowFlags  fl)
    : reprintMulticopyDocument("CreditMemoCopies",     "CreditMemoWatermark",
                               "CreditMemoShowPrices", parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  list()->addColumn(tr("C/M #"),    _orderColumn, Qt::AlignRight, true,  "docnumber");
  list()->addColumn(tr("Doc. Date"), _dateColumn, Qt::AlignCenter,true,  "cmhead_docdate");
  list()->addColumn(tr("Cust. #"),            -1, Qt::AlignLeft,  true,  "cust_number");
  list()->addColumn(tr("Customer"),           -1, Qt::AlignLeft,  true,  "cust_name");
  list()->addColumn(tr("Report"),             -1, Qt::AlignLeft,  false, "reportname");

  _docListQueryString = "SELECT cmhead_id, cust_id,"
                        "      cmhead_number AS docnumber, cmhead_docdate,"
                        "      cust_number, cust_name,"
                        "      findCustomerForm(cust_id, 'C') AS reportname"
                        "  FROM cmhead"
                        "  JOIN custinfo ON (cmhead_cust_id=cust_id)"
                        " WHERE checkCreditMemoSitePrivs(cmhead_id)"
                        " ORDER BY cmhead_docdate DESC;" ;
  setDoctype("CM");
  setReportKey("cmhead_id");

  sPopulate();
}

reprintCreditMemos::~reprintCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

void reprintCreditMemos::languageChange()
{
  retranslateUi(this);
}
