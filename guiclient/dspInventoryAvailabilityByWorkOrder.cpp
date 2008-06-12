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
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
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

#include "dspInventoryAvailabilityByWorkOrder.h"

#include <QVariant>
#include <QMenu>
#include <QMessageBox>
#include <openreports.h>
#include "inputManager.h"
#include "dspAllocations.h"
#include "dspOrders.h"
#include "dspRunningAvailability.h"
#include "workOrder.h"
#include "purchaseOrder.h"
#include "createCountTagsByItem.h"
#include "dspSubstituteAvailabilityByItem.h"

/*
 *  Constructs a dspInventoryAvailabilityByWorkOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspInventoryAvailabilityByWorkOrder::dspInventoryAvailabilityByWorkOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_womatl, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_onlyShowShortages, SIGNAL(clicked()), this, SLOT(sFillList()));

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _womatl->addColumn("itemType",          0,           Qt::AlignCenter );
  _womatl->addColumn(tr("Item Number"),   _itemColumn, Qt::AlignLeft   );
  _womatl->addColumn(tr("Description"),   -1,          Qt::AlignLeft   );
  _womatl->addColumn(tr("UOM"),           _uomColumn,  Qt::AlignCenter );
  _womatl->addColumn(tr("QOH"),           _qtyColumn,  Qt::AlignRight  );
  _womatl->addColumn(tr("This Alloc."),   _qtyColumn,  Qt::AlignRight  );
  _womatl->addColumn(tr("Total Alloc."),  _qtyColumn,  Qt::AlignRight  );
  _womatl->addColumn(tr("Orders"),        _qtyColumn,  Qt::AlignRight  );
  _womatl->addColumn(tr("This Avail."),   _qtyColumn,  Qt::AlignRight  );
  _womatl->addColumn(tr("Total Avail."),  _qtyColumn,  Qt::AlignRight  );

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspInventoryAvailabilityByWorkOrder::~dspInventoryAvailabilityByWorkOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspInventoryAvailabilityByWorkOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityByWorkOrder::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  _onlyShowShortages->setChecked(pParams.inList("onlyShowShortages"));

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspInventoryAvailabilityByWorkOrder::sPrint()
{
  if(!_wo->isValid())
  {
    QMessageBox::warning(this, tr("Invalid W/O Selected"),
      tr("You must specify a valid Work Order Number.") );
    return;
  }

  ParameterList params;

  params.append("wo_id", _wo->id());

  if(_onlyShowShortages->isChecked())
    params.append("onlyShowShortages");

  orReport report("WOMaterialAvailabilityByWorkOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInventoryAvailabilityByWorkOrder::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  int menuItem;

  menuItem = pMenu->insertItem("View Allocations...", this, SLOT(sViewAllocations()), 0);
  if (selected->text(6).toDouble() == 0.0)
    pMenu->setItemEnabled(menuItem, FALSE);
    
  menuItem = pMenu->insertItem("View Orders...", this, SLOT(sViewOrders()), 0);
  if (selected->text(7).toDouble() == 0.0)
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem("Running Availability...", this, SLOT(sRunningAvailability()), 0);
  menuItem = pMenu->insertItem("Substitute Availability...", this, SLOT(sViewSubstituteAvailability()), 0);

  if (selected->text(0) == "P")
  {
    pMenu->insertSeparator();
    menuItem = pMenu->insertItem("Issue Purchase Order...", this, SLOT(sIssuePO()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (selected->text(1) == "M")
  {
    pMenu->insertSeparator();
    menuItem = pMenu->insertItem("Issue Work Order...", this, SLOT(sIssueWO()), 0);
    if (!_privileges->check("MaintainWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  pMenu->insertSeparator();
  menuItem = pMenu->insertItem("Issue Count Tag...", this, SLOT(sIssueCountTag()), 0);
  if (!_privileges->check("IssueCountTags"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspInventoryAvailabilityByWorkOrder::sViewAllocations()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  q.bindValue(":womatl_id", _womatl->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", _womatl->id());
    params.append("byDate", q.value("womatl_duedate"));
    params.append("run");

    dspAllocations *newdlg = new dspAllocations();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspInventoryAvailabilityByWorkOrder::sViewOrders()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  q.bindValue(":womatl_id", _womatl->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", _womatl->id());
    params.append("byDate", q.value("womatl_duedate"));
    params.append("run");

    dspOrders *newdlg = new dspOrders();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspInventoryAvailabilityByWorkOrder::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _womatl->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sViewSubstituteAvailability()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  q.bindValue(":womatl_id", _womatl->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", _womatl->id());
    params.append("byDate", q.value("womatl_duedate"));
    params.append("run");

    dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
//  ToDo
}

void dspInventoryAvailabilityByWorkOrder::sIssuePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _womatl->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sIssueWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _womatl->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _womatl->id());

  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByWorkOrder::sFillList()
{
  _womatl->clear();

  if (_wo->isValid())
  {
    QString sql( "SELECT itemsite_id, womatl_id, type,"
                 "       item_number, item_description, uom_name, item_picklist,"
                 "       qoh, formatQty(qoh) AS f_qoh,"
                 "       formatQty(wobalance) AS f_wobalance,"
                 "       formatQty(allocated) AS f_allocated,"
                 "       ordered, formatQty(ordered) AS f_ordered,"
                 "       (qoh + ordered - wobalance) AS woavail,"
                 "       formatQty(qoh + ordered - wobalance) AS f_woavail,"
                 "       (qoh + ordered - allocated) AS totalavail,"
                 "       formatQty(qoh + ordered - allocated) AS f_totalavail,"
                 "       reorderlevel "
                 "FROM ( SELECT itemsite_id, womatl_id,"
                 "              CASE WHEN itemsite_supply THEN item_type"
                 "                   ELSE ''"
                 "              END AS type,"
                 "              item_number, (item_descrip1 || ' ' || item_descrip2) AS item_description,"
                 "              uom_name, item_picklist,"
                 "              noNeg(itemsite_qtyonhand) AS qoh,"
                 "              noNeg(itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, womatl_qtyreq - womatl_qtyiss)) AS wobalance,"
                 "              qtyAllocated(itemsite_id, womatl_duedate) AS allocated,"
                 "              qtyOrdered(itemsite_id, womatl_duedate) AS ordered,"
                 "              CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel"
                 "       FROM wo, womatl, itemsite, item, uom "
                 "       WHERE ( (womatl_wo_id=wo_id)"
                 "        AND (womatl_itemsite_id=itemsite_id)"
                 "        AND (itemsite_item_id=item_id)"
                 "        AND (item_inv_uom_id=uom_id)"
                 "        AND (womatl_wo_id=:wo_id)) ) AS data " );

    if (_onlyShowShortages->isChecked())
      sql += "WHERE ( ((qoh + ordered - allocated) < 0)"
             " OR ((qoh + ordered - wobalance) < 0) ) ";

    sql += "ORDER BY item_number;";

    q.prepare(sql);
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    XTreeWidgetItem * last = 0;
    while (q.next())
    {
      last = new XTreeWidgetItem( _womatl, last,
                                  q.value("itemsite_id").toInt(), q.value("womatl_id").toInt(),
                                  q.value("type"), q.value("item_number"),
                                  q.value("item_description"), q.value("uom_name"),
                                  q.value("f_qoh"), q.value("f_wobalance"),
                                  q.value("f_allocated"), q.value("f_ordered"),
                                  q.value("f_woavail"), q.value("f_totalavail") );

      if (q.value("qoh").toDouble() < 0)
        last->setTextColor(4, "red");
      else if (q.value("qoh").toDouble() < q.value("reorderlevel").toDouble())
        last->setTextColor(4, "orange");

      if (q.value("woavail").toDouble() < 0.0)
        last->setTextColor(8, "red");
      else if (q.value("woavail").toDouble() <= q.value("reorderlevel").toDouble())
        last->setTextColor(8, "orange");

      if (q.value("totalavail").toDouble() < 0.0)
        last->setTextColor(9, "red");
      else if (q.value("totalavail").toDouble() <= q.value("reorderlevel").toDouble())
        last->setTextColor(9, "orange");
    }
  }
}

