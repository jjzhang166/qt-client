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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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
#include <QStatusBar>
#include <QVariant>

#include <metasql.h>

#include "xmessagebox.h"
#include "inputManager.h"
#include "distributeInventory.h"
#include "issueLineToShipping.h"
#include "shipOrder.h"
#include "storedProcErrorLookup.h"

issueToShipping::issueToShipping(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    connect(_so, SIGNAL(newId(int)), this, SLOT(sHandleSalesOrder(int)));
    connect(_to, SIGNAL(newId(int)), this, SLOT(sHandleTransferOrder(int)));
    connect(_ship, SIGNAL(clicked()), this, SLOT(sShip()));
    connect(_issueLine, SIGNAL(clicked()), this, SLOT(sIssueLineBalance()));
    connect(_issueAll, SIGNAL(clicked()), this, SLOT(sIssueAllBalance()));
    connect(_issueStock, SIGNAL(clicked()), this, SLOT(sIssueStock()));
    connect(_returnStock, SIGNAL(clicked()), this, SLOT(sReturnStock()));
    connect(_bcFind, SIGNAL(clicked()), this, SLOT(sBcFind()));

    _so->setType((cSoOpen && cSoReleased));

    _ship->setEnabled(_privleges->check("ShipOrders"));

    omfgThis->inputManager()->notify(cBCItem, this, this, SLOT(sCatchItemid(int)));
    omfgThis->inputManager()->notify(cBCItemSite, this, this, SLOT(sCatchItemsiteid(int)));
    omfgThis->inputManager()->notify(cBCSalesOrder, this, this, SLOT(sCatchSoheadid(int)));
    omfgThis->inputManager()->notify(cBCSalesOrderLineItem, this, this, SLOT(sCatchSoitemid(int)));
    omfgThis->inputManager()->notify(cBCTransferOrder, this, this, SLOT(sCatchToheadid(int)));
    omfgThis->inputManager()->notify(cBCTransferOrderLineItem, this, this, SLOT(sCatchToitemid(int)));
    omfgThis->inputManager()->notify(cBCWorkOrder, this, this, SLOT(sCatchWoid(int)));

    _soitem->addColumn(tr("#"),           _seqColumn,   Qt::AlignCenter );
    _soitem->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
    _soitem->addColumn(tr("Description"),  -1,          Qt::AlignLeft   );
    _soitem->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter );
    _soitem->addColumn(tr("Scheduled Date"),     _qtyColumn,   Qt::AlignRight  );
    _soitem->addColumn(tr("Ordered"),     _qtyColumn,   Qt::AlignRight  );
    _soitem->addColumn(tr("Shipped"),     _qtyColumn,   Qt::AlignRight  );
    _soitem->addColumn(tr("Returned"),    _qtyColumn,   Qt::AlignRight  );
    _soitem->addColumn(tr("Balance"),     _qtyColumn,   Qt::AlignRight  );
    _soitem->addColumn(tr("At Shipping"), _qtyColumn,   Qt::AlignRight  );
    _soitem->setSelectionMode(Q3ListView::Extended);

    _so->setFocus();

    _bcQty->setValidator(omfgThis->qtyVal());

    _to->setVisible(_metrics->boolean("MultiWhs"));
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
    q.prepare( "SELECT coitem_cohead_id "
               "FROM coitem "
               "WHERE (coitem_id=:sohead_id);" );
    q.bindValue(":sohead_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _so->setId(q.value("coitem_cohead_id").toInt());
      _soitem->setFocus();
      _ordertype = "SO";
      _so->setEnabled(true);
      _to->setEnabled(false);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    _to->setId(param.toInt());
    _soitem->setFocus();
    _ordertype = "TO";
    _so->setEnabled(false);
    _to->setEnabled(true);
  }

  return NoError;
}

void issueToShipping::sHandleSalesOrder(const int pSoheadid)
{
  if (pSoheadid != -1)
  {
    q.prepare( "SELECT cohead_holdtype "
               "FROM cohead "
               "WHERE (cohead_id=:sohead_id);" );
    q.bindValue(":sohead_id", pSoheadid);
    q.exec();
    if (q.first())
    {
      _ordertype = "SO";
      if (q.value("cohead_holdtype").toString() == "P")
      {
        QMessageBox::critical( this, tr("Cannot Issue Stock"),
                               tr("<p>The selected Sales Order is on Packing Hold and must be taken off of Packing Hold before any inventory may be issued to it.") );
        _so->setId(-1);
        _so->setFocus();
      }
      else if (q.value("cohead_holdtype").toString() == "C")
      {
        QMessageBox::critical( this, tr("Cannot Issue Stock"),
                               tr("<p>The selected Sales Order is on Credit Hold and must be taken off of Credit Hold before any inventory may be issued to it.") );
        _so->setId(-1);
        _so->setFocus();
      }
      else
      {
        _to->setId(-1);
        sFillList();
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _soitem->clear();
}

void issueToShipping::sHandleTransferOrder(const int porderid)
{
  if (porderid != -1)
  {
    _ordertype = "TO";
    _so->setId(-1);
    sFillList();
  }
  else
    _soitem->clear();
}

void issueToShipping::sCatchSoheadid(int pSoheadid)
{
  _so->setId(pSoheadid);
  for (XListViewItem *cursor = _soitem->firstChild(); cursor != 0; cursor = cursor->nextSibling())
    _soitem->setSelected(cursor, TRUE);
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
    _so->setId(q.value("coitem_cohead_id").toInt());
    _soitem->clearSelection();
    _soitem->setId(pSoitemid);
    sIssueStock();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void issueToShipping::sCatchToheadid(int pToheadid)
{
  _to->setId(pToheadid);
  for (XListViewItem *cursor = _soitem->firstChild(); cursor != 0; cursor = cursor->nextSibling())
    _soitem->setSelected(cursor, TRUE);
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
    _to->setId(q.value("toitem_tohead_id").toInt());
    _soitem->clearSelection();
    _soitem->setId(porderitemid);
    sIssueStock();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void issueToShipping::sCatchItemsiteid(int pItemsiteid)
{
  if (_ordertype == "SO")
  {
    q.prepare( "SELECT coitem_id "
	       "FROM coitem "
	       "WHERE ((coitem_itemsite_id=:itemsite_id)"
	       "  AND  (coitem_cohead_id=:sohead_id) );" );
    q.bindValue(":itemsite_id", pItemsiteid);
    q.bindValue(":sohead_id", _so->id());
    q.exec();
    if (q.first())
    {
      _soitem->clearSelection();
      _soitem->setId(q.value("coitem_id").toInt());
      sIssueStock();
    }
    else
      audioReject();
  }
  else if (_ordertype == "TO")
  {
    q.prepare( "SELECT toitem_id "
	       "FROM toitem, tohead, itemsite "
	       "WHERE ((toitem_item_id=itemsite_item_id)"
	       "  AND  (toitem_tohead_id=tohead_id)"
	       "  AND  (tohead_src_warehous_id=itemsite_warehous_id)"
	       "  AND  (itemsite_id=:itemsite_id)"
	       "  AND  (toitem_tohead_id=:tohead_id) );" );
    q.bindValue(":itemsite_id", pItemsiteid);
    q.bindValue(":tohead_id", _to->id());
    q.exec();
    if (q.first())
    {
      _soitem->clearSelection();
      _soitem->setId(q.value("toitem_id").toInt());
      sIssueStock();
    }
    else
      audioReject();
  }
  else
    audioReject();

}

void issueToShipping::sCatchItemid(int pItemid)
{
  if (_ordertype == "SO")
  {
    q.prepare( "SELECT coitem_id "
	       "FROM coitem, itemsite "
	       "WHERE ( (coitem_itemsite_id=itemsite_id)"
	       "  AND   (itemsite_item_id=:item_id)"
	       "  AND   (coitem_cohead_id=:sohead_id) );" );
    q.bindValue(":item_id", pItemid);
    q.bindValue(":sohead_id", _so->id());
    q.exec();
    if (q.first())
    {
      _soitem->clearSelection();
      _soitem->setId(q.value("coitem_id").toInt());
      sIssueStock();
    }
    else
      audioReject();
  }
  else if (_ordertype == "TO")
  {
    q.prepare( "SELECT toitem_id "
	       "FROM toitem "
	       "WHERE ( (toitem_item_id=:item_id)"
	       "  AND   (toitem_tohead_id=:tohead_id) );" );
    q.bindValue(":item_id", pItemid);
    q.bindValue(":tohead_id", _to->id());
    q.exec();
    if (q.first())
    {
      _soitem->clearSelection();
      _soitem->setId(q.value("toitem_id").toInt());
      sIssueStock();
    }
    else
      audioReject();
  }
  else
    audioReject();
}

void issueToShipping::sCatchWoid(int pWoid)
{
  q.prepare( " SELECT coitem_cohead_id, coitem_id"
            "  FROM coitem"
            " WHERE ((coitem_order_id=:wo_id)"
            "   AND  (coitem_cohead_id=:sohead_id));" );
  q.bindValue(":wo_id", pWoid);
  q.bindValue(":sohead_id", _so->id());
  q.exec();
  if (q.first())
  {
    _so->setId(q.value("coitem_cohead_id").toInt());
    _soitem->clearSelection();
    _soitem->setId(q.value("coitem_id").toInt());
    sIssueStock();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void issueToShipping::sIssueStock()
{
  bool update  = FALSE;
  XListViewItem *cursor = _soitem->firstChild();
  while (cursor != 0)
  {
    if (_soitem->isSelected(cursor))
    {
      ParameterList params;
      if (_ordertype == "SO")
	params.append("soitem_id", cursor->id());
      else if (_ordertype == "TO")
	params.append("toitem_id", cursor->id());

      if(_requireInventory->isChecked())
        params.append("requireInventory");
      
      issueLineToShipping newdlg(this, "", TRUE);
      if (newdlg.set(params) == NoError && newdlg.exec() != QDialog::Rejected)
        update = TRUE;
    }

    cursor = cursor->nextSibling();
  }

  if (update)
    sFillList();
}

bool issueToShipping::sufficientItemInventory(int porderitemid)
{
  if(_requireInventory->isChecked())
  {
    q.prepare("SELECT sufficientInventoryToShipItem(:ordertype, :itemid) AS result;");
    q.bindValue(":itemid", porderitemid);
    q.bindValue(":ordertype", _ordertype);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	ParameterList errp;
	if (_ordertype == "SO")
	  errp.append("soitem_id", porderitemid);
	else if (_ordertype == "TO")
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
	if (! q.first() && q.lastError().type() != QSqlError::None)
	    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	systemError(this,
		    storedProcErrorLookup("sufficientInventoryToShipItem",
					  result)
		    .arg(q.value("item_number").toString())
		    .arg(q.value("warehous_code").toString()), __FILE__, __LINE__);
	return false;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  return true;
}

bool issueToShipping::sufficientInventory(int porderheadid)
{
  if(_requireInventory->isChecked())
  {
    q.prepare("SELECT sufficientInventoryToShipOrder(:ordertype, :orderid) AS result;");
    q.bindValue(":orderid", porderheadid);
    q.bindValue(":ordertype", _ordertype);
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
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  return true;
}

void issueToShipping::sIssueLineBalance()
{
  XListViewItem *cursor = _soitem->firstChild();
  while (cursor != 0)
  {
    if (_soitem->isSelected(cursor))
    {

      if (! sufficientItemInventory(cursor->id()))
	return;

      q.prepare("SELECT issueLineBalanceToShipping(:ordertype, :soitem_id, CURRENT_TIMESTAMP) AS result;");
      q.bindValue(":ordertype", _ordertype);
      q.bindValue(":soitem_id", cursor->id());
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("issueLineBalanceToShipping", result),
		      __FILE__, __LINE__);
          return;
        }
        else
          distributeInventory::SeriesAdjust(q.value("result").toInt(), this);
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    cursor = cursor->nextSibling();
  }

  sFillList();
}

void issueToShipping::sIssueAllBalance()
{
  int orderid = -1;
  if (_ordertype == "SO")
    orderid = _so->id();
  else if (_ordertype == "TO")
    orderid = _to->id();

  if (! sufficientInventory(orderid))
    return;

  q.prepare("SELECT issueAllBalanceToShipping(:ordertype, :order_id, 0, CURRENT_TIMESTAMP) AS result;");
  q.bindValue(":ordertype", _ordertype);
  q.bindValue(":order_id", orderid);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("issueAllBalanceToShipping", result),
                  __FILE__, __LINE__);
      return;
    }
    else
      distributeInventory::SeriesAdjust(q.value("result").toInt(), this);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void issueToShipping::sReturnStock()
{
  XListViewItem *cursor = _soitem->firstChild();
  while (cursor != 0)
  {
    if (_soitem->isSelected(cursor))
    {
      q.prepare("SELECT returnItemShipments(:ordertype, :soitem_id, 0, CURRENT_TIMESTAMP) AS result;");
      q.bindValue(":ordertype", _ordertype);
      q.bindValue(":soitem_id", cursor->id());
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          systemError( this, storedProcErrorLookup("returnItemShipments", result),
		      __FILE__, __LINE__);
          return;
        }
        else
          distributeInventory::SeriesAdjust(q.value("result").toInt(), this);
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    cursor = cursor->nextSibling();
  }

  sFillList();
}

void issueToShipping::sShip()
{
  if (_ordertype == "SO")
  {
    q.prepare( "SELECT cohead_holdtype "
	       "FROM cohead "
	       "WHERE (cohead_id=:sohead_id);" );
    q.bindValue(":sohead_id", _so->id());
    q.exec();
    if (q.first())
    {
      if ( (q.value("cohead_holdtype").toString() == "P") ||
	   (q.value("cohead_holdtype").toString() == "C") ||
	   (q.value("cohead_holdtype").toString() == "S") )
      {
	QMessageBox::warning(this, tr("Cannot Ship Order"),
			   tr("<p>The selected Sales Order cannot be shipped "
			      "as it has been placed on Shipping Hold. It must "
			      "be released from Shipping Hold before it may be "
			      "shipped." ) );
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  q.prepare( "SELECT shiphead_id "
             "FROM shiphead "
             "WHERE ((NOT shiphead_shipped)"
	     "  AND  (shiphead_order_type=:ordertype)"
             "  AND  (shiphead_order_id=:order_id) );" );
  q.bindValue(":ordertype", _ordertype);
  if (_ordertype == "SO")
    q.bindValue(":order_id", _so->id());
  else if (_ordertype == "TO")
    q.bindValue(":order_id", _to->id());

  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("shiphead_id", q.value("shiphead_id").toInt());

    shipOrder newdlg(this, "", TRUE);
    if (newdlg.set(params) == NoError && newdlg.exec() != QDialog::Rejected)
    {
      _so->setId(-1);
      _so->setFocus();
      _so->setEnabled(true);
      _to->setId(-1);
      _to->setEnabled(true);
    }
  }
  else if (q.lastError().type() != QSqlError::None)
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
  _soitem->clear();

  ParameterList listp;
  if (_ordertype == "SO")
    listp.append("sohead_id", _so->id());
  else if (_ordertype == "TO")
    listp.append("tohead_id", _to->id());
  listp.append("ordertype", _ordertype);

  QString sql = "<? if exists(\"sohead_id\") ?>"
	     "SELECT coitem_id AS lineitem_id,"
	     "       coitem_linenumber AS linenumber, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
	     "       warehous_code,"
             "       formatDate(coitem_scheddate) AS f_scheddate,"
             "       formatQty(coitem_qtyord) AS f_qtyord,"
             "       formatQty(coitem_qtyshipped) AS f_qtyshipped,"
             "       formatQty(coitem_qtyreturned) AS f_qtyreturned,"
             "       formatQty(noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned)) AS f_balance,"
             "       formatQty(COALESCE(SUM(shipitem_qty), 0)) AS f_atshipping,"
             "       (noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) <> COALESCE(SUM(shipitem_qty), 0)) AS tagged, "
             "       CASE WHEN coitem_scheddate > current_date THEN 1 "
             "            ELSE 0 "
             "       END AS in_future "
             "FROM itemsite, item, whsinfo,"
             "     coitem LEFT OUTER JOIN"
             "      ( shipitem JOIN shiphead"
             "        ON ( (shipitem_shiphead_id=shiphead_id) AND (NOT shiphead_shipped) )"
             "      ) ON  (shipitem_orderitem_id=coitem_id) "
             "WHERE ( (coitem_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (itemsite_warehous_id=warehous_id)"
             " AND (coitem_status NOT IN ('C','X'))"
             " AND (coitem_cohead_id=<? value(\"sohead_id\") ?>) ) "
             "GROUP BY coitem_id, coitem_linenumber, item_number,"
             "         item_descrip1, item_descrip2, warehous_code,"
             "         coitem_scheddate, "
             "         coitem_qtyord, coitem_qtyshipped, coitem_qtyreturned "
             "ORDER BY coitem_scheddate, linenumber;"
	     "<? elseif exists(\"tohead_id\") ?>"
	     "SELECT toitem_id AS lineitem_id,"
	     "       toitem_linenumber AS linenumber, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
	     "       tohead_srcname AS warehous_code,"
             "       formatDate(toitem_schedshipdate) AS f_scheddate,"
             "       formatQty(toitem_qty_ordered) AS f_qtyord,"
             "       formatQty(toitem_qty_shipped) AS f_qtyshipped,"
             "       formatQty(0) AS f_qtyreturned,"
             "       formatQty(noNeg(toitem_qty_ordered - toitem_qty_shipped)) AS f_balance,"
             "       formatQty(COALESCE(SUM(shipitem_qty), 0)) AS f_atshipping,"
             "       (noNeg(toitem_qty_ordered - toitem_qty_shipped) <> COALESCE(SUM(shipitem_qty), 0)) AS tagged, "
             "       CASE WHEN toitem_schedshipdate > CURRENT_DATE THEN 1 "
             "            ELSE 0 "
             "       END AS in_future "
             "FROM item, tohead,"
             "     toitem LEFT OUTER JOIN"
             "      ( shipitem JOIN shiphead"
             "        ON ( (shipitem_shiphead_id=shiphead_id) AND (NOT shiphead_shipped) )"
             "      ) ON  (shipitem_orderitem_id=toitem_id) "
             "WHERE ( (toitem_item_id=item_id)"
             " AND (toitem_status NOT IN ('C','X'))"
	     " AND (toitem_tohead_id=tohead_id)"
             " AND (tohead_id=<? value(\"tohead_id\") ?>) ) "
             "GROUP BY toitem_id, toitem_linenumber, item_number,"
             "         item_descrip1, item_descrip2, tohead_srcname,"
             "         toitem_schedshipdate, "
             "         toitem_qty_ordered, toitem_qty_shipped "
             "ORDER BY toitem_schedshipdate, linenumber;"
	     "<? endif ?>"
	     ;
  MetaSQLQuery listm(sql);
  XSqlQuery listq = listm.toQuery(listp);
  XListViewItem *last = 0;
  while (listq.next())
  {
    last = new XListViewItem(_soitem, last,
			     listq.value("lineitem_id").toInt(),
			     listq.value("linenumber"),
			     listq.value("item_number"),
			     listq.value("itemdescrip"),
			     listq.value("warehous_code"),
			     listq.value("f_scheddate"),
			     listq.value("f_qtyord"),
			     listq.value("f_qtyshipped"),
			     listq.value("f_qtyreturned"),
			     listq.value("f_balance"),
			     listq.value("f_atshipping") );
    if (listq.value("tagged").toBool())
      last->setColor("red");
    if ((listq.value("in_future").toBool()) && (listq.value("tagged").toBool()))
      last->setColor("darkgreen");
  }
  if (listq.lastError().type() != QSqlError::None)
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
  if (_ordertype == "SO")
    findp.append("sohead_id", _so->id());
  else if (_ordertype == "TO")
    findp.append("tohead_id", _to->id());
  findp.append("ordertype", _ordertype);
  findp.append("bc", _bc->text());

  MetaSQLQuery findm(sql);
  q = findm.toQuery(findp);

  if(!q.first())
  {
    if (q.lastError().type() != QSqlError::None)
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
  if (_ordertype == "SO")
    params.append("soitem_id", coitemid);
  else if (_ordertype == "TO")
    params.append("toitem_id", coitemid);

  if (_requireInventory->isChecked())
    params.append("requireInventory");
  params.append("qty", _bcQty->text());
  params.append("issue");
  params.append("snooze");

  issueLineToShipping newdlg(this, "", TRUE);
  if (newdlg.set(params) != NoError)
    return;
  sFillList();

  _bc->clear();

  // Check to see if the order is fully completed yet.
  // If so we can pop a quick message informing the user.
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
  if (_ordertype == "SO")
    fullp.append("sohead_id", _so->id());
  else if (_ordertype == "TO")
    fullp.append("tohead_id", _to->id());
  fullp.append("ordertype", _ordertype);
  fullp.append("bc", _bc->text());

  MetaSQLQuery fullm(sql);
  q = fullm.toQuery(fullp);

  // If there are no records then the order is complete
  if(!q.first())
  {
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    XMessageBox::message( this, QMessageBox::Information, tr("Order Complete"),
      tr("All items for this order have been issued to shipping.") );
  }
}
