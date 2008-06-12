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

//  menuInventory.cpp
//  Created 08/22/2000, JSL
//  Copyright (c) 2000-2008, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QPixmap>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"
#include "inputManager.h"

#include "itemSite.h"
#include "itemSites.h"

#include "adjustmentTrans.h"
#include "transferTrans.h"
#include "transferOrders.h"
#include "transferOrder.h"
#include "scrapTrans.h"
#include "expenseTrans.h"
#include "transformTrans.h"
#include "resetQOHBalances.h"
#include "materialReceiptTrans.h"
#include "relocateInventory.h"

#include "reassignLotSerial.h"

#include "createCountTagsByParameterList.h"
#include "createCountTagsByItem.h"
#include "createCycleCountTags.h"
#include "countSlip.h"
#include "countTag.h"
#include "enterMiscCount.h"
#include "zeroUncountedCountTagsByWarehouse.h"
#include "postCountSlips.h"
#include "postCountTags.h"
#include "purgePostedCountSlips.h"
#include "purgePostedCounts.h"
#include "thawItemSitesByClassCode.h"

#include "packingListBatch.h"
#include "issueToShipping.h"
#include "maintainShipping.h"
#include "shipOrder.h"
#include "recallOrders.h"
#include "purgeShippingRecords.h"
#include "externalShipping.h"

#include "enterPoReceipt.h"
#include "enterPoReturn.h"
#include "unpostedPoReceipts.h"

#include "printPackingList.h"
#include "printPackingListBatchByShipvia.h"
#include "printShippingForm.h"
#include "printShippingForms.h"
#include "printLabelsBySo.h"
#include "printLabelsByTo.h"
#include "printLabelsByInvoice.h"
#include "printLabelsByOrder.h"

#include "destinations.h"
#include "carriers.h"

#include "dspShipmentsBySalesOrder.h"
#include "dspBacklogByItem.h"
#include "dspBacklogByCustomer.h"
#include "dspBacklogByParameterList.h"
#include "dspSummarizedBacklogByWarehouse.h"
#include "dspShipmentsBySalesOrder.h"
#include "dspShipmentsByDate.h"
#include "dspShipmentsByShipment.h"

#include "dspFrozenItemSites.h"
#include "dspCountSlipEditList.h"
#include "dspCountTagEditList.h"
#include "dspCountSlipsByWarehouse.h"
#include "dspCountTagsByItem.h"
#include "dspCountTagsByWarehouse.h"
#include "dspCountTagsByClassCode.h"

#include "itemAvailabilityWorkbench.h"

#include "dspItemSitesByItem.h"
#include "dspItemSitesByParameterList.h"
#include "dspValidLocationsByItem.h"
#include "dspQOHByItem.h"
#include "dspQOHByParameterList.h"
#include "dspQOHByLocation.h"
#include "dspInventoryLocator.h"
#include "dspSlowMovingInventoryByClassCode.h"
#include "dspExpiredInventoryByClassCode.h"
#include "dspInventoryAvailabilityByItem.h"
#include "dspInventoryAvailabilityByParameterList.h"
#include "dspInventoryAvailabilityBySourceVendor.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "dspInventoryBufferStatusByParameterList.h"
#include "dspInventoryHistoryByItem.h"
#include "dspInventoryHistoryByOrderNumber.h"
#include "dspInventoryHistoryByParameterList.h"
#include "dspDetailedInventoryHistoryByLotSerial.h"
#include "dspDetailedInventoryHistoryByLocation.h"
#include "dspUsageStatisticsByItem.h"
#include "dspUsageStatisticsByClassCode.h"
#include "dspUsageStatisticsByItemGroup.h"
#include "dspUsageStatisticsByWarehouse.h"
#include "dspTimePhasedUsageStatisticsByItem.h"

#include "printItemLabelsByClassCode.h"

#include "warehouses.h"
#include "warehouse.h"
#include "locations.h"
#include "costCategories.h"
#include "expenseCategories.h"

#include "dspUnbalancedQOHByClassCode.h"
#include "updateABCClass.h"
#include "updateCycleCountFrequency.h"
#include "updateItemSiteLeadTimes.h"
#include "updateReorderLevelByItem.h"
#include "updateReorderLevels.h"
#include "updateReorderLevelsByClassCode.h"
#include "updateOUTLevelByItem.h"
#include "updateOUTLevels.h"
#include "updateOUTLevelsByClassCode.h"
#include "summarizeInvTransByClassCode.h"
#include "createItemSitesByClassCode.h"
#include "characteristics.h"

#include "menuInventory.h"

menuInventory::menuInventory(GUIClient *Pparent) :
  QObject(Pparent, "imModule")
{
  parent = Pparent;

  toolBar = new QToolBar(tr("Inventory Tools"));
  toolBar->setObjectName("Inventory Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowIMToolbar"))
    parent->addToolBar(toolBar);

  mainMenu = new QMenu(parent);
  itemSitesMenu	= new QMenu(parent);
  warehouseMenu = new QMenu(parent);
  transferOrderMenu = new QMenu(parent);
  transactionsMenu	= new QMenu(parent);
  lotSerialControlMenu	= new QMenu(parent);
  physicalMenu = new QMenu(parent);
  physicalCreateTagsMenu = new QMenu(parent);
  physicalReportsMenu = new QMenu(parent);
  physicalReportsTagsMenu = new QMenu(parent);
  shippingMenu = new QMenu(parent);
  shippingReportsMenu = new QMenu(parent);
  shippingFormsMenu = new QMenu(parent);
  receivingMenu = new QMenu(parent); 
  receivingFormsMenu = new QMenu(parent);
  formsMenu = new QMenu(parent);
  formsShipLabelsMenu = new QMenu(parent);
  reportsMenu		= new QMenu(parent);
  reportsItemsitesMenu = new QMenu(parent);
  reportsQohMenu = new QMenu(parent);
  reportsInvAvailMenu = new QMenu(parent);
  reportsInvBufrStsMenu = new QMenu(parent);
  reportsInvHistMenu = new QMenu(parent);
  reportsDtlInvHistMenu = new QMenu(parent);
  reportsItemUsgMenu = new QMenu(parent);
  reportsBacklogMenu		= new QMenu(parent);
  reportsShipmentsMenu		= new QMenu(parent);
  masterInfoMenu = new QMenu(parent);
  updateItemInfoMenu = new QMenu(parent);
  updateItemInfoReorderMenu = new QMenu(parent);
  updateItemInfoOutMenu = new QMenu(parent);
  utilitiesMenu	 = new QMenu(parent);
  
  mainMenu->setObjectName("menu.im");
  itemSitesMenu->setObjectName("menu.im.itemsites");
  warehouseMenu->setObjectName("menu.im.warehouse");
  transferOrderMenu->setObjectName("menu.im.transferorder");
  transactionsMenu->setObjectName("menu.im.transactions");
  lotSerialControlMenu->setObjectName("menu.im.lotserialcontrol");
  physicalMenu->setObjectName("menu.im.physical");
  physicalCreateTagsMenu->setObjectName("menu.im.physicalcreatetags");
  physicalReportsMenu->setObjectName("menu.im.physicalreports");
  physicalReportsTagsMenu->setObjectName("menu.im.physicalreportstags");
  shippingMenu->setObjectName("menu.im.shipping");
  shippingReportsMenu->setObjectName("menu.im.shippingreports");
  shippingFormsMenu->setObjectName("menu.im.shippingforms");
  receivingMenu->setObjectName("menu.im.receiving");
  receivingFormsMenu->setObjectName("menu.im.receivingforms");
  formsMenu->setObjectName("menu.im.forms");
  formsShipLabelsMenu->setObjectName("menu.im.formsshiplabels");
  reportsMenu->setObjectName("menu.im.reports");
  reportsItemsitesMenu->setObjectName("menu.im.reportsitemsites");
  reportsQohMenu->setObjectName("menu.im.reports.qoh");
  reportsInvAvailMenu->setObjectName("menu.im.reportsinvavail");
  reportsInvBufrStsMenu->setObjectName("menu.im.reportsinvbufrsts");
  reportsInvHistMenu->setObjectName("menu.im.reportsinvhist");
  reportsDtlInvHistMenu->setObjectName("menu.im.reportdtlinvhist");
  reportsItemUsgMenu->setObjectName("menu.im.reportsitemusg");
  reportsBacklogMenu->setObjectName("menu.im.reportsbacklog");
  reportsShipmentsMenu->setObjectName("menu.im.reportsshipments");
  masterInfoMenu->setObjectName("menu.im.masterinfo");
  updateItemInfoMenu->setObjectName("menu.im.updateiteminfo");
  updateItemInfoReorderMenu->setObjectName("menu.im.updateiteminforeorder");
  updateItemInfoOutMenu->setObjectName("menu.im.updateiteminfoout");
  utilitiesMenu->setObjectName("menu.im.utilities");
  
  //Handle single vs. multi-warehouse scenario
  QString whsModule;
  QString whsLabel;
  if (_metrics->boolean("MultiWhs"))
  {
    whsModule="im.warehouses";
    whsLabel=tr("&List...");
  }
  else
  {
    whsModule="im.warehouse";
    whsLabel=tr("&Maintain...");
  }

  actionProperties acts[] = {
    //  Inventory |  Transfer Orders
    // TODO: switch between visibility of Inter-Warehouse Transfer and Transfer Orders based on config param?
    { "menu",				tr("Transfer &Order"),	  (char*)transferOrderMenu,	mainMenu,	  true,	NULL, NULL, _metrics->boolean("MultiWhs"), NULL},
    { "im.interWarehouseTransfer",	tr("&New..."),	  SLOT(sNewTransferOrder()),	transferOrderMenu, _privileges->check("MaintainTransferOrders"),	NULL, NULL, _metrics->boolean("MultiWhs"), NULL }, 
    { "im.transferOrders",		tr("&List..."),	  SLOT(sTransferOrders()),	transferOrderMenu, (_privileges->check("ViewTransferOrders") || _privileges->check("MaintainTransferOrders")),	NULL, NULL, _metrics->boolean("MultiWhs"), NULL }, 

    //  Inventory | Physical Inventory
    { "menu",				tr("&Physical Inventory"),		   (char*)physicalMenu,			 mainMenu,	true,	NULL, NULL, true, NULL },
    
    //  Inventory | Physical Inventory | Create Count Tags
    { "menu",				tr("&Create Count Tags"),   (char*)physicalCreateTagsMenu,	 physicalMenu,      true,	NULL, NULL, true, NULL },
    { "im.createCountTagsByPlannerCode",tr("by &Planner Code..."),SLOT(sCreateCountTagsByPlannerCode()), physicalCreateTagsMenu, _privileges->check("IssueCountTags"),	NULL, NULL, true, NULL }, 
    { "im.createCountTagsByClassCode",	tr("by &Class Code..."),  SLOT(sCreateCountTagsByClassCode()),  physicalCreateTagsMenu, _privileges->check("IssueCountTags"),	NULL, NULL, true, NULL }, 
    { "im.createCountTagsByItem",	tr("by &Item..."),	   SLOT(sCreateCountTagsByItem()),	 physicalCreateTagsMenu, _privileges->check("IssueCountTags"),	NULL, NULL, true, NULL }, 
    { "im.createCycleCountTags", tr("Create C&ycle Count Tags..."),SLOT(sCreateCycleCountTags()), physicalMenu, _privileges->check("IssueCountTags"),	NULL, NULL, true, NULL }, 
    { "separator",			NULL,					   NULL,				 physicalMenu,	true, NULL, NULL, true, NULL},
    { "im.enterCountSlip",		tr("Enter Count &Slip..."),		   SLOT(sEnterCountSlip()), 		 physicalMenu, _privileges->check("EnterCountSlips"),	NULL, NULL, true, NULL }, 
    { "im.enterCountTag",		tr("Enter Count &Tag..."),		   SLOT(sEnterCountTags()), 		 physicalMenu, _privileges->check("EnterCountTags"),	NULL, NULL, true, NULL }, 
    { "im.enterMiscInventoryCount",	tr("Enter &Misc. Inventory Count..."),	   SLOT(sEnterMiscCount()), 		 physicalMenu, _privileges->check("EnterMiscCounts"),	NULL, NULL, true, NULL }, 
    { "im.zeroUncountedCountTags",	tr("&Zero Uncounted Count Tags..."),	   SLOT(sZeroUncountedTagsByWarehouse()),physicalMenu, _privileges->check("ZeroCountTags"),	NULL, NULL, true, NULL }, 
    { "separator",			NULL,					   NULL,				 physicalMenu,	true, NULL, NULL, true, NULL },
    { "im.thawItemSitesByClassCode",	tr("Tha&w Item Sites..."),	   SLOT(sThawItemSitesByClassCode()),	 physicalMenu, _privileges->check("ThawInventory"),	NULL, NULL, true, NULL }, 
    { "separator",			NULL,					   NULL,				 physicalMenu,	true, NULL, NULL, true, NULL },
    { "im.postCountSlips",		tr("Post Count S&lips..."),		   SLOT(sPostCountSlipsByWarehouse()),	 physicalMenu, _privileges->check("PostCountSlips"),	NULL, NULL, true, NULL }, 
    { "im.postCountTags",		tr("Post Count T&ags..."),		   SLOT(sPostCountTags()),		 physicalMenu, _privileges->check("PostCountTags"),	NULL, NULL, true, NULL }, 
    { "im.purgeCountSlips",		tr("&Purge Posted Count Slips..."),	   SLOT(sPurgePostedCountSlips()),	 physicalMenu, _privileges->check("PurgeCountSlips"),	NULL, NULL, true, NULL }, 
    { "im.purgeCountTags",		tr("P&urge Posted Count Tags..."),	   SLOT(sPurgePostedCountTags()),	 physicalMenu, _privileges->check("PurgeCountTags"),	NULL, NULL, true, NULL }, 
    { "separator",			NULL,					   NULL,				 physicalMenu,	true, NULL, NULL, true , NULL },	
    
    //  Inventory | Physical Inventory | Reports
    { "menu",				tr("&Reports"),		  (char*)physicalReportsMenu,	    physicalMenu,	  true,	NULL, NULL, true , NULL },
    { "im.dspFrozenItemSites",		tr("&Frozen Item Sites..."),	  SLOT(sDspFrozenItemSites()),	    physicalReportsMenu, _privileges->check("ViewItemSites"),	NULL, NULL, true, NULL }, 
    { "separator",			NULL,				  NULL,				    physicalReportsMenu, true, NULL, NULL, true , NULL },
    { "im.dspCountSlipEditList",	tr("Count S&lip Edit List..."),	  SLOT(sDspCountSlipEditList()),    physicalReportsMenu, _privileges->check("ViewCountTags"),	NULL, NULL, true, NULL }, 
    { "im.dspCountTagEditList",		tr("Count Ta&g Edit List..."),	  SLOT(sDspCountTagEditList()),	    physicalReportsMenu, _privileges->check("ViewCountTags"),	NULL, NULL, true, NULL }, 
    { "separator",			NULL,				  NULL,				    physicalReportsMenu, true, NULL, NULL, true , NULL },
    
    { "im.dspCountSlipsByWarehouse",	tr("Count &Slips..."),SLOT(sDspCountSlipsByWarehouse()),physicalReportsMenu, _privileges->check("ViewCountTags"),	NULL, NULL, true, NULL }, 
    
    //  Inventory | Physical Inventory | Reports | Count Tags
    { "menu",				tr("Count &Tags"),		  (char*)physicalReportsTagsMenu,	    physicalReportsMenu,	  _privileges->check("ViewCountTags"),	NULL, NULL, true , NULL },
    { "im.dspCountTagsByItem",		tr("by &Item..."),	  SLOT(sDspCountTagsByItem()),	    physicalReportsTagsMenu, _privileges->check("ViewCountTags"),	NULL, NULL, true, NULL }, 
    { "im.dspCountTagsByWarehouse",	tr("by &Warehouse..."), SLOT(sDspCountTagsByWarehouse()), physicalReportsTagsMenu, _privileges->check("ViewCountTags"),	NULL, NULL, true, NULL }, 
    { "im.dspCountTagsByClassCode",	tr("by &Class Code..."), SLOT(sDspCountTagsByClassCode()), physicalReportsTagsMenu, _privileges->check("ViewCountTags"),	NULL, NULL, true, NULL },     

    { "separator",			NULL,				  NULL,				mainMenu, true, NULL, NULL, true , NULL },

    //  Inventory | Receiving
    { "menu",	tr("R&eceiving"),  (char*)receivingMenu,	mainMenu,	true,	NULL, NULL, true , NULL },
    { "sr.enterReceipt", tr("&New Receipt..."), SLOT(sEnterReceipt()), receivingMenu, _privileges->check("EnterReceipts"), NULL, NULL, true, NULL },
    { "sr.postReceipts", tr("&List Unposted Receipts..."), SLOT(sPostReceipts()), receivingMenu, _privileges->check("EnterReceipts"), new QPixmap(":/images/postReceipts.png"), toolBar,  true, NULL },
    { "separator", NULL, NULL, receivingMenu, true, NULL, NULL, true, NULL },
    { "sr.enterReturn",  tr("Purchase Order &Return..."),  SLOT(sEnterReturn()),  receivingMenu, _privileges->check("EnterReturns"), NULL, NULL, true, NULL },
    { "separator",			NULL,				  NULL,				receivingMenu, true, NULL, NULL, true , NULL },

    //  Inventory | Receiving | Forms
    { "menu",	tr("&Forms"),  (char*)receivingFormsMenu,	receivingMenu,	true,	NULL, NULL, true , NULL },
    { "sr.printReceivingLabelsByPo", tr("&Print Receiving Labels..."), SLOT(sPrintReceivingLabelsByPo()), receivingFormsMenu, _privileges->check("ViewShipping"), NULL, NULL, true, NULL },

    //  Inventory | Shipping
    { "menu",	tr("&Shipping"),  (char*)shippingMenu,	mainMenu,	true,	NULL, NULL, true , NULL },
    { "sr.issueStockToShipping", tr("&Issue Stock to Shipping..."), SLOT(sIssueStockToShipping()), shippingMenu, _privileges->check("IssueStockToShipping"), new QPixmap(":/images/issueStockToShipping.png"), toolBar,  true, NULL },
    { "sr.maintainShippingContents", tr("&Maintain Shipping Contents..."), SLOT(sDspShippingContents()), shippingMenu, _privileges->check("ViewShipping"), NULL, NULL, true, NULL },
    { "separator", NULL, NULL, shippingMenu, true, NULL, NULL, true, NULL },
    { "sr.shipOrder", tr("&Ship Order..."), SLOT(sShipOrders()), shippingMenu, _privileges->check("ShipOrders"), NULL, NULL, true, NULL },
    { "sr.recallOrdersToShipping", tr("R&ecall Orders to Shipping..."), SLOT(sRecallOrders()), shippingMenu, _privileges->check("RecallOrders"), NULL, NULL, true, NULL },
    
    { "separator",					NULL,						 NULL,	shippingMenu,	true, NULL, NULL,  true , NULL },
    
    // Inventory | Shipping | Forms
    { "menu",				tr("&Forms"),				(char*)shippingFormsMenu,		    shippingMenu,	 true,					NULL, NULL, true , NULL },
    { "sr.packingListBatch", tr("Packing List &Batch..."), SLOT(sPackingListBatch()), shippingFormsMenu, (_privileges->check("MaintainPackingListBatch") || _privileges->check("ViewPackingListBatch")), NULL, NULL, true, NULL },
    { "sr.printPackingListBatchByShipvia", tr("Print Packing List Batch by Ship &Via..."), SLOT(sPrintPackingListBatchByShipvia()), shippingFormsMenu, _privileges->check("PrintPackingLists"), NULL, NULL, true, NULL },
    { "sr.printPackingList", tr("&Packing List..."), SLOT(sPrintPackingLists()), shippingFormsMenu, _privileges->check("PrintPackingLists"), NULL, NULL, true, NULL },
    { "separator", NULL, NULL, shippingFormsMenu, true, NULL, NULL, true, NULL },
    { "sr.printShippingForm", tr("&Shipping Form..."), SLOT(sPrintShippingForm()), shippingFormsMenu, _privileges->check("PrintBillsOfLading"), NULL, NULL, true, NULL },
    { "sr.printShippingForms", tr("S&hipping Forms..."), SLOT(sPrintShippingForms()), shippingFormsMenu, _privileges->check("PrintBillsOfLading"), NULL, NULL, true, NULL },
    
    // Inventory | Shipping | Forms | Shipping Labels
    { "menu",	tr("Shipping &Labels"),  (char*)formsShipLabelsMenu,	shippingFormsMenu,	true,	NULL, NULL, true , NULL },
    { "sr.printShippingLabelsBySo", tr("by &S/O #..."), SLOT(sPrintShippingLabelsBySo()), formsShipLabelsMenu, _privileges->check("ViewShipping"), NULL, NULL, true, NULL },
    { "sr.printShippingLabelsByTo", tr("by &T/O #..."), SLOT(sPrintShippingLabelsByTo()), formsShipLabelsMenu, _privileges->check("ViewTransferOrders"), NULL, NULL, true, NULL },
    { "sr.printShippingLabelsByInvoice", tr("by &Invoice..."), SLOT(sPrintShippingLabelsByInvoice()), formsShipLabelsMenu, _privileges->check("ViewShipping"), NULL, NULL, true, NULL },

    // Inventory | Shipping | Reports
    { "menu",				tr("&Reports"),				(char*)shippingReportsMenu,		    shippingMenu,	 true,					NULL, NULL, true , NULL },

    // Inventory | Shipping | Reports | Backlog
    { "menu",				tr("&Backlog"),				(char*)reportsBacklogMenu,		    shippingReportsMenu,	 true,					NULL, NULL, true , NULL },
    { "sr.dspBacklogByItem", tr("by &Item..."), SLOT(sDspBacklogByItem()), reportsBacklogMenu, _privileges->check("ViewSalesOrders"), NULL, NULL, true, NULL },
    { "sr.dspBacklogByCustomer", tr("by &Customer..."), SLOT(sDspBacklogByCustomer()), reportsBacklogMenu, _privileges->check("ViewSalesOrders"), NULL, NULL, true, NULL },
    { "sr.dspBacklogByProductCategory", tr("by &Product Category..."), SLOT(sDspBacklogByProductCategory()), reportsBacklogMenu, _privileges->check("ViewSalesOrders"), NULL, NULL, true, NULL },
    { "sr.dspSummarizedBacklogByWarehouse", tr("S&ummarized Backlog..."), SLOT(sDspSummarizedBacklogByWarehouse()), shippingReportsMenu, _privileges->check("ViewSalesOrders"), NULL, NULL, true, NULL },

    { "separator", NULL, NULL, shippingReportsMenu, true, NULL, NULL, true, NULL },
    
    // Inventory | Shipping | Reports | Shipments
    { "menu",				tr("&Shipments"),				(char*)reportsShipmentsMenu,		    shippingReportsMenu,	 true,					NULL, NULL, true , NULL },
    { "sr.dspShipmentsBySalesOrder", tr("by Sales &Order..."), SLOT(sDspShipmentsBySalesOrder()), reportsShipmentsMenu, _privileges->check("ViewShipping"), NULL, NULL, true, NULL },
    { "sr.dspShipmentsByDate", tr("by &Date..."), SLOT(sDspShipmentsByDate()), reportsShipmentsMenu, _privileges->check("ViewShipping"), NULL, NULL, true, NULL },
    { "sr.dspShipmentsByShipment", tr("by &Shipment..."), SLOT(sDspShipmentsByShipment()), reportsShipmentsMenu, _privileges->check("ViewShipping"), NULL, NULL, true, NULL },
    
    //  Inventory |  Transactions
    { "menu",				tr("&Transactions"),	  (char*)transactionsMenu,	mainMenu,	  true,	NULL, NULL, true	, NULL },
    { "im.miscellaneousAdjustment",	tr("&Adjustment..."),SLOT(sAdjustmentTrans()),	transactionsMenu, _privileges->check("CreateAdjustmentTrans"),	NULL, NULL, true, NULL }, 

    { "im.scrap",			tr("&Scrap..."),			  SLOT(sScrapTrans()),		transactionsMenu, _privileges->check("CreateScrapTrans"),	NULL, NULL, true, NULL }, 
    { "im.expense",			tr("E&xpense..."),		  SLOT(sExpenseTrans()),	transactionsMenu, _privileges->check("CreateExpenseTrans"),	NULL, NULL, true, NULL }, 
    { "im.materialReceipt",		tr("&Material Receipt..."),	  SLOT(sReceiptTrans()),	transactionsMenu, _privileges->check("CreateReceiptTrans"),	NULL, NULL, true, NULL }, 
    { "im.transform",			tr("Trans&form..."),		  SLOT(sTransformTrans()),	transactionsMenu, _privileges->check("CreateTransformTrans"),	NULL, NULL,  _metrics->boolean("Transforms"), NULL }, 
    { "separator",			NULL,				  NULL,				transactionsMenu, true, NULL, NULL, true , NULL },
    { "im.resetQOHBalances",		tr("&Reset QOH Balances..."),	  SLOT(sResetQOHBalances()),	transactionsMenu, _privileges->check("CreateAdjustmentTrans"),	NULL, NULL, true, NULL }, 
    { "separator",			NULL,				  NULL,				transactionsMenu, true, NULL, NULL, true , NULL },
    { "im.interWarehouseTransfer",	tr("&Warehouse Transfer..."),SLOT(sTransferTrans()),	transactionsMenu, _privileges->check("CreateInterWarehouseTrans"),	NULL, NULL, _metrics->boolean("MultiWhs"), NULL },
    { "im.relocateInventory",		tr("Re&locate Inventory..."),	  SLOT(sRelocateInventory()),	transactionsMenu, _privileges->check("RelocateInventory"),	NULL, NULL, true, NULL }, 
    
    {  "separator",			NULL,					  NULL,					mainMenu,	true, NULL, NULL, true , NULL },
    
    // Inventory | Forms
    { "menu",	tr("&Forms"),  (char*)formsMenu,	mainMenu,	true,	NULL, NULL, true , NULL },
    { "im.printItemLabelsByClassCode",			tr("Print &Item Labels..."),		SLOT(sPrintItemLabelsByClassCode()),		formsMenu, _privileges->check("ViewItemSites"),	NULL, NULL, true, NULL }, 

    //  Inventory | Reports
    { "menu",				tr("&Reports"),			  (char*)reportsMenu,			mainMenu,	true,	NULL, NULL, true , NULL },
    
    //  Inventory | Reports | Item Sites
    { "menu",				tr("&Item Sites"),			  (char*)reportsItemsitesMenu,			reportsMenu,	_privileges->check("ViewItemSites"),	NULL, NULL, true , NULL },
    { "im.dspItemSitesByPlannerCode",	tr("by &Planner Code..."),	  SLOT(sDspItemSitesByPlannerCode()),	reportsItemsitesMenu, _privileges->check("ViewItemSites"),	NULL, NULL, true, NULL }, 
    { "im.dspItemSitesByCostCategory",	tr("by C&ost Category..."),	  SLOT(sDspItemSitesByCostCategory()),	reportsItemsitesMenu, _privileges->check("ViewItemSites"),	NULL, NULL, true, NULL },
    { "im.dspItemSitesByClassCode",	tr("by &Class Code..."),	  SLOT(sDspItemSitesByClassCode()),	reportsItemsitesMenu, _privileges->check("ViewItemSites"),	NULL, NULL, true, NULL },  
    { "im.dspItemSitesByItem",		tr("by &Item..."),		  SLOT(sDspItemSitesByItem()),		reportsItemsitesMenu, _privileges->check("ViewItemSites"),	NULL, NULL, true, NULL }, 
    
    { "im.dspValidLocationsByItem",	tr("&Valid Locations by Item..."),	  SLOT(sDspValidLocationsByItem()),	reportsMenu, _privileges->check("ViewLocations"),	NULL, NULL, true, NULL }, 
    {  "separator",			NULL,					  NULL,					reportsMenu,	true, NULL, NULL, true , NULL },
    
    //  Inventory | Reports | Quantities On Hand
    { "menu",				tr("&Quantities On Hand"),			  (char*)reportsQohMenu,			reportsMenu,	 _privileges->check("ViewQOH"),	NULL, NULL, true , NULL },
    { "im.dspQOHByClassCode",		tr("by &Class Code..."),SLOT(sDspQOHByClassCode()),		reportsQohMenu, _privileges->check("ViewQOH"),	NULL, NULL, true, NULL }, 
    { "im.dspQOHByItemGroup",		tr("by Item &Group..."),SLOT(sDspQOHByItemGroup()),		reportsQohMenu, _privileges->check("ViewQOH"),	NULL, NULL, true, NULL },
    { "im.dspQOHByLocation",		tr("by &Location..."),  SLOT(sDspQOHByLocation()),		reportsQohMenu, _privileges->check("ViewQOH"),	NULL, NULL, true, NULL }, 
    { "im.dspQOHByItem",		tr("by &Item..."),	  SLOT(sDspQOHByItem()),		reportsQohMenu, _privileges->check("ViewQOH"),	NULL, NULL, true, NULL },  
    
    { "im.dspLocationLotSerialDetail",	tr("&Location/Lot/Serial # Detail..."),	  SLOT(sDspLocationLotSerialDetail()),	reportsMenu, _privileges->check("ViewQOH"),	NULL, NULL, _metrics->boolean("LotSerialControl"), NULL }, 
    { "im.dspExpiredInventory",		tr("&Expired Inventory..."),		  SLOT(sDspExpiredInventoryByClassCode()), reportsMenu, _privileges->check("ViewQOH"),	NULL, NULL, _metrics->boolean("LotSerialControl"), NULL }, 
    
    { "im.dspSlowMovingInventory",	tr("Slow &Moving Inventory..."),		  SLOT(sDspSlowMovingInventoryByClassCode()), reportsMenu, _privileges->check("ViewQOH"),	NULL, NULL, true, NULL }, 
    {  "separator",			NULL,					  NULL,					reportsMenu,	true, NULL, NULL, true , NULL },
    
    //  Inventory| Reports | Inventory Availability
    { "menu",				tr("Inventory &Availability"),			  (char*)reportsInvAvailMenu,			reportsMenu,	 _privileges->check("ViewInventoryAvailability"),	NULL, NULL, true , NULL },
    { "im.dspInventoryAvailabilityByPlannerCode",	tr("by &Planner Code..."), SLOT(sDspInventoryAvailabilityByPlannerCode()), reportsInvAvailMenu, _privileges->check("ViewInventoryAvailability"), new QPixmap(":/images/dspInventoryAvailabilityByPlannerCode.png"), toolBar, true , "Inventory Availability by Planner Code" },
    { "im.dspInventoryAvailabilityByClassCode",		tr("by &Class Code..."),	 SLOT(sDspInventoryAvailabilityByClassCode()), reportsInvAvailMenu, _privileges->check("ViewInventoryAvailability"),	NULL, NULL, true, NULL }, 
    { "im.dspInventoryAvailabilityBySourceVendor",	tr("by &Source Vendor..."),SLOT(sDspInventoryAvailabilityBySourceVendor()), reportsInvAvailMenu, _privileges->check("ViewInventoryAvailability"),	NULL, NULL, true, NULL },
    { "im.dspInventoryAvailabilityByItemGroup",		tr("by Item &Group..."),	 SLOT(sDspInventoryAvailabilityByItemGroup()), reportsInvAvailMenu, _privileges->check("ViewInventoryAvailability"),	NULL, NULL, true, NULL }, 
    { "im.dspInventoryAvailabilityByItem",		tr("by &Item..."),	 SLOT(sDspInventoryAvailabilityByItem()), reportsInvAvailMenu, _privileges->check("ViewInventoryAvailability"),	NULL, NULL, true, NULL },
    { "im.dspSubstituteAvailabilityByRootItem",		tr("&Substitute Availability..."),	 SLOT(sDspSubstituteAvailabilityByRootItem()), reportsMenu, _privileges->check("ViewInventoryAvailability"),	NULL, NULL, true, NULL }, 
    
    //  Inventory | Reports | Inventory Status
    { "menu",				tr("Inventory S&tatus"),			  (char*)reportsInvBufrStsMenu,			reportsMenu,	_privileges->check("ViewInventoryBufferStatus"),	NULL, NULL,  _metrics->boolean("BufferMgt") , NULL },
    { "im.dspInventoryBufferStatusByPlannerCode",	tr("by &Planner Code..."),SLOT(sDspInventoryBufferStatusByPlannerCode()), reportsInvBufrStsMenu, _privileges->check("ViewInventoryBufferStatus"),	NULL, NULL, _metrics->boolean("BufferMgt"), NULL },
    { "im.dspInventoryBufferStatusByClassCode",		tr("by &Class Code..."),	 SLOT(sDspInventoryBufferStatusByClassCode()), reportsInvBufrStsMenu, _privileges->check("ViewInventoryBufferStatus") ,	NULL, NULL, _metrics->boolean("BufferMgt"), NULL },  
    { "im.dspInventoryBufferStatusByItemGroup",		tr("by &Item Group..."),	 SLOT(sDspInventoryBufferStatusByItemGroup()), reportsInvBufrStsMenu, _privileges->check("ViewInventoryBufferStatus"),	NULL, NULL, _metrics->boolean("BufferMgt"), NULL }, 

    { "separator",					NULL,						 NULL,	reportsMenu,	true, NULL, NULL,  true , NULL },
    
    //  Inventory| Reports | Inventory History
    { "menu",				tr("Inventory &History"),			  (char*)reportsInvHistMenu,			reportsMenu,	_privileges->check("ViewInventoryHistory"),	NULL, NULL, true , NULL },
    { "im.dspInventoryHistoryByPlannerCode",		tr("by &Planner Code..."),	 SLOT(sDspInventoryHistoryByPlannerCode()), reportsInvHistMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, true, NULL },
    { "im.dspInventoryHistoryByClassCode",		tr("by &Class Code..."),	 SLOT(sDspInventoryHistoryByClassCode()), reportsInvHistMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, true, NULL }, 
    { "im.dspInventoryHistoryByOrderNumber",		tr("by &Order Number..."),	 SLOT(sDspInventoryHistoryByOrderNumber()), reportsInvHistMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, true, NULL }, 
    { "im.dspInventoryHistoryByItemGroup",		tr("by Item &Group..."),	 SLOT(sDspInventoryHistoryByItemGroup()), reportsInvHistMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, true, NULL },
    { "im.dspInventoryHistoryByItem",			tr("by &Item..."),		 SLOT(sDspInventoryHistoryByItem()), reportsInvHistMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, true, NULL }, 
    
    //  Inventory | Reports | Detailed Inventory History
    { "menu",				tr("&Detailed Inventory History"),			  (char*)reportsDtlInvHistMenu,			reportsMenu,	 _privileges->check("ViewInventoryHistory"),	NULL, NULL, true , NULL },
    { "im.dspDetailedInventoryHistoryByLocation",	tr("by Lo&cation..."),	SLOT(sDspDetailedInventoryHistoryByLocation()), reportsDtlInvHistMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, true, NULL }, 
    { "im.dspDetailedInventoryHistoryByLot/SerialNumber",tr("by &Lot/Serial #..."),	SLOT(sDspDetailedInventoryHistoryByLotSerial()), reportsDtlInvHistMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, _metrics->boolean("LotSerialControl"), NULL }, 

    //  Inventory | Reports | Usage Statistics
    { "menu",				tr("&Usage Statistics "),			  (char*)reportsItemUsgMenu,			reportsMenu,	 _privileges->check("ViewInventoryHistory"),	NULL, NULL, true , NULL },
    { "im.dspItemUsageStatisticsByWarehouse",		tr("by &Warehouse..."),		SLOT(sDspItemUsageStatisticsByWarehouse()), reportsItemUsgMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, true, NULL }, 
    { "im.dspItemUsageStatisticsByClassCode",		tr("by &Class Code..."),		SLOT(sDspItemUsageStatisticsByClassCode()), reportsItemUsgMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, true, NULL },
    { "im.dspItemUsageStatisticsByItemGroup",		tr("by Item &Group..."),		SLOT(sDspItemUsageStatisticsByItemGroup()), reportsItemUsgMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, true, NULL }, 
    { "im.dspItemUsageStatisticsByItem",		tr("by &Item..."),			SLOT(sDspItemUsageStatisticsByItem()), reportsItemUsgMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, true, NULL }, 
        
    { "im.dspTimePhasedItemUsageStatisticsByItem",	tr("Time &Phased Usage Statistics..."),	SLOT(sDspTimePhasedUsageStatisticsByItem()), reportsMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, true, NULL }, 

    { "separator",			NULL,				  NULL,				mainMenu, true, NULL, NULL, true , NULL },

    // Inventory | Warehouse
    { "menu",			tr("&Warehouse"),	(char*)warehouseMenu,	     mainMenu,	     true,												NULL, NULL, true , NULL },
    { whsModule,		whsLabel,		SLOT(sWarehouses()),	     warehouseMenu, (_privileges->check("MaintainWarehouses")) || (_privileges->check("ViewWarehouses") ),		NULL, NULL, true, NULL }, 
    { "im.warehousesLocations",	tr("L&ocations..."),	SLOT(sWarehouseLocations()), warehouseMenu, (_privileges->check("MaintainLocations")) || (_privileges->check("ViewLocations") ),			NULL, NULL, true, NULL }, 

    // Inventory | Item Site
    { "menu",				tr("&Item Site"),             		(char*)itemSitesMenu,	mainMenu,	true,	NULL, NULL, true	, NULL },
    { "im.newItemSite",			tr("&New..."),		SLOT(sNewItemSite()),	itemSitesMenu,	_privileges->check("MaintainItemSites"), NULL, NULL, true , NULL },
    { "im.listItemSites",		tr("&List..."),		SLOT(sItemSites()),	itemSitesMenu,	(_privileges->check("MaintainItemSites") || _privileges->check("ViewItemSites")), new QPixmap(":/images/itemSites.png"), toolBar, true , "List Item Sites" },							  
    { "separator", NULL, NULL, itemSitesMenu,	true, NULL, NULL, true , NULL },
    { "im.itemAvailabilityWorkbench",	tr("&Workbench..."),	SLOT(sDspItemAvailabilityWorkbench()),	itemSitesMenu, _privileges->check("ViewItemAvailabilityWorkbench"), new QPixmap(":/images/itemAvailabilityWorkbench.png"), toolBar, true , "Item Availability Workbench"},
    
    //  Inventory | Lot/Serial Control
    { "menu",				tr("&Lot/Serial Control"),	(char*)lotSerialControlMenu,	mainMenu, true,	NULL, NULL, _metrics->boolean("LotSerialControl") , NULL },
    { "im.dspLocationLotSerialDetail",	tr("&Location Detail..."),	SLOT(sDspLocationLotSerialDetail()), lotSerialControlMenu, _privileges->check("ViewQOH"),	NULL, NULL,  _metrics->boolean("LotSerialControl"), NULL }, 
    { "im.dspDetailedInventoryHistoryByLot/SerialNumber", tr("&Detailed Inventory History..."), SLOT(sDspDetailedInventoryHistoryByLotSerial()), lotSerialControlMenu, _privileges->check("ViewInventoryHistory"),	NULL, NULL, _metrics->boolean("LotSerialControl"), NULL }, 
    { "separator",			NULL,				NULL,	lotSerialControlMenu,	true, NULL, NULL,  _metrics->boolean("LotSerialControl") , NULL },

    { "im.reassignLotSerialNumber",	tr("&Reassign Lot/Serial #..."),	SLOT(sReassignLotSerialNumber()), lotSerialControlMenu, _privileges->check("ReassignLotSerial"),	NULL, NULL, _metrics->boolean("LotSerialControl"), NULL }, 
    
    { "separator", NULL, NULL, mainMenu,	true, NULL, NULL, true , NULL },
    
    // Inventory | Master Information
    { "menu",			tr("&Master Information"),	(char*)masterInfoMenu,	     mainMenu,	     true,												NULL, NULL, true , NULL },
    { "im.costCategories",	tr("&Cost Categories..."),	SLOT(sCostCategories()),     masterInfoMenu, (_privileges->check("MaintainCostCategories")) || (_privileges->check("ViewCostCategories") ),	NULL, NULL, true, NULL }, 
    { "im.expenseCategories",	tr("&Expense Categories..."),	SLOT(sExpenseCategories()),  masterInfoMenu, (_privileges->check("MaintainExpenseCategories")) || (_privileges->check("ViewExpenseCategories") ),	NULL, NULL, true, NULL },
    { "im.characteristics",	tr("C&haracteristics..."),	SLOT(sCharacteristics()),    masterInfoMenu, (_privileges->check("MaintainCharacteristics") || _privileges->check("ViewCharacteristics") ),	NULL, NULL, true, NULL },

    // Inventory | Utilities
    { "menu",					  tr("&Utilities"),			 		(char*)utilitiesMenu,			mainMenu,	true,							NULL, NULL, true, NULL },
    { "im.dspUnbalancedQOHByClassCode",		  tr("U&nbalanced QOH..."), 		SLOT(sDspUnbalancedQOHByClassCode()),	utilitiesMenu, _privileges->check("ViewItemSites"),			NULL, NULL, true, NULL },
    { "separator",				  NULL,					 		NULL,					utilitiesMenu,	true, 							NULL, NULL, true, NULL },
     
    // Inventory | Utilities | Update Item Controls
    { "menu",					tr("&Update Item Controls"),			(char*)updateItemInfoMenu,			utilitiesMenu,	    true,					NULL, NULL, true, NULL },
    { "im.updateABCClass",			tr("&ABC Class..."),				SLOT(sUpdateABCClass()), 			updateItemInfoMenu, _privileges->check("UpdateABCClass"),	NULL, NULL, true, NULL },
    { "im.updateCycleCountFrequency",		tr("&Cycle Count Frequency..."),			SLOT(sUpdateCycleCountFreq()),			updateItemInfoMenu, _privileges->check("UpdateCycleCountFreq"),	NULL, NULL, true, NULL },
    { "im.updateItemSiteLeadTimes",		tr("&Item Site Lead Times..."),			SLOT(sUpdateItemSiteLeadTimes()),		updateItemInfoMenu, _privileges->check("UpdateLeadTime"),	NULL, NULL, true, NULL },

    //  Inventory | Utilities | Update Item Controls | Update Reorder Levels
    { "menu",				tr("&Reorder Levels"),			  (char*)updateItemInfoReorderMenu,			updateItemInfoMenu,	 _privileges->check("UpdateReorderLevels"),	NULL, NULL, true , NULL },
    { "im.updateReorderLevelsByItem",		tr("by &Item..."),		SLOT(sUpdateReorderLevelByItem()),		updateItemInfoReorderMenu, _privileges->check("UpdateReorderLevels"),	NULL, NULL, true, NULL },
    { "im.updateReorderLevelsByPlannerCode",	tr("by &Planner Code..."),	SLOT(sUpdateReorderLevelsByPlannerCode()),	updateItemInfoReorderMenu, _privileges->check("UpdateReorderLevels"),	NULL, NULL, true, NULL },
    { "im.updateReorderLevelsByClassCode",	tr("by &Class Code..."), 		SLOT(sUpdateReorderLevelsByClassCode()),	updateItemInfoReorderMenu, _privileges->check("UpdateReorderLevels"),	NULL, NULL, true, NULL },

    //  Inventory | Utilities | Update Item Controls | Update Order Up To Levels
    { "menu",				tr("&Order Up To Levels"),			  (char*)updateItemInfoOutMenu,			updateItemInfoMenu,	_privileges->check("UpdateOUTLevels"),	NULL, NULL, true , NULL },
    { "im.updateOrderUpToLevelsByItem",		tr("by &Item..."),		SLOT(sUpdateOUTLevelByItem()), 			updateItemInfoOutMenu, _privileges->check("UpdateOUTLevels"),	NULL, NULL, true, NULL },
    { "im.updateOrderUpToLevelsByPlannerCode",	tr("by &Planner Code..."),	SLOT(sUpdateOUTLevelsByPlannerCode()),		updateItemInfoOutMenu, _privileges->check("UpdateOUTLevels"),	NULL, NULL, true, NULL },
    { "im.updateOrderUpToLevelsByClassCode",	tr("by &Class Code..."),	SLOT(sUpdateOUTLevelsByClassCode()),		updateItemInfoOutMenu, _privileges->check("UpdateOUTLevels"),	NULL, NULL, true, NULL },
    
    { "im.summarizeTransactionHistoryByClassCode",tr("Summarize &Transaction History..."), SLOT(sSummarizeInvTransByClassCode()),	utilitiesMenu, _privileges->check("SummarizeInventoryTransactions"),	NULL, NULL, true, NULL },
    { "im.createItemSitesByClassCode",		  tr("&Create Item Sites..."),		SLOT(sCreateItemSitesByClassCode()),	utilitiesMenu, _privileges->check("MaintainItemSites"),			NULL, NULL, true, NULL },
    { "separator", NULL, NULL, utilitiesMenu, true, NULL, NULL, true, NULL },
    { "sr.purgeShippingRecords", tr("&Purge Shipping Records..."), SLOT(sPurgeShippingRecords()), utilitiesMenu, _privileges->check("PurgeShippingRecords"), NULL, NULL, true, NULL },
    { "sr.externalShipping",     tr("Maintain E&xternal Shipping Records..."), SLOT(sExternalShipping()), utilitiesMenu, externalShipping::userHasPriv(), NULL, NULL, true, NULL }
  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));



#if 0
  mainMenu->insertItem(tr("&Graphs"),                 graphsMenu           );
#endif

  parent->populateCustomMenu(mainMenu, "Inventory");
  parent->menuBar()->insertItem(tr("&Inventory"), mainMenu);

//  Create connections to the module specific inputManager SIGNALS
  parent->inputManager()->notify(cBCLocationContents, this, this, SLOT(sCatchLocationContents(int)));
  parent->inputManager()->notify(cBCCountTag, this, this, SLOT(sCatchCountTag(int)));
}

void menuInventory::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  for (unsigned int i = 0; i < numElems; i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == QString("menu"))
    {
      acts[i].menu->insertItem(acts[i].actionTitle, (QMenu*)(acts[i].slot));
    }
    else if (acts[i].actionName == QString("separator"))
    {
      acts[i].menu->addSeparator();
    }
    else if ((acts[i].toolBar != NULL) && (acts[i].toolBar != NULL))
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv,
					  *(acts[i].pixmap),
					  acts[i].toolBar,
                      acts[i].toolTip) );
    }
    else if (acts[i].toolBar != NULL)
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv,
					  *(acts[i].pixmap),
					  acts[i].toolBar,
                      acts[i].actionTitle) );
    }
    else
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv ) );
    }
  }
}

void menuInventory::sNewItemSite()
{
  ParameterList params;
  params.append("mode", "new");

  itemSite newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sItemSites()
{
  omfgThis->handleNewWindow(new itemSites());
}

void menuInventory::sAdjustmentTrans()
{
  ParameterList params;
  params.append("mode", "new");

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sTransferTrans()
{
  ParameterList params;
  params.append("mode", "new");

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sNewTransferOrder()
{
  transferOrder::newTransferOrder(-1, -1);
}

void menuInventory::sTransferOrders()
{
  omfgThis->handleNewWindow(new transferOrders());
}

void menuInventory::sReceiptTrans()
{
  ParameterList params;
  params.append("mode", "new");

  materialReceiptTrans *newdlg = new materialReceiptTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sTransformTrans()
{
  ParameterList params;
  params.append("mode", "new");

  transformTrans *newdlg = new transformTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sScrapTrans()
{
  ParameterList params;
  params.append("mode", "new");

  scrapTrans *newdlg = new scrapTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sExpenseTrans()
{
  ParameterList params;
  params.append("mode", "new");

  expenseTrans *newdlg = new expenseTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sResetQOHBalances()
{
  resetQOHBalances(parent, "", TRUE).exec();
}

void menuInventory::sRelocateInventory()
{
  relocateInventory(parent, "", TRUE).exec();
}


//  Lot/Serial Control
void menuInventory::sReassignLotSerialNumber()
{
  reassignLotSerial newdlg(parent, "", TRUE);
  newdlg.exec();
}


void menuInventory::sCreateCountTagsByClassCode()
{
  ParameterList params;
  params.append("classcode");

  createCountTagsByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sCreateCountTagsByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  createCountTagsByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sCreateCountTagsByItem()
{
  createCountTagsByItem(parent, "", TRUE).exec();
}

void menuInventory::sCreateCycleCountTags()
{
  createCycleCountTags(parent, "", TRUE).exec();
}

void menuInventory::sEnterCountSlip()
{
  ParameterList params;
  params.append("mode", "new");

  countSlip newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sEnterCountTags()
{
  ParameterList params;
  params.append("mode", "new");

  countTag newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sEnterMiscCount()
{
  enterMiscCount(parent, "", TRUE).exec();
}

void menuInventory::sZeroUncountedTagsByWarehouse()
{
  zeroUncountedCountTagsByWarehouse(parent, "", TRUE).exec();
}

void menuInventory::sThawItemSitesByClassCode()
{
  thawItemSitesByClassCode(parent, "", TRUE).exec();
}

void menuInventory::sPostCountSlipsByWarehouse()
{
  postCountSlips(parent, "", TRUE).exec();
}

void menuInventory::sPostCountTags()
{
  postCountTags(parent, "", TRUE).exec();
}

void menuInventory::sPurgePostedCountSlips()
{
  purgePostedCountSlips(parent, "", TRUE).exec();
}

void menuInventory::sPurgePostedCountTags()
{
  purgePostedCounts(parent, "", TRUE).exec();
}


void menuInventory::sPackingListBatch()
{
  omfgThis->handleNewWindow(new packingListBatch());
}

void menuInventory::sDspShippingContents()
{
  omfgThis->handleNewWindow(new maintainShipping());
}

void menuInventory::sIssueStockToShipping()
{
  omfgThis->handleNewWindow(new issueToShipping());
}

void menuInventory::sPrintPackingLists()
{
  printPackingList(parent, "", TRUE).exec();
}

void menuInventory::sPrintPackingListBatchByShipvia()
{
  printPackingListBatchByShipvia(parent, "", TRUE).exec();
}

void menuInventory::sPrintShippingForm()
{
  printShippingForm(parent, "", TRUE).exec();
}

void menuInventory::sPrintShippingForms()
{
  printShippingForms(parent, "", TRUE).exec();
}

void menuInventory::sPrintShippingLabelsBySo()
{
  printLabelsBySo(parent, "", TRUE).exec();
}

void menuInventory::sPrintShippingLabelsByTo()
{
  printLabelsByTo(parent, "", TRUE).exec();
}

void menuInventory::sPrintShippingLabelsByInvoice()
{
  printLabelsByInvoice(parent, "", TRUE).exec();
}

void menuInventory::sPrintReceivingLabelsByPo()
{
  printLabelsByOrder(parent, "", TRUE).exec();
}


void menuInventory::sShipOrders()
{
  shipOrder(parent, "", TRUE).exec();
}

void menuInventory::sRecallOrders()
{
  omfgThis->handleNewWindow(new recallOrders());
}

void menuInventory::sPurgeShippingRecords()
{
  purgeShippingRecords(parent, "", TRUE).exec();
}

void menuInventory::sEnterReceipt()
{
  ParameterList params;
  params.append("mode", "new");

  enterPoReceipt *newdlg = new enterPoReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sEnterReturn()
{
  ParameterList params;
  params.append("mode", "new");

  enterPoReturn *newdlg = new enterPoReturn();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sPostReceipts()
{
  //ParameterList params;

  unpostedPoReceipts *newdlg = new unpostedPoReceipts();
  //newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDestinations()
{
  omfgThis->handleNewWindow(new destinations());
}

void menuInventory::sCarriers()
{
  omfgThis->handleNewWindow(new carriers());
}

void menuInventory::sAddRate()
{
}

void menuInventory::sDspRatesByDestination()
{
}

void menuInventory::sDspRatesByCarrier()
{
}

void menuInventory::sDspBacklogByItem()
{
  omfgThis->handleNewWindow(new dspBacklogByItem());
}

void menuInventory::sDspBacklogByCustomer()
{
  omfgThis->handleNewWindow(new dspBacklogByCustomer());
}

void menuInventory::sDspBacklogByProductCategory()
{
  ParameterList params;
  params.append("prodcat");

  dspBacklogByParameterList *newdlg = new dspBacklogByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspSummarizedBacklogByWarehouse()
{
  omfgThis->handleNewWindow(new dspSummarizedBacklogByWarehouse());
}

void menuInventory::sDspShipmentsBySalesOrder()
{
  omfgThis->handleNewWindow(new dspShipmentsBySalesOrder());
}

void menuInventory::sDspShipmentsByDate()
{
  omfgThis->handleNewWindow(new dspShipmentsByDate());
}

void menuInventory::sDspShipmentsByShipment()
{
  omfgThis->handleNewWindow(new dspShipmentsByShipment());
}


void menuInventory::sDspFrozenItemSites()
{
  omfgThis->handleNewWindow(new dspFrozenItemSites());
}

void menuInventory::sDspCountSlipEditList()
{
  omfgThis->handleNewWindow(new dspCountSlipEditList());
}

void menuInventory::sDspCountTagEditList()
{
  omfgThis->handleNewWindow(new dspCountTagEditList());
}

void menuInventory::sDspCountSlipsByWarehouse()
{
  omfgThis->handleNewWindow(new dspCountSlipsByWarehouse());
}

void menuInventory::sDspCountTagsByItem()
{
  omfgThis->handleNewWindow(new dspCountTagsByItem());
}

void menuInventory::sDspCountTagsByWarehouse()
{
  omfgThis->handleNewWindow(new dspCountTagsByWarehouse());
}

void menuInventory::sDspCountTagsByClassCode()
{
  omfgThis->handleNewWindow(new dspCountTagsByClassCode());
}

void menuInventory::sDspItemAvailabilityWorkbench()
{
  omfgThis->handleNewWindow(new itemAvailabilityWorkbench());
}

void menuInventory::sDspItemSitesByItem()
{
  omfgThis->handleNewWindow(new dspItemSitesByItem());
}

void menuInventory::sDspItemSitesByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspItemSitesByParameterList *newdlg = new dspItemSitesByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspItemSitesByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspItemSitesByParameterList *newdlg = new dspItemSitesByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspItemSitesByCostCategory()
{
  ParameterList params;
  params.append("costcat");

  dspItemSitesByParameterList *newdlg = new dspItemSitesByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspValidLocationsByItem()
{
  omfgThis->handleNewWindow(new dspValidLocationsByItem());
}

void menuInventory::sDspQOHByItem()
{
  omfgThis->handleNewWindow(new dspQOHByItem());
}

void menuInventory::sDspQOHByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspQOHByParameterList *newdlg = new dspQOHByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspQOHByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspQOHByParameterList *newdlg = new dspQOHByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspQOHByLocation()
{
  omfgThis->handleNewWindow(new dspQOHByLocation());
}

void menuInventory::sDspLocationLotSerialDetail()
{
  omfgThis->handleNewWindow(new dspInventoryLocator());
}

void menuInventory::sDspSlowMovingInventoryByClassCode()
{
  omfgThis->handleNewWindow(new dspSlowMovingInventoryByClassCode());
}

void menuInventory::sDspExpiredInventoryByClassCode()
{
  omfgThis->handleNewWindow(new dspExpiredInventoryByClassCode());
}

void menuInventory::sDspInventoryAvailabilityByItem()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityByItem());
}

void menuInventory::sDspInventoryAvailabilityByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspInventoryAvailabilityByParameterList *newdlg = new dspInventoryAvailabilityByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspInventoryAvailabilityByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspInventoryAvailabilityByParameterList *newdlg = new dspInventoryAvailabilityByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspInventoryAvailabilityByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspInventoryAvailabilityByParameterList *newdlg = new dspInventoryAvailabilityByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspInventoryAvailabilityBySourceVendor()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityBySourceVendor());
}

void menuInventory::sDspSubstituteAvailabilityByRootItem()
{
  omfgThis->handleNewWindow(new dspSubstituteAvailabilityByItem());
}

void menuInventory::sDspInventoryBufferStatusByItem()
{
  //omfgThis->handleNewWindow(new dspInventoryBufferStatusByItem());
}

void menuInventory::sDspInventoryBufferStatusByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspInventoryBufferStatusByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspInventoryBufferStatusByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspInventoryHistoryByItem()
{
  omfgThis->handleNewWindow(new dspInventoryHistoryByItem());
}

void menuInventory::sDspInventoryHistoryByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspInventoryHistoryByParameterList *newdlg = new dspInventoryHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspInventoryHistoryByOrderNumber()
{
  omfgThis->handleNewWindow(new dspInventoryHistoryByOrderNumber());
}

void menuInventory::sDspInventoryHistoryByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspInventoryHistoryByParameterList *newdlg = new dspInventoryHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspInventoryHistoryByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspInventoryHistoryByParameterList *newdlg = new dspInventoryHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sDspDetailedInventoryHistoryByLotSerial()
{
  omfgThis->handleNewWindow(new dspDetailedInventoryHistoryByLotSerial());
}

void menuInventory::sDspDetailedInventoryHistoryByLocation()
{
  omfgThis->handleNewWindow(new dspDetailedInventoryHistoryByLocation());
}

void menuInventory::sDspItemUsageStatisticsByItem()
{
  omfgThis->handleNewWindow(new dspUsageStatisticsByItem());
}

void menuInventory::sDspItemUsageStatisticsByClassCode()
{
  omfgThis->handleNewWindow(new dspUsageStatisticsByClassCode());
}

void menuInventory::sDspItemUsageStatisticsByItemGroup()
{
  omfgThis->handleNewWindow(new dspUsageStatisticsByItemGroup());
}

void menuInventory::sDspItemUsageStatisticsByWarehouse()
{
  omfgThis->handleNewWindow(new dspUsageStatisticsByWarehouse());
}

void menuInventory::sDspTimePhasedUsageStatisticsByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedUsageStatisticsByItem());
}



void menuInventory::sPrintItemLabelsByClassCode()
{
  printItemLabelsByClassCode(parent, "", TRUE).exec();
}


//  Master Information
void menuInventory::sWarehouses()
{
  if (_metrics->boolean("MultiWhs"))
    omfgThis->handleNewWindow(new warehouses());
  else
  {
    omfgThis->handleNewWindow(new warehouse());
  }
}

void menuInventory::sWarehouseLocations()
{
  omfgThis->handleNewWindow(new locations());
}

void menuInventory::sCostCategories()
{
  omfgThis->handleNewWindow(new costCategories());
}

void menuInventory::sExpenseCategories()
{
  omfgThis->handleNewWindow(new expenseCategories());
}

//  Utilities
void menuInventory::sDspUnbalancedQOHByClassCode()
{
  omfgThis->handleNewWindow(new dspUnbalancedQOHByClassCode());
}

void menuInventory::sUpdateABCClass()
{
  updateABCClass(parent, "", TRUE).exec();
}

void menuInventory::sUpdateCycleCountFreq()
{
  updateCycleCountFrequency(parent, "", TRUE).exec();
}

void menuInventory::sUpdateItemSiteLeadTimes()
{
  updateItemSiteLeadTimes(parent, "", TRUE).exec();
}

void menuInventory::sUpdateReorderLevelByItem()
{
  updateReorderLevelByItem(parent, "", TRUE).exec();
}

void menuInventory::sUpdateReorderLevelsByPlannerCode()
{
  updateReorderLevels(parent, "", TRUE).exec();
}

void menuInventory::sUpdateReorderLevelsByClassCode()
{
  updateReorderLevelsByClassCode(parent, "", TRUE).exec();
}

void menuInventory::sUpdateOUTLevelByItem()
{
  updateOUTLevelByItem(parent, "", TRUE).exec();
}

void menuInventory::sUpdateOUTLevelsByPlannerCode()
{
  updateOUTLevels(parent, "", TRUE).exec();
}

void menuInventory::sUpdateOUTLevelsByClassCode()
{
  updateOUTLevelsByClassCode(parent, "", TRUE).exec();
}

void menuInventory::sSummarizeInvTransByClassCode()
{
  summarizeInvTransByClassCode(parent, "", TRUE).exec();
}

void menuInventory::sCreateItemSitesByClassCode()
{
  createItemSitesByClassCode(parent, "", TRUE).exec();
}

//  inputManager SIGNAL handlers
void menuInventory::sCatchLocationContents(int pLocationid)
{
  ParameterList params;
  params.append("location_id", pLocationid);
  params.append("run");

  dspQOHByLocation *newdlg = new dspQOHByLocation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sCatchCountTag(int pCnttagid)
{
  ParameterList params;
  params.append("cnttag_id", pCnttagid);
  params.append("mode", "edit");

  countTag newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sCharacteristics()
{
  omfgThis->handleNewWindow(new characteristics());
}

void menuInventory::sExternalShipping()
{
  externalShipping newdlg(parent, "", TRUE);
  newdlg.exec();
}
