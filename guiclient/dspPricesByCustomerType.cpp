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

#include "dspPricesByCustomerType.h"

#include <parameter.h>
#include <openreports.h>

#define CURR_COL	7
#define COST_COL	8
#define MARGIN_COL	9

dspPricesByCustomerType::dspPricesByCustomerType(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));

  _custtype->setType(XComboBox::CustomerTypes);

  _price->addColumn(tr("Schedule"),    _itemColumn,  Qt::AlignLeft  );
  _price->addColumn(tr("Source"),      _itemColumn,  Qt::AlignLeft  );
  _price->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft  );
  _price->addColumn(tr("Description"), -1,           Qt::AlignLeft  );
  _price->addColumn(tr("Price UOM"),   _uomColumn,   Qt::AlignCenter);
  _price->addColumn(tr("Qty. Break"),  _qtyColumn,   Qt::AlignRight );
  _price->addColumn(tr("Price"),       _priceColumn, Qt::AlignRight );
  _price->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft );
  _price->addColumn(tr("Ext. Cost"),   _costColumn,  Qt::AlignRight );
  _price->addColumn(tr("Mar. %"),      _prcntColumn, Qt::AlignRight );

  if (omfgThis->singleCurrency())
    _price->hideColumn(CURR_COL);
  sHandleCosts(_showCosts->isChecked());
}

dspPricesByCustomerType::~dspPricesByCustomerType()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPricesByCustomerType::languageChange()
{
  retranslateUi(this);
}

void dspPricesByCustomerType::sPrint()
{
  ParameterList params;

  params.append("custtype_id", _custtype->id());

  if(_showExpired->isChecked())
    params.append("showExpired");
  if(_showFuture->isChecked())
    params.append("showFuture");

  if(_showCosts->isChecked())
    params.append("showCosts");

  if(_useActualCosts->isChecked())
    params.append("actualCosts");
  else /*if(_useStandardCosts->isChecked())*/
    params.append("standardCosts");

  orReport report("PricesByCustomerType", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPricesByCustomerType::sHandleCosts(bool pShowCosts)
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

void dspPricesByCustomerType::sFillList()
{
  _price->clear();

  if (_custtype->isValid())
  {
    QString sql = "SELECT itemid, sourcetype, schedulename, type, itemnumber, itemdescrip, priceuom,"
                  "       CASE WHEN (qtybreak = -1) THEN :na"
                  "            ELSE formatQty(qtybreak)"
                  "       END AS f_qtybreak,"
                  "       price, formatSalesPrice(price) AS f_price, "
		  "       currConcat(curr_id) ";

    if (_showCosts->isChecked())
      sql += ", cost, COALESCE(formatCost(cost), :costna) AS f_cost,"
             " CASE WHEN ((price = 0) OR (cost = 0)) THEN :na"
             "      ELSE formatScrap((price - cost) / price)"
             " END AS f_margin ";

    sql += "FROM ( SELECT ipsprice_id AS itemid, 1 AS sourcetype,"
           "              ipshead_name AS schedulename, :custType AS type,"
           "              item_number AS itemnumber, uom_name AS priceuom, iteminvpricerat(item_id) AS invpricerat,"
           "              (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
           "              ipsprice_qtybreak AS qtybreak, "
	   "		  ipsprice_price AS price, ipshead_curr_id AS curr_id, "
	   "		  ipshead_updated AS effective ";

    if (_showCosts->isChecked())
    {
      if (_useStandardCosts->isChecked())
        sql += ", (stdcost(ipsprice_item_id) * iteminvpricerat(item_id)) AS cost ";
      else if (_useActualCosts->isChecked())
        sql += ", (actcost(ipsprice_item_id) * iteminvpricerat(item_id)) AS cost ";
    }

    sql += "FROM ipsass, ipshead, ipsprice, item, uom "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_price_uom_id=uom_id)"
           " AND (ipsass_custtype_id=:custtype_id) ";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 2 AS sourcetype,"
           "             ipshead_name AS schedulename, :custTypePattern AS type,"
           "             item_number AS itemnumber, uom_name AS priceuom, iteminvpricerat(item_id) AS invpricerat,"
           "             (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
           "             ipsprice_qtybreak AS qtybreak,"
	   "		 ipsprice_price AS price, ipshead_curr_id AS curr_id,"
	   "		 ipshead_updated AS effective ";

    if (_showCosts->isChecked())
    {
      if (_useStandardCosts->isChecked())
        sql += ", (stdcost(ipsprice_item_id) * iteminvpricerat(item_id)) AS cost ";
      else if (_useActualCosts->isChecked())
        sql += ", (actcost(ipsprice_item_id) * iteminvpricerat(item_id)) AS cost ";
    }

    sql += "FROM ipsass, ipshead, ipsprice, item, custtype, uom "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_price_uom_id=uom_id)"
           " AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
           " AND (custtype_code ~ ipsass_custtype_pattern)" 
           " AND (custtype_id=:custtype_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 3 AS sourcetype,"
           "             ipshead_name AS schedulename, (:sale || '-' || sale_name) AS type,"
           "             item_number AS itemnumber, uom_name AS priceuom, iteminvpricerat(item_id) AS invpricerat,"
           "             (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
           "             ipsprice_qtybreak AS qtybreak, "
	   "		 ipsprice_price AS price, ipshead_curr_id AS curr_id, "
	   "		 ipshead_updated AS effective ";
                  
    if (_showCosts->isChecked())
    {
      if (_useStandardCosts->isChecked())
        sql += ", (stdcost(ipsprice_item_id) * iteminvpricerat(item_id)) AS cost ";
      else if (_useActualCosts->isChecked())
        sql += ", (actcost(ipsprice_item_id) * iteminvpricerat(item_id)) AS cost ";
    }

    sql += "FROM sale, ipshead, ipsprice, item, uom "
           "WHERE ( (sale_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (item_price_uom_id=uom_id)"
           " AND (ipsprice_item_id=item_id)";

    if (!_showExpired->isChecked())
      sql += " AND (sale_enddate > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (sale_startdate <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT item_id AS itemid, 0 AS sourcetype,"
           "             '' AS schedulename, :listPrice AS type,"
           "             item_number AS itemnumber, uom_name AS priceuom, iteminvpricerat(item_id) AS invpricerat,"
           "             (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
           "             -1 AS qtybreak, item_listprice AS price, "
	   "		  baseCurrId() AS curr_id, "
	   "		  CURRENT_DATE AS effective ";

    if (_showCosts->isChecked())
    {
      if (_useStandardCosts->isChecked())
        sql += ", (stdcost(item_id) * iteminvpricerat(item_id)) AS cost ";
      else if (_useActualCosts->isChecked())
        sql += ", (actcost(item_id) * iteminvpricerat(item_id)) AS cost ";
    }

    sql += "FROM item JOIN uom ON (item_price_uom_id=uom_id)"
           "WHERE ( (item_sold)"
           " AND (NOT item_exclusive) ) ) AS data "
           "ORDER BY itemnumber, price;";

    q.prepare(sql);
    q.bindValue(":na", tr("N/A"));
    q.bindValue(":costna", tr("?????"));
    q.bindValue(":custType", tr("Cust. Type"));
    q.bindValue(":custTypePattern", tr("Cust. Type Pattern"));
    q.bindValue(":sale", tr("Sale"));
    q.bindValue(":listPrice", tr("List Price"));
    q.bindValue(":custtype_id", _custtype->id());
    q.exec();
    XTreeWidgetItem *last = 0;
    while (q.next())
    {
      last = new XTreeWidgetItem( _price, last, q.value("itemid").toInt(),
				 q.value("sourcetype").toInt(),
				 q.value("schedulename"), q.value("type"),
				 q.value("itemnumber"), q.value("itemdescrip"),
				 q.value("priceuom"), q.value("f_qtybreak"),
				 q.value("f_price"), q.value("currConcat"),
				 q.value("f_cost"),
				 q.value("f_margin"));
      if (q.value("cost").toDouble() > q.value("price").toDouble())
	last->setTextColor(MARGIN_COL, "red");
    }
  }
}
