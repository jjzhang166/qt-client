/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "openSalesOrders.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "characteristic.h"
#include "copySalesOrder.h"
#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "errorReporter.h"
#include "issueToShipping.h"
#include "printPackingList.h"
#include "printSoForm.h"
#include "salesOrder.h"
#include "storedProcErrorLookup.h"
#include "parameterwidget.h"

openSalesOrders::openSalesOrders(QWidget* parent, const char*, Qt::WindowFlags fl)
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

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartDate(QDate().currentDate().addDays(-90));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  QString holdSql = QString("SELECT 0 AS code, '%1' AS desc "
                            " UNION SELECT 1, '%2' " 
                            " UNION SELECT 2, '%3' " 
                            " UNION SELECT 3, '%4' " 
                            " UNION SELECT 4, '%5' "
                            " ORDER BY code; ")
          .arg(tr("None"))
          .arg(tr("Credit"))
          .arg(tr("Shipping"))
          .arg(tr("Packing"))
          .arg(tr("Return"));

  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);
  parameterWidget()->append(tr("Customer"), "cust_id", ParameterWidget::Customer);
  parameterWidget()->appendComboBox(tr("Customer Type"), "custtype_id", XComboBox::CustomerTypes);
  parameterWidget()->append(tr("Customer Type Pattern"), "custtype_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Customer P/O Number"), "poNumber", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Sales Rep."), "salesrep_id", XComboBox::SalesRepsActive);
  parameterWidget()->appendComboBox(tr("Sale Type"), "saletype_id", XComboBox::SaleTypes);
  parameterWidget()->append(tr("Project"), "prj_id", ParameterWidget::Project);
  parameterWidget()->append(tr("Created By"), "createdby", ParameterWidget::User);
  parameterWidget()->appendComboBox(tr("Hold Type"), "holdtype", holdSql);

  list()->addColumn(tr("Order #"),          _orderColumn,    Qt::AlignLeft,  true,  "cohead_number");
  list()->addColumn(tr("Sale Type"),        _orderColumn,    Qt::AlignLeft,  true,  "saletype_descr");
  list()->addColumn(tr("Cust. #"),          _orderColumn,    Qt::AlignLeft,  true,  "cust_number");
  list()->addColumn(tr("Customer"),         _itemColumn,     Qt::AlignLeft,  true,  "cohead_billtoname");
  list()->addColumn(tr("Ship-To"),          _itemColumn,     Qt::AlignLeft,  false, "cohead_shiptoname");
  list()->addColumn(tr("Cust. P/O Number"), _orderColumn,    Qt::AlignLeft,  true,  "cohead_custponumber");
  list()->addColumn(tr("Ordered"),          _dateColumn,     Qt::AlignCenter,true,  "cohead_orderdate");
  list()->addColumn(tr("Scheduled"),        _dateColumn,     Qt::AlignCenter,true,  "scheddate");
  list()->addColumn(tr("Total"),            _moneyColumn,    Qt::AlignRight, true,  "ordertotal");
  if (_privileges->check("ShowMarginsOnSalesOrder"))
  {
    list()->addColumn(tr("Margin"),         _moneyColumn,    Qt::AlignRight, true,  "ordermargin");
    list()->addColumn(tr("Margin %"),       _prcntColumn,    Qt::AlignRight, true,  "ordermarginpercent");
  }
  list()->addColumn(tr("Status"),           _statusColumn,   Qt::AlignCenter,false, "status");
  list()->addColumn(tr("Hold Type"),        -1,              Qt::AlignLeft  ,false, "holdtype");
  list()->addColumn(tr("Notes"),            -1,              Qt::AlignLeft,  false, "notes");
  
  setupCharacteristics("SO");
  parameterWidget()->applyDefaultFilterSet();
  
  if (_privileges->check("MaintainSalesOrders"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList()));
  connect(_showClosed, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
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

  if (_dates->isVisible())
  {
    if (!_dates->startDate().isValid())
    {
      QMessageBox::critical( this, tr("Enter Start Date"),
                             tr("You must enter a valid Start Date.") );
      _dates->setFocus();
      return false;
    }

    if (!_dates->endDate().isValid())
    {
      QMessageBox::critical( this, tr("Enter End Date"),
                             tr("You must enter a valid End Date.") );
      _dates->setFocus();
      return false;
    }
    _dates->appendValue(params);
  }

  params.append("error", tr("Error"));
  if (_showClosed->isChecked() && _showClosed->isVisible())
    params.append("showClosed");

  params.append("none",   tr("None"));
  params.append("credit", tr("Credit"));
  params.append("ship",   tr("Shipping"));
  params.append("pack",   tr("Packing"));
  params.append("return", tr("Return"));
   
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
      
  copySalesOrder newdlg(this, "", true);
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

  printPackingList newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void openSalesOrders::sAddToPackingListBatch()
{
  XSqlQuery openAddToPackingListBatch;
  if (!checkSitePrivs())
    return;
    
  openAddToPackingListBatch.prepare("SELECT addToPackingListBatch(:sohead_id) AS result;");
  openAddToPackingListBatch.bindValue(":sohead_id", list()->id());
  openAddToPackingListBatch.exec();
  if (openAddToPackingListBatch.first())
  {
    int result = openAddToPackingListBatch.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Packing List Information"),
                             storedProcErrorLookup("addToPackingListBatch", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Packing List Information"),
                                openAddToPackingListBatch, __FILE__, __LINE__))
  {
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

  printSoForm newdlg(this, "", true);
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
