/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInvoiceInformation.h"

#include <QSqlError>
#include <QVariant>

#include "invoice.h"

#include <openreports.h>
#include <invoiceList.h>
#include <metasql.h>
#include "mqlutil.h"
#include "errorReporter.h"


dspInvoiceInformation::dspInvoiceInformation(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_invoiceNumber, SIGNAL(newInvoiceNumber(QString)), this, SLOT(sParseInvoiceNumber()));
  connect(_invoiceList, SIGNAL(clicked()), this, SLOT(sInvoiceList()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sViewDetails()));

  _invoiceAmount->setPrecision(omfgThis->moneyVal());

#ifndef Q_OS_MAC
  _invoiceList->setMaximumWidth(25);
#endif

  _cust->setReadOnly(true);

  _arapply->addColumn(tr("Type"),            _dateColumn, Qt::AlignCenter,true, "doctype");
  _arapply->addColumn(tr("Doc./Ref. #"),              -1, Qt::AlignLeft,  true, "docnumber");
  _arapply->addColumn(tr("Apply Date"),      _dateColumn, Qt::AlignCenter,true, "arapply_postdate");
  _arapply->addColumn(tr("Amount"),         _moneyColumn, Qt::AlignRight, true, "arapply_applied");
  _arapply->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency(), "currabbr");
  _arapply->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight, true, "baseapplied");

  _invcheadid = -1;
}

dspInvoiceInformation::~dspInvoiceInformation()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInvoiceInformation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInvoiceInformation::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("invoiceNumber", &valid);
  if (valid)
  {
    _invoiceNumber->setInvoiceNumber(param.toString());
    _invoiceNumber->setEnabled(false);
    _invoiceList->hide();
  }

  return NoError;
}

void dspInvoiceInformation::sParseInvoiceNumber()
{
  XSqlQuery dspParseInvoiceNumber;
  dspParseInvoiceNumber.prepare( "SELECT invchead_id, invchead_cust_id, invchead_ponumber,"
             "       invchead_shipdate, invchead_invcdate,"
             "       invoiceTotal(invchead_id) AS amount,"
             "       invchead_billto_name, invchead_billto_address1,"
             "       invchead_billto_address2, invchead_billto_address3,"
             "       invchead_billto_city, invchead_billto_state, invchead_billto_zipcode,"
             "       invchead_shipto_name, invchead_shipto_address1,"
             "       invchead_shipto_address2, invchead_shipto_address3,"
             "       invchead_shipto_city, invchead_shipto_state, invchead_shipto_zipcode,"
             "       invchead_notes "
             "FROM invchead LEFT OUTER JOIN"
             "     ( invcitem LEFT OUTER JOIN item"
             "       ON (invcitem_item_id=item_id) )"
             "     ON (invcitem_invchead_id=invchead_id) "
             "WHERE (invchead_invcnumber=:invoiceNumber) "
             "GROUP BY invchead_id, invchead_cust_id, invchead_ponumber,"
             "         invchead_shipdate, invchead_invcdate,"
             "         invchead_misc_amount, invchead_freight,"
             "         invchead_billto_name, invchead_billto_address1,"
             "         invchead_billto_address2, invchead_billto_address3,"
             "         invchead_billto_city, invchead_billto_state, invchead_billto_zipcode,"
             "         invchead_shipto_name, invchead_shipto_address1,"
             "         invchead_shipto_address2, invchead_shipto_address3,"
             "         invchead_shipto_city, invchead_shipto_state, invchead_shipto_zipcode,"
             "         invchead_notes;" );
  dspParseInvoiceNumber.bindValue(":invoiceNumber", _invoiceNumber->invoiceNumber());
  dspParseInvoiceNumber.exec();
  if (dspParseInvoiceNumber.first())
  {
    _print->setEnabled(true);
    _view->setEnabled(true);

    _invcheadid = dspParseInvoiceNumber.value("invchead_id").toInt();

    _custPoNumber->setText(dspParseInvoiceNumber.value("invchead_ponumber").toString());
    _cust->setId(dspParseInvoiceNumber.value("invchead_cust_id").toInt());
    _invoiceDate->setDate(dspParseInvoiceNumber.value("invchead_invcdate").toDate());
    _shipDate->setDate(dspParseInvoiceNumber.value("invchead_shipdate").toDate());
    _invoiceAmount->setDouble(dspParseInvoiceNumber.value("amount").toDouble());

    _billToName->setText(dspParseInvoiceNumber.value("invchead_billto_name"));
    _billToAddress1->setText(dspParseInvoiceNumber.value("invchead_billto_address1"));
    _billToAddress2->setText(dspParseInvoiceNumber.value("invchead_billto_address2"));
    _billToAddress3->setText(dspParseInvoiceNumber.value("invchead_billto_address3"));
    _billToCity->setText(dspParseInvoiceNumber.value("invchead_billto_city"));
    _billToState->setText(dspParseInvoiceNumber.value("invchead_billto_state"));
    _billToZip->setText(dspParseInvoiceNumber.value("invchead_billto_zipcode"));

    _shipToName->setText(dspParseInvoiceNumber.value("invchead_shipto_name"));
    _shipToAddress1->setText(dspParseInvoiceNumber.value("invchead_shipto_address1"));
    _shipToAddress2->setText(dspParseInvoiceNumber.value("invchead_shipto_address2"));
    _shipToAddress3->setText(dspParseInvoiceNumber.value("invchead_shipto_address3"));
    _shipToCity->setText(dspParseInvoiceNumber.value("invchead_shipto_city"));
    _shipToState->setText(dspParseInvoiceNumber.value("invchead_shipto_state"));
    _shipToZip->setText(dspParseInvoiceNumber.value("invchead_shipto_zipcode"));

    _notes->setText(dspParseInvoiceNumber.value("invchead_notes").toString());

    MetaSQLQuery mql = mqlLoad("invoiceInformation", "detail");
    ParameterList params;
    if (! setParams(params))
      return;

    dspParseInvoiceNumber = mql.toQuery(params);
    _arapply->populate(dspParseInvoiceNumber);
  }
  else
  {
    if (dspParseInvoiceNumber.lastError().type() != QSqlError::NoError)
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Invoice Information"),
                         dspParseInvoiceNumber, __FILE__, __LINE__);
    _print->setEnabled(false);
    _view->setEnabled(false);
    _invoiceNumber->clear();
    _arapply->clear();
    _invcheadid = -1;
  }
}

bool dspInvoiceInformation::setParams(ParameterList &params)
{
  params.append("creditMemo", tr("C/M"));
  params.append("cashdeposit", tr("Cash Deposit"));
  params.append("error", tr("Error"));
  params.append("check", tr("Check"));
  params.append("certifiedCheck", tr("Certified Check"));
  params.append("masterCard", tr("Master Card"));
  params.append("visa", tr("Visa"));
  params.append("americanExpress", tr("American Express"));
  params.append("discoverCard", tr("Discover Card"));
  params.append("otherCreditCard", tr("Other Credit Card"));
  params.append("cash", tr("Cash"));
  params.append("wireTransfer", tr("Wire Transfer"));
  params.append("other", tr("ther"));
 
  params.append("aropen_docnumber", _invoiceNumber->invoiceNumber());

  return true;
}

void dspInvoiceInformation::sPrint()
{
  ParameterList params;
  params.append("invchead_id", _invcheadid);

  orReport report("InvoiceInformation", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInvoiceInformation::sViewDetails()
{
  invoice::viewInvoice(_invcheadid);
}

void dspInvoiceInformation::sInvoiceList()
{
  ParameterList params;
  params.append("invoiceNumber", _invoiceNumber->invoiceNumber());

  invoiceList newdlg(this, "", true);
  newdlg.set(params);
  int invoiceid = newdlg.exec();

  if (invoiceid > 0)
    _invoiceNumber->setId(invoiceid);
}

