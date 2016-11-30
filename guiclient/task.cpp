/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "task.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "userList.h"

const char *_taskStatuses[] = { "P", "O", "C" };

bool task::userHasPriv(const int pMode, const int pId)
{
  if (_privileges->check("MaintainAllProjects"))
    return true;
  bool personalPriv = _privileges->check("MaintainPersonalProjects");
  if(pMode==cView)
  {
    if(_privileges->check("ViewAllProjects"))
      return true;
    personalPriv = personalPriv || _privileges->check("ViewPersonalProjects");
  }

  if(pMode==cNew)
    return personalPriv;
  else
  {
    XSqlQuery usernameCheck;
    usernameCheck.prepare( "SELECT getEffectiveXtUser() IN (prjtask_owner_username, prjtask_username) AS canModify "
                           "FROM prjtask "
                            "WHERE (prjtask_id=:prjtask_id);" );
    usernameCheck.bindValue(":prjtask_id", pId);
    usernameCheck.exec();

    if (usernameCheck.first())
      return usernameCheck.value("canModify").toBool()&&personalPriv;
    return false;
  }
}

task::task(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_actualExp, SIGNAL(editingFinished()), this, SLOT(sExpensesAdjusted()));
  connect(_budgetExp, SIGNAL(editingFinished()), this, SLOT(sExpensesAdjusted()));
  connect(_actualHours, SIGNAL(editingFinished()), this, SLOT(sHoursAdjusted()));
  connect(_budgetHours, SIGNAL(editingFinished()), this, SLOT(sHoursAdjusted()));
  
  _budgetHours->setValidator(omfgThis->qtyVal());
  _actualHours->setValidator(omfgThis->qtyVal());
  _budgetExp->setValidator(omfgThis->costVal());
  _actualExp->setValidator(omfgThis->costVal());
  _balanceHours->setPrecision(omfgThis->qtyVal());
  _balanceExp->setPrecision(omfgThis->costVal());

  _prjid = -1;
  _prjtaskid = -1;
  
  _owner->setType(UsernameLineEdit::UsersActive);
  _assignedTo->setType(UsernameLineEdit::UsersActive);
  _charass->setType("TASK");
}

task::~task()
{
  // no need to delete child widgets, Qt does it all for us
}

void task::languageChange()
{
  retranslateUi(this);
}

enum SetResponse task::set(const ParameterList &pParams)
{
  XSqlQuery tasket;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("prj_id", &valid);
  if (valid)
    _prjid = param.toInt();

  param = pParams.value("prj_owner_username", &valid);
  if (valid)
    _owner->setUsername(param.toString());

  param = pParams.value("prj_username", &valid);
  if (valid)
    _assignedTo->setUsername(param.toString());

  param = pParams.value("prj_start_date", &valid);
  if (valid)
    _started->setDate(param.toDate());

  param = pParams.value("prj_assigned_date", &valid);
  if (valid)
    _assigned->setDate(param.toDate());

  param = pParams.value("prj_due_date", &valid);
  if (valid)
    _due->setDate(param.toDate());

  param = pParams.value("prj_completed_date", &valid);
  if (valid)
    _completed->setDate(param.toDate());

  param = pParams.value("prjtask_id", &valid);
  if (valid)
  {
    _prjtaskid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      tasket.exec("SELECT NEXTVAL('prjtask_prjtask_id_seq') AS prjtask_id;");
      if (tasket.first())
        _prjtaskid = tasket.value("prjtask_id").toInt();
      else
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Project Task Information"),
                             tasket, __FILE__, __LINE__);
      }

      connect(_assignedTo, SIGNAL(newId(int)), this, SLOT(sAssignedToChanged(int)));
      connect(_status,  SIGNAL(currentIndexChanged(int)), this, SLOT(sStatusChanged(int)));

      _alarms->setId(_prjtaskid);
      _comments->setId(_prjtaskid);
      _documents->setId(_prjtaskid);
      _charass->setId(_prjtaskid); 
    }
    if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_assignedTo, SIGNAL(newId(int)), this, SLOT(sAssignedToChanged(int)));
      connect(_status,  SIGNAL(currentIndexChanged(int)), this, SLOT(sStatusChanged(int)));
    }
    if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(false);
      _name->setEnabled(false);
      _descrip->setEnabled(false);
      _status->setEnabled(false);
      _budgetHours->setEnabled(false);
      _actualHours->setEnabled(false);
      _budgetExp->setEnabled(false);
      _actualExp->setEnabled(false);
      _owner->setEnabled(false);
      _assignedTo->setEnabled(false);
      _due->setEnabled(false);
      _assigned->setEnabled(false);
      _started->setEnabled(false);
      _completed->setEnabled(false);
      _alarms->setEnabled(false);
      _comments->setReadOnly(true);
      _charass->setReadOnly(true);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _documents->setReadOnly(true);
    }
  }

  return NoError;
}

void task::populate()
{
  XSqlQuery taskpopulate;
  taskpopulate.prepare( "SELECT prjtask.* "
             "FROM prjtask "
             "WHERE (prjtask_id=:prjtask_id);" );
  taskpopulate.bindValue(":prjtask_id", _prjtaskid);
  taskpopulate.exec();
  if (taskpopulate.first())
  {
    _number->setText(taskpopulate.value("prjtask_number"));
    _name->setText(taskpopulate.value("prjtask_name"));
    _descrip->setText(taskpopulate.value("prjtask_descrip").toString());
    _owner->setUsername(taskpopulate.value("prjtask_owner_username").toString());
    _assignedTo->setUsername(taskpopulate.value("prjtask_username").toString());
    _started->setDate(taskpopulate.value("prjtask_start_date").toDate());
    _assigned->setDate(taskpopulate.value("prjtask_assigned_date").toDate());
    _due->setDate(taskpopulate.value("prjtask_due_date").toDate());
    _completed->setDate(taskpopulate.value("prjtask_completed_date").toDate());

    for (int counter = 0; counter < _status->count(); counter++)
    {
      if (QString(taskpopulate.value("prjtask_status").toString()[0]) == _taskStatuses[counter])
        _status->setCurrentIndex(counter);
    }

    _budgetHours->setText(formatQty(taskpopulate.value("prjtask_hours_budget").toDouble()));
    _actualHours->setText(formatQty(taskpopulate.value("prjtask_hours_actual").toDouble()));
    _budgetExp->setText(formatCost(taskpopulate.value("prjtask_exp_budget").toDouble()));
    _actualExp->setText(formatCost(taskpopulate.value("prjtask_exp_actual").toDouble()));

    _alarms->setId(_prjtaskid);
    _comments->setId(_prjtaskid);   
    _documents->setId(_prjtaskid); 
    _charass->setId(_prjtaskid); 
    _documents->setType(Documents::ProjectTask);
    sHoursAdjusted();
    sExpensesAdjusted();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Task Information"),
                                taskpopulate, __FILE__, __LINE__))
  {
    return;
  }
}

void task::sSave()
{
  XSqlQuery taskSave;
  QList<GuiErrorCheck> errors;
  errors<< GuiErrorCheck(_number->text().length() == 0, _number,
                         tr("You must enter a valid Number."))
        << GuiErrorCheck(_name->text().length() == 0, _name,
                         tr("You must enter a valid Name."))
        << GuiErrorCheck(!_due->isValid(), _due,
                         tr("You must enter a valid due date."))
  ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Project Task"), errors))
    return;

  if (_mode == cNew)
  {
    taskSave.prepare( "INSERT INTO prjtask "
               "( prjtask_id, prjtask_prj_id, prjtask_number,"
               "  prjtask_name, prjtask_descrip, prjtask_status,"
               "  prjtask_hours_budget, prjtask_hours_actual,"
               "  prjtask_exp_budget, prjtask_exp_actual,"
               "  prjtask_start_date, prjtask_due_date,"
               "  prjtask_assigned_date, prjtask_completed_date,"
               "  prjtask_owner_username, prjtask_username ) "
               "VALUES "
               "( :prjtask_id, :prjtask_prj_id, :prjtask_number,"
               "  :prjtask_name, :prjtask_descrip, :prjtask_status,"
               "  :prjtask_hours_budget, :prjtask_hours_actual,"
               "  :prjtask_exp_budget, :prjtask_exp_actual,"
               "  :prjtask_start_date, :prjtask_due_date,"
               "  :prjtask_assigned_date, :prjtask_completed_date,"
               "  :prjtask_owner_username, :username );" );
    taskSave.bindValue(":prjtask_prj_id", _prjid);
  }
  else if (_mode == cEdit)
    taskSave.prepare( "UPDATE prjtask "
               "SET prjtask_number=:prjtask_number, prjtask_name=:prjtask_name,"
               "    prjtask_descrip=:prjtask_descrip, prjtask_status=:prjtask_status,"
               "    prjtask_hours_budget=:prjtask_hours_budget,"
               "    prjtask_hours_actual=:prjtask_hours_actual,"
               "    prjtask_exp_budget=:prjtask_exp_budget,"
               "    prjtask_exp_actual=:prjtask_exp_actual,"
               "    prjtask_owner_username=:prjtask_owner_username,"
               "    prjtask_username=:username,"
               "    prjtask_start_date=:prjtask_start_date,"
               "    prjtask_due_date=:prjtask_due_date,"
               "    prjtask_assigned_date=:prjtask_assigned_date,"
               "    prjtask_completed_date=:prjtask_completed_date "
               "WHERE (prjtask_id=:prjtask_id);" );

  taskSave.bindValue(":prjtask_id", _prjtaskid);
  taskSave.bindValue(":prjtask_number", _number->text());
  taskSave.bindValue(":prjtask_name", _name->text());
  taskSave.bindValue(":prjtask_descrip", _descrip->toPlainText());
  taskSave.bindValue(":prjtask_status", _taskStatuses[_status->currentIndex()]);
  taskSave.bindValue(":prjtask_owner_username", _owner->username());
  taskSave.bindValue(":username",   _assignedTo->username());
  taskSave.bindValue(":prjtask_start_date", _started->date());
  taskSave.bindValue(":prjtask_due_date", _due->date());
  taskSave.bindValue(":prjtask_assigned_date",	_assigned->date());
  taskSave.bindValue(":prjtask_completed_date", _completed->date());
  //taskSave.bindValue(":prjtask_anyuser", QVariant(_anyUser->isChecked()));
  taskSave.bindValue(":prjtask_hours_budget", _budgetHours->text().toDouble());
  taskSave.bindValue(":prjtask_hours_actual", _actualHours->text().toDouble());
  taskSave.bindValue(":prjtask_exp_budget", _budgetExp->text().toDouble());
  taskSave.bindValue(":prjtask_exp_actual", _actualExp->text().toDouble());

  taskSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Task Information"),
                                taskSave, __FILE__, __LINE__))
  {
    return;
  }

  done(_prjtaskid);
}

void task::sAssignedToChanged(const int newid)
{
  if (newid == -1)
    _assigned->clear();
  else
    _assigned->setDate(omfgThis->dbDate());
}

void task::sStatusChanged(const int pStatus)
{
  switch(pStatus)
  {
    case 0: // Concept
    default:
      _started->clear();
      _completed->clear();
      break;
    case 1: // In Process
      _started->setDate(omfgThis->dbDate());
      _completed->clear();
      break;
    case 2: // Completed
      _completed->setDate(omfgThis->dbDate());
      break;
  }
}

void task::sHoursAdjusted()
{
  _balanceHours->setText(formatQty(_budgetHours->text().toDouble() - _actualHours->text().toDouble()));
}

void task::sExpensesAdjusted()
{
  _balanceExp->setText(formatCost(_budgetExp->text().toDouble() - _actualExp->text().toDouble()));
}

void task::sNewUser()
{
  XSqlQuery taskNewUser;
}

void task::sDeleteUser()
{
  XSqlQuery taskDeleteUser;

}


void task::sFillUserList()
{
  XSqlQuery taskFillUserList;

}

