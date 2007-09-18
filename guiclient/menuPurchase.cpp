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

//  menuPurchase.cpp
//  Created 01/01/2001 JSL
//  Copyright (c) 2001-2007, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QPixmap>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "OpenMFGGUIClient.h"
#include "inputManager.h"

#include <pocluster.h>

#include "purchaseOrder.h"
#include "unpostedPurchaseOrders.h"
#include "printPurchaseOrder.h"
#include "printPurchaseOrdersByAgent.h"
#include "deliverPurchaseOrder.h"
#include "postPurchaseOrder.h"
#include "postPurchaseOrdersByAgent.h"
#include "closePurchaseOrder.h"
#include "reschedulePoitem.h"
#include "changePoitemQty.h"
#include "addPoComment.h"

#include "dspUninvoicedReceivings.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "openVouchers.h"
#include "voucheringEditList.h"
#include "postVouchers.h"

#include "itemSource.h"
#include "itemSources.h"

#include "dspPurchaseReqsByItem.h"
#include "dspPurchaseReqsByPlannerCode.h"
#include "dspItemSitesByParameterList.h"
#include "dspPoItemsByVendor.h"
#include "dspPoItemsByItem.h"
#include "dspPoItemsByDate.h"
#include "dspPoItemsByBufferStatus.h"
#include "dspPoHistory.h"
#include "dspItemSourcesByVendor.h"
#include "dspItemSourcesByItem.h"
#include "buyCard.h"
#include "dspPoItemReceivingsByVendor.h"
#include "dspPoItemReceivingsByItem.h"
#include "dspPoItemReceivingsByDate.h"
#include "dspPoPriceVariancesByVendor.h"
#include "dspPoPriceVariancesByItem.h"
#include "dspPoDeliveryDateVariancesByItem.h"
#include "dspPoDeliveryDateVariancesByVendor.h"
#include "dspPoReturnsByVendor.h"
#include "dspPOsByDate.h"
#include "dspPOsByVendor.h"

#include "printPoForm.h"
#include "printVendorForm.h"
#include "printAnnodizingPurchaseRequests.h"

#include "vendor.h"
#include "searchForCRMAccount.h"
#include "vendors.h"
#include "vendorTypes.h"
#include "plannerCodes.h"
#include "rejectCodes.h"
#include "termses.h"
#include "expenseCategories.h"
#include "apAccountAssignments.h"

#include "dspItemsWithoutItemSources.h"
#include "assignItemToPlannerCode.h"
#include "assignClassCodeToPlannerCode.h"

#include "menuPurchase.h"

menuPurchase::menuPurchase(OpenMFGGUIClient *Pparent) :
  QObject(Pparent, "poModule")
{
  parent = Pparent;

  toolBar = new QToolBar(tr("Purchase Tools"));
  toolBar->setObjectName("Purchase Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowPOToolbar"))
    parent->addToolBar(toolBar);
    
  mainMenu = new QMenu();
  requestMenu = new QMenu();
  ordersMenu = new QMenu();
  vouchersMenu = new QMenu();
  itemSourcesMenu = new QMenu();
  vendorMenu = new QMenu();
  formsMenu = new QMenu();
  reportsMenu = new QMenu();
  reportsPrMenu = new QMenu();
  reportsPoMenu = new QMenu();
  reportsPoItemsMenu = new QMenu();
  reportsItemSrcMenu = new QMenu();
  reportsRcptRtrnMenu = new QMenu();
  reportsPriceVarMenu = new QMenu();
  reportsDelvVarMenu = new QMenu();
  masterInfoMenu = new QMenu();
  utilitiesMenu = new QMenu();

  actionProperties acts[] = {
    //  Purchase | Requisitions
    { "menu", tr("Purchase &Requests"), (char*)requestMenu, mainMenu, true, NULL, NULL, true , NULL },
    { "po.dspPurchaseRequestsByPlannerCode", tr("by &Planner Code..."), SLOT(sDspPurchaseReqsByPlannerCode()), requestMenu, _privleges->check("ViewPurchaseRequests"), new QPixmap(":/images/dspPurchaseReqByPlannerCode.png"), toolBar, true , "Purchase Requests by Planner Code" },
    { "po.dspPurchaseRequestsByItem", tr("by &Item..."), SLOT(sDspPurchaseReqsByItem()), requestMenu, _privleges->check("ViewPurchaseRequests"), NULL, NULL, true , NULL },

    //  Purchase | Purchase Order
    { "menu", tr("&Purchase Order"), (char*)ordersMenu, mainMenu, true, NULL, NULL, true , NULL },
    { "po.newPurchaseOrder", tr("&New..."), SLOT(sNewPurchaseOrder()), ordersMenu, _privleges->check("MaintainPurchaseOrders"), NULL, NULL, true , NULL },
    { "po.listUnpostedPurchaseOrders", tr("&List Unposted..."), SLOT(sPurchaseOrderEditList()), ordersMenu, (_privleges->check("MaintainPurchaseOrders")) || (_privleges->check("ViewPurchaseOrders")), new QPixmap(":/images/listUnpostedPo.png"), toolBar, true , "List Unposted Purchase Orders" },
    { "separator", NULL, NULL, ordersMenu, true, NULL, NULL, true , NULL },
    { "po.postPurchaseOrder", tr("&Post..."), SLOT(sPostPurchaseOrder()), ordersMenu, _privleges->check("PostPurchaseOrders"), NULL, NULL, true , NULL },
    { "po.postPurchaseOrdersByAgent", tr("Post by A&gent..."), SLOT(sPostPurchaseOrdersByAgent()), ordersMenu, _privleges->check("PostPurchaseOrders"), NULL, NULL, true , NULL },
    { "po.closePurchaseOrder", tr("&Close..."), SLOT(sClosePurchaseOrder()), ordersMenu, _privleges->check("MaintainPurchaseOrders"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, ordersMenu, true, NULL, NULL, true , NULL },
    { "po.reschedulePurchaseOrderItem", tr("&Reschedule..."), SLOT(sReschedulePoitem()), ordersMenu, _privleges->check("ReschedulePurchaseOrders"), NULL, NULL, true , NULL },
    { "wo.changePurchaseOrderItemQty", tr("Change &Qty..."), SLOT(sChangePoitemQty()), ordersMenu, _privleges->check("ChangePurchaseOrderQty"), NULL, NULL, true , NULL },
    { "wo.addCommentToPurchaseOrder", tr("&Add Comment..."), SLOT(sAddPoComment()), ordersMenu, _privleges->check("MaintainPurchaseOrders"), NULL, NULL, true , NULL },

    //  Purchasing | Voucher
    { "menu", tr("&Voucher"), (char*)vouchersMenu, mainMenu, true, NULL, NULL, true , NULL },
    { "po.enterNewVoucher", tr("&New..."), SLOT(sEnterVoucher()), vouchersMenu, _privleges->check("MaintainVouchers"), NULL, NULL, true , NULL },
    { "po.enterNewMiscVoucher", tr("New &Miscellaneous..."), SLOT(sEnterMiscVoucher()), vouchersMenu, _privleges->check("MaintainVouchers"), NULL, NULL, true , NULL },
    { "po.listUnpostedVouchers", tr("&List Unposted..."), SLOT(sUnpostedVouchers()), vouchersMenu, (_privleges->check("MaintainVouchers") || _privleges->check("ViewVouchers")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, vouchersMenu, true, NULL, NULL, true , NULL },
    { "po.postVouchers", tr("&Post..."), SLOT(sPostVouchers()), vouchersMenu, _privleges->check("PostVouchers"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, mainMenu, true, NULL, NULL, true , NULL },

    // Purchasing | Forms
    { "menu", tr("&Forms"), (char*)formsMenu, mainMenu, true, NULL, NULL, true , NULL },
    { "po.printPurchaseOrder", tr("Print Purchase &Order..."), SLOT(sPrintPurchaseOrder()), formsMenu, _privleges->check("PrintPurchaseOrders"), NULL, NULL, true , NULL },
    { "po.printPurchaseOrdersByAgent", tr("Print Purchase Orders by &Agent..."), SLOT(sPrintPurchaseOrdersByAgent()), formsMenu, _privleges->check("PrintPurchaseOrders"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, formsMenu, true, NULL, NULL,  _metrics->boolean("EnableBatchManager") , NULL },
    { "po.schedulePoForEmailDelivery", tr("&Schedule P/O for Email Delivery..."), SLOT(sDeliverPurchaseOrder()), formsMenu, _privleges->check("PrintPurchaseOrders"), NULL, NULL, _metrics->boolean("EnableBatchManager") , NULL },
    { "separator", NULL, NULL, formsMenu, true, NULL, NULL, true , NULL },
    { "po.printPoForm", tr("Print &P/O Form..."), SLOT(sPrintPOForm()), formsMenu, _privleges->check("PrintPurchaseOrders"), NULL, NULL, true , NULL },
    { "po.printVendorForm", tr("Print &Vendor Form..."), SLOT(sPrintVendorForm()), formsMenu, (_privleges->check("MaintainVendors") || _privleges->check("ViewVendors")), NULL, NULL, true , NULL },

    //  Purchasing | Reports
    { "menu", tr("&Reports"), (char*)reportsMenu, mainMenu, true, NULL, NULL, true , NULL },
    
    { "po.dspItemSitesByPlannerCode", tr("Item &Sites..."), SLOT(sDspItemSitesByPlannerCode()), reportsMenu, _privleges->check("ViewItemSites"), NULL, NULL, true , NULL },
    
    // Purchasing | Reports | Item Sources
    { "menu", tr("&Items Sources"), (char*)reportsItemSrcMenu, reportsMenu, true, NULL, NULL, true , NULL },
    { "po.dspItemSourcesByVendor", tr("by &Vendor..."), SLOT(sDspItemSourcesByVendor()), reportsItemSrcMenu, _privleges->check("ViewItemSources"), NULL, NULL, true , NULL },
    { "po.dspItemSourcesByItem", tr("by &Item..."), SLOT(sDspItemSourcesByItem()), reportsItemSrcMenu, _privleges->check("ViewItemSources"), NULL, NULL, true , NULL },
    { "po.dspBuyCard", tr("&Buy Card..."), SLOT(sDspBuyCard()), reportsMenu, _privleges->check("ViewItemSources"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, reportsMenu, true, NULL, NULL, true , NULL },
 
    //  Purchasing | Reports | Purchase Requests
    // { "menu", tr("Purchase Re&quests"), (char*)requestMenu, reportsMenu, true, NULL, NULL, true , NULL },
    // { "separator", NULL, NULL, reportsMenu, true, NULL, NULL, true , NULL },
    
    //  Purchasing | Reports | P/Os
    { "menu", tr("&Purchase Orders"), (char*)reportsPoMenu, reportsMenu, true, NULL, NULL, true , NULL },
    { "po.dspPOsByVendor", tr("by &Vendor..."), SLOT(sDspPOsByVendor()), reportsPoMenu, _privleges->check("ViewPurchaseOrders"), NULL, NULL, true , NULL },
    { "po.dspPOsByDate", tr("by &Date..."), SLOT(sDspPOsByDate()), reportsPoMenu, _privleges->check("ViewPurchaseOrders"), NULL, NULL, true , NULL },
    
    //  Purchasing | Reports | P/O Items
    { "menu", tr("Purchase &Order Items"), (char*)reportsPoItemsMenu, reportsMenu, true, NULL, NULL, true , NULL },
    { "po.dspPoLineItemsByVendor", tr("by &Vendor..."), SLOT(sDspPoItemsByVendor()), reportsPoItemsMenu, _privleges->check("ViewPurchaseOrders"), NULL, NULL, true , NULL },
    { "po.dspPoLineItemsByDate", tr("by &Date..."), SLOT(sDspPoItemsByDate()), reportsPoItemsMenu, _privleges->check("ViewPurchaseOrders"), NULL, NULL, true , NULL },
    { "po.dspPoLineItemsByItem", tr("by &Item..."), SLOT(sDspPoItemsByItem()), reportsPoItemsMenu, _privleges->check("ViewPurchaseOrders"), NULL, NULL, true , NULL },
    
    { "po.dspPoLineItemsByBufferStatus", tr("P/O Item Bu&ffer Status..."), SLOT(sDspPoItemsByBufferStatus()), reportsMenu, _privleges->check("ViewPurchaseOrders"), NULL, NULL, _metrics->boolean("BufferMgt") , NULL },
    { "po.dspPoHistory", tr("Purchase Order &History..."), SLOT(sDspPoHistory()), reportsMenu, _privleges->check("ViewPurchaseOrders"), NULL, NULL, true , NULL },

    { "separator", NULL, NULL, reportsMenu, true, NULL, NULL, true , NULL },
    
    //  Purchasing | Reports | Receipts and Returns
    { "menu", tr("&Receipts and Returns"), (char*)reportsRcptRtrnMenu, reportsMenu, true, NULL, NULL, true , NULL },
    { "po.dspReceiptsAndReturnsByVendor", tr("by &Vendor..."), SLOT(sDspReceiptsReturnsByVendor()), reportsRcptRtrnMenu, _privleges->check("ViewReceiptsReturns"), NULL, NULL, true , NULL },
    { "po.dspReceiptsAndReturnsByDate", tr("by &Date..."), SLOT(sDspReceiptsReturnsByDate()), reportsRcptRtrnMenu, _privleges->check("ViewReceiptsReturns"), NULL, NULL, true , NULL },
    { "po.dspReceiptsAndReturnsByItem", tr("by &Item..."), SLOT(sDspReceiptsReturnsByItem()), reportsRcptRtrnMenu, _privleges->check("ViewReceiptsReturns"), NULL, NULL, true , NULL },

    { "po.uninvoicedReceipts", tr("&Uninvoiced Receipts and Returns..."), SLOT(sDspUninvoicedReceipts()), reportsMenu, (_privleges->check("ViewUninvoicedReceipts") || _privleges->check("MaintainUninvoicedReceipts")), NULL, NULL, true , NULL },
    
    { "separator", NULL, NULL, reportsMenu, true, NULL, NULL, true , NULL },
    
    //  Purchasing | Reports | Price Variances
    { "menu", tr("Price &Variances"), (char*)reportsPriceVarMenu, reportsMenu, true, NULL, NULL, true , NULL },
    { "po.dspPriceVariancesByVendor", tr("by &Vendor..."), SLOT(sDspPriceVariancesByVendor()), reportsPriceVarMenu, _privleges->check("ViewVendorPerformance"), NULL, NULL, true , NULL },
    { "po.dspPriceVariancesByItem", tr("by &Item..."), SLOT(sDspPriceVariancesByItem()), reportsPriceVarMenu, _privleges->check("ViewVendorPerformance"), NULL, NULL, true , NULL },

    //  Purchasing | Reports | Delivery Date Variance
    { "menu", tr("&Delivery Date Variances"), (char*)reportsDelvVarMenu, reportsMenu, true, NULL, NULL, true , NULL },
    { "po.dspDeliveryDateVariancesByVendor", tr("by &Vendor..."), SLOT(sDspPoDeliveryDateVariancesByVendor()), reportsDelvVarMenu, _privleges->check("ViewVendorPerformance"), NULL, NULL, true , NULL },
    { "po.dspDeliveryDateVariancesByItem", tr("by &Item..."), SLOT(sDspPoDeliveryDateVariancesByItem()), reportsDelvVarMenu, _privleges->check("ViewVendorPerformance"), NULL, NULL, true , NULL },
    
    { "po.dspRejectedMaterialByVendor", tr("Rejected &Material..."), SLOT(sDspRejectedMaterialByVendor()), reportsMenu, _privleges->check("ViewVendorPerformance"), NULL, NULL, true , NULL },
    { "po.printAnnodizingPurchaseRequests", tr("Print &Annodizing Purchase Requests..."), SLOT(sPrintAnnodizingPurchaseRequests()),  reportsMenu, _privleges->check("ViewPurchaseRequests"), NULL, NULL, _metrics->boolean("EnablePrintAnnodizingPurchaseRequests") , NULL },
    { "separator", NULL, NULL, reportsMenu, true, NULL, NULL, true , NULL },
    { "po.voucheringEditList", tr("U&nposted Vouchers..."), SLOT(sVoucheringEditList()), reportsMenu, (_privleges->check("MaintainVouchers") || _privleges->check("ViewVouchers")), NULL, NULL, true , NULL },

    { "separator", NULL, NULL, mainMenu, true, NULL, NULL, true , NULL },

    //  Purchasing | Vendor
    { "menu", tr("V&endor"), (char*)vendorMenu, mainMenu, true, NULL, NULL, true , NULL },
    { "po.newVendor", tr("&New..."), SLOT(sNewVendor()), vendorMenu, _privleges->check("MaintainVendors"), NULL, NULL, true , NULL },
    { "po.vendors", tr("&List..."), SLOT(sVendors()), vendorMenu, (_privleges->check("MaintainVendors")) || (_privleges->check("ViewVendors")), NULL, NULL, true , NULL },
    { "po.searchForVendor", tr("&Search..."), SLOT(sSearchForVendor()), vendorMenu, (_privleges->check("MaintainVendors")) || (_privleges->check("ViewVendors")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, vendorMenu, true, NULL, NULL, true , NULL },
    { "po.vendorTypes", tr("&Types..."), SLOT(sVendorTypes()), vendorMenu, (_privleges->check("MaintainVendorTypes")) || (_privleges->check("ViewVendorTypes")), NULL, NULL, true , NULL },
    
     //  P/O | Item Source
    { "menu", tr("&Item Source"), (char*)itemSourcesMenu, mainMenu, true, NULL, NULL, true , NULL },
    { "po.enterNewItemSource", tr("&New..."), SLOT(sNewItemSource()), itemSourcesMenu, _privleges->check("MaintainItemSources"), NULL, NULL, true , NULL },
    { "po.listItemSources", tr("&List..."), SLOT(sItemSources()), itemSourcesMenu, (_privleges->check("MaintainItemSources") || _privleges->check("ViewItemSources")), NULL, NULL, true , NULL },

    { "separator", NULL, NULL, mainMenu, true, NULL, NULL, true , NULL },

    //  Purchasing | Master Information
    { "menu", tr("&Master Information"), (char*)masterInfoMenu, mainMenu, true, NULL, NULL, true , NULL },
    { "po.plannerCodes", tr("&Planner Codes..."), SLOT(sPlannerCodes()), masterInfoMenu, (_privleges->check("MaintainPlannerCodes")) || (_privleges->check("ViewPlannerCodes")), NULL, NULL, true , NULL },
    { "po.rejectCodes", tr("&Reject Codes..."), SLOT(sRejectCodes()), masterInfoMenu, (_privleges->check("MaintainRejectCodes")) || (_privleges->check("ViewRejectCodes")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, masterInfoMenu, true, NULL, NULL, true , NULL },
    { "po.terms", tr("&Terms..."), SLOT(sTerms()), masterInfoMenu, (_privleges->check("MaintainTerms")) || (_privleges->check("ViewTerms")), NULL, NULL, true , NULL },
    { "po.expenseCategories", tr("&Expense Categories..."), SLOT(sExpenseCategories()), masterInfoMenu, (_privleges->check("MaintainExpenseCategories")) || (_privleges->check("ViewExpenseCategories")), NULL, NULL, true , NULL },
    { "po.apAccountAssignments", tr("&A/P Account Assignments..."), SLOT(sAPAssignments()), masterInfoMenu, (_privleges->check("MaintainVendorAccounts")) || (_privleges->check("ViewVendorAccounts")), NULL, NULL, true , NULL },

    // Purchasing | Utilities
    { "menu", tr("&Utilities"), (char*)utilitiesMenu, mainMenu, true, NULL, NULL, true , NULL },
    { "po.itemsWithoutItemSources", tr("&Items without Item Sources..."), SLOT(sItemsWithoutItemSources()), utilitiesMenu, _privleges->check("ViewItemMasters"), NULL, NULL, true , NULL },
    { "po.assignItemToPlannerCode", tr("&Assign Item to Planner Code..."), SLOT(sAssignItemToPlannerCode()), utilitiesMenu, _privleges->check("AssignItemsToPlannerCode"), NULL, NULL, true , NULL },
    { "po.assignItemsToPlannerCodeByClassCode", tr("Assign Item&s to Planner Code..."), SLOT(sAssignClassCodeToPlannerCode()), utilitiesMenu, _privleges->check("AssignItemsToPlannerCode"), NULL, NULL, true , NULL },
  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));

  parent->populateCustomMenu(mainMenu, "Purchase");
  parent->menuBar()->insertItem(tr("P&urchase"), mainMenu);
}

void menuPurchase::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  for (unsigned int i = 0; i < numElems; i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == "menu")
    {
      acts[i].menu->insertItem(acts[i].actionTitle, (QMenu*)(acts[i].slot));
    }
    else if (acts[i].actionName == "separator")
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

void menuPurchase::sNewPurchaseOrder()
{
  ParameterList params;
  params.append("mode", "new");

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuPurchase::sPurchaseOrderEditList()
{
  omfgThis->handleNewWindow(new unpostedPurchaseOrders());
}

void menuPurchase::sPrintPurchaseOrder()
{
  printPurchaseOrder(parent, "", TRUE).exec();
}

void menuPurchase::sPrintPurchaseOrdersByAgent()
{
  printPurchaseOrdersByAgent(parent, "", TRUE).exec();
}

void menuPurchase::sDeliverPurchaseOrder()
{
  deliverPurchaseOrder(parent, "", TRUE).exec();
}

void menuPurchase::sPostPurchaseOrder()
{
  postPurchaseOrder(parent, "", TRUE).exec();
}

void menuPurchase::sPostPurchaseOrdersByAgent()
{
  postPurchaseOrdersByAgent(parent, "", TRUE).exec();
}

void menuPurchase::sClosePurchaseOrder()
{
  closePurchaseOrder(parent, "", TRUE).exec();
}

void menuPurchase::sReschedulePoitem()
{
  reschedulePoitem(parent, "", TRUE).exec();
}

void menuPurchase::sChangePoitemQty()
{
  changePoitemQty(parent, "", TRUE).exec();
}

void menuPurchase::sAddPoComment()
{
  addPoComment(parent, "", TRUE).exec();
}

void menuPurchase::sDspUninvoicedReceipts()
{
  omfgThis->handleNewWindow(new dspUninvoicedReceivings());
}

void menuPurchase::sEnterVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  voucher *newdlg = new voucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuPurchase::sEnterMiscVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  miscVoucher *newdlg = new miscVoucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuPurchase::sUnpostedVouchers()
{
  omfgThis->handleNewWindow(new openVouchers());
}

void menuPurchase::sVoucheringEditList()
{
  omfgThis->handleNewWindow(new voucheringEditList());
}

void menuPurchase::sPostVouchers()
{
  postVouchers(parent, "", TRUE).exec();
}

void menuPurchase::sNewItemSource()
{
  ParameterList params;
  params.append("mode", "new");

  itemSource newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuPurchase::sItemSources()
{
  omfgThis->handleNewWindow(new itemSources());
}

void menuPurchase::sDspPurchaseReqsByItem()
{
  omfgThis->handleNewWindow(new dspPurchaseReqsByItem());
}

void menuPurchase::sDspPurchaseReqsByPlannerCode()
{
  omfgThis->handleNewWindow(new dspPurchaseReqsByPlannerCode());
}

void menuPurchase::sDspItemSitesByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspItemSitesByParameterList *newdlg = new dspItemSitesByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuPurchase::sDspPOsByDate()
{
  omfgThis->handleNewWindow(new dspPOsByDate());
}

void menuPurchase::sDspPOsByVendor()
{
  omfgThis->handleNewWindow(new dspPOsByVendor());
}

void menuPurchase::sDspPoItemsByVendor()
{
  omfgThis->handleNewWindow(new dspPoItemsByVendor());
}

void menuPurchase::sDspPoItemsByItem()
{
  omfgThis->handleNewWindow(new dspPoItemsByItem());
}

void menuPurchase::sDspPoItemsByDate()
{
  omfgThis->handleNewWindow(new dspPoItemsByDate());
}

void menuPurchase::sDspPoItemsByBufferStatus()
{
  omfgThis->handleNewWindow(new dspPoItemsByBufferStatus());
}

void menuPurchase::sDspPoHistory()
{
  omfgThis->handleNewWindow(new dspPoHistory());
}

void menuPurchase::sDspItemSourcesByVendor()
{
  omfgThis->handleNewWindow(new dspItemSourcesByVendor());
}

void menuPurchase::sDspItemSourcesByItem()
{
  omfgThis->handleNewWindow(new dspItemSourcesByItem());
}

void menuPurchase::sDspBuyCard()
{
  omfgThis->handleNewWindow(new buyCard());
}

void menuPurchase::sDspReceiptsReturnsByVendor()
{
  omfgThis->handleNewWindow(new dspPoItemReceivingsByVendor());
}

void menuPurchase::sDspReceiptsReturnsByItem()
{
  omfgThis->handleNewWindow(new dspPoItemReceivingsByItem());
}

void menuPurchase::sDspReceiptsReturnsByDate()
{
  omfgThis->handleNewWindow(new dspPoItemReceivingsByDate());
}

void menuPurchase::sDspPriceVariancesByVendor()
{
  omfgThis->handleNewWindow(new dspPoPriceVariancesByVendor());
}

void menuPurchase::sDspPriceVariancesByItem()
{
  omfgThis->handleNewWindow(new dspPoPriceVariancesByItem());
}

void menuPurchase::sDspPoDeliveryDateVariancesByItem()
{
  omfgThis->handleNewWindow(new dspPoDeliveryDateVariancesByItem());
}

void menuPurchase::sDspPoDeliveryDateVariancesByVendor()
{
  omfgThis->handleNewWindow(new dspPoDeliveryDateVariancesByVendor());
}

void menuPurchase::sDspRejectedMaterialByVendor()
{
  omfgThis->handleNewWindow(new dspPoReturnsByVendor());
}

void menuPurchase::sPrintPOForm()
{
  printPoForm(parent, "", TRUE).exec();
}

void menuPurchase::sPrintVendorForm()
{
  printVendorForm(parent, "", TRUE).exec();
}

void menuPurchase::sPrintAnnodizingPurchaseRequests()
{
  printAnnodizingPurchaseRequests(parent, "", TRUE).exec();
}

//  Master Information
void menuPurchase::sNewVendor()
{
  ParameterList params;
  params.append("mode", "new");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuPurchase::sSearchForVendor()
{
  ParameterList params;
  params.append("crmaccnt_subtype", "vend");

  searchForCRMAccount *newdlg = new searchForCRMAccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuPurchase::sVendors()
{
  omfgThis->handleNewWindow(new vendors());
}

void menuPurchase::sVendorTypes()
{
  omfgThis->handleNewWindow(new vendorTypes());
}

void menuPurchase::sPlannerCodes()
{
  omfgThis->handleNewWindow(new plannerCodes());
}

void menuPurchase::sRejectCodes()
{
  omfgThis->handleNewWindow(new rejectCodes());
}

void menuPurchase::sTerms()
{
  omfgThis->handleNewWindow(new termses());
}

void menuPurchase::sExpenseCategories()
{
  omfgThis->handleNewWindow(new expenseCategories());
}


// Utilities
void menuPurchase::sItemsWithoutItemSources()
{
  omfgThis->handleNewWindow(new dspItemsWithoutItemSources());
}

void menuPurchase::sAssignItemToPlannerCode()
{
  assignItemToPlannerCode(parent, "", TRUE).exec();
}

void menuPurchase::sAssignClassCodeToPlannerCode()
{
  assignClassCodeToPlannerCode(parent, "", TRUE).exec();
}

void menuPurchase::sAPAssignments()
{
  omfgThis->handleNewWindow(new apAccountAssignments());
}
