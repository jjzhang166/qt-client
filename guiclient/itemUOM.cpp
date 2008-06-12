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

#include "itemUOM.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <parameter.h>
#include "storedProcErrorLookup.h"

/*
 *  Constructs a itemUOM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemUOM::itemUOM(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _itemid = _itemuomconvid = -1;
  _uomidFrom = -1;
  _ignoreSignals = false;

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_uomFrom, SIGNAL(currentIndexChanged(int)), this, SLOT(sCheck()));
  connect(_uomTo, SIGNAL(currentIndexChanged(int)), this, SLOT(sCheck()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(sRemove()));

  _fromValue->setValidator(omfgThis->qtyVal());
  _toValue->setValidator(omfgThis->qtyVal());

  _fromValue->setDouble(1);
  _toValue->setDouble(1);

  _uomFrom->setType(XComboBox::UOMs);
  _uomTo->setType(XComboBox::UOMs);
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemUOM::~itemUOM()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemUOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemUOM::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    q.prepare("SELECT item_inv_uom_id"
              "  FROM item"
              " WHERE(item_id=:item_id);");
    q.bindValue(":item_id", _itemid);
    q.exec();
    if(q.first())
    {
      _uomidFrom = q.value("item_inv_uom_id").toInt();
      _ignoreSignals = true;
      _uomFrom->setId(_uomidFrom);
      _uomTo->setId(_uomidFrom);
      _ignoreSignals = false;
    }
  }

  param = pParams.value("itemuomconv_id", &valid);
  if (valid)
  {
    _itemuomconvid = param.toInt();
    _ignoreSignals = true;
    populate();
    _ignoreSignals = false;
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _toValue->setEnabled(false);
      _fromValue->setEnabled(false);
      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
      _add->setEnabled(false);
      _remove->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void itemUOM::sSave()
{
  if(_fromValue->toDouble() <= 0.0)
  {
    QMessageBox::warning(this, tr("Invalid Ratio"),
      tr("<p>You must specify a ratio value greater than 0."));
    _fromValue->setFocus();
    return;
  }

  if(_toValue->toDouble() <= 0.0)
  {
    QMessageBox::warning(this, tr("Invalid Ratio"),
      tr("<p>You must specify a ratio value greater than 0."));
    _toValue->setFocus();
    return;
  }

  if(_selected->count() == 0)
  {
    QMessageBox::warning(this, tr("No Types Selected"),
      tr("<p>You must select at least one UOM Type for this conversion."));
    return;
  }

  q.prepare("UPDATE itemuomconv"
            "   SET itemuomconv_to_value=:tovalue,"
            "       itemuomconv_from_value=:fromvalue,"
            "       itemuomconv_fractional=:fractional "
            " WHERE(itemuomconv_id=:itemuomconv_id);");
  q.bindValue(":itemuomconv_id", _itemuomconvid);
  q.bindValue(":tovalue", _toValue->toDouble());
  q.bindValue(":fromvalue", _fromValue->toDouble());
  q.bindValue(":fractional", QVariant(_fractional->isChecked(), 0));
  if(q.exec());
    accept();
}

void itemUOM::populate()
{
  q.prepare("SELECT itemuomconv_item_id, item_inv_uom_id,"
            "       itemuomconv_from_uom_id, itemuomconv_to_uom_id,"
            "       itemuomconv_from_value, itemuomconv_to_value, itemuomconv_fractional,"
            "       (uomconv_id IS NOT NULL) AS global"
            "  FROM itemuomconv"
            "  JOIN item ON (itemuomconv_item_id=item_id)"
            "  LEFT OUTER JOIN uomconv"
            "    ON ((uomconv_from_uom_id=itemuomconv_from_uom_id AND uomconv_to_uom_id=itemuomconv_to_uom_id)"
            "     OR (uomconv_to_uom_id=itemuomconv_from_uom_id AND uomconv_from_uom_id=itemuomconv_to_uom_id))"
            " WHERE((itemuomconv_id=:itemuomconv_id));");
  q.bindValue(":itemuomconv_id", _itemuomconvid);
  q.exec();
  if(q.first())
  {
    _itemid = q.value("itemuomconv_item_id").toInt();
    _uomidFrom = q.value("item_inv_uom_id").toInt();
    _uomFrom->setId(q.value("itemuomconv_from_uom_id").toInt());
    _uomTo->setId(q.value("itemuomconv_to_uom_id").toInt());
    _fromValue->setDouble(q.value("itemuomconv_from_value").toDouble());
    _toValue->setDouble(q.value("itemuomconv_to_value").toDouble());
    _fractional->setChecked(q.value("itemuomconv_fractional").toBool());
    _toValue->setEnabled(!q.value("global").toBool());
    _fromValue->setEnabled(!q.value("global").toBool());

    sFillList();
  }
}

void itemUOM::sFillList()
{
  _available->clear();
  _selected->clear();
  q.prepare("SELECT uomtype_id, uomtype_name,"
            "       uomtype_descrip, (itemuom_id IS NOT NULL) AS selected"
            "  FROM uomtype"
            "  LEFT OUTER JOIN itemuom ON ((itemuom_uomtype_id=uomtype_id)"
            "                          AND (itemuom_itemuomconv_id=:itemuomconv_id))"
            " WHERE (uomtype_id NOT IN"
            "        (SELECT uomtype_id"
            "           FROM itemuomconv"
            "           JOIN itemuom ON (itemuom_itemuomconv_id=itemuomconv_id)"
            "           JOIN uomtype ON (itemuom_uomtype_id=uomtype_id)"
            "          WHERE((itemuomconv_item_id=:item_id)"
            "            AND (itemuomconv_id != :itemuomconv_id)"
            "            AND (uomtype_multiple=false))))"
            " ORDER BY uomtype_name;");
  q.bindValue(":item_id", _itemid);
  q.bindValue(":itemuomconv_id", _itemuomconvid);
  q.exec();
  while(q.next())
  {
    QListWidgetItem *item = new QListWidgetItem(q.value("uomtype_name").toString());
    item->setToolTip(q.value("uomtype_descrip").toString());
    item->setData(Qt::UserRole, q.value("uomtype_id").toInt());

    if(q.value("selected").toBool())
      _selected->addItem(item);
    else
      _available->addItem(item);
  }
}

void itemUOM::sAdd()
{
  QList<QListWidgetItem*> items = _available->selectedItems();
  QListWidgetItem * item;
  for(int i = 0; i < items.size(); i++)
  {
    item = items.at(i);
    q.prepare("INSERT INTO itemuom"
              "      (itemuom_itemuomconv_id, itemuom_uomtype_id) "
              "VALUES(:itemuomconv_id, :uomtype_id);");
    q.bindValue(":itemuomconv_id", _itemuomconvid);
    q.bindValue(":uomtype_id", item->data(Qt::UserRole));
    q.exec();
  }
  sFillList();
}

void itemUOM::sRemove()
{
  QList<QListWidgetItem*> items = _selected->selectedItems();
  QListWidgetItem * item;
  for(int i = 0; i < items.size(); i++)
  {
    item = items.at(i);
    q.prepare("SELECT deleteItemuom(itemuom_id) AS result"
              "  FROM itemuom"
              " WHERE((itemuom_itemuomconv_id=:itemuomconv_id)"
              "   AND (itemuom_uomtype_id=:uomtype_id));");
    q.bindValue(":itemuomconv_id", _itemuomconvid);
    q.bindValue(":uomtype_id", item->data(Qt::UserRole));
    q.exec();
    // TODO: add in some addtional error checking
  }
  sFillList();
}

void itemUOM::sCheck()
{
  if(cNew != _mode || _ignoreSignals)
    return;

  _ignoreSignals = true;

  _uomFrom->setEnabled(false);
  _uomTo->setEnabled(false);

  q.prepare("SELECT itemuomconv_id"
            "  FROM itemuomconv"
            " WHERE((itemuomconv_item_id=:item_id)"
            "   AND (((itemuomconv_from_uom_id=:from_uom_id) AND (itemuomconv_to_uom_id=:to_uom_id))"
            "     OR ((itemuomconv_from_uom_id=:to_uom_id) AND (itemuomconv_to_uom_id=:from_uom_id))));");
  q.bindValue(":item_id", _itemid);
  q.bindValue(":from_uom_id", _uomFrom->id());
  q.bindValue(":to_uom_id", _uomTo->id());
  q.exec();
  if(q.first())
  {
    _itemuomconvid = q.value("itemuomconv_id").toInt();
    _mode = cEdit;
    populate();
    _ignoreSignals = false;
    return;
  }

  q.prepare("SELECT uomconv_from_uom_id, uomconv_from_value,"
            "       uomconv_to_uom_id, uomconv_to_value,"
            "       uomconv_fractional"
            "  FROM uomconv"
            " WHERE(((uomconv_from_uom_id=:from_uom_id) AND (uomconv_to_uom_id=:to_uom_id))"
            "    OR ((uomconv_from_uom_id=:to_uom_id) AND (uomconv_to_uom_id=:from_uom_id)));");
  q.bindValue(":from_uom_id", _uomFrom->id());
  q.bindValue(":to_uom_id", _uomTo->id());
  q.exec();
  if(q.first())
  {
    _uomFrom->setId(q.value("uomconv_from_uom_id").toInt());
    _uomTo->setId(q.value("uomconv_to_uom_id").toInt());
    _fromValue->setDouble(q.value("uomconv_from_value").toDouble());
    _toValue->setDouble(q.value("uomconv_to_value").toDouble());
    _fractional->setChecked(q.value("uomconv_fractional").toBool());
    _fromValue->setEnabled(false);
    _toValue->setEnabled(false);
  }
  else
  {
    _fromValue->setEnabled(true);
    _toValue->setEnabled(true);
  }

  q.exec("SELECT nextval('itemuomconv_itemuomconv_id_seq') AS result;");
  q.first();
  _itemuomconvid = q.value("result").toInt();

  q.prepare("INSERT INTO itemuomconv"
            "      (itemuomconv_id, itemuomconv_item_id, itemuomconv_from_uom_id, itemuomconv_to_uom_id,"
            "       itemuomconv_from_value, itemuomconv_to_value, itemuomconv_fractional) "
            "VALUES(:id, :item_id, :from_uom_id, :to_uom_id, :fromvalue, :tovalue, :fractional);");
  q.bindValue(":id", _itemuomconvid);
  q.bindValue(":item_id", _itemid);
  q.bindValue(":from_uom_id", _uomFrom->id());
  q.bindValue(":to_uom_id", _uomTo->id());
  q.bindValue(":fromvalue", _fromValue->toDouble());
  q.bindValue(":tovalue", _toValue->toDouble());
  q.bindValue(":fractional", QVariant(_fractional->isChecked(), 0));
  q.exec();

  sFillList();

  _ignoreSignals = false;
}

void itemUOM::reject()
{
  if(cNew == _mode)
  {
    q.prepare("DELETE FROM itemuom WHERE itemuom_itemuomconv_id=:itemuomconv_id;"
              "DELETE FROM itemuomconv WHERE itemuomconv_id=:itemuomconv_id;");
    q.bindValue(":itemuomconv_id", _itemuomconvid);
    q.exec();
  }

  XDialog::reject();
}
