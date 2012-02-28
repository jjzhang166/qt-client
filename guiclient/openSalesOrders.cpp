/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "openSalesOrders.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "copySalesOrder.h"
#include "creditcardprocessor.h"
#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "errorReporter.h"
#include "issueToShipping.h"
#include "printPackingList.h"
#include "printSoForm.h"
#include "salesOrder.h"
#include "storedProcErrorLookup.h"
#include "parameterwidget.h"

openSalesOrders::openSalesOrders(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "openSalesOrders", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Open Sales Orders"));
  setMetaSQLOptions("opensalesorders", "detail");
  setReportName("ListOpenSalesOrders");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setQueryOnStartEnabled(true);
  setAutoUpdateEnabled(true);

  _custid = -1;
  optionsWidget()->hide();

  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);
  parameterWidget()->append(tr("Customer"), "cust_id", ParameterWidget::Customer);
  parameterWidget()->appendComboBox(tr("Customer Type"), "custtype_id", XComboBox::CustomerTypes);
  parameterWidget()->append(tr("Customer Type Pattern"), "custtype_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("P/O Number"), "poNumber", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Sales Rep."), "salesrep_id", XComboBox::SalesRepsActive);

  list()->addColumn(tr("Order #"),         _orderColumn, Qt::AlignLeft,  true, "cohead_number");
  list()->addColumn(tr("Cust. #"),         _orderColumn, Qt::AlignLeft,  true, "cust_number");
  list()->addColumn(tr("Customer"),         -1,          Qt::AlignLeft,  true, "cohead_billtoname");
  list()->addColumn(tr("Ship-To"),          _itemColumn, Qt::AlignLeft,  false,"cohead_shiptoname");
  list()->addColumn(tr("Cust. P/O Number"), -1         , Qt::AlignLeft,  true, "cohead_custponumber");
  list()->addColumn(tr("Ordered"),          _dateColumn, Qt::AlignCenter,true, "cohead_orderdate");
  list()->addColumn(tr("Scheduled"),        _dateColumn, Qt::AlignCenter,true, "scheddate");
  list()->addColumn(tr("Status"),         _statusColumn, Qt::AlignLeft, false, "status");
  list()->addColumn(tr("Notes"),          -1, Qt::AlignLeft, false, "notes");
  
  if (_privileges->check("MaintainSalesOrders"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

enum SetResponse openSalesOrders::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool	   valid;
  
  param = pParams.value("run", &valid);
  if (valid)
    sFillList();

  return NoError;
}

bool openSalesOrders::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  params.append("error", tr("Error"));
  if (_showClosed->isChecked() && _showClosed->isVisible())
    params.append("showClosed");

  return true;
}

void openSalesOrders::sNew()
{
  salesOrder::newSalesOrder(_custid, this);
}

void openSalesOrders::sEdit()
{
  if (checkSitePrivs())
    salesOrder::editSalesOrder(list()->id(), false, this);
}

void openSalesOrders::sView()
{
  if (checkSitePrivs())
    salesOrder::viewSalesOrder(list()->id(), this);
}

void openSalesOrders::sCopy()
{
  if (!checkSitePrivs())
    return;
    
  ParameterList params;
  params.append("sohead_id", list()->id());
      
  copySalesOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void openSalesOrders::sDelete()
{
  (void)salesOrder::deleteSalesOrder(list()->id());
}

void openSalesOrders::sPrintPackingList()
{
  if (!checkSitePrivs())
    return;
    
  ParameterList params;
  params.append("sohead_id", list()->id());

  printPackingList newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void openSalesOrders::sAddToPackingListBatch()
{
  if (!checkSitePrivs())
    return;
    
  q.prepare("SELECT addToPackingListBatch(:sohead_id) AS result;");
  q.bindValue(":sohead_id", list()->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("addToPackingListBatch", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void openSalesOrders::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Copy..."), this, SLOT(sCopy()));
  menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Print Packing List..."), this, SLOT(sPrintPackingList()));
  menuItem->setEnabled(_privileges->check("PrintPackingLists"));

  menuItem = pMenu->addAction(tr("Add to Packing List Batch..."), this, SLOT(sAddToPackingListBatch()));
  menuItem->setEnabled(_privileges->check("MaintainPackingListBatch"));

  menuItem = pMenu->addAction(tr("Print Sales Order Form..."), this, SLOT(sPrintForms())); 
  
  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Issue to Shipping..."), this, SLOT(sIssueToShipping()));
  menuItem->setEnabled(_privileges->check("IssueStockToShipping"));

  pMenu->addAction(tr("Shipment Status..."), this, SLOT(sDspShipmentStatus()));
  pMenu->addAction(tr("Shipments..."), this, SLOT(sShipment()));
}

void openSalesOrders::sPrintForms()
{
  if (!checkSitePrivs())
    return;
    
  ParameterList params;
  params.append("sohead_id", list()->id());

  printSoForm newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

bool openSalesOrders::checkSitePrivs()
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkSOSitePrivs(:coheadid) AS result;");
    check.bindValue(":coheadid", list()->id());
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

void openSalesOrders::sIssueToShipping()
{
  ParameterList params;
  params.append("sohead_id", list()->id());
  params.append("run");

  issueToShipping *newdlg = new issueToShipping(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void openSalesOrders::sDspShipmentStatus()
{
  ParameterList params;
  params.append("sohead_id", list()->id());
  params.append("run");

  dspSalesOrderStatus *newdlg = new dspSalesOrderStatus(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void openSalesOrders::sShipment()
{
  ParameterList params;
  params.append("sohead_id", list()->id());

  dspShipmentsBySalesOrder* newdlg = new dspShipmentsBySalesOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void openSalesOrders::setCustId(int custId)
{
  _custid = custId;
  parameterWidget()->setDefault(tr("Customer"), custId, true );
}

int openSalesOrders::custId()
{
  return _custid;
}
