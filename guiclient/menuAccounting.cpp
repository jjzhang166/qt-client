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

//  menuAccounting.cpp
//  Created 08/22/2000 JSL
//  Copyright (c) 2000-2008, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"

//AP
#include "purchaseOrder.h"
#include "unpostedPurchaseOrders.h"
#include "printPurchaseOrder.h"
#include "postPurchaseOrder.h"

#include "dspUninvoicedReceivings.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "openVouchers.h"
#include "voucheringEditList.h"
#include "postVouchers.h"

#include "selectPayments.h"
#include "selectedPayments.h"
#include "prepareCheckRun.h"
#include "viewCheckRun.h"
#include "miscCheck.h"
#include "printCheck.h"
#include "printChecks.h"
#include "voidChecks.h"
#include "postCheck.h"
#include "postChecks.h"

#include "unappliedAPCreditMemos.h"
#include "apOpenItem.h"

#include "dspVendorAPHistory.h"
#include "dspCheckRegister.h"
#include "dspVoucherRegister.h"
#include "dspAPOpenItemsByVendor.h"
#include "dspTimePhasedOpenAPItems.h"

// AR
#include "invoice.h"
#include "unpostedInvoices.h"
#include "printInvoices.h"
#include "reprintInvoices.h"
#include "deliverInvoice.h"
#include "postInvoices.h"
#include "purgeInvoices.h"

#include "cashReceipt.h"
#include "cashReceiptsEditList.h"
#include "postCashReceipts.h"

#include "unappliedARCreditMemos.h"
#include "arOpenItem.h"

#include "arWorkBench.h"

#include "dspCustomerARHistory.h"
#include "dspCashReceipts.h"
#include "dspARApplications.h"
#include "dspInvoiceInformation.h"
#include "dspAROpenItemsByCustomer.h"
#include "dspAROpenItems.h"
#include "dspTimePhasedOpenARItems.h"
#include "dspInvoiceRegister.h"
#include "dspDepositsRegister.h"
#include "printJournal.h"
#include "printStatementByCustomer.h"
#include "printStatementsByCustomerType.h"

// GL
#include "glTransaction.h"
#include "glSeries.h"
#include "unpostedGlSeries.h"

#include "standardJournal.h"
#include "standardJournals.h"
#include "standardJournalGroups.h"
#include "postStandardJournal.h"
#include "postStandardJournalGroup.h"
#include "dspStandardJournalHistory.h"

#include "financialLayouts.h"
#include "financialLayout.h"
#include "dspFinancialReport.h"

#include "dspGLTransactions.h"
#include "dspSummarizedGLTransactions.h"
#include "dspGLSeries.h"
#include "dspTrialBalances.h"

#include "companies.h"
#include "profitCenters.h"
#include "subaccounts.h"
#include "accountNumbers.h"
#include "subAccntTypes.h"
#include "accountingPeriods.h"
#include "accountingYearPeriods.h"
#include "taxCodes.h"
#include "taxTypes.h"
#include "taxAuthorities.h"
#include "searchForCRMAccount.h"
#include "taxSelections.h"
#include "taxRegistrations.h"

#include "reconcileBankaccount.h"
#include "bankAdjustment.h"
#include "bankAdjustmentEditList.h"
#include "bankAdjustmentTypes.h"
#include "dspBankrecHistory.h"
#include "dspSummarizedBankrecHistory.h"

#include "budgets.h"
#include "maintainBudget.h"
#include "forwardUpdateAccounts.h"
#include "duplicateAccountNumbers.h"
#include "vendors.h"
#include "termses.h"
#include "bankAccounts.h"
#include "checkFormats.h"
#include "apAccountAssignments.h"
#include "costCategories.h"
#include "expenseCategories.h"

#include "customers.h"
#include "customerType.h"
#include "customerTypes.h"
#include "vendorTypes.h"
#include "salesCategories.h"
#include "reasonCodes.h"
#include "arAccountAssignments.h"

#include "updateLateCustCreditStatus.h"
#include "createRecurringInvoices.h"

// START_RW
#include "postGLTransactionsToExternal.h"
#include "dspRWTransactions.h"
// END_RW

#include "menuAccounting.h"

menuAccounting::menuAccounting(GUIClient *Pparent) :
 QObject(Pparent, "wmModule")
{
  parent = Pparent;

  toolBar = new QToolBar(tr("Accounting Tools"));
  toolBar->setObjectName("Accounting Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowGLToolbar"))
    parent->addToolBar(toolBar);


  mainMenu		= new QMenu(parent);
  apMenu = new QMenu(parent);
  apPurchaseMenu = new QMenu(parent);
  apVoucherMenu = new QMenu(parent);
  apMemosMenu = new QMenu(parent);
  apPaymentsMenu = new QMenu(parent);
  apFormsMenu = new QMenu(parent);
  apReportsMenu = new QMenu(parent);
  arMenu = new QMenu(parent);
  arInvoicesMenu = new QMenu(parent);
  arMemosMenu = new QMenu(parent);
  arCashReceiptsMenu = new QMenu(parent);
  arFormsMenu = new QMenu(parent);
  arReportsMenu = new QMenu(parent);
  glMenu = new QMenu(parent);
  glEnterTransactionMenu = new QMenu(parent);
  glStandardJournalsMenu = new QMenu(parent);
  bankrecMenu		= new QMenu(parent);
  financialReportsMenu	= new QMenu(parent);
  reportsMenu		= new QMenu(parent);
  calendarMenu      = new QMenu(parent);
  coaMenu           = new QMenu(parent);
  budgetMenu		= new QMenu(parent);
  taxMenu           = new QMenu(parent);
  masterInfoMenu	= new QMenu(parent);
  utilitiesMenu		= new QMenu(parent);

  mainMenu->setObjectName("menu.accnt");
  apMenu->setObjectName("menu.accnt.ap");
  apPurchaseMenu->setObjectName("menu.accnt.appurchase");
  apVoucherMenu->setObjectName("menu.accnt.apvoucher");
  apMemosMenu->setObjectName("menu.accnt.apmemos");
  apPaymentsMenu->setObjectName("menu.accnt.appayments");
  apFormsMenu->setObjectName("menu.accnt.apforms");
  apReportsMenu->setObjectName("menu.accnt.apreports");
  arMenu->setObjectName("menu.accnt.ar");
  arInvoicesMenu->setObjectName("menu.accnt.arinvoices");
  arMemosMenu->setObjectName("menu.accnt.armemos");
  arCashReceiptsMenu->setObjectName("menu.accnt.arcashreceipts");
  arFormsMenu->setObjectName("menu.accnt.arforms");
  arReportsMenu->setObjectName("menu.accnt.arreports");
  glMenu->setObjectName("menu.accnt.gl");
  glEnterTransactionMenu->setObjectName("menu.accnt.glentertransaction");
  glStandardJournalsMenu->setObjectName("menu.accnt.glstandardjournals");
  bankrecMenu->setObjectName("menu.accnt.bankrec");
  financialReportsMenu->setObjectName("menu.accnt.financialreports");
  reportsMenu->setObjectName("menu.accnt.reports");
  calendarMenu->setObjectName("menu.accnt.calendar");
  coaMenu->setObjectName("menu.accnt.coa");
  budgetMenu->setObjectName("menu.accnt.budget");
  taxMenu->setObjectName("menu.accnt.tax");
  masterInfoMenu->setObjectName("menu.accnt.masterinfo");
  utilitiesMenu->setObjectName("menu.accnt.utilities");

  actionProperties acts[] = { 
    // Accounting | Accounts Payable
    { "menu", tr("Accounts &Payable"), (char*)apMenu,	mainMenu, true,	NULL, NULL, true, NULL },
    
    // Accounting | Accaunts Payable | Purchase Orders
    { "menu", tr("Purchase &Order"), (char*)apPurchaseMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ap.enterPurchaseOrder", tr("&New..."), SLOT(sEnterPurchaseOrder()), apPurchaseMenu, _privileges->check("MaintainPurchaseOrders"), NULL, NULL, true , NULL },
    { "ap.listUnpostedPurchaseOrders", tr("&List Unposted..."), SLOT(sUnpostedPurchaseOrders()), apPurchaseMenu, (_privileges->check("MaintainPurchaseOrders") || _privileges->check("ViewPurchaseOrders")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apPurchaseMenu, true, NULL, NULL, true, NULL },
    { "ap.postPurchaseOrder", tr("&Post..."), SLOT(sPostPurchaseOrder()), apPurchaseMenu, _privileges->check("PostPurchaseOrders"), NULL, NULL, true , NULL },

    // Accounting | Accaunts Payable | Vouchers
    { "menu", tr("&Voucher"), (char*)apVoucherMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ar.enterNewVoucher", tr("&New..."), SLOT(sEnterVoucher()), apVoucherMenu, _privileges->check("MaintainVouchers"), NULL, NULL, true , NULL },
    { "ar.enterNewMiscVoucher", tr("New &Miscellaneous..."), SLOT(sEnterMiscVoucher()), apVoucherMenu, _privileges->check("MaintainVouchers"), NULL, NULL, true , NULL },
    { "ar.listUnpostedVouchers", tr("&List Unposted..."), SLOT(sUnpostedVouchers()), apVoucherMenu, (_privileges->check("MaintainVouchers") || _privileges->check("ViewVouchers")), new QPixmap(":/images/listUnpostedVouchers.png"), toolBar, true , "List Unposted Vouchers" },
    { "separator", NULL, NULL, apVoucherMenu, true, NULL, NULL, true, NULL },
    { "ar.postVouchers", tr("&Post..."), SLOT(sPostVouchers()), apVoucherMenu, _privileges->check("PostVouchers"), NULL, NULL, true , NULL },

    // Accounting | Accaunts Payable | Memos
    { "menu", tr("&Memos"), (char*)apMemosMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ap.enterMiscCreditMemo", tr("&New Misc. Credit Memo..."), SLOT(sEnterMiscApCreditMemo()), apMemosMenu, _privileges->check("MaintainAPMemos"), NULL, NULL, true , NULL },
    { "ap.unapplidCreditMemo", tr("&List Unapplied Credit Memos..."), SLOT(sUnappliedApCreditMemos()), apMemosMenu, (_privileges->check("MaintainAPMemos") || _privileges->check("ViewAPMemos")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apMemosMenu, true, NULL, NULL, true, NULL },
    { "ap.enterMiscDebitMemo", tr("New &Misc. Debit Memo..."), SLOT(sEnterMiscApDebitMemo()), apMemosMenu, _privileges->check("MaintainAPMemos"), NULL, NULL, true , NULL },

    // Accounting | Accaunts Payable |  Payments
    { "menu", tr("&Payments"), (char*)apPaymentsMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ap.selectPayments", tr("&Select..."), SLOT(sSelectPayments()), apPaymentsMenu, _privileges->check("MaintainPayments"), new QPixmap(":/images/selectPayments.png"), toolBar, true , "Select Payments" },
    { "ap.listSelectPayments", tr("&List Selected..."), SLOT(sSelectedPayments()), apPaymentsMenu, _privileges->check("MaintainPayments"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apPaymentsMenu, true, NULL, NULL, true, NULL },
    { "ap.prepareCheckRun", tr("&Prepare Check Run..."), SLOT(sPrepareCheckRun()), apPaymentsMenu, _privileges->check("MaintainPayments"), new QPixmap(":/images/prepareCheckRun.png"), toolBar, true , NULL },
    { "ap.createMiscCheck", tr("Create &Miscellaneous Check..."), SLOT(sCreateMiscCheck()), apPaymentsMenu, _privileges->check("MaintainPayments"), NULL, NULL, true , NULL },
    { "ap.viewCheckRun", tr("Vie&w Check Run..."), SLOT(sViewCheckRun()), apPaymentsMenu, _privileges->check("MaintainPayments"), new QPixmap(":/images/viewCheckRun.png"), toolBar, true , NULL },
    { "separator", NULL, NULL, apPaymentsMenu, true, NULL, NULL, true, NULL },
    { "ap.voidCheckRun", tr("&Void Check Run..."), SLOT(sVoidCheckRun()), apPaymentsMenu, _privileges->check("MaintainPayments"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apPaymentsMenu, true, NULL, NULL, true, NULL },
    { "ap.postCheck", tr("Post &Check..."), SLOT(sPostCheck()), apPaymentsMenu, _privileges->check("PostPayments"), NULL, NULL, true , NULL },
    { "ap.postChecks", tr("P&ost Checks..."), SLOT(sPostChecks()), apPaymentsMenu, _privileges->check("PostPayments"), NULL, NULL, true , NULL },
                       
    { "separator", NULL, NULL, apMenu, true, NULL, NULL, true, NULL },
    
    // Accounting | Accaunts Payable | Forms
    { "menu", tr("&Forms"), (char*)apFormsMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ap.printPurchaseOrder", tr("Print Purchase &Order..."), SLOT(sPrintPurchaseOrder()), apFormsMenu, _privileges->check("PrintPurchaseOrders"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apFormsMenu, true, NULL, NULL, true, NULL },
    { "ap.printCheck", tr("Print &Check..."), SLOT(sPrintCheck()), apFormsMenu, _privileges->check("MaintainPayments"), NULL, NULL, true , NULL },
    { "ap.printCheckRun", tr("Print Check &Run..."), SLOT(sPrintCheckRun()), apFormsMenu, _privileges->check("MaintainPayments"), NULL, NULL, true , NULL },
    
    // Accounting | Accaunts Payable |  Reports
    { "menu", tr("&Reports"), (char*)apReportsMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ap.uninvoicedReceipts", tr("&Uninvoiced Receipts and Returns..."), SLOT(sDspUninvoicedReceipts()), apReportsMenu, (_privileges->check("ViewUninvoicedReceipts") || _privileges->check("MaintainUninvoicedReceipts")), NULL, NULL, true , NULL },
    { "ap.voucheringEditList", tr("Vouchering &Edit List..."), SLOT(sVoucheringEditList()), apReportsMenu, (_privileges->check("MaintainVouchers") || _privileges->check("ViewVouchers")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, true, NULL, NULL, true, NULL },
    { "ap.dspOpenAPItemsByVendor", tr("Open &Items by Vendor..."), SLOT(sDspAPOpenItemsByVendor()), apReportsMenu, _privileges->check("ViewAPOpenItems"), NULL, NULL, true , NULL },
    { "ap.dspAPAging", tr("A/P &Aging..."), SLOT(sDspTimePhasedOpenAPItems()), apReportsMenu, _privileges->check("ViewAPOpenItems"), new QPixmap(":/images/apAging.png"), toolBar, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, true, NULL, NULL, true, NULL },
    { "ap.dspCheckRegister", tr("&Check Register..."), SLOT(sDspCheckRegister()), apReportsMenu, _privileges->check("MaintainPayments"), NULL, NULL, true , NULL },
    { "ap.dspVoucherRegister", tr("&Voucher Register..."), SLOT(sDspVoucherRegister()), apReportsMenu, (_privileges->check("MaintainVouchers") || _privileges->check("ViewVouchers")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, true, NULL, NULL, true, NULL },
    { "ap.dspVendorHistory", tr("Vendor &History..."), SLOT(sDspVendorHistory()), apReportsMenu, _privileges->check("ViewAPOpenItems"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, true, NULL, NULL, true, NULL },
    { "ap.rptPayablesJournal", tr("Pa&yables Journal..."), SLOT(sRptPayablesJournal()), apReportsMenu, _privileges->check("PrintAPJournals"), NULL, NULL, true , NULL },
    { "ap.rptCheckJournal", tr("Check &Journal..."), SLOT(sRptCheckJournal()), apReportsMenu, _privileges->check("PrintAPJournals"), NULL, NULL, true , NULL },
    
    { "separator", NULL, NULL, apMenu, true, NULL, NULL, true, NULL },
    { "ap.vendors", tr("Ve&ndors..."), SLOT(sVendors()), apMenu, (_privileges->check("MaintainVendors") || _privileges->check("ViewVendors")), NULL, NULL, true , NULL },
    
    // Accounting | Accounts Receivable
    { "menu", tr("Accounts Recei&vable"),	(char*)arMenu,	mainMenu, true, NULL, NULL, true, NULL },
  
    // Accounting | Accounts Receivable | Invoices
    { "menu", tr("&Invoice"), (char*)arInvoicesMenu,	arMenu, true,	 NULL, NULL, true, NULL },
    { "ar.createInvoice", tr("&New..."), SLOT(sCreateInvoice()), arInvoicesMenu, _privileges->check("MaintainMiscInvoices"), NULL, NULL, true , NULL },
    { "ar.listUnpostedInvoices", tr("&List Unposted..."), SLOT(sUnpostedInvoices()), arInvoicesMenu, _privileges->check("SelectBilling"), new QPixmap(":/images/unpostedInvoices.png"), toolBar, true , "List Unposted Invoices" },
    { "separator", NULL, NULL, arInvoicesMenu, true, NULL, NULL, true, NULL },
    { "ar.postInvoices", tr("&Post..."), SLOT(sPostInvoices()), arInvoicesMenu, _privileges->check("PostMiscInvoices"), NULL, NULL, true , NULL },

    // Accounting | Accounts Receivable | Memos
    { "menu", tr("&Memos"), (char*)arMemosMenu,	arMenu, true,	 NULL, NULL, true, NULL },
    { "ar.enterMiscCreditMemo", tr("&New Misc. Credit Memo..."), SLOT(sEnterMiscArCreditMemo()), arMemosMenu, _privileges->check("MaintainARMemos"), NULL, NULL, true , NULL },
    { "ar.unapplidCreditMemo", tr("&List Unapplied Credit Memos..."), SLOT(sUnappliedArCreditMemos()), arMemosMenu, (_privileges->check("MaintainARMemos") || _privileges->check("ViewARMemos")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arMemosMenu, true, NULL, NULL, true, NULL },
    { "ar.enterMiscDebitMemo", tr("New &Misc. Debit Memo..."), SLOT(sEnterMiscArDebitMemo()), arMemosMenu, _privileges->check("MaintainARMemos"), NULL, NULL, true , NULL },

    // Accounting | Accounts Receivable | Cash Receipts
    { "menu", tr("C&ash Receipt"), (char*)arCashReceiptsMenu,	arMenu, true,	 NULL, NULL, true, NULL },
    { "ar.enterCashReceipt", tr("&New..."), SLOT(sEnterCashReceipt()), arCashReceiptsMenu, _privileges->check("MaintainCashReceipts"), NULL, NULL, true , NULL },
    { "ar.cashReceiptEditList", tr("&Edit List..."), SLOT(sCashReceiptEditList()), arCashReceiptsMenu, (_privileges->check("MaintainCashReceipts") || _privileges->check("ViewCashReceipt")), new QPixmap(":/images/editCashReceipts.png"), toolBar, true , "Cash Receipt Edit List" },
    { "ar.postCashReceipts", tr("&Post..."), SLOT(sPostCashReceipts()), arCashReceiptsMenu, _privileges->check("PostCashReceipts"), NULL, NULL, true , NULL },

    { "separator", NULL, NULL, arMenu, true, NULL, NULL, true, NULL },
    { "ar.arWorkBench", tr("A/R &Workbench..."), SLOT(sArWorkBench()), arMenu, _privileges->check("ViewAROpenItems") , new QPixmap(":/images/arWorkbench.png"), toolBar, true , NULL },

    // Accounting | Accounts Receivable | Forms
    { "menu", tr("&Forms"), (char*)arFormsMenu,	arMenu, true,	 NULL, NULL, true, NULL },
    { "ar.printInvoices", tr("Print &Invoices..."), SLOT(sPrintInvoices()), arFormsMenu, _privileges->check("PrintInvoices"), NULL, NULL, true , NULL },
    { "ar.reprintInvoices", tr("&Re-Print Invoices..."), SLOT(sReprintInvoices()), arFormsMenu, _privileges->check("PrintInvoices"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arFormsMenu, true, NULL, NULL, _metrics->boolean("EnableBatchManager"), NULL },
    { "ar.scheduleInvoiceForEmailDelivery", tr("&Schedule Invoice for Email Delivery..."), SLOT(sDeliverInvoice()), arFormsMenu, _privileges->check("PrintInvoices"), NULL, NULL, _metrics->boolean("EnableBatchManager") , NULL },
    { "separator", NULL, NULL, arFormsMenu, true, NULL, NULL, true, NULL },
    { "ar.printStatementByCustomer", tr("Print S&tatement by Customer..."), SLOT(sPrintStatementByCustomer()), arFormsMenu, _privileges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "ar.printStatementsByCustomerType", tr("Print State&ments by Customer Type..."), SLOT(sPrintStatementsByCustomerType()), arFormsMenu, _privileges->check("ViewAROpenItems"), NULL, NULL, true , NULL },

    // Accounting | Accounts Receivable | Reports
    { "menu", tr("&Reports"), (char*)arReportsMenu,	arMenu, true,	 NULL, NULL, true, NULL },
    { "ar.dspInvoiceInformation", tr("&Invoice Information..."), SLOT(sDspInvoiceInformation()), arReportsMenu, _privileges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arReportsMenu, true, NULL, NULL, true, NULL },
    { "ar.dspOpenItems", tr("&Open Items..."), SLOT(sDspAROpenItems()), arReportsMenu, _privileges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "ar.dspOpenItemsByCustomer", tr("Open Items by &Customer..."), SLOT(sDspAROpenItemsByCustomer()), arReportsMenu, _privileges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "ar.dspARAging", tr("A/R A&ging..."), SLOT(sDspTimePhasedOpenItems()), arReportsMenu, _privileges->check("ViewAROpenItems"), new QPixmap(":/images/arAging.png"), toolBar, true , NULL },
    { "separator", NULL, NULL, arReportsMenu, true, NULL, NULL, true, NULL }, 
    { "ar.dspInvoiceRegister", tr("In&voice Register..."), SLOT(sDspInvoiceRegister()), arReportsMenu, _privileges->check("ViewInvoiceRegister"), NULL, NULL, true , NULL },
    { "ar.dspCashReceipts", tr("Cash &Receipts..."), SLOT(sDspCashReceipts()), arReportsMenu, _privileges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "ar.dspARApplications", tr("A/R &Applications..."), SLOT(sDspARApplications()), arReportsMenu, _privileges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "ar.dspDepositsRegister", tr("&Deposits Register..."), SLOT(sDspDepositsRegister()), arReportsMenu, _privileges->check("ViewDepositsRegister"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arReportsMenu, true, NULL, NULL, true, NULL },
    { "ar.dspCustomerHistory", tr("Customer &History..."), SLOT(sDspCustomerHistory()), arReportsMenu, _privileges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arReportsMenu, true, NULL, NULL, true, NULL },
    { "ar.rptSalesJournal", tr("Sales &Journal..."), SLOT(sRptSalesJournal()), arReportsMenu, _privileges->check("PrintARJournals"), NULL, NULL, true , NULL },
    { "ar.rptCreditMemoJournal", tr("Credit &Memo Journal..."), SLOT(sRptCreditMemoJournal()), arReportsMenu, _privileges->check("PrintARJournals"), NULL, NULL, true , NULL },

    { "separator", NULL, NULL, arMenu, true, NULL, NULL, true, NULL },
    { "ar.customers", tr("&Customers..."), SLOT(sCustomers()), arMenu, (_privileges->check("MaintainCustomerMasters") || _privileges->check("ViewCustomerMasters")), NULL, NULL, true , NULL },
  
    // Accounting | G/L
    { "menu",		    tr("General &Ledger"),		(char*)glMenu,	mainMenu,		true,					NULL, NULL, true, NULL },
    
    // Accounting | G/L | Journals
    { "menu",		    tr("&Journal Entry"),		(char*)glEnterTransactionMenu,	glMenu,		true,					NULL, NULL, true, NULL },
    { "gl.simpleEntry",	    tr("S&imple..."),	SLOT(sSimpleEntry()),		glEnterTransactionMenu,	_privileges->check("PostJournalEntries"),NULL, NULL, true, NULL },
    { "gl.seriesEntry",     tr("&Series..."),	SLOT(sSeriesEntry()),		glEnterTransactionMenu,	_privileges->check("PostJournalEntries"),NULL, NULL, true, NULL },
    { "separator",	    NULL,				NULL,			        glEnterTransactionMenu,   true,					NULL, NULL, true, NULL },
    { "gl.unpostedEntries", tr("&List Unposted..."), SLOT(sUnpostedEntries()),	glEnterTransactionMenu,	_privileges->check("PostJournalEntries"), new QPixmap(":/images/journalEntries.png"), toolBar,  true, "List Unposted Journal Entries" },

    // Accounting | G/L | Standard Journals
    { "menu",			     tr("&Standard Journals"),		   (char*)glStandardJournalsMenu,	     glMenu,		   true,					      NULL, NULL, true, NULL },
    { "gl.enterNewStandardJournal",  tr("&New..."),  SLOT(sEnterStandardJournal()),    glStandardJournalsMenu, _privileges->check("MaintainStandardJournals"),     NULL, NULL, true, NULL },
    { "gl.listStandardJournals",     tr("&List..."),	   SLOT(sStandardJournals()),	     glStandardJournalsMenu, _privileges->check("MaintainStandardJournals"),     NULL, NULL, true, NULL },
    { "gl.listStandardJournalGroups",tr("List &Groups..."),SLOT(sStandardJournalGroups()),   glStandardJournalsMenu, _privileges->check("MaintainStandardJournalGroups"),NULL, NULL, true, NULL },
    { "separator",		     NULL,				   NULL,			     glStandardJournalsMenu, true,					      NULL, NULL, true, NULL },
    { "gl.postStandardJournal",	     tr("&Post..."),	   SLOT(sPostStandardJournal()),     glStandardJournalsMenu, _privileges->check("PostStandardJournals"),	      NULL, NULL, true, NULL },
    { "gl.postStandardJournalGroup", tr("Post G&roup..."), SLOT(sPostStandardJournalGroup()),glStandardJournalsMenu, _privileges->check("PostStandardJournalGroups"),    NULL, NULL, true, NULL },

    { "menu",			tr("&Bank Reconciliation"), 	(char*)bankrecMenu,		mainMenu,    true,						NULL, NULL, true, NULL },
    { "gl.reconcileBankaccnt",	tr("&Reconcile..."),SLOT(sReconcileBankaccount()),	bankrecMenu, _privileges->check("MaintainBankRec"), new QPixmap(":/images/bankReconciliation.png"), toolBar,  true, "Reconcile Bank Account" },
    { "separator",		NULL,				NULL,				bankrecMenu, true,						NULL, NULL, true, NULL },
    { "gl.enterAdjustment",	tr("&New Adjustment..."),	SLOT(sEnterAdjustment()),	bankrecMenu, _privileges->check("MaintainBankAdjustments"),	NULL, NULL, true, NULL },
    { "gl.adjustmentEditList",	tr("Adjustment Edit &List..."),	SLOT(sAdjustmentEditList()),	bankrecMenu, (_privileges->check("MaintainBankAdjustments") || _privileges->check("ViewBankAdjustments")),NULL, NULL, true, NULL },

    { "separator",		  NULL,					NULL,					mainMenu,		true,					       NULL, NULL, true, NULL },
    
    // Accounting | Reports
    { "menu",				tr("&Reports"),				(char*)reportsMenu,			mainMenu,      true,					NULL, NULL, true, NULL },
    { "gl.dspGLTransactions",		tr("G/L &Transactions..."),		SLOT(sDspGLTransactions()),		reportsMenu, _privileges->check("ViewGLTransactions"),	NULL, NULL, true, NULL },
    { "gl.dspSummarizedGLTransactions",	tr("Su&mmarized G/L Transactions..."),	SLOT(sDspSummarizedGLTransactions()),	reportsMenu, _privileges->check("ViewGLTransactions"),	NULL, NULL, true, NULL },
    { "gl.dspGLSeries",			tr("G/L &Series..."),			SLOT(sDspGLSeries()),			reportsMenu, _privileges->check("ViewGLTransactions"),	NULL, NULL, true, NULL },
    { "gl.dspStandardJournalHistory",	tr("Standard &Journal History..."),	SLOT(sDspStandardJournalHistory()),	reportsMenu, _privileges->check("ViewGLTransactions"),	NULL, NULL, true, NULL },
    { "separator",		  NULL,					NULL,					reportsMenu,		true,					       NULL, NULL, true, NULL },
    { "gl.dspBankrecHistory",		tr("&Bank Rec. History"),		SLOT(sDspBankrecHistory()),		reportsMenu, _privileges->check("ViewBankRec"),		NULL, NULL, true, NULL },
    { "gl.dspSummarizedBankrecHistory",	tr("Summari&zed Bank Rec. History"),	SLOT(sDspSummarizedBankrecHistory()),	reportsMenu, _privileges->check("ViewBankRec"),		NULL, NULL, true, NULL },

    // Accounting | Statements
    { "menu",			  tr("Financial &Statements"),		(char*)financialReportsMenu,		mainMenu,			true,					       NULL, NULL, true, NULL },
    { "gl.createFinancialReports",tr("&New Financial Report..."),	SLOT(sNewFinancialReport()),		financialReportsMenu,		_privileges->check("MaintainFinancialLayouts"), NULL, NULL, true, NULL },
    { "gl.editFinancialReports",  tr("&List Financial Reports..."),	SLOT(sFinancialReports()),		financialReportsMenu,		_privileges->check("MaintainFinancialLayouts"), NULL, NULL, true, NULL },
    { "separator",		  NULL,					NULL,					financialReportsMenu,		true,					       NULL, NULL, true, NULL },
    { "gl.dspTrialBalances",	  tr("View &Trial Balances..."),		SLOT(sDspTrialBalances()),		financialReportsMenu,		_privileges->check("ViewTrialBalances"),	   new QPixmap(":/images/viewTrialBalance.png"), toolBar,  true, NULL },
    { "gl.viewFinancialReport",	  tr("View &Financial Report..."),	SLOT(sViewFinancialReport()),		financialReportsMenu,		_privileges->check("ViewFinancialReports"),   new QPixmap(":/images/viewFinancialReport.png"), toolBar, true, NULL },

    { "separator",		  NULL,					NULL,					mainMenu,		true,					       NULL, NULL, true, NULL },
    
    // Accounting | Fiscal Calendar
    { "menu", tr("&Fiscal Calendar"), (char*)calendarMenu, mainMenu,	true,	NULL, NULL, true, NULL },
    { "gl.accountingYearPeriods",	tr("Fiscal &Years..."),	SLOT(sAccountingYearPeriods()),	calendarMenu,	_privileges->check("MaintainAccountingPeriods"),	NULL, NULL, true, NULL },
    { "gl.accountingPeriods",	tr("Accounting &Periods..."),	SLOT(sAccountingPeriods()),	calendarMenu,	_privileges->check("MaintainAccountingPeriods"),	NULL, NULL, true, NULL },
    
    // Accounting | Account
    { "menu", tr("&Account"), (char*)coaMenu, mainMenu,	true,	NULL, NULL, true, NULL },
    { "gl.accountNumbers",	tr("&Chart of Accounts..."),	SLOT(sAccountNumbers()), coaMenu,	_privileges->check("MaintainChartOfAccounts"),	NULL, NULL, true, NULL },
    { "gl.companies",		tr("C&ompanies..."),		SLOT(sCompanies()),		coaMenu,	(_privileges->check("MaintainChartOfAccounts") && (_metrics->value("GLCompanySize").toInt() > 0)), NULL, NULL, true, NULL },
    { "gl.profitCenterNumber",	tr("&Profit Center Numbers..."),	SLOT(sProfitCenters()),	coaMenu,	(_privileges->check("MaintainChartOfAccounts") && (_metrics->value("GLProfitSize").toInt() > 0)), NULL, NULL, true, NULL },
    { "gl.subaccountNumbers",	tr("&Subaccount Numbers..."),	SLOT(sSubaccounts()), coaMenu,	(_privileges->check("MaintainChartOfAccounts") && (_metrics->value("GLSubaccountSize").toInt() > 0)), NULL, NULL, true, NULL },
    { "gl.subAccntTypes",	tr("Su&baccount Types..."),	SLOT(sSubAccntTypes()),	coaMenu,	_privileges->check("MaintainChartOfAccounts"),	NULL, NULL, true, NULL },

    // Accounting | Budget
    { "menu", tr("Bu&dget"), (char*)budgetMenu, mainMenu,	true,	NULL, NULL, true, NULL },
    { "gl.maintainBudget",	tr("&New Budget..."),	SLOT(sMaintainBudget()), budgetMenu,	_privileges->check("MaintainBudgets"),	NULL, NULL, true, NULL },
    { "gl.maintainBudget",	tr("&List Budgets..."),	SLOT(sBudgets()),	 budgetMenu,	(_privileges->check("MaintainBudgets") || _privileges->check("ViewBudgets")),	NULL, NULL, true, NULL },

    // Accounting | Tax
    { "menu", tr("&Tax"), (char*)taxMenu, mainMenu,	true,	NULL, NULL, true, NULL },
    { "gl.searchForTaxAuth",	tr("&Search for Tax Authority..."), SLOT(sTaxAuthoritySearch()),	taxMenu,	(_privileges->check("MaintainTaxAuthorities") || _privileges->check("ViewTaxAuthorities")), NULL, NULL, true, NULL },
    { "gl.taxAuthorities",	tr("Tax &Authorities..."),	SLOT(sTaxAuthorities()),	taxMenu,	(_privileges->check("MaintainTaxAuthorities") || _privileges->check("ViewTaxAuthorities")), NULL, NULL, true, NULL },
    { "gl.taxCodes",		tr("Tax &Codes..."),		SLOT(sTaxCodes()),		taxMenu,	(_privileges->check("MaintainTaxCodes") || _privileges->check("ViewTaxCodes")), NULL, NULL, true, NULL },
    { "gl.taxTypes",		tr("Tax &Types..."),		SLOT(sTaxTypes()),		taxMenu,	(_privileges->check("MaintainTaxTypes") || _privileges->check("ViewTaxTypes")), NULL, NULL, true, NULL },
    { "gl.taxSelections",	tr("Tax Se&lections..."),	SLOT(sTaxSelections()),		taxMenu,	(_privileges->check("MaintainTaxSel") || _privileges->check("ViewTaxSel")), NULL, NULL, true, NULL },
    { "gl.taxRegistatrions",	tr("Tax &Registrations..."),	SLOT(sTaxRegistrations()),	taxMenu,	_privileges->check("MaintainChartOfAccounts"),   NULL, NULL, true, NULL },
    
    { "separator",		  NULL,					NULL,					mainMenu,		true,					       NULL, NULL, true, NULL },
   
    // Accounting | Master Information
    { "menu",			tr("&Master Information"),	(char*)masterInfoMenu,		mainMenu,	true,						NULL, NULL, true, NULL },
    { "gl.postTransactionsToExternalAccountingSystem", tr("Post Transactions to External Accounting System..."), SLOT(sPostTransactionsToExternal()), utilitiesMenu, _privileges->check("ViewGLTransactions"), NULL, NULL, _metrics->boolean("EnableExternalAccountingInterface") , NULL },
    { "gl.dspRWTransactions",		tr("Display Exported Transactions..."),	SLOT(sDspRWTransactions()),	utilitiesMenu,	_privileges->check("ViewGLTransactions"), NULL, NULL, _metrics->boolean("EnableExternalAccountingInterface") , NULL },                               
    { "ap.terms", tr("Ter&ms..."), SLOT(sTerms()), masterInfoMenu, (_privileges->check("MaintainTerms") || _privileges->check("ViewTerms")), NULL, NULL, true , NULL },
    { "separator",			NULL,					NULL,				masterInfoMenu,	true,	NULL, NULL, true , NULL },
    { "ap.bankAccounts", tr("&Bank Accounts..."), SLOT(sBankAccounts()), masterInfoMenu, (_privileges->check("MaintainBankAccounts") || _privileges->check("ViewBankAccounts")), NULL, NULL, true , NULL },
    { "ap.checkFormats", tr("&Check Formats..."), SLOT(sCheckFormats()), masterInfoMenu, (_privileges->check("MaintainCheckFormats") || _privileges->check("ViewCheckFormats")), NULL, NULL, true , NULL },
    { "ap.costCategories", tr("C&ost Categories..."), SLOT(sCostCategories()), masterInfoMenu, (_privileges->check("MaintainCostCategories")) || (_privileges->check("ViewCostCategories")), NULL, NULL, true , NULL },
    { "ap.expenseCategories", tr("&Expense Categories..."), SLOT(sExpenseCategories()), masterInfoMenu, (_privileges->check("MaintainExpenseCategories")) || (_privileges->check("ViewExpenseCategories")), NULL, NULL, true , NULL },
    { "ap.apAccountAssignments", tr("A/&P Account Assignments..."), SLOT(sAPAssignments()), masterInfoMenu, (_privileges->check("MaintainVendorAccounts") || _privileges->check("ViewVendorAccounts")), NULL, NULL, true , NULL },
    { "separator",		  NULL,					NULL,					masterInfoMenu,		true,					       NULL, NULL, true, NULL },
    { "ar.customerTypes", tr("Customer &Types..."), SLOT(sCustomerTypes()), masterInfoMenu, (_privileges->check("MaintainCustomerTypes") || _privileges->check("ViewCustomerTypes")), NULL, NULL, true , NULL },
    { "ar.vendorTypes", tr("&Vendor Types..."), SLOT(sVendorTypes()), masterInfoMenu, (_privileges->check("MaintainVendorTypes")) || (_privileges->check("ViewVendorTypes")), NULL, NULL, true , NULL },
    { "ar.salesCategories", tr("&Sales Categories..."), SLOT(sSalesCategories()), masterInfoMenu, (_privileges->check("MaintainSalesCategories")) || (_privileges->check("ViewSalesCategories")), NULL, NULL, true , NULL },
    { "ar.arAccountAssignments", tr("A/R Account Assi&gnments..."), SLOT(sARAccountAssignments()), masterInfoMenu, (_privileges->check("MaintainSalesAccount") || _privileges->check("ViewSalesAccount")), NULL, NULL, true , NULL },
    { "ar.reasonCodes", tr("&Reason Codes..."), SLOT(sReasonCodes()), masterInfoMenu, _privileges->check("MaintainReasonCodes"), NULL, NULL, true , NULL },
    { "separator",		  NULL,					NULL,					masterInfoMenu,		true,					       NULL, NULL, true, NULL },
    { "gl.adjustmentTypes",	tr("&Adjustment Types..."),	SLOT(sAdjustmentTypes()),	masterInfoMenu,	(_privileges->check("MaintainAdjustmentTypes") || _privileges->check("ViewAdjustmentTypes")),	NULL, NULL, true, NULL },

    // Accounting | Utilities
    { "menu",				tr("&Utilities"),			(char*)utilitiesMenu,		mainMenu,	true,	NULL, NULL, true, NULL },
    { "gl.forwardUpdateAccounts",	tr("&Forward Update Accounts..."),	SLOT(sForwardUpdateAccounts()),	utilitiesMenu,	_privileges->check("ViewTrialBalances"),	NULL, NULL, true, NULL },
    { "gl.duplicateAccountNumbers",      tr("&Duplicate Account Numbers..."),  SLOT(sDuplicateAccountNumbers()), utilitiesMenu,  _privileges->check("MaintainChartOfAccounts"), NULL, NULL, true, NULL },
    { "separator",		  NULL,					NULL,					utilitiesMenu,		true,					       NULL, NULL, true, NULL },
    { "so.purgeInvoices", tr("Purge &Invoices..."), SLOT(sPurgeInvoices()), utilitiesMenu, _privileges->check("PurgeInvoices"), NULL, NULL, true , NULL },
    { "ar.updateLateCustCreditStatus", tr("&Update Late Customer Credit Status..."), SLOT(sUpdateLateCustCreditStatus()), utilitiesMenu, _privileges->check("UpdateCustomerCreditStatus"), NULL, NULL, _metrics->boolean("AutoCreditWarnLateCustomers"), NULL },
    { "ar.createRecurringInvoices", tr("&Create Recurring Invoices..."), SLOT(sCreateRecurringInvoices()), utilitiesMenu, _privileges->check("MaintainMiscInvoices"), NULL, NULL, true, NULL },
  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));
  
  parent->populateCustomMenu(mainMenu, "Accounting");
  parent->menuBar()->insertItem(tr("&Accounting"), mainMenu);
}

void menuAccounting::addActionsToMenu(actionProperties acts[], unsigned int numElems)
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

//  Purchase Orders
void menuAccounting::sEnterPurchaseOrder()
{
  ParameterList params;
  params.append("mode", "new");

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sUnpostedPurchaseOrders()
{
  omfgThis->handleNewWindow(new unpostedPurchaseOrders());
}

void menuAccounting::sPrintPurchaseOrder()
{
  printPurchaseOrder(parent, "", TRUE).exec();
}

void menuAccounting::sPostPurchaseOrder()
{
  postPurchaseOrder(parent, "", TRUE).exec();
}

//  Vouchers
void menuAccounting::sDspUninvoicedReceipts()
{
  omfgThis->handleNewWindow(new dspUninvoicedReceivings());
}

void menuAccounting::sEnterVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  voucher *newdlg = new voucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sEnterMiscVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  miscVoucher *newdlg = new miscVoucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sUnpostedVouchers()
{
  omfgThis->handleNewWindow(new openVouchers());
}

void menuAccounting::sVoucheringEditList()
{
  omfgThis->handleNewWindow(new voucheringEditList());
}

void menuAccounting::sPostVouchers()
{
  postVouchers(parent, "", TRUE).exec();
}


//  Payments
void menuAccounting::sSelectPayments()
{
  omfgThis->handleNewWindow(new selectPayments());
}

void menuAccounting::sSelectedPayments()
{
  omfgThis->handleNewWindow(new selectedPayments());
}

void menuAccounting::sCreateMiscCheck()
{
  ParameterList params;
  params.append("new");

  miscCheck *newdlg = new miscCheck();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sPrepareCheckRun()
{
  prepareCheckRun(parent, "", TRUE).exec();
}

void menuAccounting::sViewCheckRun()
{
  omfgThis->handleNewWindow(new viewCheckRun());
}

void menuAccounting::sPrintCheck()
{
  printCheck(parent, "", TRUE).exec();
}

void menuAccounting::sPrintCheckRun()
{
  printChecks(parent, "", TRUE).exec();
}

void menuAccounting::sVoidCheckRun()
{
  voidChecks newdlg(parent, "", TRUE);
  newdlg.exec();
}

void menuAccounting::sPostCheck()
{
  postCheck(parent, "", TRUE).exec();
}

void menuAccounting::sPostChecks()
{
  postChecks(parent, "", TRUE).exec();
}


//  Memos
void menuAccounting::sEnterMiscApCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");

  apOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sUnappliedApCreditMemos()
{
  omfgThis->handleNewWindow(new unappliedAPCreditMemos());
}

void menuAccounting::sEnterMiscApDebitMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "debitMemo");

  apOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}


// AP Displays
void menuAccounting::sDspVendorHistory()
{
  omfgThis->handleNewWindow(new dspVendorAPHistory());
}

void menuAccounting::sDspCheckRegister()
{
  omfgThis->handleNewWindow(new dspCheckRegister());
}

void menuAccounting::sDspVoucherRegister()
{
  omfgThis->handleNewWindow(new dspVoucherRegister());
}

void menuAccounting::sDspAPOpenItemsByVendor()
{
  omfgThis->handleNewWindow(new dspAPOpenItemsByVendor());
}

void menuAccounting::sDspTimePhasedOpenAPItems()
{
  omfgThis->handleNewWindow(new dspTimePhasedOpenAPItems());
}

void menuAccounting::sCreateInvoice()
{
  invoice::newInvoice(-1);
}

void menuAccounting::sUnpostedInvoices()
{
  omfgThis->handleNewWindow(new unpostedInvoices());
}

void menuAccounting::sReprintInvoices()
{
  reprintInvoices(parent, "", TRUE).exec();
}

void menuAccounting::sDeliverInvoice()
{
  deliverInvoice(parent, "", TRUE).exec();
}

void menuAccounting::sPrintInvoices()
{
  printInvoices(parent, "", TRUE).exec();
}

void menuAccounting::sPostInvoices()
{
  postInvoices(parent, "", TRUE).exec();
}

void menuAccounting::sPurgeInvoices()
{
  purgeInvoices(parent, "", TRUE).exec();
}



//  Cash Receipts
void menuAccounting::sEnterCashReceipt()
{
  ParameterList params;
  params.append("mode", "new");

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sCashReceiptEditList()
{
  omfgThis->handleNewWindow(new cashReceiptsEditList());
}

void menuAccounting::sPostCashReceipts()
{
  postCashReceipts(parent, "", TRUE).exec();
}


//  Memos
void menuAccounting::sEnterMiscArCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");

  arOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sUnappliedArCreditMemos()
{
  omfgThis->handleNewWindow(new unappliedARCreditMemos());
}

void menuAccounting::sEnterMiscArDebitMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "debitMemo");

  arOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

//  Displays
void menuAccounting::sArWorkBench()
{
  omfgThis->handleNewWindow(new arWorkBench());
}

void menuAccounting::sDspCustomerHistory()
{
  omfgThis->handleNewWindow(new dspCustomerARHistory());
}

void menuAccounting::sDspCashReceipts()
{
  omfgThis->handleNewWindow(new dspCashReceipts());
}

void menuAccounting::sDspARApplications()
{
  omfgThis->handleNewWindow(new dspARApplications());
}

void menuAccounting::sDspInvoiceInformation()
{
  omfgThis->handleNewWindow(new dspInvoiceInformation());
}

void menuAccounting::sDspInvoiceRegister()
{
  omfgThis->handleNewWindow(new dspInvoiceRegister());
}

void menuAccounting::sDspDepositsRegister()
{
  omfgThis->handleNewWindow(new dspDepositsRegister());
}

void menuAccounting::sRptSalesJournal()
{
  ParameterList params;
  params.append("type", SalesJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sRptCreditMemoJournal()
{
  ParameterList params;
  params.append("type", CreditMemoJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sRptPayablesJournal()
{
  ParameterList params;
  params.append("type", PayablesJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sRptCheckJournal()
{
  ParameterList params;
  params.append("type", CheckJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sDspAROpenItemsByCustomer()
{
  omfgThis->handleNewWindow(new dspAROpenItemsByCustomer());
}

void menuAccounting::sDspAROpenItems()
{
  omfgThis->handleNewWindow(new dspAROpenItems());
}

void menuAccounting::sDspTimePhasedOpenItems()
{
  omfgThis->handleNewWindow(new dspTimePhasedOpenARItems());
}


// General Ledger
void menuAccounting::sEnterStandardJournal()
{
  ParameterList params;
  params.append("mode", "new");

  standardJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sStandardJournals()
{
  omfgThis->handleNewWindow(new standardJournals());
}

void menuAccounting::sStandardJournalGroups()
{
  omfgThis->handleNewWindow(new standardJournalGroups());
}

void menuAccounting::sPostStandardJournal()
{
  postStandardJournal(parent, "", TRUE).exec();
}

void menuAccounting::sPostStandardJournalGroup()
{
  postStandardJournalGroup(parent, "", TRUE).exec();
}

void menuAccounting::sSimpleEntry()
{
  ParameterList params;
  params.append("mode", "new");

  glTransaction newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sSeriesEntry()
{
  ParameterList params;
  params.append("mode", "new");

  glSeries newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sUnpostedEntries()
{
  omfgThis->handleNewWindow(new unpostedGlSeries());
}

//  Financial Layouts
void menuAccounting::sFinancialReports()
{
  omfgThis->handleNewWindow(new financialLayouts());
}

void menuAccounting::sViewFinancialReport()
{
  omfgThis->handleNewWindow(new dspFinancialReport());
}

void menuAccounting::sNewFinancialReport()
{
  ParameterList params;
  params.append("mode", "new");

  financialLayout newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}


//  Displays
void menuAccounting::sDspGLTransactions()
{
  omfgThis->handleNewWindow(new dspGLTransactions());
}

void menuAccounting::sDspSummarizedGLTransactions()
{
  omfgThis->handleNewWindow(new dspSummarizedGLTransactions());
}

void menuAccounting::sDspGLSeries()
{
  omfgThis->handleNewWindow(new dspGLSeries());
}

void menuAccounting::sDspStandardJournalHistory()
{
  omfgThis->handleNewWindow(new dspStandardJournalHistory());
}

void menuAccounting::sDspTrialBalances()
{
  omfgThis->handleNewWindow(new dspTrialBalances());
}

//  Master Information
void menuAccounting::sCompanies()
{
  omfgThis->handleNewWindow(new companies());
}

void menuAccounting::sProfitCenters()
{
  omfgThis->handleNewWindow(new profitCenters());
}

void menuAccounting::sSubaccounts()
{
  omfgThis->handleNewWindow(new subaccounts());
}

void menuAccounting::sAccountNumbers()
{
  omfgThis->handleNewWindow(new accountNumbers());
}

void menuAccounting::sDuplicateAccountNumbers()
{
  omfgThis->handleNewWindow(new duplicateAccountNumbers());
}

void menuAccounting::sSubAccntTypes()
{
  omfgThis->handleNewWindow(new subAccntTypes());
}

void menuAccounting::sAccountingPeriods()
{
  omfgThis->handleNewWindow(new accountingPeriods());
}

void menuAccounting::sAccountingYearPeriods()
{
  omfgThis->handleNewWindow(new accountingYearPeriods());
}

void menuAccounting::sReconcileBankaccount()
{
  omfgThis->handleNewWindow(new reconcileBankaccount());
}

void menuAccounting::sEnterAdjustment()
{
  ParameterList params;
  params.append("mode", "new");

  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sAdjustmentEditList()
{
  omfgThis->handleNewWindow(new bankAdjustmentEditList());
}

void menuAccounting::sAdjustmentTypes()
{
  omfgThis->handleNewWindow(new bankAdjustmentTypes());
}

void menuAccounting::sTaxAuthorities()
{
  omfgThis->handleNewWindow(new taxAuthorities());
}

void menuAccounting::sTaxAuthoritySearch()
{
  ParameterList params;
  params.append("crmaccnt_subtype", "taxauth");

  searchForCRMAccount *newdlg = new searchForCRMAccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sTaxCodes()
{
  omfgThis->handleNewWindow(new taxCodes());
}

void menuAccounting::sTaxTypes()
{
  omfgThis->handleNewWindow(new taxTypes());
}

void menuAccounting::sTaxSelections()
{
  omfgThis->handleNewWindow(new taxSelections());
}

void menuAccounting::sTaxRegistrations()
{
  omfgThis->handleNewWindow(new taxRegistrations());
}

void menuAccounting::sDspBankrecHistory()
{
  omfgThis->handleNewWindow(new dspBankrecHistory());
}

void menuAccounting::sDspSummarizedBankrecHistory()
{
  omfgThis->handleNewWindow(new dspSummarizedBankrecHistory());
}

void menuAccounting::sBudgets()
{
  omfgThis->handleNewWindow(new budgets());
}

void menuAccounting::sMaintainBudget()
{
  ParameterList params;
  params.append("mode", "new");

  maintainBudget *newdlg = new maintainBudget();
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sForwardUpdateAccounts()
{
  forwardUpdateAccounts(parent, "", TRUE).exec();
}

// START_RW
//  Utilities
void menuAccounting::sPostTransactionsToExternal()
{
  postGLTransactionsToExternal(parent, "", TRUE).exec();
}

void menuAccounting::sDspRWTransactions()
{
  omfgThis->handleNewWindow(new dspRWTransactions());
}
// END_RW

void menuAccounting::sTerms()
{
  omfgThis->handleNewWindow(new termses());
}

void menuAccounting::sVendors()
{
  omfgThis->handleNewWindow(new vendors());
}

void menuAccounting::sBankAccounts()
{
  omfgThis->handleNewWindow(new bankAccounts());
}

void menuAccounting::sCheckFormats()
{
  omfgThis->handleNewWindow(new checkFormats());
}

void menuAccounting::sAPAssignments()
{
  omfgThis->handleNewWindow(new apAccountAssignments());
}

void menuAccounting::sCostCategories()
{
  omfgThis->handleNewWindow(new costCategories());
}

void menuAccounting::sExpenseCategories()
{
  omfgThis->handleNewWindow(new expenseCategories());
}

void menuAccounting::sPrintStatementByCustomer()
{
  printStatementByCustomer(parent, "", TRUE).exec();
}

void menuAccounting::sPrintStatementsByCustomerType()
{
  printStatementsByCustomerType(parent, "", TRUE).exec();
}

void menuAccounting::sCustomers()
{
  omfgThis->handleNewWindow(new customers());
}

void menuAccounting::sCustomerTypes()
{
  omfgThis->handleNewWindow(new customerTypes());
}

void menuAccounting::sVendorTypes()
{
  omfgThis->handleNewWindow(new vendorTypes());
}

void menuAccounting::sSalesCategories()
{
  omfgThis->handleNewWindow(new salesCategories());
}

void menuAccounting::sReasonCodes()
{
  omfgThis->handleNewWindow(new reasonCodes());
}

void menuAccounting::sARAccountAssignments()
{
  omfgThis->handleNewWindow(new arAccountAssignments());
}

void menuAccounting::sUpdateLateCustCreditStatus()
{
  updateLateCustCreditStatus newdlg(parent, "", TRUE);
  newdlg.exec();
}

void menuAccounting::sCreateRecurringInvoices()
{
  createRecurringInvoices newdlg(parent, "", TRUE);
  newdlg.exec();
}

