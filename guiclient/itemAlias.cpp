/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemAlias.h"

#include <QVariant>
#include <QMessageBox>

#include "errorReporter.h"
#include "guiErrorCheck.h"

itemAlias::itemAlias(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _itemaliasid = -1;
  _itemid = -1;
}

itemAlias::~itemAlias()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemAlias::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemAlias::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    itemet.prepare("SELECT item_number FROM item WHERE (item_id=:item_id);");
    itemet.bindValue(":item_id", _itemid);
    itemet.exec();
    if(itemet.first())
      _item->setText(itemet.value("item_number").toString());
  }

  param = pParams.value("itemalias_id", &valid);
  if (valid)
  {
    _itemaliasid = param.toInt();
    populate();
  }

  param = pParams.value("item_number", &valid);
  if (valid)
    _item->setText(param.toString());

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

      _number->setEnabled(false);
      _crmacct->setEnabled(false);
      _useDescription->setEnabled(false);
      _descrip1->setEnabled(false);
      _descrip2->setEnabled(false);
      _comments->setEnabled(false);

      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void itemAlias::sSave()
{
  XSqlQuery itemSave;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_number->text().trimmed().isEmpty(), _number,
                          tr("You must enter a valid Alias "
                             "before continuing"))
     ;

  _number->setText(_number->text().trimmed().toUpper());

  itemSave.prepare( "SELECT itemalias_id "
             "FROM itemalias "
             "WHERE ( (itemalias_item_id=:itemalias_item_id)"
             " AND (COALESCE(itemalias_crmacct_id, -1)=COALESCE(:itemalias_crmacct_id, -1))"
             " AND (itemalias_number=:itemalias_number)"
             " AND (itemalias_id != :itemalias_id) );" );
  itemSave.bindValue(":itemalias_id", _itemaliasid);
  itemSave.bindValue(":itemalias_item_id", _itemid);
  itemSave.bindValue(":itemalias_crmacct_id", _crmacct->id());
  itemSave.bindValue(":itemalias_number", _number->text());
  itemSave.exec();
  if (itemSave.first())
    errors << GuiErrorCheck(true, _number,
                            tr( "An Item Alias for the selected Item Number has already been defined\n"
                                "with the selected Alias Item Number and Account.\n"
                                "You may not create duplicate Item Aliases." ) );

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Item Alias"), errors))
    return;

  if (_mode == cNew)
  {
    itemSave.exec("SELECT NEXTVAL('itemalias_itemalias_id_seq') AS _itemalias_id;");
    if (itemSave.first())
      _itemaliasid = itemSave.value("_itemalias_id").toInt();

    itemSave.prepare( "INSERT INTO itemalias "
               "( itemalias_id, itemalias_item_id, itemalias_number, itemalias_crmacct_id,"
               "  itemalias_usedescrip, itemalias_descrip1, itemalias_descrip2,"
               "  itemalias_comments ) "
               "VALUES "
               "( :itemalias_id, :itemalias_item_id, :itemalias_number, :itemalias_crmacct_id,"
               "  :itemalias_usedescrip, :itemalias_descrip1, :itemalias_descrip2,"
               "  :itemalias_comments );" );
  }
  else if (_mode == cEdit)
    itemSave.prepare( "UPDATE itemalias "
               "SET itemalias_number=:itemalias_number, itemalias_comments=:itemalias_comments,"
               "    itemalias_crmacct_id=:itemalias_crmacct_id,"
               "    itemalias_usedescrip=:itemalias_usedescrip,"
               "    itemalias_descrip1=:itemalias_descrip1, itemalias_descrip2=:itemalias_descrip2 "
               "WHERE (itemalias_id=:itemalias_id);" );

  itemSave.bindValue(":itemalias_id", _itemaliasid);
  itemSave.bindValue(":itemalias_item_id", _itemid);
  itemSave.bindValue(":itemalias_number", _number->text().trimmed());
  if (_crmacct->id() != -1)
    itemSave.bindValue(":itemalias_crmacct_id", _crmacct->id());
  itemSave.bindValue(":itemalias_descrip1", _descrip1->text().trimmed());
  itemSave.bindValue(":itemalias_descrip2", _descrip2->text().trimmed());
  itemSave.bindValue(":itemalias_comments", _comments->toPlainText());
  itemSave.bindValue(":itemalias_usedescrip", QVariant(_useDescription->isChecked()));
  itemSave.exec();

  done(_itemaliasid);
}

void itemAlias::populate()
{
  XSqlQuery itempopulate;
  itempopulate.prepare( "SELECT itemalias.*, item_number "
             "FROM itemalias LEFT OUTER JOIN item ON (itemalias_item_id=item_id) "
             "WHERE (itemalias_id=:itemalias_id);" );
  itempopulate.bindValue(":itemalias_id", _itemaliasid);
  itempopulate.exec();
  if (itempopulate.first())
  {
    _itemid = itempopulate.value("itemalias_item_id").toInt();
    _item->setText(itempopulate.value("item_number").toString());
    _number->setText(itempopulate.value("itemalias_number").toString());
    _crmacct->setId(itempopulate.value("itemalias_crmacct_id").toInt());

    if (itempopulate.value("itemalias_usedescrip").toBool())
    {
      _useDescription->setChecked(true);
      _descriptionGroup->setEnabled(true);
      _descrip1->setText(itempopulate.value("itemalias_descrip1").toString());
      _descrip2->setText(itempopulate.value("itemalias_descrip2").toString());
    }
    else
    {
      _useDescription->setChecked(false);
      _descriptionGroup->setEnabled(false);
    }

    _comments->setText(itempopulate.value("itemalias_comments").toString());
  }
}

