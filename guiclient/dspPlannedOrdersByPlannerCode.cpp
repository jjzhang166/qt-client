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

#include "dspPlannedOrdersByPlannerCode.h"

#include <QMenu>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "deletePlannedOrder.h"
#include "dspRunningAvailability.h"
#include "dspUsageStatisticsByItem.h"
#include "firmPlannedOrder.h"
#include "purchaseRequest.h"
#include "workOrder.h"

dspPlannedOrdersByPlannerCode::dspPlannedOrdersByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_planord, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _plannerCode->setType(PlannerCode);

  _planord->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft,  true, "ordernum");
  _planord->addColumn(tr("Type"),        _uomColumn,   Qt::AlignCenter,true, "ordtype");
  _planord->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter,true, "warehous_code");
  _planord->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,  true, "item_number");
  _planord->addColumn(tr("Description"), -1,           Qt::AlignLeft,  true, "item_descrip");
  _planord->addColumn(tr("Start Date"),  _dateColumn,  Qt::AlignCenter,true, "planord_startdate");
  _planord->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter,true, "planord_duedate");
  _planord->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight, true, "planord_qty");
  _planord->addColumn(tr("Firm"),        _ynColumn,    Qt::AlignCenter );

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

dspPlannedOrdersByPlannerCode::~dspPlannedOrdersByPlannerCode()
{
    // no need to delete child widgets, Qt does it all for us
}

void dspPlannedOrdersByPlannerCode::languageChange()
{
    retranslateUi(this);
}

bool dspPlannedOrdersByPlannerCode::setParams(ParameterList &pParams)
{
  _warehouse->appendValue(pParams);
  _plannerCode->appendValue(pParams);

  return true;
}

void dspPlannedOrdersByPlannerCode::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("PlannedOrdersByPlannerCode", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);
}

void dspPlannedOrdersByPlannerCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()), 0);
  pMenu->setItemEnabled(menuItem, _privileges->check("ViewInventoryAvailability"));
  menuItem = pMenu->insertItem(tr("Usage Statistics..."), this, SLOT(sDspUsageStatistics()), 0);
  pMenu->setItemEnabled(menuItem, _privileges->check("ViewInventoryHistory"));

  pMenu->insertSeparator();

  if (pSelected->text(8) == "No")
  {
    menuItem = pMenu->insertItem(tr("Firm Order..."), this, SLOT(sFirmOrder()), 0);
    if (!_privileges->check("FirmPlannedOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else
  {
    menuItem = pMenu->insertItem(tr("Soften Order..."), this, SLOT(sSoftenOrder()), 0);
    if (!_privileges->check("SoftenPlannedOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("Release Order..."), this, SLOT(sReleaseOrder()), 0);
  if ( (!_privileges->check("ReleasePlannedOrders")) ||
       ((pSelected->text(1) == "W/O") && (!_privileges->check("MaintainWorkOrders")) ) ||
       ((pSelected->text(1) == "P/O") && (!_privileges->check("MaintainPurchaseRequests")) ) )
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete Order..."), this, SLOT(sDeleteOrder()), 0);
  if (!_privileges->check("DeletePlannedOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspPlannedOrdersByPlannerCode::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _planord->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPlannedOrdersByPlannerCode::sFirmOrder()
{
  ParameterList params;
  params.append("planord_id", _planord->id());

  firmPlannedOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPlannedOrdersByPlannerCode::sSoftenOrder()
{
  q.prepare( "UPDATE planord "
             "SET planord_firm=FALSE "
             "WHERE (planord_id=:planord_id);" );
  q.bindValue(":planord_id", _planord->id());
  q.exec();

  sFillList();
}

void dspPlannedOrdersByPlannerCode::sReleaseOrder()
{
  if (_planord->currentItem()->text(1) == "W/O")
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _planord->id());

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (_planord->currentItem()->text(1) == "P/O")
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _planord->id());

    purchaseRequest newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void dspPlannedOrdersByPlannerCode::sDeleteOrder()
{
  ParameterList params;
  params.append("planord_id", _planord->id());

  deletePlannedOrder newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPlannedOrdersByPlannerCode::sFillList()
{
  QString sql( "SELECT planord_id, planord_itemsite_id,"
               "       formatPloNumber(planord_id) AS ordernum,"
               "       CASE WHEN (planord_type='P') THEN 'P/O'"
               "            WHEN (planord_type='W') THEN 'W/O'"
               "            ELSE '?'"
               "       END AS ordtype,"
               "       warehous_code, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
               "       planord_startdate,"
               "       planord_duedate,"
               "       planord_qty, formatBoolYN(planord_firm),"
               "       'qty' AS planord_qty "
               "FROM planord, itemsite, warehous, item "
               "WHERE ( (planord_itemsite_id=itemsite_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (itemsite_item_id=item_id)"
               "<? if exists(\"plancode_id\") ?>"
               " AND (itemsite_plancode_id=<? value(\"plancode_id\") ?>)"
               "<? elseif exists(\"plancode_pattern\") ?>"
               " AND (itemsite_plancode_id IN (SELECT plancode_id"
               "    FROM plancode"
               "    WHERE (plancode_code ~ <? value(\"plancode_pattern\") ?>)))"
               "<? endif ?>"
               "<? if exists(\"warehous_id\") ?>"
               " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
               "<? endif ?>"
               " ) "
               "ORDER BY planord_duedate, item_number;");
 
  MetaSQLQuery mql(sql);
  ParameterList params;
  setParams(params);
  q = mql.toQuery(params);
  _planord->populate(q, TRUE);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspPlannedOrdersByPlannerCode::sDspUsageStatistics()
{
  q.prepare("SELECT itemsite_item_id "
	    "FROM itemsite "
	    "WHERE (itemsite_id=:itemsite_id);");
  q.bindValue(":itemsite_id", _planord->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("item_id", q.value("itemsite_item_id"));
    if (_warehouse->isSelected())
      params.append("warehous_id", _warehouse->id());
    params.append("run");

    dspUsageStatisticsByItem *newdlg = new dspUsageStatisticsByItem();
    SetResponse setresp = newdlg->set(params);
    if (setresp == NoError || setresp == NoError_Run)
      omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
