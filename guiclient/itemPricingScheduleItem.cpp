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

#include "itemPricingScheduleItem.h"
#include "characteristicPrice.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>

/*
 *  Constructs a itemPricingScheduleItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemPricingScheduleItem::itemPricingScheduleItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_item, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sUpdateCosts(int)));
  connect(_price, SIGNAL(idChanged(int)), _actCost, SLOT(setId(int)));
  connect(_price, SIGNAL(idChanged(int)), _stdCost, SLOT(setId(int)));
  connect(_price, SIGNAL(idChanged(int)), _listPrice, SLOT(setId(int)));
  connect(_price, SIGNAL(effectiveChanged(const QDate&)), _actCost, SLOT(setEffective(const QDate&)));
  connect(_price, SIGNAL(effectiveChanged(const QDate&)), _stdCost, SLOT(setEffective(const QDate&)));
  connect(_price, SIGNAL(effectiveChanged(const QDate&)), _listPrice, SLOT(setEffective(const QDate&)));
  connect(_price, SIGNAL(valueChanged()), this, SLOT(sUpdateMargins()));
  connect(_itemSelected, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged()));
  connect(_qtyUOM, SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
  connect(_priceUOM, SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _ipsheadid = -1;
  _ipsitemid = -1;
  _ipsprodcatid = -1;
  _invuomid = -1;
  
  _charprice->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft );
  _charprice->addColumn(tr("Value"),          -1,          Qt::AlignLeft );
  _charprice->addColumn(tr("Price"),          _priceColumn,Qt::AlignRight );

  _qtyBreak->setValidator(omfgThis->qtyVal());
  _qtyBreakCat->setValidator(omfgThis->qtyVal());
  _discount->setValidator(omfgThis->percentVal());
  _item->setType(ItemLineEdit::cSold);
  _prodcat->setType(XComboBox::ProductCategories);
  
  _tab->setTabEnabled(_tab->indexOf(_configuredPrices),FALSE);
 
  _rejectedMsg = tr("The application has encountered an error and must "
                    "stop editing this Pricing Schedule.\n%1");
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemPricingScheduleItem::~itemPricingScheduleItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemPricingScheduleItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemPricingScheduleItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("ipshead_id", &valid);
  if (valid)
    _ipsheadid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
    _price->setId(param.toInt());

  param = pParams.value("updated", &valid);
  if (valid)
    _price->setEffective(param.toDate());

  param = pParams.value("ipsitem_id", &valid);
  if (valid)
  {
    _ipsitemid = param.toInt();
    _itemSelected->setChecked(true);
    populate();
  }

  param = pParams.value("ipsprodcat_id", &valid);
  if (valid)
  {
    _ipsprodcatid = param.toInt();
    _prodcatSelected->setChecked(true);
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _item->setReadOnly(TRUE);
      _prodcat->setEnabled(FALSE);
      _typeGroup->setEnabled(FALSE);

      if(_ipsitemid != -1)
        _qtyBreak->setFocus();
      else if(_ipsprodcatid != -1)
        _qtyBreakCat->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _prodcat->setEnabled(FALSE);
      _qtyBreak->setEnabled(FALSE);
      _qtyBreakCat->setEnabled(FALSE);
      _price->setEnabled(FALSE);
      _discount->setEnabled(FALSE);
      _typeGroup->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void itemPricingScheduleItem::sSave()
{
  sSave(TRUE);
}

void itemPricingScheduleItem::sSave( bool pClose)
{
  if (_mode == cNew)
  {
    if(_itemSelected->isChecked())
    {
      q.prepare( "SELECT ipsitem_id "
                 "  FROM ipsitem "
                 " WHERE((ipsitem_ipshead_id=:ipshead_id)"
                 "   AND (ipsitem_item_id=:item_id)"
                 "   AND (ipsitem_qty_uom_id=:uom_id)"
                 "   AND (ipsitem_qtybreak=:qtybreak));" );
      q.bindValue(":ipshead_id", _ipsheadid);
      q.bindValue(":item_id", _item->id());
      q.bindValue(":qtybreak", _qtyBreak->toDouble());
      q.bindValue(":uom_id", _qtyUOM->id());
      q.exec();
      if (q.first())
      {
        QMessageBox::critical( this, tr("Cannot Create Pricing Schedule Item"),
                               tr( "There is an existing Pricing Schedule Item for the selected Pricing Schedule, Item and Quantity Break defined.\n"
                                   "You may not create duplicate Pricing Schedule Items." ) );
        return;
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }

      q.exec("SELECT NEXTVAL('ipsitem_ipsitem_id_seq') AS ipsitem_id;");
      if (q.first())
        _ipsitemid = q.value("ipsitem_id").toInt();
      //  ToDo

      q.prepare( "INSERT INTO ipsitem "
                 "( ipsitem_id, ipsitem_ipshead_id, ipsitem_item_id, ipsitem_qty_uom_id, ipsitem_qtybreak, ipsitem_price_uom_id, ipsitem_price ) "
                 "VALUES "
                 "( :ipsitem_id, :ipshead_id, :ipsitem_item_id, :qty_uom_id, :ipsitem_qtybreak, :price_uom_id, :ipsitem_price );" );
    }
    else
    {
      q.prepare( "SELECT ipsprodcat_id "
                 "FROM ipsprodcat "
                 "WHERE ( (ipsprodcat_ipshead_id=:ipshead_id)"
                 " AND (ipsprodcat_prodcat_id=:prodcat_id)"
                 " AND (ipsprodcat_qtybreak=:qtybreak) );" );
      q.bindValue(":ipshead_id", _ipsheadid);
      q.bindValue(":prodcat_id", _prodcat->id());
      q.bindValue(":qtybreak", _qtyBreakCat->toDouble());
      q.exec();
      if (q.first())
      {
        QMessageBox::critical( this, tr("Cannot Create Pricing Schedule Item"),
                               tr( "There is an existing Pricing Schedule Item for the selected Pricing Schedule, Product Category and Quantity Break defined.\n"
                                   "You may not create duplicate Pricing Schedule Items." ) );
        return;
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }

      q.exec("SELECT NEXTVAL('ipsprodcat_ipsprodcat_id_seq') AS ipsprodcat_id;");
      if (q.first())
        _ipsprodcatid = q.value("ipsprodcat_id").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }
      //  ToDo

      q.prepare( "INSERT INTO ipsprodcat "
                 "( ipsprodcat_id, ipsprodcat_ipshead_id, ipsprodcat_prodcat_id, ipsprodcat_qtybreak, ipsprodcat_discntprcnt ) "
                 "VALUES "
                 "( :ipsprodcat_id, :ipshead_id, :ipsprodcat_prodcat_id, :ipsprodcat_qtybreak, :ipsprodcat_discntprcnt );" );
    }
  }
  else if (_mode == cEdit)
  {
    if(_itemSelected->isChecked())
      q.prepare( "UPDATE ipsitem "
                 "   SET ipsitem_qty_uom_id=:qty_uom_id,"
                 "       ipsitem_qtybreak=:ipsitem_qtybreak,"
                 "       ipsitem_price_uom_id=:price_uom_id,"
                 "       ipsitem_price=:ipsitem_price "
                 "WHERE (ipsitem_id=:ipsitem_id);" );
    else
      q.prepare( "UPDATE ipsprodcat "
                 "SET ipsprodcat_qtybreak=:ipsprodcat_qtybreak, ipsprodcat_discntprcnt=:ipsprodcat_discntprcnt "
                 "WHERE (ipsprodcat_id=:ipsprodcat_id);" );
  }

  q.bindValue(":ipsitem_id", _ipsitemid);
  q.bindValue(":ipsprodcat_id", _ipsprodcatid);
  q.bindValue(":ipshead_id", _ipsheadid);
  q.bindValue(":ipsitem_item_id", _item->id());
  q.bindValue(":ipsprodcat_prodcat_id", _prodcat->id());
  q.bindValue(":ipsitem_qtybreak", _qtyBreak->toDouble());
  q.bindValue(":ipsprodcat_qtybreak", _qtyBreakCat->toDouble());
  q.bindValue(":ipsitem_price", _price->localValue());
  q.bindValue(":ipsprodcat_discntprcnt", (_discount->toDouble() / 100.0));
  q.bindValue(":qty_uom_id", _qtyUOM->id());
  q.bindValue(":price_uom_id", _priceUOM->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }

  if (pClose)
  {
    if(_itemSelected->isChecked())
      done(_ipsitemid);
    else
      done(_ipsprodcatid);
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
  if(_itemSelected->isChecked())
  {
    q.prepare( "SELECT ipsitem_item_id,"
               "       ipsitem_qty_uom_id,"
               "       ipsitem_qtybreak,"
               "       ipsitem_price_uom_id,"
               "       ipsitem_price "
               "FROM ipsitem "
               "WHERE (ipsitem_id=:ipsitem_id);" );
    q.bindValue(":ipsitem_id", _ipsitemid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("ipsitem_item_id").toInt());
      _qtyBreak->setDouble(q.value("ipsitem_qtybreak").toDouble());
      _price->setLocalValue(q.value("ipsitem_price").toDouble());
      _qtyUOM->setId(q.value("ipsitem_qty_uom_id").toInt());
      _priceUOM->setId(q.value("ipsitem_price_uom_id").toInt());

      sUpdateMargins();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
    }
  }
  else
  {
    q.prepare( "SELECT ipsprodcat_prodcat_id,"
               "       ipsprodcat_qtybreak, ipsprodcat_discntprcnt "
               "FROM ipsprodcat "
               "WHERE (ipsprodcat_id=:ipsprodcat_id);" );
    q.bindValue(":ipsprodcat_id", _ipsprodcatid);
    q.exec();
    if (q.first())
    {
      _prodcat->setId(q.value("ipsprodcat_prodcat_id").toInt());
      _qtyBreakCat->setDouble(q.value("ipsprodcat_qtybreak").toDouble());
      _discount->setText(formatPercent(q.value("ipsprodcat_discntprcnt").toDouble()));
    }
    else if (q.lastError().type() != QSqlError::None)
    {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
    }
  }
  sFillList();
}

void itemPricingScheduleItem::sUpdateCosts(int pItemid)
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
  if (q.lastError().type() != QSqlError::None)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }
  _qtyUOM->populate(uom);
  _priceUOM->populate(uom);

  XSqlQuery cost;
  cost.prepare( "SELECT item_inv_uom_id, item_price_uom_id,"
                "       formatUOMRatio(iteminvpricerat(item_id)) AS f_ratio,"
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
    _pricingRatio->setText(cost.value("f_ratio").toString());

    _stdCost->setBaseValue(cost.value("standard").toDouble());
    _actCost->setLocalValue(cost.value("actual").toDouble());

    _qtyUOM->setId(cost.value("item_inv_uom_id").toInt());
    _priceUOM->setId(cost.value("item_price_uom_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
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

    if (_stdCost->baseValue() > 0.0)
    {
      _stdMargin->setText(formatPercent((price - _stdCost->baseValue()) / price));

      if (_stdCost->baseValue() > price)
        _stdMargin->setPaletteForegroundColor(QColor("red"));
      else
        _stdMargin->setPaletteForegroundColor(QColor("black"));
    }
    else
    {
      _stdMargin->setText("N/A");
      _stdMargin->setPaletteForegroundColor(QColor("black"));
    }

    if (_actCost->baseValue() > 0.0)
    {
      _actMargin->setText(formatPercent((price - _actCost->baseValue()) / price));

      if (_actCost->baseValue() > price)
        _actMargin->setPaletteForegroundColor(QColor("red"));
      else
        _actMargin->setPaletteForegroundColor(QColor("black"));
    }
    else
    {
      _actMargin->setText("N/A");
      _actMargin->setPaletteForegroundColor(QColor("black"));
    }
  }
}

void itemPricingScheduleItem::sTypeChanged()
{
  if(_itemSelected->isChecked())
  {
    _widgetStack->setCurrentIndex(0);
    _save->setEnabled(_item->isValid());
  }
  else if(_prodcatSelected->isChecked())
  {
    _widgetStack->setCurrentIndex(1);
    _save->setEnabled(true);
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
                "       formatUOMRatio(itemuomtouomratio(item_id, :qtyuomid, :priceuomid)) AS f_ratio,"
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
    _pricingRatio->setText(cost.value("f_ratio").toString());

    _stdCost->setBaseValue(cost.value("standard").toDouble());
    _actCost->setLocalValue(cost.value("actual").toDouble());

    sUpdateMargins();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
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
    if (result == -1)
      done(-1);
     else
      sFillList();
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
    if (result == -1)
      done(-1);
     else
      sFillList();
}

void itemPricingScheduleItem::sDelete()
{
  q.prepare("DELETE FROM ipsitemchar "
            "WHERE (ipsitemchar_id=:ipsitemchar_id);");
  q.bindValue(":ipsitemchar_id", _charprice->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }
  sFillList();
}

void itemPricingScheduleItem::sFillList()
{
  q.prepare("SELECT ipsitemchar_id, char_name, ipsitemchar_value, ipsitemchar_price "
            "FROM ipsitemchar, char "
            "WHERE ((ipsitemchar_char_id=char_id) "
            "AND (ipsitemchar_ipsitem_id=:ipsitem_id)); ");
  q.bindValue(":ipsitem_id", _ipsitemid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }
  _charprice->populate(q);
}

