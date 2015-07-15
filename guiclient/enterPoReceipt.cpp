/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

// rename to enterReceipt
#include "enterPoReceipt.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <openreports.h>

#include <metasql.h>

#include "xmessagebox.h"
#include "inputManager.h"
#include "distributeInventory.h"
#include "enterPoitemReceipt.h"
#include "getLotInfo.h"
#include "mqlutil.h"
#include "printLabelsByOrder.h"
#include "storedProcErrorLookup.h"

enterPoReceipt::enterPoReceipt(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_all,		SIGNAL(clicked()),	this, SLOT(sReceiveAll()));
  connect(_enter,	SIGNAL(clicked()),	this, SLOT(sEnter()));
  connect(_order,	SIGNAL(valid(bool)),	this, SLOT(sFillList()));
  connect(_post,	SIGNAL(clicked()),	this, SLOT(sPost()));
  connect(_print,	SIGNAL(clicked()),	this, SLOT(sPrint()));
  connect(_save,	SIGNAL(clicked()),	this, SLOT(sSave()));
  connect(_orderitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_printLabel, SIGNAL(clicked()), this, SLOT(sPrintItemLabel()));
  connect(_bcFind, SIGNAL(clicked()), this, SLOT(sBcFind()));
//  connect(_orderitem, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));

  _order->setAllowedStatuses(OrderLineEdit::Open);
  _order->setAllowedTypes(OrderLineEdit::Purchase |
			  OrderLineEdit::Return |
			  OrderLineEdit::Transfer);
  _order->setToSitePrivsEnforced(true);
  _order->setLockSelected(true);

  omfgThis->inputManager()->notify(cBCItem, this, this, SLOT(sCatchItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, this, SLOT(sCatchItemsiteid(int)));
  omfgThis->inputManager()->notify(cBCPurchaseOrder, this, this, SLOT(sCatchPoheadid(int)));
  omfgThis->inputManager()->notify(cBCPurchaseOrderLineItem, this, this, SLOT(sCatchPoitemid(int)));
  omfgThis->inputManager()->notify(cBCTransferOrder, this, this, SLOT(sCatchToheadid(int)));
  omfgThis->inputManager()->notify(cBCTransferOrderLineItem, this, this, SLOT(sCatchToitemid(int)));

  if (_metrics->boolean("EnableDropShipments"))
    _dropShip->setEnabled(false);
  else
    _dropShip->hide();

  if (_metrics->boolean("EnableReturnAuth"))
  {
      _order->setExtraClause("RA", "(SELECT SUM(raitem_qtyauthorized) > 0 "
                       "  FROM raitem"
                       "  WHERE ((raitem_rahead_id=orderhead_id)"
                       "     AND (orderhead_type = 'RA'))) "
                       " AND "
                       "(SELECT true "
                       " FROM raitem"
                       " WHERE ((raitem_rahead_id=orderhead_id)"
                       "   AND  (raitem_disposition IN ('R','P','V')) "
                       "   AND  (orderhead_type = 'RA')) "
                       " LIMIT 1)");

      _order->setExtraClause("TO", "(SELECT SUM(NONEG(toitem_qty_shipped - toitem_qty_received)) > 0 "
                       "  FROM toitem"
                       "  WHERE ((toitem_tohead_id=orderhead_id)"
                       "     AND (orderhead_type = 'TO'))) ");
  }

  _orderitem->addColumn(tr("#"),            _whsColumn,  Qt::AlignCenter  , true,  "linenumber");
  _orderitem->addColumn(tr("Due Date"),     _dateColumn, Qt::AlignLeft    , true,  "duedate");
  _orderitem->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft    , true,  "item_number");
  _orderitem->addColumn(tr("Description"),  -1,          Qt::AlignLeft    , true,  "itemdescription");
  _orderitem->addColumn(tr("Inv. UOM"),     _uomColumn,  Qt::AlignCenter  , true,  "inv_uom");
  _orderitem->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter  , true,  "warehous_code");
  _orderitem->addColumn(tr("Vend. Item #"), -1,          Qt::AlignLeft    , true,  "vend_item_number");
  _orderitem->addColumn(tr("Vend. UOM"),    _uomColumn,  Qt::AlignCenter  , true,  "vend_uom");
  _orderitem->addColumn(tr("Manufacturer"), _orderColumn,  Qt::AlignLeft  , false, "manuf_name");
  _orderitem->addColumn(tr("Manuf. Item#"), _orderColumn,  Qt::AlignCenter, false, "manuf_item_number");
  _orderitem->addColumn(tr("Ordered"),      _qtyColumn,  Qt::AlignRight   , true,  "qty_ordered");
  _orderitem->addColumn(tr("Received"),     _qtyColumn,  Qt::AlignRight   , true,  "qty_received");
  _orderitem->addColumn(tr("Returned"),     _qtyColumn,  Qt::AlignRight   , true,  "qty_returned");
  _orderitem->addColumn(tr("To Receive"),   _qtyColumn,  Qt::AlignRight   , true,  "qty_toreceive");

  _captive = false;
  _soheadid = -1;
  
  _bcQty->setValidator(omfgThis->qtyVal());

  //Remove lot/serial  if no lot/serial tracking
  if (!_metrics->boolean("LotSerialControl"))
    _singleLot->hide();
}

enterPoReceipt::~enterPoReceipt()
{
  // no need to delete child widgets, Qt does it all for us
}

void enterPoReceipt::languageChange()
{
  retranslateUi(this);
}

enum SetResponse enterPoReceipt::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _captive = true;
    _order->setId(param.toInt(), "PO");
  }

  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    _captive = true;
    _order->setId(param.toInt(), "TO");
  }

  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _captive = true;
    _order->setId(param.toInt(), "RA");
  }

  return NoError;
}

void enterPoReceipt::setParams(ParameterList & params)
{
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (_order->isValid())
  {
    if (_order->isPO())
      params.append("pohead_id", _order->id());
    else if (_order->isRA())
      params.append("rahead_id", _order->id());
    else if (_order->isTO())
      params.append("tohead_id", _order->id());

    params.append("orderid",   _order->id());
    params.append("ordertype", _order->type());
  }

  params.append("nonInventory",	tr("Non-Inventory"));
  params.append("na",		tr("N/A"));
}

void enterPoReceipt::sPrint()
{
  ParameterList params;
  setParams(params);

  printLabelsByOrder newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void enterPoReceipt::sPrintItemLabel()
{
    ParameterList params;
    params.append("vendorItemLit", tr("Vendor Item#:"));
    params.append("ordertype", _order->type());
    params.append("orderitemid", _orderitem->id());
    orReport report("ReceivingLabel", params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
    }
}

void enterPoReceipt::post(const QString pType, const int pId)
{
  enterPoReceipt * w = new enterPoReceipt(0, "enterPoReceipt");
  w->setWindowModality(Qt::WindowModal);
  ParameterList params;
  if (pType == "PO")
    params.append("pohead_id", pId);
  else if (pType == "RA")
    params.append("rahead_id", pId);
  else if (pType == "TO")
    params.append("tohead_id", pId);
  w->set(params);
  w->sPost();
}

void enterPoReceipt::sPost()
{
  XSqlQuery enterPost;
  ParameterList params;	// shared by several queries
  setParams(params);

  QString checks = "SELECT SUM(qtyToReceive(recv_order_type,"
		   "                        recv_orderitem_id)) AS qtyToRecv "
		   "FROM recv, orderitem "
		   "WHERE ((recv_order_type=<? value(\"ordertype\") ?>)"
		   "  AND  (recv_orderitem_id=orderitem_id)"
                   "  AND  (orderitem_orderhead_type=<? value(\"ordertype\") ?>)"
		   "  AND  (orderitem_orderhead_id=<? value(\"orderid\") ?>));";
  MetaSQLQuery checkm(checks);
  enterPost = checkm.toQuery(params);
  if (enterPost.first())
  {
    if (enterPost.value("qtyToRecv").toDouble() <= 0)
    {
      QMessageBox::critical(this, tr("Nothing selected for Receipt"),
			    tr("<p>No Line Items have been selected "
			       "for receipt. Select at least one Line Item and "
			       "enter a Receipt before trying to Post."));
      return;
    }
  }
  else if (enterPost.lastError().type() != QSqlError::NoError)
  {
    systemError(this, enterPost.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  QString lotnum = QString::null;
  QDate expdate = omfgThis->startOfTime();
  QDate warrdate;
  bool gotlot = false;

  enterPost.exec("BEGIN;");	// because of possible insertgltransaction failures
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  QString items = "SELECT recv_id, itemsite_controlmethod, itemsite_perishable,itemsite_warrpurc, "
                  "  (recv_order_type = 'RA' AND COALESCE(itemsite_costmethod,'') = 'J') AS issuerawo, "
                  "  (recv_order_type = 'PO' AND COALESCE(itemsite_costmethod,'') = 'J' AND poitem_order_type='W') AS issuejobwo, "
                  "  (recv_order_type = 'PO' AND COALESCE(itemsite_costmethod,'') = 'J' AND poitem_order_type='S') AS issuejobso, "
                  "  COALESCE(pohead_dropship, false) AS dropship "
                  " FROM orderitem, recv "
                  "  LEFT OUTER JOIN itemsite ON (recv_itemsite_id=itemsite_id) "
                  "  LEFT OUTER JOIN poitem ON ((recv_order_type='PO') "
                  "                         AND (recv_orderitem_id=poitem_id)) "
                  "  LEFT OUTER JOIN pohead ON (poitem_pohead_id=pohead_id) "
                  " WHERE((recv_orderitem_id=orderitem_id)"
                  "   AND (orderitem_orderhead_id=<? value(\"orderid\") ?>)"
                  "   AND (orderitem_orderhead_type=<? value (\"ordertype\") ?>)"
                  "   AND (NOT recv_posted)"
                  "   AND (recv_trans_usr_name=getEffectiveXtUser())"
                  "   AND (recv_order_type=<? value (\"ordertype\") ?>))"
                  " ORDER BY orderitem_linenumber";
  MetaSQLQuery itemsm(items);
  XSqlQuery qi = itemsm.toQuery(params);
  while(qi.next())
  {
    if(_singleLot->isChecked() && !gotlot && qi.value("itemsite_controlmethod").toString() == "L")
    {
      getLotInfo newdlg(this, "", true);
      newdlg.enableExpiration(qi.value("itemsite_perishable").toBool());
      newdlg.enableWarranty(qi.value("itemsite_warrpurc").toBool());

      if(newdlg.exec() == XDialog::Accepted)
      {
        gotlot = true;
        lotnum = newdlg.lot();
        expdate = newdlg.expiration();
        warrdate = newdlg.warranty();
      }
    }

    XSqlQuery postLine;
    postLine.prepare("SELECT postReceipt(recv_id, 0) AS result "
                     "FROM recv "
                     "WHERE (recv_id=:recv_id);");
    postLine.bindValue(":recv_id", qi.value("recv_id").toInt());
    postLine.exec();
    if (postLine.first())
    {
      int result = postLine.value("result").toInt();
      if (result < 0 && result != -11) // ignore -11 as it just means there was no inventory
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("postReceipt", result),
		    __FILE__, __LINE__);
        return;
      }
  
      if (distributeInventory::SeriesAdjust(result, this, lotnum, expdate, warrdate) == XDialog::Rejected)
      {
        QMessageBox::information( this, tr("Enter Receipts"), tr("Post Canceled") );
        rollback.exec();
        return;
      }

      // Job item for Return Service; issue this to work order
      if (qi.value("issuerawo").toBool())
      {
        XSqlQuery issue;
        issue.prepare("SELECT issueWoRtnReceipt(coitem_order_id, invhist_id) AS result "
                        "FROM invhist, recv "
                        " JOIN raitem ON (raitem_id=recv_orderitem_id) "
                        " JOIN coitem ON (coitem_id=raitem_new_coitem_id) "
                        "WHERE ((invhist_series=:itemlocseries) "
                        " AND (recv_id=:id));");
        issue.bindValue(":itemlocseries", postLine.value("result").toInt());
        issue.bindValue(":id",  qi.value("recv_id").toInt());
        issue.exec();
        if (issue.lastError().type() != QSqlError::NoError)
        {
          systemError(this, issue.lastError().databaseText(), __FILE__, __LINE__);
          rollback.exec();
          return;
        }
      }
      // Job item for Purchase Order; issue this to work order
      else if (qi.value("issuejobwo").toBool())
      {
        XSqlQuery issue;
        issue.prepare("SELECT issueWoMaterial(womatl_id, "
                      "       (recv_qty * poitem_invvenduomratio), "
                      "       :itemlocseries, NOW(), invhist_id ) AS result "
                      "FROM invhist, recv "
                      " JOIN poitem ON (poitem_id=recv_orderitem_id) "
                      " JOIN womatl ON (womatl_id=poitem_order_id AND poitem_order_type='W') "
                      "WHERE ((invhist_series=:itemlocseries) "
                      "  AND  (recv_id=:id));");
        issue.bindValue(":itemlocseries", postLine.value("result").toInt());
        issue.bindValue(":id",  qi.value("recv_id").toInt());
        issue.exec();
        if (issue.first())
        {
          if (issue.value("result").toInt() < 0)
          {
            rollback.exec();
            systemError( this, storedProcErrorLookup("issueWoMaterial", issue.value("result").toInt()),
                        __FILE__, __LINE__);
            return;
          }

          issue.prepare("SELECT postItemLocSeries(:itemlocseries);");
          issue.bindValue(":itemlocseries", postLine.value("result").toInt());
          issue.exec();
        }
        else if (issue.lastError().type() != QSqlError::NoError)
        {
          systemError(this, issue.lastError().databaseText(), __FILE__, __LINE__);
          rollback.exec();
          return;
        }
      }
      // Issue drop ship orders and Job Cost items to shipping
      else if (qi.value("dropship").toBool() || qi.value("issuejobso").toBool())
      {
        XSqlQuery issue;
        issue.prepare("SELECT issueToShipping('SO', coitem_id, "
                      "  roundQty(item_fractional, (recv_qty * poitem_invvenduomratio / coitem_qty_invuomratio)), "
                      "  :itemlocseries, recv_gldistdate, invhist_id ) AS result, "
                      "  coitem_cohead_id, cohead_holdtype "
                      "FROM invhist, recv "
                      " JOIN poitem ON (poitem_id=recv_orderitem_id) "
                      " JOIN coitem ON (coitem_id=poitem_order_id AND poitem_order_type='S') "
                      " JOIN cohead ON (coitem_cohead_id=cohead_id) "
                      " JOIN itemsite ON (itemsite_id=coitem_itemsite_id)"
                      " JOIN item ON (item_id=itemsite_item_id)"
                      "WHERE ((invhist_series=:itemlocseries) "
                      " AND (recv_id=:id));");
        issue.bindValue(":itemlocseries", postLine.value("result").toInt());
        issue.bindValue(":id",  qi.value("recv_id").toInt());
        issue.exec();
        if (issue.first())
        {
          if (issue.value("result").toInt() < 0)
          {
            rollback.exec();
            systemError( this, storedProcErrorLookup("issueToShipping", issue.value("result").toInt()),
                        __FILE__, __LINE__);
            return;
          }
          if (issue.value("cohead_holdtype").toString() != "N")
          {
            QString msg = tr("This Purchase Order is being drop shipped against "
                     "a Sales Order that is on Hold.  The Sales Order must "
                     "be taken off Hold before the Receipt may be Posted.");
            rollback.exec();
            QMessageBox::warning(this, tr("Cannot Ship Order"), msg);
            return;
          }

          _soheadid = issue.value("coitem_cohead_id").toInt();
          issue.prepare("SELECT postItemLocSeries(:itemlocseries);");
          issue.bindValue(":itemlocseries", postLine.value("result").toInt());
          issue.exec();

          if (qi.value("dropship").toBool() && _soheadid != -1)
          {
            XSqlQuery ship;
            ship.prepare("SELECT shipShipment(shiphead_id, recv_gldistdate) AS result, "
                         "  shiphead_id "
                         "FROM shiphead, recv "
                         "WHERE ( (shiphead_order_type='SO') "
                         " AND (shiphead_order_id=:cohead_id) "
                         " AND (NOT shiphead_shipped) "
                         " AND (recv_id=:recv_id) );");
            ship.bindValue(":cohead_id", _soheadid);
            ship.bindValue(":recv_id",  qi.value("recv_id").toInt());
            ship.exec();
            if (ship.first())
            {
              if (ship.value("result").toInt() < 0)
              {
                rollback.exec();
                systemError( this, storedProcErrorLookup("shipShipment", ship.value("result").toInt()),
                            __FILE__, __LINE__);
                return;
              }
              if (_metrics->boolean("BillDropShip"))
              {
                int shipheadid = ship.value("shiphead_id").toInt();
                ship.prepare("SELECT selectUninvoicedShipment(:shiphead_id);");
                ship.bindValue(":shiphead_id", shipheadid);
                ship.exec();
                if (ship.lastError().type() != QSqlError::NoError)
                {
                  rollback.exec();
                  systemError(this, ship.lastError().databaseText(), __FILE__, __LINE__);
                  return;
                }
              }
            }
            else if (ship.lastError().type() != QSqlError::NoError)
            {
              systemError(this, ship.lastError().databaseText(), __FILE__, __LINE__);
              rollback.exec();
              return;
            }
          }
        }
        else if (issue.lastError().type() != QSqlError::NoError)
        {
          systemError(this, issue.lastError().databaseText(), __FILE__, __LINE__);
          rollback.exec();
          return;
        }
      }
    }
    else if (postLine.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, postLine.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  _soheadid = -1;

  enterPost.exec("COMMIT;");

  // TODO: update this to sReceiptsUpdated?
  omfgThis->sPurchaseOrderReceiptsUpdated();

  if (_captive)
  {
    _orderitem->clear();
    close();
  }
  else
  {
    _order->setId(-1);
    _close->setText(tr("&Close"));
  }
}

void enterPoReceipt::sSave()
{
  // most of the work is done in enterPoitemReceipt
  // don't call this->close() 'cause that deletes the recv records
  XWidget::close();
}

void enterPoReceipt::sEnter()
{
  ParameterList params;
  params.append("lineitem_id", _orderitem->id());
  params.append("order_type", _order->type());
  params.append("mode", "new");

  enterPoitemReceipt newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void enterPoReceipt::sFillList()
{
  XSqlQuery enterFillList;
  _orderitem->clear();

  if (_order->isRA())
  {
    enterFillList.prepare( "SELECT (rahead_expiredate < CURRENT_DATE) AS expired "
               "FROM rahead "
               "WHERE (rahead_id=:rahead_id);" );
    enterFillList.bindValue(":rahead_id", _order->id());
    enterFillList.exec();
    if (enterFillList.first())
    {
      if (enterFillList.value("expired").toBool())
      {
        QMessageBox::warning(this, tr("RMA Expired"),
                             tr("<p>The selected Return Authorization "
                                "is expired and cannot be received."));
        _order->setId(-1);
        _order->setFocus();
      }
    }
    else if (enterFillList.lastError().type() != QSqlError::NoError)
    {
      systemError(this, enterFillList.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      _order->setId(-1);
      _order->setFocus();
      return;
    }
  }

  XSqlQuery dropship;
  dropship.prepare("SELECT pohead_dropship FROM pohead WHERE (pohead_id = :pohead_id);"); 
  dropship.bindValue(":pohead_id", _order->id());
  dropship.exec();
  if(dropship.first())
  {
    if(dropship.value("pohead_dropship").toBool())
	  _dropShip->setChecked(true);
	else
	  _dropShip->setChecked(false);
  }

  disconnect(_order,	SIGNAL(valid(bool)),	this, SLOT(sFillList()));
  if (_order->isValid())
  {
    ParameterList params;
    setParams(params);
    MetaSQLQuery fillm = mqlLoad("receipt", "detail");
    enterFillList = fillm.toQuery(params);
    _orderitem->populate(enterFillList,true);
    if (enterFillList.lastError().type() != QSqlError::NoError)
    {
      systemError(this, enterFillList.lastError().databaseText(), __FILE__, __LINE__);
      connect(_order,	SIGNAL(valid(bool)),	this, SLOT(sFillList()));
      return;
    }
  }
  connect(_order,	SIGNAL(valid(bool)),	this, SLOT(sFillList()));
}

void enterPoReceipt::close()
{
  XSqlQuery enterclose;
  QList<XTreeWidgetItem*> zeroItems = _orderitem->findItems("^[0.]*$", Qt::MatchRegExp, 9);
  if (_order->isValid() &&
      zeroItems.size() != _orderitem->topLevelItemCount())
  {
    if (QMessageBox::question(this, tr("Cancel Receipts?"),
			      tr("<p>Are you sure you want to cancel all "
				 "unposted Receipts for this Order?"),
			      QMessageBox::Yes,
			      QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    {
      ParameterList params;
      setParams(params);
      QString dels = "SELECT deleteRecvForOrder(<? value (\"ordertype\") ?>,"
		     "                   <? value(\"orderid\") ?>) AS result;" ;
      MetaSQLQuery delm(dels);
      enterclose = delm.toQuery(params);
      if (enterclose.first())
      {
	int result = enterclose.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("deleteRecvForOrder", result), __FILE__, __LINE__);
	  return;
	}
      }
      else if (enterclose.lastError().type() != QSqlError::NoError)
      {
	systemError(this, enterclose.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      omfgThis->sPurchaseOrderReceiptsUpdated();
    }
    else
      return;
  }
  if(!isVisible())
    deleteLater();
  else
    XWidget::close();
}

void enterPoReceipt::sReceiveAll()
{
  XSqlQuery enterReceiveAll;
  ParameterList params;
  setParams(params);
  if (_metrics->boolean("EnableReturnAuth"))
    params.append("EnableReturnAuth", true);
  MetaSQLQuery recvm = mqlLoad("receipt", "receiveAll");
  enterReceiveAll = recvm.toQuery(params);

  while (enterReceiveAll.next())
  {
    int result = enterReceiveAll.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("enterReceipt", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  if (enterReceiveAll.lastError().type() != QSqlError::NoError)
  {
    systemError(this, enterReceiveAll.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sPurchaseOrderReceiptsUpdated();
  sFillList();
}

void enterPoReceipt::sPopulateMenu(QMenu *pMenu,  QTreeWidgetItem * /*selected*/)
{
  QAction *menuItem;
  if (_orderitem->altId() != -1)
    menuItem = pMenu->addAction(tr("Print Label..."), this, SLOT(sPrintItemLabel()));
  menuItem = pMenu->addAction(tr("Enter Receipt..."), this, SLOT(sEnter()));
}

void enterPoReceipt::sBcFind()
{
  XSqlQuery enterBcFind;
  if (!_order->isValid())
  {
    QMessageBox::warning(this, tr("Invalid Order"),
                         tr("<p>Cannot search for Items by Bar Code without "
                            "first selecting an Order."));
    _bc->setFocus();
    return;
  }

  if (_bc->text().isEmpty())
  {
    QMessageBox::warning(this, tr("No Bar Code scanned"),
                         tr("<p>Cannot search for Items by Bar Code without a "
                            "Bar Code."));
    _bc->setFocus();
    return;
  }

  double qtytoreceive = 0;

  // find item that matches barcode and is a line item in this order.
  // then call enterPoItemReceipt passing in params to preset and
  // run the receive button.
  ParameterList findbc;
  setParams(findbc);
  findbc.append("bc", _bc->text());
  MetaSQLQuery fillm = mqlLoad("receipt", "detail");
  enterBcFind = fillm.toQuery(findbc);
  if(enterBcFind.first())
    qtytoreceive = enterBcFind.value("qty_toreceive").toDouble();
  else
  {
    if (enterBcFind.lastError().type() != QSqlError::NoError)
    {
      systemError(this, enterBcFind.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    XMessageBox::message(this, QMessageBox::Warning, tr("No Match Found"),
                         tr("<p>No Items on this Order match the specified Barcode.") );
    _bc->clear();
    return;
  }

  ParameterList params;
  params.append("lineitem_id", enterBcFind.value("orderitem_id").toInt());
  params.append("order_type", _order->type());
  params.append("mode", "new");
  params.append("qty", _bcQty->toDouble() + qtytoreceive);
  params.append("receive");
  params.append("snooze");

  enterPoitemReceipt newdlg(this, "", true);
  if (newdlg.set(params) != NoError)
    return;
	
  sFillList();
  if (_autoPost->isChecked())
  {
    int id = _order->id();
    QString type = _order->type();
    sPost();
    _order->setId(id, type);
  }
  _bc->clear();
  _bcQty->setText("1");
}

void enterPoReceipt::sCatchPoheadid(int pPoheadid)
{
  _order->setId(pPoheadid, "PO");
  _orderitem->selectAll();
}

void enterPoReceipt::sCatchPoitemid(int pPoitemid)
{
  XSqlQuery enterCatchPoitemid;
  enterCatchPoitemid.prepare( "SELECT poitem_pohead_id "
             "FROM poitem "
             "WHERE (poitem_id=:poitem_id);" );
  enterCatchPoitemid.bindValue(":poitem_id", pPoitemid);
  enterCatchPoitemid.exec();
  if (enterCatchPoitemid.first())
  {
    _order->setId(enterCatchPoitemid.value("poitem_pohead_id").toInt(), "PO");
    _orderitem->clearSelection();
    _orderitem->setId(pPoitemid);
    sEnter();
  }
  else if (enterCatchPoitemid.lastError().type() != QSqlError::NoError)
  {
    systemError(this, enterCatchPoitemid.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void enterPoReceipt::sCatchToheadid(int pToheadid)
{
  _order->setId(pToheadid, "TO");
  _orderitem->selectAll();
}

void enterPoReceipt::sCatchToitemid(int porderitemid)
{
  XSqlQuery enterCatchToitemid;
  enterCatchToitemid.prepare( "SELECT toitem_tohead_id "
             "FROM toitem "
             "WHERE (toitem_id=:tohead_id);" );
  enterCatchToitemid.bindValue(":tohead_id", porderitemid);
  enterCatchToitemid.exec();
  if (enterCatchToitemid.first())
  {
    _order->setId(enterCatchToitemid.value("toitem_tohead_id").toInt(), "TO");
    _orderitem->clearSelection();
    _orderitem->setId(porderitemid);
    sEnter();
  }
  else if (enterCatchToitemid.lastError().type() != QSqlError::NoError)
  {
    systemError(this, enterCatchToitemid.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void enterPoReceipt::sCatchItemsiteid(int pItemsiteid)
{
  XSqlQuery enterCatchItemsiteid;
  enterCatchItemsiteid.prepare("SELECT orderitem_id "
            "FROM orderitem "
            "WHERE ((orderitem_itemsite_id=:itemsite) "
            "   AND (orderitem_orderhead_type=:ordertype) "
            "   AND (orderitem_orderhead_id=:orderid));");
  enterCatchItemsiteid.bindValue(":itemsite",  pItemsiteid);
  enterCatchItemsiteid.bindValue(":ordertype", _order->type());
  enterCatchItemsiteid.bindValue(":orderid",   _order->id());
  enterCatchItemsiteid.exec();
  if (enterCatchItemsiteid.first())
  {
    _orderitem->clearSelection();
    _orderitem->setId(enterCatchItemsiteid.value("orderitem_id").toInt());
    sEnter();
  }
  else
    audioReject();
}

void enterPoReceipt::sCatchItemid(int pItemid)
{
  XSqlQuery enterCatchItemid;
  enterCatchItemid.prepare( "SELECT orderitem_id "
             "FROM orderitem, itemsite "
             "WHERE ((orderitem_itemsite_id=itemsite_id)"
             "  AND  (itemsite_item_id=:item_id)"
             "   AND  (orderitem_orderhead_type=:ordertype) "
             "   AND  (orderitem_orderhead_id=:orderid));");
  enterCatchItemid.bindValue(":item_id",   pItemid);
  enterCatchItemid.bindValue(":ordertype", _order->type());
  enterCatchItemid.bindValue(":orderid",   _order->id());
  enterCatchItemid.exec();
  if (enterCatchItemid.first())
  {
    _orderitem->clearSelection();
    _orderitem->setId(enterCatchItemid.value("orderitem_id").toInt());
    sEnter();
  }
  else
    audioReject();
}

