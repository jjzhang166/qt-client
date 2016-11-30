/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "department.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include "errorReporter.h"

department::department(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(sClose()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));

  _deptid = -1;
}

department::~department()
{
  // no need to delete child widgets, Qt does it all for us
}

void department::languageChange()
{
  retranslateUi(this);
}

enum SetResponse department::set(const ParameterList& pParams)
{
  XDialog::set(pParams);
  QVariant        param;
  bool        valid;

  param = pParams.value("dept_id", &valid);
  if (valid)
  {
    _deptid = param.toInt();
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
       _number->setEnabled(false);
       _name->setEnabled(false);
       _buttonBox->clear();
       _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void department::sSave()
{
  XSqlQuery departmentSave;
  QString number = _number->text().trimmed().toUpper();

  if (number.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Department"),
                          tr("You must enter a Department Number"));
    _number->setFocus();
    return;
  }
  if (_name->text().trimmed().isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Department"),
                          tr("You must enter a Department Name"));
    _name->setFocus();
    return;
  }

  departmentSave.prepare("SELECT dept_id"
            "  FROM dept"
            " WHERE((dept_id != :dept_id)"
            "   AND (dept_number=:dept_number));");
  departmentSave.bindValue(":dept_id", _deptid);
  departmentSave.bindValue(":dept_number", number);
  departmentSave.exec();
  if(departmentSave.first())
  {
    QMessageBox::critical(this, tr("Cannot Save Department"),
                          tr("The Number you entered already exists. Please choose a different Number."));
    return;
  }
  
  if (_mode == cNew)
  {
    departmentSave.exec("SELECT NEXTVAL('dept_dept_id_seq') AS dept_id;");
    if (departmentSave.first())
      _deptid =  departmentSave.value("dept_id").toInt();
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Department Information"),
                           departmentSave, __FILE__, __LINE__);
      return;
    }
    departmentSave.prepare("INSERT INTO dept ( dept_id,  dept_number,  dept_name ) "
              "       VALUES    (:dept_id, :dept_number, :dept_name );");
  }
  else if (_mode == cEdit)
  {
    departmentSave.prepare("UPDATE dept "
              "SET dept_id=:dept_id, "
              "    dept_number=:dept_number, "
              "    dept_name=:dept_name "
              "WHERE (dept_id=:dept_id);");
  }
  departmentSave.bindValue(":dept_id",        _deptid);
  departmentSave.bindValue(":dept_number",        number);
  departmentSave.bindValue(":dept_name",        _name->text().trimmed());

  departmentSave.exec();
  if (departmentSave.lastError().type() != QSqlError::NoError)
  {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Department Information"),
                           departmentSave, __FILE__, __LINE__);
    return;
  }

  done(_deptid);
}

void department::sClose()
{
  close();
}

void department::populate()
{
  XSqlQuery departmentpopulate;
  departmentpopulate.prepare("SELECT dept_number, dept_name "
            "FROM dept "
            "WHERE (dept_id=:dept_id);");
  departmentpopulate.bindValue(":dept_id", _deptid);
  departmentpopulate.exec();
  if (departmentpopulate.first())
  {
    _number->setText(departmentpopulate.value("dept_number"));
    _name->setText(departmentpopulate.value("dept_name"));
  }
  else
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Department Information"),
                           departmentpopulate, __FILE__, __LINE__);
}

