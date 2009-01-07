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

#include "dspInventoryAvailabilityByParameterList.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "createCountTagsByItem.h"
#include "dspAllocations.h"
#include "dspInventoryHistoryByItem.h"
#include "dspOrders.h"
#include "dspRunningAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "enterMiscCount.h"
#include "mqlutil.h"
#include "postMiscProduction.h"
#include "purchaseOrder.h"
#include "purchaseRequest.h"
#include "workOrder.h"

dspInventoryAvailabilityByParameterList::dspInventoryAvailabilityByParameterList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _showByGroupInt = new QButtonGroup(this);
  _showByGroupInt->addButton(_byLeadTime);
  _showByGroupInt->addButton(_byDays);
  _showByGroupInt->addButton(_byDate);
  _showByGroupInt->addButton(_byDates);

  connect(_availability, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showReorder, SIGNAL(toggled(bool)), this, SLOT(sHandleShowReorder(bool)));
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _availability->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter,true, "warehous_code");
  _availability->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,  true, "item_number");
  _availability->addColumn(tr("Description"),  -1,          Qt::AlignLeft,  true, "itemdescrip");
  _availability->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter,true, "uom_name");
  _availability->addColumn(tr("LT"),           _whsColumn,  Qt::AlignCenter,true, "itemsite_leadtime");
  _availability->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight, true, "qoh");
  _availability->addColumn(tr("Allocated"),    _qtyColumn,  Qt::AlignRight, true, "allocated");
  _availability->addColumn(tr("Unallocated"),  _qtyColumn,  Qt::AlignRight, true, "unallocated");
  _availability->addColumn(tr("On Order"),     _qtyColumn,  Qt::AlignRight, true, "ordered");
  _availability->addColumn(tr("Reorder Lvl."), _qtyColumn,  Qt::AlignRight, true, "reorderlevel");
  _availability->addColumn(tr("OUT Level."),   _qtyColumn,  Qt::AlignRight, false, "outlevel");
  _availability->addColumn(tr("Available"),    _qtyColumn,  Qt::AlignRight, true, "available");

  if (_preferences->boolean("XCheckBox/forgetful"))
    _ignoreReorderAtZero->setChecked(true);

  sHandleShowReorder(_showReorder->isChecked());
}

dspInventoryAvailabilityByParameterList::~dspInventoryAvailabilityByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryAvailabilityByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityByParameterList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("classcode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ClassCode);

  param = pParams.value("plancode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("plancode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("plancode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::PlannerCode);

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("itemgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("itemgrp", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ItemGroup);

  switch (_parameter->type())
  {
    case ParameterGroup::ClassCode:
      setWindowTitle(tr("Inventory Availability by Class Code"));
      break;

    case ParameterGroup::PlannerCode:
      setWindowTitle(tr("Inventory Availability by Planner Code"));
      break;

    case ParameterGroup::ItemGroup:
      setWindowTitle(tr("Inventory Availability by Item Group"));
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

bool dspInventoryAvailabilityByParameterList::setParams(ParameterList &params)
{
  if ((_byDate->isChecked()) && (!_date->isValid()))
  {
    QMessageBox::critical(this, tr("Enter Valid Date"),
                          tr("<p>You have choosen to view Inventory "
			     "Availability as of a given date but have not "
			     "indicated the date. Please enter a valid date."));
    _date->setFocus();
    return false;
  }

  _parameter->appendValue(params);
  _warehouse->appendValue(params);

  if (_parameter->isAll())
  {
    if (_parameter->type() == ParameterGroup::ItemGroup)
      params.append("itemgrp");
    else if(_parameter->type() == ParameterGroup::PlannerCode)
      params.append("plancode");
    else if (_parameter->type() == ParameterGroup::ClassCode)
      params.append("classcode");
  }

  if (_byLeadTime->isChecked())
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

  return true;
}

void dspInventoryAvailabilityByParameterList::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("InventoryAvailabilityByParameterList", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);
}

void dspInventoryAvailabilityByParameterList::sPopulateMenu(QMenu *menu, QTreeWidgetItem *selected)
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)selected;
  int menuItem;

  menuItem = menu->insertItem(tr("View Inventory History..."), this, SLOT(sViewHistory()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    menu->setItemEnabled(menuItem, FALSE);

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
  if (item->rawValue("allocated").toDouble() == 0.0)
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
  if (item->rawValue("ordered").toDouble() == 0.0)
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

void dspInventoryAvailabilityByParameterList::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  if (_byLeadTime->isChecked())
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

void dspInventoryAvailabilityByParameterList::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  if (_byLeadTime->isChecked())
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

void dspInventoryAvailabilityByParameterList::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sPostMiscProduction()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());

  postMiscProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByParameterList::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByParameterList::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sViewSubstituteAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  if (_byLeadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByParameterList::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  
  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByParameterList::sHandleShowReorder(bool pValue)
{
  _ignoreReorderAtZero->setEnabled(pValue);
  if (pValue && _preferences->boolean("XCheckBox/forgetful"))
    _showShortages->setChecked(TRUE);
}

void dspInventoryAvailabilityByParameterList::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("inventoryAvailability", "general");
  q = mql.toQuery(params);
  _availability->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
