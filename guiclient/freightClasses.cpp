/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "freightClasses.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "freightClass.h"
#include "storedProcErrorLookup.h"

freightClasses::freightClasses(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_deleteUnused, SIGNAL(clicked()), this, SLOT(sDeleteUnused()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainFreightClasses"))
  {
    connect(_freightClass, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_freightClass, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_freightClass, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_freightClass, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(false);
    _deleteUnused->setEnabled(false);
  }

  _freightClass->addColumn(tr("Code"),           70, Qt::AlignLeft, true, "freightclass_code");
  _freightClass->addColumn(tr("Description"),    -1, Qt::AlignLeft, true, "freightclass_descrip");

  sFillList(-1);
}

freightClasses::~freightClasses()
{
  // no need to delete child widgets, Qt does it all for us
}

void freightClasses::languageChange()
{
  retranslateUi(this);
}

void freightClasses::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  freightClass newdlg(this, "", true);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void freightClasses::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("freightclass_id", _freightClass->id());

  freightClass newdlg(this, "", true);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void freightClasses::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("freightclass_id", _freightClass->id());

  freightClass newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void freightClasses::sDelete()
{
  XSqlQuery freightDelete;
  freightDelete.prepare("SELECT deleteFreightClass(:freightclass_id) AS result;");
  freightDelete.bindValue(":freightclass_id", _freightClass->id());
  freightDelete.exec();
  if (freightDelete.first())
  {
    int result = freightDelete.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteFreightClass", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (freightDelete.lastError().type() != QSqlError::NoError)
  {
    systemError(this, freightDelete.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList(-1);
}

void freightClasses::sPrint()
{
  orReport report("FreightClassesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void freightClasses::sDeleteUnused()
{
  XSqlQuery freightDeleteUnused;
  if ( QMessageBox::warning( this, tr("Delete Unused Freight Classes"),
                             tr("<p>Are you sure that you wish to delete all "
                                "unused Freight Classes?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    freightDeleteUnused.exec("SELECT deleteUnusedFreightClasses() AS result;");
    if (freightDeleteUnused.first())
    {
      int result = freightDeleteUnused.value("result").toInt();
      if (result < 0)
      {
        systemError(this,
                    storedProcErrorLookup("deleteUnusedFreightClasses", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (freightDeleteUnused.lastError().type() != QSqlError::NoError)
    {
      systemError(this, freightDeleteUnused.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList(-1);
  }
}

void freightClasses::sFillList(int pId)
{
  _freightClass->populate( "SELECT freightclass_id, freightclass_code, freightclass_descrip "
                        "FROM freightclass "
                        "ORDER BY freightclass_code;", pId  );
}
