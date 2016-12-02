/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "cashReceiptsEditList.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "cashReceipt.h"
#include "errorReporter.h"
#include "getGLDistDate.h"
#include "storedProcErrorLookup.h"

cashReceiptsEditList::cashReceiptsEditList(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_cashrcpt, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,  SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,   SIGNAL(clicked()), this, SLOT(sView()));
  connect(_post,   SIGNAL(clicked()), this, SLOT(sPost()));

  _cashrcpt->addColumn(tr("Number"),           _orderColumn, Qt::AlignLeft,  true, "cashrcpt_number");
  _cashrcpt->addColumn(tr("Cust. #"),       _bigMoneyColumn, Qt::AlignLeft,  true, "cust_number");                                                                
  _cashrcpt->addColumn(tr("Name"),                       -1, Qt::AlignLeft,  true, "cust_name"); 
  _cashrcpt->addColumn(tr("Check/Doc. #"),     _orderColumn, Qt::AlignLeft,  true, "cashrcpt_docnumber");
  _cashrcpt->addColumn(tr("Bank Account"),     _orderColumn, Qt::AlignLeft,  true, "bankaccnt_name");
  _cashrcpt->addColumn(tr("Dist. Date"),        _dateColumn, Qt::AlignCenter,true, "cashrcpt_distdate");
  _cashrcpt->addColumn(tr("Funds Type"),    _bigMoneyColumn, Qt::AlignCenter,true, "cashrcpt_fundstype");
  _cashrcpt->addColumn(tr("Amount"),        _bigMoneyColumn, Qt::AlignRight, true, "cashrcpt_amount");
  _cashrcpt->addColumn(tr("Currency"),      _currencyColumn, Qt::AlignLeft,  true, "currabbr");

  if (_privileges->check("PostCashReceipts"))
    connect(_cashrcpt, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
    
  if (_privileges->check("MaintainCashReceipts"))
  {
    connect(_cashrcpt, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_cashrcpt, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_cashrcpt, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_cashrcpt, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(cashReceiptsUpdated(int, bool)), this, SLOT(sFillList()));

  sFillList();
}

cashReceiptsEditList::~cashReceiptsEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void cashReceiptsEditList::languageChange()
{
  retranslateUi(this);
}

void cashReceiptsEditList::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainCashReceipts"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainCashReceipts"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Post..."), this, SLOT(sPost()));
  menuItem->setEnabled(_privileges->check("PostCashReceipts"));
}

void cashReceiptsEditList::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void cashReceiptsEditList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cashrcpt_id", _cashrcpt->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void cashReceiptsEditList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cashrcpt_id", _cashrcpt->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void cashReceiptsEditList::sDelete()
{
  XSqlQuery cashDelete;
  cashDelete.prepare( "SELECT deleteCashRcpt(:cashrcpt_id) AS result;");
  cashDelete.bindValue(":cashrcpt_id", _cashrcpt->id());
  cashDelete.exec();
  if (cashDelete.first())
  {
    int result = cashDelete.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Cash Receipt Entry"),
                             storedProcErrorLookup("deleteCashRcpt", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Cash Receipt Entry"),
                                cashDelete, __FILE__, __LINE__))
  {
    return;
  }
  sFillList();
}

void cashReceiptsEditList::sPost()
{
  XSqlQuery cashPost;
  bool changeDate = false;
  QDate newDate = QDate();
  
  if (_privileges->check("ChangeCashRecvPostDate"))
  {
    getGLDistDate newdlg(this, "", true);
    newdlg.sSetDefaultLit(tr("Distribution Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }
  
  int journalNumber = -1;

  XSqlQuery tx;
  tx.exec("BEGIN;");
  cashPost.exec("SELECT fetchJournalNumber('C/R') AS journalnumber;");
  if (cashPost.first())
    journalNumber = cashPost.value("journalnumber").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Cash Receipt Entry"),
                                cashPost, __FILE__, __LINE__))
  {
    return;
  }

  XSqlQuery setDate;
  setDate.prepare("UPDATE cashrcpt SET cashrcpt_distdate=:distdate,"
                  "                    cashrcpt_applydate=CASE WHEN (cashrcpt_applydate < :distdate) THEN :distdate"
                  "                                            ELSE cashrcpt_applydate END "
                  "WHERE cashrcpt_id=:cashrcpt_id;");
  
  QList<XTreeWidgetItem*> selected = _cashrcpt->selectedItems();
  XTreeWidgetItem *cursor = 0;
  
  for (int i = 0; i < selected.size(); i++)
  {
    int id = ((XTreeWidgetItem*)(selected[i]))->id();
    
    if (changeDate)
    {
      setDate.bindValue(":distdate",    newDate);
      setDate.bindValue(":cashrcpt_id", id);
      setDate.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Changing Dist. Date"),
                           setDate, __FILE__, __LINE__);
    }
  }
  
  for (int i = 0; i < selected.size(); i++)
  {
    cursor = (XTreeWidgetItem*)selected.at(i);
    cashPost.prepare("SELECT postCashReceipt(:cashrcpt_id, :journalNumber) AS result;");
    cashPost.bindValue(":cashrcpt_id", cursor->id());
    cashPost.bindValue(":journalNumber", journalNumber);
    cashPost.exec();
    if (cashPost.first())
    {
      int result = cashPost.value("result").toInt();
      if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Cash Receipt Entry"),
                               storedProcErrorLookup("postCashReceipt", result),
                               __FILE__, __LINE__);
        tx.exec("ROLLBACK;");
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Cash Receipt Entry"),
                                  cashPost, __FILE__, __LINE__))
    {
      tx.exec("ROLLBACK");
      return;
    }
  }
  tx.exec("COMMIT;");
  omfgThis->sCashReceiptsUpdated(-1, true);
  sFillList();
}

bool cashReceiptsEditList::setParams(ParameterList &pParams)
{
  pParams.append("check", tr("Check"));
  pParams.append("certifiedCheck", tr("Certified Check"));
  pParams.append("masterCard", tr("Master Card"));
  pParams.append("visa", tr("Visa"));
  pParams.append("americanExpress", tr("American Express"));
  pParams.append("discoverCard", tr("Discover Card"));
  pParams.append("otherCreditCard", tr("Other Credit Card"));
  pParams.append("cash", tr("Cash"));
  pParams.append("wireTransfer", tr("Wire Transfer"));
  pParams.append("other", tr("Other"));

  return true;
}

void cashReceiptsEditList::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("CashReceiptsEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void cashReceiptsEditList::sFillList()
{
  XSqlQuery cashFillList;
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("unpostedCashReceipts", "detail");
  cashFillList = mql.toQuery(params);
  _cashrcpt->populate(cashFillList);
}
