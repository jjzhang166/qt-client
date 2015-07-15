/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspQuotesByCustomer.h"

#include <QMenu>
#include <QMessageBox>

#include "errorReporter.h"
#include "salesOrder.h"
#include "storedProcErrorLookup.h"

dspQuotesByCustomer::dspQuotesByCustomer(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspQuotesByCustomer", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Quote Lookup by Customer"));
  setListLabel(tr("Quotes"));
  setMetaSQLOptions("quotes", "detail");

  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulatePo()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Starting Order Date"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Ending Order Date"));

  list()->addColumn(tr("Quote #"),     _orderColumn, Qt::AlignLeft,   true,  "quhead_number"   );
  list()->addColumn(tr("Quote Date"),  _dateColumn,  Qt::AlignRight,  true,  "quhead_quotedate"  );
  list()->addColumn(tr("Ship-to"),     -1,           Qt::AlignLeft,   true,  "quhead_shiptoname"   );
  list()->addColumn(tr("Cust. P/O #"), 200,          Qt::AlignLeft,   true,  "quhead_custponumber"   );
  list()->addColumn(tr("Status"),     _statusColumn,  Qt::AlignCenter, true,  "quhead_status" );

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())  );
}

void dspQuotesByCustomer::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspQuotesByCustomer::sPopulatePo()
{
  _poNumber->clear();

  if ((_cust->isValid()) && (_dates->allValid()))
  {
    XSqlQuery minq;
    minq.prepare("SELECT MIN(quhead_id), quhead_custponumber "
                 "FROM quhead "
                 "WHERE ( (quhead_cust_id=:cust_id)"
                 " AND (quhead_quotedate BETWEEN :startDate AND :endDate) ) "
                 "GROUP BY quhead_custponumber "
                 "ORDER BY quhead_custponumber;" );
    _dates->bindValue(minq);
    minq.bindValue(":cust_id", _cust->id());
    minq.exec();
    _poNumber->populate(minq);
  }
}

void dspQuotesByCustomer::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  menuThis->addAction(tr("Edit..."), this, SLOT(sEditOrder()));
  menuThis->addAction(tr("View..."), this, SLOT(sViewOrder()));

  if (_privileges->check("ConvertQuotes"))
  {
    menuThis->addSeparator();
    menuThis->addAction(tr("Convert..."), this, SLOT(sConvert()));
  }
}

void dspQuotesByCustomer::sEditOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;

  ParameterList params;
  params.append("mode", "editQuote");
  params.append("quhead_id", list()->id());

  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQuotesByCustomer::sViewOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;

  ParameterList params;
  params.append("mode", "viewQuote");
  params.append("quhead_id", list()->id());

  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspQuotesByCustomer::setParams(ParameterList & params)
{
  if (! _dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Dates"),
                         tr("Enter valid Start and End dates."));
    _dates->setFocus();
    return false;
  }

  if (_selectedPO->isChecked() && ! _poNumber->isValid())
  {
    QMessageBox::warning(this, tr("Invalid Options"),
      tr("One or more options are not complete."));
    return false;
  }

  _dates->appendValue(params);
  params.append("cust_id", _cust->id());
  params.append("showExpired");
  params.append("customersOnly");
  if (_selectedPO->isChecked())
    params.append("poNumber", _poNumber->currentText());
  if (_showConverted->isChecked())
    params.append("showConverted", true);

  return true;
}

void dspQuotesByCustomer::sConvert()
{
  if (QMessageBox::question(this, tr("Convert Selected Quote(s)"),
                            tr("<p>Are you sure that you want to convert the "
                               "selected Quote(s) to Sales Order(s)?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery check;
    check.prepare("SELECT quhead_number, quhead_status, cust_creditstatus "
                  " FROM quhead"
                  "  JOIN custinfo ON (quhead_cust_id=cust_id)"
                  " WHERE (quhead_id=:quhead_id);" );

    XSqlQuery convert;
    convert.prepare("SELECT convertQuote(:quhead_id) AS sohead_id;");

    int counter = 0;
    int soheadid = -1;

    foreach (XTreeWidgetItem *cursor, list()->selectedItems())
    {
      if (checkSitePrivs(cursor->id()))
      {
        check.bindValue(":quhead_id", cursor->id());
        check.exec();
        if (check.first())
        {
          // TODO: add this check to convertquote
          if (check.value("quhead_status").toString() == "C")
          {
            QMessageBox::warning(this, tr("Cannot Convert Quote"),
                                 tr("<p>Quote #%1 has already been converted "
                                    "to a Sales Order." )
                            .arg(cursor->text("quhead_number")));
            return;
          }
        }
        else if (ErrorReporter::error(QtCriticalMsg, this,
                                      tr("Error Getting Quote"),
                                      check, __FILE__, __LINE__))
          continue;

        convert.bindValue(":quhead_id", cursor->id());
        convert.exec();
        if (convert.first())
        {
          soheadid = convert.value("sohead_id").toInt();
          if(soheadid < 0)
          {
            QMessageBox::warning(this, tr("Cannot Convert Quote"),
                                 storedProcErrorLookup("convertQuote", soheadid)
                                 .arg(check.value("quhead_number").toString()));
            continue;
          }
          counter++;
          omfgThis->sSalesOrdersUpdated(soheadid);
        }
        else if (ErrorReporter::error(QtCriticalMsg, this,
                                      tr("Converting Error"),
                                      convert, __FILE__, __LINE__))
          continue;
      }
    }

    if (counter)
      omfgThis->sQuotesUpdated(-1);

    if (counter == 1)
      salesOrder::editSalesOrder(soheadid, true);
  }
}

bool dspQuotesByCustomer::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkQuoteSitePrivs(:quheadid) AS result;");
    check.bindValue(":quheadid", orderid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view, edit, or convert this Quote as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
