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

#include "dspSummarizedSalesByCustomerTypeByItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a dspSummarizedSalesByCustomerTypeByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedSalesByCustomerTypeByItem::dspSummarizedSalesByCustomerTypeByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _customerType->setType(CustomerType);

  _sohist->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _sohist->addColumn(tr("Description"), -1 ,          Qt::AlignLeft   );
  _sohist->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter );
  _sohist->addColumn(tr("Min. Price"),  _priceColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Max. Price"),  _priceColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Avg. Price"),  _priceColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Total Units"), _qtyColumn,   Qt::AlignRight  );
  _sohist->addColumn(tr("Total $"),     _moneyColumn, Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedSalesByCustomerTypeByItem::~dspSummarizedSalesByCustomerTypeByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedSalesByCustomerTypeByItem::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedSalesByCustomerTypeByItem::sPrint()
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
  _customerType->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  orReport report("SummarizedSalesHistoryByCustomerTypeByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSummarizedSalesByCustomerTypeByItem::sFillList()
{
  _sohist->clear();

  if (!checkParameters())
    return;

  QString sql( "SELECT itemsite_id, item_number, item_descrip, warehous_code,"
               "       formatSalesPrice(minprice) AS f_minprice,"
               "       formatSalesPrice(maxprice) AS f_maxprice,"
               "       formatSalesPrice(avgprice) AS f_avgprice,"
               "       formatQty(totalunits) AS f_totalunits,"
               "       totalsales, formatMoney(totalsales) AS f_totalsales "
               "FROM ( SELECT itemsite_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
               "              warehous_code, MIN(cohist_unitprice) AS minprice, MAX(cohist_unitprice) AS maxprice,"
               "              AVG(cohist_unitprice) AS avgprice, SUM(cohist_qtyshipped) AS totalunits,"
               "              SUM(round(cohist_qtyshipped * cohist_unitprice,2)) AS totalsales "
               "       FROM cohist, cust, custtype, itemsite, item, warehous "
               "       WHERE ( (cohist_cust_id=cust_id)"
               "        AND (cust_custtype_id=custtype_id)"
               "        AND (cohist_itemsite_id=itemsite_id)"
               "        AND (itemsite_item_id=item_id)"
               "        AND (itemsite_warehous_id=warehous_id)"
               "        AND (cohist_invcdate BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_customerType->isSelected())
    sql += " AND (custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " AND (custtype_code ~ :custtype_pattern)";

  sql += ") "
         "GROUP BY itemsite_id, item_number, item_descrip1, item_descrip2, warehous_code ) AS data "
         "ORDER BY item_number, warehous_code;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _customerType->bindValue(q);
  _dates->bindValue(q);
  q.exec();
  if (q.first())
  {
    double totalSales = 0.0;
    XTreeWidgetItem *last = 0;

    do
    {
      last = new XTreeWidgetItem(_sohist, last, q.value("itemsite_id").toInt(),
				 q.value("item_number"),
				 q.value("item_descrip"),
				 q.value("warehous_code"),
				 q.value("f_minprice"),
				 q.value("f_maxprice"),
				 q.value("f_avgprice"),
				 q.value("f_totalunits"),
				 q.value("f_totalsales") );

      totalSales += q.value("totalsales").toDouble();
    }
    while (q.next());

    new XTreeWidgetItem( _sohist, last, -1,
                       "", tr("Totals"), "", "", "", "", "",
                       formatMoney(totalSales) );

    _sohist->setDragString("itemsiteid=");
  }
}

bool dspSummarizedSalesByCustomerTypeByItem::checkParameters()
{
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
                            tr("Please enter a valid End Data.") );
      _dates->setFocus();
    }
    return FALSE;
  }

  return TRUE;
}

