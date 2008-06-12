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

#include "releaseWorkOrdersByPlannerCode.h"

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

releaseWorkOrdersByPlannerCode::releaseWorkOrdersByPlannerCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  QButtonGroup* _dateGroupInt = new QButtonGroup(this);
  _dateGroupInt->addButton(_startDate);
  _dateGroupInt->addButton(_dueDate);

  connect(_release, SIGNAL(clicked()), this, SLOT(sRelease()));

  _plannerCode->setType(PlannerCode);
}

releaseWorkOrdersByPlannerCode::~releaseWorkOrdersByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void releaseWorkOrdersByPlannerCode::languageChange()
{
  retranslateUi(this);
}

bool releaseWorkOrdersByPlannerCode::setParams(ParameterList &pParams)
{
  if (!_cutoffDate->isValid())
  {
    QMessageBox::critical( this, tr("Enter Cutoff Date"),
                           tr("Please enter a valid Cutoff Date.") );
    _cutoffDate->setFocus();
    return false;
  }

  pParams.append("cutOffDate", _cutoffDate->date());
  if (_startDate->isChecked())
    pParams.append("byStartDate");
  else if (_dueDate->isChecked())
    pParams.append("byDueDate");

  _warehouse->appendValue(pParams);
  _plannerCode->appendValue(pParams);

  return true;
}

void releaseWorkOrdersByPlannerCode::sRelease()
{
  /* we're going to use essentially the same query twice:
     1 - to print the paperwork
     2 - to release the work orders
   */
  QString sql( "SELECT <? if exists(\"paperwork\") ?>"
	       "       wo_id, CAST(wo_qtyord AS INTEGER) AS wo_qtyord_int "
	       "       <? else ?>"
	       "       releaseWo(wo_id, FALSE) "
	       "       <? endif ?>"
	       "FROM wo, itemsite, plancode "
	       "WHERE ((wo_itemsite_id=itemsite_id)"
	       "  AND  (itemsite_plancode_id=plancode_id)"
	       "  AND  (wo_status='E')"
	       "<? if exists (\"byStartDate\") ?>"
	       "  AND  (wo_startdate<=<? value(\"cutOffDate\") ?>)"
	       "<? elseif exists (\"byDueDate\") ?>"
	       "  AND  (wo_duedate<=<? value(\"cutOffDate\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"warehous_id\") ?>"
	       "  AND  (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"plancode_id\") ?>"
	       "  AND  (itemsite_plancode_id=<? value(\"plancode_id\") ?>)"
	       "<? elseif exists(\"plancode_pattern\") ?>"
	       "  AND  (itemsite_plancode_id IN ("
	       "    SELECT plancode_id"
	       "    FROM plancode"
	       "    WHERE (plancode_code ~ <? value(\"plancode_pattern\") ?>)))"
	       "<? endif ?>"
	       ");"
	       );
  if (_pickList->isChecked() || _routing->isChecked() ||
      _woLabel->isChecked()  || _packingList->isChecked())
  {
    ParameterList wop;
    if (! setParams(wop))
      return;
    wop.append("paperwork");

    MetaSQLQuery wom(sql);
    q = wom.toQuery(wop);

    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    QPrinter  printer(QPrinter::HighResolution);
    bool      setupPrinter = TRUE;
    bool      userCanceled = false;

    while (q.next())
    {
      if (setupPrinter &&
	  orReport::beginMultiPrint(&printer, userCanceled) == false)
      {
	if(!userCanceled)
	  systemError(this, tr("<p>Could not initialize printing system for "
			       "multiple reports."));
	return;
      }

      ParameterList params;
      params.append("wo_id",   q.value("wo_id"));
      params.append("labelTo", q.value("wo_qtyord_int"));

      if (_pickList->isChecked())
      {
	orReport report("PickList", params);

	if (report.isValid() && report.print(&printer, setupPrinter))
	  setupPrinter = FALSE;
	else
	{
	  report.reportError(this);
	  orReport::endMultiPrint(&printer);
	  return;
	}
      }

      if (_routing->isChecked())
      {
	orReport report("Routing", params);

	if (report.isValid() && report.print(&printer, setupPrinter))
	  setupPrinter = FALSE;
	else
	{
	  report.reportError(this);
	  orReport::endMultiPrint(&printer);
	  return;
	}
      }

      if (_woLabel->isChecked())
      {
	orReport report("WOLabel", params);
	if (report.isValid() && report.print(&printer, setupPrinter))
	  setupPrinter = FALSE;
	else
	{
	  report.reportError(this);
	  orReport::endMultiPrint(&printer);
	  return;
	}
      }

      if (_packingList->isChecked())
      {
	XSqlQuery query;
	query.prepare( "SELECT cohead_id, findCustomerForm(cohead_cust_id, 'L') AS reportname "
		       "FROM cohead, coitem, wo "
		       "WHERE ( (coitem_cohead_id=cohead_id)"
		       " AND (wo_ordid=coitem_id)"
		       " AND (wo_ordtype='S')"
		       " AND (wo_id=:wo_id) );" );
	query.bindValue(":wo_id", q.value("wo_id"));	// from outer loop query
	query.exec();
	if (query.first())
	{
	  ParameterList params;
	  params.append("sohead_id", query.value("cohead_id"));
	  params.append("head_id",  query.value("cohead_id"));
	  params.append("head_type",  "SO");
	  if (_metrics->boolean("MultiWhs"))
	    params.append("MultiWhs");

	  orReport report(query.value("reportname").toString(), params);
	  if (report.isValid() && report.print(&printer, setupPrinter))
	    setupPrinter = FALSE;
	  else
	  {
	    report.reportError(this);
	    orReport::endMultiPrint(&printer);
	    return;
	  }
	}
	else if (query.lastError().type() != QSqlError::None)
	{
	  systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
	  orReport::endMultiPrint(&printer);
	  return;
	}
      }
    }

    if (! setupPrinter)	// we tried to print something
      orReport::endMultiPrint(&printer);

    if (QMessageBox::question(this, tr("Print Correctly?"),
			      tr("<p>Did the documents all print correctly?"),
			      QMessageBox::Yes,
			      QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;
  }

  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sWorkOrdersUpdated(-1, TRUE);

  accept();
}
