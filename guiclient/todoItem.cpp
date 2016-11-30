/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "todoItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "storedProcErrorLookup.h"
#include "errorReporter.h"

bool todoItem::userHasPriv(const int pMode, const int pId)
{
  if (_privileges->check("MaintainAllToDoItems"))
    return true;
  bool personalPriv = _privileges->check("MaintainPersonalToDoItems");
  if(pMode==cView)
  {
    if(_privileges->check("ViewAllToDoItems"))
      return true;
    personalPriv = personalPriv || _privileges->check("ViewPersonalToDoItems");
  }

  if(pMode==cNew)
    return personalPriv;
  else
  {
    XSqlQuery usernameCheck;
    usernameCheck.prepare( "SELECT getEffectiveXtUser() IN (todoitem_owner_username, todoitem_username) AS canModify "
                           "FROM todoitem "
                            "WHERE (todoitem_id=:todoitem_id);" );
    usernameCheck.bindValue(":todoitem_id", pId);
    usernameCheck.exec();

    if (usernameCheck.first())
      return usernameCheck.value("canModify").toBool()&&personalPriv;
    return false;
  }
}

todoItem::todoItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  if(!_privileges->check("EditOwner")) _owner->setEnabled(false);

  connect(_buttonBox,	SIGNAL(rejected()),	this,	SLOT(sClose()));
  connect(_incident,	SIGNAL(newId(int)),	this,	SLOT(sHandleIncident()));
  connect(_buttonBox,	SIGNAL(accepted()),	this,	SLOT(sSave()));

  _started->setAllowNullDate(true);
  _due->setAllowNullDate(true);
  _assigned->setAllowNullDate(true);
  _completed->setAllowNullDate(true);
  _priority->setType(XComboBox::IncidentPriority);

  _assignedTo->setUsername(omfgThis->username());
  _owner->setUsername(omfgThis->username());
  _owner->setType(UsernameLineEdit::UsersActive);
  _assignedTo->setType(UsernameLineEdit::UsersActive);

  _assignedTo->setEnabled(_privileges->check("MaintainAllToDoItems"));

}

void todoItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse todoItem::set(const ParameterList &pParams)
{
  XSqlQuery todoet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("username", &valid);
  if (valid)
    _assignedTo->setUsername(param.toString());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      todoet.exec("SELECT NEXTVAL('todoitem_todoitem_id_seq') AS todoitem_id");
      if (todoet.first())
      {
        _todoitemid = todoet.value("todoitem_id").toInt();
        _alarms->setId(_todoitemid);
        _comments->setId(_todoitemid);
        _documents->setId(_todoitemid);
        _recurring->setParent(_todoitemid, "TODO");
      }

      _assignedTo->setEnabled(_privileges->check("ReassignToDoItems"));
      _assignedTo->setEnabled(_privileges->check("MaintainAllToDoItems"));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _name->setEnabled(false);
      _incident->setEnabled(false);
      _ophead->setEnabled(false);
      _assigned->setEnabled(false);
      _due->setEnabled(false);
      _assignedTo->setEnabled(_privileges->check("ReassignToDoItems"));
      _description->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _owner->setEnabled(false);
      _name->setEnabled(false);
      _priority->setEnabled(false);
      _incident->setEnabled(false);
      _ophead->setEnabled(false);
      _started->setEnabled(false);
      _assigned->setEnabled(false);
      _due->setEnabled(false);
      _completed->setEnabled(false);
      _pending->setEnabled(false);
      _deferred->setEnabled(false);
      _neither->setEnabled(false);
      _assignedTo->setEnabled(false);
      _description->setEnabled(false);
      _notes->setEnabled(false);
      _alarms->setReadOnly(true);
      _comments->setReadOnly(true);
      _documents->setReadOnly(true);
      _crmacct->setReadOnly(true);
      _cntct->setReadOnly(true);

      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  param = pParams.value("incdt_id", &valid);
  if (valid)
    _incident->setId(param.toInt());

  param = pParams.value("priority_id", &valid);
  if (valid)
    _priority->setId(param.toInt());

  param = pParams.value("ophead_id", &valid);
  if (valid)
    _ophead->setId(param.toInt());

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
    _crmacct->setEnabled(false);
    _incident->setExtraClause(QString(" (incdt_crmacct_id=%1) ")
				.arg(_crmacct->id()));
    _ophead->setExtraClause(QString(" (ophead_crmacct_id=%1) ")
                                .arg(_crmacct->id()));
  }

  param = pParams.value("todoitem_id", &valid);
  if (valid)
  {
    _todoitemid = param.toInt();
    sPopulate();
  }

  return NoError;
}

void todoItem::sSave()
{
  XSqlQuery todoSave;
  RecurrenceWidget::RecurrenceChangePolicy cp = _recurring->getChangePolicy();
  if (cp == RecurrenceWidget::NoPolicy)
    return;

  QString storedProc;
  XSqlQuery beginq("BEGIN;");
  XSqlQuery rollbackq;
  rollbackq.prepare("ROLLBACK;");
  if (_mode == cNew)
  {
    todoSave.prepare( "SELECT createTodoItem(:todoitem_id, :username, :name, :description, "
	       "  :incdt_id, :crmacct_id, :ophead_id, :started, :due, :status, "
               "  :assigned, :completed, :priority, :notes, :owner, :cntct_id) AS result;");
    storedProc = "createTodoItem";
  }
  else if (_mode == cEdit)
  {
    todoSave.prepare( "SELECT updateTodoItem(:todoitem_id, "
	       "  :username, :name, :description, "
	       "  :incdt_id, :crmacct_id, :ophead_id, :started, :due, :status, "
               "  :assigned, :completed, :priority, :notes, :active, :owner, :cntct_id) AS result;");
    storedProc = "updateTodoItem";
  }
  todoSave.bindValue(":todoitem_id", _todoitemid);
  todoSave.bindValue(":owner", _owner->username());
  todoSave.bindValue(":username",   _assignedTo->username());
  if(_assigned->date().isValid())
    todoSave.bindValue(":assigned", _assigned->date());
  todoSave.bindValue(":name",		_name->text());
  todoSave.bindValue(":description",	_description->text());
  if (_incident->id() > 0)
    todoSave.bindValue(":incdt_id",	_incident->id());	// else NULL
  if (_crmacct->id() > 0)
    todoSave.bindValue(":crmacct_id",	_crmacct->id());	// else NULL
  todoSave.bindValue(":started",	_started->date());
  todoSave.bindValue(":due",		_due->date());
  todoSave.bindValue(":assigned",	_assigned->date());
  todoSave.bindValue(":completed",	_completed->date());
  if(_priority->isValid())
    todoSave.bindValue(":priority", _priority->id());
  todoSave.bindValue(":notes",		_notes->toPlainText());
  todoSave.bindValue(":active",	QVariant(_active->isChecked()));
  if(_ophead->id() > 0)
    todoSave.bindValue(":ophead_id", _ophead->id());

  QString status;
  if (_completed->date().isValid())
    status = "C";
  else if (_deferred->isChecked())
    status = "D";
  else if (_pending->isChecked())
    status = "P";
  else if (_started->date().isValid())
    status = "I";
  else
    status = "N";
  todoSave.bindValue(":status", status);

  if (_cntct->isValid())
    todoSave.bindValue(":cntct_id", _cntct->id());

  todoSave.exec();
  if (todoSave.first())
  {
    int result = todoSave.value("result").toInt();
    if (result < 0)
    {
      rollbackq.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving To Do Information"),
                             storedProcErrorLookup(storedProc, result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (todoSave.lastError().type() != QSqlError::NoError)
  {
    rollbackq.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving To Do Information"),
                         todoSave, __FILE__, __LINE__);
    return;
  }

  // TODO: make this part of {create,update}TodoItem?
  if (_recurring->isRecurring())
  {
    XSqlQuery recurq;
    recurq.prepare("UPDATE todoitem"
                   "   SET todoitem_recurring_todoitem_id=:parent_id"
                   " WHERE todoitem_id=:id;");
    recurq.bindValue(":parent_id", _recurring->parentId());
    recurq.bindValue(":id",        _todoitemid);
    if (! recurq.exec())
    {
      rollbackq.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving To Do Information"),
                           recurq, __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    XSqlQuery recurq;
    recurq.prepare("UPDATE todoitem"
                   "   SET todoitem_recurring_todoitem_id=NULL"
                   " WHERE todoitem_id=:id;");
    recurq.bindValue(":id",        _todoitemid);
    if (! recurq.exec())
    {
      rollbackq.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving To Do Information"),
                           recurq, __FILE__, __LINE__);
      return;
    }
  }

  QString errmsg;
  if (! _recurring->save(true, cp, &errmsg))
  {
    rollbackq.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: %2")
                         .arg(windowTitle())
                         .arg(errmsg),__FILE__,__LINE__);
    return;
  }

  XSqlQuery commitq("COMMIT;");

  done(_todoitemid);
}

void todoItem::sPopulate()
{
  XSqlQuery todoPopulate;
  todoPopulate.prepare( "SELECT * "
             "FROM todoitem "
             "WHERE (todoitem_id=:todoitem_id);" );
  todoPopulate.bindValue(":todoitem_id", _todoitemid);
  todoPopulate.exec();
  if (todoPopulate.first())
  {
    _owner->setUsername(todoPopulate.value("todoitem_owner_username").toString());
    _assignedTo->setUsername(todoPopulate.value("todoitem_username").toString());
    _name->setText(todoPopulate.value("todoitem_name").toString());
    _priority->setNull();
    if(!todoPopulate.value("todoitem_priority_id").toString().isEmpty())
      _priority->setId(todoPopulate.value("todoitem_priority_id").toInt());
    _incident->setId(todoPopulate.value("todoitem_incdt_id").toInt());
    _ophead->setId(todoPopulate.value("todoitem_ophead_id").toInt());
    _started->setDate(todoPopulate.value("todoitem_start_date").toDate());
    _assigned->setDate(todoPopulate.value("todoitem_assigned_date").toDate());
    _due->setDate(todoPopulate.value("todoitem_due_date").toDate(), true);
    _completed->setDate(todoPopulate.value("todoitem_completed_date").toDate());
    _description->setText(todoPopulate.value("todoitem_description").toString());
    _notes->setText(todoPopulate.value("todoitem_notes").toString());
    _crmacct->setId(todoPopulate.value("todoitem_crmacct_id").toInt());
    _cntct->setId(todoPopulate.value("todoitem_cntct_id").toInt());
    _active->setChecked(todoPopulate.value("todoitem_active").toBool());

    if (todoPopulate.value("todoitem_status").toString() == "P")
      _pending->setChecked(true);
    else if (todoPopulate.value("todoitem_status").toString() == "D")
      _deferred->setChecked(true);
    else
      _neither->setChecked(true);

    if (cEdit == _mode && 
	(omfgThis->username()==todoPopulate.value("todoitem_creator_username").toString() ||
         _privileges->check("MaintainAllToDoItems")))
    {
      _name->setEnabled(true);
      _incident->setEnabled(true);
      _ophead->setEnabled(true);
      _assigned->setEnabled(true);
      _due->setEnabled(true);
      _description->setEnabled(true);
    }

    _alarms->setId(_todoitemid);
    _comments->setId(_todoitemid);
    _documents->setId(_todoitemid);
    _recurring->setParent(todoPopulate.value("todoitem_recurring_todoitem_id").isNull() ?
                          _todoitemid : todoPopulate.value("todoitem_recurring_todoitem_id").toInt(),
                          "TODO");
    _cntct->setId(todoPopulate.value("todoitem_cntct_id").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving To Do Information"),
                                todoPopulate, __FILE__, __LINE__))
  {
    return;
  }
}

void todoItem::sClose()
{
  reject();
}

void todoItem::sHandleIncident()
{
  _crmacct->setEnabled(! _incident->isValid());

  if (_incident->isValid())
  {
    XSqlQuery incdtq;
    incdtq.prepare("SELECT incdt_crmacct_id "
		   "FROM incdt "
		   "WHERE (incdt_id=:incdt_id);");
    incdtq.bindValue(":incdt_id", _incident->id());
    incdtq.exec();
    if (incdtq.first())
      _crmacct->setId(incdtq.value("incdt_crmacct_id").toInt());
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Incident Information"),
                                  incdtq, __FILE__, __LINE__))
    {
      return;
    }
  }
}
