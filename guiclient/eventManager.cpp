/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "eventManager.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QTimer>
#include <QVariant>

#include <metasql.h>

#include "changeWoQty.h"
#include "createCountTagsByItem.h"
#include "dspInventoryAvailability.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspInventoryHistory.h"
#include "mqlutil.h"
#include "printPackingList.h"
#include "printWoTraveler.h"
#include "rescheduleWo.h"
#include "salesOrder.h"
#include "salesOrderItem.h"
#include "storedProcErrorLookup.h"
#include "purchaseOrderItem.h"
#include "todoItem.h"
#include "incident.h"
#include "task.h"
#include "errorReporter.h"

eventManager::eventManager(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_ack,		SIGNAL(clicked()),     this, SLOT(sAcknowledge()));
  connect(_delete,	SIGNAL(clicked()),     this, SLOT(sDelete()));
  connect(_autoUpdate,	SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_event, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_event,	SIGNAL(valid(bool)),	this, SLOT(sHandleEventValid(bool)));
  connect(_selectedUser,	SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showAcknowledged,	SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_usr,			SIGNAL(newID(int)),    this, SLOT(sFillList()));
  connect(_warehouse,		SIGNAL(updated()),     this, SLOT(sFillList()));

  if (!_privileges->check("ViewOtherEvents"))
    _selectedUser->setEnabled(false);

  _event->addColumn("orderId",         0,               Qt::AlignCenter, true, "evntlog_ord_id");
  _event->addColumn("newDate",         0,               Qt::AlignCenter, true, "evntlog_newdate" );
  _event->addColumn("newQty",          0,               Qt::AlignCenter, true, "evntlog_newvalue" );
  _event->addColumn(tr("Site"),        _whsColumn,      Qt::AlignCenter, true, "warehous_code" );
  _event->addColumn(tr("Time"),        _timeDateColumn, Qt::AlignLeft  , true, "evntlog_evnttime" );
  _event->addColumn(tr("Acknowleged"), _timeDateColumn, Qt::AlignLeft  , true, "evntlog_dispatched" );
  _event->addColumn(tr("Event Type"),  _itemColumn,     Qt::AlignLeft  , true, "evnttype_name" );
  _event->addColumn(tr("Order #"),     -1,              Qt::AlignLeft  , true, "evntlog_number" );

  sFillList();
}

eventManager::~eventManager()
{
  // no need to delete child widgets, Qt does it all for us
}

void eventManager::languageChange()
{
  retranslateUi(this);
}

void eventManager::sHandleEventValid(bool pvalid)
{
  _ack->setEnabled(pvalid &&
      ((_currentUser->isChecked() && _privileges->check("DispatchOwnEvents")) ||
       (_selectedUser->isChecked() && _privileges->check("DispatchOtherEvents"))) );
  _delete->setEnabled(pvalid &&
      ((_currentUser->isChecked() && _privileges->check("DeleteOwnEvents")) ||
       (_selectedUser->isChecked() && _privileges->check("DeleteOtherEvents"))) );
}

void eventManager::sPopulateMenu(QMenu *menu)
{
  QAction *menuItem;

  if (_event->currentItem()->rawValue("evntlog_dispatched").toString().length() == 0)
  {
    menuItem = menu->addAction(tr("Acknowledge"), this, SLOT(sAcknowledge()));
    if ( ((_currentUser->isChecked()) && (!_privileges->check("DispatchOwnEvents"))) ||
         ((_selectedUser->isChecked()) && (!_privileges->check("DispatchOtherEvents"))) )
        menuItem->setEnabled(false);
  }

  menuItem = menu->addAction(tr("Delete"), this, SLOT(sDelete()));
  if ( ((_currentUser->isChecked()) && (!_privileges->check("DeleteOwnEvents"))) ||
       ((_selectedUser->isChecked()) && (!_privileges->check("DeleteOtherEvents"))) )
      menuItem->setEnabled(false);

  // if multiple items are selected then keep the menu short
  QList<XTreeWidgetItem*> list = _event->selectedItems();
  if (list.size() > 1)
    return;

  if ( (_event->currentItem()->rawValue("evnttype_name").toString() == "WoCreated") ||
       (_event->currentItem()->rawValue("evnttype_name").toString() == "WoDueDateChanged") ||
       (_event->currentItem()->rawValue("evnttype_name").toString() == "WoQtyChanged") )
  {
    menu->addSeparator();

    menuItem = menu->addAction(tr("Inventory Availability by Work Order..."), this, SLOT(sInventoryAvailabilityByWorkOrder()));
  }
  
  else if ( (_event->currentItem()->rawValue("evnttype_name").toString() == "POitemCreate") )
  {
    menu->addSeparator();

    menuItem = menu->addAction(tr("View Purchase Order Item..."), this, SLOT(sViewPurchaseOrderItem()));
  }

  else if ( (_event->currentItem()->rawValue("evnttype_name").toString() == "SoitemCreated") ||
            (_event->currentItem()->rawValue("evnttype_name").toString() == "SoitemQtyChanged") ||
            (_event->currentItem()->rawValue("evnttype_name").toString() == "SoitemSchedDateChanged") )
  {
    menu->addSeparator();

    menuItem = menu->addAction(tr("View Sales Order..."), this, SLOT(sViewSalesOrder()));
    menuItem = menu->addAction(tr("View Sales Order Item..."), this, SLOT(sViewSalesOrderItem()));
    menuItem = menu->addAction(tr("Print Packing List..."), this, SLOT(sPrintPackingList()));
  }

  else if (_event->currentItem()->rawValue("evnttype_name").toString() == "SoCommentsChanged")
  {
    menu->addSeparator();

    menuItem = menu->addAction(tr("View Sales Order..."), this, SLOT(sViewSalesOrder()));
    menuItem = menu->addAction(tr("Print Packing List..."), this, SLOT(sPrintPackingList()));
  }

  else if (_event->currentItem()->rawValue("evnttype_name").toString() == "QOHBelowZero")
  {
    menu->addSeparator();

    menuItem = menu->addAction(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()));
    menuItem = menu->addAction(tr("View Inventory History..."), this, SLOT(sViewInventoryHistory()));
    menuItem = menu->addAction(tr("View Inventory Availability..."), this, SLOT(sViewInventoryAvailability()));
  }

  else if (_event->currentItem()->rawValue("evnttype_name").toString() == "RWoQtyRequestChange")
  {
    menu->addSeparator();

    menuItem = menu->addAction(tr("Recall Work Order"), this, SLOT(sRecallWo()));
    menuItem = menu->addAction(tr("Change W/O Quantity..."), this, SLOT(sChangeWoQty()));
    menuItem = menu->addAction(tr("Print W/O Traveler..."), this, SLOT(sPrintWoTraveler()));
  }

  else if (_event->currentItem()->rawValue("evnttype_name").toString() == "RWoDueDateRequestChange")
  {
    menu->addSeparator();

    menuItem = menu->addAction(tr("Recall Work Order"), this, SLOT(sRecallWo()));
    menuItem = menu->addAction(tr("Change W/O Due Date..."), this, SLOT(sChangeWoDueDate()));
    menuItem = menu->addAction(tr("Print W/O Traveler..."), this, SLOT(sPrintWoTraveler()));
  }

  else if (_event->currentItem()->rawValue("evnttype_name").toString() == "RWoRequestCancel")
  {
    menu->addSeparator();

    menuItem = menu->addAction(tr("Recall Work Order"), this, SLOT(sRecallWo()));
    menuItem = menu->addAction(tr("Delete Work Order..."), this, SLOT(sDeleteWorkOrder()));
  }

  else if (_event->currentItem()->rawValue("evnttype_name").toString() == "TodoAlarm")
  {
    menu->addSeparator();

    menuItem = menu->addAction(tr("View Todo Item..."), this, SLOT(sViewTodoItem()));
  }

  else if (_event->currentItem()->rawValue("evnttype_name").toString() == "IncidentAlarm")
  {
    menu->addSeparator();

    menuItem = menu->addAction(tr("View Incident..."), this, SLOT(sViewIncident()));
  }

  else if (_event->currentItem()->rawValue("evnttype_name").toString() == "TaskAlarm")
  {
    menu->addSeparator();

    menuItem = menu->addAction(tr("View Project Task..."), this, SLOT(sViewTask()));
  }
}

void eventManager::sInventoryAvailabilityByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());

  dspInventoryAvailabilityByWorkOrder *newdlg = new dspInventoryAvailabilityByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void eventManager::sViewSalesOrder()
{
  XSqlQuery eventViewSalesOrder;
  eventViewSalesOrder.prepare( "SELECT coitem_cohead_id "
             "FROM coitem "
             "WHERE (coitem_id=:coitem_id);" );
  eventViewSalesOrder.bindValue(":coitem_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
  eventViewSalesOrder.exec();
  if (eventViewSalesOrder.first())
    salesOrder::viewSalesOrder(eventViewSalesOrder.value("coitem_cohead_id").toInt());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving S/O Information"),
                                eventViewSalesOrder, __FILE__, __LINE__))
  {
    return;
  }
}

void eventManager::sViewSalesOrderItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sViewPurchaseOrderItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("poitem_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
      
  purchaseOrderItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sPrintPackingList()
{
  XSqlQuery eventPrintPackingList;
  eventPrintPackingList.prepare( "SELECT coitem_cohead_id "
             "FROM coitem "
             "WHERE (coitem_id=:coitem_id);" );
  eventPrintPackingList.bindValue(":coitem_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
  eventPrintPackingList.exec();
  if (eventPrintPackingList.first())
  {
    ParameterList params;
    params.append("sohead_id", eventPrintPackingList.value("coitem_cohead_id").toInt());

    printPackingList newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Printing Packing List"),
                                eventPrintPackingList, __FILE__, __LINE__))
  {
    return;
  }
}

void eventManager::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sViewInventoryHistory()
{
  ParameterList params;
  params.append("itemsite_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
  
  dspInventoryHistory *newdlg = new dspInventoryHistory();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void eventManager::sViewInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
  params.append("run");
  
  dspInventoryAvailability *newdlg = new dspInventoryAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void eventManager::sRecallWo()
{
  XSqlQuery eventRecallWo;
  eventRecallWo.prepare("SELECT recallWo(:wo_id, false) AS result;");
  eventRecallWo.bindValue(":wo_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
  eventRecallWo.exec();
  if (eventRecallWo.first())
  {
    int result = eventRecallWo.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Recalling Work Order"),
                             storedProcErrorLookup("recallWo", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Recalling Work Order"),
                                eventRecallWo, __FILE__, __LINE__))
  {
    return;
  }
}

void eventManager::sChangeWoQty()
{
  ParameterList params;
  params.append("wo_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
  params.append("newQty", _event->currentItem()->rawValue("evntlog_newvalue").toDouble());

  changeWoQty newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sChangeWoDueDate()
{
//  ToDo
#if 0
  (new rescheduleWo( _event->currentItem()->rawValue("evntlog_ord_id").toInt(),
                     _event->currentItem()->rawValue("evntlog_newdate"),
                     omfgThis ))->show();
#endif
}

void eventManager::sPrintWoTraveler()
{
  ParameterList params;
  params.append("wo_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());

  printWoTraveler newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sDeleteWorkOrder()
{
  XSqlQuery eventDeleteWorkOrder;
  if ( QMessageBox::warning( this, tr("Delete Work Order?"),
                             tr("Are you sure that you want to delete the selected Work Order?"),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    eventDeleteWorkOrder.prepare("SELECT deleteWo(:wo_id, true) AS returnVal;");
    eventDeleteWorkOrder.bindValue(":wo_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
    eventDeleteWorkOrder.exec();
    if (eventDeleteWorkOrder.first())
    {
      int result = eventDeleteWorkOrder.value("result").toInt();
      if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Work Order"),
                               storedProcErrorLookup("deleteWo", result),
                               __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Work Order"),
                                  eventDeleteWorkOrder, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void eventManager::sViewTodoItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
      
  todoItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
      
  incident newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sViewTask()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prjtask_id", _event->currentItem()->rawValue("evntlog_ord_id").toInt());
      
  task newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sAcknowledge()
{
  XSqlQuery eventAcknowledge;
  eventAcknowledge.prepare( "UPDATE evntlog "
             "SET evntlog_dispatched=CURRENT_TIMESTAMP "
             "WHERE (evntlog_id=:evntlog_id)" );

  QList<XTreeWidgetItem*> list = _event->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
    eventAcknowledge.bindValue(":evntlog_id", ((XTreeWidgetItem*)(list[i]))->id());
    eventAcknowledge.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Event Log"),
                                  eventAcknowledge, __FILE__, __LINE__))
    {
      return;
    }
  }

  sFillList();
}

void eventManager::sDelete()
{
  XSqlQuery eventDelete;
  eventDelete.prepare( "DELETE FROM evntlog "
             "WHERE (evntlog_id=:evntlog_id);" );

  QList<XTreeWidgetItem*> list = _event->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
    eventDelete.bindValue(":evntlog_id", ((XTreeWidgetItem*)(list[i]))->id());
    eventDelete.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Removing Event Log Entry"),
                                  eventDelete, __FILE__, __LINE__))
    {
      return;
    }
  }

  sFillList();
}

void eventManager::sFillList()
{
  XSqlQuery eventFillList;
  MetaSQLQuery mql = mqlLoad("events", "detail");
  ParameterList params;
  params.append("username", _currentUser->isChecked() ? omfgThis->username() :
							_usr->currentText());
  _warehouse->appendValue(params);
  if (_showAcknowledged->isChecked())
    params.append("showAcknowledged");
  eventFillList = mql.toQuery(params);
  _event->populate(eventFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Event Log Information"),
                                eventFillList, __FILE__, __LINE__))
  {
    return;
  }
}

void eventManager::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

