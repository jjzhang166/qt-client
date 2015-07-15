/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxClass.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QCloseEvent>

#include "storedProcErrorLookup.h"

taxClass::taxClass(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxClass, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

taxClass::~taxClass()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxClass::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxClass::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("taxclass_id", &valid);
  if (valid)
  {
    _taxclassid = param.toInt();
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

      _taxClass->setEnabled(false);
      _description->setEnabled(false);
	  _seq->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void taxClass::sCheck()
{
  XSqlQuery taxCheck;
  _taxClass->setText(_taxClass->text().trimmed());
  if ( (_mode == cNew) && (_taxClass->text().length()) )
  {
	taxCheck.prepare( "SELECT taxclass_id "
               "FROM taxclass "
               "WHERE (UPPER(taxclass_code) = UPPER(:taxclass_code));" );
    taxCheck.bindValue(":taxclass_code", _taxClass->text());
    taxCheck.exec();
    if (taxCheck.first())
    {
      _taxclassid = taxCheck.value("taxclass_id").toInt();
      _mode = cEdit;
      populate();

      _taxClass->setEnabled(false);
    }
  }
}

void taxClass::sSave()
{
  XSqlQuery taxSave;
  if (_taxClass->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Tax Class"),
                            tr("You must enter a valid Code.") );
      return;
  }
  
  if (_mode == cEdit)
  {
    taxSave.prepare( "SELECT taxclass_id "
               "FROM taxclass "
               "WHERE ( (taxclass_id <> :taxclass_id)"
               " AND (UPPER(taxclass_code) = UPPER(:taxclass_code)) );");
    taxSave.bindValue(":taxclass_id", _taxclassid);
  }
  else
  {
    taxSave.prepare( "SELECT taxclass_id "
               "FROM taxclass "
               "WHERE (taxclass_code = :taxclass_code);");
  }
  taxSave.bindValue(":taxclass_code", _taxClass->text().trimmed());
  taxSave.exec();
  if (taxSave.first())
  {
    QMessageBox::critical( this, tr("Cannot Create Tax Class"),
			   tr( "A Tax Class with the entered code already exists."
			       "You may not create a Tax Class with this code." ) );
    _taxClass->setFocus();
    return;
  }
  else if (taxSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  taxSave.exec("BEGIN;");

  if (_mode == cEdit)
  {
    taxSave.prepare( "UPDATE taxclass "
               "SET taxclass_code=:taxclass_code,"
               "    taxclass_descrip=:taxclass_descrip, "
			   "    taxclass_sequence=:taxclass_sequence "
               "WHERE (taxclass_id=:taxclass_id);" );
  }
  else if (_mode == cNew)
  {
    taxSave.exec("SELECT NEXTVAL('taxclass_taxclass_id_seq') AS taxclass_id;");
    if (taxSave.first())
      _taxclassid = taxSave.value("taxclass_id").toInt();
    else if (taxSave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, taxSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    taxSave.prepare( "INSERT INTO taxclass "
               "(taxclass_id, taxclass_code, taxclass_descrip, taxclass_sequence)" 
               "VALUES "
               "(:taxclass_id, :taxclass_code, :taxclass_descrip, :taxclass_sequence);" ); 
  }
  taxSave.bindValue(":taxclass_id", _taxclassid);
  taxSave.bindValue(":taxclass_code", _taxClass->text().trimmed());
  taxSave.bindValue(":taxclass_descrip", _description->text());
  taxSave.bindValue(":taxclass_sequence", _seq->value());
  taxSave.exec();
  if (taxSave.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, taxSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  taxSave.exec("COMMIT;");

  done(_taxclassid);
}

void taxClass::populate()
{
  XSqlQuery taxpopulate;
  taxpopulate.prepare( "SELECT taxclass_code, taxclass_descrip, taxclass_sequence "
             "FROM taxclass "
             "WHERE (taxclass_id=:taxclass_id);" );
  taxpopulate.bindValue(":taxclass_id", _taxclassid);
  taxpopulate.exec();
  if (taxpopulate.first())
  {
    _taxClass->setText(taxpopulate.value("taxclass_code").toString());
    _description->setText(taxpopulate.value("taxclass_descrip").toString());
	_seq->setValue(taxpopulate.value("taxclass_sequence").toInt());
  }
  else if (taxpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void taxClass::done( int r )
{
  XDialog::done( r );
  close();
}
