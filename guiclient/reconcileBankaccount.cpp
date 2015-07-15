/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reconcileBankaccount.h"

#include <QApplication>
#include <QCursor>
#include <QMessageBox>
#include <QInputDialog>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>

#include "mqlutil.h"
#include "bankAdjustment.h"
#include "importData.h"
#include "toggleBankrecCleared.h"
#include "storedProcErrorLookup.h"

reconcileBankaccount::reconcileBankaccount(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

    connect(_addAdjustment, SIGNAL(clicked()),  this, SLOT(sAddAdjustment()));
    connect(_bankaccnt, SIGNAL(newID(int)),     this, SLOT(sBankaccntChanged()));
    connect(_cancel,	SIGNAL(clicked()),      this, SLOT(sCancel()));
    connect(_checks,    SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(sChecksToggleCleared()));
    connect(_endBal,	SIGNAL(editingFinished()),    this, SLOT(populate()));
    connect(_openBal,	SIGNAL(editingFinished()),    this, SLOT(populate()));
    connect(_receipts,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(sReceiptsToggleCleared()));
    connect(_reconcile,	SIGNAL(clicked()),      this, SLOT(sReconcile()));
    connect(_save,	    SIGNAL(clicked()),      this, SLOT(sSave()));
    connect(_import,	    SIGNAL(clicked()),      this, SLOT(sImport()));
    connect(_update,	SIGNAL(clicked()),      this, SLOT(populate()));
    connect(_startDate, SIGNAL(newDate(QDate)), this, SLOT(sDateChanged()));
    connect(_endDate,   SIGNAL(newDate(QDate)), this, SLOT(sDateChanged()));

    _receipts->addColumn(tr("Cleared"),       _ynColumn * 2, Qt::AlignCenter );
    _receipts->addColumn(tr("Date"),            _dateColumn, Qt::AlignCenter );
    _receipts->addColumn(tr("Doc. Type"),     _ynColumn * 2, Qt::AlignCenter );
    _receipts->addColumn(tr("Doc. Number"),     _itemColumn, Qt::AlignLeft   );
    _receipts->addColumn(tr("Notes"),                    -1, Qt::AlignLeft   );
    _receipts->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignCenter );
    _receipts->addColumn(tr("Exch. Rate"),  _bigMoneyColumn, Qt::AlignRight  );
    _receipts->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight  );
    _receipts->addColumn(tr("Amount"),      _bigMoneyColumn, Qt::AlignRight  );
    
    _checks->addColumn(tr("Cleared"),       _ynColumn * 2, Qt::AlignCenter , true, "cleared");
    _checks->addColumn(tr("Date"),            _dateColumn, Qt::AlignCenter , true, "transdate");
    _checks->addColumn(tr("Doc. Type"),     _ynColumn * 2, Qt::AlignCenter , true, "doc_type");
    _checks->addColumn(tr("Doc. Number"),     _itemColumn, Qt::AlignLeft   , true, "doc_number");
    _checks->addColumn(tr("Notes"),                    -1, Qt::AlignLeft   , true, "notes");
    _checks->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignCenter , true, "doc_curr");
    _checks->addColumn(tr("Exch. Rate"),  _bigMoneyColumn, Qt::AlignRight  , true, "doc_exchrate");
    _checks->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight  , true, "base_amount");
    _checks->addColumn(tr("Amount"),      _bigMoneyColumn, Qt::AlignRight  , true, "amount");

    _clearedReceipts->setPrecision(omfgThis->moneyVal());
    _clearedChecks->setPrecision(omfgThis->moneyVal());
    _endBal2->setPrecision(omfgThis->moneyVal());
    _clearBal->setPrecision(omfgThis->moneyVal());
    _diffBal->setPrecision(omfgThis->moneyVal());
    
    _bankrecid = -1;	// do this before _bankaccnt->populate()
    _bankaccntid = -1;	// do this before _bankaccnt->populate()
    _datesAreOK = false;
    
    _bankaccnt->populate("SELECT bankaccnt_id,"
			 "       (bankaccnt_name || '-' || bankaccnt_descrip) "
			 "FROM bankaccnt "
			 "ORDER BY bankaccnt_name;");
    _currency->setLabel(_currencyLit);

    _addAdjustment->setEnabled(_privileges->check("MaintainBankAdjustments"));

    if (_metrics->boolean("CashBasedTax"))
    {
      _allowEdit->setText(tr("Exchange Rate/Effective Date Edit"));
    }
  
    _import->setVisible(_metrics->boolean("ImportBankReconciliation"));
  
    connect(omfgThis, SIGNAL(bankAdjustmentsUpdated(int, bool)), this, SLOT(populate()));
    connect(omfgThis, SIGNAL(checksUpdated(int, int, bool)), this, SLOT(populate()));
    connect(omfgThis, SIGNAL(cashReceiptsUpdated(int, bool)), this, SLOT(populate()));
    connect(omfgThis, SIGNAL(glSeriesUpdated()), this, SLOT(populate()));
}

reconcileBankaccount::~reconcileBankaccount()
{
    // no need to delete child widgets, Qt does it all for us
}

void reconcileBankaccount::languageChange()
{
    retranslateUi(this);
}

void reconcileBankaccount::sCancel()
{
  XSqlQuery reconcileCancel;
  if(_bankrecid != -1)
  {
    reconcileCancel.prepare("SELECT count(*) AS num"
	      "  FROM bankrec"
	      " WHERE (bankrec_id=:bankrecid); ");
    reconcileCancel.bindValue(":bankrecid", _bankrecid);
    reconcileCancel.exec();
    if (reconcileCancel.first() && reconcileCancel.value("num").toInt() > 0)
    {
      if (QMessageBox::question(this, tr("Cancel Bank Reconciliation?"),
				tr("<p>Are you sure you want to Cancel this Bank "
				   "Reconciliation and delete all of the Cleared "
				   "notations for this time period?"),
				 QMessageBox::Yes,
				 QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
	return;
      }

      reconcileCancel.prepare( "SELECT deleteBankReconciliation(:bankrecid) AS result;" );
      reconcileCancel.bindValue(":bankrecid", _bankrecid);
      reconcileCancel.exec();
      if (reconcileCancel.first())
      {
	int result = reconcileCancel.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("deleteBankReconciliation", result),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (reconcileCancel.lastError().type() != QSqlError::NoError)
      {
	systemError(this, reconcileCancel.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }
  close();
}

bool reconcileBankaccount::sSave(bool closeWhenDone)
{
  XSqlQuery reconcileSave;
  reconcileSave.prepare("SELECT count(*) AS num"
            "  FROM bankrec"
            " WHERE (bankrec_id=:bankrecid); ");
  reconcileSave.bindValue(":bankrecid", _bankrecid);
  reconcileSave.exec();
  if (reconcileSave.first() && reconcileSave.value("num").toInt() > 0)
    reconcileSave.prepare("UPDATE bankrec"
              "   SET bankrec_bankaccnt_id=:bankaccntid,"
              "       bankrec_opendate=:startDate,"
              "       bankrec_enddate=:endDate,"
              "       bankrec_openbal=:openbal,"
              "       bankrec_endbal=:endbal "
              " WHERE (bankrec_id=:bankrecid); ");
  else if (reconcileSave.value("num").toInt() == 0)
    reconcileSave.prepare("INSERT INTO bankrec "
              "(bankrec_id, bankrec_bankaccnt_id,"
              " bankrec_opendate, bankrec_enddate,"
              " bankrec_openbal, bankrec_endbal) "
              "VALUES "
              "(:bankrecid, :bankaccntid,"
              " :startDate, :endDate,"
              " :openbal, :endbal); ");
  else if (reconcileSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, reconcileSave.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  reconcileSave.bindValue(":bankrecid", _bankrecid);
  reconcileSave.bindValue(":bankaccntid", _bankaccntid);
  reconcileSave.bindValue(":startDate", _startDate->date());
  reconcileSave.bindValue(":endDate", _endDate->date());
  reconcileSave.bindValue(":openbal", _openBal->localValue());
  reconcileSave.bindValue(":endbal", _endBal->localValue());
  reconcileSave.exec();
  if (reconcileSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, tr("<p>There was an error creating records to reconcile "
			 "this account: <br><pre>%1</pre>")
			.arg(reconcileSave.lastError().databaseText()), __FILE__, __LINE__);
    return false;
  }

  if (closeWhenDone)
    return close();

  return true;
}

void reconcileBankaccount::sReconcile()
{
  XSqlQuery reconcileReconcile;
  if(_bankrecid == -1)
  {
    QMessageBox::critical( this, tr("Cannot Reconcile Account"),
      tr("<p>There was an error trying to reconcile this account. "
         "Please contact your Systems Administrator.") );
    return;
  }

  if (!_startDate->isValid())
  {
    QMessageBox::warning( this, tr("Missing Opening Date"),
      tr("<p>No Opening Date was specified for this reconciliation. Please specify an Opening Date.") );
    _startDate->setFocus();
    return;
  }

  if (!_endDate->isValid())
  {
    QMessageBox::warning( this, tr("Missing Ending Date"),
      tr("<p>No Ending Date was specified for this reconciliation. Please specify an Ending Date.") );
    _endDate->setFocus();
    return;
  }

  if (_endDate->date() < _startDate->date())
  {
    QMessageBox::warning( this, tr("Invalid End Date"),
                           tr("The end date cannot be earlier than the start date.") );
    _endDate->setFocus();
    return;
  }

  if(!_datesAreOK)
  {
    QMessageBox::critical( this, tr("Dates already reconciled"),
                tr("The date range you have entered already has "
                   "reconciled dates in it. Please choose a different "
                   "date range.") );
    _startDate->setFocus();
    _datesAreOK = false;
    return;
  }

  double begBal = _openBal->localValue();
  double endBal = _endBal->localValue();

  // calculate cleared balance
  MetaSQLQuery mbal = mqlLoad("bankrec", "clearedbalance");
  ParameterList params;
  params.append("bankaccntid", _bankaccnt->id());
  params.append("bankrecid", _bankrecid);
  params.append("endBal", endBal);
  params.append("begBal", begBal);
  params.append("curr_id",   _currency->id());
  params.append("effective", _startDate->date());
  params.append("expires",   _endDate->date());
  XSqlQuery bal = mbal.toQuery(params);
  if(!bal.first())
  {
    systemError(this, bal.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(bal.value("diff_value").toDouble() != 0.0)
  {
    QMessageBox::critical( this, tr("Balances Do Not Match"),
      tr("The cleared amounts do not balance with the specified\n"
         "beginning and ending balances.\n"
         "Please correct this before continuing.") );
    return;
  }

  if (! sSave(false))
    return;

  reconcileReconcile.prepare("SELECT postBankReconciliation(:bankrecid) AS result;");
  reconcileReconcile.bindValue(":bankrecid", _bankrecid);
  reconcileReconcile.exec();
  if (reconcileReconcile.first())
  {
    int result = reconcileReconcile.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("postBankReconciliation", result),
		  __FILE__, __LINE__);
      return;
    }
    _bankrecid = -1;
    close();
  }
  else if (reconcileReconcile.lastError().type() != QSqlError::NoError)
  {
    systemError(this, reconcileReconcile.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

/* 
   Note that the SELECTs here are UNIONs of the gltrans table (in the base
   currency), sltrans table (in the base currency) and the bankadj table
   (in the bank account's currency).
*/
void reconcileBankaccount::populate()
{
  qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

  double begBal = _openBal->localValue();
  double endBal = _endBal->localValue();

  int currid = -1;

  ParameterList params;
  params.append("bankaccntid", _bankaccnt->id());
  params.append("bankrecid", _bankrecid);

  // fill receipts list
  currid = _receipts->id();
  _receipts->clear();
  MetaSQLQuery mrcp = mqlLoad("bankrec", "receipts");
  XSqlQuery rcp = mrcp.toQuery(params);
  if (rcp.lastError().type() != QSqlError::NoError)
  {
    systemError(this, rcp.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  int jrnlnum = 0;
  XTreeWidgetItem * parent = 0;
  XTreeWidgetItem * lastChild = 0;
  XTreeWidgetItem * last = 0;
  bool cleared = true;
  double amount = 0.0;
  bool amountNull = true;
  while (rcp.next())
  {
    if(rcp.value("use").toString() == "C/R")
    {
      if(rcp.value("jrnlnum").toInt() != jrnlnum || (0 == parent))
      {
        if(parent != 0)
        {
          parent->setText(0, (cleared ? tr("Yes") : tr("No")));
          parent->setText(8, amountNull ? tr("?????") : formatMoney(amount));
        }
        jrnlnum = rcp.value("jrnlnum").toInt();
        last = new XTreeWidgetItem( _receipts, last,
          jrnlnum, 9, "", formatDate(rcp.value("f_jrnldate").toDate()), tr("JS"), rcp.value("jrnlnum"));
        parent = last;
        cleared = true;
        amount = 0.0;
	amountNull = true;
        lastChild = 0;
      }
      cleared = (cleared && rcp.value("cleared").toBool());
      amount += rcp.value("amount").toDouble();
      amountNull = rcp.value("amount").isNull();
      
      lastChild = new XTreeWidgetItem( parent, lastChild,
        rcp.value("id").toInt(), rcp.value("altid").toInt(),
        (rcp.value("cleared").toBool() ? tr("Yes") : tr("No")),
        formatDate(rcp.value("f_date").toDate()), rcp.value("doc_type"), rcp.value("docnumber"),
        rcp.value("notes"),
        rcp.value("doc_curr"),
        rcp.value("doc_exchrate").isNull() ? tr("?????") : formatNumber(rcp.value("doc_exchrate").toDouble(), 6), // 6 dp to match bankrec-receipts metasql
        rcp.value("base_amount").isNull() ? tr("?????") : formatMoney(rcp.value("base_amount").toDouble()),
        rcp.value("amount").isNull() ? tr("?????") : formatMoney(rcp.value("amount").toDouble()) );
    }
    else
    {
      if(parent != 0)
      {
        parent->setText(0, (cleared ? tr("Yes") : tr("No")));
        parent->setText(8, formatMoney(amount));
      }
      parent = 0;
      cleared = true;
      amount = 0.0;
      amountNull = true;
      lastChild = 0;
      last = new XTreeWidgetItem( _receipts, last,
        rcp.value("id").toInt(), rcp.value("altid").toInt(),
        (rcp.value("cleared").toBool() ? tr("Yes") : tr("No")),
        formatDate(rcp.value("f_date").toDate()), rcp.value("doc_type"), rcp.value("docnumber"),
        rcp.value("notes"),
        rcp.value("doc_curr"),
        rcp.value("doc_exchrate").isNull() ? tr("?????") : formatNumber(rcp.value("doc_exchrate").toDouble(), 6), // 6 dp to match bankrec-receipts metasql
        rcp.value("base_amount").isNull() ? tr("?????") : formatMoney(rcp.value("base_amount").toDouble()),
        rcp.value("amount").isNull() ? tr("?????") : formatMoney(rcp.value("amount").toDouble()) );
    }
  }
  if(parent != 0)
  {
    parent->setText(0, (cleared ? tr("Yes") : tr("No")));
    parent->setText(8, amountNull ? tr("?????") : formatMoney(amount));
  }

  if(currid != -1)
    _receipts->setCurrentItem(_receipts->topLevelItem(currid));
  if(_receipts->currentItem())
    _receipts->scrollToItem(_receipts->currentItem());

  // fill checks list
  currid = _checks->id();
  _checks->clear();
  MetaSQLQuery mchk = mqlLoad("bankrec", "checks");
  XSqlQuery chk = mchk.toQuery(params);
  if (chk.lastError().type() != QSqlError::NoError)
  {
    systemError(this, chk.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _checks->populate(chk, true);

  if(currid != -1)
    _checks->setCurrentItem(_checks->topLevelItem(currid));
  if(_checks->currentItem())
    _checks->scrollToItem(_checks->currentItem());

  params.append("summary", true);

  // fill receipts cleared value
  rcp = mrcp.toQuery(params);
  if (rcp.first())
    _clearedReceipts->setDouble(rcp.value("cleared_amount").toDouble());
  else if (rcp.lastError().type() != QSqlError::NoError)
  {
    systemError(this, rcp.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // fill checks cleared value
  chk = mchk.toQuery(params);
  if (chk.first())
    _clearedChecks->setDouble(chk.value("cleared_amount").toDouble());
  else if (chk.lastError().type() != QSqlError::NoError)
  {
    systemError(this, chk.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // calculate cleared balance
  MetaSQLQuery mbal = mqlLoad("bankrec", "clearedbalance");
  params.append("endBal", endBal);
  params.append("begBal", begBal);
  params.append("curr_id",   _currency->id());
  params.append("effective", _startDate->date());
  params.append("expires",   _endDate->date());
  XSqlQuery bal = mbal.toQuery(params);
  bool enableRec = false;
  if(bal.first())
  {
    _clearBal->setDouble(bal.value("cleared_amount").toDouble());
    _endBal2->setDouble(bal.value("end_amount").toDouble());
    _diffBal->setDouble(bal.value("diff_amount").toDouble());

    QString stylesheet;

    if(bal.value("diff_value").toDouble() == 0.0)
    {
      if(_startDate->isValid() && _endDate->isValid())
        enableRec = true;
    }
    else
      stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());

    _diffBal->setStyleSheet(stylesheet);
  }
  else if (bal.lastError().type() != QSqlError::NoError)
  {
    systemError(this, bal.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  //_reconcile->setEnabled(enableRec);

  qApp->restoreOverrideCursor();
}

void reconcileBankaccount::sImport()
{
  XSqlQuery reconcileImport;

  // set the metric
  reconcileImport.prepare("SELECT setMetric('ImportBankRecId', :bankrecid::TEXT) AS result; ");
  reconcileImport.bindValue(":bankrecid", _bankrecid);
  reconcileImport.exec();
  if (reconcileImport.lastError().type() != QSqlError::NoError)
  {
    systemError(this, reconcileImport.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // import the reconciliation file
  importData *newdlg = new importData();
  newdlg->setWindowModality(Qt::WindowModal);
  omfgThis->handleNewWindow(newdlg);
}

void reconcileBankaccount::sAddAdjustment()
{
  ParameterList params;
  params.append("mode", "new");

  params.append("bankaccnt_id", _bankaccnt->id());

  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->setWindowModality(Qt::WindowModal);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void reconcileBankaccount::sReceiptsToggleCleared()
{
  XSqlQuery reconcileReceiptsToggleCleared;
  XTreeWidgetItem *item = (XTreeWidgetItem*)_receipts->currentItem();
  XTreeWidgetItem *child = 0;
  bool setto = true;

  if(0 == item)
    return;

  _receipts->scrollToItem(item);

  if(item->altId() == 9)
  {
    setto = item->text(0) == tr("No");
    for (int i = 0; i < item->childCount(); i++)
    {
      child = item->child(i);
      if(child->text(0) != (setto ? tr("Yes") : tr("No")))
      {
        double rate = QLocale().toDouble(child->text(6));
        double amount = QLocale().toDouble(child->text(7));

        if (_allowEdit->isChecked() && child->text(0) != tr("Yes"))
        {
          ParameterList params;
          params.append("transtype", "receipt");
          params.append("bankaccntid", _bankaccnt->id());
          params.append("bankrecid", _bankrecid);
          params.append("sourceid", child->id());
          if(child->altId()==1)
            params.append("source", "GL");
          else if(child->altId()==2)
            params.append("source", "SL");
          else if(child->altId()==3)
            params.append("source", "AD");
          toggleBankrecCleared newdlg(this, "", true);
          newdlg.set(params);
          newdlg.exec();
        }
        else
        {
          reconcileReceiptsToggleCleared.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid, :currrate, :baseamount) AS cleared");
          reconcileReceiptsToggleCleared.bindValue(":bankrecid", _bankrecid);
          reconcileReceiptsToggleCleared.bindValue(":sourceid", child->id());
          if(child->altId()==1)
            reconcileReceiptsToggleCleared.bindValue(":source", "GL");
          else if(child->altId()==2)
            reconcileReceiptsToggleCleared.bindValue(":source", "SL");
          else if(child->altId()==3)
            reconcileReceiptsToggleCleared.bindValue(":source", "AD");
          reconcileReceiptsToggleCleared.bindValue(":currrate", rate);
          reconcileReceiptsToggleCleared.bindValue(":baseamount", amount);
          reconcileReceiptsToggleCleared.exec();
          if(reconcileReceiptsToggleCleared.first())
            child->setText(0, (reconcileReceiptsToggleCleared.value("cleared").toBool() ? tr("Yes") : tr("No") ));
          else if (reconcileReceiptsToggleCleared.lastError().type() != QSqlError::NoError)
          {
            systemError(this, reconcileReceiptsToggleCleared.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
        }
      }
    }
    item->setText(0, (setto ? tr("Yes") : tr("No")));
    populate();
  }
  else
  {
    double rate = QLocale().toDouble(item->text(6));
    double amount = QLocale().toDouble(item->text(7));
    
    if (_allowEdit->isChecked() && item->text(0) != tr("Yes"))
    {
      ParameterList params;
      params.append("transtype", "receipt");
      params.append("bankaccntid", _bankaccnt->id());
      params.append("bankrecid", _bankrecid);
      params.append("sourceid", item->id());
      if(item->altId()==1)
        params.append("source", "GL");
      else if(item->altId()==2)
        params.append("source", "SL");
      else if(item->altId()==3)
        params.append("source", "AD");
      toggleBankrecCleared newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
      populate();
    }
    else
    {
      reconcileReceiptsToggleCleared.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid, :currrate, :baseamount) AS cleared");
      reconcileReceiptsToggleCleared.bindValue(":bankrecid", _bankrecid);
      reconcileReceiptsToggleCleared.bindValue(":sourceid", item->id());
      if(item->altId()==1)
        reconcileReceiptsToggleCleared.bindValue(":source", "GL");
      else if(item->altId()==2)
        reconcileReceiptsToggleCleared.bindValue(":source", "SL");
      else if(item->altId()==3)
        reconcileReceiptsToggleCleared.bindValue(":source", "AD");
      reconcileReceiptsToggleCleared.bindValue(":currrate", rate);
      reconcileReceiptsToggleCleared.bindValue(":baseamount", amount);
      reconcileReceiptsToggleCleared.exec();
      if(reconcileReceiptsToggleCleared.first())
      {
        item->setText(0, (reconcileReceiptsToggleCleared.value("cleared").toBool() ? tr("Yes") : tr("No") ));

        item = (XTreeWidgetItem*)item->QTreeWidgetItem::parent();
        if(item != 0 && item->altId() == 9)
        {
          setto = true;
          for (int i = 0; i < item->childCount(); i++)
          {
            setto = (setto && (item->child(i)->text(0) == tr("Yes")));
          }
          item->setText(0, (setto ? tr("Yes") : tr("No")));
          populate();
        }
      }
      else
      {
        populate();
        if (reconcileReceiptsToggleCleared.lastError().type() != QSqlError::NoError)
        {
          systemError(this, reconcileReceiptsToggleCleared.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
      }
    }
  }
}

void reconcileBankaccount::sChecksToggleCleared()
{
  XSqlQuery reconcileChecksToggleCleared;
  XTreeWidgetItem *item = (XTreeWidgetItem*)_checks->currentItem();

  if(0 == item)
    return;

  _checks->scrollToItem(item);

  double rate = item->rawValue("doc_exchrate").toDouble();
  double amount = item->rawValue("base_amount").toDouble();
  
  if (_allowEdit->isChecked() && item->text(0) != tr("Yes"))
  {
    ParameterList params;
    params.append("transtype", "check");
    params.append("bankaccntid", _bankaccnt->id());
    params.append("bankrecid", _bankrecid);
    params.append("sourceid", item->id());
    if(item->altId()==1)
      params.append("source", "GL");
    else if(item->altId()==2)
      params.append("source", "SL");
    else if(item->altId()==3)
      params.append("source", "AD");
    toggleBankrecCleared newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
    populate();
  }
  else
  {
    reconcileChecksToggleCleared.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid, :currrate, :baseamount) AS cleared");
    reconcileChecksToggleCleared.bindValue(":bankrecid", _bankrecid);
    reconcileChecksToggleCleared.bindValue(":sourceid", item->id());
    if(item->altId()==1)
      reconcileChecksToggleCleared.bindValue(":source", "GL");
    else if(item->altId()==2)
      reconcileChecksToggleCleared.bindValue(":source", "SL");
    else if(item->altId()==3)
      reconcileChecksToggleCleared.bindValue(":source", "AD");
    reconcileChecksToggleCleared.bindValue(":currrate", rate);
    reconcileChecksToggleCleared.bindValue(":baseamount", amount);
    reconcileChecksToggleCleared.exec();
    if(reconcileChecksToggleCleared.first())
    {
      item->setText(0, (reconcileChecksToggleCleared.value("cleared").toBool() ? tr("Yes") : tr("No") ));
      populate();
    }
    else
    {
      populate();
      if (reconcileChecksToggleCleared.lastError().type() != QSqlError::NoError)
      {
        systemError(this, reconcileChecksToggleCleared.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }
}

void reconcileBankaccount::sBankaccntChanged()
{
  XSqlQuery reconcileBankaccntChanged;
  if(_bankrecid != -1)
  {
    reconcileBankaccntChanged.prepare("SELECT count(*) AS num"
	          "  FROM bankrecitem"
	          " WHERE (bankrecitem_bankrec_id=:bankrecid); ");
    reconcileBankaccntChanged.bindValue(":bankrecid", _bankrecid);
    reconcileBankaccntChanged.exec();
    if (reconcileBankaccntChanged.first() && reconcileBankaccntChanged.value("num").toInt() > 0)
    {
      if (QMessageBox::question(this, tr("Save Bank Reconciliation?"),
				                      tr("<p>Do you want to save this Bank Reconciliation?"),
				                QMessageBox::No,
				                QMessageBox::Yes | QMessageBox::Default) == QMessageBox::Yes)
      {
	    sSave(false);
      }
      else
	  {
        reconcileBankaccntChanged.prepare( "SELECT deleteBankReconciliation(:bankrecid) AS result;" );
        reconcileBankaccntChanged.bindValue(":bankrecid", _bankrecid);
        reconcileBankaccntChanged.exec();
        if (reconcileBankaccntChanged.first())
        {
	      int result = reconcileBankaccntChanged.value("result").toInt();
	      if (result < 0)
	      {
	        systemError(this, storedProcErrorLookup("deleteBankReconciliation", result),
		                __FILE__, __LINE__);
	        return;
	      }
        }
        else if (reconcileBankaccntChanged.lastError().type() != QSqlError::NoError)
        {
	      systemError(this, reconcileBankaccntChanged.lastError().databaseText(), __FILE__, __LINE__);
	      return;
        }
      }
	}
  }

  _bankaccntid = _bankaccnt->id();
  XSqlQuery accntq;
  accntq.prepare("SELECT bankaccnt_curr_id "
            "FROM bankaccnt WHERE bankaccnt_id = :accntId;");
  accntq.bindValue(":accntId", _bankaccnt->id());
  accntq.exec();
  if (accntq.first())
    _currency->setId(accntq.value("bankaccnt_curr_id").toInt());
  else if (accntq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, accntq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accntq.prepare("SELECT * FROM bankrec "
		 "WHERE ((bankrec_bankaccnt_id=:accntId)"
		 "  AND  (NOT bankrec_posted));");
  accntq.bindValue(":accntId", _bankaccnt->id());
  accntq.exec();
  if (accntq.first())
  {
    _bankrecid = accntq.value("bankrec_id").toInt();
    _startDate->setDate(accntq.value("bankrec_opendate").toDate(), true);
    _endDate->setDate(accntq.value("bankrec_enddate").toDate(), true);
    _openBal->setLocalValue(accntq.value("bankrec_openbal").toDouble());
    _endBal->setLocalValue(accntq.value("bankrec_endbal").toDouble());
  }
  else if (accntq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, accntq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    accntq.prepare("SELECT NEXTVAL('bankrec_bankrec_id_seq') AS bankrec_id");
    accntq.exec();
    if (accntq.first())
      _bankrecid = accntq.value("bankrec_id").toInt();
    else if (accntq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, accntq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    accntq.prepare("SELECT bankrec_enddate + 1 AS startdate, "
                   " bankrec_endbal AS openbal "
                   "FROM bankrec "
                   "WHERE (bankrec_bankaccnt_id=:accntId) "
                   "ORDER BY bankrec_enddate DESC "
                   "LIMIT 1");
    accntq.bindValue(":accntId", _bankaccnt->id());
    accntq.exec();
    if (accntq.first())
    {
      _startDate->setDate(accntq.value("startdate").toDate());
      _openBal->setLocalValue(accntq.value("openbal").toDouble());
    }
    else
    {
      _startDate->clear();
      _openBal->clear();
    }
    if (accntq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, accntq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  populate();
}

void reconcileBankaccount::sDateChanged()
{
  XSqlQuery reconcileDateChanged;
  reconcileDateChanged.prepare("SELECT true AS reconciled "
            "FROM bankrec "
            "WHERE ((bankrec_bankaccnt_id = :bankaccnt_id) "
            "AND (bankrec_posted) "
            "AND (bankrec_opendate <= :end_date) "
            "AND (bankrec_enddate >= :start_date)) "
            "GROUP BY bankrec_bankaccnt_id");
  reconcileDateChanged.bindValue(":bankaccnt_id", _bankaccnt->id());
  reconcileDateChanged.bindValue(":end_date", _endDate->date().toString(Qt::ISODate));
  reconcileDateChanged.bindValue(":start_date", _startDate->date().toString(Qt::ISODate));

  reconcileDateChanged.exec();
  if(reconcileDateChanged.first())
  {
    QMessageBox::critical( this, tr("Dates already reconciled"),
	        tr("The date range you have entered already has "
	           "reconciled dates in it. Please choose a different "
	           "date range.") );
    _startDate->setFocus();
    _datesAreOK = false;
    return;
  }
  else if(reconcileDateChanged.lastError().type() != QSqlError::NoError)
  {
    systemError(this, reconcileDateChanged.lastError().databaseText(), __FILE__, __LINE__);
    _datesAreOK = false;
	return;
  }
  else
  {
    if (! sSave(false))
      return;
    
    _datesAreOK = true;
  }
}

void reconcileBankaccount::openReconcileBankaccount(int pBankaccntid, QWidget *parent)
{
  // Check for a window.
  if (pBankaccntid == -1)
  {
    QWidgetList list = omfgThis->windowList();
    for (int i = 0; i < list.size(); i++)
    {
      QWidget *w = list.at(i);
      if (QString::compare(w->objectName(), "reconcileBankaccount")==0)
      {
        w->setFocus();
        if (omfgThis->showTopLevel())
        {
          w->raise();
          w->activateWindow();
        }
        return;
      }
    }
  }
  
  // If none found then create one.
  ParameterList params;
  if (pBankaccntid != -1)
    params.append("bankaccnt_id", pBankaccntid);
  
  reconcileBankaccount *newdlg = new reconcileBankaccount(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

