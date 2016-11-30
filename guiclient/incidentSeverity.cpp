/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentSeverity.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include "errorReporter.h"

incidentSeverity::incidentSeverity(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

incidentSeverity::~incidentSeverity()
{
}

void incidentSeverity::languageChange()
{
  retranslateUi(this);
}

enum SetResponse incidentSeverity::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("incdtseverity_id", &valid);
  if (valid)
  {
    _incdtseverityId = param.toInt();
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

void incidentSeverity::sCheck()
{
  XSqlQuery incidentCheck;
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    incidentCheck.prepare( "SELECT incdtseverity_id "
               "FROM incdtseverity "
               "WHERE (UPPER(incdtseverity_name)=UPPER(:incdtseverity_name));" );
    incidentCheck.bindValue(":incdtseverity_name", _name->text());
    incidentCheck.exec();
    if (incidentCheck.first())
    {
      _incdtseverityId = incidentCheck.value("incdtseverity_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
  }
}

void incidentSeverity::sSave()
{
  XSqlQuery incidentSave;
  if(_name->text().length() == 0)
  {
    QMessageBox::critical(this, tr("Severity Name Required"),
      tr("You must enter a Severity Name to continue.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    incidentSave.exec("SELECT NEXTVAL('incdtseverity_incdtseverity_id_seq') AS _incdtseverity_id");
    if (incidentSave.first())
      _incdtseverityId = incidentSave.value("_incdtseverity_id").toInt();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Incident Severity"),
                                  incidentSave, __FILE__, __LINE__))
    {
      return;
    }

    incidentSave.prepare( "INSERT INTO incdtseverity "
               "(incdtseverity_id, incdtseverity_name, incdtseverity_order, incdtseverity_descrip)"
               " VALUES "
               "(:incdtseverity_id, :incdtseverity_name, :incdtseverity_order, :incdtseverity_descrip);" );
  }
  else if (_mode == cEdit)
  {
    incidentSave.prepare( "SELECT incdtseverity_id "
               "FROM incdtseverity "
               "WHERE ( (UPPER(incdtseverity_name)=UPPER(:incdtseverity_name))"
               " AND (incdtseverity_id<>:incdtseverity_id) );" );
    incidentSave.bindValue(":incdtseverity_id", _incdtseverityId);
    incidentSave.bindValue(":incdtseverity_name", _name->text());
    incidentSave.exec();
    if (incidentSave.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Incident Severity"),
                            tr("You may not rename this Incident Severity with "
			       "the entered value as it is in use by another "
			       "Incident Severity.") );
      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Incident Severity"),
                                  incidentSave, __FILE__, __LINE__))
    {
      return;
    }

    incidentSave.prepare( "UPDATE incdtseverity "
               "SET incdtseverity_name=:incdtseverity_name, "
	       "    incdtseverity_order=:incdtseverity_order, "
	       "    incdtseverity_descrip=:incdtseverity_descrip "
               "WHERE (incdtseverity_id=:incdtseverity_id);" );
  }

  incidentSave.bindValue(":incdtseverity_id", _incdtseverityId);
  incidentSave.bindValue(":incdtseverity_name", _name->text());
  incidentSave.bindValue(":incdtseverity_order", _order->value());
  incidentSave.bindValue(":incdtseverity_descrip", _descrip->toPlainText());
  incidentSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Incident Severity"),
                                incidentSave, __FILE__, __LINE__))
  {
    return;
  }

  done(_incdtseverityId);
}

void incidentSeverity::populate()
{
  XSqlQuery incidentpopulate;
  incidentpopulate.prepare( "SELECT * "
             "FROM incdtseverity "
             "WHERE (incdtseverity_id=:incdtseverity_id);" );
  incidentpopulate.bindValue(":incdtseverity_id", _incdtseverityId);
  incidentpopulate.exec();
  if (incidentpopulate.first())
  {
    _name->setText(incidentpopulate.value("incdtseverity_name").toString());
    _order->setValue(incidentpopulate.value("incdtseverity_order").toInt());
    _descrip->setText(incidentpopulate.value("incdtseverity_descrip").toString());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Incident Severity Information"),
                                incidentpopulate, __FILE__, __LINE__))
  {
    return;
  }
}
