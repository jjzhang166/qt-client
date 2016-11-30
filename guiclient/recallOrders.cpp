/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "recallOrders.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>
#include "mqlutil.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

recallOrders::recallOrders(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_query,      SIGNAL(clicked()),                  this, SLOT(sFillList()));
  connect(_recall,	   SIGNAL(clicked()),	                 this, SLOT(sRecall()));
  connect(omfgThis,    SIGNAL(invoicesUpdated(int, bool)), this, SLOT(sFillList()));

  _showInvoiced->setEnabled(_privileges->check("RecallInvoicedShipment"));
  
  _ship->addColumn(tr("Ship Date"),	_dateColumn,  Qt::AlignCenter, true, "shiphead_shipdate" );
  _ship->addColumn(tr("Order #"),	_orderColumn, Qt::AlignLeft  , true, "number");
  _ship->addColumn(tr("Shipment #"),    _orderColumn, Qt::AlignLeft  , true, "shiphead_number");
  _ship->addColumn(tr("Customer"),      -1,           Qt::AlignLeft  , true, "cohead_billtoname" );
  _ship->addColumn(tr("Invoiced"),	_ynColumn,    Qt::AlignCenter, true, "shipitem_invoiced" );

  sFillList();
}

recallOrders::~recallOrders()
{
  // no need to delete child widgets, Qt does it all for us
}

void recallOrders::languageChange()
{
  retranslateUi(this);
}

void recallOrders::sRecall()
{
  XSqlQuery recallRecall;
  if (!checkSitePrivs(_ship->id()))
    return;

  if (_ship->altId() != -1)
  {    
    int answer = QMessageBox::question(this, tr("Purge Invoice?"),
                            tr("<p>There is an unposted Invoice associated with "
                               "this Shipment.  This Invoice will be purged "
                               "as part of the recall process. <p> "
                               "OK to continue? "),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No);
    if (answer == QMessageBox::No)
      return;
  }

  recallRecall.prepare("SELECT recallShipment(:shiphead_id) AS result;");
  recallRecall.bindValue(":shiphead_id", _ship->id());
  recallRecall.exec();
  if (recallRecall.first())
  {
    int result = recallRecall.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Recalling Shipment"),
                             storedProcErrorLookup("recallShipment", result),
                             __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Recalling Shipment"),
                                recallRecall, __FILE__, __LINE__))
  {
    return;
  }
}

void recallOrders::sFillList()
{
  if (_showInvoiced->isChecked())
  {
    if (!_customer->isValid())
    {
      QMessageBox::critical(this, tr("Missing Parameter"),
                            tr("You must select a Customer if Show Invoiced is checked.")) ;
      _customer->setFocus();
      return;
    }
    if (!_dateRange->allValid())
    {
      QMessageBox::critical(this, tr("Missing Parameter"),
                            tr("You must select a Date Range if Show Invoiced is checked.")) ;
      _dateRange->setFocus();
      return;
    }
  }

  MetaSQLQuery mql = mqlLoad("shipments", "recall");
  ParameterList params;

  if (_showInvoiced->isChecked())
    params.append("showInvoiced", true);
  if (_customer->isValid())
    params.append("cust_id", _customer->id());
  if (_dateRange->allValid())
    _dateRange->appendValue(params);
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  XSqlQuery r = mql.toQuery(params);
  _ship->clear();
  _ship->populate(r, true);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Shipment Information"),
                                r, __FILE__, __LINE__))
  {
    return;
  }
}

bool recallOrders::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkShipmentSitePrivs(:shipheadid) AS result;");
    check.bindValue(":shipheadid", orderid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not recall this Shipment as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
