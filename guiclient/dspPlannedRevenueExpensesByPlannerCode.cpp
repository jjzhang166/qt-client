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

#include "dspPlannedRevenueExpensesByPlannerCode.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

dspPlannedRevenueExpensesByPlannerCode::dspPlannedRevenueExpensesByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  QButtonGroup* _costGroupInt = new QButtonGroup(this);
  _costGroupInt->addButton(_useStandardCost);
  _costGroupInt->addButton(_useActualCost);

  QButtonGroup* _salesPriceGroupInt = new QButtonGroup(this);
  _salesPriceGroupInt->addButton(_useListPrice);
  _salesPriceGroupInt->addButton(_useAveragePrice);

  connect(_planord, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _plannerCode->setType(PlannerCode);

  _planord->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft, true, "plonumber");
  _planord->addColumn(tr("Type"),        _uomColumn,   Qt::AlignCenter,true, "plotype");
  _planord->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,  true, "item_number");
  _planord->addColumn(tr("Description"), -1,           Qt::AlignLeft,  true, "itemdescrip");
  _planord->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter,true, "planord_duedate");
  _planord->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight, true, "planord_qty");
  _planord->addColumn(tr("Firm"),        _ynColumn,    Qt::AlignCenter,true, "plofirm");
  _planord->addColumn(tr("Cost"),        _moneyColumn, Qt::AlignRight, true, "plocost");
  _planord->addColumn(tr("Revenue"),     _moneyColumn, Qt::AlignRight, true, "plorevenue");
  _planord->addColumn(tr("Gr. Profit"),  _moneyColumn, Qt::AlignRight, true, "profit");

  _startDate->setAllowNullDate(true);
  _startDate->setNullString(tr("Earliest"));
  _startDate->setNullDate(omfgThis->startOfTime());
  _endDate->setAllowNullDate(true);
  _endDate->setNullString(tr("Latest"));
  _endDate->setNullDate(omfgThis->endOfTime());

  _startEvalDate->setAllowNullDate(true);
  _startEvalDate->setNullString(tr("Earliest"));
  _startEvalDate->setNullDate(omfgThis->startOfTime());
  _endEvalDate->setAllowNullDate(true);
  _endEvalDate->setNullString(tr("Latest"));
  _endEvalDate->setNullDate(omfgThis->endOfTime());
}

dspPlannedRevenueExpensesByPlannerCode::~dspPlannedRevenueExpensesByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPlannedRevenueExpensesByPlannerCode::languageChange()
{
  retranslateUi(this);
}

bool dspPlannedRevenueExpensesByPlannerCode::setParams(ParameterList &params)
{
  params.append("startDate", _startDate->date());
  params.append("endDate", _endDate->date());
  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  if (_useStandardCost->isChecked())
    params.append("useStandardCost");

  if (_useActualCost->isChecked())
    params.append("useActualCost");

  if (_useListPrice->isChecked())
    params.append("useListPrice");

  if (_useAveragePrice->isChecked())
  {
    params.append("useAveragePrice");
    params.append("startEvalDate", _startEvalDate->date());
    params.append("endEvalDate", _endEvalDate->date());
  }

  return true;
}

void dspPlannedRevenueExpensesByPlannerCode::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("PlannedRevenueExpensesByPlannerCode", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPlannedRevenueExpensesByPlannerCode::sPopulateMenu(QMenu *, QTreeWidgetItem *)
{
}

void dspPlannedRevenueExpensesByPlannerCode::sFillList()
{
  _planord->clear();

  ParameterList params;
  setParams(params);

  QString sql( "SELECT *,"
               "       plorevenue - plocost AS profit,"
               "       CASE WHEN plocost > plorevenue THEN 'error'"
               "       END AS plorevenue_xtforegroundrole,"
               "       'qty' AS planord_qty_xtnumericrole,"
               "       'curr' AS plocost_xtnumericrole,"
               "       'curr' AS plorevenue_xtnumericrole,"
               "       'curr' AS profit_xtnumericrole,"
               "       0 AS plocost_xttotalrole,"
               "       0 AS plorevenue_xttotalrole,"
               "       0 AS profit_xttotalrole "
               "FROM ( SELECT planord_id, planord_itemsite_id, planord_duedate,"
               "       formatPloNumber(planord_id) AS plonumber,"
               "       CASE WHEN (planord_type='P') THEN 'P/O'"
               "            WHEN (planord_type='W') THEN 'W/O'"
               "            ELSE '?'"
               "       END AS plotype,"
               "       item_number,"
	       "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
               "       planord_qty, formatBoolYN(planord_firm) AS plofirm,"
	       "<? if exists(\"useActualCost\") ?>"
	       "       (actcost(item_id) * planord_qty)"
	       "<? elseif exists(\"useStandardCost\") ?>"
	       "       (stdcost(item_id) * planord_qty)"
	       "<? endif ?> AS plocost,"
	       "<? if exists(\"useListPrice\") ?>"
	       "       (item_listprice * planord_qty) "
	       "<? elseif exists(\"useAveragePrice\") ?>"
	       "       (CASE WHEN(averageSalesPrice(itemsite_id,"
	       "                               <? value(\"startEvalDate\") ?>,"
	       "                               <? value(\"endEvalDate\") ?>)=0)"
	       "                               THEN item_listprice"
	       "             ELSE averageSalesPrice(itemsite_id,"
	       "                               <? value(\"startEvalDate\") ?>,"
	       "                               <? value(\"endEvalDate\") ?>)"
	       "             END * planord_qty)"
	       "<? endif ?> AS plorevenue "
	       "FROM planord, itemsite, item "
	       "WHERE ((planord_itemsite_id=itemsite_id)"
	       " AND (itemsite_item_id=item_id)"
	       " AND (item_sold)"
	       " AND (planord_duedate BETWEEN <? value(\"startDate\") ?>"
	       "			  AND <? value(\"endDate\") ?>)"
	       "<? if exists(\"plancode_id\") ?>"
	       " AND (itemsite_plancode_id=<? value(\"plancode_id\") ?>)"
	       "<? elseif exists(\"plancode_pattern\") ?>"
	       " AND (itemsite_plancode_id IN (SELECT plancode_id"
	       "      FROM plancode"
	       "      WHERE (plancode_code ~ <? value(\"plancode_pattern\") ?>)))"
	       "<? endif ?>"
	       "<? if exists(\"warehous_id\") ?>"
	       " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	       "<? endif ?>"
	       ") ) AS data "
	       "ORDER BY planord_duedate, item_number;" );

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _planord->populate(q, true);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // set color if profit is negative
  XTreeWidgetItem *last = _planord->topLevelItem(_planord->topLevelItemCount() - 1);
  if (last && last->data(9, Qt::UserRole).toMap().value("raw").toDouble() < 0)
    last->setTextColor(9, "red");
}
