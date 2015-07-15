/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "enterPoitemReceipt.h"

#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include <openreports.h>

#include <metasql.h>

#include "xmessagebox.h"
#include "distributeInventory.h"
#include "itemSite.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"

enterPoitemReceipt::enterPoitemReceipt(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sReceive()));
  connect(_toReceive, SIGNAL(editingFinished()), this, SLOT(sDetermineToReceiveInv()));
  connect(_toReceive, SIGNAL(editingFinished()), this, SLOT(sHandleExtendedCost()));
  connect(_purchCost,  SIGNAL(editingFinished()), this, SLOT(sHandleExtendedCost()));

  _invVendorUOMRatio->setPrecision(omfgThis->ratioVal());
  _ordered->setPrecision(omfgThis->qtyVal());
  _received->setPrecision(omfgThis->qtyVal());
  _returned->setPrecision(omfgThis->qtyVal());
  _toReceiveInv->setPrecision(omfgThis->qtyVal());

  _toReceive->setValidator(omfgThis->qtyVal());
  _receiptDate->setDate(QDate::currentDate());

  _mode		= cView;
  _orderitemid	= -1;
  _ordertype	= "";
  _receivable	= 0.0;
  _recvid	= -1;
  _snooze = false;
}

enterPoitemReceipt::~enterPoitemReceipt()
{
  // no need to delete child widgets, Qt does it all for us
}

void enterPoitemReceipt::languageChange()
{
  retranslateUi(this);
}

bool enterPoitemReceipt::correctReceipt(int pRecvid, QWidget *pParent)
{
  XSqlQuery entercorrectReceipt;
  //Validate - Drop Ship receipts may not be corrected
  entercorrectReceipt.prepare("SELECT (count(*) > 0) AS result "
            "FROM recv JOIN pohead ON ((recv_order_type='PO') AND (recv_order_number=pohead_number)) "
            "WHERE ((recv_id=:recvid) "
            "  AND  (COALESCE(pohead_dropship, false))); ");
  entercorrectReceipt.bindValue(":recvid", pRecvid);
  entercorrectReceipt.exec();
  if (entercorrectReceipt.first())
  {
    if (entercorrectReceipt.value("result").toBool())
    {
      QMessageBox::warning(pParent, tr("Cannot Correct"),
                            tr("<p>Receipt is a Drop Shipment.  The received quantity may not be changed.  "
                               "You must use Purchase Order Return to make corrections." ));
      return XDialog::Rejected;
    }
  }
  
  //Validate - Split receipts may not be corrected
  entercorrectReceipt.prepare("SELECT (count(*) > 0) AS result "
            "FROM recv "
            "WHERE (((recv_id=:recvid) "
            "  AND (recv_splitfrom_id IS NOT NULL)) "
            "  OR (recv_splitfrom_id=:recvid)); ");
  entercorrectReceipt.bindValue(":recvid", pRecvid);
  entercorrectReceipt.exec();
  if (entercorrectReceipt.first())
  {
    if (entercorrectReceipt.value("result").toBool())
    {
      QMessageBox::warning(pParent, tr("Cannot Correct"),
                           tr("<p>Receipt has been split.  The received quantity may not be changed."));
      return XDialog::Rejected;
    }
    else
    {
      ParameterList params;
      params.append("mode", "edit");
      params.append("recv_id", pRecvid);

      enterPoitemReceipt newdlg(pParent, "", true);
      newdlg.set(params);

      if (newdlg.exec() != XDialog::Rejected)
        return true;
    }
  }
  return false;
}

enum SetResponse enterPoitemReceipt::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _toReceiveLit->setText(tr("Correct Qty. to:"));
      _freightLit->setText(tr("Correct Freight to:"));
      _item->setEnabled(false);
      setWindowTitle(tr("Correct Item Receipt"));
    }
  }

  param = pParams.value("order_type", &valid);
  if (valid)
    _ordertype = param.toString();

  param = pParams.value("lineitem_id", &valid);
  if (valid)
  {
    _orderitemid = param.toInt();
    populate();
  }

  param = pParams.value("recv_id", &valid);
  if (valid)
  {
    _recvid = param.toInt();
    populate();
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _toReceive->setDouble(param.toDouble());

  _snooze = pParams.inList("snooze");

  if(pParams.inList("receive"))
    sReceive();

  return NoError;
}

void enterPoitemReceipt::populate()
{
  XSqlQuery enterpopulate;
  ParameterList params;

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  if (_metrics->boolean("EnableReturnAuth"))
    params.append("EnableReturnAuth");

  // NOTE: this crashes if popm is defined and toQuery() is called outside the blocks
  if (_mode == cNew)
  {
    MetaSQLQuery popm = mqlLoad("itemReceipt", "populateNew");

    params.append("ordertype",    _ordertype);
    params.append("orderitem_id", _orderitemid);

    enterpopulate = popm.toQuery(params);
  }
  else if (_mode == cEdit)
  {
    MetaSQLQuery popm = mqlLoad("itemReceipt", "populateEdit");
    params.append("recv_id", _recvid);
    enterpopulate = popm.toQuery(params);
  }
  else
  {
    systemError(this, tr("<p>Incomplete Parameter List: "
			 "_orderitem_id=%1, _ordertype=%2, _mode=%3.")
                       .arg(_orderitemid)
                       .arg(_ordertype)
                       .arg(_mode) );
    return;
  }

  if (enterpopulate.first())
  {
    _orderNumber->setText(enterpopulate.value("order_number").toString());
    _lineNumber->setText(enterpopulate.value("orderitem_linenumber").toString());
    _vendorItemNumber->setText(enterpopulate.value("vend_item_number").toString());
    _vendorDescrip->setText(enterpopulate.value("vend_item_descrip").toString());
    _vendorUOM->setText(enterpopulate.value("vend_uom").toString());
    _invVendorUOMRatio->setDouble(enterpopulate.value("orderitem_qty_invuomratio").toDouble());
    _dueDate->setDate(enterpopulate.value("duedate").toDate());
    _ordered->setDouble(enterpopulate.value("orderitem_qty_ordered").toDouble());
    _received->setDouble(enterpopulate.value("qtyreceived").toDouble());
    _returned->setDouble(enterpopulate.value("qtyreturned").toDouble());
    _receivable = enterpopulate.value("receivable").toDouble();
    _notes->setText(enterpopulate.value("notes").toString());
    _receiptDate->setDate(enterpopulate.value("effective").toDate());
    _freight->setId(enterpopulate.value("curr_id").toInt());
    _freight->setLocalValue(enterpopulate.value("recv_freight").toDouble());

    if (_ordertype.isEmpty())
      _ordertype = enterpopulate.value("recv_order_type").toString();
    if (_ordertype == "PO")
      _orderType->setText(tr("P/O"));
    else if (_ordertype == "TO")
    {
      _returnedLit->setText(tr("Qty. Shipped:"));
      _orderType->setText(tr("T/O"));
    }
    else if (_ordertype == "RA")
      _orderType->setText(tr("R/A"));

    int itemsiteid = enterpopulate.value("itemsiteid").toInt();
    if (itemsiteid > 0)
      _item->setItemsiteid(itemsiteid);
    _item->setEnabled(false);

    _purchCost->setId(enterpopulate.value("recv_purchcost_curr_id").toInt());
    _purchCost->setLocalValue(enterpopulate.value("recv_purchcost").toDouble());
    _purchCost->setEnabled(enterpopulate.value("costmethod_average").toBool() && _metrics->boolean("AllowReceiptCostOverride"));

    _extendedCost->setId(enterpopulate.value("recv_purchcost_curr_id").toInt());

    if (enterpopulate.value("inventoryitem").toBool() && itemsiteid <= 0)
    {
      MetaSQLQuery ism = mqlLoad("itemReceipt", "sourceItemSite");
      XSqlQuery isq = ism.toQuery(params);
      if (isq.first())
      {
        itemsiteid = itemSite::createItemSite(this,
                      isq.value("itemsite_id").toInt(),
                      isq.value("warehous_id").toInt(),
                      true);
        if (itemsiteid < 0)
          return;
        _item->setItemsiteid(itemsiteid);
      }
      else if (isq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, isq.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }
  else if (enterpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, enterpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void enterPoitemReceipt::sReceive()
{
  XSqlQuery enterReceive;
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");
    
  if(_metrics->boolean("DisallowReceiptExcessQty") && _receivable < _toReceive->toDouble())
  {
    XMessageBox::message( (isVisible() ? this : parentWidget()), QMessageBox::Warning, tr("Cannot Receive"),
                          tr(  "<p>Cannot receive more quantity than ordered." ),
                          QString::null, QString::null, _snooze );
    return;
  }

  if(_ordertype == "RA" && _receivable < _toReceive->toDouble())
  {
    XMessageBox::message( (isVisible() ? this : parentWidget()), QMessageBox::Warning, tr("Cannot Receive"),
                          tr(  "<p>Cannot receive more quantity than authorized." ),
                          QString::null, QString::null, _snooze );
    return;
  }

  double tolerance = _metrics->value("ReceiptQtyTolerancePct").toDouble() / 100.0;
  if(_metrics->boolean("WarnIfReceiptQtyDiffers") &&
      (_receivable < _toReceive->toDouble() * (1.0 - tolerance) ||
       _receivable > _toReceive->toDouble() * (1.0 + tolerance)))
  {
    if(XMessageBox::message( (isVisible() ? this : parentWidget()) , QMessageBox::Question, tr("Receipt Qty. Differs"),
        tr("<p>The Qty entered does not match the receivable Qty for this order. "
		   "Do you wish to continue anyway?"),
        tr("Yes"), tr("No"), _snooze, 0, 1) == 1)
      return;
  }

  int result = 0;
  QString storedProc;
  if (_mode == cNew)
  {
    enterReceive.prepare("SELECT enterReceipt(:ordertype, :poitem_id, :qty, :freight, :notes, "
              ":curr_id, :effective, :purchcost) AS result;");
    enterReceive.bindValue(":poitem_id",	_orderitemid);
    enterReceive.bindValue(":ordertype",	_ordertype);
    storedProc = "enterReceipt";
  }
  else if (_mode == cEdit)
  {
    enterReceive.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
    enterReceive.prepare("UPDATE recv SET recv_notes = :notes WHERE (recv_id=:recv_id);" );
    enterReceive.bindValue(":notes",	_notes->toPlainText());
    enterReceive.bindValue(":recv_id",	_recvid);
    enterReceive.exec();
    if (enterReceive.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, enterReceive.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    enterReceive.prepare("SELECT correctReceipt(:recv_id, :qty, :freight, 0, "
              ":curr_id, :effective, :purchcost) AS result;");
    enterReceive.bindValue(":recv_id", _recvid);
    storedProc = "correctReceipt";
  }

  enterReceive.bindValue(":qty",		_toReceive->toDouble());
  enterReceive.bindValue(":freight",	_freight->localValue());
  enterReceive.bindValue(":notes",		_notes->toPlainText());
  enterReceive.bindValue(":curr_id",	_freight->id());
  enterReceive.bindValue(":effective",	_receiptDate->date());
  enterReceive.bindValue(":purchcost",     _purchCost->localValue());
  enterReceive.exec();
  if (enterReceive.first())
  {
    result = enterReceive.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup(storedProc, result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (enterReceive.lastError().type() != QSqlError::NoError)
  {
      rollback.exec();
      systemError(this, enterReceive.lastError().databaseText(), __FILE__, __LINE__);
      return;
  }

  if(cEdit == _mode)
  {
    if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
    {
      rollback.exec();
      XMessageBox::message( (isVisible() ? this : parentWidget()), QMessageBox::Warning, tr("Enter PO Receipt"),
                            tr(  "<p>Transaction Cancelled." ),
                            QString::null, QString::null, _snooze );
      return;
    }

    enterReceive.exec("COMMIT;");
  }

  omfgThis->sPurchaseOrderReceiptsUpdated();
  accept();
  
  if (_printLabel->isChecked())
    sPrintItemLabel();
}

void enterPoitemReceipt::sDetermineToReceiveInv()
{
    _toReceiveInv->setDouble(_invVendorUOMRatio->toDouble() * _toReceive->toDouble());
}

void enterPoitemReceipt::sPrintItemLabel()
{
    ParameterList params;
    params.append("vendorItemLit", tr("Vendor Item#:"));
    params.append("ordertype", _ordertype);
    params.append("orderitemid", _orderitemid);
    orReport report("ReceivingLabel", params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
    }
}

void enterPoitemReceipt::sHandleExtendedCost()
{
  _extendedCost->setLocalValue(_toReceive->toDouble() * _purchCost->localValue());
}
