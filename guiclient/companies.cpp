/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "companies.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <parameter.h>

#include "company.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

companies::companies(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_company, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_view,   SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainChartOfAccounts"))
  {
    connect(_company, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_company, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_company, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_company, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _company->addColumn(tr("Number"), _itemColumn, Qt::AlignCenter,true, "company_number" );
  _company->addColumn(tr("Description"),     -1, Qt::AlignLeft,  true, "company_descrip");

  sFillList();
}

companies::~companies()
{
  // no need to delete child widgets, Qt does it all for us
}

void companies::languageChange()
{
  retranslateUi(this);
}

void companies::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  company *newdlg = new company(this, "", true);
  newdlg->set(params);
  if (newdlg->exec() != XDialog::Rejected)
    sFillList();
}

void companies::sEdit()
{
  ParameterList params;
  params.append("company_id", _company->id());
  params.append("mode", "edit");

  company *newdlg = new company(this, "", true);
  newdlg->set(params);
  if (newdlg->exec() != XDialog::Rejected)
    sFillList();
}

void companies::sView()
{
  ParameterList params;
  params.append("company_id", _company->id());
  params.append("mode", "view");

  company *newdlg = new company(this, "", true);
  newdlg->set(params);
  newdlg->exec();
}

void companies::sDelete()
{
  XSqlQuery companiesDelete;
  companiesDelete.prepare("SELECT deleteCompany(:id) AS result;");
  companiesDelete.bindValue(":id", _company->id());
  companiesDelete.exec();
  if (companiesDelete.first())
  {
    int result = companiesDelete.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Company"),
                             storedProcErrorLookup("deleteCompany", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Company"),
                                companiesDelete, __FILE__, __LINE__))
  {
    return;
  }

  sFillList();
}

void companies::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainChartOfAccounts"));

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainChartOfAccounts"));
}

void companies::sFillList()
{
  XSqlQuery companiesFillList;
  companiesFillList.prepare( "SELECT * "
             "FROM company "
             "ORDER BY company_number;" );
  companiesFillList.exec();
  _company->populate(companiesFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Company Information"),
                                companiesFillList, __FILE__, __LINE__))
  {
    return;
  }
}
