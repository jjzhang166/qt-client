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

#include "dspSlowMovingInventoryByClassCode.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "adjustmentTrans.h"
#include "enterMiscCount.h"
#include "transferTrans.h"
#include "createCountTagsByItem.h"

dspSlowMovingInventoryByClassCode::dspSlowMovingInventoryByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  _orderByGroupInt = new QButtonGroup(this);
  _orderByGroupInt->addButton(_itemNumber);
  _orderByGroupInt->addButton(_dateLastUsed);
  _orderByGroupInt->addButton(_inventoryValue);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemsite, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_showValue, SIGNAL(toggled(bool)), this, SLOT(sHandleValue(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ClassCode);

  _itemsite->addColumn(tr("Whs."),          _whsColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Item Number"),   _itemColumn, Qt::AlignLeft   );
  _itemsite->addColumn(tr("Description"),   -1,                  Qt::AlignLeft   );
  _itemsite->addColumn(tr("UOM"),           _uomColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Last Movement"), _itemColumn, Qt::AlignCenter );
  _itemsite->addColumn(tr("QOH"),           _qtyColumn,  Qt::AlignRight  );
  _itemsite->addColumn(tr("Unit Cost"),     _costColumn, Qt::AlignRight  );
  _itemsite->addColumn(tr("Value"),         _costColumn, Qt::AlignRight  );

  sHandleValue(_showValue->isChecked());

  _showValue->setEnabled(_privileges->check("ViewInventoryValue"));
}

dspSlowMovingInventoryByClassCode::~dspSlowMovingInventoryByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSlowMovingInventoryByClassCode::languageChange()
{
  retranslateUi(this);
}

bool dspSlowMovingInventoryByClassCode::setParams(ParameterList & params)
{
  if(!_cutoffDate->isValid())
  {
    QMessageBox::warning(this, tr("No Cutoff Date"),
        tr("You must specify a cutoff date."));
    _cutoffDate->setFocus();
    return false;
  }

  params.append("cutoffDate", _cutoffDate->date());
  _warehouse->appendValue(params);
  _classCode->appendValue(params);

  if (_itemNumber->isChecked())
    params.append("orderByItemNumber");

  if (_dateLastUsed->isChecked())
    params.append("orderByDateLastUsed");

  if (_inventoryValue->isChecked())
    params.append("orderByInventoryValue");

  if(_showValue->isChecked())
    params.append("showValue");

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if (_useActualCosts->isChecked())
    params.append("useActualCosts");

  if (_itemNumber->isChecked())
    params.append("sortByItem");
  else if (_dateLastUsed->isChecked())
    params.append("sortByDate");


  return true;
}

void dspSlowMovingInventoryByClassCode::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("SlowMovingInventoryByClassCode", params);

  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
  }
}

void dspSlowMovingInventoryByClassCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Transfer to another Warehouse..."), this, SLOT(sTransfer()), 0);
    if (!_privileges->check("CreateInterWarehouseTrans"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Adjust this QOH..."), this, SLOT(sAdjust()), 0);
    if (!_privileges->check("CreateAdjustmentTrans"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Reset this QOH to 0..."), this, SLOT(sReset()), 0);
    if (!_privileges->check("CreateAdjustmentTrans"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Enter Misc. Count..."), this, SLOT(sMiscCount()), 0);
    if (!_privileges->check("EnterMiscCounts"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
    if (!_privileges->check("IssueCountTags"))
      pMenu->setItemEnabled(menuItem, FALSE);
  } 
}

void dspSlowMovingInventoryByClassCode::sTransfer()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSlowMovingInventoryByClassCode::sAdjust()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSlowMovingInventoryByClassCode::sReset()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());
  params.append("qty", 0.0);

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSlowMovingInventoryByClassCode::sMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());

  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspSlowMovingInventoryByClassCode::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspSlowMovingInventoryByClassCode::sHandleValue(bool pShowValue)
{
  _itemsite->setColumnHidden(6, !pShowValue);
  _itemsite->setColumnHidden(7, !pShowValue);

  _costsGroup->setEnabled(pShowValue);
}

void dspSlowMovingInventoryByClassCode::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  _itemsite->clear();

  QString sql( "SELECT itemsite_id, warehous_code, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, uom_name,"
               "       itemsite_qtyonhand, formatQty(itemsite_qtyonhand) AS f_qoh,"
               "       formatDate(itemsite_datelastused) AS f_datelastused,"
               "       formatCost(cost) AS f_unitcost,"
               "       noNeg(cost * itemsite_qtyonhand) AS value,"
               "       formatMoney(noNeg(cost * itemsite_qtyonhand)) AS f_value,"
               "       cost "
               "FROM ( SELECT itemsite_id, warehous_code, item_number,"
               "              item_descrip1, item_descrip2, uom_name,"
               "              itemsite_qtyonhand, itemsite_datelastused,"
	       "<? if exists(\"useActualCosts\") ?>"
	       "              actcost(itemsite_item_id) "
	       "<? else ?>"
	       "              stdcost(itemsite_item_id) "
	       "<? endif ?> AS cost "
	       "FROM itemsite, item, warehous, uom "
	       "WHERE ((itemsite_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
	       " AND (itemsite_warehous_id=warehous_id)"
	       " AND (itemsite_active)"
	       " AND (itemsite_datelastused < <? value(\"cutoffDate\") ?>)"
	       "<? if exists(\"warehous_id\") ?>"
	       " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"classcode_id\") ?>"
	       " AND (item_classcode_id=<? value(\"classcode_id\") ?>)"
	       "<? elseif exists(\"classcode_pattern\") ?>"
	       " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE classcode_code ~ <? value(\"classcode_pattern\") ?>))"
	       "<? endif ?>"
	       ") ) AS data "
	       "ORDER BY warehous_code, "
	       "<? if exists(\"sortByItem\") ?>"
	       "         item_number"
	       "<? elseif exists(\"sortByDate\") ?>"
	       "         itemsite_datelastused"
	       "<? else ?>"
	       "         noNeg(cost * itemsite_qtyonhand) DESC"
	       "<? endif ?>"
	       ";");

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  double qty   = 0.0;
  double value = 0.0;
  XTreeWidgetItem *last = 0;
  while (q.next())
  {
    last = new XTreeWidgetItem( _itemsite, last, q.value("itemsite_id").toInt(),
				q.value("warehous_code"), q.value("item_number"),
				q.value("itemdescrip"), q.value("uom_name"),
				q.value("f_datelastused"), q.value("f_qoh") );

    last->setText(6, q.value("f_unitcost").toString());
    last->setText(7, q.value("f_value").toString());

    if (q.value("itemsite_qtyonhand").toDouble() > 0.0)
    {
      qty += q.value("itemsite_qtyonhand").toDouble();
      value += q.value("value").toDouble();
    }
  }
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
