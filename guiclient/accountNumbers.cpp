/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "accountNumbers.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <mqlutil.h>
#include <openreports.h>

#include "accountNumber.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

accountNumbers::accountNumbers(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_account,        SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_delete,           SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,             SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,              SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,            SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_showExternal, SIGNAL(toggled(bool)), this, SLOT(sBuildList()));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_type, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_type, SIGNAL(activated(int)), this, SLOT(populateSubTypes()));
  connect(_subType, SIGNAL(newID(int)), this, SLOT(sFillList()));

  connect(omfgThis, SIGNAL(configureGLUpdated()), this, SLOT(sBuildList()));

  _type->setAllowNull(true);
  QString qryType = QString( "SELECT  1, '%1' UNION "
                             "SELECT  2, '%2' UNION "
                             "SELECT  3, '%3' UNION "
                             "SELECT  4, '%4' UNION "
                             "SELECT  5, '%5' ")
  .arg(tr("Asset"))
  .arg(tr("Liability"))
  .arg(tr("Expense"))
  .arg(tr("Revenue"))
  .arg(tr("Equity"));

  _type->populate(qryType);

  _subType->setAllowNull(true);
  populateSubTypes();

  _showExternal->setVisible(_metrics->boolean("MultiCompanyFinancialConsolidation"));

  _externalCol = -1;

  sBuildList();
}

accountNumbers::~accountNumbers()
{
  // no need to delete child widgets, Qt does it all for us
}

void accountNumbers::languageChange()
{
  retranslateUi(this);
}

void accountNumbers::sDelete()
{
  XSqlQuery deleteAccount;
  deleteAccount.prepare("SELECT deleteAccount(:accnt_id) AS result;");
  deleteAccount.bindValue(":accnt_id", _account->id());
  deleteAccount.exec();
  if (deleteAccount.first())
  {
    int result = deleteAccount.value("result").toInt();
    if (result < 0)
    {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Account"),
                             storedProcErrorLookup("deleteAccounting", result),
                             __FILE__, __LINE__);
        return;
    }
    sFillList();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Account"),
                                deleteAccount, __FILE__, __LINE__))
  {
      return;
  }
}

void accountNumbers::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  accountNumber newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void accountNumbers::sEdit()
{
  ParameterList params;
  // don't allow editing external accounts
  if (_externalCol >= 0 &&
      _account->currentItem()->rawValue("company_external").toBool())
    params.append("mode", "view");
  else
    params.append("mode", "edit");
  params.append("accnt_id", _account->id());

  accountNumber newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

bool accountNumbers::setParams(ParameterList &pParams)
{
  if (_metrics->boolean("MultiCompanyFinancialConsolidation") &&
      _showExternal->isChecked())
    pParams.append("showExternal");
  if (_showInactive->isChecked())
    pParams.append("showInactive");

  pParams.append("asset",     tr("Asset"));
  pParams.append("expense",   tr("Expense"));
  pParams.append("liability", tr("Liability"));
  pParams.append("equity",    tr("Equity"));
  pParams.append("revenue",   tr("Revenue"));

  if (_type->id() == 1)
    pParams.append("accnt_type", "A");
  else if (_type->id() == 2)
    pParams.append("accnt_type", "L");
  else if (_type->id() == 3)
    pParams.append("accnt_type", "E");
  else if (_type->id() == 4)
    pParams.append("accnt_type", "R");
  else if (_type->id() == 5)
    pParams.append("accnt_type", "Q");

  if (_subType->currentIndex() > 0)
    pParams.append("subaccnt_type", _subType->id());    

  return true;
}

void accountNumbers::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("AccountNumberMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void accountNumbers::sFillList()
{
  XSqlQuery accountFillList;
  ParameterList params;
  if (! setParams(params))
    return;

  bool ok = true;
  QString errorString;
  MetaSQLQuery mql = MQLUtil::mqlLoad("accountNumbers", "detail", errorString, &ok);
  if(!ok)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Accounts"),
                                errorString, __FILE__, __LINE__);
    return;
  }
  accountFillList = mql.toQuery(params);

  _account->populate(accountFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Accounts"),
                                accountFillList, __FILE__, __LINE__))
  {
      return;
  }
}

void accountNumbers::sBuildList()
{
  _account->setColumnCount(0);
  _externalCol = -1;

  if (_metrics->value("GLCompanySize").toInt() > 0)
  {
    _account->addColumn(tr("Company"),       50, Qt::AlignCenter, true, "accnt_company");
    _externalCol++;
  }

  if (_metrics->value("GLProfitSize").toInt() > 0)
  {
    _account->addColumn(tr("Profit"),        50, Qt::AlignCenter, true, "accnt_profit");
    _externalCol++;
  }

  _account->addColumn(tr("Main Segment"), 100, Qt::AlignCenter, true, "accnt_number");
  _externalCol++;

  if (_metrics->value("GLSubaccountSize").toInt() > 0)
  {
    _account->addColumn(tr("Sub."),          50, Qt::AlignCenter, true, "accnt_sub");
    _externalCol++;
  }

  _account->addColumn(tr("Description"),     -1, Qt::AlignLeft,   true, "accnt_descrip");
  _externalCol++;

  _account->addColumn(tr("Type"),            75, Qt::AlignLeft ,  true, "accnt_type");
  _externalCol++;

  _account->addColumn(tr("Sub. Type Code"),  75, Qt::AlignLeft,  false, "subaccnttype_code");
  _externalCol++;

  _account->addColumn(tr("Sub. Type"),      100, Qt::AlignLeft,  false, "subaccnttype_descrip");
  _externalCol++;

  _account->addColumn(tr("Active"),          75, Qt::AlignLeft,  false, "accnt_active");
  _externalCol++;

  if (_metrics->value("Application") != "PostBooks")
  {
    _account->addColumn(tr("External"), _ynColumn, Qt::AlignCenter, false, "company_external");
    _externalCol++;
  }
  else
    _externalCol = -1;

  sFillList();
}

void accountNumbers::sHandleButtons()
{
  // don't allow editing external accounts
  if (_externalCol >= 0 && _account->currentItem() &&
      _account->currentItem()->rawValue("company_external").toBool())
    _edit->setText(tr("View"));
  else
    _edit->setText(tr("Edit"));
}

void accountNumbers::populateSubTypes()
{
  XSqlQuery sub;
  sub.prepare("SELECT subaccnttype_id, (subaccnttype_code||'-'||subaccnttype_descrip) "
              "FROM subaccnttype "
              "WHERE (subaccnttype_accnt_type=:subaccnttype_accnt_type) "
              "ORDER BY subaccnttype_code; ");
  if (_type->id() == 1)
    sub.bindValue(":subaccnttype_accnt_type", "A");
  else if (_type->id() == 2)
    sub.bindValue(":subaccnttype_accnt_type", "L");
  else if (_type->id() == 3)
    sub.bindValue(":subaccnttype_accnt_type", "E");
  else if (_type->id() == 4)
    sub.bindValue(":subaccnttype_accnt_type", "R");
  else if (_type->id() == 5)
    sub.bindValue(":subaccnttype_accnt_type", "Q");
  sub.exec();
  _subType->populate(sub);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving SubTypes"),
                                sub, __FILE__, __LINE__))
  {
      return;
  }
  
}
