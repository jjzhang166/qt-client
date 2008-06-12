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

#include "dspExpediteExceptionsByPlannerCode.h"

#include <QVariant>
#include <QStatusBar>
#include <QMenu>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a dspExpediteExceptionsByPlannerCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspExpediteExceptionsByPlannerCode::dspExpediteExceptionsByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_exception, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _plannerCode->setType(PlannerCode);

  _exception->addColumn(tr("Order Type/#"),   _itemColumn, Qt::AlignCenter );
  _exception->addColumn(tr("Whs."),           _whsColumn,  Qt::AlignCenter );
  _exception->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft   );
  _exception->addColumn(tr("Descriptions"),   -1,          Qt::AlignLeft   );
  _exception->addColumn(tr("Start/Due Date"), _itemColumn, Qt::AlignLeft   );
  _exception->addColumn(tr("Exception"),      120,         Qt::AlignLeft   );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspExpediteExceptionsByPlannerCode::~dspExpediteExceptionsByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspExpediteExceptionsByPlannerCode::languageChange()
{
  retranslateUi(this);
}

void dspExpediteExceptionsByPlannerCode::sFillList()
{
  _exception->clear();

  QString sql( "SELECT planord_id AS order_id, 1 AS order_code,"
               "       CASE WHEN (planord_type='W') THEN ('PW/O-' || formatPloNumber(planord_id))"
               "            WHEN (planord_type='P') THEN ('PP/O-' || formatPloNumber(planord_id))"
               "            ELSE TEXT(planord_number)"
               "       END AS order_number,"
               "       warehous_code, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
               "       CASE WHEN (planord_type='W') THEN formatDate(planord_startdate)"
               "            ELSE formatDate(planord_duedate)"
               "       END AS f_keydate,"
               "       CASE WHEN (planord_type='W') THEN planord_startdate"
               "            ELSE planord_duedate"
               "       END AS keydate, :releaseOrder AS exception "
               "FROM planord, itemsite, item, warehous "
               "WHERE ( (planord_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND ( ( (planord_type='W') AND (planord_startdate <= (CURRENT_DATE + :days)) )"
               "    OR ( (planord_type='P') AND (planord_duedate <= (CURRENT_DATE + :days)) ) ) " );

  if (_warehouse->isSelected())
    sql += "AND (itemsite_warehous_id=:warehous_id)";

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  sql += " ) ";

  sql += "UNION SELECT wo_id AS order_id, 2 AS order_code, ('W/O-' || formatWoNumber(wo_id)) AS order_number,"
         "             warehous_code, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
         "             formatDate(wo_startdate) AS f_keydate, wo_startdate AS keydate, :startProduction AS exception "
         "FROM wo, itemsite, item, warehous "
         "WHERE ( (wo_itemsite_id=itemsite_id)"
         " AND (itemsite_item_id=item_id)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (wo_status IN ('O', 'E', 'R'))"
         " AND (wo_startdate <= (CURRENT_DATE + :days))";

  if (_warehouse->isSelected())
    sql += "AND (itemsite_warehous_id=:warehous_id)";

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  sql += " ) ";

  sql += "UNION SELECT wo_id AS order_id, 2 AS order_code, ('W/O-' || formatWoNumber(wo_id)) AS order_number,"
         "             warehous_code, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
         "             formatDate(wo_startdate) AS f_keydate, wo_startdate AS keydate, :expediteProduction AS exception "
         "FROM wo, itemsite, item, warehous "
         "WHERE ( (wo_itemsite_id=itemsite_id)"
         " AND (itemsite_item_id=item_id)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (wo_status='I')"
         " AND (wo_startdate <= (CURRENT_DATE + :days))";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  sql += " ) "
         "ORDER BY keydate;";

  q.prepare(sql);
  q.bindValue(":releaseOrder", tr("Release Order"));
  q.bindValue(":startProduction", tr("Start Production"));
  q.bindValue(":expediteProduction", tr("Expedite Production"));
  q.bindValue(":days", _days->value());
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();
  if (q.first())
  {
    XTreeWidgetItem *last = NULL;

    do
      last = new XTreeWidgetItem( _exception, last, q.value("order_id").toInt(), q.value("order_code").toInt(),
                                q.value("order_number"), q.value("warehous_code"),
                                q.value("item_number"), q.value("item_descrip"),
                                q.value("f_keydate"), q.value("exception") );
    while (q.next());
  }
}

void dspExpediteExceptionsByPlannerCode::sPrint()
{
  ParameterList params;

  _plannerCode->appendValue(params);
  _warehouse->appendValue(params);

  params.append("lookAheadDays", _days->value());

  orReport report("ExpediteExceptionsByPlannerCode", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspExpediteExceptionsByPlannerCode::sPopulateMenu( QMenu *, QTreeWidgetItem * )
{

}
