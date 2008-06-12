/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "todoItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "storedProcErrorLookup.h"

todoItem::todoItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _seq->setValidator(new QIntValidator(0, 99999, this));

  connect(_close,	SIGNAL(clicked()),	this,	SLOT(sClose()));
  connect(_incident,	SIGNAL(newId(int)),	this,	SLOT(sHandleIncident()));
  connect(_save,	SIGNAL(clicked()),	this,	SLOT(sSave()));

  _started->setAllowNullDate(true);
  _due->setAllowNullDate(true);
  _assigned->setAllowNullDate(true);
  _completed->setAllowNullDate(true);

  q.prepare("SELECT usr_id "
	    "FROM usr "
	    "WHERE (usr_username=CURRENT_USER);");
  q.exec();
  if (q.first())
    _usr->setId(q.value("usr_id").toInt());
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    reject();
  }
}

void todoItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse todoItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("usr_id", &valid);
  if (valid)
    _usr->setId(param.toInt());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _name->setFocus();
      _reassignUsr->setEnabled(_privileges->check("MaintainOtherTodoLists") ||
			  _privileges->check("ReassignTodoListItem"));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _name->setEnabled(FALSE);
      _seq->setEnabled(FALSE);
      _incident->setEnabled(FALSE);
      _ophead->setEnabled(FALSE);
      _assigned->setEnabled(FALSE);
      _due->setEnabled(FALSE);
      _reassignUsr->setEnabled(_privileges->check("MaintainOtherTodoLists") ||
			    _privileges->check("ReassignTodoListItem"));
      _description->setEnabled(FALSE);

      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _seq->setEnabled(FALSE);
      _incident->setEnabled(FALSE);
      _ophead->setEnabled(FALSE);
      _started->setEnabled(FALSE);
      _assigned->setEnabled(FALSE);
      _due->setEnabled(FALSE);
      _completed->setEnabled(FALSE);
      _pending->setEnabled(FALSE);
      _deferred->setEnabled(FALSE);
      _neither->setEnabled(FALSE);
      _reassignUsr->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _notes->setEnabled(FALSE);

      _close->setText(tr("&Close"));
      _save->hide();
      
      _close->setFocus();
    }
  }

  param = pParams.value("incdt_id", &valid);
  if (valid)
    _incident->setId(param.toInt());

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
  QString storedProc;
  if (_mode == cNew)
  {
    q.prepare( "SELECT createTodoItem(:usr_id, :name, :description, "
	       "  :incdt_id, :crmacct_id, :ophead_id, :started, :due, :status, "
	       "  :assigned, :completed, :seq, :notes) AS result;");
    storedProc = "createTodoItem";
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT updateTodoItem(:todoitem_id, "
	       "  :usr_id, :name, :description, "
	       "  :incdt_id, :crmacct_id, :ophead_id, :started, :due, :status, "
	       "  :assigned, :completed, :seq, :notes, :active) AS result;");
    storedProc = "updateTodoItem";
    q.bindValue(":todoitem_id", _todoitemid);
  }
  if (_reassignUsr->id() > 0)
  {
    q.bindValue(":usr_id",   _reassignUsr->id());
    q.bindValue(":assigned", _assigned->date().isValid() ? _assigned->date() :
							  QDate::currentDate());
  }
  else
  {
    q.bindValue(":usr_id",	_usr->id());
    q.bindValue(":assigned",	_assigned->date());
  }

  q.bindValue(":name",		_name->text());
  q.bindValue(":description",	_description->text());
  if (_incident->id() > 0)
    q.bindValue(":incdt_id",	_incident->id());	// else NULL
  if (_crmacct->id() > 0)
    q.bindValue(":crmacct_id",	_crmacct->id());	// else NULL
  q.bindValue(":started",	_started->date());
  q.bindValue(":due",		_due->date());
  q.bindValue(":assigned",	_assigned->date());
  q.bindValue(":completed",	_completed->date());
  q.bindValue(":seq",		_seq->text().toInt());
  q.bindValue(":notes",		_notes->text());
  q.bindValue(":active",	QVariant(_active->isChecked(), 0));
  if(_ophead->id() > 0)
    q.bindValue(":ophead_id", _ophead->id());

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
  q.bindValue(":status", status);
  
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup(storedProc, result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  accept();
}

void todoItem::sPopulate()
{
  q.prepare( "SELECT * "
             "FROM todoitem "
             "WHERE (todoitem_id=:todoitem_id);" );
  q.bindValue(":todoitem_id", _todoitemid);
  q.exec();
  if (q.first())
  {
    _usr->setId(q.value("todoitem_usr_id").toInt());
    _name->setText(q.value("todoitem_name").toString());
    _seq->setText(q.value("todoitem_seq").toString());
    _incident->setId(q.value("todoitem_incdt_id").toInt());
    _ophead->setId(q.value("todoitem_ophead_id").toInt());
    _started->setDate(q.value("todoitem_start_date").toDate());
    _assigned->setDate(q.value("todoitem_assigned_date").toDate());
    _due->setDate(q.value("todoitem_due_date").toDate());
    _completed->setDate(q.value("todoitem_completed_date").toDate());
    _description->setText(q.value("todoitem_description").toString());
    _notes->setText(q.value("todoitem_notes").toString());
    _crmacct->setId(q.value("todoitem_crmacct_id").toInt());
    _active->setChecked(q.value("todoitem_active").toBool());

    if (q.value("todoitem_status").toString() == "P")
      _pending->setChecked(true);
    else if (q.value("todoitem_status").toString() == "D")
      _deferred->setChecked(true);
    else
      _neither->setChecked(true);

    if (cEdit == _mode && 
	(omfgThis->username()==q.value("todoitem_creator_username").toString() ||
	 _privileges->check("OverrideTodoListItemData")))
    {
      _name->setEnabled(true);
      _incident->setEnabled(true);
      _ophead->setEnabled(true);
      _assigned->setEnabled(true);
      _due->setEnabled(true);
      _description->setEnabled(true);
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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
    else if (incdtq.lastError().type() != QSqlError::None)
    {
      systemError(this, incdtq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
