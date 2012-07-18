/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "selectPayment.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "applyDiscount.h"
#include "errorReporter.h"

selectPayment::selectPayment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_discount, SIGNAL(clicked()),      this, SLOT(sDiscount()));
  connect(_save,     SIGNAL(clicked()),      this, SLOT(sSave()));
  connect(_selected, SIGNAL(idChanged(int)), this, SLOT(sPriceGroup()));

  _bankaccnt->setAllowNull(TRUE);
  _bankaccnt->setType(XComboBox::APBankAccounts);

  _vendor->setReadOnly(TRUE);
  sPriceGroup();
}

selectPayment::~selectPayment()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectPayment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse selectPayment::set(const ParameterList &pParams)
{
  XSqlQuery selectet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
    _bankaccnt->setId(param.toInt());

  param = pParams.value("apopen_id", &valid);
  if (valid)
  {
    _apopenid = param.toInt();

    selectet.prepare( "SELECT apselect_id "
               "FROM apselect "
               "WHERE (apselect_apopen_id=:apopen_id);" );
    selectet.bindValue(":apopen_id", _apopenid);
    selectet.exec();
    if (selectet.first())
    {
      _mode = cEdit;
      _apselectid = selectet.value("apselect_id").toInt();
    }
    else if (selectet.lastError().type() != QSqlError::NoError)
    {
      systemError(this, selectet.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
    else
    {
      _mode = cNew;
      _apselectid = -1;
      _discountAmount->setLocalValue(0.0);
    }

    populate();
  }

  return NoError;
}

void selectPayment::sSave()
{
  XSqlQuery selectSave;
  if (_selected->isZero())
  {
    QMessageBox::warning( this, tr("Cannot Select for Payment"),
      tr("<p>You must specify an amount greater than zero. "
         "If you want to clear this selection you may do so "
         "from the screen you selected this payment from.") );
    return;
  }
  else if ((_selected->localValue() + _discountAmount->localValue()) > (_amount->localValue() + 0.0000001))
  {
    QMessageBox::warning( this, tr("Cannot Select for Payment"),
      tr("You must specify an amount smaller than or equal to the Balance.") );
    _selected->setFocus();
    return;
  }

  if (_bankaccnt->id() == -1)
  {
    QMessageBox::warning( this, tr("Cannot Select for Payment"),
                          tr("<p>You must select a Bank Account from which "
			     "this Payment is to be paid.") );
    _bankaccnt->setFocus();
    return;
  }

  selectSave.prepare("SELECT bankaccnt_curr_id, currConcat(bankaccnt_curr_id) AS currAbbr "
	    "FROM bankaccnt "
	    "WHERE bankaccnt_id = :accntid;");
  selectSave.bindValue(":accntid", _bankaccnt->id());
  selectSave.exec();
  if (selectSave.first())
  {
    if (selectSave.value("bankaccnt_curr_id").toInt() != _selected->id())
    {
	int response = QMessageBox::question(this,
			     tr("Currencies Do Not Match"),
			     tr("<p>The currency selected for this payment "
				 "(%1) is not the same as the currency for the "
				 "Bank Account (%2). Would you like to use "
				 "this Bank Account anyway?")
			       .arg(_selected->currAbbr())
			       .arg(selectSave.value("currAbbr").toString()),
			      QMessageBox::Yes,
			      QMessageBox::No | QMessageBox::Default);
	if (response == QMessageBox::No)
	{
	    _bankaccnt->setFocus();
	    return;
	}
    }
  }
  else
  {
    systemError(this, selectSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cNew)
  {
    selectSave.exec("SELECT NEXTVAL('apselect_apselect_id_seq') AS apselect_id;");
    if (selectSave.first())
      _apselectid = selectSave.value("apselect_id").toInt();
    else if (selectSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, selectSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    selectSave.prepare( "INSERT INTO apselect "
               "( apselect_id, apselect_apopen_id,"
               "  apselect_amount, apselect_bankaccnt_id, "
	       "  apselect_curr_id, apselect_date, apselect_discount ) "
               "VALUES "
               "( :apselect_id, :apselect_apopen_id,"
               "  :apselect_amount, :apselect_bankaccnt_id, "
	       "  :apselect_curr_id, :apselect_docdate, :apselect_discount );" );
    selectSave.bindValue(":apselect_apopen_id", _apopenid);
  }
  else if (_mode == cEdit)
    selectSave.prepare( "UPDATE apselect "
               "SET apselect_amount=:apselect_amount, "
	       " apselect_bankaccnt_id=:apselect_bankaccnt_id, "
	       " apselect_curr_id=:apselect_curr_id, "
	       " apselect_date=:apselect_docdate,"
               " apselect_discount=:apselect_discount "
               "WHERE (apselect_id=:apselect_id);" );

  selectSave.bindValue(":apselect_id", _apselectid);
  selectSave.bindValue(":apselect_amount", _selected->localValue());
  selectSave.bindValue(":apselect_bankaccnt_id", _bankaccnt->id());
  selectSave.bindValue(":apselect_curr_id", _selected->id());
  selectSave.bindValue(":apselect_docdate", _docDate->date());
  selectSave.bindValue(":apselect_discount", _discountAmount->localValue());
  selectSave.exec();
  if (selectSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, selectSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sPaymentsUpdated(_bankaccnt->id(), _apselectid, TRUE);

  done (_apselectid);
}

void selectPayment::populate()
{
  XSqlQuery selectpopulate;
  selectpopulate.prepare( "SELECT apopen_vend_id, apopen_docnumber, apopen_ponumber,"
             "       apopen_docdate, apopen_duedate,"
             "       apopen_amount, "
	     "       COALESCE(apselect_curr_id, apopen_curr_id) AS curr_id, "
             "       (apopen_amount - apopen_paid"
             "          - COALESCE((SELECT SUM(checkitem_amount + checkitem_discount) "
             "                        FROM checkitem, checkhead "
             "                       WHERE ((checkitem_checkhead_id=checkhead_id) "
             "                         AND (checkitem_apopen_id=apopen_id) "
             "                         AND (NOT checkhead_void) "
             "                         AND (NOT checkhead_posted)) "
             "                     ),xmoney(0))) AS f_amount,"
             "       COALESCE(apselect_amount, (apopen_amount - apopen_paid"
             "          - COALESCE((SELECT SUM(checkitem_amount + checkitem_discount) "
             "                        FROM checkitem, checkhead "
             "                       WHERE ((checkitem_checkhead_id=checkhead_id) "
             "                         AND (checkitem_apopen_id=apopen_id) "
             "                         AND (NOT checkhead_void) "
             "                         AND (NOT checkhead_posted)) "
             "                     ),xmoney(0)))) AS f_selected,"
             "       COALESCE(apselect_discount, xmoney(0)) AS discount,"
             "       COALESCE(apselect_bankaccnt_id, -1) AS bankaccnt_id,"
             "       (terms_code || '-' || terms_descrip) AS f_terms "
             "FROM terms RIGHT OUTER JOIN apopen ON (apopen_terms_id=terms_id) "
	     "      LEFT OUTER JOIN apselect ON (apselect_apopen_id=apopen_id) "
             "WHERE apopen_id=:apopen_id;" );
  selectpopulate.bindValue(":apopen_id", _apopenid);
  selectpopulate.exec();
  if (selectpopulate.first())
  {
    _selected->setId(selectpopulate.value("curr_id").toInt());
    _selected->setLocalValue(selectpopulate.value("f_selected").toDouble());
    _vendor->setId(selectpopulate.value("apopen_vend_id").toInt());
    _docNumber->setText(selectpopulate.value("apopen_docnumber").toString());
    _poNum->setText(selectpopulate.value("apopen_ponumber").toString());
    _docDate->setDate(selectpopulate.value("apopen_docdate").toDate());
    _dueDate->setDate(selectpopulate.value("apopen_duedate").toDate());
    _terms->setText(selectpopulate.value("f_terms").toString());
    _total->setLocalValue(selectpopulate.value("apopen_amount").toDouble());
    _discountAmount->setLocalValue(selectpopulate.value("discount").toDouble());
    _amount->setLocalValue(selectpopulate.value("f_amount").toDouble());
    if(selectpopulate.value("bankaccnt_id").toInt() != -1)
      _bankaccnt->setId(selectpopulate.value("bankaccnt_id").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Selections"),
                                selectpopulate, __FILE__, __LINE__))
    return;
}

void selectPayment::sDiscount()
{
  ParameterList params;
  params.append("apopen_id", _apopenid);
  params.append("curr_id", _selected->id());
  if(_discountAmount->localValue() != 0.0)
    params.append("amount", _discountAmount->localValue());

  applyDiscount newdlg(this, "", TRUE);
  newdlg.set(params);

  if(newdlg.exec() != XDialog::Rejected)
  {
    _discountAmount->setLocalValue(newdlg._amount->localValue());
    if(_discountAmount->localValue() + _selected->localValue() > _amount->localValue())
      _selected->setLocalValue(_amount->localValue() - _discountAmount->localValue());
  }
}

void selectPayment::sPriceGroup()
{
  if (! omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_selected->currAbbr()));
}
