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

#include "dspUnbalancedQOHByClassCode.h"

#include <QMenu>
#include <QSqlError>

#include "createCountTagsByItem.h"
#include "dspInventoryAvailabilityByItem.h"
#include "itemSite.h"
#include "storedProcErrorLookup.h"

dspUnbalancedQOHByClassCode::dspUnbalancedQOHByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_itemsite, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ClassCode);

  _itemsite->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter );
  _itemsite->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _itemsite->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _itemsite->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter );
  _itemsite->addColumn(tr("QOH"),         _qtyColumn,   Qt::AlignRight  );
  _itemsite->addColumn(tr("QOH Detail."), _qtyColumn,   Qt::AlignRight  );
  _itemsite->addColumn(tr("NN QOH"),      _qtyColumn,   Qt::AlignRight  );
  _itemsite->addColumn(tr("NN Detail."),  _qtyColumn,   Qt::AlignRight  );
}

dspUnbalancedQOHByClassCode::~dspUnbalancedQOHByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspUnbalancedQOHByClassCode::languageChange()
{
  retranslateUi(this);
}

void dspUnbalancedQOHByClassCode::sBalance()
{
  q.prepare("SELECT balanceItemsite(:itemsite_id) AS result;");
  q.bindValue(":itemsite_id", _itemsite->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("balanceItemsite", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspUnbalancedQOHByClassCode::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspUnbalancedQOHByClassCode::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspUnbalancedQOHByClassCode::sInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("byLeadTime");
  params.append("run");

  dspInventoryAvailabilityByItem *newdlg = new dspInventoryAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUnbalancedQOHByClassCode::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspUnbalancedQOHByClassCode::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Balance Item Site..."), this, SLOT(sBalance()), 0);

  pMenu->insertSeparator();

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

void dspUnbalancedQOHByClassCode::sFillList()
{
  QString sql( "SELECT itemsite_id, warehous_code, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2), uom_name,"
               "       formatQty(itemsite_qtyonhand),"
               "       formatQty(detailedQOH(itemsite_id, FALSE)),"
               "       formatQty(itemsite_nnqoh),"
               "       formatQty(detailedNNQOH(itemsite_id, FALSE)) "
               "FROM warehous, item, itemsite, uom "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND ( (itemsite_loccntrl) OR (itemsite_controlmethod IN ('L', 'S')) )"
               " AND ( (itemsite_qtyonhand <> detailedQOH(itemsite_id, FALSE))"
               "    OR (itemsite_nnqoh <> detailedNNQOH(itemsite_id, FALSE)) )" );

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY item_number;";

  q.prepare(sql);
  q.bindValue(":regular", tr("Regular"));
  q.bindValue(":none", tr("None"));
  q.bindValue(":lot", tr("Lot #"));
  q.bindValue(":serial", tr("Serial #"));
  _warehouse->bindValue(q);
  _classCode->bindValue(q);
  q.exec();
  _itemsite->populate(q);
}
