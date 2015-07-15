/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "voucherItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "mqlutil.h"
#include "voucherItemDistrib.h"
#include "enterPoitemReceipt.h"
#include "splitReceipt.h"
#include "taxDetail.h"

voucherItem::voucherItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  XSqlQuery voucherItem;
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_uninvoiced, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(sToggleReceiving(QTreeWidgetItem*)));
  connect(_uninvoiced, SIGNAL(populateMenu(QMenu*,XTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, XTreeWidgetItem*)));
  connect(_freightToVoucher, SIGNAL(editingFinished()), this, SLOT(sFillList()));
  connect(_vodist, SIGNAL(populated()), this, SLOT(sCalculateTax()));
  connect(_taxtype,	  SIGNAL(newID(int)), this, SLOT(sCalculateTax()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));

  _item->setReadOnly(true);
  
  _unitPrice->setPrecision(omfgThis->priceVal());
  _extPrice->setPrecision(omfgThis->moneyVal());
  _lineFreight->setPrecision(omfgThis->moneyVal());

  _ordered->setValidator(omfgThis->qtyVal());
  _received->setValidator(omfgThis->qtyVal());
  _rejected->setValidator(omfgThis->qtyVal());
  _uninvoicedReceived->setValidator(omfgThis->qtyVal());
  _uninvoicedRejected->setValidator(omfgThis->qtyVal());

  _qtyToVoucher->setValidator(omfgThis->qtyVal());
  _amtToVoucher->setValidator(omfgThis->moneyVal());

  _vodist->addColumn(tr("Cost Element"), -1,           Qt::AlignLeft, true, "costelem_type");
  _vodist->addColumn(tr("Amount"),       _priceColumn, Qt::AlignRight,true, "vodist_amount");

  _uninvoiced->addColumn(tr("Receipt/Reject"), -1,          Qt::AlignCenter, true, "action");
  _uninvoiced->addColumn(tr("Date"),           _dateColumn, Qt::AlignCenter, true, "item_date");
  _uninvoiced->addColumn(tr("Qty."),           _qtyColumn,  Qt::AlignRight,  true, "qty");
  _uninvoiced->addColumn(tr("Unit Price"),     _moneyColumn,Qt::AlignRight,  true, "unitprice");
  _uninvoiced->addColumn(tr("Tagged"),         _ynColumn,   Qt::AlignCenter, true, "f_tagged");
  
  _rejectedMsg = tr("The application has encountered an error and must "
                    "stop editing this Voucher Item.\n%1");

  _inTransaction = true;
  voucherItem.exec("BEGIN;"); //Lot's of things can happen in here that can cause problems if cancelled out.  Let's make it easy to roll it back.
}

voucherItem::~voucherItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void voucherItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse voucherItem::set(const ParameterList &pParams)
{
  XSqlQuery setVoucher;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _freightToVoucher->setId(param.toInt());
    _tax->setId(param.toInt());
  }

  param = pParams.value("effective", &valid);
  if (valid)
  {
    _freightToVoucher->setEffective(param.toDate());
    _tax->setEffective(param.toDate());
  }

  param = pParams.value("vohead_id", &valid);
  if (valid)
  {
    _voheadid = param.toInt();
    setVoucher.prepare("SELECT vohead_taxzone_id "
	          "FROM vohead "
	          "WHERE (vohead_id = :vohead_id);");
    setVoucher.bindValue(":vohead_id", _voheadid);
    setVoucher.exec();
    if (setVoucher.first())
      _taxzoneid = setVoucher.value("vohead_taxzone_id").toInt();
    else if (setVoucher.lastError().type() != QSqlError::NoError)
    {
      systemError(this, setVoucher.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
	else
	  _taxzoneid = -1;
  }
  else
    _voheadid = -1;

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    _poitemid = param.toInt();

    setVoucher.prepare( "SELECT pohead_number, poitem_linenumber, poitem_taxtype_id, "
               "       COALESCE(itemsite_id, -1) AS itemsiteid,"
               "       poitem_vend_item_number, poitem_vend_uom, poitem_vend_item_descrip,"
               "       poitem_duedate,"
               "       poitem_qty_ordered,"
               "       poitem_qty_received,"
               "       poitem_qty_returned,"
               "       ( SELECT COALESCE(SUM(recv_qty), 0)"
               "                    FROM recv"
               "                    WHERE ( (recv_posted)"
               "                     AND (NOT recv_invoiced)"
               "                     AND (recv_vohead_id IS NULL)"
               "                     AND (recv_orderitem_id=poitem_id) ) ) AS f_received,"
               "       ( SELECT COALESCE(SUM(poreject_qty), 0)"
               "                    FROM poreject"
               "                    WHERE ( (poreject_posted)"
               "                     AND (NOT poreject_invoiced)"
               "                     ANd (poreject_vohead_id IS NULL)"
               "                     AND (poreject_poitem_id=poitem_id) ) ) AS f_rejected,"
               "       poitem_unitprice,"
               "       poitem_unitprice * poitem_qty_ordered AS f_extprice,"
               "       poitem_freight "
               "FROM pohead, "
	       " poitem LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id) "
           "WHERE ( (poitem_pohead_id=pohead_id)"
               " AND (poitem_id=:poitem_id) );" );
    setVoucher.bindValue(":poitem_id", _poitemid);
    setVoucher.exec();
    if (setVoucher.first())
    {
      _poNumber->setText(setVoucher.value("pohead_number").toString());
      _lineNumber->setText(setVoucher.value("poitem_linenumber").toString());
      _vendItemNumber->setText(setVoucher.value("poitem_vend_item_number").toString());
      _vendUOM->setText(setVoucher.value("poitem_vend_uom").toString());
      _vendDescription->setText(setVoucher.value("poitem_vend_item_descrip").toString());
      _dueDate->setDate(setVoucher.value("poitem_duedate").toDate());
      _ordered->setText(setVoucher.value("poitem_qty_ordered").toDouble());
      _received->setText(setVoucher.value("poitem_qty_received").toDouble());
      _rejected->setText(setVoucher.value("poitem_qty_returned").toDouble());
      _uninvoicedReceived->setText(setVoucher.value("f_received").toDouble());
      _uninvoicedRejected->setText(setVoucher.value("f_rejected").toDouble());
      _unitPrice->setText(setVoucher.value("poitem_unitprice").toDouble());
      _extPrice->setText(setVoucher.value("f_extprice").toDouble());
      _lineFreight->setText(setVoucher.value("poitem_freight").toDouble());
	  _taxtype->setId(setVoucher.value("poitem_taxtype_id").toInt());
      if (setVoucher.value("itemsiteid") != -1)
        _item->setItemsiteid(setVoucher.value("itemsiteid").toInt());
    }
    else if (setVoucher.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(setVoucher.lastError().databaseText()),
                  __FILE__, __LINE__);
      reject();
      return UndefinedError;
    }
  }
  else
    _poitemid = -1;

  if ( (_voheadid != -1) && (_poitemid != -1) )
  {
    setVoucher.prepare( "SELECT voitem_id, voitem_close, voitem_taxtype_id, "
               "       voitem_qty,"
               "       voitem_freight,"
               "       ( SELECT SUM(vodist_amount) "
               "         FROM vodist "
               "         WHERE ((vodist_vohead_id=:vohead_id) "
               "           AND  (vodist_poitem_id=:poitem_id)) ) AS voitem_amt "
               "FROM voitem "
               "WHERE ( (voitem_vohead_id=:vohead_id)"
               " AND (voitem_poitem_id=:poitem_id) );" );
    setVoucher.bindValue(":vohead_id", _voheadid);
    setVoucher.bindValue(":poitem_id", _poitemid);
    setVoucher.exec();
    if (setVoucher.first())
    {
      _voitemid = setVoucher.value("voitem_id").toInt();
      _closePoitem->setChecked(setVoucher.value("voitem_close").toBool());
      _qtyToVoucher->setText(setVoucher.value("voitem_qty").toDouble());
      _amtToVoucher->setDouble(setVoucher.value("voitem_amt").toDouble());
      _freightToVoucher->setLocalValue(setVoucher.value("voitem_freight").toDouble());
      _taxtype->setId(setVoucher.value("voitem_taxtype_id").toInt());
    }
    else if (setVoucher.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(setVoucher.lastError().databaseText()),
                  __FILE__, __LINE__);
      reject();
      return UndefinedError;
    }
    else
    {
      _voitemid = -1;
      _closePoitem->setChecked(false);
      _qtyToVoucher->clear();
      _amtToVoucher->clear();
      _freightToVoucher->clear();
    }

	setVoucher.prepare( "SELECT SUM(COALESCE(taxhist_tax, 0.00)) AS taxamt "
	           "FROM voitem LEFT OUTER JOIN voitemtax "
			   " ON (voitem_id = taxhist_parent_id) "
               "WHERE ( (voitem_vohead_id=:vohead_id)"
               " AND (voitem_poitem_id=:poitem_id) );" );
    setVoucher.bindValue(":vohead_id", _voheadid);
    setVoucher.bindValue(":poitem_id", _poitemid);
    setVoucher.exec();
    if (setVoucher.first())
	  _tax->setLocalValue(setVoucher.value("taxamt").toDouble());
	else if (setVoucher.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(setVoucher.lastError().databaseText()),
                  __FILE__, __LINE__);
      reject();
      return UndefinedError;
    }
    else
      _tax->clear();
  }

  sFillList();
  _saved = true;
  return NoError;
}

void voucherItem::sSave()
{
  XSqlQuery voucherSave;
  if (_qtyToVoucher->toDouble() <= 0.0)
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher Item"),
                           tr("You must enter a postive Quantity to Voucher before saving this Voucher Item") );
    _qtyToVoucher->setFocus();
    return;
  }

  // Check to make sure there is at least distribution for this Voucher Item
  voucherSave.prepare( "SELECT vodist_id "
             "FROM vodist "
             "WHERE ( (vodist_vohead_id=:vohead_id)"
             " AND (vodist_poitem_id=:poitem_id) ) "
             "LIMIT 1;" );
  voucherSave.bindValue(":vohead_id", _voheadid);
  voucherSave.bindValue(":poitem_id", _poitemid);
  voucherSave.exec();
  if (!voucherSave.first())
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher Item"),
                           tr("You must make at least one distribution for this Voucher Item before you may save it.") );
    return;
  }
  if (voucherSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(voucherSave.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  // Check for vendor matching requirement
  voucherSave.prepare( "SELECT vend_id "
             " FROM vendinfo,pohead,poitem "
	     " WHERE (	(vend_id=pohead_vend_id) "
	     " AND (pohead_id=poitem_pohead_id) "
	     " AND (poitem_id=:poitem_id) "
	     " AND (vend_match) ); " );
  voucherSave.bindValue(":poitem_id", _poitemid);
  voucherSave.exec();
  if (voucherSave.first())
  {
    voucherSave.prepare( "SELECT formatMoney(poitem_unitprice * :voitem_qty) AS f_povalue FROM poitem "
		" WHERE ((poitem_unitprice * :voitem_qty) <> "
		" (SELECT SUM(vodist_amount) "
		"	FROM vodist " 
		"       WHERE ( (vodist_vohead_id=:vohead_id) "
		"       AND (vodist_poitem_id=:poitem_id) ) )"
		" AND (poitem_id=:poitem_id) ); " );
    voucherSave.bindValue(":vohead_id", _voheadid);
    voucherSave.bindValue(":poitem_id", _poitemid);
    voucherSave.bindValue(":voitem_qty", _qtyToVoucher->toDouble());
    voucherSave.exec();
  	if (voucherSave.first())
    {
    QString msg;
    msg = "The P/O value of ";
    msg.append( voucherSave.value("f_povalue").toString() );
    msg.append( " does not match the total distributed value.\nInvoice matching is required for this vendor.\nStop and correct?" );
    if ( QMessageBox::warning( this, tr("Invoice Value Mismatch"), msg, tr("Yes"), tr("No"), QString::null ) != 1 )
          return;
    }
  }
  else if (voucherSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(voucherSave.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  // Update the qty vouchered
  voucherSave.prepare( "UPDATE voitem "
             "SET voitem_close=:voitem_close,"
             "    voitem_freight=:voitem_freight, "
			 "    voitem_taxtype_id=:voitem_taxtype_id "
             "WHERE (voitem_id=:voitem_id);"
             "UPDATE vodist "
             "SET vodist_qty=:qty "
             "WHERE ((vodist_vohead_id=:vohead_id)"
             " AND (vodist_poitem_id=:poitem_id) );" );
  voucherSave.bindValue(":qty", _qtyToVoucher->toDouble());
  voucherSave.bindValue(":poitem_id", _poitemid);
  voucherSave.bindValue(":voitem_id", _voitemid);
  voucherSave.bindValue(":vohead_id", _voheadid);
  voucherSave.bindValue(":voitem_close", QVariant(_closePoitem->isChecked()));
  voucherSave.bindValue(":voitem_freight", _freightToVoucher->localValue());
  if (_taxtype->id() != -1)
    voucherSave.bindValue(":voitem_taxtype_id", _taxtype->id());
  voucherSave.exec();
  if (voucherSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(voucherSave.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }
  voucherSave.exec("COMMIT;");
  
  _inTransaction = false;
  accept();
}

void voucherItem::sNew()
{
  XSqlQuery voucherNew;
  voucherNew.prepare( "SELECT COALESCE(SUM(vodist_amount),0) AS f_amount "
             "	FROM vodist "
             " WHERE ( (vodist_vohead_id=:vohead_id) "
             "   AND (vodist_poitem_id=:poitem_id) );" );
  voucherNew.bindValue(":vohead_id", _voheadid);
  voucherNew.bindValue(":poitem_id", _poitemid);
  voucherNew.exec();
  if (voucherNew.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(voucherNew.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  ParameterList params;
  params.append("vohead_id", _voheadid);
  params.append("poitem_id", _poitemid);
  params.append("mode", "new");
  params.append("curr_id", _freightToVoucher->id());
  params.append("effective", _freightToVoucher->effective());
  if (voucherNew.first())
    params.append("amount", (_amtToVoucher->toDouble() - voucherNew.value("f_amount").toDouble()) );
  else
    params.append("amount", _amtToVoucher->toDouble());

  voucherItemDistrib newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void voucherItem::sEdit()
{
  ParameterList params;
  params.append("vodist_id", _vodist->id());
  params.append("mode", "edit");
  params.append("curr_id", _freightToVoucher->id());
  params.append("effective", _freightToVoucher->effective());

  voucherItemDistrib newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void voucherItem::sDelete()
{
  XSqlQuery voucherDelete;
  voucherDelete.prepare( "DELETE FROM vodist "
             "WHERE (vodist_id=:vodist_id);" );
  voucherDelete.bindValue(":vodist_id", _vodist->id());
  voucherDelete.exec();
  if (voucherDelete.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(voucherDelete.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  sFillList();
}

void voucherItem::sToggleReceiving(QTreeWidgetItem *pItem)
{
  XSqlQuery voucherToggleReceiving;
  double vch = 0;
  double amt = 0;
  double rcv = 0;
  double rej = 0;

  QString s;
  XTreeWidgetItem* item = (XTreeWidgetItem*)pItem;
  if(item->id() == -1)
    return;

  // TODO: translation
  if (item->text(4) == "Yes")
    item->setText(4, "No");
  else
    item->setText(4, "Yes");

  for (int i = 0; i < _uninvoiced->topLevelItemCount(); i++)
  {
    XTreeWidgetItem *item = _uninvoiced->topLevelItem(i);
    double pp  = item->data(3, Xt::RawRole).toDouble();
    double qty = item->data(2, Xt::RawRole).toDouble();

    if (item->text(4) == "Yes")
    {
      vch = vch + qty;
      amt = amt + (qty * pp);
    }
    else
    {
      if (item->text(0) == "Receiving")
      {
        rcv = rcv + qty;
      }
      else
      {
        rej = rej + qty;
      }
    }
  }
  _qtyToVoucher->setDouble(vch);
  _amtToVoucher->setDouble(amt);
  _uninvoicedReceived->setDouble(rcv);
  _uninvoicedRejected->setDouble(rej);

  // Check PO Close flag

  if ( ((_ordered->toDouble() <= (_received->toDouble() - _rejected->toDouble()))) && (_uninvoicedReceived->toDouble() == 0) && (_uninvoicedRejected->toDouble() == 0) )
        _closePoitem->setChecked(true);
  else
	_closePoitem->setChecked(false);
  
  // Save the voitem information
  if (_voitemid != -1)
  {
    voucherToggleReceiving.prepare( "UPDATE voitem "
               "SET voitem_qty=:voitem_qty "
               "WHERE (voitem_id=:voitem_id);" );
    voucherToggleReceiving.bindValue(":voitem_id", _voitemid);
  }
  else
  {
  // Get next voitem id
    voucherToggleReceiving.prepare("SELECT NEXTVAL('voitem_voitem_id_seq') AS voitemid");
    voucherToggleReceiving.exec();
    if (voucherToggleReceiving.first())
      _voitemid = (voucherToggleReceiving.value("voitemid").toInt());
    else if (voucherToggleReceiving.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(voucherToggleReceiving.lastError().databaseText()),
                  __FILE__, __LINE__);
      reject();
      return;
    }
    
    voucherToggleReceiving.prepare( "INSERT INTO voitem "
               "(voitem_id, voitem_vohead_id, voitem_poitem_id, voitem_close, voitem_qty, voitem_freight) "
               "VALUES "
               "(:voitem_id, :vohead_id, :poitem_id, :voitem_close, :voitem_qty, :voitem_freight);" );
  }

  voucherToggleReceiving.bindValue(":voitem_id", _voitemid);
  voucherToggleReceiving.bindValue(":vohead_id", _voheadid);
  voucherToggleReceiving.bindValue(":poitem_id", _poitemid);
  voucherToggleReceiving.bindValue(":voitem_close", QVariant(_closePoitem->isChecked()));
  voucherToggleReceiving.bindValue(":voitem_qty", _qtyToVoucher->toDouble());
  voucherToggleReceiving.bindValue(":voitem_freight", _freightToVoucher->localValue());
  voucherToggleReceiving.exec();
  if (voucherToggleReceiving.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(voucherToggleReceiving.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }
  
  // Update the receipt record
  if (item->text(4) == "Yes")
  {
    if (item->altId() == 1)
      voucherToggleReceiving.prepare( "UPDATE recv "
                 "SET recv_vohead_id=:vohead_id,recv_voitem_id=:voitem_id "
                 "WHERE (recv_id=:target_id);" );
    else if (item->altId() == 2)
      voucherToggleReceiving.prepare( "UPDATE poreject "
                 "SET poreject_vohead_id=:vohead_id,poreject_voitem_id=:voitem_id "
                 "WHERE (poreject_id=:target_id);" );
  }
  else
  {
    if (item->altId() == 1)
      voucherToggleReceiving.prepare( "UPDATE recv "
                 "SET recv_vohead_id=NULL,recv_voitem_id=NULL "
                 "WHERE ((recv_id=:target_id)"
                 "  AND  (recv_vohead_id=:vohead_id));" );
    else if (item->altId() == 2)
      voucherToggleReceiving.prepare( "UPDATE poreject "
                 "SET poreject_vohead_id=NULL,poreject_voitem_id=NULL "
                 "WHERE ((poreject_id=:target_id)"
                 "  AND  (poreject_vohead_id=:vohead_id));" );
  }

  voucherToggleReceiving.bindValue(":vohead_id", _voheadid);
  voucherToggleReceiving.bindValue(":voitem_id", _voitemid);
  voucherToggleReceiving.bindValue(":target_id", item->id());
  voucherToggleReceiving.exec();
  if (voucherToggleReceiving.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(voucherToggleReceiving.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

}

void voucherItem::sFillList()
{
  MetaSQLQuery distmql = mqlLoad("voucherItem", "distributions");

  ParameterList params;
  params.append("none", tr("None"));
  params.append("poitem_id", _poitemid);
  params.append("vohead_id", _voheadid);
  XSqlQuery distq = distmql.toQuery(params);
  _vodist->populate(distq);
  if (distq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(distq.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  // Fill univoiced receipts list
  MetaSQLQuery recmql = mqlLoad("voucherItem", "receipts");

  params.append("receiving", tr("Receiving"));
  params.append("reject", tr("Reject"));
  XSqlQuery recq = recmql.toQuery(params);
  _uninvoiced->populate(recq, true);
  if (recq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(recq.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  // Display the total distributed amount
  XSqlQuery totalDist;
  totalDist.prepare( "SELECT SUM(vodist_amount) AS totalamount "
                     "FROM vodist "
                     "WHERE ( (vodist_vohead_id=:vohead_id)"
                     "  AND   (vodist_poitem_id=:poitem_id) );" );
  totalDist.bindValue(":vohead_id", _voheadid);
  totalDist.bindValue(":poitem_id", _poitemid);
  totalDist.exec();
  if (totalDist.first())
    _totalDistributed->setLocalValue(totalDist.value("totalamount").toDouble() +
                                     _tax->localValue() +
                                     _freightToVoucher->localValue());
  else if (totalDist.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(totalDist.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }
 }

void voucherItem::sCorrectReceiving()
{
  XSqlQuery voucherFillList;
  if (enterPoitemReceipt::correctReceipt(_uninvoiced->id(), this) != XDialog::Rejected)
    sFillList();
}

void voucherItem::sSplitReceipt()
{
  ParameterList params;
  params.append("recv_id", _uninvoiced->id());

  splitReceipt newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void voucherItem::sPopulateMenu(QMenu *pMenu,  XTreeWidgetItem *selected)
{
  QAction *menuItem;
  
  if ( (selected->rawValue("action") == "REC") && (selected->text(4) == "No") )
  {
    menuItem = pMenu->addAction(tr("Correct Receipt..."), this, SLOT(sCorrectReceiving()));
    menuItem->setEnabled(_privileges->check("EnterReceipts"));

    menuItem = pMenu->addAction(tr("Split Receipt..."), this, SLOT(sSplitReceipt()));
    menuItem->setEnabled(_privileges->check("EnterReceipts"));
  }
}

void voucherItem::reject()
{
  XSqlQuery voucherreject;
  if(_inTransaction)
  {
    voucherreject.exec("ROLLBACK;");
    _inTransaction = false;
  }
  XDialog::reject();
}

void voucherItem::closeEvent(QCloseEvent * event)
{
  XSqlQuery vouchercloseEvent;
  if(_inTransaction)
  {
    vouchercloseEvent.exec("ROLLBACK;");
    _inTransaction = false;
  }
  XDialog::closeEvent(event);
}

void voucherItem::sCalculateTax()
{
  _saved = false;
  XSqlQuery calcq;
  calcq.prepare( "SELECT SUM(COALESCE(tax, 0.00)) AS totaltax "
                 "FROM (SELECT calculateTax(vohead_taxzone_id, :taxtype_id, "
				 " vohead_docdate, vohead_curr_id, vodist_amount) AS tax "
                 " FROM vohead JOIN vodist ON(vohead_id=vodist_vohead_id) "
                 " WHERE (vohead_id=:vohead_id) "
                 " AND (vodist_poitem_id=:poitem_id) "
				 ") data;"); 
  calcq.bindValue(":vohead_id", _voheadid);
  calcq.bindValue(":taxtype_id", _taxtype->id());
  calcq.bindValue(":poitem_id", _poitemid);
  calcq.exec();
  if (calcq.first())
    _tax->setLocalValue(calcq.value("totaltax").toDouble());
  else if (calcq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void voucherItem::sTaxDetail()
{
  XSqlQuery voucherTaxDetail;
  voucherTaxDetail.prepare( "SELECT SUM(vodist_amount) AS distamount "
             "FROM vodist "
             "WHERE ( (vodist_vohead_id=:vohead_id)"
             " AND (vodist_poitem_id=:poitem_id) );" );
  voucherTaxDetail.bindValue(":vohead_id", _voheadid);
  voucherTaxDetail.bindValue(":poitem_id", _poitemid);
  voucherTaxDetail.exec();
  if (voucherTaxDetail.first())
    _distamount = voucherTaxDetail.value("distamount").toDouble();
  else if (voucherTaxDetail.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(voucherTaxDetail.lastError().databaseText()),
                __FILE__, __LINE__);
    return;
  }

  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("taxzone_id", _taxzoneid);
  params.append("taxtype_id", _taxtype->id());
  params.append("date", _tax->effective());
  params.append("subtotal", _distamount);
  params.append("curr_id", _tax->id());
  params.append("sense", -1);
  
  if(cView == _mode)
    params.append("readOnly");
  
  if(_saved == true)
  {
	params.append("order_id", _voitemid);
    params.append("order_type", "VI");
  }

  newdlg.set(params);
  
  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    if (_taxtype->id() != newdlg.taxtype())
      _taxtype->setId(newdlg.taxtype());
  }
}
