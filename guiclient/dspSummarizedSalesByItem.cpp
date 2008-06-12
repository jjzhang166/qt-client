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

#include "dspSummarizedSalesByItem.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include "dspSalesHistoryByItem.h"

/*
 *  Constructs a dspSummarizedSalesByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedSalesByItem::dspSummarizedSalesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sohist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _customerType->setType(CustomerType);
  _productCategory->setType(ProductCategory);

  _sohist->addColumn(tr("Item"),        _itemColumn, Qt::AlignLeft   );
  _sohist->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _sohist->addColumn(tr("First Sale"),  _dateColumn, Qt::AlignCenter );
  _sohist->addColumn(tr("Last Sale"),   _dateColumn, Qt::AlignCenter );
  _sohist->addColumn(tr("Total Qty."),  _qtyColumn,  Qt::AlignRight  );
  _sohist->addColumn(tr("Total Sales"), _qtyColumn,  Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedSalesByItem::~dspSummarizedSalesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedSalesByItem::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedSalesByItem::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("View Sales Detail..."), this, SLOT(sViewDetail()), 0);
}

void dspSummarizedSalesByItem::sViewDetail()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);
  _customerType->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _sohist->id());
  params.append("run");

  dspSalesHistoryByItem *newdlg = new dspSalesHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSummarizedSalesByItem::sPrint()
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
  _customerType->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_orderByItemNumber->isChecked())
    params.append("orderByItemNumber");
  else if (_orderByQtyVolume->isChecked())
    params.append("orderByQtyVolume");
  else if (_orderBySalesVolume->isChecked())
    params.append("orderBySalesVolume");

  orReport report("SummarizedSalesHistoryByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSummarizedSalesByItem::sFillList()
{
  if (!checkParameters())
    return;

  QString sql( "SELECT item_id, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2) AS itemdescription,"
               "       formatDate(MIN(cohist_invcdate)) AS f_firstdate,"
               "       formatDate(MAX(cohist_invcdate)) AS f_lastdate,"
               "       formatQty(SUM(cohist_qtyshipped)) AS f_totalunits,"
               "       formatMoney(SUM(round(cohist_qtyshipped * cohist_unitprice,2))) AS f_totalsales,"
               "       SUM(round(cohist_qtyshipped * cohist_unitprice,2)) AS totalsales "
               "FROM cohist, cust, item, itemsite "
               "WHERE ( (cohist_cust_id=cust_id)"
               " AND (cohist_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (cohist_invcdate BETWEEN :startDate AND :endDate)" );

  if (_productCategory->isSelected())
    sql += " AND (item_prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += " AND (item_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :prodcat_pattern)))";

  if (_customerType->isSelected())
    sql += " AND (cust_custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ :custtype_pattern)))";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "GROUP BY item_id, item_number, item_descrip1, item_descrip2 ";

  if (_orderByItemNumber->isChecked())
    sql += "ORDER BY item_number;";
  else if (_orderByQtyVolume->isChecked())
    sql += "ORDER BY SUM(cohist_qtyshipped) DESC;";
  else if (_orderBySalesVolume->isChecked())
    sql += "ORDER BY SUM(cohist_qtyshipped * cohist_unitprice) DESC;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _customerType->bindValue(q);
  _productCategory->bindValue(q);
  _dates->bindValue(q);
  q.exec();
  _sohist->populate(q);

  if (q.first())
  {
    double totalSales = 0.0;

    do
      totalSales += q.value("totalsales").toDouble();
    while (q.next());

    new XTreeWidgetItem(_sohist,
			_sohist->topLevelItem(_sohist->topLevelItemCount() - 1),
			-1,
                        QVariant(tr("Totals")), "", "", "", "", 
                        formatMoney(totalSales) );
  }

}

bool dspSummarizedSalesByItem::checkParameters()
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
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
    }
    return FALSE;
  }

  return TRUE;
}

