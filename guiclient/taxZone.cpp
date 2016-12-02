/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxZone.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QCloseEvent>

#include "storedProcErrorLookup.h"
#include "errorReporter.h"

taxZone::taxZone(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxZone, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

taxZone::~taxZone()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxZone::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxZone::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("taxzone_id", &valid);
  if (valid)
  {
    _taxzoneid = param.toInt();
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

      _taxZone->setEnabled(false);
      _description->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void taxZone::sCheck()
{
  XSqlQuery taxCheck;
  _taxZone->setText(_taxZone->text().trimmed());
  if ( (_mode == cNew) && (_taxZone->text().length()) )
  {
	taxCheck.prepare( "SELECT taxzone_id "
               "FROM taxzone "
               "WHERE (UPPER(taxzone_code)=UPPER(:taxzone_code));" );
    taxCheck.bindValue(":taxzone_code", _taxZone->text());
    taxCheck.exec();
    if (taxCheck.first())
    {
      _taxzoneid = taxCheck.value("taxzone_id").toInt();
      _mode = cEdit;
      populate();

      _taxZone->setEnabled(false);
    }
  }
}

void taxZone::sSave()
{
  XSqlQuery taxSave;
  if (_taxZone->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Tax Zone"),
                            tr("You must enter a valid Code.") );
      return;
  }
  
  if (_mode == cEdit)
  {
    taxSave.prepare( "SELECT taxzone_id "
               "FROM taxzone "
               "WHERE ( (taxzone_id<>:taxzone_id)"
               " AND (UPPER(taxzone_code)=UPPER(:taxzone_code)) );");
    taxSave.bindValue(":taxzone_id", _taxzoneid);
  }
  else
  {
    taxSave.prepare( "SELECT taxzone_id "
               "FROM taxzone "
               "WHERE (taxzone_code=:taxzone_code);");
  }
  taxSave.bindValue(":taxzone_code", _taxZone->text().trimmed());
  taxSave.exec();
  if (taxSave.first())
  {
    QMessageBox::critical( this, tr("Cannot Create Tax Zone"),
			   tr( "A Tax Zone with the entered code already exists."
			       "You may not create a Tax Zone with this code." ) );
    _taxZone->setFocus();
    return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Tax Zone Information"),
                                taxSave, __FILE__, __LINE__))
  {
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  taxSave.exec("BEGIN;");

  if (_mode == cEdit)
  {
    taxSave.prepare( "UPDATE taxzone "
               "SET taxzone_code=:taxzone_code,"
               "    taxzone_descrip=:taxzone_descrip "
               "WHERE (taxzone_id=:taxzone_id);" );
  }
  else if (_mode == cNew)
  {
    taxSave.exec("SELECT NEXTVAL('taxzone_taxzone_id_seq') AS taxzone_id;");
    if (taxSave.first())
      _taxzoneid = taxSave.value("taxzone_id").toInt();
    else if (taxSave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Tax Zone Information"),
                           taxSave, __FILE__, __LINE__);
      return;
    }

    taxSave.prepare( "INSERT INTO taxzone "
               "(taxzone_id, taxzone_code, taxzone_descrip)" 
               "VALUES "
               "(:taxzone_id, :taxzone_code, :taxzone_descrip);" ); 
  }
  taxSave.bindValue(":taxzone_id", _taxzoneid);
  taxSave.bindValue(":taxzone_code", _taxZone->text().trimmed());
  taxSave.bindValue(":taxzone_descrip", _description->text());
  taxSave.exec();
  if (taxSave.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Tax Zone Information"),
                         taxSave, __FILE__, __LINE__);
    return;
  }

  taxSave.exec("COMMIT;");

  done(_taxzoneid);
}

void taxZone::populate()
{
  XSqlQuery taxpopulate;
  taxpopulate.prepare( "SELECT taxzone_code, taxzone_descrip "
             "FROM taxzone " 
             "WHERE (taxzone_id=:taxzone_id);" );
  taxpopulate.bindValue(":taxzone_id", _taxzoneid);
  taxpopulate.exec();
  if (taxpopulate.first())
  {
    _taxZone->setText(taxpopulate.value("taxzone_code").toString());
    _description->setText(taxpopulate.value("taxzone_descrip").toString());
	
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Tax Zone Information"),
                                taxpopulate, __FILE__, __LINE__))
  {
    return;
  }
}

void taxZone::done( int r )
{
  XDialog::done( r );
  close();
}
