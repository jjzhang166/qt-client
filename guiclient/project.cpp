  /*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "project.h"

#include <QMenu>
#include <QAction>
#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <openreports.h>
#include <comment.h>
#include <metasql.h>

#include "mqlutil.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
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

const char *_projectStatuses[] = { "P", "O", "C" };

project::project(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  if(!_privileges->check("EditOwner")) _owner->setEnabled(false);

  connect(_buttonBox,     SIGNAL(rejected()),        this, SLOT(sClose()));
  connect(_buttonBox,     SIGNAL(accepted()),        this, SLOT(sSave()));
  connect(_queryTasks,    SIGNAL(clicked()),         this, SLOT(sFillTaskList()));
  connect(_newTask,       SIGNAL(clicked()),         this, SLOT(sNewTask()));
  connect(_editTask,      SIGNAL(clicked()),         this, SLOT(sEditTask()));
  connect(_viewTask,      SIGNAL(clicked()),         this, SLOT(sViewTask()));
  connect(_deleteTask,    SIGNAL(clicked()),         this, SLOT(sDeleteTask()));
  connect(_number,        SIGNAL(editingFinished()), this, SLOT(sNumberChanged()));
  connect(_crmacct,       SIGNAL(newId(int)),        this, SLOT(sCRMAcctChanged(int)));
  connect(_prjtask, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_showSo, SIGNAL(toggled(bool)), this, SLOT(sFillTaskList()));
  connect(_showPo, SIGNAL(toggled(bool)), this, SLOT(sFillTaskList()));
  connect(_showWo, SIGNAL(toggled(bool)), this, SLOT(sFillTaskList()));
  connect(_showIn, SIGNAL(toggled(bool)), this, SLOT(sFillTaskList()));

  _charass->setType("PROJ");

  _prjtask->addColumn(tr("Name"),        _itemColumn,  Qt::AlignLeft,   true,  "name"   );
  _prjtask->addColumn(tr("Status"),      _orderColumn, Qt::AlignLeft,   true,  "status"   );
  _prjtask->addColumn(tr("Item #"),      _itemColumn,  Qt::AlignLeft,   true,  "item"   );
  _prjtask->addColumn(tr("Description"), -1          , Qt::AlignLeft,   true,  "descrip" );
  _prjtask->addColumn(tr("Account/Customer"), -1          , Qt::AlignLeft,   false,  "customer" );
  _prjtask->addColumn(tr("Contact"), -1          , Qt::AlignLeft,   false,  "contact" );
  _prjtask->addColumn(tr("City"), -1          , Qt::AlignLeft,   false,  "city" );
  _prjtask->addColumn(tr("State"), -1          , Qt::AlignLeft,   false,  "state" );
  _prjtask->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight,  true,  "qty"  );
  _prjtask->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignLeft,   true,  "uom"  );
  _prjtask->addColumn(tr("Value"),      _qtyColumn,   Qt::AlignRight,  true,  "value"  );
  _prjtask->addColumn(tr("Due Date"),      _dateColumn,   Qt::AlignRight,  true,  "due"  );
  _prjtask->addColumn(tr("Assigned"),      _dateColumn,   Qt::AlignRight,  true,  "assigned"  );
  _prjtask->addColumn(tr("Started"),      _dateColumn,   Qt::AlignRight,  true,  "started"  );
  _prjtask->addColumn(tr("Completed"),      _dateColumn,   Qt::AlignRight,  true,  "completed"  );
  _prjtask->addColumn(tr("Hrs. Budget"),      _qtyColumn,   Qt::AlignRight,  true,  "hrs_budget"  );
  _prjtask->addColumn(tr("Hrs. Actual"),      _qtyColumn,   Qt::AlignRight,  true,  "hrs_actual"  );
  _prjtask->addColumn(tr("Hrs. Balance"),      _qtyColumn,   Qt::AlignRight,  true,  "hrs_balance"  );
  _prjtask->addColumn(tr("Exp. Budget"),      _priceColumn,   Qt::AlignRight,  true,  "exp_budget"  );
  _prjtask->addColumn(tr("Exp. Actual"),      _priceColumn,   Qt::AlignRight,  true,  "exp_actual"  );
  _prjtask->addColumn(tr("Exp. Balance"),      _priceColumn,   Qt::AlignRight,  true,  "exp_balance"  );

  _owner->setUsername(omfgThis->username());
  _assignedTo->setUsername(omfgThis->username());
  _owner->setType(UsernameLineEdit::UsersActive);
  _assignedTo->setType(UsernameLineEdit::UsersActive);

  _totalHrBud->setPrecision(omfgThis->qtyVal());
  _totalHrAct->setPrecision(omfgThis->qtyVal());
  _totalHrBal->setPrecision(omfgThis->qtyVal());
  _totalExpBud->setPrecision(omfgThis->moneyVal());
  _totalExpAct->setPrecision(omfgThis->moneyVal());
  _totalExpBal->setPrecision(omfgThis->moneyVal());
  
  _saved=false;

  QMenu * newMenu = new QMenu;
  QAction *menuItem;
  newMenu->addAction(tr("Task..."), this, SLOT(sNewTask()));
  newMenu->addSeparator();
  menuItem = newMenu->addAction(tr("Incident"), this, SLOT(sNewIncident()));
  menuItem->setEnabled(_privileges->check("MaintainPersonalIncidents") ||
                       _privileges->check("MaintainAllIncidents"));
  menuItem = newMenu->addAction(tr("Quote"), this, SLOT(sNewQuotation()));
  menuItem->setEnabled(_privileges->check("MaintainQuotes"));
  menuItem = newMenu->addAction(tr("Sales Order"), this, SLOT(sNewSalesOrder()));
  menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));
  menuItem = newMenu->addAction(tr("Purchase Order"),   this, SLOT(sNewPurchaseOrder()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  menuItem = newMenu->addAction(tr("Work Order"),   this, SLOT(sNewWorkOrder()));
  menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));
  _newTask->setMenu(newMenu); 

  QMenu * printMenu = new QMenu;
  printMenu->addAction(tr("Print Tasks"), this, SLOT(sPrintTasks()));
  printMenu->addAction(tr("Print Orders"), this, SLOT(sPrintOrders()));
  _print->setMenu(printMenu);

  populate();
}

project::~project()
{
  // no need to delete child widgets, Qt does it all for us
}

void project::languageChange()
{
  retranslateUi(this);
}

enum SetResponse project::set(const ParameterList &pParams)
{
  XSqlQuery projectet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("username", &valid);
  if (valid)
    _assignedTo->setUsername(param.toString());

  param = pParams.value("prj_id", &valid);
  if (valid)
  {
    _prjid = param.toInt();
    populate();
    _charass->setId(_prjid);
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
    _crmacct->setEnabled(false);
  }

  param = pParams.value("cntct_id", &valid);
  if (valid)
  {
    _cntct->setId(param.toInt());
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      connect(_assignedTo, SIGNAL(newId(int)), this, SLOT(sAssignedToChanged(int)));
      connect(_status,  SIGNAL(currentIndexChanged(int)), this, SLOT(sStatusChanged(int)));
      connect(_prjtask, SIGNAL(valid(bool)), this, SLOT(sHandleButtons(bool)));
      connect(_prjtask, SIGNAL(valid(bool)), this, SLOT(sHandleButtons(bool)));
      connect(_prjtask, SIGNAL(itemSelected(int)), _editTask, SLOT(animateClick()));

      projectet.exec("SELECT NEXTVAL('prj_prj_id_seq') AS prj_id;");
      if (projectet.first())
        _prjid = projectet.value("prj_id").toInt();
      else if (projectet.lastError().type() == QSqlError::NoError)
      {
        systemError(this, projectet.lastError().text(), __FILE__, __LINE__);
        return UndefinedError;
      }

      _comments->setId(_prjid);
      _documents->setId(_prjid);
      _charass->setId(_prjid);
      _recurring->setParent(_prjid, "J");
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _number->setEnabled(false);

      connect(_assignedTo, SIGNAL(newId(int)), this, SLOT(sAssignedToChanged(int)));
      connect(_status,  SIGNAL(currentIndexChanged(int)), this, SLOT(sStatusChanged(int)));
      connect(_prjtask, SIGNAL(valid(bool)), this, SLOT(sHandleButtons(bool)));
      connect(_prjtask, SIGNAL(valid(bool)), this, SLOT(sHandleButtons(bool)));
      connect(_prjtask, SIGNAL(itemSelected(int)), _editTask, SLOT(animateClick()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _owner->setEnabled(false);
      _number->setEnabled(false);
      _status->setEnabled(false);
      _name->setEnabled(false);
      _descrip->setEnabled(false);
      _so->setEnabled(false);
      _wo->setEnabled(false);
      _po->setEnabled(false);
      _assignedTo->setEnabled(false);
      _crmacct->setEnabled(false);
      _cntct->setEnabled(false);
      _newTask->setEnabled(false);
      connect(_prjtask, SIGNAL(itemSelected(int)), _viewTask, SLOT(animateClick()));
      _comments->setReadOnly(true);
      _charass->setReadOnly(true);
      _documents->setReadOnly(true);
      _started->setEnabled(false);
      _assigned->setEnabled(false);
      _due->setEnabled(false);
      _completed->setEnabled(false);
      _recurring->setEnabled(false);
      _projectType->setEnabled(false);
      _buttonBox->removeButton(_buttonBox->button(QDialogButtonBox::Save));
      _buttonBox->removeButton(_buttonBox->button(QDialogButtonBox::Cancel));
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }
    
  return NoError;
}

void project::sHandleButtons(bool valid)
{
  if(_prjtask->altId() == 5)
  {
    _editTask->setEnabled(valid);
    _deleteTask->setEnabled(valid);
    _viewTask->setEnabled(valid);
  } else {
    _editTask->setEnabled(false);
    _deleteTask->setEnabled(false);
    _viewTask->setEnabled(false);
  }
}

void project::sPopulateMenu(QMenu *pMenu,  QTreeWidgetItem *selected)
{
  QAction *menuItem;

  if(_prjtask->altId() == 5)
  {
    menuItem = pMenu->addAction(tr("Edit Task..."), this, SLOT(sEditTask()));
    menuItem->setEnabled(_privileges->check("MaintainAllProjects") || _privileges->check("MaintainPersonalProjects"));

    menuItem = pMenu->addAction(tr("View Task..."), this, SLOT(sViewTask()));
    menuItem->setEnabled(_privileges->check("MaintainAllProjects") ||
			 _privileges->check("MaintainPersonalProjects") ||
                         _privileges->check("ViewAllProjects")  ||
			 _privileges->check("ViewPersonalProjects"));
  }

  if(_prjtask->altId() == 15)
  {
    menuItem = pMenu->addAction(tr("Edit Quote..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes"));

    menuItem = pMenu->addAction(tr("View Quote..."), this, SLOT(sViewOrder()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes") ||
                         _privileges->check("ViewQuotes") );
  }

  if(_prjtask->altId() == 17)
  {
    menuItem = pMenu->addAction(tr("Edit Quote Item..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes"));

    menuItem = pMenu->addAction(tr("View Quote Item..."), this, SLOT(sViewOrder()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes") ||
                         _privileges->check("ViewQuotes"));
  }

  if(_prjtask->altId() == 25)
  {
    menuItem = pMenu->addAction(tr("Edit Sales Order..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

    menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sViewOrder()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") ||
                         _privileges->check("ViewSalesOrders"));
  }

  if(_prjtask->altId() == 27)
  {
    menuItem = pMenu->addAction(tr("Edit Sales Order Item..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

    menuItem = pMenu->addAction(tr("View Sales Order Item..."), this, SLOT(sViewOrder()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") ||
                         _privileges->check("ViewSalesOrders"));
  }

  if(_prjtask->altId() == 35)
  {
    menuItem = pMenu->addAction(tr("Edit Invoice..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));

    menuItem = pMenu->addAction(tr("View Invoice..."), this, SLOT(sViewOrder()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices") ||
                         _privileges->check("ViewMiscInvoices"));
  }

  if(_prjtask->altId() == 37)
  {
    menuItem = pMenu->addAction(tr("Edit Invoice Item..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));

    menuItem = pMenu->addAction(tr("View Invoice Item..."), this, SLOT(sViewOrder()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices") ||
                         _privileges->check("ViewMiscInvoices"));
  }

  if(_prjtask->altId() == 45)
  {
    menuItem = pMenu->addAction(tr("Edit Work Order..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));

    menuItem = pMenu->addAction(tr("View Work Order..."), this, SLOT(sViewOrder()));
    menuItem->setEnabled(_privileges->check("MaintainWorkOrders") ||
                         _privileges->check("ViewWorkOrders"));
  }

  if(_prjtask->altId() == 55)
  {
    menuItem = pMenu->addAction(tr("View Purchase Request..."), this, SLOT(sViewOrder()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseRequests") ||
                         _privileges->check("ViewPurchaseRequests"));
  }

  if(_prjtask->altId() == 65)
  {
    menuItem = pMenu->addAction(tr("Edit Purchase Order..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

    menuItem = pMenu->addAction(tr("View Purchase Order..."), this, SLOT(sViewOrder()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                         _privileges->check("ViewPurchaseOrders"));
  }

  if(_prjtask->altId() == 67)
  {
    menuItem = pMenu->addAction(tr("Edit Purchase Order Item..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

    menuItem = pMenu->addAction(tr("View Purchase Order Item..."), this, SLOT(sViewOrder()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                         _privileges->check("ViewPurchaseOrders"));
  }

  if(_prjtask->altId() == 105)
  {
    menuItem = pMenu->addAction(tr("Edit Incident..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainPersonalIncidents") ||
			_privileges->check("MaintainAllIncidents"));

    menuItem = pMenu->addAction(tr("View Incident..."), this, SLOT(sViewOrder()));
    menuItem->setEnabled(_privileges->check("ViewPersonalIncidents") ||
			_privileges->check("ViewAllIncidents") ||
			_privileges->check("MaintainPersonalIncidents") ||
			_privileges->check("MaintainAllIncidents"));
  }
}

void project::populate()
{
  XSqlQuery projectpopulate;
  projectpopulate.prepare( "SELECT * "
             "FROM prj "
             "WHERE (prj_id=:prj_id);" );
  projectpopulate.bindValue(":prj_id", _prjid);
  projectpopulate.exec();
  if (projectpopulate.first())
  {
    _saved = true;
    _owner->setUsername(projectpopulate.value("prj_owner_username").toString());
    _number->setText(projectpopulate.value("prj_number").toString());
    _name->setText(projectpopulate.value("prj_name").toString());
    _descrip->setText(projectpopulate.value("prj_descrip").toString());
    _so->setChecked(projectpopulate.value("prj_so").toBool());
    _wo->setChecked(projectpopulate.value("prj_wo").toBool());
    _po->setChecked(projectpopulate.value("prj_po").toBool());
    _assignedTo->setUsername(projectpopulate.value("prj_username").toString());
    _cntct->setId(projectpopulate.value("prj_cntct_id").toInt());
    _crmacct->setId(projectpopulate.value("prj_crmacct_id").toInt());
    _started->setDate(projectpopulate.value("prj_start_date").toDate());
    _assigned->setDate(projectpopulate.value("prj_assigned_date").toDate());
    _due->setDate(projectpopulate.value("prj_due_date").toDate());
    _completed->setDate(projectpopulate.value("prj_completed_date").toDate());
    _projectType->setId(projectpopulate.value("prj_prjtype_id").toInt());
    for (int counter = 0; counter < _status->count(); counter++)
    {
      if (QString(projectpopulate.value("prj_status").toString()[0]) == _projectStatuses[counter])
        _status->setCurrentIndex(counter);
    }

    _recurring->setParent(projectpopulate.value("prj_recurring_prj_id").isNull() ?
                            _prjid : projectpopulate.value("prj_recurring_prj_id").toInt(),
                          "J");
  }

  XSqlQuery projectType;
  projectType.prepare( "SELECT prjtype_id, prjtype_descr FROM prjtype WHERE prjtype_active "
                       "UNION "
                       "SELECT prjtype_id, prjtype_descr FROM prjtype "
                       "JOIN prj ON (prj_prjtype_id=prjtype_id) "
                       "WHERE (prj_id=:prj_id);" );
  projectType.bindValue(":prj_id", _prjid);
  projectType.exec();
  _projectType->populate(projectType);
  if (projectType.lastError().type() != QSqlError::NoError)
  {
    systemError(this, projectType.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillTaskList();
  _comments->setId(_prjid);
  _documents->setId(_prjid);
  emit populated(_prjid);
}

void project::sAssignedToChanged(const int newid)
{
  if (newid == -1)
    _assigned->clear();
  else
    _assigned->setDate(omfgThis->dbDate());
}

void project::sStatusChanged(const int pStatus)
{
  switch(pStatus)
  {
    case 0: // Concept
    default:
      _started->clear();
      _completed->clear();
      break;
    case 1: // In Process
      _started->setDate(omfgThis->dbDate());
      _completed->clear();
      break;
    case 2: // Completed
      _completed->setDate(omfgThis->dbDate());
      break;
  }
}

void project::sCRMAcctChanged(const int newid)
{
  _cntct->setSearchAcct(newid);
}

void project::sClose()
{
  XSqlQuery projectClose;
  if (_mode == cNew)
  {
    projectClose.prepare( "SELECT deleteproject(:prj_id);" );
    projectClose.bindValue(":prj_id", _prjid);
    projectClose.exec();
  }

  reject();
}

bool project::sSave(bool partial)
{
  XSqlQuery projectSave;
  QList<GuiErrorCheck> errors;
  errors<< GuiErrorCheck(_number->text().isEmpty(), _number,
                         tr("No Project Number was specified. You must specify a project number "
                            "before saving it."))
        << GuiErrorCheck(!partial && !_due->isValid(), _due,
                         tr("You must specify a due date before "
                            "saving it."))
  ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Project"), errors))
    return false;

  RecurrenceWidget::RecurrenceChangePolicy cp = _recurring->getChangePolicy();
  if (cp == RecurrenceWidget::NoPolicy)
    return false;

  XSqlQuery rollbackq;
  rollbackq.prepare("ROLLBACK;");
  XSqlQuery begin("BEGIN;");

  if (!_saved)
    projectSave.prepare( "INSERT INTO prj "
               "( prj_id, prj_number, prj_name, prj_descrip,"
               "  prj_so, prj_wo, prj_po, prj_status, prj_owner_username, "
               "  prj_start_date, prj_due_date, prj_assigned_date,"
               "  prj_completed_date, prj_username, prj_recurring_prj_id,"
               "  prj_crmacct_id, prj_cntct_id, prj_prjtype_id) "
               "VALUES "
               "( :prj_id, :prj_number, :prj_name, :prj_descrip,"
               "  :prj_so, :prj_wo, :prj_po, :prj_status, :prj_owner_username,"
               "  :prj_start_date, :prj_due_date, :prj_assigned_date,"
               "  :prj_completed_date, :username, :prj_recurring_prj_id,"
               "  :prj_crmacct_id, :prj_cntct_id, :prj_prjtype_id);" );
  else
    projectSave.prepare( "UPDATE prj "
               "SET prj_number=:prj_number, prj_name=:prj_name, prj_descrip=:prj_descrip,"
               "    prj_so=:prj_so, prj_wo=:prj_wo, prj_po=:prj_po, prj_status=:prj_status, "
               "    prj_owner_username=:prj_owner_username, prj_start_date=:prj_start_date, "
               "    prj_due_date=:prj_due_date, prj_assigned_date=:prj_assigned_date,"
               "    prj_completed_date=:prj_completed_date,"
               "    prj_username=:username,"
               "    prj_recurring_prj_id=:prj_recurring_prj_id,"
               "    prj_crmacct_id=:prj_crmacct_id,"
               "    prj_cntct_id=:prj_cntct_id, "
               "    prj_prjtype_id=:prj_prjtype_id "
               "WHERE (prj_id=:prj_id);" );

  projectSave.bindValue(":prj_id", _prjid);
  projectSave.bindValue(":prj_number", _number->text().trimmed().toUpper());
  projectSave.bindValue(":prj_name", _name->text());
  projectSave.bindValue(":prj_descrip", _descrip->toPlainText());
  projectSave.bindValue(":prj_status", _projectStatuses[_status->currentIndex()]);
  projectSave.bindValue(":prj_so", QVariant(_so->isChecked()));
  projectSave.bindValue(":prj_wo", QVariant(_wo->isChecked()));
  projectSave.bindValue(":prj_po", QVariant(_po->isChecked()));
  projectSave.bindValue(":prj_owner_username", _owner->username());
  projectSave.bindValue(":username", _assignedTo->username());
  if (_crmacct->id() > 0)
    projectSave.bindValue(":prj_crmacct_id", _crmacct->id());
  if (_cntct->id() > 0)
    projectSave.bindValue(":prj_cntct_id", _cntct->id());
  if (_projectType->id() > 0)
    projectSave.bindValue(":prj_prjtype_id", _projectType->id());
  projectSave.bindValue(":prj_start_date", _started->date());
  projectSave.bindValue(":prj_due_date",	_due->date());
  projectSave.bindValue(":prj_assigned_date", _assigned->date());
  projectSave.bindValue(":prj_completed_date", _completed->date());
  if (_recurring->isRecurring())
    projectSave.bindValue(":prj_recurring_prj_id", _recurring->parentId());

  projectSave.exec();
  if (projectSave.lastError().type() != QSqlError::NoError)
  {
    rollbackq.exec();
    systemError(this, projectSave.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  QString errmsg;
  if (! _recurring->save(true, cp, &errmsg))
  {
    qDebug("recurring->save failed");
    rollbackq.exec();
    systemError(this, errmsg, __FILE__, __LINE__);
    return false;
  }

  projectSave.exec("COMMIT;");
  emit saved(_prjid);

  if (!partial)
    done(_prjid);
  else
    _saved=true;
  return true;
}

void project::sPrintTasks()
{
  ParameterList params;

  params.append("prj_id", _prjid);

  orReport report("ProjectTaskList", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void project::sPrintOrders()
{
  ParameterList params;

  params.append("prj_id", _prjid);

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

  orReport report("OrderActivityByProject", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void project::sNewTask()
{
  if (!_saved)
  {
    if (!sSave(true))
      return;
  }
    
  ParameterList params;
  params.append("mode", "new");
  params.append("prj_id", _prjid);
  params.append("prj_owner_username", _owner->username());
  params.append("prj_username", _assignedTo->username());
  params.append("prj_start_date", _started->date());
  params.append("prj_due_date",	_due->date());
  params.append("prj_assigned_date", _assigned->date());
  params.append("prj_completed_date", _completed->date());

  task newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillTaskList();
}

void project::sEditTask()
{
  if(_prjtask->altId() != 5)
    return;

  ParameterList params;
  params.append("mode", "edit");
  params.append("prjtask_id", _prjtask->id());

  task newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillTaskList();
}

void project::sViewTask()
{
  if(_prjtask->altId() != 5)
    return;

  ParameterList params;
  params.append("mode", "view");
  params.append("prjtask_id", _prjtask->id());

  task newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void project::sDeleteTask()
{
  if(_prjtask->altId() != 5)
    return;
  
  XSqlQuery projectDeleteTask;
  projectDeleteTask.prepare("SELECT deleteProjectTask(:prjtask_id) AS result; ");
  projectDeleteTask.bindValue(":prjtask_id", _prjtask->id());
  projectDeleteTask.exec();
  if(projectDeleteTask.first())
  {
    int result = projectDeleteTask.value("result").toInt();
    if(result < 0)
    {
      QString errmsg;
      switch(result)
      {
        case -1:
          errmsg = tr("Project task not found.");
          break;
        case -2:
          errmsg = tr("Actual hours have been posted to this project task.");
          break;
        case -3:
          errmsg = tr("Actual expenses have been posted to this project task.");
          break;
        default:
          errmsg = tr("Error #%1 encountered while trying to delete project task.").arg(result);
      }
      QMessageBox::critical( this, tr("Cannot Delete Project Task"),
        tr("Could not delete the project task for one or more reasons.\n") + errmsg);
      return;
    }
  }
  emit deletedTask();
  sFillTaskList();
}

void project::sFillTaskList()
{
// Populate Summary of Task Activity
  MetaSQLQuery mql = mqlLoad("projectTasks", "detail");

  ParameterList params;
  params.append("prj_id", _prjid);
  XSqlQuery qry = mql.toQuery(params);
  if (qry.first())
  {
    _totalHrBud->setDouble(qry.value("totalhrbud").toDouble());
    _totalHrAct->setDouble(qry.value("totalhract").toDouble());
    _totalHrBal->setDouble(qry.value("totalhrbal").toDouble());
    _totalExpBud->setDouble(qry.value("totalexpbud").toDouble());
    _totalExpAct->setDouble(qry.value("totalexpact").toDouble());
    _totalExpBal->setDouble(qry.value("totalexpbal").toDouble());
  }
  if (qry.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
/* Not sure why the totals are zeroed out 
   else
  {
    _totalHrBud->setDouble(0.0);
    _totalHrAct->setDouble(0.0);
    _totalHrBal->setDouble(0.0);
    _totalExpBud->setDouble(0.0);
    _totalExpAct->setDouble(0.0);
    _totalExpBal->setDouble(0.0);
  }  */

// Populate Task List
  MetaSQLQuery mqltask = mqlLoad("orderActivityByProject", "detail");
  
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

  if(_showIn->isChecked())
    params.append("showIn");

  if (! _privileges->check("ViewAllProjects") && ! _privileges->check("MaintainAllProjects"))
    params.append("owner_username", omfgThis->username());

  XSqlQuery qrytask = mqltask.toQuery(params);

  _prjtask->populate(qrytask, true);
  _prjtask->expandAll();
}

void project::sNumberChanged()
{
  XSqlQuery projectNumberChanged;
  if((cNew == _mode) && (_number->text().length()))
  {
    _number->setText(_number->text().trimmed().toUpper());

    projectNumberChanged.prepare( "SELECT prj_id"
               "  FROM prj"
               " WHERE (prj_number=:prj_number);" );
    projectNumberChanged.bindValue(":prj_number", _number->text());
    projectNumberChanged.exec();
    if(projectNumberChanged.first())
    {
      _number->setEnabled(false);
      _prjid = projectNumberChanged.value("prj_id").toInt();
      _mode = cEdit;
      populate();
    }
    else
    {
      _number->setEnabled(false);
      _mode = cNew;
    }
  }
}

void project::sNewIncident()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("prj_id",  _prjid);
  params.append("crmacct_id",  _crmacct->id());
  params.append("cntct_id",  _cntct->id());
  
  incident *newdlg = new incident(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillTaskList();
}

void project::sNewQuotation()
{
  ParameterList params;
  params.append("mode", "newQuote");
  params.append("prj_id",  _prjid);

  salesOrder *newdlg = new salesOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillTaskList();
}

void project::sNewSalesOrder()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("prj_id",  _prjid);

  salesOrder *newdlg = new salesOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillTaskList();
}


void project::sNewPurchaseOrder()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("prj_id",  _prjid);

  purchaseOrder *newdlg = new purchaseOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillTaskList();
}

void project::sNewWorkOrder()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("prj_id",  _prjid);

  workOrder *newdlg = new workOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillTaskList();
}

void project::sEditOrder()
{
  ParameterList params;

  if(_prjtask->altId() == 15)
  {
    params.append("mode", "editQuote");
    params.append("quhead_id", _prjtask->id());

    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(_prjtask->altId() == 17)
  {
    params.append("mode", "editQuote");
    params.append("soitem_id", _prjtask->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(_prjtask->altId() == 25)
  {
    params.append("mode",      "edit");
    params.append("sohead_id", _prjtask->id());
    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
  }
  else if(_prjtask->altId() == 27)
  {
    params.append("mode", "edit");
    params.append("soitem_id", _prjtask->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(_prjtask->altId() == 35)
  {
    invoice::editInvoice(_prjtask->id(), this);
  }
  else if(_prjtask->altId() == 37)
  {
    params.append("mode", "edit");
    params.append("invcitem_id", _prjtask->id());

    invoiceItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(_prjtask->altId() == 45)
  {
    params.append("mode", "edit");
    params.append("wo_id", _prjtask->id());

    workOrder *newdlg = new workOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(_prjtask->altId() == 65)
  {
    params.append("mode", "edit");
    params.append("pohead_id", _prjtask->id());

    purchaseOrder *newdlg = new purchaseOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(_prjtask->altId() == 67)
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("poitem_id", _prjtask->id());

    purchaseOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(_prjtask->altId() == 105)
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("incdt_id", _prjtask->id());

    incident newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

void project::sViewOrder()
{
  ParameterList params;

  if(_prjtask->altId() == 5)
  {
    params.append("mode", "view");
    params.append("prjtask_id", _prjtask->id());

    task newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(_prjtask->altId() == 15)
  {
    params.append("mode", "viewQuote");
    params.append("quhead_id", _prjtask->id());

    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(_prjtask->altId() == 17)
  {
    params.append("mode", "viewQuote");
    params.append("soitem_id", _prjtask->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(_prjtask->altId() == 25)
  {
    params.append("mode",      "view");
    params.append("sohead_id", _prjtask->id());
    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
  }
  else if(_prjtask->altId() == 27)
  {
    params.append("mode", "view");
    params.append("soitem_id", _prjtask->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(_prjtask->altId() == 35)
  {
    invoice::viewInvoice(_prjtask->id(), this);
  }
  else if(_prjtask->altId() == 37)
  {
    params.append("mode", "view");
    params.append("invcitem_id", _prjtask->id());

    invoiceItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(_prjtask->altId() == 45)
  {
    params.append("mode", "view");
    params.append("wo_id", _prjtask->id());

    workOrder *newdlg = new workOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(_prjtask->altId() == 55)
  {
    params.append("mode", "view");
    params.append("pr_id", _prjtask->id());

    purchaseRequest newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(_prjtask->altId() == 65)
  {
    params.append("mode", "view");
    params.append("pohead_id", _prjtask->id());

    purchaseOrder *newdlg = new purchaseOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(_prjtask->altId() == 67)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("poitem_id", _prjtask->id());

    purchaseOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(_prjtask->altId() == 105)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("incdt_id", _prjtask->id());

    incident newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

