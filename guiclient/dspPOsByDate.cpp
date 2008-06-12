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

#include "dspPOsByDate.h"

#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>
#include <metasql.h>

#include "purchaseOrder.h"

dspPOsByDate::dspPOsByDate(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_poitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Starting Due Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Ending Due Date:"));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  _poitem->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight, true, "pohead_number");
  _poitem->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter,true, "warehousecode");
  _poitem->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter,true, "poitemstatus");
  _poitem->addColumn(tr("Vendor"),      _itemColumn,  Qt::AlignLeft,  true, "vend_name");
  _poitem->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter,true, "minDueDate");
}

dspPOsByDate::~dspPOsByDate()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPOsByDate::languageChange()
{
  retranslateUi(this);
}

bool dspPOsByDate::setParams(ParameterList &pParams)
{
  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr( "Please enter a valid Start Date." ) );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr( "Please eneter a valid End Date." ) );
    _dates->setFocus();
    return false;
  }

  _dates->appendValue(pParams);
  _warehouse->appendValue(pParams);

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  if (_showClosed->isChecked())
    pParams.append("showClosed");

  pParams.append("closed",      tr("Closed"));
  pParams.append("unposted",    tr("Unposted"));
  pParams.append("partial",     tr("Partial"));
  pParams.append("received",    tr("Received"));
  pParams.append("open",        tr("Open"));

  return true;
}

void dspPOsByDate::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("POsByDate", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPOsByDate::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (pSelected->text(2) == "U")
  {
    menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);
  if ((!_privileges->check("MaintainPurchaseOrders")) && (!_privileges->check("ViewPurchaseOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspPOsByDate::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByDate::sViewOrder()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByDate::sFillList()
{
  QString sql( "SELECT pohead_id, pohead_number,"
               "       warehous_code AS warehousecode,"
               "       CASE WHEN(poitem_status='C') THEN <? value(\"closed\") ?>"
               "            WHEN(poitem_status='U') THEN <? value(\"unposted\") ?>"
               "            WHEN(poitem_status='O'"
               "                 AND (SUM(poitem_qty_received-poitem_qty_returned) > 0)"
               "                 AND (SUM(poitem_qty_ordered)>SUM(poitem_qty_received-poitem_qty_returned))) THEN <? value(\"partial\") ?>"
               "            WHEN(poitem_status='O'"
               "                 AND (SUM(poitem_qty_received-poitem_qty_returned) > 0)"
               "                 AND (SUM(poitem_qty_ordered)=SUM(poitem_qty_received-poitem_qty_returned))) THEN <? value(\"received\") ?>"
               "            WHEN(poitem_status='O') THEN <? value(\"open\") ?>"
               "            ELSE poitem_status"
               "       END AS poitemstatus,"
               "       vend_name,"
               "       MIN(poitem_duedate) AS minDueDate,"
               "       CASE WHEN (MIN(poitem_duedate) < CURRENT_DATE) THEN 'error'"
               "       END AS minDueDate_qtforegroundrole "
               "  FROM vend, poitem, pohead "
               "       LEFT OUTER JOIN warehous ON (pohead_warehous_id=warehous_id)"
               " WHERE ((poitem_pohead_id=pohead_id)"
               "   AND  (pohead_vend_id=vend_id)"
               "   AND  (poitem_duedate BETWEEN <? value(\"startDate\") ?>"
               "                            AND <? value(\"endDate\") ?>)"
               "<? if exists(\"warehous_id\") ?>"
               "   AND (pohead_warehous_id=<? value(\"warehous_id\") ?>) "
               "<? endif ?>"
               "<? if not exists(\"showClosed\") ?>"
               "   AND (poitem_status!='C')"
               "<? endif ?>"
               "<? if exists(\"agentUsername\") ?>"
               "   AND (pohead_agent_username=<? value(\"agentUsername\") ?>)"
               "<? endif ?>"
               ") "
               "GROUP BY pohead_id, pohead_number, warehous_code,"
               "         poitem_status, vend_name "
               "ORDER BY minDueDate;");

  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _poitem->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
