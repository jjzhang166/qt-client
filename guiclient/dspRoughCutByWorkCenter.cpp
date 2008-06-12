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

#include "dspRoughCutByWorkCenter.h"

#include <QMenu>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

dspRoughCutByWorkCenter::dspRoughCutByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  QButtonGroup* _workCenterGroupInt = new QButtonGroup(this);
  _workCenterGroupInt->addButton(_allWorkCenters);
  _workCenterGroupInt->addButton(_selectedWorkCenter);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sQuery()));
  connect(_roughCut, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _roughCut->addColumn(tr("Whs."),         _whsColumn,  Qt::AlignCenter );
  _roughCut->addColumn(tr("Work Center"),  -1,          Qt::AlignLeft   );
  _roughCut->addColumn(tr("Total Setup"),  _timeColumn, Qt::AlignRight  );
  _roughCut->addColumn(tr("Setup $"),      _costColumn, Qt::AlignRight  );
  _roughCut->addColumn(tr("Total Run"),    _timeColumn, Qt::AlignRight  );
  _roughCut->addColumn(tr("Run $"),        _costColumn, Qt::AlignRight  );

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   TRUE);
}

dspRoughCutByWorkCenter::~dspRoughCutByWorkCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspRoughCutByWorkCenter::languageChange()
{
  retranslateUi(this);
}

bool dspRoughCutByWorkCenter::setParams(ParameterList &params)
{
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_selectedWorkCenter->isChecked())
    params.append("wrkcnt_id", _wrkcnt->id());

  return true;
}

void dspRoughCutByWorkCenter::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("RoughCutCapacityPlanByWorkCenter", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspRoughCutByWorkCenter::sPopulateMenu(QMenu *, QTreeWidgetItem *)
{
}

void dspRoughCutByWorkCenter::sQuery()
{
  ParameterList params;
  setParams(params);

  QString sql( "SELECT wrkcnt_id, warehous_code, wrkcnt_code,"
               "       formatTime(SUM(planoper_sutime)),"
               "       formatCost(SUM(planoper_sutime) * wrkcnt_setuprate / 60.0),"
               "       formatTime(SUM(planoper_rntime)),"
               "       formatCost(SUM(planoper_rntime) * wrkcnt_runrate / 60.0) "
               "FROM planoper, planord, wrkcnt, warehous "
               "WHERE ( (planoper_planord_id=planord_id)"
               " AND (planoper_wrkcnt_id=wrkcnt_id)"
               " AND (wrkcnt_warehous_id=warehous_id)"
               " AND (planord_startdate BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?>)"
	       "<? if exists(\"wrkcnt_id\") ?>"
	       " AND (wrkcnt_id=<? value(\"wrkcnt_id\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"warehous_id\") ?>"
	       " AND (warehous_id=<? value(\"warehous_id\") ?>)"
	       "<? endif ?>"
	       ") "
	       "GROUP BY wrkcnt_id, warehous_code, wrkcnt_code,"
	       "         wrkcnt_setuprate, wrkcnt_runrate "
	       "ORDER BY warehous_code, wrkcnt_code;" );

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _roughCut->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
