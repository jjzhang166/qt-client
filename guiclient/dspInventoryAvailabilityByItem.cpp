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

#include "dspInventoryAvailabilityByItem.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <openreports.h>

#include "createCountTagsByItem.h"
#include "dspAllocations.h"
#include "dspInventoryHistoryByItem.h"
#include "dspOrders.h"
#include "dspRunningAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "enterMiscCount.h"
#include "inputManager.h"
#include "purchaseOrder.h"
#include "purchaseRequest.h"
#include "workOrder.h"

dspInventoryAvailabilityByItem::dspInventoryAvailabilityByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  _showByGroupInt = new QButtonGroup(this);
  _showByGroupInt->addButton(_leadTime);
  _showByGroupInt->addButton(_byDays);
  _showByGroupInt->addButton(_byDate);
  _showByGroupInt->addButton(_byDates);

  connect(_availability, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showReorder, SIGNAL(toggled(bool)), this, SLOT(sHandleShowReorder(bool)));

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _availability->addColumn(tr("Whs."),         -1,         Qt::AlignCenter );
  _availability->addColumn(tr("LT"),           _whsColumn, Qt::AlignCenter );
  _availability->addColumn(tr("QOH"),          _qtyColumn, Qt::AlignRight  );
  _availability->addColumn(tr("Allocated"),    _qtyColumn, Qt::AlignRight  );
  _availability->addColumn(tr("Unallocated"),  _qtyColumn, Qt::AlignRight  );
  _availability->addColumn(tr("On Order"),     _qtyColumn, Qt::AlignRight  );
  _availability->addColumn(tr("Reorder Lvl."), _qtyColumn, Qt::AlignRight  );
  _availability->addColumn(tr("OUT Level"),    _qtyColumn, Qt::AlignRight  );
  _availability->addColumn(tr("Available"),    _qtyColumn, Qt::AlignRight  );

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _item->setFocus();

  if (_preferences->boolean("XCheckBox/forgetful"))
    _ignoreReorderAtZero->setChecked(true);

  sHandleShowReorder(_showReorder->isChecked());
}

dspInventoryAvailabilityByItem::~dspInventoryAvailabilityByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryAvailabilityByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityByItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(TRUE);
  }

  _leadTime->setChecked(pParams.inList("byLeadTime"));

  param = pParams.value("byDays", &valid);
  if (valid)
  {
   _byDays->setChecked(TRUE);
   _days->setValue(param.toInt());
  }

  param = pParams.value("byDate", &valid);
  if (valid)
  {
   _byDate->setChecked(TRUE);
   _date->setDate(param.toDate());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspInventoryAvailabilityByItem::sPrint()
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Invalid Data"),
                      tr("You must enter a valid Item Number for this report.") );
    _item->setFocus();
    return;
  }

  ParameterList params;
  params.append("item_id", _item->id());
  _warehouse->appendValue(params);

  if (_leadTime->isChecked())
    params.append("byLeadTime");
  else if (_byDays->isChecked())
    params.append("byDays", _days->text().toInt());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byDates");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }


  if(_showReorder->isChecked())
    params.append("showReorder");

  if(_ignoreReorderAtZero->isChecked())
    params.append("ignoreReorderAtZero");

  if(_showShortages->isChecked())
    params.append("showShortages");

  orReport report("InventoryAvailabilityByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInventoryAvailabilityByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Inventory History..."), this, SLOT(sViewHistory()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
  if (selected->text(3).remove(',').toDouble() == 0.0)
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
  if (selected->text(5).remove(',').toDouble() == 0.0)
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sRunningAvailability()), 0);

  pMenu->insertSeparator();

  if (((XTreeWidgetItem *)selected)->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Create P/R..."), this, SLOT(sCreatePR()), 0);
    if (!_privileges->check("MaintainPurchaseRequests"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Create P/O..."), this, SLOT(sCreatePO()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();
  }
  else if (((XTreeWidgetItem *)selected)->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Create W/O..."), this, SLOT(sCreateWO()), 0);
    if (!_privileges->check("MaintainWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();
  }

  menuItem = pMenu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
  if (!_privileges->check("IssueCountTags"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Enter Misc. Inventory Count..."), this, SLOT(sEnterMiscCount()), 0);
  if (!_privileges->check("EnterMiscCounts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  pMenu->insertItem(tr("View Substitute Availability..."), this, SLOT(sViewSubstituteAvailability()), 0);

}

void dspInventoryAvailabilityByItem::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byRange");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byRange");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByItem::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sViewSubstituteAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());

  if (_leadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByItem::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());

  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByItem::sHandleShowReorder(bool pValue)
{
  _ignoreReorderAtZero->setEnabled(pValue);
  if (pValue && _preferences->boolean("XCheckBox/forgetful"))
    _showShortages->setChecked(TRUE);
}

void dspInventoryAvailabilityByItem::sFillList()
{
  _availability->clear();

  if ((_byDate->isChecked()) && (!_date->isValid()))
  {
    QMessageBox::critical( this, tr("Enter Valid Date"),
                           tr( "You have choosen to view Inventory Availabilty as of a given date but have not.\n"
                               "indicated the date.  Please enter a valid date." ) );
    _date->setFocus();
    return;
  }

  QString sql( "SELECT itemsite_id, itemtype, warehous_id, warehous_code, itemsite_leadtime,"
               "       formatQty(qoh) AS f_qoh,"
               "       formatQty(noNeg(qoh - allocated)) AS f_unallocated,"
               "       formatQty(allocated) AS f_allocated,"
               "       formatQty(ordered) AS f_ordered,"
               "       formatQty(reorderlevel) AS f_reorderlevel,"
               "       formatQty(outlevel) AS f_outlevel,"
               "       (qoh - allocated + ordered) AS available,"
               "       formatQty(qoh - allocated + ordered) AS f_available,"
               "       ((qoh - allocated + ordered) < 0) AS stockout,"
               "       ((qoh - allocated + ordered) <= reorderlevel) AS reorder "
               "FROM ( SELECT itemsite_id,"
               "              CASE WHEN (item_type IN ('P', 'O')) THEN 1"
               "                   WHEN (item_type IN ('M')) THEN 2"
               "                   ELSE 0"
               "              END AS itemtype,"
               "              warehous_id, warehous_code, itemsite_leadtime,"
               "              CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
               "              CASE WHEN(itemsite_useparams) THEN itemsite_ordertoqty ELSE 0.0 END AS outlevel,"
               "              itemsite_qtyonhand AS qoh," );

  if (_leadTime->isChecked())
    sql += " qtyAllocated(itemsite_id, itemsite_leadtime) AS allocated,"
           " qtyOrdered(itemsite_id, itemsite_leadtime) AS ordered ";

  else if (_byDays->isChecked())
    sql += " qtyAllocated(itemsite_id, :days) AS allocated,"
           " qtyOrdered(itemsite_id, :days) AS ordered ";

  else if (_byDate->isChecked())
    sql += " qtyAllocated(itemsite_id, (:date - CURRENT_DATE)) AS allocated,"
           " qtyOrdered(itemsite_id, (:date - CURRENT_DATE)) AS ordered ";

  else if (_byDates->isChecked())
    sql += " qtyAllocated(itemsite_id, :startDate, :endDate) AS allocated,"
           " qtyOrdered(itemsite_id, :startDate, :endDate) AS ordered ";


   sql += "FROM itemsite, warehous, item "
          "WHERE ( (itemsite_active)"
          " AND (itemsite_warehous_id=warehous_id)"
          " AND (itemsite_item_id=item_id)"
          " AND (item_id=:item_id)";

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") ) AS data ";

  if (_showReorder->isChecked())
  {
    sql += "WHERE ( ((qoh - allocated + ordered) <= reorderlevel) ";

    if (_ignoreReorderAtZero->isChecked())
      sql += " AND (NOT ( ((qoh - allocated + ordered) = 0) AND (reorderlevel = 0)) ) ) ";
    else
      sql += ") ";
  }
  else if (_showShortages->isChecked())
    sql += "WHERE ((qoh - allocated + ordered) < 0) ";

  sql += "ORDER BY warehous_code DESC;";

  q.prepare(sql);
  q.bindValue(":days", _days->value());
  q.bindValue(":date", _date->date());
  q.bindValue(":startDate", _startDate->date());
  q.bindValue(":endDate", _endDate->date());
  q.bindValue(":item_id", _item->id());
  _warehouse->bindValue(q);
  q.exec();
  XTreeWidgetItem * last = 0;
  while (q.next())
  {
    last = new XTreeWidgetItem( _availability, last,
                                q.value("itemsite_id").toInt(), q.value("itemtype").toInt(),
                                q.value("warehous_code"), q.value("itemsite_leadtime"),
                                q.value("f_qoh"), q.value("f_allocated"),
                                q.value("f_unallocated"), q.value("f_ordered"),
                                q.value("f_reorderlevel"), q.value("f_outlevel"),
                                q.value("f_available") );

    if (_byDates->isChecked())
      last->setTextColor(2, "grey");

    if (q.value("stockout").toBool())
      last->setTextColor(8, "red");
    else if (q.value("reorder").toBool())
      last->setTextColor(8, "orange");
  }
}
