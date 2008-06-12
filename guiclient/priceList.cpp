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

#include "priceList.h"

#include <QVariant>
#include <QSqlError>

/*
 *  Constructs a priceList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
priceList::priceList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_price, SIGNAL(itemSelected(int)), this, SLOT(sSelect()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_price, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
  connect(_price, SIGNAL(valid(bool)), _select, SLOT(setEnabled(bool)));

  _price->addColumn(tr("Schedule"),    _itemColumn,  Qt::AlignLeft  );
  _price->addColumn(tr("Source"),      _itemColumn,  Qt::AlignLeft  );
  _price->addColumn(tr("Qty. Break"),  _qtyColumn,   Qt::AlignRight );
  _price->addColumn(tr("Price"),       _priceColumn, Qt::AlignRight );
  _price->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft );
  _price->addColumn(tr("Price (in curr)"), _priceColumn, Qt::AlignRight );
  // column title reset in priceList::set

  if (omfgThis->singleCurrency())
  {
      _price->hideColumn(4);
      _price->hideColumn(5);
  }

  _shiptoid = -1;
}

/*
 *  Destroys the object and frees any allocated resources
 */
priceList::~priceList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void priceList::languageChange()
{
    retranslateUi(this);
}

enum SetResponse priceList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _cust->setId(param.toInt());
    _cust->setReadOnly(TRUE);
  }

  param = pParams.value("shipto_id", &valid);
  if (valid)
    _shiptoid = param.toInt();

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _curr_id = param.toInt();
    if (! omfgThis->singleCurrency())
    {
      QString _currConcat;
      q.prepare("SELECT currConcat(:curr_id) AS currConcat;");
      q.bindValue(":curr_id", _curr_id);
      q.exec();
      if (q.first())
        _currConcat = q.value("currConcat").toString();
      else
        _currConcat = tr("?????");
      _price->headerItem()->setText(5, tr("Price\n(in %1)").arg(_currConcat));
    }
  }

  param = pParams.value("effective", &valid);
  _effective = (valid) ? param.toDate() : QDate::currentDate();

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _qty->setText(formatQty(param.toDouble()));
    _qty->setEnabled(FALSE);
  }

  sFillList();

  return NoError;
}

void priceList::sSelect()
{
  switch (_price->id())
  {
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
      q.prepare( "SELECT currToCurr(ipshead_curr_id, :curr_id, ipsitem_price, "
		 "		    :effective) AS price "
                 "FROM ipsitem JOIN ipshead ON (ipsitem_ipshead_id = ipshead_id) "
                 "WHERE (ipsitem_id=:ipsitem_id);" );
      q.bindValue(":ipsitem_id", _price->altId());

      break;

    case 11:
    case 12:
    case 13:
    case 14:
    case 16:
      q.prepare( "SELECT currToLocal(:curr_id,"
	         "         item_listprice - (item_listprice * ipsprodcat_discntprcnt),"
		 "         :effective) AS price "
                 "  FROM ipsprodcat JOIN item ON (ipsprodcat_prodcat_id=item_prodcat_id AND item_id=:item_id) "
                 " WHERE (ipsprodcat_id=:ipsprodcat_id);" );
      q.bindValue(":item_id", _item->id());
      q.bindValue(":ipsprodcat_id", _price->altId());

      break;

    case 5:
      q.prepare( "SELECT currToLocal(:curr_id, "
      		 "	  item_listprice - (item_listprice * cust_discntprcnt),"
		 "	  :effective) AS price "
                 "FROM cust, item "
                 "WHERE ( (cust_id=:cust_id)"
                 " AND (item_id=:item_id) );" );
      q.bindValue(":cust_id", _cust->id());
      q.bindValue(":item_id", _item->id());

      break;

    default:
      q.prepare( "SELECT 0 AS price;" );
  }

  q.bindValue(":curr_id", _curr_id);
  q.bindValue(":effective", _effective);
  q.exec();
  if (q.first())
    _selectedPrice = q.value("price").toDouble();
  else
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void priceList::sFillList()
{
  q.prepare( "SELECT source, sourceid, schedulename, type,"
             "       CASE WHEN (qtybreak = -1) THEN :na"
             "            ELSE formatQty(qtybreak)"
             "       END,"
             "       formatSalesPrice(price), currConcat(curr_id),"
	     "	     formatSalesPrice(currToCurr(curr_id, :curr_id, price, :effective)) "
             "FROM ( SELECT 1 + CASE WHEN(ipsprice_source='P') THEN 10 ELSE 0 END AS source, ipsprice_id AS sourceid,"
             "              ipshead_name AS schedulename, :customer AS type,"
             "              ipsprice_qtybreak AS qtybreak, ipsprice_price AS price,"
	     "		    ipshead_curr_id AS curr_id "
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) = 0)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"

             "       UNION SELECT 2 + CASE WHEN(ipsprice_source='P') THEN 10 ELSE 0 END AS source, ipsprice_id AS sourceid,"
             "                    ipshead_name AS schedulename, :custType AS type,"
             "                    ipsprice_qtybreak AS qtybreak, ipsprice_price AS price,"
	     "                    ipshead_curr_id AS curr_id "
             "       FROM ipsass, ipshead, ipsprice, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_custtype_id=cust_custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"

             "       UNION SELECT 3 + CASE WHEN(ipsprice_source='P') THEN 10 ELSE 0 END AS source, ipsprice_id AS sourceid,"
             "                    ipshead_name AS schedulename, :custTypePattern AS type,"
             "                    ipsprice_qtybreak AS qtybreak, ipsprice_price AS price,"
	     "                    ipshead_curr_id AS curr_id "
             "       FROM ipsass, ipshead, ipsprice, custtype, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
             "        AND (custtype_code ~ ipsass_custtype_pattern)"
             "        AND (cust_custtype_id=custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"

             "       UNION SELECT 6 + CASE WHEN(ipsprice_source='P') THEN 10 ELSE 0 END AS source, ipsprice_id AS sourceid,"
             "                    ipshead_name AS schedulename, :shipTo AS type,"
             "                    ipsprice_qtybreak AS qtybreak, ipsprice_price AS price,"
	     "                    ipshead_curr_id AS curr_id "
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_shipto_id=:shipto_id)"
             "        AND (ipsass_shipto_id != -1)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
     
             "       UNION SELECT 7 + CASE WHEN(ipsprice_source='P') THEN 10 ELSE 0 END AS source, ipsprice_id AS sourceid,"
             "                    ipshead_name AS schedulename, :shipToPattern AS type,"
             "                    ipsprice_qtybreak AS qtybreak, ipsprice_price AS price,"
	     "                    ipshead_curr_id AS curr_id "
             "       FROM ipsass, ipshead, ipsprice, shipto "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (shipto_id=:shipto_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) > 0)"
             "        AND (shipto_num ~ ipsass_shipto_pattern)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
     
             "       UNION SELECT 4 + CASE WHEN(ipsprice_source='P') THEN 10 ELSE 0 END AS source, ipsprice_id AS sourceid,"
             "                    ipshead_name AS schedulename, (:sale || '-' || sale_name) AS type,"
             "                    ipsprice_qtybreak AS qtybreak, ipsprice_price AS price,"
	     "                    ipshead_curr_id AS curr_id "
             "       FROM sale, ipshead, ipsprice "
             "       WHERE ((sale_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (CURRENT_DATE BETWEEN sale_startdate AND (sale_enddate - 1)) ) "

             "       UNION SELECT 5 AS source, item_id AS sourceid,"
             "               '' AS schedulename, :listPrice AS type,"
             "               -1 AS qtybreak, "
	     "		    (item_listprice - (item_listprice * cust_discntprcnt)) AS price, "
	     "		    baseCurrId() AS curr_id "
             "       FROM item, cust "
             "       WHERE ( (item_sold)"
             "        AND (NOT item_exclusive)"
             "        AND (item_id=:item_id)"
             "        AND (cust_id=:cust_id) ) ) AS data "
             "ORDER BY price;" );
  q.bindValue(":na", tr("N/A"));
  q.bindValue(":customer", tr("Customer"));
  q.bindValue(":shipTo", tr("Cust. Ship-To"));
  q.bindValue(":shipToPattern", tr("Cust. Ship-To Pattern"));
  q.bindValue(":custType", tr("Cust. Type"));
  q.bindValue(":custTypePattern", tr("Cust. Type Pattern"));
  q.bindValue(":sale", tr("Sale"));
  q.bindValue(":listPrice", tr("List Price"));
  q.bindValue(":item_id", _item->id());
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":shipto_id", _shiptoid);
  q.bindValue(":curr_id", _curr_id);
  q.bindValue(":effective", _effective);
  q.exec();
  _price->populate(q, TRUE);

  for (int i = 0; i < _price->topLevelItemCount(); i++)
  {
    QTreeWidgetItem *cursor = _price->topLevelItem(i);
    if ( (cursor->text(2) != tr("N/A")) &&
         (_qty->toDouble() >= cursor->text(2).toDouble()) )
    {
      _price->setCurrentItem(cursor);
      break;
    }
  }
}
