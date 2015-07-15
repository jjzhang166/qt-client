/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSourceSearch.h"

#include <QVariant>
#include <QMessageBox>
#include <parameter.h>
#include <openreports.h>
#include <metasql.h>
#include "mqlutil.h"

/*
 *  Constructs a itemSourceSearch as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
itemSourceSearch::itemSourceSearch(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _vendid = -1;
  _itemid = -1;

  // signals and slots connections
  connect(_searchNumber, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDescrip1, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDescrip2, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchVendNumber, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchVendDescrip, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_search, SIGNAL(textChanged(QString)), this, SLOT(sFillList()));

  _itemsrc->addColumn(tr("Item Number"),     _itemColumn, Qt::AlignLeft, true, "item_number" );
  _itemsrc->addColumn(tr("Description"),              -1, Qt::AlignLeft, true, "item_descrip" );
  _itemsrc->addColumn(tr("Vendor"),          _itemColumn, Qt::AlignLeft, true, "vend_name" );
  _itemsrc->addColumn(tr("Vendor Item"),     _itemColumn, Qt::AlignLeft, true, "itemsrc_vend_item_number" );
  _itemsrc->addColumn(tr("Vendor Descrip"),  _itemColumn, Qt::AlignLeft, true, "itemsrc_vend_item_descrip" );
  _itemsrc->addColumn(tr("Manufacturer"),    _itemColumn, Qt::AlignLeft, true, "itemsrc_manuf_name" );
  _itemsrc->addColumn(tr("Manuf. Item#"),    _itemColumn, Qt::AlignLeft, true, "itemsrc_manuf_item_number" );
  _itemsrc->addColumn(tr("Manuf. Descrip."), _itemColumn, Qt::AlignLeft,false, "itemsrc_manuf_item_descrip" );
  _itemsrc->addColumn(tr("Contract"),                 -1, Qt::AlignLeft, true, "contrct_number");

}

/*
 *  Destroys the object and frees any allocated resources
 */
itemSourceSearch::~itemSourceSearch()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemSourceSearch::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemSourceSearch::set(const ParameterList & pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if(valid)
    _vendid = param.toInt();

  param = pParams.value("search", &valid);
  if(valid)
    _search->setText(param.toString());

  sFillList();

  return NoError;
}

void itemSourceSearch::sFillList()
{
  XSqlQuery itemFillList;
  _itemsrc->clear();
  MetaSQLQuery mql = mqlLoad("itemSources", "search");
  ParameterList params;
  params.append("vend_id", _vendid);
  params.append("item_id", _itemid);
  params.append("non", tr("Non-Inventory"));
  if(_searchNumber->isChecked())
    params.append("searchNumber", _search->text());
  if(_searchVendNumber->isChecked())
    params.append("searchVendNumber", _search->text());
  if(_searchDescrip1->isChecked())
    params.append("searchDescrip1", _search->text());
  if(_searchDescrip2->isChecked())
    params.append("searchDescrip2", _search->text());
  if(_searchVendDescrip->isChecked())
    params.append("searchVendDescrip", _search->text());
  if(_searchManufName->isChecked())
    params.append("searchManufName", _search->text());
  if(_searchManufNumber->isChecked())
    params.append("searchManufNumber", _search->text());

  itemFillList = mql.toQuery(params);
  _itemsrc->populate(itemFillList, true);
}

int itemSourceSearch::itemsrcId()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
  {
    if(item->altId() == 1)
      return item->id();
  }
  return -1;
}

int itemSourceSearch::expcatId()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
  {
    if(item->altId() == 2)
      return item->id();
  }
  return -1;
}

QString itemSourceSearch::vendItemNumber()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(3);
  return QString();
}

QString itemSourceSearch::vendItemDescrip()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(4);
  return QString();
}

QString itemSourceSearch::manufName()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(5);
  return QString();
}

QString itemSourceSearch::manufItemNumber()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(6);
  return QString();
}

QString itemSourceSearch::manufItemDescrip()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(7);
  return QString();
}

