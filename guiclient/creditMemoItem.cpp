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

#include "creditMemoItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "priceList.h"
#include "taxDetail.h"

creditMemoItem::creditMemoItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

#ifndef Q_WS_MAC
  _listPrices->setMaximumWidth(25);
#endif

  connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
  //connect(_extendedPrice, SIGNAL(valueChanged()), this, SLOT(sLookupTax()));
  connect(_item,	  SIGNAL(newId(int)),     this, SLOT(sPopulateItemInfo()));
  connect(_listPrices,	  SIGNAL(clicked()),      this, SLOT(sListPrices()));
  connect(_netUnitPrice,  SIGNAL(valueChanged()), this, SLOT(sCalculateDiscountPrcnt()));
  connect(_netUnitPrice,  SIGNAL(valueChanged()), this, SLOT(sCalculateExtendedPrice()));
  connect(_netUnitPrice,  SIGNAL(idChanged(int)), this, SLOT(sPriceGroup()));
  connect(_qtyToCredit,	  SIGNAL(textChanged(const QString&)), this, SLOT(sCalculateExtendedPrice()));
  connect(_save,	  SIGNAL(clicked()),      this, SLOT(sSave()));
  connect(_taxCode,	  SIGNAL(newID(int)),	  this, SLOT(sLookupTax()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_taxType,	  SIGNAL(newID(int)),	  this, SLOT(sLookupTaxCode()));
  connect(_qtyUOM, SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
  connect(_pricingUOM, SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));

  _mode = cNew;
  _cmitemid = -1;
  _cmheadid = -1;
  _custid = -1;
  _invoiceNumber = -1;
  _priceRatio = 1.0;
  _qtyShippedCache = 0.0;
  _shiptoid = -1;
  _taxauthid	= -1;
  _taxCache.clear();
  _qtyinvuomratio = 1.0;
  _priceinvuomratio = 1.0;
  _invuomid = -1;

  _qtyToCredit->setValidator(omfgThis->qtyVal());
  _qtyReturned->setValidator(omfgThis->qtyVal());
  _discountFromSale->setValidator(new QDoubleValidator(-9999, 100, 2, this));

  _taxType->setEnabled(_privileges->check("OverrideTax"));
  _taxCode->setEnabled(_privileges->check("OverrideTax"));
  
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
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
  QVariant param;
  bool     valid;
  bool     restrict = FALSE;

  param = pParams.value("cmhead_id", &valid);
  if (valid)
  {
    _cmheadid = param.toInt();
    q.prepare("SELECT cmhead_taxauth_id,"
	      "       COALESCE(cmhead_tax_curr_id, cmhead_curr_id) AS taxcurr "
	      "FROM cmhead "
	      "WHERE (cmhead_id=:cmhead_id);");
    q.bindValue(":cmhead_id", _cmheadid);
    q.exec();
    if (q.first())
    {
      _taxauthid = q.value("cmhead_taxauth_id").toInt();
      _tax->setId(q.value("taxcurr").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("rsncode_id", &valid);
  if (valid)
    _rsnCode->setId(param.toInt());

  param = pParams.value("cust_id", &valid);
  if (valid)
    _custid = param.toInt();

  param = pParams.value("shipto_id", &valid);
  if (valid)
    _shiptoid = param.toInt();

  param = pParams.value("invoiceNumber", &valid);
  if (valid)
  {
    if ( (param.toInt() == 0) || (param.toInt() == -1) )
      _invoiceNumber = -1;
    else
    {
      _invoiceNumber = param.toInt();

      if (_metrics->boolean("RestrictCreditMemos"))
        restrict = TRUE;
    }
  }

  param = pParams.value("creditMemoNumber", &valid);
  if (valid)
    _orderNumber->setText(param.toString());

  param = pParams.value("curr_id", &valid);
  if (valid)
    _netUnitPrice->setId(param.toInt());

  param = pParams.value("effective", &valid);
  if (valid)
    _netUnitPrice->setEffective(param.toDate());

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

      q.prepare( "SELECT (COALESCE(MAX(cmitem_linenumber), 0) + 1) AS n_linenumber "
                 "FROM cmitem "
                 "WHERE (cmitem_cmhead_id=:cmhead_id);" );
      q.bindValue(":cmhead_id", _cmheadid);
      q.exec();
      if (q.first())
        _lineNumber->setText(q.value("n_linenumber").toString());
      else if (q.lastError().type() == QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
      connect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));

      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qtyReturned->setFocus();

      connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
      connect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qtyReturned->setEnabled(FALSE);
      _qtyToCredit->setEnabled(FALSE);
      _netUnitPrice->setEnabled(FALSE);
      _discountFromSale->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _taxType->setEnabled(FALSE);
      _taxCode->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);

      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  if (restrict)
    _item->setQuery( QString( "SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2, item_active,"
                              "                item_config, item_type, uom_name "
                              "FROM invchead, invcitem, item, uom "
                              "WHERE ( (invcitem_invchead_id=invchead_id)"
                              " AND (invcitem_item_id=item_id)"
                              " AND (item_inv_uom_id=uom_id)"
                              " AND (invchead_invcnumber=%1) ) "
                              "ORDER BY item_number;" )
                     .arg(_invoiceNumber) );
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

  return NoError;
}

void creditMemoItem::sSave()
{
  if (_qtyToCredit->toDouble() == 0.0)
  {
    QMessageBox::warning(this, tr("Invalid Credit Quantity"),
                         tr("<p>You have not selected a quantity of the "
			    "selected item to credit. If you wish to return a "
			    "quantity to stock but not issue a Credit Memo "
			    "then you should enter a Return to Stock "
			    "transaction from the I/M module.  Otherwise, you "
			    "must enter a quantity to credit." ));
    _qtyToCredit->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('cmitem_cmitem_id_seq') AS _cmitem_id");
    if (q.first())
      _cmitemid  = q.value("_cmitem_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO cmitem "
               "( cmitem_id, cmitem_cmhead_id, cmitem_linenumber, cmitem_itemsite_id,"
               "  cmitem_qtyreturned, cmitem_qtycredit,"
               "  cmitem_qty_uom_id, cmitem_qty_invuomratio,"
               "  cmitem_price_uom_id, cmitem_price_invuomratio,"
               "  cmitem_unitprice, cmitem_tax_id, cmitem_taxtype_id,"
	       "  cmitem_tax_pcta, cmitem_tax_pctb, cmitem_tax_pctc,"
	       "  cmitem_tax_ratea, cmitem_tax_rateb, cmitem_tax_ratec,"
               "  cmitem_comments, cmitem_rsncode_id ) "
               "SELECT :cmitem_id, :cmhead_id, :cmitem_linenumber, itemsite_id,"
               "       :cmitem_qtyreturned, :cmitem_qtycredit,"
               "       :qty_uom_id, :qty_invuomratio,"
               "       :price_uom_id, :price_invuomratio,"
               "       :cmitem_unitprice, :cmitem_tax_id, :cmitem_taxtype_id,"
	       "       :cmitem_tax_pcta, :cmitem_tax_pctb, :cmitem_tax_pctc,"
	       "       :cmitem_tax_ratea, :cmitem_tax_rateb, :cmitem_tax_ratec,"
	       "       :cmitem_comments, :cmitem_rsncode_id "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
  }
  else
    q.prepare( "UPDATE cmitem "
               "SET cmitem_qtyreturned=:cmitem_qtyreturned, cmitem_qtycredit=:cmitem_qtycredit,"
               "    cmitem_qty_uom_id=:qty_uom_id,"
               "    cmitem_qty_invuomratio=:qty_invuomratio,"
               "    cmitem_price_uom_id=:price_uom_id,"
               "    cmitem_price_invuomratio=:price_invuomratio,"
               "    cmitem_unitprice=:cmitem_unitprice,"
	       "    cmitem_tax_id=:cmitem_tax_id,"
	       "    cmitem_taxtype_id=:cmitem_taxtype_id,"
	       "    cmitem_tax_pcta=:cmitem_tax_pcta,"
	       "    cmitem_tax_pctb=:cmitem_tax_pctb,"
	       "    cmitem_tax_pctc=:cmitem_tax_pctc,"
	       "    cmitem_tax_ratea=:cmitem_tax_ratea,"
	       "    cmitem_tax_rateb=:cmitem_tax_rateb,"
	       "    cmitem_tax_ratec=:cmitem_tax_ratec,"
	       "    cmitem_comments=:cmitem_comments,"
               "    cmitem_rsncode_id=:cmitem_rsncode_id "
               "WHERE (cmitem_id=:cmitem_id);" );

  q.bindValue(":cmitem_id", _cmitemid);
  q.bindValue(":cmhead_id", _cmheadid);
  q.bindValue(":cmitem_linenumber", _lineNumber->text().toInt());
  q.bindValue(":cmitem_qtyreturned", _qtyReturned->toDouble());
  q.bindValue(":cmitem_qtycredit", _qtyToCredit->toDouble());
  q.bindValue(":qty_uom_id", _qtyUOM->id());
  q.bindValue(":qty_invuomratio", _qtyinvuomratio);
  q.bindValue(":price_uom_id", _pricingUOM->id());
  q.bindValue(":price_invuomratio", _priceinvuomratio);
  q.bindValue(":cmitem_unitprice", _netUnitPrice->localValue());
  if (_taxCode->isValid())
    q.bindValue(":cmitem_tax_id",	_taxCode->id());
  if (_taxType->isValid())
    q.bindValue(":cmitem_taxtype_id",	_taxType->id());
  q.bindValue(":cmitem_tax_pcta",	_taxCache.linePct(0));
  q.bindValue(":cmitem_tax_pctb",	_taxCache.linePct(1));
  q.bindValue(":cmitem_tax_pctc",	_taxCache.linePct(2));
  q.bindValue(":cmitem_tax_ratea",	_taxCache.line(0));
  q.bindValue(":cmitem_tax_rateb",	_taxCache.line(1));
  q.bindValue(":cmitem_tax_ratec",	_taxCache.line(2));
  q.bindValue(":cmitem_comments", _comments->text());
  q.bindValue(":cmitem_rsncode_id", _rsnCode->id());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_cmitemid);
}

void creditMemoItem::sPopulateItemInfo()
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
  item.exec();
  if (item.first())
  {
    _invuomid = item.value("item_inv_uom_id").toInt();
    _priceRatio = item.value("iteminvpricerat").toDouble();
    _qtyUOM->setId(item.value("item_inv_uom_id").toInt());
    _pricingUOM->setId(item.value("item_price_uom_id").toInt());
    _priceinvuomratio = item.value("iteminvpricerat").toDouble();
    _qtyinvuomratio = 1.0;
    _ratio->setText(item.value("f_invpricerat").toString());
    // {_listPrice,_unitCost}->setBaseValue() because they're stored in base
    _listPrice->setBaseValue(item.value("item_listprice").toDouble());
    _unitCost->setBaseValue(item.value("f_cost").toDouble());
    _taxType->setId(item.value("taxtype_id").toInt());
  }
  else if (item.lastError().type() != QSqlError::None)
  {
    systemError(this, item.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_invoiceNumber != -1)
  {
    XSqlQuery cmitem;
    cmitem.prepare( "SELECT invcitem_warehous_id,"
                    "       invcitem_qty_uom_id, invcitem_qty_invuomratio,"
                    "       invcitem_price_uom_id, invcitem_price_invuomratio,"
                    "       invcitem_billed * invcitem_qty_invuomratio AS f_billed,"
                    "       currToCurr(invchead_curr_id, :curr_id, "
		    "                  invcitem_price / invcitem_price_invuomratio, invchead_invcdate) AS invcitem_price_local "
                    "FROM invchead, invcitem "
                    "WHERE ( (invcitem_invchead_id=invchead_id)"
                    " AND (invchead_invcnumber=:invoiceNumber)"
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
      _ratio->setText(formatUOMRatio(_priceinvuomratio));
      _salePrice->setLocalValue(cmitem.value("invcitem_price_local").toDouble() * _priceinvuomratio);

      if (_mode == cNew)
        _netUnitPrice->setLocalValue(cmitem.value("invcitem_price_local").toDouble() * _priceinvuomratio);

      _warehouse->setId(cmitem.value("invcitem_warehous_id").toInt());
      _qtyShippedCache = cmitem.value("f_billed").toDouble();
      _qtyShipped->setText(formatQty(cmitem.value("f_billed").toDouble() / _qtyinvuomratio));
    }
    else if (cmitem.lastError().type() != QSqlError::None)
    {
      systemError(this, cmitem.lastError().databaseText(), __FILE__, __LINE__);
      _salePrice->clear();
      return;
    }
  }
}

void creditMemoItem::populate()
{
  XSqlQuery cmitem;
  cmitem.prepare("SELECT cmitem.*, "
                 "       formatQty(cmitem_qtycredit) AS tocredit,"
                 "       formatQty(cmitem_qtyreturned) AS toreturn,"
		 "       cmhead_taxauth_id,"
		 "       COALESCE(cmhead_tax_curr_id, cmhead_curr_id) AS taxcurr "

                 "FROM cmitem, cmhead "
                 "WHERE ((cmitem_cmhead_id=cmhead_id)"
		 "  AND  (cmitem_id=:cmitem_id));" );
  cmitem.bindValue(":cmitem_id", _cmitemid);
  cmitem.exec();
  if (cmitem.first())
  {
    _netUnitPrice->setLocalValue(cmitem.value("cmitem_unitprice").toDouble());
    // do _item and _taxauth before other tax stuff because of signal cascade
    _taxauthid = cmitem.value("cmhead_taxauth_id").toInt();
    _tax->setId(cmitem.value("taxcurr").toInt());
    _item->setItemsiteid(cmitem.value("cmitem_itemsite_id").toInt());
    _lineNumber->setText(cmitem.value("cmitem_linenumber").toString());
    _qtyToCredit->setText(cmitem.value("tocredit").toString());
    _qtyReturned->setText(cmitem.value("toreturn").toString());
    _comments->setText(cmitem.value("cmitem_comments").toString());
    _taxCode->setId(cmitem.value("cmitem_tax_id").toInt());
    _taxType->setId(cmitem.value("cmitem_taxtype_id").toInt());
    _taxCache.setLinePct(cmitem.value("cmitem_tax_pcta").toDouble(),
			 cmitem.value("cmitem_tax_pctb").toDouble(),
			 cmitem.value("cmitem_tax_pctc").toDouble());
    _taxCache.setLine(cmitem.value("cmitem_tax_ratea").toDouble(),
		      cmitem.value("cmitem_tax_rateb").toDouble(),
		      cmitem.value("cmitem_tax_ratec").toDouble());
    _tax->setLocalValue(_taxCache.total());

    _rsnCode->setId(cmitem.value("cmitem_rsncode_id").toInt());

    sCalculateDiscountPrcnt();
  }
  else if (cmitem.lastError().type() != QSqlError::None)
  {
    systemError(this, cmitem.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void creditMemoItem::sCalculateExtendedPrice()
{
  _extendedPrice->setLocalValue(((_qtyToCredit->toDouble() * _qtyinvuomratio) / _priceinvuomratio) * _netUnitPrice->localValue());
  sLookupTax();
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
      _discountFromList->setText(formatPercent(1 - (unitPrice / _listPrice->localValue())) );

    if (_salePrice->isZero())
      _discountFromSale->setText("N/A");
    else
      _discountFromSale->setText(formatPercent(1 - (unitPrice / _salePrice->localValue())) );
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
	_netUnitPrice->setLocalValue(q.value("price").toDouble() * (_priceinvuomratio / _priceRatio));
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
      _netUnitPrice->setLocalValue(newdlg._selectedPrice * (_priceinvuomratio / _priceRatio));
      sCalculateDiscountPrcnt();
    }
  }
}

void creditMemoItem::sLookupTax()
{
  XSqlQuery calcq;
  calcq.prepare("SELECT currToCurr(:extcurr, COALESCE(taxauth_curr_id, :extcurr),"
		"         calculateTax(:tax_id, :ext, 0, 'A'), :date) AS valA,"
		"       currToCurr(:extcurr, COALESCE(taxauth_curr_id, :extcurr),"
		"         calculateTax(:tax_id, :ext, 0, 'B'), :date) AS valB,"
		"       currToCurr(:extcurr, COALESCE(taxauth_curr_id, :extcurr),"
		"         calculateTax(:tax_id, :ext, 0, 'C'), :date) AS valC "
		"FROM taxauth "
		"WHERE (taxauth_id=:auth);");

  calcq.bindValue(":extcurr", _extendedPrice->id());
  calcq.bindValue(":tax_id",  _taxCode->id());
  calcq.bindValue(":ext",     _extendedPrice->localValue());
  calcq.bindValue(":date",    _extendedPrice->effective());
  calcq.bindValue(":auth",    _taxauthid);
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

void creditMemoItem::sTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("tax_id",   _taxCode->id());
  params.append("curr_id",  _tax->id());
  params.append("valueA",   _taxCache.line(0));
  params.append("valueB",   _taxCache.line(1));
  params.append("valueC",   _taxCache.line(2));
  params.append("pctA",	    _taxCache.linePct(0));
  params.append("pctB",	    _taxCache.linePct(1));
  params.append("pctC",	    _taxCache.linePct(2));
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

void creditMemoItem::sLookupTaxCode()
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

void creditMemoItem::sQtyUOMChanged()
{
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

void creditMemoItem::sPriceUOMChanged()
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
  _ratio->setText(formatUOMRatio(_priceinvuomratio));

  updatePriceInfo();
}

void creditMemoItem::updatePriceInfo()
{
  XSqlQuery item;
  item.prepare("SELECT item_listprice"
               "  FROM item"
               " WHERE(item_id=:item_id);");
  item.bindValue(":item_id", _item->id());
  item.exec();
  item.first();
  _listPrice->setBaseValue(item.value("item_listprice").toDouble() * (_priceinvuomratio / _priceRatio));
}
