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

#include "dspTimePhasedProductionByPlannerCode.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include <datecluster.h>
#include "dspInventoryHistoryByParameterList.h"
#include "guiclient.h"
#include "submitReport.h"

/*
 *  Constructs a dspTimePhasedProductionByPlannerCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedProductionByPlannerCode::dspTimePhasedProductionByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  QButtonGroup* _unitsGroupInt = new QButtonGroup(this);
  _unitsGroupInt->addButton(_inventoryUnits);
  _unitsGroupInt->addButton(_capacityUnits);
  _unitsGroupInt->addButton(_altCapacityUnits);

  // signals and slots connections
  connect(_query, SIGNAL(clicked()), this, SLOT(sCalculate()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_production, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
  connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));

  statusBar()->hide();

  _plannerCode->setType(PlannerCode);

  _production->addColumn(tr("Planner Code"), _itemColumn, Qt::AlignLeft   );
  _production->addColumn(tr("Whs."),         _whsColumn,  Qt::AlignCenter );
  _production->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignLeft   );

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspTimePhasedProductionByPlannerCode::~dspTimePhasedProductionByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedProductionByPlannerCode::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedProductionByPlannerCode::sPrint()
{
  if (_periods->isPeriodSelected())
  {
    orReport report("TimePhasedProductionByPlannerCode", buildParameters());
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      return;
    }
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

void dspTimePhasedProductionByPlannerCode::sSubmit()
{
  if (_periods->isPeriodSelected())
  {
    ParameterList params(buildParameters());
    params.append("report_name", "TimePhasedProductionByPlannerCode");

    submitReport newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.check() == cNoReportDefinition)
      QMessageBox::critical( this, tr("Report Definition Not Found"),
                             tr( "The report defintions for this report, \"TimePhasedProductionByPlannerCode\" cannot be found.\n"
                                 "Please contact your Systems Administrator and report this issue." ) );
    else
      newdlg.exec();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

ParameterList dspTimePhasedProductionByPlannerCode::buildParameters()
{
  ParameterList params;

  _plannerCode->appendValue(params);
  _warehouse->appendValue(params);

  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  QList<QVariant> periodList;
  for (int i = 0; i < selected.size(); i++)
    periodList.append(((XTreeWidgetItem*)selected[i])->id());
  params.append("period_id_list", periodList);

  if (_capacityUnits->isChecked())
    params.append("capacityUnits");
  else if(_altCapacityUnits->isChecked())
    params.append("altCapacityUnits");
  else if(_inventoryUnits->isChecked())
    params.append("inventoryUnits");

  if(_showInactive->isChecked())
    params.append("showInactive");

  return params;
}

void dspTimePhasedProductionByPlannerCode::sViewTransactions()
{
  ParameterList params;
  params.append("plancode_id", _production->id());
  params.append("warehous_id", _production->altId());
  params.append("startDate", _columnDates[_column - 3].startDate);
  params.append("endDate", _columnDates[_column - 3].endDate);
  params.append("transtype", "R");
  params.append("run");

  dspInventoryHistoryByParameterList *newdlg = new dspInventoryHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedProductionByPlannerCode::sPopulateMenu(QMenu *menu, QTreeWidgetItem *, int pColumn)
{
  int menuItem;

  _column = pColumn;
  if (_column > 2)
  {
    menuItem = menu->insertItem(tr("View Transactions..."), this, SLOT(sViewTransactions()), 0);
    if (!_privileges->check("ViewInventoryHistory"))
      menu->setItemEnabled(menuItem, FALSE);
  }
}

void dspTimePhasedProductionByPlannerCode::sCalculate()
{
  _columnDates.clear();
  _production->setColumnCount(3);

  QString sql("SELECT plancode_id, warehous_id, plancode_code, warehous_code, ");

  if (_inventoryUnits->isChecked())
    sql += "uom_name AS uom";

  else if (_capacityUnits->isChecked())
    sql += "itemcapuom(item_id) AS uom";

  else if (_altCapacityUnits->isChecked())
    sql += "itemaltcapuom(item_id) AS uom";

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    if (_inventoryUnits->isChecked())
      sql += QString(", formatQty(SUM(summProd(itemsite_id, %1))) AS bucket%2")
	     .arg(cursor->id())
	     .arg(columns++);

    else if (_capacityUnits->isChecked())
      sql += QString(", formatQty(SUM(summProd(itemsite_id, %1) * itemcapinvrat(item_id))) AS bucket%2")
	     .arg(cursor->id())
	     .arg(columns++);

    else if (_altCapacityUnits->isChecked())
      sql += QString(", formatQty(SUM(summProd(itemsite_id, %1) * itemaltcapinvrat(item_id))) AS bucket%2")
	     .arg(cursor->id())
	     .arg(columns++);

    _production->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM itemsite, item, uom, warehous, plancode "
         "WHERE ((itemsite_item_id=item_id)"
         " AND (item_inv_uom_id=uom_id)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (itemsite_plancode_id=plancode_id)";

  if (!_showInactive->isChecked())
    sql += " AND (itemsite_active)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";
 
  if (_plannerCode->isSelected())
    sql += " AND (plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (plancode_code ~ :plancode_pattern)";

  sql += " ) "
         "GROUP BY plancode_id, warehous_id, plancode_code, warehous_code, uom;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();
  _production->populate(q, TRUE);
}

