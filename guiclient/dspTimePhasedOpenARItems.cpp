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

#include "dspTimePhasedOpenARItems.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QVector>
#include <QWorkspace>
#include <QMenu>
#include <datecluster.h>
#include <openreports.h>
#include "printStatementByCustomer.h"
#include "dspAROpenItemsByCustomer.h"
#include "submitReport.h"

/*
 *  Constructs a dspTimePhasedOpenARItems as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedOpenARItems::dspTimePhasedOpenARItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_selectedCustomerType, SIGNAL(toggled(bool)), _customerTypes, SLOT(setEnabled(bool)));
  connect(_customerTypePattern, SIGNAL(toggled(bool)), _customerType, SLOT(setEnabled(bool)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_aropen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
  connect(_selectedCustomer, SIGNAL(toggled(bool)), _cust, SLOT(setEnabled(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));
  connect(_custom, SIGNAL(toggled(bool)), this, SLOT(sToggleCustom()));
  
  _customerTypes->setType(XComboBox::CustomerTypes);
  
  _aropen->addColumn(tr("Cust. #"),  _orderColumn, Qt::AlignLeft );
  _aropen->addColumn(tr("Customer"), 180,          Qt::AlignLeft );

  _allCustomers->setFocus();
  
  _asOf->setDate(omfgThis->dbDate(), true);
  sToggleCustom();

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspTimePhasedOpenARItems::~dspTimePhasedOpenARItems()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedOpenARItems::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedOpenARItems::sPrint()
{
  if ((_custom->isChecked() && _periods->isPeriodSelected()) || (!_custom->isChecked()))
  {
    QString reportName;
    if(_custom->isChecked())
      reportName = "TimePhasedOpenARItems";
    else
      reportName = "ARAging";
    orReport report(reportName, buildParameters());
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

void dspTimePhasedOpenARItems::sSubmit()
{
  if ((_custom->isChecked() && _periods->isPeriodSelected()) || (!_custom->isChecked()))
  {
    ParameterList params(buildParameters());
    if(_custom->isChecked())
      params.append("report_name", "TimePhasedOpenARItems");
    else
      params.append("report_name", "ARAging");

    submitReport newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.check() == cNoReportDefinition)
      QMessageBox::critical( this, tr("Report Definition Not Found"),
                             tr( "The report defintions for this report, \"TimePhasedOpenARItems\" cannot be found.\n"
                                 "Please contact your Systems Administrator and report this issue." ) );
    else
      newdlg.exec();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

ParameterList dspTimePhasedOpenARItems::buildParameters()
{
  ParameterList params;

  if (_selectedCustomer->isChecked())
    params.append("cust_id", _cust->id());
  else if (_selectedCustomerType->isChecked())
    params.append("custtype_id", _customerTypes->id());
  else if (_customerTypePattern->isChecked())
    params.append("custtype_pattern", _customerType->text());

  if(_custom->isChecked())
  {
    QList<QTreeWidgetItem*> selected = _periods->selectedItems();
    QList<QVariant> periodList;
    for (int i = 0; i < selected.size(); i++)
      periodList.append(((XTreeWidgetItem*)selected[i])->id());
    params.append("period_id_list", periodList);
  }
  else
    params.append("relDate", _asOf->date());

  return params;
}

void dspTimePhasedOpenARItems::sViewOpenItems()
{
  ParameterList params;
  params.append("cust_id", _aropen->id());
  if (_custom->isChecked())
  {
    params.append("startDate", _periods->getSelected(_column - 1)->startDate());
    params.append("endDate", _periods->getSelected(_column - 1)->endDate());
  }
  else
  {
    QDate asOfDate;
    asOfDate = _asOf->date();
    if (_column == 3)
      params.append("startDate", asOfDate );
    else if (_column == 4)
    {
      params.append("startDate", asOfDate.addDays(-30) );
      params.append("endDate", asOfDate);
    }
    else if (_column == 5)
    {
      params.append("startDate",asOfDate.addDays(-60) );
      params.append("endDate", asOfDate.addDays(-31));
    }
    else if (_column == 6)
    {
      params.append("startDate",asOfDate.addDays(-90) );
      params.append("endDate", asOfDate.addDays(-61));
    }
    else if (_column == 7)
      params.append("endDate",asOfDate.addDays(-91) );
  }
  params.append("run");
  params.append("asofDate", _asOf->date());

  dspAROpenItemsByCustomer *newdlg = new dspAROpenItemsByCustomer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedOpenARItems::sPrintStatement()
{
  ParameterList params;
  params.append("cust_id", _aropen->id());
  params.append("print");

  printStatementByCustomer newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspTimePhasedOpenARItems::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int pColumn)
{
  int menuItem;
  _column = pColumn;
  
  if ((_column > 1) && (_aropen->id() > 0))
  {
    menuItem = pMenu->insertItem(tr("View Open Items..."), this, SLOT(sViewOpenItems()), 0);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Print Statement..."), this, SLOT(sPrintStatement()), 0);
  }
}

void dspTimePhasedOpenARItems::sFillList()
{
  if (_custom->isChecked())
    sFillCustom();
  else
    sFillStd();
}

void dspTimePhasedOpenARItems::sFillCustom()
{
  if (!_periods->isPeriodSelected())
  {
    if (isVisible())
      QMessageBox::warning( this, tr("Select Calendar Periods"),
                            tr("Please select one or more Calendar Periods") );
    return;
  }

  _columnDates.clear();
  _aropen->clear();
  _aropen->setColumnCount(2);

  QString sql("SELECT cust_id, cust_number, cust_name");

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    sql += QString(", openARItemsValue(cust_id, %2) AS bucket%1")
	   .arg(columns++)
	   .arg(cursor->id());

    _aropen->addColumn(formatDate(cursor->startDate()), _bigMoneyColumn, Qt::AlignRight);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM cust ";

  if (_selectedCustomer->isChecked())
    sql += "WHERE (cust_id=:cust_id)";
  else if (_selectedCustomerType->isChecked())
    sql += "WHERE (cust_custtype_id=:custtype_id)";
  else if (_customerTypePattern->isChecked())
    sql += "WHERE (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ :custtype_pattern))) ";

  sql += "ORDER BY cust_number;";

  q.prepare(sql);
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":custtype_id", _customerTypes->id());
  q.bindValue(":custtype_pattern", _customerType->text().upper());
  q.exec();
  if (q.first())
  {
    QVector<Numeric> totals(columns);

    XTreeWidgetItem * last = 0;
    do
    {
      double lineTotal = 0.0;

      XTreeWidgetItem *item = new XTreeWidgetItem( _aropen, last, q.value("cust_id").toInt(),
                                                   q.value("cust_number"), q.value("cust_name") );

      for (int column = 1; column < columns; column++)
      {
        QString bucketName = QString("bucket%1").arg(column);
        item->setText((column + 1), formatMoney(q.value(bucketName).toDouble()));
        totals[column] += q.value(bucketName).toDouble();
        lineTotal += q.value(bucketName).toDouble();
      }

      if (lineTotal == 0.0)
        delete item;
      else
        last = item;
    }
    while (q.next());

//  Add the totals row
    XTreeWidgetItem *total = new XTreeWidgetItem(_aropen, last, -1, QVariant(tr("Totals:")));
    for (int column = 1; column < columns; column++)
      total->setText((column + 1), formatMoney(totals[column].toDouble()));
  }
}

void dspTimePhasedOpenARItems::sFillStd()
{
  _aropen->clear();

  QString sql("SELECT araging_cust_id, araging_cust_number, araging_cust_name,formatMoney(SUM(araging_total_val)),"
              "       formatMoney(SUM(araging_cur_val)),formatMoney(SUM(araging_thirty_val)),"
              "       formatMoney(SUM(araging_sixty_val)),formatMoney(SUM(araging_ninety_val)),"
              "       formatMoney(SUM(araging_plus_val)),0 AS sequence"
              "  FROM araging(:asofDate) ");

  if (_selectedCustomer->isChecked())
    sql += "WHERE (araging_cust_id=:cust_id)";
  else if (_selectedCustomerType->isChecked())
    sql += "WHERE (araging_cust_custtype_id=:custtype_id)";
  else if (_customerTypePattern->isChecked())
    sql += "WHERE (araging_custtype_code ~ :custtype_pattern) ";

  sql += "GROUP BY araging_cust_number,araging_cust_id,araging_cust_name ";
  
  //Get totals
  sql += " UNION SELECT 0, 'Totals:', '',formatMoney(SUM(araging_total_val)),"
         "              formatMoney(SUM(araging_cur_val)),formatMoney(SUM(araging_thirty_val)),"
         "              formatMoney(SUM(araging_sixty_val)),formatMoney(SUM(araging_ninety_val)),"
         "              formatMoney(SUM(araging_plus_val)),1 AS sequence"
         "         FROM araging(:asofDate) ";

  if (_selectedCustomer->isChecked())
    sql += "WHERE (araging_cust_id=:cust_id)";
  else if (_selectedCustomerType->isChecked())
    sql += "WHERE (araging_cust_custtype_id=:custtype_id)";
  else if (_customerTypePattern->isChecked())
    sql += "WHERE (araging_custtype_code ~ :custtype_pattern) ";

  sql += "ORDER BY sequence,araging_cust_number";
  sql += ";";

  q.prepare(sql);
  q.bindValue(":asofDate",_asOf->date());
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":custtype_id", _customerTypes->id());
  q.bindValue(":custtype_pattern", _customerType->text().upper());
  q.exec();
  if (q.first())
    _aropen->populate(q);
}

void dspTimePhasedOpenARItems::sToggleCustom()
{
  if (_custom->isChecked())
  {
    _calendarLit->setHidden(FALSE);
    _calendar->setHidden(FALSE);
    _periods->setHidden(FALSE);
    _asOf->setDate(omfgThis->dbDate(), true);
    _asOf->setEnabled(FALSE);
  }
  else
  {
    _calendarLit->setHidden(TRUE);
    _calendar->setHidden(TRUE);
    _periods->setHidden(TRUE);
    _asOf->setEnabled(TRUE);
    _aropen->setColumnCount(2);
    _aropen->addColumn(tr("Total Open"), _bigMoneyColumn, Qt::AlignRight);
    _aropen->addColumn(tr("0+ Days"), _bigMoneyColumn, Qt::AlignRight);
    _aropen->addColumn(tr("0-30 Days"), _bigMoneyColumn, Qt::AlignRight);
    _aropen->addColumn(tr("31-60 Days"), _bigMoneyColumn, Qt::AlignRight);
    _aropen->addColumn(tr("61-90 Days"), _bigMoneyColumn, Qt::AlignRight);
    _aropen->addColumn(tr("90+ Days"), _bigMoneyColumn, Qt::AlignRight);
  }
}

