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

#include "dspTimePhasedUsageStatisticsByItem.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <datecluster.h>
#include <parameter.h>

#include "dspInventoryHistoryByItem.h"
#include "guiclient.h"
#include "submitReport.h"

dspTimePhasedUsageStatisticsByItem::dspTimePhasedUsageStatisticsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_query, SIGNAL(clicked()), this, SLOT(sCalculate()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_usage, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));

  _usage->addColumn(tr("Transaction Type"), 120,        Qt::AlignLeft   );
  _usage->addColumn(tr("Whs."),             _whsColumn, Qt::AlignCenter );

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
}

dspTimePhasedUsageStatisticsByItem::~dspTimePhasedUsageStatisticsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspTimePhasedUsageStatisticsByItem::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedUsageStatisticsByItem::sPrint()
{
  if (_periods->isPeriodSelected())
  {
    orReport report("TimePhasedStatisticsByItem", buildParameters());
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

void dspTimePhasedUsageStatisticsByItem::sSubmit()
{
  if (_periods->isPeriodSelected())
  {
    ParameterList params(buildParameters());
    params.append("report_name", "TimePhasedStatisticsByItem");

    submitReport newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.check() == cNoReportDefinition)
      QMessageBox::critical( this, tr("Report Definition Not Found"),
                             tr( "The report defintions for this report, \"TimePhasedStatisticsByItem\" cannot be found.\n"
                                 "Please contact your Systems Administrator and report this issue." ) );
    else
      newdlg.exec();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

ParameterList dspTimePhasedUsageStatisticsByItem::buildParameters()
{
  ParameterList params;

  params.append("item_id", _item->id());
  _warehouse->appendValue(params);

  params.append("calendar_id", _calendar->id());

  QList<QVariant> periodList;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
    periodList.append(((XTreeWidgetItem*)selected[i])->id());
  params.append("period_id_list", periodList);

  return params;
}

void dspTimePhasedUsageStatisticsByItem::sViewTransactions()
{
  if (_column > 1)
  {
    ParameterList params;
    params.append("itemsite_id", _usage->id());
    params.append("startDate", _columnDates[_column - 2].startDate);
    params.append("endDate", _columnDates[_column - 2].endDate);
    params.append("run");

    QString type = _usage->currentItem()->text(0);
    if (type == "Received")
      params.append("transtype", "R");
    else if (type == "Issued")
      params.append("transtype", "I");
    else if (type == "Sold")
      params.append("transtype", "S");
    else if (type == "Scrap")
      params.append("transtype", "SC");
    else if (type == "Adjustments")
      params.append("transtype", "A");

    dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedUsageStatisticsByItem::sPopulateMenu(QMenu *menu, QTreeWidgetItem *, int pColumn)
{
  int menuItem;

  _column = pColumn;

  menuItem = menu->insertItem(tr("View Transactions..."), this, SLOT(sViewTransactions()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    menu->setItemEnabled(menuItem, FALSE);
}

void dspTimePhasedUsageStatisticsByItem::sCalculate()
{
  if (TRUE)
  {
    _columnDates.clear();
    _usage->clear();
    _usage->setColumnCount(2);

    QString sql("SELECT itemsite_id, warehous_code");

    int columns = 1;
    QList<QTreeWidgetItem*> selected = _periods->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
      sql += QString( ", formatQty(summTransR(itemsite_id, %1)) AS r_bucket%2,"
		      "formatQty(summTransI(itemsite_id, %3)) AS i_bucket%4,"
		      "formatQty(summTransS(itemsite_id, %5)) AS s_bucket%6,"
		      "formatQty(summTransC(itemsite_id, %7)) AS c_bucket%8," )
	     .arg(cursor->id())
	     .arg(columns)
	     .arg(cursor->id())
	     .arg(columns)
	     .arg(cursor->id())
	     .arg(columns)
	     .arg(cursor->id())
	     .arg(columns);

      sql += QString("formatQty(summTransA(itemsite_id, %1)) AS a_bucket%2")
	     .arg(cursor->id())
	     .arg(columns++);

      _usage->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight);

      _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
    }

    sql += QString( " FROM itemsite, warehous "
                    "WHERE ((itemsite_warehous_id=warehous_id)"
                    " AND (itemsite_item_id=%1)" )
           .arg(_item->id());

    if (_warehouse->isSelected())
	sql += QString(" AND (itemsite_warehous_id=%1)")
			    .arg(_warehouse->id());

    sql += ") "
           "ORDER BY warehous_code;";

    q.prepare(sql);
    q.exec();
    if (q.first())
    {
      do
      {
        XTreeWidgetItem *received;
        XTreeWidgetItem *issued;
        XTreeWidgetItem *sold;
        XTreeWidgetItem *scrap;
        XTreeWidgetItem *adjustments;

        received    = new XTreeWidgetItem(_usage,           q.value("itemsite_id").toInt(), QVariant(tr("Received")),    q.value("warehous_code") );
        issued      = new XTreeWidgetItem(_usage, received, q.value("itemsite_id").toInt(), QVariant(tr("Issued")),      q.value("warehous_code") );
        sold        = new XTreeWidgetItem(_usage, issued,   q.value("itemsite_id").toInt(), QVariant(tr("Sold")),        q.value("warehous_code") );
        scrap       = new XTreeWidgetItem(_usage, sold,     q.value("itemsite_id").toInt(), QVariant(tr("Scrap")),       q.value("warehous_code") );
        adjustments = new XTreeWidgetItem(_usage, scrap,    q.value("itemsite_id").toInt(), QVariant(tr("Adjustments")), q.value("warehous_code") );

        for (int bucketCounter = 1; bucketCounter < columns; bucketCounter++)
        {
          received->setText((bucketCounter + 1), q.value(QString("r_bucket%1").arg(bucketCounter)).toString());
          issued->setText((bucketCounter + 1), q.value(QString("i_bucket%1").arg(bucketCounter)).toString());
          sold->setText((bucketCounter + 1), q.value(QString("s_bucket%1").arg(bucketCounter)).toString());
          scrap->setText((bucketCounter + 1), q.value(QString("c_bucket%1").arg(bucketCounter)).toString());
          adjustments->setText((bucketCounter + 1), q.value(QString("a_bucket%1").arg(bucketCounter)).toString());
        }
      }
      while (q.next());
    }
  }
}
