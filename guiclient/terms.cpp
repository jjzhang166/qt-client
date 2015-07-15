/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "terms.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

terms::terms(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_proximo, SIGNAL(toggled(bool)), _cutOffDay, SLOT(setEnabled(bool)));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_days, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_code, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  
  _discountPercent->setValidator(omfgThis->percentVal());
}

terms::~terms()
{
  // no need to delete child widgets, Qt does it all for us
}

void terms::languageChange()
{
  retranslateUi(this);
}

enum SetResponse terms::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("terms_id", &valid);
  if (valid)
  {
    _termsid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _code->setEnabled(false);
      _description->setEnabled(false);
      _typeGroup->setEnabled(false);
      _ap->setEnabled(false);
      _ar->setEnabled(false);
      _fincharg->setEnabled(false);
      _dueDays->setEnabled(false);
      _discountDays->setEnabled(false);
      _discountPercent->setEnabled(false);
      _cutOffDay->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

bool terms::sCheck()
{
  XSqlQuery termsCheck;
  _code->setText(_code->text().trimmed());
  if ( (_mode == cNew) && (_code->text().length()) )
  {
    termsCheck.prepare( "SELECT terms_id "
               "FROM terms "
               "WHERE (UPPER(terms_code)=UPPER(:terms_code));" );
    termsCheck.bindValue(":terms_code", _code->text());
    termsCheck.exec();
    if (termsCheck.first())
    {
      _termsid = termsCheck.value("terms_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(false);
      return true;
    }
  }
  return false;
}

void terms::sSave()
{
  XSqlQuery termsSave;
  if(_code->text().trimmed().isEmpty())
  {
    QMessageBox::warning(this, tr("Cannot Save Terms"),
      tr("You must specify a code for the Terms."));
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    if (sCheck())
    {
      QMessageBox::warning( this, tr("Cannot Save Terms Code"),
                            tr("This Terms code already exists.  You have been placed in edit mode.") );
      return;
    }

    termsSave.exec("SELECT NEXTVAL('terms_terms_id_seq') AS _terms_id");
    if (termsSave.first())
      _termsid = termsSave.value("_terms_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    termsSave.prepare( "INSERT INTO terms "
               "( terms_id, terms_code, terms_descrip, terms_type,"
               "  terms_ap, terms_ar, terms_fincharg,"
               "  terms_duedays, terms_discdays, terms_discprcnt, terms_cutoffday ) "
               "VALUES "
               "( :terms_id, :terms_code, :terms_descrip, :terms_type,"
               "  :terms_ap, :terms_ar, :terms_fincharg,"
               "  :terms_duedays, :terms_discdays, :terms_discprcnt, :terms_cutoffday );" );
  }
  else if (_mode == cEdit)
  {
    termsSave.prepare( "SELECT terms_id "
               "FROM terms "
               "WHERE ( (UPPER(terms_code)=UPPER(:terms_code))"
               " AND (terms_id<>:terms_id) );" );
    termsSave.bindValue(":terms_id", _termsid);
    termsSave.bindValue(":terms_code", _code->text().trimmed());
    termsSave.exec();
    if (termsSave.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Terms"),
                            tr("You may not rename this Terms code with the entered name as it is in use by another Terms code.") );
      return;
    }

    termsSave.prepare( "UPDATE terms "
               "SET terms_code=:terms_code, terms_descrip=:terms_descrip, terms_type=:terms_type,"
               "    terms_ap=:terms_ap, terms_ar=:terms_ar, terms_fincharg=:terms_fincharg,"
               "    terms_duedays=:terms_duedays, terms_discdays=:terms_discdays,"
               "    terms_discprcnt=:terms_discprcnt, terms_cutoffday=:terms_cutoffday "
               "WHERE (terms_id=:terms_id);" );
  }

  if (_days->isChecked())
    termsSave.bindValue(":terms_type", "D");
  else
    termsSave.bindValue(":terms_type", "P");

  termsSave.bindValue(":terms_id", _termsid);
  termsSave.bindValue(":terms_code", _code->text().trimmed());
  termsSave.bindValue(":terms_descrip", _description->text().trimmed());
  termsSave.bindValue(":terms_ap", QVariant(_ap->isChecked()));
  termsSave.bindValue(":terms_ar", QVariant(_ar->isChecked()));
  if (_ar->isChecked())
    termsSave.bindValue(":terms_fincharg", QVariant(_fincharg->isChecked()));
  else
    termsSave.bindValue(":terms_fincharg", false);
  termsSave.bindValue(":terms_duedays", _dueDays->value());
  termsSave.bindValue(":terms_discdays", _discountDays->value());
  termsSave.bindValue(":terms_discprcnt", (_discountPercent->toDouble() / 100.0));
  termsSave.bindValue(":terms_cutoffday", _cutOffDay->value());
  termsSave.exec();

  done(_termsid);
}

void terms::sTypeChanged()
{
  if (_days->isChecked())
  {
    _dueDaysLit->setText(tr("Due Days:"));
    _discountDaysLit->setText(tr("Discnt. Days:"));

    _dueDays->setMinimum(0);
    _dueDays->setMaximum(999);
    _discountDays->setMinimum(0);
    _discountDays->setMaximum(999);
  }
  else
  {
    _dueDaysLit->setText(tr("Due Day:"));
    _discountDaysLit->setText(tr("Discnt. Day:"));

    _dueDays->setMinimum(1);
    _dueDays->setMaximum(31);
    _discountDays->setMinimum(1);
    _discountDays->setMaximum(31);
    _cutOffDay->setMaximum(31);
  }
}

void terms::populate()
{
  XSqlQuery termspopulate;
  termspopulate.prepare( "SELECT * "
             "FROM terms "
             "WHERE (terms_id=:terms_id);" );
  termspopulate.bindValue(":terms_id", _termsid);
  termspopulate.exec();
  if (termspopulate.first())
  {
    _code->setText(termspopulate.value("terms_code").toString());
    _description->setText(termspopulate.value("terms_descrip").toString());
    _ap->setChecked(termspopulate.value("terms_ap").toBool());
    _ar->setChecked(termspopulate.value("terms_ar").toBool());
    _fincharg->setChecked(termspopulate.value("terms_fincharg").toBool());
    _dueDays->setValue(termspopulate.value("terms_duedays").toInt());
    _discountPercent->setText(termspopulate.value("terms_discprcnt").toDouble() * 100);
    _discountDays->setValue(termspopulate.value("terms_discdays").toInt());

    if (termspopulate.value("terms_type").toString() == "D")
    {
      _days->setChecked(true);
      if (_mode == cEdit)
        _cutOffDay->setEnabled(false);
    }
    else if (termspopulate.value("terms_type").toString() == "P")
    {
      _proximo->setChecked(true);
      _cutOffDay->setValue(termspopulate.value("terms_cutoffday").toInt());
    }
  }
}
