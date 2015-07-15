/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "saleTypes.h"

#include <QVariant>
#include <QMessageBox>
#include <QMenu>
#include <metasql.h>
#include "mqlutil.h"
#include <parameter.h>
#include <openreports.h>
#include "saleType.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"
#include "guiclient.h"

saleTypes::saleTypes(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_new,      SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit,     SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view,     SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete,   SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_print,    SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close,    SIGNAL(clicked()), this, SLOT(close()));
  connect(_saletype, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  _saletype->addColumn(tr("Code"),       _itemColumn, Qt::AlignLeft,   true,  "saletype_code" );
  _saletype->addColumn(tr("Active"),     _ynColumn,   Qt::AlignCenter, true,  "saletype_active" );
  _saletype->addColumn(tr("Default"),    _ynColumn,   Qt::AlignCenter, true,  "saletype_default" );
  _saletype->addColumn(tr("Description"), -1,         Qt::AlignLeft,   true,  "saletype_descr" );

  if (_privileges->check("MaintainSaleTypes"))
  {
    connect(_saletype, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_saletype, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_saletype, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_saletype, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList(-1);
}

saleTypes::~saleTypes()
{
  // no need to delete child widgets, Qt does it all for us
}

void saleTypes::languageChange()
{
  retranslateUi(this);
}

void saleTypes::sDelete()
{
  MetaSQLQuery mql = mqlLoad("saletype", "table");
  ParameterList params;
  params.append("DeleteMode");
  params.append("saletype_id", _saletype->id());
  XSqlQuery saleTypeDelete = mql.toQuery(params);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error deleting Sale Type"),
                           saleTypeDelete, __FILE__, __LINE__))
    return;
  sFillList(-1);
}

void saleTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  saleType newdlg(this, "", true);
  newdlg.set(params);
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void saleTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("saletype_id", _saletype->id());

  saleType newdlg(this, "", true);
  newdlg.set(params);
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void saleTypes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("saletype_id", _saletype->id());

  saleType newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void saleTypes::sFillList(int pId)
{
  MetaSQLQuery mql = mqlLoad("saletype", "table");
  ParameterList params;
  params.append("ViewMode");
  XSqlQuery saleTypePopulate = mql.toQuery(params);
  _saletype->populate( saleTypePopulate, pId  );
}

void saleTypes::sPrint()
{
  orReport report("SaleTypesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

