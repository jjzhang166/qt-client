/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "creditMemoItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

#include "priceList.h"
#include "taxDetail.h"
#include "xdoublevalidator.h"

creditMemoItem::creditMemoItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

#ifndef Q_OS_MAC
  _listPrices->setMaximumWidth(25);
#endif

  connect(_discountFromSale, SIGNAL(editingFinished()),              this, SLOT(sCalculateFromDiscount()));
  connect(_extendedPrice,    SIGNAL(valueChanged()),                 this, SLOT(sCalculateTax()));
  connect(_item,	     SIGNAL(newId(int)),                     this, SLOT(sPopulateItemInfo()));
  connect(_warehouse,	     SIGNAL(newID(int)),                     this, SLOT(sPopulateItemsiteInfo()));
  connect(_listPrices,	     SIGNAL(clicked()),                      this, SLOT(sListPrices()));
  connect(_netUnitPrice,     SIGNAL(valueChanged()),                 this, SLOT(sCalculateDiscountPrcnt()));
  connect(_netUnitPrice,     SIGNAL(valueChanged()),                 this, SLOT(sCalculateExtendedPrice()));
  connect(_netUnitPrice,     SIGNAL(idChanged(int)),                 this, SLOT(sPriceGroup()));
  connect(_qtyToCredit,	     SIGNAL(textChanged(const QString&)),    this, SLOT(sCalculateExtendedPrice()));
  connect(_save,	     SIGNAL(clicked()),                      this, SLOT(sSave()));
  connect(_taxLit,           SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_taxType,	     SIGNAL(newID(int)),	             this, SLOT(sCalculateTax()));
  connect(_qtyUOM,           SIGNAL(newID(int)),                     this, SLOT(sQtyUOMChanged()));
  connect(_pricingUOM,       SIGNAL(newID(int)),                     this, SLOT(sPriceUOMChanged()));
  connect(_itemSelected,     SIGNAL(clicked()),                      this, SLOT(sHandleSelection()));
  connect(_miscSelected,     SIGNAL(clicked()),                      this, SLOT(sHandleSelection()));

  _mode = cNew;
  _cmitemid = -1;
  _cmheadid = -1;
  _custid = -1;
  _invoiceNumber = -1;
  _priceRatio = 1.0;
  _qtyShippedCache = 0.0;
  _listPriceCache = 0.0;
  _shiptoid = -1;
  _taxzoneid	= -1;
  _qtyinvuomratio = 1.0;
  _priceinvuomratio = 1.0;
  _invuomid = -1;
  _saved = false;
  _revAccnt->setType(0x08);

  _qtyToCredit->setValidator(omfgThis->qtyVal());
  _qtyReturned->setValidator(omfgThis->qtyVal());
  _qtyShipped->setPrecision(omfgThis->qtyVal());
  _discountFromList->setPrecision(omfgThis->percentVal());
  _discountFromSale->setValidator(new XDoubleValidator(-9999, 100, 2, this));

  _taxType->setEnabled(_privileges->check("OverrideTax"));
  _revAccnt->setEnabled(_privileges->check("CreditMemoItemAccountOverride"));

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  _item->setFocus();
  adjustSize();
}

creditMemoItem::~creditMemoItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void creditMemoItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse creditMemoItem::set(const ParameterList &pParams)
{
  XSqlQuery creditet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  bool     vrestrict = false;


  param = pParams.value("cmhead_id", &valid);
  if (valid)
  {
    _cmheadid = param.toInt();
    creditet.prepare("SELECT * "
              "FROM cmhead "
              "WHERE (cmhead_id=:cmhead_id);");
    creditet.bindValue(":cmhead_id", _cmheadid);
    creditet.exec();
    if (creditet.first())
    {
      _custid = creditet.value("cmhead_cust_id").toInt();
      _shiptoid = creditet.value("cmhead_shipto_id").toInt();
      _orderNumber->setText(creditet.value("cmhead_number").toString());
      if (! creditet.value("cmhead_invcnumber").toString().isEmpty())
        _invoiceNumber = creditet.value("cmhead_invcnumber").toInt();
      if ( (_invoiceNumber != -1) && (_metrics->boolean("RestrictCreditMemos")) )
        vrestrict = true;
      _taxzoneid = creditet.value("cmhead_taxzone_id").toInt();
      _tax->setId(creditet.value("cmhead_curr_id").toInt());
      _tax->setEffective(creditet.value("cmhead_docdate").toDate());
      _netUnitPrice->setId(creditet.value("cmhead_curr_id").toInt());
      _netUnitPrice->setEffective(creditet.value("cmhead_docdate").toDate());
      _rsnCode->setId(creditet.value("cmhead_rsncode_id").toInt());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                                  creditet, __FILE__, __LINE__))
    {
      return UndefinedError;
    }
  }

  param = pParams.value("cmitem_id", &valid);
  if (valid)
  {
    _cmitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      creditet.prepare( "SELECT (COALESCE(MAX(cmitem_linenumber), 0) + 1) AS n_linenumber "
                 "FROM cmitem "
                 "WHERE (cmitem_cmhead_id=:cmhead_id);" );
      creditet.bindValue(":cmhead_id", _cmheadid);
      creditet.exec();
      if (creditet.first())
        _lineNumber->setText(creditet.value("n_linenumber").toString());
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                                    creditet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }

      connect(_discountFromSale, SIGNAL(editingFinished()), this, SLOT(sCalculateFromDiscount()));
      connect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _item->setReadOnly(true);
      _warehouse->setEnabled(false);

      connect(_discountFromSale, SIGNAL(editingFinished()), this, SLOT(sCalculateFromDiscount()));
      connect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(true);
      _warehouse->setEnabled(false);
      _miscGroup->setEnabled(false);
      _qtyReturned->setEnabled(false);
      _qtyToCredit->setEnabled(false);
      _netUnitPrice->setEnabled(false);
      _discountFromSale->setEnabled(false);
      _comments->setReadOnly(true);
      _taxType->setEnabled(false);
      _rsnCode->setEnabled(false);
      _revAccnt->setEnabled(false);

      _save->hide();
      _close->setText(tr("&Close"));
    }
  }

  if (vrestrict)
    _item->setQuery( QString( "SELECT DISTINCT item_id, item_number,"
                              "                (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
                              "                item_upccode,"
                              "                item_descrip1, item_descrip2,"
                              "                item_active, item_config, item_type, uom_name, item_upccode "
                              "FROM invchead, invcitem, item, uom "
                              "WHERE ( (invcitem_invchead_id=invchead_id)"
                              " AND (invcitem_item_id=item_id)"
                              " AND (item_inv_uom_id=uom_id)"
                              " AND (invchead_invcnumber='%1') ) "
                              "ORDER BY item_number" )
                     .arg(_invoiceNumber) );
  else
    _item->addExtraClause( QString("(item_id IN (SELECT custitem FROM custitem(%1, %2) ) )").arg(_custid).arg(_shiptoid) );

  return NoError;
}

void creditMemoItem::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_qtyToCredit->toDouble() == 0.0, _qtyToCredit,
                          tr("<p>You have not selected a quantity of the "
			    "selected item to credit. If you wish to return a "
			    "quantity to stock but not issue a Sales Credit "
			    "then you should enter a Return to Stock "
			    "transaction from the I/M module.  Otherwise, you "
			    "must enter a quantity to credit."))
         << GuiErrorCheck(!_itemSelected->isChecked() && !_itemNumber->text().length(), _itemNumber,
                          tr("<p>You must enter an Item Number for this Miscellaneous Credit Memo Item before you may save it."))
         << GuiErrorCheck(!_itemSelected->isChecked() && !_itemDescrip->toPlainText().length(), _itemDescrip,
                          tr("<p>You must enter a Item Description for this Miscellaneous Credit Memo Item before you may save it."))
         << GuiErrorCheck(!_itemSelected->isChecked() && !_salescat->isValid(), _salescat,
                          tr("<p>You must select a Sales Category for this Miscellaneous Credit Memo Item before you may save it."))
  ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Credit Memo Item"), errors))
    return;

  XSqlQuery getItemSite;
  XSqlQuery creditSave;

  if (_mode == cNew)
  {
    creditSave.exec("SELECT NEXTVAL('cmitem_cmitem_id_seq') AS _cmitem_id");
    if (creditSave.first())
      _cmitemid  = creditSave.value("_cmitem_id").toInt();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Credit Memo Item Information"),
                                  creditSave, __FILE__, __LINE__))
    {
      return;
    }

    creditSave.prepare( "INSERT INTO cmitem "
               "( cmitem_id, cmitem_cmhead_id, cmitem_linenumber, cmitem_itemsite_id,"
               "  cmitem_qtyreturned, cmitem_qtycredit, cmitem_updateinv,"
               "  cmitem_qty_uom_id, cmitem_qty_invuomratio,"
               "  cmitem_price_uom_id, cmitem_price_invuomratio,"
               "  cmitem_unitprice, cmitem_listprice, cmitem_taxtype_id,"
               "  cmitem_comments, cmitem_rsncode_id, "
               "  cmitem_number, cmitem_descrip, cmitem_salescat_id, cmitem_rev_accnt_id ) "
               "SELECT :cmitem_id, :cmhead_id, :cmitem_linenumber, :itemsite_id,"
               "       :cmitem_qtyreturned, :cmitem_qtycredit, :cmitem_updateinv,"
               "       :qty_uom_id, :qty_invuomratio,"
               "       :price_uom_id, :price_invuomratio,"
               "       :cmitem_unitprice, :cmitem_listprice, :cmitem_taxtype_id,"
               "       :cmitem_comments, :cmitem_rsncode_id, "
               "       :cmitem_number, :cmitem_descrip, :cmitem_salescat_id, :cmitem_rev_accnt_id ;");
  }
  else
    creditSave.prepare( "UPDATE cmitem "
               "SET cmitem_qtyreturned=:cmitem_qtyreturned,"
               "    cmitem_qtycredit=:cmitem_qtycredit,"
               "    cmitem_updateinv=:cmitem_updateinv,"
               "    cmitem_qty_uom_id=:qty_uom_id,"
               "    cmitem_qty_invuomratio=:qty_invuomratio,"
               "    cmitem_price_uom_id=:price_uom_id,"
               "    cmitem_price_invuomratio=:price_invuomratio,"
               "    cmitem_unitprice=:cmitem_unitprice,"
               "    cmitem_taxtype_id=:cmitem_taxtype_id,"
               "    cmitem_comments=:cmitem_comments,"
               "    cmitem_rsncode_id=:cmitem_rsncode_id, "
               "    cmitem_number=:cmitem_number, "
               "    cmitem_descrip=:cmitem_descrip, "
               "    cmitem_salescat_id=:cmitem_salescat_id, "
               "    cmitem_rev_accnt_id=:cmitem_rev_accnt_id "
               "WHERE (cmitem_id=:cmitem_id);" );

  creditSave.bindValue(":cmitem_id", _cmitemid);
  creditSave.bindValue(":cmhead_id", _cmheadid);
  creditSave.bindValue(":cmitem_linenumber", _lineNumber->text().toInt());
  creditSave.bindValue(":cmitem_qtyreturned", _qtyReturned->toDouble());
  creditSave.bindValue(":cmitem_qtycredit", _qtyToCredit->toDouble());
  creditSave.bindValue(":cmitem_updateinv", QVariant(_updateInv->isChecked()));
  if(!_miscSelected->isChecked())
    creditSave.bindValue(":qty_uom_id", _qtyUOM->id());
  creditSave.bindValue(":qty_invuomratio", _qtyinvuomratio);
  if(!_miscSelected->isChecked())
    creditSave.bindValue(":price_uom_id", _pricingUOM->id());
  creditSave.bindValue(":price_invuomratio", _priceinvuomratio);
  creditSave.bindValue(":cmitem_unitprice", _netUnitPrice->localValue());
  creditSave.bindValue(":cmitem_listprice", _listPrice->baseValue());
  if (_taxType->isValid())
    creditSave.bindValue(":cmitem_taxtype_id",	_taxType->id());
  creditSave.bindValue(":cmitem_comments", _comments->toPlainText());
  creditSave.bindValue(":cmitem_rsncode_id", _rsnCode->id());
  if (_itemSelected->isChecked())
  {
    getItemSite.prepare("SELECT itemsite_id FROM itemsite "
                        "WHERE ((itemsite_item_id=:item_id) "
                        " AND   (itemsite_warehous_id=:warehous_id));");
    getItemSite.bindValue(":item_id", _item->id());
    getItemSite.bindValue(":warehous_id", _warehouse->id());
    getItemSite.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Determine Item Site"),
                             getItemSite, __FILE__, __LINE__))
    {
      return;
    }
    else
    {
       if (getItemSite.first())
         creditSave.bindValue(":itemsite_id", getItemSite.value("itemsite_id").toInt());
    }
  }
  else
  {
    creditSave.bindValue(":cmitem_number", _itemNumber->text());
    creditSave.bindValue(":cmitem_descrip", _itemDescrip->toPlainText());
    creditSave.bindValue(":cmitem_salescat_id", _salescat->id());
  }
  if (_revAccnt->id() > 0)
    creditSave.bindValue(":cmitem_rev_accnt_id", _revAccnt->id());
  creditSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Credit Memo Item Information"),
                                creditSave, __FILE__, __LINE__))
  {
    return;
  }

  done(_cmitemid);
}

void creditMemoItem::sPopulateItemInfo()
{
  // Get list of active, valid Selling UOMs
  sPopulateUOM();

  XSqlQuery item;
  item.prepare( "SELECT item_inv_uom_id, item_price_uom_id,"
                "       iteminvpricerat(item_id) AS iteminvpricerat,"
                "       listPrice(item_id, :cust_id, :shipto_id, :whsid) AS listprice,"
                "       stdCost(item_id) AS f_cost,"
		            "       getItemTaxType(item_id, :taxzone) AS taxtype_id "
                "  FROM item"
                " WHERE (item_id=:item_id);" );
  item.bindValue(":item_id", _item->id());
  item.bindValue(":taxzone", _taxzoneid);
  item.bindValue(":whsid",   _warehouse->id());
  item.bindValue(":cust_id", _custid);
  item.bindValue(":shipto_id", _shiptoid);
  item.exec();
  if (item.first())
  {
    _invuomid = item.value("item_inv_uom_id").toInt();
    _priceRatio = item.value("iteminvpricerat").toDouble();
    _qtyUOM->setId(item.value("item_inv_uom_id").toInt());
    _pricingUOM->setId(item.value("item_price_uom_id").toInt());
    _priceinvuomratio = item.value("iteminvpricerat").toDouble();
    _qtyinvuomratio = 1.0;
    _ratio=item.value("iteminvpricerat").toDouble();
    // {_listPrice,_unitCost}->setBaseValue() because they're stored in base
    _listPriceCache = item.value("listprice").toDouble();
    _listPrice->setBaseValue(_listPriceCache);
    _unitCost->setBaseValue(item.value("f_cost").toDouble());
    _taxType->setId(item.value("taxtype_id").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Item Information"),
                                item, __FILE__, __LINE__))
  {
    return;
  }

  if (_invoiceNumber != -1)
  {
    XSqlQuery cmitem;
    cmitem.prepare("SELECT invcitem_warehous_id,"
                   "       invcitem_qty_uom_id, invcitem_qty_invuomratio,"
                   "       invcitem_price_uom_id, invcitem_price_invuomratio,"
                   "       invcitem_billed * invcitem_qty_invuomratio AS f_billed,"
                   "       currToCurr(invchead_curr_id, :curr_id, "
                   "                  invcitem_price / invcitem_price_invuomratio, invchead_invcdate) AS invcitem_price_local "
                   "FROM invchead, invcitem "
                   "WHERE ( (invcitem_invchead_id=invchead_id)"
                   " AND (invchead_invcnumber=text(:invoiceNumber))"
                   " AND (invcitem_item_id=:item_id) ) "
                   "LIMIT 1;" );
    cmitem.bindValue(":invoiceNumber", _invoiceNumber);
    cmitem.bindValue(":item_id", _item->id());
    cmitem.bindValue(":curr_id", _netUnitPrice->id());
    cmitem.exec();
    if (cmitem.first())
    {
      _qtyUOM->setId(cmitem.value("invcitem_qty_uom_id").toInt());
      _pricingUOM->setId(cmitem.value("invcitem_price_uom_id").toInt());
      _priceinvuomratio = cmitem.value("invcitem_price_invuomratio").toDouble();
      _ratio=_priceinvuomratio;
      _salePrice->setLocalValue(cmitem.value("invcitem_price_local").toDouble() * _priceinvuomratio);

      if (_mode == cNew)
        _netUnitPrice->setLocalValue(cmitem.value("invcitem_price_local").toDouble() * _priceinvuomratio);

      _warehouse->setId(cmitem.value("invcitem_warehous_id").toInt());
      _qtyShippedCache = cmitem.value("f_billed").toDouble();
      _qtyShipped->setDouble(cmitem.value("f_billed").toDouble() / _qtyinvuomratio);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Item Information"),
                                  cmitem, __FILE__, __LINE__))
    {
      _salePrice->clear();
      return;
    }
  }
}

void creditMemoItem::sPopulateItemsiteInfo()
{
  XSqlQuery itemsite;
  itemsite.prepare( "SELECT itemsite_controlmethod, itemsite_costmethod"
                "  FROM itemsite"
                " WHERE ( (itemsite_item_id=:item_id)"
                "   AND   (itemsite_warehous_id=:warehous_id) );" );
  itemsite.bindValue(":item_id", _item->id());
  itemsite.bindValue(":warehous_id", _warehouse->id());
  itemsite.exec();
  if (itemsite.first())
  {
    if (itemsite.value("itemsite_controlmethod").toString() == "N" ||
        itemsite.value("itemsite_costmethod").toString() == "J")
    {
      _qtyReturned->setDouble(0.0);
      _qtyReturned->setEnabled(false);
      _updateInv->setChecked(false);
      _updateInv->setEnabled(false);
    }
    else
    {
      _qtyReturned->clear();
      _qtyReturned->setEnabled(true);
      _updateInv->setChecked(true);
      _updateInv->setEnabled(true);
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Item Information"),
                                itemsite, __FILE__, __LINE__))
  {
    return;
  }
}

void creditMemoItem::populate()
{
  XSqlQuery cmitem;
  cmitem.prepare("SELECT cmitem.*,  "
                 "       cmhead_taxzone_id, cmhead_curr_id, "
                 "      (SELECT SUM(taxhist_tax * -1) "
		 "       FROM cmitemtax WHERE (cmitem_id=taxhist_parent_id)) AS tax,"
                 "       itemsite_warehous_id, itemsite_costmethod "
                 "FROM cmhead, cmitem "
                 "LEFT OUTER JOIN itemsite ON (cmitem_itemsite_id=itemsite_id)"
                 "WHERE ( (cmitem_cmhead_id=cmhead_id)"
                 "  AND   (cmitem_id=:cmitem_id) );" );
  cmitem.bindValue(":cmitem_id", _cmitemid);
  cmitem.exec();
  if (cmitem.first())
  {
    _cmheadid = cmitem.value("cmitem_cmhead_id").toInt();
    _taxzoneid = cmitem.value("cmhead_taxzone_id").toInt();
    _rsnCode->setId(cmitem.value("cmitem_rsncode_id").toInt());
    _revAccnt->setId(cmitem.value("cmitem_rev_accnt_id").toInt());

    if (cmitem.value("cmitem_itemsite_id").toInt() > 0)
    {
      _itemSelected->setChecked(true);
      _item->setItemsiteid(cmitem.value("cmitem_itemsite_id").toInt());
      _warehouse->setId(cmitem.value("itemsite_warehous_id").toInt());
    }
    else
    {
      _miscSelected->setChecked(true);
      _itemNumber->setText(cmitem.value("cmitem_number"));
      _itemDescrip->setText(cmitem.value("cmitem_descrip").toString());
      _salescat->setId(cmitem.value("cmitem_salescat_id").toInt());
      _qtyReturned->setEnabled(false);
    }
    _lineNumber->setText(cmitem.value("cmitem_linenumber").toString());
    _netUnitPrice->setLocalValue(cmitem.value("cmitem_unitprice").toDouble());
    _qtyToCredit->setDouble(cmitem.value("cmitem_qtycredit").toDouble());
    _qtyReturned->setDouble(cmitem.value("cmitem_qtyreturned").toDouble());
    // TODO: should this include itemsite_
    if (cmitem.value("cmitem_raitem_id").toInt() > 0 ||
        cmitem.value("itemsite_costmethod").toString() == "J")
    {
      _updateInv->setChecked(false);
      _updateInv->setEnabled(false);
      _qtyReturned->setEnabled(false);
    }
    else
    {
      _updateInv->setChecked(cmitem.value("cmitem_updateinv").toBool());
      _updateInv->setEnabled(true);
      _qtyReturned->setEnabled(true);
    }
    _qtyUOM->setId(cmitem.value("cmitem_qty_uom_id").toInt());
    _ratio=cmitem.value("cmitem_qty_invuomratio").toDouble();
    _pricingUOM->setId(cmitem.value("cmitem_price_uom_id").toInt());
    _priceinvuomratio = cmitem.value("cmitem_price_invuomratio").toDouble();
    _comments->setText(cmitem.value("cmitem_comments").toString());
    _taxType->setId(cmitem.value("cmitem_taxtype_id").toInt());
    _tax->setId(cmitem.value("cmhead_curr_id").toInt());
    _tax->setLocalValue(cmitem.value("tax").toDouble());
    sCalculateDiscountPrcnt();
    _listPrices->setEnabled(true);
    _saved=true;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Item Information"),
                                cmitem, __FILE__, __LINE__))
  {
    return;
  }
}

void creditMemoItem::sCalculateExtendedPrice()
{
  _extendedPrice->setLocalValue(((_qtyToCredit->toDouble() * _qtyinvuomratio) / _priceinvuomratio) * _netUnitPrice->localValue());
  sCalculateTax();
}

void creditMemoItem::sCalculateDiscountPrcnt()
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
      _discountFromList->setDouble((1 - (unitPrice / _listPrice->localValue())) * 100);

    if (_salePrice->isZero())
      _discountFromSale->setText("N/A");
    else
      _discountFromSale->setDouble((1 - (unitPrice / _salePrice->localValue())) * 100);
  }
}

void creditMemoItem::sCalculateFromDiscount()
{
  double discount = _discountFromSale->toDouble() / 100.0;

  if (_salePrice->isZero())
    _discountFromSale->setText(tr("N/A"));
  else
    _netUnitPrice->setLocalValue(_salePrice->localValue() - (_salePrice->localValue() * discount));
}

void creditMemoItem::sPriceGroup()
{
  if (! omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_netUnitPrice->currAbbr()));
}

void creditMemoItem::sListPrices()
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
    _netUnitPrice->setLocalValue(newdlg._selectedPrice * (_priceinvuomratio / _priceRatio));
    sCalculateDiscountPrcnt();
  }
}

void creditMemoItem::sCalculateTax()
{
  _saved = false;
  XSqlQuery calcq;
  calcq.prepare( "SELECT calculateTax(cmhead_taxzone_id,:taxtype_id,cmhead_docdate,cmhead_curr_id,ROUND(:ext,2)) AS tax "
                 "FROM cmhead "
                 "WHERE (cmhead_id=:cmhead_id); ");
  calcq.bindValue(":cmhead_id", _cmheadid);
  calcq.bindValue(":taxtype_id", _taxType->id());
  calcq.bindValue(":ext", _extendedPrice->localValue());
  calcq.exec();
  if (calcq.first())
    _tax->setLocalValue(calcq.value("tax").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                                calcq, __FILE__, __LINE__))
  {
    return;
  }
}

void creditMemoItem::sTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("taxzone_id", _taxzoneid);
  params.append("taxtype_id", _taxType->id());
  params.append("date", _netUnitPrice->effective());
  params.append("subtotal", _extendedPrice->localValue());
  params.append("curr_id",  _tax->id());
  params.append("sense", -1);

  if(cView == _mode)
    params.append("readOnly");

  if(_saved == true)
  {
    params.append("order_id", _cmitemid);
    params.append("order_type", "CI");
  }

  newdlg.set(params);

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    if (_taxType->id() != newdlg.taxtype())
      _taxType->setId(newdlg.taxtype());
  }
}

void creditMemoItem::sPopulateUOM()
{
  if (_item->id() != -1)
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
    
    // Also have to factor UOMs previously used on Sales Credit now inactive
    if (_cmitemid != -1)
    {
      XSqlQuery cmuom;
      cmuom.prepare("SELECT cmitem_qty_uom_id, cmitem_price_uom_id "
                    "  FROM cmitem"
                    " WHERE(cmitem_id=:cmitem_id);");
      cmuom.bindValue(":cmitem_id", _cmitemid);
      cmuom.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Sales Credit UOMs"),
                               cmuom, __FILE__, __LINE__))
        return;
      else if (cmuom.first())
      {
        params.append("uom_id", cmuom.value("cmitem_qty_uom_id"));
        params.append("uom_id2", cmuom.value("cmitem_price_uom_id"));
      }
    }
    
    XSqlQuery uom = muom.toQuery(params);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting UOMs"),
                             uom, __FILE__, __LINE__))
      return;
    
    int saveqtyuomid = _qtyUOM->id();
    int savepriceuomid = _pricingUOM->id();
    disconnect(_qtyUOM,     SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
    disconnect(_pricingUOM, SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));
    _qtyUOM->populate(uom);
    _pricingUOM->populate(uom);
    _qtyUOM->setId(saveqtyuomid);
    _pricingUOM->setId(savepriceuomid);
    connect(_qtyUOM,     SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
    connect(_pricingUOM, SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));
  }
}

void creditMemoItem::sQtyUOMChanged()
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
  
  if(_qtyUOM->id() == _invuomid)
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
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                         invuom, __FILE__, __LINE__);
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

void creditMemoItem::sPriceUOMChanged()
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
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Credit Memo Information"),
                         invuom, __FILE__, __LINE__);
  }
  _ratio=_priceinvuomratio;

  _listPrice->setBaseValue(_listPriceCache * (_priceinvuomratio / _priceRatio));
}

void creditMemoItem::sHandleSelection()
{
  _itemGroup->setEnabled(_itemSelected->isChecked());
  _qtyReturned->setEnabled(_itemSelected->isChecked());
  _updateInv->setEnabled(_itemSelected->isChecked());
  _miscGroup->setEnabled(!_itemSelected->isChecked());
}
