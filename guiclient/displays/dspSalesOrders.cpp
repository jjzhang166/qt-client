/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesOrders.h"

#include <QMessageBox>
#include <QVariant>

#include "copySalesOrder.h"
#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "returnAuthorization.h"
#include "salesOrder.h"
#include "parameterwidget.h"

dspSalesOrders::dspSalesOrders(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspSalesOrders", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Sales Orders"));
  setListLabel(tr("Sales Orders"));
  setMetaSQLOptions("salesOrders", "detail");
  /* setReportName("ListSalesOrders");  */
  setParameterWidgetVisible(true);
  if(_privileges->check("MaintainSalesOrders"))
    setNewVisible(true);
  setQueryOnStartEnabled(false);
  setAutoUpdateEnabled(true);

  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);
  parameterWidget()->append(tr("Customer"), "cust_id", ParameterWidget::Customer);
  parameterWidget()->appendComboBox(tr("Customer Type"), "custtype_id", XComboBox::CustomerTypes);
  parameterWidget()->append(tr("Customer Type Pattern"), "custtype_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("End Date"),   "endDate",   ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("P/O Number"), "poNumber", ParameterWidget::Text);
  parameterWidget()->append(tr("Project"), "prj_id", ParameterWidget::Project);
  parameterWidget()->appendComboBox(tr("Sales Rep."), "salesrep_id", XComboBox::SalesRepsActive);
  parameterWidget()->appendComboBox(tr("Sale Type"), "saletype_id", XComboBox::SaleTypes);
  parameterWidget()->append(tr("Hide Closed"), "hideClosed", ParameterWidget::Exists);

  list()->addColumn(tr("Customer"),    _itemColumn,  Qt::AlignLeft,   true,  "cust_number"   );
  list()->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft,   true,  "cohead_number"   );
  list()->addColumn(tr("Sale Type"),   _orderColumn, Qt::AlignLeft,   true,  "saletype_descr"   );
  list()->addColumn(tr("Ordered"),     _dateColumn,  Qt::AlignRight,  true,  "cohead_orderdate"  );
  list()->addColumn(tr("Scheduled"),   _dateColumn,  Qt::AlignRight,  true,  "min_scheddate"  );
  list()->addColumn(tr("Status"),      _itemColumn,  Qt::AlignCenter, true,  "order_status" );
  list()->addColumn(tr("Ship-to"),     -1,           Qt::AlignLeft,   true,  "cohead_shiptoname"   );
  list()->addColumn(tr("Cust. P/O #"), 200,          Qt::AlignLeft,   true,  "cohead_custponumber"   );

  parameterWidget()->applyDefaultFilterSet();
  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())  );
}

void dspSalesOrders::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspSalesOrders::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  if(list()->id() == -1)
    return;

  if(_privileges->check("MaintainSalesOrders"))
    menuThis->addAction(tr("Edit..."), this, SLOT(sEditOrder()));
  menuThis->addAction(tr("View..."), this, SLOT(sViewOrder()));
  if(_privileges->check("MaintainSalesOrders"))
  {
    menuThis->addSeparator();
    menuThis->addAction(tr("Copy..."), this, SLOT(sCopyOrder()));
  }
  menuThis->addSeparator();
  menuThis->addAction(tr("Shipment Status..."), this, SLOT(sDspShipmentStatus()));
  menuThis->addAction(tr("Shipments..."), this, SLOT(sDspShipments()));

  if ( (_metrics->boolean("EnableReturnAuth")) && (_privileges->check("MaintainReturns")) )
  {
    menuThis->addSeparator();
    menuThis->addAction(tr("Create Return Authorization..."), this, SLOT(sCreateRA()));
  }
}

bool dspSalesOrders::setParams(ParameterList & params)
{
  parameterWidget()->appendValue(params);
  params.append("noLines", tr("No Lines"));
  params.append("closed", tr("Closed"));
  params.append("open", tr("Open"));
  params.append("partial", tr("Partial"));

  return true;
}

void dspSalesOrders::sNew()
{
  salesOrder::newSalesOrder(-1, this);
}

void dspSalesOrders::sEditOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  salesOrder::editSalesOrder(list()->id(), false);
}

void dspSalesOrders::sViewOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  salesOrder::viewSalesOrder(list()->id());
}

void dspSalesOrders::sCopyOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  ParameterList params;
  params.append("sohead_id", list()->id());
      
  copySalesOrder newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspSalesOrders::sCreateRA()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  ParameterList params;
  params.append("mode", "new");
  params.append("sohead_id", list()->id());

  returnAuthorization *newdlg = new returnAuthorization();
  if (newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
  else
    QMessageBox::critical(this, tr("Could Not Open Window"),
			  tr("The new Return Authorization could not be created"));
}

void dspSalesOrders::sDspShipmentStatus()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  ParameterList params;
  params.append("sohead_id", list()->id());
  params.append("run");

  dspSalesOrderStatus *newdlg = new dspSalesOrderStatus();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSalesOrders::sDspShipments()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  ParameterList params;
  params.append("sohead_id", list()->id());
  params.append("run");

  dspShipmentsBySalesOrder *newdlg = new dspShipmentsBySalesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspSalesOrders::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkSOSitePrivs(:coheadid) AS result;");
    check.bindValue(":coheadid", orderid);
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

