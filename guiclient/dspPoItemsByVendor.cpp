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

#include "dspPoItemsByVendor.h"

#include <QMenu>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>
#include <QMessageBox>

#include <metasql.h>
#include <openreports.h>

#include "purchaseOrder.h"
#include "purchaseOrderItem.h"
#include "reschedulePoitem.h"
#include "changePoitemQty.h"
#include "dspRunningAvailability.h"

#define POITEM_STATUS_COL 10

dspPoItemsByVendor::dspPoItemsByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_poitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_vendor, SIGNAL(newId(int)), this, SLOT(sPopulatePo()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _agent->setText(omfgThis->username());

  _poitem->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight  );
  _poitem->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter );
  _poitem->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter );
  _poitem->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter );
  _poitem->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _poitem->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _poitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter );
  _poitem->addColumn(tr("Ordered"),     _qtyColumn,   Qt::AlignRight  );
  _poitem->addColumn(tr("Received"),    _qtyColumn,   Qt::AlignRight  );
  _poitem->addColumn(tr("Returned"),    _qtyColumn,   Qt::AlignRight  );
  _poitem->addColumn("poitem_status",   10,           Qt::AlignCenter );

  _vendor->setFocus();
}

dspPoItemsByVendor::~dspPoItemsByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPoItemsByVendor::languageChange()
{
  retranslateUi(this);
}

void dspPoItemsByVendor::setParams(ParameterList & params)
{
  params.append("vend_id", _vendor->id());

  _warehouse->appendValue(params);

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  if (_selectedPO->isChecked())
    params.append("poNumber", _poNumber->currentText());

  if (_closedItems->isChecked())
    params.append("closedItems");
  else if (_openItems->isChecked())
    params.append("openItems");

  params.append("nonInv",	tr("NonInv - "));
  params.append("closed",	tr("Closed"));
  params.append("unposted",	tr("Unposted"));
  params.append("partial",	tr("Partial"));
  params.append("received",	tr("Received"));
  params.append("open",		tr("Open"));
}

void dspPoItemsByVendor::sPrint()
{
  if (!_vendor->isValid())
  {
    QMessageBox::warning( this, tr("Enter Vendor Number"),
                          tr( "Please enter a valid Vendor Number." ) );
    _vendor->setFocus();
    return;
  }

  ParameterList params;
  setParams(params);

  orReport report("POLineItemsByVendor", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);

}

void dspPoItemsByVendor::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (pSelected->text(POITEM_STATUS_COL) == "U")
  {
    menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);
  if ((!_privileges->check("MaintainPurchaseOrders")) && (!_privileges->check("ViewPurchaseOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sRunningAvailability()), 0);

  if (!_privileges->check("ViewInventoryAvailability"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  if (pSelected->text(POITEM_STATUS_COL) == "U")
  {
    menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("View Item..."), this, SLOT(sViewItem()), 0);
  if ((!_privileges->check("MaintainPurchaseOrders")) && (!_privileges->check("ViewPurchaseOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);

  if (pSelected->text(POITEM_STATUS_COL) != "C")
  {
    menuItem = pMenu->insertItem(tr("Reschedule..."), this, SLOT(sReschedule()), 0);
    if (!_privileges->check("ReschedulePurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Change Qty..."), this, SLOT(sChangeQty()), 0);
    if (!_privileges->check("ChangePurchaseOrderQty"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();
  }

  if (pSelected->text(POITEM_STATUS_COL) == "O")
  {
    menuItem = pMenu->insertItem(tr("Close Item..."), this, SLOT(sCloseItem()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (pSelected->text(POITEM_STATUS_COL) == "C")
  {
    menuItem = pMenu->insertItem(tr("Open Item..."), this, SLOT(sOpenItem()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspPoItemsByVendor::sRunningAvailability()
{
  q.prepare("SELECT poitem_itemsite_id "
            "FROM poitem "
            "WHERE (poitem_id=:poitemid); ");
  q.bindValue(":poitemid", _poitem->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("poitem_itemsite_id").toInt());
    params.append("run");

    dspRunningAvailability *newdlg = new dspRunningAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspPoItemsByVendor::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPoItemsByVendor::sViewOrder()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPoItemsByVendor::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("poitem_id", _poitem->altId());

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspPoItemsByVendor::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("poitem_id", _poitem->altId());

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspPoItemsByVendor::sReschedule()
{
  ParameterList params;
  params.append("poitem_id", _poitem->altId());

  reschedulePoitem newdlg(this, "", TRUE);
  if(newdlg.set(params) != UndefinedError)
    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
}

void dspPoItemsByVendor::sChangeQty()
{
  ParameterList params;
  params.append("poitem_id", _poitem->altId());

  changePoitemQty newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPoItemsByVendor::sCloseItem()
{
  q.prepare( "UPDATE poitem "
             "SET poitem_status='C' "
             "WHERE (poitem_id=:poitem_id);" );
  q.bindValue(":poitem_id", _poitem->altId());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspPoItemsByVendor::sOpenItem()
{
  q.prepare( "UPDATE poitem "
             "SET poitem_status='O' "
             "WHERE (poitem_id=:poitem_id);" );
  q.bindValue(":poitem_id", _poitem->altId());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspPoItemsByVendor::sFillList()
{
  _poitem->clear();

  QString sql( "SELECT pohead_id, poitem_id, pohead_number,"
	       "       poitem_status,"
               "       CASE WHEN(poitem_status='C') THEN <? value(\"closed\") ?>"
               "            WHEN(poitem_status='U') THEN <? value(\"unposted\") ?>"
               "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered>(poitem_qty_received-poitem_qty_returned))) THEN <? value(\"partial\") ?>"
               "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered=(poitem_qty_received-poitem_qty_returned))) THEN <? value(\"received\") ?>"
               "            WHEN(poitem_status='O') THEN <? value(\"open\") ?>"
               "            ELSE poitem_status"
               "       END AS poitemstatus,"
               "       CASE WHEN (itemsite_id IS NULL) THEN ( SELECT warehous_code"
               "                                              FROM warehous"
               "                                              WHERE (pohead_warehous_id=warehous_id) )"
               "            ELSE ( SELECT warehous_code"
               "                   FROM warehous"
               "                   WHERE (itemsite_warehous_id=warehous_id) )"
               "       END AS warehousecode,"
               "       COALESCE(item_number, (<? value(\"nonInv\") ?> || poitem_vend_item_number)) AS itemnumber,"
               "       COALESCE(item_descrip1, firstLine(poitem_vend_item_descrip)) AS itemdescrip,"
               "       COALESCE(uom_name, poitem_vend_uom) AS itemuom,"
               "       formatDate(poitem_duedate) AS f_duedate,"
               "       formatQty(poitem_qty_ordered) AS f_qtyordered,"
               "       formatQty(poitem_qty_received) AS f_qtyreceived,"
               "       formatQty(poitem_qty_returned) AS f_qtyreturned,"
               "       (poitem_duedate < CURRENT_DATE) AS late "
               "FROM pohead,"
               "     poitem LEFT OUTER JOIN"
               "     ( itemsite JOIN item"
               "       ON (itemsite_item_id=item_id) JOIN uom ON (item_inv_uom_id=uom_id))"
               "     ON (poitem_itemsite_id=itemsite_id) "
               "WHERE ((poitem_pohead_id=pohead_id)"
	       "<? if exists(\"warehous_id\") ?>"
	       " AND (((itemsite_id IS NULL) AND"
	       "       (pohead_warehous_id=<? value(\"warehous_id\") ?>) ) OR"
	       "      ((itemsite_id IS NOT NULL) AND"
	       "       (itemsite_warehous_id=<? value(\"warehous_id\") ?>) ) )"
	       "<? endif ?>"
	       "<? if exists(\"agentUsername\") ?>"
	       " AND (pohead_agent_username=<? value(\"agentUsername\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"poNumber\") ?>"
	       " AND (pohead_number=<? value(\"poNumber\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"openItems\") ?>"
	       " AND (poitem_status='O')"
	       "<? endif ?>"
	       "<? if exists(\"closedItems\") ?>"
	       " AND (poitem_status='C')"
	       "<? endif ?>"
	       " AND (pohead_vend_id=<? value(\"vend_id\") ?>) ) "
	       "ORDER BY poitem_duedate, pohead_number, poitem_linenumber;" );
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    XTreeWidgetItem *last = 0;
    do
    {
      last = new XTreeWidgetItem(_poitem, last,
				 q.value("pohead_id").toInt(),
				 q.value("poitem_id").toInt(),
				 q.value("pohead_number"),
				 q.value("warehousecode"),
				 q.value("poitemstatus"),
				 q.value("f_duedate"),
				 q.value("itemnumber"),
				 q.value("itemdescrip"),
				 q.value("itemuom"),
				 q.value("f_qtyordered"),
				 q.value("f_qtyreceived"),
				 q.value("f_qtyreturned") );
      last->setText(POITEM_STATUS_COL, q.value("poitem_status"));
      if (q.value("late").toBool())
        last->setTextColor(3, "red");
    }
    while (q.next());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspPoItemsByVendor::sPopulatePo()
{
  _poNumber->clear();

  if ( _vendor->isValid()) 
  {
    q.prepare( "SELECT pohead_id, pohead_number "
               "FROM pohead "
               "WHERE ( (pohead_vend_id=:vend_id) ) "
               "ORDER BY pohead_number;" );
    q.bindValue(":vend_id", _vendor->id());
    q.exec();
    _poNumber->populate(q);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
