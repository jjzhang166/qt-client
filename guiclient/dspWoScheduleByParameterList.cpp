/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "dspWoScheduleByParameterList.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "bom.h"
#include "boo.h"
#include "changeWoQty.h"
#include "closeWo.h"
#include "correctOperationsPosting.h"
#include "correctProductionPosting.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspRunningAvailability.h"
#include "dspWoEffortByWorkOrder.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "dspWoOperationsByWorkOrder.h"
#include "explodeWo.h"
#include "implodeWo.h"
#include "issueWoMaterialItem.h"
#include "postOperations.h"
#include "postProduction.h"
#include "printWoTraveler.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "salesOrderInformation.h"
#include "storedProcErrorLookup.h"
#include "workOrder.h"

dspWoScheduleByParameterList::dspWoScheduleByParameterList(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_wo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_postProduction, SIGNAL(clicked()), this, SLOT(sPostProduction()));
  connect(_postOperations, SIGNAL(clicked()), this, SLOT(sPostOperations()));
  connect(_printTraveler, SIGNAL(clicked()), this, SLOT(sPrintTraveler()));
  connect(_wo, SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtons()));

  _dates->setStartCaption(tr("Start W/O Start Date:"));
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("End W/O Start Date:"));

  _wo->addColumn(tr("parentType"),  0,             Qt::AlignCenter );
  _wo->addColumn(tr("W/O #"),       _orderColumn,  Qt::AlignLeft   );
  _wo->addColumn(tr("Status"),      _statusColumn, Qt::AlignCenter );
  _wo->addColumn(tr("Pri."),        _statusColumn, Qt::AlignCenter );
  _wo->addColumn(tr("Whs."),        _whsColumn,    Qt::AlignCenter );
  _wo->addColumn(tr("Item Number"), _itemColumn,   Qt::AlignLeft   );
  _wo->addColumn(tr("Description"), -1,            Qt::AlignLeft   );
  _wo->addColumn(tr("UOM"),         _uomColumn,    Qt::AlignCenter );
  _wo->addColumn(tr("Ordered"),     _qtyColumn,    Qt::AlignRight  );
  _wo->addColumn(tr("Received"),    _qtyColumn,    Qt::AlignRight  );
  _wo->addColumn(tr("Start Date"),  _dateColumn,   Qt::AlignRight  );
  _wo->addColumn(tr("Due Date"),    _dateColumn,   Qt::AlignRight  );
  _wo->addColumn(tr("Condition"),      _dateColumn,   Qt::AlignLeft   );

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
  
  if (!_metrics->boolean("Routings"))
    _postOperations->hide();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

dspWoScheduleByParameterList::~dspWoScheduleByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspWoScheduleByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspWoScheduleByParameterList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("classcode", &valid);
  if (valid)
  {
    _parameter->setType(ClassCode);
    setCaption(tr("W/O Schedule by Class Code"));
  }

  param = pParams.value("plancode", &valid);
  if (valid)
  {
    _parameter->setType(PlannerCode);
    setCaption(tr("W/O Schedule by Planner Code"));
  }

  param = pParams.value("plancode_id", &valid);
  if (valid)
    _parameter->setId(param.toInt());

  param = pParams.value("itemgrp", &valid);
  if (valid)
  {
    _parameter->setType(ItemGroup);
    setCaption(tr("W/O Schedule by Item Group"));
  }

  param = pParams.value("wrkcnt", &valid);
  if (valid)
  {
    _parameter->setType(WorkCenter);
    setCaption(tr("W/O Schedule by Work Center"));
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());
  
  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());
    
  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspWoScheduleByParameterList::setParams(ParameterList &pParams)
{
  _warehouse->appendValue(pParams);
  _parameter->appendValue(pParams);
  _dates->appendValue(pParams);

  if (_showOnlyRI->isChecked())
    pParams.append("showOnlyRI");

  if (_showOnlyTopLevel->isChecked())
    pParams.append("showOnlyTopLevel");

  if(_sortByStartDate->isChecked())
    pParams.append("sortByStartDate");
  else if(_sortByDueDate->isChecked())
    pParams.append("sortByDueDate");
  else
    pParams.append("sortByItemNumber");

  return true;
}

void dspWoScheduleByParameterList::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("WOScheduleByParameterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoScheduleByParameterList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sPostProduction()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  postProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sCorrectProductionPosting()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  correctProductionPosting newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sPostOperations()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  postOperations newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sCorrectOperationsPosting()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  correctOperationsPosting newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sReleaseWO()
{
  q.prepare("SELECT releaseWo(:wo_id, FALSE);");
  q.bindValue(":wo_id", _wo->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
}

void dspWoScheduleByParameterList::sRecallWO()
{
  q.prepare("SELECT recallWo(:wo_id, FALSE);");
  q.bindValue(":wo_id", _wo->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
}

void dspWoScheduleByParameterList::sExplodeWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  explodeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sImplodeWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  implodeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sDeleteWO()
{
  q.prepare( "SELECT wo_ordtype "
             "FROM wo "
             "WHERE (wo_id=:wo_id);" );
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first())
  {
    QString question;
    if (q.value("wo_ordtype") == "W")
      question = tr("The Work Order that you selected to delete is a child of "
		    "another Work Order.  If you delete the selected Work "
		    "Order then the Work Order Materials Requirements for the "
		    "Component Item will remain but the Work Order to relieve "
		    "that demand will not. Are you sure that you want to "
		    "delete the selected Work Order?" );
    else if (q.value("wo_ordtype") == "S")
      question = tr("The Work Order that you selected to delete was created to "
		    "satisfy a Sales Order demand.  If you delete the selected "
		    "Work Order then the Sales Order demand will remain but "
		    "the Work Order to relieve that demand will not. Are you "
		    "sure that you want to delete the selected Work Order?" );
    else
      question = tr("Are you sure that you want to delete the selected "
		    "Work Order?");

    q.prepare("SELECT deleteWo(:wo_id, TRUE) AS returnVal;");
    q.bindValue(":wo_id", _wo->id());
    q.exec();

    if (q.first())
    {
      int result = q.value("returnVal").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteWo", result));
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

    omfgThis->sWorkOrdersUpdated(-1, TRUE);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspWoScheduleByParameterList::sCloseWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  closeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sPrintTraveler()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  printWoTraveler newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sViewWomatl()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sViewWooper()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoOperationsByWorkOrder *newdlg = new dspWoOperationsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sInventoryAvailabilityByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspInventoryAvailabilityByWorkOrder *newdlg = new dspInventoryAvailabilityByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sReprioritizeWo()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  reprioritizeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sRescheduleWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  rescheduleWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sChangeWOQty()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  changeWoQty newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sViewParentSO()
{
  ParameterList params;
  params.append("soitem_id", _wo->altId());

  salesOrderInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sViewParentWO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _wo->altId());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sPopulateMenu(QMenu *pMenu,  QTreeWidgetItem *selected)
{
  QString status(selected->text(2));
  int     menuItem;

  menuItem = pMenu->insertItem(tr("Edit W/O"), this, SLOT(sEdit()), 0);
  if (!_privleges->check("MaintainWorkOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View W/O"), this, SLOT(sView()), 0);

  pMenu->insertSeparator();

  if (status == "E")
  {
    menuItem = pMenu->insertItem(tr("Release W/O"), this, SLOT(sReleaseWO()), 0);
    if (!_privleges->check("ReleaseWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (status == "R")
  {
    menuItem = pMenu->insertItem(tr("Recall W/O"), this, SLOT(sRecallWO()), 0);
    if (!_privleges->check("RecallWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if ((status == "E") || (status == "R") || (status == "I"))
  {
    menuItem = pMenu->insertItem(tr("Post Production..."), this, SLOT(sPostProduction()), 0);
    if (!_privleges->check("PostProduction"))
      pMenu->setItemEnabled(menuItem, FALSE);

    if (status != "E")
    {
      menuItem = pMenu->insertItem(tr("Correct Production Posting..."), this, SLOT(sCorrectProductionPosting()), 0);
      if (!_privleges->check("PostProduction"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }

    if (_metrics->boolean("Routings"))
    {
      menuItem = pMenu->insertItem(tr("Post Operations..."), this, SLOT(sPostOperations()), 0);
      if (!_privleges->check("PostWoOperations"))
        pMenu->setItemEnabled(menuItem, FALSE);

      if (status != "E")
      { 
        menuItem = pMenu->insertItem(tr("Correct Operations Posting..."), this, SLOT(sCorrectOperationsPosting()), 0);
        if (!_privleges->check("PostWoOperations"))
          pMenu->setItemEnabled(menuItem, FALSE);
      }
    }

    pMenu->insertSeparator();
  }

  if (status == "O")
  {
    menuItem = pMenu->insertItem(tr("Explode W/O..."), this, SLOT(sExplodeWO()), 0);
    if (!_privleges->check("ExplodeWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (status == "E")
  {
    menuItem = pMenu->insertItem(tr("Implode W/O..."), this, SLOT(sImplodeWO()), 0);
    if (!_privleges->check("ImplodeWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if ((status == "O") || (status == "E"))
  {
    menuItem = pMenu->insertItem(tr("Delete W/O..."), this, SLOT(sDeleteWO()), 0);
    if (!_privleges->check("DeleteWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else
  {
    menuItem = pMenu->insertItem(tr("Close W/O..."), this, SLOT(sCloseWO()), 0);
    if (!_privleges->check("CloseWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  pMenu->insertSeparator();

  if ((status == "E") || (status == "R") || (status == "I"))
  {
    menuItem = pMenu->insertItem(tr("View W/O Material Requirements..."), this, SLOT(sViewWomatl()), 0);
    if (!_privleges->check("ViewWoMaterials"))
      pMenu->setItemEnabled(menuItem, FALSE);
      
    if (_metrics->boolean("Routings"))
    {
      menuItem = pMenu->insertItem(tr("View W/O Operations..."), this, SLOT(sViewWooper()), 0);
      if (!_privleges->check("ViewWoOperations"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }

    menuItem = pMenu->insertItem(tr("Inventory Availability by Work Order..."), this, SLOT(sInventoryAvailabilityByWorkOrder()), 0);
    if (!_privleges->check("ViewInventoryAvailability"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Print Traveler..."), this, SLOT(sPrintTraveler()), 0);
    if (!_privleges->check("PrintWorkOrderPaperWork"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Issue Material Item..."), this, SLOT(sIssueWoMaterialItem()));
    pMenu->setItemEnabled(menuItem, _privleges->check("IssueWoMaterials"));
  }

  if ((status == "O") || (status == "E"))
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Reprioritize W/O..."), this, SLOT(sReprioritizeWo()), 0);
    if (!_privleges->check("ReprioritizeWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Reschedule W/O..."), this, SLOT(sRescheduleWO()), 0);
    if (!_privleges->check("RescheduleWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Change W/O Quantity..."), this, SLOT(sChangeWOQty()), 0);
    if (!_privleges->check("ChangeWorkOrderQty"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  pMenu->insertSeparator();
  
  menuItem = pMenu->insertItem(tr("View Bill of Materials..."), this, SLOT(sViewBOM()), 0);
  pMenu->setItemEnabled(menuItem, _privleges->check("ViewBOMs"));
  if (_metrics->boolean("Routings"))
  {
    menuItem = pMenu->insertItem(tr("View Bill of Operations..."), this, SLOT(sViewBOO()), 0);
    pMenu->setItemEnabled(menuItem, _privleges->check("ViewBOOs"));
  }

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()), 0);

  if (_metrics->boolean("Routings"))
  {
    menuItem = pMenu->insertItem(tr("Production Time Clock by Work Order..."), this, SLOT(sDspWoEffortByWorkOrder()), 0);
    pMenu->setItemEnabled(menuItem, (_privleges->check("MaintainWoTimeClock") || _privleges->check("ViewWoTimeClock")));
  }

  if (_wo->altId() != -1)
  {
    if (selected->text(0) == "S")
    {
      pMenu->insertSeparator();
      menuItem = pMenu->insertItem(tr("View Parent Sales Order Information..."), this, SLOT(sViewParentSO()), 0);
    }
    else if (selected->text(0) == "W")
    {
      pMenu->insertSeparator();
      menuItem = pMenu->insertItem(tr("View Parent Work Order Information..."), this, SLOT(sViewParentWO()), 0);
    }
  }
}

void dspWoScheduleByParameterList::sFillList()
{
  int woid = _wo->id();
  _wo->clear();

  QString sql( "SELECT wo_id, wo_ordtype,"
               "       CASE WHEN (wo_ordid IS NULL) THEN -1"
               "            ELSE wo_ordid"
               "       END AS orderid,"
               "       formatWONumber(wo_id) as wonumber,"
               "       wo_status, wo_priority, warehous_code,"
               "       item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
               "       uom_name,"
               "       formatQty(wo_qtyord) as ordered,"
               "       formatQty(wo_qtyrcv) as received,"
               "       formatDate(wo_startdate) as startdate,"
               "       formatDate(wo_duedate) as duedate,"
	       "       ((wo_startdate<=CURRENT_DATE)"
	       "         AND (wo_status IN ('O','E','S','R'))) AS latestart,"
               "       (wo_duedate<=CURRENT_DATE) AS latedue "
               "FROM wo, itemsite, warehous, item, uom "
               "WHERE ( (wo_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (itemsite_warehous_id=warehous_id)"
	       " AND (wo_startdate BETWEEN <? value(\"startDate\") ?>"
	       "                       AND <? value(\"endDate\") ?>)"
	       "<? if exists(\"warehous_id\") ?>"
	       " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"showOnlyRI\") ?>"
	       " AND (wo_status IN ('R','I'))"
	       "<? else ?>"
	       " AND (wo_status<>'C')"
	       "<? endif ?>"
	       "<? if exists(\"showOnlyTopLevel\") ?>"
	       " AND (wo_ordtype<>'W')"
	       "<? endif ?>"
	       "<? if exists(\"classcode_id\") ?>"
	       " AND (item_classcode_id=<? value(\"classcode_id\") ?>)"
	       "<? elseif exists(\"itemgrp_id\") ?>"
	       " AND (item_id IN (SELECT itemgrpitem_item_id FROM itemgrpitem WHERE (itemgrpitem_itemgrp_id=<? value(\"itemgrp_id\") ?>)))"
	       "<? elseif exists(\"plancode_id\") ?>"
	       " AND (itemsite_plancode_id=<? value(\"plancode_id\") ?>)"
	       "<? elseif exists(\"wrkcnt_id\") ?>"
	       " AND (wo_id IN (SELECT wooper_wo_id FROM wooper WHERE (wooper_wrkcnt_id=<? value(\"wrkcnt_id\") ?>)))"
	       "<? elseif exists(\"classcode_pattern\") ?>"
	       " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ <? value(\"classcode_pattern\") ?>)))"
	       "<? elseif exists(\"itemgrp_pattern\") ?>"
	       " AND (item_id IN (SELECT itemgrpitem_item_id FROM itemgrpitem, itemgrp WHERE ( (itemgrpitem_itemgrp_id=itemgrp_id) AND (itemgrp_name ~ <? value(\"itemgrp_pattern\") ?>) ) ))"
	       "<? elseif exists(\"plancode_pattern\") ?>"
	       " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ <? value(\"plancode_pattern\") ?>)))"
	       "<? elseif exists(\"wrkcnt_pattern\") ?>"
	       " AND (wo_id IN (SELECT wooper_wo_id FROM wooper, wrkcnt WHERE ((wooper_wrkcnt_id=wrkcnt_id) AND (wrkcnt_code ~ <? value(\"wrkcnt_pattern\") ?>))))"
	       "<? endif ?>"
	       ") "
	       "ORDER BY "
	       "<? if exists(\"sortByStartDate\") ?>"
	       "	wo_startdate,"
	       "<? elseif exists(\"sortByDueDate\") ?>"
	       "	wo_duedate,"
	       "<? elseif exists(\"sortByItemNumber\") ?>"
	       "        item_number,"
	       "<? endif ?>"
	       " wo_number, wo_subnumber" );

  MetaSQLQuery mql(sql);
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  while (q.next())
  {
    XTreeWidgetItem *last = new XTreeWidgetItem( _wo, q.value("wo_id").toInt(), q.value("orderid").toInt(),
                                             q.value("wo_ordtype"), q.value("wonumber"),
                                             q.value("wo_status"), q.value("wo_priority"),
                                             q.value("warehous_code"), q.value("item_number"),
                                             q.value("description"), q.value("uom_name"),
                                             q.value("ordered"), q.value("received"),
                                             q.value("startdate") );
    last->setText(11, q.value("duedate").toString());

    if (q.value("latestart").toBool())
      last->setTextColor(10, "red");

    if (q.value("latedue").toBool())
    {
      last->setTextColor(11, "red");
      last->setText(12, tr("Overdue"));
      last->setTextColor(12, "red");
    }
    else
      last->setText(12, tr("On Time"));

    if(last->id() == woid)
      _wo->setCurrentItem(last);
  }

  sHandleButtons();
}

void dspWoScheduleByParameterList::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

void dspWoScheduleByParameterList::sHandleButtons()
{
  QTreeWidgetItem * selected = _wo->currentItem();
  if(selected)
  {
    if((selected->text(2) == "E") || (selected->text(2) == "R") || (selected->text(2) == "I"))
    {
      _postProduction->setEnabled(_privleges->check("PostProduction"));
      _postOperations->setEnabled(_privleges->check("PostWoOperations"));
      _printTraveler->setEnabled(_privleges->check("PrintWorkOrderPaperWork"));      
      return;
    }
  }
  
  _postProduction->setEnabled(false);
  _postOperations->setEnabled(false);
  _printTraveler->setEnabled(false);
}

void dspWoScheduleByParameterList::sDspWoEffortByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoEffortByWorkOrder *newdlg = new dspWoEffortByWorkOrder();
  enum SetResponse setresp = newdlg->set(params);
  if (setresp == NoError || setresp == NoError_Run)
    omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sIssueWoMaterialItem()
{
  issueWoMaterialItem newdlg(this);
  ParameterList params;
  params.append("wo_id", _wo->id());
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void dspWoScheduleByParameterList::sDspRunningAvailability()
{
  q.prepare("SELECT wo_itemsite_id FROM wo WHERE (wo_id=:id);");
  q.bindValue(":id", _wo->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("wo_itemsite_id"));
    params.append("run");

    dspRunningAvailability *newdlg = new dspRunningAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspWoScheduleByParameterList::sViewBOM()
{
  q.prepare("SELECT itemsite_item_id "
	    "FROM wo, itemsite "
	    "WHERE ((wo_itemsite_id=itemsite_id)"
	    "  AND  (wo_id=:id));");
  q.bindValue(":id", _wo->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("item_id", q.value("itemsite_item_id"));
    params.append("mode", "view");

    BOM *newdlg = new BOM();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspWoScheduleByParameterList::sViewBOO()
{
  q.prepare("SELECT itemsite_item_id "
	    "FROM wo, itemsite "
	    "WHERE ((wo_itemsite_id=itemsite_id)"
	    "  AND  (wo_id=:id));");
  q.bindValue(":id", _wo->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("item_id", q.value("itemsite_item_id"));
    params.append("mode", "view");

    boo *newdlg = new boo();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
