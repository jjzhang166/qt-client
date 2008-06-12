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

#include "opportunity.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QMenu>

#include "storedProcErrorLookup.h"
#include "todoItem.h"
#include "characteristicAssignment.h"

/*
 *  Constructs a opportunity as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
opportunity::opportunity(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_deleteTodoItem, SIGNAL(clicked()), this, SLOT(sDeleteTodoItem()));
  connect(_editTodoItem, SIGNAL(clicked()), this, SLOT(sEditTodoItem()));
  connect(_newTodoItem, SIGNAL(clicked()), this, SLOT(sNewTodoItem()));
  connect(_todoList, SIGNAL(itemSelected(int)), _editTodoItem, SLOT(animateClick()));
  connect(_todoList, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this, SLOT(sPopulateTodoMenu(QMenu*)));
  connect(_todoList, SIGNAL(valid(bool)), this, SLOT(sHandleTodoPrivs()));
  connect(_viewTodoItem, SIGNAL(clicked()), this, SLOT(sViewTodoItem()));
  connect(_charass, SIGNAL(itemSelected(int)), _editCharacteristic, SLOT(animateClick()));
  connect(_newCharacteristic, SIGNAL(clicked()), this, SLOT(sNewCharacteristic()));
  connect(_editCharacteristic, SIGNAL(clicked()), this, SLOT(sEditCharacteristic()));
  connect(_deleteCharacteristic, SIGNAL(clicked()), this, SLOT(sDeleteCharacteristic()));

  _opheadid = -1;

  _todoList->addColumn(tr("Seq"),   25, Qt::AlignRight);
  _todoList->addColumn(tr("User"), _userColumn, Qt::AlignLeft );
  _todoList->addColumn(tr("Name"),  100, Qt::AlignLeft );
  _todoList->addColumn(tr("Description"),  -1, Qt::AlignLeft );
  _todoList->addColumn(tr("Status"),  _statusColumn, Qt::AlignLeft );
  _todoList->addColumn(tr("Due Date"), _dateColumn, Qt::AlignLeft );

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft );
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft );
  _charass->addColumn(tr("Default"),        _ynColumn,   Qt::AlignCenter );

  q.prepare("SELECT usr_id "
     "FROM usr "
     "WHERE (usr_username=CURRENT_USER);");
  q.exec();
  if (q.first())
  {
    _myUsrId = q.value("usr_id").toInt();
    _owner->setId(_myUsrId);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    close();
  }

  _saved = false;
}

/*
 *  Destroys the object and frees any allocated resources
 */
opportunity::~opportunity()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void opportunity::languageChange()
{
  retranslateUi(this);
}

enum SetResponse opportunity::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("ophead_id", &valid);
  if (valid)
  {
    _opheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));

      _comments->setReadOnly(true);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));

      _crmacct->setEnabled(true);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _crmacct->setEnabled(false);
      _owner->setEnabled(false);
      _oppstage->setEnabled(false);
      _oppsource->setEnabled(false);
      _opptype->setEnabled(false);
      _notes->setReadOnly(true);
      _name->setEnabled(false);
      _targetDate->setEnabled(false);
      _actualDate->setEnabled(false);
      _amount->setEnabled(false);
      _probability->setEnabled(false);
      _deleteTodoItem->setEnabled(false);
      _editTodoItem->setEnabled(false);
      _newTodoItem->setEnabled(false);
      _newCharacteristic->setEnabled(FALSE);

      _save->hide();
      _cancel->setText(tr("&Close"));
      _cancel->setFocus();
      _comments->setReadOnly(true);
    }
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
    _crmacct->setEnabled(false);
  }

  sHandleTodoPrivs();
  return NoError;
}

void opportunity::sCancel()
{
  if (_saved && cNew == _mode)
  {
    q.prepare("SELECT deleteOpportunity(:ophead_id) AS result;");
    q.bindValue(":ophead_id", _opheadid);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteOpportunity", result));
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  reject();
}

void opportunity::sSave()
{
  if (! save(false)) // if error
    return;

  accept();
}

bool opportunity::save(bool partial)
{
  if (! partial)
  {
    if(_crmacct->id() == -1)
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
	tr("You must specify the Account that this opportunity is for.") );
      return false;
    }

    if(_name->text().trimmed().isEmpty())
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
	tr("You must specify a Name for this opportunity report.") );
      _name->setFocus();
      return false;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (!q.exec("BEGIN"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  if (cNew == _mode && !_saved)
    q.prepare("INSERT INTO ophead"
              "      (ophead_name, ophead_crmacct_id,"
              "       ophead_owner_username,"
              "       ophead_opstage_id, ophead_opsource_id,"
              "       ophead_optype_id, ophead_probability_prcnt,"
              "       ophead_amount, ophead_curr_id, ophead_target_date,"
              "       ophead_actual_date,"
              "       ophead_notes) "
              "VALUES(:ophead_name, :ophead_crmacct_id,"
              "       :ophead_owner_username,"
              "       :ophead_opstage_id, :ophead_opsource_id,"
              "       :ophead_optype_id, :ophead_probability_prcnt,"
              "       :ophead_amount, :ophead_curr_id, :ophead_target_date,"
              "       :ophead_actual_date,"
              "       :ophead_notes);" );
  else if (cEdit == _mode || _saved)
    q.prepare("UPDATE ophead"
              "   SET ophead_name=:ophead_name,"
              "       ophead_crmacct_id=:ophead_crmacct_id,"
              "       ophead_owner_username=:ophead_owner_username,"
              "       ophead_opstage_id=:ophead_opstage_id,"
              "       ophead_opsource_id=:ophead_opsource_id,"
              "       ophead_optype_id=:ophead_optype_id,"
              "       ophead_probability_prcnt=:ophead_probability_prcnt,"
              "       ophead_amount=:ophead_amount,"
              "       ophead_curr_id=:ophead_curr_id,"
              "       ophead_target_date=:ophead_target_date,"
              "       ophead_actual_date=:ophead_actual_date,"
              "       ophead_notes=:ophead_notes"
              " WHERE (ophead_id=:ophead_id); ");

  q.bindValue(":ophead_id", _opheadid);
  q.bindValue(":ophead_name", _name->text());
  if (_crmacct->id() > 0)
    q.bindValue(":ophead_crmacct_id", _crmacct->id());
  if(_owner->isValid())
    q.bindValue(":ophead_owner_username", _owner->username());
  if(_oppstage->isValid())
    q.bindValue(":ophead_opstage_id", _oppstage->id());
  if(_oppsource->isValid())
    q.bindValue(":ophead_opsource_id", _oppsource->id());
  if(_opptype->isValid())
    q.bindValue(":ophead_optype_id", _opptype->id());
  if(!_probability->text().isEmpty())
    q.bindValue(":ophead_probability_prcnt", _probability->text().toInt());
  if(!_amount->isZero())
  {
    q.bindValue(":ophead_amount", _amount->localValue());
    q.bindValue(":ophead_curr_id", _amount->id());
  }
  if(_targetDate->isValid())
    q.bindValue(":ophead_target_date", _targetDate->date());
  if(_actualDate->isValid())
    q.bindValue(":ophead_actual_date", _actualDate->date());
  q.bindValue(":ophead_notes", _notes->text());

  if(!q.exec() && q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  q.exec("COMMIT;");
  if(q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  if (cNew == _mode && ! _saved)
  {
    q.exec("SELECT CURRVAL('ophead_ophead_id_seq') AS result;");
    if (q.first())
      _opheadid = q.value("result").toInt();
    else if(q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  _saved = true;
  return true;
}

void opportunity::populate()
{ 
  q.prepare("SELECT ophead_name,"
            "       ophead_crmacct_id,"
            "       ophead_owner_username,"
            "       ophead_opstage_id, ophead_opsource_id,"
            "       ophead_optype_id,"
            "       ophead_probability_prcnt, ophead_amount,"
            "       COALESCE(ophead_curr_id, basecurrid()) AS curr_id,"
            "       ophead_target_date, ophead_actual_date,"
            "       ophead_notes"
            "  FROM ophead"
            " WHERE(ophead_id=:ophead_id); ");
  q.bindValue(":ophead_id", _opheadid);
  q.exec();
  if(q.first())
  {
    _name->setText(q.value("ophead_name").toString());
    _crmacct->setId(q.value("ophead_crmacct_id").toInt());
    _owner->setUsername(q.value("ophead_owner_username").toString());

    _oppstage->setNull();
    if(!q.value("ophead_opstage_id").toString().isEmpty())
      _oppstage->setId(q.value("ophead_opstage_id").toInt());

    _oppsource->setNull();
    if(!q.value("ophead_opsource_id").toString().isEmpty())
      _oppsource->setId(q.value("ophead_opsource_id").toInt());

    _opptype->setNull();
    if(!q.value("ophead_optype_id").toString().isEmpty())
      _opptype->setId(q.value("ophead_optype_id").toInt());

    _probability->clear();
    if(!q.value("ophead_probability_prcnt").toString().isEmpty())
      _probability->setText(q.value("ophead_probability_prcnt").toInt());

    _amount->clear();
    _amount->setId(q.value("curr_id").toInt());
    if(!q.value("ophead_amount").toString().isEmpty())
      _amount->setLocalValue(q.value("ophead_amount").toDouble());

    _targetDate->clear();
    if(!q.value("ophead_target_date").toString().isEmpty())
      _targetDate->setDate(q.value("ophead_target_date").toDate());

    _actualDate->clear();
    if(!q.value("ophead_actual_date").toString().isEmpty())
      _actualDate->setDate(q.value("ophead_actual_date").toDate());

    _notes->setText(q.value("ophead_notes").toString());

    _comments->setId(_opheadid);

    sFillTodoList();
    sFillCharList();
  }
}

void opportunity::sNewTodoItem()
{
  if (! save(true))
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("ophead_id", _opheadid);

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillTodoList();
}

void opportunity::sEditTodoItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillTodoList();
}

void opportunity::sViewTodoItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  newdlg.exec();
}

void opportunity::sDeleteTodoItem()
{
  q.prepare("SELECT deleteTodoItem(:todoitem_id) AS result;");
  q.bindValue(":todoitem_id", _todoList->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteTodoItem", result));
      return;
    }
    else
      sFillTodoList();
    }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void opportunity::sFillTodoList()
{
  XTreeWidgetItem* last = 0;
  q.prepare("SELECT todoitem_id, todoitem_usr_id, todoitem_seq, "
	    "       usr_username, todoitem_name, "
	    "       firstLine(todoitem_description) AS todoitem_description, "
	    "       todoitem_status, todoitem_due_date "
	    "FROM usr, todoitem "
	    "WHERE ( (todoitem_ophead_id=:ophead_id) "
	    "  AND   (todoitem_usr_id=usr_id)"
	    "  AND   (todoitem_active) ) "
	    "ORDER BY todoitem_seq, usr_username;");

  q.bindValue(":ophead_id", _opheadid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _todoList->clear();
  while (q.next())
  {
    last = new XTreeWidgetItem(_todoList, last,
			     q.value("todoitem_id").toInt(),
			     q.value("todoitem_usr_id").toInt(),
			     q.value("todoitem_seq").toString(),
			     q.value("usr_username").toString(),
			     q.value("todoitem_name").toString(),
			     q.value("todoitem_description").toString(),
			     q.value("todoitem_status").toString(),
			     q.value("todoitem_due_date").isNull() ? "" :
				    q.value("todoitem_due_date").toString());
    if (q.value("todoitem_status") != "C")
    {
      if (q.value("todoitem_due_date").toDate() < QDate::currentDate())
	last->setTextColor("red");
      else if (q.value("todoitem_due_date").toDate() > QDate::currentDate())
	last->setTextColor("green");
    }
  }
}

void opportunity::sPopulateTodoMenu(QMenu *pMenu)
{
  int menuItem;

  bool newPriv = (cNew == _mode || cEdit == _mode) &&
      (_privileges->check("MaintainPersonalTodoList") ||
       _privileges->check("MaintainOtherTodoLists") );

  bool editPriv = (cNew == _mode || cEdit == _mode) && (
      (_myUsrId == _todoList->altId() && _privileges->check("MaintainPersonalTodoList")) ||
      (_myUsrId != _todoList->altId() && _privileges->check("MaintainOtherTodoLists")) );

  bool viewPriv =
      (_myUsrId == _todoList->altId() && _privileges->check("ViewPersonalTodoList")) ||
      (_myUsrId != _todoList->altId() && _privileges->check("ViewOtherTodoLists"));

  menuItem = pMenu->insertItem(tr("New..."), this, SLOT(sNewTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, newPriv);

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEditTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sViewTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, viewPriv);

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDeleteTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);
}

void opportunity::sHandleTodoPrivs()
{
  bool newPriv = (cNew == _mode || cEdit == _mode) &&
      (_privileges->check("MaintainPersonalTodoList") ||
       _privileges->check("MaintainOtherTodoLists") );

  bool editPriv = (cNew == _mode || cEdit == _mode) && (
      (_myUsrId == _todoList->altId() && _privileges->check("MaintainPersonalTodoList")) ||
      (_myUsrId != _todoList->altId() && _privileges->check("MaintainOtherTodoLists")) );

  bool viewPriv =
      (_myUsrId == _todoList->altId() && _privileges->check("ViewPersonalTodoList")) ||
      (_myUsrId != _todoList->altId() && _privileges->check("ViewOtherTodoLists"));

  _newTodoItem->setEnabled(newPriv);
  _editTodoItem->setEnabled(editPriv && _todoList->id() > 0);
  _viewTodoItem->setEnabled((editPriv || viewPriv) && _todoList->id() > 0);
  _deleteTodoItem->setEnabled(editPriv && _todoList->id() > 0);

  if (editPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_viewTodoItem, SLOT(animateClick()));
    connect(_todoList,	SIGNAL(itemSelected(int)), _editTodoItem, SLOT(animateClick()));
  }
  else if (viewPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_editTodoItem, SLOT(animateClick()));
    connect(_todoList,	SIGNAL(itemSelected(int)), _viewTodoItem, SLOT(animateClick()));
  }
}

void opportunity::sNewCharacteristic()
{
  if (! save(true))
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("ophead_id", _opheadid);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharList();
}

void opportunity::sEditCharacteristic()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharList();
}

void opportunity::sDeleteCharacteristic()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();

  sFillCharList();
}

void opportunity::sFillCharList()
{
  q.prepare( "SELECT charass_id, char_name, charass_value, formatBoolYN(charass_default) "
             "FROM charass, char "
             "WHERE ( (charass_target_type='OPP')"
             " AND (charass_char_id=char_id)"
             " AND (charass_target_id=:ophead_id) ) "
             "ORDER BY char_name;" );
  q.bindValue(":ophead_id", _opheadid);
  q.exec();
  _charass->populate(q);
}

