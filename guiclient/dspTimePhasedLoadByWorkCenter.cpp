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

#include "dspTimePhasedLoadByWorkCenter.h"

#include <QMenu>
#include <QMessageBox>

#include <datecluster.h>
#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "dspWoOperationsByWorkCenter.h"

dspTimePhasedLoadByWorkCenter::dspTimePhasedLoadByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_load, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _load->addColumn(tr("Work Center"), _itemColumn, Qt::AlignLeft);
}

dspTimePhasedLoadByWorkCenter::~dspTimePhasedLoadByWorkCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspTimePhasedLoadByWorkCenter::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedLoadByWorkCenter::sPrint()
{
  if (_periods->isPeriodSelected())
  {
    ParameterList params;

    _warehouse->appendValue(params);

    QList<QTreeWidgetItem*> selected = _periods->selectedItems();
    QList<QVariant> periodList;
    for (int i = 0; i < selected.size(); i++)
      periodList.append(((XTreeWidgetItem*)selected[i])->id());
    params.append("period_id_list", periodList);

    orReport report("TimePhasedLoadByWorkCenter", params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      return;
    }
  }
  else
    QMessageBox::critical(this, tr("Incomplete criteria"),
                          tr("<p>The criteria you specified are not complete. "
			     "Please make sure all fields are correctly filled "
			     "out before running the report." ) );
}

void dspTimePhasedLoadByWorkCenter::sViewLoad()
{
  ParameterList params;
  params.append("wrkcnt_id", _load->id());
  params.append("startDate", _columnDates[_column - 1].startDate);
  params.append("endDate", _columnDates[_column - 1].endDate);
  params.append("run");

  dspWoOperationsByWorkCenter *newdlg = new dspWoOperationsByWorkCenter();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedLoadByWorkCenter::sPopulateMenu(QMenu *menu, QTreeWidgetItem *, int pColumn)
{
  int menuItem;

  _column = pColumn;
  if (_column > 0)
  {
    menuItem = menu->insertItem(tr("View Load..."), this, SLOT(sViewLoad()), 0);
    if ( (!_privileges->check("MaintainWoOperations")) && (!_privileges->check("ViewWoOperations")) )
      menu->setItemEnabled(menuItem, FALSE);
  }
}

void dspTimePhasedLoadByWorkCenter::sFillList()
{
  _columnDates.clear();
  _load->setColumnCount(1);

  QString sql("SELECT wrkcnt_id, wrkcnt_code ");

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    sql += QString(", formatTime(loadByWorkCenter(wrkcnt_id, %1)) AS bucket%2")
	   .arg(cursor->id())
	   .arg(columns);

    _load->addColumn(formatDate(cursor->startDate()), _timeColumn, Qt::AlignRight);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM wrkcnt ";

  if (_warehouse->isSelected())
    sql += "WHERE (wrkcnt_warehous_id=:warehous_id) ";

  sql += "ORDER BY wrkcnt_code;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.exec();
  _load->populate(q);
}
