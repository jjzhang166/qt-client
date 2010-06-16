/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPurchaseReqsByPlannerCode.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "dspRunningAvailability.h"
#include "mqlutil.h"
#include "purchaseOrder.h"

dspPurchaseReqsByPlannerCode::dspPurchaseReqsByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_pr, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  _plannerCode->setType(ParameterGroup::PlannerCode);

  _pr->addColumn(tr("P/R #"),        _orderColumn,  Qt::AlignLeft,   true,  "pr_number");
  _pr->addColumn(tr("Sub #"),        _orderColumn,  Qt::AlignLeft,   true,  "pr_subnumber");
  _pr->addColumn(tr("Item Number"),  _itemColumn,   Qt::AlignLeft,   true,  "item_number"   );
  _pr->addColumn(tr("Description"),  -1,            Qt::AlignLeft,   true,  "description"   );
  _pr->addColumn(tr("Status"),       _statusColumn, Qt::AlignCenter, true,  "pr_status" );
  _pr->addColumn(tr("Parent Order"), _itemColumn,   Qt::AlignLeft,   true,  "parent"   );
  _pr->addColumn(tr("Create Date"),        -1,    Qt::AlignLeft,  true,  "pr_createdate"  );
  _pr->addColumn(tr("Due Date"),     _dateColumn,   Qt::AlignCenter, true,  "pr_duedate" );
  _pr->addColumn(tr("Qty."),         _qtyColumn,    Qt::AlignRight,  true,  "pr_qtyreq"  );
  _pr->addColumn(tr("Notes"),        -1,    Qt::AlignLeft,  true,  "pr_releasenote"  );
}

dspPurchaseReqsByPlannerCode::~dspPurchaseReqsByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPurchaseReqsByPlannerCode::languageChange()
{
  retranslateUi(this);
}

bool dspPurchaseReqsByPlannerCode::setParams(ParameterList &params)
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return false;
  }

  _plannerCode->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  params.append("manual", tr("Manual"));
  params.append("other",  tr("Other"));

  return true;
}

void dspPurchaseReqsByPlannerCode::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("PurchaseReqsByPlannerCode", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPurchaseReqsByPlannerCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()), 0);
  if (!_privileges->check("ViewInventoryAvailability"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Release P/R..."), this, SLOT(sRelease()), 0);
  if (!_privileges->check("MaintainPurchaseOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete P/R..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainPurchaseRequests"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspPurchaseReqsByPlannerCode::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _pr->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPurchaseReqsByPlannerCode::sRelease()
{
  ParameterList params;
  params.append("mode", "releasePr");
  params.append("pr_id", _pr->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
  else
    delete newdlg;

  sFillList();
}

void dspPurchaseReqsByPlannerCode::sDelete()
{
  q.prepare("SELECT deletePr(:pr_id) AS _result;");
  q.bindValue(":pr_id", _pr->id());
  q.exec();

  sFillList();
}

void dspPurchaseReqsByPlannerCode::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("purchase", "purchaserequests");
  q = mql.toQuery(params);
  _pr->populate(q, TRUE);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
