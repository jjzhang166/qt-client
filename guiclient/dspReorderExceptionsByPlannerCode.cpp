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

#include "dspReorderExceptionsByPlannerCode.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "dspRunningAvailability.h"
#include "workOrder.h"

dspReorderExceptionsByPlannerCode::dspReorderExceptionsByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_exception, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _plannerCode->setType(PlannerCode);

  _exception->addColumn(tr("Whs."),           _whsColumn,  Qt::AlignCenter );
  _exception->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft   );
  _exception->addColumn(tr("Description"),    -1,          Qt::AlignLeft   );
  _exception->addColumn(tr("Exception Date"), _dateColumn, Qt::AlignCenter );
  _exception->addColumn(tr("Reorder Level"),  _qtyColumn,  Qt::AlignRight  );
  _exception->addColumn(tr("Proj. Avail."),   _qtyColumn,  Qt::AlignRight  );
}

dspReorderExceptionsByPlannerCode::~dspReorderExceptionsByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspReorderExceptionsByPlannerCode::languageChange()
{
  retranslateUi(this);
}

bool dspReorderExceptionsByPlannerCode::setParams(ParameterList &params)
{
  params.append("lookAheadDays", _days->value());

  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  if (_includePlanned->isChecked())
    params.append("includePlannedOrders");

  return true;
}

void dspReorderExceptionsByPlannerCode::sFillList()
{
  ParameterList params;
  setParams(params);

  // TODO: why is this select 3 layers deep?
  QString sql("SELECT itemsite_id, itemtype, warehous_code, item_number,"
              "       (item_descrip1 || ' ' || item_descrip2),"
              "       formatDate(reorderdate), formatQty(reorderlevel),"
              "       formatQty((itemsite_qtyonhand -"
	      "                  qtyAllocated(itemsite_id, reorderdate) +"
	      "                  qtyOrdered(itemsite_id, reorderdate))),"
              "       reorderdate "
              "FROM ( SELECT itemsite_id,"
              "              CASE WHEN (item_type IN ('M', 'B', 'T')) THEN 1"
              "                   WHEN (item_type IN ('P', 'O')) THEN 2"
              "                   ELSE 3"
              "              END AS itemtype,"
              "              warehous_code, item_number, item_descrip1,"
	      "              item_descrip2,"
              "              reorderDate(itemsite_id,"
	      "                          <? value(\"lookAheadDays\") ?>,"
	      "                          <? value(\"includePlannedOrders\") ?>)"
	      "                          AS reorderdate,"
	      "              itemsite_qtyonhand, reorderlevel "
              "       FROM ( SELECT itemsite_id, itemsite_item_id,"
	      "                     itemsite_warehous_id, itemsite_qtyonhand,"
              "                     CASE WHEN(itemsite_useparams) THEN"
	      "                          itemsite_reorderlevel"
	      "                     ELSE 0.0"
	      "                     END AS reorderlevel"
              "              FROM itemsite "
	      "              WHERE (true"
	      "<? if exists(\"warehous_id\") ?>"
	      "                AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	      "<? endif ?>"
	      "<? if exists(\"plancode_id\") ?>"
	      "                AND (itemsite_plancode_id=<? value(\"plancode_id\") ?>)"
	      "<? elseif exists(\"plancode_pattern\") ?>"
	      "                AND (itemsite_plancode_id IN (SELECT plancode_id"
	      "                      FROM plancode"
	      "                      WHERE (plancode_code ~ <? value(\"plancode_pattern\") ?>)))"
	      "<? endif ?>"
	      "                    ) "
	      "            ) AS itemsitedata, item, warehous "
	      " WHERE ( (itemsite_item_id=item_id)"
	      "  AND (itemsite_warehous_id=warehous_id) ) ) AS data "
	      "WHERE (reorderdate IS NOT NULL) "
	      "ORDER BY reorderdate, item_number;" );

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _exception->populate(q, TRUE);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspReorderExceptionsByPlannerCode::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("ReorderExceptionsByPlannerCode", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspReorderExceptionsByPlannerCode::sPopulateMenu( QMenu *pMenu )
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Running Availability..."), this, SLOT(sRunningAvailability()), 0);
  if (!_privileges->check("ViewInventoryAvailability"))
    pMenu->setItemEnabled(menuItem, FALSE);

  if (_exception->altId() == 1)
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Create Work Order..."), this, SLOT(sCreateWorkOrder()), 0);
    if (!_privileges->check("MaintainWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspReorderExceptionsByPlannerCode::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _exception->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspReorderExceptionsByPlannerCode::sCreateWorkOrder()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _exception->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}
