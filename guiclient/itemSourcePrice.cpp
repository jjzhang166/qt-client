/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSourcePrice.h"
#include "xdoublevalidator.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

#include <QMessageBox>
#include <QValidator>
#include <QVariant>

itemSourcePrice::itemSourcePrice(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_itemSelected, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged(bool)));
  connect(_discountSelected, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged(bool)));

  _qtyBreak->setValidator(omfgThis->qtyVal());
  _discount->setValidator(new XDoubleValidator(-999, 999, decimalPlaces("percent"), this));
  _fixedAmtDiscount->setValidator(omfgThis->negMoneyVal());

  _site->setAll();
  if (!_metrics->boolean("MultiWhs"))
  {
    _site->hide();
    _dropship->hide();
  }

  _discount->hide();
  _fixedAmtDiscount->hide();
  _listCost->hide();
  _discountLit->hide();
  _fixedAmtDiscountLit->hide();
  _listCostLit->hide();

  _itemsrcpid = -1;
  _itemsrcid = -1;
}

itemSourcePrice::~itemSourcePrice()
{
    // no need to delete child widgets, Qt does it all for us
}

void itemSourcePrice::languageChange()
{
    retranslateUi(this);
}

enum SetResponse itemSourcePrice::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsrc_id", &valid);
  if (valid)
    _itemsrcid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
    _price->setId(param.toInt());

  param = pParams.value("curr_effective", &valid);
  if (valid)
    _price->setEffective(param.toDate());

  param = pParams.value("itemsrcp_id", &valid);
  if (valid)
  {
    _itemsrcpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      itemet.exec("SELECT NEXTVAL('itemsrcp_itemsrcp_id_seq') AS _itemsrcp_id;");
      if (itemet.first())
        _itemsrcpid = itemet.value("_itemsrcp_id").toInt();
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Source Price Information"),
                                    itemet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _qtyBreak->setEnabled(false);
      _price->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void itemSourcePrice::sSave()
{
  QList<GuiErrorCheck> errors;
  XSqlQuery itemSave;
  itemSave.prepare("SELECT itemsrcp_id"
                   "  FROM itemsrcp"
                   " WHERE ((itemsrcp_id != :itemsrcp_id)"
                   "   AND  (itemsrcp_itemsrc_id=:itemsrcp_itemsrc_id)"
                   "   AND  (itemsrcp_warehous_id=:itemsrcp_warehous_id)"
                   "   AND  (itemsrcp_dropship=:itemsrcp_dropship)"
                   "   AND  (itemsrcp_qtybreak=:itemsrcp_qtybreak));");
  itemSave.bindValue(":itemsrcp_id", _itemsrcpid);
  itemSave.bindValue(":itemsrcp_itemsrc_id", _itemsrcid);
  itemSave.bindValue(":itemsrcp_warehous_id", _site->id());
  itemSave.bindValue(":itemsrcp_dropship", QVariant(_dropship->isChecked()));
  itemSave.bindValue(":itemsrcp_qtybreak", _qtyBreak->toDouble());
  itemSave.exec();
  if(itemSave.first())
  {
    if (_metrics->boolean("MultiWhs"))
      errors << GuiErrorCheck(true, _qtyBreak,
                              tr("A Qty. Break with the specified Site, Drop Ship and Qty. \n"
                                 "already exists for this Item Source.") );
    else
      errors << GuiErrorCheck(true, _qtyBreak,
                              tr("A Qty. Break with the specified Qty. already exists for this Item Source.") );
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Item Source Price"), errors))
    return;

  if (_mode == cNew)
  {
    itemSave.prepare( "INSERT INTO itemsrcp "
                      "(itemsrcp_id, itemsrcp_itemsrc_id, itemsrcp_type,"
                      " itemsrcp_warehous_id, itemsrcp_dropship,"
                      " itemsrcp_qtybreak, itemsrcp_price,"
                      " itemsrcp_discntprcnt, itemsrcp_fixedamtdiscount,"
                      " itemsrcp_updated, itemsrcp_curr_id) "
                      "VALUES "
                      "(:itemsrcp_id, :itemsrcp_itemsrc_id, :itemsrcp_type,"
                      " :itemsrcp_warehous_id, :itemsrcp_dropship,"
                      " :itemsrcp_qtybreak, :itemsrcp_price,"
                      " :itemsrcp_discntprcnt, :itemsrcp_fixedamtdiscount,"
                      " CURRENT_DATE, :itemsrcp_curr_id);" );
  }
  else if (_mode == cEdit)
    itemSave.prepare( "UPDATE itemsrcp "
                      "SET itemsrcp_type=:itemsrcp_type,"
                      "    itemsrcp_warehous_id=:itemsrcp_warehous_id,"
                      "    itemsrcp_dropship=:itemsrcp_dropship,"
                      "    itemsrcp_qtybreak=:itemsrcp_qtybreak,"
                      "    itemsrcp_price=:itemsrcp_price,"
                      "    itemsrcp_discntprcnt=:itemsrcp_discntprcnt,"
                      "    itemsrcp_fixedamtdiscount=:itemsrcp_fixedamtdiscount,"
                      "    itemsrcp_updated=CURRENT_DATE,"
                      "    itemsrcp_curr_id=:itemsrcp_curr_id "
                      "WHERE (itemsrcp_id=:itemsrcp_id);" );

  itemSave.bindValue(":itemsrcp_id", _itemsrcpid);
  itemSave.bindValue(":itemsrcp_itemsrc_id", _itemsrcid);
  itemSave.bindValue(":itemsrcp_warehous_id", _site->id());
  itemSave.bindValue(":itemsrcp_dropship", QVariant(_dropship->isChecked()));
  itemSave.bindValue(":itemsrcp_qtybreak", _qtyBreak->toDouble());

  if(_itemSelected->isChecked())
  {
    itemSave.bindValue(":itemsrcp_type", "N");
    itemSave.bindValue(":itemsrcp_price", _price->localValue());
    itemSave.bindValue(":itemsrcp_curr_id", _price->id());
  }
  else if(_discountSelected->isChecked())
  {
    itemSave.bindValue(":itemsrcp_type", "D");
    itemSave.bindValue(":itemsrcp_discntprcnt", (_discount->toDouble() / 100.0));
    itemSave.bindValue(":itemsrcp_fixedamtdiscount", (_fixedAmtDiscount->toDouble()));
  }

  itemSave.exec();

  done(_itemsrcpid);
}

void itemSourcePrice::populate()
{
  XSqlQuery itempopulate;
  itempopulate.prepare( "SELECT itemsrcp.*, item_listcost "
                        "FROM itemsrcp JOIN itemsrc ON (itemsrc_id=itemsrcp_itemsrc_id) "
                        "              JOIN item ON (item_id=itemsrc_item_id) "
                        "WHERE (itemsrcp_id=:itemsrcp_id);" );
  itempopulate.bindValue(":itemsrcp_id", _itemsrcpid);
  itempopulate.exec();
  if (itempopulate.first())
  {
    _itemsrcid = itempopulate.value("itemsrcp_itemsrc_id").toInt();
    if(itempopulate.value("itemsrcp_type").toString() == "D")
      _discountSelected->setChecked(true);
    else
      _itemSelected->setChecked(true);
    if (itempopulate.value("itemsrcp_warehous_id").toInt() > 0)
      _site->setId(itempopulate.value("itemsrcp_warehous_id").toInt());
    else
      _site->setAll();
    _dropship->setChecked(itempopulate.value("itemsrcp_dropship").toBool());
    _qtyBreak->setDouble(itempopulate.value("itemsrcp_qtybreak").toDouble());
    _price->setLocalValue(itempopulate.value("itemsrcp_price").toDouble());
    _price->setEffective(itempopulate.value("itemsrcp_updated").toDate());
    _price->setId(itempopulate.value("itemsrcp_curr_id").toInt());
    _discount->setDouble(itempopulate.value("itemsrcp_discntprcnt").toDouble() * 100.0);
    _fixedAmtDiscount->setDouble(itempopulate.value("itemsrcp_fixedamtdiscount").toDouble());
    _listCost->setBaseValue(itempopulate.value("item_listcost").toDouble());
  }
}

void itemSourcePrice::sTypeChanged(bool pChecked)
{
  if(!pChecked)
    return;

  if(_itemSelected->isChecked())
  {
    _price->show();
    _priceLit->show();
    _discount->hide();
    _fixedAmtDiscount->hide();
    _listCost->hide();
    _discountLit->hide();
    _fixedAmtDiscountLit->hide();
    _listCostLit->hide();
  }
  else if(_discountSelected->isChecked())
  {
    _price->hide();
    _priceLit->hide();
    _discount->show();
    _fixedAmtDiscount->show();
    _listCost->show();
    _discountLit->show();
    _fixedAmtDiscountLit->show();
    _listCostLit->show();
  }
}

