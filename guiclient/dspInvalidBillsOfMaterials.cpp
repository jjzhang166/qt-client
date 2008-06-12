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

#include "dspInvalidBillsOfMaterials.h"

#include <QMenu>
#include <QVariant>

#include <parameter.h>

#include "item.h"
#include "itemSite.h"

dspInvalidBillsOfMaterials::dspInvalidBillsOfMaterials(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_exceptions, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query,  SIGNAL(clicked()),     this, SLOT(sFillList()));
  connect(_update, SIGNAL(toggled(bool)), this, SLOT(sHandleUpdate()));

  _exceptions->addColumn("componentItemid",                 0, Qt::AlignCenter);
  _exceptions->addColumn("componentWarehouseId",            0, Qt::AlignCenter);
  _exceptions->addColumn(tr("Whs."),               _whsColumn, Qt::AlignCenter);
  _exceptions->addColumn(tr("Parent Item #"),     _itemColumn, Qt::AlignLeft  );
  _exceptions->addColumn(tr("Component Item #"),  _itemColumn, Qt::AlignLeft  );
  _exceptions->addColumn(tr("Component Item Description"), -1, Qt::AlignLeft  );

  if (_preferences->boolean("XCheckBox/forgetful"))
    _update->setChecked(true);
}

dspInvalidBillsOfMaterials::~dspInvalidBillsOfMaterials()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInvalidBillsOfMaterials::languageChange()
{
    retranslateUi(this);
}

void dspInvalidBillsOfMaterials::sEditItem()
{
  item::editItem(_exceptions->altId());
}

void dspInvalidBillsOfMaterials::sCreateItemSite()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id",     _exceptions->currentItem()->text(0).toInt());
  params.append("warehous_id", _exceptions->currentItem()->text(1).toInt());
  
  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInvalidBillsOfMaterials::sEditItemSite()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", _exceptions->id());
  
  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInvalidBillsOfMaterials::sHandleUpdate()
{
  if (_update->isChecked())
  {
    connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
    connect(omfgThis, SIGNAL(itemsitesUpdated()), this, SLOT(sFillList()));
  }
  else
  {
    disconnect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
    disconnect(omfgThis, SIGNAL(itemsitesUpdated()), this, SLOT(sFillList()));
  }
}

void dspInvalidBillsOfMaterials::sFillList()
{
  QString sql( "SELECT itemsite_id, parent.item_id, component.item_id, warehous_id, warehous_code,"
               "       parent.item_number AS parentitem,"
               "       component.item_number AS componentitem, (component.item_descrip1 || ' ' || component.item_descrip2) "
               "FROM bomitem, itemsite, item AS parent, item AS component, warehous "
               "WHERE ( (bomitem_parent_item_id=parent.item_id)"
               " AND (bomitem_item_id=component.item_id)"
               " AND (CURRENT_DATE BETWEEN bomitem_effective AND (bomitem_expires - 1))"
               " AND (itemsite_item_id=parent.item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (parent.item_type='M')"
               " AND (itemsite_supply)"
               " AND (itemsite_active)"
               " AND (component.item_id NOT IN ( SELECT itemsite_item_id"
               "                                 FROM itemsite"
               "                                 WHERE ((itemsite_warehous_id=warehous_id)"
               "                                  AND (itemsite_active)) ) )" );

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY warehous_code, parentitem, componentitem;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.exec();

  _exceptions->populate(q, TRUE);                               
}

void dspInvalidBillsOfMaterials::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Parent Item..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Parent Item Site..."), this, SLOT(sEditItemSite()), 0);
  if (!_privileges->check("MaintainItemSites"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Create Component Item Site..."), this, SLOT(sCreateItemSite()), 0);
  if (!_privileges->check("MaintainItemSites"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

