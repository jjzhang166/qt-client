/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUserPrivileges.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include "parameterwidget.h"
#include "xtreewidget.h"
#include "user.h"
#include "group.h"

dspUserPrivileges::dspUserPrivileges(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : display(parent, name, fl)
{
  setWindowTitle(tr("User Privilege Audit"));
  setReportName("UserPrivileges");
  setMetaSQLOptions("permissions", "privileges");
  setParameterWidgetVisible(true);
  setUseAltId(true);
  _printing = false;

  _grpSql = "SELECT grp_id, grp_name FROM grp ORDER BY grp_name;"; 

  parameterWidget()->append(tr("Username"), "username", ParameterWidget::User);
  parameterWidget()->appendComboBox(tr("Role"), "role", _grpSql);
  parameterWidget()->append(tr("Privilege"), "privilege", ParameterWidget::Text);

  parameterWidget()->applyDefaultFilterSet();

  list()->addColumn(tr("Username"),    100,  Qt::AlignLeft, true,  "priv_username" );
  list()->addColumn(tr("Role"),        100,  Qt::AlignLeft, true,  "grp_name"      );
  list()->addColumn(tr("Module"),       -1,  Qt::AlignLeft, true,  "priv_module"   );
  list()->addColumn(tr("Privilege Name"),  -1, Qt::AlignLeft,  true,  "priv_name" );
  list()->addColumn(tr("Description"),     -1, Qt::AlignLeft,  true,  "priv_descrip" );
}

bool dspUserPrivileges::setParams(ParameterList & params)
{
  if (!display::setParams(params))
    return false;

  if(_printing)
    params.append("print");

  return true;
}

void dspUserPrivileges::sPrint()
{
  _printing = true;
  display::sPrint();
  _printing = false;
}

void dspUserPrivileges::sPreview()
{
  _printing = true;
  display::sPreview();
  _printing = false;
}

void dspUserPrivileges::sEditUser()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("username", list()->rawValue("priv_username"));

  user newdlg(this);
  newdlg.set(params);

  newdlg.exec();
  sFillList();
}

void dspUserPrivileges::sEditRole()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("grp_id", list()->altId());

  group newdlg(this);
  newdlg.set(params);

  newdlg.exec();
}

void dspUserPrivileges::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int pColumn)
{
  Q_UNUSED(pColumn);
  QAction *menuItem;

  menuItem = pMenu->addAction("Maintain User...", this, SLOT(sEditUser()));
  if (!_privileges->check("MaintainUsers"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction("Maintain Role...", this, SLOT(sEditRole()));
  if (!_privileges->check("MaintainGroups") || list()->altId() == -1)
    menuItem->setEnabled(false);

}

