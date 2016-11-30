/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "account1099.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

account1099::account1099(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close,   SIGNAL(clicked()), this, SLOT(close()));
  connect(_query,   SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_addmisc, SIGNAL(clicked()), this, SLOT(sAddMisc()));
  connect(_addrent, SIGNAL(clicked()), this, SLOT(sAddRent()));
  connect(_del1099, SIGNAL(clicked()), this, SLOT(sRemove()));

  _list->addColumn(tr("Number"),          _orderColumn,  Qt::AlignLeft,     true,  "accnt_name"   );
  _list->addColumn(tr("Description"),     _orderColumn,  Qt::AlignLeft,     true,  "accnt_descrip"   );
  _list->addColumn(tr("Include in 1099"), _orderColumn,  Qt::AlignCenter,   true,  "accnt_1099_form_type"   );
  
  sFillList();
}

account1099::~account1099()
{
    // no need to delete child widgets, Qt does it all for us
}

void account1099::languageChange()
{
    retranslateUi(this);
}

void account1099::sFillList()
{
  ParameterList params;
  setParams(params);
  
  MetaSQLQuery mql = mqlLoad("1099", "accounts");
  XSqlQuery dspFillList = mql.toQuery(params);
  _list->populate(dspFillList);
}

void account1099::setParams(ParameterList & params)
{
  params.append("accnt_id", _list->id());
}

void account1099::sAddMisc()
{
  ParameterList params;
  setParams(params);
  params.append("box", "MISC");
  XSqlQuery qry;
  MetaSQLQuery mql = mqlLoad("1099", "addaccount");
  qry = mql.toQuery(params);
  sFillList();
}

void account1099::sAddRent()
{
  ParameterList params;
  setParams(params);
  params.append("box", "RENT");
  XSqlQuery qry;
  MetaSQLQuery mql = mqlLoad("1099", "addaccount");
  qry = mql.toQuery(params);
  sFillList();
}

void account1099::sRemove()
{
  ParameterList params;
  setParams(params);
  XSqlQuery qry;
  MetaSQLQuery mql = mqlLoad("1099", "removeaccount");
  qry = mql.toQuery(params);
  sFillList();
}
