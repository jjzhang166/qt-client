/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "classCode.h"

#include <QVariant>
#include <QMessageBox>

#include "errorReporter.h"
#include "guiErrorCheck.h"

classCode::classCode(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_classCode, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

classCode::~classCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void classCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse classCode::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    _classcodeid = param.toInt();
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

      _classCode->setEnabled(false);
      _description->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void classCode::sSave()
{
  XSqlQuery classSave;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_classCode->text().length() == 0, _classCode,
                          tr("You must enter a valid Class Code "
                             "before continuing"))
     ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Class Code"), errors))
    return;

  if (_mode == cEdit)
    classSave.prepare( "UPDATE classcode "
               "SET classcode_code=:classcode_code, classcode_descrip=:classcode_descrip "
               "WHERE (classcode_id=:classcode_id);" );
  else if (_mode == cNew)
  {
    classSave.exec("SELECT NEXTVAL('classcode_classcode_id_seq') AS classcode_id");
    if (classSave.first())
      _classcodeid = classSave.value("classcode_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
 
    classSave.prepare( "INSERT INTO classcode "
               "( classcode_id, classcode_code, classcode_descrip ) "
               "VALUES "
               "( :classcode_id, :classcode_code, :classcode_descrip );" );
  }

  classSave.bindValue(":classcode_id", _classcodeid);
  classSave.bindValue(":classcode_code", _classCode->text());
  classSave.bindValue(":classcode_descrip", _description->text());
  classSave.exec();

  done(_classcodeid);
}

void classCode::sCheck()
{
  XSqlQuery classCheck;
  _classCode->setText(_classCode->text().trimmed());
  if ( (_mode == cNew) && (_classCode->text().length()) )
  {
    classCheck.prepare( "SELECT classcode_id "
               "FROM classcode "
               "WHERE (UPPER(classcode_code)=UPPER(:classcode_code));" );
    classCheck.bindValue(":classcode_code", _classCode->text());
    classCheck.exec();
    if (classCheck.first())
    {
      _classcodeid = classCheck.value("classcode_id").toInt();
      _mode = cEdit;
      populate();

      _classCode->setEnabled(false);
    }
  }
}

void classCode::populate()
{
  XSqlQuery classpopulate;
  classpopulate.prepare( "SELECT classcode_code, classcode_descrip "
             "FROM classcode "
             "WHERE (classcode_id=:classcode_id);" );
  classpopulate.bindValue(":classcode_id", _classcodeid);
  classpopulate.exec();
  if (classpopulate.first())
  {
    _classCode->setText(classpopulate.value("classcode_code"));
    _description->setText(classpopulate.value("classcode_descrip"));
  }
}

