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

#include "plannedSchedules.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QMenu>

#include <openreports.h>

#include "plannedSchedule.h"
#include "copyPlannedSchedule.h"

plannedSchedules::plannedSchedules(QWidget * parent, const char * name, Qt::WFlags fl)
  : XMainWindow(parent, name, fl)
{
  setupUi(this);

  statusBar()->hide();

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_list, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _list->addColumn(tr("Start Date"),   _dateColumn, Qt::AlignCenter );
  _list->addColumn(tr("End Date"),     _dateColumn, Qt::AlignCenter );
  _list->addColumn(tr("Whs."),         _whsColumn,  Qt::AlignCenter );
  _list->addColumn(tr("Schd. Number"), _itemColumn, Qt::AlignLeft   );
  _list->addColumn(tr("Status"),        _whsColumn,  Qt::AlignCenter );
  _list->addColumn(tr("Description"),  -1,          Qt::AlignLeft   );

  sFillList();
}

plannedSchedules::~plannedSchedules()
{
}

void plannedSchedules::languageChange()
{
  retranslateUi(this);
}

void plannedSchedules::sPopulateMenu(QMenu* pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Release"), this, SLOT(sRelease()), 0);
}

void plannedSchedules::sRelease()
{
  q.prepare("UPDATE pschhead SET pschhead_status = 'R' WHERE (pschhead_id=:pschhead_id);");
  q.bindValue(":pschhead_id", _list->id());
  q.exec();
  sFillList();
}

void plannedSchedules::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  plannedSchedule newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedules::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pschhead_id", _list->id());

  plannedSchedule newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedules::sCopy()
{
  ParameterList params;
  params.append("pschhead_id", _list->id());

  copyPlannedSchedule newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedules::sPrint()
{
  orReport report("PlannedSchedulesMasterList");
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void plannedSchedules::sFillList()
{
  q.prepare("SELECT pschhead_id,"
            "       formatDate(pschhead_start_date),"
            "       formatDate(pschhead_end_date),"
            "       warehous_code,"
            "       pschhead_number,"
            "       pschhead_status,"
            "       pschhead_descrip "
            "  FROM pschhead JOIN warehous ON (pschhead_warehous_id=warehous_id) "
            " ORDER BY pschhead_start_date, pschhead_end_date, warehous_code, pschhead_number; ");
  q.exec();
  _list->populate(q);
}

