/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCashReceipts.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <datecluster.h>
#include <openreports.h>
#include "arOpenItem.h"
#include "cashReceipt.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

dspCashReceipts::dspCashReceipts(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspCashReceipts", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Cash Receipts"));
  setListLabel(tr("Cash Receipts"));
  setReportName("CashReceipts");
  setMetaSQLOptions("cashReceipts", "detail");
  setNewVisible(true);
  setUseAltId(true);

  _fundsType->populate("SELECT fundstype_id, fundstype_name, fundstype_code FROM fundstype;");

  connect(_applications, SIGNAL(toggled(bool)), list(), SLOT(clear()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartDate(QDate().currentDate().addDays(-90));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Number"),      _orderColumn,    Qt::AlignLeft, true,  "cashrcpt_number" );
  list()->addColumn(tr("Source"),      _itemColumn,     Qt::AlignLeft,   true,  "source" );
  list()->addColumn(tr("Cust. #"),     _orderColumn,    Qt::AlignLeft, true,  "cust_number" );
  list()->addColumn(tr("Customer"),    -1,              Qt::AlignLeft,   true,  "cust_name"   );
  list()->addColumn(tr("Posted"),      _ynColumn,       Qt::AlignCenter, true,  "posted" );
  list()->addColumn(tr("Voided"),      _ynColumn,       Qt::AlignCenter, true,  "voided" );
  list()->addColumn(tr("Date"),        _dateColumn,     Qt::AlignCenter, true,  "postdate" );
  list()->addColumn(tr("Apply-To"),    -1,     Qt::AlignLeft, true,  "target" );
  list()->addColumn(tr("Amount"),      _moneyColumn, Qt::AlignRight,  true,  "applied"  );
  list()->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   true,  "currAbbr"   );
  list()->addColumn(tr("Base Amount"), _moneyColumn, Qt::AlignRight,  true,  "base_applied"  );

  newAction()->setEnabled(_privileges->check("MaintainCashReceipts"));
}

bool dspCashReceipts::setParams(ParameterList &pParams)
{
  if (!display::setParams(pParams)) {
    return false;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }

  _customerSelector->appendValue(pParams);
  _dates->appendValue(pParams);
  if (_fundsType->isValid())
    pParams.append("fundstype", _fundsType->code());
  pParams.append("creditMemo", tr("Credit Memo"));
  pParams.append("debitMemo", tr("Debit Memo"));
  pParams.append("cashdeposit", tr("Customer Deposit"));
  pParams.append("invoice", tr("Invoice"));
  pParams.append("cash", tr("Cash"));
  pParams.append("check", tr("Check"));
  pParams.append("certifiedCheck", tr("Cert. Check"));
  pParams.append("masterCard", tr("Master Card"));
  pParams.append("visa", tr("Visa"));
  pParams.append("americanExpress", tr("AmEx"));
  pParams.append("discoverCard", tr("Discover"));
  pParams.append("otherCreditCard", tr("Other C/C"));
  pParams.append("wireTransfer", tr("Wire Trans."));
  pParams.append("other", tr("Other"));
  pParams.append("unapplied", tr("Customer Deposit"));
  pParams.append("unposted", tr("Unposted"));
  pParams.append("voided", tr("Voided"));

  if (_applications->isChecked())
  {
    list()->hideColumn("cashrcpt_number");
    pParams.append("LegacyDisplayMode");
  }
  else
    list()->showColumn("cashrcpt_number");
  pParams.append("includeFormatted");

  return true;
}

void dspCashReceipts::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem = 0;

  if (list()->id() > -1)
  {
    // Cash Receipt
    if (!list()->currentItem()->rawValue("posted").toBool() && !list()->currentItem()->rawValue("voided").toBool())
    {
      menuItem = pMenu->addAction(tr("Edit Cash Receipt..."), this, SLOT(sEditCashrcpt()));
      menuItem->setEnabled(_privileges->check("MaintainCashReceipts"));
    }

    menuItem = pMenu->addAction(tr("View Cash Receipt..."), this, SLOT(sViewCashrcpt()));
    menuItem->setEnabled(_privileges->check("ViewCashReceipts") || _privileges->check("MaintainCashReceipts"));

    if (!list()->currentItem()->rawValue("voided").toBool())
    {
      if (!list()->currentItem()->rawValue("posted").toBool())
      {
        menuItem = pMenu->addAction(tr("Post Cash Receipt"), this, SLOT(sPostCashrcpt()));
        menuItem->setEnabled(_privileges->check("PostCashReceipts"));
      }
      else if (!list()->altId())
      {
        menuItem = pMenu->addAction(tr("Void Posted Cash Receipt"), this, SLOT(sReversePosted()));
        menuItem->setEnabled(_privileges->check("VoidPostedCashReceipts"));
      }
    }

    // Open Item
    if (list()->currentItem()->id("target") > -1 )
    {
      pMenu->addSeparator();
      menuItem = pMenu->addAction(tr("Edit Receivable Item..."), this, SLOT(sEditAropen()));
      menuItem->setEnabled(_privileges->check("EditAROpenItem"));
      menuItem = pMenu->addAction(tr("View Receivable Item..."), this, SLOT(sViewAropen()));
      menuItem->setEnabled(_privileges->check("ViewAROpenItems") || _privileges->check("EditAROpenItem"));
    }
  }
}

void dspCashReceipts::sEditAropen()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", list()->currentItem()->id("target"));
  arOpenItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCashReceipts::sViewAropen()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", list()->currentItem()->id("target"));
  arOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspCashReceipts::sNew()
{
  sNewCashrcpt();
}

void dspCashReceipts::sNewCashrcpt()
{
  ParameterList params;
  params.append("mode", "new");
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());

  cashReceipt *newdlg = new cashReceipt(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCashReceipts::sEditCashrcpt()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cashrcpt_id", list()->currentItem()->id("source"));

  cashReceipt *newdlg = new cashReceipt(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCashReceipts::sViewCashrcpt()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cashrcpt_id", list()->currentItem()->id("source"));

  cashReceipt *newdlg = new cashReceipt(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCashReceipts::sPostCashrcpt()
{
  XSqlQuery dspPostCashrcpt;
  int journalNumber = -1;

  XSqlQuery tx;
  tx.exec("BEGIN;");
  dspPostCashrcpt.exec("SELECT fetchJournalNumber('C/R') AS journalnumber;");
  if (dspPostCashrcpt.first())
    journalNumber = dspPostCashrcpt.value("journalnumber").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Cash Receipt"),
                                dspPostCashrcpt, __FILE__, __LINE__))
  {
      return;
  }

  dspPostCashrcpt.prepare("SELECT postCashReceipt(:cashrcpt_id, :journalNumber) AS result;");
  dspPostCashrcpt.bindValue(":cashrcpt_id", list()->currentItem()->id("source"));
  dspPostCashrcpt.bindValue(":journalNumber", journalNumber);
  dspPostCashrcpt.exec();
  if (dspPostCashrcpt.first())
  {
    int result = dspPostCashrcpt.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Cash Receipt"),
                               storedProcErrorLookup("postCashReceipt", result),
                               __FILE__, __LINE__);
      tx.exec("ROLLBACK;");
      return;
    }
  }
  else if (dspPostCashrcpt.lastError().type() != QSqlError::NoError)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Cash Receipt"),
                         dspPostCashrcpt, __FILE__, __LINE__);
    tx.exec("ROLLBACK;");
    return;
  }

  tx.exec("COMMIT;");
  omfgThis->sCashReceiptsUpdated(list()->currentItem()->id("source"), true);
  sFillList();
}

void dspCashReceipts::sReversePosted()
{
  XSqlQuery dspReversePosted;
  if (QMessageBox::warning(this, tr("Reverse Entire Posting?"),
                                  tr("<p>This will reverse all applications related "
                                     "to this cash receipt.  Are you sure you want "
                                     "to do this?"),
                                     QMessageBox::Yes | QMessageBox::Default,
                                     QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
  {
    dspReversePosted.prepare("SELECT reverseCashReceipt(:cashrcpt_id, fetchJournalNumber('C/R')) AS result;");
    dspReversePosted.bindValue(":cashrcpt_id", list()->currentItem()->id("source"));
    dspReversePosted.exec();
    if (dspReversePosted.first())
    {
      int result = dspReversePosted.value("result").toInt();
      if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Reversing Posted Cash Receipt"),
                               storedProcErrorLookup("reverseCashReceipt", result),
                               __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Reversing Posted Cash Receipt"),
                                  dspReversePosted, __FILE__, __LINE__))
    {
      return;
    }
    sFillList();
  }
}
