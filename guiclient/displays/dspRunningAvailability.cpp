/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspRunningAvailability.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "dspWoSchedule.h"
#include "firmPlannedOrder.h"
#include "purchaseRequest.h"
#include "salesOrder.h"
#include "transferOrder.h"
#include "workOrder.h"
#include "purchaseOrder.h"
#include "errorReporter.h"

dspRunningAvailability::dspRunningAvailability(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspRunningAvailability", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Running Availability"));
  setListLabel(tr("Running Availability"));
  setReportName("RunningAvailability");
  setMetaSQLOptions("runningAvailability", "detail");
  setUseAltId(true);

  _ready = true;

  connect(list(),       SIGNAL(populated()), this, SLOT(sHandleResort()));
  connect(list(),       SIGNAL(resorted()), this, SLOT(sHandleResort()));

  list()->addColumn(tr("Order Type"),             _itemColumn,  Qt::AlignLeft,  true, "ordertype");
  list()->addColumn(tr("Order #"),                _itemColumn,  Qt::AlignLeft,  true, "ordernumber");
  list()->addColumn(tr("Source/Destination"),     -1,           Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Due Date"),               _dateColumn,  Qt::AlignLeft,  true, "duedate");
  list()->addColumn(tr("Amount"),                 _moneyColumn, Qt::AlignRight, true, "amount");
  list()->addColumn(tr("Ordered"),                _qtyColumn,   Qt::AlignRight, true, "qtyordered");
  list()->addColumn(tr("Received"),               _qtyColumn,   Qt::AlignRight, true, "qtyreceived");
  if (_metrics->boolean("EnableSOReservations"))
    list()->addColumn(tr("Reserved"),             _qtyColumn,   Qt::AlignRight, true, "reserved");
  list()->addColumn(tr("Balance"),                _qtyColumn,   Qt::AlignRight, true, "balance");
  list()->addColumn(tr("Running Avail."),         _qtyColumn,   Qt::AlignRight, true, "runningavail");
  list()->addColumn(tr("Running Netable"),        _qtyColumn,   Qt::AlignRight, true, "runningnetable");
  list()->addColumn(tr("Notes"),                  _itemColumn,  Qt::AlignLeft,  true, "notes");

  _qoh->setValidator(omfgThis->qtyVal());
  _netableqoh->setValidator(omfgThis->qtyVal());
  _reorderLevel->setValidator(omfgThis->qtyVal());
  _orderMultiple->setValidator(omfgThis->qtyVal());
  _orderToQty->setValidator(omfgThis->qtyVal());

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

void dspRunningAvailability::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspRunningAvailability::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _ready = false;
    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(true);
    _showPlanned->setFocus();
    _showPlanned->init();
    _ready = true;
    sFillList();
  }

  return NoError;
}

bool dspRunningAvailability::setParams(ParameterList & params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Item Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  params.append("item_id",	_item->id());
  params.append("warehous_id",	_warehouse->id());

  params.append("firmPo",	tr("Planned P/O (firmed)"));
  params.append("plannedPo",	tr("Planned P/O"));
  params.append("firmWo",	tr("Planned W/O (firmed)"));
  params.append("plannedWo",	tr("Planned W/O"));
  params.append("firmTo",	tr("Planned T/O (firmed)"));
  params.append("plannedTo",	tr("Planned T/O"));
  params.append("firmWoReq",	tr("Planned W/O Req. (firmed)"));
  params.append("plannedWoReq",	tr("Planned W/O Req."));
  params.append("pr",		tr("Purchase Request"));

  if (_showPlanned->isChecked())
    params.append("showPlanned");

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (_metrics->boolean("EnableSOReservations"))
    params.append("showReserved");
  
  if (_metrics->value("Application") == "Standard")
  {
    XSqlQuery xtmfg;
    xtmfg.exec("SELECT pkghead_name FROM pkghead WHERE pkghead_name='xtmfg'");
    if (xtmfg.first())
      params.append("Manufacturing");
    params.append("showMRPplan");
  }

  params.append("qoh", _qoh->toDouble()); // metasql only?
  params.append("netableqoh", _netableqoh->toDouble()); // metasql only?

  return true;
}

void dspRunningAvailability::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;
  
  QString ordertype = pSelected->text(list()->column("ordertype"));

  if (ordertype == tr("Planned W/O (firmed)") ||
      ordertype == tr("Planned W/O") ||
      ordertype == tr("Planned P/O (firmed)") ||
      ordertype == tr("Planned P/O") )
  {
    if (ordertype == tr("Planned W/O (firmed)") || ordertype == tr("Planned P/O (firmed)") )
      pMenu->addAction(tr("Soften Order..."), this, SLOT(sSoftenOrder()));
    else
      pMenu->addAction(tr("Firm Order..."), this, SLOT(sFirmOrder()));
 
    pMenu->addAction(tr("Release Order..."), this, SLOT(sReleaseOrder()));
    pMenu->addAction(tr("Delete Order..."), this, SLOT(sDeleteOrder()));
  }
  
  else if (ordertype.contains("W/O") &&
	  !(ordertype == tr("Planned W/O Req. (firmed)") || ordertype == tr("Planned W/O Req.")))
  {
    pMenu->addAction(tr("View Work Order Details..."), this, SLOT(sViewWo()));
    menuItem = pMenu->addAction(tr("Work Order Schedule by Item..."), this, SLOT(sDspWoScheduleByWorkOrder()));
    menuItem->setEnabled( _privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders"));
  }
  else if (ordertype == "S/O")
  {
    menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sViewSo()));
    menuItem->setEnabled(_privileges->check("ViewSalesOrders"));
    menuItem = pMenu->addAction(tr("Edit Sales Order..."), this, SLOT(sEditSo()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));
  }

  else if (ordertype == "T/O")
  {
    menuItem = pMenu->addAction(tr("View Transfer Order..."), this, SLOT(sViewTo()));
    menuItem->setEnabled(_privileges->check("ViewTransferOrders"));
    menuItem = pMenu->addAction(tr("Edit Transfer Order..."), this, SLOT(sEditTo()));
    menuItem->setEnabled(_privileges->check("MaintainTransferOrders"));
  }

  else if (ordertype == "P/O")
  {
    menuItem = pMenu->addAction(tr("View Purchase Order..."), this, SLOT(sViewPo()));
    menuItem->setEnabled(_privileges->check("ViewPurchaseOrders"));
    menuItem = pMenu->addAction(tr("Edit Purchase Order..."), this, SLOT(sEditPo()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  }

}

void dspRunningAvailability::sFirmOrder()
{
  ParameterList params;
  params.append("planord_id", list()->id());

  firmPlannedOrder newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspRunningAvailability::sSoftenOrder()
{
  XSqlQuery dspSoftenOrder;
  dspSoftenOrder.prepare( "UPDATE planord "
             "SET planord_firm=false "
             "WHERE (planord_id=:planord_id);" );
  dspSoftenOrder.bindValue(":planord_id", list()->id());
  dspSoftenOrder.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Planned Order Information"),
                                dspSoftenOrder, __FILE__, __LINE__))
  {
    return;
  }

  sFillList();
}

void dspRunningAvailability::sReleaseOrder()
{
  // TODO
  if (list()->currentItem()->text(list()->column("ordertype")) == tr("Planned W/O (firmed)") ||
      list()->currentItem()->text(list()->column("ordertype")) == tr("Planned W/O"))
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", list()->id());

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
#if 0
    if (newdlg.exec() != XDialog::Rejected)
    {
      sDeleteOrder();
      sFillList();
    }
#endif
  }
  else if (list()->currentItem()->text(list()->column("ordertype")) == tr("Planned P/O (firmed)") ||
	  list()->currentItem()->text(list()->column("ordertype")) == tr("Planned P/O"))
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", list()->id());

    purchaseRequest newdlg(this, "", true);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void dspRunningAvailability::sDeleteOrder()
{
  XSqlQuery dspDeleteOrder;
  dspDeleteOrder.prepare( "SELECT deletePlannedOrder(:planord_id, false) AS result;" );
  dspDeleteOrder.bindValue(":planord_id", list()->id());
  dspDeleteOrder.exec();
  if (dspDeleteOrder.first())
  {
    /* TODO: uncomment when deletePlannedOrder returns INTEGER instead of BOOLEAN
    int result = dspDeleteOrder.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deletePlannedOrder", result), __FILE__, __LINE__);
      return;
    }
    */
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Planned Order Information"),
                                dspDeleteOrder, __FILE__, __LINE__))
  {
    return;
  }

  sFillList();
}

void dspRunningAvailability::sViewSo()
{
  ParameterList params;
  salesOrder::viewSalesOrder(list()->id());
}

void dspRunningAvailability::sViewTo()
{
  ParameterList params;
  transferOrder::viewTransferOrder(list()->id());
}

void dspRunningAvailability::sViewWo()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspRunningAvailability::sViewPo()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspRunningAvailability::sEditSo()
{
  ParameterList params;
  salesOrder::editSalesOrder(list()->id(), true);
}

void dspRunningAvailability::sEditTo()
{
  ParameterList params;
  transferOrder::editTransferOrder(list()->id(), true);
}

void dspRunningAvailability::sEditWo()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", list()->id());
  
  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspRunningAvailability::sEditPo()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", list()->id());
  
  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspRunningAvailability::sFillList()
{
  XSqlQuery dspFillList;
  ParameterList params;
  if (setParams(params) && _ready)
  {
    dspFillList.prepare( "SELECT qtyAvailable(itemsite_id) AS availqoh,"
               "       qtyNetable(itemsite_id) AS netableqoh,"
               "       CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
               "       CASE WHEN(itemsite_useparams) THEN itemsite_ordertoqty   ELSE 0.0 END AS ordertoqty,"
               "       CASE WHEN(itemsite_useparams) THEN itemsite_multordqty   ELSE 0.0 END AS multorderqty "
               "FROM item, itemsite "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=:warehous_id)"
               " AND (item_id=:item_id) );" );
    dspFillList.bindValue(":item_id", _item->id());
    dspFillList.bindValue(":warehous_id", _warehouse->id());
    dspFillList.exec();
    if (dspFillList.first())
    {
      _qoh->setDouble(dspFillList.value("availqoh").toDouble());
      _netableqoh->setDouble(dspFillList.value("netableqoh").toDouble());
      _reorderLevel->setDouble(dspFillList.value("reorderlevel").toDouble());
      _orderMultiple->setDouble(dspFillList.value("multorderqty").toDouble());
      _orderToQty->setDouble(dspFillList.value("ordertoqty").toDouble());

      display::sFillList();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  dspFillList, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void dspRunningAvailability::sDspWoScheduleByWorkOrder()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("run");

  dspWoSchedule *newdlg = new dspWoSchedule();
  SetResponse setresp = newdlg->set(params);
  if (setresp == NoError || setresp == NoError_Run)
    omfgThis->handleNewWindow(newdlg);
}

void dspRunningAvailability::sHandleResort()
{
  for (int i = 0; i < list()->topLevelItemCount(); i++)
  {
    XTreeWidgetItem *item = list()->topLevelItem(i);
    if (item->data(list()->column("runningavail"), Qt::DisplayRole).toDouble() < 0)
      item->setTextColor(list()->column("runningavail"), namedColor("error"));
    else if (item->data(list()->column("runningavail"), Qt::DisplayRole).toDouble() < _reorderLevel->toDouble())
      item->setTextColor(list()->column("runningavail"), namedColor("warning"));
    else
      item->setTextColor(list()->column("runningavail"), namedColor(""));

    if (item->data(list()->column("runningnetable"), Qt::DisplayRole).toDouble() < 0)
      item->setTextColor(list()->column("runningnetable"), namedColor("error"));
    else if (item->data(list()->column("runningnetable"), Qt::DisplayRole).toDouble() < _reorderLevel->toDouble())
      item->setTextColor(list()->column("runningnetable"), namedColor("warning"));
    else
      item->setTextColor(list()->column("runningnetable"), namedColor(""));
  }
}
