/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSubstitute.h"

#include <QVariant>
#include <QMessageBox>

#define cItemSub    0x01
#define cBOMItemSub 0x02

itemSubstitute::itemSubstitute(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_substitute, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _uomRatio->setValidator(omfgThis->ratioVal());
  _uomRatio->setText("1.0");
}

itemSubstitute::~itemSubstitute()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemSubstitute::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemSubstitute::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("bomitem_id", &valid);
  if (valid)
  {
    _type = cBOMItemSub;
    _bomitemid = param.toInt();
    itemet.prepare( "SELECT bomitem_item_id "
               "FROM bomitem "
               "WHERE (bomitem_id=:bomitem_id);" );
    itemet.bindValue(":bomitem_id", _bomitemid);
    itemet.exec();
    if (itemet.first())
    {
      _item->setId(itemet.value("bomitem_item_id").toInt());
      _item->setReadOnly(true);
    }
  }

  param = pParams.value("bomitem_item_id", &valid);
  if (valid)
  {
    _type = cBOMItemSub;
    _item->setId(param.toInt());
    _item->setReadOnly(true);
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _type = cItemSub;
    _item->setId(param.toInt());
    _item->setReadOnly(true);
  }

  param = pParams.value("itemsub_id", &valid);
  if (valid)
  {
    _type = cItemSub;
    _itemsubid = param.toInt();
    populate();
  }

  param = pParams.value("bomitemsub_id", &valid);
  if (valid)
  {
    _type = cBOMItemSub;
    _itemsubid = param.toInt();

    _item->setEnabled(false);

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
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(true);
      _substitute->setReadOnly(true);
      _uomRatio->setEnabled(false);
      _ranking->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void itemSubstitute::sSave()
{
  XSqlQuery itemSave;
  if (_type == cItemSub)
  {
    if (_item->id() == _substitute->id())
    {
      QMessageBox::critical( this, tr("Cannot Create Substitute"),
                             tr("You cannot define an Item to be a substitute for itself.") );
      _substitute->setFocus();
      return;
    }

    itemSave.prepare( "SELECT itemsub_id "
               "  FROM itemsub "
               " WHERE((itemsub_parent_item_id=:parentItem_id)"
               "   AND (itemsub_sub_item_id=:item_id)"
               "   AND (itemsub_id != :itemsub_id) );" );
    itemSave.bindValue(":parentItem_id", _item->id());
    itemSave.bindValue(":item_id", _substitute->id());
    itemSave.bindValue(":itemsub_id", _itemsubid);
    itemSave.exec();
    if (itemSave.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Duplicate Substitute"),
                             tr( "This substitution has already been defined for the selected Item.\n"
                                 "You may edit the existing Substitution but you may not create a duplicate." ) );
      _substitute->setFocus();
      return;
    }

    if (_mode == cNew)
    {
      itemSave.exec("SELECT NEXTVAL('itemsub_itemsub_id_seq') AS _itemsub_id");
      if (itemSave.first())
        _itemsubid = itemSave.value("_itemsub_id").toInt();
//  ToDo

      itemSave.prepare( "INSERT INTO itemsub "
                 "( itemsub_id, itemsub_parent_item_id, itemsub_sub_item_id,"
                 "  itemsub_uomratio, itemsub_rank ) "
                 "VALUES "
                 "( :itemsub_id, :itemsub_parent_item_id, :itemsub_sub_item_id,"
                 "  :itemsub_uomratio, :itemsub_rank );" );
    }
    else if (_mode == cEdit)
      itemSave.prepare( "UPDATE itemsub "
                 "   SET itemsub_uomratio=:itemsub_uomratio,"
                 "       itemsub_rank=:itemsub_rank,"
                 "       itemsub_sub_item_id=:itemsub_sub_item_id"
                 " WHERE(itemsub_id=:itemsub_id);" );

    itemSave.bindValue(":itemsub_id", _itemsubid);
    itemSave.bindValue(":itemsub_parent_item_id", _item->id());
    itemSave.bindValue(":itemsub_sub_item_id", _substitute->id());
    itemSave.bindValue(":itemsub_uomratio", _uomRatio->toDouble());
    itemSave.bindValue(":itemsub_rank", _ranking->value());
    itemSave.exec();
  }
  else if (_type == cBOMItemSub)
  {
    if (_mode == cNew)
    {
      itemSave.prepare( "SELECT bomitemsub_id "
                 "FROM bomitemsub "
                 "WHERE ( (bomitemsub_bomitem_id=:bomitem_id)"
                 " AND (bomitemsub_item_id=:item_id) );" );
      itemSave.bindValue(":bomitem_id", _bomitemid);
      itemSave.bindValue(":item_id", _substitute->id());
      itemSave.exec();
      if (itemSave.first())
      {
        QMessageBox::critical( this, tr("Cannot Create Duplicate Substitute"),
                               tr( "This substitution has already been defined for the selected BOM Item.\n"
                                   "You may edit the existing Substitution but you may not create a duplicate." ) );
        _substitute->setFocus();
        return;
      }
    }

    if (_mode == cNew)
    {
      itemSave.exec("SELECT NEXTVAL('bomitemsub_bomitemsub_id_seq') AS bomitemsub_id");
      if (itemSave.first())
        _itemsubid = itemSave.value("bomitemsub_id").toInt();
//  ToDo

      itemSave.prepare( "INSERT INTO bomitemsub "
                 "( bomitemsub_id, bomitemsub_bomitem_id, bomitemsub_item_id,"
                 "  bomitemsub_uomratio, bomitemsub_rank ) "
                 "VALUES "
                 "( :bomitemsub_id, :bomitemsub_bomitem_id, :bomitemsub_item_id,"
                 "  :bomitemsub_uomratio, :bomitemsub_rank );" );
    }
    else if (_mode == cEdit)
      itemSave.prepare( "UPDATE bomitemsub "
                 "SET bomitemsub_uomratio=:bomitemsub_uomratio, bomitemsub_rank=:bomitemsub_rank "
                 "WHERE (bomitemsub_id=:bomitemsub_id);" );

    itemSave.bindValue(":bomitemsub_id", _itemsubid);
    itemSave.bindValue(":bomitemsub_bomitem_id", _bomitemid);
    itemSave.bindValue(":bomitemsub_item_id", _substitute->id());
    itemSave.bindValue(":bomitemsub_uomratio", _uomRatio->toDouble());
    itemSave.bindValue(":bomitemsub_rank", _ranking->value());
    itemSave.exec();
  }

  done(_itemsubid);
}

void itemSubstitute::populate()
{
  XSqlQuery itempopulate;
  if (_type == cItemSub)
  {
    itempopulate.prepare( "SELECT itemsub_parent_item_id, itemsub_sub_item_id,"
               "       itemsub_uomratio, itemsub_rank "
               "FROM itemsub "
               "WHERE (itemsub_id=:itemsub_id);" );
    itempopulate.bindValue(":itemsub_id", _itemsubid);
    itempopulate.exec();
    if (itempopulate.first())
    {
      _item->setId(itempopulate.value("itemsub_parent_item_id").toInt());
      _substitute->setId(itempopulate.value("itemsub_sub_item_id").toInt());
      _ranking->setValue(itempopulate.value("itemsub_rank").toInt());
      _uomRatio->setDouble(itempopulate.value("itemsub_uomratio").toDouble());
    }
  }
  else if (_type == cBOMItemSub)
  {
    itempopulate.prepare( "SELECT bomitemsub_bomitem_id, item_id,  bomitemsub_item_id,"
               "       bomitemsub_uomratio, bomitemsub_rank "
               "FROM bomitemsub, bomitem, item "
               "WHERE ( (bomitemsub_bomitem_id=bomitem_id)"
               " AND (bomitem_item_id=item_id)"
               " AND (bomitemsub_id=:bomitemsub_id) );" );
    itempopulate.bindValue(":bomitemsub_id", _itemsubid);
    itempopulate.exec();
    if (itempopulate.first())
    {
      _item->setId(itempopulate.value("item_id").toInt());
      _substitute->setId(itempopulate.value("bomitemsub_item_id").toInt());
      _ranking->setValue(itempopulate.value("bomitemsub_rank").toInt());
      _uomRatio->setDouble(itempopulate.value("bomitemsub_uomratio").toDouble());
    }
  }
}

