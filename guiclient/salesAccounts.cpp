/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesAccounts.h"

#include <QMessageBox>
#include <QSqlError>

#include "mqlutil.h"
#include <metasql.h>
#include <parameter.h>
#include <openreports.h>

#include "salesAccount.h"
#include "guiclient.h"

salesAccounts::salesAccounts(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _salesaccnt->addColumn(tr("Site"),                      _whsColumn,  Qt::AlignCenter  , true, "warehouscode");
  _salesaccnt->addColumn(tr("Cust. Type"),                _itemColumn, Qt::AlignCenter  , true, "custtypecode");
  _salesaccnt->addColumn(tr("Shipping Zone"),             _itemColumn, Qt::AlignCenter  , true, "shipzonecode");
  _salesaccnt->addColumn(tr("Sale Type"),                 _itemColumn, Qt::AlignCenter  , true, "saletypecode");
  _salesaccnt->addColumn(tr("Prod. Cat."),                _itemColumn, Qt::AlignCenter  , true, "prodcatcode");
  _salesaccnt->addColumn(tr("Sales Accnt. #"),            _itemColumn, Qt::AlignCenter  , true, "salesaccount");
  _salesaccnt->addColumn(tr("Return Accnt. #"),           _itemColumn, Qt::AlignCenter  , true, "creditaccount");
  _salesaccnt->addColumn(tr("COS Accnt. #"),              _itemColumn, Qt::AlignCenter  , true, "cosaccount");
  _salesaccnt->addColumn(tr("Returns Accnt. #"),          _itemColumn, Qt::AlignCenter  , true, "returnsaccount");
  _salesaccnt->addColumn(tr("Cost of Returns Accnt. #"),  _itemColumn, Qt::AlignCenter  , true, "coraccount" );
  _salesaccnt->addColumn(tr("Cost of Warranty Accnt. #"), _itemColumn, Qt::AlignCenter  , true, "cowaccount" );

  if (! _metrics->boolean("EnableReturnAuth"))
  {
    _salesaccnt->hideColumn("returnsaccount");;
    _salesaccnt->hideColumn("coraccount");
    _salesaccnt->hideColumn("cowaccount");
  }

  if (_privileges->check("MaintainSalesAccount"))
  {
    connect(_salesaccnt, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_salesaccnt, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_salesaccnt, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_salesaccnt, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

salesAccounts::~salesAccounts()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesAccounts::languageChange()
{
  retranslateUi(this);
}

void salesAccounts::sPrint()
{
  orReport report("SalesAccountAssignmentsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void salesAccounts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  salesAccount newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesAccounts::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("salesaccnt_id", _salesaccnt->id());

  salesAccount newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesAccounts::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("salesaccnt_id", _salesaccnt->id());

  salesAccount newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void salesAccounts::sDelete()
{
  XSqlQuery salesDelete;
  salesDelete.prepare( "DELETE FROM salesaccnt "
             "WHERE (salesaccnt_id=:salesaccnt_id);" );
  salesDelete.bindValue(":salesaccnt_id", _salesaccnt->id());
  salesDelete.exec();
  if (salesDelete.lastError().type() != QSqlError::NoError)
  {
    systemError(this, salesDelete.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void salesAccounts::sFillList()
{
  MetaSQLQuery mql = mqlLoad("salesAccounts", "detail");

  ParameterList params;
  params.append("any", tr("Any"));
  params.append("notapplicable", tr("N/A"));

  XSqlQuery fillq = mql.toQuery(params);
  _salesaccnt->populate(fillq);
  if (fillq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, fillq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
