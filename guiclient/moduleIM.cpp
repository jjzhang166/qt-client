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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

//  moduleIM.cpp
//  Created 08/22/2000, JSL
//  Copyright (c) 2000-2007, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QPixmap>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "OpenMFGGUIClient.h"
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

#include "lotSerialHistory.h"
#include "lotSerialComments.h"
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

#include "dspFrozenItemSites.h"
#include "dspCountSlipEditList.h"
#include "dspCountTagEditList.h"
#include "dspCountSlipsByWarehouse.h"
#include "dspCountTagsByItem.h"
#include "dspCountTagsByWarehouse.h"
#include "dspCountTagsByClassCode.h"

#include "rptFrozenItemSites.h"
#include "rptCountSlipEditList.h"
#include "rptCountTagEditList.h"
#include "rptCountSlipsByWarehouse.h"
#include "rptCountTagsByItem.h"
#include "rptCountTagsByWarehouse.h"
#include "rptCountTagsByClassCode.h"

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
//#include "dspInventoryBufferStatusByItem.h"
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

#include "rptItemSitesByItem.h"
#include "rptItemSitesByParameterList.h"
#include "rptValidLocationsByItem.h"
#include "rptQOHByItem.h"
#include "rptQOHByParameterList.h"
#include "rptQOHByLocation.h"
#include "rptInventoryLocator.h"
#include "rptSlowMovingInventoryByClassCode.h"
#include "rptExpiredInventoryByClassCode.h"
#include "rptInventoryAvailabilityByItem.h"
#include "rptInventoryAvailabilityByParameterList.h"
#include "rptInventoryAvailabilityBySourceVendor.h"
#include "rptSubstituteAvailabilityByRootItem.h"
//#include "rptInventoryBufferStatusByItem.h"
#include "rptInventoryBufferStatusByParameterList.h"
#include "rptInventoryHistoryByItem.h"
#include "rptInventoryHistoryByOrderNumber.h"
#include "rptInventoryHistoryByParameterList.h"
#include "rptDetailedInventoryHistoryByLotSerial.h"
#include "rptDetailedInventoryHistoryByLocation.h"
#include "rptUsageStatisticsByItem.h"
#include "rptUsageStatisticsByClassCode.h"
#include "rptUsageStatisticsByItemGroup.h"
#include "rptUsageStatisticsByWarehouse.h"
#include "rptTimePhasedUsageStatisticsByItem.h"
#include "rptLocationDispatchList.h"
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

#include "moduleIM.h"

// TODO: change to new menuing system code
moduleIM::moduleIM(OpenMFGGUIClient *Pparent) :
  QObject(Pparent, "imModule")
{
  parent = Pparent;

  toolBar = new QToolBar(tr("I/M Tools"));
  toolBar->setObjectName("I/M Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowIMToolbar"))
    parent->addToolBar(toolBar);

  mainMenu		= new QMenu();
  itemSitesMenu		= new QMenu();
  transactionsMenu	= new QMenu();
  lotSerialControlMenu	= new QMenu();
  physicalDisplaysMenu	= new QMenu();
  physicalReportsMenu	= new QMenu();
  physicalMenu		= new QMenu();
  displaysMenu		= new QMenu();
  reportsMenu		= new QMenu();
  masterInfoMenu	= new QMenu();
  updateItemInfoMenu	= new QMenu();
  utilitiesMenu		= new QMenu();
  
  //Handle single vs. multi-warehouse scenario
  QString whsModule;
  QString whsLabel;
  if (_metrics->boolean("MultiWhs"))
  {
    whsModule="im.warehouses";
    whsLabel=tr("Warehouses...");
  }
  else
  {
    whsModule="im.warehouse";
    whsLabel=tr("Warehouse...");
  }

  actionProperties acts[] = {
    { "menu",				tr("&Item Sites"),             		(char*)itemSitesMenu,	mainMenu,	true,	NULL, NULL, true	},
    { "im.newItemSite",			tr("Enter New Item Site..."),		SLOT(sNewItemSite()),	itemSitesMenu,	_privleges->check("MaintainItemSites"), NULL, NULL, true },
    { "im.listItemSites",		tr("List Item Sites..."),		SLOT(sItemSites()),	itemSitesMenu,	(_privleges->check("MaintainItemSites") || _privleges->check("ViewItemSites")), new QPixmap(":/images/itemSites.png"), toolBar, true },
    { "separator",			NULL,					NULL,			itemSitesMenu,	true, NULL, NULL, true },
    { "im.itemAvailabilityWorkbench",	tr("Item Availability Workbench..."),	SLOT(sDspItemAvailabilityWorkbench()),	itemSitesMenu, _privleges->check("ViewItemAvailabilityWorkbench"), new QPixmap(":/images/itemAvailabilityWorkbench.png"), toolBar, true },									  

    //  I/M | Inventory Transactions
    { "menu",				tr("I&nventory Transactions"),	  (char*)transactionsMenu,	mainMenu,	  true,	NULL, NULL, true	},
    { "im.miscellaneousAdjustment",	tr("Miscellaneous Adjustment..."),SLOT(sAdjustmentTrans()),	transactionsMenu, _privleges->check("CreateAdjustmentTrans"),	NULL, NULL, true}, 
    // TODO: switch between visibility of Inter-Warehouse Transfer and Transfer Orders based on config param?
    { "im.interWarehouseTransfer",	tr("Inter-Warehouse Transfer..."),SLOT(sTransferTrans()),	transactionsMenu, _privleges->check("CreateInterWarehouseTrans"),	NULL, NULL, _metrics->boolean("MultiWhs")}, 
    { "im.scrap",			tr("Scrap..."),			  SLOT(sScrapTrans()),		transactionsMenu, _privleges->check("CreateScrapTrans"),	NULL, NULL, true}, 
    { "im.expense",			tr("Expense..."),		  SLOT(sExpenseTrans()),	transactionsMenu, _privleges->check("CreateExpenseTrans"),	NULL, NULL, true}, 
    { "im.materialReceipt",		tr("Material Receipt..."),	  SLOT(sReceiptTrans()),	transactionsMenu, _privleges->check("CreateReceiptTrans"),	NULL, NULL, true}, 
    { "im.transform",			tr("Transform..."),		  SLOT(sTransformTrans()),	transactionsMenu, _privleges->check("CreateTransformTrans"),	NULL, NULL, _metrics->boolean("Transforms")}, 
    { "separator",			NULL,				  NULL,				transactionsMenu, true, NULL, NULL, _metrics->boolean("MultiWhs") },
    { "im.transferOrders",		tr("List Transfer Orders..."),	  SLOT(sTransferOrders()),	transactionsMenu, (_privleges->check("ViewTransferOrders") || _privleges->check("MaintainTransferOrders")),	NULL, NULL, _metrics->boolean("MultiWhs")}, 
    { "im.interWarehouseTransfer",	tr("New Transfer Order..."),	  SLOT(sNewTransferOrder()),	transactionsMenu, _privleges->check("MaintainTransferOrders"),	NULL, NULL, _metrics->boolean("MultiWhs")}, 
    { "separator",			NULL,				  NULL,				transactionsMenu, true, NULL, NULL, true },
    { "im.resetQOHBalances",		tr("Reset QOH Balances..."),	  SLOT(sResetQOHBalances()),	transactionsMenu, _privleges->check("CreateAdjustmentTrans"),	NULL, NULL, true}, 
    { "separator",			NULL,				  NULL,				transactionsMenu, true, NULL, NULL, true },
    { "im.relocateInventory",		tr("Relocate Inventory..."),	  SLOT(sRelocateInventory()),	transactionsMenu, _privleges->check("RelocateInventory"),	NULL, NULL, true}, 

    //  I/M | Lot/Serial Control
    { "menu",				tr("&Lot/Serial Control"),	(char*)lotSerialControlMenu,	mainMenu,	true,	NULL, NULL, _metrics->boolean("LotSerialControl") },
 // { "im.lotSerialHistory",		tr("Lot/Serial History..."),	SLOT(sLotSerialHistory()),	lotSerialControlMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, _metrics->boolean("LotSerialControl")},
    { "im.lotSerialComments",		tr("Lot/Serial Comments..."),	SLOT(sLotSerialComments()),	lotSerialControlMenu, _privleges->check("ViewInventoryHistory")  ,	NULL, NULL, _metrics->boolean("LotSerialControl")}, 
    { "im.dspLocationLotSerialDetail",	tr("Location/Lot/Serial # Detail..."),	SLOT(sDspLocationLotSerialDetail()), lotSerialControlMenu, _privleges->check("ViewQOH") ,	NULL, NULL, _metrics->boolean("LotSerialControl")}, 
    { "im.dspDetailedInventoryHistoryByLot/SerialNumber", tr("Detailed Inventory History by Lot/Serial #..."), SLOT(sDspDetailedInventoryHistoryByLotSerial()), lotSerialControlMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, _metrics->boolean("LotSerialControl")}, 
    { "separator",			NULL,				NULL,	lotSerialControlMenu,	true, NULL, NULL, _metrics->boolean("LotSerialControl") },

    { "im.reassignLotSerialNumber",	tr("Reassign Lot/Serial #..."),	SLOT(sReassignLotSerialNumber()), lotSerialControlMenu, _privleges->check("ReassignLotSerial"),	NULL, NULL, _metrics->boolean("LotSerialControl")}, 

    { "menu",				tr("&Physical Inventory..."),		   (char*)physicalMenu,			 mainMenu,	true,	NULL, NULL, true },
    { "im.createCountTagsByClassCode",	tr("Create Count Tags by Class Code..."),  SLOT(sCreateCountTagsByClassCode()),  physicalMenu, _privleges->check("IssueCountTags"),	NULL, NULL, true}, 
    { "im.createCountTagsByPlannerCode",tr("Create Count Tags by Planner Code..."),SLOT(sCreateCountTagsByPlannerCode()),physicalMenu, _privleges->check("IssueCountTags"),	NULL, NULL, true}, 
    { "im.createCountTagsByItem",	tr("Create Count Tags by Item..."),	   SLOT(sCreateCountTagsByItem()),	 physicalMenu, _privleges->check("IssueCountTags"),	NULL, NULL, true}, 
    { "im.createCycleCountTags", tr("Create Cycle Count Tags..."),SLOT(sCreateCycleCountTags()), physicalMenu, _privleges->check("IssueCountTags"),	NULL, NULL, true}, 
    { "separator",			NULL,					   NULL,				 physicalMenu,	true, NULL, NULL, true },
    { "im.enterCountSlip",		tr("Enter Count Slip..."),		   SLOT(sEnterCountSlip()), 		 physicalMenu, _privleges->check("EnterCountSlips"),	NULL, NULL, true}, 
    { "im.enterCountTag",		tr("Enter Count Tag..."),		   SLOT(sEnterCountTags()), 		 physicalMenu, _privleges->check("EnterCountTags"),	NULL, NULL, true}, 
    { "im.enterMiscInventoryCount",	tr("Enter Misc. Inventory Count..."),	   SLOT(sEnterMiscCount()), 		 physicalMenu, _privleges->check("EnterMiscCounts"),	NULL, NULL, true}, 
    { "im.zeroUncountedCountTags",	tr("Zero Uncounted Count Tags..."),	   SLOT(sZeroUncountedTagsByWarehouse()),physicalMenu, _privleges->check("ZeroCountTags"),	NULL, NULL, true}, 
    { "separator",			NULL,					   NULL,				 physicalMenu,	true, NULL, NULL, true },
    { "im.thawItemSitesByClassCode",	tr("Thaw Item Sites by Class Code..."),	   SLOT(sThawItemSitesByClassCode()),	 physicalMenu, _privleges->check("ThawInventory"),	NULL, NULL, true}, 
    { "separator",			NULL,					   NULL,				 physicalMenu,	true, NULL, NULL, true },
    { "im.postCountSlips",		tr("Post Count Slips..."),		   SLOT(sPostCountSlipsByWarehouse()),	 physicalMenu, _privleges->check("PostCountSlips"),	NULL, NULL, true}, 
    { "im.postCountTags",		tr("Post Count Tags..."),		   SLOT(sPostCountTags()),		 physicalMenu, _privleges->check("PostCountTags"),	NULL, NULL, true}, 
    { "im.purgeCountSlips",		tr("Purge Posted Count Slips..."),	   SLOT(sPurgePostedCountSlips()),	 physicalMenu, _privleges->check("PurgeCountSlips"),	NULL, NULL, true}, 
    { "im.purgeCountTags",		tr("Purge Posted Count Tags..."),	   SLOT(sPurgePostedCountTags()),	 physicalMenu, _privleges->check("PurgeCountTags"),	NULL, NULL, true}, 
    { "separator",			NULL,					   NULL,				 physicalMenu,	true, NULL, NULL, true },

    //  I/M | Physical Inventory | Displays
    { "menu",				tr("&Displays"),		  (char*)physicalDisplaysMenu,	    physicalMenu,	  true,	NULL, NULL, true },
    { "im.dspFrozenItemSites",		tr("Frozen Item Sites..."),	  SLOT(sDspFrozenItemSites()),	    physicalDisplaysMenu, _privleges->check("ViewItemSites"),	NULL, NULL, true}, 
    { "separator",			NULL,				  NULL,				    physicalDisplaysMenu, true, NULL, NULL, true },
    { "im.dspCountSlipEditList",	tr("Count Slip Edit List..."),	  SLOT(sDspCountSlipEditList()),    physicalDisplaysMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 
    { "im.dspCountTagEditList",		tr("Count Tag Edit List..."),	  SLOT(sDspCountTagEditList()),	    physicalDisplaysMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 
    { "separator",			NULL,				  NULL,				    physicalDisplaysMenu, true, NULL, NULL, true },
    { "im.dspCountSlipsByWarehouse",	tr("Count Slips by Warehouse..."),SLOT(sDspCountSlipsByWarehouse()),physicalDisplaysMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 
    { "separator",			NULL,				  NULL,				    physicalDisplaysMenu, true, NULL, NULL, true },
    { "im.dspCountTagsByItem",		tr("Count Tags by Item..."),	  SLOT(sDspCountTagsByItem()),	    physicalDisplaysMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 
    { "im.dspCountTagsByWarehouse",	tr("Count Tags by Warehouse..."), SLOT(sDspCountTagsByWarehouse()), physicalDisplaysMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 
    { "im.dspCountTagsByClassCode",	tr("Count Tags by ClassCode..."), SLOT(sDspCountTagsByClassCode()), physicalDisplaysMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 

    //  I/M | Physical Inventory | Reports
    { "menu",				tr("&Reports"),			  (char*)physicalReportsMenu,	    physicalMenu,	 true,	NULL, NULL, true },
    { "im.rptFrozenItemSites",		tr("Frozen Item Sites..."),	  SLOT(sRptFrozenItemSites()),	    physicalReportsMenu, _privleges->check("ViewItemSites"),	NULL, NULL, true}, 
    { "separator",			NULL,				  NULL,				    physicalReportsMenu, true, NULL, NULL, true },
    { "im.rptCountSlipEditList",	tr("Count Slip Edit List..."),	  SLOT(sRptCountSlipEditList()),    physicalReportsMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 
    { "im.rptCountTagEditList",		tr("Count Tag Edit List..."),	  SLOT(sRptCountTagEditList()),     physicalReportsMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 
    { "separator",			NULL,				  NULL,				    physicalReportsMenu, true, NULL, NULL, true },
    { "im.rptCountSlipsByWarehouse",	tr("Count Slips by Warehouse..."),SLOT(sRptCountSlipsByWarehouse()),physicalReportsMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 
    { "separator",			NULL,				  NULL,				    physicalReportsMenu, true, NULL, NULL, true },
    { "im.rptCountTagsByItem",		tr("Count Tags by Item..."),	  SLOT(sRptCountTagsByItem()),	    physicalReportsMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 
    { "im.rptCountTagsByWarehouse",	tr("Count Tags by Warehouse..."), SLOT(sRptCountTagsByWarehouse()), physicalReportsMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 
    { "im.rptCountTagsByClassCode",	tr("Count Tags by ClassCode..."), SLOT(sRptCountTagsByClassCode()), physicalReportsMenu, _privleges->check("ViewCountTags"),	NULL, NULL, true}, 

    //  I/M | Displays
    { "menu",				tr("&Displays"),			  (char*)displaysMenu,			mainMenu,	true,	NULL, NULL, true },
    { "im.dspItemSitesByItem",		tr("Item Sites by Item..."),		  SLOT(sDspItemSitesByItem()),		displaysMenu, _privleges->check("ViewItemSites"),	NULL, NULL, true}, 
    { "im.dspItemSitesByClassCode",	tr("Item Sites by Class Code..."),	  SLOT(sDspItemSitesByClassCode()),	displaysMenu, _privleges->check("ViewItemSites"),	NULL, NULL, true}, 
    { "im.dspItemSitesByPlannerCode",	tr("Item Sites by Planner Code..."),	  SLOT(sDspItemSitesByPlannerCode()),	displaysMenu, _privleges->check("ViewItemSites"),	NULL, NULL, true}, 
    { "im.dspItemSitesByCostCategory",	tr("Item Sites by Cost Category..."),	  SLOT(sDspItemSitesByCostCategory()),	displaysMenu, _privleges->check("ViewItemSites"),	NULL, NULL, true}, 
    { "im.dspValidLocationsByItem",	tr("Valid Locations by Item..."),	  SLOT(sDspValidLocationsByItem()),	displaysMenu, _privleges->check("ViewLocations"),	NULL, NULL, true}, 
    {  "separator",			NULL,					  NULL,					displaysMenu,	true, NULL, NULL, true },
    { "im.dspQOHByItem",		tr("Quantities On Hand by Item..."),	  SLOT(sDspQOHByItem()),		displaysMenu, _privleges->check("ViewQOH"),	NULL, NULL, true}, 
    { "im.dspQOHByClassCode",		tr("Quantities On Hand by Class Code..."),SLOT(sDspQOHByClassCode()),		displaysMenu, _privleges->check("ViewQOH"),	NULL, NULL, true}, 
    { "im.dspQOHByItemGroup",		tr("Quantities On Hand by Item Group..."),SLOT(sDspQOHByItemGroup()),		displaysMenu, _privleges->check("ViewQOH"),	NULL, NULL, true}, 
    { "im.dspQOHByLocation",		tr("Quantities On Hand by Location..."),  SLOT(sDspQOHByLocation()),		displaysMenu, _privleges->check("ViewQOH"),	NULL, NULL, true}, 
    { "im.dspLocationLotSerialDetail",	tr("Location/Lot/Serial # Detail..."),	  SLOT(sDspLocationLotSerialDetail()),	displaysMenu, _privleges->check("ViewQOH"),	NULL, NULL, _metrics->boolean("LotSerialControl")}, 
    { "im.dspSlowMovingInventory",	tr("Slow Moving Inventory..."),		  SLOT(sDspSlowMovingInventoryByClassCode()), displaysMenu, _privleges->check("ViewQOH"),	NULL, NULL, true}, 
    { "im.dspExpiredInventory",		tr("Expired Inventory..."),		  SLOT(sDspExpiredInventoryByClassCode()), displaysMenu, _privleges->check("ViewQOH"),	NULL, NULL, _metrics->boolean("LotSerialControl")}, 
    { "separator",					NULL,					  	 NULL,					displaysMenu,	true, NULL, NULL, true },
    { "im.dspInventoryAvailabilityByItem",		tr("Inventory Availability by Item..."),	 SLOT(sDspInventoryAvailabilityByItem()), displaysMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true},
    { "im.dspInventoryAvailabilityByItemGroup",		tr("Inventory Availability by Item Group..."),	 SLOT(sDspInventoryAvailabilityByItemGroup()), displaysMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true}, 
    { "im.dspInventoryAvailabilityByClassCode",		tr("Inventory Availability by Class Code..."),	 SLOT(sDspInventoryAvailabilityByClassCode()), displaysMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true}, 
    { "im.dspInventoryAvailabilityByPlannerCode",	tr("Inventory Availability by Planner Code..."), SLOT(sDspInventoryAvailabilityByPlannerCode()), displaysMenu, _privleges->check("ViewInventoryAvailability"), new QPixmap(":/images/dspInventoryAvailabilityByPlannerCode.png"), toolBar, true },
    { "im.dspInventoryAvailabilityBySourceVendor",	tr("Inventory Availability by Source Vendor..."),SLOT(sDspInventoryAvailabilityBySourceVendor()), displaysMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true}, 
    { "im.dspSubstituteAvailabilityByRootItem",		tr("Substitute Availability by Root Item..."),	 SLOT(sDspSubstituteAvailabilityByRootItem()), displaysMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true}, 
    {  "separator",					NULL,						 NULL,	displaysMenu,	true, NULL, NULL, _metrics->boolean("BufferMgt") },
 // { "im.dspInventoryBufferStatusByItem",		tr("Inventory Buffer Status by Item..."),	 SLOT(sDspInventoryBufferStatusByItem()), displaysMenu, _privleges->check("ViewInventoryBufferStatus"),	NULL, NULL, _metrics->boolean("BufferMgt")}, 
    { "im.dspInventoryBufferStatusByItemGroup",		tr("Inventory Buffer Status by Item Group..."),	 SLOT(sDspInventoryBufferStatusByItemGroup()), displaysMenu, _privleges->check("ViewInventoryBufferStatus") ,	NULL, NULL, _metrics->boolean("BufferMgt")}, 
    { "im.dspInventoryBufferStatusByClassCode",		tr("Inventory Buffer Status by Class Code..."),	 SLOT(sDspInventoryBufferStatusByClassCode()), displaysMenu, _privleges->check("ViewInventoryBufferStatus"),	NULL, NULL, _metrics->boolean("BufferMgt")}, 
    { "im.dspInventoryBufferStatusByPlannerCode",	tr("Inventory Buffer Status by Planner Code..."),SLOT(sDspInventoryBufferStatusByPlannerCode()), displaysMenu, _privleges->check("ViewInventoryBufferStatus"),	NULL, NULL, _metrics->boolean("BufferMgt")}, 
    { "separator",					NULL,						 NULL,	displaysMenu,	true, NULL, NULL, true },
    { "im.dspInventoryHistoryByItem",			tr("Inventory History by Item..."),		 SLOT(sDspInventoryHistoryByItem()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true},
    { "im.dspInventoryHistoryByItemGroup",		tr("Inventory History by Item Group..."),	 SLOT(sDspInventoryHistoryByItemGroup()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.dspInventoryHistoryByOrderNumber",		tr("Inventory History by Order Number..."),	 SLOT(sDspInventoryHistoryByOrderNumber()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.dspInventoryHistoryByClassCode",		tr("Inventory History by Class Code..."),	 SLOT(sDspInventoryHistoryByClassCode()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.dspInventoryHistoryByPlannerCode",		tr("Inventory History by Planner Code..."),	 SLOT(sDspInventoryHistoryByPlannerCode()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "separator",					NULL,							NULL,	displaysMenu,	true, NULL, NULL, true },
    { "im.dspDetailedInventoryHistoryByLot/SerialNumber",tr("Detailed Inventory History by Lot/Serial #..."),	SLOT(sDspDetailedInventoryHistoryByLotSerial()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL,  _metrics->boolean("LotSerialControl")}, 
    { "im.dspDetailedInventoryHistoryByLocation",	tr("Detailed Inventory History by Location..."),	SLOT(sDspDetailedInventoryHistoryByLocation()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "separator",					NULL,							NULL,	displaysMenu,	true, NULL, NULL, true },
    { "im.dspItemUsageStatisticsByItem",		tr("Item Usage Statistics by Item..."),			SLOT(sDspItemUsageStatisticsByItem()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.dspItemUsageStatisticsByClassCode",		tr("Item Usage Statistics by Class Code..."),		SLOT(sDspItemUsageStatisticsByClassCode()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.dspItemUsageStatisticsByItemGroup",		tr("Item Usage Statistics by Item Group..."),		SLOT(sDspItemUsageStatisticsByItemGroup()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.dspItemUsageStatisticsByWarehouse",		tr("Item Usage Statistics by Warehouse..."),		SLOT(sDspItemUsageStatisticsByWarehouse()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.dspTimePhasedItemUsageStatisticsByItem",	tr("Time-Phased Item Usage Statistics by Item..."),	SLOT(sDspTimePhasedUsageStatisticsByItem()), displaysMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 

    //  I/M | Reports
    { "menu",				tr("&Reports"),				(char*)reportsMenu,		    mainMenu,	 true,					NULL, NULL, true },
    { "im.rptItemSitesByItem",		tr("Item Sites by Item..."),		SLOT(sRptItemSitesByItem()),	    reportsMenu, _privleges->check("ViewItemSites"),	NULL, NULL, true}, 
    { "im.rptItemSitesByClassCode",	tr("Item Sites by Class Code..."),	SLOT(sRptItemSitesByClassCode()),   reportsMenu, _privleges->check("ViewItemSites"),	NULL, NULL, true}, 
    { "im.rptItemSitesByPlannerCode",	tr("Item Sites by Planner Code..."),	SLOT(sRptItemSitesByPlannerCode()), reportsMenu, _privleges->check("ViewItemSites"),	NULL, NULL, true}, 
    { "im.rptItemSitesByCostCategory",	tr("Item Sites by Cost Category..."),	SLOT(sRptItemSitesByCostCategory()),reportsMenu, _privleges->check("ViewItemSites"),	NULL, NULL, true}, 
    { "im.rptValidLocationsByItem",	tr("Valid Locations by Item..."),	SLOT(sRptValidLocationsByItem()),   reportsMenu, _privleges->check("ViewLocations"),	NULL, NULL, true}, 
    { "separator",			NULL,					NULL,				    reportsMenu, true,					NULL, NULL, true },

    { "im.rptQOHByItem",		tr("Quantities On Hand by Item..."),	  SLOT(sRptQOHByItem()),		     reportsMenu, _privleges->check("ViewQOH"),	NULL, NULL, true}, 
    { "im.rptQOHByClassCode",		tr("Quantities On Hand by Class Code..."),SLOT(sRptQOHByClassCode()),		     reportsMenu, _privleges->check("ViewQOH"),	NULL, NULL, true}, 
    { "im.rptQOHByItemGroup",		tr("Quantities On Hand by Item Group..."),SLOT(sRptQOHByItemGroup()),		     reportsMenu, _privleges->check("ViewQOH"),	NULL, NULL, true}, 
    { "im.rptQOHByLocation",		tr("Quantities On Hand by Location..."),  SLOT(sRptQOHByLocation()),		     reportsMenu, _privleges->check("ViewQOH"),	NULL, NULL, true}, 
    { "im.rptLocationLotSerialDetail",	tr("Location/Lot/Serial # Detail..."),	  SLOT(sRptLocationLotSerialDetail()),	     reportsMenu, _privleges->check("ViewQOH"),	NULL, NULL, _metrics->boolean("LotSerialControl")}, 
    { "im.rptSlowMovingInventory",	tr("Slow Moving Inventory..."),		  SLOT(sRptSlowMovingInventoryByClassCode()),reportsMenu, _privleges->check("ViewQOH"), NULL, NULL, true}, 
    { "im.rptExpiredInventory",		tr("Expired Inventory..."),		  SLOT(sRptExpiredInventoryByClassCode()),   reportsMenu, _privleges->check("ViewQOH"),	NULL, NULL, _metrics->boolean("LotSerialControl")}, 
    { "separator",			NULL,					  NULL,					     reportsMenu, true,				NULL, NULL, true },
    { "im.rptInventoryAvailabilityByItem",		tr("Inventory Availability by Item..."),	 SLOT(sRptInventoryAvailabilityByItem()), reportsMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true}, 
    { "im.rptInventoryAvailabilityByItemGroup",		tr("Inventory Availability by Item Group..."),	 SLOT(sRptInventoryAvailabilityByItemGroup()), reportsMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true}, 
    { "im.rptInventoryAvailabilityByClassCode",		tr("Inventory Availability by Class Code..."),	 SLOT(sRptInventoryAvailabilityByClassCode()), reportsMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true}, 
    { "im.rptInventoryAvailabilityByPlannerCode",	tr("Inventory Availability by Planner Code..."), SLOT(sRptInventoryAvailabilityByPlannerCode()), reportsMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true}, 
    { "im.rptInventoryAvailabilityBySourceVendor",	tr("Inventory Availability by Source Vendor..."),SLOT(sRptInventoryAvailabilityBySourceVendor()), reportsMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true}, 
    { "im.rptSubstituteAvailabilityByRootItem",		tr("Substitute Availability by Root Item..."),	 SLOT(sRptSubstituteAvailabilityByRootItem()), reportsMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true}, 
    { "separator",					NULL,	NULL,	reportsMenu,	true, NULL, NULL, _metrics->boolean("BufferMgt") },
 // { "im.rptInventoryBufferStatusByItem",		tr("Inventory Buffer Status by Item..."),	 SLOT(sRptInventoryBufferStatusByItem()), reportsMenu, _privleges->check("ViewInventoryBufferStatus"),	NULL, NULL, _metrics->boolean("BufferMgt")}, 
    { "im.rptInventoryBufferStatusByItemGroup",		tr("Inventory Buffer Status by Item Group..."),	 SLOT(sRptInventoryBufferStatusByItemGroup()), reportsMenu, _privleges->check("ViewInventoryBufferStatus") ,	NULL, NULL, _metrics->boolean("BufferMgt")}, 
    { "im.rptInventoryBufferStatusByClassCode",		tr("Inventory Buffer Status by Class Code..."),	 SLOT(sRptInventoryBufferStatusByClassCode()), reportsMenu, _privleges->check("ViewInventoryBufferStatus") ,	NULL, NULL, _metrics->boolean("BufferMgt")}, 
    { "im.rptInventoryBufferStatusByPlannerCode",	tr("Inventory Buffer Status by Planner Code..."),SLOT(sRptInventoryBufferStatusByPlannerCode()), reportsMenu, _privleges->check("ViewInventoryBufferStatus") ,	NULL, NULL, _metrics->boolean("BufferMgt")}, 
    { "separator",					NULL,						 NULL,					    reportsMenu, true, NULL, NULL, true },
    { "im.rptInventoryHistoryByItem",			tr("Inventory History by Item..."),		 SLOT(sRptInventoryHistoryByItem()), 	    reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.rptInventoryHistoryByItemGroup",		tr("Inventory History by Item Group..."),	 SLOT(sRptInventoryHistoryByItemGroup()),   reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.rptInventoryHistoryByOrderNumber",		tr("Inventory History by Order Number..."),	 SLOT(sRptInventoryHistoryByOrderNumber()), reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.rptInventoryHistoryByClassCode",		tr("Inventory History by Class Code..."),	 SLOT(sRptInventoryHistoryByClassCode()),   reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.rptInventoryHistoryByPlannerCode",		tr("Inventory History by Planner Code..."),	 SLOT(sRptInventoryHistoryByPlannerCode()), reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "separator",					NULL,						 NULL,					    reportsMenu, true, NULL, NULL, true },
    { "im.rptDetailedInventoryHistoryByLot/SerialNumber",tr("Detailed Inventory History by Lot/Serial #..."),	SLOT(sRptDetailedInventoryHistoryByLotSerial()),reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, _metrics->boolean("LotSerialControl")}, 
    { "im.rptDetailedInventoryHistoryByLocation",	tr("Detailed Inventory History by Location..."),	SLOT(sRptDetailedInventoryHistoryByLocation()),	reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "separator",					NULL,							NULL,						reportsMenu,	true, NULL, NULL, true },
    { "im.rptItemUsageStatisticsByItem",		tr("Item Usage Statistics by Item..."),			SLOT(sRptItemUsageStatisticsByItem()),		reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.rptItemUsageStatisticsByClassCode",		tr("Item Usage Statistics by Class Code..."),		SLOT(sRptItemUsageStatisticsByClassCode()),	reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.rptItemUsageStatisticsByItemGroup",		tr("Item Usage Statistics by Item Group..."),		SLOT(sRptItemUsageStatisticsByItemGroup()),	reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.rptItemUsageStatisticsByWarehouse",		tr("Item Usage Statistics by Warehouse..."),		SLOT(sRptItemUsageStatisticsByWarehouse()),	reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "im.rptTimePhasedItemUsageStatisticsByItem",	tr("Time-Phased Item Usage Statistics by Item..."), 	SLOT(sRptTimePhasedUsageStatisticsByItem()),	reportsMenu, _privleges->check("ViewInventoryHistory"),	NULL, NULL, true}, 
    { "separator",					NULL,							NULL,						reportsMenu, true, NULL, NULL, true },
    { "im.printItemLabelsByClassCode",			tr("Print Item Labels by Class Code..."),		SLOT(sPrintItemLabelsByClassCode()),		reportsMenu, _privleges->check("ViewItemSites"),	NULL, NULL, true}, 

    { "menu",			tr("&Master Information"),	(char*)masterInfoMenu,	     mainMenu,	     true,												NULL, NULL, true },
    { whsModule,		whsLabel,		SLOT(sWarehouses()),	     masterInfoMenu, (_privleges->check("MaintainWarehouses")) || (_privleges->check("ViewWarehouses") ),		NULL, NULL, true}, 
    { "im.warehousesLocations",	tr("Warehouse Locations..."),	SLOT(sWarehouseLocations()), masterInfoMenu, (_privleges->check("MaintainLocations")) || (_privleges->check("ViewLocations") ),			NULL, NULL, true}, 
    { "im.costCategories",	tr("Cost Categories..."),	SLOT(sCostCategories()),     masterInfoMenu, (_privleges->check("MaintainCostCategories")) || (_privleges->check("ViewCostCategories") ),	NULL, NULL, true}, 
    { "im.expenseCategories",	tr("Expense Categories..."),	SLOT(sExpenseCategories()),  masterInfoMenu, (_privleges->check("MaintainExpenseCategories")) || (_privleges->check("ViewExpenseCategories") ),	NULL, NULL, true},
    { "im.characteristics",	tr("Characteristics..."),	SLOT(sCharacteristics()),    masterInfoMenu, (_privleges->check("MaintainCharacteristics") || _privleges->check("ViewCharacteristics") ),	NULL, NULL, true},

    // I/M | Utilties | Update Item Controls
    { "menu",					tr("&Update Item Controls"),			(char*)updateItemInfoMenu,			mainMenu,	    true,					NULL, NULL, true},
    { "im.updateABCClass",			tr("ABC Class..."),				SLOT(sUpdateABCClass()), 			updateItemInfoMenu, _privleges->check("UpdateABCClass"),	NULL, NULL, true},
    { "im.updateCycleCountFrequency",		tr("Cycle Count Frequency..."),			SLOT(sUpdateCycleCountFreq()),			updateItemInfoMenu, _privleges->check("UpdateCycleCountFreq"),	NULL, NULL, true},
    { "im.updateItemSiteLeadTimes",		tr("Item Site Lead Times..."),			SLOT(sUpdateItemSiteLeadTimes()),		updateItemInfoMenu, _privleges->check("UpdateLeadTime"),	NULL, NULL, true},
    { "separator",				NULL,						NULL,						updateItemInfoMenu, true, 					NULL, NULL, true},
    { "im.updateReorderLevelsByItem",		tr("Reorder Levels by Item..."),		SLOT(sUpdateReorderLevelByItem()),		updateItemInfoMenu, _privleges->check("UpdateReorderLevels"),	NULL, NULL, true},
    { "im.updateReorderLevelsByPlannerCode",	tr("Reorder Levels by Planner Code..."),	SLOT(sUpdateReorderLevelsByPlannerCode()),	updateItemInfoMenu, _privleges->check("UpdateReorderLevels"),	NULL, NULL, true},
    { "im.updateReorderLevelsByClassCode",	tr("Reorder Levels by Class Code..."), 		SLOT(sUpdateReorderLevelsByClassCode()),	updateItemInfoMenu, _privleges->check("UpdateReorderLevels"),	NULL, NULL, true},
    { "separator",				NULL,						NULL,						updateItemInfoMenu, true, 					NULL, NULL, true},
    { "im.updateOrderUpToLevelsByItem",		tr("Order Up To Levels by Item..."),		SLOT(sUpdateOUTLevelByItem()), 			updateItemInfoMenu, _privleges->check("UpdateOUTLevels"),	NULL, NULL, true},
    { "im.updateOrderUpToLevelsByPlannerCode",	tr("Order Up To Levels by Planner Code..."),	SLOT(sUpdateOUTLevelsByPlannerCode()),		updateItemInfoMenu, _privleges->check("UpdateOUTLevels"),	NULL, NULL, true},
    { "im.updateOrderUpToLevelsByClassCode",	tr("Order Up To Levels by Class Code..."),	SLOT(sUpdateOUTLevelsByClassCode()),		updateItemInfoMenu, _privleges->check("UpdateOUTLevels"),	NULL, NULL, true},

    // I/M | Utilities
    { "menu",					  tr("&Utilities"),			 		(char*)utilitiesMenu,			mainMenu,	true,							NULL, NULL, true},
    { "im.dspUnbalancedQOHByClassCode",		  tr("Unbalanced QOH by Class Code..."), 		SLOT(sDspUnbalancedQOHByClassCode()),	utilitiesMenu, _privleges->check("ViewItemSites"),			NULL, NULL, true},
    { "separator",				  NULL,					 		NULL,					utilitiesMenu,	true, 							NULL, NULL, true},
    { "im.summarizeTransactionHistoryByClassCode",tr("Summarize Transaction History by Class Code..."), SLOT(sSummarizeInvTransByClassCode()),	utilitiesMenu, _privleges->check("SummarizeInventoryTransactions"),	NULL, NULL, true},
    { "im.createItemSitesByClassCode",		  tr("Create Item Sites by Class Code..."),		SLOT(sCreateItemSitesByClassCode()),	utilitiesMenu, _privleges->check("MaintainItemSites"),			NULL, NULL, true},
  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));



#if 0
  mainMenu->insertItem(tr("&Graphs"),                 graphsMenu           );
#endif

  parent->populateCustomMenu(mainMenu, "I/M");
  parent->menuBar()->insertItem(tr("&I/M"), mainMenu);

//  Create connections to the module specific inputManager SIGNALS
  parent->inputManager()->notify(cBCLocationContents, this, this, SLOT(sCatchLocationContents(int)));
  parent->inputManager()->notify(cBCCountTag, this, this, SLOT(sCatchCountTag(int)));
}

void moduleIM::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  for (unsigned int i = 0; i < numElems; i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == "menu")
    {
      if (acts[i].priv)
      {
        acts[i].menu->insertItem(acts[i].actionTitle, (QMenu*)(acts[i].slot));
      }
    }
    else if (acts[i].actionName == "separator")
    {
      acts[i].menu->addSeparator();
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
					  acts[i].toolBar) );
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

void moduleIM::sNewItemSite()
{
  ParameterList params;
  params.append("mode", "new");

  itemSite newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sItemSites()
{
  omfgThis->handleNewWindow(new itemSites());
}

void moduleIM::sAdjustmentTrans()
{
  ParameterList params;
  params.append("mode", "new");

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sTransferTrans()
{
  ParameterList params;
  params.append("mode", "new");

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sNewTransferOrder()
{
  transferOrder::newTransferOrder(-1, -1);
}

void moduleIM::sTransferOrders()
{
  omfgThis->handleNewWindow(new transferOrders());
}

void moduleIM::sReceiptTrans()
{
  ParameterList params;
  params.append("mode", "new");

  materialReceiptTrans *newdlg = new materialReceiptTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sTransformTrans()
{
  ParameterList params;
  params.append("mode", "new");

  transformTrans *newdlg = new transformTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sScrapTrans()
{
  ParameterList params;
  params.append("mode", "new");

  scrapTrans *newdlg = new scrapTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sExpenseTrans()
{
  ParameterList params;
  params.append("mode", "new");

  expenseTrans *newdlg = new expenseTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sResetQOHBalances()
{
  resetQOHBalances(parent, "", TRUE).exec();
}

void moduleIM::sRelocateInventory()
{
  relocateInventory(parent, "", TRUE).exec();
}


//  Lot/Serial Control
void moduleIM::sLotSerialHistory()
{
  lotSerialHistory newdlg(parent, "", TRUE);
  newdlg.exec();
}

void moduleIM::sLotSerialComments()
{
  lotSerialComments newdlg(parent, "", TRUE);
  newdlg.exec();
}

void moduleIM::sReassignLotSerialNumber()
{
  reassignLotSerial newdlg(parent, "", TRUE);
  newdlg.exec();
}


void moduleIM::sCreateCountTagsByClassCode()
{
  ParameterList params;
  params.append("classcode");

  createCountTagsByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sCreateCountTagsByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  createCountTagsByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sCreateCountTagsByItem()
{
  createCountTagsByItem(parent, "", TRUE).exec();
}

void moduleIM::sCreateCycleCountTags()
{
  createCycleCountTags(parent, "", TRUE).exec();
}

void moduleIM::sEnterCountSlip()
{
  ParameterList params;
  params.append("mode", "new");

  countSlip newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sEnterCountTags()
{
  ParameterList params;
  params.append("mode", "new");

  countTag newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sEnterMiscCount()
{
  enterMiscCount(parent, "", TRUE).exec();
}

void moduleIM::sZeroUncountedTagsByWarehouse()
{
  zeroUncountedCountTagsByWarehouse(parent, "", TRUE).exec();
}

void moduleIM::sThawItemSitesByClassCode()
{
  thawItemSitesByClassCode(parent, "", TRUE).exec();
}

void moduleIM::sPostCountSlipsByWarehouse()
{
  postCountSlips(parent, "", TRUE).exec();
}

void moduleIM::sPostCountTags()
{
  postCountTags(parent, "", TRUE).exec();
}

void moduleIM::sPurgePostedCountSlips()
{
  purgePostedCountSlips(parent, "", TRUE).exec();
}

void moduleIM::sPurgePostedCountTags()
{
  purgePostedCounts(parent, "", TRUE).exec();
}

void moduleIM::sDspFrozenItemSites()
{
  omfgThis->handleNewWindow(new dspFrozenItemSites());
}

void moduleIM::sDspCountSlipEditList()
{
  omfgThis->handleNewWindow(new dspCountSlipEditList());
}

void moduleIM::sDspCountTagEditList()
{
  omfgThis->handleNewWindow(new dspCountTagEditList());
}

void moduleIM::sDspCountSlipsByWarehouse()
{
  omfgThis->handleNewWindow(new dspCountSlipsByWarehouse());
}

void moduleIM::sDspCountTagsByItem()
{
  omfgThis->handleNewWindow(new dspCountTagsByItem());
}

void moduleIM::sDspCountTagsByWarehouse()
{
  omfgThis->handleNewWindow(new dspCountTagsByWarehouse());
}

void moduleIM::sDspCountTagsByClassCode()
{
  omfgThis->handleNewWindow(new dspCountTagsByClassCode());
}

void moduleIM::sRptFrozenItemSites()
{
  rptFrozenItemSites(parent, "", TRUE).exec();
}

void moduleIM::sRptCountSlipEditList()
{
  rptCountSlipEditList(parent, "", TRUE).exec();
}

void moduleIM::sRptCountTagEditList()
{
  rptCountTagEditList(parent, "", TRUE).exec();
}

void moduleIM::sRptCountSlipsByWarehouse()
{
  rptCountSlipsByWarehouse(parent, "", TRUE).exec();
}

void moduleIM::sRptCountTagsByItem()
{
  rptCountTagsByItem(parent, "", TRUE).exec();
}

void moduleIM::sRptCountTagsByWarehouse()
{
  rptCountTagsByWarehouse(parent, "", TRUE).exec();
}

void moduleIM::sRptCountTagsByClassCode()
{
  rptCountTagsByClassCode(parent, "", TRUE).exec();
}

void moduleIM::sDspItemAvailabilityWorkbench()
{
  omfgThis->handleNewWindow(new itemAvailabilityWorkbench());
}

void moduleIM::sDspItemSitesByItem()
{
  omfgThis->handleNewWindow(new dspItemSitesByItem());
}

void moduleIM::sDspItemSitesByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspItemSitesByParameterList *newdlg = new dspItemSitesByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspItemSitesByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspItemSitesByParameterList *newdlg = new dspItemSitesByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspItemSitesByCostCategory()
{
  ParameterList params;
  params.append("costcat");

  dspItemSitesByParameterList *newdlg = new dspItemSitesByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspValidLocationsByItem()
{
  omfgThis->handleNewWindow(new dspValidLocationsByItem());
}

void moduleIM::sDspQOHByItem()
{
  omfgThis->handleNewWindow(new dspQOHByItem());
}

void moduleIM::sDspQOHByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspQOHByParameterList *newdlg = new dspQOHByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspQOHByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspQOHByParameterList *newdlg = new dspQOHByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspQOHByLocation()
{
  omfgThis->handleNewWindow(new dspQOHByLocation());
}

void moduleIM::sDspLocationLotSerialDetail()
{
  omfgThis->handleNewWindow(new dspInventoryLocator());
}

void moduleIM::sDspSlowMovingInventoryByClassCode()
{
  omfgThis->handleNewWindow(new dspSlowMovingInventoryByClassCode());
}

void moduleIM::sDspExpiredInventoryByClassCode()
{
  omfgThis->handleNewWindow(new dspExpiredInventoryByClassCode());
}

void moduleIM::sDspInventoryAvailabilityByItem()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityByItem());
}

void moduleIM::sDspInventoryAvailabilityByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspInventoryAvailabilityByParameterList *newdlg = new dspInventoryAvailabilityByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspInventoryAvailabilityByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspInventoryAvailabilityByParameterList *newdlg = new dspInventoryAvailabilityByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspInventoryAvailabilityByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspInventoryAvailabilityByParameterList *newdlg = new dspInventoryAvailabilityByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspInventoryAvailabilityBySourceVendor()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityBySourceVendor());
}

void moduleIM::sDspSubstituteAvailabilityByRootItem()
{
  omfgThis->handleNewWindow(new dspSubstituteAvailabilityByItem());
}

void moduleIM::sDspInventoryBufferStatusByItem()
{
  //omfgThis->handleNewWindow(new dspInventoryBufferStatusByItem());
}

void moduleIM::sDspInventoryBufferStatusByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspInventoryBufferStatusByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspInventoryBufferStatusByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspInventoryHistoryByItem()
{
  omfgThis->handleNewWindow(new dspInventoryHistoryByItem());
}

void moduleIM::sDspInventoryHistoryByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspInventoryHistoryByParameterList *newdlg = new dspInventoryHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspInventoryHistoryByOrderNumber()
{
  omfgThis->handleNewWindow(new dspInventoryHistoryByOrderNumber());
}

void moduleIM::sDspInventoryHistoryByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspInventoryHistoryByParameterList *newdlg = new dspInventoryHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspInventoryHistoryByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspInventoryHistoryByParameterList *newdlg = new dspInventoryHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sDspDetailedInventoryHistoryByLotSerial()
{
  omfgThis->handleNewWindow(new dspDetailedInventoryHistoryByLotSerial());
}

void moduleIM::sDspDetailedInventoryHistoryByLocation()
{
  omfgThis->handleNewWindow(new dspDetailedInventoryHistoryByLocation());
}

void moduleIM::sDspItemUsageStatisticsByItem()
{
  omfgThis->handleNewWindow(new dspUsageStatisticsByItem());
}

void moduleIM::sDspItemUsageStatisticsByClassCode()
{
  omfgThis->handleNewWindow(new dspUsageStatisticsByClassCode());
}

void moduleIM::sDspItemUsageStatisticsByItemGroup()
{
  omfgThis->handleNewWindow(new dspUsageStatisticsByItemGroup());
}

void moduleIM::sDspItemUsageStatisticsByWarehouse()
{
  omfgThis->handleNewWindow(new dspUsageStatisticsByWarehouse());
}

void moduleIM::sDspTimePhasedUsageStatisticsByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedUsageStatisticsByItem());
}

void moduleIM::sRptItemSitesByItem()
{
  rptItemSitesByItem(parent, "", TRUE).exec();
}

void moduleIM::sRptItemSitesByClassCode()
{
  ParameterList params;
  params.append("classcode");

  rptItemSitesByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptItemSitesByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  rptItemSitesByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptItemSitesByCostCategory()
{
  ParameterList params;
  params.append("costcat");

  rptItemSitesByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptValidLocationsByItem()
{
  rptValidLocationsByItem(parent, "", TRUE).exec();
}

void moduleIM::sRptQOHByItem()
{
  rptQOHByItem(parent, "", TRUE).exec();
}

void moduleIM::sRptQOHByClassCode()
{
  ParameterList params;
  params.append("classcode");

  rptQOHByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptQOHByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  rptQOHByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptQOHByLocation()
{
  rptQOHByLocation(parent, "", TRUE).exec();
}

void moduleIM::sRptLocationLotSerialDetail()
{
  rptInventoryLocator(parent, "", TRUE).exec();
}

void moduleIM::sRptSlowMovingInventoryByClassCode()
{
  rptSlowMovingInventoryByClassCode(parent, "", TRUE).exec();
}

void moduleIM::sRptExpiredInventoryByClassCode()
{
  rptExpiredInventoryByClassCode(parent, "", TRUE).exec();
}

void moduleIM::sRptInventoryAvailabilityByItem()
{
  rptInventoryAvailabilityByItem(parent, "", TRUE).exec();
}

void moduleIM::sRptInventoryAvailabilityByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  rptInventoryAvailabilityByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptInventoryAvailabilityByClassCode()
{
  ParameterList params;
  params.append("classcode");

  rptInventoryAvailabilityByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptInventoryAvailabilityByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  rptInventoryAvailabilityByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptInventoryAvailabilityBySourceVendor()
{
  rptInventoryAvailabilityBySourceVendor(parent, "", TRUE).exec();
}

void moduleIM::sRptSubstituteAvailabilityByRootItem()
{
  rptSubstituteAvailabilityByRootItem(parent, "", TRUE).exec();
}

void moduleIM::sRptInventoryHistoryByItem()
{
  rptInventoryHistoryByItem(parent, "", TRUE).exec();
}

void moduleIM::sRptInventoryHistoryByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  rptInventoryHistoryByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptInventoryHistoryByOrderNumber()
{
  rptInventoryHistoryByOrderNumber(parent, "", TRUE).exec();
}

void moduleIM::sRptInventoryHistoryByClassCode()
{
  ParameterList params;
  params.append("classcode");

  rptInventoryHistoryByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptInventoryHistoryByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  rptInventoryHistoryByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptDetailedInventoryHistoryByLotSerial()
{
  rptDetailedInventoryHistoryByLotSerial(parent, "", TRUE).exec();
}

void moduleIM::sRptDetailedInventoryHistoryByLocation()
{
  rptDetailedInventoryHistoryByLocation(parent, "", TRUE).exec();
}

void moduleIM::sRptItemUsageStatisticsByItem()
{
  rptUsageStatisticsByItem(parent, "", TRUE).exec();
}

void moduleIM::sRptItemUsageStatisticsByClassCode()
{
  rptUsageStatisticsByClassCode(parent, "", TRUE).exec();
}

void moduleIM::sRptItemUsageStatisticsByItemGroup()
{
  rptUsageStatisticsByItemGroup(parent, "", TRUE).exec();
}

void moduleIM::sRptItemUsageStatisticsByWarehouse()
{
  rptUsageStatisticsByWarehouse(parent, "", TRUE).exec();
}

void moduleIM::sRptTimePhasedUsageStatisticsByItem()
{
  rptTimePhasedUsageStatisticsByItem(parent, "", TRUE).exec();
}

void moduleIM::sRptInventoryBufferStatusByItem()
{
  //rptInventoryBufferStatusByItem(parent, "", TRUE).exec();
}

void moduleIM::sRptInventoryBufferStatusByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  rptInventoryBufferStatusByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptInventoryBufferStatusByClassCode()
{
  ParameterList params;
  params.append("classcode");

  rptInventoryBufferStatusByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sRptInventoryBufferStatusByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  rptInventoryBufferStatusByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sBarCodeDispatchList()
{
  rptLocationDispatchList(parent, "", TRUE).exec();
}

void moduleIM::sPrintItemLabelsByClassCode()
{
  printItemLabelsByClassCode(parent, "", TRUE).exec();
}


//  Master Information
void moduleIM::sWarehouses()
{
  if (_metrics->boolean("MultiWhs"))
    omfgThis->handleNewWindow(new warehouses());
  else
  {
    omfgThis->handleNewWindow(new warehouse());
  }
}

void moduleIM::sWarehouseLocations()
{
  omfgThis->handleNewWindow(new locations());
}

void moduleIM::sCostCategories()
{
  omfgThis->handleNewWindow(new costCategories());
}

void moduleIM::sExpenseCategories()
{
  omfgThis->handleNewWindow(new expenseCategories());
}

//  Utilities
void moduleIM::sDspUnbalancedQOHByClassCode()
{
  omfgThis->handleNewWindow(new dspUnbalancedQOHByClassCode());
}

void moduleIM::sUpdateABCClass()
{
  updateABCClass(parent, "", TRUE).exec();
}

void moduleIM::sUpdateCycleCountFreq()
{
  updateCycleCountFrequency(parent, "", TRUE).exec();
}

void moduleIM::sUpdateItemSiteLeadTimes()
{
  updateItemSiteLeadTimes(parent, "", TRUE).exec();
}

void moduleIM::sUpdateReorderLevelByItem()
{
  updateReorderLevelByItem(parent, "", TRUE).exec();
}

void moduleIM::sUpdateReorderLevelsByPlannerCode()
{
  updateReorderLevels(parent, "", TRUE).exec();
}

void moduleIM::sUpdateReorderLevelsByClassCode()
{
  updateReorderLevelsByClassCode(parent, "", TRUE).exec();
}

void moduleIM::sUpdateOUTLevelByItem()
{
  updateOUTLevelByItem(parent, "", TRUE).exec();
}

void moduleIM::sUpdateOUTLevelsByPlannerCode()
{
  updateOUTLevels(parent, "", TRUE).exec();
}

void moduleIM::sUpdateOUTLevelsByClassCode()
{
  updateOUTLevelsByClassCode(parent, "", TRUE).exec();
}

void moduleIM::sSummarizeInvTransByClassCode()
{
  summarizeInvTransByClassCode(parent, "", TRUE).exec();
}

void moduleIM::sCreateItemSitesByClassCode()
{
  createItemSitesByClassCode(parent, "", TRUE).exec();
}

//  inputManager SIGNAL handlers
void moduleIM::sCatchLocationContents(int pLocationid)
{
  ParameterList params;
  params.append("location_id", pLocationid);
  params.append("run");

  dspQOHByLocation *newdlg = new dspQOHByLocation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleIM::sCatchCountTag(int pCnttagid)
{
  ParameterList params;
  params.append("cnttag_id", pCnttagid);
  params.append("mode", "edit");

  countTag newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleIM::sCharacteristics()
{
  omfgThis->handleNewWindow(new characteristics());
}
