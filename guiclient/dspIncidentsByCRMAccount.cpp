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

#include "dspIncidentsByCRMAccount.h"

#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "crmaccount.h"
#include "incident.h"
#include "todoItem.h"

dspIncidentsByCRMAccount::dspIncidentsByCRMAccount(QWidget* parent, const char* name, Qt::WFlags fl)
  : XMainWindow(parent, name, fl)
{
  setupUi(this);

  QButtonGroup* _crmacctGroupInt = new QButtonGroup(this);
  _crmacctGroupInt->addButton(_allAccts);
  _crmacctGroupInt->addButton(_selectedAcct);

  statusBar()->hide();

  _createdDate->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _createdDate->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  connect(_list,  SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),
				     this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _list->setRootIsDecorated(true);

  _list->addColumn(tr("Account Number"),	    80, Qt::AlignLeft  );
  _list->addColumn(tr("Account Name"),		   100, Qt::AlignLeft  );
  _list->addColumn(tr("Incident"),	  _orderColumn, Qt::AlignRight );
  _list->addColumn(tr(""),			    20, Qt::AlignCenter);
  _list->addColumn(tr("Summary"),	            -1, Qt::AlignLeft  );
  _list->addColumn(tr("Entered/Assigned"), _dateColumn, Qt::AlignLeft  );
  _list->addColumn(tr("Status"),         _statusColumn, Qt::AlignCenter);
  _list->addColumn(tr("Assigned To"),	   _userColumn, Qt::AlignLeft  );
  _list->addColumn(tr("To-Do Due"),	   _dateColumn, Qt::AlignLeft  );

  _list->setIndentation(10);
}

dspIncidentsByCRMAccount::~dspIncidentsByCRMAccount()
{
}

void dspIncidentsByCRMAccount::languageChange()
{
    retranslateUi(this);
}

enum SetResponse dspIncidentsByCRMAccount::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _selectedAcct->setChecked(true);
    _crmacct->setId(param.toInt());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}


void dspIncidentsByCRMAccount::sPrint()
{
  if (_selectedAcct->isChecked() && _crmacct->id() <= 0)
  {
    QMessageBox::critical(this, tr("No CRM Account"),
			  tr("Please select a CRM Account before Printing."),
			  QMessageBox::Ok, QMessageBox::NoButton);
    _crmacct->setFocus();
    return;
  }

  ParameterList params;
  setParams(params);

  orReport report("IncidentsByCRMAccount", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspIncidentsByCRMAccount::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if (_list->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Edit CRM Account..."), this, SLOT(sEditCRMAccount()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCRMAccounts"));
    menuItem = pMenu->insertItem(tr("View CRM Account..."), this, SLOT(sViewCRMAccount()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewCRMAccounts") ||
				    _privileges->check("MaintainCRMAccounts"));
  }
  else if (_list->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Edit Incident..."), this, SLOT(sEditIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainIncidents"));
    menuItem = pMenu->insertItem(tr("View Incident..."), this, SLOT(sViewIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewIncidents") ||
				    _privileges->check("MaintainIncidents"));
  }
  else if (_list->altId() == 3)
  {
    menuItem = pMenu->insertItem(tr("Edit To-Do Item..."), this, SLOT(sEditTodoItem()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainOtherTodoLists"));

    menuItem = pMenu->insertItem(tr("View To-Do Item..."), this, SLOT(sViewTodoItem()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewOtherTodoLists"));
  }
}

void dspIncidentsByCRMAccount::sEditCRMAccount()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("crmacct_id", _list->id());

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
}

void dspIncidentsByCRMAccount::sEditIncident()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", _list->id());

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspIncidentsByCRMAccount::sEditTodoItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", _list->id());

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspIncidentsByCRMAccount::sViewCRMAccount()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("crmacct_id", _list->id());

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
}

void dspIncidentsByCRMAccount::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", _list->id());

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void dspIncidentsByCRMAccount::sViewTodoItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", _list->id());

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

bool dspIncidentsByCRMAccount::setParams(ParameterList &params)
{
  params.append("new",		tr("New"));
  params.append("feedback",	tr("Feedback"));
  params.append("confirmed",	tr("Confirmed"));
  params.append("assigned",	tr("Assigned"));
  params.append("resolved",	tr("Resolved"));
  params.append("closed",	tr("Closed"));

  if (_selectedAcct->isChecked() && _crmacct->id() <= 0)
  {
    QMessageBox::critical(this, tr("No CRM Account"),
			  tr("Please select a CRM Account before Querying or Printing."),
			  QMessageBox::Ok, QMessageBox::NoButton);
    _crmacct->setFocus();
    return false;
  }

  if (! _allAccts->isChecked())
    params.append("crmacct_id", _crmacct->id());

  if (_showClosed->isChecked())
    params.append("showClosed");

  if (_showAcctsWOIncdts->isChecked())
    params.append("showAcctsWOIncdts");

  _createdDate->appendValue(params);

  return true;
}

void dspIncidentsByCRMAccount::sFillList()
{
  QString sql = "SELECT  crmacct_id, incdt_id, todoitem_id, "
		"	  crmacct_number, crmacct_name, "
		"	  incdt_number, DATE(incdt_timestamp) AS incdt_timestamp, "
		"         CASE WHEN(incdt_status='N') THEN <? value(\"new\") ?>"
		"              WHEN(incdt_status='F') THEN <? value(\"feedback\") ?>"
		"              WHEN(incdt_status='C') THEN <? value(\"confirmed\") ?>"
		"              WHEN(incdt_status='A') THEN <? value(\"assigned\") ?>"
		"              WHEN(incdt_status='R') THEN <? value(\"resolved\") ?>"
		"              WHEN(incdt_status='L') THEN <? value(\"closed\") ?>"
		"              ELSE incdt_status"
		"         END AS incdt_status,"
		"         incdt_assigned_username, incdt_summary, "
		"	  COALESCE(TEXT(todoitem_seq), '') AS todoitem_seq, todoitem_due_date, todoitem_name, "
		"	  COALESCE(usr_username, '') AS todoitem_usrname, "
		"	  todoitem_assigned_date, todoitem_status, "
		"	  incdtseverity_name, "
		"	  incdtpriority_name "
		"  FROM crmacct "
		"      <? if exists(\"showAcctsWOIncdts\") ?> "
		"	LEFT OUTER "
		"      <? endif ?> "
		"      JOIN incdt ON (incdt_crmacct_id=crmacct_id "
		"                     AND (incdt_timestamp BETWEEN <? value(\"startDate\") ?> "
		"                                              AND <? value(\"endDate\") ?>) "
		"                     <? if not exists(\"showClosed\") ?> "
		"                       AND incdt_status != 'L' "
		"                     <? endif ?>) "
		"      LEFT OUTER JOIN todoitem ON (todoitem_incdt_id=incdt_id) "
		"      LEFT OUTER JOIN usr ON (usr_id = todoitem_usr_id) "
		"      LEFT OUTER JOIN incdtseverity ON (incdt_incdtseverity_id = incdtseverity_id) "
		"      LEFT OUTER JOIN incdtpriority ON (incdt_incdtpriority_id = incdtpriority_id) "
		"  WHERE ((todoitem_status IS NULL OR todoitem_status != 'C') "
		"    <? if exists(\"crmacct_id\") ?> "
		"      AND (crmacct_id=<? value(\"crmacct_id\") ?>) "
		"    <? endif ?> "
		"    <? if exists(\"showAcctsWOIncdts\") ?> "
		"      AND (incdt_id IS NULL OR (true "
		"    <? endif ?> "
		"    <? if exists(\"showAcctsWOIncdts\") ?> "
		"      )) "
		"    <? endif ?> "
		"    )  "
		"  ORDER BY crmacct_name, incdt_timestamp, todoitem_due_date; "
		;
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__,__LINE__);
    return;
  }

  _list->clear();
  XTreeWidgetItem *lastCrmacct	= NULL;
  XTreeWidgetItem *lastIncdt	= NULL;
  XTreeWidgetItem *lastTodo	= NULL;
  int lastCrmacctId	= -1;
  int lastIncdtId	= -1;
  int lastTodoId	= -1;
  while (q.next())
  {
    if (q.value("crmacct_id").toInt() != lastCrmacctId)
      lastCrmacct = new XTreeWidgetItem(_list, lastCrmacct,
			       q.value("crmacct_id").toInt(), 1,
			       q.value("crmacct_number"),
			       q.value("crmacct_name"),
			       "", "", "", "", "", "", "" );
    if (!q.value("incdt_id").isNull() &&
         q.value("incdt_id").toInt() != lastIncdtId)
      lastIncdt = new XTreeWidgetItem(lastCrmacct, lastIncdt,
			       q.value("incdt_id").toInt(), 2,
			       "", "",
			       q.value("incdt_number"),
			       "", 
			       q.value("incdt_summary"),
			       q.value("incdt_timestamp"),
			       q.value("incdt_status"),
			       q.value("incdt_assigned_username"),
			       "" );
    if (!q.value("todoitem_id").isNull() &&
         q.value("todoitem_id").toInt() != lastTodoId)
      lastTodo = new XTreeWidgetItem(lastIncdt, lastTodo,
			       q.value("todoitem_id").toInt(), 3,
			       "", "",
			       "",
			       tr(">"),
			       q.value("todoitem_name"),
			       q.value("todoitem_assigned_date"),
			       q.value("todoitem_status"),
			       q.value("todoitem_usrname"),
			       q.value("todoitem_due_date") );
  lastCrmacctId = q.value("crmacct_id").toInt();
  lastIncdtId = q.value("incdt_id").toInt();
  lastTodoId = q.value("todoitem_id").toInt();
  }
}
