/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
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

#include "distributeInventory.h"
#include "enterPoitemReceipt.h"
#include "getLotInfo.h"
#include "mqlutil.h"
#include "printLabelsByOrder.h"
#include "storedProcErrorLookup.h"

enterPoReceipt::enterPoReceipt(QWidget* parent, const char* name, Qt::WFlags fl)
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
//  connect(_orderitem, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));

  _order->setAllowedStatuses(OrderLineEdit::Open);
  _order->setAllowedTypes(OrderLineEdit::Purchase |
			  OrderLineEdit::Return |
			  OrderLineEdit::Transfer);
  _order->setToSitePrivsEnforced(TRUE);

  if (_metrics->boolean("EnableReturnAuth"))
  {
      _order->setExtraClause("RA", "(SELECT SUM(raitem_qtyauthorized) > 0 "
                       "  FROM raitem"
                       "  WHERE ((raitem_rahead_id=orderhead_id)"
                       "     AND (orderhead_type = 'RA'))) "
                       " AND "
                       "(SELECT TRUE "
                       " FROM raitem"
                       " WHERE ((raitem_rahead_id=orderhead_id)"
                       "   AND  (raitem_disposition IN ('R','P','V')) "
                       "   AND  (orderhead_type = 'RA')) "
                       " LIMIT 1)");
  }
  _order->setFocus();

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

  _captive = FALSE;
  
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
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _order->setId(param.toInt(), "PO");
    _orderitem->setFocus();
  }

  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _order->setId(param.toInt(), "TO");
    _orderitem->setFocus();
  }

  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _order->setId(param.toInt(), "RA");
    _orderitem->setFocus();
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

  printLabelsByOrder newdlg(this, "", TRUE);
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
  q = checkm.toQuery(params);
  if (q.first())
  {
    if (q.value("qtyToRecv").toDouble() <= 0)
    {
      QMessageBox::critical(this, tr("Nothing selected for Receipt"),
			    tr("<p>No Line Items have been selected "
			       "for receipt. Select at least one Line Item and "
			       "enter a Receipt before trying to Post."));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  QString lotnum = QString::null;
  QDate expdate = omfgThis->startOfTime();
  QDate warrdate;
  bool gotlot = false;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  QString items = "SELECT recv_id, itemsite_controlmethod, itemsite_perishable,itemsite_warrpurc"
                  "  FROM orderitem, recv LEFT OUTER JOIN itemsite ON (recv_itemsite_id=itemsite_id)"
                  " WHERE((recv_orderitem_id=orderitem_id)"
                  "   AND (orderitem_orderhead_id=<? value(\"orderid\") ?>)"
                  "   AND (orderitem_orderhead_type=<? value (\"ordertype\") ?>)"
                  "   AND (NOT recv_posted)"
                  "   AND (recv_trans_usr_name=CURRENT_USER)"
                  "   AND (recv_order_type=<? value (\"ordertype\") ?>))";
  MetaSQLQuery itemsm(items);
  XSqlQuery qi = itemsm.toQuery(params);
  while(qi.next())
  {
    if(_singleLot->isChecked() && !gotlot && qi.value("itemsite_controlmethod").toString() == "L")
    {
      getLotInfo newdlg(this, "", TRUE);
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

    q.exec("BEGIN;");	// because of possible insertgltransaction failures
    q.prepare("SELECT postReceipt(:recv_id, 0) AS result;");
    q.bindValue(":recv_id", qi.value("recv_id").toInt());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
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
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    q.exec("COMMIT;");
  } 

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

  enterPoitemReceipt newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void enterPoReceipt::sFillList()
{
  _orderitem->clear();

  disconnect(_order,	SIGNAL(valid(bool)),	this, SLOT(sFillList()));
  if (_order->isValid())
  {
    ParameterList params;
    setParams(params);
    MetaSQLQuery fillm = mqlLoad("receipt", "detail");
    q = fillm.toQuery(params);
    _orderitem->populate(q,true);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      connect(_order,	SIGNAL(valid(bool)),	this, SLOT(sFillList()));
      return;
    }
  }
  connect(_order,	SIGNAL(valid(bool)),	this, SLOT(sFillList()));
}

void enterPoReceipt::close()
{
  QList<QTreeWidgetItem*> zeroItems = _orderitem->findItems("^[0.]*$", Qt::MatchRegExp, 9);
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
      q = delm.toQuery(params);
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("deleteRecvForOrder", result), __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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
  ParameterList params;
  setParams(params);
  if (_metrics->boolean("EnableReturnAuth"))
    params.append("EnableReturnAuth", TRUE);
  MetaSQLQuery recvm = mqlLoad("receipt", "receiveAll");
  q = recvm.toQuery(params);

  while (q.next())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("enterReceipt", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sPurchaseOrderReceiptsUpdated();
  sFillList();
}

void enterPoReceipt::sPopulateMenu(QMenu *pMenu,  QTreeWidgetItem * /*selected*/)
{
  int     menuItem;
  if (_orderitem->altId() != -1)
    menuItem = pMenu->insertItem(tr("Print Label..."), this, SLOT(sPrintItemLabel()), 0);
  menuItem = pMenu->insertItem(tr("Enter Receipt..."), this, SLOT(sEnter()), 0);
}

