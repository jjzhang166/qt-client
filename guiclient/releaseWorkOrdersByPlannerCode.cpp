/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "releaseWorkOrdersByPlannerCode.h"

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

releaseWorkOrdersByPlannerCode::releaseWorkOrdersByPlannerCode(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  QButtonGroup* _dateGroupInt = new QButtonGroup(this);
  _dateGroupInt->addButton(_startDate);
  _dateGroupInt->addButton(_dueDate);

  connect(_release, SIGNAL(clicked()), this, SLOT(sRelease()));

  _plannerCode->setType(ParameterGroup::PlannerCode);
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
  XSqlQuery releaseRelease;
  /* we're going to use essentially the same query twice:
     1 - to print the paperwork
     2 - to release the work orders
   */
  QString sql( "SELECT <? if exists(\"paperwork\") ?>"
	       "       wo_id, CAST(wo_qtyord AS INTEGER) AS wo_qtyord_int "
	       "       <? else ?>"
	       "       releaseWo(wo_id, false) "
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
    releaseRelease = wom.toQuery(wop);

    if (releaseRelease.lastError().type() != QSqlError::NoError)
    {
      systemError(this, releaseRelease.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    QPrinter  printer(QPrinter::HighResolution);
    bool      setupPrinter = true;
    bool      userCanceled = false;

    while (releaseRelease.next())
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
      params.append("wo_id",   releaseRelease.value("wo_id"));
      params.append("labelTo", releaseRelease.value("wo_qtyord_int"));

      if (_pickList->isChecked())
      {
	orReport report("PickList", params);

	if (report.isValid() && report.print(&printer, setupPrinter))
	  setupPrinter = false;
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
	  setupPrinter = false;
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
	  setupPrinter = false;
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
	query.bindValue(":wo_id", releaseRelease.value("wo_id"));	// from outer loop query
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
	    setupPrinter = false;
	  else
	  {
	    report.reportError(this);
	    orReport::endMultiPrint(&printer);
	    return;
	  }
	}
	else if (query.lastError().type() != QSqlError::NoError)
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
  releaseRelease = mql.toQuery(params);
  if (releaseRelease.lastError().type() != QSqlError::NoError)
  {
    systemError(this, releaseRelease.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sWorkOrdersUpdated(-1, true);

  accept();
}
