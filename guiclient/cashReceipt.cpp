/*
 * Common Public Attribution License Version 1.0.
 *
 * The contents of this file are subject to the Common Public Attribution
 * License Version 1.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla
 * Public License Version 1.1 but Sections 14 and 15 have been added to
 * cover use of software over a computer network and provide for limited
 * attribution for the Original Developer. In addition, Exhibit A has
 * been modified to be consistent with Exhibit B.
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite.
 *
 * The Original Developer is not the Initial Developer and is __________.
 * If left blank, the Original Developer is the Initial Developer.
 * The Initial Developer of the Original Code is OpenMFG, LLC,
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved.
 *
 * Contributor(s): ______________________.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the xTuple End-User License Agreeement (the xTuple License), in which
 * case the provisions of the xTuple License are applicable instead of
 * those above.  If you wish to allow use of your version of this file only
 * under the terms of the xTuple License and not to allow others to use
 * your version of this file under the CPAL, indicate your decision by
 * deleting the provisions above and replace them with the notice and other
 * provisions required by the xTuple License. If you do not delete the
 * provisions above, a recipient may use your version of this file under
 * either the CPAL or the xTuple License.
 *
 * EXHIBIT B.  Attribution Information
 *
 * Attribution Copyright Notice:
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 *
 * Attribution Phrase:
 * Powered by PostBooks, an open source solution from xTuple
 *
 * Attribution URL: www.xtuple.org
 * (to be included in the "Community" menu of the application if possible)
 *
 * Graphic Image as provided in the Covered Code, if any.
 * (online at www.xtuple.com/poweredby)
 *
 * Display of Attribution Information is required in Larger Works which
 * are defined in the CPAL as a work which combines Covered Code or
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "cashReceipt.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <stdlib.h>

#include "cashReceiptItem.h"
#include "cashReceiptMiscDistrib.h"
#include "creditCard.h"
#include "creditcardprocessor.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"

const char *_fundsTypes[] = { "C", "T", "M", "V", "A", "D", "R", "K", "W", "O" };
const int _fundsTypeCount = 9;

cashReceipt::cashReceipt(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustomerInfo(int)));
  connect(_received, SIGNAL(lostFocus()), this, SLOT(sUpdateBalance()));
  connect(_applyToBalance, SIGNAL(clicked()), this, SLOT(sApplyToBalance()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));
  connect(_applyLineBalance, SIGNAL(clicked()), this, SLOT(sApplyLineBalance()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_searchDocNum, SIGNAL(textChanged(const QString&)), this, SLOT(sSearchDocNumChanged()));
  connect(_applied, SIGNAL(valueChanged()), this, SLOT(sUpdateBalance()));
  connect(_miscDistribs, SIGNAL(valueChanged()), this, SLOT(sUpdateBalance()));
  connect(_received, SIGNAL(valueChanged()), this, SLOT(sUpdateBalance()));
  connect(_received, SIGNAL(idChanged(int)), this, SLOT(sFillApplyList()));
  connect(_received, SIGNAL(idChanged(int)), this, SLOT(sFillMiscList()));
  connect(_received, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(sFillApplyList()));
  connect(_received, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(sFillMiscList()));
  connect(_received, SIGNAL(idChanged(int)), this, SLOT(sChangeCurrency(int)));
  connect(_newCC, SIGNAL(clicked()), this, SLOT(sNewCreditCard()));
  connect(_editCC, SIGNAL(clicked()), this, SLOT(sEditCreditCard()));
  connect(_viewCC, SIGNAL(clicked()), this, SLOT(sViewCreditCard()));
  connect(_upCC, SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_downCC, SIGNAL(clicked()), this, SLOT(sMoveDown()));
  connect(_fundsType, SIGNAL(activated(int)), this, SLOT(setCreditCard()));

  QButtonGroup * bg = new QButtonGroup(this);
  bg->addButton(_balCreditMemo);
  bg->addButton(_balCustomerDeposit);

  _applied->clear();

  _cust->setAutoFocus(false);

  _CCCVV->setValidator(new QIntValidator(100, 9999, this));

  _bankaccnt->setType(XComboBox::ARBankAccounts);
  _salescat->setType(XComboBox::SalesCategories);

  _aropen->addColumn(tr("Doc. Type"), -1,              Qt::AlignCenter );
  _aropen->addColumn(tr("Doc. #"),    _orderColumn,    Qt::AlignCenter );
  _aropen->addColumn(tr("Ord. #"),    _orderColumn,    Qt::AlignCenter );
  _aropen->addColumn(tr("Doc. Date"), _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Due Date"),  _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Open"),      _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency());
  _aropen->addColumn(tr("Applied"),   _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency());
  _aropen->addColumn(tr("All Pending"),_moneyColumn,   Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency());

  _cashrcptmisc->addColumn(tr("Account #"), _itemColumn,     Qt::AlignCenter );
  _cashrcptmisc->addColumn(tr("Notes"),     -1,              Qt::AlignLeft   );
  _cashrcptmisc->addColumn(tr("Amount"),    _bigMoneyColumn, Qt::AlignRight  );

  _cc->addColumn(tr("Sequence"),        _itemColumn,  Qt::AlignLeft );
  _cc->addColumn(tr("Type"),            _itemColumn,  Qt::AlignLeft );
  _cc->addColumn(tr("Number"),          _itemColumn,  Qt::AlignRight );
  _cc->addColumn(tr("Active"),          _itemColumn,  Qt::AlignLeft );
  _cc->addColumn(tr("Name"),            _itemColumn,  Qt::AlignLeft );
  _cc->addColumn(tr("Expiration Date"), -1,           Qt::AlignLeft );

  if (!_metrics->boolean("CCAccept") && ! _privleges->check("ProcessCreditCards"))
    _tab->removeTab(_tab->indexOf(_creditCardTab));

  if(_metrics->boolean("HideApplyToBalance"))
    _applyToBalance->hide();

  if(_metrics->boolean("EnableCustomerDeposits"))
    _balCustomerDeposit->setChecked(true);
  else
  {
   _applyBalLit->hide();
   _balCreditMemo->hide();
   _balCustomerDeposit->hide();
  }

  _overapplied = false;
  _cashrcptid = -1;
}

cashReceipt::~cashReceipt()
{
  // no need to delete child widgets, Qt does it all for us
}

void cashReceipt::languageChange()
{
  retranslateUi(this);
}

enum SetResponse cashReceipt::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cashrcpt_id", &valid);
  if (valid)
  {
    _cashrcptid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('cashrcpt_cashrcpt_id_seq') AS cashrcpt_id;");
      if (q.first())
        _cashrcptid = q.value("cashrcpt_id").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _distDate->setDate(omfgThis->dbDate(), true);

      q.prepare( "INSERT INTO cashrcpt "
                 "( cashrcpt_id, cashrcpt_cust_id, cashrcpt_distdate, cashrcpt_amount,"
                 "  cashrcpt_fundstype, cashrcpt_bankaccnt_id ) "
                 "VALUES "
                 "( :cashrcpt_id, -1, CURRENT_DATE, 0.0,"
                 "  '', -1 );" );
      q.bindValue(":cashrcpt_id", _cashrcptid);
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _cust->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _tab->removeTab(_tab->indexOf(_creditCardTab));

      _cust->setReadOnly(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _tab->removeTab(_tab->indexOf(_creditCardTab));

      _cust->setReadOnly(TRUE);
      _received->setEnabled(FALSE);
      _fundsType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _bankaccnt->setEnabled(FALSE);
      _distDate->setEnabled(FALSE);
      _aropen->setEnabled(FALSE);
      _cashrcptmisc->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _applyToBalance->setEnabled(FALSE);
      _add->setEnabled(FALSE);
      _balCreditMemo->setEnabled(false);
      _balCustomerDeposit->setEnabled(false);
      _save->hide();
      _close->setText(tr("&Close"));
      _altAccnt->setEnabled(false);
      disconnect(_cashrcptmisc, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      disconnect(_cashrcptmisc, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

      _close->setFocus();
    }

    // if this cash receipt was by credit card cash then prevent changes
    _ccEdit = (_mode == cEdit) &&
	      (_origFunds == "A" || _origFunds == "D" ||
	       _origFunds == "M" || _origFunds == "V");

    if(_ccEdit)
    {
      _received->setEnabled(FALSE);
      _fundsType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _bankaccnt->setEnabled(FALSE);
      _distDate->setEnabled(FALSE);
    }

  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());

  return NoError;
}

void cashReceipt::sApplyToBalance()
{
  XSqlQuery applyToBal;
  applyToBal.prepare( "UPDATE cashrcpt "
             "SET cashrcpt_cust_id=:cust_id, cashrcpt_curr_id=:curr_id "
             "WHERE (cashrcpt_id=:cashrcpt_id);" );
  applyToBal.bindValue(":cust_id", _cust->id());
  applyToBal.bindValue(":cashrcpt_id", _cashrcptid);
  applyToBal.bindValue(":curr_id", _received->id());
  applyToBal.exec();

  applyToBal.prepare("SELECT applyCashReceiptToBalance(:cashrcpt_id, "
                     "             :amount, :curr_id) AS result;");
  applyToBal.bindValue(":cashrcpt_id", _cashrcptid);
  applyToBal.bindValue(":amount", _received->localValue());
  applyToBal.bindValue(":curr_id", _received->id());
  applyToBal.exec();
  if (applyToBal.lastError().type() != QSqlError::NoError)
      systemError(this, applyToBal.lastError().databaseText(), __FILE__, __LINE__);

  sFillApplyList();
}

void cashReceipt::sApply()
{
  bool update  = FALSE;
  QList<QTreeWidgetItem*> list = _aropen->selectedItems();
  XTreeWidgetItem *cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    ParameterList params;

    if(cursor->altId() != -1)
    {
      params.append("mode", "edit");
      params.append("cashrcptitem_id", cursor->altId());
    }
    else
      params.append("mode", "new");
    params.append("cashrcpt_id", _cashrcptid);
    params.append("aropen_id", cursor->id());
    params.append("curr_id", _received->id());

    cashReceiptItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
      update = TRUE;
  }

  if (update)
    sFillApplyList();
}

void cashReceipt::sApplyLineBalance()
{
  QList<QTreeWidgetItem*> list = _aropen->selectedItems();
  XTreeWidgetItem *cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    q.prepare( "SELECT applycashreceiptlinebalance(:cashrcpt_id,"
	       "            :cashrcptitem_aropen_id,:amount,:curr_id) AS result;" );
    q.bindValue(":cashrcpt_id", _cashrcptid);
    q.bindValue(":cashrcptitem_aropen_id", cursor->id());
    q.bindValue(":amount", _received->localValue());
    q.bindValue(":curr_id", _received->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("applyCashReceiptLineBalance", result), __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillApplyList();
}

void cashReceipt::sClear()
{
  QList<QTreeWidgetItem*> list = _aropen->selectedItems();
  XTreeWidgetItem *cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    q.prepare( "DELETE FROM cashrcptitem "
               " WHERE ((cashrcptitem_aropen_id=:aropen_id) "
               " AND (cashrcptitem_cashrcpt_id=:cashrcpt_id));" );
    q.bindValue(":cashrcpt_id", _cashrcptid);
    q.bindValue(":aropen_id", cursor->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillApplyList();
}

void cashReceipt::sAdd()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cashrcpt_id", _cashrcptid);
  params.append("curr_id", _received->id());
  params.append("effective", _received->effective());

  cashReceiptMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillMiscList();
}

void cashReceipt::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cashrcptmisc_id", _cashrcptmisc->id());
  params.append("curr_id", _received->id());
  params.append("effective", _received->effective());

  cashReceiptMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillMiscList();
}

void cashReceipt::sDelete()
{
  q.prepare( "DELETE FROM cashrcptmisc "
             "WHERE (cashrcptmisc_id=:cashrcptmisc_id);" );
  q.bindValue(":cashrcptmisc_id", _cashrcptmisc->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillMiscList();
}

void cashReceipt::close()
{
  if (_mode == cNew && _cashrcptid >= 0)
  {
    q.prepare("SELECT deleteCashRcpt(:cashrcpt_id) AS result;");
    q.bindValue(":cashrcpt_id", _cashrcptid);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("deleteCashRcpt", result));
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  QMainWindow::close();
}

void cashReceipt::sSave()
{
  sFillApplyList(); // one more time to double-check _overapplied
  if (_overapplied &&
      QMessageBox::question(this, tr("Overapplied?"),
                            tr("This Cash Receipt appears to apply too much to"
                               " at least one of the Open Items. Do you want "
                               "to save the current applications anyway?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  int _bankaccnt_curr_id = -1;
  QString _bankaccnt_currAbbr;
  q.prepare( "SELECT bankaccnt_curr_id, "
             "       currConcat(bankaccnt_curr_id) AS currAbbr "
             "  FROM bankaccnt "
             " WHERE (bankaccnt_id=:bankaccnt_id);");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
    _bankaccnt_curr_id = q.value("bankaccnt_curr_id").toInt();
    _bankaccnt_currAbbr = q.value("currAbbr").toString();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_received->id() != _bankaccnt_curr_id &&
      QMessageBox::question(this, tr("Cash Receipt Transaction Not In Bank Currency"),
                          tr("<p>This transaction is specified in %1 while the "
                             "Bank Account is specified in %2. Do you wish to "
                             "convert at the current Exchange Rate?"
			     "<p>If not, click NO "
                             "and change the Bank Account in the POST TO field.")
                          .arg(_received->currAbbr())
                          .arg(_bankaccnt_currAbbr),
                          QMessageBox::Yes|QMessageBox::Escape,
                          QMessageBox::No |QMessageBox::Default) != QMessageBox::Yes)
  {
    _received->setFocus();
    return;
  }

  QString fundsType = QString(*(_fundsTypes + _fundsType->currentItem()));
  if (!_ccEdit &&
      (fundsType == "A" || fundsType == "D" || fundsType == "M" || fundsType == "V"))
  {
    CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
    _save->setEnabled(false);
    int ccpayid = -1;
    QString ordernum = _docNumber->text().isEmpty() ?
		      QString::number(_cashrcptid) : _docNumber->text();
    int returnVal = cardproc->charge(_cc->id(),
				     _CCCVV->text().isEmpty() ? -1 : _CCCVV->text().toInt(),
				     _received->localValue(),
				     0, false, 0, 0,
				     _received->id(),
				     ordernum, ordernum, ccpayid,
				     QString("cashrcpt"), _cashrcptid);
    if (returnVal < 0)
      QMessageBox::critical(this, tr("Credit Card Processing Error"),
			    cardproc->errorMsg());
    else if (returnVal > 0)
      QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			   cardproc->errorMsg());
    else if (! cardproc->errorMsg().isEmpty())
      QMessageBox::information(this, tr("Credit Card Processing Note"),
			   cardproc->errorMsg());

    _save->setEnabled(true);
    if (returnVal < 0)
      return;

    // cardproc->credit() does an update but doesn't have all the required data
    q.prepare( "UPDATE cashrcpt "
	       "SET cashrcpt_distdate=:cashrcpt_distdate,"
	       "    cashrcpt_notes=:cashrcpt_notes || '\n' || cashrcpt_notes,"
	       "    cashrcpt_salescat_id=:cashrcpt_salescat_id, "
	       "    cashrcpt_usecustdeposit=:cashrcpt_usecustdeposit "
	       "WHERE (cashrcpt_id=:cashrcpt_id);" );
  }
  else
    q.prepare( "UPDATE cashrcpt "
	       "SET cashrcpt_cust_id=:cashrcpt_cust_id,"
	       "    cashrcpt_amount=:cashrcpt_amount,"
	       "    cashrcpt_fundstype=:cashrcpt_fundstype,"
	       "    cashrcpt_docnumber=:cashrcpt_docnumber,"
	       "    cashrcpt_bankaccnt_id=:cashrcpt_bankaccnt_id,"
	       "    cashrcpt_distdate=:cashrcpt_distdate,"
	       "    cashrcpt_notes=:cashrcpt_notes, "
	       "    cashrcpt_salescat_id=:cashrcpt_salescat_id, "
	       "    cashrcpt_curr_id=:curr_id,"
	       "    cashrcpt_usecustdeposit=:cashrcpt_usecustdeposit "
	       "WHERE (cashrcpt_id=:cashrcpt_id);" );

  q.bindValue(":cashrcpt_id", _cashrcptid);
  q.bindValue(":cashrcpt_cust_id", _cust->id());
  q.bindValue(":cashrcpt_amount", _received->localValue());
  q.bindValue(":cashrcpt_fundstype", QString(*(_fundsTypes + _fundsType->currentItem())));
  q.bindValue(":cashrcpt_docnumber", _docNumber->text());
  q.bindValue(":cashrcpt_bankaccnt_id", _bankaccnt->id());
  q.bindValue(":cashrcpt_distdate", _distDate->date());
  q.bindValue(":cashrcpt_notes", _notes->text().stripWhiteSpace());
  q.bindValue(":cashrcpt_usecustdeposit", QVariant(_balCustomerDeposit->isChecked(), 0));
  q.bindValue(":curr_id", _received->id());
  if(_altAccnt->isChecked())
    q.bindValue(":cashrcpt_salescat_id", _salescat->id());
  else
    q.bindValue(":cashrcpt_salescat_id", -1);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sCashReceiptsUpdated(_cashrcptid, TRUE);
  _cashrcptid = -1;

  close();
}

void cashReceipt::sPopulateCustomerInfo(int)
{
  if (_mode == cNew)
  {
    XSqlQuery cust;
    cust.prepare("SELECT cust_curr_id FROM cust WHERE cust_id = :cust_id;");
    cust.bindValue(":cust_id", _cust->id());
    cust.exec();
    if (cust.first())
      _received->setId(cust.value("cust_curr_id").toInt());
    else if (cust.lastError().type() != QSqlError::None)
    {
      systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillApplyList();
}

void cashReceipt::sFillApplyList()
{
  if (_cust->isValid())
  {
    _cust->setReadOnly(TRUE);

    _aropen->clear();
    MetaSQLQuery mql = mqlLoad(":/ar/aropenApplications.mql");
    ParameterList params;
    params.append("cashrcpt_id", _cashrcptid);
    params.append("cust_id",     _cust->id());
    params.append("debitMemo",   tr("D/M"));
    params.append("invoice",     tr("Invoice"));
    XSqlQuery apply;
    apply = mql.toQuery(params);
    XTreeWidgetItem *last = 0;
    _overapplied = false;
    while (apply.next())
    {
      last = new XTreeWidgetItem(_aropen, last,
                                 apply.value("aropen_id").toInt(),
                                 apply.value("alt_id").toInt(),
                                 apply.value("doctype"),
                                 apply.value("aropen_docnumber"),
                                 apply.value("aropen_ordernumber"),
                                 apply.value("f_docdate"),
                                 apply.value("f_duedate"),
                                 apply.value("f_balance"),
                                 apply.value("balance_curr"),
                                 apply.value("f_applied"),
                                 apply.value("applied_curr"),
                                 apply.value("f_pending"),
                                 apply.value("balance_curr"));
      if (apply.value("pending").toDouble() > apply.value("balance").toDouble())
      {
        last->setTextColor("red");
        _overapplied = true;
      }
    }
    if (apply.lastError().type() != QSqlError::None)
    {
      systemError(this, apply.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    apply.prepare( "SELECT SUM(COALESCE(cashrcptitem_amount, 0)) AS total "
               "FROM cashrcptitem "
               "WHERE (cashrcptitem_cashrcpt_id=:cashrcpt_id);" );
    apply.bindValue(":cashrcpt_id", _cashrcptid);
    apply.exec();
    if (apply.first())
      _applied->setLocalValue(apply.value("total").toDouble());
    else if (apply.lastError().type() != QSqlError::None)
    {
      systemError(this, apply.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  _received->setCurrencyEditable(_applied->isZero() && _miscDistribs->isZero());
}

void cashReceipt::sFillMiscList()
{
  XSqlQuery misc;
  misc.prepare( "SELECT cashrcptmisc_id, formatGLAccount(cashrcptmisc_accnt_id),"
             "       firstLine(cashrcptmisc_notes), "
             "       formatMoney(cashrcptmisc_amount)"
             "FROM cashrcptmisc "
             "WHERE (cashrcptmisc_cashrcpt_id=:cashrcpt_id);" );
  misc.bindValue(":cashrcpt_id", _cashrcptid);
  misc.exec();
  _cashrcptmisc->populate(misc);
  if (misc.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  misc.prepare( "SELECT SUM(cashrcptmisc_amount) AS total "
             "FROM cashrcptmisc "
             "WHERE (cashrcptmisc_cashrcpt_id=:cashrcpt_id);" );
  misc.bindValue(":cashrcpt_id", _cashrcptid);
  misc.exec();
  if (misc.first())
    _miscDistribs->setLocalValue(misc.value("total").toDouble());
  else if (misc.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _miscDistribs->clear();
  _received->setCurrencyEditable(_applied->isZero() && _miscDistribs->isZero());
}

void cashReceipt::sUpdateBalance()
{
  _balance->setLocalValue(_received->localValue() - _applied->localValue() -
                          _miscDistribs->localValue());
  if (!_balance->isZero())
    _balance->setPaletteForegroundColor(QColor("red"));
}

void cashReceipt::populate()
{
  q.prepare( "SELECT cashrcpt_cust_id, cashrcpt_amount, cashrcpt_curr_id, "
             "       cashrcpt_fundstype, cashrcpt_docnumber, cashrcpt_bankaccnt_id,"
             "       cashrcpt_distdate, cashrcpt_notes, cashrcpt_salescat_id,"
             "       cashrcpt_usecustdeposit "
             "FROM cashrcpt "
             "WHERE (cashrcpt_id=:cashrcpt_id);" );
  q.bindValue(":cashrcpt_id", _cashrcptid);
  q.exec();
  if (q.first())
  {
    _cust->setId(q.value("cashrcpt_cust_id").toInt());
    _received->set(q.value("cashrcpt_amount").toDouble(),
                   q.value("cashrcpt_curr_id").toInt(),
                   q.value("cashrcpt_distdate").toDate(), false);
    _docNumber->setText(q.value("cashrcpt_docnumber").toString());
    _bankaccnt->setId(q.value("cashrcpt_bankaccnt_id").toInt());
    _distDate->setDate(q.value("cashrcpt_distdate").toDate(), true);
    _notes->setText(q.value("cashrcpt_notes").toString());
    if(q.value("cashrcpt_salescat_id").toInt() != -1)
    {
      _altAccnt->setChecked(TRUE);
      _salescat->setId(q.value("cashrcpt_salescat_id").toInt());
    }
    if(q.value("cashrcpt_usecustdeposit").toBool())
      _balCustomerDeposit->setChecked(true);
    else
      _balCreditMemo->setChecked(true);

    for (int counter = 0; counter < _fundsType->count(); counter++)
      if (QString(q.value("cashrcpt_fundstype").toString()[0]) == _fundsTypes[counter])
        {
          _fundsType->setCurrentItem(counter);
        }

    _origFunds = q.value("cashrcpt_fundstype").toString();
    _cust->setId(q.value("cashrcpt_cust_id").toInt());

    sFillApplyList();
    sFillMiscList();
    setCreditCard();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void cashReceipt::sSearchDocNumChanged()
{
  QString sub = _searchDocNum->text().stripWhiteSpace();
  if(sub.isEmpty())
    return;

  QList<QTreeWidgetItem*> list = _aropen->findItems(sub, Qt::MatchFixedString|Qt::MatchCaseSensitive, 1);
  if(list.isEmpty())
    list = _aropen->findItems(sub, Qt::MatchFixedString|Qt::MatchStartsWith|Qt::MatchCaseSensitive, 1);

  if(!list.isEmpty())
  {
    _aropen->setCurrentItem(list.at(0));
    _aropen->scrollTo(_aropen->currentIndex());
  }
}

void cashReceipt::sChangeCurrency( int newId)
{
  if (_received->isEnabled())
  {
    XSqlQuery id;
    id.prepare( "UPDATE cashrcpt "
                "SET cashrcpt_curr_id=:curr_id "
                "WHERE (cashrcpt_id=:cashrcpt_id);" );
    id.bindValue(":cashrcpt_id", _cashrcptid);
    id.bindValue(":curr_id", newId);
    id.exec();
    if (id.lastError().type() != QSqlError::NoError)
      systemError(this, id.lastError().databaseText(), __FILE__, __LINE__);
  }
}

void cashReceipt::setCreditCard()
{
  q.prepare( "SELECT expireCreditCard(:cust_id, setbytea(:key)) AS result;");
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":key", omfgThis->_key);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("expireCreditCard", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }


  q.prepare( "SELECT ccard_id,"
             "       ccard_seq,"
             "       CASE WHEN (ccard_type='M') THEN :masterCard"
             "            WHEN (ccard_type='V') THEN :visa"
             "            WHEN (ccard_type='A') THEN :americanExpress"
             "            WHEN (ccard_type='D') THEN :discover"
             "            ELSE :other"
             "       END AS creditcard,"
             "       formatccnumber(decrypt(setbytea(ccard_number), setbytea(:key), 'bf')) AS ccard_number,"
             "       formatBoolYN(ccard_active), "
             "       formatbytea(decrypt(setbytea(ccard_name), setbytea(:key), 'bf')) AS ccard_name,"
             "       formatbytea(decrypt(setbytea(ccard_month_expired), setbytea(:key), 'bf')) ||  '-' ||formatbytea(decrypt(setbytea(ccard_year_expired), setbytea(:key), 'bf')) AS ccard_expired "
             "FROM ccard "
             "WHERE ((ccard_cust_id=:cust_id) "
             " AND   (ccard_type=:ccard_type)) "
             " AND   (ccard_active)"
             "ORDER BY ccard_seq;" );
  q.bindValue(":cust_id",    _cust->id());
  q.bindValue(":ccard_type", QString(*(_fundsTypes + _fundsType->currentItem())));
  q.bindValue(":masterCard", tr("MasterCard"));
  q.bindValue(":visa",       tr("VISA"));
  q.bindValue(":americanExpress", tr("American Express"));
  q.bindValue(":discover",   tr("Discover"));
  q.bindValue(":other",      tr("Other"));
  q.bindValue(":key",        omfgThis->_key);
  q.exec();
  _cc->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void cashReceipt::sNewCreditCard()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    setCreditCard();

}

void cashReceipt::sEditCreditCard()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", _cust->id());
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    setCreditCard();
}

void cashReceipt::sViewCreditCard()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", _cust->id());
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void cashReceipt::sMoveUp()
{
  q.prepare("SELECT moveCcardUp(:ccard_id) AS result;");
  q.bindValue(":ccard_id", _cc->id());
  q.exec();

  setCreditCard();
}

void cashReceipt::sMoveDown()
{
  q.prepare("SELECT moveCcardDown(:ccard_id) AS result;");
  q.bindValue(":ccard_id", _cc->id());
  q.exec();

  setCreditCard();
}
