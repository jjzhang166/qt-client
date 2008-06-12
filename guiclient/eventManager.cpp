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

#include "eventManager.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QTimer>
#include <QVariant>

#include <metasql.h>

#include "changeWoQty.h"
#include "createCountTagsByItem.h"
#include "dspInventoryAvailabilityByItem.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspInventoryHistoryByItem.h"
#include "mqlutil.h"
#include "printPackingList.h"
#include "printWoTraveler.h"
#include "rescheduleWo.h"
#include "salesOrder.h"
#include "salesOrderItem.h"
#include "storedProcErrorLookup.h"
#include "purchaseOrderItem.h"

eventManager::eventManager(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
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
    _selectedUser->setEnabled(FALSE);

  _event->addColumn("orderId",         0,               Qt::AlignCenter );
  _event->addColumn("newDate",         0,               Qt::AlignCenter );
  _event->addColumn("newQty",          0,               Qt::AlignCenter );
  _event->addColumn(tr("Whs."),        _whsColumn,      Qt::AlignCenter );
  _event->addColumn(tr("Time"),        _timeDateColumn, Qt::AlignLeft   );
  _event->addColumn(tr("Acknowleged"), _timeDateColumn, Qt::AlignLeft   );
  _event->addColumn(tr("Event Type"),  _itemColumn,     Qt::AlignLeft   );
  _event->addColumn(tr("Order #"),     -1,              Qt::AlignLeft   );

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
  int menuItem;

  if (_event->currentItem()->text(5).length() == 0)
  {
    menuItem = menu->insertItem(tr("Acknowledge"), this, SLOT(sAcknowledge()), 0);
    if ( ((_currentUser->isChecked()) && (!_privileges->check("DispatchOwnEvents"))) ||
         ((_selectedUser->isChecked()) && (!_privileges->check("DispatchOtherEvents"))) )
        menu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = menu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
  if ( ((_currentUser->isChecked()) && (!_privileges->check("DeleteOwnEvents"))) ||
       ((_selectedUser->isChecked()) && (!_privileges->check("DeleteOtherEvents"))) )
      menu->setItemEnabled(menuItem, FALSE);

  // if multiple items are selected then keep the menu short
  QList<QTreeWidgetItem*> list = _event->selectedItems();
  if (list.size() > 1)
    return;

  if ( (_event->currentItem()->text(6) == "WoCreated") ||
       (_event->currentItem()->text(6) == "WoDueDateChanged") ||
       (_event->currentItem()->text(6) == "WoQtyChanged") )
  {
    menu->insertSeparator();

    menuItem = menu->insertItem(tr("Inventory Availability by Work Order..."), this, SLOT(sInventoryAvailabilityByWorkOrder()), 0);
  }
  
  else if ( (_event->currentItem()->text(6) == "POitemCreate") )
  {
    menu->insertSeparator();

    menuItem = menu->insertItem(tr("View Purchase Order Item..."), this, SLOT(sViewPurchaseOrderItem()), 0);
  }

  else if ( (_event->currentItem()->text(6) == "SoitemCreated") ||
            (_event->currentItem()->text(6) == "SoitemQtyChanged") ||
            (_event->currentItem()->text(6) == "SoitemSchedDateChanged") )
  {
    menu->insertSeparator();

    menuItem = menu->insertItem(tr("View Sales Order..."), this, SLOT(sViewSalesOrder()), 0);
    menuItem = menu->insertItem(tr("View Sales Order Item..."), this, SLOT(sViewSalesOrderItem()), 0);
    menuItem = menu->insertItem(tr("Print Packing List..."), this, SLOT(sPrintPackingList()), 0);
  }

  else if (_event->currentItem()->text(6) == "SoCommentsChanged")
  {
    menu->insertSeparator();

    menuItem = menu->insertItem(tr("View Sales Order..."), this, SLOT(sViewSalesOrder()), 0);
    menuItem = menu->insertItem(tr("Print Packing List..."), this, SLOT(sPrintPackingList()), 0);
  }

  else if (_event->currentItem()->text(6) == "QOHBelowZero")
  {
    menu->insertSeparator();

    menuItem = menu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
    menuItem = menu->insertItem(tr("View Inventory History..."), this, SLOT(sViewInventoryHistory()), 0);
    menuItem = menu->insertItem(tr("View Inventory Availability..."), this, SLOT(sViewInventoryAvailability()), 0);
  }

  else if (_event->currentItem()->text(6) == "RWoQtyRequestChange")
  {
    menu->insertSeparator();

    menuItem = menu->insertItem(tr("Recall Work Order"), this, SLOT(sRecallWo()), 0);
    menuItem = menu->insertItem(tr("Change W/O Quantity..."), this, SLOT(sChangeWoQty()), 0);
    menuItem = menu->insertItem(tr("Print W/O Traveler..."), this, SLOT(sPrintWoTraveler()), 0);
  }

  else if (_event->currentItem()->text(6) == "RWoDueDateRequestChange")
  {
    menu->insertSeparator();

    menuItem = menu->insertItem(tr("Recall Work Order"), this, SLOT(sRecallWo()), 0);
    menuItem = menu->insertItem(tr("Change W/O Due Date..."), this, SLOT(sChangeWoDueDate()), 0);
    menuItem = menu->insertItem(tr("Print W/O Traveler..."), this, SLOT(sPrintWoTraveler()), 0);
  }

  else if (_event->currentItem()->text(6) == "RWoRequestCancel")
  {
    menu->insertSeparator();

    menuItem = menu->insertItem(tr("Recall Work Order"), this, SLOT(sRecallWo()), 0);
    menuItem = menu->insertItem(tr("Delete Work Order..."), this, SLOT(sDeleteWorkOrder()), 0);
  }
}

void eventManager::sInventoryAvailabilityByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", _event->currentItem()->text(0).toInt());

  dspInventoryAvailabilityByWorkOrder *newdlg = new dspInventoryAvailabilityByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void eventManager::sViewSalesOrder()
{
  q.prepare( "SELECT coitem_cohead_id "
             "FROM coitem "
             "WHERE (coitem_id=:coitem_id);" );
  q.bindValue(":coitem_id", _event->currentItem()->text(0).toInt());
  q.exec();
  if (q.first())
    salesOrder::viewSalesOrder(q.value("coitem_cohead_id").toInt());
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void eventManager::sViewSalesOrderItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", _event->currentItem()->text(0).toInt());
      
  salesOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sViewPurchaseOrderItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("poitem_id", _event->currentItem()->text(0).toInt());
      
  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sPrintPackingList()
{
  q.prepare( "SELECT coitem_cohead_id "
             "FROM coitem "
             "WHERE (coitem_id=:coitem_id);" );
  q.bindValue(":coitem_id", _event->currentItem()->text(0).toInt());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("sohead_id", q.value("coitem_cohead_id").toInt());

    printPackingList newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void eventManager::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _event->currentItem()->text(0).toInt());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sViewInventoryHistory()
{
  ParameterList params;
  params.append("itemsite_id", _event->currentItem()->text(0).toInt());
  
  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void eventManager::sViewInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _event->currentItem()->text(0).toInt());
  
  dspInventoryAvailabilityByItem *newdlg = new dspInventoryAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void eventManager::sRecallWo()
{
  q.prepare("SELECT recallWo(:wo_id, FALSE) AS result;");
  q.bindValue(":wo_id", _event->currentItem()->text(0).toInt());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("recallWo", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void eventManager::sChangeWoQty()
{
  ParameterList params;
  params.append("wo_id", _event->currentItem()->text(0).toInt());
  params.append("newQty", _event->currentItem()->text(2).toDouble());

  changeWoQty newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sChangeWoDueDate()
{
//  ToDo
#if 0
  (new rescheduleWo( _event->currentItem()->text(0).toInt(),
                     _event->currentItem()->text(1),
                     omfgThis ))->show();
#endif
}

void eventManager::sPrintWoTraveler()
{
  ParameterList params;
  params.append("wo_id", _event->currentItem()->text(0).toInt());

  printWoTraveler newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void eventManager::sDeleteWorkOrder()
{
  if ( QMessageBox::warning( this, tr("Delete Work Order?"),
                             tr("Are you sure that you want to delete the selected Work Order?"),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare("SELECT deleteWo(:wo_id, TRUE) AS returnVal;");
    q.bindValue(":wo_id", _event->currentItem()->text(0).toInt());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteWo", result), __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void eventManager::sAcknowledge()
{
  q.prepare( "UPDATE evntlog "
             "SET evntlog_dispatched=CURRENT_TIMESTAMP "
             "WHERE (evntlog_id=:evntlog_id)" );

  QList<QTreeWidgetItem*> list = _event->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
    q.bindValue(":evntlog_id", ((XTreeWidgetItem*)(list[i]))->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillList();
}

void eventManager::sDelete()
{
  q.prepare( "DELETE FROM evntlog "
             "WHERE (evntlog_id=:evntlog_id);" );

  QList<QTreeWidgetItem*> list = _event->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
    q.bindValue(":evntlog_id", ((XTreeWidgetItem*)(list[i]))->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillList();
}

void eventManager::sFillList()
{
  MetaSQLQuery mql = mqlLoad(":/sys/eventManager/FillListDetail.mql");
  ParameterList params;
  params.append("username", _currentUser->isChecked() ? omfgThis->username() :
							_usr->currentText());
  _warehouse->appendValue(params);
  if (_showAcknowledged->isChecked())
    params.append("showAcknowledged");
  q = mql.toQuery(params);
  _event->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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

