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
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
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

#include "returnAuthCheck.h"
#include "storedProcErrorLookup.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

returnAuthCheck::returnAuthCheck(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_bankaccnt, SIGNAL(newID(int)),    this, SLOT(sPopulateBankInfo(int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _date->setDate(omfgThis->dbDate(), true);

  _bankaccnt->setAllowNull(TRUE);
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
    q.prepare("SELECT createCheck(:bankaccnt_id, 'C', :recipid,"
	      "                   :checkDate, :amount, :curr_id, NULL,"
	      "                   NULL, :for, :notes, TRUE, :aropen_id) AS result; ");
    q.bindValue(":bankaccnt_id", _bankaccnt->id());
    q.bindValue(":recipid",	_custid);
    q.bindValue(":checkDate", _date->date());
    q.bindValue(":amount",	_amount->localValue());
    q.bindValue(":curr_id",	_amount->id());
    q.bindValue(":for",	_for->text().stripWhiteSpace());
    q.bindValue(":notes", _notes->text().stripWhiteSpace());
	q.bindValue(":aropen_id", _aropenid);
	q.exec();
    if (q.first())
    {
      _checkid = q.value("result").toInt();
      if (_checkid < 0)
      {
        systemError(this, storedProcErrorLookup("createCheck", _checkid),
		    __FILE__, __LINE__);
        return;
      }
      q.prepare( "SELECT checkhead_number "
               "FROM checkhead "
               "WHERE (checkhead_id=:check_id);" );
      q.bindValue(":check_id", _checkid);
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
	  done(TRUE);
	}
    else if (q.lastError().type() != QSqlError::None)
    {
     systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, checkNumber.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void returnAuthCheck::populate()
{
  q.prepare("SELECT cust_id,cust_name,cmhead_number,cmhead_curr_id, "
	        "'Return Authorization ' || rahead_number::text AS memo, "
			"'Applied Against Credit Memo ' || cmhead_number::text AS note, "
			"aropen_id,aropen_amount "
			"FROM rahead,cmhead,custinfo,aropen "
			"WHERE ((cmhead_cust_id=cust_id) "
			"AND (rahead_id=cmhead_rahead_id) "
			"AND (cmhead_id=:cmhead_id) "
			"AND (aropen_doctype='C') "
			"AND (aropen_docnumber=cmhead_number));");
  q.bindValue(":cmhead_id",_cmheadid);
  q.exec();
  if (q.first())
  {
    _custid=q.value("cust_id").toInt();
	_aropenid=q.value("aropen_id").toInt();
    _custName->setText(q.value("cust_name").toString());
	_creditmemo->setText(q.value("cmhead_number").toString());
	_cmheadcurrid = q.value("cmhead_curr_id").toInt();
	_amount->setId(q.value("cmhead_curr_id").toInt());
	_amount->setLocalValue(q.value("aropen_amount").toDouble());
	_for->setText(q.value("memo").toString());
	_notes->setText(q.value("note").toString());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT bankaccnt_id "
	    "FROM bankaccnt "
	    "WHERE (bankaccnt_ap"
	    "  AND  (bankaccnt_type='K')"
	    "  AND  (bankaccnt_curr_id=:cmcurrid));");
  q.bindValue(":cmcurrid", _cmheadcurrid);
  q.exec();
  if (q.first())
    _bankaccnt->setId(q.value("bankaccnt_id").toInt());
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _bankaccnt->setId(-1);
}
