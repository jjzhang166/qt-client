/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "subaccounts.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include "storedProcErrorLookup.h"
#include "subaccount.h"

subaccounts::subaccounts(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_subaccnt, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view,   SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainChartOfAccounts"))
  {
    connect(_subaccnt, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_subaccnt, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_subaccnt, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_subaccnt, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _subaccnt->addColumn(tr("Number"),_itemColumn, Qt::AlignCenter,true, "subaccnt_number");
  _subaccnt->addColumn(tr("Description"),    -1, Qt::AlignLeft,  true, "subaccnt_descrip");

  sFillList();
}

subaccounts::~subaccounts()
{
  // no need to delete child widgets, Qt does it all for us
}

void subaccounts::languageChange()
{
  retranslateUi(this);
}

void subaccounts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  subaccount *newdlg = new subaccount(this, "", true);
  newdlg->set(params);
  if (newdlg->exec() != XDialog::Rejected)
    sFillList();
}

void subaccounts::sEdit()
{
  ParameterList params;
  params.append("subaccnt_id", _subaccnt->id());
  params.append("mode", "edit");

  subaccount *newdlg = new subaccount(this, "", true);
  newdlg->set(params);
  if (newdlg->exec() != XDialog::Rejected)
    sFillList();
}

void subaccounts::sView()
{
  ParameterList params;
  params.append("subaccnt_id", _subaccnt->id());
  params.append("mode", "view");

  subaccount *newdlg = new subaccount(this, "", true);
  newdlg->set(params);
  newdlg->exec();
}

void subaccounts::sDelete()
{
  XSqlQuery subaccountsDelete;
  subaccountsDelete.prepare( "SELECT deleteSubaccount(:id) AS result;");
  subaccountsDelete.bindValue(":id", _subaccnt->id());
  subaccountsDelete.exec();
  if (subaccountsDelete.first())
  {
    int result = subaccountsDelete.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteSubaccount", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (subaccountsDelete.lastError().type() != QSqlError::NoError)
  {
    systemError(this, subaccountsDelete.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void subaccounts::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  pMenu->addAction("View...", this, SLOT(sView()));

  menuItem = pMenu->addAction("Edit...", this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainChartOfAccounts"));

  menuItem = pMenu->addAction("Delete...", this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainChartOfAccounts"));
}

void subaccounts::sFillList()
{
  XSqlQuery subaccountsFillList;
  subaccountsFillList.prepare( "SELECT * "
             "FROM subaccnt "
             "ORDER BY subaccnt_number;" );
  subaccountsFillList.exec();
  _subaccnt->populate(subaccountsFillList);
  if (subaccountsFillList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, subaccountsFillList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
