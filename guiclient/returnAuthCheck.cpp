/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "returnAuthCheck.h"
#include "storedProcErrorLookup.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

returnAuthCheck::returnAuthCheck(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_bankaccnt, SIGNAL(newID(int)),    this, SLOT(sPopulateBankInfo(int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _date->setDate(omfgThis->dbDate(), true);

  _bankaccnt->setAllowNull(true);
  _bankaccnt->setType(XComboBox::APBankAccounts);
  _cmheadcurrid = CurrDisplay::baseId();
}

returnAuthCheck::~returnAuthCheck()
{
  // no need to delete child widgets, Qt does it all for us
}

void returnAuthCheck::languageChange()
{
  retranslateUi(this);
}

enum SetResponse returnAuthCheck::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("amount", &valid);
  if (valid)
    _amount->setLocalValue(param.toDouble());

  param = pParams.value("cmhead_id", &valid);
  if (valid)
    _cmheadid=param.toInt();

  populate();

  return NoError;
}

void returnAuthCheck::sSave()
{
  XSqlQuery returnSave;
  if (!_date->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must enter a date for this check.") );
    _date->setFocus();
    return;
  }

  else if (_amount->isZero())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must enter an amount for this check.") );
    return;
  }

  else if (!_bankaccnt->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must select a bank account for this check.") );
    _date->setFocus();
    return;
  }

  else
  {
    returnSave.prepare("SELECT createCheck(:bankaccnt_id, 'C', :recipid,"
	      "                   :checkDate, :amount, :curr_id, NULL,"
	      "                   NULL, :for, :notes, true, :aropen_id) AS result; ");
    returnSave.bindValue(":bankaccnt_id", _bankaccnt->id());
    returnSave.bindValue(":recipid",	_custid);
    returnSave.bindValue(":checkDate", _date->date());
    returnSave.bindValue(":amount",	_amount->localValue());
    returnSave.bindValue(":curr_id",	_amount->id());
    returnSave.bindValue(":for",	_for->text().trimmed());
    returnSave.bindValue(":notes", _notes->toPlainText().trimmed());
	returnSave.bindValue(":aropen_id", _aropenid);
	returnSave.exec();
    if (returnSave.first())
    {
      _checkid = returnSave.value("result").toInt();
      if (_checkid < 0)
      {
        systemError(this, storedProcErrorLookup("createCheck", _checkid),
		    __FILE__, __LINE__);
        return;
      }
      returnSave.prepare( "SELECT checkhead_number "
               "FROM checkhead "
               "WHERE (checkhead_id=:check_id);" );
      returnSave.bindValue(":check_id", _checkid);
      returnSave.exec();
      if (returnSave.lastError().type() != QSqlError::NoError)
      {
        systemError(this, returnSave.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
	  done(true);
	}
    else if (returnSave.lastError().type() != QSqlError::NoError)
    {
     systemError(this, returnSave.lastError().databaseText(), __FILE__, __LINE__);
        return;
    }
  }
}

void returnAuthCheck::sClose()
{
  done(-1);
}

void returnAuthCheck::sPopulateBankInfo(int pBankaccntid)
{
  XSqlQuery returnPopulateBankInfo;
  if ( pBankaccntid == -1 )
  {
    _amount->setId(_cmheadcurrid);
    _checkNum->clear();
  }
  else
  {
    XSqlQuery checkNumber;
    checkNumber.prepare( "SELECT bankaccnt_nextchknum, bankaccnt_curr_id "
			 "FROM bankaccnt "
			 "WHERE (bankaccnt_id=:bankaccnt_id);" );
    checkNumber.bindValue(":bankaccnt_id", pBankaccntid);
    checkNumber.exec();
    if (checkNumber.first())
    {
      if (checkNumber.value("bankaccnt_curr_id").toInt() != _cmheadcurrid)
      {
	QMessageBox::critical(this, tr("Bank Uses Different Currency"),
			      tr("<p>The currency of the bank account is not "
				 "the same as the currency of the credit "
				 "memo. You may not use this bank account."));
	_amount->setId(_cmheadcurrid);
	_bankaccnt->setId(-1);
	return;
      }
      _checkNum->setText(checkNumber.value("bankaccnt_nextchknum").toString());
      _amount->setId(checkNumber.value("bankaccnt_curr_id").toInt());
    }
    else if (returnPopulateBankInfo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, checkNumber.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void returnAuthCheck::populate()
{
  XSqlQuery returnpopulate;
  returnpopulate.prepare("SELECT cust_id,cust_name,cmhead_number,cmhead_curr_id, "
	        "'Return Authorization ' || rahead_number::text AS memo, "
			"'Applied Against Return ' || cmhead_number::text AS note, "
			"aropen_id,aropen_amount "
			"FROM rahead,cmhead,custinfo,aropen "
			"WHERE ((cmhead_cust_id=cust_id) "
			"AND (rahead_id=cmhead_rahead_id) "
			"AND (cmhead_id=:cmhead_id) "
			"AND (aropen_doctype='C') "
			"AND (aropen_docnumber=cmhead_number));");
  returnpopulate.bindValue(":cmhead_id",_cmheadid);
  returnpopulate.exec();
  if (returnpopulate.first())
  {
    _custid=returnpopulate.value("cust_id").toInt();
	_aropenid=returnpopulate.value("aropen_id").toInt();
    _custName->setText(returnpopulate.value("cust_name").toString());
	_creditmemo->setText(returnpopulate.value("cmhead_number").toString());
	_cmheadcurrid = returnpopulate.value("cmhead_curr_id").toInt();
	_amount->setId(returnpopulate.value("cmhead_curr_id").toInt());
	_amount->setLocalValue(returnpopulate.value("aropen_amount").toDouble());
	_for->setText(returnpopulate.value("memo").toString());
	_notes->setText(returnpopulate.value("note").toString());
  }
  else if (returnpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, returnpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  returnpopulate.prepare("SELECT bankaccnt_id "
	    "FROM bankaccnt "
	    "WHERE (bankaccnt_ap"
	    "  AND  (bankaccnt_type='K')"
	    "  AND  (bankaccnt_curr_id=:cmcurrid));");
  returnpopulate.bindValue(":cmcurrid", _cmheadcurrid);
  returnpopulate.exec();
  if (returnpopulate.first())
    _bankaccnt->setId(returnpopulate.value("bankaccnt_id").toInt());
  else if (returnpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, returnpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _bankaccnt->setId(-1);
}
