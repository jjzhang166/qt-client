/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "viewCheckRun.h"

#include <QInputDialog>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "miscCheck.h"
#include "mqlutil.h"
#include "postCheck.h"
#include "postChecks.h"
#include "prepareCheckRun.h"
#include "printCheck.h"
#include "printChecks.h"
#include "storedProcErrorLookup.h"

viewCheckRun::viewCheckRun(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);  
  
  connect(_check, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(sHandleItemSelection()));
  connect(_check, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu *)));
  connect(_bankaccnt,     SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_delete,         SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,           SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_newMiscCheck,   SIGNAL(clicked()), this, SLOT(sNewMiscCheck()));
  connect(_prepareCheckRun,SIGNAL(clicked()), this, SLOT(sPrepareCheckRun()));
  connect(_replace,        SIGNAL(clicked()), this, SLOT(sReplace()));
  connect(_replaceAll,     SIGNAL(clicked()), this, SLOT(sReplaceAll()));
  connect(_vendorgroup,    SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_vendorgroup,    SIGNAL(updated()), this, SLOT(sHandleVendorGroup()));
  connect(_void,           SIGNAL(clicked()), this, SLOT(sVoid()));

  _check->addColumn(tr("Void"),                _ynColumn,       Qt::AlignCenter,true, "checkhead_void");
  _check->addColumn(tr("Misc."),               _ynColumn,       Qt::AlignCenter,true, "checkhead_misc" );
  _check->addColumn(tr("Prt'd"),               _ynColumn,       Qt::AlignCenter,true, "checkhead_printed" );
  _check->addColumn(tr("Document #"),          _itemColumn,     Qt::AlignCenter,true, "number" );
  _check->addColumn(tr("Recipient/Invc./CM #"),-1,              Qt::AlignLeft,  true, "description"   );
  _check->addColumn(tr("Payment Date") ,       _dateColumn,     Qt::AlignCenter,true, "checkdate" );
  _check->addColumn(tr("Amount"),              _moneyColumn,    Qt::AlignRight, true, "amount"  );
  _check->addColumn(tr("Currency"),            _currencyColumn, Qt::AlignLeft,  true, "currAbbr" );
  if (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled"))
    _check->addColumn(tr("EFT Batch"),        _orderColumn,     Qt::AlignLeft,  true, "checkhead_ach_batch" );

  if (omfgThis->singleCurrency())
      _check->hideColumn("curr_concat");

  connect(omfgThis, SIGNAL(checksUpdated(int, int, bool)), this, SLOT(sFillList(int)));

  sFillList(); 
}

viewCheckRun::~viewCheckRun()
{
  // no need to delete child widgets, Qt does it all for us
}

void viewCheckRun::languageChange()
{
  retranslateUi(this);
}

void viewCheckRun::sVoid()
{
  XSqlQuery viewVoid;
  viewVoid.prepare( "SELECT checkhead_bankaccnt_id, voidCheck(checkhead_id) AS result "
             "FROM checkhead "
             "WHERE (checkhead_id=:checkhead_id);" );
  viewVoid.bindValue(":checkhead_id", _check->id());
  viewVoid.exec();
  if (viewVoid.first())
  {
    int result = viewVoid.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("voidCheck", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(viewVoid.value("checkhead_bankaccnt_id").toInt(), _check->id(), true);
  }
  else if (viewVoid.lastError().type() != QSqlError::NoError)
  {
    systemError(this, viewVoid.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sDelete()
{
  XSqlQuery viewDelete;
  viewDelete.prepare( "SELECT checkhead_bankaccnt_id, deleteCheck(checkhead_id) AS result "
             "FROM checkhead "
             "WHERE (checkhead_id=:checkhead_id);" );
  viewDelete.bindValue(":checkhead_id", _check->id());
  viewDelete.exec();
  if (viewDelete.first())
  {
    int result = viewDelete.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteCheck", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(viewDelete.value("checkhead_bankaccnt_id").toInt(), _check->id(), true);
  }
  else if (viewDelete.lastError().type() != QSqlError::NoError)
  {
    systemError(this, viewDelete.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sNewMiscCheck()
{
  ParameterList params;
  params.append("new");
  params.append("bankaccnt_id", _bankaccnt->id());
  if (_vendorgroup->isSelectedVend())
    params.append("vend_id", _vendorgroup->vendId());

  miscCheck *newdlg = new miscCheck(this, "_miscCheck", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void viewCheckRun::sEdit()
{
  ParameterList params;
  params.append("edit");
  params.append("check_id", _check->id());

  miscCheck *newdlg = new miscCheck();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void viewCheckRun::sReplace()
{
  XSqlQuery viewReplace;
  viewReplace.prepare( "SELECT checkhead_bankaccnt_id, replaceVoidedCheck(:check_id) AS result "
             "FROM checkhead "
             "WHERE (checkhead_id=:check_id);" );
  viewReplace.bindValue(":check_id", _check->id());
  viewReplace.exec();
  if (viewReplace.first())
  {
    int result = viewReplace.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("replaceVoidedCheck", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated( viewReplace.value("checkhead_bankaccnt_id").toInt(),
                                viewReplace.value("result").toInt(), true);
  }
  else if (viewReplace.lastError().type() != QSqlError::NoError)
  {
    systemError(this, viewReplace.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sReplaceAll()
{
  XSqlQuery viewReplaceAll;
  viewReplaceAll.prepare("SELECT replaceAllVoidedChecks(:bankaccnt_id) AS result;");
  viewReplaceAll.bindValue(":bankaccnt_id", _bankaccnt->id());
  viewReplaceAll.exec();
  if (viewReplaceAll.first())
  {
    int result = viewReplaceAll.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("replaceAllVoidedChecks", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(_bankaccnt->id(), -1, true);
  }
  else if (viewReplaceAll.lastError().type() != QSqlError::NoError)
  {
    systemError(this, viewReplaceAll.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sPrint()
{
  ParameterList params;
  params.append("check_id", _check->id());

  printCheck *newdlg = new printCheck();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void viewCheckRun::sPost()
{
  ParameterList params;
  params.append("check_id", _check->id());

  postCheck newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void viewCheckRun::sAltExchRate()
{
  XSqlQuery exchrateq;
  exchrateq.prepare("SELECT COALESCE(checkhead_alt_curr_rate, checkhead_curr_rate, 1.0) AS exchrate "
                    "FROM checkhead "
                    "WHERE (checkhead_id=:checkhead_id);");
  exchrateq.bindValue(":checkhead_id", _check->id());
  exchrateq.exec();
  if (exchrateq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, exchrateq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else if (exchrateq.first())
  {
    double rate = exchrateq.value("exchrate").toDouble();
    bool ok;
    rate = QInputDialog::getDouble(this, tr("Exchange Rate"),
                                   tr("New Rate:"),
                                   rate, 0, 100, 5, &ok);
    if ( !ok )
      return;
    
    exchrateq.prepare("UPDATE checkhead SET checkhead_alt_curr_rate=:exchrate "
                      "WHERE (checkhead_id=:checkhead_id);");
    exchrateq.bindValue(":checkhead_id", _check->id());
    exchrateq.bindValue(":exchrate", rate);
    exchrateq.exec();
    if (exchrateq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, exchrateq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void viewCheckRun::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;
  
  if(_metrics->boolean("AltCashExchangeRate"))
  {
    menuItem = pMenu->addAction(tr("Alternate Exchange Rate"), this, SLOT(sAltExchRate()));
    menuItem->setEnabled(_privileges->check("PostPayments"));
  }
}

void viewCheckRun::sHandleItemSelection()
{
  XTreeWidgetItem *selected = _check->currentItem();
  bool select = false;
  bool checkPrint = true;

  if (! selected)
  {
    _void->setEnabled(false);
    _delete->setEnabled(false);
    _replace->setEnabled(false);
    select = true;

    _edit->setEnabled(false);

    return;
  }

  if (selected->rawValue("checkhead_void").toBool())
  {
    _void->setEnabled(false);
    _delete->setEnabled(true);
    _replace->setEnabled(true);

    _edit->setEnabled(false);
  }
  else if (! selected->rawValue("checkhead_void").isNull() &&
           ! selected->rawValue("checkhead_void").toBool())
  {
    // This was not allowing voiding of ACH checks. No strong case could be
    // made to disallow this since ACH is manuall at this time. Should that
    // requirement change this is the original line
    //_void->setEnabled(selected->rawValue("checkhead_ach_batch").isNull());
    _void->setEnabled(true);
    _delete->setEnabled(false);
    _replace->setEnabled(false);
    select = selected->rawValue("checkhead_ach_batch").isNull();

    _edit->setEnabled(selected->rawValue("checkhead_misc").toBool() &&
                      ! selected->rawValue("checkhead_printed").toBool());
  }

  // Based on Bank Account settings allow/disallow posting of payment without
  // having printed the payment
  XSqlQuery bankRequiresPrint;
  bankRequiresPrint.prepare("SELECT bankaccnt_prnt_check AS result FROM bankaccnt WHERE bankaccnt_id =:bankaccnt_id;");
  bankRequiresPrint.bindValue(":bankaccnt_id", _bankaccnt->id());
  bankRequiresPrint.exec();
  if (bankRequiresPrint.first())
    checkPrint = bankRequiresPrint.value("result").toBool();
  
  QMenu * printMenu = new QMenu;
  if (select && checkPrint)
    printMenu->addAction(tr("Selected Payment..."), this, SLOT(sPrint()));
  if (_vendorgroup->isAll()  && checkPrint)
    printMenu->addAction(tr("Payment Run..."), this, SLOT(sPrintCheckRun()));
  printMenu->addAction(tr("Edit List"), this, SLOT(sPrintEditList()));
  _print->setMenu(printMenu); 

  QMenu * postMenu = new QMenu;
  if ((selected->rawValue("checkhead_printed").toBool() || !checkPrint) &&
      _privileges->check("PostPayments"))
    postMenu->addAction(tr("Selected Payment..."), this, SLOT(sPost()));
  if (_vendorgroup->isAll())
    postMenu->addAction(tr("All Payments..."), this, SLOT(sPostChecks()));
  _postCheck->setMenu(postMenu); 
}

void viewCheckRun::sFillList(int pBankaccntid)
{
  XSqlQuery viewFillList;
  if (pBankaccntid == _bankaccnt->id())
    sFillList();
}

void viewCheckRun::sFillList()
{
  bool checkPrint;
  // Based on Bank Account settings allow/disallow posting of payment without
  // having printed the payment
  XSqlQuery bankRequiresPrint;
  bankRequiresPrint.prepare("SELECT bankaccnt_prnt_check AS result FROM bankaccnt WHERE bankaccnt_id =:bankaccnt_id;");
  bankRequiresPrint.bindValue(":bankaccnt_id", _bankaccnt->id());
  bankRequiresPrint.exec();
  if (bankRequiresPrint.first())
    checkPrint = bankRequiresPrint.value("result").toBool();

  XSqlQuery viewFillList;
  QMenu * printMenu = new QMenu;
  if (_vendorgroup->isAll() && checkPrint)
    printMenu->addAction(tr("Payment Run..."), this, SLOT(sPrintCheckRun()));
  printMenu->addAction(tr("Edit List"),   this, SLOT(sPrintEditList()));
  _print->setMenu(printMenu);   

  QMenu * postMenu = new QMenu;
  if (_vendorgroup->isAll())
    postMenu->addAction(tr("Post All..."), this, SLOT(sPostChecks()));
  _postCheck->setMenu(postMenu); 
  
  MetaSQLQuery mql = mqlLoad("checkRegister", "detail");
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id());
  params.append("showTotal");
  params.append("newOnly");
  params.append("showDetail");
  params.append("voucher", tr("Voucher"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("creditMemo", tr("Credit Memo"));
  _vendorgroup->appendValue(params);
  viewFillList = mql.toQuery(params);
  _check->populate(viewFillList);
  if (viewFillList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, viewFillList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sPrintEditList()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 
  _vendorgroup->appendValue(params);
    
  orReport report("ViewAPCheckRunEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void viewCheckRun::sPrintCheckRun()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 

  printChecks newdlg(this, "", true);
  newdlg.set(params);
  newdlg.setWindowModality(Qt::WindowModal);
  newdlg.exec();
}

void viewCheckRun::sPostChecks()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 

  postChecks newdlg(this, "", true);
  newdlg.set(params);
  newdlg.setWindowModality(Qt::WindowModal);
  newdlg.exec();
}

void viewCheckRun::sHandleVendorGroup()
{
  _replaceAll->setEnabled(_vendorgroup->isAll());
}

void viewCheckRun::sPrepareCheckRun()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 

  prepareCheckRun newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}
