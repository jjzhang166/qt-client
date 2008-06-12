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

#include "dspSummarizedSalesHistoryByShippingZone.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>

/*
 *  Constructs a dspSummarizedSalesHistoryByShippingZone as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedSalesHistoryByShippingZone::dspSummarizedSalesHistoryByShippingZone(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectedShippingZone, SIGNAL(toggled(bool)), _shipZone, SLOT(setEnabled(bool)));

  _productCategory->setType(ProductCategory);

  _shipZone->populate( "SELECT shipzone_id, (shipzone_name || '-' || shipzone_descrip) "
                       "FROM shipzone "
                       "ORDER BY shipzone_name;" );

  _sohist->addColumn(tr("Zone"),        _itemColumn,  Qt::AlignCenter );
  _sohist->addColumn(tr("Customer"),    200,          Qt::AlignLeft   );
  _sohist->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _sohist->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _sohist->addColumn(tr("Shipped"),     _qtyColumn,   Qt::AlignRight  );
  _sohist->addColumn(tr("Ext. Price"),  _moneyColumn, Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedSalesHistoryByShippingZone::~dspSummarizedSalesHistoryByShippingZone()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedSalesHistoryByShippingZone::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedSalesHistoryByShippingZone::sFillList()
{
  QString sql( "SELECT shipzone_id, cust_id, "
               "       shipzone_name, (cust_number || '-' || cust_name),"
               "       item_number, (item_descrip1 || ' ' || item_descrip2),"
               "       formatQty(SUM(cohist_qtyshipped)),"
               "       formatMoney(SUM(round(cohist_qtyshipped * cohist_unitprice,2))) "
               "FROM cohist, cust, shipto, itemsite, item, shipzone "
               "WHERE ((cohist_cust_id=cust_id)"
               " AND (cohist_shipto_id=shipto_id)"
               " AND (cohist_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (shipto_shipzone_id=shipzone_id)"
               " AND (cohist_shipdate BETWEEN :startDate and :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_productCategory->isSelected())
    sql += " AND (item_prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += " AND (item_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :prodcat_pattern)))";

  if (_selectedShippingZone->isChecked())
    sql += " AND (shipzone_id=:shipzone_id)";

  sql += ") "
         "GROUP BY shipzone_id, cust_id, shipzone_name, cust_number, cust_name, item_number, item_descrip1, item_descrip2;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _productCategory->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":shipzone_id", _shipZone->id());
  q.exec();
  _sohist->populate(q, TRUE);
}

void dspSummarizedSalesHistoryByShippingZone::sPrint()
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

  if(_selectedShippingZone->isChecked())
    params.append("shipzone_id", _shipZone->id());

  orReport report("SummarizedSalesHistoryByShippingZone", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

