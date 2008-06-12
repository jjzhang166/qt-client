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

//  menuManufacture.cpp
//  Created 08/22/2000 JSL
//  Copyright (c) 2000-2008, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"

#include "workOrder.h"
#include "explodeWo.h"
#include "implodeWo.h"
#include "woTimeClock.h"
#include "closeWo.h"

#include "printWoTraveler.h"
#include "printWoPickList.h"
#include "printWoRouting.h"

#include "releaseWorkOrdersByPlannerCode.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "changeWoQty.h"
#include "purgeClosedWorkOrders.h"

#include "woMaterialItem.h"
#include "deleteWoMaterialItem.h"
#include "workOrderMaterials.h"

#include "issueWoMaterialBatch.h"
#include "issueWoMaterialItem.h"
#include "returnWoMaterialBatch.h"
#include "returnWoMaterialItem.h"
#include "scrapWoMaterialFromWIP.h"

#include "postProduction.h"
#include "correctProductionPosting.h"
#include "postMiscProduction.h"

#include "woOperation.h"
#include "workOrderOperations.h"
#include "postOperations.h"
#include "correctOperationsPosting.h"

#include "dspWoScheduleByItem.h"
#include "dspWoScheduleByParameterList.h"
#include "dspWoScheduleByWorkOrder.h"
#include "dspWoBufferStatusByParameterList.h"
#include "dspWoHistoryByItem.h"
#include "dspWoHistoryByNumber.h"
#include "dspWoHistoryByClassCode.h"
#include "dspWoMaterialsByItem.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspPendingAvailability.h"
#include "dspWoOperationsByWorkOrder.h"
#include "dspWoOperationsByWorkCenter.h"
#include "dspWoOperationBufrStsByWorkCenter.h"
#include "dspJobCosting.h"
#include "dspMaterialUsageVarianceByBOMItem.h"
#include "dspMaterialUsageVarianceByItem.h"
#include "dspMaterialUsageVarianceByComponentItem.h"
#include "dspMaterialUsageVarianceByWorkOrder.h"
#include "dspMaterialUsageVarianceByWarehouse.h"
#include "dspLaborVarianceByBOOItem.h"
#include "dspLaborVarianceByItem.h"
#include "dspLaborVarianceByWorkCenter.h"
#include "dspLaborVarianceByWorkOrder.h"
#include "dspBreederDistributionVarianceByItem.h"
#include "dspBreederDistributionVarianceByWarehouse.h"
#include "dspWoSoStatus.h"
#include "dspWoSoStatusMismatch.h"
#include "dspWoEffortByUser.h"
#include "dspWoEffortByWorkOrder.h"

#include "printWoForm.h"
#include "printProductionEntrySheet.h"

#include "menuManufacture.h"

menuManufacture::menuManufacture(GUIClient *Pparent) :
  QObject(Pparent, "poModule")
{
  parent = Pparent;

  toolBar = new QToolBar(tr("Manufacture Tools"));
  toolBar->setObjectName("Manufacture Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowWOToolbar"))
    parent->addToolBar(toolBar);

  mainMenu	 = new QMenu(parent);
  ordersMenu	 = new QMenu(parent);
  formsMenu = new QMenu(parent);
  materialsMenu	 = new QMenu(parent);
  materialsIssueMenu = new QMenu(parent);
  materialsReturnMenu = new QMenu(parent);
  operationsMenu = new QMenu(parent);
  transactionsMenu = new QMenu(parent);
  reportsMenu	 = new QMenu(parent);
  reportsScheduleMenu = new QMenu(parent);
  reportsBufrStsMenu = new QMenu(parent);
  reportsHistoryMenu = new QMenu(parent);
  reportsMatlReqMenu = new QMenu(parent);
  reportsOperationsMenu = new QMenu(parent);
  reportsWoTcMenu = new QMenu(parent);
  reportsMatlUseVarMenu = new QMenu(parent);
  reportsLaborVarMenu = new QMenu(parent);
  reportsBrdrDistVarMenu = new QMenu(parent);
  reportsOpenWoMenu = new QMenu(parent);
  utilitiesMenu = new QMenu(parent);

  mainMenu->setObjectName("menu.manu");
  ordersMenu->setObjectName("menu.manu.orders");
  formsMenu->setObjectName("menu.manu.forms");
  materialsMenu->setObjectName("menu.manu.materials");
  materialsIssueMenu->setObjectName("menu.manu.materialsissue");
  materialsReturnMenu->setObjectName("menu.manu.materialsreturn");
  operationsMenu->setObjectName("menu.manu.operations");
  transactionsMenu->setObjectName("menu.manu.transactions");
  reportsMenu->setObjectName("menu.manu.reports");
  reportsScheduleMenu->setObjectName("menu.manu.reportsschedule");
  reportsBufrStsMenu->setObjectName("menu.manu.reportsbufrsts");
  reportsHistoryMenu->setObjectName("menu.manu.reportshistory");
  reportsMatlReqMenu->setObjectName("menu.manu.reportsmatlreq");
  reportsOperationsMenu->setObjectName("menu.manu.reportsoperations");
  reportsWoTcMenu->setObjectName("menu.manu.reportswotc");
  reportsMatlUseVarMenu->setObjectName("menu.manu.reportsmatlusevar");
  reportsLaborVarMenu->setObjectName("menu.manu.reportslaborvar");
  reportsBrdrDistVarMenu->setObjectName("menu.manu.reportsbrdrdistvar");
  reportsOpenWoMenu->setObjectName("menu.manu.reportsopenwo");
  utilitiesMenu->setObjectName("menu.manu.utilities");

  actionProperties acts[] = {
    // Production | Control
    { "menu",			tr("&Work Order"),		(char*)ordersMenu,	  mainMenu,    true,	0, 0, true, NULL },
    { "wo.newWorkOrder",	tr("&New..."),	SLOT(sNewWorkOrder()),	  ordersMenu, _privileges->check("MaintainWorkOrders"), 0, 0, true, NULL },
    { "separator",		NULL,				NULL,			  ordersMenu, true,	0, 0,	true, NULL },
    { "wo.explodeWorkOrder",	tr("E&xplode..."),	SLOT(sExplodeWorkOrder()), ordersMenu, _privileges->check("ExplodeWorkOrders"), 0, 0, true, NULL },
    { "wo.implodeWorkOrder",	tr("&Implode..."),	SLOT(sImplodeWorkOrder()), ordersMenu, _privileges->check("ImplodeWorkOrders"), 0, 0, true, NULL },
    { "wo.releaseWorkOrdersByPlannerCode", tr("&Release..."), SLOT(sReleaseWorkOrdersByPlannerCode()),	ordersMenu, _privileges->check("ReleaseWorkOrders"), 0, 0, true, NULL },
    { "wo.closeWorkOrder",	tr("&Close..."),	SLOT(sCloseWorkOrder()),  ordersMenu, _privileges->check("CloseWorkOrders"), 0, 0, true, NULL },
    { "separator",		NULL,				NULL,			  ordersMenu, true,	0, 0,	true, NULL },
    { "wo.reprioritizeWorkOrder",	   tr("Re&prioritize..."),	SLOT(sReprioritizeWorkOrder()), ordersMenu, _privileges->check("ReprioritizeWorkOrders"), 0, 0, true, NULL },
    { "wo.rescheduleWorkOrder",		   tr("Re&schedule..."),	SLOT(sRescheduleWorkOrder()),   ordersMenu, _privileges->check("RescheduleWorkOrders"), 0, 0, true, NULL },
    { "wo.changeWorkOrderQuantity",	   tr("Change &Quantity..."), SLOT(sChangeWorkOrderQty()),	ordersMenu, _privileges->check("ChangeWorkOrderQty"), 0, 0, true, NULL },

    //  Production | W/O Materials
    { "menu",				tr("&Materials"),				(char*)materialsMenu,			mainMenu,	true,	0, 0,	true, NULL },
    { "wo.createWoMaterialRequirement",	tr("&New..."),	SLOT(sCreateWoMaterialRequirement()), materialsMenu, _privileges->check("MaintainWoMaterials"), 0, 0, true, NULL },
    { "wo.maintainWoMaterialRequirements",tr("&Maintain..."),	SLOT(sMaintainWoMaterials()), 		materialsMenu, _privileges->check("MaintainWoMaterials"), 0, 0, true, NULL },
/*  This can be done in maintain matl req.  Remove by version 3.0 if no objections.    
    { "wo.deleteWoMaterialRequirement",	tr("&Delete..."),	SLOT(sDeleteWoMaterialRequirement()), materialsMenu, _privileges->check("MaintainWoMaterials"), 0, 0, true, NULL },
*/
    //  Production | Operations
    { "menu",				tr("&Operations"),			(char*)operationsMenu,	mainMenu,	true,	0, 0, _metrics->boolean("Routings"), NULL },
    { "wo.createWoOperation",		tr("&New..."),		SLOT(sCreateWoOperation()), operationsMenu, _privileges->check("MaintainWoOperations"), 0, 0, _metrics->boolean("Routings"), NULL },
    { "wo.maintainWoOperation",		tr("&Maintain..."),	SLOT(sMaintainWoOperations()), operationsMenu, _privileges->check("MaintainWoOperations"), 0, 0, _metrics->boolean("Routings"), NULL },

    { "separator",		NULL,				NULL,			  mainMenu, true,	0, 0,	true, NULL },
    
    //  Production | Transactions
    { "menu",				tr("&Transactions"),			(char*)transactionsMenu,	mainMenu,	true,	0, 0, true, NULL },
    
    //  Production |Transactions | Issue
    { "menu",				tr("&Issue Material"),				(char*)materialsIssueMenu,			transactionsMenu,	true,	0, 0,	true, NULL },
    { "wo.issueWoMaterialBatch",	tr("&Batch..."),	SLOT(sIssueWoMaterialBatch()), materialsIssueMenu, _privileges->check("IssueWoMaterials"), 0, 0, true, NULL },
    { "wo.issueWoMaterialItem",		tr("&Item..."),	SLOT(sIssueWoMaterialItem()), materialsIssueMenu, _privileges->check("IssueWoMaterials"), 0, 0, true, NULL },
    
    //  Production | Transactions | Return
    { "menu",				tr("Ret&urn Material"),				(char*)materialsReturnMenu,			transactionsMenu,	true,	0, 0,	true, NULL },
    { "wo.returnWoMaterialBatch",	tr("&Batch..."),	SLOT(sReturnWoMaterialBatch()), materialsReturnMenu, _privileges->check("ReturnWoMaterials"), 0, 0, true, NULL },
    { "wo.returnWoMaterialItem",	tr("&Item..."),	SLOT(sReturnWoMaterialItem()), materialsReturnMenu, _privileges->check("ReturnWoMaterials"), 0, 0, true, NULL },
    
    { "wo.scrapWoMaterialFromWo",	tr("&Scrap..."),	SLOT(sScrapWoMaterialFromWo()), transactionsMenu, _privileges->check("ScrapWoMaterials"), 0, 0, true, NULL },
    { "separator",			   NULL,				NULL,				transactionsMenu, true,	0, 0,	_metrics->boolean("Routings") , NULL },
    { "wo.woTimeClock",		tr("Shop Floor &Workbench..."),	SLOT(sWoTimeClock()),	  transactionsMenu, _privileges->check("WoTimeClock"), new QPixmap(":/images/shopFloorWorkbench.png"), toolBar, _metrics->boolean("Routings"), NULL },
    { "wo.postOperations",		tr("&Post Operation..."),		SLOT(sPostOperations()), transactionsMenu, _privileges->check("PostWoOperations"), 0, 0, _metrics->boolean("Routings"), NULL },
    { "wo.correctOperationsPosting",	tr("Co&rrect Operation Posting..."),	SLOT(sCorrectOperationsPosting()), transactionsMenu, _privileges->check("PostWoOperations"), 0, 0, _metrics->boolean("Routings"), NULL },
    { "separator",			   NULL,				NULL,				transactionsMenu, true,	0, 0,	true, NULL },
    { "wo.postProduction",		tr("Post Productio&n..."),		SLOT(sPostProduction()), transactionsMenu, _privileges->check("PostProduction"), 0, 0, true, NULL },
    { "wo.correctProductionPosting",	tr("C&orrect Production Posting..."),	SLOT(sCorrectProductionPosting()), transactionsMenu, _privileges->check("PostProduction"), 0, 0, true, NULL },
    { "wo.closeWorkOrder",	tr("&Close Work Order..."),	SLOT(sCloseWorkOrder()),  transactionsMenu, _privileges->check("CloseWorkOrders"), 0, 0, true, NULL },
    { "separator",			   NULL,				NULL,				transactionsMenu, true,	0, 0,	true, NULL },
    { "wo.postMiscProduction",		tr("Post &Misc. Production..."),	SLOT(sPostMiscProduction()), transactionsMenu, _privileges->check("PostMiscProduction"), 0, 0, true, NULL },

    { "separator",		NULL,				NULL,			  mainMenu, true,	0, 0,	true, NULL },
    
    // Production | Forms
    { "menu",			tr("&Forms"),		(char*)formsMenu,	  mainMenu,    true,	0, 0, true, NULL },
    { "wo.printTraveler",	tr("Print &Traveler..."),	SLOT(sPrintTraveler()),	  formsMenu, _privileges->check("PrintWorkOrderPaperWork"), 0, 0, true, NULL },
    { "wo.printPickList",	tr("Print &Pick List..."),	SLOT(sPrintPickList()),	  formsMenu, _privileges->check("PrintWorkOrderPaperWork"), 0, 0, true, NULL },
    { "wo.printRouting",	tr("Print &Routing..."),		SLOT(sPrintRouting()),	  formsMenu, _privileges->check("PrintWorkOrderPaperWork"), 0, 0, _metrics->boolean("Routings"), NULL },
    { "separator",		NULL,				NULL,			  formsMenu, true,	0, 0,	true, NULL },
    { "wo.rptPrintWorkOrderForm",		tr("Print &Work Order Form..."),	SLOT(sPrintWorkOrderForm()), formsMenu, _privileges->check("PrintWorkOrderPaperWork"), 0, 0, true, NULL },
    { "wo.rptPrintProductionEntrySheet",	tr("Print Production &Entry Sheet..."),	SLOT(sPrintProductionEntrySheet()), formsMenu, _privileges->check("PrintWorkOrderPaperWork"), 0, 0, _metrics->boolean("Routings"), NULL },
    
    //  Production | Reports
    { "menu",				tr("&Reports"),	(char*)reportsMenu,	mainMenu,	true,	0, 0,	true, NULL },
    
    //  Production | Reports | Schedule
    { "menu",				tr("Work Order &Schedule"),	(char*)reportsScheduleMenu,	reportsMenu,	true,	0, 0,	true, NULL },
    { "wo.dspWoScheduleByPlannerCode",	tr("by &Planner Code..."),	SLOT(sDspWoScheduleByPlannerCode()), reportsScheduleMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), new QPixmap(":/images/dspWoScheduleByPlannerCode.png"), toolBar, true, "Work Order Schedule by Planner Code" },
    { "wo.dspWoScheduleByClassCode",	tr("by &Class Code..."),	SLOT(sDspWoScheduleByClassCode()), reportsScheduleMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, true, NULL },
    { "wo.dspWoScheduleByWorkCenter",	tr("by &Work Center..."),	SLOT(sDspWoScheduleByWorkCenter()), reportsScheduleMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, _metrics->boolean("Routings"), NULL },
    { "wo.dspWoScheduleByItemGroup",	tr("by Item &Group..."),	SLOT(sDspWoScheduleByItemGroup()), reportsScheduleMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, true, NULL },
    { "wo.dspWoScheduleByItem",		tr("by &Item..."),	SLOT(sDspWoScheduleByItem()), reportsScheduleMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, true, NULL },
    { "wo.dspWoScheduleByWorkOrder",	tr("by &Work Order..."),	SLOT(sDspWoScheduleByWorkOrder()), reportsScheduleMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, true, NULL },

    //  Production | Reports | Status
    { "menu",				tr("Work Order Sta&tus"),	(char*)reportsBufrStsMenu,	reportsMenu,	true,	0, 0,	_metrics->boolean("BufferMgt"), NULL },
    { "wo.dspWoBufferStatusByPlannerCode",	tr("by &Planner Code..."),	SLOT(sDspWoBufferStatusByPlannerCode()), reportsBufrStsMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")) , 0, 0, _metrics->boolean("BufferMgt"), NULL },
    { "wo.dspWoBufferStatusByClassCode",	tr("by &Class Code..."),	SLOT(sDspWoBufferStatusByClassCode()), reportsBufrStsMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")),  0, 0, _metrics->boolean("BufferMgt") , NULL },
    { "wo.dspWoBufferStatusByItemGroup",	tr("by Item &Group..."),	SLOT(sDspWoBufferStatusByItemGroup()), reportsBufrStsMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, _metrics->boolean("BufferMgt"), NULL },

    { "separator",			NULL,	NULL,	reportsMenu,	true,	0, 0,	true, NULL },
    
    //  Production | Reports | Material Requirements
    { "menu",				tr("&Material Requirements"),	(char*)reportsMatlReqMenu,	reportsMenu,	true,	0, 0,	true, NULL },
    { "wo.dspWoMaterialRequirementsByWorkOrder",	tr("by &Work Order..."),	SLOT(sDspWoMaterialsByWo()), reportsMatlReqMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, true, NULL },
    { "wo.dspWoMaterialRequirementsByComponentItem",	tr("by &Component Item..."),	SLOT(sDspWoMaterialsByComponentItem()), reportsMatlReqMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, true, NULL },

    { "wo.dspInventoryAvailabilityByWorkOrder",		tr("&Inventory Availability..."),	SLOT(sDspInventoryAvailabilityByWorkOrder()), reportsMenu, _privileges->check("ViewInventoryAvailability"), 0, 0, true, NULL },
    { "wo.dspPendingWoMaterialAvailability",	tr("&Pending Material Availability..."),	SLOT(sDspPendingAvailability()), reportsMenu, _privileges->check("ViewInventoryAvailability"), 0, 0, true, NULL },
    { "separator",				NULL,					NULL,	reportsMenu,	true,	0, 0,	_metrics->boolean("Routings"), NULL },

    //  Production | Reports | Operations
    { "menu",				tr("&Operations"),	(char*)reportsOperationsMenu,	reportsMenu,	true,	0, 0,	_metrics->boolean("Routings"), NULL },
    { "wo.dspWoOperationsByWorkCenter",		tr("by &Work Center..."),	SLOT(sDspWoOperationsByWorkCenter()), reportsOperationsMenu, (_privileges->check("MaintainWoOperations") || _privileges->check("ViewWoOperations")), 0, 0, _metrics->boolean("Routings"), NULL },
    { "wo.dspWoOperationsByWorkOrder",		tr("by Work &Order..."),	SLOT(sDspWoOperationsByWo()), reportsOperationsMenu, (_privileges->check("MaintainWoOperations") || _privileges->check("ViewWoOperations")), 0, 0, _metrics->boolean("Routings"), NULL },

    { "wo.dspWoOperationBufrStsByWorkCenter",	tr("Operation Buf&fer Status..."),	SLOT(sDspWoOperationBufrStsByWorkCenter()), reportsMenu, (_privileges->check("MaintainWoOperations") || _privileges->check("ViewWoOperations")), 0, 0, _metrics->boolean("Routings") && _metrics->boolean("BufferMgt"), NULL },

    //  Production | Reports | Production Time Clock
    { "menu",				tr("Production &Time Clock"),	(char*)reportsWoTcMenu,	reportsMenu,	true,	0, 0,	_metrics->boolean("Routings"), NULL },
    { "wo.dspWoEffortByUser",		tr("by &User..."),	SLOT(sDspWoEffortByUser()), reportsWoTcMenu, (_privileges->check("MaintainWoTimeClock") || _privileges->check("ViewWoTimeClock")), 0, 0, _metrics->boolean("Routings"), NULL },
    { "wo.dspWoEffortByWorkOrder",	tr("by &Work Order..."),	SLOT(sDspWoEffortByWorkOrder()), reportsWoTcMenu, (_privileges->check("MaintainWoTimeClock") || _privileges->check("ViewWoTimeClock")), 0, 0, _metrics->boolean("Routings"), NULL },

    { "separator",			NULL,	NULL,	reportsMenu,	true,	0, 0,	true, NULL },
    
    //  Production | Reports | History
    { "menu",				tr("&History"),	(char*)reportsHistoryMenu,	reportsMenu,	true,	0, 0,	true, NULL },
    { "wo.dspWoHistoryByClassCode",	tr("by &Class Code..."),	SLOT(sDspWoHistoryByClassCode()), reportsHistoryMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, true, NULL },
    { "wo.dspWoHistoryByItem",		tr("by &Item..."),	SLOT(sDspWoHistoryByItem()), reportsHistoryMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, true, NULL },
    { "wo.dspWoHistoryByNumber",	tr("by &W/O Number..."),	SLOT(sDspWoHistoryByNumber()), reportsHistoryMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, true, NULL },

    { "separator",			NULL,	NULL,	reportsMenu,	true,	0, 0,	true, NULL },
    { "wo.dspJobCosting",	tr("&Job Costing..."),	SLOT(sDspJobCosting()), reportsMenu, _privileges->check("ViewCosts"), 0, 0, true, NULL },
    
    //  Production | Reports | Material Usage Variance
    { "menu",				tr("Material &Usage Variance"),	(char*)reportsMatlUseVarMenu,	reportsMenu,	true,	0, 0,	true, NULL },
    { "wo.dspMaterialUsageVarianceByWarehouse",	tr("by Ware&house..."),	SLOT(sDspMaterialUsageVarianceByWarehouse()), reportsMatlUseVarMenu, _privileges->check("ViewMaterialVariances"), 0, 0, true, NULL },
    { "wo.dspMaterialUsageVarianceByItem",	tr("by &Item..."),	SLOT(sDspMaterialUsageVarianceByItem()), reportsMatlUseVarMenu, _privileges->check("ViewMaterialVariances"), 0, 0, true, NULL },
    { "wo.dspMaterialUsageVarianceByBOMItem",	tr("by &BOM Item..."),	SLOT(sDspMaterialUsageVarianceByBOMItem()), reportsMatlUseVarMenu, _privileges->check("ViewMaterialVariances"), 0, 0, true, NULL },
    { "wo.dspMaterialUsageVarianceByComponentItem",	tr("by &Component Item..."),	SLOT(sDspMaterialUsageVarianceByComponentItem()), reportsMatlUseVarMenu, _privileges->check("ViewMaterialVariances"), 0, 0, true, NULL },
    { "wo.dspMaterialUsageVarianceByWorkOrder",	tr("by &Work Order..."),	SLOT(sDspMaterialUsageVarianceByWorkOrder()), reportsMatlUseVarMenu, _privileges->check("ViewMaterialVariances"), 0, 0, true, NULL },

    //  Production | Reports | Labor Variance
    { "menu",				tr("&Labor Variance"),	(char*)reportsLaborVarMenu,	reportsMenu,	true,	0, 0,	_metrics->boolean("Routings"), NULL },
    { "wo.dspLaborVarianceByWorkCenter",	tr("by &Work Center..."),	SLOT(sDspLaborVarianceByWorkCenter()), reportsLaborVarMenu, _privileges->check("ViewLaborVariances"), 0, 0, _metrics->boolean("Routings"), NULL },
    { "wo.dspLaborVarianceByItem",		tr("by &Item..."),	SLOT(sDspLaborVarianceByItem()), reportsLaborVarMenu, _privileges->check("ViewLaborVariances"), 0, 0, _metrics->boolean("Routings"), NULL },
    { "wo.dspLaborVarianceByBOOItem",		tr("by &BOO Item..."),	SLOT(sDspLaborVarianceByBOOItem()), reportsLaborVarMenu, _privileges->check("ViewLaborVariances"), 0, 0, _metrics->boolean("Routings"), NULL },
    { "wo.dspLaborVarianceByWorkOrder",		tr("by Work &Order..."),	SLOT(sDspLaborVarianceByWorkOrder()), reportsLaborVarMenu, _privileges->check("ViewLaborVariances"), 0, 0, _metrics->boolean("Routings"), NULL },

    //  Production | Reports | Breeder Distribution Variance
    { "menu",				tr("Breeder &Distribution Variance"),	(char*)reportsBrdrDistVarMenu,	reportsMenu,	true,	0, 0,	_metrics->boolean("BBOM"), NULL },
    { "wo.dspBreederDistributionVarianceByWarehouse",	tr("by &Warehouse..."),	SLOT(sDspBreederDistributionVarianceByWarehouse()), reportsBrdrDistVarMenu, _privileges->check("ViewBreederVariances"), 0, 0, _metrics->boolean("BBOM"), NULL },
    { "wo.dspBreederDistributionVarianceByItem",	tr("by &Item..."),	SLOT(sDspBreederDistributionVarianceByItem()), reportsBrdrDistVarMenu, _privileges->check("ViewBreederVariances"), 0, 0, _metrics->boolean("BBOM"), NULL },

    { "separator",					NULL,	NULL,	reportsMenu,	true,	0, 0,	true, NULL },
    
    //  Production | Reports | Open Work Orders
    { "menu",				tr("Ope&n Work Orders"),	(char*)reportsOpenWoMenu,	reportsMenu,	true,	0, 0,	true, NULL },
    { "wo.dspOpenWorkOrdersWithClosedParentSalesOrders",tr("with &Closed Parent Sales Orders..."),	SLOT(sDspWoSoStatusMismatch()), reportsOpenWoMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, true, NULL },
    { "wo.dspOpenWorkOrdersWithParentSalesOrders",	tr("with &Parent Sales Orders..."),	SLOT(sDspWoSoStatus()), reportsOpenWoMenu, (_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders")), 0, 0, true, NULL },

    { "separator", NULL, NULL, mainMenu, true,	0, 0, true, NULL },
    { "menu", tr("&Utilities"), (char*)utilitiesMenu, mainMenu, true, NULL, NULL, true, NULL },
    { "wo.purgeClosedWorkOrder",	   tr("Pur&ge Closed Work Orders..."),	SLOT(sPurgeClosedWorkOrders()),	utilitiesMenu, _privileges->check("PurgeWorkOrders"), 0, 0, true, NULL },
  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));

  parent->populateCustomMenu(mainMenu, "Manufacture");
  parent->menuBar()->insertItem(tr("&Manufacture"), mainMenu);
}

void menuManufacture::addActionsToMenu(actionProperties acts[], unsigned int numElems)
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

void menuManufacture::sNewWorkOrder()
{
  ParameterList params;
  params.append("mode", "new");

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuManufacture::sExplodeWorkOrder()
{
  explodeWo(parent, "", TRUE).exec();
}

void menuManufacture::sImplodeWorkOrder()
{
  implodeWo(parent, "", TRUE).exec();
}

void menuManufacture::sWoTimeClock()
{
  omfgThis->handleNewWindow(new woTimeClock());
}

void menuManufacture::sPrintTraveler()
{
  printWoTraveler(parent, "", TRUE).exec();
}

void menuManufacture::sPrintPickList()
{
  printWoPickList(parent, "", TRUE).exec();
}

void menuManufacture::sPrintRouting()
{
  printWoRouting(parent, "", TRUE).exec();
}

void menuManufacture::sCloseWorkOrder()
{
  closeWo(parent, "", TRUE).exec();
}

void menuManufacture::sReleaseWorkOrdersByPlannerCode()
{
  releaseWorkOrdersByPlannerCode(parent, "", TRUE).exec();
}

void menuManufacture::sReprioritizeWorkOrder()
{
  reprioritizeWo(parent, "", TRUE).exec();
}

void menuManufacture::sRescheduleWorkOrder()
{
  rescheduleWo(parent, "", TRUE).exec();
}

void menuManufacture::sChangeWorkOrderQty()
{
  changeWoQty(parent, "", TRUE).exec();
}

void menuManufacture::sPurgeClosedWorkOrders()
{
  purgeClosedWorkOrders(parent, "", TRUE).exec();
}

void menuManufacture::sCreateWoMaterialRequirement()
{
  ParameterList params;
  params.append("mode", "new");

  woMaterialItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuManufacture::sDeleteWoMaterialRequirement()
{
  deleteWoMaterialItem(parent, "", TRUE).exec();
}

void menuManufacture::sMaintainWoMaterials()
{
  omfgThis->handleNewWindow(new workOrderMaterials());
}

void menuManufacture::sIssueWoMaterialBatch()
{
  issueWoMaterialBatch(parent, "", TRUE).exec();
}

void menuManufacture::sIssueWoMaterialItem()
{
  issueWoMaterialItem(parent, "", TRUE).exec();
}

void menuManufacture::sReturnWoMaterialBatch()
{
  returnWoMaterialBatch(parent, "", TRUE).exec();
}

void menuManufacture::sReturnWoMaterialItem()
{
  returnWoMaterialItem(parent, "", TRUE).exec();
}

void menuManufacture::sScrapWoMaterialFromWo()
{
  scrapWoMaterialFromWIP(parent, "", TRUE).exec();
}

void menuManufacture::sPostProduction()
{
  postProduction(parent, "", TRUE).exec();
}

void menuManufacture::sCorrectProductionPosting()
{
  correctProductionPosting(parent, "", TRUE).exec();
}

void menuManufacture::sPostMiscProduction()
{
  postMiscProduction(parent, "", TRUE).exec();
}

void menuManufacture::sCreateWoOperation()
{
  ParameterList params;
  params.append("mode", "new");

  woOperation newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuManufacture::sMaintainWoOperations()
{
  omfgThis->handleNewWindow(new workOrderOperations());
}

void menuManufacture::sPostOperations()
{
  postOperations(parent, "", TRUE).exec();
}

void menuManufacture::sCorrectOperationsPosting()
{
  correctOperationsPosting(parent, "", TRUE).exec();
}

void menuManufacture::sDspWoHistoryByItem()
{
  omfgThis->handleNewWindow(new dspWoHistoryByItem());
}

void menuManufacture::sDspWoHistoryByNumber()
{
  omfgThis->handleNewWindow(new dspWoHistoryByNumber());
}

void menuManufacture::sDspWoHistoryByClassCode()
{
  omfgThis->handleNewWindow(new dspWoHistoryByClassCode());
}

void menuManufacture::sDspWoMaterialsByComponentItem()
{
  omfgThis->handleNewWindow(new dspWoMaterialsByItem());
}

void menuManufacture::sDspWoMaterialsByWo()
{
  omfgThis->handleNewWindow(new dspWoMaterialsByWorkOrder());
}

void menuManufacture::sDspInventoryAvailabilityByWorkOrder()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityByWorkOrder());
}

void menuManufacture::sDspPendingAvailability()
{
  omfgThis->handleNewWindow(new dspPendingAvailability());
}

void menuManufacture::sDspWoOperationsByWorkCenter()
{
  omfgThis->handleNewWindow(new dspWoOperationsByWorkCenter());
}

void menuManufacture::sDspWoOperationsByWo()
{
  omfgThis->handleNewWindow(new dspWoOperationsByWorkOrder());
}

void menuManufacture::sDspWoOperationBufrStsByWorkCenter()
{
  omfgThis->handleNewWindow(new dspWoOperationBufrStsByWorkCenter());
}

void menuManufacture::sDspWoScheduleByItem()
{
  omfgThis->handleNewWindow(new dspWoScheduleByItem());
}

void menuManufacture::sDspWoScheduleByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspWoScheduleByParameterList *newdlg = new dspWoScheduleByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuManufacture::sDspWoScheduleByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspWoScheduleByParameterList *newdlg = new dspWoScheduleByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuManufacture::sDspWoScheduleByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspWoScheduleByParameterList *newdlg = new dspWoScheduleByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuManufacture::sDspWoScheduleByWorkCenter()
{
  ParameterList params;
  params.append("wrkcnt");

  dspWoScheduleByParameterList *newdlg = new dspWoScheduleByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuManufacture::sDspWoScheduleByWorkOrder()
{
  omfgThis->handleNewWindow(new dspWoScheduleByWorkOrder());
}

void menuManufacture::sDspWoBufferStatusByItem()
{
  //omfgThis->handleNewWindow(new dspWoBufferStatusByItem());
}

void menuManufacture::sDspWoBufferStatusByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuManufacture::sDspWoBufferStatusByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuManufacture::sDspWoBufferStatusByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuManufacture::sDspJobCosting()
{
  omfgThis->handleNewWindow(new dspJobCosting());
}

void menuManufacture::sDspMaterialUsageVarianceByBOMItem()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByBOMItem());
}

void menuManufacture::sDspMaterialUsageVarianceByItem()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByItem());
}

void menuManufacture::sDspMaterialUsageVarianceByComponentItem()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByComponentItem());
}

void menuManufacture::sDspMaterialUsageVarianceByWorkOrder()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByWorkOrder());
}

void menuManufacture::sDspWoEffortByWorkOrder()
{
  omfgThis->handleNewWindow(new dspWoEffortByWorkOrder());
}

void menuManufacture::sDspWoEffortByUser()
{
  omfgThis->handleNewWindow(new dspWoEffortByUser());
}

void menuManufacture::sDspMaterialUsageVarianceByWarehouse()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByWarehouse());
}

void menuManufacture::sDspLaborVarianceByBOOItem()
{
  omfgThis->handleNewWindow(new dspLaborVarianceByBOOItem());
}

void menuManufacture::sDspLaborVarianceByItem()
{
  omfgThis->handleNewWindow(new dspLaborVarianceByItem());
}

void menuManufacture::sDspLaborVarianceByWorkCenter()
{
  omfgThis->handleNewWindow(new dspLaborVarianceByWorkCenter());
}

void menuManufacture::sDspLaborVarianceByWorkOrder()
{
  omfgThis->handleNewWindow(new dspLaborVarianceByWorkOrder());
}

void menuManufacture::sDspBreederDistributionVarianceByItem()
{
  omfgThis->handleNewWindow(new dspBreederDistributionVarianceByItem());
}

void menuManufacture::sDspBreederDistributionVarianceByWarehouse()
{
  omfgThis->handleNewWindow(new dspBreederDistributionVarianceByWarehouse());
}

void menuManufacture::sDspWoSoStatusMismatch()
{
  omfgThis->handleNewWindow(new dspWoSoStatusMismatch());
}

void menuManufacture::sDspWoSoStatus()
{
  omfgThis->handleNewWindow(new dspWoSoStatus());
}

void menuManufacture::sPrintWorkOrderForm()
{
  printWoForm(parent, "", TRUE).exec();
}

void menuManufacture::sPrintProductionEntrySheet()
{
  printProductionEntrySheet(parent, "", TRUE).exec();
}
