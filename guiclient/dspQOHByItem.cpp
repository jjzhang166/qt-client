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

#include "dspQOHByItem.h"

#include <QMenu>

#include <openreports.h>
#include <parameter.h>

#include "adjustmentTrans.h"
#include "createCountTagsByItem.h"
#include "dspInventoryLocator.h"
#include "enterMiscCount.h"
#include "inputManager.h"
#include "transferTrans.h"

dspQOHByItem::dspQOHByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_qoh, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showValue, SIGNAL(toggled(bool)), this, SLOT(sHandleValue(bool)));

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _qoh->addColumn(tr("Whs."),             -1,           Qt::AlignCenter );
  _qoh->addColumn(tr("Default Location"), _itemColumn,  Qt::AlignLeft   );
  _qoh->addColumn(tr("Reorder Lvl."),     _qtyColumn,   Qt::AlignRight  );
  _qoh->addColumn(tr("QOH"),              _qtyColumn,   Qt::AlignRight  );
  _qoh->addColumn(tr("Non-Netable"),      _qtyColumn,   Qt::AlignRight  );
  _qoh->addColumn(tr("Unit Cost"),        _costColumn,  Qt::AlignRight  );
  _qoh->addColumn(tr("Value"),            _costColumn,  Qt::AlignRight  );
  _qoh->addColumn(tr("NN Value"),         _costColumn,  Qt::AlignRight  );
  sHandleValue(_showValue->isChecked());

  _showValue->setEnabled(_privileges->check("ViewInventoryValue"));

  sHandleValue(_showValue->isChecked());

  _item->setFocus();
}

dspQOHByItem::~dspQOHByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspQOHByItem::languageChange()
{
  retranslateUi(this);
}

void dspQOHByItem::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  _warehouse->appendValue(params);

  if (_showValue->isChecked())
    params.append("showValue");

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if (_useActualCosts->isChecked())
    params.append("useActualCosts");

  orReport report("QOHByItem", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspQOHByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if (((XTreeWidgetItem *)pSelected)->id() != -1)
  {
    int menuItem;

    if (((XTreeWidgetItem *)pSelected)->altId())
    {
      pMenu->insertItem(tr("View Location/Lot/Serial # Detail..."), this, SLOT(sViewDetail()), 0);
      pMenu->insertSeparator();
    }
  
    if (_metrics->boolean("MultiWhs"))
    {
      menuItem = pMenu->insertItem(tr("Transfer to another Warehouse..."), this, SLOT(sTransfer()), 0);
      if (!_privileges->check("CreateInterWarehouseTrans"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }

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

void dspQOHByItem::sViewDetail()
{
  ParameterList params;
  params.append("itemsite_id", _qoh->id());
  params.append("run");

  dspInventoryLocator *newdlg = new dspInventoryLocator();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sTransfer()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _qoh->id());

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sAdjust()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _qoh->id());

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sReset()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _qoh->id());
  params.append("qty", 0.0);

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _qoh->id());

  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspQOHByItem::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _qoh->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspQOHByItem::sHandleValue(bool pShowValue)
{
  _qoh->setColumnHidden(5, !pShowValue);
  _qoh->setColumnHidden(6, !pShowValue);
  _qoh->setColumnHidden(7, !pShowValue);
  _costsGroup->setEnabled(pShowValue);
}

void dspQOHByItem::sFillList()
{
  _qoh->clear();

  int itemsiteid = _qoh->id();

  QString sql( "SELECT itemsite_id, detail,"
               "       warehous_code,"
               "       CASE WHEN (NOT useDefaultLocation(itemsite_id)) THEN :none"
               "            ELSE defaultLocationName(itemsite_id)"
               "       END AS defaultlocation,"
               "       formatQty(reorderlevel) AS f_reorderlevel,"
               "       formatQty(qoh) AS f_qoh,"
               "       CASE WHEN (itemsite_loccntrl) THEN formatQty(nnqoh)"
               "            ELSE :na"
               "       END AS f_nnqoh,"
               "       formatCost(cost) AS f_unitcost,"
               "       formatMoney(cost * qoh) AS f_value,"
               "       CASE WHEN (itemsite_loccntrl) THEN formatMoney(cost * nnqoh)"
               "            ELSE :na"
               "       END AS f_nnvalue,"
               "       cost, reorderlevel, qoh, nnqoh "
               "FROM ( SELECT itemsite_id, itemsite_loccntrl,"
               "              CASE WHEN ((itemsite_loccntrl) OR (itemsite_controlmethod IN ('L', 'S'))) THEN 1"
               "                   ELSE 0"
               "              END AS detail,"
               "              warehous_code,"
               "              CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
               "              itemsite_qtyonhand AS qoh,"
               "              itemsite_nnqoh AS nnqoh," );

  if (_useStandardCosts->isChecked())
    sql += " stdcost(item_id) AS cost ";
  else if (_useActualCosts->isChecked())
    sql += " actcost(item_id) AS cost ";

  sql += "FROM itemsite, item, warehous "
         "WHERE ( (itemsite_item_id=item_id)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (itemsite_item_id=:item_id)"
         " AND (itemsite_active)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") ) AS data "
         "ORDER BY warehous_code;";

  q.prepare(sql);
  q.bindValue(":none", tr("None"));
  q.bindValue(":na", tr("N/A"));
  q.bindValue(":item_id", _item->id());
  _warehouse->bindValue(q);
  q.exec();
  if (q.first())
  {
    XTreeWidgetItem *selected     = 0;
    XTreeWidgetItem *last         = 0;
    double        netable         = 0.0;
    double        netableValue    = 0.0;
    double        nonNetable      = 0.0;
    double        nonNetableValue = 0.0;

    do
    {
      last = new XTreeWidgetItem( _qoh, last, q.value("itemsite_id").toInt(), q.value("detail").toInt(),
                                  q.value("warehous_code"), q.value("defaultlocation"),
                                  q.value("f_reorderlevel"), q.value("f_qoh"),
                                  q.value("f_nnqoh") );

      last->setText(5, q.value("f_unitcost").toString());
      last->setText(6, q.value("f_value").toString());
      last->setText(7, q.value("f_nnvalue").toString());

      if (q.value("qoh").toDouble() < 0)
        last->setTextColor(3, "red");
      else if (q.value("reorderlevel").toDouble() > q.value("qoh").toDouble())
        last->setTextColor(3, "orange");

      if (q.value("itemsite_id") == itemsiteid)
        selected = last;

      if (q.value("qoh").toDouble() > 0.0)
      {
        netable += q.value("qoh").toDouble();
        netableValue += (q.value("cost").toDouble() * q.value("qoh").toDouble());
      }

      if (q.value("nnqoh").toDouble() > 0.0)
      {
        nonNetable += q.value("nnqoh").toDouble();
        nonNetableValue += (q.value("cost").toDouble() * q.value("nnqoh").toDouble());
      }
    }
    while (q.next());

    last = new XTreeWidgetItem( _qoh, last, -1, 0, "", tr("Totals"), "",
                                formatQty(netable), formatQty(nonNetable) );
    last->setText(6, formatMoney(netableValue));
    last->setText(7, formatMoney(nonNetableValue));

    if (selected != NULL)
    {
      _qoh->setCurrentItem(selected);
      _qoh->scrollTo(_qoh->currentIndex());
    }
  }
}
