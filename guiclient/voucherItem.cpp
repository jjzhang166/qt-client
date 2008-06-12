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

#include "voucherItem.h"

// #include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "voucherItemDistrib.h"
#include "enterPoitemReceipt.h"
#include "splitReceipt.h"

voucherItem::voucherItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_uninvoiced, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(sToggleReceiving(QTreeWidgetItem*)));
  connect(_uninvoiced, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_freightToVoucher, SIGNAL(lostFocus()), this, SLOT(sFillList()));

  _item->setReadOnly(TRUE);
  _qtyToVoucher->setValidator(omfgThis->qtyVal());

  _vodist->addColumn(tr("Cost Element"), -1,           Qt::AlignLeft, true, "costelem_type");
  _vodist->addColumn(tr("Amount"),       _priceColumn, Qt::AlignRight,true, "vodist_amount");

  _uninvoiced->addColumn(tr("Receipt/Reject"), -1,          Qt::AlignCenter, true, "action");
  _uninvoiced->addColumn(tr("Date"),           _dateColumn, Qt::AlignCenter, true, "item_date");
  _uninvoiced->addColumn(tr("Qty."),           _qtyColumn,  Qt::AlignRight,  true, "qty");
  _uninvoiced->addColumn(tr("Tagged"),         _ynColumn,   Qt::AlignCenter, true, "f_tagged");
  
  _rejectedMsg = tr("The application has encountered an error and must "
                    "stop editing this Voucher Item.\n%1");

  _inTransaction = TRUE;
  q.exec("BEGIN;"); //Lot's of things can happen in here that can cause problems if cancelled out.  Let's make it easy to roll it back.
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
  QVariant param;
  bool     valid;

  param = pParams.value("curr_id", &valid);
  if (valid)
    _freightToVoucher->setId(param.toInt());

  param = pParams.value("effective", &valid);
  if (valid)
    _freightToVoucher->setEffective(param.toDate());

  param = pParams.value("vohead_id", &valid);
  if (valid)
    _voheadid = param.toInt();
  else
    _voheadid = -1;

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    _poitemid = param.toInt();

    q.prepare( "SELECT pohead_number, poitem_linenumber,"
               "       COALESCE(itemsite_id, -1) AS itemsiteid,"
               "       poitem_vend_item_number, poitem_vend_uom, poitem_vend_item_descrip,"
               "       poitem_duedate,"
               "       formatQty(poitem_qty_ordered) AS qtyordered,"
               "       formatQty(poitem_qty_received) AS qtyreceived,"
               "       formatQty(poitem_qty_returned) AS qtyreturned,"
               "       formatQty( ( SELECT COALESCE(SUM(porecv_qty), 0)"
               "                    FROM porecv"
               "                    WHERE ( (porecv_posted)"
               "                     AND (NOT porecv_invoiced)"
       	       "                     AND (porecv_vohead_id IS NULL)"
               "                     AND (porecv_poitem_id=poitem_id) ) ) ) AS f_received,"
               "       formatQty( ( SELECT COALESCE(SUM(poreject_qty), 0)"
               "                    FROM poreject"
               "                    WHERE ( (poreject_posted)"
               "                     AND (NOT poreject_invoiced)"
               "                     ANd (poreject_vohead_id IS NULL)"
               "                     AND (poreject_poitem_id=poitem_id) ) ) ) AS f_rejected,"
               "       formatMoney(poitem_unitprice) AS f_unitprice,"
               "       formatMoney(poitem_unitprice * poitem_qty_ordered) AS f_extprice,"
               "       formatMoney(poitem_freight) AS f_linefreight "
               "FROM pohead, "
	       " poitem LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id) "
           "WHERE ( (poitem_pohead_id=pohead_id)"
               " AND (poitem_id=:poitem_id) );" );
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    if (q.first())
    {
      _poNumber->setText(q.value("pohead_number").toString());
      _lineNumber->setText(q.value("poitem_linenumber").toString());
      _vendItemNumber->setText(q.value("poitem_vend_item_number").toString());
      _vendUOM->setText(q.value("poitem_vend_uom").toString());
      _vendDescription->setText(q.value("poitem_vend_item_descrip").toString());
      _dueDate->setDate(q.value("poitem_duedate").toDate());
      _ordered->setText(q.value("qtyordered").toString());
      _received->setText(q.value("qtyreceived").toString());
      _rejected->setText(q.value("qtyreturned").toString());
      _uninvoicedReceived->setText(q.value("f_received").toString());
      _uninvoicedRejected->setText(q.value("f_rejected").toString());
      _unitPrice->setText(q.value("f_unitprice").toString());
      _extPrice->setText(q.value("f_extprice").toString());
      _lineFreight->setText(q.value("f_linefreight").toString());

      if (q.value("itemsiteid") != -1)
        _item->setItemsiteid(q.value("itemsiteid").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
      reject();
      return UndefinedError;
    }
  }
  else
    _poitemid = -1;

  if ( (_voheadid != -1) && (_poitemid != -1) )
  {
    q.prepare( "SELECT voitem_id, voitem_close,"
               "       formatQty(voitem_qty) AS f_qty,"
               "       voitem_freight "
               "FROM voitem "
               "WHERE ( (voitem_vohead_id=:vohead_id)"
               " AND (voitem_poitem_id=:poitem_id) );" );
    q.bindValue(":vohead_id", _voheadid);
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    if (q.first())
    {
      _voitemid = q.value("voitem_id").toInt();
      _closePoitem->setChecked(q.value("voitem_close").toBool());
      _qtyToVoucher->setText(q.value("f_qty").toString());
      _freightToVoucher->setLocalValue(q.value("voitem_freight").toDouble());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
      reject();
      return UndefinedError;
    }
    else
    {
      _voitemid = -1;
      _closePoitem->setChecked(FALSE);
      _qtyToVoucher->clear();
      _freightToVoucher->clear();
    }
  }

  sFillList();

  return NoError;
}

void voucherItem::sSave()
{
  if (_qtyToVoucher->toDouble() <= 0.0)
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher Item"),
                           tr("You must enter a postive Quantity to Voucher before saving this Voucher Item") );
    _qtyToVoucher->setFocus();
    return;
  }

//  Check to make sure there is at least distribution for this Voucher Item
  q.prepare( "SELECT vodist_id "
             "FROM vodist "
             "WHERE ( (vodist_vohead_id=:vohead_id)"
             " AND (vodist_poitem_id=:poitem_id) ) "
             "LIMIT 1;" );
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":poitem_id", _poitemid);
  q.exec();
  if (!q.first())
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher Item"),
                           tr("You must make at least one distribution for this Voucher Item before you may save it.") );
    return;
  }
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

// Check for vendor matching requirement
  q.prepare( "SELECT vend_id "
             " FROM vendinfo,pohead,poitem "
	     " WHERE (	(vend_id=pohead_vend_id) "
	     " AND (pohead_id=poitem_pohead_id) "
	     " AND (poitem_id=:poitem_id) "
	     " AND (vend_match) ); " );
  q.bindValue(":poitem_id", _poitemid);
  q.exec();
  if (q.first())
  {
    q.prepare( "SELECT formatMoney(poitem_unitprice * :voitem_qty) AS f_povalue FROM poitem "
		" WHERE ((poitem_unitprice * :voitem_qty) <> "
		" (SELECT SUM(vodist_amount) "
		"	FROM vodist " 
		"       WHERE ( (vodist_vohead_id=:vohead_id) "
		"       AND (vodist_poitem_id=:poitem_id) ) )"
		" AND (poitem_id=:poitem_id) ); " );
    q.bindValue(":vohead_id", _voheadid);
    q.bindValue(":poitem_id", _poitemid);
    q.bindValue(":voitem_qty", _qtyToVoucher->toDouble());
    q.exec();
  	if (q.first())
    {
    QString msg;
    msg = "The P/O value of ";
    msg.append( q.value("f_povalue").toString() );
    msg.append( " does not match the total distributed value.\nInvoice matching is required for this vendor.\nStop and correct?" );
    if ( QMessageBox::warning( this, tr("Invoice Value Mismatch"), msg, tr("Yes"), tr("No"), QString::null ) != 1 )
          return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

//  Update the qty vouchered
  q.prepare( "UPDATE voitem "
             "SET voitem_close=:voitem_close,"
             "    voitem_freight=:voitem_freight "
             "WHERE (voitem_id=:voitem_id);"
             "UPDATE vodist "
             "SET vodist_qty=:qty "
             "WHERE ((vodist_vohead_id=:vohead_id)"
             " AND (vodist_poitem_id=:poitem_id) );" );
  q.bindValue(":qty", _qtyToVoucher->toDouble());
  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":voitem_id", _voitemid);
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":voitem_close", QVariant(_closePoitem->isChecked(), 0));
  q.bindValue(":voitem_freight", _freightToVoucher->localValue());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  q.exec("COMMIT;");
  _inTransaction = FALSE;
  accept();
}

void voucherItem::sNew()
{
  q.prepare( "SELECT (poitem_unitprice * :voitem_qty - "
		" (SELECT COALESCE(SUM(vodist_amount),0) "
		"	FROM vodist " 
		"       WHERE ( (vodist_vohead_id=:vohead_id) "
		"       AND (vodist_poitem_id=:poitem_id) ) " 
		" ) ) AS f_amount FROM poitem "
		" WHERE (poitem_id=:poitem_id); " );
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":voitem_qty", _qtyToVoucher->toDouble());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
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
  if (q.first())
  	params.append("amount", q.value("f_amount").toDouble() );
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  voucherItemDistrib newdlg(this, "", TRUE);
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

  voucherItemDistrib newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void voucherItem::sDelete()
{
  q.prepare( "DELETE FROM vodist "
             "WHERE (vodist_id=:vodist_id);" );
  q.bindValue(":vodist_id", _vodist->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  sFillList();
}

void voucherItem::sToggleReceiving(QTreeWidgetItem *pItem)
{
  double n;
  QString s;
  XTreeWidgetItem* item = (XTreeWidgetItem*)pItem;
  if(item->id() == -1)
    return;
  if (item->text(3) == "Yes")
  {
    item->setText(3, "No");
    if (item->text(0) == "Receiving")
    {
    	n = _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(item->text(2));
    	n = n - _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(s.setNum(n));

      n = _uninvoicedReceived->toDouble();
      _uninvoicedReceived->setText(item->text(2));
      n = n + _uninvoicedReceived->toDouble();
      _uninvoicedReceived->setText(s.setNum(n));
    }
    else
    {
    	n = _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(item->text(2));
    	n = n + _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(s.setNum(n));

      n = _uninvoicedRejected->toDouble();
      _uninvoicedRejected->setText(item->text(2));
      n = n + _rejected->toDouble();
      _uninvoicedRejected->setText(s.setNum(n));
    }
  }
  else 
  {
    item->setText(3, "Yes");
    if (item->text(0) == "Receiving")
    {
    	n = _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(item->text(2));
    	n = n + _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(s.setNum(n));

      n = _uninvoicedReceived->toDouble();
      _uninvoicedReceived->setText(item->text(2));
      n = n - _uninvoicedReceived->toDouble();
      _uninvoicedReceived->setText(s.setNum(n));
    }
    else
    {
      n = _qtyToVoucher->toDouble();
      _qtyToVoucher->setText(item->text(2));
      n = n - _qtyToVoucher->toDouble();
      _qtyToVoucher->setText(s.setNum(n));

      n = _uninvoicedRejected->toDouble();
      _uninvoicedRejected->setText(item->text(2));
      n = n - _uninvoicedRejected->toDouble();
      _uninvoicedRejected->setText(s.setNum(n));
    }
  }

//Check PO Close flag

  if ( ((_ordered->toDouble() <= (_received->toDouble() - _rejected->toDouble()))) && (_uninvoicedReceived->toDouble() == 0) && (_uninvoicedRejected->toDouble() == 0) )
        _closePoitem->setChecked(true);
  else
	_closePoitem->setChecked(false);
  
//  Save the voitem information
  if (_voitemid != -1)
  {
    q.prepare( "UPDATE voitem "
               "SET voitem_qty=:voitem_qty "
               "WHERE (voitem_id=:voitem_id);" );
    q.bindValue(":voitem_id", _voitemid);
  }
  else
  {
//  Get next voitem id
    q.prepare("SELECT NEXTVAL('voitem_voitem_id_seq') AS voitemid");
    q.exec();
    if (q.first())
      _voitemid = (q.value("voitemid").toInt());
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
      reject();
      return;
    }
    
    q.prepare( "INSERT INTO voitem "
               "(voitem_id, voitem_vohead_id, voitem_poitem_id, voitem_close, voitem_qty, voitem_freight) "
               "VALUES "
               "(:voitem_id, :vohead_id, :poitem_id, :voitem_close, :voitem_qty, :voitem_freight);" );
  }

  q.bindValue(":voitem_id", _voitemid);
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":voitem_close", QVariant(_closePoitem->isChecked(), 0));
  q.bindValue(":voitem_qty", _qtyToVoucher->toDouble());
  q.bindValue(":voitem_freight", _freightToVoucher->localValue());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }
  
//Update the receipt record
  if (item->text(3) == "Yes")
  {
    if (item->altId() == 1)
      q.prepare( "UPDATE recv "
                 "SET recv_vohead_id=:vohead_id,recv_voitem_id=:voitem_id "
                 "WHERE (recv_id=:target_id);" );
    else if (item->altId() == 2)
      q.prepare( "UPDATE poreject "
                 "SET poreject_vohead_id=:vohead_id,poreject_voitem_id=:voitem_id "
                 "WHERE (poreject_id=:target_id);" );
  }
  else
  {
    if (item->altId() == 1)
      q.prepare( "UPDATE recv "
                 "SET recv_vohead_id=NULL,recv_voitem_id=NULL "
                 "WHERE ((recv_id=:target_id)"
                 "  AND  (recv_vohead_id=:vohead_id));" );
    else if (item->altId() == 2)
      q.prepare( "UPDATE poreject "
                 "SET poreject_vohead_id=NULL,poreject_voitem_id=NULL "
                 "WHERE ((poreject_id=:target_id)"
                 "  AND  (poreject_vohead_id=:vohead_id));" );
  }

  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":voitem_id", _voitemid);
  q.bindValue(":target_id", item->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

}

void voucherItem::sFillList()
{
  q.prepare( "SELECT vodist_id,"
             "       COALESCE(costelem_type, :none) AS costelem_type,"
             "       vodist_amount, 'currval' AS vodist_amount_xtnumericrole "
             "FROM vodist "
             "     LEFT OUTER JOIN costelem ON (vodist_costelem_id=costelem_id) "
             "WHERE ( (vodist_poitem_id=:poitem_id)"
             " AND (vodist_vohead_id=:vohead_id) );" );
  q.bindValue(":none", tr("None"));
  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":vohead_id", _voheadid);
  q.exec();
  _vodist->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  q.prepare( "SELECT SUM(vodist_amount) AS totalamount "
             "FROM vodist "
             "WHERE ( (vodist_vohead_id=:vohead_id)"
             " AND (vodist_poitem_id=:poitem_id) );" );
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":poitem_id", _poitemid);
  q.exec();
  if (q.first())
    _totalDistributed->setLocalValue(q.value("totalamount").toDouble() +
					_freightToVoucher->localValue());
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }
          
  //Fill univoiced receipts list
  q.prepare( "SELECT porecv_id AS item_id, 1 AS item_type, :receiving AS action,"
             "       porecv_date AS item_date,"
             "       porecv_qty AS qty, 'qty' AS qty_xtnumericrole,"
             "       formatBoolYN(porecv_vohead_id=:vohead_id) AS f_tagged,"
             "       0 AS qty_xttotalrole "
             "FROM porecv "
             "WHERE ( (NOT porecv_invoiced)"
             " AND ((porecv_vohead_id IS NULL) OR (porecv_vohead_id=:vohead_id))"
             " AND (porecv_poitem_id=:poitem_id) ) "

             "UNION "
             "SELECT poreject_id AS item_id, 2 AS item_type, :reject AS action,"
             "       poreject_date AS item_date,"
             "       poreject_qty * -1 AS qty, 'qty', "
             "       formatBoolYN(poreject_vohead_id=:vohead_id) AS f_tagged,"
             "       0 AS qty_xttotalrole "
             "FROM poreject "
             "WHERE ( (poreject_posted)"
             " AND (NOT poreject_invoiced)"
             " AND ((poreject_vohead_id IS NULL) OR (poreject_vohead_id=:vohead_id))"
             " AND (poreject_poitem_id=:poitem_id) );" );
  q.bindValue(":receiving", tr("Receiving"));
  q.bindValue(":reject", tr("Reject"));
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":poitem_id", _poitemid);
  q.exec();
  _uninvoiced->populate(q, true);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }
}

void voucherItem::sCorrectReceiving()
{
  if (enterPoitemReceipt::correctReceipt(_uninvoiced->id(), this) != XDialog::Rejected)
    sFillList();
}

void voucherItem::sSplitReceipt()
{
  ParameterList params;
  params.append("recv_id", _uninvoiced->id());

  splitReceipt newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void voucherItem::sPopulateMenu(QMenu *pMenu,  QTreeWidgetItem *selected)
{
  int menuItem;
  
  if (selected->text(3) == "No")
  {
    menuItem = pMenu->insertItem(tr("Correct Receipt..."), this, SLOT(sCorrectReceiving()), 0);
    if (!_privileges->check("EnterReceipts"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Split Receipt..."), this, SLOT(sSplitReceipt()), 0);
    if (!_privileges->check("EnterReceipts"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void voucherItem::reject()
{
  if(_inTransaction)
  {
    q.exec("ROLLBACK;");
    _inTransaction = false;
  }
  XDialog::reject();
}
