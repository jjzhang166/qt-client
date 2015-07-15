/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemGroup.h"

#include <QMessageBox>
#include <QVariant>

#include "itemcluster.h"

itemGroup::itemGroup(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_newParent, SIGNAL(clicked()), this, SLOT(sNewParent()));
  connect(_deleteParent, SIGNAL(clicked()), this, SLOT(sDeleteParent()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));

  _itemgrpitem->addColumn(tr("Name"),        _itemColumn,  Qt::AlignLeft, true, "item_number" );
  _itemgrpitem->addColumn(tr("Description"), -1,           Qt::AlignLeft, true, "item_descrip" );

  _itemgrpparent->addColumn(tr("Name"),        _itemColumn,  Qt::AlignLeft, true, "itemgrp_name" );
  _itemgrpparent->addColumn(tr("Description"), -1,           Qt::AlignLeft, true, "itemgrp_descrip" );
}

itemGroup::~itemGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemGroup::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemGroup::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
  {
    _itemgrpid = param.toInt();
    populate();
    emit newId(_itemgrpid);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      itemet.exec("SELECT NEXTVAL('itemgrp_itemgrp_id_seq') AS itemgrp_id;");
      if (itemet.first())
        _itemgrpid = itemet.value("itemgrp_id").toInt();

      sFillList();

      connect(_itemgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_itemgrpparent, SIGNAL(valid(bool)), _deleteParent, SLOT(setEnabled(bool)));
      emit newId(_itemgrpid);
      emit newMode(_mode);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_itemgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_itemgrpparent, SIGNAL(valid(bool)), _deleteParent, SLOT(setEnabled(bool)));
      emit newMode(_mode);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(false);
      _descrip->setEnabled(false);
      _catalog->setEnabled(false);
      _new->setEnabled(false);
      _newParent->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
      emit newMode(_mode);
    }
  }

  return NoError;
}

void itemGroup::sCheck()
{
  XSqlQuery itemCheck;
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length()))
  {
    itemCheck.prepare( "SELECT itemgrp_id "
               "FROM itemgrp "
               "WHERE (UPPER(itemgrp_name)=UPPER(:itemgrp_name));" );
    itemCheck.bindValue(":itemgrp_name", _name->text());
    itemCheck.exec();
    if (itemCheck.first())
    {
      _itemgrpid = itemCheck.value("itemgrp_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);

      emit newId(_itemgrpid);
      emit newMode(_mode);
    }
  }
}

void itemGroup::sClose()
{
  XSqlQuery itemClose;
  if (_mode == cNew)
  {
    itemClose.prepare( "DELETE FROM itemgrpitem "
               "WHERE (itemgrpitem_itemgrp_id=:itemgrp_id);"

                "DELETE FROM itemgrp "
                "WHERE (itemgrp_id=:itemgrp_id);" );
    itemClose.bindValue(":itemgrp_id", _itemgrpid);
    itemClose.exec();
  }

  close();
}

void itemGroup::sSave()
{
  XSqlQuery itemSave;
  if (_name->text().trimmed().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot Save Item Group"),
                          tr("You must enter a Name for this Item Group before you may save it.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
    itemSave.prepare( "INSERT INTO itemgrp "
               "(itemgrp_id, itemgrp_name, itemgrp_descrip, itemgrp_catalog) "
               "VALUES "
               "(:itemgrp_id, :itemgrp_name, :itemgrp_descrip, :itemgrp_catalog);" );
  else
    itemSave.prepare( "UPDATE itemgrp "
               "SET itemgrp_name=:itemgrp_name,"
               "    itemgrp_descrip=:itemgrp_descrip,"
               "    itemgrp_catalog=:itemgrp_catalog "
               "WHERE (itemgrp_id=:itemgrp_id);" );

  itemSave.bindValue(":itemgrp_id", _itemgrpid);
  itemSave.bindValue(":itemgrp_name", _name->text());
  itemSave.bindValue(":itemgrp_descrip", _descrip->text());
  itemSave.bindValue(":itemgrp_catalog", QVariant(_catalog->isChecked()));
  itemSave.exec();
  
  if (_catalog->isChecked())
  {
    itemSave.prepare( "UPDATE itemgrp "
                     "SET itemgrp_catalog=false "
                     "WHERE (itemgrp_id != :itemgrp_id);" );
    
    itemSave.bindValue(":itemgrp_id", _itemgrpid);
    itemSave.exec();
  }

  omfgThis->sItemGroupsUpdated(_itemgrpid, true);

  close();
}

void itemGroup::sDelete()
{
  XSqlQuery itemDelete;
  itemDelete.prepare( "DELETE FROM itemgrpitem "
             "WHERE (itemgrpitem_id=:itemgrpitem_id);" );
  itemDelete.bindValue(":itemgrpitem_id", _itemgrpitem->id());
  itemDelete.exec();

  sFillList();
}

void itemGroup::sDeleteParent()
{
  XSqlQuery itemDelete;
  itemDelete.prepare( "DELETE FROM itemgrpitem "
                     "WHERE (itemgrpitem_id=:itemgrpitem_id);" );
  itemDelete.bindValue(":itemgrpitem_id", _itemgrpparent->id());
  itemDelete.exec();
  
  sFillList();
}

void itemGroup::sNew()
{
  if (!_memberItem->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Add Item to Item Group"),
                         tr("Please select a new Member Item.") );
    return;
  }
  
  XSqlQuery itemNew;
  int itemid = _memberItem->id();
  if (itemid != -1)
  {
    itemNew.prepare( "SELECT itemgrpitem_id "
               "FROM itemgrpitem "
               "WHERE ( (itemgrpitem_itemgrp_id=:itemgrp_id)"
               " AND (itemgrpitem_item_type='I')"
               " AND (itemgrpitem_item_id=:item_id) );" );
    itemNew.bindValue(":itemgrp_id", _itemgrpid);
    itemNew.bindValue(":item_id", itemid);
    itemNew.exec();
    if (itemNew.first())
    {
      QMessageBox::warning( this, tr("Cannot Add Item to Item Group"),
                            tr("The selected Item is already a member of this Item Group") );
      return;
    }

    itemNew.prepare( "INSERT INTO itemgrpitem "
               "(itemgrpitem_itemgrp_id, itemgrpitem_item_type, itemgrpitem_item_id) "
               "VALUES "
               "(:itemgrpitem_itemgrp_id, 'I', :itemgrpitem_item_id);" );
    itemNew.bindValue(":itemgrpitem_itemgrp_id", _itemgrpid);
    itemNew.bindValue(":itemgrpitem_item_id", itemid);
    itemNew.exec();

    _memberItem->setId(-1);
    sFillList();
  }
}

void itemGroup::sNewParent()
{
  if (_parentGroup->id() < 1)
  {
    QMessageBox::warning( this, tr("Cannot Add Parent Group to Item Group"),
                         tr("Please select a new Parent Group.") );
    return;
  }
  
  XSqlQuery itemNew;
  int itemid = _parentGroup->id();
  if (itemid != -1)
  {
    itemNew.prepare( "SELECT itemgrpitem_id "
                    "FROM itemgrpitem "
                    "WHERE ( (itemgrpitem_item_id=:itemgrp_id)"
                    " AND (itemgrpitem_item_type='G')"
                    " AND (itemgrpitem_itemgrp_id=:item_id) );" );
    itemNew.bindValue(":itemgrp_id", _itemgrpid);
    itemNew.bindValue(":item_id", itemid);
    itemNew.exec();
    if (itemNew.first())
    {
      QMessageBox::warning( this, tr("Cannot Add Parent Group to Item Group"),
                           tr("The selected Group is already a parent of this Item Group") );
      return;
    }
    
    itemNew.prepare( "WITH RECURSIVE indentedgroups(id, name, descrip, catalog, depth, path, cycle) AS ( "
                    "SELECT itemgrp_id AS id, "
                    "       itemgrp_name AS name, "
                    "       itemgrp_descrip AS descrip, "
                    "       itemgrp_catalog AS catalog, "
                    "       0 AS depth, array[itemgrp_id] AS path, false AS cycle "
                    "FROM itemgrp WHERE (itemgrp_id=:itemgrp_id) "
                    "UNION ALL "
                    "SELECT itemgrp_id AS id, "
                    "       itemgrp_name AS name, "
                    "       itemgrp_descrip AS descrip, "
                    "       NULL AS catalog, "
                    "       (depth+1) AS depth, (path || itemgrp_id) AS path, (itemgrp_id = any(path)) AS cycle "
                    "FROM indentedgroups JOIN itemgrpitem ON (itemgrpitem_itemgrp_id=id) "
                    "  JOIN itemgrp ON (itemgrp_id=itemgrpitem_item_id AND itemgrpitem_item_type='G') "
                    "WHERE (NOT cycle) AND (itemgrp_id=:item_id) "
                    ") "
                    "SELECT id, name, descrip, catalog, depth, path, cycle "
                    "FROM indentedgroups "
                    "WHERE (id != :itemgrp_id);");
    itemNew.bindValue(":itemgrp_id", _itemgrpid);
    itemNew.bindValue(":item_id", itemid);
    itemNew.exec();
    if (itemNew.first())
    {
      QMessageBox::warning( this, tr("Cannot Add Parent Group to Item Group"),
                           tr("The selected Group is already a child of this Item Group") );
      return;
    }
    
    itemNew.prepare( "INSERT INTO itemgrpitem "
                    "(itemgrpitem_itemgrp_id, itemgrpitem_item_type, itemgrpitem_item_id) "
                    "VALUES "
                    "(:itemgrpitem_item_id, 'G', :itemgrpitem_itemgrp_id);" );
    itemNew.bindValue(":itemgrpitem_itemgrp_id", _itemgrpid);
    itemNew.bindValue(":itemgrpitem_item_id", itemid);
    itemNew.exec();
    
    _parentGroup->setId(-1);
    sFillList();
  }
}

void itemGroup::sFillList()
{
  XSqlQuery itemFillList;
  itemFillList.prepare( "SELECT itemgrpitem_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip "
             "FROM itemgrpitem JOIN item ON (item_id=itemgrpitem_item_id) "
             "WHERE ( (itemgrpitem_item_type='I') "
             " AND (itemgrpitem_itemgrp_id=:itemgrp_id) ) "
             "ORDER BY item_number;" );
  itemFillList.bindValue(":itemgrp_id", _itemgrpid);
  itemFillList.exec();
  _itemgrpitem->populate(itemFillList);

  XSqlQuery parentFillList;
  parentFillList.prepare( "SELECT itemgrpitem_id, itemgrp_name, itemgrp_descrip "
                       "FROM itemgrpitem JOIN itemgrp ON (itemgrp_id=itemgrpitem_itemgrp_id) "
                       "WHERE ( (itemgrpitem_item_type='G') "
                       " AND (itemgrpitem_item_id=:itemgrp_id) ) "
                       "ORDER BY itemgrp_name;" );
  parentFillList.bindValue(":itemgrp_id", _itemgrpid);
  parentFillList.exec();
  _itemgrpparent->populate(parentFillList);
  
  XSqlQuery groupFillList;
  groupFillList.prepare( "SELECT itemgrp_id, itemgrp_name "
                        "FROM itemgrp "
                        "WHERE (itemgrp_id != :itemgrp_id) "
                        "ORDER BY itemgrp_name;" );
  groupFillList.bindValue(":itemgrp_id", _itemgrpid);
  groupFillList.exec();
  _parentGroup->populate(groupFillList);
}

void itemGroup::populate()
{
  XSqlQuery itempopulate;
  itempopulate.prepare( "SELECT * "
             "FROM itemgrp "
             "WHERE (itemgrp_id=:itemgrp_id);" );
  itempopulate.bindValue(":itemgrp_id", _itemgrpid);
  itempopulate.exec();
  if (itempopulate.first())
  {
    _name->setText(itempopulate.value("itemgrp_name").toString());
    _descrip->setText(itempopulate.value("itemgrp_descrip").toString());
    _catalog->setChecked(itempopulate.value("itemgrp_catalog").toBool());
    if (_catalog->isChecked())
      _catalog->setEnabled(false);

    sFillList();
  }
}
