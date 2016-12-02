/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printStatementsByCustomerType.h"

#include <metasql.h>

#include "mqlutil.h"
#include "errorReporter.h"

printStatementsByCustomerType::printStatementsByCustomerType(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : printSinglecopyDocument(parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  _asOf->setDate(omfgThis->dbDate(), true);
  _customerTypes->setType(ParameterGroup::CustomerType);
  if (_preferences->boolean("XCheckBox/forgetful"))
    _dueonly->setChecked(true);
  setPrintEnabled(true);

  setDoctype("AR");
  setReportKey("cust_id");

  bool    ok = false;
  QString msg;
  MetaSQLQuery custm = MQLUtil::mqlLoad("customers", "statement", msg, &ok);
  if (ok)
    _docinfoQueryString = custm.getSource();
  else
    ErrorReporter::error(QtCriticalMsg, this, tr("Getting Customers to Print"),
                         msg, __FILE__, __LINE__);
}

printStatementsByCustomerType::~printStatementsByCustomerType()
{
  // no need to delete child widgets, Qt does it all for us
}

void printStatementsByCustomerType::languageChange()
{
  retranslateUi(this);
}

ParameterList printStatementsByCustomerType::getParams(XSqlQuery *docq)
{
  ParameterList params = printSinglecopyDocument::getParams(docq);

  params.append("invoice",  tr("Invoice"));
  params.append("return",   tr("Sales Credit"));
  params.append("debit",    tr("Debit Memo"));
  params.append("credit",   tr("Credit Memo"));
  params.append("deposit",  tr("Deposit"));
  params.append("cust_id",  docq->value("cust_id").toInt());
  params.append("asofdate", _asOf->date());

  return params;
}

ParameterList printStatementsByCustomerType::getParamsDocList()
{
  ParameterList params = printSinglecopyDocument::getParamsDocList();

  _customerTypes->appendValue(params);
  if (_graceDays->value() > 0)
    params.append("graceDays", _graceDays->value());

  if (_dueonly->isChecked())
	  params.append("graceDays", _graceDays->value());

  params.append("asofDate", _asOf->date());
  return params;
}
