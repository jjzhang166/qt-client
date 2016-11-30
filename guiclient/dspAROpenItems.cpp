/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspAROpenItems.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QToolBar>
#include <QToolButton>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

#include <currcluster.h>

#include "applyARCreditMemo.h"
#include "arOpenItem.h"
#include "cashReceipt.h"
#include "creditMemo.h"
#include "creditcardprocessor.h"
#include "customer.h"
#include "distributeInventory.h"
#include "dspInvoiceInformation.h"
#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "getGLDistDate.h"
#include "invoice.h"
#include "incident.h"
#include "printArOpenItem.h"
#include "printCreditMemo.h"
#include "printInvoice.h"
#include "printStatementByCustomer.h"
#include "salesOrder.h"
#include "storedProcErrorLookup.h"

#include "errorReporter.h"

dspAROpenItems::dspAROpenItems(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspAROpenItems", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Open Receivables"));
  setListLabel(tr("Receivables"));
  setReportName("AROpenItems");
  setMetaSQLOptions("arOpenItems", "detail");
  setUseAltId(true);
  setNewVisible(true);

  connect(_customerSelector, SIGNAL(updated()), list(), SLOT(clear()));
  connect(_customerSelector, SIGNAL(updated()), this, SLOT(sHandlePrintGroup()));
  connect(_closed, SIGNAL(toggled(bool)), this, SLOT(sClosedToggled(bool)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  QString baseBalanceTitle(tr("Balance"));
  QString baseAmountTitle(tr("Amount"));
  QString basePaidTitle(tr("Paid"));

  if (! omfgThis->singleCurrency())
  {
    baseBalanceTitle = tr("Balance (in %1)").arg(CurrDisplay::baseCurrAbbr());
    baseAmountTitle  = tr("Amount (in %1)").arg(CurrDisplay::baseCurrAbbr());
    basePaidTitle    = tr("Paid (in %1)").arg(CurrDisplay::baseCurrAbbr());
  }

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Doc. Type"),              -1, Qt::AlignLeft,   true,  "doctype");
  list()->addColumn(tr("Posted"),          _ynColumn, Qt::AlignCenter, true,  "posted");
  list()->addColumn(tr("Recurring"),       _ynColumn, Qt::AlignCenter, false, "recurring");
  list()->addColumn(tr("Open"),            _ynColumn, Qt::AlignCenter, false, "open");
  list()->addColumn(tr("Doc. #"),       _orderColumn, Qt::AlignLeft,   true,  "docnumber");
  list()->addColumn(tr("Cust./Assign To"),_itemColumn, Qt::AlignLeft,  false, "cust_number");
  list()->addColumn(tr("Name/Desc."),             -1, Qt::AlignLeft,   true,  "cust_name");
  list()->addColumn(tr("Order/Incident"),_itemColumn, Qt::AlignRight,  false, "ordernumber");
  list()->addColumn(tr("Doc. Date"),     _dateColumn, Qt::AlignCenter, true,  "docdate");
  list()->addColumn(tr("Due Date"),      _dateColumn, Qt::AlignCenter, true,  "aropen_duedate");
  list()->addColumn(tr("Amount"),       _moneyColumn, Qt::AlignRight,  true,  "amount");
  list()->addColumn(baseAmountTitle,    _moneyColumn, Qt::AlignRight,  false,  "base_amount");
  list()->addColumn(tr("Paid"),         _moneyColumn, Qt::AlignRight,  true,  "paid");
  list()->addColumn(basePaidTitle,      _moneyColumn, Qt::AlignRight,  false,  "base_paid");
  list()->addColumn(tr("Balance"),      _moneyColumn, Qt::AlignRight,  !omfgThis->singleCurrency(),  "balance");
  list()->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,   !omfgThis->singleCurrency(),  "currAbbr");
  list()->addColumn(baseBalanceTitle,   _moneyColumn, Qt::AlignRight,  true,  "base_balance");
  list()->addColumn(tr("Credit Card"),            -1, Qt::AlignLeft,   false, "ccard_number");
  list()->addColumn(tr("Notes"),                  -1, Qt::AlignLeft,   false, "notes");
  
  connect(omfgThis, SIGNAL(creditMemosUpdated()), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(invoicesUpdated(int, bool)), this, SLOT(sFillList()));

  disconnect(newAction(), SIGNAL(triggered()), this, SLOT(sNew()));
  connect(newAction(), SIGNAL(triggered()), this, SLOT(sCreateInvoice()));

  QToolButton * newBtn = (QToolButton*)toolBar()->widgetForAction(newAction());
  newBtn->setPopupMode(QToolButton::MenuButtonPopup);
  QAction *menuItem;
  QMenu * newMenu = new QMenu;
  menuItem = newMenu->addAction(tr("Invoice"), this, SLOT(sCreateInvoice()));
  menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));
  menuItem = newMenu->addAction(tr("Misc. Debit Memo"),   this, SLOT(sEnterMiscArDebitMemo()));
  menuItem->setEnabled(_privileges->check("MaintainARMemos"));
  newMenu->addSeparator();
  menuItem = newMenu->addAction(tr("Sales Credit"), this, SLOT(sNewCreditMemo()));
  menuItem->setEnabled(_privileges->check("MaintainCreditMemos"));
  menuItem = newMenu->addAction(tr("Misc. Credit Memo"),   this, SLOT(sEnterMiscArCreditMemo()));
  menuItem->setEnabled(_privileges->check("MaintainARMemos"));
  newBtn->setMenu(newMenu);

  
  _asOf->setDate(omfgThis->dbDate(), true);
  _closed->hide();
}

enum SetResponse dspAROpenItems::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  if (pParams.inList("byDueDate"))
    _dueDate->setChecked(true);

  param = pParams.value("cust_id", &valid);
  if (valid)
    _customerSelector->setCustId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());
    
  param = pParams.value("asofDate", &valid);
  if (valid)
  {
    _asOf->setDate(param.toDate());
    _asOf->setEnabled(false);
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspAROpenItems::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem, int)
{
  QAction *menuItem = 0;

  if ( _privileges->check("MaintainCustomerMasters") || _privileges->check("ViewCustomerMasters") )
    menuItem = pMenu->addAction(tr("Open Customer..."), this, SLOT(sOpenCustomer()));

  if (((XTreeWidgetItem *)pItem)->altId() < 4)
  {
    menuItem = pMenu->addAction(tr("Print..."), this, SLOT(sPrintItem()));
    if (((XTreeWidgetItem *)pItem)->altId() == 0)
    // Invoice
      menuItem->setEnabled(_privileges->check("ViewMiscInvoices") || _privileges->check("MaintainMiscInvoices"));
    else if (((XTreeWidgetItem *)pItem)->altId() == 1 && ((XTreeWidgetItem *)pItem)->id("docnumber") > -1)
    // Credit Memo
     menuItem->setEnabled(_privileges->check("ViewCreditMemos") || _privileges->check("MaintainCreditMemos"));
    else
    // Open Item
      menuItem->setEnabled(_privileges->check("ViewAROpenItems") || _privileges->check("EditAROpenItem"));
  }
      
  pMenu->addSeparator();
  if (((XTreeWidgetItem *)pItem)->altId() == 0 && ((XTreeWidgetItem *)pItem)->rawValue("posted") == 0)
  // Invoice
  {
    menuItem = pMenu->addAction(tr("Edit Invoice..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));
  }
  else if (((XTreeWidgetItem *)pItem)->altId() == 1 && ((XTreeWidgetItem *)pItem)->id("docnumber") > -1)
  // Credit Memo
  {
    menuItem = pMenu->addAction(tr("Edit Sales Credit..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainCreditMemos"));
  }
  else if (((XTreeWidgetItem *)pItem)->id() > 0)
  // Open Item
  {
    menuItem = pMenu->addAction(tr("Edit Receivable Item..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("EditAROpenItem"));
  }
  else
  // Incident
  {
    menuItem = pMenu->addAction(tr("Edit Incident..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainAllIncidents"));
  }

  if (((XTreeWidgetItem *)pItem)->id() > 0)
  // Open Item
  {
    menuItem = pMenu->addAction(tr("View Receivable Item..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("EditAROpenItem") || _privileges->check("ViewAROpenItems"));
  }
  
  if (((XTreeWidgetItem *)pItem)->altId() == 0)
  // Invoice
  {
    if(((XTreeWidgetItem *)pItem)->rawValue("posted") != 0)
    {
      menuItem = pMenu->addAction(tr("Void Posted Invoice..."), this, SLOT(sVoidInvoiceDetails()));
      menuItem->setEnabled(_privileges->check("VoidPostedInvoices"));

      menuItem = pMenu->addAction(tr("Edit Posted Invoice..."), this, SLOT(sEditInvoiceDetails()));
      menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));
    }

    menuItem = pMenu->addAction(tr("View Invoice..."), this, SLOT(sViewInvoiceDetails()));
    menuItem->setEnabled(_privileges->check("ViewMiscInvoices"));
  
    menuItem = pMenu->addAction(tr("View Invoice Information..."), this, SLOT(sViewInvoice()));
    menuItem->setEnabled(_privileges->check("ViewMiscInvoices"));
  }
  else if (((XTreeWidgetItem *)pItem)->altId() == 1 &&
           ((XTreeWidgetItem *)pItem)->id("docnumber") > -1)
  // Credit Memo
  {
    if(((XTreeWidgetItem *)pItem)->rawValue("posted") != 0)
    {
      menuItem = pMenu->addAction(tr("Void Posted Sales Credit..."), this, SLOT(sVoidCreditMemo()));
      menuItem->setEnabled(_privileges->check("VoidPostedARCreditMemos"));
    }

    menuItem = pMenu->addAction(tr("View Sales Credit..."), this, SLOT(sViewCreditMemo()));
    menuItem->setEnabled(_privileges->check("MaintainCreditMemos") || _privileges->check("ViewCreditMemos"));
  }
  else if (((XTreeWidgetItem *)pItem)->altId() == 4)
  // Incident
  {
    menuItem = pMenu->addAction(tr("View Incident..."), this, SLOT(sViewIncident()));
    menuItem->setEnabled(_privileges->check("ViewAllIncidents") || _privileges->check("MaintainAllIncidents"));
  }
  
  if (((XTreeWidgetItem *)pItem)->altId() < 2 &&
      ((XTreeWidgetItem *)pItem)->id() == -1 &&
      !((XTreeWidgetItem *)pItem)->rawValue("posted").toBool())
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("Post..."), this, SLOT(sPost()));
    menuItem->setEnabled(_privileges->check("PostARDocuments"));
    
    if (((XTreeWidgetItem *)pItem)->altId() == 0)
    {
      menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDeleteInvoice()));
      menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));
    }
    else
    {
      menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDeleteCreditMemo()));
      menuItem->setEnabled(_privileges->check("MaintainCreditMemos"));
    }
  }
  
  if ((((XTreeWidgetItem *)pItem)->altId() == 1 || ((XTreeWidgetItem *)pItem)->altId() == 3) && 
      ((XTreeWidgetItem *)pItem)->rawValue("posted").toBool() && 
      ((XTreeWidgetItem *)pItem)->rawValue("open").toBool() )
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("Apply Credit Memo..."), this, SLOT(sApplyAropenCM()));
    menuItem->setEnabled(_privileges->check("ApplyARMemos"));
  }

  if ((((XTreeWidgetItem *)pItem)->id("ordernumber") > 0 && 
      ((XTreeWidgetItem *)pItem)->altId() == 0) )
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("Edit Sales Order..."), this, SLOT(sEditSalesOrder()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") || _privileges->check("ViewSalesOrders"));
    menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sViewSalesOrder()));
    menuItem->setEnabled(_privileges->check("ViewSalesOrders"));
    menuItem = pMenu->addAction(tr("Shipment Status..."), this, SLOT(sDspShipmentStatus()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") || _privileges->check("ViewSalesOrders"));
    menuItem = pMenu->addAction(tr("Shipments..."), this, SLOT(sShipment()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") || _privileges->check("ViewSalesOrders"));
  }
  
  
  if ((((XTreeWidgetItem *)pItem)->altId() == 0 || 
      ((XTreeWidgetItem *)pItem)->altId() == 2) && 
      ((XTreeWidgetItem *)pItem)->rawValue("posted").toBool() && 
      ((XTreeWidgetItem *)pItem)->rawValue("open").toBool() )
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("New Cash Receipt..."), this, SLOT(sNewCashrcpt()));
    menuItem->setEnabled(_privileges->check("MaintainCashReceipts"));
  }

  if ((_metrics->boolean("CCAccept") &&
       ((XTreeWidgetItem *)pItem)->id("ccard_number") > 0) )
  {
    menuItem = pMenu->addAction(tr("Refund"), this, SLOT(sCCRefundCM()));
    menuItem->setEnabled( _privileges->check("ProcessCreditCards") &&
                          ((XTreeWidgetItem *)pItem)->rawValue("balance").toDouble() < 0);
  }

  if (((XTreeWidgetItem *)pItem)->id() > -1 && 
     ((XTreeWidgetItem *)pItem)->rawValue("posted").toBool() && 
     ((XTreeWidgetItem *)pItem)->rawValue("open").toBool() )
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("New Incident..."), this, SLOT(sIncident()));
    if (!_privileges->check("MaintainAllIncidents"))
      menuItem->setEnabled(false);
  }

}

void dspAROpenItems::sOpenCustomer()
{
  XSqlQuery customerq;
  ParameterList params;
  if ( _privileges->check("MaintainCustomerMasters") )
    params.append("mode", "edit");
  else
    params.append("mode", "view");
  
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());
  else if (list()->id() > -1)
  {
    customerq.prepare("SELECT aropen_cust_id FROM aropen WHERE aropen_id=:aropen_id;");
    customerq.bindValue(":aropen_id", list()->id());
    customerq.exec();
    if (customerq.first())
      params.append("cust_id", customerq.value("aropen_cust_id").toInt());
  }
  else if (list()->id() == -1 && list()->altId() == 0)
  {
    customerq.prepare("SELECT invchead_cust_id FROM invchead WHERE invchead_id=:invchead_id;");
    customerq.bindValue(":invchead_id", list()->currentItem()->id("docnumber"));
    customerq.exec();
    if (customerq.first())
      params.append("cust_id", customerq.value("invchead_cust_id").toInt());
  }
  else if (list()->id() == -1 && list()->altId() == 1)
  {
    customerq.prepare("SELECT cmhead_cust_id FROM cmhead WHERE cmhead_id=:cmhead_id;");
    customerq.bindValue(":cmhead_id", list()->currentItem()->id("docnumber"));
    customerq.exec();
    if (customerq.first())
      params.append("cust_id", customerq.value("cmhead_cust_id").toInt());
  }
  else
    return;

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sApplyAropenCM()
{
  ParameterList params;
  params.append("aropen_id", list()->id());

  applyARCreditMemo newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItems::sCCRefundCM()
{
  XSqlQuery dspCCRefundCM;
  if (list()->id("ccard_number") < 0)
  {
    QMessageBox::warning(this, tr("Cannot Refund by Credit Card"),
			 tr("<p>The application cannot refund this "
			    "transaction using a credit card."));
    return;
  }
  
  int     ccardid = -1;
  double  total   =  0.0;
  double  tax     =  0.0;
  double  freight =  0.0;
  double  duty    =  0.0;
  int     currid  = -1;
  bool    taxexempt = false;
  QString docnum;
  QString refnum;
  int     ccpayid = list()->currentItem()->id("ccard_number");

  dspCCRefundCM.prepare("SELECT ccpay_ccard_id, aropen_amount - aropen_paid AS balance, "
	    "       aropen_curr_id, aropen_docnumber "
            "FROM aropen "
            "     JOIN payaropen ON (aropen_id=payaropen_aropen_id) "
            "     JOIN ccpay ON (payaropen_ccpay_id=ccpay_id) "
            "WHERE ((aropen_id=:aropen_id)"
            "  AND  (ccpay_id=:ccpay_id));");
  dspCCRefundCM.bindValue(":aropen_id", list()->id());
  dspCCRefundCM.bindValue(":ccpay_id",  ccpayid);
  dspCCRefundCM.exec();
  if (dspCCRefundCM.first())
  {
    ccardid = dspCCRefundCM.value("ccpay_ccard_id").toInt();
    total   = dspCCRefundCM.value("balance").toDouble();
    currid  = dspCCRefundCM.value("aropen_curr_id").toInt();
    docnum  = dspCCRefundCM.value("aropen_docnumber").toString();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving AR Information"),
                                dspCCRefundCM, __FILE__, __LINE__))
  {
    return;
  }
  else
  {
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          tr("Could not find a Credit Card to use for "
                             "this Credit transaction."));
    return;
  }

  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (! cardproc)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
			  CreditCardProcessor::errorMsg());
  else
  {
    int refid = list()->id();
    int returnVal = cardproc->credit(ccardid, "-2", total, tax, taxexempt,
				     freight, duty, currid,
				     docnum, refnum, ccpayid, "aropen", refid);
    if (returnVal < 0)
      QMessageBox::critical(this, tr("Credit Card Processing Error"),
			    cardproc->errorMsg());
    else if (returnVal > 0)
      QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			   cardproc->errorMsg());
    else if (! cardproc->errorMsg().isEmpty())
      QMessageBox::information(this, tr("Credit Card Processing Note"),
			   cardproc->errorMsg());
  }

  sFillList();
}

void dspAROpenItems::sDeleteCreditMemo()
{
  if (QMessageBox::question(this, tr("Delete Selected Credit Memos?"),
                            tr("<p>Are you sure that you want to delete the "
			       "selected Credit Memos?"),
                            QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare("SELECT deleteCreditMemo(:cmhead_id) AS result;");

    if (checkCreditMemoSitePrivs(list()->currentItem()->id("docnumber")))
    {
      delq.bindValue(":cmhead_id", (list()->currentItem()->id("docnumber")));
      delq.exec();
      if (delq.first())
      {
            if (! delq.value("result").toBool())
              ErrorReporter::error(QtCriticalMsg, this,
                                 tr("Error Deleting Credit Memo %1\n").arg(list()->currentItem()->text("docnumber")),
                                 delq, __FILE__, __LINE__);
      }
      else if (delq.lastError().type() != QSqlError::NoError)
        ErrorReporter::error(QtCriticalMsg,
                         this, tr("Error Deleting Credit Memo %1\n").arg(list()->currentItem()->text("docnumber")),
                         delq, __FILE__, __LINE__);
    }

    omfgThis->sCreditMemosUpdated();
  }
}

void dspAROpenItems::sDeleteInvoice()
{
  XSqlQuery dspDeleteInvoice;
  if ( QMessageBox::warning( this, tr("Delete Selected Invoices"),
                             tr("<p>Are you sure that you want to delete the "
			        "selected Invoices?"),
                             tr("Delete"), tr("Cancel"), QString::null, 1, 1 ) == 0)
  {
    dspDeleteInvoice.prepare("SELECT deleteInvoice(:invchead_id) AS result;");

    if (checkInvoiceSitePrivs(list()->currentItem()->id("docnumber")))
    {
      dspDeleteInvoice.bindValue(":invchead_id", list()->currentItem()->id("docnumber"));
      dspDeleteInvoice.exec();
      if (dspDeleteInvoice.first())
      {
            int result = dspDeleteInvoice.value("result").toInt();
            if (result < 0)
            {
              ErrorReporter::error(QtCriticalMsg, this,
                                   tr("Error Deleting Invoice %1\n").arg(list()->currentItem()->text("docnumber")),
                                   storedProcErrorLookup("deleteInvoice", result),
                                   __FILE__, __LINE__);
            }
      }
      else
          ErrorReporter::error(QtCriticalMsg, this,
                               tr("Error Deleting Invoice %1\n").arg(list()->currentItem()->text("docnumber")),
                               dspDeleteInvoice, __FILE__, __LINE__);
    }

    omfgThis->sInvoicesUpdated(-1, true);
    omfgThis->sBillingSelectionUpdated(-1, -1);
  }
}

void dspAROpenItems::sDspShipmentStatus()
{
  if (checkSalesOrderSitePrivs(list()->currentItem()->id("ordernumber")))
  {
    ParameterList params;
    params.append("sohead_id", list()->currentItem()->id("ordernumber"));
    params.append("run");

    dspSalesOrderStatus *newdlg = new dspSalesOrderStatus(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAROpenItems::sEdit()
{
  ParameterList params;
    
  if (list()->altId() == 0 && list()->id() == -1)
  // Edit Unposted Invoice
  {
    if (checkInvoiceSitePrivs(list()->currentItem()->id("docnumber")))
    {
      params.append("invchead_id", list()->currentItem()->id("docnumber"));
      params.append("mode", "edit");
      invoice* newdlg = new invoice(this);
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      return;
    }
    return;
  }
  else if (list()->altId() == 1 && list()->currentItem()->id("docnumber") > -1 && list()->id() -1)
  // Edit Unposted Credit Memo
  {
    if (checkCreditMemoSitePrivs(list()->currentItem()->id("docnumber")))
    {
      params.append("cmhead_id", list()->currentItem()->id("docnumber"));
      params.append("mode", "edit");
      creditMemo* newdlg = new creditMemo(this);
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      return;
    }
    return;
  }
  else if (list()->id() > 0)
  // Edit AR Open Item
  {
    params.append("mode", "edit");
    params.append("aropen_id", list()->id());
    arOpenItem newdlg(this, "", true);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
  else
  // Edit Incident
  {
    params.append("mode", "edit");
    params.append("incdt_id", list()->currentItem()->id("ordernumber"));
    incident newdlg(this, "", true);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void dspAROpenItems::sEditSalesOrder()
{
  if (checkSalesOrderSitePrivs(list()->currentItem()->id("ordernumber")))
    salesOrder::editSalesOrder(list()->currentItem()->id("ordernumber"), false);
}

void dspAROpenItems::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", list()->id());
  arOpenItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItems::sViewSalesOrder()
{
  if (checkSalesOrderSitePrivs(list()->currentItem()->id("ordernumber")))
    salesOrder::viewSalesOrder(list()->currentItem()->id("ordernumber"));
}

void dspAROpenItems::sViewCreditMemo()
{
  ParameterList params;
  params.append("cmhead_id", list()->currentItem()->id("docnumber"));
  params.append("mode", "view");
  creditMemo* newdlg = new creditMemo(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sVoidCreditMemo()
{
  XSqlQuery dspVoidCreditMemo;
  XTreeWidgetItem *pItem = list()->currentItem();
  if(pItem->rawValue("posted") != 0 &&
      QMessageBox::question(this, tr("Void Posted Credit Memo?"),
                            tr("<p>This Credit Memo has already been posted. "
                               "Are you sure you want to void it?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
  {
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery post;
  post.prepare("SELECT voidCreditMemo(:cmhead_id) AS result;");

  dspVoidCreditMemo.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  post.bindValue(":cmhead_id", list()->currentItem()->id("docnumber"));
  post.exec();
  if (post.first())
  {
    int result = post.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this,
                              tr("Error Voiding Credit Memo %1\n").arg(list()->currentItem()->text("docnumber")),
                               storedProcErrorLookup("voidCreditMemo", result),
                               __FILE__, __LINE__);
      return;
    }
    else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Void Memo"), tr("Transaction Canceled") );
      return;
    }

    dspVoidCreditMemo.exec("COMMIT;");
    sFillList();
  }
  else if (post.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg,this,
                         tr("Error Voiding Credit Memo %1\n").arg(list()->currentItem()->text("docnumber")),
                         post, __FILE__, __LINE__);
  }
}

void dspAROpenItems::sEnterMiscArCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());

  arOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void dspAROpenItems::sEnterMiscArDebitMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "debitMemo");
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());

  arOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void dspAROpenItems::sCreateInvoice()
{
  invoice::newInvoice(_customerSelector->custId());
}

void dspAROpenItems::sNewCashrcpt()
{
  XSqlQuery dspNewCashrcpt;
  ParameterList params;
  params.append("mode", "new");
  if (list()->id() > -1)
  {
    dspNewCashrcpt.prepare("SELECT aropen_cust_id FROM aropen WHERE aropen_id=:aropen_id;");
    dspNewCashrcpt.bindValue(":aropen_id", list()->id());
    dspNewCashrcpt.exec();
    if (dspNewCashrcpt.first())
    {
      params.append("cust_id", dspNewCashrcpt.value("aropen_cust_id").toInt());
      params.append("docnumber", list()->currentItem()->text("docnumber"));
    }
  }
  else
  {
    if (_customerSelector->isSelectedCust())
      params.append("cust_id", _customerSelector->custId());
  }

  cashReceipt *newdlg = new cashReceipt(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sNewCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());

  creditMemo *newdlg = new creditMemo(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sViewInvoice()
{
  ParameterList params;
  params.append("invoiceNumber", list()->currentItem()->text("docnumber"));
  dspInvoiceInformation* newdlg = new dspInvoiceInformation(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sEditInvoiceDetails()
{
  XTreeWidgetItem *pItem = list()->currentItem();
  if(pItem->rawValue("posted") != 0 &&
      QMessageBox::question(this, tr("Edit Posted Invoice?"),
                            tr("<p>This Invoice has already been posted. "
                               "Are you sure you want to edit it?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
  {
    return;
  }

  ParameterList params;
  params.append("invchead_id", list()->currentItem()->id("docnumber"));
  params.append("mode", "edit");
  invoice* newdlg = new invoice(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sVoidInvoiceDetails()
{
  XSqlQuery dspVoidInvoiceDetails;
  XTreeWidgetItem *pItem = list()->currentItem();
  if(pItem->rawValue("posted") != 0 &&
      QMessageBox::question(this, tr("Void Posted Invoice?"),
                            tr("<p>This Invoice has already been posted. "
                               "Are you sure you want to void it?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
  {
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery post;
  post.prepare("SELECT voidInvoice(:invchead_id) AS result;");

  dspVoidInvoiceDetails.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  post.bindValue(":invchead_id", list()->currentItem()->id("docnumber"));
  post.exec();
  if (post.first())
  {
    int result = post.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error Voiding Invoice %1\n").arg(list()->currentItem()->text("docnumber")),
                           storedProcErrorLookup("voidInvoice", result),
                           __FILE__, __LINE__);
      return;
    }
    else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Void Invoice"), tr("Transaction Canceled") );
      return;
    }

    dspVoidInvoiceDetails.exec("COMMIT;");
    sFillList();
  }
  else if (post.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this,
                         tr("Error Voiding Invoice %1\n").arg(list()->currentItem()->text("docnumber")),
                         post, __FILE__, __LINE__);
  }
}

void dspAROpenItems::sViewInvoiceDetails()
{
  ParameterList params;
  params.append("invchead_id", list()->currentItem()->id("docnumber"));
  params.append("mode", "view");
  invoice* newdlg = new invoice(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sIncident()
{
  XSqlQuery dspIncident;
  dspIncident.prepare("SELECT crmacct_id, crmacct_cntct_id_1 "
            "FROM crmacct, aropen "
            "WHERE ((aropen_id=:aropen_id) "
            "AND (crmacct_cust_id=aropen_cust_id));");
  dspIncident.bindValue(":aropen_id", list()->id());
  dspIncident.exec();
  if (dspIncident.first())
  {
    ParameterList params;
    params.append("mode", "new");
    params.append("aropen_id", list()->id());
    params.append("crmacct_id", dspIncident.value("crmacct_id"));
    params.append("cntct_id", dspIncident.value("crmacct_cntct_id_1"));
    incident newdlg(this, 0, true);
    newdlg.set(params);

    if (newdlg.exec() == XDialog::Accepted)
      sFillList();
  }
}

void dspAROpenItems::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", list()->currentItem()->id("ordernumber"));
  incident newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

bool dspAROpenItems::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  _customerSelector->appendValue(params);
  if (_docDate->isChecked())
    _dates->appendValue(params);
  else
  {
    params.append("startDueDate", _dates->startDate());
    params.append("endDueDate", _dates->endDate());
  }
  params.append("invoice", tr("Invoice"));
  params.append("return", tr("Sales Credit"));
  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("cashdeposit", tr("Customer Deposit"));
  params.append("asofDate", _asOf->date());
  if (_incidentsOnly->isChecked())
    params.append("incidentsOnly");
  if (_debits->isChecked())
    params.append("debitsOnly");
  else if (_credits->isChecked())
    params.append("creditsOnly");
  if (_unposted->isChecked())
    params.append("showUnposted");
  if (_closed->isChecked())
    params.append("showClosed");

  params.append("key", omfgThis->_key);
  return true;
}

void dspAROpenItems::sPreview()
{
  if (_printStatement->isChecked())
  {
    MetaSQLQuery mql("SELECT findCustomerForm(cust_id, 'S') AS reportname"
                     "  FROM custinfo"
                     " WHERE (cust_id=<? value('cust_id') ?>);");
    ParameterList params;
    params.append("cust_id", _customerSelector->custId());
    params.append("invoice",  tr("Invoice"));
    params.append("return",   tr("Sales Credit"));
    params.append("debit",    tr("Debit Memo"));
    params.append("credit",   tr("Credit Memo"));
    params.append("deposit",  tr("Deposit"));
    params.append("asofdate", _asOf->date());
    XSqlQuery qry = mql.toQuery(params);
    if (qry.first())
      setReportName(qry.value("reportname").toString());
    display::sPreview(params);
  }
  else
  {
    setReportName("AROpenItems");
    display::sPreview();
  }
}

void dspAROpenItems::sPrint()
{
  if (_printStatement->isChecked())
  {
    printStatementByCustomer newdlg(this, "", true);
    ParameterList params;
    params.append("cust_id", _customerSelector->custId());
    params.append("asofDate", _asOf->date());
    params.append("print");
    newdlg.set(params);
  }
  else
  {
    ParameterList params;
    if (! setParams(params))
      return;
    params.append("includeFormatted");

    orReport report(reportName(), params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
}

void dspAROpenItems::sPrintItem()
{
  ParameterList params;

  if (list()->altId() == 0)
  {
    // Print Invoice
    printInvoice newdlg(this, "", true);
    
    if (checkInvoiceSitePrivs(list()->currentItem()->id("docnumber")))
    {
      params.append("invchead_id", list()->currentItem()->id("docnumber"));
      params.append("persistentPrint");

      newdlg.set(params);
      
      if (!newdlg.isSetup())
      {
        if(newdlg.exec() == QDialog::Rejected)
          return;
        newdlg.setSetup(true);
      }

      omfgThis->sInvoicesUpdated(-1, true);
      return;
    }
    else
      return;
  }
  else if (list()->altId() == 1 && list()->currentItem()->id("docnumber") > -1)
  {
    // Print Credit Memo
    if (checkCreditMemoSitePrivs(list()->currentItem()->id("docnumber")))
    {
      params.append("cmhead_id", list()->currentItem()->id("docnumber"));
      params.append("persistentPrint");

      printCreditMemo newdlg(this, "", true);
      newdlg.set(params);

      if (!newdlg.isSetup())
      {
        if(newdlg.exec() == QDialog::Rejected)
          return;
        newdlg.setSetup(true);
      }
      omfgThis->sCreditMemosUpdated();
      return;
    }
    else
      return;
  }
  else if (list()->id() > 0)
  // Print AR Open Item
  {
    params.append("aropen_id", list()->id());

    printArOpenItem newdlg(this, "", true);
    if (newdlg.set(params) == NoError)
      newdlg.exec();
  }
}

void dspAROpenItems::sPost()
{
  if (list()->altId() == 1 && list()->id() == -1)
    sPostCreditMemo();
  else if (list()->altId() == 0 || list()->id() == -1)
    sPostInvoice();
}

void dspAROpenItems::sPostCreditMemo()
{
  XSqlQuery dspPostCreditMemo;
  if (list()->altId() != 1 || list()->id() > 0)
    return;

  int id = list()->currentItem()->id("docnumber");

  if (!checkCreditMemoSitePrivs(id))
    return;
    
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (_privileges->check("ChangeSOMemoPostDate"))
  {
    getGLDistDate newdlg(this, "", true);
    newdlg.sSetDefaultLit(tr("Credit Memo Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  XSqlQuery setDate;
  setDate.prepare("UPDATE cmhead SET cmhead_gldistdate=:distdate "
		  "WHERE cmhead_id=:cmhead_id;");

  if (changeDate)
  {
    setDate.bindValue(":distdate",  newDate);
    setDate.bindValue(":cmhead_id", id);
    setDate.exec();
    if (setDate.lastError().type() != QSqlError::NoError)
    {
          ErrorReporter::error(QtCriticalMsg, this,
                               tr("Error Posting Credit Memo %1\n").arg(list()->currentItem()->text("docnumber")),
                               setDate, __FILE__, __LINE__);
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");
    
  XSqlQuery postq;
  postq.prepare("SELECT postCreditMemo(:cmhead_id, 0) AS result;");
 
  XSqlQuery tx;
  tx.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations

  postq.bindValue(":cmhead_id", id);
  postq.exec();
  if (postq.first())
  {
    int result = postq.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error Posting Credit Memo %1\n").arg(list()->currentItem()->text("docnumber")),
                           storedProcErrorLookup("postCreditMemo", result),
                           __FILE__, __LINE__);
      return;
    }
    else
    {
      if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Post Credit Memo"), tr("Transaction Canceled") );
        return;
      }

      dspPostCreditMemo.exec("COMMIT;");
    }
  }
  // contains() string is hard-coded in stored procedure
  else if (postq.lastError().databaseText().contains("post to closed period"))
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this,
                         tr("Error Posting Credit Memo %1\n").arg(list()->currentItem()->text("docnumber")),
                         postq, __FILE__, __LINE__);
    return;
  }
  else if (postq.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this,
                         tr("Error Posting Credit Memo#%1.\n%2")
                                     .arg(list()->currentItem()->text("docnumber"))
                                     .arg(postq.lastError().databaseText()),
                         postq, __FILE__, __LINE__);
  }

  omfgThis->sCreditMemosUpdated();
}

void dspAROpenItems::sPostInvoice()
{
  XSqlQuery dspPostInvoice;
  if (list()->altId() != 0 || list()->id() > 0)
    return;
    
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (!checkInvoiceSitePrivs(list()->currentItem()->id("docnumber")))
    return;

  if (_privileges->check("ChangeARInvcDistDate"))
  {
    getGLDistDate newdlg(this, "", true);
    newdlg.sSetDefaultLit(tr("Invoice Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  int journal = -1;
  dspPostInvoice.exec("SELECT fetchJournalNumber('AR-IN') AS result;");
  if (dspPostInvoice.first())
  {
    journal = dspPostInvoice.value("result").toInt();
    if (journal < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error Posting Invoice#%1.\n%2")
                                       .arg(list()->currentItem()->text("docnumber"))
                                       .arg(dspPostInvoice.lastError().databaseText()),
                             storedProcErrorLookup("fetchJournalNumber", journal),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (dspPostInvoice.lastError().type() != QSqlError::NoError)
  {
    ErrorReporter::error(QtCriticalMsg, this,
                         tr("Error Posting Invoice#%1.\n%2")
                                     .arg(list()->currentItem()->text("docnumber"))
                                     .arg(dspPostInvoice.lastError().databaseText()),
                         dspPostInvoice, __FILE__, __LINE__);
    return;
  }

  XSqlQuery xrate;
  xrate.prepare("SELECT curr_rate "
		"FROM curr_rate, invchead "
		"WHERE ((curr_id=invchead_curr_id)"
		"  AND  (invchead_id=:invchead_id)"
		"  AND  (invchead_invcdate BETWEEN curr_effective AND curr_expires));");
  // if SUM becomes dependent on curr_id then move XRATE before it in the loop
  XSqlQuery sum;
  sum.prepare("SELECT invoicetotal(:invchead_id) AS subtotal;");

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery post;
  post.prepare("SELECT postInvoice(:invchead_id, :journal) AS result;");

  XSqlQuery setDate;
  setDate.prepare("UPDATE invchead SET invchead_gldistdate=:distdate "
		  "WHERE invchead_id=:invchead_id;");

  if (changeDate)
  {
    setDate.bindValue(":distdate",    newDate);
    setDate.bindValue(":invchead_id", list()->currentItem()->id("docnumber"));
    setDate.exec();
    if (setDate.lastError().type() != QSqlError::NoError)
      ErrorReporter::error(QtCriticalMsg, this,
                         tr("Error Posting Invoice#%1.\n%2")
                                       .arg(list()->currentItem()->text("docnumber"))
                                       .arg(setDate.lastError().databaseText()),
                         setDate, __FILE__, __LINE__);
  }

  sum.bindValue(":invchead_id", list()->currentItem()->id("docnumber"));
  sum.exec();
  if (sum.first())
  {
    if (sum.value("subtotal").toDouble() == 0)
    {
      if (QMessageBox::question(this, tr("Invoice Has Value 0"),
                          tr("Invoice #%1 has a total value of 0.\n"
                             "Would you like to post it anyway?")
                            .arg(list()->currentItem()->text("docnumber")),
                          QMessageBox::Yes,
                          QMessageBox::No | QMessageBox::Default)
      == QMessageBox::No)
        return;
    }
    else
    {
      xrate.bindValue(":invchead_id", list()->currentItem()->id("docnumber"));
      xrate.exec();
      if (xrate.lastError().type() != QSqlError::NoError)
      {
        ErrorReporter::error(QtCriticalMsg, this,
                             tr("Error Posting Invoice#%1.\n%2")
                                         .arg(list()->currentItem()->text("docnumber"))
                                         .arg(xrate.lastError().databaseText()),
                             xrate, __FILE__, __LINE__);
        return;
      }
      else if (!xrate.first() || xrate.value("curr_rate").isNull())
      {
        ErrorReporter::error(QtCriticalMsg, this,
                             tr("Error Posting Invoice#%1.\n%2")
                                         .arg(list()->currentItem()->text("docnumber"))
                                         .arg(xrate.lastError().databaseText()),
                             xrate, __FILE__, __LINE__);
        return;
      }
    }
  }
  else if (sum.lastError().type() != QSqlError::NoError)
  {
    ErrorReporter::error(QtCriticalMsg, this,
                         tr("Error Posting Invoice#%1.\n%2")
                                     .arg(list()->currentItem()->text("docnumber"))
                                     .arg(sum.lastError().databaseText()),
                         sum, __FILE__, __LINE__);
    return;
  }

  XSqlQuery tx;
  tx.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations

  post.bindValue(":invchead_id", list()->currentItem()->id("docnumber"));
  post.bindValue(":journal",     journal);
  post.exec();
  if (post.first())
  {
    int result = post.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error Posting Invoice#%1.\n%2")
                                       .arg(list()->currentItem()->text("docnumber"))
                                       .arg(post.lastError().databaseText()),
                           storedProcErrorLookup("postInvoice", result),
                           __FILE__, __LINE__);
    }
    else
    {
      if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Post Invoice"), tr("Transaction Canceled") );
        return;
      }

      dspPostInvoice.exec("COMMIT;");
    }
  }
  // contains() string is hard-coded in stored procedure
  else if (post.lastError().databaseText().contains("post to closed period"))
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this,
                         tr("Error Posting Invoice#%1.\n%2")
                                     .arg(list()->currentItem()->text("docnumber"))
                                     .arg(post.lastError().databaseText()),
                         post, __FILE__, __LINE__);
  }
  else if (post.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this,
                         tr("Error Posting Invoice#%1.\n%2")
                                     .arg(list()->currentItem()->text("docnumber"))
                                     .arg(post.lastError().databaseText()),
                         post, __FILE__, __LINE__);
  }

  omfgThis->sInvoicesUpdated(-1, true);
}

void dspAROpenItems::sShipment()
{
  if (checkSalesOrderSitePrivs(list()->currentItem()->id("ordernumber")))
  {
    ParameterList params;
    params.append("sohead_id", list()->currentItem()->id("ordernumber"));

    dspShipmentsBySalesOrder* newdlg = new dspShipmentsBySalesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAROpenItems::sHandlePrintGroup()
{
  _printGroup->setEnabled(_customerSelector->isValid() &&
                          _customerSelector->isSelectedCust());
}

bool dspAROpenItems::checkInvoiceSitePrivs(int invcid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkInvoiceSitePrivs(:invcheadid) AS result;");
    check.bindValue(":invcheadid", invcid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Invoice as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

bool dspAROpenItems::checkCreditMemoSitePrivs(int cmid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkCreditMemoSitePrivs(:cmheadid) AS result;");
    check.bindValue(":cmheadid", cmid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Credit Memo as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

bool dspAROpenItems::checkSalesOrderSitePrivs(int soid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkSOSitePrivs(:coheadid) AS result;");
    check.bindValue(":coheadid", soid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Sales Order as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

void dspAROpenItems::sClosedToggled(bool checked)
{
  if (checked)
    _dates->setStartDate(QDate().currentDate().addDays(-90));
  else
    _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
}

