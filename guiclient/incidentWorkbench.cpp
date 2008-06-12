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

#include "incidentWorkbench.h"

#include <QSqlError>

#include <openreports.h>
#include <metasql.h>

#include "guiclient.h"
#include "incident.h"

incidentWorkbench::incidentWorkbench(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_autoUpdate,	SIGNAL(toggled(bool)),	this,	SLOT(sHandleAutoUpdate(bool)));
  connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
  connect(_new,		SIGNAL(clicked()),	this,	SLOT(sNew()));
  connect(_print,	SIGNAL(clicked()),	this,	SLOT(sPrint()));
  connect(_query,	SIGNAL(clicked()),	this,	SLOT(sFillList()));
  connect(_reset,	SIGNAL(clicked()),	this,	SLOT(sReset()));
  connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));

  _incdt->addColumn(tr("Number"),      _itemColumn, Qt::AlignLeft );
  _incdt->addColumn(tr("Account"),     _itemColumn, Qt::AlignLeft );
  _incdt->addColumn(tr("Status"),      _itemColumn, Qt::AlignLeft );
  _incdt->addColumn(tr("Assigned To"), _userColumn, Qt::AlignLeft ); 
  _incdt->addColumn(tr("Summary"),     -1,          Qt::AlignLeft );

  _createdDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _createdDates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _createdDates->setStartCaption(tr("First Creation Date:"));
  _createdDates->setEndCaption(tr("Last Creation Date:"));

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _statusFeedback->setChecked(true);
    _statusConfirmed->setChecked(true);
    _statusNew->setChecked(true);
    _statusAssigned->setChecked(true);
  }

  _assignedTo->setType(User);
}

incidentWorkbench::~incidentWorkbench()
{
    // no need to delete child widgets, Qt does it all for us
}

void incidentWorkbench::languageChange()
{
    retranslateUi(this);
}

void incidentWorkbench::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  incident newdlg(this, 0, true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void incidentWorkbench::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", _incdt->id());

  incident newdlg(this, 0, true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void incidentWorkbench::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", _incdt->id());

  incident newdlg(this, 0, true);
  newdlg.set(params);
  newdlg.exec();
}

void incidentWorkbench::sReset()
{
  _statusNew->setChecked(true);
  _statusFeedback->setChecked(true);
  _statusConfirmed->setChecked(true);
  _statusAssigned->setChecked(true);
  _statusResolved->setChecked(false);
  _statusClosed->setChecked(false);
  _createdDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _createdDates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _textPattern->clear();
}

void incidentWorkbench::setParams(ParameterList & params)
{
  params.append("new",		tr("New"));
  params.append("feedback",	tr("Feedback"));
  params.append("confirmed",	tr("Confirmed"));
  params.append("assigned",	tr("Assigned"));
  params.append("resolved",	tr("Resolved"));
  params.append("closed",	tr("Closed"));

  _assignedTo->appendValue(params);

  if(_statusNew->isChecked())
    params.append("isnew");
  if(_statusFeedback->isChecked())
    params.append("isfeedback");
  if(_statusConfirmed->isChecked())
    params.append("isconfirmed");
  if(_statusAssigned->isChecked())
    params.append("isassigned");
  if(_statusResolved->isChecked())
    params.append("isresolved");
  if(_statusClosed->isChecked())
    params.append("isclosed");

  if(!_textPattern->text().trimmed().isEmpty())
    params.append("pattern", _textPattern->text().trimmed());

  _createdDates->appendValue(params);
}

void incidentWorkbench::sFillList()
{
  QString sql="SELECT incdt_id,"
              "       incdt_number,"
	      "       crmacct_name,"
              "       CASE WHEN(incdt_status='N') THEN <? value(\"new\") ?>"
              "            WHEN(incdt_status='F') THEN <? value(\"feedback\") ?>"
              "            WHEN(incdt_status='C') THEN <? value(\"confirmed\") ?>"
              "            WHEN(incdt_status='A') THEN <? value(\"assigned\") ?>"
              "            WHEN(incdt_status='R') THEN <? value(\"resolved\") ?>"
              "            WHEN(incdt_status='L') THEN <? value(\"closed\") ?>"
              "            ELSE incdt_status"
              "       END,"
              "       incdt_assigned_username,"
              "       incdt_summary "
              "  FROM incdt, crmacct"
              " WHERE ((incdt_crmacct_id=crmacct_id)"
	      " <? if exists(\"usr_id\") ?> "
	      " AND (incdt_assigned_username IN (SELECT usr_username FROM usr"
	      "                     WHERE usr_id = <? value(\"usr_id\") ?>)) "
	      " <? elseif exists(\"usr_pattern\") ?> "
	      " AND (incdt_assigned_username ~* <? value(\"usr_pattern\") ?>) "
	      " <? endif ?>"
	      " AND (incdt_status IN ('' "
	      "   <? if exists(\"isnew\") ?>,		'N' <? endif ?> "
	      "   <? if exists(\"isfeedback\") ?>,	'F' <? endif ?> "
	      "   <? if exists(\"isconfirmed\") ?>,	'C' <? endif ?> "
	      "   <? if exists(\"isassigned\") ?>,	'A' <? endif ?> "
	      "   <? if exists(\"isresolved\") ?>,	'R' <? endif ?> "
	      "   <? if exists(\"isclosed\") ?>,	'L' <? endif ?> "
	      " ))"
	      " <? if exists(\"pattern\") ?> "
	      " AND ((incdt_summary ~* <? value(\"pattern\") ?>)"
	      "  OR  (incdt_descrip ~* <? value(\"pattern\") ?>)"
	      "  OR  (incdt_id IN (SELECT comment_source_id"
	      "             FROM comment"
	      "            WHERE((comment_source='INCDT')"
              "              AND (comment_text ~* <? value(\"pattern\") ?>)))))"
	      " <? endif ?>"
	      " AND (incdt_timestamp BETWEEN <? value(\"startDate\") ?> "
	      "				 AND <? value(\"endDate\") ?>) "
	      ") ORDER BY incdt_number; ";

  ParameterList params;
  setParams(params);

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);

  _incdt->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void incidentWorkbench::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("IncidentWorkbenchList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void incidentWorkbench::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

