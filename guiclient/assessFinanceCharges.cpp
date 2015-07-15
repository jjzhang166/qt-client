/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "assessFinanceCharges.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
//#include <QStatusBar>
#include <metasql.h>
#include "mqlutil.h"
#include <parameter.h>
#include <openreports.h>
#include "storedProcErrorLookup.h"
#include "xtreewidget.h"

assessFinanceCharges::assessFinanceCharges(QWidget* parent, const char* name, Qt::WindowFlags fl)
: XWidget(parent, name, fl)
{
  setupUi(this);
  _assessmentDate->setDate(omfgThis->dbDate());
  
  //setup table
  _invoiceList->addColumn(tr("Cust. #"),               _orderColumn,    Qt::AlignLeft,   true, "cust_number");
  _invoiceList->addColumn(tr("Cust. Name"),            -1,              Qt::AlignLeft,   true, "cust_name");
  _invoiceList->addColumn(tr("Invoice #"),             _orderColumn,    Qt::AlignLeft,   true, "aropen_docnumber");
  _invoiceList->addColumn(tr("Invoice Date"),          _dateColumn,     Qt::AlignCenter, true, "aropen_docdate");
  _invoiceList->addColumn(tr("Due Date"),              _dateColumn,     Qt::AlignCenter, true, "aropen_duedate");
  _invoiceList->addColumn(tr("Invoice Amt."),          _bigMoneyColumn, Qt::AlignRight,  true, "aropen_amount");
  _invoiceList->addColumn(tr("Paid Amt."),             _bigMoneyColumn, Qt::AlignRight,  true, "aropen_paid");
  _invoiceList->addColumn(tr("Overdue Balance"),       _bigMoneyColumn, Qt::AlignRight,  true, "balance");
  _invoiceList->addColumn(tr("Previous F.C. Date"),    _dateColumn,     Qt::AlignCenter, true, "aropen_fincharg_date");
  _invoiceList->addColumn(tr("Previous F.C. Amt."),    _bigMoneyColumn, Qt::AlignRight,  true, "aropen_fincharg_amount");
  _invoiceList->addColumn(tr("Finance Charge"),        _bigMoneyColumn, Qt::AlignRight,  true, "fincharge");
  
  //setup slots
  connect(_customerSelector, SIGNAL(newState(int)), this, SLOT(sFillList()));
  connect(_customerSelector, SIGNAL(newCustId(int)), this, SLOT(sFillList()));
  connect(_customerSelector, SIGNAL(newCustTypeId(int)), this, SLOT(sFillList()));
  connect(_customerSelector, SIGNAL(newTypePattern(QString)), this, SLOT(sFillList()));
  connect(_customerSelector, SIGNAL(newCustGroupId(int)), this, SLOT(sFillList()));
  connect(_assessmentDate,   SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
  
  connect(_assessCharges,SIGNAL(clicked()), this, SLOT(sAssessCharges()));
  
  sFillList();
}

assessFinanceCharges::~assessFinanceCharges()
{
  // no need to delete child widgets, Qt does it all for us
}

void assessFinanceCharges::languageChange()
{
  retranslateUi(this);
}

void assessFinanceCharges::sFillList()
{
  _invoiceList->clear();
  XSqlQuery arFillInvoiceList;
  MetaSQLQuery mql = mqlLoad("assessFinanceCharges", "detail");
  ParameterList params;
  _customerSelector->appendValue(params);
  params.append("assessmentDate", _assessmentDate->date());
  arFillInvoiceList = mql.toQuery(params);
  _invoiceList->populate(arFillInvoiceList);
}

void assessFinanceCharges::sAssessCharges()
{
  XSqlQuery arAssessCharges;
  arAssessCharges.exec("BEGIN;");
  QList<XTreeWidgetItem*> selected = _invoiceList->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    arAssessCharges.prepare("SELECT assessFinanceCharge(:aropen_id, :assessDate, :assessAmount) AS result;");
    arAssessCharges.bindValue(":aropen_id", ((XTreeWidgetItem*)(selected[i]))->id());
    arAssessCharges.bindValue(":assessDate", _assessmentDate->date());
    arAssessCharges.bindValue(":assessAmount", ((XTreeWidgetItem*)(selected[i]))->rawValue("fincharge").toDouble());
    arAssessCharges.exec();
    if (arAssessCharges.first())
    {
      int result = arAssessCharges.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("assessFinanceCharge", result),
                    __FILE__, __LINE__);
        arAssessCharges.exec("ROLLBACK;");
        return;
      }
    }
    else if (arAssessCharges.lastError().type() != QSqlError::NoError)
    {
      systemError(this, arAssessCharges.lastError().databaseText(), __FILE__, __LINE__);
      arAssessCharges.exec("ROLLBACK;");
      return;
    }
  }
  arAssessCharges.exec("COMMIT;");
  sFillList();
}
