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

#include "dspOrders.h"

#include <QMenu>
#include <QVariant>

#include <metasql.h>

#include "changePoitemQty.h"
#include "changeWoQty.h"
#include "mqlutil.h"
#include "printWoTraveler.h"
#include "reprioritizeWo.h"
#include "reschedulePoitem.h"
#include "rescheduleWo.h"

dspOrders::dspOrders(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_orders, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _item->setReadOnly(TRUE);
  _warehouse->setEnabled(FALSE);

  _orders->addColumn(tr("Type"),         _docTypeColumn, Qt::AlignCenter );
  _orders->addColumn(tr("Order #"),      -1,             Qt::AlignLeft   );
  _orders->addColumn(tr("Total"),        _qtyColumn,     Qt::AlignRight  );
  _orders->addColumn(tr("Received"),     _qtyColumn,     Qt::AlignRight  );
  _orders->addColumn(tr("Balance"),      _qtyColumn,     Qt::AlignRight  );
  _orders->addColumn(tr("Running Bal."), _qtyColumn,     Qt::AlignRight  );
  _orders->addColumn(tr("Required"),     _dateColumn,    Qt::AlignCenter );

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

dspOrders::~dspOrders()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspOrders::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspOrders::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    _item->setItemsiteid(param.toInt());

  _leadTime->setChecked(pParams.inList("byLeadTime"));

  param = pParams.value("byDate", &valid);
  if (valid)
  {
    _byDate->setChecked(TRUE);
    _date->setDate(param.toDate());
  }

  param = pParams.value("byDays", &valid);
  if (valid)
  {
    _byDays->setChecked(TRUE);
    _days->setValue(param.toInt());
  }

  _byRange->setChecked(pParams.inList("byRange"));

  param = pParams.value("startDate", &valid);
  if (valid)
    _startDate->setDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _endDate->setDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspOrders::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if (_orders->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Reschedule P/O Item..."), this, SLOT(sReschedulePoitem()), 0);
    if (!_privileges->check("ReschedulePurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Change P/O Item Quantity..."), this, SLOT(sChangePoitemQty()), 0);
    if (!_privileges->check("ChangePurchaseOrderQty"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (_orders->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Reprioritize W/O..."), this, SLOT(sReprioritizeWo()), 0);
    if (!_privileges->check("ReprioritizeWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Reschedule W/O..."), this, SLOT(sRescheduleWO()), 0);
    if (!_privileges->check("RescheduleWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Change W/O Quantity..."), this, SLOT(sChangeWOQty()), 0);
    if (!_privileges->check("ChangeWorkOrderQty"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Print Traveler..."), this, SLOT(sPrintTraveler()), 0);
    if (!_privileges->check("PrintWorkOrderPaperWork"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspOrders::sReprioritizeWo()
{
  ParameterList params;
  params.append("wo_id", _orders->id());

  reprioritizeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrders::sRescheduleWO()
{
  ParameterList params;
  params.append("wo_id", _orders->id());

  rescheduleWo newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrders::sChangeWOQty()
{
  ParameterList params;
  params.append("wo_id", _orders->id());

  changeWoQty newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrders::sPrintTraveler()
{
  ParameterList params;
  params.append("wo_id", _orders->id());

  printWoTraveler newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspOrders::sReschedulePoitem()
{
  ParameterList params;
  params.append("poitem_id", _orders->id());

  reschedulePoitem newdlg(this, "", TRUE);
  if(newdlg.set(params) != UndefinedError)
    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
}

void dspOrders::sChangePoitemQty()
{
  ParameterList params;
  params.append("poitem_id", _orders->id());

  changePoitemQty newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrders::sFillList()
{
  _orders->clear();

  if ( (_item->isValid()) &&
       ( (_leadTime->isChecked()) || (_byDays->isChecked()) ||
         ((_byDate->isChecked()) && (_date->isValid())) ||
         (_byRange->isChecked() && _startDate->isValid() && _endDate->isValid()) ) )
  {
    MetaSQLQuery mql = mqlLoad(":/im/displays/Orders/FillListDetail.mql");
    ParameterList params;
    params.append("warehous_id", _warehouse->id());
    params.append("item_id",     _item->id());
    params.append("itemType",    _item->itemType());
    if (_leadTime->isChecked())
      params.append("useLeadTime");
    else if (_byDays->isChecked())
      params.append("days",      _days->value());
    else if (_byDate->isChecked())
      params.append("date",      _date->date());
    else if (_byRange->isChecked())
    {
      params.append("startDate", _startDate->date());
      params.append("endDate",   _endDate->date());
    }

    q = mql.toQuery(params);
    double          runningBal = 0;
    XTreeWidgetItem *last = 0;
    while (q.next())
    {
      runningBal += q.value("balanceqty").toDouble();

      last = new XTreeWidgetItem(_orders, last,
				 q.value("source_id").toInt(),
				 q.value("type").toInt(),
				 q.value("order_type"), q.value("order_number"),
				 q.value("totalqty"), q.value("relievedqty"),
				 q.value("balanceqty"), formatQty(runningBal),
				 q.value("duedate") );
      if (q.value("late").toBool())
	last->setTextColor(6, "red");
    }
  }
}
