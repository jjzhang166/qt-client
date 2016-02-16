  /*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "priceList.h"

#include <QSqlError>
#include <QVariant>

#include "mqlutil.h"

priceList::priceList(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XDialog(parent, name, fl)
{
  setupUi(this);

  connect(_cust,   SIGNAL(newId(int)),        this,    SLOT(sNewCust()));
  connect(_item,   SIGNAL(newId(int)),        this,    SLOT(sNewItem()));
  connect(_warehouse, SIGNAL(newID(int)),     this,    SLOT(sNewItem()));
  connect(_close,  SIGNAL(clicked()),         this,    SLOT(reject()));
  connect(_price,  SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
  connect(_price,  SIGNAL(itemSelected(int)), this,    SLOT(sSelect()));
  connect(_price,  SIGNAL(valid(bool)),       _select, SLOT(setEnabled(bool)));
  connect(_select, SIGNAL(clicked()),         this,    SLOT(sSelect()));

  _price->addColumn(tr("Schedule"),                  -1, Qt::AlignLeft,  true, "schedulename");
  _price->addColumn(tr("Source"),           _itemColumn, Qt::AlignLeft,  true, "type");
  _price->addColumn(tr("Qty. Break"),        _qtyColumn, Qt::AlignRight, true, "qty_break");
  _price->addColumn(tr("Qty. UOM"),          _qtyColumn, Qt::AlignRight, true, "qty_uom");
  _price->addColumn(tr("Price"),           _priceColumn, Qt::AlignRight, true, "base_price");
  _price->addColumn(tr("Price UOM"),       _priceColumn, Qt::AlignRight, true, "price_uom");
  _price->addColumn(tr("Percent"),         _prcntColumn, Qt::AlignRight, true, "discountpercent");
  _price->addColumn(tr("Fixed Amt."),      _priceColumn, Qt::AlignRight, true, "discountfixed");
  _price->addColumn(tr("Type"),             _itemColumn, Qt::AlignLeft,  true, "price_type");
  _price->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignLeft,  true, "currency");
  _price->addColumn(tr("Price (in Base)"), _priceColumn, Qt::AlignRight, true, "price");
  // column title reset in priceList::set

  if (omfgThis->singleCurrency())
  {
    _price->hideColumn(_price->column("price"));
    _price->hideColumn(_price->column("currency"));
  }

  _shiptoid = -1;
  _shiptonum = "";
  _prodcatid = -1;
  _custtypeid = -1;
  _custtypecode = "";
  _iteminvpricerat = 1.0;

  _qty->setValidator(omfgThis->qtyVal());
  _listPrice->setPrecision(omfgThis->priceVal());
  _listCost->setPrecision(omfgThis->priceVal());
  _unitCost->setPrecision(omfgThis->costVal());
  _unitCost->setVisible(_privileges->check("ViewSOItemUnitCost"));
  _unitCostLit->setVisible(_privileges->check("ViewSOItemUnitCost"));

}

priceList::~priceList()
{
  // no need to delete child widgets, Qt does it all for us
}

void priceList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse priceList::set(const ParameterList &pParams)
{
  XSqlQuery priceet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _cust->setId(param.toInt());
    _cust->setReadOnly(true);
  }

  param = pParams.value("shipto_id", &valid);
  if (valid)
  {
    _shiptoid = param.toInt();
    sNewShipto();
  }

  param = pParams.value("warehouse_id", &valid);
  if (valid)
  {
    _warehouse->setId(param.toInt());
    _warehouse->setEnabled(false);
  }
    
  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _curr_id = param.toInt();
    if (! omfgThis->singleCurrency())
    {
      priceet.prepare("SELECT currConcat(:curr_id) AS currConcat;");
      priceet.bindValue(":curr_id", _curr_id);
      priceet.exec();
      if (priceet.first())
        _price->headerItem()->setText(_price->column("price"),
                                      tr("Price\n(in %1)")
                                        .arg(priceet.value("currConcat").toString()));
    }
  }

  param = pParams.value("effective", &valid);
  _effective = (valid) ? param.toDate() : QDate::currentDate();

  param = pParams.value("asof", &valid);
  _asOf = (valid) ? param.toDate() : QDate::currentDate();

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _qty->setDouble(param.toDouble());
    _qty->setEnabled(false);
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(true);
  }
  
  sFillList();

  return NoError;
}

void priceList::sNewCust()
{
  if (_cust->isValid())
  {
    XSqlQuery custq;
    custq.prepare("SELECT custtype_id, custtype_code"
                  "  FROM custinfo LEFT OUTER JOIN custtype ON (custtype_id=cust_custtype_id)"
                  " WHERE (cust_id=:id);");
    custq.bindValue(":id", _cust->id());
    custq.exec();
    if (custq.first())
    {
      _custtypeid = custq.value("custtype_id").toInt();
      _custtypecode = custq.value("custtype_code").toString();
    }
    else if (custq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, custq.lastError().text(), __FILE__, __LINE__);
      return;
    }
  }
}

void priceList::sNewShipto()
{
  if (_shiptoid > 0)
  {
    XSqlQuery shiptoq;
    shiptoq.prepare("SELECT shipto_num"
                    "  FROM shiptoinfo"
                    " WHERE (shipto_id=:id);");
    shiptoq.bindValue(":id", _shiptoid);
    shiptoq.exec();
    if (shiptoq.first())
    {
      _shiptonum = shiptoq.value("shipto_num").toString();
    }
    else if (shiptoq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shiptoq.lastError().text(), __FILE__, __LINE__);
      return;
    }
  }
}

void priceList::sNewItem()
{
  _listPrice->clear();
  _listCost->clear();
  _unitCost->clear();
  if (_item->isValid())
  {
    XSqlQuery itemq;
    itemq.prepare("SELECT item_listprice, item_listcost, item_prodcat_id,"
                  "       itemuomtouomratio(item_id, item_inv_uom_id, item_price_uom_id) AS iteminvpricerat,"
                  "       itemCost(:item_id, :cust_id, :shipto_id, :qty, item_inv_uom_id, item_price_uom_id,"
                  "                :curr_id, :effective, :asof, :warehous_id) AS item_unitcost"
                  "  FROM item LEFT OUTER JOIN itemsite ON (itemsite_item_id=item_id AND itemsite_warehous_id=:warehous_id)"
                  " WHERE (item_id=:item_id);");
    itemq.bindValue(":item_id",          _item->id());
    itemq.bindValue(":cust_id",          _cust->id());
    itemq.bindValue(":shipto_id",        _shiptoid);
    itemq.bindValue(":qty",              _qty->toDouble());
    itemq.bindValue(":curr_id",          _curr_id);
    itemq.bindValue(":effective",        _effective);
    itemq.bindValue(":asof",             _asOf);
    itemq.bindValue(":warehous_id",      _warehouse->id());
    itemq.exec();
    if (itemq.first())
    {
      _listPrice->setDouble(itemq.value("item_listprice").toDouble());
      _listCost->setDouble(itemq.value("item_listcost").toDouble());
      _unitCost->setDouble(itemq.value("item_unitcost").toDouble());
      _iteminvpricerat = itemq.value("iteminvpricerat").toDouble();
      _prodcatid = itemq.value("item_prodcat_id").toInt();
    }
    else if (itemq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemq.lastError().text(), __FILE__, __LINE__);
      return;
    }
  }
}

void priceList::sSelect()
{
  _selectedPrice = _price->rawValue("base_price").toDouble();

  accept();
}

void priceList::sFillList()
{
  bool    ok = false;
  QString errString;
  MetaSQLQuery pricelistm = MQLUtil::mqlLoad("pricelist", "detail",
                                          errString, &ok);
  if (! ok)
  {
    systemError(this, errString, __FILE__, __LINE__);
    return;
  }

  ParameterList pricelistp;
  pricelistp.append("na",               tr("N/A"));
  pricelistp.append("customer",         tr("Customer"));
  pricelistp.append("shipTo",           tr("Cust. Ship-To"));
  pricelistp.append("shipToPattern",    tr("Cust. Ship-To Pattern"));
  pricelistp.append("custType",         tr("Cust. Type"));
  pricelistp.append("custTypePattern",  tr("Cust. Type Pattern"));
  pricelistp.append("sale",             tr("Sale"));
  pricelistp.append("listPrice",        tr("List Price"));
  pricelistp.append("nominal",          tr("Nominal"));
  pricelistp.append("discount",         tr("Discount"));
  pricelistp.append("markup",           tr("Markup"));
  pricelistp.append("item_id",          _item->id());
  pricelistp.append("prodcat_id",       _prodcatid);
  pricelistp.append("cust_id",          _cust->id());
  pricelistp.append("custtype_id",      _custtypeid);
  pricelistp.append("custtype_code",    _custtypecode);
  pricelistp.append("shipto_id",        _shiptoid);
  pricelistp.append("shipto_num",       _shiptonum);
  pricelistp.append("curr_id",          _curr_id);
  pricelistp.append("effective",        _effective);
  pricelistp.append("asof",             _asOf);
  pricelistp.append("item_listprice",   _listPrice->toDouble());
  pricelistp.append("item_listcost",    _listCost->toDouble());
  pricelistp.append("item_unitcost",    (_unitCost->toDouble() / _iteminvpricerat));

  XSqlQuery pricelistq = pricelistm.toQuery(pricelistp);
  _price->populate(pricelistq, true);
}
