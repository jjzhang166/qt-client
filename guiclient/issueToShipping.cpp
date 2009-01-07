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


#include "issueToShipping.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "xmessagebox.h"
#include "inputManager.h"
#include "distributeInventory.h"
#include "issueLineToShipping.h"
#include "shipOrder.h"
#include "storedProcErrorLookup.h"

issueToShipping::issueToShipping(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_ship, SIGNAL(clicked()), this, SLOT(sShip()));
  connect(_issueLine, SIGNAL(clicked()), this, SLOT(sIssueLineBalance()));
  connect(_issueAll, SIGNAL(clicked()), this, SLOT(sIssueAllBalance()));
  connect(_issueStock, SIGNAL(clicked()), this, SLOT(sIssueStock()));
  connect(_order,       SIGNAL(valid(bool)), this, SLOT(sFillList()));
  connect(_returnStock, SIGNAL(clicked()), this, SLOT(sReturnStock()));
  connect(_bcFind, SIGNAL(clicked()), this, SLOT(sBcFind()));

  _order->setAllowedStatuses(OrderLineEdit::Open);
  _order->setAllowedTypes(OrderLineEdit::Sales |
                          OrderLineEdit::Transfer);

  _ship->setEnabled(_privileges->check("ShipOrders"));

  omfgThis->inputManager()->notify(cBCItem, this, this, SLOT(sCatchItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, this, SLOT(sCatchItemsiteid(int)));
  omfgThis->inputManager()->notify(cBCSalesOrder, this, this, SLOT(sCatchSoheadid(int)));
  omfgThis->inputManager()->notify(cBCSalesOrderLineItem, this, this, SLOT(sCatchSoitemid(int)));
  omfgThis->inputManager()->notify(cBCTransferOrder, this, this, SLOT(sCatchToheadid(int)));
  omfgThis->inputManager()->notify(cBCTransferOrderLineItem, this, this, SLOT(sCatchToitemid(int)));
  omfgThis->inputManager()->notify(cBCWorkOrder, this, this, SLOT(sCatchWoid(int)));

  _soitem->addColumn(tr("#"),           _seqColumn,   Qt::AlignCenter, true, "linenumber");
  _soitem->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true, "item_number");
  _soitem->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true, "itemdescrip");
  _soitem->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true, "warehous_code");
  _soitem->addColumn(tr("Sched. Date"), _qtyColumn, Qt::AlignRight,  true, "scheddate");
  _soitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignLeft,   true, "uom_name");
  _soitem->addColumn(tr("Ordered"),     _qtyColumn,   Qt::AlignRight,  true, "qtyord");
  _soitem->addColumn(tr("Shipped"),     _qtyColumn,   Qt::AlignRight,  true, "qtyshipped");
  _soitem->addColumn(tr("Returned"),    _qtyColumn,   Qt::AlignRight,  false, "qtyreturned");
  _soitem->addColumn(tr("Balance"),     _qtyColumn,   Qt::AlignRight,  false, "balance");
  _soitem->addColumn(tr("At Shipping"), _qtyColumn,   Qt::AlignRight,  true, "atshipping");
  _soitem->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _order->setFocus();
  _order->setFromSitePrivsEnforced(TRUE);

  _bcQty->setValidator(omfgThis->qtyVal());

  if(_metrics->boolean("EnableSOReservations"))
  {
    _requireInventory->setChecked(true);
    _requireInventory->setEnabled(false);
  }

  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate());
  
}

issueToShipping::~issueToShipping()
{
  // no need to delete child widgets, Qt does it all for us
}

void issueToShipping::languageChange()
{
  retranslateUi(this);
}

enum SetResponse issueToShipping::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _order->setId(param.toInt(), "SO");
    _soitem->setFocus();
  }

  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    _order->setId(param.toInt(), "TO");
    _soitem->setFocus();
  }

  return NoError;
}

void issueToShipping::sCatchSoheadid(int pSoheadid)
{
  _order->setId(pSoheadid, "SO");
  _soitem->selectAll();
}

void issueToShipping::sCatchSoitemid(int pSoitemid)
{
  q.prepare( "SELECT coitem_cohead_id "
             "FROM coitem "
             "WHERE (coitem_id=:sohead_id);" );
  q.bindValue(":sohead_id", pSoitemid);
  q.exec();
  if (q.first())
  {
    _order->setId(q.value("coitem_cohead_id").toInt(), "SO");
    _soitem->clearSelection();
    _soitem->setId(pSoitemid);
    sIssueStock();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void issueToShipping::sCatchToheadid(int pToheadid)
{
  _order->setId(pToheadid, "TO");
  _soitem->selectAll();
}

void issueToShipping::sCatchToitemid(int porderitemid)
{
  q.prepare( "SELECT toitem_tohead_id "
             "FROM toitem "
             "WHERE (toitem_id=:tohead_id);" );
  q.bindValue(":tohead_id", porderitemid);
  q.exec();
  if (q.first())
  {
    _order->setId(q.value("toitem_tohead_id").toInt(), "TO");
    _soitem->clearSelection();
    _soitem->setId(porderitemid);
    sIssueStock();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void issueToShipping::sCatchItemsiteid(int pItemsiteid)
{
  q.prepare("SELECT orderitem_id "
            "FROM orderitem "
            "WHERE ((orderitem_itemsite_id=:itemsite) "
            "   AND (orderitem_orderhead_type=:ordertype) "
            "   AND (orderitem_orderhead_id=:orderid));");
  q.bindValue(":itemsite",  pItemsiteid);
  q.bindValue(":ordertype", _order->type());
  q.bindValue(":orderid",   _order->id());
  q.exec();
  if (q.first())
  {
    _soitem->clearSelection();
    _soitem->setId(q.value("orderitem_id").toInt());
    sIssueStock();
  }
  else
    audioReject();
}

void issueToShipping::sCatchItemid(int pItemid)
{
  q.prepare( "SELECT orderitem_id "
             "FROM orderitem, itemsite "
             "WHERE ((orderitem_itemsite_id=itemsite_id)"
             "  AND  (itemsite_item_id=:item_id)"
            "   AND  (orderitem_orderhead_type=:ordertype) "
            "   AND  (orderitem_orderhead_id=:orderid));");
  q.bindValue(":item_id",   pItemid);
  q.bindValue(":ordertype", _order->type());
  q.bindValue(":orderid",   _order->id());
  q.exec();
  if (q.first())
  {
    _soitem->clearSelection();
    _soitem->setId(q.value("orderitem_id").toInt());
    sIssueStock();
  }
  else
    audioReject();
}

void issueToShipping::sCatchWoid(int pWoid)
{
  if (_order->isSO())
  {
    q.prepare( " SELECT coitem_cohead_id, coitem_id"
              "  FROM coitem"
              " WHERE ((coitem_order_id=:wo_id)"
              "   AND  (coitem_cohead_id=:sohead_id));" );
    q.bindValue(":wo_id",     pWoid);
    q.bindValue(":sohead_id", _order->id());
    q.exec();
    if (q.first())
    {
      _order->setId(q.value("coitem_cohead_id").toInt());
      _soitem->clearSelection();
      _soitem->setId(q.value("coitem_id").toInt());
      sIssueStock();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    audioReject();
}

void issueToShipping::sIssueStock()
{
  bool update  = FALSE;
  QList<QTreeWidgetItem*> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("order_id",   ((XTreeWidgetItem*)selected[i])->id());
    params.append("order_type", _order->type());
    params.append("transTS",    _transDate->date());

    if(_requireInventory->isChecked())
      params.append("requireInventory");
    
    issueLineToShipping newdlg(this, "", TRUE);
    if (newdlg.set(params) == NoError && newdlg.exec() != XDialog::Rejected)
      update = TRUE;
  }

  if (update)
    sFillList();
}

bool issueToShipping::sufficientItemInventory(int porderitemid)
{
  if(_requireInventory->isChecked() ||
     (_order->isSO() && _metrics->boolean("EnableSOReservations")))
  {
    q.prepare("SELECT sufficientInventoryToShipItem(:ordertype, :itemid) AS result;");
    q.bindValue(":itemid", porderitemid);
    q.bindValue(":ordertype", _order->type());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	ParameterList errp;
	if (_order->isSO())
	  errp.append("soitem_id", porderitemid);
	else if (_order->isTO())
	  errp.append("toitem_id", porderitemid);

	QString errs = "<? if exists(\"soitem_id\") ?>"
		  "SELECT item_number, warehous_code "
		  "  FROM coitem, item, itemsite, whsinfo "
		  " WHERE ((coitem_itemsite_id=itemsite_id)"
		  "   AND  (itemsite_item_id=item_id)"
		  "   AND  (itemsite_warehous_id=warehous_id)"
		  "   AND  (coitem_id=<? value(\"soitem_id\") ?>));"
		  "<? elseif exists(\"toitem_id\")?>"
		  "SELECT item_number, tohead_srcname AS warehous_code "
		  "  FROM toitem, tohead, item "
		  " WHERE ((toitem_item_id=item_id)"
		  "   AND  (toitem_tohead_id=tohead_id)"
		  "   AND  (toitem_id=<? value(\"toitem_id\") ?>));"
		  "<? endif ?>" ;
	MetaSQLQuery errm(errs);
	q = errm.toQuery(errp);
	if (! q.first() && q.lastError().type() != QSqlError::NoError)
	    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	systemError(this,
		    storedProcErrorLookup("sufficientInventoryToShipItem",
					  result)
		    .arg(q.value("item_number").toString())
		    .arg(q.value("warehous_code").toString()), __FILE__, __LINE__);
	return false;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  return true;
}

bool issueToShipping::sufficientInventory(int porderheadid)
{
  if (_requireInventory->isChecked() ||
      (_order->isSO() && _metrics->boolean("EnableSOReservations")))
  {
    q.prepare("SELECT sufficientInventoryToShipOrder(:ordertype, :orderid) AS result;");
    q.bindValue(":orderid", porderheadid);
    q.bindValue(":ordertype", _order->type());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("sufficientInventoryToShipOrder", result), __FILE__, __LINE__);
        return false;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  return true;
}

void issueToShipping::sIssueLineBalance()
{
  QList<QTreeWidgetItem*> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
    if (! sufficientItemInventory(cursor->id()))
      return;
    
    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");
      
    q.exec("BEGIN;");
    q.prepare("SELECT issueLineBalanceToShipping(:ordertype, :soitem_id, :ts) AS result;");
    q.bindValue(":ordertype", _order->type());
    q.bindValue(":soitem_id", cursor->id());
    q.bindValue(":ts",        _transDate->date());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("issueLineBalanceToShipping", result),
              __FILE__, __LINE__);
        return;
      }
      else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Issue to Shipping"), tr("Issue Canceled") );
        return;
      }
      q.exec("COMMIT;");
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillList();
}

void issueToShipping::sIssueAllBalance()
{
  int orderid = _order->id();

  if (! sufficientInventory(orderid))
    return;

  for (int i = 0; i < _soitem->topLevelItemCount(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)_soitem->topLevelItem(i);
    if (! sufficientItemInventory(cursor->id()))
      return;

    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    q.exec("BEGIN;");
    q.prepare("SELECT issueLineBalanceToShipping(:ordertype, :soitem_id, :ts) AS result;");
    q.bindValue(":ordertype", _order->type());
    q.bindValue(":soitem_id", cursor->id());
    q.bindValue(":ts",        _transDate->date());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("issueLineBalanceToShipping", result),
              __FILE__, __LINE__);
        return;
      }
      else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Issue to Shipping"), tr("Issue Canceled") );
        return;
      }
      q.exec("COMMIT;");
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillList();
}

void issueToShipping::sReturnStock()
{
  QList<QTreeWidgetItem*> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
    
    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    q.exec("BEGIN");
    q.prepare("SELECT returnItemShipments(:ordertype, :soitem_id, 0, :ts) AS result;");
    q.bindValue(":ordertype", _order->type());
    q.bindValue(":soitem_id", cursor->id());
    q.bindValue(":ts",        _transDate->date());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        rollback.exec();
        systemError( this, storedProcErrorLookup("returnItemShipments", result),
              __FILE__, __LINE__);
        return;
      }
      else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Issue to Shipping"), tr("Return Canceled") );
        return;
      }      
      q.exec("COMMIT;"); 
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillList();
}

void issueToShipping::sShip()
{
  q.prepare( "SELECT shiphead_id "
             "FROM shiphead JOIN shipitem ON (shipitem_shiphead_id=shiphead_id) "
             "WHERE ((NOT shiphead_shipped)"
             "  AND  (shiphead_order_type=:ordertype)"
             "  AND  (shiphead_order_id=:order_id) ) "
             "LIMIT 1;" );
  q.bindValue(":order_id",  _order->id());
  q.bindValue(":ordertype", _order->type());

  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("shiphead_id", q.value("shiphead_id").toInt());

    shipOrder newdlg(this, "", TRUE);
    if (newdlg.set(params) == NoError && newdlg.exec() != XDialog::Rejected)
    {
      _transDate->setDate(omfgThis->dbDate());
      _order->setId(-1);
      _order->setFocus();
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    QMessageBox::information( this, tr("Cannot Ship Order"),
                              tr("<p>You must issue some amount of Stock to "
				 "this Order before you may ship it.") );
}

void issueToShipping::sFillList()
{
  ParameterList listp;
  if (_order->id() < 0)
  {
    _soitem->clear();
    _order->setFocus();
    return;
  }
  else if (_order->isSO())
  {
    q.prepare( "SELECT cohead_holdtype "
               "FROM cohead "
               "WHERE (cohead_id=:sohead_id);" );
    q.bindValue(":sohead_id", _order->id());
    q.exec();
    if (q.first())
    {
      if (q.value("cohead_holdtype").toString() == "C")
      {
        QMessageBox::critical( this, tr("Cannot Issue Stock"),
                               storedProcErrorLookup("issuetoshipping", -12));
        _order->setId(-1);
        _order->setFocus();
      }
      else if (q.value("cohead_holdtype").toString() == "P")
      {
        QMessageBox::critical( this, tr("Cannot Issue Stock"),
                               storedProcErrorLookup("issuetoshipping", -13));
        _order->setId(-1);
        _order->setFocus();
      }
      else if (q.value("cohead_holdtype").toString() == "R")
      {
        QMessageBox::critical( this, tr("Cannot Issue Stock"),
                               storedProcErrorLookup("issuetoshipping", -14));
        _order->setId(-1);
        _order->setFocus();
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      _order->setId(-1);
      _order->setFocus();
      return;
    }

    listp.append("sohead_id", _order->id());
  }
  else if (_order->isTO())
  {
    listp.append("tohead_id", _order->id());
  }
  else
  {
    systemError(this, tr("Unrecognized order type %1").arg(_order->type()),
                __FILE__, __LINE__);
    return;
  }

  listp.append("ordertype", _order->type());

  // TODO: add qty_returned to orderitem and all of this can become an orderitem select!
  QString sql = "SELECT *, "
             "       noNeg(qtyord - qtyshipped + qtyreturned) AS balance,"
             "       'qty' AS qtyord_xtnumericrole,"
             "       'qty' AS qtyshipped_xtnumericrole,"
             "       'qty' AS qtyreturned_xtnumericrole,"
             "       'qty' AS balance_xtnumericrole,"
             "       'qty' AS atshipping_xtnumericrole,"
             "       CASE WHEN (scheddate > CURRENT_DATE AND"
             "                  noNeg(qtyord - qtyshipped + qtyreturned) <> atshipping) THEN 'future'"
             "            WHEN (noNeg(qtyord - qtyshipped + qtyreturned) <> atshipping) THEN 'expired'"
             "       END AS qtforegroundrole "
             "FROM ("
             "<? if exists(\"sohead_id\") ?>"
	     "SELECT coitem_id AS lineitem_id,"
	     "       formatSoLineNumber(coitem_id) AS linenumber, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
	     "       warehous_code,"
             "       coitem_scheddate AS scheddate,"
             "       uom_name,"
             "       coitem_qtyord AS qtyord,"
             "       coitem_qtyshipped AS qtyshipped,"
             "       coitem_qtyreturned AS qtyreturned,"
             "       COALESCE(SUM(shipitem_qty), 0) AS atshipping "
             "FROM itemsite, item, site(), uom,"
             "     coitem LEFT OUTER JOIN"
             "      ( shipitem JOIN shiphead"
             "        ON ( (shipitem_shiphead_id=shiphead_id) AND (NOT shiphead_shipped) )"
             "      ) ON  (shipitem_orderitem_id=coitem_id) "
             "WHERE ( (coitem_itemsite_id=itemsite_id)"
             " AND (coitem_qty_uom_id=uom_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (itemsite_warehous_id=warehous_id)"
             " AND (coitem_status NOT IN ('C','X'))"
             " AND (item_type != 'K')"
             " AND (coitem_cohead_id=<? value(\"sohead_id\") ?>) ) "
             "GROUP BY coitem_id, linenumber, item_number,"
             "         item_descrip1, item_descrip2, warehous_code,"
             "         coitem_scheddate, uom_name,"
             "         coitem_qtyord, coitem_qtyshipped, coitem_qtyreturned "
	     "<? elseif exists(\"tohead_id\") ?>"
	     "SELECT toitem_id AS lineitem_id,"
	     "       toitem_linenumber AS linenumber, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
	     "       tohead_srcname AS warehous_code,"
             "       toitem_schedshipdate AS scheddate,"
             "       uom_name,"
             "       toitem_qty_ordered AS qtyord,"
             "       toitem_qty_shipped AS qtyshipped,"
             "       0 AS qtyreturned,"
             "       COALESCE(SUM(shipitem_qty), 0) AS atshipping "
             "FROM item, tohead, site(), uom,"
             "     toitem LEFT OUTER JOIN"
             "      ( shipitem JOIN shiphead"
             "        ON ( (shipitem_shiphead_id=shiphead_id) AND (NOT shiphead_shipped) )"
             "      ) ON  (shipitem_orderitem_id=toitem_id) "
             "WHERE ( (toitem_item_id=item_id)"
             " AND (toitem_status NOT IN ('C','X'))"
             " AND (toitem_tohead_id=tohead_id)"
             " AND (tohead_src_warehous_id=warehous_id)"
             " AND (item_inv_uom_id=uom_id)"
             " AND (tohead_id=<? value(\"tohead_id\") ?>) ) "
             "GROUP BY toitem_id, toitem_linenumber, item_number,"
             "         item_descrip1, item_descrip2, tohead_srcname,"
             "         toitem_schedshipdate, uom_name,"
             "         toitem_qty_ordered, toitem_qty_shipped "
	     "<? endif ?>"
             ") AS sub "
             "ORDER BY scheddate, linenumber;"
	     ;
  MetaSQLQuery listm(sql);
  XSqlQuery listq = listm.toQuery(listp);
  _soitem->populate(listq);
  if (listq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, listq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void issueToShipping::sBcFind()
{
  if (_bc->text().isEmpty())
  {
    QMessageBox::warning(this, tr("No Bar Code scanned"),
			 tr("<p>Cannot search for Items by Bar Code without a "
			    "Bar Code."));
    _bc->setFocus();
    return;
  }

  // find item that matches barcode and is a line item in this order.
  // then call issueLineToShipping passing in params to preset and
  // run the issue button.
  // TODO: can we make this an orderitem select?
  QString sql = "<? if exists(\"sohead_id\") ?>"
	    "SELECT coitem_id AS lineitem_id, noNeg(noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) - COALESCE(SUM(shipitem_qty), 0)) AS balance"
            "  FROM itemsite, item,"
            "       coitem LEFT OUTER JOIN"
            "        ( shipitem JOIN shiphead"
            "          ON ( (shipitem_shiphead_id=shiphead_id)"
	    "               AND (NOT shiphead_shipped"
	    "               AND shiphead_order_type=<? value(\"ordertype\") ?>) )"
            "        ) ON  (shipitem_orderitem_id=coitem_id) "
            " WHERE ((coitem_itemsite_id=itemsite_id)"
            "   AND  (itemsite_item_id=item_id)"
            "   AND  (coitem_status NOT IN ('C', 'X'))"
            "   AND  (coitem_cohead_id=<? value(\"sohead_id\") ?>)"
            "   AND  (item_upccode=<? value(\"bc\") ?>))"
            " GROUP BY coitem_id, coitem_qtyord, coitem_qtyshipped, coitem_qtyreturned; "
	    "<? elseif exists(\"tohead_id\") ?>"
	    "SELECT toitem_id AS lineitem_id, noNeg(noNeg(toitem_qty_ordered - toitem_qty_shipped) - COALESCE(SUM(shipitem_qty), 0)) AS balance"
            "  FROM item,"
            "       toitem LEFT OUTER JOIN"
            "        ( shipitem JOIN shiphead"
            "          ON ( (shipitem_shiphead_id=shiphead_id)"
	    "               AND (NOT shiphead_shipped"
	    "               AND shiphead_order_type=<? value(\"ordertype\") ?>) )"
            "        ) ON  (shipitem_orderitem_id=toitem_id) "
            " WHERE ((toitem_item_id=item_id)"
            "   AND  (toitem_status NOT IN ('C', 'X'))"
            "   AND  (toitem_tohead_id=<? value(\"tohead_id\") ?>)"
            "   AND  (item_upccode=<? value(\"bc\") ?>))"
            " GROUP BY toitem_id, toitem_qty_ordered, toitem_qty_shipped; "
	    "<? endif ?>"
	    ;

  ParameterList findp;
  if (_order->isSO())
    findp.append("sohead_id", _order->id());
  else if (_order->isTO())
    findp.append("tohead_id", _order->id());
  findp.append("ordertype",   _order->type());
  findp.append("bc",          _bc->text());

  MetaSQLQuery findm(sql);
  q = findm.toQuery(findp);

  if(!q.first())
  {
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    XMessageBox::message(this, QMessageBox::Warning, tr("No Match Found"),
      tr("<p>No Items on this Sales Order match the specified Barcode.") ); 
    _bc->clear();
    return;
  }

  int coitemid = q.value("lineitem_id").toInt();
  while(q.value("balance").toDouble() == 0 && q.next())
    coitemid = q.value("lineitem_id").toInt();

  ParameterList params;
  if (_order->isSO())
    params.append("soitem_id", coitemid);
  else if (_order->isTO())
    params.append("toitem_id", coitemid);

  if (_requireInventory->isChecked())
    params.append("requireInventory");
  params.append("qty", _bcQty->toDouble());
  params.append("issue");
  params.append("snooze");

  issueLineToShipping newdlg(this, "", TRUE);
  if (newdlg.set(params) != NoError)
    return;
  sFillList();

  _bc->clear();

  // Check to see if the order is fully completed yet.
  // If so we can pop a quick message informing the user.
  // TODO: can we make this an orderitem select?
  sql = "<? if exists(\"sohead_id\") ?>"
	"SELECT coitem_id,"
	"       noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) - COALESCE(SUM(shipitem_qty), 0) AS remaining"
	"  FROM coitem LEFT OUTER JOIN"
	"         ( shipitem JOIN shiphead"
	"           ON ( (shipitem_shiphead_id=shiphead_id)"
	"                 AND (NOT shiphead_shipped)"
	"                 AND (shiphead_order_type=<? value(\"ordertype\") ?>))"
	"         ) ON  (shipitem_orderitem_id=coitem_id)"
	" WHERE ( (coitem_status NOT IN ('C','X'))"
	"   AND   (coitem_cohead_id=<? value(\"sohead_id\") ?>) )"
	" GROUP BY coitem_id, coitem_qtyord, coitem_qtyshipped, coitem_qtyreturned "
	"HAVING ((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) - COALESCE(SUM(shipitem_qty),0)) > 0);"
	"<? elseif exists(\"tohead_id\") ?>"
	"SELECT toitem_id,"
	"       noNeg(toitem_qty_ordered - toitem_qty_shipped) - COALESCE(SUM(shipitem_qty), 0) AS remaining"
	"  FROM toitem LEFT OUTER JOIN"
	"         ( shipitem JOIN shiphead"
	"           ON ( (shipitem_shiphead_id=shiphead_id)"
	"                 AND (NOT shiphead_shipped)"
	"                 AND (shiphead_order_type=<? value(\"ordertype\") ?>))"
	"         ) ON  (shipitem_orderitem_id=toitem_id)"
	" WHERE ( (toitem_status NOT IN ('C','X'))"
	"   AND   (toitem_tohead_id=<? value(\"tohead_id\") ?>) )"
	" GROUP BY toitem_id, toitem_qty_ordered, toitem_qty_shipped "
	"HAVING ((noNeg(toitem_qty_ordered - toitem_qty_shipped) - COALESCE(SUM(shipitem_qty),0)) > 0);"
	"<? endif ?>"
	;

  ParameterList fullp;
  if (_order->isSO())
    fullp.append("sohead_id", _order->id());
  else if (_order->isTO())
    fullp.append("tohead_id", _order->id());
  fullp.append("ordertype",   _order->type());
  fullp.append("bc",          _bc->text());

  MetaSQLQuery fullm(sql);
  q = fullm.toQuery(fullp);

  // If there are no records then the order is complete
  if(!q.first())
  {
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    XMessageBox::message( this, QMessageBox::Information, tr("Order Complete"),
      tr("All items for this order have been issued to shipping.") );
  }
}
