/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "accountNumber.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>

accountNumber::accountNumber(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_type, SIGNAL(activated(int)), this, SLOT(populateSubTypes()));

  _currency->setLabel(_currencyLit);

  // Until we find out what use there is for an account-level currency, hide it
  _currencyLit->hide();
  _currency->hide();
  
  _subType->setAllowNull(true);
  populateSubTypes();
  
  if (_metrics->value("GLCompanySize").toInt() == 0)
  {
    _company->hide();
    _sep1Lit->hide();
  }

  if (_metrics->value("GLProfitSize").toInt() == 0)
  {
    _profit->hide();
    _sep2Lit->hide();
  }

  if (_metrics->value("GLSubaccountSize").toInt() == 0)
  {
    _sub->hide();
    _sep3Lit->hide();
  }
  if (!_metrics->boolean("ManualForwardUpdate"))
    _forwardUpdate->hide();

  _wasActive = false;
}

accountNumber::~accountNumber()
{
  // no need to delete child widgets, Qt does it all for us
}

void accountNumber::languageChange()
{
  retranslateUi(this);
}

enum SetResponse accountNumber::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
  {
    _accntid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _number->setValidator(new QRegExpValidator(QRegExp(QString("\\S{0,%1}").arg(_metrics->value("GLMainSize").toInt())),
                                                 _number));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _company->setEnabled(false);
      _profit->setEnabled(false);
      _number->setEnabled(false);
      _sub->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _company->setEnabled(false);
      _profit->setEnabled(false);
      _number->setEnabled(false);
      _sub->setEnabled(false);
      _description->setEnabled(false);
      _extReference->setEnabled(false);
      _forwardUpdate->setEnabled(false);
      _active->setEnabled(false);
      _comments->setEnabled(false);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void accountNumber::sSave()
{
  XSqlQuery accountSave;
  if (_mode == cEdit && _wasActive && !_active->isChecked())
  {
    QString glsum("SELECT trialbal_ending AS bal"
                  "  FROM trialbal, period"
                  " WHERE((period_id=trialbal_period_id)"
                  "   AND (NOT period_closed)"
                  "   AND (trialbal_accnt_id=<? value(\"accnt_id\") ?>))"
                  " ORDER BY period_start DESC"
                  " LIMIT 1;");
    ParameterList pl;
    pl.append("accnt_id", _accntid);
    MetaSQLQuery mm(glsum);
    accountSave = mm.toQuery(pl);
    if(accountSave.first() && accountSave.value("bal").toInt() != 0)
    {
      if(QMessageBox::warning(this, tr("Account has Balance"),
                            tr("<p>This Account has a balance. "
			       "Are you sure you want to mark it inactive?"),
                            QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
        return;
      }
    }
  }

  QString sql("SELECT accnt_id "
              "FROM ONLY accnt "
              "WHERE ( (accnt_number=<? value(\"accnt_number\") ?>)"
	      "<? if exists(\"accnt_company\") ?>"
	      " AND (accnt_company=<? value(\"accnt_company\") ?>)"
	      "<? endif ?>"
	      "<? if exists(\"accnt_profit\") ?>"
	      " AND (accnt_profit=<? value(\"accnt_profit\") ?>)"
	      "<? endif ?>"
	      "<? if exists(\"accnt_sub\") ?>"
	      " AND (accnt_sub=<? value(\"accnt_sub\") ?>)"
	      "<? endif ?>"
	      "<? if exists(\"accnt_id\") ?>"
	      " AND (accnt_id<><? value(\"accnt_id\") ?>)"
	      "<? endif ?>"
	      " );" );

  ParameterList params;
  params.append("accnt_number", _number->text().trimmed());

  if (_metrics->value("GLCompanySize").toInt())
    params.append("accnt_company", _company->currentText());

  if (_metrics->value("GLProfitSize").toInt())
    params.append("accnt_profit", _profit->currentText());

  if (_metrics->value("GLSubaccountSize").toInt())
    params.append("accnt_sub", _sub->currentText());

  if (_mode == cEdit)
    params.append("accnt_id", _accntid);

  MetaSQLQuery mql(sql);
  accountSave = mql.toQuery(params);
  if (accountSave.first())
  {
    QMessageBox::warning( this, tr("Cannot Save Account"),
                          tr("<p>This Account cannot be saved as an Account "
			     "with the same number already exists.") );
    return;
  }
  else if (accountSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, accountSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cNew)
  {
    if(_number->text().trimmed().isEmpty())
    {
      QMessageBox::warning(this, tr("No Account Number"),
			   tr("<p>You must specify an account number before "
			      "you may save this record."));
      return;
    }

    accountSave.exec("SELECT NEXTVAL('accnt_accnt_id_seq') AS _accnt_id;");
    if (accountSave.first())
      _accntid = accountSave.value("_accnt_id").toInt();
    else if (accountSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, accountSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    accountSave.prepare( "INSERT INTO accnt "
               "( accnt_id,"
               "  accnt_company, accnt_profit, accnt_number, accnt_sub,"
               "  accnt_forwardupdate, accnt_active,"
               "  accnt_type, accnt_descrip, accnt_extref, accnt_comments, "
	       "  accnt_subaccnttype_code, accnt_curr_id ) "
               "VALUES "
               "( :accnt_id,"
               "  :accnt_company, :accnt_profit, :accnt_number, :accnt_sub,"
               "  :accnt_forwardupdate, :accnt_active,"
               "  :accnt_type, :accnt_descrip, :accnt_extref, :accnt_comments,"
               "  (SELECT subaccnttype_code FROM subaccnttype WHERE subaccnttype_id=:accnt_subaccnttype_id), "
	       "  :accnt_curr_id );" );
  }
  else if (_mode == cEdit)
    accountSave.prepare( "UPDATE accnt "
               "SET accnt_company=:accnt_company, accnt_profit=:accnt_profit,"
               "    accnt_number=:accnt_number, accnt_sub=:accnt_sub,"
               "    accnt_forwardupdate=:accnt_forwardupdate,"
               "    accnt_active=:accnt_active,"
               "    accnt_type=:accnt_type, accnt_descrip=:accnt_descrip, accnt_extref=:accnt_extref,"
               "    accnt_comments=:accnt_comments,"
               "    accnt_subaccnttype_code=(SELECT subaccnttype_code FROM subaccnttype WHERE subaccnttype_id=:accnt_subaccnttype_id),"
	       "    accnt_curr_id=:accnt_curr_id "
               "WHERE (accnt_id=:accnt_id);" );

  accountSave.bindValue(":accnt_id", _accntid);
  accountSave.bindValue(":accnt_company", _company->currentText());
  accountSave.bindValue(":accnt_profit", _profit->currentText());
  accountSave.bindValue(":accnt_number", _number->text());
  accountSave.bindValue(":accnt_sub", _sub->currentText());
  accountSave.bindValue(":accnt_descrip", _description->text());
  accountSave.bindValue(":accnt_extref", _extReference->text());
  accountSave.bindValue(":accnt_forwardupdate", QVariant(_forwardUpdate->isChecked()));
  accountSave.bindValue(":accnt_active", QVariant(_active->isChecked()));
  accountSave.bindValue(":accnt_comments", _comments->toPlainText());
  accountSave.bindValue(":accnt_curr_id", _currency->id());
  accountSave.bindValue(":accnt_subaccnttype_id", _subType->id());

  if (_type->currentIndex() == 0)
    accountSave.bindValue(":accnt_type", "A");
  else if (_type->currentIndex() == 1)
    accountSave.bindValue(":accnt_type", "L");
  else if (_type->currentIndex() == 2)
    accountSave.bindValue(":accnt_type", "E");
  else if (_type->currentIndex() == 3)
    accountSave.bindValue(":accnt_type", "R");
  else if (_type->currentIndex() == 4)
    accountSave.bindValue(":accnt_type", "Q");

  accountSave.exec();
  if (accountSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, accountSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_accntid);
}

void accountNumber::populate()
{
  XSqlQuery populateAccount;
  populateAccount.prepare( "SELECT accnt.*, subaccnttype_id, "
             "       CASE WHEN (gltrans_id IS NULL) THEN false ELSE true END AS used "
             "FROM accnt "
             "  LEFT OUTER JOIN subaccnttype ON (subaccnttype_code=accnt_subaccnttype_code) "
             "  LEFT OUTER JOIN gltrans ON (accnt_id=gltrans_accnt_id) "
             "WHERE (accnt_id=:accnt_id)"
             "LIMIT 1" );
  populateAccount.bindValue(":accnt_id", _accntid);
  populateAccount.exec();
  if (populateAccount.first())
  {
    if (_metrics->value("GLCompanySize").toInt())
      _company->setText(populateAccount.value("accnt_company"));

    if (_metrics->value("GLProfitSize").toInt())
      _profit->setText(populateAccount.value("accnt_profit"));

    if (_metrics->value("GLSubaccountSize").toInt())
      _sub->setText(populateAccount.value("accnt_sub"));

    _number->setText(populateAccount.value("accnt_number"));
    _description->setText(populateAccount.value("accnt_descrip"));
    _extReference->setText(populateAccount.value("accnt_extref"));
    _forwardUpdate->setChecked(populateAccount.value("accnt_forwardupdate").toBool());
    _active->setChecked(populateAccount.value("accnt_active").toBool());
    _wasActive = _active->isChecked();
    _comments->setText(populateAccount.value("accnt_comments").toString());
    _currency->setId(populateAccount.value("accnt_curr_id").toInt());

    if (populateAccount.value("accnt_type").toString() == "A")
      _type->setCurrentIndex(0);
    else if (populateAccount.value("accnt_type").toString() == "L")
      _type->setCurrentIndex(1);
    else if (populateAccount.value("accnt_type").toString() == "E")
      _type->setCurrentIndex(2);
    else if (populateAccount.value("accnt_type").toString() == "R")
      _type->setCurrentIndex(3);
    else if (populateAccount.value("accnt_type").toString() == "Q")
      _type->setCurrentIndex(4);

    _type->setDisabled(populateAccount.value("used").toBool());

    populateSubTypes();
    _subType->setId(populateAccount.value("subaccnttype_id").toInt());
  }
  else if (populateAccount.lastError().type() != QSqlError::NoError)
  {
    systemError(this, populateAccount.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountNumber::populateSubTypes()
{
  XSqlQuery sub;
  sub.prepare("SELECT subaccnttype_id, (subaccnttype_code||'-'||subaccnttype_descrip) "
              "FROM subaccnttype "
              "WHERE (subaccnttype_accnt_type=:subaccnttype_accnt_type) "
              "ORDER BY subaccnttype_code; ");
  if (_type->currentIndex() == 0)
    sub.bindValue(":subaccnttype_accnt_type", "A");
  else if (_type->currentIndex() == 1)
    sub.bindValue(":subaccnttype_accnt_type", "L");
  else if (_type->currentIndex() == 2)
    sub.bindValue(":subaccnttype_accnt_type", "E");
  else if (_type->currentIndex() == 3)
    sub.bindValue(":subaccnttype_accnt_type", "R");
  else if (_type->currentIndex() == 4)
    sub.bindValue(":subaccnttype_accnt_type", "Q");
  sub.exec();
  _subType->populate(sub);
  if (sub.lastError().type() != QSqlError::NoError)
  {
    systemError(this, sub.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
}

