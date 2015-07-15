/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemAvailabilityWorkbench.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "dspCostedIndentedBOM.h"
#include "dspInventoryAvailability.h"
#include "dspInventoryHistory.h"
#include "dspInventoryLocator.h"
#include "dspPoItemsByItem.h"
#include "dspPoItemReceivingsByItem.h"
#include "dspPricesByCustomer.h"
#include "dspQuotesByItem.h"
#include "dspRunningAvailability.h"
#include "dspSalesHistory.h"
#include "dspSalesOrdersByItem.h"
#include "dspSingleLevelWhereUsed.h"
#include "item.h"
#include "parameterwidget.h"

itemAvailabilityWorkbench::itemAvailabilityWorkbench(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _dspInventoryAvailability = new dspInventoryAvailability(this, "dspInventoryAvailabilty", Qt::Widget);
  _dspInventoryAvailability->setObjectName("dspInventoryAvailability");
  _availabilityPage->layout()->addWidget(_dspInventoryAvailability);
  _dspInventoryAvailability->setCloseVisible(false);
  _dspInventoryAvailability->setQueryOnStartEnabled(false);
  _dspInventoryAvailability->setParameterWidgetVisible(false);
  _dspInventoryAvailability->setAutoUpdateEnabled(false);
  _dspInventoryAvailability->optionsWidget()->show();
  _dspInventoryAvailability->list()->hideColumn("item_number");
  _dspInventoryAvailability->list()->hideColumn("itemdescrip");
  _dspInventoryAvailability->list()->hideColumn("uom_name");
  _dspInventoryAvailability->findChild<QWidget*>("_showGroup")->hide();
  // set asof to Itemsite Lead Time to avoid invalid date prompt
  _dspInventoryAvailability->findChild<QComboBox*>("_asof")->setCurrentIndex(0);
  
  _dspRunningAvailability = new dspRunningAvailability(this, "dspRunningAvailabilty", Qt::Widget);
  _dspRunningAvailability->setObjectName("dspRunningAvailability");
  _runningAvailabilityPage->layout()->addWidget(_dspRunningAvailability);
  _dspRunningAvailability->setCloseVisible(false);
  _dspRunningAvailability->setQueryOnStartEnabled(false);
  _dspRunningAvailability->findChild<QWidget*>("_item")->hide();
  
  _dspInventoryLocator = new dspInventoryLocator(this, "dspInventoryLocator", Qt::Widget);
  _dspInventoryLocator->setObjectName("dspInventoryLocator");
  _locationDetailPage->layout()->addWidget(_dspInventoryLocator);
  _dspInventoryLocator->setCloseVisible(false);
  _dspInventoryLocator->setQueryOnStartEnabled(false);
  _dspInventoryLocator->findChild<QWidget*>("_item")->hide();
  _dspInventoryLocator->findChild<QWidget*>("_itemGroup")->hide();
  
  _dspCostedIndentedBOM = new dspCostedIndentedBOM(this, "dspCostedIndentedBOM", Qt::Widget);
  _dspCostedIndentedBOM->setObjectName("dspCostedIndentedBOM");
  _costedIndentedBOMPage->layout()->addWidget(_dspCostedIndentedBOM);
  _dspCostedIndentedBOM->setCloseVisible(false);
  _dspCostedIndentedBOM->setQueryOnStartEnabled(false);
  _dspCostedIndentedBOM->findChild<QWidget*>("_item")->hide();
  
  _dspSingleLevelWhereUsed = new dspSingleLevelWhereUsed(this, "dspSingleLevelWhereUsed", Qt::Widget);
  _dspSingleLevelWhereUsed->setObjectName("dspSingleLevelWhereUsed");
  _whereUsedPage->layout()->addWidget(_dspSingleLevelWhereUsed);
  _dspSingleLevelWhereUsed->setCloseVisible(false);
  _dspSingleLevelWhereUsed->setQueryOnStartEnabled(false);
  _dspSingleLevelWhereUsed->findChild<QWidget*>("_item")->hide();
  
  _dspInventoryHistory = new dspInventoryHistory(this, "dspInventoryHistory", Qt::Widget);
  _dspInventoryHistory->setObjectName("dspInventoryHistory");
  _inventoryHistoryPage->layout()->addWidget(_dspInventoryHistory);
  _dspInventoryHistory->setCloseVisible(false);
  _dspInventoryHistory->setQueryOnStartEnabled(false);
//  _dspInventoryHistory->setParameterWidgetVisible(false);
  _dspInventoryHistory->setAutoUpdateEnabled(false);
//  _dspInventoryHistory->setStartDate(QDate().currentDate().addDays(-365));
//  _dspInventoryHistory->list()->hideColumn("item_number");
  
  _dspPoItemReceivingsByItem = new dspPoItemReceivingsByItem(this, "dspPoItemReceivingsByItem", Qt::Widget);
  _dspPoItemReceivingsByItem->setObjectName("dspPoItemReceivingsByItem");
  _receivingHistoryPage->layout()->addWidget(_dspPoItemReceivingsByItem);
  _dspPoItemReceivingsByItem->setCloseVisible(false);
  _dspPoItemReceivingsByItem->setQueryOnStartEnabled(false);
  _dspPoItemReceivingsByItem->findChild<QWidget*>("_item")->hide();
  _dspPoItemReceivingsByItem->findChild<QWidget*>("_itemGroup")->hide();
  _dspPoItemReceivingsByItem->findChild<DateCluster*>("_dates")->setStartDate(QDate().currentDate().addDays(-365));
  _dspPoItemReceivingsByItem->findChild<DateCluster*>("_dates")->setEndDate(QDate().currentDate());
  
  _dspSalesHistory = new dspSalesHistory(this, "dspSalesHistory", Qt::Widget);
  _dspSalesHistory->setObjectName("dspSalesHistory");
  _salesHistoryPage->layout()->addWidget(_dspSalesHistory);
  _dspSalesHistory->setCloseVisible(false);
  _dspSalesHistory->setQueryOnStartEnabled(false);
  _dspSalesHistory->setParameterWidgetVisible(false);
  _dspSalesHistory->setAutoUpdateEnabled(false);
  _dspSalesHistory->setStartDate(QDate().currentDate().addDays(-365));
  _dspSalesHistory->list()->hideColumn("item_number");
  _dspSalesHistory->list()->hideColumn("itemdescription");
  
  _dspPoItemsByItem = new dspPoItemsByItem(this, "dspPoItemsByItem", Qt::Widget);
  _dspPoItemsByItem->setObjectName("dspPoItemsByItem");
  _purchaseOrderItemsPage->layout()->addWidget(_dspPoItemsByItem);
  _dspPoItemsByItem->setCloseVisible(false);
  _dspPoItemsByItem->setQueryOnStartEnabled(false);
  _dspPoItemsByItem->findChild<QWidget*>("_item")->hide();
  _dspPoItemsByItem->findChild<QWidget*>("_itemGroup")->hide();
  
  _dspSalesOrdersByItem = new dspSalesOrdersByItem(this, "dspSalesOrdersByItem", Qt::Widget);
  _dspSalesOrdersByItem->setObjectName("dspSalesOrdersByItem");
  _salesOrderItemsPage->layout()->addWidget(_dspSalesOrdersByItem);
  _dspSalesOrdersByItem->setCloseVisible(false);
  _dspSalesOrdersByItem->setQueryOnStartEnabled(false);
  _dspSalesOrdersByItem->findChild<QWidget*>("_item")->hide();
  
  _dspQuotesByItem = new dspQuotesByItem(this, "dspQuotesByItem", Qt::Widget);
  _dspQuotesByItem->setObjectName("dspQuotesByItem");
  _quoteItemsPage->layout()->addWidget(_dspQuotesByItem);
  _dspQuotesByItem->setCloseVisible(false);
  _dspQuotesByItem->setQueryOnStartEnabled(false);
  _dspQuotesByItem->findChild<QWidget*>("_item")->hide();
  
  _dspPricesByCustomer = new dspPricesByCustomer(this, "dspPricesByCustomer", Qt::Widget);
  _dspPricesByCustomer->setObjectName("dspPricesByCustomer");
  _customerPricesPage->layout()->addWidget(_dspPricesByCustomer);
  _dspPricesByCustomer->setCloseVisible(false);
  _dspPricesByCustomer->setQueryOnStartEnabled(false);
  _dspPricesByCustomer->findChild<QWidget*>("_item")->hide();
  
  _itemMaster = new item(this, "item", Qt::Widget);
  _itemMaster->setObjectName("item");
  _itemPage->layout()->addWidget(_itemMaster);
  _itemMaster->findChild<QWidget*>("_itemNumber")->hide();
  _itemMaster->findChild<QWidget*>("_itemNumberLit")->hide();
  _itemMaster->findChild<QWidget*>("_description1")->hide();
  _itemMaster->findChild<QWidget*>("_description2")->hide();
  _itemMaster->findChild<QWidget*>("_descriptionLit")->hide();
  _itemMaster->findChild<QWidget*>("_save")->hide();
  _itemMaster->findChild<QWidget*>("_close")->hide();
  _itemMaster->findChild<QWidget*>("_print")->hide();
  _itemMaster->findChild<QWidget*>("_newCharacteristic")->hide();
  _itemMaster->findChild<QWidget*>("_editCharacteristic")->hide();
  _itemMaster->findChild<QWidget*>("_deleteCharacteristic")->hide();
  _itemMaster->findChild<QWidget*>("_newAlias")->hide();
  _itemMaster->findChild<QWidget*>("_editAlias")->hide();
  _itemMaster->findChild<QWidget*>("_deleteAlias")->hide();
  _itemMaster->findChild<QWidget*>("_newSubstitute")->hide();
  _itemMaster->findChild<QWidget*>("_editSubstitute")->hide();
  _itemMaster->findChild<QWidget*>("_deleteSubstitute")->hide();
  _itemMaster->findChild<QWidget*>("_newTransform")->hide();
  _itemMaster->findChild<QWidget*>("_deleteTransform")->hide();
  _itemMaster->findChild<QWidget*>("_newItemSite")->hide();
  _itemMaster->findChild<QWidget*>("_editItemSite")->hide();
  _itemMaster->findChild<QWidget*>("_deleteItemSite")->hide();
  _itemMaster->findChild<QWidget*>("_itemtaxNew")->hide();
  _itemMaster->findChild<QWidget*>("_itemtaxEdit")->hide();
  _itemMaster->findChild<QWidget*>("_itemtaxDelete")->hide();
  _itemMaster->findChild<QWidget*>("_newUOM")->hide();
  _itemMaster->findChild<QWidget*>("_editUOM")->hide();
  _itemMaster->findChild<QWidget*>("_deleteUOM")->hide();
  _itemMaster->findChild<QWidget*>("_newSrc")->hide();
  _itemMaster->findChild<QWidget*>("_editSrc")->hide();
  _itemMaster->findChild<QWidget*>("_deleteSrc")->hide();
  _itemMaster->findChild<QWidget*>("_copySrc")->hide();
  _itemMaster->findChild<QTabWidget*>("_tab")->removeTab(2);
  _itemMaster->findChild<QWidget*>("_active")->setEnabled(false);
  _itemMaster->findChild<QWidget*>("_sold")->setEnabled(false);
  _itemMaster->findChild<QWidget*>("_itemGroup")->setEnabled(false);
  _itemMaster->findChild<QWidget*>("_weightGroup")->setEnabled(false);
  
  connect(_tab, SIGNAL(currentChanged(int)), this, SLOT(sFillList()));
  connect(_availabilityButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_runningAvailabilityButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_locationDetailButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_costedIndentedBOMButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_whereUsedButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_inventoryHistoryButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_receivingHistoryButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_salesHistoryButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_purchaseOrderItemsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_salesOrderItemsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_quoteItemsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_customerPricesButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_item,	SIGNAL(newId(int)), this, SLOT(populate()));

  // General
  if (!_privileges->check("ViewInventoryAvailability") && !_privileges->check("ViewQOH"))
    _tab->removeTab(0);
  else
  {
    if (!_privileges->check("ViewInventoryAvailability"))
    {
      _availabilityButton->hide();
      _runningAvailabilityButton->hide();
    }
    if (!_privileges->check("ViewQOH"))
      _locationDetailButton->hide();
  }
  
  if (!_privileges->check("ViewPurchaseOrders") && !_privileges->check("ViewSalesOrders") && !_privileges->check("ViewQuotes"))
    _tab->removeTab(1);
  else
  {
    if (!_privileges->check("ViewPurchaseOrders"))
      _purchaseOrderItemsButton->hide();
    if (!_privileges->check("ViewSalesOrders"))
      _salesOrderItemsButton->hide();
    if (!_privileges->check("ViewQuotes"))
      _quoteItemsButton->hide();
  }
  
  if (!_privileges->check("ViewInventoryHistory") && !_privileges->check("ViewReceiptsReturns") && !_privileges->check("ViewSalesHistory"))
    _tab->removeTab(2);
  else
  {
    if (!_privileges->check("ViewInventoryHistory"))
      _inventoryHistoryButton->hide();
    if (!_privileges->check("ViewReceiptsReturns"))
      _receivingHistoryButton->hide();
    if (!_privileges->check("ViewSalesHistory"))
      _salesHistoryButton->hide();
  }

  if (!_privileges->check("ViewItemMaster") && !_privileges->check("MaintainItemMasters"))
    _tab->removeTab(3);

  if (!_privileges->check("ViewCosts") && !_privileges->check("ViewBOMs"))
    _tab->removeTab(4);
  else
  {
    if (!_privileges->check("ViewCosts"))
      _costedIndentedBOMButton->hide();
    if (!_privileges->check("ViewBOMs"))
      _whereUsedButton->hide();
  }
}

itemAvailabilityWorkbench::~itemAvailabilityWorkbench()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemAvailabilityWorkbench::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemAvailabilityWorkbench::set( const ParameterList & pParams )
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  return NoError;
}

void itemAvailabilityWorkbench::sHandleButtons()
{
  if (_availabilityButton->isChecked())
    _availabilityStack->setCurrentIndex(0);
  else if (_runningAvailabilityButton->isChecked())
    _availabilityStack->setCurrentIndex(1);
  else if (_locationDetailButton->isChecked())
    _availabilityStack->setCurrentIndex(2);
  
  if (_costedIndentedBOMButton->isChecked())
    _bomStack->setCurrentIndex(0);
  else if (_whereUsedButton->isChecked())
    _bomStack->setCurrentIndex(1);

  if (_inventoryHistoryButton->isChecked())
    _historyStack->setCurrentIndex(0);
  else if (_receivingHistoryButton->isChecked())
    _historyStack->setCurrentIndex(1);
  else if (_salesHistoryButton->isChecked())
    _historyStack->setCurrentIndex(2);
  
  if (_purchaseOrderItemsButton->isChecked())
    _ordersStack->setCurrentIndex(0);
  else if (_salesOrderItemsButton->isChecked())
    _ordersStack->setCurrentIndex(1);
  else if (_quoteItemsButton->isChecked())
    _ordersStack->setCurrentIndex(2);
  else if (_customerPricesButton->isChecked())
    _ordersStack->setCurrentIndex(3);
  
  sFillList();
}

void itemAvailabilityWorkbench::populate()
{
  _dspInventoryAvailability->setItemId(_item->id());
  _dspRunningAvailability->findChild<ItemCluster*>("_item")->setId(_item->id());
  _dspInventoryLocator->findChild<ItemCluster*>("_item")->setId(_item->id());
  _dspCostedIndentedBOM->findChild<ItemCluster*>("_item")->setId(_item->id());
  _dspSingleLevelWhereUsed->findChild<ItemCluster*>("_item")->setId(_item->id());
  _dspInventoryHistory->setItemId(_item->id());
  _dspPoItemReceivingsByItem->findChild<ItemCluster*>("_item")->setId(_item->id());
  _dspSalesHistory->setItemId(_item->id());
  _dspPoItemsByItem->findChild<ItemCluster*>("_item")->setId(_item->id());
  _dspSalesOrdersByItem->findChild<ItemCluster*>("_item")->setId(_item->id());
  _dspQuotesByItem->findChild<ItemCluster*>("_item")->setId(_item->id());
  _dspPricesByCustomer->findChild<ItemCluster*>("_item")->setId(_item->id());
  _itemMaster->setId(_item->id());
  _itemMaster->findChild<QWidget*>("_sold")->setEnabled(false);
  
  sFillList();
}

void itemAvailabilityWorkbench::sFillList()
{
  if (!_item->isValid())
    return;
  
  bool _sold = false;
  XSqlQuery itemq;
  itemq.prepare("SELECT item_sold FROM item "
                "WHERE (item_id=:item_id);");
  itemq.bindValue(":item_id", _item->id());
  itemq.exec();
  if (itemq.first())
    _sold = itemq.value("item_sold").toBool();
  if (_tab->currentIndex() == _tab->indexOf(_availabilityTab))
  {
    if (_availabilityButton->isChecked())
      _dspInventoryAvailability->sFillList();
    else if (_runningAvailabilityButton->isChecked())
      _dspRunningAvailability->sFillList();
    else if (_locationDetailButton->isChecked())
      _dspInventoryLocator->sFillList();
  }
  else if (_tab->currentIndex() == _tab->indexOf(_bomTab))
  {
    if (_costedIndentedBOMButton->isChecked())
      _dspCostedIndentedBOM->sFillList();
    else if (_whereUsedButton->isChecked())
      _dspSingleLevelWhereUsed->sFillList();
  }
  else if (_tab->currentIndex() == _tab->indexOf(_historyTab))
  {
    if (_inventoryHistoryButton->isChecked())
      _dspInventoryHistory->sFillList();
    else if (_receivingHistoryButton->isChecked())
      _dspPoItemReceivingsByItem->sFillList();
    else if (_salesHistoryButton->isChecked() && _sold)
      _dspSalesHistory->sFillList();
  }
  else if (_tab->currentIndex() == _tab->indexOf(_ordersTab))
  {
    if (_purchaseOrderItemsButton->isChecked())
      _dspPoItemsByItem->sFillList();
    else if (_salesOrderItemsButton->isChecked() && _sold)
      _dspSalesOrdersByItem->sFillList();
    else if (_quoteItemsButton->isChecked() && _sold)
      _dspQuotesByItem->sFillList();
//    else if (_customerPricesButton->isChecked())
//      _dspPricesByCustomer->sFillList();
  }
}
