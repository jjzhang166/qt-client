/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "subAccntType.h"

#include <QVariant>
#include <QMessageBox>
#include "errorReporter.h"

subAccntType::subAccntType(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_code, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

subAccntType::~subAccntType()
{
  // no need to delete child widgets, Qt does it all for us
}

void subAccntType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse subAccntType::set( const ParameterList & pParams )
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("subaccnttype_id", &valid);
  if (valid)
  {
    _subaccnttypeid = param.toInt();
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
      _type->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void subAccntType::sSave()
{
  XSqlQuery subSave;
  if (_mode == cEdit)
  {
    subSave.prepare( "SELECT subaccnttype_id "
               "FROM subaccnttype "
               "WHERE ( (subaccnttype_id<>:subaccnttype_id)"
               " AND (subaccnttype_code=:subaccnttype_code) );");
    subSave.bindValue(":subaccnttype_id", _subaccnttypeid);
    subSave.bindValue(":subaccnttype_code", _code->text());
    subSave.exec();
    if (subSave.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Subaccount Type"),
                             tr( "A Subaccount Type with the entered code already exists."
                                 "You may not create a Subaccount Type with this code." ) );
      _code->setFocus();
      return;
    }

    subSave.prepare( "UPDATE subaccnttype "
               "SET subaccnttype_code=:subaccnttype_code,"
               "       subaccnttype_descrip=:subaccnttype_descrip,"
               "       subaccnttype_accnt_type=:subaccnttype_accnt_type "
               "WHERE (subaccnttype_id=:subaccnttype_id);" );
    subSave.bindValue(":subaccnttype_id", _subaccnttypeid);
    subSave.bindValue(":subaccnttype_code", _code->text());
    subSave.bindValue(":subaccnttype_descrip", _description->text());
    if (_type->currentIndex() == 0)
      subSave.bindValue(":subaccnttype_accnt_type", "A");
    else if (_type->currentIndex() == 1)
      subSave.bindValue(":subaccnttype_accnt_type", "L");
    else if (_type->currentIndex() == 2)
      subSave.bindValue(":subaccnttype_accnt_type", "E");
    else if (_type->currentIndex() == 3)
      subSave.bindValue(":subaccnttype_accnt_type", "R");
    else if (_type->currentIndex() == 4)
      subSave.bindValue(":subaccnttype_accnt_type", "Q");
    subSave.exec();
  }
  else if (_mode == cNew)
  {
    subSave.prepare( "SELECT subaccnttype_id "
               "FROM subaccnttype "
               "WHERE (subaccnttype_code=:subaccnttype_code);");
    subSave.bindValue(":subaccnttype_code", _code->text().trimmed());
    subSave.exec();
    if (subSave.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Subaccount Type"),
                             tr( "A Subaccount Type with the entered code already exists.\n"
                                 "You may not create a Subaccount Type with this code." ) );
      _code->setFocus();
      return;
    }

    subSave.exec("SELECT NEXTVAL('subaccnttype_subaccnttype_id_seq') AS subaccnttype_id;");
    if (subSave.first())
      _subaccnttypeid = subSave.value("subaccnttype_id").toInt();
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sub Account Information"),
                           subSave, __FILE__, __LINE__);
      return;
    }

    subSave.prepare( "INSERT INTO subaccnttype "
               "( subaccnttype_id, subaccnttype_code,"
               "  subaccnttype_descrip, subaccnttype_accnt_type ) "
               "VALUES "
               "( :subaccnttype_id, :subaccnttype_code, :subaccnttype_descrip, :subaccnttype_accnt_type );" );
    subSave.bindValue(":subaccnttype_id", _subaccnttypeid);
    subSave.bindValue(":subaccnttype_code", _code->text());
    subSave.bindValue(":subaccnttype_descrip", _description->text());
    if (_type->currentIndex() == 0)
      subSave.bindValue(":subaccnttype_accnt_type", "A");
    else if (_type->currentIndex() == 1)
      subSave.bindValue(":subaccnttype_accnt_type", "L");
    else if (_type->currentIndex() == 2)
      subSave.bindValue(":subaccnttype_accnt_type", "E");
    else if (_type->currentIndex() == 3)
      subSave.bindValue(":subaccnttype_accnt_type", "R");
    else if (_type->currentIndex() == 4)
      subSave.bindValue(":subaccnttype_accnt_type", "Q");
    subSave.exec();
  }

  done(_subaccnttypeid);
}

void subAccntType::sCheck()
{
  XSqlQuery subCheck;
  _code->setText(_code->text().trimmed());
//  if ( (_mode == cNew) && (_code->text().length()) )
  if (_code->text().length())
  {
    subCheck.prepare( "SELECT subaccnttype_id "
               "FROM subaccnttype "
               "WHERE (subaccnttype_code=:subaccnttype_code);" );
    subCheck.bindValue(":subaccnttype_code", _code->text());
    subCheck.exec();
    if (subCheck.first())
    {
      _subaccnttypeid = subCheck.value("subaccnttype_id").toInt();
      _mode = cEdit;
      populate();

//      _code->setEnabled(false);
    }
  }
}

void subAccntType::populate()
{
  XSqlQuery subpopulate;
  subpopulate.prepare( "SELECT subaccnttype_code, subaccnttype_accnt_type, subaccnttype_descrip "
             "FROM subaccnttype "
             "WHERE (subaccnttype_id=:subaccnttype_id);" );
  subpopulate.bindValue(":subaccnttype_id", _subaccnttypeid);
  subpopulate.exec();
  if (subpopulate.first())
  {
    _code->setText(subpopulate.value("subaccnttype_code").toString());
    _description->setText(subpopulate.value("subaccnttype_descrip").toString());
    
    if (subpopulate.value("subaccnttype_accnt_type").toString() == "A")
      _type->setCurrentIndex(0);
    else if (subpopulate.value("subaccnttype_accnt_type").toString() == "L")
      _type->setCurrentIndex(1);
    else if (subpopulate.value("subaccnttype_accnt_type").toString() == "E")
      _type->setCurrentIndex(2);
    else if (subpopulate.value("subaccnttype_accnt_type").toString() == "R")
      _type->setCurrentIndex(3);
    else if (subpopulate.value("subaccnttype_accnt_type").toString() == "Q")
      _type->setCurrentIndex(4);
  }
}

