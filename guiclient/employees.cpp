/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "employees.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "employee.h"
#include "errorReporter.h"
#include "storedProcErrorLookup.h"
#include "parameterwidget.h"

employees::employees(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "employees", fl)
{
  setWindowTitle(tr("Employees"));
  setReportName("EmployeeList");
  setMetaSQLOptions("employees", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  QString qryEmpGrp = QString( "SELECT  empgrp_id, empgrp_name FROM empgrp;");

  parameterWidget()->appendComboBox(tr("Employee Group"), "emp_group", qryEmpGrp);
  parameterWidget()->append(tr("Show Active Only"), "activeOnly", ParameterWidget::Exists);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);
  parameterWidget()->applyDefaultFilterSet();

  connect(omfgThis, SIGNAL(employeeUpdated(int)),     this, SLOT(sFillList()));

  list()->addColumn(tr("Site"),   _whsColumn,  Qt::AlignLeft, true, "warehous_code");
  list()->addColumn(tr("Active"), _ynColumn,   Qt::AlignLeft, true, "emp_active");
  list()->addColumn(tr("Code"),   _itemColumn, Qt::AlignLeft, true, "emp_code");
  list()->addColumn(tr("Number"), -1,          Qt::AlignLeft, true, "emp_number");
  list()->addColumn(tr("First"),  _itemColumn, Qt::AlignLeft, true, "cntct_first_name");
  list()->addColumn(tr("Last"),   _itemColumn, Qt::AlignLeft, true, "cntct_last_name");

  setupCharacteristics("EMP");
  parameterWidget()->applyDefaultFilterSet();

  connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sOpen()));

  if (!_privileges->check("MaintainEmployees"))
    newAction()->setEnabled(false);
}

void employees::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  employee* newdlg = new employee();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void employees::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("emp_id", list()->id());

  employee* newdlg = new employee();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void employees::sDelete()
{
  if (QMessageBox::question(this, tr("Delete?"),
                            tr("Are you sure you want to delete this Employee?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  XSqlQuery delq;
  delq.prepare("DELETE FROM charass WHERE charass_target_type = 'EMP' AND charass_target_id = :emp_id;"
               "DELETE FROM empgrpitem WHERE empgrpitem_emp_id = :emp_id;"
               "DELETE FROM emp WHERE emp_id = :emp_id;");
  delq.bindValue(":emp_id", list()->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error deleting Employee"),
                           delq, __FILE__, __LINE__))
    return;
  sFillList();
}

void employees::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("emp_id", list()->id());

  employee* newdlg = new employee();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void employees::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  bool editPriv =
      (_privileges->check("MaintainEmployees"));

  bool viewPriv =
      (_privileges->check("ViewEmployees"));

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(viewPriv);

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuItem->setEnabled(editPriv);
}

void employees::sOpen()
{
  bool editPriv =
      (_privileges->check("MaintainEmployees"));

  bool viewPriv =
      (_privileges->check("ViewEmployees"));

  if (editPriv)
    sEdit();
  else if (viewPriv)
    sView();
}
