/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QMessageBox>

#include "itemGroups.h"
#include <metasql.h>
#include <parameter.h>
#include "mqlutil.h"
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
  connect(_showTopLevel, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

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
  
  connect(omfgThis, SIGNAL(itemGroupsUpdated(int, bool)), this, SLOT(sFillList()));

  sFillList();
}

itemGroups::~itemGroups()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemGroups::languageChange()
{
  retranslateUi(this);
}

ParameterList itemGroups::getParams()
{
  ParameterList p;

  if (_showTopLevel->isChecked())
    p.append("showTopLevel", true);

  return p;
}

void itemGroups::sDelete()
{
  if (QMessageBox::question(this, tr("Really delete this item group?"),
                            tr("<p>Are you sure you want to delete this item group?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::Yes)
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
    sFillList();
  }
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

void itemGroups::sFillList()
{
  MetaSQLQuery mql = mqlLoad("itemgroups", "detail");
  XSqlQuery igrp = mql.toQuery(getParams());
  _itemgrp->populate(igrp);
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

