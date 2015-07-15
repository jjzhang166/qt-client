/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemUOM.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <parameter.h>
#include "storedProcErrorLookup.h"

itemUOM::itemUOM(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _itemid = _itemuomconvid = -1;
  _uomidFrom = -1;
  _ignoreSignals = false;

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_uomFrom, SIGNAL(currentIndexChanged(int)), this, SLOT(sCheck()));
  connect(_uomTo, SIGNAL(currentIndexChanged(int)), this, SLOT(sCheck()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(sRemove()));

  _fromValue->setValidator(omfgThis->ratioVal());
  _toValue->setValidator(omfgThis->ratioVal());

  _fromValue->setDouble(1);
  _toValue->setDouble(1);

  _uomFrom->setType(XComboBox::UOMs);
  _uomTo->setType(XComboBox::UOMs);
}

itemUOM::~itemUOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemUOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemUOM::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    itemet.prepare("SELECT item_inv_uom_id"
              "  FROM item"
              " WHERE(item_id=:item_id);");
    itemet.bindValue(":item_id", _itemid);
    itemet.exec();
    if(itemet.first())
    {
      _uomidFrom = itemet.value("item_inv_uom_id").toInt();
      _ignoreSignals = true;
      _uomFrom->setId(_uomidFrom);
      _uomTo->setId(_uomidFrom);
      _ignoreSignals = false;
    }
  }

  param = pParams.value("inventoryUOM", &valid);
  if (valid)
  {
      _uomidFrom = param.toInt();
      _ignoreSignals = true;
      _uomFrom->setId(_uomidFrom);
      _uomTo->setId(_uomidFrom);
      _ignoreSignals = false;
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
    {
      _active->setChecked(true);
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

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
    }
  }

  return NoError;
}

void itemUOM::sSave()
{
  XSqlQuery itemSave;
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

  itemSave.prepare("UPDATE itemuomconv"
            "   SET itemuomconv_to_value=:tovalue,"
            "       itemuomconv_from_value=:fromvalue,"
            "       itemuomconv_fractional=:fractional, "
            "       itemuomconv_active=:active "
            " WHERE(itemuomconv_id=:itemuomconv_id);");
  itemSave.bindValue(":itemuomconv_id", _itemuomconvid);
  itemSave.bindValue(":tovalue", _toValue->toDouble());
  itemSave.bindValue(":fromvalue", _fromValue->toDouble());
  itemSave.bindValue(":fractional", QVariant(_fractional->isChecked()));
  itemSave.bindValue(":active", QVariant(_active->isChecked()));
  if(itemSave.exec())
    accept();
}

void itemUOM::populate()
{
  XSqlQuery itempopulate;
  itempopulate.prepare("SELECT itemuomconv_item_id, item_inv_uom_id,"
            "       itemuomconv_from_uom_id, itemuomconv_to_uom_id,"
            "       itemuomconv_from_value, itemuomconv_to_value, itemuomconv_fractional,"
            "       itemuomconv_active, (uomconv_id IS NOT NULL) AS global"
            "  FROM itemuomconv"
            "  JOIN item ON (itemuomconv_item_id=item_id)"
            "  LEFT OUTER JOIN uomconv"
            "    ON ((uomconv_from_uom_id=itemuomconv_from_uom_id AND uomconv_to_uom_id=itemuomconv_to_uom_id)"
            "     OR (uomconv_to_uom_id=itemuomconv_from_uom_id AND uomconv_from_uom_id=itemuomconv_to_uom_id))"
            " WHERE((itemuomconv_id=:itemuomconv_id));");
  itempopulate.bindValue(":itemuomconv_id", _itemuomconvid);
  itempopulate.exec();
  if(itempopulate.first())
  {
    _itemid = itempopulate.value("itemuomconv_item_id").toInt();
    _uomidFrom = itempopulate.value("item_inv_uom_id").toInt();
    _uomFrom->setId(itempopulate.value("itemuomconv_from_uom_id").toInt());
    _uomTo->setId(itempopulate.value("itemuomconv_to_uom_id").toInt());
    _fromValue->setDouble(itempopulate.value("itemuomconv_from_value").toDouble());
    _toValue->setDouble(itempopulate.value("itemuomconv_to_value").toDouble());
    _fractional->setChecked(itempopulate.value("itemuomconv_fractional").toBool());
    _active->setChecked(itempopulate.value("itemuomconv_active").toBool());
    _toValue->setEnabled(!itempopulate.value("global").toBool());
    _fromValue->setEnabled(!itempopulate.value("global").toBool());

    sFillList();
  }
}

void itemUOM::sFillList()
{
  XSqlQuery itemFillList;
  _available->clear();
  _selected->clear();
  itemFillList.prepare("SELECT uomtype_id, uomtype_name,"
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
  itemFillList.bindValue(":item_id", _itemid);
  itemFillList.bindValue(":itemuomconv_id", _itemuomconvid);
  itemFillList.exec();
  while(itemFillList.next())
  {
    QListWidgetItem *item = new QListWidgetItem(itemFillList.value("uomtype_name").toString());
    item->setToolTip(itemFillList.value("uomtype_descrip").toString());
    item->setData(Qt::UserRole, itemFillList.value("uomtype_id").toInt());

    if(itemFillList.value("selected").toBool())
      _selected->addItem(item);
    else
      _available->addItem(item);
  }
  _add->setEnabled(true);
  _remove->setEnabled(true);
}

void itemUOM::sAdd()
{
  XSqlQuery itemAdd;
  QList<QListWidgetItem*> items = _available->selectedItems();
  QListWidgetItem * item;
  for(int i = 0; i < items.size(); i++)
  {
    item = items.at(i);
    itemAdd.prepare("INSERT INTO itemuom"
              "      (itemuom_itemuomconv_id, itemuom_uomtype_id) "
              "VALUES(:itemuomconv_id, :uomtype_id);");
    itemAdd.bindValue(":itemuomconv_id", _itemuomconvid);
    itemAdd.bindValue(":uomtype_id", item->data(Qt::UserRole));
    itemAdd.exec();
  }
  sFillList();
}

void itemUOM::sRemove()
{
  XSqlQuery itemRemove;
  QList<QListWidgetItem*> items = _selected->selectedItems();
  QListWidgetItem * item;
  for(int i = 0; i < items.size(); i++)
  {
    item = items.at(i);
    itemRemove.prepare("SELECT deleteItemuom(itemuom_id) AS result"
              "  FROM itemuom"
              " WHERE((itemuom_itemuomconv_id=:itemuomconv_id)"
              "   AND (itemuom_uomtype_id=:uomtype_id));");
    itemRemove.bindValue(":itemuomconv_id", _itemuomconvid);
    itemRemove.bindValue(":uomtype_id", item->data(Qt::UserRole));
    itemRemove.exec();
    // TODO: add in some addtional error checking
  }
  sFillList();
}

void itemUOM::sCheck()
{
  XSqlQuery itemCheck;
  if(cNew != _mode || _ignoreSignals)
    return;

  if (!(_uomFrom->isValid() && _uomTo->isValid()))
    return;
  if (_uomFrom->id() == _uomTo->id())
    return;

  _ignoreSignals = true;

  _uomFrom->setEnabled(false);
  _uomTo->setEnabled(false);

  itemCheck.prepare("SELECT itemuomconv_id"
            "  FROM itemuomconv"
            " WHERE((itemuomconv_item_id=:item_id)"
            "   AND (((itemuomconv_from_uom_id=:from_uom_id) AND (itemuomconv_to_uom_id=:to_uom_id))"
            "     OR ((itemuomconv_from_uom_id=:to_uom_id) AND (itemuomconv_to_uom_id=:from_uom_id))));");
  itemCheck.bindValue(":item_id", _itemid);
  itemCheck.bindValue(":from_uom_id", _uomFrom->id());
  itemCheck.bindValue(":to_uom_id", _uomTo->id());
  itemCheck.exec();
  if(itemCheck.first())
  {
    _itemuomconvid = itemCheck.value("itemuomconv_id").toInt();
    _mode = cEdit;
    populate();
    _ignoreSignals = false;
    return;
  }

  itemCheck.prepare("SELECT uomconv_from_uom_id, uomconv_from_value,"
            "       uomconv_to_uom_id, uomconv_to_value,"
            "       uomconv_fractional"
            "  FROM uomconv"
            " WHERE(((uomconv_from_uom_id=:from_uom_id) AND (uomconv_to_uom_id=:to_uom_id))"
            "    OR ((uomconv_from_uom_id=:to_uom_id) AND (uomconv_to_uom_id=:from_uom_id)));");
  itemCheck.bindValue(":from_uom_id", _uomFrom->id());
  itemCheck.bindValue(":to_uom_id", _uomTo->id());
  itemCheck.exec();
  if(itemCheck.first())
  {
    _uomFrom->setId(itemCheck.value("uomconv_from_uom_id").toInt());
    _uomTo->setId(itemCheck.value("uomconv_to_uom_id").toInt());
    _fromValue->setDouble(itemCheck.value("uomconv_from_value").toDouble());
    _toValue->setDouble(itemCheck.value("uomconv_to_value").toDouble());
    _fractional->setChecked(itemCheck.value("uomconv_fractional").toBool());
    _fromValue->setEnabled(false);
    _toValue->setEnabled(false);
  }
  else
  {
    _fromValue->setEnabled(true);
    _toValue->setEnabled(true);
  }

  itemCheck.exec("SELECT nextval('itemuomconv_itemuomconv_id_seq') AS result;");
  itemCheck.first();
  _itemuomconvid = itemCheck.value("result").toInt();

  itemCheck.prepare("INSERT INTO itemuomconv"
            "      (itemuomconv_id, itemuomconv_item_id, itemuomconv_from_uom_id, itemuomconv_to_uom_id,"
            "       itemuomconv_from_value, itemuomconv_to_value, itemuomconv_fractional) "
            "VALUES(:id, :item_id, :from_uom_id, :to_uom_id, :fromvalue, :tovalue, :fractional);");
  itemCheck.bindValue(":id", _itemuomconvid);
  itemCheck.bindValue(":item_id", _itemid);
  itemCheck.bindValue(":from_uom_id", _uomFrom->id());
  itemCheck.bindValue(":to_uom_id", _uomTo->id());
  itemCheck.bindValue(":fromvalue", _fromValue->toDouble());
  itemCheck.bindValue(":tovalue", _toValue->toDouble());
  itemCheck.bindValue(":fractional", QVariant(_fractional->isChecked()));
  itemCheck.exec();

  sFillList();

  _ignoreSignals = false;
}

void itemUOM::reject()
{
  XSqlQuery itemreject;
  if(cNew == _mode)
  {
    itemreject.prepare("DELETE FROM itemuom WHERE itemuom_itemuomconv_id=:itemuomconv_id;"
              "DELETE FROM itemuomconv WHERE itemuomconv_id=:itemuomconv_id;");
    itemreject.bindValue(":itemuomconv_id", _itemuomconvid);
    itemreject.exec();
  }

  if(cEdit == _mode && _selected->count() == 0)
  {
    QMessageBox::warning(this, tr("No Types Selected"),
      tr("<p>You must select at least one UOM Type for this conversion."));
    return;
  }

  XDialog::reject();
}
