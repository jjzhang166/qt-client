/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
*/

#include "returnAuthorizationItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"
#include "errorReporter.h"

#include "priceList.h"
#include "taxDetail.h"
#include "storedProcErrorLookup.h"
#include "returnAuthItemLotSerial.h"

returnAuthorizationItem::returnAuthorizationItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  XSqlQuery returnreturnAuthorizationItem;
  setupUi(this);

#ifndef Q_OS_MAC
  _listPrices->setMaximumWidth(25);
  _saleListPrices->setMaximumWidth(25);
#endif

  _coitemid                = -1;
  _coitemitemsiteid        = -1;
  _costmethod              = "";
  _creditmethod            = "";
  _crmacctid               = -1;
  _custid                  = -1;
  _dispositionCache        = -1;
  _invuomid                = -1;
  _mode                    = cNew;
  _orderId                 = -1;
  _origsoid                = -1;
  _preferredShipWarehousid = -1;
  _preferredWarehousid     = -1;
  _priceRatio              = 1.0;
  _priceinvuomratio        = 1.0;
  _qtyAuthCache            = 0.0;
  _qtycredited             = 0;
  _qtyinvuomratio          = 1.0;
  _raheadid                = -1;
  _raitemid                = -1;
  _shiptoid                = -1;
  _soldQty                 = 0;
  _status                  = "O";
  _taxzoneid               = -1;
  _unitcost                = 0;

  // order and id are important. don't change them without makeing adjustments below
  _disposition->append(0, tr("Credit"),  "C");
  _disposition->append(1, tr("Return"),  "R");
  _disposition->append(2, tr("Replace"), "P");
  _disposition->append(3, tr("Service"), "V");
  _disposition->append(4, tr("Ship"),    "S");

  connect(_discountFromSale,     SIGNAL(editingFinished()),                    this, SLOT(sCalculateFromDiscount()));
  connect(_saleDiscountFromSale, SIGNAL(editingFinished()),                    this, SLOT(sCalculateSaleFromDiscount()));
  connect(_extendedPrice,        SIGNAL(valueChanged()),                 this, SLOT(sCalculateTax()));
  connect(_item,                 SIGNAL(newId(int)),                     this, SLOT(sPopulateItemInfo()));
  connect(_listPrices,           SIGNAL(clicked()),                      this, SLOT(sListPrices()));
  connect(_saleListPrices,       SIGNAL(clicked()),                      this, SLOT(sSaleListPrices()));
  connect(_netUnitPrice,         SIGNAL(valueChanged()),                 this, SLOT(sCalculateDiscountPrcnt()));
  connect(_netUnitPrice,         SIGNAL(valueChanged()),                 this, SLOT(sCalculateExtendedPrice()));
  connect(_netUnitPrice,         SIGNAL(idChanged(int)),                 this, SLOT(sPriceGroup()));
  connect(_saleNetUnitPrice,     SIGNAL(valueChanged()),                 this, SLOT(sCalculateSaleDiscountPrcnt()));
  connect(_saleNetUnitPrice,     SIGNAL(valueChanged()),                 this, SLOT(sCalculateSaleExtendedPrice()));
  connect(_qtyAuth,              SIGNAL(textChanged(const QString&)),    this, SLOT(sCalculateExtendedPrice()));
  connect(_qtyAuth,              SIGNAL(textChanged(const QString&)),    this, SLOT(sCalculateSaleExtendedPrice()));
  connect(_qtyAuth,              SIGNAL(textChanged(const QString&)),    this, SLOT(sPopulateOrderInfo()));
  connect(_qtyAuth,              SIGNAL(textChanged(const QString&)),    this, SLOT(sCalcWoUnitCost()));
  connect(_save,                 SIGNAL(clicked()),                      this, SLOT(sSaveClicked()));
  connect(_taxLit,               SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_taxType,              SIGNAL(newID(int)),                     this, SLOT(sCalculateTax()));
  connect(_qtyUOM,               SIGNAL(newID(int)),                     this, SLOT(sQtyUOMChanged()));
  connect(_qtyUOM,               SIGNAL(newID(int)),                     this, SLOT(sPopulateOrderInfo()));
  connect(_pricingUOM,           SIGNAL(newID(int)),                     this, SLOT(sPriceUOMChanged()));
  connect(_salePricingUOM,       SIGNAL(newID(int)),                     this, SLOT(sSalePriceUOMChanged()));
  connect(_disposition,          SIGNAL(newID(int)),                     this, SLOT(sDispositionChanged()));
  connect(_shipWhs,              SIGNAL(newID(int)),                     this, SLOT(sDetermineAvailability()));
  connect(_scheduledDate,        SIGNAL(newDate(const QDate&)),          this, SLOT(sDetermineAvailability()));
  connect(_qtyAuth,              SIGNAL(editingFinished()),              this, SLOT(sDetermineAvailability()));
  connect(_createOrder,          SIGNAL(toggled(bool)),                  this, SLOT(sHandleWo(bool)));
  connect(_showAvailability,     SIGNAL(toggled(bool)),                  this, SLOT(sDetermineAvailability()));
  connect(this,                  SIGNAL(rejected()),                     this, SLOT(rejectEvent()));
  connect(_authLotSerial,        SIGNAL(toggled(bool)),                  _qtyUOM, SLOT(setDisabled(bool)));

  connect(_new,                  SIGNAL(clicked()),	                     this, SLOT(sNew()));
  connect(_edit,	               SIGNAL(clicked()),	                     this, SLOT(sEdit()));
  connect(_delete,	             SIGNAL(clicked()),	                     this, SLOT(sDelete()));

  _raitemls->addColumn(tr("Lot/Serial"),  -1,           Qt::AlignLeft , true, "ls_number"  );
  _raitemls->addColumn(tr("Warranty"),	  _dateColumn,  Qt::AlignRight, true, "lsreg_expiredate"  );
  _raitemls->addColumn(tr("Registered"),  _qtyColumn,   Qt::AlignRight, true, "raitemls_qtyregistered"  );
  _raitemls->addColumn(tr("Authorized"),  _qtyColumn,   Qt::AlignRight, true, "raitemls_qtyauthorized"  );
  _raitemls->addColumn(tr("Received"),    _qtyColumn,   Qt::AlignRight, true, "raitemls_qtyreceived"  );

  _item->setType(ItemLineEdit::cSold | ItemLineEdit::cActive);
  _item->addExtraClause( QString("(itemsite_active)") );  // ItemLineEdit::cActive doesn't compare against the itemsite record
  _item->addExtraClause( QString("(itemsite_sold)") );    // ItemLineEdit::cSold doesn't compare against the itemsite record

  _orderQty->setValidator(omfgThis->qtyVal());
  _qtyAuth->setValidator(omfgThis->qtyVal());
  _discountFromSale->setValidator(omfgThis->negPercentVal());
  _saleDiscountFromSale->setValidator(omfgThis->negPercentVal());
  _taxType->setEnabled(_privileges->check("OverrideTax"));
  _availabilityGroup->setEnabled(_showAvailability->isChecked());

  _qtySold->setPrecision(omfgThis->qtyVal());
  _qtyReceived->setPrecision(omfgThis->qtyVal());
  _qtyShipped->setPrecision(omfgThis->qtyVal());
  _onHand->setPrecision(omfgThis->qtyVal());
  _allocated->setPrecision(omfgThis->qtyVal());
  _unallocated->setPrecision(omfgThis->qtyVal());
  _onOrder->setPrecision(omfgThis->qtyVal());
  _available->setPrecision(omfgThis->qtyVal());
  _discountFromList->setPrecision(omfgThis->percentVal());

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

  adjustSize();

  _altcosAccntid->setType(GLCluster::cRevenue | GLCluster::cExpense);

  returnreturnAuthorizationItem.exec("BEGIN;"); //In case problems or we cancel out
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
  XSqlQuery returnet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _raheadid = param.toInt();
    returnet.prepare("SELECT rahead.*,"
              "       COALESCE(cust_preferred_warehous_id,-1) AS prefwhs, "
              "       COALESCE(rahead_orig_cohead_id,-1) AS cohead_id, "
              "       crmacct_id "
              "FROM rahead, custinfo, crmacct "
              "WHERE ( (rahead_cust_id=cust_id) "
              "AND (rahead_id=:rahead_id) "
              "AND (rahead_cust_id=crmacct_cust_id) );");
    returnet.bindValue(":rahead_id", _raheadid);
    returnet.exec();
    if (returnet.first())
    {
      _authNumber->setText(returnet.value("rahead_number").toString());
      _taxzoneid = returnet.value("rahead_taxzone_id").toInt();
      _tax->setId(returnet.value("rahead_curr_id").toInt());
      _rsnCode->setId(returnet.value("rahead_rsncode_id").toInt());
      _custid = returnet.value("rahead_cust_id").toInt();
      _shiptoid = returnet.value("rahead_shipto_id").toInt();
      _netUnitPrice->setId(returnet.value("rahead_curr_id").toInt());
      _netUnitPrice->setEffective(returnet.value("rahead_authdate").toDate());
      _saleNetUnitPrice->setId(returnet.value("rahead_curr_id").toInt());
      _saleNetUnitPrice->setEffective(returnet.value("rahead_authdate").toDate());
      _preferredWarehousid = returnet.value("prefwhs").toInt();
      _creditmethod = returnet.value("rahead_creditmethod").toString();
      _crmacctid = returnet.value("crmacct_id").toInt();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Information"),
                                  returnet, __FILE__, __LINE__))
    {
      return UndefinedError;
    }
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _preferredWarehousid = param.toInt();
    _warehouse->setId(_preferredWarehousid);
  }

  param = pParams.value("shipwarehous_id", &valid);
  if (valid)
  {
    _preferredShipWarehousid = param.toInt();
    _shipWhs->setId(_preferredShipWarehousid);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      connect(_discountFromSale, SIGNAL(editingFinished()), this, SLOT(sCalculateFromDiscount()));
      connect(_saleDiscountFromSale, SIGNAL(editingFinished()), this, SLOT(sCalculateSaleFromDiscount()));

      returnet.prepare( "SELECT (COALESCE(MAX(raitem_linenumber), 0) + 1) AS n_linenumber "
                 "FROM raitem "
                 "WHERE (raitem_rahead_id=:rahead_id);" );
      returnet.bindValue(":rahead_id", _raheadid);
      returnet.exec();
      if (returnet.first())
        _lineNumber->setText(returnet.value("n_linenumber").toString());
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Information"),
                                    returnet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }

      returnet.prepare( "SELECT rahead_disposition "
                 "FROM rahead "
                 "WHERE (rahead_id=:rahead_id);" );
      returnet.bindValue(":rahead_id", _raheadid);
      returnet.exec();
      if (returnet.first())
      {
        _disposition->setCode(returnet.value("rahead_disposition").toString());
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Information"),
                                    returnet, __FILE__, __LINE__))
      {
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
      _comments->setType(Comments::ReturnAuthItem);
      _comments->setReadOnly(true);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _item->setReadOnly(true);
      _warehouse->setEnabled(false);
      _shipWhs->setEnabled(false);
      _comments->setType(Comments::ReturnAuthItem);
      _comments->setReadOnly(false);

      connect(_discountFromSale, SIGNAL(editingFinished()), this, SLOT(sCalculateFromDiscount()));
      connect(_saleDiscountFromSale, SIGNAL(editingFinished()), this, SLOT(sCalculateSaleFromDiscount()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(true);
      _warehouse->setEnabled(false);
      _shipWhs->setEnabled(false);
      _disposition->setEnabled(false);
      _qtyAuth->setEnabled(false);
      _qtyUOM->setEnabled(false);
      _netUnitPrice->setEnabled(false);
      _saleNetUnitPrice->setEnabled(false);
      _listPrices->setEnabled(false);
      _saleListPrices->setEnabled(false);
      _pricingUOM->setEnabled(false);
      _salePricingUOM->setEnabled(false);
      _discountFromSale->setEnabled(false);
      _saleDiscountFromSale->setEnabled(false);
      _notes->setReadOnly(true);
      _taxType->setEnabled(false);
      _rsnCode->setEnabled(false);
      _altcosAccntid->setEnabled(false);
      _showAvailability->setEnabled(false);
      _createOrder->setEnabled(false);
      _scheduledDate->setEnabled(false);
      _warranty->setEnabled(false);
      _comments->setType(Comments::ReturnAuthItem);
      _comments->setReadOnly(true);

      _save->hide();
      _close->setText(tr("&Close"));
    }
  }

  param = pParams.value("orig_cohead_id", &valid);
  if (valid)
    _origsoid = param.toInt();

  param = pParams.value("raitem_id", &valid);
  if (valid)
  {
    _raitemid = param.toInt();
    populate();
  }

  return NoError;
}

void returnAuthorizationItem::sSaveClicked()
{
  XSqlQuery returnSaveClicked;
  if (sSave())
  {
    returnSaveClicked.exec("COMMIT;");
    done(_raitemid);
  }
}

bool returnAuthorizationItem::sSave()
{
  XSqlQuery returnSave;

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
    returnSave.exec("SELECT NEXTVAL('raitem_raitem_id_seq') AS _raitem_id");
    if (returnSave.first())
    {
      _raitemid  = returnSave.value("_raitem_id").toInt();
      _comments->setId(_raitemid);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Information"),
                                  returnSave, __FILE__, __LINE__))
    {
      reject();
    }

    returnSave.prepare( "INSERT INTO raitem "
               "( raitem_id, raitem_rahead_id, raitem_linenumber, raitem_itemsite_id,"
               "  raitem_disposition, raitem_qtyauthorized, "
               "  raitem_qty_uom_id, raitem_qty_invuomratio,"
               "  raitem_price_uom_id, raitem_price_invuomratio,"
               "  raitem_unitprice, raitem_taxtype_id, "
               "  raitem_notes, raitem_rsncode_id, raitem_cos_accnt_id, "
               "  raitem_scheddate, raitem_warranty, raitem_coitem_itemsite_id, "
               "  raitem_saleprice, raitem_unitcost, raitem_custpn ) "
               "SELECT :raitem_id, :rahead_id, :raitem_linenumber, rcv.itemsite_id,"
               "       :raitem_disposition, :raitem_qtyauthorized,"
               "       :qty_uom_id, :qty_invuomratio,"
               "       :price_uom_id, :price_invuomratio,"
               "       :raitem_unitprice, :raitem_taxtype_id, "
               "       :raitem_notes, :raitem_rsncode_id, :raitem_cos_accnt_id, "
               "       :raitem_scheddate, :raitem_warranty, shp.itemsite_id, "
               "       :raitem_saleprice, :raitem_unitcost, :raitem_custpn "
               "FROM itemsite rcv "
               "  LEFT OUTER JOIN itemsite shp ON "
               "        (shp.itemsite_item_id=rcv.itemsite_item_id) "
               "    AND (shp.itemsite_warehous_id=COALESCE(:shipWhs_id,-1)) "
               "WHERE ( (rcv.itemsite_item_id=:item_id)"
               " AND (rcv.itemsite_warehous_id=:warehous_id) );" );
  }
  else
  {
    returnSave.prepare( "UPDATE raitem "
               "SET raitem_disposition=:raitem_disposition, "
               "    raitem_qtyauthorized=:raitem_qtyauthorized, "
               "    raitem_qty_uom_id=:qty_uom_id,"
               "    raitem_qty_invuomratio=:qty_invuomratio,"
               "    raitem_price_uom_id=:price_uom_id,"
               "    raitem_price_invuomratio=:price_invuomratio,"
               "    raitem_unitprice=:raitem_unitprice,"
               "    raitem_taxtype_id=:raitem_taxtype_id, "
               "    raitem_notes=:raitem_notes,"
               "    raitem_rsncode_id=:raitem_rsncode_id, "
               "    raitem_cos_accnt_id=:raitem_cos_accnt_id, "
               "    raitem_scheddate=:raitem_scheddate, "
               "    raitem_warranty=:raitem_warranty, "
               "    raitem_saleprice=:raitem_saleprice, "
               "    raitem_coitem_itemsite_id=:coitem_itemsite_id, "
               "    raitem_unitcost=:raitem_unitcost, "
               "    raitem_custpn=:raitem_custpn "
               "WHERE (raitem_id=:raitem_id);" );

     if (_disposition->code() == "P" ||
         _disposition->code() == "V" ||
         _disposition->code() == "S")
     {
       XSqlQuery coitemsite;
       coitemsite.prepare("SELECT itemsite_id "
                          "FROM itemsite "
                          "WHERE ((itemsite_item_id=:item_id)"
                          " AND (itemsite_warehous_id=:warehous_id) "
                          " AND (itemsite_active) "
                          " AND (itemsite_sold));");
       coitemsite.bindValue(":item_id", _item->id());
       coitemsite.bindValue(":warehous_id",_shipWhs->id());
       coitemsite.exec();
       if (coitemsite.first())
         _coitemitemsiteid=coitemsite.value("itemsite_id").toInt();
       else
       {
         QMessageBox::critical( this, tr("Item site not found"),
             tr("<p>No valid item site record was found for the selected "
                "item at the selected shipping site.") );
         return false;
       }
       returnSave.bindValue(":coitem_itemsite_id",_coitemitemsiteid);
     }
  }

  returnSave.bindValue(":raitem_id", _raitemid);
  returnSave.bindValue(":rahead_id", _raheadid);
  returnSave.bindValue(":raitem_linenumber", _lineNumber->text().toInt());
  returnSave.bindValue(":raitem_qtyauthorized", _qtyAuth->toDouble());
  returnSave.bindValue(":raitem_disposition",   _disposition->code());
  returnSave.bindValue(":qty_uom_id", _qtyUOM->id());
  returnSave.bindValue(":qty_invuomratio", _qtyinvuomratio);
  returnSave.bindValue(":price_uom_id", _pricingUOM->id());
  returnSave.bindValue(":price_invuomratio", _priceinvuomratio);
  returnSave.bindValue(":raitem_unitprice", _netUnitPrice->localValue());
  if (_taxType->isValid())
    returnSave.bindValue(":raitem_taxtype_id", _taxType->id());
  returnSave.bindValue(":raitem_notes", _notes->toPlainText());
  returnSave.bindValue(":raitem_notes", _notes->toPlainText());
  if (_rsnCode->isValid())
    returnSave.bindValue(":raitem_rsncode_id", _rsnCode->id());
  returnSave.bindValue(":item_id", _item->id());
  returnSave.bindValue(":warehous_id", _warehouse->id());

  if (_disposition->code() == "P" ||
      _disposition->code() == "V" ||
      _disposition->code() == "S")
    returnSave.bindValue(":shipWhs_id", _shipWhs->id());

  if (_altcosAccntid->id() != -1)
    returnSave.bindValue(":raitem_cos_accnt_id", _altcosAccntid->id());
  returnSave.bindValue(":raitem_scheddate", _scheduledDate->date());
  returnSave.bindValue(":raitem_warranty",QVariant(_warranty->isChecked()));
  returnSave.bindValue(":raitem_saleprice", _saleNetUnitPrice->localValue());
  if (_costmethod=="A")
    returnSave.bindValue(":raitem_unitcost", _unitCost->localValue());
  returnSave.bindValue(":raitem_custpn", _customerPN->text());
  returnSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Information"),
                                returnSave, __FILE__, __LINE__))
  {
    reject();
  }

//  Check to see if a S/O should be re-scheduled
  if (_orderId != -1)
  {
    if (_qtyAuth->toDouble() == 0)
    {
      returnSave.prepare("SELECT deletewo(:woid, true, true) AS result;");
      returnSave.bindValue(":woid", _orderId);
      returnSave.exec();
      if (returnSave.value("result").toInt() < 0)
      {
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Retreiving RA Information"),
                               storedProcErrorLookup("deleteWo", returnSave.value("result").toInt()),
                               __FILE__, __LINE__);
        reject();
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Information"),
                                    returnSave, __FILE__, __LINE__))
      {
        reject();
      }
    }
    else if (_scheduledDate->date() != _cScheduledDate)
    {
      if (QMessageBox::question(this, tr("Reschedule W/O?"),
                tr("<p>The Scheduled Date for this Line "
                    "Item has been changed.  Should the "
                    "W/O for this Line Item be Re-"
                    "Scheduled to reflect this change?"),
                   QMessageBox::Yes | QMessageBox::Default,
                   QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
      {
        returnSave.prepare("SELECT changeWoDates(:wo_id, :schedDate, :schedDate, true) AS result;");
        returnSave.bindValue(":wo_id", _orderId);
        returnSave.bindValue(":schedDate", _scheduledDate->date());
        returnSave.exec();
        if (returnSave.first())
        {
          int result = returnSave.value("result").toInt();
          if (result < 0)
          {
              ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Information"),
                                   storedProcErrorLookup("changeWoDates", result),
                                   __FILE__, __LINE__);
            reject();
          }
          _cScheduledDate = _scheduledDate->date();
        }
        else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Information"),
                                      returnSave, __FILE__, __LINE__))
        {
          reject();
        }
      }
    }

    if (_qtyAuth->toDouble() != _cQtyOrdered &&
        _qtyAuth->toDouble() > 0)
    {
      if (_item->itemType() == "M")
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
          returnSave.prepare("SELECT changeWoQty(:wo_id, :qty, true) AS result;");
          returnSave.bindValue(":wo_id", _orderId);
          returnSave.bindValue(":qty", _qtyAuth->toDouble() * _qtyinvuomratio);
          returnSave.exec();
          if (ErrorReporter::error(QtCriticalMsg, this, tr("Change Work Order Quantity"),
                                   returnSave, __FILE__, __LINE__))
          {
            reject();
          }
        }
      }
    }
  }

  //If this save has resulted in a link to an shipping S/O, we need to signal that
  if (_disposition->code() == "P" ||
      _disposition->code() == "V" ||
      _disposition->code() == "S")
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
      omfgThis->sSalesOrdersUpdated(returnSave.value("rahead_new_cohead_id").toInt());
      //  If requested, create a new W/O or P/R for this coitem
      if ( ( (_mode == cNew) || (_mode == cEdit) ) &&
         (_createOrder->isChecked()) &&
         (_qtyAuth->toDouble() > 0) &&
         (_orderId == -1) )
      {
        QString chartype;
        if (_item->itemType() == "M")
        {
          returnSave.prepare( "SELECT createWo(:orderNumber, itemsite_id, :qty, itemsite_leadtime, :dueDate, :comments, :parent_type, :parent_id) AS result, itemsite_id "
                     "FROM itemsite "
                     "WHERE ( (itemsite_item_id=:item_id)"
                     " AND (itemsite_warehous_id=:warehous_id) );" );
          returnSave.bindValue(":orderNumber", so.value("cohead_number").toInt());
          returnSave.bindValue(":qty", _orderQty->toDouble());
          returnSave.bindValue(":dueDate", _scheduledDate->date());
          returnSave.bindValue(":comments", so.value("cust_name").toString() + "\n" + _notes->toPlainText());
          returnSave.bindValue(":item_id", _item->id());
          returnSave.bindValue(":warehous_id", _shipWhs->id());
          returnSave.bindValue(":parent_type", QString("S"));
          returnSave.bindValue(":parent_id", so.value("raitem_new_coitem_id").toInt());
          returnSave.exec();
        }
        if (returnSave.first())
        {
          _orderId = returnSave.value("result").toInt();
          if (_orderId < 0)
          {
            QString procname;
            if (_item->itemType() == "M")
              procname = "createWo";
            else
              procname = "unnamed stored procedure";
            ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Information"),
                                 storedProcErrorLookup(procname, _orderId),
                                 __FILE__, __LINE__);
            reject();
          }

          if (_item->itemType() == "M")
          {
            omfgThis->sWorkOrdersUpdated(_orderId, true);

    //  Update the newly created coitem with the newly create wo_id
            returnSave.prepare( "UPDATE coitem "
                       "SET coitem_order_type='W', coitem_order_id=:orderid "
                       "WHERE (coitem_id=:soitem_id);" );
            returnSave.bindValue(":orderid", _orderId);
            returnSave.bindValue(":soitem_id", so.value("raitem_new_coitem_id").toInt());
            returnSave.exec();
            if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Information"),
                                               returnSave, __FILE__, __LINE__))
            {
              reject();
            }
          }
        }
      }
      // Update W/O with any changes to notes
      if ( (_mode == cEdit) &&
         (_createOrder->isChecked()) &&
         (_qtyAuth->toDouble() > 0) &&
         (_orderId != -1) &&
         (_notes->toPlainText().length() > 0) )
      {
        if (_item->itemType() == "M")
        {
          returnSave.prepare("UPDATE wo SET wo_prodnotes=:comments WHERE (wo_id=:wo_id);");
          returnSave.bindValue(":wo_id", _orderId);
          returnSave.bindValue(":comments", so.value("cust_name").toString() + "\n" + _notes->toPlainText());
          returnSave.exec();
          if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating RA Information"),
                                        returnSave, __FILE__, __LINE__))
          {
            reject();
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
  // Get list of active, valid Selling UOMs
  sPopulateUOM();

  XSqlQuery item;
  item.prepare( "SELECT item_inv_uom_id, item_price_uom_id,"
                "       iteminvpricerat(item_id) AS iteminvpricerat,"
                "       item_listprice, "
                "       stdCost(item_id) AS stdcost,"
                "       getItemTaxType(item_id, :taxzone_id) AS taxtype_id "
                "  FROM item"
                " WHERE (item_id=:item_id);" );
  item.bindValue(":item_id", _item->id());
  item.bindValue(":taxzone_id", _taxzoneid);
  item.exec();
  if (item.first())
  {
    _priceRatio = item.value("iteminvpricerat").toDouble();
    _qtyUOM->setId(item.value("item_inv_uom_id").toInt());
    _pricingUOM->setId(item.value("item_price_uom_id").toInt());
    _salePricingUOM->setId(item.value("item_price_uom_id").toInt());
    _priceinvuomratio = item.value("iteminvpricerat").toDouble();
    _qtyinvuomratio = 1.0;
    _invuomid = item.value("item_inv_uom_id").toInt();
    _listPrice->setBaseValue(item.value("item_listprice").toDouble());
    _unitCost->setBaseValue(item.value("stdcost").toDouble());
    _taxType->setId(item.value("taxtype_id").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Information"),
                                item, __FILE__, __LINE__))
  {
    return;
  }

  if (_item->itemType() == "M" && _costmethod != "J")
    _createOrder->setEnabled(_mode == cNew || _mode == cEdit);
  else
    _createOrder->setEnabled(false);
  if (_costmethod == "J")
    _createOrder->setChecked(true);

  disconnect(_shipWhs,              SIGNAL(newID(int)),                     this, SLOT(sPopulateItemsiteInfo()));
  _warehouse->findItemsites(_item->id());
  _shipWhs->findItemsites(_item->id());
  if(_preferredWarehousid > 0)
    _warehouse->setId(_preferredWarehousid);
  if(_preferredShipWarehousid > 0)
    _shipWhs->setId(_preferredShipWarehousid);
  connect(_shipWhs,              SIGNAL(newID(int)),                     this, SLOT(sPopulateItemsiteInfo()));
  
  sPopulateItemsiteInfo();
}

void returnAuthorizationItem::sPopulateItemsiteInfo()
{
  if (_item->isValid())
  {
    XSqlQuery itemsite;
    itemsite.prepare( "SELECT itemsite_leadtime, itemsite_controlmethod, "
                      "       itemsite_createwo, itemsite_createpr, "
                      "       itemsite_costmethod, itemsite_id "
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
      _costmethod = itemsite.value("itemsite_costmethod").toString();

      if (_costmethod == "J")
      {
        _createOrder->setChecked(true);
        _createOrder->setEnabled(false);
      }
      else if (cNew == _mode)
      {
        if ( _disposition->code() == "V" && _costmethod != "J")
        {
          QMessageBox::warning( this, tr("Cannot use Service Disposition"),
                                tr("<p>Only Items Sites using the Job cost method may have a Disposition of Service.") );
          _item->setId(-1);
          _item->setFocus();
          return;
        }

        if (_item->itemType() == "M" && _costmethod != "J")
          _createOrder->setChecked(itemsite.value("itemsite_createwo").toBool());
        else
        {
          _createOrder->setChecked(false);
          _createOrder->setEnabled(false);
        }
      }

     if ( _disposition->code() == "R" ||
          _disposition->code() == "P")
     {
      if (_costmethod == "A")
      {
        if (cNew != _mode)
          _unitCost->setLocalValue(_unitcost);
        else if (_origsoid != -1)
        {
          XSqlQuery uc;
          uc.prepare("SELECT COALESCE(SUM(cohist_unitcost * cohist_qtyshipped) / "
                     "  SUM(cohist_qtyshipped),0) AS unitcost "
                     "FROM cohist "
                     " JOIN cohead ON ((cohist_doctype='I') "
                     "             AND (cohist_ordernumber=cohead_number)) "
                     "WHERE ((cohead_id=:cohead_id) "
                     "  AND (cohist_itemsite_id=:itemsite_id); ");
          uc.bindValue(":cohead_id", _origsoid);
          uc.bindValue(":itemsite_id", itemsite.value("itemsite_id").toInt());
          uc.exec();
          if (uc.first())
            _unitCost->setLocalValue(uc.value("unitcost").toDouble());
          else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Item Information"),
                                        uc, __FILE__, __LINE__))
          {
            return;
          }
        }
        _unitCost->setEnabled(cView != _mode);
      }
      else
        _unitCost->setEnabled(_costmethod == "A");
     }
     else
       _unitCost->setEnabled(false);

      _tab->setTabEnabled(_tab->indexOf(_lotserial),
      (itemsite.value("itemsite_controlmethod").toString() == "L" ||
       itemsite.value("itemsite_controlmethod").toString() == "S"));
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Item Information"),
                                  itemsite, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void returnAuthorizationItem::populate()
{
  XSqlQuery raitem;
  raitem.prepare("SELECT rahead_number, rahead_curr_id, rahead_authdate, rahead_taxzone_id, raitem.*, "
                 "       formatRaLineNumber(raitem_id) AS linenumber,"
                 "       och.cohead_number AS orig_number, formatSoLineNumber(oc.coitem_id) AS orig_linenumber, "
                 "       nch.cohead_number AS new_number, formatSoLineNumber(nc.coitem_id) AS new_linenumber, "
                 "       COALESCE(raitem_orig_coitem_id,-1) AS ra_coitem_id, oc.coitem_price, "
                 "       oc.coitem_qtyshipped AS qtysold,"
                 "       raitem_qtyauthorized AS qtyauth,"
                 "       raitem_qtyreceived AS qtyrcvd,"
                 "       nc.coitem_qtyshipped AS qtyshipd,"
                 "       rahead_curr_id AS taxcurr, "
                 "       item_inv_uom_id, "
                 "       COALESCE(nc.coitem_order_id,-1) AS coitem_order_id, "
                 "       nc.coitem_order_type AS coitem_order_type, "
                 "       rcv.itemsite_warehous_id AS itemsite_warehous_id, "
                 "       shp.itemsite_warehous_id AS shipWhs_id, "
                 "       qtyToReceive('RA', raitem_id) AS qtytorcv, "
                 "       crmacct_id, "
                 "       COALESCE(nc.coitem_price, raitem_saleprice) AS saleprice, "
                 "       COALESCE(nch.cohead_curr_id, rahead_curr_id) AS salecurrid, "
                 "       COALESCE(nch.cohead_orderdate, rahead_authdate) AS saledate "
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
  _comments->setId(_raitemid);
  raitem.exec();
  if (raitem.first())
  {
    _authNumber->setText(raitem.value("rahead_number").toString());
    _unitcost = raitem.value("raitem_unitcost").toDouble();

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
    _disposition->setCode(raitem.value("raitem_disposition").toString());
    _orderId = raitem.value("coitem_order_id").toInt();
    _netUnitPrice->setId(raitem.value("rahead_curr_id").toInt());
    _netUnitPrice->setEffective(raitem.value("rahead_authdate").toDate());
    _netUnitPrice->setLocalValue(raitem.value("raitem_unitprice").toDouble());
    _saleNetUnitPrice->setId(raitem.value("salecurrid").toInt());
    _saleNetUnitPrice->setEffective(raitem.value("saledate").toDate());
    _saleNetUnitPrice->setLocalValue(raitem.value("saleprice").toDouble());
    // do _item and _taxzone before other tax stuff because of signal cascade
    _taxzoneid = raitem.value("rahead_taxzone_id").toInt();
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
    _lineNumber->setText(raitem.value("linenumber").toString());
    _qtyAuth->setDouble(raitem.value("qtyauth").toDouble());
    _qtyReceived->setDouble(raitem.value("qtyrcvd").toDouble());
    _qtyShipped->setDouble(raitem.value("qtyshipd").toDouble());
    _notes->setText(raitem.value("raitem_notes").toString());
    _rsnCode->setId(raitem.value("raitem_rsncode_id").toInt());
    _altcosAccntid->setId(raitem.value("raitem_cos_accnt_id").toInt());
    _scheduledDate->setDate(raitem.value("raitem_scheddate").toDate());
    _warranty->setChecked(raitem.value("raitem_warranty").toBool());
    _status = raitem.value("raitem_status").toString();
    _qtycredited = raitem.value("raitem_qtycredited").toDouble();
    _customerPN->setText(raitem.value("raitem_custpn").toString());

    _cQtyOrdered = _qtyAuth->toDouble();
    _cScheduledDate = _scheduledDate->date();
    _crmacctid = raitem.value("crmacct_id").toInt();

    _coitemid = raitem.value("ra_coitem_id").toInt();
    _coitemitemsiteid = raitem.value("raitem_coitem_itemsite_id").toInt();
    if (_coitemitemsiteid == 0)
      _coitemitemsiteid = -1;
    if (_coitemid != -1)
    {
      _origSoNumber->setText(raitem.value("orig_number").toString());
      _origSoLineNumber->setText(raitem.value("orig_linenumber").toString());
      _qtySold->setDouble(raitem.value("qtysold").toDouble());
      _soldQty=raitem.value("qtysold").toDouble();
      _qtyUOM->setEnabled(false);
      _pricingUOM->setEnabled(false);
      _salePricingUOM->setEnabled(false);
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
      _disposition->setEnabled(false);

    if (_orderId != -1)
    {
      XSqlQuery query;

      if (raitem.value("coitem_order_type").toString() == "W")
      {
        query.prepare( "SELECT wo_status,"
                       "       wo_qtyord AS qty,"
                       "       wo_duedate, warehous_id, warehous_code "
                       "FROM wo, itemsite, whsinfo "
                       "WHERE ((wo_itemsite_id=itemsite_id)"
                       " AND (itemsite_warehous_id=warehous_id)"
                       " AND (wo_id=:wo_id));" );
        query.bindValue(":wo_id", _orderId);
        query.exec();
        if (query.first())
        {
          _createOrder->setChecked(true);

          _orderQty->setDouble(query.value("qty").toDouble());
          _orderDueDate->setDate(query.value("wo_duedate").toDate());
          _orderStatus->setText(query.value("wo_status").toString());

          if ((query.value("wo_status").toString() == "R") ||
              (query.value("wo_status").toString() == "C") ||
              (query.value("wo_status").toString() == "I"))
          {
            _createOrder->setEnabled(false);
            if (_costmethod == "J")
              _qtyAuth->setEnabled(false);
          }
        }
        else
        {
          _orderId = -1;
          _createOrder->setChecked(false);
        }
      }
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Item Information"),
                                raitem, __FILE__, __LINE__))
  {
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
      _discountFromList->setDouble((1 - (unitPrice / _listPrice->localValue())) * 100 );

    if (_salePrice->isZero())
      _discountFromSale->setText("N/A");
    else
      _discountFromSale->setDouble((1 - (unitPrice / _salePrice->localValue())) * 100 );
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

void returnAuthorizationItem::sCalculateSaleExtendedPrice()
{
  _saleExtendedPrice->setLocalValue(((_qtyAuth->toDouble() * _qtyinvuomratio) / _priceinvuomratio) * _saleNetUnitPrice->localValue());
}

void returnAuthorizationItem::sCalculateSaleDiscountPrcnt()
{
  double unitPrice = _saleNetUnitPrice->localValue();

  if (unitPrice == 0.0)
  {
    _saleDiscountFromSale->setText("N/A");
  }
  else
  {
    if (_listPrice->isZero())
      _saleDiscountFromSale->setText("N/A");
    else
      _saleDiscountFromSale->setDouble((1 - (unitPrice / _listPrice->localValue())) * 100 );
  }
}

void returnAuthorizationItem::sCalculateSaleFromDiscount()
{
  double discount = _saleDiscountFromSale->toDouble() / 100.0;

  if (_listPrice->isZero())
    _saleDiscountFromSale->setText(tr("N/A"));
  else
    _saleNetUnitPrice->setLocalValue(_listPrice->localValue() - (_listPrice->localValue() * discount));
}

void returnAuthorizationItem::sListPrices()
{
  XSqlQuery returnListPrices;
  returnListPrices.prepare( "SELECT formatSalesPrice(currToCurr(ipshead_curr_id, :curr_id, ipsprice_price, :effective)) AS price"
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
             "       FROM ipsass, ipshead, ipsprice, custinfo "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_custtype_id=cust_custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, custtype, custinfo "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
             "        AND (custtype_code ~ ipsass_custtype_pattern)"
             "        AND (cust_custtype_id=custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)))"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, shiptoinfo "
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
             "       FROM item, custinfo "
             "       WHERE ( (item_sold)"
             "        AND (NOT item_exclusive)"
             "        AND (item_id=:item_id)"
             "        AND (cust_id=:cust_id) );");
  returnListPrices.bindValue(":item_id", _item->id());
  returnListPrices.bindValue(":cust_id", _custid);
  returnListPrices.bindValue(":shipto_id", _shiptoid);
  returnListPrices.bindValue(":curr_id", _netUnitPrice->id());
  returnListPrices.bindValue(":effective", _netUnitPrice->effective());
  returnListPrices.exec();
  if (returnListPrices.size() == 1)
  {
    returnListPrices.first();
    _netUnitPrice->setLocalValue(returnListPrices.value("price").toDouble() * (_priceRatio / _priceinvuomratio));
  }
  else
  {
    ParameterList params;
    params.append("cust_id", _custid);
    params.append("shipto_id", _shiptoid);
    params.append("item_id", _item->id());
    params.append("warehous_id", _warehouse->id());
    // don't params.append("qty", ...) as we don't know how many were purchased
    params.append("curr_id", _netUnitPrice->id());
    params.append("effective", _netUnitPrice->effective());

    priceList newdlg(this);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Accepted)
    {
      _netUnitPrice->setLocalValue(newdlg._selectedPrice * (_priceRatio / _priceinvuomratio));
      sCalculateDiscountPrcnt();
    }
  }
}

void returnAuthorizationItem::sSaleListPrices()
{
  XSqlQuery returnSaleListPrices;
  returnSaleListPrices.prepare( "SELECT formatSalesPrice(currToCurr(ipshead_curr_id, :curr_id, ipsprice_price, :effective)) AS price"
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
             "       FROM ipsass, ipshead, ipsprice, custinfo "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_custtype_id=cust_custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, custtype, custinfo "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
             "        AND (custtype_code ~ ipsass_custtype_pattern)"
             "        AND (cust_custtype_id=custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)))"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, shiptoinfo "
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
             "       FROM item, custinfo "
             "       WHERE ( (item_sold)"
             "        AND (NOT item_exclusive)"
             "        AND (item_id=:item_id)"
             "        AND (cust_id=:cust_id) );");
  returnSaleListPrices.bindValue(":item_id", _item->id());
  returnSaleListPrices.bindValue(":cust_id", _custid);
  returnSaleListPrices.bindValue(":shipto_id", _shiptoid);
  returnSaleListPrices.bindValue(":curr_id", _saleNetUnitPrice->id());
  returnSaleListPrices.bindValue(":effective", _saleNetUnitPrice->effective());
  returnSaleListPrices.exec();
  if (returnSaleListPrices.size() == 1)
  {
    returnSaleListPrices.first();
    _saleNetUnitPrice->setLocalValue(returnSaleListPrices.value("price").toDouble() * (_priceRatio / _priceinvuomratio));
  }
  else
  {
    ParameterList params;
    params.append("cust_id", _custid);
    params.append("shipto_id", _shiptoid);
    params.append("item_id", _item->id());
    // don't params.append("qty", ...) as we don't know how many were purchased
    params.append("curr_id", _saleNetUnitPrice->id());
    params.append("effective", _saleNetUnitPrice->effective());

    priceList newdlg(this);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Accepted)
    {
      _saleNetUnitPrice->setLocalValue(newdlg._selectedPrice * (_priceRatio / _priceinvuomratio));
      sCalculateSaleDiscountPrcnt();
    }
  }
}

void returnAuthorizationItem::sCalculateTax()
{
  XSqlQuery calcq;

  calcq.prepare("SELECT calculateTax(rahead_taxzone_id,:taxtype_id,rahead_authdate,rahead_curr_id,ROUND(:ext,2)) AS tax "
                "FROM rahead "
                "WHERE (rahead_id=:rahead_id); " );

  calcq.bindValue(":rahead_id", _raheadid);
  calcq.bindValue(":taxtype_id", _taxType->id());
  calcq.bindValue(":ext", _extendedPrice->localValue());
  calcq.exec();
  if (calcq.first())
    _tax->setLocalValue(calcq.value("tax").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Calculating Tax Information"),
                                calcq, __FILE__, __LINE__))
  {
    return;
  }
}

void returnAuthorizationItem::sTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("taxzone_id",   _taxzoneid);
  params.append("taxtype_id",  _taxType->id());
  params.append("date", _netUnitPrice->effective());
  params.append("curr_id", _netUnitPrice->id());
  params.append("subtotal", _extendedPrice->localValue());
  params.append("readOnly");

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    if (_taxType->id() != newdlg.taxtype())
      _taxType->setId(newdlg.taxtype());
  }
}

void returnAuthorizationItem::sPopulateUOM()
{
  // Get list of active, valid Selling UOMs
  MetaSQLQuery muom = mqlLoad("uoms", "item");
  
  ParameterList params;
  params.append("uomtype", "Selling");
  params.append("item_id", _item->id());
  
  // Include Global UOMs
  if (_privileges->check("MaintainUOMs"))
  {
    params.append("includeGlobal", true);
    params.append("global", tr("-Global"));
  }
  
  // Also have to factor UOMs previously used on Return Auth now inactive
  if (_raitemid != -1)
  {
    XSqlQuery cmuom;
    cmuom.prepare("SELECT raitem_qty_uom_id, raitem_price_uom_id "
                  "  FROM raitem"
                  " WHERE(raitem_id=:raitem_id);");
    cmuom.bindValue(":raitem_id", _raitemid);
    cmuom.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Returns UOMs"),
                             cmuom, __FILE__, __LINE__))
      return;
    else if (cmuom.first())
    {
      params.append("uom_id", cmuom.value("raitem_qty_uom_id"));
      params.append("uom_id2", cmuom.value("raitem_price_uom_id"));
    }
  }
  XSqlQuery uom = muom.toQuery(params);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting UOMs"),
                           uom, __FILE__, __LINE__))
    return;
  
  int saveqtyuomid = _qtyUOM->id();
  int savepriceuomid = _pricingUOM->id();
  int savesaleuomid = _salePricingUOM->id();
  disconnect(_qtyUOM,         SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
  disconnect(_pricingUOM,     SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));
  disconnect(_salePricingUOM, SIGNAL(newID(int)), this, SLOT(sSalePriceUOMChanged()));
  _qtyUOM->populate(uom);
  _pricingUOM->populate(uom);
  _salePricingUOM->populate(uom);
  _qtyUOM->setId(saveqtyuomid);
  _pricingUOM->setId(savepriceuomid);
  _salePricingUOM->setId(savesaleuomid);
  connect(_qtyUOM,         SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
  connect(_pricingUOM,     SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));
  connect(_salePricingUOM, SIGNAL(newID(int)), this, SLOT(sSalePriceUOMChanged()));
}

void returnAuthorizationItem::sQtyUOMChanged()
{
  // Check for Global UOM Conversion that must be setup for Item
  if (_qtyUOM->code() == "G")
  {
    if (QMessageBox::question(this, tr("Use Global UOM?"),
                              tr("<p>This Global UOM Conversion is not setup for this Item."
                                 "<p>Do you want to add this UOM conversion to this Item?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
    {
      // create itemuomconv and itemuom
      XSqlQuery adduom;
      adduom.prepare("SELECT createItemUomConv(:item_id, :uom_id, :uom_type) AS result;");
      adduom.bindValue(":item_id", _item->id());
      adduom.bindValue(":uom_id", _qtyUOM->id());
      adduom.bindValue(":uom_type", "Selling");
      adduom.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Creating Item UOM Conv"),
                               adduom, __FILE__, __LINE__))
        return;
      
      // repopulate uom comboboxes
      sPopulateUOM();
    }
    else
    {
      _qtyUOM->setId(_invuomid);
    }
  }
  
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
      ErrorReporter::error(QtCriticalMsg, this, tr("Retrieving RA Item Information"),
                         invuom, __FILE__, __LINE__);
  }

  if(_qtyUOM->id() != _invuomid)
  {
    _pricingUOM->setId(_qtyUOM->id());
    _pricingUOM->setEnabled(false);
    _salePricingUOM->setId(_qtyUOM->id());
    _salePricingUOM->setEnabled(false);
  }
  else
  {
    bool isEditing = (_mode == cNew || _mode == cEdit);
    _pricingUOM->setEnabled(isEditing);
    _salePricingUOM->setEnabled(isEditing);
  }
  
  sCalculateExtendedPrice();
  sCalculateSaleExtendedPrice();
}

void returnAuthorizationItem::sPriceUOMChanged()
{
  if(_pricingUOM->id() == -1 || _qtyUOM->id() == -1)
    return;

  // Check for Global UOM Conversion that must be setup for Item
  if (_pricingUOM->code() == "G")
  {
    if (QMessageBox::question(this, tr("Use Global UOM?"),
                              tr("<p>This Global UOM Conversion is not setup for this Item."
                                 "<p>Do you want to add this UOM conversion to this Item?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
    {
      XSqlQuery adduom;
      adduom.prepare("SELECT createItemUomConv(:item_id, :uom_id, :uom_type) AS result;");
      adduom.bindValue(":item_id", _item->id());
      adduom.bindValue(":uom_id", _pricingUOM->id());
      adduom.bindValue(":uom_type", "Selling");
      adduom.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Creating Item UOM Conv"),
                               adduom, __FILE__, __LINE__))
        return;
      
      // repopulate uom comboboxes
      sPopulateUOM();
    }
    else
    {
      _pricingUOM->setId(_invuomid);
    }
  }
  
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
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Item Information"),
                         invuom, __FILE__, __LINE__);
  }
  //_ratio->setText(formatUOMRatio(_priceinvuomratio));

  updatePriceInfo();
}

void returnAuthorizationItem::sSalePriceUOMChanged()
{
  // Check for Global UOM Conversion that must be setup for Item
  if (_salePricingUOM->code() == "G")
  {
    if (QMessageBox::question(this, tr("Use Global UOM?"),
                              tr("<p>This Global UOM Conversion is not setup for this Item."
                                 "<p>Do you want to add this UOM conversion to this Item?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
    {
      XSqlQuery adduom;
      adduom.prepare("SELECT createItemUomConv(:item_id, :uom_id, :uom_type) AS result;");
      adduom.bindValue(":item_id", _item->id());
      adduom.bindValue(":uom_id", _salePricingUOM->id());
      adduom.bindValue(":uom_type", "Selling");
      adduom.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Creating Item UOM Conv"),
                               adduom, __FILE__, __LINE__))
        return;
      
      // repopulate uom comboboxes
      int saveqtyuomid = _qtyUOM->id();
      int savepriceuomid = _pricingUOM->id();
      int savesaleuomid = _salePricingUOM->id();
      disconnect(_qtyUOM,         SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
      disconnect(_pricingUOM,     SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));
      disconnect(_salePricingUOM, SIGNAL(newID(int)), this, SLOT(sSalePriceUOMChanged()));
      sPopulateUOM();
      _qtyUOM->setId(saveqtyuomid);
      _pricingUOM->setId(savepriceuomid);
      _salePricingUOM->setId(savesaleuomid);
      connect(_qtyUOM,         SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
      connect(_pricingUOM,     SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));
      connect(_salePricingUOM, SIGNAL(newID(int)), this, SLOT(sSalePriceUOMChanged()));
    }
    else
    {
      _salePricingUOM->setId(_invuomid);
    }
  }
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
  bool isEditing = (_mode == cNew || _mode == cEdit);

  if ( (_disposition->code() == "V") && // service
       (_item->id() != -1) &&
       (_costmethod != "J") )
  {
    QMessageBox::warning( this, tr("Cannot use Service Disposition"),
                          tr("<p>Only Items Sites using the Job cost method may have a Disposition of Service.") );
    _disposition->setFocus();
    _disposition->setId(_dispositionCache);
    return;
  }
  else if (_disposition->code() == "V")
  {
    _shipWhs->setId(_warehouse->id());
    _shipWhs->setEnabled(false);
  }
  else if ( (_disposition->id() < 2) && // credit or return
       (_orderId != -1) )
  {
    QMessageBox::warning( this, tr("Cannot change Disposition"),
                          tr("<p>A work order is associated with this Return. "
                             "First delete the work order, then change this disposition.") );
    _disposition->setFocus();
    _disposition->setId(_dispositionCache);
    return;
  }
  else if (_disposition->code() == "C" || _disposition->code() == "R")
  {
    _shipWhs->setId(_warehouse->id());
    _shipWhs->setVisible(false);
  }
  else
  {
    _shipWhs->setVisible(true);
    _shipWhs->setEnabled(isEditing);
  }

  if (_mode == cNew)
      _item->addExtraClause( QString("(item_id IN (SELECT custitem FROM custitem(%1, %2) ) )").arg(_custid).arg(_shiptoid) );

  if (_disposition->code() == "V" || _disposition->code() == "S") // service or ship
  {
    _netUnitPrice->setLocalValue(0);
    _netUnitPrice->setEnabled(false);
    _listPrices->setEnabled(false);
    _pricingUOM->setEnabled(false);
    _discountFromSale->setEnabled(false);
  }
  else
  {
    _netUnitPrice->setEnabled(isEditing);
    _listPrices->setEnabled(isEditing);
    _pricingUOM->setEnabled(isEditing);
    _discountFromSale->setEnabled(isEditing);
  }

  if (_disposition->code() == "P" ||
      _disposition->code() == "V" ||
      _disposition->code() == "S")      // replace, service, or ship
  {
    _tab->setTabEnabled(_tab->indexOf(_supply),true);
    _scheduledDate->setEnabled(isEditing);
    _altcosAccntid->setEnabled(isEditing);
    _shipWhsLit->setVisible(true);
    _shipWhs->setVisible(true);
    _saleNetUnitPrice->setEnabled(isEditing);
    _saleListPrices->setEnabled(isEditing);
    _salePricingUOM->setEnabled(isEditing);
    _saleDiscountFromSale->setEnabled(isEditing);
  }
  else
  {
    _tab->setTabEnabled(_tab->indexOf(_supply),false);
    _scheduledDate->clear();
    _scheduledDate->setEnabled(false);
    _altcosAccntid->setEnabled(isEditing);
    _shipWhsLit->setVisible(false);
    _shipWhs->setVisible(false);
    _saleNetUnitPrice->setLocalValue(0);
    _saleNetUnitPrice->setEnabled(false);
    _saleListPrices->setEnabled(false);
    _salePricingUOM->setEnabled(false);
    _saleDiscountFromSale->setEnabled(false);
  }

  if (_creditmethod == "N")
  {
    _netUnitPrice->setLocalValue(0);
    _netUnitPrice->setEnabled(false);
    _listPrices->setEnabled(false);
    _pricingUOM->setEnabled(false);
    _discountFromSale->setEnabled(false);
    disconnect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));
  }

  _dispositionCache = _disposition->id();
  sPopulateItemInfo();
  sPopulateItemsiteInfo();
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

      if (_item->itemType() == "M")
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
                        "SELECT deleteWo(:wo_id, true) AS result;");
          query.bindValue(":wo_id", _orderId);
          query.exec();
          if (query.first())
          {
            int result = query.value("result").toInt();
            if (result < 0)
            {
              ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating RA Item Information"),
                                     storedProcErrorLookup("deleteWo", result),
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
              if (_qtyReceived->toDouble() == 0 &&
                  _qtyShipped->toDouble() == 0 &&
                  _qtycredited == 0 &&
                  _status == "O")
                _disposition->setEnabled(_mode == cNew || _mode == cEdit);

              _createOrder->setChecked(false);
            }
          }
          else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating RA Item Information"),
                                        query, __FILE__, __LINE__))
          {
             _createOrder->setChecked(true); // if (pCreate) => won't recurse
            return;
          }

          omfgThis->sWorkOrdersUpdated(-1, true);
        }
        else
          _createOrder->setChecked(true);
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
      qty.prepare( "SELECT validateOrderQty(itemsite_id, :qty, true) AS qty "
                   "FROM itemsite "
                   "WHERE ((itemsite_item_id=:item_id)"
                   " AND (itemsite_warehous_id=:warehous_id));" );
      qty.bindValue(":qty", _qtyAuth->toDouble() * _qtyinvuomratio);
      qty.bindValue(":item_id", _item->id());
      qty.bindValue(":warehous_id", ((_item->itemType() == "M") ? _shipWhs->id() : _shipWhs->id()));
      qty.exec();
      if (qty.first())
        _orderQty->setDouble(qty.value("qty").toDouble());

      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Item Information"),
                                    qty, __FILE__, __LINE__))
      {
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
                          "       availableqoh,"
                          "       allocated,"
                          "       noNeg(availableqoh - allocated) AS unallocated,"
                          "       ordered,"
                          "       (availableqoh - allocated + ordered) AS available,"
                          "       itemsite_leadtime "
                          "FROM ( SELECT itemsite_id, qtyAvailable(itemsite_id) AS availableqoh,"
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
      _onHand->setDouble(availability.value("availableqoh").toDouble());
      _allocated->setDouble(availability.value("allocated").toDouble());
      _unallocated->setDouble(availability.value("unallocated").toDouble());
      _onOrder->setDouble(availability.value("ordered").toDouble());
      _available->setDouble(availability.value("available").toDouble());
      _leadtime->setText(availability.value("itemsite_leadtime").toString());

      QString stylesheet;
      if (availability.value("available").toDouble() < _availabilityQtyOrdered)
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
      _available->setStyleSheet(stylesheet);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Item Information"),
                                  availability, __FILE__, __LINE__))
    {
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
  XSqlQuery returnCalcWoUnitCost;
  if (_costmethod == "J" && _orderId > -1 && _qtyAuth->toDouble() != 0)
  {
    returnCalcWoUnitCost.prepare("SELECT COALESCE(SUM(wo_postedvalue),0) AS wo_value "
              "FROM wo,raitem "
              "WHERE ((wo_ordtype='S') "
              "  AND  (wo_ordid=raitem_new_coitem_id) "
              "  AND  (raitem_id=:raitem_id));");
	returnCalcWoUnitCost.bindValue(":raitem_id", _raitemid);
	returnCalcWoUnitCost.exec();
	if (returnCalcWoUnitCost.first())
      _unitCost->setBaseValue(returnCalcWoUnitCost.value("wo_value").toDouble() / _qtyAuth->toDouble() * _qtyinvuomratio);
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

		returnAuthItemLotSerial newdlg(this, "", true);
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
		fill = false;
		QList<XTreeWidgetItem*> selected = _raitemls->selectedItems();
		for (int i = 0; i < selected.size(); i++)
		{
			ParameterList params;
      params.append("raitem_id", _raitemid);
      params.append("item_id", _item->id());
      params.append("warehouse_id", _warehouse->id());
      params.append("uom", _qtyUOM->currentText());
      params.append("ls_id", ((XTreeWidgetItem*)(selected[i]))->altId());

			if (_mode==cView)
				params.append("mode", "view");
			else
				params.append("mode", "edit");

			returnAuthItemLotSerial newdlg(this, "", true);
			newdlg.set(params);

			int raitemlsid = newdlg.exec();
			if (raitemlsid > 0)
				fill = true;
			else if (raitemlsid == -1)
				reject();
		}
		if (fill)
			populate();
	}
}

void returnAuthorizationItem::sDelete()
{
  XSqlQuery returnDelete;
  QList<XTreeWidgetItem*> selected = _raitemls->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
        QString sql ( "DELETE FROM raitemls WHERE (raitemls_id=:raitemls_id);" );
        returnDelete.prepare(sql);
        returnDelete.bindValue(":raitemls_id",  ((XTreeWidgetItem*)(selected[i]))->id());
        returnDelete.exec();
        if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting RA Item Information"),
                                      returnDelete, __FILE__, __LINE__))
        {
          reject();
        }
  }
  populate();
}

void returnAuthorizationItem::sFillList()
{
  XSqlQuery returnFillList;
  returnFillList.prepare("SELECT raitemls_id,ls_id,ls_number, "
            "       MAX(lsreg_expiredate) AS lsreg_expiredate, "
            "       COALESCE(SUM(lsreg_qty / raitem_qty_invuomratio),0) AS raitemls_qtyregistered, "
            "       raitemls_qtyauthorized, raitemls_qtyreceived, "
            "      'qty' AS raitemls_qtyregistered_xtnumericrole, "
            "      'qty' AS raitemls_qtyauthorized_xtnumericrole, "
            "      'qty' AS raitemls_qtyreceived_xtnumericrole "
            "FROM raitemls "
            "  LEFT OUTER JOIN lsreg ON (lsreg_ls_id=raitemls_ls_id) "
            "                       AND (lsreg_crmacct_id=:crmacct_id), "
            "  ls, raitem "
            "WHERE ( (raitemls_raitem_id=raitem_id) "
            "  AND   (raitem_id=:raitem_id) "
            "  AND   (raitemls_ls_id=ls_id) ) "
            "GROUP BY raitemls_id,ls_id,ls_number,raitemls_qtyauthorized,"
            "         raitemls_qtyreceived "
            "ORDER BY ls_number;" );
  returnFillList.bindValue(":raitem_id", _raitemid);
  returnFillList.bindValue(":crmacct_id", _crmacctid);
  returnFillList.bindValue(":na", tr("N/A"));
  returnFillList.exec();
  _raitemls->populate(returnFillList,true);
  _authLotSerial->setDisabled(returnFillList.first());
  _authLotSerial->setChecked(returnFillList.first());
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving RA Item Information"),
                                returnFillList, __FILE__, __LINE__))
  {
    reject();
  }
}

void returnAuthorizationItem::rejectEvent()
{
  XSqlQuery returnrejectEvent;
  returnrejectEvent.exec("ROLLBACK");
}



