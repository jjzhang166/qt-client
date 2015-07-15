/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "freightClass.h"

#include <QVariant>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>

freightClass::freightClass(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_freightClass, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

freightClass::~freightClass()
{
  // no need to delete child widgets, Qt does it all for us
}

void freightClass::languageChange()
{
  retranslateUi(this);
}

enum SetResponse freightClass::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("freightclass_id", &valid);
  if (valid)
  {
    _freightclassid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _freightClass->setEnabled(false);
      _description->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void freightClass::sSave()
{
  XSqlQuery freightSave;
  if (_freightClass->text().length() == 0)
  {
    QMessageBox::information( this, tr("No Freight Class Entered"),
                              tr("You must enter a valid Freight Class before saving this Item Type.") );
    _freightClass->setFocus();
    return;
  }

  if (_mode == cEdit)
    freightSave.prepare( "UPDATE freightclass "
               "SET freightclass_code=:freightclass_code, freightclass_descrip=:freightclass_descrip "
               "WHERE (freightclass_id=:freightclass_id);" );
  else if (_mode == cNew)
  {
    freightSave.exec("SELECT NEXTVAL('freightclass_freightclass_id_seq') AS freightclass_id");
    if (freightSave.first())
      _freightclassid = freightSave.value("freightclass_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
 
    freightSave.prepare( "INSERT INTO freightclass "
               "( freightclass_id, freightclass_code, freightclass_descrip ) "
               "VALUES "
               "( :freightclass_id, :freightclass_code, :freightclass_descrip );" );
  }

  freightSave.bindValue(":freightclass_id", _freightclassid);
  freightSave.bindValue(":freightclass_code", _freightClass->text());
  freightSave.bindValue(":freightclass_descrip", _description->text());
  freightSave.exec();
  if (freightSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, freightSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_freightclassid);
}

void freightClass::sCheck()
{
  XSqlQuery freightCheck;
  _freightClass->setText(_freightClass->text().trimmed());
  if ( (_mode == cNew) && (_freightClass->text().length()) )
  {
    freightCheck.prepare( "SELECT freightclass_id "
               "FROM freightclass "
               "WHERE (UPPER(freightclass_code)=UPPER(:freightclass_code));" );
    freightCheck.bindValue(":freightclass_code", _freightClass->text());
    freightCheck.exec();
    if (freightCheck.first())
    {
      _freightclassid = freightCheck.value("freightclass_id").toInt();
      _mode = cEdit;
      populate();

      _freightClass->setEnabled(false);
    }
  }

  _buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
}

void freightClass::populate()
{
  XSqlQuery freightpopulate;
  freightpopulate.prepare( "SELECT * "
             "FROM freightclass "
             "WHERE (freightclass_id=:freightclass_id);" );
  freightpopulate.bindValue(":freightclass_id", _freightclassid);
  freightpopulate.exec();
  if (freightpopulate.first())
  {
    _freightClass->setText(freightpopulate.value("freightclass_code"));
    _description->setText(freightpopulate.value("freightclass_descrip"));
  }
}

