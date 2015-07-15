/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentResolution.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

incidentResolution::incidentResolution(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

incidentResolution::~incidentResolution()
{
}

void incidentResolution::languageChange()
{
  retranslateUi(this);
}

enum SetResponse incidentResolution::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("incdtresolution_id", &valid);
  if (valid)
  {
    _incdtresolutionId = param.toInt();
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

      _name->setEnabled(false);
      _order->setEnabled(false);
      _descrip->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void incidentResolution::sCheck()
{
  XSqlQuery incidentCheck;
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    incidentCheck.prepare( "SELECT incdtresolution_id "
               "FROM incdtresolution "
               "WHERE (UPPER(incdtresolution_name)=UPPER(:incdtresolution_name));" );
    incidentCheck.bindValue(":incdtresolution_name", _name->text());
    incidentCheck.exec();
    if (incidentCheck.first())
    {
      _incdtresolutionId = incidentCheck.value("incdtresolution_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
  }
}

void incidentResolution::sSave()
{
  XSqlQuery incidentSave;
  if(_name->text().length() == 0)
  {
    QMessageBox::critical(this, tr("Resolution Name Required"),
      tr("You must enter a Resolution Name to continue.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    incidentSave.exec("SELECT NEXTVAL('incdtresolution_incdtresolution_id_seq') AS _incdtresolution_id");
    if (incidentSave.first())
      _incdtresolutionId = incidentSave.value("_incdtresolution_id").toInt();
    else if (incidentSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, incidentSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    incidentSave.prepare( "INSERT INTO incdtresolution "
               "(incdtresolution_id, incdtresolution_name, incdtresolution_order, incdtresolution_descrip)"
               " VALUES "
               "(:incdtresolution_id, :incdtresolution_name, :incdtresolution_order, :incdtresolution_descrip);" );
  }
  else if (_mode == cEdit)
  {
    incidentSave.prepare( "SELECT incdtresolution_id "
               "FROM incdtresolution "
               "WHERE ( (UPPER(incdtresolution_name)=UPPER(:incdtresolution_name))"
               " AND (incdtresolution_id<>:incdtresolution_id) );" );
    incidentSave.bindValue(":incdtresolution_id", _incdtresolutionId);
    incidentSave.bindValue(":incdtresolution_name", _name->text());
    incidentSave.exec();
    if (incidentSave.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Incident Resolution"),
                            tr("You may not rename this Incident Resolution with "
			       "the entered value as it is in use by another "
			       "Incident Resolution.") );
      return;
    }
    else if (incidentSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, incidentSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    incidentSave.prepare( "UPDATE incdtresolution "
               "SET incdtresolution_name=:incdtresolution_name, "
	       "    incdtresolution_order=:incdtresolution_order, "
	       "    incdtresolution_descrip=:incdtresolution_descrip "
               "WHERE (incdtresolution_id=:incdtresolution_id);" );
  }

  incidentSave.bindValue(":incdtresolution_id", _incdtresolutionId);
  incidentSave.bindValue(":incdtresolution_name", _name->text());
  incidentSave.bindValue(":incdtresolution_order", _order->value());
  incidentSave.bindValue(":incdtresolution_descrip", _descrip->toPlainText());
  incidentSave.exec();
  if (incidentSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, incidentSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_incdtresolutionId);
}

void incidentResolution::populate()
{
  XSqlQuery incidentpopulate;
  incidentpopulate.prepare( "SELECT * "
             "FROM incdtresolution "
             "WHERE (incdtresolution_id=:incdtresolution_id);" );
  incidentpopulate.bindValue(":incdtresolution_id", _incdtresolutionId);
  incidentpopulate.exec();
  if (incidentpopulate.first())
  {
    _name->setText(incidentpopulate.value("incdtresolution_name").toString());
    _order->setValue(incidentpopulate.value("incdtresolution_order").toInt());
    _descrip->setText(incidentpopulate.value("incdtresolution_descrip").toString());
  }
  else if (incidentpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, incidentpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
