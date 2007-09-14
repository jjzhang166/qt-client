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

#include "dspRunningAvailability.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "firmPlannedOrder.h"
#include "mqlutil.h"
#include "purchaseRequest.h"
#include "rptRunningAvailability.h"
#include "salesOrder.h"
#include "transferOrder.h"
#include "workOrder.h"

#define ORDERTYPE_COL		0
#define ORDERNUM_COL		1
#define DUEDATE_COL		3
#define RUNNINGAVAIL_COL	7

dspRunningAvailability::dspRunningAvailability(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_availability, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_item,	SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print,	SIGNAL(clicked()),  this, SLOT(sPrint()));
  connect(_showPlanned,	SIGNAL(clicked()),  this, SLOT(sFillList()));
  connect(_warehouse,	SIGNAL(newID(int)), this, SLOT(sFillList()));

  _availability->addColumn(tr("Order Type"),      _itemColumn, Qt::AlignLeft  );
  _availability->addColumn(tr("Order #"),         _itemColumn, Qt::AlignLeft  );
  _availability->addColumn(tr("Source/Destination"),       -1, Qt::AlignLeft  );
  _availability->addColumn(tr("Due Date"),        _dateColumn, Qt::AlignLeft  );
  _availability->addColumn(tr("Ordered"),         _qtyColumn,  Qt::AlignRight );
  _availability->addColumn(tr("Received"),        _qtyColumn,  Qt::AlignRight );
  _availability->addColumn(tr("Balance"),         _qtyColumn,  Qt::AlignRight );
  _availability->addColumn(tr("Running Avail."),  _qtyColumn,  Qt::AlignRight );

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

dspRunningAvailability::~dspRunningAvailability()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspRunningAvailability::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspRunningAvailability::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(TRUE);
    _showPlanned->setFocus();
  }

  return NoError;
}

void dspRunningAvailability::setParams(ParameterList & params)
{
  params.append("item_id",	_item->id());
  params.append("warehous_id",	_warehouse->id());

  params.append("firmPo",	tr("Planned P/O (firmed)"));
  params.append("plannedPo",	tr("Planned P/O"));
  params.append("firmWo",	tr("Planned W/O (firmed)"));
  params.append("plannedWo",	tr("Planned W/O"));
  params.append("firmWoReq",	tr("Planned W/O Req. (firmed)"));
  params.append("plannedWoReq",	tr("Planned W/O Req."));
  params.append("pr",		tr("Purchase Request"));

  if (_showPlanned->isChecked())
    params.append("showPlanned");

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (_metrics->value("Application") == "OpenMFG")
    params.append("showMRPplan");
}

void dspRunningAvailability::sPrint()
{
  ParameterList params;
  setParams(params);
  params.append("print");

  rptRunningAvailability newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspRunningAvailability::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (pSelected->text(ORDERNUM_COL) == tr("Planned W/O (firmed)") ||
      pSelected->text(ORDERNUM_COL) == tr("Planned W/O") ||
      pSelected->text(ORDERNUM_COL) == tr("Planned P/O (firmed)") ||
      pSelected->text(ORDERNUM_COL) == tr("Planned P/O") )
  {
    if (pSelected->text(ORDERNUM_COL) == tr("Planned W/O (firmed)") ||
	pSelected->text(ORDERNUM_COL) == tr("Planned P/O (firmed)") )
      pMenu->insertItem(tr("Soften Order..."), this, SLOT(sSoftenOrder()), 0);
    else
      pMenu->insertItem(tr("Firm Order..."), this, SLOT(sFirmOrder()), 0);
 
    pMenu->insertItem(tr("Release Order..."), this, SLOT(sReleaseOrder()), 0);
    pMenu->insertItem(tr("Delete Order..."), this, SLOT(sDeleteOrder()), 0);
  }
  
  else if (pSelected->text(ORDERTYPE_COL).contains("W/O") &&
	  !(pSelected->text(ORDERNUM_COL) == tr("Planned W/O Req. (firmed)") ||
	    pSelected->text(ORDERNUM_COL) == tr("Planned W/O Req.")))
    pMenu->insertItem(tr("View Work Order Details..."), this, SLOT(sViewWo()), 0);
  else if (pSelected->text(ORDERTYPE_COL) == "S/O")
  {
    menuItem = pMenu->insertItem(tr("View Sales Order..."), this, SLOT(sViewSo()), 0);
    pMenu->setItemEnabled(menuItem, _privleges->check("ViewSalesOrders"));
  }

  else if (pSelected->text(ORDERTYPE_COL) == "T/O")
  {
    menuItem = pMenu->insertItem(tr("View Transfer Order..."), this, SLOT(sViewTo()), 0);
    pMenu->setItemEnabled(menuItem, _privleges->check("ViewTransferOrders"));
  }

}

void dspRunningAvailability::sFirmOrder()
{
  ParameterList params;
  params.append("planord_id", _availability->id());

  firmPlannedOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void dspRunningAvailability::sSoftenOrder()
{
  q.prepare( "UPDATE planord "
             "SET planord_firm=FALSE "
             "WHERE (planord_id=:planord_id);" );
  q.bindValue(":planord_id", _availability->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void dspRunningAvailability::sReleaseOrder()
{
  // TODO
  if (_availability->currentItem()->text(ORDERTYPE_COL) == tr("Planned W/O (firmed)") ||
      _availability->currentItem()->text(ORDERTYPE_COL) == tr("Planned W/O"))
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _availability->id());

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
#if 0
    if (newdlg.exec() != QDialog::Rejected)
    {
      sDeleteOrder();
      sFillList();
    }
#endif
  }
  else if (_availability->currentItem()->text(ORDERTYPE_COL) == tr("Planned P/O (firmed)") ||
	  _availability->currentItem()->text(ORDERTYPE_COL) == tr("Planned P/O"))
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _availability->id());

    purchaseRequest newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
      sFillList();
  }
}

void dspRunningAvailability::sDeleteOrder()
{
  q.prepare( "SELECT deletePlannedOrder(:planord_id, FALSE) AS result;" );
  q.bindValue(":planord_id", _availability->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    /* TODO: uncomment when deletePlannedOrder returns INTEGER instead of BOOLEAN
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deletePlannedOrder", result), __FILE__, __LINE__);
      return;
    }
    */
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void dspRunningAvailability::sViewSo()
{
  ParameterList params;
  salesOrder::viewSalesOrder(_availability->id());
}

void dspRunningAvailability::sViewTo()
{
  ParameterList params;
  transferOrder::viewTransferOrder(_availability->id());
}

void dspRunningAvailability::sViewWo()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _availability->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspRunningAvailability::sFillList()
{
  _availability->clear();

  if (_item->isValid())
  {
    q.prepare( "SELECT itemsite_qtyonhand,"
               "       formatQty(itemsite_qtyonhand) AS f_qoh,"
               "       CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
               "       formatQty(CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END) AS f_reorderlevel,"
               "       formatQty(CASE WHEN(itemsite_useparams) THEN itemsite_ordertoqty ELSE 0.0 END) AS f_ordertoqty,"
               "       formatQty(CASE WHEN(itemsite_useparams) THEN itemsite_multordqty ELSE 0.0 END) AS f_multorderqty "
               "FROM item, itemsite "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=:warehous_id)"
               " AND (item_id=:item_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.exec();
    if (q.first())
    {
      _qoh->setText(q.value("f_qoh").toString());
      _reorderLevel->setText(q.value("f_reorderlevel").toString());
      _orderMultiple->setText(q.value("f_multorderqty").toString());
      _orderToQty->setText(q.value("f_ordertoqty").toString());

      double  reorderLevel        = q.value("reorderlevel").toDouble();
      double  runningAvailability = q.value("itemsite_qtyonhand").toDouble();
      QString sql;

      MetaSQLQuery mql = mqlLoad(":/ms/displays/RunningAvailability/FillListDetail.mql");
      ParameterList params;
      setParams(params);
      q = mql.toQuery(params);
      XTreeWidgetItem *last = 0;
      while (q.next())
      {
	runningAvailability += q.value("balance").toDouble();

	last = new XTreeWidgetItem(_availability, last,
				   q.value("orderid").toInt(),
				   q.value("altorderid").toInt(),
				   q.value("ordertype"),
				   q.value("ordernumber"),
				   q.value("item_number"),
				   q.value("duedate"),
				   q.value("f_qtyordered"),
				   q.value("f_qtyreceived"),
				   q.value("f_balance"),
				   formatQty(runningAvailability) );

	if (q.value("late").toBool())
	  last->setTextColor(DUEDATE_COL, "red");

	if (runningAvailability < 0.0)
	  last->setTextColor(RUNNINGAVAIL_COL, "red");
	else if (runningAvailability < reorderLevel)
	  last->setTextColor(RUNNINGAVAIL_COL, "orange");

	if (last->text(ORDERTYPE_COL).contains("Planned P/O") ||
	    last->text(ORDERTYPE_COL).contains("Planned W/O") )
	  last->setTextColor("blue");
      }
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _qoh->setText("0.00");
    _reorderLevel->setText("0.00");
    _orderMultiple->setText("0.00");
    _orderToQty->setText("0.00");
  }
}
