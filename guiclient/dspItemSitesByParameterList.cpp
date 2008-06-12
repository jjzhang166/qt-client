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

#include "dspItemSitesByParameterList.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include "createCountTagsByItem.h"
#include "dspInventoryAvailabilityByItem.h"
#include "itemSite.h"

/*
 *  Constructs a dspItemSitesByParameterList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemSitesByParameterList::dspItemSitesByParameterList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_itemsite, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _parameter->setType(ClassCode);

  _itemsite->addColumn(tr("Whs."),          _whsColumn,   Qt::AlignCenter );
  _itemsite->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignLeft   );
  _itemsite->addColumn(tr("Description"),   -1,           Qt::AlignLeft   );
  _itemsite->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter );
  _itemsite->addColumn(tr("QOH"),           _qtyColumn,   Qt::AlignRight  );
  _itemsite->addColumn(tr("Loc. Cntrl."),   _dateColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Cntrl. Meth."),  _dateColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Sold Ranking"),  _dateColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Last Cnt'd"),    _dateColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Last Used"),     _dateColumn,  Qt::AlignCenter );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemSitesByParameterList::~dspItemSitesByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemSitesByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspItemSitesByParameterList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  _showInactive->setChecked(pParams.inList("showInactive"));

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

  param = pParams.value("costcat_id", &valid);
  if (valid)
  {
    _parameter->setType(CostCategory);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("costcat_pattern", &valid);
  if (valid)
  {
    _parameter->setType(CostCategory);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("costcat", &valid);
  if (valid)
    _parameter->setType(CostCategory);

  switch (_parameter->type())
  {
    case ClassCode:
      setCaption(tr("Item Sites by Class Code"));
      break;

    case PlannerCode:
      setCaption(tr("Item Sites by Planner Code"));
      break;

    case ItemGroup:
      setCaption(tr("Item Sites by Item Group"));
      break;

    case CostCategory:
      setCaption(tr("Item Sites by Cost Category"));
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

void dspItemSitesByParameterList::sPrint()
{
  ParameterList params;
  _parameter->appendValue(params);
  _warehouse->appendValue(params);

  if(_showInactive->isChecked())
    params.append("showInactive");

  orReport report("ItemSitesByParameterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemSitesByParameterList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByParameterList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByParameterList::sInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("run");
  params.append("byLeadTime");

  dspInventoryAvailabilityByItem *newdlg = new dspInventoryAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSitesByParameterList::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByParameterList::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Item Site..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainItemSites")) && (!_privileges->check("ViewItemSites")))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Item Site..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemSites"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("View Inventory Availability..."), this, SLOT(sInventoryAvailability()), 0);
  if (!_privileges->check("ViewInventoryAvailability"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
  if (!_privileges->check("IssueCountTags"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspItemSitesByParameterList::sFillList()
{
 QString sql( "SELECT itemsite_id, warehous_code, item_number,"
               "      (item_descrip1 || ' ' || item_descrip2), uom_name,"
               "      formatQty(itemsite_qtyonhand),"
               "      formatBoolYN(itemsite_loccntrl),"
               "      CASE WHEN itemsite_controlmethod='R' THEN :regular"
               "           WHEN itemsite_controlmethod='N' THEN :none"
               "           WHEN itemsite_controlmethod='L' THEN :lot"
               "           WHEN itemsite_controlmethod='S' THEN :serial"
               "      END,"
               "      CASE WHEN (itemsite_sold) THEN TEXT(itemsite_soldranking)"
               "           ELSE :na"
               "      END,"
               "      formatDate(itemsite_datelastcount, 'Never'),"
               "      formatDate(itemsite_datelastused, 'Never') "
               "FROM itemsite, warehous, item, uom "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (itemsite_warehous_id=warehous_id)" );

  if (_parameter->isSelected())
  {
    if (_parameter->type() == ClassCode)
      sql += " AND (item_classcode_id=:classcode_id)";
    else if (_parameter->type() == ItemGroup)
      sql += " AND (item_id IN (SELECT itemgrpitem_item_id FROM itemgrpitem WHERE (itemgrpitem_itemgrp_id=:itemgrp_id)))";
    else if (_parameter->type() == PlannerCode)
      sql += " AND (itemsite_plancode_id=:plancode_id)";
    else if (_parameter->type() == CostCategory)
      sql += " AND (itemsite_costcat_id=:costcat_id)";
  }
  else if (_parameter->isPattern())
  {
    if (_parameter->type() == ClassCode)
      sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";
    else if (_parameter->type() == ItemGroup)
      sql += " AND (item_id IN (SELECT itemgrpitem_item_id FROM itemgrpitem, itemgrp WHERE ( (itemgrpitem_itemgrp_id=itemgrp_id) AND (itemgrp_name ~ :itemgrp_pattern) ) ))";
    else if (_parameter->type() == PlannerCode)
      sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";
    else if (_parameter->type() == CostCategory)
      sql += " AND (itemsite_costcat_id IN (SELECT costcat_id FROM costcat WHERE (costcat_code ~ :costcat_pattern)))";
  }

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  if (!_showInactive->isChecked())
    sql += " AND (itemsite_active)";

  sql += ") "
         "ORDER BY item_number;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _parameter->bindValue(q);
  q.bindValue(":regular", tr("Regular"));
  q.bindValue(":none", tr("None"));
  q.bindValue(":lot", tr("Lot #"));
  q.bindValue(":serial", tr("Serial #"));
  q.bindValue(":na", tr("N/A"));
  q.exec();
  _itemsite->populate(q);
}

