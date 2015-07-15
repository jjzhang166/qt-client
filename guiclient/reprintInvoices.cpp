/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reprintInvoices.h"

#include <mqlutil.h>

#include "errorReporter.h"

reprintInvoices::reprintInvoices(QWidget    *parent,
                                 const char *name,
                                 bool        modal,
                                 Qt::WindowFlags  fl)
    : reprintMulticopyDocument("InvoiceCopies",     "InvoiceWatermark",
                               "InvoiceShowPrices",
                               parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  list()->addColumn(tr("Invoice #"),      _orderColumn, Qt::AlignRight, true, "docnumber");
  list()->addColumn(tr("Doc. Date"),       _dateColumn, Qt::AlignCenter,true, "invchead_invcdate");
  list()->addColumn(tr("Cust. #"),                  -1, Qt::AlignLeft,  true, "cust_number");
  list()->addColumn(tr("Customer"),                 -1, Qt::AlignLeft,  true, "cust_name");
  list()->addColumn(tr("Total Amount"),_bigMoneyColumn, Qt::AlignRight, true, "extprice" );
  list()->addColumn(tr("Balance"),     _bigMoneyColumn, Qt::AlignRight, true, "balance" );
  list()->addColumn(tr("Report"),                   -1, Qt::AlignLeft,  false,"reportname");

  QString      errmsg;
  bool         ok  = false;
  MetaSQLQuery mql = MQLUtil::mqlLoad("invoices", "detail", errmsg, &ok);
  if (! ok)
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Invoices"),
                         errmsg, __FILE__, __LINE__);

  setDoctype("IN");
  setReportKey("invchead_id");
  _docListQueryString = mql.getSource();
}

reprintInvoices::~reprintInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void reprintInvoices::languageChange()
{
  retranslateUi(this);
}

ParameterList reprintInvoices::getParamsDocList()
{
  ParameterList params;
  params.append("invc_pattern", _invoicePattern->text().trimmed());
  params.append("getForm");

  if (_dates->allValid())
    _dates->appendValue(params);

  if (_balanceOnly->isChecked())
    params.append("balanceOnly");

  return params;
}
