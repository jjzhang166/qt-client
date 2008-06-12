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

#include "todoList.h"

#include "xdialog.h"
#include <QMenu>
#include <QSqlError>
#include <QVariant>
#include <metasql.h>
#include <openreports.h>

#include "todoItem.h"
#include "incident.h"
#include "storedProcErrorLookup.h"

#define ENABLEUP(Priv, I) (Priv && I->id() > 0 && \
			   I->currentItem() > 0 && \
			   I->indexOfTopLevelItem(I->currentItem()) != 0)
#define ENABLEDOWN(Priv, I) (Priv && I->id() > 0 && \
			     I->currentItem() > 0 && \
			     I->indexOfTopLevelItem(I->currentItem()) < \
				  (I->topLevelItemCount() - 1) && \
			     I->topLevelItem(I->indexOfTopLevelItem(I->currentItem()) + 1)->text(0) == "T")

todoList::todoList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  _dueDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dueDates->setEndNull(tr("Latest"),	  omfgThis->endOfTime(),   TRUE);
  _dueDates->setStartCaption(tr("First Due Date:"));
  _dueDates->setEndCaption(tr("Last Due Date:"));

  _incdtDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _incdtDates->setEndNull(tr("Latest"),	  omfgThis->endOfTime(),   TRUE);
  _incdtDates->setStartCaption(tr("First Incident Date:"));
  _incdtDates->setEndCaption(tr("Last Incident Date:"));

  _usr->setEnabled(_privileges->check("MaintainOtherTodoLists"));
  _usr->setType(User);
  q.prepare("SELECT usr_id "
	    "FROM usr "
	    "WHERE (usr_username=CURRENT_USER);");
  q.exec();
  if (q.first())
  {
    _myUsrId = q.value("usr_id").toInt();
    _usr->setId(_myUsrId);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    close();
  }

  connect(_active,	SIGNAL(toggled(bool)),	this,	SLOT(sFillList()));
  connect(_autoUpdate,	SIGNAL(toggled(bool)),	this,	SLOT(sHandleAutoUpdate(bool)));
  connect(_close,	SIGNAL(clicked()),	this,	SLOT(sClose()));
  connect(_completed,	SIGNAL(toggled(bool)),	this,	SLOT(sFillList()));
  connect(_delete,	SIGNAL(clicked()),	this,	SLOT(sDelete()));
  connect(_dueDates,	SIGNAL(updated()),	this,   SLOT(sFillList()));
  connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
  connect(_incdtDates,	SIGNAL(updated()),	this,   SLOT(sFillList()));
  connect(_incidents,	SIGNAL(toggled(bool)),	this,	SLOT(sFillList()));
  connect(_moveDown,	SIGNAL(clicked()),	this,	SLOT(sMoveDown()));
  connect(_moveUp,	SIGNAL(clicked()),	this,	SLOT(sMoveUp()));
  connect(_new,		SIGNAL(clicked()),	this,	SLOT(sNew()));
  connect(_print,	SIGNAL(clicked()),	this,	SLOT(sPrint()));
  connect(_todoList,	SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_todoList,	SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)),
	    this,	SLOT(sPopulateMenu(QMenu*)));
  connect(_todoList,	SIGNAL(itemSelectionChanged()),	this,	SLOT(handlePrivs()));
  connect(_usr,		SIGNAL(updated()),	this,	SLOT(sFillList()));
  connect(_usr,		SIGNAL(updated()),	this,	SLOT(handlePrivs()));
  connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));

  _todoList->addColumn(tr("Type"),    _statusColumn,	Qt::AlignCenter, true, "type");
  _todoList->addColumn(tr("Seq"),	 _seqColumn,	Qt::AlignRight,  true, "seq");
  _todoList->addColumn(tr("User"),	_userColumn,	Qt::AlignLeft,   true, "usr");
  _todoList->addColumn(tr("Name"),		100,	Qt::AlignLeft,   true, "name");
  _todoList->addColumn(tr("Description"),	 -1,	Qt::AlignLeft,   true, "descrip");
  _todoList->addColumn(tr("Status"),  _statusColumn,	Qt::AlignLeft,   true, "status");
  _todoList->addColumn(tr("Due Date"),	_dateColumn,	Qt::AlignLeft,   true, "due");
  _todoList->addColumn(tr("Incident"), _orderColumn,	Qt::AlignLeft,   true, "incdt");

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _active->setChecked(true);
    _incidents->setChecked(true);
  }

  handlePrivs();
  sFillList();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

void todoList::languageChange()
{
    retranslateUi(this);
}

void todoList::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if (_todoList->currentItem()->text(0) == "T")
  {
    bool editPriv =
	(_myUsrId == _todoList->altId() && _privileges->check("MaintainPersonalTodoList")) ||
	(_myUsrId != _todoList->altId() && _privileges->check("MaintainOtherTodoLists"));

    bool viewPriv =
	(_myUsrId == _todoList->altId() && _privileges->check("ViewPersonalTodoList")) ||
	(_myUsrId != _todoList->altId() && _privileges->check("ViewOtherTodoLists"));

    menuItem = pMenu->insertItem(tr("Move Up"), this, SLOT(sMoveUp()), 0);
    pMenu->setItemEnabled(menuItem, ENABLEUP(editPriv, _todoList));

    menuItem = pMenu->insertItem(tr("Move Down"), this, SLOT(sMoveDown()), 0);
    pMenu->setItemEnabled(menuItem, ENABLEDOWN(editPriv, _todoList));

    menuItem = pMenu->insertItem(tr("New..."), this, SLOT(sNew()), 0);
    pMenu->setItemEnabled(menuItem, editPriv);

    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, editPriv);

    menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
    pMenu->setItemEnabled(menuItem, viewPriv);

    menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
    pMenu->setItemEnabled(menuItem, editPriv);
  }

  if (! _todoList->currentItem()->text(7).isEmpty())
  {
    menuItem = pMenu->insertItem(tr("Edit Incident"), this, SLOT(sEditIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainIncidents"));
    menuItem = pMenu->insertItem(tr("View Incident"), this, SLOT(sViewIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewIncidents") ||
				    _privileges->check("MaintainIncidents"));
  }
}

enum SetResponse todoList::set(const ParameterList& pParams)
{
  QVariant param;
  bool	   valid;

  param = pParams.value("usr_id", &valid);
  if (valid)
  {
    _usr->setId(param.toInt());
    handlePrivs();
    sFillList();
  }

  return NoError;
}

void todoList::handlePrivs()
{
  //bool editNewPriv  = false;
  bool editTodoPriv = false;
  bool viewTodoPriv = false;

  if (! _todoList->currentItem())
  {
    _moveUp->setEnabled(false);
    _moveDown->setEnabled(false);
  }
  else if (_todoList->currentItem()->text(0) == "T")
  {
    editTodoPriv =
      (_myUsrId == _todoList->altId() && _privileges->check("MaintainPersonalTodoList")) ||
      (_privileges->check("MaintainOtherTodoLists"));

    viewTodoPriv =
      (_myUsrId == _todoList->altId() && _privileges->check("ViewPersonalTodoList")) ||
      (_privileges->check("ViewOtherTodoLists"));
    _moveUp->setEnabled(ENABLEUP(editTodoPriv, _todoList));
    _moveDown->setEnabled(ENABLEDOWN(editTodoPriv, _todoList));
  }
  else if (_todoList->currentItem()->text(0) == "I")
  {
    editTodoPriv = false;
    viewTodoPriv = false;
    _moveUp->setEnabled(false);
    _moveDown->setEnabled(false);
  }

  _usr->setEnabled(_privileges->check("MaintainOtherTodoLists") ||
		   _privileges->check("ViewOtherTodoLists"));
  _new->setEnabled(_privileges->check("MaintainOtherTodoLists") ||
                   _privileges->check("ViewOtherTodoLists"));
  _edit->setEnabled(editTodoPriv && _todoList->id() > 0);
  _view->setEnabled((editTodoPriv || viewTodoPriv) && _todoList->id() > 0);
  _delete->setEnabled(editTodoPriv && _todoList->id() > 0);

  if (editTodoPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_view, SLOT(animateClick()));
    connect(_todoList,	SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else if (viewTodoPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_edit, SLOT(animateClick()));
    connect(_todoList,	SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
}

void todoList::sClose()
{
  close();
}

void todoList::sMoveUp()
{
  q.prepare("SELECT todoItemMoveUp(:todoitem_id) AS returnVal;");
  q.bindValue(":todoitem_id", _todoList->id());
  q.exec();
  if (q.first())
  {
    int returnVal = q.value("returnVal").toInt();
    if (returnVal < 0)
    {
      systemError(this, storedProcErrorLookup("todoItemMoveUp", returnVal),
		  __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void todoList::sMoveDown()
{
  q.prepare("SELECT todoItemMoveDown(:todoitem_id) AS returnVal;");
  q.bindValue(":todoitem_id", _todoList->id());
  q.exec();
  if (q.first())
  {
    int returnVal = q.value("returnVal").toInt();
    if (returnVal < 0)
    {
      systemError(this, storedProcErrorLookup("todoItemMoveUp", returnVal),
		  __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void todoList::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  if (_usr->isSelected())
    params.append("usr_id", _usr->id());

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void todoList::sDelete()
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
      sFillList();
    }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

}

void todoList::setParams(ParameterList &params)
{
  if (_active->isChecked())
    params.append("active");
  if (_completed->isChecked())
    params.append("completed");
  if (_incidents->isChecked())
  {
    params.append("incidents");
    params.append("incdtStartDate", _incdtDates->startDate());
    params.append("incdtEndDate",   _incdtDates->endDate());
  }
  _usr->appendValue(params);
  _dueDates->appendValue(params);
}

void todoList::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("TodoList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void todoList::sFillList()
{
  QString sql = "SELECT todoitem_id AS id, todoitem_usr_id AS altId, "
		"       'T' AS type, todoitem_seq AS seq, "
		"       todoitem_name AS name, "
		"       firstLine(todoitem_description) AS descrip, "
		"       todoitem_status AS status, todoitem_due_date AS due, "
		"       usr_username AS usr, incdt_number AS incdt,"
                "       CASE WHEN (todoitem_status != 'C'AND "
                "                  todoitem_due_date < CURRENT_DATE) THEN 'expired'"
                "            WHEN (todoitem_status != 'C'AND "
                "                  todoitem_due_date > CURRENT_DATE) THEN 'future'"
                "       END AS due_qtforegroundrole "
		"FROM usr, todoitem LEFT OUTER JOIN "
		"     incdt ON (incdt_id=todoitem_incdt_id) "
		"WHERE ( (todoitem_usr_id=usr_id)"
		"  AND   (todoitem_due_date BETWEEN <? value(\"startDate\") ?> "
		"                               AND <? value(\"endDate\") ?>) "
		"  <? if not exists(\"completed\") ?>"
		"  AND   (todoitem_status != 'C')"
		"  <? endif ?>"
		"  <? if exists(\"usr_id\") ?> "
		"  AND (todoitem_usr_id=<? value(\"usr_id\") ?>) "
		"  <? elseif exists(\"usr_pattern\" ?>"
		"  AND (todoitem_usr_id IN (SELECT usr_id "
		"        FROM usr "
		"        WHERE (usr_username ~ <? value(\"usr_pattern\") ?>))) "
		"  <? endif ?>"
		"  <? if exists(\"active\") ?>AND (todoitem_active) <? endif ?>"
		"       ) "
		"<? if exists(\"incidents\")?>"
		"UNION "
		"SELECT incdt_id AS id, usr_id AS altId, "
		"       'I' AS type, NULL AS seq, "
		"       incdt_summary AS name, "
		"       firstLine(incdt_descrip) AS descrip, "
		"       incdt_status AS status,  NULL AS due, "
		"       incdt_assigned_username AS usr, incdt_number AS incdt,"
                "       NULL AS due_qtforegroundrole "
		"FROM incdt LEFT OUTER JOIN usr ON (usr_username=incdt_assigned_username)"
		"WHERE ((incdt_timestamp BETWEEN <? value(\"incdtStartDate\") ?>"
		"                            AND <? value(\"incdtEndDate\") ?>)"
		"  <? if not exists(\"completed\") ?> "
		"   AND (incdt_status != 'L')"
		"  <? endif ?>"
		"  <? if exists(\"usr_id\") ?> "
		"  AND (usr_id=<? value(\"usr_id\") ?>) "
		"  <? elseif exists(\"usr_pattern\" ?>"
		"  AND (usr_id IN (SELECT usr_id "
		"        FROM usr "
		"        WHERE (usr_username ~ <? value(\"usr_pattern\") ?>))) "
		"  <? endif ?>"
		"       ) "
		"<? endif ?>"
		"ORDER BY due, seq, usr;";

  ParameterList params;
  setParams(params);

  MetaSQLQuery mql(sql);
  XSqlQuery itemQ = mql.toQuery(params);

  _todoList->populate(itemQ, true);

  if (itemQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  handlePrivs();
}

void todoList::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

int todoList::getIncidentId()
{
  int returnVal = -1;

  if (_todoList->currentItem()->text(0) == "I")
    returnVal = _todoList->id();
  else if (! _todoList->currentItem()->text(7).isEmpty())
  {
    XSqlQuery incdt;
    incdt.prepare("SELECT incdt_id FROM incdt WHERE (incdt_number=:number);");
    incdt.bindValue(":number", _todoList->currentItem()->text(7).toInt());
    if (incdt.exec() && incdt.first())
     returnVal = incdt.value("incdt_id").toInt();
    else if (incdt.lastError().type() != QSqlError::None)
      systemError(this, incdt.lastError().databaseText(), __FILE__, __LINE__);
  }

  return returnVal;
}

void todoList::sEditIncident()
{

  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", getIncidentId());

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", getIncidentId());

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}
