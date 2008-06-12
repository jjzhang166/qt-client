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

#include "dspPurchaseReqsByPlannerCode.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include "dspRunningAvailability.h"
#include "purchaseOrder.h"

/*
 *  Constructs a dspPurchaseReqsByPlannerCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPurchaseReqsByPlannerCode::dspPurchaseReqsByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_pr, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  _plannerCode->setType(PlannerCode);

  _pr->addColumn(tr("Item Number"),  _itemColumn,   Qt::AlignLeft   );
  _pr->addColumn(tr("Description"),  -1,            Qt::AlignLeft   );
  _pr->addColumn(tr("Status"),       _statusColumn, Qt::AlignCenter );
  _pr->addColumn(tr("Parent Order"), _itemColumn,   Qt::AlignLeft   );
  _pr->addColumn(tr("Due Date"),     _dateColumn,   Qt::AlignCenter );
  _pr->addColumn(tr("Qty."),         _qtyColumn,    Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPurchaseReqsByPlannerCode::~dspPurchaseReqsByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPurchaseReqsByPlannerCode::languageChange()
{
  retranslateUi(this);
}

void dspPurchaseReqsByPlannerCode::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _plannerCode->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

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
  _pr->clear();

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  QString sql( "SELECT pr_id, itemsite_id,"
               "       item_number, (item_descrip1 || ' ' || item_descrip2), pr_status,"
               "       CASE WHEN (pr_order_type='W') THEN ('W/O ' || ( SELECT formatWoNumber(womatl_wo_id)"
               "                                                       FROM womatl"
               "                                                       WHERE (womatl_id=pr_order_id) ) )"
               "            WHEN (pr_order_type='S') THEN ('S/O ' || (SELECT formatSoNumber(pr_order_id)))"
               "            WHEN (pr_order_type='F') THEN ('Planned Order')"
               "            WHEN (pr_order_type='M') THEN :manual"
               "            ELSE :other"
               "       END,"
               " formatDate(pr_duedate), formatQty(pr_qtyreq) "
               "FROM pr, itemsite, item "
               "WHERE ( (pr_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (pr_duedate BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern) ) )";

  sql += ") "
         "ORDER BY pr_duedate;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":manual", tr("Manual"));
  q.bindValue(":other", tr("Other"));
  q.exec();
  _pr->populate(q, TRUE);
}

