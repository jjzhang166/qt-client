/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentPriority.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include "errorReporter.h"

incidentPriority::incidentPriority(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

incidentPriority::~incidentPriority()
{
}

void incidentPriority::languageChange()
{
  retranslateUi(this);
}

enum SetResponse incidentPriority::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("incdtpriority_id", &valid);
  if (valid)
  {
    _incdtpriorityId = param.toInt();
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

void incidentPriority::sCheck()
{
  XSqlQuery incidentCheck;
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    incidentCheck.prepare( "SELECT incdtpriority_id "
               "FROM incdtpriority "
               "WHERE (UPPER(incdtpriority_name)=UPPER(:incdtpriority_name));" );
    incidentCheck.bindValue(":incdtpriority_name", _name->text());
    incidentCheck.exec();
    if (incidentCheck.first())
    {
      _incdtpriorityId = incidentCheck.value("incdtpriority_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
  }
}

void incidentPriority::sSave()
{
  XSqlQuery incidentSave;
  if(_name->text().length() == 0)
  {
    QMessageBox::critical(this, tr("Priority Name Required"),
      tr("You must enter a Priority Name to continue.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    incidentSave.exec("SELECT NEXTVAL('incdtpriority_incdtpriority_id_seq') AS _incdtpriority_id");
    if (incidentSave.first())
      _incdtpriorityId = incidentSave.value("_incdtpriority_id").toInt();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Incident Priority"),
                                  incidentSave, __FILE__, __LINE__))
    {
      return;
    }

    incidentSave.prepare( "INSERT INTO incdtpriority "
               "(incdtpriority_id, incdtpriority_name, incdtpriority_order, incdtpriority_descrip)"
               " VALUES "
               "(:incdtpriority_id, :incdtpriority_name, :incdtpriority_order, :incdtpriority_descrip);" );
  }
  else if (_mode == cEdit)
  {
    incidentSave.prepare( "SELECT incdtpriority_id "
               "FROM incdtpriority "
               "WHERE ( (UPPER(incdtpriority_name)=UPPER(:incdtpriority_name))"
               " AND (incdtpriority_id<>:incdtpriority_id) );" );
    incidentSave.bindValue(":incdtpriority_id", _incdtpriorityId);
    incidentSave.bindValue(":incdtpriority_name", _name->text());
    incidentSave.exec();
    if (incidentSave.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Incident Priority"),
                            tr("You may not rename this Incident Priority with "
			       "the entered value as it is in use by another "
			       "Incident Priority.") );
      return;
    }
     else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Incident Priority"),
                                  incidentSave, __FILE__, __LINE__))
    {
      return;
    }

    incidentSave.prepare( "UPDATE incdtpriority "
               "SET incdtpriority_name=:incdtpriority_name, "
	       "    incdtpriority_order=:incdtpriority_order, "
	       "    incdtpriority_descrip=:incdtpriority_descrip "
               "WHERE (incdtpriority_id=:incdtpriority_id);" );
  }

  incidentSave.bindValue(":incdtpriority_id", _incdtpriorityId);
  incidentSave.bindValue(":incdtpriority_name", _name->text());
  incidentSave.bindValue(":incdtpriority_order", _order->value());
  incidentSave.bindValue(":incdtpriority_descrip", _descrip->toPlainText());
  incidentSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Incident Priority"),
                                incidentSave, __FILE__, __LINE__))
  {
    return;
  }

  done(_incdtpriorityId);
}

void incidentPriority::populate()
{
  XSqlQuery incidentpopulate;
  incidentpopulate.prepare( "SELECT * "
             "FROM incdtpriority "
             "WHERE (incdtpriority_id=:incdtpriority_id);" );
  incidentpopulate.bindValue(":incdtpriority_id", _incdtpriorityId);
  incidentpopulate.exec();
  if (incidentpopulate.first())
  {
    _name->setText(incidentpopulate.value("incdtpriority_name").toString());
    _order->setValue(incidentpopulate.value("incdtpriority_order").toInt());
    _descrip->setText(incidentpopulate.value("incdtpriority_descrip").toString());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Incident Priority Information"),
                                incidentpopulate, __FILE__, __LINE__))
  {
    return;
  }
}
