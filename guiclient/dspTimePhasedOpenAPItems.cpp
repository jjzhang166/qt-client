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

#include "dspTimePhasedOpenAPItems.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QMenu>
#include <datecluster.h>
#include <QWorkspace>
#include <q3valuevector.h>
#include <openreports.h>
#include "dspAPOpenItemsByVendor.h"
#include "guiclient.h"
#include "submitReport.h"

/*
 *  Constructs a dspTimePhasedOpenAPItems as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedOpenAPItems::dspTimePhasedOpenAPItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_vendorTypePattern, SIGNAL(toggled(bool)), _vendorType, SLOT(setEnabled(bool)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_apopen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectedVendor, SIGNAL(toggled(bool)), _vend, SLOT(setEnabled(bool)));
  connect(_selectedVendorType, SIGNAL(toggled(bool)), _vendorTypes, SLOT(setEnabled(bool)));
  connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));
  connect(_custom, SIGNAL(toggled(bool)), this, SLOT(sToggleCustom()));
  
  _vendorTypes->setType(XComboBox::VendorTypes);
  
  _apopen->addColumn(tr("Vend. #"), _orderColumn, Qt::AlignLeft );
  _apopen->addColumn(tr("Vendor"),  180,          Qt::AlignLeft );
    
  _asOf->setDate(omfgThis->dbDate(), true);
  sToggleCustom();

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();

  if(_preferences->value("APAgingDefaultDate") == "doc")
    _useDocDate->setChecked(true);
  else
    _useDistDate->setChecked(true);
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspTimePhasedOpenAPItems::~dspTimePhasedOpenAPItems()
{
  // no need to delete child widgets, Qt does it all for us
  QString str("dist");
  if(_useDocDate->isChecked())
    str = "doc";
  _preferences->set("APAgingDefaultDate", str);
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedOpenAPItems::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedOpenAPItems::sPrint()
{
  if ((_custom->isChecked() && _periods->isPeriodSelected()) || (!_custom->isChecked() && _asOf->isValid()))
  {
    QString reportName;
    if(_custom->isChecked())
      reportName = "TimePhasedOpenAPItems";
    else
      reportName = "APAging";
    
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

void dspTimePhasedOpenAPItems::sSubmit()
{
  if ((_custom->isChecked() && _periods->isPeriodSelected()) || (!_custom->isChecked() && _asOf->isValid()))
  {
    ParameterList params(buildParameters());
    if(_custom->isChecked())
      params.append("report_name", "TimePhasedOpenAPItems");
    else
      params.append("report_name", "APAging");

    if(_useDocDate->isChecked())
      params.append("useDocDate");
    else
      params.append("useDistDate");

    submitReport newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.check() == cNoReportDefinition)
      QMessageBox::critical( this, tr("Report Definition Not Found"),
                             tr( "The report defintions for this report, \"TimePhasedOpenAPItems\" cannot be found.\n"
                                 "Please contact your Systems Administrator and report this issue." ) );
    else
      newdlg.exec();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

ParameterList dspTimePhasedOpenAPItems::buildParameters()
{
  ParameterList params;

  if (_selectedVendor->isChecked())
    params.append("vend_id", _vend->id());
  else if (_selectedVendorType->isChecked())
    params.append("vendtype_id", _vendorTypes->id());
  else if (_vendorTypePattern->isChecked())
    params.append("vendtype_pattern", _vendorType->text());

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

void dspTimePhasedOpenAPItems::sViewOpenItems()
{
  ParameterList params;
  params.append("vend_id", _apopen->id());
  if (_custom->isChecked())
  {
    params.append("startDate", _columnDates[_column - 2].startDate);
    params.append("endDate", _columnDates[_column - 2].endDate);
  }
  else
  {
    QDate asOfDate;
    asOfDate = _asOf->date();
    if (_column == 3)
    {
      params.append("startDate", asOfDate );
    }
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
  params.append("asofDate",_asOf->date());

  dspAPOpenItemsByVendor *newdlg = new dspAPOpenItemsByVendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedOpenAPItems::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  int intMenuItem;
  _column = pColumn;

  if ((_column > 1) && (_apopen->id() > 0))
    intMenuItem = menuThis->insertItem(tr("View Open Items..."), this, SLOT(sViewOpenItems()), 0);
}

void dspTimePhasedOpenAPItems::sFillList()
{
  if (_custom->isChecked())
    sFillCustom();
  else
    sFillStd();
}

void dspTimePhasedOpenAPItems::sFillCustom()
{
  if (!_periods->isPeriodSelected())
  {
    if (isVisible())
      QMessageBox::warning( this, tr("Select Calendar Periods"),
                            tr("Please select one or more Calendar Periods") );
    return;
  }

  _columnDates.clear();
  _apopen->clear();
  _apopen->setColumnCount(2);

  QString sql("SELECT vend_id, vend_number, vend_name");

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    sql += QString(", openAPItemsValue(vend_id, %2) AS bucket%1")
          .arg(columns++)
          .arg(cursor->id());

    _apopen->addColumn(formatDate(cursor->startDate()), _bigMoneyColumn, Qt::AlignRight);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM vend ";

  if (_selectedVendor->isChecked())
    sql += "WHERE (vend_id=:vend_id)";
  else if (_selectedVendorType->isChecked())
    sql += "WHERE (vend_vendtype_id=:vendtype_id)";
  else if (_vendorTypePattern->isChecked())
    sql += "WHERE (vend_vendtype_id IN (SELECT vendtype_id FROM vendtype WHERE (vendtype_code ~ :vendtype_code))) ";

  sql += "ORDER BY vend_number;";

  q.prepare(sql);
  q.bindValue(":vend_id", _vend->id());
  q.bindValue(":vendtype_id", _vendorTypes->id());
  q.bindValue(":vendtype_code", _vendorType->text().upper());
  q.exec();
  if (q.first())
  {
    Q3ValueVector<Numeric> totals(columns);;
    XTreeWidgetItem *last = 0;

    do
    {
      double lineTotal = 0.0;

      last = new XTreeWidgetItem( _apopen, last, q.value("vend_id").toInt(),
                                 q.value("vend_number"), q.value("vend_name") );

      for (int column = 1; column < columns; column++)
      {
        QString bucketName = QString("bucket%1").arg(column);
        last->setText((column + 1), formatMoney(q.value(bucketName).toDouble()));
        totals[column] += q.value(bucketName).toDouble();
        lineTotal += q.value(bucketName).toDouble();
      }

      if (lineTotal == 0.0)
      {
        delete last;
        last = _apopen->topLevelItem(_apopen->topLevelItemCount() - 1);
      }
    }
    while (q.next());

//  Add the totals row
    XTreeWidgetItem *total = new XTreeWidgetItem(_apopen, last, -1, QVariant(tr("Totals:")));
    for (int column = 1; column < columns; column++)
      total->setText((column + 1), formatMoney(totals[column].toDouble()));
  }
}

void dspTimePhasedOpenAPItems::sFillStd()
{
  _apopen->clear();

  QString sql("SELECT apaging_vend_id, apaging_vend_number, apaging_vend_name,formatMoney(SUM(apaging_total_val)),"
              "       formatMoney(SUM(apaging_cur_val)),formatMoney(SUM(apaging_thirty_val)),"
              "       formatMoney(SUM(apaging_sixty_val)),formatMoney(SUM(apaging_ninety_val)),"
              "       formatMoney(SUM(apaging_plus_val)),0 AS sequence"
              "  FROM apaging(:asofDate, :useDocDate) ");

  if (_selectedVendor->isChecked())
    sql += "WHERE (apaging_vend_id=:vend_id)";
  else if (_selectedVendorType->isChecked())
    sql += "WHERE (apaging_vend_vendtype_id=:vendtype_id)";
  else if (_vendorTypePattern->isChecked())
    sql += "WHERE (apaging_vendtype_code ~ :vendtype_pattern) ";

  sql += "GROUP BY apaging_vend_number,apaging_vend_id,apaging_vend_name ";
  
  //Get totals
  sql += " UNION SELECT 0, 'Totals:', '',formatMoney(SUM(apaging_total_val)),"
         "formatMoney(SUM(apaging_cur_val)),formatMoney(SUM(apaging_thirty_val)),"
         "formatMoney(SUM(apaging_sixty_val)),formatMoney(SUM(apaging_ninety_val)),"
         "formatMoney(SUM(apaging_plus_val)),1 AS sequence"
         " FROM apaging(:asofDate, :useDocDate) ";

  if (_selectedVendor->isChecked())
    sql += "WHERE (apaging_vend_id=:vend_id)";
  else if (_selectedVendorType->isChecked())
    sql += "WHERE (apaging_vend_vendtype_id=:vendtype_id)";
  else if (_vendorTypePattern->isChecked())
    sql += "WHERE (apaging_vendtype_code ~ :vendtype_pattern) ";

  sql += "ORDER BY sequence,apaging_vend_number";
  sql += ";";

  q.prepare(sql);
  q.bindValue(":asofDate",_asOf->date());
  q.bindValue(":useDocDate", QVariant(_useDocDate->isChecked(), 0));
  q.bindValue(":vend_id", _vend->id());
  q.bindValue(":vendtype_id", _vendorTypes->id());
  q.bindValue(":vendtype_pattern", _vendorType->text().upper());
  q.exec();
  if (q.first())
    _apopen->populate(q);
}

void dspTimePhasedOpenAPItems::sToggleCustom()
{
  if (_custom->isChecked())
  {
    _calendarLit->setHidden(FALSE);
    _calendar->setHidden(FALSE);
    _periods->setHidden(FALSE);
    _asOf->setDate(omfgThis->dbDate(), true);
    _asOf->setEnabled(FALSE);
    _useGroup->setHidden(TRUE);
  }
  else
  {
    _calendarLit->setHidden(TRUE);
    _calendar->setHidden(TRUE);
    _periods->setHidden(TRUE);
    _asOf->setEnabled(TRUE);
    _useGroup->setHidden(FALSE);

    _apopen->setColumnCount(2);
    _apopen->addColumn(tr("Total Open"), _bigMoneyColumn, Qt::AlignRight);
    _apopen->addColumn(tr("0+ Days"), _bigMoneyColumn, Qt::AlignRight);
    _apopen->addColumn(tr("0-30 Days"), _bigMoneyColumn, Qt::AlignRight);
    _apopen->addColumn(tr("31-60 Days"), _bigMoneyColumn, Qt::AlignRight);
    _apopen->addColumn(tr("61-90 Days"), _bigMoneyColumn, Qt::AlignRight);
    _apopen->addColumn(tr("90+ Days"), _bigMoneyColumn, Qt::AlignRight);
  }
}
