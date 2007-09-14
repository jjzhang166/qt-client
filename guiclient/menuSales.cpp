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

//  menuSales.cpp
//  Created 02/22/2000 JSL
//  Copyright (c) 2000-2007, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QToolBar>

#include <parameter.h>

#include "OpenMFGGUIClient.h"

#include "salesOrder.h"
#include "openSalesOrders.h"
#include "rescheduleSoLineItems.h"
#include "quotes.h"

#include "packingListBatch.h"
#include "printPackingList.h"

#include "uninvoicedShipments.h"
#include "selectShippedOrders.h"
#include "selectOrderForBilling.h"
#include "billingEditList.h"
#include "dspBillingSelections.h"
#include "postBillingSelections.h"
#include "unpostedInvoices.h"
#include "reprintInvoices.h"
#include "deliverInvoice.h"
#include "printInvoices.h"
#include "printInvoicesByShipvia.h"
#include "purgeInvoices.h"

#include "creditMemo.h"
#include "unpostedCreditMemos.h"
#include "creditMemoEditList.h"
#include "printCreditMemos.h"
#include "reprintCreditMemos.h"
#include "postCreditMemos.h"
#include "purgeCreditMemos.h"

#include "postInvoices.h"

#include "itemListPrice.h"
#include "updateListPricesByProductCategory.h"
#include "itemPricingSchedules.h"
#include "pricingScheduleAssignments.h"
#include "sales.h"
#include "updatePricesByProductCategory.h"
#include "updatePricesByPricingSchedule.h"

#include "dspPricesByItem.h"
#include "dspPricesByCustomer.h"
#include "dspPricesByCustomerType.h"

#include "dspSalesOrdersByCustomer.h"
#include "dspSalesOrdersByItem.h"
#include "dspSalesOrdersByCustomerPO.h"
#include "dspSalesOrdersByParameterList.h"
#include "dspQuotesByCustomer.h"
#include "dspQuotesByItem.h"
#include "dspCustomersByCustomerType.h"
#include "dspCustomersByCharacteristic.h"
#include "dspCustomerInformation.h"
#include "dspInventoryAvailabilityByItem.h"
#include "dspInventoryAvailabilityBySalesOrder.h"
#include "dspSalesOrderStatus.h"
#include "dspBacklogByItem.h"
#include "dspBacklogBySalesOrder.h"
#include "dspBacklogByCustomer.h"
#include "dspBacklogByParameterList.h"
#include "dspSummarizedBacklogByWarehouse.h"
#include "dspPartiallyShippedOrders.h"
#include "dspEarnedCommissions.h"
#include "dspBriefEarnedCommissions.h"
#include "dspSummarizedTaxableSales.h"

#include "dspSalesHistoryByCustomer.h"
#include "dspSalesHistoryByBilltoName.h"
#include "dspSalesHistoryByShipTo.h"
#include "dspSalesHistoryByParameterList.h"
#include "dspSalesHistoryByItem.h"
#include "dspSalesHistoryBySalesrep.h"
#include "dspBriefSalesHistoryByCustomer.h"
#include "dspBriefSalesHistoryByCustomerType.h"
#include "dspBriefSalesHistoryBySalesRep.h"
#include "dspBookingsByCustomer.h"
#include "dspBookingsByCustomerGroup.h"
#include "dspBookingsByShipTo.h"
#include "dspBookingsByProductCategory.h"
#include "dspBookingsByItem.h"
#include "dspBookingsBySalesRep.h"
#include "dspSummarizedSalesByCustomer.h"
#include "dspSummarizedSalesByCustomerType.h"
#include "dspSummarizedSalesByCustomerByItem.h"
#include "dspSummarizedSalesByCustomerTypeByItem.h"
#include "dspSummarizedSalesByItem.h"
#include "dspSummarizedSalesBySalesRep.h"
#include "dspSummarizedSalesHistoryByShippingZone.h"
#include "dspTimePhasedBookingsByItem.h"
#include "dspTimePhasedBookingsByProductCategory.h"
#include "dspTimePhasedBookingsByCustomer.h"
#include "dspTimePhasedSalesByItem.h"
#include "dspTimePhasedSalesByProductCategory.h"
#include "dspTimePhasedSalesByCustomer.h"
#include "dspTimePhasedSalesByCustomerGroup.h"
#include "dspTimePhasedSalesByCustomerByItem.h"

#include "printSoForm.h"
#include "deliverSalesOrder.h"

#include "customer.h"
#include "searchForCRMAccount.h"
#include "customers.h"
#include "prospect.h"
#include "prospects.h"
#include "updateCreditStatusByCustomer.h"
#include "customerGroups.h"
#include "shipVias.h"
#include "shippingChargeTypes.h"
#include "taxCodes.h"
#include "customerTypes.h"
#include "termses.h"
#include "salesReps.h"
#include "shippingZones.h"
#include "shippingForms.h"
#include "salesAccounts.h"
#include "arAccountAssignments.h"
#include "customerFormAssignments.h"
#include "salesCategories.h"

#include "dspCustomerInformationExport.h"
#include "reassignCustomerTypeByCustomerType.h"
#include "characteristics.h"

#include "archRestoreSalesHistory.h"

// START_RW
#include "rwInterface.h"
#include "exportCustomers.h"
// END_RW

#include "menuSales.h"

menuSales::menuSales(OpenMFGGUIClient *pParent) :
  QObject(pParent, "soModule")
{
  parent = pParent;

  toolBar = new QToolBar(tr("Sales Tools"));
  toolBar->setObjectName("Sales Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowSOToolbar"))
    parent->addToolBar(toolBar);

  mainMenu = new QMenu();
  quotesMenu = new QMenu();
  ordersMenu = new QMenu();
  billingMenu = new QMenu();
  billingInvoicesMenu = new QMenu();
  billingCreditMemosMenu= new QMenu();
  billingFormsMenu= new QMenu();
  lookupMenu = new QMenu();
  lookupQuoteMenu = new QMenu();
  lookupSoMenu = new QMenu();
  formsMenu = new QMenu();
  reportsMenu = new QMenu();
  reportsCustomersMenu = new QMenu();
  reportsInvAvailMenu = new QMenu();
  reportsBacklogMenu = new QMenu();
  analysisMenu = new QMenu();
  analysisBookMenu = new QMenu();
  analysisSumHistMenu = new QMenu();
  analysisHistMenu = new QMenu();
  analysisBrfHistMenu = new QMenu();
  analysisTpBookMenu = new QMenu();
  analysisTpHistMenu = new QMenu();
  prospectMenu = new QMenu();
  customerMenu = new QMenu();
  pricingReportsMenu = new QMenu();
  pricingMenu = new QMenu();
  pricingUpdateMenu = new QMenu();
  masterInfoMenu = new QMenu();
  utilitiesMenu = new QMenu();

  actionProperties acts[] = {
    // Sales | Quotes
    { "menu",	tr("&Quote"),	(char*)quotesMenu,	mainMenu,	true,	NULL, NULL, true, NULL },
    { "so.newQuote", tr("&New..."),	SLOT(sNewQuote()), quotesMenu, _privleges->check("MaintainQuotes"),	NULL, NULL, true, NULL },
    { "so.listQuotes", tr("&List..."),	SLOT(sQuotes()), quotesMenu, (_privleges->check("MaintainQuotes") || _privleges->check("ViewQuotes")),	NULL, NULL, true, NULL },
    
    // Sales | Sales Order
    { "menu",	tr("&Sales Order"),	(char*)ordersMenu,	mainMenu,	true,	NULL, NULL, true, NULL },
    { "so.newSalesOrder", 	     tr("&New..."),		SLOT(sNewSalesOrder()),   ordersMenu, _privleges->check("MaintainSalesOrders"), NULL, NULL,	 true, NULL },
    { "so.listOpenSalesOrders",      tr("&List Open..."),	SLOT(sOpenSalesOrders()), ordersMenu, (_privleges->check("MaintainSalesOrders") || _privleges->check("ViewSalesOrders")),	new QPixmap(":/images/listOpenSalesOrders.png"), toolBar,  true, "List Open Sales Orders" },
    { "separator",	NULL,	NULL,	ordersMenu,	true,		NULL, NULL, true, NULL },
    { "so.rescheduleAllSoLineItems", tr("&Reschedule..."),	SLOT(sRescheduleSoLineItems()),  ordersMenu, _privleges->check("MaintainSalesOrders"),	 NULL, NULL, true, NULL },
   
    // Sales | Billing
    { "menu",	tr("&Billing"),     (char*)billingMenu,		mainMenu,	true,	NULL, NULL, true, NULL },
    
    // Sales | Billing | Invoice
    { "menu",	tr("&Invoice"),   (char*)billingInvoicesMenu,	billingMenu,	true,	NULL, NULL, true, NULL },
    { "so.uninvoicedShipments",		     tr("&Uninvoiced Shipments..."),			SLOT(sUninvoicedShipments()), 		billingInvoicesMenu, _privleges->check("SelectBilling"),	 new QPixmap(":/images/uninvoicedShipments"), toolBar, true, NULL },
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	true,		NULL, NULL, true, NULL },
    { "so.selectAllShippedOrdersForBilling", tr("Select &All Shipped Orders for Billing..."),	SLOT(sSelectShippedOrdersForBilling()), billingInvoicesMenu, _privleges->check("SelectBilling"),	NULL, NULL, true, NULL },
    { "so.selectOrderForBilling",	     tr("Select &Order for Billing..."),			SLOT(sSelectOrderForBilling()),		billingInvoicesMenu, _privleges->check("SelectBilling"),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	true,		NULL, NULL, true, NULL },
    { "so.billingEditList",		     tr("&Billing Edit List..."),	SLOT(sBillingEditList()), billingInvoicesMenu, _privleges->check("SelectBilling"),	NULL, NULL, true, NULL },
    { "so.dspBillingSelections",	     tr("Billing &Selections..."),	SLOT(sDspBillingSelections()), billingInvoicesMenu, _privleges->check("SelectBilling"), new QPixmap(":/images/billingSelections"), toolBar, true, NULL },
    { "so.postBillingSelections",	     tr("&Post Billing Selections..."),	SLOT(sPostBillingSelections()), billingInvoicesMenu, _privleges->check("SelectBilling"),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	true,		NULL, NULL, true, NULL },
    { "so.listUnpostedInvoices",	     tr("&List Unposted Invoices..."),	SLOT(sUnpostedInvoices()), billingInvoicesMenu, _privleges->check("SelectBilling"),	NULL, NULL,  true, NULL },
    { "so.postInvoices",		     tr("Post &Invoices..."),		SLOT(sPostInvoices()), billingInvoicesMenu, _privleges->check("PostMiscInvoices"),	NULL, NULL, true, NULL },

    // Sales | Billing | Credit Memo
    { "menu",	tr("&Credit Memo"), (char*)billingCreditMemosMenu,	billingMenu,	true,	NULL, NULL, true, NULL },
    { "so.newCreditMemo",		     tr("&New..."),		SLOT(sNewCreditMemo()), billingCreditMemosMenu, _privleges->check("MaintainCreditMemos"),	NULL, NULL, true, NULL },
    { "so.listUnpostedCreditMemos",	     tr("&List Unposted..."),	SLOT(sUnpostedCreditMemos()), billingCreditMemosMenu, (_privleges->check("MaintainCreditMemos") || _privleges->check("ViewCreditMemos")),	NULL, NULL, true, NULL },
    { "so.creditMemoEditList",		     tr("&Edit List..."),	SLOT(sCreditMemoEditList()), billingCreditMemosMenu, (_privleges->check("MaintainCreditMemos") || _privleges->check("ViewCreditMemos")),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	billingCreditMemosMenu,	true,		NULL, NULL, true, NULL },
    { "so.postCreditMemos",		     tr("&Post..."),	SLOT(sPostCreditMemos()), billingCreditMemosMenu, _privleges->check("PostARDocuments"),	NULL, NULL, true, NULL },

    { "separator",	NULL,	NULL,	billingMenu,	true,		NULL, NULL, true, NULL },
    
    // Sales | Billing | Forms
    { "menu",	tr("&Forms"), (char*)billingFormsMenu,	billingMenu,	true,	NULL, NULL, true, NULL },
    { "so.printInvoices",		     tr("&Print Invoices..."),		SLOT(sPrintInvoices()), billingFormsMenu, _privleges->check("PrintInvoices"),	NULL, NULL, true, NULL },
    { "so.printInvoicesByShipvia",	     tr("Print Invoices by Ship &Via..."),	SLOT(sPrintInvoicesByShipvia()), billingFormsMenu, _privleges->check("PrintInvoices"),	NULL, NULL, true, NULL },
    { "so.reprintInvoices",		     tr("&Re-Print Invoices..."),	SLOT(sReprintInvoices()), billingFormsMenu, _privleges->check("PrintInvoices"),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	billingFormsMenu,	true,		NULL, NULL,  _metrics->boolean("EnableBatchManager") , NULL },
    { "so.scheduleInvoiceForEmailDelivery",  tr("Schedule Invoice for &Email Delivery..."),	SLOT(sDeliverInvoice()), billingFormsMenu, _privleges->check("PrintInvoices"),	NULL, NULL, _metrics->boolean("EnableBatchManager") , NULL },
    { "separator",	NULL,	NULL,	billingFormsMenu,	true,		NULL, NULL, true , NULL },
    { "so.printCreditMemos",		     tr("Print &Credit Memos..."),	SLOT(sPrintCreditMemos()), billingFormsMenu, _privleges->check("PrintCreditMemos"),	NULL, NULL, true, NULL },
    { "so.reprintCreditMemos",		     tr("Re-Print Credit &Memos..."),	SLOT(sReprintCreditMemos()), billingFormsMenu, _privleges->check("PrintCreditMemos"),	NULL, NULL, true, NULL },

// START_RW
    { "separator",	NULL,	NULL,	billingMenu,	true,	NULL,	NULL,	_metrics->boolean("EnableExternalAccountingInterface"), NULL },
    { "so.exportAROpenItemsAndDistributions", tr("Export Unposted A/R Open Items and Distributions..."), SLOT(sPostAROpenAndDist()), billingMenu, _privleges->check("PostARDocuments"), NULL, NULL, _metrics->boolean("EnableExternalAccountingInterface"), NULL },
// END_RW

    { "separator",	NULL,	NULL,	mainMenu,	true,		NULL, NULL, true, NULL },
    
    // Sales | Lookup
    { "menu",	tr("&Lookup"),           (char*)lookupMenu,	mainMenu,	true,	NULL, NULL, true, NULL },
    
    // Sales | Lookup | Quote Lookup
    { "menu",	tr("&Quote"),           (char*)lookupQuoteMenu,	lookupMenu,	true,	NULL, NULL, true, NULL },
    { "so.dspQuoteLookupByCustomer", tr("by &Customer..."),	SLOT(sDspQuoteLookupByCustomer()), lookupQuoteMenu, _privleges->check("ViewQuotes"),	NULL, NULL, true, NULL },
    { "so.dspQuoteOrderLookupByItem", tr("by &Item..."),	SLOT(sDspQuoteLookupByItem()), lookupQuoteMenu, _privleges->check("ViewQuotes"),	NULL, NULL, true, NULL },
    
    // Sales | Lookup | Sales Order Lookup
    { "menu",	tr("&Sales Order"),           (char*)lookupSoMenu,	lookupMenu,	true,	NULL, NULL, true, NULL },
    { "so.dspSalesOrderLookupByCustomerType", tr("by Customer &Type..."),	SLOT(sDspOrderLookupByCustomerType()), lookupSoMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true, NULL },
    { "so.dspSalesOrderLookupByCustomer", tr("by &Customer..."),	SLOT(sDspOrderLookupByCustomer()), lookupSoMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true, NULL },
    { "so.dspSalesOrderLookupByCustomerPO", tr("by Customer &PO..."),	SLOT(sDspOrderLookupByCustomerPO()), lookupSoMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true, NULL },
    { "so.dspSalesOrderLookupByItem", tr("by &Item..."),	SLOT(sDspOrderLookupByItem()), lookupSoMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true, NULL },
    
    { "separator",	NULL,	NULL,	lookupMenu,	true,		NULL, NULL, true, NULL }, 
    { "so.dspSalesOrderStatus", tr("Sales Order S&tatus..."),	SLOT(sDspSalesOrderStatus()), lookupMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true, NULL },   
  
    // Sales | Forms
    { "menu",	tr("&Forms"),           (char*)formsMenu,	mainMenu,	true,	NULL, NULL, true, NULL },
    { "so.printSalesOrderForm", tr("Print Sales &Order Form..."),	SLOT(sPrintSalesOrderForm()), formsMenu, (_privleges->check("MaintainSalesOrders") || _privleges->check("ViewSalesOrders")),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	formsMenu,	true,		NULL, NULL, _metrics->boolean("EnableBatchManager"), NULL }, 
    { "so.deliverSalesOrder", tr("&Schedule S/O for Email Delivery..."),	SLOT(sDeliverSalesOrder()), formsMenu, _privleges->check("ViewSalesOrders") ,	NULL, NULL, _metrics->boolean("EnableBatchManager"), NULL },
    { "separator",	NULL,	NULL,	formsMenu,	true,		NULL, NULL, true , NULL }, 
    { "so.packingListBatch", tr("Packing &List Batch..."),	SLOT(sPackingListBatch()), formsMenu, (_privleges->check("MaintainPackingListBatch") || _privleges->check("ViewPackingListBatch")),	NULL, NULL, true, NULL },
    { "so.printPackingList", tr("&Print Packing List..."),	SLOT(sPrintPackingList()), formsMenu, _privleges->check("PrintPackingLists"),	NULL, NULL, true, NULL },


    // Sales | Reports
    { "menu",	tr("&Reports"),           (char*)reportsMenu,	mainMenu,	true,	NULL, NULL, true, NULL },
    { "so.dspSummarizedBacklogByWarehouse", tr("Su&mmarized Backlog..."),	SLOT(sDspSummarizedBacklogByWarehouse()), reportsMenu, _privleges->check("ViewSalesOrders"),	new QPixmap(":/images/dspSummarizedBacklogByWhse.png"), toolBar,  true, NULL },

    // Sales | Reports | Backlog
    { "menu",	tr("&Backlog"),           (char*)reportsBacklogMenu,	reportsMenu,	true,	NULL, NULL, true, NULL },
    { "so.dspBacklogByCustomerType", tr("by Customer &Type..."),	SLOT(sDspBacklogByCustomerType()), reportsBacklogMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true, NULL },
    { "so.dspBacklogByCustomerGroup", tr("by Customer &Group..."),	SLOT(sDspBacklogByCustomerGroup()), reportsBacklogMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true, NULL },
    { "so.dspBacklogByCustomer", tr("by &Customer..."),	SLOT(sDspBacklogByCustomer()), reportsBacklogMenu, _privleges->check("ViewSalesOrders"), NULL, NULL,	 true, NULL },
    { "separator",	NULL,	NULL,	reportsBacklogMenu,	true,		NULL, NULL, true, NULL },
    { "so.dspBacklogBySalesOrder", tr("by Sales &Order..."),	SLOT(sDspBacklogBySalesOrder()), reportsBacklogMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	reportsBacklogMenu,	true,		NULL, NULL, true, NULL },
    { "so.dspBacklogByProductCategory", tr("by &Product Category..."),	SLOT(sDspBacklogByProductCategory()), reportsBacklogMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true, NULL },
    { "so.dspBacklogByItem", tr("by &Item..."),	SLOT(sDspBacklogByItem()), reportsBacklogMenu, _privleges->check("ViewSalesOrders"), NULL, NULL,	 true, NULL },
    
    { "so.dspPartiallyShippedOrders", tr("&Partially Shipped Orders..."),	SLOT(sDspPartiallyShippedOrders()), reportsMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	reportsMenu,	true,		NULL, NULL, true, NULL },   
    
    // Sales | Reports | Inventory Availability
    { "menu",	tr("&Inventory Availability"),           (char*)reportsInvAvailMenu,	reportsMenu,	true,	NULL, NULL, true, NULL },
    { "so.dspInventoryAvailabilityBySalesOrder", tr("by Sales &Order..."),	SLOT(sDspInventoryAvailabilityBySalesOrder()), reportsInvAvailMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true, NULL },
    { "so.dspInventoryAvailabilityByItem", tr("by &Item..."),	SLOT(sDspInventoryAvailabilityByItem()), reportsInvAvailMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true, NULL },    

    { "separator",	NULL,	NULL,	reportsMenu,	true,		NULL, NULL, true, NULL },
    { "so.dspEarnedCommissions", tr("&Earned Commissions..."),	SLOT(sDspEarnedCommissions()), reportsMenu, _privleges->check("ViewCommissions"),	NULL, NULL, true, NULL },
    { "so.dspBriefEarnedCommissions", tr("B&rief Earned Commissions..."),	SLOT(sDspBriefEarnedCommissions()), reportsMenu, _privleges->check("ViewCommissions"),	NULL, NULL, true, NULL },
    { "so.dspSummarizedTaxableSales", tr("Summarized &Taxable Sales..."),	SLOT(sDspSummarizedTaxableSales()), reportsMenu, _privleges->check("ViewCommissions"),	NULL, NULL, true, NULL },

    { "separator",	NULL,	NULL,	reportsMenu,	true,		NULL, NULL, true, NULL },
    
    // Sales | Reports | Customers
    { "menu",	tr("&Customers"),           (char*)reportsCustomersMenu,	reportsMenu,	true,	NULL, NULL, true, NULL },
    { "so.dspCustomersByCustomerType", tr("by Customer &Type..."),	SLOT(sDspCustomersByCusttype()), reportsCustomersMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")),	NULL, NULL, true, NULL },
    { "so.dspCustomersByCharacteristic", tr("by C&haracteristic..."),	SLOT(sDspCustomersByCharacteristic()), reportsCustomersMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")),	NULL, NULL, true, NULL },

    // Sales | Analysis
    { "menu",	tr("&Analysis"),           (char*)analysisMenu,	mainMenu,	true,	NULL, NULL, true, NULL },

    // Sales | Analysis | Bookings
    { "menu",	tr("&Bookings"),           (char*)analysisBookMenu,	analysisMenu,	true,	NULL, NULL, true, NULL },
    { "sa.dspBookingsBySalesRep", tr("by &Sales Rep..."), SLOT(sDspBookingsBySalesRep()), analysisBookMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisBookMenu,	true,		NULL, NULL, true, NULL },
    { "sa.dspBookingsByCustomerGroup", tr("by Customer &Group..."), SLOT(sDspBookingsByCustomerGroup()), analysisBookMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "sa.dspBookingsByCustomer", tr("by &Customer..."), SLOT(sDspBookingsByCustomer()), analysisBookMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "sa.dspBookingsByShipTo", tr("by S&hip-To..."), SLOT(sDspBookingsByShipTo()), analysisBookMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisBookMenu,	true,		NULL, NULL, true, NULL },
    { "sa.dspBookingsByProductCategory", tr("by &Product Category..."), SLOT(sDspBookingsByProductCategory()), analysisBookMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "sa.dspBookingsByItem", tr("by &Item..."), SLOT(sDspBookingsByItem()), analysisBookMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },

    // Sales | Analysis | Time Phased Bookings
    { "menu",	tr("Time-Phased B&ookings"),           (char*)analysisTpBookMenu,	analysisMenu,	true,	NULL, NULL, true, NULL },
    { "sa.dspTimePhasedBookingsByCustomer", tr("by &Customer..."), SLOT(sDspTimePhasedBookingsByCustomer()), analysisTpBookMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisTpBookMenu,	true,		NULL, NULL, true, NULL },
    { "sa.dspTimePhasedBookingsByProductCategory", tr("by &Product Category..."), SLOT(sDspTimePhasedBookingsByProductCategory()), analysisTpBookMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "sa.dspTimePhasedBookingsByItem", tr("by &Item..."), SLOT(sDspTimePhasedBookingsByItem()), analysisTpBookMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    
    { "separator",	NULL,	NULL,	analysisMenu,	true,		NULL, NULL, true, NULL },
    
    // Sales | Analysis | Sales History
    { "menu",	tr("Sales &History"),           (char*)analysisHistMenu,	analysisMenu,	true,	NULL, NULL, true, NULL },
    { "sa.dspSalesHistoryBySalesRep", tr("by &Sales Rep..."), SLOT(sDspSalesHistoryBySalesRep()), analysisHistMenu, _privleges->check("ViewSalesHistory"), NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisHistMenu,	true,		NULL, NULL, true, NULL },
    { "sa.dspSalesHistoryByCustomerType", tr("by Customer &Type..."), SLOT(sDspSalesHistoryByCustomerType()), analysisHistMenu, _privleges->check("ViewSalesHistory"), NULL, NULL, true , NULL },
    { "sa.dspSalesHistoryByCustomerGroup", tr("by Customer &Group..."), SLOT(sDspSalesHistoryByCustomerGroup()), analysisHistMenu, _privleges->check("ViewSalesHistory"), NULL, NULL, true , NULL },
    { "sa.dspSalesHistoryByCustomer", tr("by &Customer..."), SLOT(sDspSalesHistoryByCustomer()), analysisHistMenu, _privleges->check("ViewSalesHistory"), NULL, NULL, true , NULL },
    { "sa.dspSalesHistoryByBillToName", tr("by &Bill-To Name..."), SLOT(sDspSalesHistoryByBilltoName()), analysisHistMenu, _privleges->check("ViewSalesHistory"), NULL, NULL, true , NULL },
    { "sa.dspSalesHistoryByShipTo", tr("by S&hip-To..."), SLOT(sDspSalesHistoryByShipTo()), analysisHistMenu, _privleges->check("ViewSalesHistory"), NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisHistMenu,	true,		NULL, NULL, true, NULL },
    { "sa.dspSalesHistoryByProductCategory", tr("by &Product Category..."), SLOT(sDspSalesHistoryByProductCategory()), analysisHistMenu, _privleges->check("ViewSalesHistory"), NULL, NULL, true , NULL },
    { "sa.dspSalesHistoryByItem", tr("by &Item..."), SLOT(sDspSalesHistoryByItem()), analysisHistMenu, _privleges->check("ViewSalesHistory"), NULL, NULL, true , NULL },

    // Sales | Analysis | Brief Sales History
    { "menu",	tr("Brie&f Sales History"),           (char*)analysisBrfHistMenu,	analysisMenu,	true,	NULL, NULL, true, NULL },
    { "sa.dspBriefSalesHistoryBySalesRep", tr("by &Sales Rep..."), SLOT(sDspBriefSalesHistoryBySalesRep()), analysisBrfHistMenu, _privleges->check("ViewSalesHistory"), NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisBrfHistMenu,	true,		NULL, NULL, true, NULL },
    { "sa.dspBriefSalesHistoryByCustomerType", tr("by Customer &Type..."), SLOT(sDspBriefSalesHistoryByCustomerType()), analysisBrfHistMenu, _privleges->check("ViewSalesHistory"), NULL, NULL, true , NULL },
    { "sa.dspBriefSalesHistoryByCustomer", tr("by &Customer..."), SLOT(sDspBriefSalesHistoryByCustomer()), analysisBrfHistMenu, _privleges->check("ViewSalesHistory"), NULL, NULL, true , NULL },
    
    // Sales | Analysis | Summarized Sales History
    { "menu",	tr("Summari&zed Sales History"),           (char*)analysisSumHistMenu,	analysisMenu,	true,	NULL, NULL, true, NULL },
    { "sa.dspSummarizedSalesHistoryBySalesRep", tr("by &Sales Rep..."), SLOT(sDspSummarizedSalesBySalesRep()), analysisSumHistMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "sa.dspSummarizedSalesHistoryByShippingZoneByItem", tr("by Shipping &Zone by Item..."), SLOT(sDspSummarizedSalesHistoryByShippingZone()), analysisSumHistMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisSumHistMenu,	true,		NULL, NULL, true, NULL },
    { "sa.dspSummarizedSalesHistoryByCustomerType", tr("by Customer &Type..."), SLOT(sDspSummarizedSalesByCustomerType()), analysisSumHistMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "sa.dspSummarizedSalesHistoryByCustomer", tr("by &Customer..."), SLOT(sDspSummarizedSalesByCustomer()), analysisSumHistMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "sa.dspSummarizedSalesHistoryByCustomerTypeByItem", tr("by Customer T&ype by Item..."), SLOT(sDspSummarizedSalesByCustomerTypeByItem()), analysisSumHistMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "sa.dspSummarizedSalesHistoryByCustomerByItem", tr("by C&ustomer by Item..."), SLOT(sDspSummarizedSalesByCustomerByItem()), analysisSumHistMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisSumHistMenu,	true,		NULL, NULL, true, NULL },
    { "sa.dspSummarizedSalesHistoryByItem", tr("by &Item..."), SLOT(sDspSummarizedSalesByItem()), analysisSumHistMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true , NULL },

    // Sales | Analysis | Time-Phased Sales History
    { "menu",	tr("Time &Phased Sales History"),           (char*)analysisTpHistMenu,	analysisMenu,	true,	NULL, NULL, true, NULL },
    { "sa.dspTimePhasedSalesHistoryByCustomerGroup", tr("by Customer &Group..."), SLOT(sDspTimePhasedSalesByCustomerGroup()), analysisTpHistMenu, (_privleges->check("ViewSalesHistory") && _privleges->check("ViewCustomerPrices")), NULL, NULL, true , NULL },
    { "sa.dspTimePhasedSalesHistoryByCustomer", tr("by &Customer..."), SLOT(sDspTimePhasedSalesByCustomer()), analysisTpHistMenu, (_privleges->check("ViewSalesHistory") && _privleges->check("ViewCustomerPrices")), NULL, NULL, true , NULL },
    { "sa.dspTimePhasedSalesHistoryByCustomerByItem", tr("by C&ustomer by Item..."), SLOT(sDspTimePhasedSalesByCustomerByItem()), analysisTpHistMenu, (_privleges->check("ViewSalesHistory") && _privleges->check("ViewCustomerPrices")), NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisTpHistMenu,	true,		NULL, NULL, true, NULL },
    { "sa.dspTimePhasedSalesHistoryByProductCategory", tr("by &Product Category..."), SLOT(sDspTimePhasedSalesByProductCategory()), analysisTpHistMenu, (_privleges->check("ViewSalesHistory") && _privleges->check("ViewCustomerPrices")), NULL, NULL, true , NULL },
    { "sa.dspTimePhasedSalesHistoryByItem", tr("by &Item..."), SLOT(sDspTimePhasedSalesByItem()), analysisTpHistMenu, (_privleges->check("ViewSalesHistory") && _privleges->check("ViewCustomerPrices")), NULL, NULL, true , NULL },

    { "separator",	NULL,	NULL,	mainMenu,	true,		NULL, NULL, true, NULL },

    // Sales | Prospect
    { "menu",	tr("&Prospect"),       (char*)prospectMenu,	mainMenu,	true,	NULL, NULL, true, NULL },
    { "so.enterNewProspect", tr("&New..."),	SLOT(sNewProspect()), prospectMenu, _privleges->check("MaintainProspectMasters"),	NULL, NULL, true, NULL },
    { "so.prospects", tr("&List..."),	SLOT(sProspects()), prospectMenu, (_privleges->check("MaintainProspectMasters") || _privleges->check("ViewProspectMasters")),	NULL, NULL, true, NULL },
    { "so.searchForProspect", tr("&Search..."),	SLOT(sSearchForProspect()), prospectMenu, (_privleges->check("MaintainProspects") || _privleges->check("ViewProspects")),	NULL, NULL, true, NULL },
   
    // Sales | Customer
    { "menu",	tr("&Customer"),       (char*)customerMenu,	mainMenu,	true,	NULL, NULL, true, NULL },
    { "so.enterNewCustomer", tr("&New..."),	SLOT(sNewCustomer()), customerMenu, _privleges->check("MaintainCustomerMasters"),	NULL, NULL, true, NULL },
    { "so.customers", tr("&List..."),	SLOT(sCustomers()), customerMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")),	NULL, NULL, true, NULL },
    { "so.searchForCustomer", tr("&Search..."),	SLOT(sSearchForCustomer()), customerMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	customerMenu,	true,		NULL, NULL, true, NULL },
    { "so.dspCustomerInformation",   tr("&Workbench..."),	SLOT(sDspCustomerInformation()), customerMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")), new QPixmap(":/images/customerInformationWorkbench.png"), toolBar,  true, "Customer Information Workbench" },
    { "separator",	NULL,	NULL,	customerMenu,	true,		NULL, NULL, true, NULL },
    { "so.customerTypes", tr("&Types..."),	SLOT(sCustomerTypes()), customerMenu, (_privleges->check("MaintainCustomerTypes") || _privleges->check("ViewCustomerTypes")),	NULL, NULL, true, NULL },
    { "so.customerGroups", tr("&Groups..."),	SLOT(sCustomerGroups()), customerMenu, (_privleges->check("MaintainCustomerGroups") || _privleges->check("ViewCustomerGroups")),	NULL, NULL, true, NULL },
   
    // Sales | Item Pricing
    { "menu",	tr("&Item Pricing"),       (char*)pricingMenu,	mainMenu,	true,	NULL, NULL, true, NULL },
    { "so.itemListPrice", tr("Item &List Price..."),	SLOT(sItemListPrice()), pricingMenu, (_privleges->check("MaintainListPrices") || _privleges->check("ViewListPrices")),	NULL, NULL, true, NULL },
    { "so.updateListPricesByProductCategory", tr("&Update List Prices..."),	SLOT(sUpdateListPricesByProductCategory()), pricingMenu, _privleges->check("MaintainListPrices"),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	pricingMenu,	true,		NULL, NULL, true, NULL },
    { "so.pricingSchedules", tr("Pricing &Schedules..."),	SLOT(sPricingSchedules()), pricingMenu, (_privleges->check("MaintainListPrices") || _privleges->check("ViewListPrices")),	NULL, NULL, true, NULL },
    { "so.pricingScheduleAssignments", tr("Pricing Schedule Assi&gnments..."),	SLOT(sPricingScheduleAssignments()), pricingMenu, _privleges->check("AssignPricingSchedules"),	NULL, NULL, true, NULL },
    { "so.sales", tr("S&ales..."),	SLOT(sSales()), pricingMenu, _privleges->check("CreateSales"),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	pricingMenu,	true,		NULL, NULL, true, NULL },

    // Sales | Item Pricing | Update Prices
    { "menu",	tr("Update &Prices"),       (char*)pricingUpdateMenu,	pricingMenu,	true,	NULL, NULL, true, NULL },
    { "so.updatePricesByProductCategory", tr("by Product &Category..."),	SLOT(sUpdatePricesByProductCategory()), pricingUpdateMenu, _privleges->check("UpdatePricingSchedules"),	NULL, NULL, true, NULL },
    { "so.updatePricesByPricingSchedule", tr("by Pricing &Schedule..."),	SLOT(sUpdatePricesByPricingSchedule()), pricingUpdateMenu, _privleges->check("UpdatePricingSchedules"),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	pricingMenu,	true,		NULL, NULL, true, NULL },

    // Sales | Item Pricing | Reports
    { "menu",	tr("&Reports"),	(char*)pricingReportsMenu,	pricingMenu,	true,	NULL, NULL, true, NULL },
    { "so.dspPricesByCustomerType", tr("Prices by Customer &Type..."),	SLOT(sDspPricesByCustomerType()), pricingReportsMenu, _privleges->check("ViewCustomerPrices"), NULL, NULL,	 true, NULL },
    { "so.dspPricesByCustomer", tr("Prices by &Customer..."),	SLOT(sDspPricesByCustomer()), pricingReportsMenu, _privleges->check("ViewCustomerPrices"), NULL, NULL,	 true, NULL },
    { "so.dspPricesByItem", tr("Prices by &Item..."),	SLOT(sDspPricesByItem()), pricingReportsMenu, _privleges->check("ViewCustomerPrices"), NULL, NULL,	 true, NULL },
    
    { "separator",	NULL,	NULL,	mainMenu,	true,		NULL, NULL, true, NULL },

    { "menu",	tr("&Master Information"), (char*)masterInfoMenu,	mainMenu,	true,	NULL, NULL, true, NULL },
    { "so.characteristics", tr("C&haracteristics..."),	SLOT(sCharacteristics()), masterInfoMenu, (_privleges->check("MaintainCharacteristics") || _privleges->check("ViewCharacteristics")),	NULL, NULL, true, NULL },
    { "so.salesReps", tr("&Sales Reps..."),	SLOT(sSalesReps()), masterInfoMenu, (_privleges->check("MaintainSalesReps") || _privleges->check("ViewSalesReps")),	NULL, NULL, true, NULL },
    { "so.shippingZones", tr("Shipping &Zones..."),	SLOT(sShippingZones()), masterInfoMenu, (_privleges->check("MaintainShippingZones") || _privleges->check("ViewShippingZones")),	NULL, NULL, true, NULL },
    { "so.shipVias", tr("Ship &Vias..."),	SLOT(sShipVias()), masterInfoMenu, (_privleges->check("MaintainShipVias") || _privleges->check("ViewShipVias")),	NULL, NULL, true, NULL },
    { "so.shippingChargeTypes", tr("Shipping &Charge Types..."),	SLOT(sShippingChargeTypes()), masterInfoMenu, (_privleges->check("MaintainShippingChargeTypes") || _privleges->check("ViewShippingChargeTypes")),	NULL, NULL, true, NULL },
    { "so.taxCodes", tr("Ta&x Codes..."),	SLOT(sTaxCodes()), masterInfoMenu, (_privleges->check("MaintainTaxCodes") || _privleges->check("ViewTaxCodes")),	NULL, NULL, true, NULL },
    { "so.shippingForms", tr("Shipping &Forms..."),	SLOT(sShippingForms()), masterInfoMenu, (_privleges->check("MaintainShippingForms") || _privleges->check("ViewShippingForms")),	NULL, NULL, true, NULL },
    { "so.salesCategories", tr("Sales Cate&gories..."),	SLOT(sSalesCategories()), masterInfoMenu, (_privleges->check("MaintainSalesCategories")) || (_privleges->check("ViewSalesCategories")),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	masterInfoMenu,	true,		NULL, NULL, true, NULL },
    { "so.terms", tr("&Terms..."),	SLOT(sTerms()), masterInfoMenu, (_privleges->check("MaintainTerms") || _privleges->check("ViewTerms")),	NULL, NULL, true, NULL },
    { "so.salesAccountAssignments", tr("Sa&les Account Assignments..."),	SLOT(sSalesAccountAssignments()), masterInfoMenu, (_privleges->check("MaintainSalesAccount") || _privleges->check("ViewSalesAccount")),	NULL, NULL, true, NULL },
    { "so.arAccountAssignments", tr("&A/R Account Assignments..."),	SLOT(sARAccountAssignments()), masterInfoMenu, (_privleges->check("MaintainSalesAccount") || _privleges->check("ViewSalesAccount")),	NULL, NULL, true, NULL },
    { "so.customerFormAssignments", tr("C&ustomer Form Assignments..."),	SLOT(sCustomerFormAssignments()), masterInfoMenu, _privleges->check("MaintainCustomerMasters"),	NULL, NULL, true, NULL },

    { "menu",	tr("&Utilities"),         (char*)utilitiesMenu,	mainMenu,	true,	NULL, NULL, true, NULL },
    { "so.customerInformationExport", tr("&Customer Information Export..."),	SLOT(sDspCustomerInformationExport()), utilitiesMenu, _privleges->check("MaintainCustomerMasters"),	NULL, NULL, true, NULL },
    { "so.reassignCustomerTypeByCustomerType", tr("&Reassign Customer Type by Customer Type..."),	SLOT(sReassignCustomerTypeByCustomerType()), utilitiesMenu, _privleges->check("MaintainCustomerMasters"),	NULL, NULL, true, NULL },
    { "so.updateCreditStatusByCustomer", tr("&Update Credit Status by Customer..."),	SLOT(sUpdateCreditStatusByCustomer()), utilitiesMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("UpdateCustomerCreditStatus")),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	utilitiesMenu,	true,		NULL, NULL, true, NULL },
    { "so.purgeInvoices",		     tr("Purge &Invoices..."),		SLOT(sPurgeInvoices()), utilitiesMenu, _privleges->check("PurgeInvoices"),	NULL, NULL, true, NULL },
    { "so.purgeCreditMemos",		     tr("Purge Credit &Memos..."),	SLOT(sPurgeCreditMemos()), utilitiesMenu, _privleges->check("PurgeCreditMemos"),	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	utilitiesMenu,	true,		NULL, NULL, true, NULL },
    { "sa.archieveSalesHistory", tr("&Archive Sales History..."), SLOT(sArchiveSalesHistory()), utilitiesMenu, _privleges->check("ArchiveSalesHistory"), NULL, NULL, true , NULL },
    { "sa.restoreSalesHistory", tr("Restore &Sales History..."), SLOT(sRestoreSalesHistory()), utilitiesMenu, _privleges->check("RestoreSalesHistory"), NULL, NULL, true , NULL },

// START_RW
    { "so.exportCustomers", tr("Export Customers..."), SLOT(sExportCustomers()), utilitiesMenu, _privleges->check("ViewCustomerMasters"), NULL, NULL, _metrics->boolean("EnableExternalAccountingInterface"), NULL }
// END_RW
  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));

  parent->populateCustomMenu(mainMenu, "Sales");
  parent->menuBar()->insertItem(tr("&Sales"), mainMenu);
}

void menuSales::addActionsToMenu(actionProperties acts[], unsigned int numElems)
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

//  Orders
void menuSales::sNewSalesOrder()
{
  salesOrder::newSalesOrder(-1);
}

void menuSales::sOpenSalesOrders()
{
  omfgThis->handleNewWindow(new openSalesOrders());
}

void menuSales::sRescheduleSoLineItems()
{
  rescheduleSoLineItems(parent, "", TRUE).exec();
}

void menuSales::sPackingListBatch()
{
  omfgThis->handleNewWindow(new packingListBatch());
}

void menuSales::sPrintPackingList()
{
  printPackingList(parent, "", TRUE).exec();
}

void menuSales::sNewQuote()
{
  ParameterList params;
  params.append("mode", "newQuote");

  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sQuotes()
{
  omfgThis->handleNewWindow(new quotes());
}


//  Billing
void menuSales::sUninvoicedShipments()
{
  omfgThis->handleNewWindow(new uninvoicedShipments());
}

void menuSales::sSelectShippedOrdersForBilling()
{
  selectShippedOrders(parent, "", TRUE).exec();
}

void menuSales::sSelectOrderForBilling()
{
  ParameterList params;
  params.append("mode", "new");

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sBillingEditList()
{
  omfgThis->handleNewWindow(new billingEditList());
}

void menuSales::sDspBillingSelections()
{
  omfgThis->handleNewWindow(new dspBillingSelections());
}

void menuSales::sPostBillingSelections()
{
  postBillingSelections(parent, "", TRUE).exec();
}

void menuSales::sUnpostedInvoices()
{
  omfgThis->handleNewWindow(new unpostedInvoices());
}

void menuSales::sPrintInvoices()
{
  printInvoices(parent, "", TRUE).exec();
}

void menuSales::sPrintInvoicesByShipvia()
{
  printInvoicesByShipvia(parent, "", TRUE).exec();
}

void menuSales::sReprintInvoices()
{
  reprintInvoices(parent, "", TRUE).exec();
}

void menuSales::sDeliverInvoice()
{
  deliverInvoice(parent, "", TRUE).exec();
}

void menuSales::sDeliverSalesOrder()
{
  deliverSalesOrder(parent, "", TRUE).exec();
}

void menuSales::sPostInvoices()
{
  postInvoices(parent, "", TRUE).exec();
}

void menuSales::sPurgeInvoices()
{
  purgeInvoices(parent, "", TRUE).exec();
}


void menuSales::sNewCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sUnpostedCreditMemos()
{
  omfgThis->handleNewWindow(new unpostedCreditMemos());
}

void menuSales::sCreditMemoEditList()
{
  omfgThis->handleNewWindow(new creditMemoEditList());
}

void menuSales::sPrintCreditMemos()
{
  printCreditMemos(parent, "", TRUE).exec();
}

void menuSales::sReprintCreditMemos()
{
  reprintCreditMemos(parent, "", TRUE).exec();
}

void menuSales::sPostCreditMemos()
{
  postCreditMemos(parent, "", TRUE).exec();
}

void menuSales::sPurgeCreditMemos()
{
  purgeCreditMemos(parent, "", TRUE).exec();
}


//  S/O | Item Pricing
void menuSales::sItemListPrice()
{
  itemListPrice newdlg(parent, "", TRUE);
  newdlg.exec();
}

void menuSales::sUpdateListPricesByProductCategory()
{
  updateListPricesByProductCategory newdlg(parent, "", TRUE);
  newdlg.exec();
}

void menuSales::sPricingSchedules()
{
  omfgThis->handleNewWindow(new itemPricingSchedules());
}

void menuSales::sPricingScheduleAssignments()
{
  omfgThis->handleNewWindow(new pricingScheduleAssignments());
}

void menuSales::sSales()
{
  omfgThis->handleNewWindow(new sales());
}

void menuSales::sUpdatePricesByProductCategory()
{
  updatePricesByProductCategory(parent, "", TRUE).exec();
}

void menuSales::sUpdatePricesByPricingSchedule()
{
  updatePricesByPricingSchedule(parent, "", TRUE).exec();
}

void menuSales::sDspPricesByItem()
{
  omfgThis->handleNewWindow(new dspPricesByItem());
}

void menuSales::sDspPricesByCustomer()
{
  omfgThis->handleNewWindow(new dspPricesByCustomer());
}

void menuSales::sDspPricesByCustomerType()
{
  omfgThis->handleNewWindow(new dspPricesByCustomerType());
}

void menuSales::sDspCustomersByCusttype()
{
  omfgThis->handleNewWindow(new dspCustomersByCustomerType());
}

void menuSales::sDspCustomersByCharacteristic()
{
  omfgThis->handleNewWindow(new dspCustomersByCharacteristic());
}

void menuSales::sDspCustomerInformation()
{
  // see notes on Mantis bug 4024 for explanation of why this is a modal dialog
  omfgThis->handleNewWindow(new dspCustomerInformation());
}

void menuSales::sDspSalesOrderStatus()
{
  omfgThis->handleNewWindow(new dspSalesOrderStatus());
}

void menuSales::sDspInventoryAvailabilityByItem()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityByItem());
}

void menuSales::sDspInventoryAvailabilityBySalesOrder()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityBySalesOrder());
}

void menuSales::sDspOrderLookupByCustomer()
{
  omfgThis->handleNewWindow(new dspSalesOrdersByCustomer());
}

void menuSales::sDspOrderLookupByCustomerType()
{
  ParameterList params;
  params.append("custtype");

  dspSalesOrdersByParameterList *newdlg = new dspSalesOrdersByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspOrderLookupByItem()
{
  omfgThis->handleNewWindow(new dspSalesOrdersByItem());
}

void menuSales::sDspOrderLookupByCustomerPO()
{
  omfgThis->handleNewWindow(new dspSalesOrdersByCustomerPO());
}

void menuSales::sDspQuoteLookupByCustomer()
{
  omfgThis->handleNewWindow(new dspQuotesByCustomer());
}

void menuSales::sDspQuoteLookupByItem()
{
  omfgThis->handleNewWindow(new dspQuotesByItem());
}

void menuSales::sDspBacklogByItem()
{
  omfgThis->handleNewWindow(new dspBacklogByItem());
}

void menuSales::sDspBacklogBySalesOrder()
{
  omfgThis->handleNewWindow(new dspBacklogBySalesOrder());
}

void menuSales::sDspBacklogByCustomer()
{
  omfgThis->handleNewWindow(new dspBacklogByCustomer());
}

void menuSales::sDspBacklogByCustomerType()
{
  ParameterList params;
  params.append("custtype");

  dspBacklogByParameterList *newdlg = new dspBacklogByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspBacklogByCustomerGroup()
{
  ParameterList params;
  params.append("custgrp");

  dspBacklogByParameterList *newdlg = new dspBacklogByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspBacklogByProductCategory()
{
  ParameterList params;
  params.append("prodcat");

  dspBacklogByParameterList *newdlg = new dspBacklogByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspSummarizedBacklogByWarehouse()
{
  omfgThis->handleNewWindow(new dspSummarizedBacklogByWarehouse());
}

void menuSales::sDspPartiallyShippedOrders()
{
  omfgThis->handleNewWindow(new dspPartiallyShippedOrders());
}

void menuSales::sDspEarnedCommissions()
{
  omfgThis->handleNewWindow(new dspEarnedCommissions());
}

void menuSales::sDspBriefEarnedCommissions()
{
  omfgThis->handleNewWindow(new dspBriefEarnedCommissions());
}

void menuSales::sDspSummarizedTaxableSales()
{
  omfgThis->handleNewWindow(new dspSummarizedTaxableSales());
}


void menuSales::sDspSalesHistoryByCustomer()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByCustomer());
}

void menuSales::sDspSalesHistoryByBilltoName()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByBilltoName());
}

void menuSales::sDspSalesHistoryByShipTo()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByShipTo());
}

void menuSales::sDspSalesHistoryByItem()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByItem());
}

void menuSales::sDspSalesHistoryBySalesRep()
{
  omfgThis->handleNewWindow(new dspSalesHistoryBySalesrep());
}

void menuSales::sDspSalesHistoryByProductCategory()
{
  ParameterList params;
  params.append("prodcat");

  dspSalesHistoryByParameterList *newdlg = new dspSalesHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspSalesHistoryByCustomerType()
{
  ParameterList params;
  params.append("custtype");

  dspSalesHistoryByParameterList *newdlg = new dspSalesHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspSalesHistoryByCustomerGroup()
{
  ParameterList params;
  params.append("custgrp");

  dspSalesHistoryByParameterList *newdlg = new dspSalesHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspBriefSalesHistoryByCustomer()
{
  omfgThis->handleNewWindow(new dspBriefSalesHistoryByCustomer());
}

void menuSales::sDspBriefSalesHistoryByCustomerType()
{
  omfgThis->handleNewWindow(new dspBriefSalesHistoryByCustomerType());
}

void menuSales::sDspBriefSalesHistoryBySalesRep()
{
  omfgThis->handleNewWindow(new dspBriefSalesHistoryBySalesRep());
}

void menuSales::sDspBookingsByCustomer()
{
  omfgThis->handleNewWindow(new dspBookingsByCustomer());
}

void menuSales::sDspBookingsByCustomerGroup()
{
  omfgThis->handleNewWindow(new dspBookingsByCustomerGroup());
}

void menuSales::sDspBookingsByShipTo()
{
  omfgThis->handleNewWindow(new dspBookingsByShipTo());
}

void menuSales::sDspBookingsByItem()
{
  omfgThis->handleNewWindow(new dspBookingsByItem());
}

void menuSales::sDspBookingsByProductCategory()
{
  omfgThis->handleNewWindow(new dspBookingsByProductCategory());
}

void menuSales::sDspBookingsBySalesRep()
{
  omfgThis->handleNewWindow(new dspBookingsBySalesRep());
}

void menuSales::sDspSummarizedSalesByCustomer()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomer());
}

void menuSales::sDspSummarizedSalesByCustomerType()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomerType());
}

void menuSales::sDspSummarizedSalesByCustomerByItem()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomerByItem());
}

void menuSales::sDspSummarizedSalesByCustomerTypeByItem()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomerTypeByItem());
}

void menuSales::sDspSummarizedSalesByItem()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByItem());
}

void menuSales::sDspSummarizedSalesBySalesRep()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesBySalesRep());
}

void menuSales::sDspSummarizedSalesHistoryByShippingZone()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesHistoryByShippingZone());
}

void menuSales::sDspTimePhasedBookingsByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedBookingsByItem());
}

void menuSales::sDspTimePhasedBookingsByProductCategory()
{
  omfgThis->handleNewWindow(new dspTimePhasedBookingsByProductCategory());
}

void menuSales::sDspTimePhasedBookingsByCustomer()
{
  omfgThis->handleNewWindow(new dspTimePhasedBookingsByCustomer());
}

void menuSales::sDspTimePhasedSalesByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByItem());
}

void menuSales::sDspTimePhasedSalesByProductCategory()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByProductCategory());
}

void menuSales::sDspTimePhasedSalesByCustomer()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByCustomer());
}

void menuSales::sDspTimePhasedSalesByCustomerGroup()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByCustomerGroup());
}

void menuSales::sDspTimePhasedSalesByCustomerByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByCustomerByItem());
}

void menuSales::sPrintSalesOrderForm()
{
  printSoForm(parent, "", TRUE).exec();
}


//  Master Information
void menuSales::sNewCustomer()
{
  ParameterList params;
  params.append("mode", "new");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sSearchForCustomer()
{
  ParameterList params;
  params.append("crmaccnt_subtype", "cust");

  searchForCRMAccount *newdlg = new searchForCRMAccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sCustomers()
{
  omfgThis->handleNewWindow(new customers());
}

void menuSales::sUpdateCreditStatusByCustomer()
{
  updateCreditStatusByCustomer(parent, "", TRUE).exec();
}

void menuSales::sCustomerGroups()
{
  omfgThis->handleNewWindow(new customerGroups());
}

void menuSales::sCustomerTypes()
{
  omfgThis->handleNewWindow(new customerTypes());
}

void menuSales::sNewProspect()
{
  ParameterList params;
  params.append("mode", "new");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sSearchForProspect()
{
  ParameterList params;
  params.append("crmaccnt_subtype", "prospect");

  searchForCRMAccount *newdlg = new searchForCRMAccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sProspects()
{
  omfgThis->handleNewWindow(new prospects());
}

void menuSales::sSalesReps()
{
  omfgThis->handleNewWindow(new salesReps());
}

void menuSales::sShippingZones()
{
  omfgThis->handleNewWindow(new shippingZones());
}

void menuSales::sShipVias()
{
  omfgThis->handleNewWindow(new shipVias());
}

void menuSales::sShippingChargeTypes()
{
  omfgThis->handleNewWindow(new shippingChargeTypes());
}

void menuSales::sTaxCodes()
{
  omfgThis->handleNewWindow(new taxCodes());
}

void menuSales::sTerms()
{
  omfgThis->handleNewWindow(new termses());
}

void menuSales::sShippingForms()
{
  omfgThis->handleNewWindow(new shippingForms());
}

void menuSales::sSalesCategories()
{
  omfgThis->handleNewWindow(new salesCategories());
}

void menuSales::sSalesAccountAssignments()
{
  omfgThis->handleNewWindow(new salesAccounts());
}

void menuSales::sARAccountAssignments()
{
  omfgThis->handleNewWindow(new arAccountAssignments());
}

void menuSales::sCustomerFormAssignments()
{
  omfgThis->handleNewWindow(new customerFormAssignments());
}

void menuSales::sDspCustomerInformationExport()
{
  omfgThis->handleNewWindow(new dspCustomerInformationExport());
}

void menuSales::sReassignCustomerTypeByCustomerType()
{
  reassignCustomerTypeByCustomerType(parent, "", TRUE).exec();
}

// START_RW
void menuSales::sPostAROpenAndDist()
{
  if ( (_metrics->value("AccountingSystem") == "RW2000") ||
       (_metrics->value("AccountingSystem") == "RealWorld91") )
  {
    if (QMessageBox::critical( omfgThis, tr("Create New AROPEN and ARDIST Files?"),
                               tr( "Creating new Export Files will delete the previous Export Files.\n"
                                   "You should make sure that the previous Export Files have been\n"
                                   "imported into RealWorld before Proceeding.\n\n"
                                   "Are you sure that you want to Create New Export Files?" ),
                                   tr("&Yes"), tr("&No"), QString::null, 0, 1  ) != 0)
      return;

    rwInterface::exportArdist(omfgThis);
    rwInterface::exportAropen(omfgThis);
  }

  if ( (_metrics->value("AccountingSystem") == "RW2000") ||
       (_metrics->value("AccountingSystem") == "RealWorld91") )
  {
    if ( QMessageBox::information( omfgThis, tr("Mark Distributions as Posted"),
                                   tr( "New ARDIST and AROPEN files have been generated in\n"
                                       "the RealWorld directory.  You should now use the RealWorld\n"
                                       "arfu/arutil tool to import these files.  After you have\n"
                                       "successfully imported the ARDIST and AROPEN files click\n"
                                       "the 'Post' button to mark these items as distributed.\n"
                                       "If, for any reason, you were unable to post the ARDIST\n"
                                       "and AROPEN files click on the 'Do Not Post' button and\n"
                                       "Re-Post Invoices to re-create the ARDIST and AROPEN files.\n" ),
                                   tr("&Post"), tr("Do &Not Post"), QString::null, 0, 1) == 0)
      XSqlQuery().exec( "SELECT postSoGLTransactions();"
                        "SELECT postAropenItems();" );
  }
}

void menuSales::sExportCustomers()
{
  omfgThis->handleNewWindow(new exportCustomers());
}
// END_RW

void menuSales::sCharacteristics()
{
  omfgThis->handleNewWindow(new characteristics());
}

void menuSales::sArchiveSalesHistory()
{
  ParameterList params;
  params.append("archieve");

  archRestoreSalesHistory newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuSales::sRestoreSalesHistory()
{
  ParameterList params;
  params.append("restore");

  archRestoreSalesHistory newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}
