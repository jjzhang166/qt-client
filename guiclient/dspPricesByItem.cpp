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

#include "dspPricesByItem.h"

#include <openreports.h>
#include <parameter.h>

#define CURR_COL	5
#define COST_COL	6
#define MARGIN_COL	7

dspPricesByItem::dspPricesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));

  _item->setType(ItemLineEdit::cSold);

  _price->addColumn(tr("Schedule"),      _itemColumn, Qt::AlignLeft  );
  _price->addColumn(tr("Source"),        _itemColumn, Qt::AlignLeft  );
  _price->addColumn(tr("Customer/Customer Type"), -1, Qt::AlignLeft  );
  _price->addColumn(tr("Qty. Break"),     _qtyColumn, Qt::AlignRight );
  _price->addColumn(tr("Price"),        _priceColumn, Qt::AlignRight );
  _price->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft  );
  _price->addColumn(tr("Cost"),          _costColumn, Qt::AlignRight );
  _price->addColumn(tr("Margin"),       _prcntColumn, Qt::AlignRight );

  if (omfgThis->singleCurrency())
    _price->hideColumn(CURR_COL);
  sHandleCosts(_showCosts->isChecked());

  _item->setFocus();
}

dspPricesByItem::~dspPricesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPricesByItem::languageChange()
{
  retranslateUi(this);
}

void dspPricesByItem::sPrint()
{
  ParameterList params;

  params.append("item_id", _item->id());

  if(_showExpired->isChecked())
    params.append("showExpired");
  if(_showFuture->isChecked())
    params.append("showFuture");

  if (_showCosts->isChecked())
    params.append("showCosts");

  if (_useActualCosts->isChecked())
    params.append("actualCosts");
  else /*if(_useStandardCosts->isChecked())*/
    params.append("standardCosts");

  orReport report("PricesByItem", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPricesByItem::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
  {
    _price->showColumn(COST_COL);
    _price->showColumn(MARGIN_COL);
  }
  else
  {
    _price->hideColumn(COST_COL);
    _price->hideColumn(MARGIN_COL);
  }

  _costsGroup->setEnabled(pShowCosts);
}

void dspPricesByItem::sFillList()
{
  _price->clear();

  if (_item->isValid())
  {
    double cost = 0.0;

    if (_showCosts->isChecked())
    {
      if (_useStandardCosts->isChecked())
          q.prepare( "SELECT (stdCost(item_id) * iteminvpricerat(item_id)) AS cost "
                     "FROM item "
                     "WHERE (item_id=:item_id);");
      else if (_useActualCosts->isChecked())
          q.prepare( "SELECT (actCost(item_id) * iteminvpricerat(item_id)) AS cost "
                     "FROM item "
                     "WHERE (item_id=:item_id);");

      q.bindValue(":item_id", _item->id());
      q.exec();
      if (q.first())
        cost = q.value("cost").toDouble();
//  ToDo
    }

    QString sql( "SELECT ipsprice_id AS itemid, 1 AS sourcetype,"
                 "       ipshead_name AS schedulename, :customer AS type,"
                 "       cust_name AS typename,"
                 "       CASE WHEN (ipsprice_qtybreak = -1) THEN :na"
                 "            ELSE formatQty(ipsprice_qtybreak)"
                 "       END AS f_qtybreak,"
                 "       ipsprice_price AS price, currConcat(ipshead_curr_id) AS currConcat "
                 "FROM ipsass, ipshead, ipsprice, cust, item "
                 "WHERE ( (ipsass_ipshead_id=ipshead_id)"
                 " AND (ipsprice_ipshead_id=ipshead_id)"
                 " AND (ipsass_cust_id=cust_id)"
                 " AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) = 0)"
                 " AND (ipsprice_item_id=item_id)"
                 " AND (item_id=:item_id)" );

    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 2 AS sourcetype,"
           "             ipshead_name AS schedulename, :custType AS type,"
           "             (custtype_code || '-' || custtype_descrip) AS typename,"
           "             CASE WHEN (ipsprice_qtybreak = -1) THEN :na"
           "                  ELSE formatQty(ipsprice_qtybreak)"
           "             END AS f_qtybreak,"
           "             ipsprice_price AS price, currConcat(ipshead_curr_id) AS currConcat "
           "FROM ipsass, ipshead, ipsprice, custtype, item "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (ipsass_custtype_id=custtype_id)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_id=:item_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 3 AS sourcetype,"
           "             ipshead_name AS schedulename, :custTypePattern AS type,"
           "             (custtype_code || '-' || custtype_descrip) AS typename,"
           "             CASE WHEN (ipsprice_qtybreak = -1) THEN :na"
           "                  ELSE formatQty(ipsprice_qtybreak)"
           "             END AS f_qtybreak,"
           "             ipsprice_price AS price, currConcat(ipshead_curr_id) AS currConcat "
           "FROM ipsass, ipshead, ipsprice, custtype, item "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
           " AND (custtype_code ~ ipsass_custtype_pattern)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_id=:item_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 4 AS sourcetype,"
           "             ipshead_name AS schedulename, :sale AS type,"
           "             sale_name AS typename,"
           "             CASE WHEN (ipsprice_qtybreak = -1) THEN :na"
           "                  ELSE formatQty(ipsprice_qtybreak)"
           "             END AS f_qtybreak,"
           "             ipsprice_price AS price, currConcat(ipshead_curr_id) AS currConcat "
           "FROM sale, ipshead, ipsprice, item "
           "WHERE ( (sale_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (ipsprice_item_id=:item_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (sale_enddate > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (sale_startdate <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 5 AS sourcetype,"
           "             ipshead_name AS schedulename, :shipTo AS type,"
           "             (cust_name || '-' || shipto_num) AS typename,"
           "             CASE WHEN (ipsprice_qtybreak = -1) THEN :na"
           "                  ELSE formatQty(ipsprice_qtybreak)"
           "             END AS f_qtybreak,"
           "             ipsprice_price AS price, currConcat(ipshead_curr_id) AS currConcat "
           "FROM ipsass, ipshead, ipsprice, cust, shipto, item "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (ipsass_shipto_id=shipto_id)"
           " AND (shipto_cust_id=cust_id)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_id=:item_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 6 AS sourcetype,"
           "             ipshead_name AS schedulename, :shipToPattern AS type,"
           "             (cust_name || '-' || shipto_num) AS typename,"
           "             CASE WHEN (ipsprice_qtybreak = -1) THEN :na"
           "                  ELSE formatQty(ipsprice_qtybreak)"
           "             END AS f_qtybreak,"
           "             ipsprice_price AS price, currConcat(ipshead_curr_id) AS currConcat "
           "FROM ipsass, ipshead, ipsprice, cust, shipto, item "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (COALESCE(LENGTH(ipsass_shipto_pattern),0) > 0)"
           " AND (shipto_num ~ ipsass_shipto_pattern)"
           " AND (ipsass_cust_id=cust_id)"
           " AND (shipto_cust_id=cust_id)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_id=:item_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT item_id AS itemid, 0 AS sourcetype,"
           "             :listPrice AS schedulename, :na AS type,"
           "             '' AS typename,"
           "             :na AS f_qtybreak,"
           "             item_listprice AS price, currConcat(baseCurrId()) AS currConcat "
           "FROM item "
           "WHERE ( (NOT item_exclusive)"
           " AND (item_id=:item_id) ) "
           "ORDER BY price;";

    q.prepare(sql);
    q.bindValue(":na", tr("N/A"));
    q.bindValue(":customer", tr("Customer"));
    q.bindValue(":shipTo", tr("Cust. Ship-To"));
    q.bindValue(":shipToPattern", tr("Cust. Ship-To Pattern"));
    q.bindValue(":custType", tr("Cust. Type"));
    q.bindValue(":custTypePattern", tr("Cust. Type Pattern"));
    q.bindValue(":sale", tr("Sale"));
    q.bindValue(":listPrice", tr("List Price"));
    q.bindValue(":item_id", _item->id());
    q.exec();
    XTreeWidgetItem *last = 0;
    while (q.next())
    {
      double price = q.value("price").toDouble();
      last = new XTreeWidgetItem(_price, last, q.value("itemid").toInt(),
				 q.value("sourcetype").toInt(),
				 q.value("schedulename"), q.value("type"),
				 q.value("typename"), q.value("f_qtybreak"),
				 formatSalesPrice(q.value("price").toDouble()),
				 q.value("currConcat"),
				 formatCost(cost),
				 (price != 0) ? formatPercent(((price - cost) / price)) : QString());

      if (cost > price)
	last->setTextColor(MARGIN_COL, "red");
    }
  }
}
