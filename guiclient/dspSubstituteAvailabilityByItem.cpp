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

#include "dspSubstituteAvailabilityByItem.h"

#include <QMenu>
#include <QVariant>

#include <openreports.h>

#include "dspAllocations.h"
#include "dspOrders.h"

dspSubstituteAvailabilityByItem::dspSubstituteAvailabilityByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  _showByGroupInt = new QButtonGroup(this);
  _showByGroupInt->addButton(_leadTime);
  _showByGroupInt->addButton(_byDays);
  _showByGroupInt->addButton(_byDate);

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _availability->addColumn(tr("Whs."),          _whsColumn,  Qt::AlignCenter );
  _availability->addColumn(tr("Item Number"),   _itemColumn, Qt::AlignLeft   );
  _availability->addColumn(tr("Description"),   -1,          Qt::AlignLeft   );
  _availability->addColumn(tr("LT"),            _whsColumn,  Qt::AlignCenter );
  _availability->addColumn(tr("QOH"),           _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Allocated"),     _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("On Order"),      _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Reorder Lvl."),  _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Available"),     _qtyColumn,  Qt::AlignRight  );
}

dspSubstituteAvailabilityByItem::~dspSubstituteAvailabilityByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSubstituteAvailabilityByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSubstituteAvailabilityByItem::set(const ParameterList &pParams)
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
  {
    _warehouse->setId(param.toInt());
    _warehouse->setEnabled(FALSE);
  }

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(TRUE);
    _warehouse->setEnabled(FALSE);
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

void dspSubstituteAvailabilityByItem::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  
  _warehouse->appendValue(params);

  if (_leadTime->isChecked())
    params.append("byLeadTime");
  else if (_byDays->isChecked())
    params.append("byDays", _days->value());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  orReport report("SubstituteAvailabilityByRootItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSubstituteAvailabilityByItem::sViewAllocations()
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

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSubstituteAvailabilityByItem::sViewOrders()
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

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSubstituteAvailabilityByItem::sPopulateMenu(QMenu *menu)
{
  menu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
  menu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
}

void dspSubstituteAvailabilityByItem::sFillList()
{
  if ((_leadTime->isChecked()) || (_byDays->isChecked()) ||  ((_byDate->isChecked()) && (_date->isValid())))
  {
    _availability->clear();

    QString sql = "SELECT sub.itemsite_id AS s_itemsite_id,"
                  " warehous_code, item_number,"
                  " (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,";

    if (_normalize->isChecked())
      sql += " formatQty(sub.itemsite_qtyonhand * itemsub_uomratio) AS qtyonhand,"
             " formatQty(CASE WHEN(sub.itemsite_useparams) THEN sub.itemsite_reorderlevel ELSE 0.0 END * itemsub_uomratio) AS reorderlevel,"
             " sub.itemsite_leadtime AS leadtime, itemsub_rank,";
    else
      sql += " formatQty(sub.itemsite_qtyonhand) AS qtyonhand,"
             " formatQty(CASE WHEN(sub.itemsite_useparams) THEN sub.itemsite_reorderlevel ELSE 0.0 END) AS reorderlevel,"
             " sub.itemsite_leadtime AS leadtime, itemsub_rank,";

    if (_leadTime->isChecked())
    {
      if (_normalize->isChecked())
        sql += " formatQty(qtyAllocated(sub.itemsite_id, sub.itemsite_leadtime) * itemsub_uomratio) AS allocated,"
               " formatQty(qtyOrdered(sub.itemsite_id, sub.itemsite_leadtime) * itemsub_uomratio) AS ordered, "
               " formatQty((sub.itemsite_qtyonhand * itemsub_uomratio) + (qtyOrdered(sub.itemsite_id, sub.itemsite_leadtime) * itemsub_uomratio) - (qtyAllocated(sub.itemsite_id, sub.itemsite_leadtime) * itemsub_uomratio)) AS available ";
      else
        sql += " formatQty(qtyAllocated(sub.itemsite_id, sub.itemsite_leadtime)) AS allocated,"
               " formatQty(qtyOrdered(sub.itemsite_id, sub.itemsite_leadtime)) AS ordered, "
               " formatQty(sub.itemsite_qtyonhand + qtyOrdered(sub.itemsite_id, sub.itemsite_leadtime) - qtyAllocated(sub.itemsite_id, sub.itemsite_leadtime)) AS available ";
    }
    else if (_byDays->isChecked())
    {
      if (_normalize->isChecked())
        sql += " formatQty(qtyAllocated(sub.itemsite_id, :days) * itemsub_uomratio) AS allocated,"
               " formatQty(qtyOrdered(sub.itemsite_id, :days) * itemsub_uomratio) AS ordered, "
               " formatQty((sub.itemsite_qtyonhand * itemsub_uomratio) + (qtyOrdered(sub.itemsite_id, :days) * itemsub_uomratio) - (qtyAllocated(sub.itemsite_id, :days) * itemsub_uomratio)) AS available ";
      else
        sql += " formatQty(qtyAllocated(sub.itemsite_id, :days)) AS allocated,"
               " formatQty(qtyOrdered(sub.itemsite_id, :days)) AS ordered, "
               " formatQty(sub.itemsite_qtyonhand + qtyOrdered(sub.itemsite_id, :days) - qtyAllocated(sub.itemsite_id, :days)) AS available ";
    }
    else if (_byDate->isChecked())
    {
      if (_normalize->isChecked())
        sql += " formatQty(qtyAllocated(sub.itemsite_id, (:date - CURRENT_DATE)) * itemsub_uomratio) AS allocated,"
               " formatQty(qtyOrdered(sub.itemsite_id, (:date - CURRENT_DATE)) * itemsub_uomratio) AS ordered, "
               " formatQty((sub.itemsite_qtyonhand * itemsub_uomratio) + (qtyOrdered(sub.itemsite_id, (:date - CURRENT_DATE)) * itemsub_uomratio) - (qtyAllocated(sub.itemsite_id, (:date - CURRENT_DATE)) * itemsub_uomratio)) AS available ";
      else
        sql += " formatQty(qtyAllocated(sub.itemsite_id, (:date - CURRENT_DATE))) AS allocated,"
               " formatQty(qtyOrdered(sub.itemsite_id, (:date - CURRENT_DATE))) AS ordered, "
               " formatQty(sub.itemsite_qtyonhand + qtyOrdered(sub.itemsite_id, (:date - CURRENT_DATE)) - qtyAllocated(sub.itemsite_id, (:date - CURRENT_DATE))) AS available ";
    }

    sql += "FROM item, itemsite AS sub, itemsite AS root, warehous, itemsub "
           "WHERE ( (sub.itemsite_item_id=item_id)"
           " AND (root.itemsite_item_id=itemsub_parent_item_id)"
           " AND (sub.itemsite_item_id=itemsub_sub_item_id)"
           " AND (root.itemsite_warehous_id=sub.itemsite_warehous_id)"
           " AND (sub.itemsite_warehous_id=warehous_id)"
           " AND (root.itemsite_item_id=:item_id)";

    if (_warehouse->isSelected())
      sql += " AND (root.itemsite_warehous_id=:warehous_id)";

    sql += ") "
           "ORDER BY itemsub_rank";

    q.prepare(sql);
    q.bindValue(":days", _days->value());
    q.bindValue(":date", _date->date());
    q.bindValue(":item_id", _item->id());
    _warehouse->bindValue(q);
    q.exec();
    XTreeWidgetItem *last = 0;
    while (q.next())
    {
      last = new XTreeWidgetItem(_availability, last,
				 q.value("s_itemsite_id").toInt(),
				 q.value("warehous_code"),
				 q.value("item_number"),
				 q.value("itemdescrip"),
				 q.value("leadtime"),
				 q.value("qtyonhand"),
				 q.value("allocated"),
				 q.value("ordered"),
				 q.value("reorderlevel"),
				 q.value("available") );

      if (last->text(7).toDouble() >= last->text(8).toDouble())
        last->setTextColor(8, "red");
    }
  }
}
