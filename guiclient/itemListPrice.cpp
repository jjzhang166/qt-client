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

#include "itemListPrice.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

/*
 *  Constructs a itemListPrice as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemListPrice::itemListPrice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_item, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulate()));
  connect(_listPrice, SIGNAL(lostFocus()), this, SLOT(sUpdateMargins()));

  _item->setType(ItemLineEdit::cSold);
  _listPrice->setValidator(omfgThis->priceVal());

  if (!_privileges->check("MaintainListPrices"))
  {
    _listPrice->setEnabled(FALSE);
    _close->setText(tr("&Close"));
    _save->hide();
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemListPrice::~itemListPrice()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemListPrice::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemListPrice::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  return NoError;
}

void itemListPrice::sSave()
{
  q.prepare( "UPDATE item "
             "SET item_listprice=:item_listprice "
             "WHERE (item_id=:item_id);" );
  q.bindValue(":item_listprice", _listPrice->toDouble());
  q.bindValue(":item_id", _item->id());
  q.exec();

  accept();
}

void itemListPrice::sPopulate()
{
  q.prepare( "SELECT uom_name, invpricerat,"
             "       formatSalesPrice(item_listprice) AS f_listprice,"
             "       formatUOMRatio(iteminvpricerat(item_id)) AS f_ratio,"
             "       formatSalesPrice(item_listprice / iteminvpricerat(item_id)) AS f_extprice,"
             "       standardcost, formatCost(standardcost) AS f_standardcost,"
             "       actualcost, formatCost(actualCost) AS f_actualcost,"
             "       formatCost(standardcost * iteminvpricerat(item_id)) AS f_extstandardcost,"
             "       formatCost(actualCost * iteminvpricerat(item_id)) AS f_extactualcost "
             "FROM ( SELECT item_id, uom_name, item_listprice, iteminvpricerat(item_id) AS invpricerat,"
             "              stdCost(item_id) AS standardcost,"
             "              actCost(item_id) AS actualcost "
             "       FROM item JOIN uom ON (item_price_uom_id=uom_id)"
             "       WHERE (item_id=:item_id) ) AS data;" );
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    _cachedRatio = q.value("invpricerat").toDouble();
    _cachedStdCost = q.value("standardcost").toDouble();
    _cachedActCost = q.value("actualcost").toDouble();

    _listPrice->setText(q.value("f_listprice").toString());
    _priceUOM->setText(q.value("uom_name").toString());
    _pricingRatio->setText(q.value("f_ratio").toString());
    _extPrice->setText(q.value("f_extprice").toString());

    _stdCost->setText(q.value("f_standardcost").toString());
    _actCost->setText(q.value("f_actualcost").toString());
    _extStdCost->setText(q.value("f_extstandardcost").toString());
    _extActCost->setText(q.value("f_extactualcost").toString());
  }

  sUpdateMargins();
}

void itemListPrice::sUpdateMargins()
{
  if (_item->isValid())
  {
    double price = _listPrice->toDouble() / _cachedRatio;

    _extPrice->setText(formatSalesPrice(price));

    if (_cachedStdCost > 0.0)
    {
      _stdMargin->setText(formatPercent((price - _cachedStdCost) / price));

      if (_cachedStdCost > price)
        _stdMargin->setPaletteForegroundColor(QColor("red"));
      else
        _stdMargin->setPaletteForegroundColor(QColor("black"));
    }
    else
    {
      _stdMargin->setText("N/A");
      _stdMargin->setPaletteForegroundColor(QColor("black"));
    }

    if (_cachedActCost > 0.0)
    {
      _actMargin->setText(formatPercent((price - _cachedActCost) / price));

      if (_cachedActCost > price)
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
