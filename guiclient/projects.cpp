/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "projects.h"
#include "projectCopy.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QVariant>
#include <QSqlError>

#include <parameter.h>

#include "parameterwidget.h"
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
#include "incident.h"

#define DEBUG true

projects::projects(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "projects", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Projects"));
  setMetaSQLOptions("projects", "detail_nohierarchy");
  setReportName("ProjectsList");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);
  setUseAltId(true);

  //  Project Details
  list()->addColumn(tr("Project"),                        _orderColumn, Qt::AlignLeft,   true,  "prj_number");
  list()->addColumn(tr("Project Name"),                   -1,           Qt::AlignLeft,   true,  "prj_name");
  list()->addColumn(tr("Project Description"),            -1,           Qt::AlignLeft,   true,  "prj_descrip");
  list()->addColumn(tr("Status"),                         _itemColumn,  Qt::AlignCenter, true,  "project_status" );
  list()->addColumn(tr("Project Type"),                   _itemColumn,  Qt::AlignCenter, true,  "project_type" );
  list()->addColumn(tr("Owner"),                          _userColumn,  Qt::AlignLeft,   true,  "prj_owner_username");
  list()->addColumn(tr("Assigned To"),                    _userColumn,  Qt::AlignLeft,   true,  "prj_username");
  list()->addColumn(tr("Account"),                        _userColumn,  Qt::AlignLeft,   true,  "crmacct_name");
  list()->addColumn(tr("Contact"),                        _userColumn,  Qt::AlignLeft,   true,  "cntct_name");
  list()->addColumn(tr("City"),                           -1,           Qt::AlignLeft,   true,  "addr_city");
  list()->addColumn(tr("State"),                          -1,           Qt::AlignLeft,   true,  "addr_state");
  list()->addColumn(tr("Project Due"),                    _dateColumn,  Qt::AlignCenter, true,  "prj_due_date");
  list()->addColumn(tr("Project Assigned"),               _dateColumn,  Qt::AlignCenter, true,  "prj_assigned_date");
  list()->addColumn(tr("Project Started"),                _dateColumn,  Qt::AlignCenter, true,  "prj_start_date");
  list()->addColumn(tr("Project Completed"),              _dateColumn,  Qt::AlignCenter, true,  "prj_completed_date");

  connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sOpen()));

  if (!_privileges->check("MaintainAllProjects") && !_privileges->check("MaintainPersonalProjects"))
    newAction()->setEnabled(false);

  _salesOrders->setChecked(false);
  _workOrders->setChecked(false);
  _purchaseOrders->setChecked(false);
  _incidents->setChecked(false);
  _showHierarchy->setChecked(false);

  connect(omfgThis, SIGNAL(projectsUpdated(int)), this, SLOT(sFillList()));
  connect(_showComplete, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
//  connect(_salesOrders, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
//  connect(_workOrders, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
//  connect(_purchaseOrders, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
//  connect(_incidents, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
//  connect(_showHierarchy, SIGNAL(toggled(bool)), this, SLOT(sBuildList()));
  _ordersGroup->hide();

  QString qryType = QString( "SELECT prjtype_id, prjtype_descr FROM prjtype " );

  QString qryStatus = QString( "SELECT  1, '%1' UNION "
                               "SELECT  2, '%2' UNION "
                               "SELECT  3, '%3'")
  .arg(tr("Complete"))
  .arg(tr("Concept"))
  .arg(tr("In-Process"));
  
  parameterWidget()->append(tr("Owner"), "owner_username", ParameterWidget::User);
  parameterWidget()->append(tr("AssignedTo"), "assigned_username", ParameterWidget::User);
  parameterWidget()->append(tr("Account"), "crmacct_id", ParameterWidget::Crmacct);
  parameterWidget()->append(tr("Contact"), "cntct_id", ParameterWidget::Contact);
  parameterWidget()->appendComboBox(tr("Project Type"), "prjtype_id", qryType);
  parameterWidget()->appendComboBox(tr("Project Status"), "prjstatus_id", qryStatus);
  parameterWidget()->append(tr("Project"), "prj_id", ParameterWidget::Project);
  parameterWidget()->append(tr("Project Task"), "project_task", ParameterWidget::Text);
  parameterWidget()->append(tr("Sales Order"), "cohead_id", ParameterWidget::SalesOrder);
  parameterWidget()->append(tr("Work Order"), "wo_id", ParameterWidget::WorkOrder);
  parameterWidget()->append(tr("Purchase Order"), "pohead_id", ParameterWidget::PurchaseOrder);
  parameterWidget()->append(tr("Start Start Date"), "startStartDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Start End Date"), "startEndDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Due Start Date"), "dueStartDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Due End Date"), "dueEndDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Assigned Start Date"), "assignedStartDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Assigned End Date"), "assignedEndDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Completed Start Date"), "completedStartDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Completed End Date"), "completedEndDate", ParameterWidget::Date, QDate::currentDate());

  setupCharacteristics("PROJ");
          
  _statuses << "None" << "C" << "P" << "O";

//  sBuildList();
}

void projects::sBuildList()
{

  list()->setColumnCount(0);

  if (!_showHierarchy->isChecked())
  {
//  List Hierarchy Display
    setMetaSQLOptions("projects", "detail");
    list()->addColumn(tr("Number"),       _orderColumn,  Qt::AlignLeft,   true,  "name");
    list()->addColumn(tr("Name"),                   -1,  Qt::AlignLeft,   true,  "item");
    list()->addColumn(tr("Description"),            -1,  Qt::AlignLeft,   true,  "descrip");
    list()->addColumn(tr("Status"),        _itemColumn,  Qt::AlignCenter, true,  "status" );
    list()->addColumn(tr("Project Type"),        _itemColumn,  Qt::AlignCenter, true,  "project_type" );
    list()->addColumn(tr("Owner"),         _userColumn,  Qt::AlignLeft,   false, "prj_owner_username");
    list()->addColumn(tr("Assigned To"),   _userColumn,  Qt::AlignLeft,   true,  "prj_username");
    list()->addColumn(tr("Account/Customer"),   _userColumn,  Qt::AlignLeft,   true,  "customer");
    list()->addColumn(tr("Contact"),       _userColumn,  Qt::AlignLeft,   false,  "contact");
    list()->addColumn(tr("City"),       -1,  Qt::AlignLeft,   false,  "city");
    list()->addColumn(tr("State"),       -1,  Qt::AlignLeft,   false,  "state");
    list()->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight,  true,  "qty"  );
    list()->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignLeft,   true,  "uom"  );
    list()->addColumn(tr("Value"),      _qtyColumn,   Qt::AlignRight,  true,  "value"  );
    list()->addColumn(tr("Due"),           _dateColumn,  Qt::AlignCenter, true,  "due");
    list()->addColumn(tr("Assigned"),      _dateColumn,  Qt::AlignCenter, true,  "assigned");
    list()->addColumn(tr("Started"),       _dateColumn,  Qt::AlignCenter, true,  "started");
    list()->addColumn(tr("Completed"),     _dateColumn,  Qt::AlignCenter, true,  "completed");
    list()->addColumn(tr("Budget Hrs."),   _costColumn,  Qt::AlignRight,  true,  "hrs_budget");
    list()->addColumn(tr("Actual Hrs."),   _costColumn,  Qt::AlignRight,  true,  "hrs_actual");
    list()->addColumn(tr("Balance Hrs."),  _costColumn,  Qt::AlignRight,  true,  "hrs_balance");
    list()->addColumn(tr("Budget Exp."),   _priceColumn,  Qt::AlignRight,  false,  "exp_budget");
    list()->addColumn(tr("Actual Exp."),   _priceColumn,  Qt::AlignRight,  false,  "exp_actual");
    list()->addColumn(tr("Balance Exp."),  _priceColumn,  Qt::AlignRight,  false,  "exp_balance");
  } else {
//  List flat display (for CSV export purposes)
    setMetaSQLOptions("projects", "detail_nohierarchy");
//  Project Details
    list()->addColumn(tr("Project"),       _orderColumn,  Qt::AlignLeft,   true,  "prj_number");
    list()->addColumn(tr("Project Name"),                   -1,  Qt::AlignLeft,   true,  "prj_name");
    list()->addColumn(tr("Project Description"),            -1,  Qt::AlignLeft,   true,  "prj_descrip");
    list()->addColumn(tr("Status"),        _itemColumn,  Qt::AlignCenter, true,  "project_status" );
    list()->addColumn(tr("Project Type"),        _itemColumn,  Qt::AlignCenter, true,  "project_type" );
    list()->addColumn(tr("Owner"),         _userColumn,  Qt::AlignLeft,   true, "prj_owner_username");
    list()->addColumn(tr("Assigned To"),   _userColumn,  Qt::AlignLeft,   true,  "prj_username");
    list()->addColumn(tr("Account"),   _userColumn,  Qt::AlignLeft,   true,  "crmacct_name");
    list()->addColumn(tr("Contact"),       _userColumn,  Qt::AlignLeft,   true,  "cntct_name");
    list()->addColumn(tr("City"),       -1,  Qt::AlignLeft,   true,  "addr_city");
    list()->addColumn(tr("State"),       -1,  Qt::AlignLeft,   true,  "addr_state");
    list()->addColumn(tr("Project Due"),           _dateColumn,  Qt::AlignCenter, true,  "prj_due_date");
    list()->addColumn(tr("Project Assigned"),      _dateColumn,  Qt::AlignCenter, true,  "prj_assigned_date");
    list()->addColumn(tr("Project Started"),       _dateColumn,  Qt::AlignCenter, true,  "prj_start_date");
    list()->addColumn(tr("Project Completed"),     _dateColumn,  Qt::AlignCenter, true,  "prj_completed_date");
//  Task Details
    list()->addColumn(tr("Task"),       _orderColumn,  Qt::AlignLeft,   true,  "prjtask_number");
    list()->addColumn(tr("Task Name"),                   -1,  Qt::AlignLeft,   true,  "prjtask_name");
    list()->addColumn(tr("Task Description"),            -1,  Qt::AlignLeft,   true,  "prjtask_descrip");
    list()->addColumn(tr("Task Status"),        _itemColumn,  Qt::AlignCenter, true,  "task_status" );
    list()->addColumn(tr("Task Owner"),         _userColumn,  Qt::AlignLeft,   true, "prjtask_owner_username");
    list()->addColumn(tr("Task Assigned To"),   _userColumn,  Qt::AlignLeft,   true,  "prjtask_username");
    list()->addColumn(tr("Customer"),   _userColumn,  Qt::AlignLeft,   true,  "cust_name");
    list()->addColumn(tr("Task Due"),           _dateColumn,  Qt::AlignCenter, true,  "prjtask_due_date");
    list()->addColumn(tr("Task Assigned"),      _dateColumn,  Qt::AlignCenter, true,  "prjtask_assigned_date");
    list()->addColumn(tr("Task Started"),       _dateColumn,  Qt::AlignCenter, true,  "prjtask_start_date");
    list()->addColumn(tr("Task Completed"),     _dateColumn,  Qt::AlignCenter, true,  "prjtask_completed_date");
    list()->addColumn(tr("Budget Hrs."),   _costColumn,  Qt::AlignRight,  true,  "prjtask_hours_budget");
    list()->addColumn(tr("Actual Hrs."),   _costColumn,  Qt::AlignRight,  true,  "prjtask_hours_actual");
    list()->addColumn(tr("Balance Hrs."),  _costColumn,  Qt::AlignRight,  true,  "prjtask_hours_balance");
    list()->addColumn(tr("Budget Exp."),   _priceColumn,  Qt::AlignRight,  true,  "prjtask_exp_budget");
    list()->addColumn(tr("Actual Exp."),   _priceColumn,  Qt::AlignRight,  true,  "prjtask_exp_actual");
    list()->addColumn(tr("Balance Exp."),  _priceColumn,  Qt::AlignRight,  true,  "prjtask_exp_balance");
    setupCharacteristics("PROJ");
    _showHierarchy->setEnabled(false);
  }

  sFillList();
}

void projects::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  project newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void projects::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  bool editPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("MaintainPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("MaintainPersonalProjects")) ||
      (_privileges->check("MaintainAllProjects"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("ViewPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("ViewPersonalProjects")) ||
      (_privileges->check("ViewAllProjects"));

  if(list()->altId() == 1)
  {
    menuItem = pMenu->addAction(tr("Edit Project..."), this, SLOT(sEdit()));
    menuItem->setEnabled(editPriv);

    menuItem = pMenu->addAction(tr("View Project..."), this, SLOT(sView()));
    menuItem->setEnabled(viewPriv);
  
    menuItem = pMenu->addAction("Delete Project...", this, SLOT(sDelete()));
    menuItem->setEnabled(editPriv);

    menuItem = pMenu->addAction("Copy Project...", this, SLOT(sCopy()));
    menuItem->setEnabled(editPriv);

  }

  if(list()->altId() == 5)
  {
    menuItem = pMenu->addAction(tr("Edit Task..."), this, SLOT(sEdit()));
    menuItem->setEnabled(editPriv);

    menuItem = pMenu->addAction(tr("View Task..."), this, SLOT(sView()));
    menuItem->setEnabled(viewPriv);

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
    menuItem = pMenu->addAction(tr("Edit Invoice..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));

    menuItem = pMenu->addAction(tr("View Invoice..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices") ||
                         _privileges->check("ViewMiscInvoices"));
  }

  if(list()->altId() == 37)
  {
    menuItem = pMenu->addAction(tr("Edit Invoice Item..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));

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

  if(list()->altId() == 105)
  {
    menuItem = pMenu->addAction(tr("Edit Incident..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainPersonalIncidents") ||
			_privileges->check("MaintainAllIncidents"));

    menuItem = pMenu->addAction(tr("View Incident..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("ViewPersonalIncidents") ||
			_privileges->check("ViewAllIncidents") ||
			_privileges->check("MaintainPersonalIncidents") ||
			_privileges->check("MaintainAllIncidents"));
  }

}

void projects::sEdit()
{
  ParameterList params;

  if(list()->altId() == 1)
  {
    params.append("prj_id", list()->id());
    params.append("mode", "edit");
 
    project newdlg(this, "", true);
    newdlg.set(params);
    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }

  else if(list()->altId() == 5)
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

  else if(list()->altId() == 105)
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("incdt_id", list()->id());

    incident newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

void projects::sView()
{
  ParameterList params;

  if(list()->altId() == 1)
  {
    params.append("prj_id", list()->id());
    params.append("mode", "view");
 
    project newdlg(this, "", true);
    newdlg.set(params);
    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
  else if(list()->altId() == 5)
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
  else if(list()->altId() == 105)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("incdt_id", list()->id());

    incident newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

void projects::sDelete()
{
  XSqlQuery projectsDelete;
  projectsDelete.prepare("SELECT deleteProject(:prj_id) AS result");
  projectsDelete.bindValue(":prj_id", list()->id());
  projectsDelete.exec();
  if(projectsDelete.first())
  {
    int result = projectsDelete.value("result").toInt();
    if(result < 0)
    {
      QString errmsg;
      switch(result)
      {
        case -1:
          errmsg = tr("One or more Quote's refer to this project.");
          break;
        case -2:
          errmsg = tr("One or more Sales Orders refer to this project.");
          break;
        case -3:
          errmsg = tr("One or more Work Orders refer to this project.");
          break;
        case -4:
          errmsg = tr("One or more Purchase Requests refer to this project.");
          break;
        case -5:
          errmsg = tr("One or more Purchase order Items refer to this project.");
          break;
        case -6:
          errmsg = tr("One or more Invoices refer to this project.");
          break;
        default:
          errmsg = tr("Error #%1 encountered while trying to delete project.").arg(result);
      }
      QMessageBox::critical( this, tr("Cannot Delete Project"),
        tr("Could not delete the project for one or more reasons.\n") + errmsg);
      return;
    }
    else if (projectsDelete.lastError().type() != QSqlError::NoError)
    {
      systemError(this, projectsDelete.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  sFillList();
}

void projects::sCopy()
{
  if (DEBUG)
    qDebug("Project sCopy() Project ID: %d)", list()->id());  

  if (list()->id() == -1)
  {
    QMessageBox::information(this, tr("Project Copy"), tr("Please select a project to copy first"));
    return;
  }
  
  ParameterList params;
  params.append("prj_id", list()->id());
  
  projectCopy newdlg(parentWidget(), "", true);
  newdlg.set(params);
  newdlg.exec();

  sFillList();
}

bool projects::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  bool valid;
  QVariant param;
  
  param = params.value("prjstatus_id", &valid);
  if (valid)
    params.append("prjstatus", _statuses.at(param.toInt()));
  
  if (_showComplete->isChecked())
    params.append("showComplete",true);

  if (_salesOrders->isChecked())
    params.append("showSo", true);

  if (_workOrders->isChecked())
    params.append("showWo", true);

  if (_purchaseOrders->isChecked())
    params.append("showPo", true);

  if (_incidents->isChecked())
    params.append("showIn", true);

  params.append("planning", tr("Concept"));
  params.append("open", tr("In-Process"));
  params.append("complete", tr("Complete"));
  params.append("undefined", tr("Undefined"));
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
  params.append("task", tr("Task"));
  params.append("tasks", tr("Tasks"));

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

  return true;
}

void projects::sOpen()
{
  bool editPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("MaintainPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("MaintainPersonalProjects")) ||
      (_privileges->check("MaintainAllProjects"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("ViewPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("ViewPersonalProjects")) ||
      (_privileges->check("ViewAllProjects"));

  if (editPriv)
    sEdit();
  else if (viewPriv)
    sView();

}

