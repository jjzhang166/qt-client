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

#include "dspBriefSalesHistoryByCustomer.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a dspBriefSalesHistoryByCustomer as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspBriefSalesHistoryByCustomer::dspBriefSalesHistoryByCustomer(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _productCategory->setType(ProductCategory);

  _sohist->addColumn(tr("S/O #"),      _orderColumn, Qt::AlignLeft   );
  _sohist->addColumn(tr("Cust. P/O #"), -1,          Qt::AlignLeft   );
  _sohist->addColumn(tr("Invoice #"),  _orderColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Ord. Date"),  _dateColumn,  Qt::AlignCenter );
  _sohist->addColumn(tr("Invc. Date"), _dateColumn,  Qt::AlignCenter );
  _sohist->addColumn(tr("Total"),      _moneyColumn, Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspBriefSalesHistoryByCustomer::~dspBriefSalesHistoryByCustomer()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspBriefSalesHistoryByCustomer::languageChange()
{
  retranslateUi(this);
}

void dspBriefSalesHistoryByCustomer::sPrint()
{
  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _productCategory->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("cust_id", _cust->id());

  orReport report("BriefSalesHistoryByCustomer", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBriefSalesHistoryByCustomer::sFillList()
{
  _sohist->clear();

  if (!checkParameters())
    return;

  QString sql( "SELECT cohist_ordernumber, cohist_ponumber, cohist_invcnumber,"
               "       formatDate(cohist_orderdate) AS f_orderdate,"
               "       formatDate(cohist_invcdate, 'Return') AS f_invcdate,"
               "       SUM(round(cohist_qtyshipped * cohist_unitprice,2)) AS extended,"
               "       formatMoney(SUM(round(cohist_qtyshipped * cohist_unitprice,2))) AS f_extended "
               "FROM cohist, itemsite, item "
               "WHERE ( (cohist_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (cohist_invcdate BETWEEN :startDate AND :endDate)"
               " AND (cohist_cust_id=:cust_id)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_productCategory->isSelected())
    sql += " AND (item_prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += " AND (item_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :prodcat_pattern)))";

  sql += ") "
         "GROUP BY cohist_ordernumber, cohist_ponumber, cohist_invcnumber,"
         "         cohist_orderdate, cohist_invcdate "
         "ORDER BY cohist_invcdate, cohist_orderdate;";

  q.prepare(sql);
  _dates->bindValue(q);
  q.bindValue(":cust_id", _cust->id());
  _warehouse->bindValue(q);
  _productCategory->bindValue(q);
  q.exec();
  if (q.first())
  {
    double        totalSales = 0.0;

    XTreeWidgetItem *last = 0;
    do
    {
      last = new XTreeWidgetItem( _sohist, last, -1, 
			       q.value("cohist_ordernumber"), q.value("cohist_ponumber"),
			       q.value("cohist_invcnumber"), q.value("f_orderdate"),
			       q.value("f_invcdate"), q.value("f_extended") );

      totalSales += q.value("extended").toDouble();
    }
    while (q.next());

    XTreeWidgetItem *totals = new XTreeWidgetItem(_sohist, last, -1, QVariant(tr("Totals")));
    totals->setText(5, formatMoney(totalSales));
  }
}

bool dspBriefSalesHistoryByCustomer::checkParameters()
{
    if (!_cust->isValid())
    {
        if(isVisible()) {
            QMessageBox::warning( this, tr("Enter Customer Number"),
                                  tr("Please enter a valid Customer Number.") );
            _cust->setFocus();
        }
        return FALSE;
    }

    if (!_dates->startDate().isValid())
    {
        if(isVisible()) {
            QMessageBox::warning( this, tr("Enter Start Date"),
                                  tr("Please enter a valid Start Date.") );
            _dates->setFocus();
        }
        return FALSE;
    }

    if (!_dates->endDate().isValid())
    {
        if(isVisible()) {
            QMessageBox::warning( this, tr("Enter End Date"),
                                  tr("Please enter a valid End Date.") );
            _dates->setFocus();
        }
        return FALSE;
    }

  return TRUE;
}
