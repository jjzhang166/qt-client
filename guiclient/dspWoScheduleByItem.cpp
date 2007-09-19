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

#include "dspWoScheduleByItem.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>
#include <Q3PopupMenu>
#include <QSqlError>
#include "postProduction.h"
#include "correctProductionPosting.h"
#include "postOperations.h"
#include "correctOperationsPosting.h"
#include "implodeWo.h"
#include "explodeWo.h"
#include "closeWo.h"
#include "printWoTraveler.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "changeWoQty.h"
#include "workOrder.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "dspWoOperationsByWorkOrder.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "rptWoScheduleByItem.h"

/*
 *  Constructs a dspWoScheduleByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoScheduleByItem::dspWoScheduleByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wo, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));

  statusBar()->hide();

  _item->setType(ItemLineEdit::cGeneralManufactured);
  _dates->setStartCaption(tr("Start W/O Start Date:"));
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("End W/O Start Date:"));

  _showOnlyRI->setChecked(_preferences->boolean("ShowRIWorkOrdersByDefault"));

  _wo->addColumn(tr("W/O #"),      -1,            Qt::AlignLeft   );
  _wo->addColumn(tr("Status"),     _statusColumn, Qt::AlignCenter );
  _wo->addColumn(tr("Pri."),       _statusColumn, Qt::AlignCenter );
  _wo->addColumn(tr("Whs."),       _whsColumn,    Qt::AlignCenter );
  _wo->addColumn(tr("Ordered"),    _qtyColumn,    Qt::AlignRight  );
  _wo->addColumn(tr("Received"),   _qtyColumn,    Qt::AlignRight  );
  _wo->addColumn(tr("Start Date"), _dateColumn,   Qt::AlignCenter );
  _wo->addColumn(tr("Due Date"),   _dateColumn,   Qt::AlignCenter );
  
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _item->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoScheduleByItem::~dspWoScheduleByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoScheduleByItem::languageChange()
{
  retranslateUi(this);
}

void dspWoScheduleByItem::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _item->id());
  params.append("print");

  if (_showOnlyRI->isChecked())
    params.append("showOnlyRI");

  if (_showOnlyTopLevel->isChecked())
    params.append("showOnlyTopLevel");

  if (_sortByStartDate->isChecked())
    params.append("sortByStartDate");
  else
    params.append("sortByDueDate");

  rptWoScheduleByItem newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspWoScheduleByItem::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sPostProduction()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  postProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sCorrectProductionPosting()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  correctProductionPosting newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sPostOperations()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  postOperations newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sCorrectOperationsPosting()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  correctOperationsPosting newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sReleaseWO()
{
  q.prepare("SELECT releaseWo(:wo_id, FALSE);");
  q.bindValue(":wo_id", _wo->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
}

void dspWoScheduleByItem::sRecallWO()
{
  q.prepare("SELECT recallWo(:wo_id, FALSE);");
  q.bindValue(":wo_id", _wo->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
}

void dspWoScheduleByItem::sExplodeWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  explodeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sImplodeWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  implodeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sDeleteWO()
{
  q.prepare( "SELECT wo_ordtype, wo_ordid, wo_status "
             "FROM wo "
             "WHERE (wo_id=:wo_id);" );
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first())
  {
    bool toDelete = FALSE;
    QString woStatus = q.value("wo_status").toString();

    if (q.value("wo_ordtype") == "W")
    {
      if (QMessageBox::warning( this, tr("Delete Work Order"),
                                tr( "The Work Order that you selected to delete is a child\n"
                                    "of another Work Order.  If you delete the selected\n"
                                    "Work Order then the Work Order Materials Requirements for\n"
                                    "the Component Item will remain but the Work Order to\n"
                                    "relieve that demand will not.\n"
                                    "Are you sure that you want to delete the selected Work Order?" ),
                                tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
        toDelete = TRUE;
    }
    else if (q.value("wo_ordtype") == "S")
    {
      if (QMessageBox::warning( this, tr("Delete Work Order"),
                                tr( "The Work Order that you selected to delete was created\n"
                                    "to satisfy a Sales Order demand.  If you delete the selected\n"
                                    "Work Order then the Sales Order demand will remain but the\n"
                                    "Work Order to relieve that demand will not.\n"
                                    "Are you sure that you want to delete the selected Work Order?" ),
                                tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
        toDelete = TRUE;
    }
    else
    {
      if (QMessageBox::warning( this, tr("Delete Work Order"),
                                tr( "Are you sure that you want to delete the selected Work Order?" ),
                                tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
        toDelete = TRUE;
    }

    if (toDelete)
    {
      q.prepare("SELECT deleteWo(:wo_id, TRUE) AS returnVal;");
      q.bindValue(":wo_id", _wo->id());
      q.exec();
      if (q.first() && q.value("returnVal").toInt() < 0)
      {
	QString msg;
	if (q.value("returnVal").toInt() == -1)
	{
	  msg = tr("Work Order %1 cannot be deleted because time clock "
			 "entries exist for it.\n")
		   .arg(_wo->columnText(1));
	  if (woStatus == "C")
	    msg += "Please use Close Work Order instead of Delete Work Order.";
	}
	else 
	  msg = tr("Work Order %1 cannot be deleted (reason %2).")
		   .arg(_wo->columnText(1))
		   .arg(q.value("returnVal").toString());

	QMessageBox::information(this, tr("Work Order Postings Exist"), msg);
	return;
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

	return;
      }

      omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
    }
  }
}

void dspWoScheduleByItem::sCloseWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  closeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sPrintTraveler()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  printWoTraveler newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sReprioritizeWo()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  reprioritizeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sRescheduleWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  rescheduleWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sChangeWOQty()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  changeWoQty newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sViewWomatl()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sViewWooper()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoOperationsByWorkOrder *newdlg = new dspWoOperationsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sInventoryAvailabilityByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspInventoryAvailabilityByWorkOrder *newdlg = new dspInventoryAvailabilityByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sPopulateMenu(Q3PopupMenu *pMenu, Q3ListViewItem *selected)
{
  QString status(selected->text(1));
  int     menuItem;

  menuItem = pMenu->insertItem(tr("Edit W/O"), this, SLOT(sEdit()), 0);
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
}

void dspWoScheduleByItem::sFillList()
{
  _wo->clear();

  if ((_item->isValid()) && (_dates->allValid()))
  {
    QString sql( "SELECT wo_id,"
                 " formatWONumber(wo_id) AS wonumber,"
                 " wo_status, wo_priority, warehous_code,"
                 " formatQty(wo_qtyord) AS qtyord,"
                 " formatQty(wo_qtyrcv) AS qtyrcv,"
                 " formatDate(wo_startdate) AS startdate,"
                 " formatDate(wo_duedate) AS duedate,"
                 " ((wo_startdate<=CURRENT_DATE) AND (wo_status IN ('O','E','S','R'))) AS latestart,"
                 " (wo_duedate<=CURRENT_DATE) AS latedue "
                 "FROM wo, itemsite, warehous "
                 "WHERE ((wo_itemsite_id=itemsite_id)"
                 " AND (itemsite_warehous_id=warehous_id)"
                 " AND (itemsite_item_id=:item_id)"
                 " AND (wo_startdate BETWEEN :startDate AND :endDate)" );

    if (_showOnlyRI->isChecked())
      sql += " AND (wo_status IN ('R','I'))";
    else
      sql += " AND (wo_status<>'C')";

    if (_showOnlyTopLevel->isChecked())
      sql += " AND (wo_ordtype<>'W')";

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += ") ";

    if (_sortByStartDate->isChecked())
      sql += "ORDER BY wo_startdate, wo_number, wo_subnumber";
    else
      sql += "ORDER BY wo_duedate, wo_number, wo_subnumber";

    q.prepare(sql);
    _warehouse->bindValue(q);
    _dates->bindValue(q);
    q.bindValue(":item_id", _item->id());
    q.exec();
    while (q.next())
    {
      XListViewItem *last = new XListViewItem( _wo, _wo->lastItem(), q.value("wo_id").toInt(),
                                               q.value("wonumber"), q.value("wo_status"),
                                               q.value("wo_priority"), q.value("warehous_code"),
                                               q.value("qtyord"), q.value("qtyrcv"),
                                               q.value("startdate"), q.value("duedate") );

      if (q.value("latestart").toBool())
        last->setColor(6, "red");

      if (q.value("latedue").toBool())
        last->setColor(7, "red");
    }
  }
}

void dspWoScheduleByItem::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

