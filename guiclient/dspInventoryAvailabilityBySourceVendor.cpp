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

#include "dspInventoryAvailabilityBySourceVendor.h"

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
#include "purchaseOrder.h"
#include "purchaseRequest.h"

dspInventoryAvailabilityBySourceVendor::dspInventoryAvailabilityBySourceVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  _vendorGroupInt = new QButtonGroup(this);
  _vendorGroupInt->addButton(_allVendors);
  _vendorGroupInt->addButton(_selectedVendor);
  _vendorGroupInt->addButton(_selectedVendorType);
  _vendorGroupInt->addButton(_vendorTypePattern);

  _showByGroupInt = new QButtonGroup(this);
  _showByGroupInt->addButton(_leadTime);
  _showByGroupInt->addButton(_byDays);
  _showByGroupInt->addButton(_byDate);
  _showByGroupInt->addButton(_byDates);

  connect(_availability, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showReorder, SIGNAL(toggled(bool)), this, SLOT(sHandleShowReorder(bool)));

  _vendorTypes->setType(XComboBox::VendorTypes);

  _availability->addColumn(tr("Vendor #"),     _itemColumn, Qt::AlignLeft   );
  _availability->addColumn(tr("Item"),         _itemColumn, Qt::AlignLeft   );
  _availability->addColumn(tr("Description"),  -1,          Qt::AlignLeft   );
  _availability->addColumn(tr("Whs."),         _whsColumn,  Qt::AlignCenter );
  _availability->addColumn(tr("LT"),           _whsColumn,  Qt::AlignCenter );
  _availability->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Allocated"),    _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Unallocated"),  _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("On Order"),     _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Reorder Lvl."), _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("OUT Level"),    _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Available"),    _qtyColumn,  Qt::AlignRight  );
  
  if (_preferences->boolean("XCheckBox/forgetful"))
    _ignoreReorderAtZero->setChecked(true);

  sHandleShowReorder(_showReorder->isChecked());
}

dspInventoryAvailabilityBySourceVendor::~dspInventoryAvailabilityBySourceVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryAvailabilityBySourceVendor::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityBySourceVendor::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
  
  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspInventoryAvailabilityBySourceVendor::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);

  if (_selectedVendor->isChecked())
    params.append("vend_id", _vend->id());
  else if (_selectedVendorType->isChecked())
    params.append("vendtype_id", _vendorTypes->id());
  else if (_vendorTypePattern->isChecked())
    params.append("vendtype_pattern", _vendorType->text());

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

  orReport report("InventoryAvailabilityBySourceVendor", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInventoryAvailabilityBySourceVendor::sPopulateMenu(QMenu *menu, QTreeWidgetItem *selected)
{
  int menuItem;

  menuItem = menu->insertItem(tr("View Inventory History..."), this, SLOT(sViewHistory()), 0);

  menuItem = menu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
  if (selected->text(6).remove(',').toDouble() == 0.0)
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
  if (selected->text(8).remove(',').toDouble() == 0.0)
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Running Availability..."), this, SLOT(sRunningAvailability()), 0);

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("Create P/R..."), this, SLOT(sCreatePR()), 0);
  if (!_privileges->check("MaintainPurchaseRequests"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Create P/O..."), this, SLOT(sCreatePO()), 0);
  if (!_privileges->check("MaintainPurchaseOrders"))
    menu->setItemEnabled(menuItem, FALSE);

  menu->insertSeparator();

  menu->insertItem(tr("View Substitute Availability..."), this, SLOT(sViewSubstituteAvailability()), 0);

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
  if (!_privileges->check("IssueCountTags"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Enter Misc. Inventory Count..."), this, SLOT(sEnterMiscCount()), 0);
  if (!_privileges->check("EnterMiscCounts"))
    menu->setItemEnabled(menuItem, FALSE);
}

void dspInventoryAvailabilityBySourceVendor::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySourceVendor::sViewAllocations()
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

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySourceVendor::sViewOrders()
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

void dspInventoryAvailabilityBySourceVendor::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySourceVendor::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityBySourceVendor::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySourceVendor::sViewSubstituteAvailability()
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

  dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySourceVendor::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityBySourceVendor::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  
  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityBySourceVendor::sHandleShowReorder(bool pValue)
{
  _ignoreReorderAtZero->setEnabled(pValue);
  if (pValue && _preferences->boolean("XCheckBox/forgetful"))
    _showShortages->setChecked(TRUE);
}

void dspInventoryAvailabilityBySourceVendor::sFillList()
{
  _availability->clear();

  if ((_byDate->isChecked()) && (!_date->isValid()))
  {
    QMessageBox::critical(this, tr("Enter Valid Date"),
                          tr("<p>You have choosen to view Inventory Availability"
			     " as of a given date but have not indicated "
			     "the date.  Please enter a valid date.") );
    _date->setFocus();
    return;
  }

  if ((_byDates->isChecked()) && ( (!_startDate->isValid()) || (!_endDate->isValid()) ) )
  {
    QMessageBox::critical(this, tr("Enter Dates"),
                          tr("<p>You have choosen to view Inventory "
			     "Availability as of a given Start and End Date but "
			     "have not indicated the dates.  Please enter "
			     "valid dates." ) );
    _startDate->setFocus();
    return;
  }

  QString sql( "SELECT itemsite_id,"
               "       vend_number,"
               "       item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
               "       warehous_id, warehous_code, itemsite_leadtime,"
               "       formatQty(qoh) AS f_qoh,"
               "       formatQty(noNeg(qoh - allocated)) AS f_unallocated,"
               "       formatQty(noNeg(allocated)) AS f_allocated,"
               "       formatQty(ordered) AS f_ordered,"
               "       formatQty(reorderlevel) AS f_reorderlevel,"
               "       formatQty(outlevel) AS f_outlevel,"
               "       (qoh - allocated + ordered) AS available,"
               "       formatQty(qoh - allocated + ordered) AS f_available,"
               "       ((qoh - allocated + ordered) < 0) AS stockout,"
               "       ((qoh - allocated + ordered) <= reorderlevel) AS reorder "
               "FROM ( SELECT itemsite_id, vend_number,"
               "              item_number, item_descrip1, item_descrip2,"
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

   sql += "FROM item, itemsite, warehous, vend, itemsrc "
          "WHERE ( (itemsite_active)"
          " AND (itemsite_item_id=item_id)"
          " AND (itemsrc_item_id=item_id)"
          " AND (itemsite_warehous_id=warehous_id)"
          " AND (itemsrc_vend_id=vend_id)";

  if (_selectedVendor->isChecked())
    sql += " AND (vend_id=:vend_id)";
  else if (_selectedVendorType->isChecked())
    sql += " AND (vend_vendtype_id=:vendtype_id)";
  else if (_vendorTypePattern->isChecked())
    sql += " AND (vend_vendtype_id IN (SELECT vendtype_id FROM vendtype WHERE (vendtype_code ~ :vendtype_code))) ";

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

  if (_preferences->boolean("ListNumericItemNumbersFirst"))
    sql += "ORDER BY toNumeric(item_number, 999999999999999), item_number, warehous_code DESC;";
  else
    sql += "ORDER BY vend_number, item_number, warehous_code DESC;";

  q.prepare(sql);
  q.bindValue(":days", _days->value());
  q.bindValue(":date", _date->date());
  q.bindValue(":startDate", _startDate->date());
  q.bindValue(":endDate", _endDate->date());
  q.bindValue(":vend_id", _vend->id());
  q.bindValue(":vendtype_id", _vendorTypes->id());
  q.bindValue(":vendtype_code", _vendorType->text().upper());
  _warehouse->bindValue(q);
  q.exec();
  XTreeWidgetItem * last = 0;
  while (q.next())
  {
    last = new XTreeWidgetItem( _availability, last,
                                q.value("itemsite_id").toInt(),
                                q.value("vend_number"), q.value("item_number"),
                                q.value("description"), q.value("warehous_code"),
                                q.value("itemsite_leadtime"), q.value("f_qoh"),
                                q.value("f_allocated"), q.value("f_unallocated"),
                                q.value("f_ordered"), q.value("f_reorderlevel"),
                                q.value("f_outlevel") );

    last->setText(11, q.value("f_available").toString());

    if (_byDates->isChecked())
      last->setTextColor(5, "grey");

    if (q.value("stockout").toBool())
      last->setTextColor(11, "red");
    else if (q.value("reorder").toBool())
      last->setTextColor(11, "orange");
  }
}

