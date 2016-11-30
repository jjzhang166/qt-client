/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTax1099.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <account1099.h>
#include <openreports.h>

dspTax1099::dspTax1099(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_setaccnt, SIGNAL(clicked()), this, SLOT(sSetAccount()));
  connect(_chg1099, SIGNAL(clicked()), this, SLOT(sChg1099()));
  connect(_p1099, SIGNAL(clicked()), this, SLOT(sPrint1099()));
  connect(_p1096, SIGNAL(clicked()), this, SLOT(sPrint1096()));

  _list->addColumn(tr("Number"),         _orderColumn,  Qt::AlignLeft,     true,  "vend_number"   );
  _list->addColumn(tr("Name"),           _orderColumn,  Qt::AlignLeft,     true,  "vend_name"   );
  _list->addColumn(tr("Tax Id"),         _orderColumn,  Qt::AlignLeft,     true,  "taxreg_number"   );
  _list->addColumn(tr("Amount"),         _moneyColumn,  Qt::AlignRight,    true,  "checkamount" );
  _list->addColumn(tr("Rent"),           _moneyColumn,  Qt::AlignRight,    true,  "rentamt" );
  _list->addColumn(tr("Misc."),          _moneyColumn,  Qt::AlignRight,    true,  "miscamt" );
  _list->addColumn(tr("Address"),        -1,            Qt::AlignLeft,     true,  "vaddr" );
  _list->addColumn(tr("Gets 1099"),      _orderColumn,  Qt::AlignCenter,   true,  "v1099" );
  _list->addColumn(tr("Check Date"),     _orderColumn,  Qt::AlignCenter,   true,  "checkhead_checkdate" );
  _list->addColumn(tr("Check Number"),   _orderColumn,  Qt::AlignLeft,     true,  "checkhead_number" );
  _list->addColumn(tr("Expense Number"), _orderColumn,  Qt::AlignLeft,     true,  "accountdescrip" );
}

dspTax1099::~dspTax1099()
{
    // no need to delete child widgets, Qt does it all for us
}

void dspTax1099::languageChange()
{
    retranslateUi(this);
}

void dspTax1099::sFillList()
{
  ParameterList params;
  setParams(params);
  
  if (_sumall->isChecked() || _sum1099->isChecked())
  {
    MetaSQLQuery mql = mqlLoad("1099", "summary");
    XSqlQuery dspFillList = mql.toQuery(params);
    _list->populate(dspFillList);
  }
  else
  {
    MetaSQLQuery mql = mqlLoad("1099", "detail");
    XSqlQuery dspFillList = mql.toQuery(params);
    _list->populate(dspFillList);
  }
}

void dspTax1099::setParams(ParameterList & params)
{
  params.append("fromdate", _fromdate->date());
  params.append("todate", _todate->date());
  params.append("vend_id", _list->id());
  if (_sumall->isChecked())
  {
    params.append("showxxx", "sall");
    _p1099->hide();
    _p1096->hide();
  }
  else if (_sum1099->isChecked())
  {
    params.append("showall", "s1099");
    _p1099->show();
    _p1096->show();
  }
  else if (_detailall->isChecked())
  {
    params.append("showxxx", "dall");
    _p1099->hide();
    _p1096->hide();
  }
  else if (_detail1099->isChecked())
  {
    params.append("showall", "d1099");
    _p1099->hide();
    _p1096->hide();
  }
  else
  {
    params.append("showhat", "dall");
    _p1099->hide();
    _p1096->hide();
  }
}

void dspTax1099::sChg1099()
{
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql = mqlLoad("1099", "setvendor");
  XSqlQuery qry = mql.toQuery(params);
  sFillList();
}

void dspTax1099::sSetAccount()
{
  ParameterList params;
  setParams(params);
  account1099 *newdlg = new account1099();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTax1099::sPrint()
{
  orReport report("1099Info");
  if (report.isValid())
  {
    ParameterList params;
    setParams(params);
    report.setParamList(params);
    report.print();
  }
  else
    report.reportError(this);
}

void dspTax1099::sPrint1099()
{
  orReport report("1099Form");
  if (report.isValid())
  {
    ParameterList params;
    setParams(params);
    report.setParamList(params);
    report.print();
  }
  else
    report.reportError(this);
}

void dspTax1099::sPrint1096()
{
  orReport report("1096Form");
  if (report.isValid())
  {
    ParameterList params;
    setParams(params);
    report.setParamList(params);
    report.print();
  }
  else
    report.reportError(this);
}

