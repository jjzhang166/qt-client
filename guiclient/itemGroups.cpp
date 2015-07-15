/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemGroups.h"

#include <parameter.h>
#include "itemGroup.h"
#include "guiclient.h"

itemGroups::itemGroups(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_itemgrp, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  _itemgrp->setRootIsDecorated(true);
  _itemgrp->addColumn(tr("Name"),            _itemColumn, Qt::AlignLeft, true, "name" );
  _itemgrp->addColumn(tr("Description"),     -1,          Qt::AlignLeft, true, "descrip" );
  _itemgrp->addColumn(tr("Product Catalog"), _itemColumn, Qt::AlignLeft, true, "catalog" );
  _itemgrp->setIndentation(10);
  _itemgrp->setPopulateLinear();
  
  connect(_itemgrp, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));

  if (_privileges->check("MaintainItemGroups"))
  {
    _new->setEnabled(true);
  }
  else
  {
    _new->setEnabled(false);
  }
  
  connect(omfgThis, SIGNAL(itemGroupsUpdated(int, bool)), this, SLOT(sFillList(int)));

  sFillList(-1);
}

itemGroups::~itemGroups()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemGroups::languageChange()
{
  retranslateUi(this);
}

void itemGroups::sDelete()
{
  XSqlQuery itemDelete;
  itemDelete.prepare( "DELETE FROM itemgrpitem "
             "WHERE (itemgrpitem_itemgrp_id=:itemgrp_id);"

             "DELETE FROM itemgrpitem "
             "WHERE ((itemgrpitem_item_id=:itemgrp_id) AND (itemgrpitem_item_type='G'));"
                                        
             "DELETE FROM itemgrp "
             "WHERE (itemgrp_id=:itemgrp_id);" );
  itemDelete.bindValue(":itemgrp_id", _itemgrp->id());
  itemDelete.exec();

  sFillList(-1);
}


void itemGroups::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  itemGroup *newdlg = new itemGroup();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemGroups::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemgrp_id", _itemgrp->id());

  itemGroup *newdlg = new itemGroup();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemGroups::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemgrp_id", _itemgrp->id());

  itemGroup *newdlg = new itemGroup();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemGroups::sFillList(int)
{
  _itemgrp->populate("WITH RECURSIVE indentedgroups(id, name, descrip, catalog, depth, path, cycle) AS ( "
                     "SELECT itemgrp_id AS id, "
                     "       itemgrp_name AS name, "
                     "       itemgrp_descrip AS descrip, "
                     "       itemgrp_catalog AS catalog, "
                     "       0 AS depth, array[itemgrp_id] AS path, false AS cycle "
                     "FROM itemgrp "
                     "UNION ALL "
                     "SELECT itemgrp_id AS id, "
                     "       CASE itemgrpitem_item_type WHEN 'I' THEN item_number ELSE itemgrp_name END AS name, "
                     "       CASE itemgrpitem_item_type WHEN 'I' THEN item_descrip1 ELSE itemgrp_descrip END AS descrip, "
                     "       NULL AS catalog, "
                     "       (depth+1) AS depth, (path || itemgrp_id) AS path, (itemgrp_id = any(path)) AS cycle "
                     "FROM indentedgroups JOIN itemgrpitem ON (itemgrpitem_itemgrp_id=id) "
                     "  LEFT OUTER JOIN item ON (item_id=itemgrpitem_item_id AND itemgrpitem_item_type='I') "
                     "  LEFT OUTER JOIN itemgrp ON (itemgrp_id=itemgrpitem_item_id AND itemgrpitem_item_type='G') "
                     "WHERE (NOT cycle)"
                     ") "
                     "SELECT id, name, descrip, catalog, depth AS xtindentrole, path, cycle "
                     "FROM indentedgroups "
                     "ORDER BY path, name;");
}

void itemGroups::sHandleButtons()
{
  if (_itemgrp->id() > 0)
  {
    _view->setEnabled(true);
    if (_privileges->check("MaintainItemGroups"))
    {
      _edit->setEnabled(true);
      _delete->setEnabled(true);
    }
    else
    {
      _edit->setEnabled(false);
      _delete->setEnabled(false);
    }
  }
  else
  {
    _view->setEnabled(false);
    _edit->setEnabled(false);
    _delete->setEnabled(false);
  }
}

