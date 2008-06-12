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

#include "dspInventoryBufferStatusByParameterList.h"

#include <QMenu>
#include <QVariant>
#include <QMessageBox>
#include <QStringList>
#include <openreports.h>
#include "dspInventoryHistoryByItem.h"
#include "dspAllocations.h"
#include "dspOrders.h"
#include "dspRunningAvailability.h"
#include "workOrder.h"
#include "postMiscProduction.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "createCountTagsByItem.h"
#include "enterMiscCount.h"

/*
 *  Constructs a dspInventoryBufferStatusByParameterList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspInventoryBufferStatusByParameterList::dspInventoryBufferStatusByParameterList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_availability, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _availability->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft   );
  _availability->addColumn(tr("Description"),  -1,          Qt::AlignLeft   );
  _availability->addColumn(tr("Whs."),         _whsColumn,  Qt::AlignCenter );
  _availability->addColumn(tr("LT"),           _whsColumn,  Qt::AlignCenter );
  _availability->addColumn(tr("Type"),         _qtyColumn,  Qt::AlignCenter );
  _availability->addColumn(tr("Status"),       _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Allocated"),    _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Unallocated"),  _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("On Order"),     _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Reorder Lvl."), _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("OUT Level."),   _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Available"),    _qtyColumn,  Qt::AlignRight  );

}

/*
 *  Destroys the object and frees any allocated resources
 */
dspInventoryBufferStatusByParameterList::~dspInventoryBufferStatusByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspInventoryBufferStatusByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInventoryBufferStatusByParameterList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    _parameter->setType(ClassCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ClassCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("classcode", &valid);
  if (valid)
    _parameter->setType(ClassCode);

  param = pParams.value("plancode_id", &valid);
  if (valid)
  {
    _parameter->setType(PlannerCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("plancode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(PlannerCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("plancode", &valid);
  if (valid)
    _parameter->setType(PlannerCode);

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(ItemGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("itemgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ItemGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("itemgrp", &valid);
  if (valid)
    _parameter->setType(ItemGroup);

  switch (_parameter->type())
  {
    case ClassCode:
      setCaption(tr("Inventory Buffer Status by Class Code"));
      break;

    case PlannerCode:
      setCaption(tr("Inventory Buffer Status by Planner Code"));
      break;

    case ItemGroup:
      setCaption(tr("Inventory Buffer Status by Item Group"));
      break;

    default:
      break;
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspInventoryBufferStatusByParameterList::sPrint()
{
  ParameterList params;
  _parameter->appendValue(params);
  _warehouse->appendValue(params);

  if (_parameter->isAll())
  {
    if (_parameter->type() == ItemGroup)
      params.append("itemgrp");
    else if(_parameter->type() == PlannerCode)
      params.append("plancode");
    else if (_parameter->type() == ClassCode)
      params.append("classcode");
  }

  if (_GreaterThanZero->isChecked())
    params.append("GreaterThanZero");
  else if (_EmergencyZone->isChecked())
    params.append("EmergencyZone");
  else if (_All->isChecked())
    params.append("All");

  orReport report("InventoryBufferStatusByParameterList", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);

}

void dspInventoryBufferStatusByParameterList::sPopulateMenu(QMenu *menu, QTreeWidgetItem *selected)
{
  int menuItem;

  menuItem = menu->insertItem(tr("View Inventory History..."), this, SLOT(sViewHistory()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    menu->setItemEnabled(menuItem, FALSE);

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
  if (selected->text(5).remove(',').toDouble() == 0.0)
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
  if (selected->text(7).remove(',').toDouble() == 0.0)
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Running Availability..."), this, SLOT(sRunningAvailability()), 0);

  menu->insertSeparator();

  if (((XTreeWidgetItem *)selected)->altId() == 1)
  {
    menuItem = menu->insertItem(tr("Create P/R..."), this, SLOT(sCreatePR()), 0);
    if (!_privileges->check("MaintainPurchaseRequests"))
      menu->setItemEnabled(menuItem, FALSE);

    menuItem = menu->insertItem(tr("Create P/O..."), this, SLOT(sCreatePO()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      menu->setItemEnabled(menuItem, FALSE);

    menu->insertSeparator();
  }
  else if (((XTreeWidgetItem *)selected)->altId() == 2)
  {
    menuItem = menu->insertItem(tr("Create W/O..."), this, SLOT(sCreateWO()), 0);
    if (!_privileges->check("MaintainWorkOrders"))
      menu->setItemEnabled(menuItem, FALSE);

    menuItem = menu->insertItem(tr("Post Misc. Production..."), this, SLOT(sPostMiscProduction()), 0);
    if (!_privileges->check("PostMiscProduction"))
      menu->setItemEnabled(menuItem, FALSE);

    menu->insertSeparator();
  }
    
  menu->insertItem(tr("View Substitute Availability..."), this, SLOT(sViewSubstituteAvailability()), 0);

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
  if (!_privileges->check("IssueCountTags"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Enter Misc. Inventory Count..."), this, SLOT(sEnterMiscCount()), 0);
  if (!_privileges->check("EnterMiscCounts"))
    menu->setItemEnabled(menuItem, FALSE);
}

void dspInventoryBufferStatusByParameterList::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("byLeadTime");
  params.append("run");

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("byLeadTime");
  params.append("run");


  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sPostMiscProduction()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());

  postMiscProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryBufferStatusByParameterList::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryBufferStatusByParameterList::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sViewSubstituteAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");
  params.append("byLeadTime", TRUE);

  dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryBufferStatusByParameterList::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  
  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryBufferStatusByParameterList::sFillList()
{
  _availability->clear();

  QString sql( "SELECT itemsite_id, itemtype,"
               "       item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
               "       warehous_id, warehous_code, itemsite_leadtime,"
               "       CASE WHEN (bufrsts_type='T') THEN :time"
               "            ELSE :stock"
               "       END AS bufrststype,"
               "       bufrsts_status,"
               "       formatQty(qoh) AS f_qoh,"
               "       formatQty(allocated) AS f_allocated,"
               "       formatQty(noNeg(qoh - allocated)) AS f_unallocated,"
               "       formatQty(ordered) AS f_ordered,"
               "       formatQty(reorderlevel) AS f_reorderlevel,"
               "       formatQty(outlevel) AS f_outlevel,"
               "       formatQty(qoh - allocated + ordered) AS f_available,"
               "       emergency "
               "  FROM ( SELECT itemsite_id,"
               "                CASE WHEN (item_type IN ('P', 'O')) THEN 1"
               "                     WHEN (item_type IN ('M')) THEN 2"
               "                     ELSE 0"
               "                END AS itemtype,"
               "                item_number, item_descrip1, item_descrip2,"
               "                warehous_id, warehous_code, itemsite_leadtime,"
               "                bufrsts_status, bufrsts_type,"
               "                itemsite_qtyonhand AS qoh,"
               "                itemsite_reorderlevel AS reorderlevel,"
               "                itemsite_ordertoqty AS outlevel," 
               "                qtyAllocated(itemsite_id, endoftime()) AS allocated,"
               "                qtyOrdered(itemsite_id, endoftime()) AS ordered,"
               "                (bufrsts_status > 65) AS emergency "
               "           FROM item, itemsite, warehous, bufrsts "
               "          WHERE ( (itemsite_active)"
               "            AND   (itemsite_item_id=item_id)"
               "            AND   (itemsite_warehous_id=warehous_id)"
               "            AND   (bufrsts_target_type='I')"
               "            AND   (bufrsts_target_id=itemsite_id)"
               "            AND   (bufrsts_date=current_date)");

  if  (_GreaterThanZero->isChecked())
    sql += " AND (bufrsts_status > 0) ";

  else if (_EmergencyZone->isChecked())
    sql += " AND (bufrsts_status > 65)";
  
  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  if (_parameter->isSelected())
  {
    if (_parameter->type() == ClassCode)
      sql += " AND (item_classcode_id=:classcode_id)";
    else if (_parameter->type() == ItemGroup)
      sql += " AND (item_id IN (SELECT itemgrpitem_item_id FROM itemgrpitem WHERE (itemgrpitem_itemgrp_id=:itemgrp_id)))";
    else if (_parameter->type() == PlannerCode)
      sql += " AND (itemsite_plancode_id=:plancode_id)";
  }
  else if (_parameter->isPattern())
  {
    if (_parameter->type() == ClassCode)
      sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";
    else if (_parameter->type() == ItemGroup)
      sql += " AND (item_id IN (SELECT itemgrpitem_item_id FROM itemgrpitem, itemgrp WHERE ( (itemgrpitem_itemgrp_id=itemgrp_id) AND (itemgrp_name ~ :itemgrp_pattern) ) ))";
    else if (_parameter->type() == PlannerCode)
      sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";
  }
  else if (_parameter->type() == ItemGroup)
    sql += " AND (item_id IN (SELECT DISTINCT itemgrpitem_item_id FROM itemgrpitem))";

  sql += ") ) as data ";

  sql += "ORDER BY bufrsts_status DESC, item_number, warehous_code DESC;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _parameter->bindValue(q);
  q.bindValue(":stock", tr("Stock"));
  q.bindValue(":time", tr("Time"));
  q.exec();
  XTreeWidgetItem * last = 0;
  while (q.next())
  {
    last = new XTreeWidgetItem( _availability, last,
                                q.value("itemsite_id").toInt(), q.value("itemtype").toInt(),
                                q.value("item_number").toString(), q.value("itemdescrip"),
                                q.value("warehous_code"), q.value("itemsite_leadtime"),
                                q.value("bufrststype"), q.value("bufrsts_status"),
                                q.value("f_qoh"), q.value("f_allocated"),
                                q.value("f_unallocated"), q.value("f_ordered"));
    last->setText(10, q.value("f_reorderlevel").toString());
    last->setText(11, q.value("f_outlevel").toString());
    last->setText(12, q.value("f_available").toString());

    if (q.value("emergency").toBool())
      last->setTextColor(5, QColor("red"));
  }
}
