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

#include "returnAuthorizationItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "priceList.h"
#include "taxDetail.h"
#include "storedProcErrorLookup.h"
#include "returnAuthItemLotSerial.h"

returnAuthorizationItem::returnAuthorizationItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

#ifndef Q_WS_MAC
  _listPrices->setMaximumWidth(25);
#endif

  _mode = cNew;
  _raitemid = -1;
  _raheadid = -1;
  _custid = -1;
  _priceRatio = 1.0;
  _qtyAuthCache = 0.0;
  _shiptoid = -1;
  _taxauthid = -1;
  _taxCache.clear();
  _qtyinvuomratio = 1.0;
  _priceinvuomratio = 1.0;
  _invuomid = -1;
  _orderId = -1;
  _preferredWarehouseid = -1;
  _status = "O";
  _qtycredited = 0;
  _soldQty = 0;
  _coitemid = -1;
  _crmacctid = -1;

  connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
  connect(_extendedPrice, SIGNAL(valueChanged()), this, SLOT(sLookupTax()));
  connect(_item,          SIGNAL(newId(int)),     this, SLOT(sPopulateItemInfo()));
  connect(_item, SIGNAL(privateIdChanged(int)), this, SLOT(sPopulateItemsiteInfo()));
  connect(_shipWhs, SIGNAL(newID(int)), this, SLOT(sPopulateItemsiteInfo()));
  connect(_listPrices,    SIGNAL(clicked()),      this, SLOT(sListPrices()));
  connect(_netUnitPrice,  SIGNAL(valueChanged()), this, SLOT(sCalculateDiscountPrcnt()));
  connect(_netUnitPrice,  SIGNAL(valueChanged()), this, SLOT(sCalculateExtendedPrice()));
  connect(_netUnitPrice,  SIGNAL(idChanged(int)), this, SLOT(sPriceGroup()));
  connect(_qtyAuth,       SIGNAL(textChanged(const QString&)), this, SLOT(sCalculateExtendedPrice()));
  connect(_qtyAuth,       SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateOrderInfo()));
  connect(_qtyAuth,       SIGNAL(textChanged(const QString&)), this, SLOT(sCalcWoUnitCost()));
  connect(_save,          SIGNAL(clicked()),      this, SLOT(sSaveClicked()));
  connect(_taxCode,       SIGNAL(newID(int)),     this, SLOT(sLookupTax()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_taxType,       SIGNAL(newID(int)),     this, SLOT(sLookupTaxCode()));
  connect(_qtyUOM, SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
  connect(_qtyUOM, SIGNAL(newID(int)), this, SLOT(sPopulateOrderInfo()));
  connect(_pricingUOM, SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));
  connect(_disposition, SIGNAL(currentIndexChanged(int)), this, SLOT(sDispositionChanged()));
  connect(_shipWhs, SIGNAL(newID(int)), this, SLOT(sDetermineAvailability()));
  connect(_scheduledDate, SIGNAL(newDate(const QDate&)), this, SLOT(sDetermineAvailability()));
  connect(_qtyAuth, SIGNAL(lostFocus()), this, SLOT(sDetermineAvailability()));
  connect(_createOrder, SIGNAL(toggled(bool)), this, SLOT(sHandleWo(bool)));
  connect(_showAvailability, SIGNAL(toggled(bool)), this, SLOT(sDetermineAvailability()));
  connect(this, SIGNAL(rejected()), this, SLOT(rejectEvent()));
  connect(_authLotSerial, SIGNAL(toggled(bool)), _qtyUOM, SLOT(setDisabled(bool)));
      
  connect(_new,		SIGNAL(clicked()),	this,	SLOT(sNew()));
  connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
  connect(_delete,	SIGNAL(clicked()),	this,	SLOT(sDelete()));
    
  _raitemls->addColumn(tr("Lot/Serial"),  -1,  Qt::AlignLeft   );
  _raitemls->addColumn(tr("Warranty"),	  _dateColumn,  Qt::AlignRight  );
  _raitemls->addColumn(tr("Registered"),  _qtyColumn,   Qt::AlignRight  );
  _raitemls->addColumn(tr("Authorized"),  _qtyColumn,   Qt::AlignRight  );
  _raitemls->addColumn(tr("Received"),    _qtyColumn,   Qt::AlignRight  );

  _qtyAuth->setValidator(omfgThis->qtyVal());
  _discountFromSale->setValidator(new QDoubleValidator(-9999, 100, 2, this));
  _taxType->setEnabled(_privileges->check("OverrideTax"));
  _taxCode->setEnabled(_privileges->check("OverrideTax"));
  _showAvailability->setChecked(_preferences->boolean("ShowSOItemAvailability"));
  
  //If not multi-warehouse hide whs controls
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
    _shipWhs->hide();
    _shipWhsLit->hide();
  } 

  //Remove lot/serial  if no lot/serial tracking
  if (!_metrics->boolean("LotSerialControl"))
    _tab->removeTab(_tab->indexOf(_lotserial));
  
  resize(minimumSize());
  
  q.exec("BEGIN;"); //In case problems or we cancel out
}

returnAuthorizationItem::~returnAuthorizationItem()
{
    // no need to delete child widgets, Qt does it all for us
}

void returnAuthorizationItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse returnAuthorizationItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _raheadid = param.toInt();
    q.prepare("SELECT *,"
              "       rahead_curr_id AS taxcurr, "
              "       COALESCE(cust_preferred_warehous_id,-1) AS prefwhs, "
              "       COALESCE(rahead_orig_cohead_id,-1) AS cohead_id, "
              "       crmacct_id "
              "FROM rahead, custinfo, crmacct "
              "WHERE ( (rahead_cust_id=cust_id) "
              "AND (rahead_id=:rahead_id) "
              "AND (rahead_cust_id=crmacct_cust_id) );");
    q.bindValue(":rahead_id", _raheadid);
    q.exec();
    if (q.first())
    {
      _authNumber->setText(q.value("rahead_number").toString());
      _taxauthid = q.value("rahead_taxauth_id").toInt();
      _tax->setId(q.value("taxcurr").toInt());
      _rsnCode->setId(q.value("rahead_rsncode_id").toInt());
      _custid = q.value("rahead_cust_id").toInt();
      _shiptoid = q.value("rahead_shipto_id").toInt();
      _netUnitPrice->setId(q.value("rahead_curr_id").toInt());
      _netUnitPrice->setEffective(q.value("rahead_authdate").toDate());
      _preferredWarehouseid = q.value("prefwhs").toInt();
      _creditmethod = q.value("rahead_creditmethod").toString();
      _crmacctid = q.value("crmacct_id").toInt();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("raitem_id", &valid);
  if (valid)
  {
    _raitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      
      connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));

      q.prepare( "SELECT (COALESCE(MAX(raitem_linenumber), 0) + 1) AS n_linenumber "
                 "FROM raitem "
                 "WHERE (raitem_rahead_id=:rahead_id);" );
      q.bindValue(":rahead_id", _raheadid);
      q.exec();
      if (q.first())
        _lineNumber->setText(q.value("n_linenumber").toString());
      else if (q.lastError().type() == QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      q.prepare( "SELECT rahead_disposition "
                 "FROM rahead "
                 "WHERE (rahead_id=:rahead_id);" );
      q.bindValue(":rahead_id", _raheadid);
      q.exec();
      if (q.first())
      {

        if (q.value("rahead_disposition").toString() == "C")
          sDispositionChanged();
        else if (q.value("rahead_disposition").toString() == "R")
          _disposition->setCurrentItem(1);
        else if (q.value("rahead_disposition").toString() == "P")
          _disposition->setCurrentItem(2);
        else if (q.value("rahead_disposition").toString() == "V")
          _disposition->setCurrentItem(3);
        else if (q.value("rahead_disposition").toString() == "M")
          _disposition->setCurrentItem(4);
      }
      else if (q.lastError().type() == QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }


      _origSoNumber->hide();
      _origSoNumberLit->hide();
      _origSoLineNumber->hide();
      _origSoLineNumberLit->hide();
      _newSoNumber->hide();
      _newSoNumberLit->hide();
      _newSoLineNumber->hide();
      _newSoLineNumberLit->hide();
      _qtySold->hide();
      _qtySoldLit->hide();
      _discountFromSalePrcntLit->hide();
      _discountFromSale->hide();
      _salePrice->hide();
      _salePriceLit->hide();
      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _shipWhs->setEnabled(FALSE);
      _qtyAuth->setFocus();

      connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
 
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _shipWhs->setEnabled(FALSE);
      _disposition->setEnabled(FALSE);
      _qtyAuth->setEnabled(FALSE);
      _qtyUOM->setEnabled(FALSE);
      _netUnitPrice->setEnabled(FALSE);
      _listPrices->setEnabled(FALSE);
      _pricingUOM->setEnabled(FALSE);
      _discountFromSale->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _taxType->setEnabled(FALSE);
      _taxCode->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);
      _altcosAccntid->setEnabled(FALSE);
      _showAvailability->setEnabled(FALSE);
      _createOrder->setEnabled(FALSE);
      _scheduledDate->setEnabled(FALSE);
      _warranty->setEnabled(FALSE);

      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  return NoError; 
}

void returnAuthorizationItem::sSaveClicked()
{
  if (sSave())
  {
    q.exec("COMMIT;");
    done(_raitemid);
  }
}

bool returnAuthorizationItem::sSave()
{ 
  char *dispositionTypes[] = { "C", "R", "P", "V", "S" };
  
  if (!(_scheduledDate->isValid()) && _scheduledDate->isEnabled())
  {
    QMessageBox::warning( this, windowTitle(),
                          tr("<p>You must enter a valid Schedule Date.") );
    _scheduledDate->setFocus();
    return false;
  }
  
  if ( (_coitemid != -1) && (_qtyAuth->toDouble() > _soldQty) )
    if (QMessageBox::question(this, tr("Over Authorize"),
            tr("<p>The authorized quantity exceeds "
                "the original sold quantity on the "
                "original Sales Order.  "
                "Do you want to correct the quantity?"),
               QMessageBox::Yes | QMessageBox::Default,
               QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
  {
      _qtyAuth->setFocus();
      return false;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('raitem_raitem_id_seq') AS _raitem_id");
    if (q.first())
      _raitemid  = q.value("_raitem_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      reject();
    }

    q.prepare( "INSERT INTO raitem "
               "( raitem_id, raitem_rahead_id, raitem_linenumber, raitem_itemsite_id,"
               "  raitem_disposition, raitem_qtyauthorized, "
               "  raitem_qty_uom_id, raitem_qty_invuomratio,"
               "  raitem_price_uom_id, raitem_price_invuomratio,"
               "  raitem_unitprice, raitem_tax_id,raitem_taxtype_id, "
               "  raitem_notes, raitem_rsncode_id, raitem_cos_accnt_id, "
               "  raitem_scheddate, raitem_warranty, raitem_coitem_itemsite_id) "
               "SELECT :raitem_id, :rahead_id, :raitem_linenumber, rcv.itemsite_id,"
               "       :raitem_disposition, :raitem_qtyauthorized,"
               "       :qty_uom_id, :qty_invuomratio,"
               "       :price_uom_id, :price_invuomratio,"
               "       :raitem_unitprice, :raitem_tax_id, :raitem_taxtype_id, "
               "       :raitem_notes, :raitem_rsncode_id, :raitem_cos_accnt_id, "
               "       :raitem_scheddate, :raitem_warranty, "
               "       shp.itemsite_id "
               "FROM itemsite rcv "
               "  LEFT OUTER JOIN itemsite shp ON "
               "        (shp.itemsite_item_id=rcv.itemsite_item_id) "
               "    AND (shp.itemsite_warehous_id=COALESCE(:shipWhs_id,-1)) "
               "WHERE ( (rcv.itemsite_item_id=:item_id)"
               " AND (rcv.itemsite_warehous_id=:warehous_id) );" );
  }
  else
    q.prepare( "UPDATE raitem "
               "SET raitem_disposition=:raitem_disposition, "
               "    raitem_qtyauthorized=:raitem_qtyauthorized, "
               "    raitem_qty_uom_id=:qty_uom_id,"
               "    raitem_qty_invuomratio=:qty_invuomratio,"
               "    raitem_price_uom_id=:price_uom_id,"
               "    raitem_price_invuomratio=:price_invuomratio,"
               "    raitem_unitprice=:raitem_unitprice,"
               "    raitem_tax_id=:raitem_tax_id,"
               "    raitem_taxtype_id=:raitem_taxtype_id, "
               "    raitem_notes=:raitem_notes,"
               "    raitem_rsncode_id=:raitem_rsncode_id, "
               "    raitem_cos_accnt_id=:raitem_cos_accnt_id, "
               "    raitem_scheddate=:raitem_scheddate, "
               "    raitem_warranty=:raitem_warranty "
               "WHERE (raitem_id=:raitem_id);" );

  q.bindValue(":raitem_id", _raitemid);
  q.bindValue(":rahead_id", _raheadid);
  q.bindValue(":raitem_linenumber", _lineNumber->text().toInt());
  q.bindValue(":raitem_qtyauthorized", _qtyAuth->toDouble());
  q.bindValue(":raitem_disposition", QString(dispositionTypes[_disposition->currentItem()]));
  q.bindValue(":qty_uom_id", _qtyUOM->id());
  q.bindValue(":qty_invuomratio", _qtyinvuomratio);
  q.bindValue(":price_uom_id", _pricingUOM->id());
  q.bindValue(":price_invuomratio", _priceinvuomratio);
  q.bindValue(":raitem_unitprice", _netUnitPrice->localValue());
  if (_taxType->isValid())
    q.bindValue(":raitem_taxtype_id", _taxType->id());
  q.bindValue(":raitem_notes", _notes->text());
  if (_taxCode->isValid())
    q.bindValue(":raitem_tax_id", _taxCode->id());
  q.bindValue(":raitem_notes", _notes->text());
  if (_rsnCode->isValid())
    q.bindValue(":raitem_rsncode_id", _rsnCode->id());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  if (_disposition->currentIndex() >= 2)
    q.bindValue(":shipWhs_id", _shipWhs->id());
  if (_altcosAccntid->id() != -1)
    q.bindValue(":raitem_cos_accnt_id", _altcosAccntid->id()); 
  q.bindValue(":raitem_scheddate", _scheduledDate->date());
  q.bindValue(":raitem_warranty",QVariant(_warranty->isChecked(), 0));
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    reject();
  }
  
//  Check to see if a S/O should be re-scheduled
  if (_orderId != -1)
  {
    if (_scheduledDate->date() != _cScheduledDate)
    {
      if (QMessageBox::question(this, tr("Reschedule W/O?"),
                tr("<p>The Scheduled Date for this Line "
                    "Item has been changed.  Should the "
                    "W/O for this Line Item be Re-"
                    "Scheduled to reflect this change?"),
                   QMessageBox::Yes | QMessageBox::Default,
                   QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
      {
        q.prepare("SELECT changeWoDates(:wo_id, :schedDate, :schedDate, TRUE) AS result;");
        q.bindValue(":wo_id", _orderId);
        q.bindValue(":schedDate", _scheduledDate->date());
        q.exec();
        if (q.first())
        {
          int result = q.value("result").toInt();
          if (result < 0)
          {
            systemError(this, storedProcErrorLookup("changeWoDates", result),
            __FILE__, __LINE__);
            reject();
          }
        }
        else if (q.lastError().type() != QSqlError::None)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          reject();
        }
      }
    }
    if (_qtyAuth->toDouble() != _cQtyOrdered)
    {
      if ((_item->itemType() == "M") || (_item->itemType() == "J"))
      {
        if (QMessageBox::question(this, tr("Change W/O Quantity?"),
                  tr("<p>The quantity authorized for this Return "
                     "Authorization Line Item has been changed. "
                     "Should the quantity required for the "
                     "associated Work Order be changed to "
                     "reflect this?"),
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
        {
          q.prepare("SELECT changeWoQty(:wo_id, :qty, TRUE) AS result;");
          q.bindValue(":wo_id", _orderId);
          q.bindValue(":qty", _qtyAuth->toDouble() * _qtyinvuomratio);
          q.exec();
          if (q.first())
          {
            int result = q.value("result").toInt();
            if (result < 0)
            {
              systemError(this, storedProcErrorLookup("changeWoQty", result),
              __FILE__, __LINE__);
              reject();
            }
          }
          else if (q.lastError().type() != QSqlError::None)
          {
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
            reject();
          }
        }
      }
    }
  }
  //If this save has resulted in a link to an shipping S/O, we need to signal that
  if (_disposition->currentItem() > 1)
  {
    XSqlQuery so;
    so.prepare("SELECT raitem_new_coitem_id, cohead_number, "
               " cust_name "
               "FROM raitem,custinfo,cohead,coitem "
               "WHERE ((raitem_id=:raitem_id) "
               "AND (raitem_new_coitem_id IS NOT NULL) "
               "AND (raitem_new_coitem_id=coitem_id) "
               "AND (cust_id=cohead_cust_id) "
               "AND (coitem_cohead_id=cohead_id));");
    so.bindValue(":raitem_id", _raitemid);
    so.exec();
    if (so.first())
    {
      omfgThis->sSalesOrdersUpdated(q.value("rahead_new_cohead_id").toInt());
      //  If requested, create a new W/O or P/R for this coitem
      if ( ( (_mode == cNew) || (_mode == cEdit) ) &&
         (_createOrder->isChecked())               &&
         (_orderId == -1) )
      {
        QString chartype;
        if ((_item->itemType() == "M") || (_item->itemType() == "J"))
        {
          q.prepare( "SELECT createWo(:orderNumber, itemsite_id, :qty, itemsite_leadtime, :dueDate, :comments, :parent_type, :parent_id) AS result, itemsite_id "
                     "FROM itemsite "
                     "WHERE ( (itemsite_item_id=:item_id)"
                     " AND (itemsite_warehous_id=:warehous_id) );" );
          q.bindValue(":orderNumber", so.value("cohead_number").toInt());
          q.bindValue(":qty", _orderQty->text().toDouble());
          q.bindValue(":dueDate", _scheduledDate->date());
          q.bindValue(":comments", so.value("cust_name").toString() + "\n" + _notes->text());
          q.bindValue(":item_id", _item->id());
          q.bindValue(":warehous_id", _shipWhs->id());
          q.bindValue(":parent_type", QString("S"));
          q.bindValue(":parent_id", so.value("raitem_new_coitem_id").toInt());
          q.exec();
        }
        if (q.first())
        {
          _orderId = q.value("result").toInt();
          if (_orderId < 0)
          {
            QString procname;
            if ((_item->itemType() == "M") || (_item->itemType() == "J"))
              procname = "createWo";
            else
              procname = "unnamed stored procedure";
            systemError(this, storedProcErrorLookup(procname, _orderId),
                __FILE__, __LINE__);
            reject();
          }

          if ((_item->itemType() == "M") || (_item->itemType() == "J"))
          {
            omfgThis->sWorkOrdersUpdated(_orderId, TRUE);

    //  Update the newly created coitem with the newly create wo_id
            q.prepare( "UPDATE coitem "
                       "SET coitem_order_type='W', coitem_order_id=:orderid "
                       "WHERE (coitem_id=:soitem_id);" );
            q.bindValue(":orderid", _orderId);
            q.bindValue(":soitem_id", so.value("raitem_new_coitem_id").toInt());
            q.exec();
            if (q.lastError().type() != QSqlError::None)
            {
              systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
              reject();
            }
          }
        }
      }
    }
  }
  _mode = cEdit;
  return true; 
}

void returnAuthorizationItem::sPopulateItemInfo()
{
  XSqlQuery uom;
  uom.prepare("SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN uom ON (item_inv_uom_id=uom_id)"
              " WHERE(item_id=:item_id)"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_to_uom_id=uom_id)"
              " WHERE((itemuomconv_from_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_from_uom_id=uom_id)"
              " WHERE((itemuomconv_to_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " ORDER BY uom_name;");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  _qtyUOM->populate(uom);
  _pricingUOM->populate(uom);

  XSqlQuery item;
  item.prepare( "SELECT item_inv_uom_id, item_price_uom_id,"
                "       iteminvpricerat(item_id) AS iteminvpricerat, formatUOMRatio(iteminvpricerat(item_id)) AS f_invpricerat,"
                "       item_listprice, "
                "       stdCost(item_id) AS f_cost,"
                "       getItemTaxType(item_id, :taxauth) AS taxtype_id "
                "  FROM item"
                " WHERE (item_id=:item_id);" );
  item.bindValue(":item_id", _item->id());
  item.bindValue(":taxauth", _taxauthid);
  item.exec();
  if (item.first())
  {
    _priceRatio = item.value("iteminvpricerat").toDouble();
    _qtyUOM->setId(item.value("item_inv_uom_id").toInt());
    _pricingUOM->setId(item.value("item_price_uom_id").toInt());
    _priceinvuomratio = item.value("iteminvpricerat").toDouble();
    _qtyinvuomratio = 1.0;
    _invuomid = item.value("item_inv_uom_id").toInt();
    _listPrice->setBaseValue(item.value("item_listprice").toDouble());
	_unitCost->setBaseValue(item.value("f_cost").toDouble());
    _taxType->setId(item.value("taxtype_id").toInt()); 
  }
  else if (item.lastError().type() != QSqlError::None)
  {
    systemError(this, item.lastError().databaseText(), __FILE__, __LINE__);
    return;
  } 

  if (_item->itemType() == "M")
    _createOrder->setEnabled(TRUE);

  else if (_item->itemType() == "J")
  {
    _createOrder->setChecked(TRUE);
    _createOrder->setEnabled(FALSE);
  }

  else
    _createOrder->setEnabled(FALSE);
}

void returnAuthorizationItem::sPopulateItemsiteInfo()
{
  if (_item->isValid())
  {
    XSqlQuery itemsite;
    itemsite.prepare( "SELECT itemsite_leadtime, itemsite_controlmethod, "
                      "       itemsite_createwo, itemsite_createpr "
                      "FROM item, itemsite "
                      "WHERE ( (itemsite_item_id=item_id)"
                      " AND (itemsite_warehous_id=:warehous_id)"
                      " AND (item_id=:item_id) );" );
    itemsite.bindValue(":warehous_id", _shipWhs->id());
    itemsite.bindValue(":item_id", _item->id());
    itemsite.exec();
    if (itemsite.first())
    {
      _leadTime = itemsite.value("itemsite_leadtime").toInt();

      if (cNew == _mode)
      {
        if (_item->itemType() == "M")
          _createOrder->setChecked(itemsite.value("itemsite_createwo").toBool());

        else if (_item->itemType() == "J")
        {
          _createOrder->setChecked(TRUE);
          _createOrder->setEnabled(FALSE);
        }

        else
        {
          _createOrder->setChecked(FALSE);
          _createOrder->setEnabled(FALSE);
        }
      }
      
       _tab->setTabEnabled(_tab->indexOf(_lotserial),
       (itemsite.value("itemsite_controlmethod").toString() == "L" ||
        itemsite.value("itemsite_controlmethod").toString() == "S"));
    }
    else if (itemsite.lastError().type() != QSqlError::None)
    {
      systemError(this, itemsite.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void returnAuthorizationItem::populate()
{
  XSqlQuery raitem;
  raitem.prepare("SELECT rahead_number, rahead_curr_id, rahead_authdate, rahead_taxauth_id, raitem.*, "
                 "       och.cohead_number AS orig_number, oc.coitem_linenumber AS orig_linenumber, "
                 "       nch.cohead_number AS new_number, nc.coitem_linenumber AS new_linenumber, "
                 "       COALESCE(raitem_orig_coitem_id,-1) AS ra_coitem_id, oc.coitem_price, "
                 "       formatQty(oc.coitem_qtyshipped) AS qtysold,"   
                 "       formatQty(raitem_qtyauthorized) AS qtyauth,"
                 "       formatQty(raitem_qtyreceived) AS qtyrcvd,"
                 "       formatQty(nc.coitem_qtyshipped) AS qtyshipd,"
                 "       rahead_taxauth_id,"
                 "       rahead_curr_id AS taxcurr, "
                 "       item_inv_uom_id, "
                 "       COALESCE(nc.coitem_order_id,-1) AS coitem_order_id, "
                 "       nc.coitem_order_type AS coitem_order_type, "
                 "       rcv.itemsite_warehous_id AS itemsite_warehous_id, "
                 "       shp.itemsite_warehous_id AS shipWhs_id, "
                 "       qtyToReceive('RA', raitem_id) AS qtytorcv, "
                 "       crmacct_id "
                 "FROM raitem "
                 "  LEFT OUTER JOIN coitem oc ON (raitem_orig_coitem_id=oc.coitem_id) "
                 "  LEFT OUTER JOIN cohead och ON (oc.coitem_cohead_id=och.cohead_id) "
                 "  LEFT OUTER JOIN coitem nc ON (raitem_new_coitem_id=nc.coitem_id) "
                 "  LEFT OUTER JOIN cohead nch ON (nc.coitem_cohead_id=nch.cohead_id)"
                 "  LEFT OUTER JOIN itemsite shp ON (raitem_coitem_itemsite_id=shp.itemsite_id),"
                 "  rahead, itemsite rcv, item, crmacct "
                 "WHERE ((raitem_rahead_id=rahead_id)"
                 " AND  (raitem_id=:raitem_id) "
                 " AND  (raitem_itemsite_id=rcv.itemsite_id) "
                 " AND  (item_id=rcv.itemsite_item_id) "
                 " AND  (rahead_cust_id=crmacct_cust_id) );" );
  raitem.bindValue(":raitem_id", _raitemid);
  raitem.exec();
  if (raitem.first())
  {
    _authNumber->setText(raitem.value("rahead_number").toString());

    if (raitem.value("new_number").toInt() > 0)
    {
      _newSoNumber->setText(raitem.value("new_number").toString());
      _newSoLineNumber->setText(raitem.value("new_linenumber").toString());
    }
    else
    {
      _newSoNumberLit->hide();
      _newSoLineNumberLit->hide();
      _newSoNumber->hide();
      _newSoLineNumber->hide();
    }
    if (raitem.value("raitem_disposition").toString() == "C")
      sDispositionChanged();
    else if (raitem.value("raitem_disposition").toString() == "R")
      _disposition->setCurrentItem(1);
    else if (raitem.value("raitem_disposition").toString() == "P")
      _disposition->setCurrentItem(2);
    else if (raitem.value("raitem_disposition").toString() == "V")
      _disposition->setCurrentItem(3);
    else if (raitem.value("raitem_disposition").toString() == "S")
      _disposition->setCurrentItem(4);
	  _orderId = raitem.value("coitem_order_id").toInt();
    _netUnitPrice->setId(raitem.value("rahead_curr_id").toInt());
    _netUnitPrice->setEffective(raitem.value("rahead_authdate").toDate());
    _netUnitPrice->setLocalValue(raitem.value("raitem_unitprice").toDouble());
    // do _item and _taxauth before other tax stuff because of signal cascade
    _taxauthid = raitem.value("rahead_taxauth_id").toInt();
    _item->setItemsiteid(raitem.value("raitem_itemsite_id").toInt());
    _warehouse->setId(raitem.value("itemsite_warehous_id").toInt());
    _shipWhs->setId(raitem.value("shipWhs_id").toInt());
    _taxType->setId(raitem.value("raitem_taxtype_id").toInt());
    _tax->setId(raitem.value("taxcurr").toInt());
    _invuomid=raitem.value("item_inv_uom_id").toInt();
    _qtyUOM->setId(raitem.value("raitem_qty_uom_id").toInt());
    _pricingUOM->setId(raitem.value("raitem_price_uom_id").toInt());
    _qtyinvuomratio = raitem.value("raitem_qty_invuomratio").toDouble();
    _priceinvuomratio = raitem.value("raitem_price_invuomratio").toDouble();
    _lineNumber->setText(raitem.value("raitem_linenumber").toString());
    _qtyAuth->setText(raitem.value("qtyauth").toString());
    _qtyReceived->setText(raitem.value("qtyrcvd").toString());
    _qtyShipped->setText(raitem.value("qtyshipd").toString());
    _notes->setText(raitem.value("raitem_notes").toString());
    _taxCode->setId(raitem.value("raitem_tax_id").toInt());
    _rsnCode->setId(raitem.value("raitem_rsncode_id").toInt());
    _altcosAccntid->setId(raitem.value("raitem_cos_accnt_id").toInt());
    _scheduledDate->setDate(raitem.value("raitem_scheddate").toDate());
    _warranty->setChecked(raitem.value("raitem_warranty").toBool());
    _status = raitem.value("raitem_status").toString();
    _qtycredited = raitem.value("raitem_qtycredited").toDouble();

    _cQtyOrdered = _qtyAuth->toDouble();
    _cScheduledDate = _scheduledDate->date();
    _crmacctid = raitem.value("crmacct_id").toInt();

    _coitemid = raitem.value("ra_coitem_id").toInt();
    if (_coitemid != -1)
    {
      _origSoNumber->setText(raitem.value("orig_number").toString());
      _origSoLineNumber->setText(raitem.value("orig_linenumber").toString());
      _qtySold->setText(raitem.value("qtysold").toString());
      _soldQty=raitem.value("qtysold").toDouble();
      _qtyUOM->setEnabled(FALSE);
      _pricingUOM->setEnabled(FALSE);
      _salePrice->setId(raitem.value("rahead_curr_id").toInt());
      _salePrice->setEffective(raitem.value("rahead_authdate").toDate());
      _salePrice->setLocalValue(raitem.value("coitem_price").toDouble());
    }
    else
    {
      _origSoNumber->hide();
      _origSoNumberLit->hide();
      _origSoLineNumber->hide();
      _origSoLineNumberLit->hide();
      _discountFromSalePrcntLit->hide();
      _discountFromSale->hide();
      _qtySold->hide();
      _qtySoldLit->hide();
      _salePrice->hide();
      _salePriceLit->hide();
    }
    sDetermineAvailability();
    
    
    
    if (raitem.value("qtyrcvd").toDouble() > 0 || 
        raitem.value("qtyshipd").toDouble() > 0 ||
		raitem.value("qtytorcv").toDouble() > 0 ||
        _qtycredited > 0)
      _disposition->setEnabled(FALSE); 

    if (_orderId != -1)
    {
      XSqlQuery query;

      if (raitem.value("coitem_order_type").toString() == "W")
      {
        query.prepare( "SELECT wo_status,"
                       "       formatQty(wo_qtyord) AS f_qty,"
                       "       wo_duedate, warehous_id, warehous_code "
                       "FROM wo, itemsite, warehous "
                       "WHERE ((wo_itemsite_id=itemsite_id)"
                       " AND (itemsite_warehous_id=warehous_id)"
                       " AND (wo_id=:wo_id));" );
        query.bindValue(":wo_id", _orderId);
        query.exec();
        if (query.first())
        {
          _createOrder->setChecked(TRUE);

          _orderQty->setText(query.value("f_qty").toString());
          _orderDueDate->setDate(query.value("wo_duedate").toDate());
          _orderStatus->setText(query.value("wo_status").toString());

          if ((query.value("wo_status").toString() == "R") || (query.value("wo_status").toString() == "C"))
            _createOrder->setEnabled(FALSE);
        }
        else
        {
          _orderId = -1;
          _createOrder->setChecked(FALSE);
        }
      }
    }
  }
  else if (raitem.lastError().type() != QSqlError::None)
  {
    systemError(this, raitem.lastError().databaseText(), __FILE__, __LINE__);
    return;
  } 
  sFillList();
}

void returnAuthorizationItem::sCalculateExtendedPrice()
{
  _extendedPrice->setLocalValue(((_qtyAuth->toDouble() * _qtyinvuomratio) / _priceinvuomratio) * _netUnitPrice->localValue());
}

void returnAuthorizationItem::sCalculateDiscountPrcnt()
{
  double unitPrice = _netUnitPrice->localValue();

  if (unitPrice == 0.0)
  {
    _discountFromList->setText("N/A");
    _discountFromSale->setText("N/A");
  }
  else
  {
    if (_listPrice->isZero())
      _discountFromList->setText("N/A");
    else
      _discountFromList->setText(formatPercent(1 - (unitPrice / _listPrice->localValue())) );

    if (_salePrice->isZero())
      _discountFromSale->setText("N/A");
    else
      _discountFromSale->setText(formatPercent(1 - (unitPrice / _salePrice->localValue())) );
  }
}

void returnAuthorizationItem::sCalculateFromDiscount()
{
  double discount = _discountFromSale->toDouble() / 100.0;

  if (_salePrice->isZero())
    _discountFromSale->setText(tr("N/A"));
  else
    _netUnitPrice->setLocalValue(_salePrice->localValue() - (_salePrice->localValue() * discount));
}

void returnAuthorizationItem::sPriceGroup()
{
  if (! omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_netUnitPrice->currAbbr())); 
}

void returnAuthorizationItem::sListPrices()
{
  q.prepare( "SELECT formatSalesPrice(currToCurr(ipshead_curr_id, :curr_id, ipsprice_price, :effective)) AS price"
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) = 0)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1) ) )"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_shipto_id=:shipto_id)"
             "        AND (ipsass_shipto_id != -1)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_custtype_id=cust_custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, custtype, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
             "        AND (custtype_code ~ ipsass_custtype_pattern)"
             "        AND (cust_custtype_id=custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)))"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, shipto "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (shipto_id=:shipto_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) > 0)"
             "        AND (shipto_num ~ ipsass_shipto_pattern)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM sale, ipshead, ipsprice "
             "       WHERE ((sale_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (CURRENT_DATE BETWEEN sale_startdate AND (sale_enddate - 1)) ) "

             "       UNION SELECT formatSalesPrice(item_listprice - (item_listprice * cust_discntprcnt)) AS price "
             "       FROM item, cust "
             "       WHERE ( (item_sold)"
             "        AND (NOT item_exclusive)"
             "        AND (item_id=:item_id)"
             "        AND (cust_id=:cust_id) );");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":cust_id", _custid);
  q.bindValue(":shipto_id", _shiptoid);
  q.bindValue(":curr_id", _netUnitPrice->id());
  q.bindValue(":effective", _netUnitPrice->effective());
  q.exec();
  if (q.size() == 1)
  {
    q.first();
    _netUnitPrice->setLocalValue(q.value("price").toDouble() * (_priceRatio / _priceinvuomratio));
  }
  else
  {
    ParameterList params;
    params.append("cust_id", _custid);
    params.append("shipto_id", _shiptoid);
    params.append("item_id", _item->id());
    // don't params.append("qty", ...) as we don't know how many were purchased
    params.append("curr_id", _netUnitPrice->id());
    params.append("effective", _netUnitPrice->effective());

    priceList newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Accepted)
    {
      _netUnitPrice->setLocalValue(newdlg._selectedPrice * (_priceRatio / _priceinvuomratio));
      sCalculateDiscountPrcnt();
    }
  }
}

void returnAuthorizationItem::sLookupTax()
{
  XSqlQuery calcq;

  calcq.prepare("SELECT calculateTax(:tax_id, :ext, 0, 'A') AS valA,"
                "               calculateTax(:tax_id, :ext, 0, 'B') AS valB,"
                "               calculateTax(:tax_id, :ext, 0, 'C') AS valC;");

  calcq.bindValue(":extcurr", _extendedPrice->id());
  calcq.bindValue(":tax_id",  _taxCode->id());
  calcq.bindValue(":ext",     _extendedPrice->localValue());
  calcq.exec();
  if (calcq.first())
  {
    _taxCache.setLine(calcq.value("valA").toDouble(),
                      calcq.value("valB").toDouble(),
                      calcq.value("valC").toDouble());
    _tax->setLocalValue(_taxCache.total());
  }
  else if (calcq.lastError().type() != QSqlError::None)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  } 
}

void returnAuthorizationItem::sTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("tax_id",   _taxCode->id());
  params.append("curr_id",  _tax->id());
  params.append("valueA",   _taxCache.line(0));
  params.append("valueB",   _taxCache.line(1));
  params.append("valueC",   _taxCache.line(2));
  params.append("pctA",     _taxCache.linePct(0));
  params.append("pctB",     _taxCache.linePct(1));
  params.append("pctC",     _taxCache.linePct(2));
  params.append("subtotal", CurrDisplay::convert(_extendedPrice->id(), _tax->id(),
                                                 _extendedPrice->localValue(),
                                                 _extendedPrice->effective()));
  if (cView == _mode)
    params.append("readOnly");

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    _taxCache.setLine(newdlg.amountA(), newdlg.amountB(), newdlg.amountC());
    _taxCache.setLinePct(newdlg.pctA(), newdlg.pctB(),    newdlg.pctC());

    if (_taxCode->id() != newdlg.tax())
      _taxCode->setId(newdlg.tax());

    _tax->setLocalValue(_taxCache.total());
  } 
}

void returnAuthorizationItem::sLookupTaxCode()
{ 
  XSqlQuery taxq;
  taxq.prepare("SELECT tax_ratea, tax_rateb, tax_ratec, tax_id "
               "FROM tax "
               "WHERE (tax_id=getTaxSelection(:auth, :type));");
  taxq.bindValue(":auth",    _taxauthid);
  taxq.bindValue(":type",    _taxType->id());
  taxq.exec();
  if (taxq.first())
  {
    _taxCache.setLinePct(taxq.value("tax_ratea").toDouble(),
                         taxq.value("tax_rateb").toDouble(),
                         taxq.value("tax_ratec").toDouble());
    _taxCode->setId(taxq.value("tax_id").toInt());
  }
  else if (taxq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _taxCode->setId(-1); 
}

void returnAuthorizationItem::sQtyUOMChanged()
{
  if(_qtyUOM->id() == _invuomid || _item->id() == -1)
    _qtyinvuomratio = 1.0;
  else
  {
    XSqlQuery invuom;
    invuom.prepare("SELECT itemuomtouomratio(item_id, :uom_id, item_inv_uom_id) AS ratio"
                   "  FROM item"
                   " WHERE(item_id=:item_id);");
    invuom.bindValue(":item_id", _item->id());
    invuom.bindValue(":uom_id", _qtyUOM->id());
    invuom.exec();
    if(invuom.first())
      _qtyinvuomratio = invuom.value("ratio").toDouble();
    else
      systemError(this, invuom.lastError().databaseText(), __FILE__, __LINE__);
  }

  if(_qtyUOM->id() != _invuomid)
  {
    _pricingUOM->setId(_qtyUOM->id());
    _pricingUOM->setEnabled(false);
  }
  else
    _pricingUOM->setEnabled(true);
  sCalculateExtendedPrice(); 
}

void returnAuthorizationItem::sPriceUOMChanged()
{
  if(_pricingUOM->id() == -1 || _qtyUOM->id() == -1)
    return;

  if(_pricingUOM->id() == _invuomid)
    _priceinvuomratio = 1.0;
  else
  {
    XSqlQuery invuom;
    invuom.prepare("SELECT itemuomtouomratio(item_id, :uom_id, item_inv_uom_id) AS ratio"
                   "  FROM item"
                   " WHERE(item_id=:item_id);");
    invuom.bindValue(":item_id", _item->id());
    invuom.bindValue(":uom_id", _pricingUOM->id());
    invuom.exec();
    if(invuom.first())
      _priceinvuomratio = invuom.value("ratio").toDouble();
    else
      systemError(this, invuom.lastError().databaseText(), __FILE__, __LINE__);
  }
  //_ratio->setText(formatUOMRatio(_priceinvuomratio));

  updatePriceInfo(); 
}

void returnAuthorizationItem::updatePriceInfo()
{
  XSqlQuery item;
  item.prepare("SELECT item_listprice"
               "  FROM item"
               " WHERE(item_id=:item_id);");
  item.bindValue(":item_id", _item->id());
  item.exec();
  item.first();
  _listPrice->setBaseValue((item.value("item_listprice").toDouble() * _priceRatio)  * _priceinvuomratio);
  sCalculateExtendedPrice();
} 

void returnAuthorizationItem::sDispositionChanged()
{
  if ( (_disposition->currentIndex() == 3) && 
          (_item->id() != -1) &&
	  (_item->itemType() != "J") )
  {
    QMessageBox::warning( this, tr("Cannot use Service Disposition"),
      tr("<p>Only Items of type Job may have a disposition of Service.") );
	_disposition->setFocus();
	_disposition->setCurrentIndex(_dispositionCache);
	return;
  }

  if ( (_disposition->currentIndex() < 2) && 
	  (_orderId != -1) )
  {
    QMessageBox::warning( this, tr("Cannot change Disposition"),
      tr("<p>A work order is associated with this Return. "
	     "First delete the work order, then change this disposition.") );
	_disposition->setFocus();
	_disposition->setCurrentIndex(_dispositionCache);
	return;
  }

  if (_disposition->currentIndex() == 3)
  _item->setQuery( QString( "SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                              "                uom_name, item_type, item_config "
                              "FROM item, itemsite, uom "
                              "WHERE ( (itemsite_item_id=item_id)"
                              " AND (item_inv_uom_id=uom_id)"
                              " AND (itemsite_active)"
                              " AND (item_active)"
							  " AND (item_type = 'J')"
                              " AND (customerCanPurchase(item_id, %1, %2)) ) "
                              "ORDER BY item_number;" )
                     .arg(_custid).arg(_shiptoid) );
  else
  _item->setQuery( QString( "SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                              "                uom_name, item_type, item_config "
                              "FROM item, itemsite, uom "
                              "WHERE ( (itemsite_item_id=item_id)"
                              " AND (item_inv_uom_id=uom_id)"
                              " AND (itemsite_active)"
                              " AND (item_active)"
                              " AND (customerCanPurchase(item_id, %1, %2)) ) "
                              "ORDER BY item_number;" )
                     .arg(_custid).arg(_shiptoid) );

  if (_disposition->currentIndex() > 2)
  {
    _netUnitPrice->setLocalValue(0);
    _netUnitPrice->setEnabled(FALSE);
    _listPrices->setEnabled(FALSE);
    _pricingUOM->setEnabled(FALSE);
    _discountFromSale->setEnabled(FALSE);
  }
  else
  {
    _netUnitPrice->setEnabled(TRUE);
    _listPrices->setEnabled(TRUE);
    _pricingUOM->setEnabled(TRUE);
    _discountFromSale->setEnabled(TRUE);
  } 
  
  if (_disposition->currentIndex() >= 2)
  {
    _tab->setTabEnabled(_tab->indexOf(_supply),TRUE);
    _scheduledDate->setEnabled(TRUE);
    _altcosAccntid->setEnabled(TRUE);
    _shipWhsLit->setVisible(TRUE);
    _shipWhs->setVisible(TRUE);
  }
  else
  {
    _tab->setTabEnabled(_tab->indexOf(_supply),FALSE);
    _scheduledDate->clear();
    _scheduledDate->setEnabled(FALSE);
    _altcosAccntid->setEnabled(FALSE);
    _shipWhsLit->setVisible(FALSE);
    _shipWhs->setVisible(FALSE);
  } 

  if (_creditmethod == "N")
  {
    _netUnitPrice->setLocalValue(0);
    _netUnitPrice->setEnabled(FALSE);
    _listPrices->setEnabled(FALSE);
    _pricingUOM->setEnabled(FALSE);
    _discountFromSale->setEnabled(FALSE); 
    disconnect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));
  }

  _dispositionCache = _disposition->currentIndex();
}

void returnAuthorizationItem::sHandleWo(bool pCreate)
{
  if (pCreate)
  {
    if (_orderId == -1)
      sPopulateOrderInfo();
  }

  else
  {
    if (_orderId != -1)
    {
      XSqlQuery query;

      if ((_item->itemType() == "M") || (_item->itemType() == "J"))
      {
        if (QMessageBox::question(this, tr("Delete Work Order"),
                                  tr("<p>You are requesting to delete the Work "
                                     "Order created for the Sales Order Item linked to this Return. "
                                     "Are you sure you want to do this?"),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
        {
          query.prepare("UPDATE coitem SET coitem_order_id=-1,coitem_order_type=NULL "
                        "FROM wo "
                        "WHERE ((coitem_id=wo_ordid) "
                        "AND (wo_id=:wo_id)); "
                        "SELECT deleteWo(:wo_id, TRUE) AS result;");
          query.bindValue(":wo_id", _orderId);
          query.exec();
          if (query.first())
          {
            int result = query.value("result").toInt();
            if (result < 0)
            {
              systemError(this, storedProcErrorLookup("deleteWo", result),
                          __FILE__, __LINE__);
              _createOrder->setChecked(true); // if (pCreate) => won't recurse
              return;
            }
            else
            {
              _orderId = -1;
              _orderQty->clear();
              _orderDueDate->clear();
              _orderStatus->clear();
              if (_qtyReceived->text().toDouble() == 0 && 
                  _qtyShipped->text().toDouble() == 0 && 
                  _qtycredited == 0 &&
                  _status == "O")
                _disposition->setEnabled(TRUE); 

              _createOrder->setChecked(FALSE);
            }
          }
          else if (query.lastError().type() != QSqlError::NoError)
          {
            systemError(this, query.lastError().databaseText(),
                        __FILE__, __LINE__);
            _createOrder->setChecked(true); // if (pCreate) => won't recurse
            return;
          }

          omfgThis->sWorkOrdersUpdated(-1, TRUE);
        }
        else
          _createOrder->setChecked(TRUE);
      }
    }
  }
}

void returnAuthorizationItem::sPopulateOrderInfo()
{
  if (_createOrder->isChecked())
  {
    _orderDueDate->setDate(_scheduledDate->date());

    if (_createOrder->isChecked())
    {
      XSqlQuery qty;
      qty.prepare( "SELECT validateOrderQty(itemsite_id, :qty, TRUE) AS qty "
                   "FROM itemsite "
                   "WHERE ((itemsite_item_id=:item_id)"
                   " AND (itemsite_warehous_id=:warehous_id));" );
      qty.bindValue(":qty", _qtyAuth->toDouble() * _qtyinvuomratio);
      qty.bindValue(":item_id", _item->id());
      qty.bindValue(":warehous_id", ((_item->itemType() == "M") || (_item->itemType() == "J") ? _shipWhs->id() : _shipWhs->id()));
      qty.exec();
      if (qty.first())
        _orderQty->setText(qty.value("qty").toString());

      else if (qty.lastError().type() != QSqlError::None)
      {
        systemError(this, qty.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }
}

void returnAuthorizationItem::sDetermineAvailability()
{
  if(  (_item->id()==_availabilityLastItemid)
    && (_shipWhs->id()==_availabilityLastWarehousid)
    && (_scheduledDate->date()==_availabilityLastSchedDate)
    && (_showAvailability->isChecked()==_availabilityLastShow)
    && ((_qtyAuth->toDouble() * _qtyinvuomratio)==_availabilityQtyOrdered) )
    return;

  _availabilityLastItemid = _item->id();
  _availabilityLastWarehousid = _shipWhs->id();
  _availabilityLastSchedDate = _scheduledDate->date();
  _availabilityLastShow = _showAvailability->isChecked();
  _availabilityQtyOrdered = (_qtyAuth->toDouble() * _qtyinvuomratio);

  if ((_item->isValid()) && (_scheduledDate->isValid()) && (_showAvailability->isChecked()) )
  {
    XSqlQuery availability;
    availability.prepare( "SELECT itemsite_id,"
                          "       formatQty(qoh) AS f_qoh,"
                          "       formatQty(allocated) AS f_allocated,"
                          "       formatQty(noNeg(qoh - allocated)) AS f_unallocated,"
                          "       formatQty(ordered) AS f_ordered,"
                          "       (qoh - allocated + ordered) AS available,"
                          "       formatQty(qoh - allocated + ordered) AS f_available, "
                          "       itemsite_leadtime "
                          "FROM ( SELECT itemsite_id, itemsite_qtyonhand AS qoh,"
                          "              qtyAllocated(itemsite_id, DATE(:date)) AS allocated,"
                          "              qtyOrdered(itemsite_id, DATE(:date)) AS ordered, "
                          "              itemsite_leadtime "
                          "       FROM itemsite, item "
                          "       WHERE ((itemsite_item_id=item_id)"
                          "        AND (item_id=:item_id)"
                          "        AND (itemsite_warehous_id=:warehous_id)) ) AS data;" );
    availability.bindValue(":date", _scheduledDate->date());
    availability.bindValue(":item_id", _item->id());
    availability.bindValue(":warehous_id", _shipWhs->id());
    availability.exec();
    if (availability.first())
    {
      _onHand->setText(availability.value("f_qoh").toString());
      _allocated->setText(availability.value("f_allocated").toString());
      _unallocated->setText(availability.value("f_unallocated").toString());
      _onOrder->setText(availability.value("f_ordered").toString());
      _available->setText(availability.value("f_available").toString());
      _leadtime->setText(availability.value("itemsite_leadtime").toString());

      if (availability.value("available").toDouble() < _availabilityQtyOrdered)
        _available->setPaletteForegroundColor(QColor("red"));
      else
        _available->setPaletteForegroundColor(QColor("black"));
    }
    else if (availability.lastError().type() != QSqlError::None)
    {
      systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _onHand->clear();
    _allocated->clear();
    _unallocated->clear();
    _onOrder->clear();
    _available->clear();
    _leadtime->clear();
  }
}

void returnAuthorizationItem::sCalcWoUnitCost()
{
  if (_item->itemType() == "J" && _orderId > -1 && _qtyAuth->toDouble() != 0)
  {
    q.prepare("SELECT COALESCE(SUM(wo_postedvalue),0) AS wo_value "
		      "FROM wo,raitem "
			  "WHERE ((wo_ordtype='S') "
			  "AND (wo_ordid=raitem_new_coitem_id) "
			  "AND (raitem_id=:raitem_id));");
	q.bindValue(":raitem_id", _raitemid);
	q.exec();
	if (q.first())
      _unitCost->setBaseValue(q.value("wo_value").toDouble() / _qtyAuth->toDouble() * _qtyinvuomratio);
  }
}

void returnAuthorizationItem::sNew()
{
	if (sSave())
	{
		ParameterList params;
		params.append("raitem_id", _raitemid);
		params.append("item_id", _item->id());
		params.append("warehouse_id", _warehouse->id());
		params.append("uom", _qtyUOM->currentText());
		params.append("mode", "new");

		returnAuthItemLotSerial newdlg(this, "", TRUE);
		newdlg.set(params);
		int raitemlsid = newdlg.exec();
		if (raitemlsid > 0)
			populate();
		else if (raitemlsid == -1)
			reject();
	}
}

void returnAuthorizationItem::sEdit()
{
	if (sSave())
	{
		bool fill;
		fill = FALSE;
		QList<QTreeWidgetItem*> selected = _raitemls->selectedItems();
		for (int i = 0; i < selected.size(); i++)
		{
			ParameterList params;
                        params.append("raitem_id", _raitemid);			
                        params.append("ls_id", ((XTreeWidgetItem*)(selected[i]))->altId());

			if (_mode==cView)
				params.append("mode", "view");
			else
				params.append("mode", "edit");

			returnAuthItemLotSerial newdlg(this, "", TRUE);
			newdlg.set(params);
	  
			int raitemlsid = newdlg.exec();
			if (raitemlsid > 0)
				fill = TRUE;
			else if (raitemlsid == -1)
				reject();
		}
		if (fill)
			populate();
	}
}

void returnAuthorizationItem::sDelete()
{
  QList<QTreeWidgetItem*> selected = _raitemls->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
        QString sql ( "DELETE FROM raitemls "
			"WHERE (raitemls_id=:raitemls_id);" );
        q.prepare(sql);
        q.bindValue(":raitemls_id",  ((XTreeWidgetItem*)(selected[i]))->id());
        q.exec();
        if (q.lastError().type() != QSqlError::NoError)
        {
           systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
           reject();
        }
  }
  populate(); 
}

void returnAuthorizationItem::sFillList()
{ 
  q.prepare("SELECT raitemls_id,ls_id,ls_number, "
		" MAX(lsreg_expiredate), formatqty(COALESCE(SUM(lsreg_qty / raitem_qty_invuomratio),0)), formatqty(raitemls_qtyauthorized), "
		" formatqty(raitemls_qtyreceived) "
		"FROM raitemls "
		"  LEFT OUTER JOIN lsreg ON (lsreg_ls_id=raitemls_ls_id) "
                "                       AND (lsreg_crmacct_id=:crmacct_id), "
		"  ls, raitem "
		"WHERE ((raitemls_raitem_id=raitem_id) "
                "AND (raitem_id=:raitem_id) "
		"AND (raitemls_ls_id=ls_id) ) "
                "GROUP BY raitemls_id,ls_id,ls_number,raitemls_qtyauthorized,"
                "  raitemls_qtyreceived "
                "ORDER BY ls_number;" );
  q.bindValue(":raitem_id", _raitemid);
  q.bindValue(":crmacct_id", _crmacctid);
  q.bindValue(":na", tr("N/A"));
  q.exec();
  _authLotSerial->setDisabled(q.first());
  _authLotSerial->setChecked(q.first());
  _raitemls->populate(q,true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    reject();
  }
}

void returnAuthorizationItem::rejectEvent()
{
  q.exec("ROLLBACK");
}



