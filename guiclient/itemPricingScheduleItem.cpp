/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemPricingScheduleItem.h"
#include "characteristicPrice.h"
#include "xdoublevalidator.h"

#include <metasql.h>
#include "mqlutil.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "errorReporter.h"

itemPricingScheduleItem::itemPricingScheduleItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  XSqlQuery itemitemPricingScheduleItem;
  setupUi(this);

  _save = _buttonBox->button(QDialogButtonBox::Save);
  _save->setEnabled(false);

  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sUpdateCosts(int)));
  connect(_price, SIGNAL(idChanged(int)), _actCost, SLOT(setId(int)));
  connect(_price, SIGNAL(idChanged(int)), _stdCost, SLOT(setId(int)));
  connect(_price, SIGNAL(idChanged(int)), _listPrice, SLOT(setId(int)));
  connect(_price, SIGNAL(effectiveChanged(const QDate&)), _actCost, SLOT(setEffective(const QDate&)));
  connect(_price, SIGNAL(effectiveChanged(const QDate&)), _stdCost, SLOT(setEffective(const QDate&)));
  connect(_price, SIGNAL(effectiveChanged(const QDate&)), _listPrice, SLOT(setEffective(const QDate&)));
  connect(_price, SIGNAL(valueChanged()), this, SLOT(sUpdateMargins()));
  connect(_itemSelected, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged(bool)));
  connect(_discountSelected, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged(bool)));
  connect(_markupSelected, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged(bool)));
  connect(_freightSelected, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged(bool)));
  connect(_qtyUOM, SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
  connect(_priceUOM, SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_dscbyItem, SIGNAL(toggled(bool)), this, SLOT(sCheckEnable()));
  connect(_dscbyprodcat, SIGNAL(toggled(bool)), this, SLOT(sCheckEnable()));
  connect(_markupbyItem, SIGNAL(toggled(bool)), this, SLOT(sCheckEnable()));
  connect(_markupbyprodcat, SIGNAL(toggled(bool)), this, SLOT(sCheckEnable()));
  connect(_item, SIGNAL(valid(bool)), this, SLOT(sCheckEnable()));
  connect(_dscitem, SIGNAL(valid(bool)), this, SLOT(sCheckEnable()));
  connect(_markupitem, SIGNAL(valid(bool)), this, SLOT(sCheckEnable()));
  connect(_prodcat, SIGNAL(valid(bool)), this, SLOT(sCheckEnable()));
  connect(_markupProdcat, SIGNAL(valid(bool)), this, SLOT(sCheckEnable()));

  _ipsheadid = -1;
  _ipsitemid = -1;
  _ipsfreightid = -1;
  _invuomid = -1;
  
  _charprice->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true,  "char_name" );
  _charprice->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true,  "ipsitemchar_value" );
  _charprice->addColumn(tr("Price"),          _priceColumn,Qt::AlignRight, true, "ipsitemchar_price" );

  _qtyBreak->setValidator(omfgThis->qtyVal());
  _qtyBreakCat->setValidator(omfgThis->qtyVal());
  _markupQtyBreakCat->setValidator(omfgThis->qtyVal());
  _qtyBreakFreight->setValidator(omfgThis->weightVal());
  _discount->setValidator(new XDoubleValidator(-999, 999, decimalPlaces("percent"), this));
  _fixedAmtDiscount->setValidator(omfgThis->negMoneyVal());
  _markup->setValidator(new XDoubleValidator(-999, 999, decimalPlaces("percent"), this));
  _fixedAmtMarkup->setValidator(omfgThis->negMoneyVal());
  _pricingRatio->setPrecision(omfgThis->percentVal());
  _stdMargin->setPrecision(omfgThis->percentVal());
  _actMargin->setPrecision(omfgThis->percentVal());
  _item->setType(ItemLineEdit::cSold);
  _zoneFreight->setType(XComboBox::ShippingZones);
  _shipViaFreight->setType(XComboBox::ShipVias);
  _freightClass->setType(XComboBox::FreightClasses);
  
  _tab->setTabEnabled(_tab->indexOf(_configuredPrices),FALSE);
 
  itemitemPricingScheduleItem.exec("SELECT uom_name FROM uom WHERE (uom_item_weight);");
  if (itemitemPricingScheduleItem.first())
  {
    QString uom = itemitemPricingScheduleItem.value("uom_name").toString();
    QString title (tr("Price per "));
    title += uom;
    _perUOMFreight->setText(title);
    _qtyBreakFreightUOM->setText(uom);
  }

  if (_metrics->boolean("WholesalePriceCosting"))
      _markupLit->setText(tr("Markup Percent (Wholesale Price):"));
  else
      _markupLit->setText(tr("Markup Percent (Inventory Cost):"));

  _rejectedMsg = tr("The application has encountered an error and must "
                    "stop editing this Pricing Schedule.\n%1");
}

itemPricingScheduleItem::~itemPricingScheduleItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemPricingScheduleItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemPricingScheduleItem::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XSqlQuery prodcatet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("ipshead_id", &valid);
  if (valid)
    _ipsheadid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _price->setId(param.toInt());
    _priceFreight->setId(param.toInt());
  }

  param = pParams.value("updated", &valid);
  if (valid)
  {
    _price->setEffective(param.toDate());
    _priceFreight->setEffective(param.toDate());
  }

  param = pParams.value("ipsitem_id", &valid);
  if (valid)
  {
    _ipsitemid = param.toInt();
    populate();
  }

  param = pParams.value("ipsfreight_id", &valid);
  if (valid)
  {
    _ipsfreightid = param.toInt();
    _freightSelected->setChecked(true);
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _item->setReadOnly(TRUE);
      _prodcat->setEnabled(FALSE);
      _markupProdcat->setEnabled(FALSE);
      _typeGroup->setEnabled(FALSE);
      _dscitem->setReadOnly(TRUE);
      _markupitem->setReadOnly(TRUE);
      _discountBy->setEnabled(FALSE);
      _markupBy->setEnabled(FALSE);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _prodcat->setEnabled(FALSE);
      _qtyBreak->setEnabled(FALSE);
      _qtyBreakCat->setEnabled(FALSE);
      _qtyBreakFreight->setEnabled(FALSE);
      _price->setEnabled(FALSE);
      _discount->setEnabled(FALSE);
      _fixedAmtDiscount->setEnabled(FALSE);
      _markup->setEnabled(FALSE);
      _fixedAmtMarkup->setEnabled(FALSE);
      _priceFreight->setEnabled(FALSE);
      _typeGroup->setEnabled(FALSE);
      _typeFreightGroup->setEnabled(FALSE);
      _siteFreight->setEnabled(FALSE);
      _zoneFreightGroup->setEnabled(FALSE);
      _shipViaFreightGroup->setEnabled(FALSE);
      _freightClassGroup->setEnabled(FALSE);
      _dscitem->setReadOnly(TRUE);
      _discountBy->setEnabled(FALSE);
      _markupBy->setEnabled(FALSE);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void itemPricingScheduleItem::sCheckEnable()
{
  if(_itemSelected->isChecked())
  {
    if (_item->isValid())
      _save->setEnabled(true);
    else
      _save->setEnabled(false);
  }
  if (_dscbyItem->isChecked())
  {
    _prodcatLit->hide();
    _prodcat->hide();
    _itemGroup->show();
    if (_dscitem->isValid())
      _save->setEnabled(true);
    else
      _save->setEnabled(false);
  }
  if (_dscbyprodcat->isChecked())
  {
    _itemGroup->hide();
    _prodcatLit->show();
    _prodcat->show();
    if (_prodcat->isValid())
      _save->setEnabled(true);
    else
      _save->setEnabled(false);
  }
  if (_markupbyItem->isChecked())
  {
    _markupProdcatLit->hide();
    _markupProdcat->hide();
    _markupItemGroup->show();
    if (_markupitem->isValid())
      _save->setEnabled(true);
    else
      _save->setEnabled(false);
  }
  if (_markupbyprodcat->isChecked())
  {
    _markupItemGroup->hide();
    _markupProdcatLit->show();
    _markupProdcat->show();
    if (_markupProdcat->isValid())
      _save->setEnabled(true);
    else
      _save->setEnabled(false);
  }
}

void itemPricingScheduleItem::sSave()
{
  sSave(TRUE);
}

void itemPricingScheduleItem::sSave( bool pClose)
{
  XSqlQuery itemSave;
  if(_itemSelected->isChecked())
  {
    itemSave.prepare( "SELECT ipsitem_id "
               "  FROM ipsiteminfo "
               " WHERE((ipsitem_ipshead_id=:ipshead_id)"
               "   AND (ipsitem_item_id=:item_id)"
               "   AND (ipsitem_qty_uom_id=:uom_id)"
               "   AND (ipsitem_qtybreak=:qtybreak)"
               "   AND (ipsitem_id <> :ipsitem_id));" );
    itemSave.bindValue(":ipshead_id", _ipsheadid);
    itemSave.bindValue(":item_id", _item->id());
    itemSave.bindValue(":qtybreak", _qtyBreak->toDouble());
    itemSave.bindValue(":uom_id", _qtyUOM->id());
    itemSave.bindValue(":ipsitem_id", _ipsitemid);
    itemSave.exec();
    if (itemSave.first())
    {
      QMessageBox::critical(this, tr("Cannot Create Pricing Schedule Item"),
                            tr("<p>There is an existing Pricing Schedule Item"
                              " for the selected Pricing Schedule, Item, and "
                              "Quantity Break.<br/>You may not create "
                              "duplicate Pricing Schedule Items.</p>"));
      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Checking Duplicates"),
                                  itemSave, __FILE__, __LINE__))
    {
      done(-1);
    }
  }
  else if(_discountSelected->isChecked())
  {
    if(_dscbyItem->isChecked())
    {
      itemSave.prepare( "SELECT 1 "
                 "FROM ipsiteminfo JOIN ipshead ON (ipsitem_ipshead_id=ipshead_id) "
                 "WHERE ((ipshead_id = :ipshead_id)"
                 "   AND (ipsitem_item_id = :item_id)"
                 "   AND (ipsitem_qtybreak = :qtybreak)"
                 "   AND (ipsitem_id <> :ipsitem_id));" );

      itemSave.bindValue(":ipshead_id", _ipsheadid);
      itemSave.bindValue(":item_id", _dscitem->id());
      itemSave.bindValue(":qtybreak", _qtyBreakCat->toDouble());
      itemSave.bindValue(":ipsitem_id", _ipsitemid);
      itemSave.exec();
      if (itemSave.first())
      {
        QMessageBox::critical(this, tr("Cannot Create Pricing Schedule Item"),
                              tr("<p>There is an existing Pricing Schedule Item"
                                " for the selected Pricing Schedule, Item, and "
                                "Quantity Break.<br/>You may not create "
                                "duplicate Pricing Schedule Items.</p>"));
        return;
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Checking Duplicates"),
                                    itemSave, __FILE__, __LINE__))
      {
        done(-1);
      }
    }
    else if(_dscbyprodcat->isChecked())
    {
      itemSave.prepare( "SELECT ipsitem_id "
                 "FROM ipsiteminfo "
                 "WHERE ( (ipsitem_ipshead_id=:ipshead_id)"
                 " AND (ipsitem_prodcat_id=:prodcat_id)"
                 " AND (ipsitem_qtybreak=:qtybreak)"
                 " AND (ipsitem_id <> :ipsitem_id) );" );
      itemSave.bindValue(":ipshead_id", _ipsheadid);
      itemSave.bindValue(":prodcat_id", _prodcat->id());
      itemSave.bindValue(":qtybreak", _qtyBreakCat->toDouble());
      itemSave.bindValue(":ipsitem_id", _ipsitemid);
      itemSave.exec();
      if (itemSave.first())
      {
        QMessageBox::critical( this, tr("Cannot Create Pricing Schedule Item"),
                               tr( "There is an existing Pricing Schedule Item for the selected Pricing Schedule, Product Category and Quantity Break defined.\n"
                                   "You may not create duplicate Pricing Schedule Items." ) );
        return;
      }
      else if (itemSave.lastError().type() != QSqlError::NoError)
      {
        systemError(this, _rejectedMsg.arg(itemSave.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }
    }
  }
  else if(_markupSelected->isChecked())
  {
    if(_markupbyItem->isChecked())
    {
      itemSave.prepare( "SELECT 1 "
                 "FROM ipsiteminfo JOIN ipshead ON (ipsitem_ipshead_id=ipshead_id) "
                 "WHERE ((ipshead_id = :ipshead_id)"
                 "   AND (ipsitem_item_id = :item_id)"
                 "   AND (ipsitem_qtybreak = :qtybreak)"
                 "   AND (ipsitem_id <> :ipsitem_id));" );

      itemSave.bindValue(":ipshead_id", _ipsheadid);
      itemSave.bindValue(":item_id", _markupitem->id());
      itemSave.bindValue(":qtybreak", _markupQtyBreakCat->toDouble());
      itemSave.bindValue(":ipsitem_id", _ipsitemid);
      itemSave.exec();
      if (itemSave.first())
      {
        QMessageBox::critical( this, tr("Cannot Create Pricing Schedule Item"),
                               tr( "There is an existing Pricing Schedule Item for the selected Pricing Schedule, Item and Quantity Break defined.\n"
                                   "You may not create duplicate Pricing Schedule Items." ) );
        return;
      }
      else if (itemSave.lastError().type() != QSqlError::NoError)
      {
        systemError(this, _rejectedMsg.arg(itemSave.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }
    }
    else if(_markupbyprodcat->isChecked())
    {
      itemSave.prepare( "SELECT ipsitem_id "
                 "FROM ipsiteminfo "
                 "WHERE ( (ipsitem_ipshead_id=:ipshead_id)"
                 " AND (ipsitem_prodcat_id=:prodcat_id)"
                 " AND (ipsitem_qtybreak=:qtybreak)"
                 " AND (ipsitem_id <> :ipsitem_id) );" );
      itemSave.bindValue(":ipshead_id", _ipsheadid);
      itemSave.bindValue(":prodcat_id", _markupProdcat->id());
      itemSave.bindValue(":qtybreak", _markupQtyBreakCat->toDouble());
      itemSave.bindValue(":ipsitem_id", _ipsitemid);
      itemSave.exec();
      if (itemSave.first())
      {
        QMessageBox::critical( this, tr("Cannot Create Pricing Schedule Item"),
                               tr( "There is an existing Pricing Schedule Item for the selected Pricing Schedule, Product Category and Quantity Break defined.\n"
                                   "You may not create duplicate Pricing Schedule Items." ) );
        return;
      }
      else if (itemSave.lastError().type() != QSqlError::NoError)
      {
        systemError(this, _rejectedMsg.arg(itemSave.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }
    }
  }
  else if(_freightSelected->isChecked())
  {
    MetaSQLQuery mql = mqlLoad("pricingFreight", "detail");

    ParameterList params;
    params.append("checkDup", true);
    params.append("ipshead_id", _ipsheadid);
    if (_siteFreight->isSelected())
      params.append("warehous_id", _siteFreight->id());
    if (_selectedZoneFreight->isChecked())
      params.append("shipzone_id", _zoneFreight->id());
    if (_selectedFreightClass->isChecked())
      params.append("freightclass_id", _freightClass->id());
    if (_selectedShipViaFreight->isChecked())
      params.append("shipvia", _shipViaFreight->currentText());
    params.append("qtybreak", _qtyBreakFreight->toDouble());
    params.append("ipsfreight_id", _ipsfreightid);
    itemSave = mql.toQuery(params);
    if (itemSave.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Pricing Schedule Item"),
                             tr( "There is an existing Pricing Schedule Item for the selected Pricing Schedule, Freight Criteria and Quantity Break defined.\n"
                                 "You may not create duplicate Pricing Schedule Items." ) );
      return;
    }
    else if (itemSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(itemSave.lastError().databaseText()),
                __FILE__, __LINE__);
      done(-1);
    }
  }

  if (_mode == cNew)
  {
    if(_itemSelected->isChecked() || _discountSelected->isChecked() || _markupSelected->isChecked())
    {
      itemSave.exec("SELECT NEXTVAL('ipsitem_ipsitem_id_seq') AS ipsitem_id;");
      if (itemSave.first())
        _ipsitemid = itemSave.value("ipsitem_id").toInt();
      else if (itemSave.lastError().type() != QSqlError::NoError)
      {
        systemError(this, _rejectedMsg.arg(itemSave.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }

      itemSave.prepare( "INSERT INTO ipsiteminfo "
                 "( ipsitem_id, ipsitem_ipshead_id, ipsitem_item_id, ipsitem_prodcat_id,"
                 "  ipsitem_qty_uom_id, ipsitem_qtybreak, "
                 "  ipsitem_price_uom_id, ipsitem_price, "
                 "  ipsitem_discntprcnt, ipsitem_fixedamtdiscount, "
                 "  ipsitem_type, ipsitem_warehous_id ) "
                 "VALUES "
                 "( :ipsitem_id, :ipshead_id, :ipsitem_item_id, :ipsitem_prodcat_id, "
                 "  :ipsitem_qty_uom_id, :ipsitem_qtybreak, "
                 "  :ipsitem_price_uom_id, :ipsitem_price, "
                 "  :ipsitem_discntprcnt, :ipsitem_fixedamtdiscount, "
                 "  :ipsitem_type, :ipsitem_warehous_id);" );
    }
    else if(_freightSelected->isChecked())
    {
      itemSave.exec("SELECT NEXTVAL('ipsfreight_ipsfreight_id_seq') AS ipsfreight_id;");
      if (itemSave.first())
        _ipsfreightid = itemSave.value("ipsfreight_id").toInt();
      else if (itemSave.lastError().type() != QSqlError::NoError)
      {
        systemError(this, _rejectedMsg.arg(itemSave.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }

      itemSave.prepare( "INSERT INTO ipsfreight "
                 "( ipsfreight_id, ipsfreight_ipshead_id, ipsfreight_qtybreak, ipsfreight_price,"
                 "  ipsfreight_type, ipsfreight_warehous_id, ipsfreight_shipzone_id,"
                 "  ipsfreight_freightclass_id, ipsfreight_shipvia ) "
                 "VALUES "
                 "( :ipsfreight_id, :ipshead_id, :ipsfreight_qtybreak, :ipsfreight_price,"
                 "  :ipsfreight_type, :ipsfreight_warehous_id, :ipsfreight_shipzone_id,"
                 "  :ipsfreight_freightclass_id, :ipsfreight_shipvia ) " );
    }
  }
  else if (_mode == cEdit)
  {
    if(_itemSelected->isChecked() || _discountSelected->isChecked() || _markupSelected->isChecked())
      itemSave.prepare( "UPDATE ipsiteminfo "
                 "   SET ipsitem_qty_uom_id=:ipsitem_qty_uom_id,"
                 "       ipsitem_qtybreak=:ipsitem_qtybreak,"
                 "       ipsitem_price_uom_id=:ipsitem_price_uom_id,"
                 "       ipsitem_price=:ipsitem_price,"
                 "       ipsitem_discntprcnt=:ipsitem_discntprcnt,"
                 "       ipsitem_fixedamtdiscount=:ipsitem_fixedamtdiscount "
                 "WHERE (ipsitem_id=:ipsitem_id);" );
    else if(_freightSelected->isChecked())
      itemSave.prepare( "UPDATE ipsfreight "
                 "   SET ipsfreight_qtybreak=:ipsfreight_qtybreak,"
                 "       ipsfreight_type=:ipsfreight_type,"
                 "       ipsfreight_price=:ipsfreight_price,"
                 "       ipsfreight_warehous_id=:ipsfreight_warehous_id,"
                 "       ipsfreight_shipzone_id=:ipsfreight_shipzone_id,"
                 "       ipsfreight_freightclass_id=:ipsfreight_freightclass_id,"
                 "       ipsfreight_shipvia=:ipsfreight_shipvia "
                 "WHERE (ipsfreight_id=:ipsfreight_id);" );
  }

  itemSave.bindValue(":ipsitem_id", _ipsitemid);
  itemSave.bindValue(":ipsfreight_id", _ipsfreightid);
  itemSave.bindValue(":ipshead_id", _ipsheadid);

  if(_itemSelected->isChecked())
  {
    itemSave.bindValue(":ipsitem_item_id", _item->id());
    itemSave.bindValue(":ipsitem_type", "N");
    itemSave.bindValue(":ipsitem_qty_uom_id", _qtyUOM->id());
    itemSave.bindValue(":ipsitem_price_uom_id", _priceUOM->id());
    itemSave.bindValue(":ipsitem_qtybreak", _qtyBreak->toDouble());
    itemSave.bindValue(":ipsitem_discntprcnt", 0.00);
    itemSave.bindValue(":ipsitem_fixedamtdiscount", 0.00);
    itemSave.bindValue(":ipsitem_price", _price->localValue());
  }
  else if((_discountSelected->isChecked()) && (_dscbyItem->isChecked()))
  {
    itemSave.bindValue(":ipsitem_item_id", _dscitem->id());
    itemSave.bindValue(":ipsitem_type", "D");
    itemSave.bindValue(":ipsitem_qtybreak", _qtyBreakCat->toDouble());
    itemSave.bindValue(":ipsitem_discntprcnt", (_discount->toDouble() / 100.0));
    itemSave.bindValue(":ipsitem_fixedamtdiscount", (_fixedAmtDiscount->toDouble()));
    itemSave.bindValue(":ipsitem_price", 0.00);
  }
  else if((_discountSelected->isChecked()) && (_dscbyprodcat->isChecked()))
  {
    itemSave.bindValue(":ipsitem_prodcat_id", _prodcat->id());
    itemSave.bindValue(":ipsitem_type", "D");
    itemSave.bindValue(":ipsitem_qtybreak", _qtyBreakCat->toDouble());
    itemSave.bindValue(":ipsitem_discntprcnt", (_discount->toDouble() / 100.0));
    itemSave.bindValue(":ipsitem_fixedamtdiscount", (_fixedAmtDiscount->toDouble()));
    itemSave.bindValue(":ipsitem_price", 0.00);
  }
  else if((_markupSelected->isChecked()) && (_markupbyItem->isChecked()))
  {
    itemSave.bindValue(":ipsitem_item_id", _markupitem->id());
    itemSave.bindValue(":ipsitem_type", "M");
    itemSave.bindValue(":ipsitem_qtybreak", _markupQtyBreakCat->toDouble());
    itemSave.bindValue(":ipsitem_discntprcnt", (_markup->toDouble() / 100.0));
    itemSave.bindValue(":ipsitem_fixedamtdiscount", (_fixedAmtMarkup->toDouble()));
    itemSave.bindValue(":ipsitem_price", 0.00);
  }
  else if((_markupSelected->isChecked()) && (_markupbyprodcat->isChecked()))
  {
    itemSave.bindValue(":ipsitem_prodcat_id", _markupProdcat->id());
    itemSave.bindValue(":ipsitem_type", "M");
    itemSave.bindValue(":ipsitem_qtybreak", _markupQtyBreakCat->toDouble());
    itemSave.bindValue(":ipsitem_discntprcnt", (_markup->toDouble() / 100.0));
    itemSave.bindValue(":ipsitem_fixedamtdiscount", (_fixedAmtMarkup->toDouble()));
    itemSave.bindValue(":ipsitem_price", 0.00);
  }

  XSqlQuery qry;
  if((_discountSelected->isChecked()) && (_dscbyItem->isChecked()))
  {
    qry.prepare("SELECT item_inv_uom_id, item_price_uom_id "
                "FROM item "
                "WHERE (item_id=:item_id);");
    qry.bindValue(":item_id", _dscitem->id());
    qry.exec();
    if (qry.first())
    {
      itemSave.bindValue(":ipsitem_qty_uom_id", qry.value("item_inv_uom_id"));
      itemSave.bindValue(":ipsitem_price_uom_id", qry.value("item_price_uom_id"));
    }
    else if (qry.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(qry.lastError().databaseText()),
                __FILE__, __LINE__);
      done(-1);
    }
  }

  if((_markupSelected->isChecked()) && (_markupbyItem->isChecked()))
  {
    qry.prepare("SELECT item_inv_uom_id, item_price_uom_id "
                "FROM item "
                "WHERE (item_id=:item_id);");
    qry.bindValue(":item_id", _markupitem->id());
    qry.exec();
    if (qry.first())
    {
      itemSave.bindValue(":ipsitem_qty_uom_id", qry.value("item_inv_uom_id"));
      itemSave.bindValue(":ipsitem_price_uom_id", qry.value("item_price_uom_id"));
    }
    else if (qry.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(qry.lastError().databaseText()),
                __FILE__, __LINE__);
      done(-1);
    }
  }

  itemSave.bindValue(":ipsfreight_qtybreak", _qtyBreakFreight->toDouble());
  itemSave.bindValue(":ipsfreight_price", _priceFreight->localValue());
  itemSave.bindValue(":qty_uom_id", _qtyUOM->id());
  itemSave.bindValue(":price_uom_id", _priceUOM->id());

  if (_flatRateFreight->isChecked())
    itemSave.bindValue(":ipsfreight_type", "F");
  else
    itemSave.bindValue(":ipsfreight_type", "P");
  if (_siteFreight->isSelected())
    itemSave.bindValue(":ipsfreight_warehous_id", _siteFreight->id());
  if (_selectedZoneFreight->isChecked())
    itemSave.bindValue(":ipsfreight_shipzone_id", _zoneFreight->id());
  if (_selectedFreightClass->isChecked())
    itemSave.bindValue(":ipsfreight_freightclass_id", _freightClass->id());
  if (_selectedShipViaFreight->isChecked())
    itemSave.bindValue(":ipsfreight_shipvia", _shipViaFreight->currentText());
  itemSave.exec();
  if (itemSave.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(itemSave.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }

  if (pClose)
  {
    if(_itemSelected->isChecked())
      done(_ipsitemid);
    if(_discountSelected->isChecked())
      done(_ipsitemid);
    if(_markupSelected->isChecked())
      done(_ipsitemid);
    if(_freightSelected->isChecked())
      done(_ipsfreightid);
  }
  else
  {
    _mode = cEdit;

    _item->setReadOnly(TRUE);
    _prodcat->setEnabled(FALSE);
    _typeGroup->setEnabled(FALSE);
  }

}

void itemPricingScheduleItem::populate()
{
  XSqlQuery itempopulate;
  if(_freightSelected->isChecked())
  {
    MetaSQLQuery mql = mqlLoad("pricingFreight", "detail");

    ParameterList params;
    params.append("ipsfreight_id", _ipsfreightid);
    itempopulate = mql.toQuery(params);
    if (itempopulate.first())
    {
      _ipsheadid=itempopulate.value("ipsfreight_ipshead_id").toInt();
      _qtyBreakFreight->setDouble(itempopulate.value("ipsfreight_qtybreak").toDouble());
      _priceFreight->setLocalValue(itempopulate.value("ipsfreight_price").toDouble());
      if (itempopulate.value("ipsfreight_type").toString() == "F")
      {
        _flatRateFreight->setChecked(true);
        _qtyBreakFreight->setEnabled(false);
      }
      else
        _perUOMFreight->setChecked(true);
      if (itempopulate.value("ipsfreight_warehous_id").toInt() > 0)
        _siteFreight->setId(itempopulate.value("ipsfreight_warehous_id").toInt());
      else
        _siteFreight->setAll();
      if (itempopulate.value("ipsfreight_shipzone_id").toInt() > 0)
      {
        _selectedZoneFreight->setChecked(true);
        _zoneFreight->setId(itempopulate.value("ipsfreight_shipzone_id").toInt());
      }
      else
        _allZonesFreight->setChecked(true);
      if (itempopulate.value("ipsfreight_freightclass_id").toInt() > 0)
      {
        _selectedFreightClass->setChecked(true);
        _freightClass->setId(itempopulate.value("ipsfreight_freightclass_id").toInt());
      }
      else
        _allFreightClasses->setChecked(true);
      //  Handle the free-form Ship Via
      _shipViaFreight->setCurrentIndex(-1);
      QString shipvia = itempopulate.value("ipsfreight_shipvia").toString();
      if (shipvia.trimmed().length() != 0)
      {
        _selectedShipViaFreight->setChecked(true);
        for (int counter = 0; counter < _shipViaFreight->count(); counter++)
          if (_shipViaFreight->itemText(counter) == shipvia)
            _shipViaFreight->setCurrentIndex(counter);

        if (_shipViaFreight->id() == -1)
        {
          _shipViaFreight->addItem(shipvia);
          _shipViaFreight->setCurrentIndex(_shipViaFreight->count() - 1);
        }
      }
      else
        _allShipViasFreight->setChecked(true);
    }
    else if (itempopulate.lastError().type() != QSqlError::NoError)
    {
       systemError(this, _rejectedMsg.arg(itempopulate.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
    }
  }
  else
  {
    itempopulate.prepare( "SELECT * "
                          "FROM ipsiteminfo "
                          "WHERE (ipsitem_id=:ipsitem_id);" );
    itempopulate.bindValue(":ipsitem_id", _ipsitemid);
    itempopulate.exec();
    if (itempopulate.first())
    {
      if(itempopulate.value("ipsitem_type").toString() == "D")
        _discountSelected->setChecked(true);
      else if(itempopulate.value("ipsitem_type").toString() == "M")
        _markupSelected->setChecked(true);
      else
        _itemSelected->setChecked(true);

      if (itempopulate.value("ipsitem_item_id").toInt() > 0)
      {
        if(itempopulate.value("ipsitem_type").toString() == "D")
          _dscbyItem->setChecked(true);
        else if(itempopulate.value("ipsitem_type").toString() == "M")
          _markupbyItem->setChecked(true);
      }
      else
      {
        if(itempopulate.value("ipsitem_type").toString() == "D")
          _dscbyprodcat->setChecked(true);
        else if(itempopulate.value("ipsitem_type").toString() == "M")
          _markupbyprodcat->setChecked(true);
      }
      _ipsheadid=itempopulate.value("ipsitem_ipshead_id").toInt();

      // Nominal
      _item->setId(itempopulate.value("ipsitem_item_id").toInt());
      _qtyBreak->setDouble(itempopulate.value("ipsitem_qtybreak").toDouble());
      _price->setLocalValue(itempopulate.value("ipsitem_price").toDouble());
      _qtyUOM->setId(itempopulate.value("ipsitem_qty_uom_id").toInt());
      _priceUOM->setId(itempopulate.value("ipsitem_price_uom_id").toInt());

      // Discount
      _dscitem->setId(itempopulate.value("ipsitem_item_id").toInt());
      _prodcat->setId(itempopulate.value("ipsitem_prodcat_id").toInt());
      _qtyBreakCat->setDouble(itempopulate.value("ipsitem_qtybreak").toDouble());
      _discount->setDouble(itempopulate.value("ipsitem_discntprcnt").toDouble() * 100.0);
      _fixedAmtDiscount->setDouble(itempopulate.value("ipsitem_fixedamtdiscount").toDouble());

      // Markup
      _markupitem->setId(itempopulate.value("ipsitem_item_id").toInt());
      _markupProdcat->setId(itempopulate.value("ipsitem_prodcat_id").toInt());
      _markupQtyBreakCat->setDouble(itempopulate.value("ipsitem_qtybreak").toDouble());
      _markup->setDouble(itempopulate.value("ipsitem_discntprcnt").toDouble() * 100.0);
      _fixedAmtMarkup->setDouble(itempopulate.value("ipsitem_fixedamtdiscount").toDouble());

      sUpdateMargins();
    }
    else if (itempopulate.lastError().type() != QSqlError::NoError)
    {
	systemError(this, _rejectedMsg.arg(itempopulate.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
    }
  }
  sFillList();
}

void itemPricingScheduleItem::sUpdateCosts(int pItemid)
{
  XSqlQuery itemUpdateCosts;
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
  if (itemUpdateCosts.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(itemUpdateCosts.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }
  _qtyUOM->populate(uom);
  _priceUOM->populate(uom);

  XSqlQuery cost;
  cost.prepare( "SELECT item_inv_uom_id, item_price_uom_id,"
                "       iteminvpricerat(item_id) AS ratio,"
                "       item_listprice, "
                "       (stdcost(item_id) * iteminvpricerat(item_id)) AS standard,"
                "       (actcost(item_id, :curr_id) * iteminvpricerat(item_id)) AS actual "
                "  FROM item"
                " WHERE (item_id=:item_id);" );
  cost.bindValue(":item_id", pItemid);
  cost.bindValue(":curr_id", _actCost->id());
  cost.exec();
  if (cost.first())
  {
    _invuomid = cost.value("item_inv_uom_id").toInt();
    _listPrice->setBaseValue(cost.value("item_listprice").toDouble());
    _pricingRatio->setDouble(cost.value("ratio").toDouble());

    _stdCost->setBaseValue(cost.value("standard").toDouble());
    _actCost->setLocalValue(cost.value("actual").toDouble());

    _qtyUOM->setId(cost.value("item_inv_uom_id").toInt());
    _priceUOM->setId(cost.value("item_price_uom_id").toInt());
  }
  else if (itemUpdateCosts.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(itemUpdateCosts.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }
  
  if (_item->isConfigured())
    _tab->setTabEnabled(_tab->indexOf(_configuredPrices),TRUE);
  else
    _tab->setTabEnabled(_tab->indexOf(_configuredPrices),FALSE);
}

void itemPricingScheduleItem::sUpdateMargins()
{
  if (_item->isValid())
  {
    double price = _price->baseValue();
    QString stylesheet;

    if (_stdCost->baseValue() > 0.0)
    {
      _stdMargin->setDouble((price - _stdCost->baseValue()) / price * 100);

      if (_stdCost->baseValue() > price)
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
    }
    else
      _stdMargin->setText("N/A");

    _stdMargin->setStyleSheet(stylesheet);

    if (_actCost->baseValue() > 0.0)
    {
      _actMargin->setDouble((price - _actCost->baseValue()) / price * 100);

      if (_actCost->baseValue() > price)
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
    }
    else
      _actMargin->setText("N/A");

    _actMargin->setStyleSheet(stylesheet);
  }
}

void itemPricingScheduleItem::sTypeChanged(bool pChecked)
{
  if(!pChecked)
    return;
  
  if(_itemSelected->isChecked())
  {
    _widgetStack->setCurrentWidget(_nominalPage);
    _save->setEnabled(_item->isValid());
    _dscbyItem->setChecked(false);
    _dscbyprodcat->setChecked(false);
    _markupbyItem->setChecked(false);
    _markupbyprodcat->setChecked(false);
  }
  else if(_discountSelected->isChecked())
  {
    _widgetStack->setCurrentWidget(_discountPage);
    _dscbyItem->setChecked(true);
    _dscbyprodcat->setChecked(false);
    _markupbyItem->setChecked(false);
    _markupbyprodcat->setChecked(false);
    _save->setEnabled(false);
    _save->setEnabled(_dscitem->isValid());
  }
  else if(_markupSelected->isChecked())
  {
    _widgetStack->setCurrentWidget(_markupPage);
    _dscbyItem->setChecked(false);
    _dscbyprodcat->setChecked(false);
    _markupbyItem->setChecked(true);
    _markupbyprodcat->setChecked(false);
    _save->setEnabled(false);
    _save->setEnabled(_markupitem->isValid());
  }
  else if(_freightSelected->isChecked())
  {
    _widgetStack->setCurrentWidget(_freightPage);
    _save->setEnabled(true);
    _dscbyItem->setChecked(false);
    _dscbyprodcat->setChecked(false);
  }
}

void itemPricingScheduleItem::sQtyUOMChanged()
{
  if(_qtyUOM->id() != _invuomid)
  {
    _priceUOM->setId(_qtyUOM->id());
    _priceUOM->setEnabled(false);
  }
  else
    _priceUOM->setEnabled(true);
  sPriceUOMChanged();
}

void itemPricingScheduleItem::sPriceUOMChanged()
{
  if(_priceUOM->id() == -1 || _qtyUOM->id() == -1)
    return;

  XSqlQuery cost;
  cost.prepare( "SELECT "
                "       itemuomtouomratio(item_id, :qtyuomid, :priceuomid) AS ratio,"
                "       ((item_listprice / iteminvpricerat(item_id)) * itemuomtouomratio(item_id, :priceuomid, item_inv_uom_id)) AS listprice, "
                "       (stdcost(item_id) * itemuomtouomratio(item_id, :priceuomid, item_inv_uom_id)) AS standard,"
                "       (actcost(item_id, :curr_id) * itemuomtouomratio(item_id, :priceuomid, item_inv_uom_id)) AS actual "
                "  FROM item"
                " WHERE (item_id=:item_id);" );
  cost.bindValue(":item_id", _item->id());
  cost.bindValue(":curr_id", _actCost->id());
  cost.bindValue(":qtyuomid", _qtyUOM->id());
  cost.bindValue(":priceuomid", _priceUOM->id());
  cost.exec();
  if (cost.first())
  {
    _listPrice->setBaseValue(cost.value("listprice").toDouble());
    _pricingRatio->setDouble(cost.value("ratio").toDouble());

    _stdCost->setBaseValue(cost.value("standard").toDouble());
    _actCost->setLocalValue(cost.value("actual").toDouble());

    sUpdateMargins();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Checking Costs"),
                                cost, __FILE__, __LINE__))
  {
    done(-1);
  }
}

void itemPricingScheduleItem::sNew()
{
  if (_mode == cNew)
    sSave(FALSE);
  ParameterList params;
  params.append("mode", "new");
  params.append("ipsitem_id", _ipsitemid);
  params.append("curr_id", _price->id());
  params.append("item_id", _item->id());

  characteristicPrice newdlg(this, "", TRUE);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
  {
    if (result == -1)
      done(-1);
     else
      sFillList();
  }
}

void itemPricingScheduleItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ipsitem_id", _ipsitemid);
  params.append("ipsitemchar_id", _charprice->id());
  params.append("curr_id", _price->id());
  params.append("item_id", _item->id());

  characteristicPrice newdlg(this, "", TRUE);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
  {
    if (result == -1)
      done(-1);
     else
      sFillList();
  }
}

void itemPricingScheduleItem::sDelete()
{
  XSqlQuery itemDelete;
  itemDelete.prepare("DELETE FROM ipsitemchar "
            "WHERE (ipsitemchar_id=:ipsitemchar_id);");
  itemDelete.bindValue(":ipsitemchar_id", _charprice->id());
  itemDelete.exec();
  if (itemDelete.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(itemDelete.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }
  sFillList();
}

void itemPricingScheduleItem::sFillList()
{
  XSqlQuery itemFillList;
  itemFillList.prepare("SELECT ipsitemchar_id, char_name, ipsitemchar_value, ipsitemchar_price, "
            "  'salesprice' AS ipsitemchar_price_xtnumericrole "
            "FROM ipsitemchar, char "
            "WHERE ((ipsitemchar_char_id=char_id) "
            "AND (ipsitemchar_ipsitem_id=:ipsitem_id)); ");
  itemFillList.bindValue(":ipsitem_id", _ipsitemid);
  itemFillList.exec();
  if (itemFillList.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(itemFillList.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }
  _charprice->populate(itemFillList);
}

