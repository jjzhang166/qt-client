/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to y6ou under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "poTypes.h"

#include <QSqlDriver>

#include <openreports.h>
#include <parameter.h>

#include "errorReporter.h"
#include "guiclient.h"
#include "poType.h"

poTypes::poTypes(QWidget *parent, const char *name, Qt::WindowFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_deleteunused, SIGNAL(clicked()), this, SLOT(sDeleteUnused()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_potype, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));

  _potype->addColumn(tr("Code"),        -1, Qt::AlignLeft, true, "potype_code" );
  _potype->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "potype_descr" );
  _potype->addColumn(tr("Active"),      -1, Qt::AlignLeft, true, "potype_active" );
  _potype->addColumn(tr("Default"),     -1, Qt::AlignLeft, true, "potype_default" );
  
  if (_privileges->check("MaintainPurchaseTypes"))
  {
    connect(_potype, SIGNAL(valid(bool)),         _edit, SLOT(setEnabled(bool)));
    connect(_potype, SIGNAL(valid(bool)),       _delete, SLOT(setEnabled(bool)));
    connect(_potype, SIGNAL(itemSelected(int)),   _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    _deleteunused->setEnabled(false);
  }

  _edit->setEnabled(false);
  _delete->setEnabled(false);

  sFillList();

  QSqlDatabase db = QSqlDatabase::database();
  if (! db.driver()->subscribedToNotifications().contains("potype"))
    db.driver()->subscribeToNotification("potype");
  connect(db.driver(), SIGNAL(notification(const QString&)),
          this,        SLOT(sNotified(const QString&)));
}

poTypes::~poTypes()
{
  // no need to delete child widgets, Qt does it all for us
}

void poTypes::languageChange()
{
  retranslateUi(this);
}

void poTypes::sDelete()
{
  XSqlQuery typeDelete;
  typeDelete.prepare( "SELECT deletepotype(:potype_id);" );
  typeDelete.bindValue(":potype_id", _potype->id());
  typeDelete.exec();

  ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Purchase Order Type"),
                            typeDelete, __FILE__, __LINE__);

  sFillList();
}

void poTypes::sDeleteUnused()
{
  XSqlQuery typeDelete;
  typeDelete.prepare( "SELECT deleteunusedpotypes();" );
  typeDelete.exec();

  ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Purchase Order Types"),
                            typeDelete, __FILE__, __LINE__);

  sFillList();
}

void poTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  poType newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void poTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("potype_id", _potype->id());

  poType newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void poTypes::sPrint()
{
  orReport report("PurchaseOrderTypes");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void poTypes::sFillList()
{
  _potype->populate("SELECT potype_id, *"
                    "  FROM potype"
                    " ORDER BY potype_code;");
}

void poTypes::sNotified(const QString &notice)
{
  if (notice == "potype")
    sFillList();
}
