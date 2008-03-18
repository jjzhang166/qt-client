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

#include "dspCustomerInformation.h"

#include <QDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QWorkspace>

#include <openreports.h>

#include "salesOrder.h"
#include "invoice.h"
#include "creditMemo.h"
#include "arOpenItem.h"
#include "customer.h"
#include "dspInvoiceInformation.h"
#include "crmaccount.h"
#include "arWorkBench.h"
#include "dspShipmentsBySalesOrder.h"
#include "dspSalesOrderStatus.h"
#include "cashReceipt.h"
#include "printSoForm.h"
#include "printInvoice.h"
#include "printCreditMemo.h"

dspCustomerInformation::dspCustomerInformation(QWidget* parent, Qt::WFlags fl)
    : QWidget(parent, fl)
{
  setupUi(this);

  connect(_arhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuArhist(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_creditMemo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuCreditMemo(QMenu*)));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulate()));
  connect(_custList, SIGNAL(clicked()), _cust, SLOT(sEllipses()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_editCreditMemo, SIGNAL(clicked()), this, SLOT(sEditCreditMemo()));
  connect(_editInvoice, SIGNAL(clicked()), this, SLOT(sEditInvoice()));
  connect(_editOrder, SIGNAL(clicked()), this, SLOT(sEditOrder()));
  connect(_editQuote, SIGNAL(clicked()), this, SLOT(sEditQuote()));
  connect(_invoice, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuInvoice(QMenu*, QTreeWidgetItem*)));
  connect(_newCreditMemo, SIGNAL(clicked()), this, SLOT(sNewCreditMemo()));
  connect(_newInvoice, SIGNAL(clicked()), this, SLOT(sNewInvoice()));
  connect(_newOrder, SIGNAL(clicked()), this, SLOT(sNewOrder()));
  connect(_newQuote, SIGNAL(clicked()), this, SLOT(sNewQuote()));
  connect(_order, SIGNAL(valid(bool)), _viewOrder, SLOT(setEnabled(bool)));
  connect(_order, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuSalesOrder(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_quote, SIGNAL(valid(bool)), _viewQuote, SLOT(setEnabled(bool)));
  connect(_quote, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuQuote(QMenu*)));
  connect(_viewCreditMemo, SIGNAL(clicked()), this, SLOT(sViewCreditMemo()));
  connect(_viewInvoice, SIGNAL(clicked()), this, SLOT(sViewInvoice()));
  connect(_viewOrder, SIGNAL(clicked()), this, SLOT(sViewOrder()));
  connect(_viewQuote, SIGNAL(clicked()), this, SLOT(sViewQuote()));
  connect(_convertQuote, SIGNAL(clicked()), this, SLOT(sConvertQuote()));
  connect(_crmAccount, SIGNAL(clicked()), this, SLOT(sCRMAccount()));
  connect(_refresh, SIGNAL(clicked()), this, SLOT(sPopulate()));
  connect(_workbenchInvoice, SIGNAL(clicked()), this, SLOT(sARWorkbench()));
  connect(_cashReceiptInvoice, SIGNAL(clicked()), this, SLOT(sCashReceipt()));
  connect(_workbenchCreditMemo, SIGNAL(clicked()), this, SLOT(sARWorkbench()));
  connect(_printOrder, SIGNAL(clicked()), this ,SLOT(sPrintSalesOrder()));
  connect(_printQuote, SIGNAL(clicked()), this ,SLOT(sPrintQuote()));
  connect(_printInvoice, SIGNAL(clicked()), this ,SLOT(sPrintInvoice()));
  connect(_printCreditMemo, SIGNAL(clicked()), this ,SLOT(sPrintCreditMemo()));

#ifndef Q_WS_MAC
  _custList->setMaximumWidth(25);
#endif

  // setup arhist list
  _arhist->addColumn(tr("Open"),      _orderColumn, Qt::AlignCenter );
  _arhist->addColumn(tr("Doc. Type"), _dateColumn,  Qt::AlignCenter );
  _arhist->addColumn(tr("Doc. #"),    _orderColumn, Qt::AlignRight  );
  _arhist->addColumn(tr("Doc. Date"), _dateColumn,  Qt::AlignCenter );
  _arhist->addColumn(tr("Due. Date"), _dateColumn,  Qt::AlignCenter );
  _arhist->addColumn(tr("Amount"),    _moneyColumn, Qt::AlignRight  );
  _arhist->addColumn(tr("Balance"),   _moneyColumn, Qt::AlignRight  );
  _arhist->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft);

  // setup Quote list
  _quote->addColumn(tr("Quote #"),    _orderColumn, Qt::AlignRight  );
  _quote->addColumn(tr("P/O Number"), _itemColumn,  Qt::AlignLeft   );
  _quote->addColumn(tr("Quote Date"), _dateColumn,  Qt::AlignCenter );
  if(_privleges->check("MaintainQuotes"))
  {
    connect(_quote, SIGNAL(valid(bool)), _editQuote, SLOT(setEnabled(bool)));
    connect(_quote, SIGNAL(itemSelected(int)), _editQuote, SLOT(animateClick()));
  }
  else
  {
    _newQuote->setEnabled(false);
    connect(_quote, SIGNAL(itemSelected(int)), _viewQuote, SLOT(animateClick()));
  }
  if (_privleges->check("ConvertQuotes"))
    connect(_quote, SIGNAL(valid(bool)), _convertQuote, SLOT(setEnabled(bool)));
  connect(omfgThis, SIGNAL(quotesUpdated(int, bool)), this, SLOT(sFillQuoteList()));

  // setup Order list
  _order->addColumn(tr("S/O #"),            _orderColumn, Qt::AlignLeft   );
  _order->addColumn(tr("Cust. P/O Number"), _itemColumn,  Qt::AlignLeft   );
  _order->addColumn(tr("Ordered"),          _dateColumn,  Qt::AlignCenter );
  _order->addColumn(tr("Scheduled"),        _dateColumn,  Qt::AlignCenter );
  if(_privleges->check("MaintainSalesOrders"))
  {
    connect(_order, SIGNAL(valid(bool)), _editOrder, SLOT(setEnabled(bool)));
    connect(_order, SIGNAL(itemSelected(int)), _editOrder, SLOT(animateClick()));
  }
  else
  {
    _newOrder->setEnabled(false);
    connect(_order, SIGNAL(itemSelected(int)), _viewOrder, SLOT(animateClick()));
  }
  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillOrderList()));

  // setup Invoice list
  _invoice->addColumn(tr("Posted"),     _ynColumn,    Qt::AlignCenter );
  _invoice->addColumn(tr("Open"),       _ynColumn,    Qt::AlignCenter );
  _invoice->addColumn(tr("Invoice #"),  _orderColumn, Qt::AlignLeft   );
  _invoice->addColumn(tr("S/O #"),      _orderColumn, Qt::AlignLeft   );
  _invoice->addColumn(tr("Invc. Date"), _dateColumn,  Qt::AlignCenter );
  _invoice->addColumn(tr("Due Date"),   _dateColumn,  Qt::AlignCenter );
  _invoice->addColumn(tr("Amount"),     _moneyColumn, Qt::AlignRight  );
  _invoice->addColumn(tr("Balance"),    _moneyColumn, Qt::AlignRight  );
  _invoice->addColumn(tr("Currency"),   _currencyColumn, Qt::AlignLeft);
  if(_privleges->check("MaintainMiscInvoices"))
  {
    connect(_invoice, SIGNAL(valid(bool)), _editInvoice, SLOT(setEnabled(bool)));
    connect(_invoice, SIGNAL(itemSelected(int)), _editInvoice, SLOT(animateClick()));
  }
  else
  {
    _newInvoice->setEnabled(false);
    connect(_invoice, SIGNAL(itemSelected(int)), _viewInvoice, SLOT(animateClick()));
  }
  if(_privleges->check("MaintainMiscInvoices") || _privleges->check("ViewMiscInvoices"))
    connect(_invoice, SIGNAL(valid(bool)), _viewInvoice, SLOT(setEnabled(bool)));
  connect(omfgThis, SIGNAL(invoicesUpdated(int, bool)), this, SLOT(sFillInvoiceList()));

  // setup CreditMemo list
  _creditMemo->addColumn(tr("Posted"),    _ynColumn,    Qt::AlignCenter );
  _creditMemo->addColumn(tr("Open"),      _ynColumn,    Qt::AlignCenter );
  _creditMemo->addColumn(tr("Type"),      _ynColumn,    Qt::AlignCenter );
  _creditMemo->addColumn(tr("C/M #"),     _orderColumn, Qt::AlignLeft   );
  _creditMemo->addColumn(tr("Doc. Date"), _dateColumn,  Qt::AlignCenter );
  _creditMemo->addColumn(tr("Amount"),    _moneyColumn, Qt::AlignRight  );
  _creditMemo->addColumn(tr("Balance"),   _moneyColumn, Qt::AlignRight  );
  _creditMemo->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft);
  if(!_privleges->check("MaintainCreditMemos"))
    _newCreditMemo->setEnabled(false);
  connect(_creditMemo, SIGNAL(valid(bool)), this, SLOT(sCreditMemoSelected(bool)));
  connect(_creditMemo, SIGNAL(itemSelected(int)), _viewCreditMemo, SLOT(animateClick()));
  connect(omfgThis, SIGNAL(creditMemosUpdated()), this, SLOT(sFillCreditMemoList()));

  _payments->addColumn(tr("Type"),         _whsColumn,      Qt::AlignLeft  );
  _payments->addColumn(tr("Status"),       _whsColumn,      Qt::AlignLeft  );
  _payments->addColumn(tr("Timestamp"),    _timeDateColumn, Qt::AlignLeft  );
  _payments->addColumn(tr("Entered By"),   _userColumn,     Qt::AlignLeft  );
  _payments->addColumn(tr("Total Amount"), _moneyColumn,    Qt::AlignRight );
  _payments->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignLeft  );
  _payments->addColumn(tr("S/O #"),        _orderColumn,    Qt::AlignLeft  );
  _payments->addColumn(tr("Reference"),    _orderColumn,    Qt::AlignLeft  );
  _payments->addColumn(tr("Allocated"),    _moneyColumn,    Qt::AlignRight );
  _payments->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignLeft  );

  if (omfgThis->singleCurrency())
  {
    _arhist->hideColumn(7);
    _invoice->hideColumn(7);
    _creditMemo->hideColumn(6);
    _payments->hideColumn(5);
    _payments->hideColumn(8);
  }

  _edit->setEnabled(_privleges->check("MaintainCustomerMasters"));
  _crmAccount->setEnabled(_privleges->check("MaintainCRMAccounts") || _privleges->check("ViewCRMAccounts"));
  _workbenchInvoice->setEnabled(_privleges->check("ViewAROpenItems"));
  _cashReceiptInvoice->setEnabled(_privleges->check("MaintainCashReceipts"));
  _workbenchCreditMemo->setEnabled(_privleges->check("ViewAROpenItems"));
}

dspCustomerInformation::~dspCustomerInformation()
{
    // no need to delete child widgets, Qt does it all for us
}

void dspCustomerInformation::languageChange()
{
    retranslateUi(this);
}

enum SetResponse dspCustomerInformation::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _cust->setId(param.toInt());
    _cust->setReadOnly(TRUE);
    _custList->setEnabled(FALSE);
  }
  
  if(pParams.inList("modal"))
  {
    disconnect(_close, SIGNAL(clicked()), this, SLOT(close()));
    disconnect(_creditMemo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuCreditMemo(QMenu*)));
    disconnect(_invoice, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuInvoice(QMenu*)));
    disconnect(_order, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuSalesOrder(QMenu*)));
    disconnect(_quote, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuQuote(QMenu*)));
    if(_privleges->check("MaintainQuotes"))
      disconnect(_quote, SIGNAL(itemSelected(int)), _editQuote, SLOT(animateClick()));
    else
      disconnect(_quote, SIGNAL(itemSelected(int)), _viewQuote, SLOT(animateClick()));
    if(_privleges->check("MaintainSalesOrders"))
      disconnect(_order, SIGNAL(itemSelected(int)), _editOrder, SLOT(animateClick()));
    else
      disconnect(_order, SIGNAL(itemSelected(int)), _viewOrder, SLOT(animateClick()));
    if(_privleges->check("MaintainMiscInvoices"))
      disconnect(_invoice, SIGNAL(itemSelected(int)), _editInvoice, SLOT(animateClick()));
    else
      disconnect(_invoice, SIGNAL(itemSelected(int)), _viewInvoice, SLOT(animateClick()));
    disconnect(_creditMemo, SIGNAL(itemSelected(int)), _viewCreditMemo, SLOT(animateClick()));

    connect(_close, SIGNAL(clicked()), parentWidget(), SLOT(close()));

    _edit->hide();
    _editCreditMemo->hide();
    _editInvoice->hide();
    _editOrder->hide();
    _editQuote->hide();
    _viewCreditMemo->hide();
    _viewInvoice->hide();
    _viewOrder->hide();
    _viewQuote->hide();
    _newCreditMemo->hide();
    _newInvoice->hide();
    _newOrder->hide();
    _newQuote->hide();
    _convertQuote->hide();
  }
  return NoError;
}

void dspCustomerInformation::sPopulateCustInfo()
{
  if (_cust->id() != -1)
  {
    _comment->setId(_cust->id());

    XSqlQuery query;
    query.prepare( "SELECT cust_name,"
                   "       formatBoolYN(cust_active) AS f_active,"
                   "       cust_address1, cust_address2, cust_address3,"
                   "       cust_city, cust_state, cust_zipcode,"
                   "       cust_contact, cust_phone, cust_fax, cust_email,"
                   "       cust_corraddress1, cust_corraddress2, cust_corraddress3,"
                   "       cust_corrcity, cust_corrstate, cust_corrzipcode,"
                   "       cust_corrcontact, cust_corrphone, cust_corrfax, cust_corremail,"
                   "       cust_creditstatus, cust_comments, cust_shipvia,"
                   "       (custtype_code || '-' || custtype_descrip) AS f_custtype,"
                   "       (terms_code || '-' || terms_descrip) AS f_terms,"
                   "       formatMoney(cust_creditlmt) AS f_custcreditlmt "
                   "  FROM cust, custtype, terms "
                   " WHERE ((cust_custtype_id=custtype_id)"
                   "   AND  (cust_terms_id=terms_id)"
                   "   AND  (cust_id=:cust_id));" );
    query.bindValue(":cust_id", _cust->id());
    query.exec();
    if (query.first())
    {
      _name->setText(query.value("cust_name").toString());
      _active->setText(query.value("f_active").toString());
      _corrAddress1->setText(query.value("cust_corraddress1").toString());
      _corrAddress2->setText(query.value("cust_corraddress2").toString());
      _corrAddress3->setText(query.value("cust_corraddress3").toString());
      _corrCity->setText(query.value("cust_corrcity").toString());
      _corrState->setText(query.value("cust_corrstate").toString());
      _corrZipCode->setText(query.value("cust_corrzipcode").toString());
      _corrContact->setText(query.value("cust_corrcontact").toString());
      _corrPhone->setText(query.value("cust_corrphone").toString());
      _corrFAX->setText(query.value("cust_corrfax").toString());
      _corrEmail->setText(query.value("cust_corremail").toString());
      _billAddress1->setText(query.value("cust_address1").toString());
      _billAddress2->setText(query.value("cust_address2").toString());
      _billAddress3->setText(query.value("cust_address3").toString());
      _billCity->setText(query.value("cust_city").toString());
      _billState->setText(query.value("cust_state").toString());
      _billZipCode->setText(query.value("cust_zipcode").toString());
      _billContact->setText(query.value("cust_contact").toString());
      _billPhone->setText(query.value("cust_phone").toString());
      _billFAX->setText(query.value("cust_fax").toString());
      _billEmail->setText(query.value("cust_email").toString());
      _notes->setText(query.value("cust_comments").toString());
      _type->setText(query.value("f_custtype").toString());
      _terms->setText(query.value("f_terms").toString());
      _shipvia->setText(query.value("cust_shipvia").toString());
      _creditLimit->setText(query.value("f_custcreditlmt").toString());

      if (query.value("cust_creditstatus").toString() == "G")
      {
        _creditStatus->setText(tr("In Good Standing"));
        _creditStatus->setPaletteForegroundColor(QColor("black"));
      }
      else if (query.value("cust_creditstatus").toString() == "W")
      {
        _creditStatus->setText(tr("On Credit Warning"));
        _creditStatus->setPaletteForegroundColor(QColor("orange"));
      }
      else if (query.value("cust_creditstatus").toString() == "H")
      {
        _creditStatus->setText(tr("On Credit Hold"));
        _creditStatus->setPaletteForegroundColor(QColor("red"));
      }

      query.prepare( "SELECT formatDate(MIN(cohist_invcdate)) AS firstdate,"
                     "       formatDate(MAX(cohist_invcdate)) AS lastdate "
                     "FROM cohist "
                     "WHERE (cohist_cust_id=:cust_id);" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
      {
        _firstSaleDate->setText(query.value("firstdate").toString());
        _lastSaleDate->setText(query.value("lastdate").toString());
      }
      query.prepare( "SELECT formatMoney(COALESCE(SUM(round(cohist_qtyshipped * cohist_unitprice,2)), 0)) AS lysales "
                     "FROM cohist "
                     "WHERE ( (cohist_invcdate BETWEEN (DATE_TRUNC('year', CURRENT_TIMESTAMP) - INTERVAL '1 year') AND"
                     "                                 (DATE_TRUNC('year', CURRENT_TIMESTAMP) - INTERVAL '1 day'))"
                     " AND (cohist_cust_id=:cust_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
        _lastYearSales->setText(query.value("lysales").toString());

      query.prepare( "SELECT formatMoney(COALESCE(SUM(round(cohist_qtyshipped * cohist_unitprice,2)), 0)) AS ytdsales "
                     "FROM cohist "
                     "WHERE ( (cohist_invcdate>=DATE_TRUNC('year', CURRENT_TIMESTAMP))"
                     " AND (cohist_cust_id=:cust_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
        _ytdSales->setText(query.value("ytdsales").toString());

      query.prepare( "SELECT formatMoney( COALESCE( SUM( (noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) *"
                     "                                   (coitem_price / coitem_price_invuomratio) ), 0 ) ) AS backlog "
                     "FROM cohead, coitem, itemsite, item "
                     "WHERE ( (coitem_cohead_id=cohead_id)"
                     " AND (coitem_itemsite_id=itemsite_id)"
                     " AND (itemsite_item_id=item_id)"
                     " AND (coitem_status='O')"
                     " AND (cohead_cust_id=:cust_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
        _backlog->setText(query.value("backlog").toString());
      
      query.prepare( "SELECT formatMoney( COALESCE( SUM( CASE WHEN (aropen_doctype IN ('I', 'D')) THEN (aropen_amount - aropen_paid)"
                     "                                       ELSE ((aropen_amount - aropen_paid) * -1)"
                     "                                   END ), 0 ) ) AS f_balance "
                     "FROM aropen "
                     "WHERE ( (aropen_open)"
                     " AND (aropen_cust_id=:cust_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
        _openBalance->setText(query.value("f_balance").toString());

      query.prepare( "SELECT formatMoney( noNeg( COALESCE( SUM( CASE WHEN (aropen_doctype IN ('I', 'D')) THEN (aropen_amount - aropen_paid)"
                     "                                      ELSE ((aropen_amount - aropen_paid) * -1)"
                     "                                   END ), 0 ) ) ) AS f_balance "
                     "FROM aropen "
                     "WHERE ( (aropen_open)"
                     " AND (aropen_duedate < CURRENT_DATE)"
                     " AND (aropen_cust_id=:cust_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
        _lateBalance->setText(query.value("f_balance").toString());
    }
  }
}

void dspCustomerInformation::sPopulate()
{
  sPopulateCustInfo();
  sFillARHistory();
  sFillQuoteList();
  sFillOrderList();
  sFillInvoiceList();
  sFillCreditMemoList();
  sFillPaymentsList();
}

void dspCustomerInformation::sEdit()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("mode", "edit");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sPrint()
{
  if(!_cust->isValid())
  {
    QMessageBox::warning(this, tr("No Customer Selected"),
      tr("You must select a valid customer before printing.") );
    return;
  }
  ParameterList params;
  params.append("cust_id", _cust->id());

  orReport report("CustomerInformation", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCustomerInformation::sFillQuoteList()
{
  _quote->clear();

  q.prepare( "SELECT DISTINCT quhead_id, quhead_number,"
             "                quhead_custponumber, formatDate(quhead_quotedate) "
             "FROM quhead "
             "WHERE (quhead_cust_id=:cust_id) "
             "ORDER BY quhead_number;" );
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  _quote->populate(q);
}

void dspCustomerInformation::sNewQuote()
{
  ParameterList params;
  params.append("mode", "newQuote");
  params.append("cust_id", _cust->id());

  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sEditQuote()
{
  ParameterList params;
  params.append("mode", "editQuote");
  params.append("quhead_id", _quote->id());

  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sViewQuote()
{
  ParameterList params;
  params.append("mode", "viewQuote");
  params.append("quhead_id", _quote->id());

  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sFillARHistory()
{
  _arhist->clear();

  if(_cust->id() == -1)
    return;

  XSqlQuery query;
  query.prepare( "SELECT aropen_id AS aropenid, -1 AS applyid,"
                 "       aropen_docnumber AS sortnumber,"
                 "       aropen_docnumber AS docnumber,"
                 "       formatBoolYN(aropen_open) AS f_open,"
                 "       CASE WHEN (aropen_doctype='I') THEN :invoice"
                 "            WHEN (aropen_doctype='C') THEN :creditMemo"
                 "            WHEN (aropen_doctype='D') THEN :debitMemo"
                 "            WHEN (aropen_doctype='R') THEN :cashdeposit"
                 "            ELSE :other"
                 "       END AS documenttype,"
                 "       formatDate(aropen_docdate) AS f_docdate,"
                 "       formatDate(aropen_duedate) AS f_duedate,"
                 "       formatMoney(aropen_amount) AS f_amount,"
                 "       formatMoney((aropen_amount - aropen_paid)) AS f_balance,"
                 "       currConcat(aropen_curr_id) AS currAbbr "
                 "FROM aropen "
                 "WHERE (aropen_cust_id=:cust_id) "

                 "UNION "
                 "SELECT -1 AS aropenid, arapply_source_aropen_id AS applyid,"
                 "       aropen_docnumber AS sortnumber,"
                 "       CASE WHEN (arapply_source_doctype IN ('C','R')) THEN arapply_source_docnumber"
                 "            WHEN (arapply_source_doctype='K') THEN arapply_refnumber"
                 "            ELSE :error"
                 "       END AS docnumber,"
                 "       '' AS f_open,"
                 "       CASE WHEN (arapply_source_doctype='C') THEN :creditMemo"
                 "            WHEN (arapply_source_doctype='R') THEN :cashdeposit"
                 "            WHEN (arapply_fundstype='C') THEN :check"
                 "            WHEN (arapply_fundstype='T') THEN :certifiedCheck"
                 "            WHEN (arapply_fundstype='M') THEN :masterCard"
                 "            WHEN (arapply_fundstype='V') THEN :visa"
                 "            WHEN (arapply_fundstype='A') THEN :americanExpress"
                 "            WHEN (arapply_fundstype='D') THEN :discoverCard"
                 "            WHEN (arapply_fundstype='R') THEN :otherCreditCard"
                 "            WHEN (arapply_fundstype='K') THEN :cash"
                 "            WHEN (arapply_fundstype='W') THEN :wireTransfer"
                 "            WHEN (arapply_fundstype='O') THEN :other"
                 "       END AS documenttype,"
                 "       formatDate(arapply_postdate) AS f_docdate,"
                 "       '' AS f_duedate,"
                 "       formatMoney(arapply_applied) AS f_amount,"
                 "       '' AS f_balance,"
                 "       currConcat(arapply_curr_id) AS currAbbr "
                 "FROM arapply, aropen "
                 "WHERE ( (arapply_target_doctype IN ('I', 'D'))"
                 " AND (arapply_target_doctype=aropen_doctype)"
                 " AND (arapply_target_docnumber=aropen_docnumber)"
                 " AND (arapply_cust_id=:cust_id)"
                 " AND (aropen_cust_id=:cust_id) ) "

                 "UNION "
                 "SELECT -1 AS aropenid, arapply_target_aropen_id AS applyid,"
                 "       aropen_docnumber AS sortnumber,"
                 "       arapply_target_docnumber AS docnumber,"
                 "       '' AS f_open,"
                 "       CASE WHEN (arapply_target_doctype='I') THEN :invoice"
                 "            WHEN (arapply_target_doctype='D') THEN :debitMemo"
                 "            ELSE :other"
                 "       END AS documenttype,"
                 "       formatDate(arapply_postdate) AS f_docdate,"
                 "       '' AS f_duedate,"
                 "       formatMoney(arapply_applied) AS f_amount,"
                 "       '' AS f_balance,"
                 "       currConcat(arapply_curr_id) AS currAbbr "
                 "FROM arapply, aropen "
                 "WHERE ( (arapply_source_doctype IN ('K', 'C', 'R'))"
                 " AND (arapply_source_doctype=aropen_doctype)"
                 " AND (arapply_source_docnumber=aropen_docnumber)"
                 " AND (arapply_cust_id=:cust_id)"
                 " AND (aropen_cust_id=:cust_id) ) "

                 "ORDER BY sortnumber, applyid;" );

  query.bindValue(":invoice", tr("Invoice"));
  query.bindValue(":creditMemo", tr("C/M"));
  query.bindValue(":debitMemo", tr("D/M"));
  query.bindValue(":check", tr("Check"));
  query.bindValue(":certifiedCheck", tr("Certified Check"));
  query.bindValue(":masterCard", tr("Master Card"));
  query.bindValue(":visa", tr("Visa"));
  query.bindValue(":americanExpress", tr("American Express"));
  query.bindValue(":discoverCard", tr("Discover Card"));
  query.bindValue(":otherCreditCard", tr("Other Credit Card"));
  query.bindValue(":cash", tr("Cash"));
  query.bindValue(":wireTransfer", tr("Wire Transfer"));
  query.bindValue(":cashdeposit", tr("Cash Deposit"));
  query.bindValue(":other", tr("Other"));
  query.bindValue(":cust_id", _cust->id());
  query.exec();
  if (query.first())
  {
    XTreeWidgetItem *document = NULL;
    do
    {
      if (query.value("applyid").toInt() == -1)
        document = new XTreeWidgetItem( _arhist, document,
                                      query.value("aropenid").toInt(), query.value("applyid").toInt(),
                                      query.value("f_open"), query.value("documenttype"),
                                      query.value("docnumber"), query.value("f_docdate"),
                                      query.value("f_duedate"), query.value("f_amount"),
                                      query.value("f_balance"), query.value("currAbbr") );
       else if (document != NULL)
        new XTreeWidgetItem( document,
                           query.value("aropenid").toInt(), query.value("applyid").toInt(),
                           "", query.value("documenttype"),
                           query.value("docnumber"), query.value("f_docdate"),
                           "", query.value("f_amount"),
                           "" );
    }
    while (query.next());
  }
  // End Population of A/R History
}
void dspCustomerInformation::sFillOrderList()
{
  QString sql( "SELECT DISTINCT cohead_id, cohead_number,"
             "       cohead_custponumber,"
             "       formatDate(cohead_orderdate) AS f_ordered,"
             "       formatDate(MIN(coitem_scheddate)) AS f_scheduled "
             "  FROM cohead LEFT OUTER JOIN coitem ON (coitem_cohead_id=cohead_id) "
             " WHERE ( ");
  if (!_orderShowclosed->isChecked())
    sql += " ((coitem_status = 'O') OR (coitem_status IS NULL)) AND ";
    
  sql +=     " (cohead_cust_id=:cust_id) )"
             "GROUP BY cohead_id, cohead_number,"
             "         cohead_custponumber, cohead_orderdate "
             "ORDER BY cohead_number; ";
  q.prepare(sql);
  q.bindValue(":cust_id", _cust->id());
  q.exec();

  _order->clear();
  _order->populate(q);
  //_order->setDragString("soheadid=");
}

void dspCustomerInformation::sNewOrder()
{
  salesOrder::newSalesOrder(_cust->id());
}

void dspCustomerInformation::sEditOrder()
{
  salesOrder::editSalesOrder(_order->id(), false);
}

void dspCustomerInformation::sViewOrder()
{
  salesOrder::viewSalesOrder(_order->id());
}

void dspCustomerInformation::sEditInvOrder()
{
    q.prepare("SELECT cohead_id "
              "FROM invchead, cohead "
              "WHERE ((cohead_number=invchead_ordernumber) "
              "  AND (invchead_id=:invoice_id));");
    q.bindValue(":invoice_id",  _invoice->id());
    q.exec();
    if (q.first())
      salesOrder::editSalesOrder(q.value("cohead_id").toInt(), false);
}

void dspCustomerInformation::sViewInvOrder()
{
    q.prepare("SELECT cohead_id "
              "FROM invchead, cohead "
              "WHERE ((cohead_number=invchead_ordernumber) "
              "  AND (invchead_id=:invoice_id));");
    q.bindValue(":invoice_id",  _invoice->id());
    q.exec();
    if (q.first())
      salesOrder::viewSalesOrder(q.value("cohead_id").toInt());
}

void dspCustomerInformation::sFillInvoiceList()
{
  QString sql("SELECT invchead_id AS id, -1 AS altId,"
            "       formatBoolYN(invchead_posted) AS f_posted,"
            "       formatBoolYN(COALESCE(aropen_open, FALSE)) AS f_open,"
            "       text(invchead_invcnumber) AS invcnumber,"
            "       text(invchead_ordernumber) AS invchead_ordernumber,"
            "       formatDate(invchead_invcdate) AS f_docdate,"
            "       formatdate(aropen_duedate) AS f_duedate,"
            "       formatMoney(COALESCE(aropen_amount,0)) AS f_amount,"
            "       formatMoney(COALESCE(aropen_amount - aropen_paid,0)) AS f_balance,"
            "       currConcat(COALESCE(aropen_curr_id,-1)) AS f_curr,"
            "       ((COALESCE(aropen_duedate,current_date) < current_date) "
            "       AND COALESCE(aropen_open,FALSE)) AS pastdue "
            "  FROM invchead LEFT OUTER JOIN aropen"
            "         ON (aropen_docnumber=invchead_invcnumber"
            "         AND aropen_doctype='I'"
            "         AND aropen_cust_id=invchead_cust_id)"
            " WHERE (invchead_cust_id=:cust_id)");
            
  if (!_invoiceShowclosed->isChecked())
    sql +=  "   AND (COALESCE(aropen_open,TRUE)) ";
    
  sql +=    " UNION "
            "SELECT aropen_id AS id, -2 AS altId,"
            "       formatBoolYN(true) AS f_posted,"
            "       formatBoolYN(aropen_open) AS f_open,"
            "       text(aropen_docnumber) AS invcnumber,"
            "       aropen_ordernumber,"
            "       formatDate(aropen_docdate) AS f_docdate,"
            "       formatDate(aropen_duedate) AS f_duedate,"
            "       formatMoney(aropen_amount) AS f_amount,"
            "       formatMoney(aropen_amount - aropen_paid) AS f_balance,"
            "       currConcat(aropen_curr_id) AS f_curr,"
            "       ((COALESCE(aropen_duedate,current_date) < current_date) "
            "       AND aropen_open) AS pastdue "
            "  FROM aropen LEFT OUTER JOIN invchead"
            "         ON (aropen_docnumber=invchead_invcnumber"
            "         AND aropen_doctype='I'"
            "         AND aropen_cust_id=invchead_cust_id)"
            " WHERE ((aropen_doctype='I')"
            "   AND  (invchead_id IS NULL)";
            
  if (!_invoiceShowclosed->isChecked())
    sql +=  "   AND (aropen_open) ";
    
  sql +=    "   AND  (aropen_cust_id=:cust_id) ) "
            "ORDER BY invcnumber DESC;";
  q.prepare(sql);
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  _invoice->clear();
//  _invoice->populate(q, true);
  
  XTreeWidgetItem *last = 0;
  while (q.next())
  {
    last = new XTreeWidgetItem(_invoice, last, q.value("id").toInt(),
                               q.value("altId").toInt(),
                               q.value("f_posted"),
                               q.value("f_open"),
                               q.value("invcnumber"),
                               q.value("invchead_ordernumber"),
                               q.value("f_docdate"),
                               q.value("f_duedate"),
                               q.value("f_amount"),
                               q.value("f_balance"),
             q.value("f_curr") );

    if (q.value("pastdue").toBool())
      last->setTextColor(5, "red");
  }

}

void dspCustomerInformation::sNewInvoice()
{
  invoice::newInvoice(_cust->id());
}

void dspCustomerInformation::sEditInvoice()
{
  int  invcId		= _invoice->id();
  bool invcPosted	= false;

  q.prepare("SELECT invchead_id AS id, invchead_posted AS posted "
            "FROM invchead "
            "WHERE (invchead_invcnumber=:docnum);");
  q.bindValue(":docnum", _invoice->currentItem()->text(2));
  q.exec();
  if (q.first())
  {
    invcId	= q.value("id").toInt();
    invcPosted	= q.value("posted").toBool();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (! invcPosted ||
      QMessageBox::question(this, tr("Edit Posted Invoice?"),
                              tr("<p>This Invoice has already been posted. "
                                 "Are you sure you want to edit it?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    invoice::editInvoice(invcId);
    sFillInvoiceList();
  }
  else
  {
    invoice::viewInvoice(invcId);
  }
}

void dspCustomerInformation::sViewInvoice()
{
  ParameterList params;
  XTreeWidgetItem *item = static_cast<XTreeWidgetItem*>(_invoice->currentItem());
  if(item)
  {
    params.append("invoiceNumber", item->text(2));
    dspInvoiceInformation* newdlg = new dspInvoiceInformation();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
  {
    params.append("mode", "view");
    params.append("aropen_id", _invoice->id());
    arOpenItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspCustomerInformation::sFillCreditMemoList()
{
  disconnect(_creditMemo, SIGNAL(valid(bool)), this, SLOT(sHandleCreditMemoPrint()));
  _printCreditMemo->setEnabled(FALSE);
    
  QString sql( "SELECT cmhead_id, -1,"
             "       formatBoolYN(false), '', '',"
             "       text(cmhead_number) AS docnumber,"
             "       formatDate(cmhead_docdate),"
             "       '', '', ''"
             "  FROM cmhead"
             " WHERE ((NOT cmhead_posted)"
             "   AND  (cmhead_cust_id=:cust_id)) "
             "UNION "
             "SELECT aropen_id, -2,"
             "       formatBoolYN(true), formatBoolYN(aropen_open),"
             "       :creditmemo,"
             "       text(aropen_docnumber) AS docnumber,"
             "       formatDate(aropen_docdate),"
             "       formatMoney(aropen_amount),"
             "       formatMoney(aropen_amount - aropen_paid),"
             "       currConcat(aropen_curr_id)"
             "  FROM aropen, cmhead "
             " WHERE ((aropen_doctype = 'C')"
             "   AND  (aropen_docnumber=cmhead_number) ");
  if (!_creditMemoShowclosed->isChecked())
    sql +=   "   AND (aropen_open) ";
    
  sql +=     "   AND  (aropen_cust_id=:cust_id) ) "
             "UNION "
             "SELECT aropen_id, -3,"
             "       formatBoolYN(true), formatBoolYN(aropen_open),"
             "       CASE WHEN (aropen_doctype='C') THEN :creditmemo"
             "            WHEN (aropen_doctype='R') THEN :cashdeposit"
             "            else aropen_doctype"
             "       END,"
             "       text(aropen_docnumber) AS docnumber,"
             "       formatDate(aropen_docdate),"
             "       formatMoney(aropen_amount),"
             "       formatMoney(aropen_amount - aropen_paid),"
             "       currConcat(aropen_curr_id)"
             "  FROM aropen "
             " WHERE ((aropen_doctype IN ('C', 'R'))"
             "   AND  (aropen_cust_id=:cust_id) ";
  if (!_creditMemoShowclosed->isChecked())
    sql +=   "   AND (aropen_open) ";
    
  sql +=     "   AND  (NOT EXISTS (SELECT cmhead_id "
             "                     FROM cmhead "
             "                     WHERE (cmhead_number=aropen_docnumber) ) ) ) "
             "ORDER BY docnumber; ";
             
  q.prepare(sql);
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":creditmemo", tr("CM"));
  q.bindValue(":cashdeposit", tr("CD"));
  q.exec();
  _creditMemo->clear();
  _creditMemo->populate(q, true);
  
  connect(_creditMemo, SIGNAL(valid(bool)), this, SLOT(sHandleCreditMemoPrint()));
}

void dspCustomerInformation::sNewCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sEditCreditMemo()
{
  int  memoType		= _creditMemo->altId();
  int  memoId		= _creditMemo->id();
  bool memoPosted	= false;

  q.prepare("SELECT -1 AS type, cmhead_id AS id, cmhead_posted AS posted "
            "FROM cmhead "
            "WHERE (cmhead_number=:docnum) "
            "UNION "
            "SELECT -2 AS type, aropen_id AS id, aropen_posted AS posted "
            "FROM aropen "
            "WHERE ((aropen_docnumber=:docnum)"
            "  AND (aropen_doctype='C') "
            ") ORDER BY type DESC LIMIT 1;");
  q.bindValue(":docnum", _creditMemo->currentItem()->text(3));
  q.exec();
  if (q.first())
  {
    memoType	= q.value("type").toInt();
    memoId	= q.value("id").toInt();
    memoPosted	= q.value("posted").toBool();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  if (! memoPosted ||
      QMessageBox::question(this, tr("Edit Posted Memo?"),
                            tr("<p>This Credit Memo has already been posted. "
                               "Are you sure you want to edit it?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    params.append("mode", "edit");
  else
    params.append("mode", "view");

  if(memoType == -1)
  {
    params.append("cmhead_id", memoId);
  
    creditMemo *newdlg = new creditMemo();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
  {
    params.append("aropen_id", memoId);
    arOpenItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
      sFillCreditMemoList();
  }
}

void dspCustomerInformation::sViewCreditMemo()
{
  int memoType	= _creditMemo->altId();
  int memoId	= _creditMemo->id();

  q.prepare("SELECT -1 AS type, cmhead_id AS id "
            "FROM cmhead "
            "WHERE (cmhead_number=:docnum) "
            "UNION "
            "SELECT -2 AS type, aropen_id AS id "
            "FROM aropen "
            "WHERE ((aropen_docnumber=:docnum)"
            "  AND (aropen_doctype='C') "
            ") ORDER BY type DESC LIMIT 1;");
  q.bindValue(":docnum", _creditMemo->currentItem()->text(3));
  q.exec();
  if (q.first())
  {
    memoType = q.value("type").toInt();
    memoId = q.value("id").toInt();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(memoType == -1)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("cmhead_id", memoId);

    creditMemo *newdlg = new creditMemo();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("aropen_id", memoId);
    arOpenItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
      sFillCreditMemoList();
  }
}

void dspCustomerInformation::sEditAropen()
{
  ParameterList params;
  params.append("mode", "edit");
  if(_arhist->id() != -1)
    params.append("aropen_id", _arhist->id());
  else
    params.append("aropen_id", _arhist->altId());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCustomerInformation::sViewAropen()
{
  ParameterList params;
  params.append("mode", "view");
  if(_arhist->id() != -1)
    params.append("aropen_id", _arhist->id());
  else
    params.append("aropen_id", _arhist->altId());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCustomerInformation::sCashReceipt()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sFillPaymentsList()
{
  q.prepare("SELECT ccpay_id, cohead_id,"
            "       CASE WHEN (ccpay_type='A') THEN :preauth"
            "            WHEN (ccpay_type='C') THEN :charge"
            "            WHEN (ccpay_type='R') THEN :refund"
            "            ELSE ccpay_type"
            "       END AS f_type,"
            "       CASE WHEN (ccpay_status='A') THEN :authorized"
            "            WHEN (ccpay_status='C') THEN :approved"
            "            WHEN (ccpay_status='D') THEN :declined"
            "            WHEN (ccpay_status='V') THEN :voided"
            "            WHEN (ccpay_status='X') THEN :noapproval"
            "            ELSE ccpay_status"
            "       END AS f_status,"
            "       formatDateTime(ccpay_transaction_datetime) AS f_datetime,"
            "       ccpay_by_username, ccpay_amount,"
            "       currConcat(ccpay_curr_id) AS ccpay_currAbbr,"
            "       COALESCE(cohead_number, ccpay_order_number),"
            "       ccpay_r_ref,"
            "       ABS(COALESCE(payco_amount, ccpay_amount)),"
            "       currConcat(COALESCE(payco_curr_id, ccpay_curr_id)) AS payco_currAbbr"
            "  FROM ccpay LEFT OUTER JOIN "
            "       (payco JOIN cohead ON (payco_cohead_id=cohead_id))"
            "         ON (payco_ccpay_id=ccpay_id)"
            " WHERE ((ccpay_cust_id=:cust_id))"
            " ORDER BY ccpay_transaction_datetime;");
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":preauth", tr("Preauthorization"));
  q.bindValue(":charge",  tr("Charge"));
  q.bindValue(":refund",  tr("Refund"));
  q.bindValue(":authorized", tr("Authorized"));
  q.bindValue(":approved",   tr("Approved"));
  q.bindValue(":declined",   tr("Declined"));
  q.bindValue(":voided",     tr("Voided"));
  q.bindValue(":noapproval", tr("No Approval Code"));
  q.exec();

  _payments->clear();
  _payments->populate(q, true);

  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspCustomerInformation::sPopulateMenuQuote( QMenu * pMenu )
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("New Quote..."), this, SLOT(sNewQuote()), 0);
  if (!_privleges->check("MaintainQuotes"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Quote..."), this, SLOT(sEditQuote()), 0);
  if (!_privleges->check("MaintainQuotes"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Quote..."), this, SLOT(sViewQuote()), 0);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Convert Quote..."), this, SLOT(sConvertQuote()), 0);
  if (!_privleges->check("ConvertQuotes"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspCustomerInformation::sPopulateMenuSalesOrder( QMenu * pMenu )
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("New Order..."), this, SLOT(sNewOrder()), 0);
  if(!_privleges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
  if(!_privleges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);

  pMenu->insertSeparator();

  pMenu->insertItem(tr("Shipment Status..."), this, SLOT(sDspShipmentStatus()), 0);
  pMenu->insertItem(tr("Shipments..."), this, SLOT(sShipment()), 0);
}

void dspCustomerInformation::sPopulateMenuInvoice( QMenu * pMenu,  QTreeWidgetItem *selected)
{
  int menuItem;
  menuItem = pMenu->insertItem(tr("New Invoice..."), this, SLOT(sNewInvoice()), 0);
  if(!_privleges->check("MaintainMiscInvoices"))
    pMenu->setItemEnabled(menuItem, FALSE);
  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Invoice..."), this, SLOT(sEditInvoice()), 0);
  if(!_privleges->check("MaintainMiscInvoices"))
    pMenu->setItemEnabled(menuItem, FALSE);
  pMenu->insertItem(tr("View Invoice..."), this, SLOT(sViewInvoice()), 0);
  pMenu->insertSeparator();
  
  if (selected->text(3).length() != 0)
  {
    menuItem = pMenu->insertItem(tr("Edit Sales Order..."), this, SLOT(sEditInvOrder()), 0);
    if(!_privleges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
    pMenu->insertItem(tr("View Sales Order..."), this, SLOT(sViewInvOrder()), 0);
    if(!_privleges->check("ViewSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  
    pMenu->insertSeparator();

    pMenu->insertItem(tr("Sales Order Shipment Status..."), this, SLOT(sInvShipmentStatus()), 0);
    pMenu->insertItem(tr("Sales Order Shipments..."), this, SLOT(sInvShipment()), 0);
  
  }
}

void dspCustomerInformation::sPopulateMenuCreditMemo( QMenu * pMenu )
{
  int menuItem;
  
  menuItem = pMenu->insertItem(tr("New Credit Memo..."), this, SLOT(sNewCreditMemo()), 0);
  if(!_privleges->check("MaintainCreditMemos"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Credit Memo..."), this, SLOT(sEditCreditMemo()), 0);
  if(((_creditMemo->altId() == -1 || _creditMemo->altId() == -2) && !_privleges->check("MaintainCreditMemos"))
   ||(_creditMemo->altId() == -3 && !_privleges->check("EditAROpenItem")))
    pMenu->setItemEnabled(menuItem, FALSE);
  pMenu->insertItem(tr("View Credit Memo..."), this, SLOT(sViewCreditMemo()), 0);
}

void dspCustomerInformation::sPopulateMenuArhist( QMenu * pMenu )
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit A/R Open Item..."), this, SLOT(sEditAropen()), 0);
  if(!_privleges->check("EditAROpenItem"))
    pMenu->setItemEnabled(menuItem, FALSE);
  pMenu->insertItem(tr("View A/R Open Item..."), this, SLOT(sViewAropen()), 0);
}

void dspCustomerInformation::sConvertQuote()
{
  if ( QMessageBox::information( this, tr("Convert Selected Quote(s)"),
                               tr("Are you sure that you want to convert the selected Quote(s) to Sales Order(s)?" ),
                               tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    XSqlQuery check;
    check.prepare( "SELECT quhead_number, cust_creditstatus "
                   "FROM quhead, cust "
                   "WHERE ( (quhead_cust_id=cust_id)"
                   " AND (quhead_id=:quhead_id) );" );

    XSqlQuery convert;
    convert.prepare("SELECT convertQuote(:quhead_id) AS sohead_id;");

    int counter = 0;
    int soheadid = -1;
    QList<QTreeWidgetItem*> selected = _quote->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      int id = ((XTreeWidgetItem *)(selected[i]))->id();

      check.bindValue(":quhead_id", id);
      check.exec();
      if (check.first())
      {
        if ( (check.value("cust_creditstatus").toString() == "H") &&
             (!_privleges->check("CreateSOForHoldCustomer")) )
        {
          QMessageBox::warning( this, tr("Cannot Convert Quote"),
              tr("<p>Quote #%1 is for a Customer that has "
                 "been placed on a Credit Hold and you do not"
                 " have privilege to create Sales Orders for "
                 "Customers on Credit Hold.  The selected "
                 "Customer must be taken off of Credit Hold "
                 "before you may create convert this Quote." )
              .arg(check.value("quhead_number").toString()) );
          return;
        }

        if ( (check.value("cust_creditstatus").toString() == "W") &&
             (!_privleges->check("CreateSOForWarnCustomer")) )
        {
          QMessageBox::warning( this, tr("Cannot Convert Quote"),
              tr("<p>Quote #%1 is for a Customer that has "
                 "been placed on a Credit Warning and you do "
                 " not have privilege to create Sales Orders "
                 "for Customers on Credit Warning.  The "
                 "selected Customer must be taken off of "
                 "Credit Warning before you may convert this "
                 "Quote." )
              .arg(check.value("quhead_number").toString()) );
          return;
        }
      }
      else if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        continue;
      }

      convert.bindValue(":quhead_id", id);
      convert.exec();
      if (convert.first())
      {
        soheadid = convert.value("sohead_id").toInt();
        if(soheadid < 0)
        {
          QMessageBox::warning( this, tr("Cannot Convert Quote"),
              tr("<p>Quote #%1 has one or more line items "
                 "without a warehouse specified. These line "
                 "items must be fixed before you may convert "
                 "this quote." )
              .arg(check.value("quhead_number").toString()) );
          return;
        }
        counter++;
      }
      else if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }

    if (counter)
    {
      omfgThis->sQuotesUpdated(-1);
      omfgThis->sSalesOrdersUpdated(-1);
    }

    if (counter == 1)
    {
      salesOrder::editSalesOrder(soheadid, true);
    }
  }
}

void dspCustomerInformation::doDialog(QWidget * parent, const ParameterList & pParams)
{
  QDialog newdlg(parent);
  QVBoxLayout * vbox = new QVBoxLayout(&newdlg);
  dspCustomerInformation * ci = new dspCustomerInformation(&newdlg);
  vbox->addWidget(ci);
  ParameterList params;
  params = pParams;
  params.append("modal");
  ci->set(params);
  newdlg.exec();
}

void dspCustomerInformation::sCreditMemoSelected(bool b)
{
  if(b)
  {
    if(((_creditMemo->altId() == -1 || _creditMemo->altId() == -2) && _privleges->check("MaintainCreditMemos"))
     ||(_creditMemo->altId() == -3 && _privleges->check("EditAROpenItem")))
    {
      _editCreditMemo->setEnabled(true);
      _viewCreditMemo->setEnabled(true);
    }
    else if(((_creditMemo->altId() == -1 || _creditMemo->altId() == -2) && _privleges->check("ViewCreditMemos"))
     ||(_creditMemo->altId() == -3 && _privleges->check("ViewAROpenItems")))
    {
      _editCreditMemo->setEnabled(false);
      _viewCreditMemo->setEnabled(true);
    }
    else
    {
      _editCreditMemo->setEnabled(false);
      _viewCreditMemo->setEnabled(false);
    }
  }
  else
  {
    _editCreditMemo->setEnabled(false);
    _viewCreditMemo->setEnabled(false);
  }
}

void dspCustomerInformation::sARWorkbench()
{
  ParameterList params;
  params.append("cust_id", _cust->id());

  arWorkBench* newdlg = new arWorkBench();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}


void dspCustomerInformation::sCRMAccount()
{
  q.prepare("SELECT crmacct_id FROM crmacct "
            "WHERE (crmacct_cust_id=:cust_id);");
  q.bindValue(":cust_id", _cust->id());
  q.exec();

  if(q.first())
    _crmacctId = q.value("crmacct_id").toInt();

  ParameterList params;
  if (!_privleges->check("MaintainCRMAccounts"))
    params.append("mode", "view");
  else
    params.append("mode", "edit");
    
  params.append("crmacct_id", _crmacctId);

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sDspShipmentStatus()
{
  ParameterList params;
  params.append("sohead_id", _order->id());
  params.append("run");

  dspSalesOrderStatus *newdlg = new dspSalesOrderStatus();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sShipment()
{
  ParameterList params;
  params.append("sohead_id", _order->id());

  dspShipmentsBySalesOrder* newdlg = new dspShipmentsBySalesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sInvShipmentStatus()
{
  q.prepare("SELECT cohead_id "
            "FROM cohead, invchead "
            "WHERE ( (cohead_number=invchead_ordernumber) "
            "AND ( invchead_id=:invoice_id)) ");
  q.bindValue(":invoice_id", _invoice->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("sohead_id", q.value("cohead_id").toInt());
    params.append("run");

    dspSalesOrderStatus *newdlg = new dspSalesOrderStatus();
    newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  }
}

void dspCustomerInformation::sInvShipment()
{
  q.prepare("SELECT cohead_id "
            "FROM cohead, invchead "
            "WHERE ( (cohead_number=invchead_ordernumber) "
            "AND ( invchead_id=:invoice_id)) ");
  q.bindValue(":invoice_id", _invoice->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("sohead_id", q.value("cohead_id").toInt());

    dspShipmentsBySalesOrder* newdlg = new dspShipmentsBySalesOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspCustomerInformation::sPrintSalesOrder()
{
    ParameterList params;
    params.append("sohead_id", _order->id());

    printSoForm newdlgS(this, "", true);
    newdlgS.set(params);
    newdlgS.exec();
}

void dspCustomerInformation::sPrintQuote()
{
  q.prepare( "SELECT findCustomerForm(quhead_cust_id, 'Q') AS reportname "
             "FROM quhead "
             "WHERE (quhead_id=:quheadid); " );
  q.bindValue(":quheadid", _quote->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("quhead_id", _quote->id());

    orReport report(q.value("reportname").toString(), params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
}

void dspCustomerInformation::sPrintInvoice()
{
  ParameterList params;
  params.append("invchead_id", _invoice->id());

  printInvoice newdlg(this, "", TRUE);
  newdlg.set(params);

  if (!newdlg.isSetup())
  {
    newdlg.exec();
    newdlg.setSetup(TRUE);
  }
}

void dspCustomerInformation::sPrintCreditMemo()
{
  int  memoId		= _creditMemo->id();

  q.prepare("SELECT cmhead_id "
            "FROM cmhead "
            "WHERE (cmhead_number=:docnum);");
  q.bindValue(":docnum", _creditMemo->currentItem()->text(3));
  q.exec();
  if (q.first())
    memoId	= q.value("cmhead_id").toInt();
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("cmhead_id", memoId);

  printCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (!newdlg.isSetup())
  {
    newdlg.exec();
    newdlg.setSetup(TRUE);
  }
}

void dspCustomerInformation::sHandleCreditMemoPrint()
{
  _printCreditMemo->setEnabled(_creditMemo->altId() == -1 || _creditMemo->altId() == -2);
}
