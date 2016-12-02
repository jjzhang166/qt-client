/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspOrderActivityByProject.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>
#include <QToolBar>
#include <QToolButton>
#include <metasql.h>

#include "guiclient.h"
#include "project.h"
#include "task.h"
#include "salesOrder.h"
#include "salesOrderItem.h"
#include "invoice.h"
#include "invoiceItem.h"
#include "workOrder.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "purchaseOrderItem.h"

dspOrderActivityByProject::dspOrderActivityByProject(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspOrderActivityByProject", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Order Activity by Project"));
  setListLabel(tr("Orders"));
  setReportName("OrderActivityByProject");
  setMetaSQLOptions("orderActivityByProject", "detail");
  setUseAltId(true);

  _run = false;

  list()->addColumn(tr("Name"),        _itemColumn,  Qt::AlignLeft,   true,  "name"   );
  list()->addColumn(tr("Status"),      _orderColumn, Qt::AlignLeft,   true,  "status"   );
  list()->addColumn(tr("Item #"),      _itemColumn,  Qt::AlignLeft,   true,  "item"   );
  list()->addColumn(tr("Description"), -1          , Qt::AlignLeft,   true,  "descrip" );
  list()->addColumn(tr("Account/Customer"), -1          , Qt::AlignLeft,   true,  "customer" );
  list()->addColumn(tr("Contact"), -1          , Qt::AlignLeft,   false,  "contact" );
  list()->addColumn(tr("City"), -1          , Qt::AlignLeft,   false,  "city" );
  list()->addColumn(tr("State"), -1          , Qt::AlignLeft,   false,  "state" );
  list()->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight,  true,  "qty"  );
  list()->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignLeft,   true,  "uom"  );
  list()->addColumn(tr("Value"),      _qtyColumn,   Qt::AlignRight,  true,  "value"  );

  list()->addColumn(tr("Due Date"),      _dateColumn,   Qt::AlignRight,  true,  "due"  );
  list()->addColumn(tr("Assigned"),      _dateColumn,   Qt::AlignRight,  true,  "assigned"  );
  list()->addColumn(tr("Started"),      _dateColumn,   Qt::AlignRight,  true,  "started"  );
  list()->addColumn(tr("Completed"),      _dateColumn,   Qt::AlignRight,  true,  "completed"  );
  list()->addColumn(tr("Hrs. Budget"),      _qtyColumn,   Qt::AlignRight,  true,  "hrs_budget"  );
  list()->addColumn(tr("Hrs. Actual"),      _qtyColumn,   Qt::AlignRight,  true,  "hrs_actual"  );
  list()->addColumn(tr("Hrs. Balance"),      _qtyColumn,   Qt::AlignRight,  true,  "hrs_balance"  );
  list()->addColumn(tr("Exp. Budget"),      _priceColumn,   Qt::AlignRight,  true,  "exp_budget"  );
  list()->addColumn(tr("Exp. Actual"),      _priceColumn,   Qt::AlignRight,  true,  "exp_actual"  );
  list()->addColumn(tr("Exp. Balance"),      _priceColumn,   Qt::AlignRight,  true,  "exp_balance"  );

  list()->setPopulateLinear(true);

  disconnect(newAction(), SIGNAL(triggered()), this, SLOT(sNew()));
  connect(newAction(), SIGNAL(triggered()), this, SLOT(sNewProjectTask()));
  connect(_project,   SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_showSo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showPo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showWo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  QToolButton * newBtn = (QToolButton*)toolBar()->widgetForAction(newAction());
  newBtn->setPopupMode(QToolButton::MenuButtonPopup);
  QAction *menuItem;
  QMenu * newMenu = new QMenu;
  menuItem = newMenu->addAction(tr("Task"), this, SLOT(sNewProjectTask()));
  menuItem->setEnabled(_privileges->check("MaintainAllProjects"));
  newMenu->addSeparator();
  menuItem = newMenu->addAction(tr("Sales Order"), this, SLOT(sNewSalesOrder()));
  menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));
  menuItem = newMenu->addAction(tr("Purchase Order"),   this, SLOT(sNewPurchaseOrder()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  menuItem = newMenu->addAction(tr("Work Order"),   this, SLOT(sNewWorkOrder()));
  menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));
  newBtn->setMenu(newMenu);

}

void dspOrderActivityByProject::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspOrderActivityByProject::set(const ParameterList &pParams)	
{
  XWidget::set(pParams);

  QVariant param;
  bool     valid;

  param = pParams.value("prj_id", &valid);
  if (valid)
    _project->setId(param.toInt());

  if (pParams.inList("run"))
  {
    _run = true;
    return NoError_Run;
  }

  return NoError;
}

void dspOrderActivityByProject::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if(list()->altId() == 5)
  {
    menuItem = pMenu->addAction(tr("Edit Task..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainAllProjects") || _privileges->check("MaintainPersonalProjects"));

    menuItem = pMenu->addAction(tr("View Task..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainAllProjects") ||
			 _privileges->check("MaintainPersonalProjects") ||
                         _privileges->check("ViewAllProjects")  ||
			 _privileges->check("ViewPersonalProjects"));

  }

  if(list()->altId() == 15)
  {
    menuItem = pMenu->addAction(tr("Edit Quote..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes"));

    menuItem = pMenu->addAction(tr("View Quote..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes") ||
                         _privileges->check("ViewQuotes") );
  }

  if(list()->altId() == 17)
  {
    menuItem = pMenu->addAction(tr("Edit Quote Item..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes"));

    menuItem = pMenu->addAction(tr("View Quote Item..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes") ||
                         _privileges->check("ViewQuotes"));
  }

  if(list()->altId() == 25)
  {
    menuItem = pMenu->addAction(tr("Edit Sales Order..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

    menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") ||
                         _privileges->check("ViewSalesOrders"));
  }

  if(list()->altId() == 27)
  {
    menuItem = pMenu->addAction(tr("Edit Sales Order Item..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

    menuItem = pMenu->addAction(tr("View Sales Order Item..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") ||
                         _privileges->check("ViewSalesOrders"));
  }

  if(list()->altId() == 35)
  {
    if (list()->rawValue("status") != "Posted")
    {
      menuItem = pMenu->addAction(tr("Edit Invoice..."), this, SLOT(sEdit()));
      menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));
    }

    menuItem = pMenu->addAction(tr("View Invoice..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices") ||
                         _privileges->check("ViewMiscInvoices"));
  }

  if(list()->altId() == 37)
  {
    if (list()->rawValue("status") != "Posted")
    {
      menuItem = pMenu->addAction(tr("Edit Invoice Item..."), this, SLOT(sEdit()));
      menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));
    }

    menuItem = pMenu->addAction(tr("View Invoice Item..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices") ||
                         _privileges->check("ViewMiscInvoices"));
  }

  if(list()->altId() == 45)
  {
    menuItem = pMenu->addAction(tr("Edit Work Order..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));

    menuItem = pMenu->addAction(tr("View Work Order..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainWorkOrders") ||
                         _privileges->check("ViewWorkOrders"));
  }

  if(list()->altId() == 55)
  {
    menuItem = pMenu->addAction(tr("View Purchase Request..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseRequests") ||
                         _privileges->check("ViewPurchaseRequests"));
  }

  if(list()->altId() == 65)
  {
    menuItem = pMenu->addAction(tr("Edit Purchase Order..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

    menuItem = pMenu->addAction(tr("View Purchase Order..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                         _privileges->check("ViewPurchaseOrders"));
  }

  if(list()->altId() == 67)
  {
    menuItem = pMenu->addAction(tr("Edit Purchase Order Item..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

    menuItem = pMenu->addAction(tr("View Purchase Order Item..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                         _privileges->check("ViewPurchaseOrders"));
  }

}

void dspOrderActivityByProject::sEdit()
{
  ParameterList params;

  if(list()->altId() == 5)
  {
    params.append("mode", "edit");
    params.append("prjtask_id", list()->id());

    task newdlg(this, "", true);
    newdlg.set(params);
    if (newdlg.exec() != XDialog::Rejected)
     sFillList();
  }
  else if(list()->altId() == 15)
  {
    params.append("mode", "editQuote");
    params.append("quhead_id", list()->id());

    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 17)
  {
    params.append("mode", "editQuote");
    params.append("soitem_id", list()->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 25)
  {
    params.append("mode",      "edit");
    params.append("sohead_id", list()->id());
    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
  }
  else if(list()->altId() == 27)
  {
    params.append("mode", "edit");
    params.append("soitem_id", list()->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 35)
  {
    invoice::editInvoice(list()->id(), this);
  }
  else if(list()->altId() == 37)
  {
    params.append("mode", "edit");
    params.append("invcitem_id", list()->id());

    invoiceItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 45)
  {
    params.append("mode", "edit");
    params.append("wo_id", list()->id());

    workOrder *newdlg = new workOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 65)
  {
    params.append("mode", "edit");
    params.append("pohead_id", list()->id());

    purchaseOrder *newdlg = new purchaseOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 67)
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("poitem_id", list()->id());

    purchaseOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspOrderActivityByProject::sView()
{
  ParameterList params;

  if(list()->altId() == 5)
  {
    params.append("mode", "view");
    params.append("prjtask_id", list()->id());

    task newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 15)
  {
    params.append("mode", "viewQuote");
    params.append("quhead_id", list()->id());

    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 17)
  {
    params.append("mode", "viewQuote");
    params.append("soitem_id", list()->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 25)
  {
    params.append("mode",      "view");
    params.append("sohead_id", list()->id());
    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
  }
  else if(list()->altId() == 27)
  {
    params.append("mode", "view");
    params.append("soitem_id", list()->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 35)
  {
    invoice::viewInvoice(list()->id(), this);
  }
  else if(list()->altId() == 37)
  {
    params.append("mode", "view");
    params.append("invcitem_id", list()->id());

    invoiceItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 45)
  {
    params.append("mode", "view");
    params.append("wo_id", list()->id());

    workOrder *newdlg = new workOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 55)
  {
    params.append("mode", "view");
    params.append("pr_id", list()->id());

    purchaseRequest newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 65)
  {
    params.append("mode", "view");
    params.append("pohead_id", list()->id());

    purchaseOrder *newdlg = new purchaseOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 67)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("poitem_id", list()->id());

    purchaseOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

bool dspOrderActivityByProject::setParams(ParameterList &params)
{
  if(_project->id() == -1)
  {
    list()->clear();
    return false;
  }

  if (!_showWo->isChecked() &&
      !_showPo->isChecked() &&
      !_showSo->isChecked())
  {
    list()->clear();
    return false;
  }


  params.append("prj_id", _project->id());
  
  params.append("so", tr("Sales Order"));
  params.append("wo", tr("Work Order"));
  params.append("po", tr("Purchase Order"));
  params.append("pr", tr("Purchase Request"));
  params.append("sos", tr("Sales Orders"));
  params.append("wos", tr("Work Orders"));
  params.append("pos", tr("Purchase Orders"));
  params.append("prs", tr("Purchase Requests"));
  params.append("quote", tr("Quote"));
  params.append("quotes", tr("Quotes"));
  params.append("invoice", tr("Invoice"));
  params.append("invoices", tr("Invoices"));

  params.append("open", tr("Open"));
  params.append("closed", tr("Closed"));
  params.append("converted", tr("Converted"));
  params.append("canceled", tr("Canceled"));
  params.append("expired", tr("Expired"));
  params.append("unposted", tr("Unposted"));
  params.append("posted", tr("Posted"));
  params.append("exploded", tr("Exploded"));
  params.append("released", tr("Released"));
  params.append("planning", tr("Concept"));
  params.append("inprocess", tr("In Process"));
  params.append("complete", tr("Complete"));
  params.append("unreleased", tr("Unreleased"));
  params.append("total", tr("Total"));

  if(_showSo->isChecked())
    params.append("showSo");

  if(_showWo->isChecked())
    params.append("showWo");

  if(_showPo->isChecked())
    params.append("showPo");

  if (! _privileges->check("ViewAllProjects") && ! _privileges->check("MaintainAllProjects"))
    params.append("owner_username", omfgThis->username());

  return true;
}

void dspOrderActivityByProject::sFillList()
{
  display::sFillList();
  list()->expandAll();
}

void dspOrderActivityByProject::showEvent(QShowEvent *event)
{
  display::showEvent(event);

  if (_run)
    sFillList();
}

void dspOrderActivityByProject::sNewProjectTask()
{

  ParameterList params;
  params.append("mode", "new");
  params.append("prj_id", _project->id());

  XSqlQuery projectpopulate;
  projectpopulate.prepare( "SELECT * "
             "FROM prj "
             "WHERE (prj_id=:prj_id);" );
  projectpopulate.bindValue(":prj_id", _project->id());
  projectpopulate.exec();
  if (projectpopulate.first())
  {   
    params.append("prj_owner_username", projectpopulate.value("prj_owner_username").toString());
    params.append("prj_username", projectpopulate.value("prj_username").toString());
    params.append("prj_start_date", projectpopulate.value("prj_start_date").toDate());
    params.append("prj_due_date",   projectpopulate.value("prj_due_date").toDate());
    params.append("prj_assigned_date", projectpopulate.value("prj_assigned_date").toDate());
    params.append("prj_completed_date", projectpopulate.value("prj_completed_date").toDate());
  } else {
    QMessageBox::warning(this, tr("Project Required"),
      tr("Please save the Project first."));
  }

  task newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrderActivityByProject::sNewSalesOrder()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("prj_id", _project->id());

  salesOrder *newdlg = new salesOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillList();
}


void dspOrderActivityByProject::sNewPurchaseOrder()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("prj_id", _project->id());

  purchaseOrder *newdlg = new purchaseOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillList();
}

void dspOrderActivityByProject::sNewWorkOrder()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("prj_id", _project->id());

  workOrder *newdlg = new workOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillList();
}
