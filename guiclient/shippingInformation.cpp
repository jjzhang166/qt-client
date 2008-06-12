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

#include "shippingInformation.h"

#include <QSqlError>
#include <QMessageBox>
#include <QVariant>

#include <metasql.h>

#include "issueToShipping.h"
#include "salesOrderItem.h"
#include "salesOrderList.h"
#include "storedProcErrorLookup.h"
#include "transferOrderItem.h"

shippingInformation::shippingInformation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  connect(_item,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)),
					  this, SLOT(sPopulateMenu(QMenu*)));
  connect(_salesOrder,	SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_salesOrder,	SIGNAL(requestList()), this, SLOT(sSalesOrderList()));
  connect(_salesOrderList, SIGNAL(clicked()), this, SLOT(sSalesOrderList()));
  connect(_save,	SIGNAL(clicked()), this, SLOT(sSave()));

  _captive = FALSE;

#ifndef Q_WS_MAC
  _salesOrderList->setMaximumWidth(25);
#endif

  _shippingForm->setCurrentItem(-1);

  _item->addColumn(tr("#"),           _seqColumn, Qt::AlignCenter );
  _item->addColumn(tr("Item"),        -1,         Qt::AlignLeft   );
  _item->addColumn(tr("At Shipping"), _qtyColumn, Qt::AlignRight  );
  _item->addColumn(tr("Net Wght."),   _qtyColumn, Qt::AlignRight  );
  _item->addColumn(tr("Tare Wght."),  _qtyColumn, Qt::AlignRight  );
  _item->addColumn(tr("Gross Wght."), _qtyColumn, Qt::AlignRight  );
}

shippingInformation::~shippingInformation()
{
  // no need to delete child widgets, Qt does it all for us
}

void shippingInformation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse shippingInformation::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cosmisc_id", &valid);	// deprecated
  if (valid)
    _shipment->setId(param.toInt());

  param = pParams.value("shiphead_id", &valid);
  if (valid)
    _shipment->setId(param.toInt());

  if (_shipment->isValid())
  {
    q.prepare( "SELECT shiphead_order_id, shiphead_order_type "
               "FROM shiphead "
               "WHERE (shiphead_id=:shiphead_id);" );
    q.bindValue(":shiphead_id", _shipment->id());
    q.exec();
    if (q.first())
    {
      _captive = TRUE;

      if (q.value("shiphead_order_type").toString() == "SO")
	_salesOrder->setId(q.value("shiphead_order_id").toInt());
      else
	_to->setId(q.value("shiphead_order_id").toInt());

      _close->setText("&Cancel");

      _shipDate->setFocus();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  return NoError;
}

void shippingInformation::sSave()
{
  if (!_shipDate->isValid())
  {
    QMessageBox::information( this, tr("No Ship Date Entered"),
                              tr("<p>You must enter a Ship Date before "
				 "selecting this order for billing.") );

    _shipDate->setFocus();
    return;
  }

  if (_shipment->id() > 0)
  {
    q.prepare( "UPDATE shiphead "
               "SET shiphead_freight=:shiphead_freight,"
	       "    shiphead_freight_curr_id=:shiphead_freight_curr_id,"
	       "    shiphead_notes=:shiphead_notes,"
               "    shiphead_shipdate=:shiphead_shipdate, shiphead_shipvia=:shiphead_shipvia,"
               "    shiphead_shipchrg_id=:shiphead_shipchrg_id, shiphead_shipform_id=:shiphead_shipform_id "
               "WHERE (shiphead_id=:shiphead_id);" );
    q.bindValue(":shiphead_id", _shipment->id());
  }
  else
    q.prepare( "INSERT INTO shiphead "
               "( shiphead_order_id, shiphead_freight,"
	       "  shiphead_freight_curr_id, shiphead_notes,"
               "  shiphead_shipdate, shiphead_shipvia, shiphead_number,"
               "  shiphead_shipchrg_id, shiphead_shipform_id, shiphead_shipped ) "
               "VALUES "
               "( :shiphead_order_id, :shiphead_freight,"
	       "  :shiphead_freight_curr_id, :shiphead_notes,"
               "  :shiphead_shipdate, :shiphead_shipvia, fetchShipmentNumber(),"
               "  :shiphead_shipchrg_id, :shiphead_shipform_id, FALSE );" );

  q.bindValue(":shiphead_order_id",		_salesOrder->id());
  q.bindValue(":shiphead_freight",		_freight->localValue());
  q.bindValue(":shiphead_freight_curr_id",	_freight->id());
  q.bindValue(":shiphead_notes", _notes->text());
  q.bindValue(":shiphead_shipdate", _shipDate->date());
  q.bindValue(":shiphead_shipvia", _shipVia->currentText());
  q.bindValue(":shiphead_shipchrg_id", _shippingCharges->id());
  q.bindValue(":shiphead_shipform_id", _shippingForm->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
  {
    _salesOrder->setId(-1);
    _to->setId(-1);
  }
}

void shippingInformation::sSalesOrderList()
{
  ParameterList params;
  params.append("sohead_id", _salesOrder->id());
  params.append("soType", cSoOpen);
  
  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  int soheadid;
  soheadid = newdlg.exec();
  if (soheadid == XDialog::Rejected)
    _salesOrder->setId(-1);
  else
    _salesOrder->setId(soheadid);
}

void shippingInformation::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("Issue Additional Stock for this Order Line to Shipping..."), this, SLOT(sIssueStock()), 0);
  menuThis->insertItem(tr("Return ALL Stock Issued for this Order Line to the Warehouse..."), this, SLOT(sReturnAllLineStock()), 0);
  menuThis->insertItem(tr("View Order Line..."), this, SLOT(sViewLine()), 0);
}

void shippingInformation::sIssueStock()
{
  ParameterList params;
  if (_salesOrder->isValid())
    params.append("sohead_id", _item->altId());
  else if (_to->isValid())
    params.append("tohead_id", _item->altId());

  issueToShipping *newdlg = new issueToShipping();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void shippingInformation::sReturnAllLineStock()
{
  q.prepare("SELECT returnItemShipments(:ordertype, :lineitemid,"
	    "                           0, CURRENT_TIMESTAMP) AS result;");
  if (_salesOrder->isValid())
    q.bindValue(":ordertype", "SO");
  else if (_to->isValid())
    q.bindValue(":ordertype", "TO");

  q.bindValue(":lineitemid", _item->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("returnItemShipments", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void shippingInformation::sViewLine()
{
  ParameterList params;
  params.append("mode", "view");
  if (_salesOrder->isValid())
  {
    params.append("soitem_id", _item->id());

    salesOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (_to->isValid())
  {
    params.append("toitem_id", _item->id());

    transferOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void shippingInformation::sFillList()
{
//  Clear any old data
  _item->clear();

  QString shs( "SELECT shiphead_notes, shiphead_shipvia,"
               "       shiphead_shipchrg_id, shiphead_shipform_id,"
               "       shiphead_freight, shiphead_freight_curr_id,"
               "       shiphead_shipdate,"
               "       CASE WHEN (shiphead_shipdate IS NULL) THEN FALSE"
               "            ELSE TRUE"
               "       END AS validdata "
               "FROM shiphead "
               "WHERE ((NOT shiphead_shipped)"
	       "  AND  (shiphead_order_type=<? value(\"ordertype\") ?>)"
               "  AND  (shiphead_order_id=<? value(\"orderid\") ?>) );" ) ;
  ParameterList shp;
  if (_salesOrder->isValid())
  {
    shp.append("ordertype", "SO");
    shp.append("orderid",   _salesOrder->id());
  }
  else if (_to->isValid())
  {
    shp.append("ordertype", "TO");
    shp.append("orderid",   _to->id());
  }
  else
  {
    _salesOrder->setId(-1);
    _orderDate->setNull();
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
    _shipDate->setNull();
    _shipVia->setNull();

    _freight->reset();
    _totalNetWeight->clear();
    _totalTareWeight->clear();
    _totalGrossWeight->clear();
    return;
  }

  MetaSQLQuery shm(shs);
  q = shm.toQuery(shp);
  bool fetchFromHead = TRUE;

  if (q.first())
  {
    if (q.value("validdata").toBool())
    {
      fetchFromHead = FALSE;

      _shipDate->setDate(q.value("shiphead_shipdate").toDate());
      _shipVia->setText(q.value("shiphead_shipvia").toString());
      _shippingCharges->setId(q.value("shiphead_shipchrg_id").toInt());
      _shippingForm->setId(q.value("shiphead_shipform_id").toInt());
      _freight->setId(q.value("shiphead_freight_curr_id").toInt());
      _freight->setLocalValue(q.value("shiphead_freight").toDouble());
      _notes->setText(q.value("shiphead_notes").toString());
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_salesOrder->isValid())
  {
    q.prepare( "SELECT cohead_orderdate AS orderdate,"
	       "       cohead_holdtype AS holdtype,"
	       "       cohead_custponumber AS ponumber,"
               "       cust_name AS name, cust_phone AS phone,"
               "       cohead_shipcomments AS shipcomments,"
	       "       cohead_shipvia AS shipvia,"
               "       cohead_shipchrg_id AS shipchrg_id,"
	       "       cohead_shipform_id AS shipform_id "
               "FROM cohead, cust "
               "WHERE ((cohead_cust_id=cust_id)"
               " AND (cohead_id=:cohead_id));" );
    q.bindValue(":cohead_id", _salesOrder->id());
  }
  else if (_to->isValid())
  {
    q.prepare( "SELECT tohead_orderdate AS orderdate,"
	       "       'N' AS holdtype,"
	       "       :to AS ponumber,"
	       "       tohead_destname AS name, tohead_destphone AS phone,"
	       "       tohead_shipcomments AS shipcomments,"
	       "       tohead_shipvia AS shipvia,"
	       "       tohead_shipchrg_id AS shipchrg_id,"
	       "       tohead_shipform_id AS shipform_id "
	       "FROM tohead "
	       "WHERE (tohead_id=:tohead_id);" );
    q.bindValue(":tohead_id", _to->id());
  }
  q.exec();
  if (q.first())
  {
    _orderDate->setDate(q.value("orderdate").toDate());
    _poNumber->setText(q.value("custponumber").toString());
    _custName->setText(q.value("name").toString());
    _custPhone->setText(q.value("phone").toString());

    QString msg;
    if (q.value("head_holdtype").toString() == "C")
      msg = storedProcErrorLookup("issuetoshipping", -12);
    else if (q.value("head_holdtype").toString() == "P")
      msg = storedProcErrorLookup("issuetoshipping", -13);
    else if (q.value("head_holdtype").toString() == "R")
      msg = storedProcErrorLookup("issuetoshipping", -14);

    if (! msg.isEmpty())
    {
      QMessageBox::warning(this, tr("Order on Hold"), msg);
      _salesOrder->setId(-1);
      _salesOrder->setFocus();
      _to->setId(-1);
      return;
    }

    if (fetchFromHead)
    {
      _shipDate->setDate(omfgThis->dbDate());

      _shippingCharges->setId(q.value("shipchrg_id").toInt());
      _shippingForm->setId(q.value("shipform_id").toInt());
      _notes->setText(q.value("shipcomments").toString());
      _shipVia->setText(q.value("shipvia").toString());
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_salesOrder->isValid())
  {
    q.prepare( "SELECT coitem_id AS itemid, coitem_cohead_id AS headid,"
	       "       coitem_linenumber AS linenumber, item_number,"
               "      (item_prodweight * itemuomtouom(item_id, coitem_price_uom_id, NULL, qtyAtShipping('SO', coitem_id))) AS netweight,"
               "      (item_packweight * itemuomtouom(item_id, coitem_price_uom_id, NULL, qtyAtShipping('SO', coitem_id))) AS tareweight,"
               "      qtyAtShipping('SO', coitem_id) AS qtyatshipping,"
               "      formatQty(qtyAtShipping('SO', coitem_id)) AS f_qtyatshipping "
               "FROM coitem, itemsite, item "
               "WHERE ((coitem_itemsite_id=itemsite_id)"
               "  AND  (itemsite_item_id=item_id)"
               "  AND  (coitem_cohead_id=:cohead_id) ) "
               "ORDER BY coitem_linenumber;" );
    q.bindValue(":cohead_id", _salesOrder->id());
  }
  else if (_to->isValid())
  {
    q.prepare( "SELECT toitem_id AS itemid, toitem_tohead_id AS headid,"
	       "       toitem_linenumber AS linenumber, item_number,"
               "      (item_prodweight * qtyAtShipping('TO', toitem_id))) AS netweight,"
               "      (item_packweight * qtyAtShipping('TO', toitem_id))) AS tareweight,"
               "      qtyAtShipping('TO', toitem_id) AS qtyatshipping,"
               "      formatQty(qtyAtShipping('TO', toitem_id)) AS f_qtyatshipping "
               "FROM toitem, item "
               "WHERE ((toitem_item_id=item_id)"
               "  AND  (toitem_tohead_id=:tohead_id) ) "
               "ORDER BY toitem_linenumber;" ) ;
    q.bindValue(":tohead_id", _to->id());
  } 
  q.exec();

  double        totalNetWeight   = 0;
  double        totalTareWeight  = 0;

  XTreeWidgetItem* last = 0;

  while (q.next())
  {
    totalNetWeight   += q.value("netweight").toDouble();
    totalTareWeight  += q.value("tareweight").toDouble();

    last = new XTreeWidgetItem( _item, last, q.value("itemid").toInt(),
		       q.value("headid").toInt(),
		       q.value("linenumber"), q.value("item_number"),
		       q.value("f_qtyatshipping"),
		       formatWeight(q.value("netweight").toDouble()),
		       formatWeight(q.value("tareweight").toDouble()),
		       formatWeight((q.value("netweight").toDouble() +
				     q.value("tareweight").toDouble())) );
  }
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _totalNetWeight->setText(formatWeight(totalNetWeight));
  _totalTareWeight->setText(formatWeight(totalTareWeight));
  _totalGrossWeight->setText(formatWeight(totalNetWeight + totalTareWeight));
}
