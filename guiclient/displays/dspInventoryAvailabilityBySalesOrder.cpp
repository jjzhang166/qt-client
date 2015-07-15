/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryAvailabilityBySalesOrder.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "createCountTagsByItem.h"
#include "dspAllocations.h"
#include "dspOrders.h"
#include "dspReservations.h"
#include "dspRunningAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "inputManager.h"
#include "purchaseOrder.h"
#include "reserveSalesOrderItem.h"
#include "storedProcErrorLookup.h"
#include "workOrder.h"

dspInventoryAvailabilityBySalesOrder::dspInventoryAvailabilityBySalesOrder(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspInventoryAvailabilityBySalesOrder", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Inventory Availability by Sales Order"));
  setListLabel(tr("Availability"));
  setReportName("InventoryAvailabilityBySalesOrder");
  setMetaSQLOptions("inventoryAvailability", "byCustOrSO");
  setUseAltId(true);
  setAutoUpdateEnabled(true);

  _so->setAllowedTypes(OrderLineEdit::Sales);
  _so->setAllowedStatuses(OrderLineEdit::Open);

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _so, SLOT(setId(int)));

  list()->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"),           -1, Qt::AlignLeft,  true, "descrip");
  list()->addColumn(tr("UOM"),           _uomColumn, Qt::AlignCenter,true, "uom_name");
  list()->addColumn(tr("Available QOH"), _qtyColumn, Qt::AlignRight, true, "qoh");
  list()->addColumn(tr("This Alloc."),   _qtyColumn, Qt::AlignRight, true, "sobalance");
  list()->addColumn(tr("Total Alloc."),  _qtyColumn, Qt::AlignRight, true, "allocated");
  list()->addColumn(tr("Orders"),        _qtyColumn, Qt::AlignRight, true, "ordered");
  list()->addColumn(tr("This Avail."),   _qtyColumn, Qt::AlignRight, true, "orderavail");
  list()->addColumn(tr("Total Avail."),  _qtyColumn, Qt::AlignRight, true, "totalavail");
  list()->addColumn(tr("At Shipping"),   _qtyColumn, Qt::AlignRight, true, "atshipping");
  list()->addColumn(tr("Start Date"),   _dateColumn, Qt::AlignCenter,true, "orderdate");
  list()->addColumn(tr("Sched. Date"),  _dateColumn, Qt::AlignCenter,true, "duedate");
  list()->setIndentation(10);

  if(!_metrics->boolean("EnableSOReservations"))
  {
    _useReservationNetting->hide();
    _useReservationNetting->setEnabled(false);
  }
  else
  {
    connect(_useReservationNetting, SIGNAL(toggled(bool)), this, SLOT(sHandleReservationNetting(bool)));
    if(_useReservationNetting->isChecked())
      sHandleReservationNetting(true);
  }
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

void dspInventoryAvailabilityBySalesOrder::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityBySalesOrder::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("onlyShowShortages", &valid);
  if (valid)
    _onlyShowShortages->setChecked(true);

  param = pParams.value("sohead_id", &valid);
  if (valid)
    _so->setId(param.toInt(),"SO");

  return NoError;
}

bool dspInventoryAvailabilityBySalesOrder::setParams(ParameterList &params)
{
  if(!_so->isValid())
  {
    QMessageBox::warning(this, tr("No Sales Order Selected"),
      tr("You must select a valid Sales Order.") );
    return false;
  }

  params.append("sohead_id", _so->id());

  if(_onlyShowShortages->isChecked())
    params.append("onlyShowShortages");
  if (_showWoSupply->isChecked())
    params.append("showWoSupply");
  if (_useReservationNetting->isChecked())
    params.append("useReservationNetting");

  return true;
}

void dspInventoryAvailabilityBySalesOrder::sPopulateMenu(QMenu *pMenu,  QTreeWidgetItem *selected, int)
{
  XSqlQuery dspPopulateMenu;
  XTreeWidgetItem * item = (XTreeWidgetItem*)selected;
  QAction *menuItem;
  
  if (list()->altId() != -1)
  {
    menuItem = pMenu->addAction("View Allocations...", this, SLOT(sViewAllocations()));
    if (item->rawValue("allocated").toDouble() == 0.0)
      menuItem->setEnabled(false);
    
    menuItem = pMenu->addAction("View Orders...", this, SLOT(sViewOrders()));
    if (item->rawValue("ordered").toDouble() == 0.0)
     menuItem->setEnabled(false);

    menuItem = pMenu->addAction("Running Availability...", this, SLOT(sRunningAvailability()));
    menuItem = pMenu->addAction("Substitute Availability...", this, SLOT(sViewSubstituteAvailability()));

    dspPopulateMenu.prepare ("SELECT item_type "
             "FROM itemsite,item "
             "WHERE ((itemsite_id=:itemsite_id)"
             "AND (itemsite_item_id=item_id)"
             "AND (itemsite_wosupply));");
    dspPopulateMenu.bindValue(":itemsite_id", list()->id());
    dspPopulateMenu.exec();
    if (dspPopulateMenu.next())
    {
      if (dspPopulateMenu.value("item_type") == "P")
      {
        pMenu->addSeparator();
        menuItem = pMenu->addAction("Issue Purchase Order...", this, SLOT(sIssuePO()));
        if (!_privileges->check("MaintainPurchaseOrders"))
          menuItem->setEnabled(false);
      }
      else if (dspPopulateMenu.value("item_type") == "M")
      {
        pMenu->addSeparator();
        menuItem = pMenu->addAction("Issue Work Order...", this, SLOT(sIssueWO()));
        if (!_privileges->check("MaintainWorkOrders"))
          menuItem->setEnabled(false);
      }
    }

    if(_metrics->boolean("EnableSOReservations"))
    {
      pMenu->addSeparator();

      pMenu->addAction(tr("Show Reservations..."), this, SLOT(sShowReservations()));
      pMenu->addSeparator();

      menuItem = pMenu->addAction(tr("Unreserve Stock"), this, SLOT(sUnreserveStock()));
      menuItem->setEnabled(_privileges->check("MaintainReservations"));
      menuItem = pMenu->addAction(tr("Reserve Stock..."), this, SLOT(sReserveStock()));
      menuItem->setEnabled(_privileges->check("MaintainReservations"));
      menuItem = pMenu->addAction(tr("Reserve Line Balance"), this, SLOT(sReserveLineBalance()));
      menuItem->setEnabled(_privileges->check("MaintainReservations"));
    }

    pMenu->addSeparator();
    menuItem = pMenu->addAction("Issue Count Tag...", this, SLOT(sIssueCountTag()));
    if (!_privileges->check("IssueCountTags"))
      menuItem->setEnabled(false);
  }
}

void dspInventoryAvailabilityBySalesOrder::sViewAllocations()
{
  XSqlQuery dspViewAllocations;
  dspViewAllocations.prepare( "SELECT coitem_scheddate "
             "FROM coitem "
             "WHERE (coitem_id=:soitem_id);" );
  dspViewAllocations.bindValue(":soitem_id", list()->altId());
  dspViewAllocations.exec();
  if (dspViewAllocations.first())
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
    params.append("byDate", dspViewAllocations.value("coitem_scheddate"));
    params.append("run");

    dspAllocations *newdlg = new dspAllocations();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspInventoryAvailabilityBySalesOrder::sViewOrders()
{
  XSqlQuery dspViewOrders;
  dspViewOrders.prepare( "SELECT coitem_scheddate "
             "FROM coitem "
             "WHERE (coitem_id=:soitem_id);" );
  dspViewOrders.bindValue(":soitem_id", list()->altId());
  dspViewOrders.exec();
  if (dspViewOrders.first())
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
    params.append("byDate", dspViewOrders.value("coitem_scheddate"));
    params.append("run");

    dspOrders *newdlg = new dspOrders();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspInventoryAvailabilityBySalesOrder::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySalesOrder::sViewSubstituteAvailability()
{
  XSqlQuery dspViewSubstituteAvailability;
  dspViewSubstituteAvailability.prepare( "SELECT coitem_scheddate "
             "FROM coitem "
             "WHERE (coitem_id=:soitem_id);" );
  dspViewSubstituteAvailability.bindValue(":soitem_id", list()->altId());
  dspViewSubstituteAvailability.exec();
  if (dspViewSubstituteAvailability.first())
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
    params.append("byDate", dspViewSubstituteAvailability.value("coitem_scheddate"));
    params.append("run");

    dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
//  ToDo
}

void dspInventoryAvailabilityBySalesOrder::sIssuePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySalesOrder::sIssueWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySalesOrder::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityBySalesOrder::sFillList()
{
  XSqlQuery qq;
  qq.prepare("SELECT cohead_number,"
             "       cohead_orderdate,"
             "       cohead_custponumber,"
             "       cust_name, cntct_phone "
             "  FROM cohead"
             "  JOIN custinfo ON (cohead_cust_id=cust_id)"
             "  LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id)"
             " WHERE (cohead_id=:sohead_id);" );
  qq.bindValue(":sohead_id", _so->id());
  qq.exec();
  if (qq.first())
  {
    _orderDate->setDate(qq.value("cohead_orderdate").toDate());
    _poNumber->setText(qq.value("cohead_custponumber").toString());
    _custName->setText(qq.value("cust_name").toString());
    _custPhone->setText(qq.value("cntct_phone").toString());
  }
               
  display::sFillList();
  list()->expandAll();
}

void dspInventoryAvailabilityBySalesOrder::sHandleReservationNetting(bool yn)
{
  if(yn)
    list()->headerItem()->setText(7, tr("This Reserve"));
  else
    list()->headerItem()->setText(7, tr("This Avail."));

  if(_so->isValid())
    sFillList();
}

void dspInventoryAvailabilityBySalesOrder::sReserveStock()
{
  ParameterList params;
  params.append("soitem_id", list()->altId());

  reserveSalesOrderItem newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void dspInventoryAvailabilityBySalesOrder::sReserveLineBalance()
{
  XSqlQuery dspReserveLineBalance;
  dspReserveLineBalance.prepare("SELECT reserveSoLineBalance(:soitem_id) AS result;");
  dspReserveLineBalance.bindValue(":soitem_id", list()->altId());
  dspReserveLineBalance.exec();
  if (dspReserveLineBalance.first())
  {
    int result = dspReserveLineBalance.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("reserveSoLineBalance", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (dspReserveLineBalance.lastError().type() != QSqlError::NoError)
  {
    systemError(this, tr("Error\n") +
                      dspReserveLineBalance.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void dspInventoryAvailabilityBySalesOrder::sUnreserveStock()
{
  XSqlQuery dspUnreserveStock;
  dspUnreserveStock.prepare("UPDATE coitem SET coitem_qtyreserved=0 WHERE coitem_id=:soitem_id;");
  dspUnreserveStock.bindValue(":soitem_id", list()->altId());
  dspUnreserveStock.exec();
  if (dspUnreserveStock.lastError().type() != QSqlError::NoError)
  {
    systemError(this, tr("Error\n") +
                      dspUnreserveStock.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void dspInventoryAvailabilityBySalesOrder::sShowReservations()
{
  ParameterList params;
  params.append("soitem_id", list()->altId());
  params.append("run");

  dspReservations * newdlg = new dspReservations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

