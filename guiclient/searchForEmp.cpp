/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "searchForEmp.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>
#include <openreports.h>

#include "employee.h"

#define DEBUG   false

// TODO: XDialog should have a default implementation that returns FALSE
bool searchForEmp::userHasPriv(const int pMode)
{
  if (DEBUG)
    qDebug("searchForEmp::userHasPriv(%d)", pMode);
  bool retval = _privileges->check("ViewEmployees") ||
                _privileges->check("MaintainEmployees");
  if (DEBUG)
    qDebug("searchForEmp::userHasPriv(%d) returning %d", pMode, retval);
  return retval;
}

// TODO: this code really belongs in XDialog
void searchForEmp::setVisible(bool visible)
{
  if (DEBUG)
    qDebug("searchForEmp::setVisible(%d) called",
           visible);
  if (! visible)
    XMainWindow::setVisible(false);

  else if (! userHasPriv())
  {
    systemError(this,
		tr("You do not have sufficient privilege to view this window"),
		__FILE__, __LINE__);
    close();
  }
  else
    XMainWindow::setVisible(true);
}

searchForEmp::searchForEmp(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_edit,         SIGNAL(clicked()),     this, SLOT(sEdit()));
  connect(_emp, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu *, QTreeWidgetItem *)));
  connect(_new,          SIGNAL(clicked()),     this, SLOT(sNew()));
  connect(_print,        SIGNAL(clicked()),     this, SLOT(sPrint()));
  connect(_searchCode,   SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDept,   SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchMgr,    SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchNumber, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchShift,  SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_search,       SIGNAL(lostFocus()),   this, SLOT(sFillList()));
  connect(_showInactive, SIGNAL(clicked()),     this, SLOT(sFillList()));
  connect(_view,         SIGNAL(clicked()),     this, SLOT(sView()));

  _emp->addColumn(tr("Code"),   _itemColumn, Qt::AlignLeft, true, "emp_code");
  _emp->addColumn(tr("Number"),          -1, Qt::AlignLeft, true, "emp_number");
  _emp->addColumn(tr("Manager"),_itemColumn, Qt::AlignLeft, true, "mgr_code");
  _emp->addColumn(tr("Dept."),  _itemColumn, Qt::AlignLeft, true, "dept_number");
  _emp->addColumn(tr("Shift"),  _itemColumn, Qt::AlignLeft, true, "shift_number");

  if (_privileges->check("MaintainEmployees"))
  {
    _new->setEnabled(true);
    connect(_emp, SIGNAL(valid(bool)),       _edit, SLOT(setEnabled(bool)));
    connect(_emp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    connect(_emp, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), SLOT(sFillList()));

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _searchCode->setChecked(true);
    _searchDept->setChecked(true);
    _searchMgr->setChecked(true);
    _searchNumber->setChecked(true);
    _searchShift->setChecked(true);
  }

  _search->setFocus();
}

searchForEmp::~searchForEmp()
{
  // no need to delete child widgets, Qt does it all for us
}

void searchForEmp::languageChange()
{
  retranslateUi(this);
}

void searchForEmp::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("EmployeeList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void searchForEmp::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  pMenu->setItemEnabled(menuItem, _privileges->check("MaintainEmployees"));

  if (((XTreeWidgetItem *)pSelected)->altId() > 0)
  {
    pMenu->insertSeparator();
    menuItem = pMenu->insertItem(tr("View Manager..."), this, SLOT(sViewMgr()), 0);
    menuItem = pMenu->insertItem(tr("Edit Manager..."), this, SLOT(sEditMgr()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainEmployees"));
  }
}

void searchForEmp::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("emp_id", _emp->id());

  employee newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void searchForEmp::sEditMgr()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("emp_id", _emp->altId());

  employee newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void searchForEmp::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  employee newdlg(this);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void searchForEmp::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("emp_id", _emp->id());

  employee newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void searchForEmp::sViewMgr()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("emp_id", _emp->altId());

  employee newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

bool searchForEmp::setParams(ParameterList &pParams)
{
  pParams.append("searchString", _search->text().upper());

  if (_searchCode->isChecked())    pParams.append("searchCode");
  if (_searchDept->isChecked())    pParams.append("searchDept");
  if (_searchMgr->isChecked())     pParams.append("searchMgr");
  if (_searchNumber->isChecked())  pParams.append("searchNumber");
  if (_searchShift->isChecked())   pParams.append("searchShift");
  if (!_showInactive->isChecked()) pParams.append("activeOnly");

  return true;
}

void searchForEmp::sFillList()
{
  QString sql("SELECT e.emp_id, m.emp_id, e.emp_code, e.emp_number,"
              "       m.emp_code AS mgr_code, dept_number, shift_number "
              "FROM emp e LEFT OUTER JOIN"
              "     emp m ON (e.emp_mgr_emp_id=m.emp_id) LEFT OUTER JOIN"
              "     shift ON (e.emp_shift_id=shift_id) LEFT OUTER JOIN"
              "     dept  ON (e.emp_dept_id=dept_id) "
              "WHERE ((LENGTH(TRIM(<? value(\"searchString\") ?>)) = 0)"
              "<? if exists(\"searchCode\") ?>"
              "   OR (e.emp_code ~* <? value(\"searchString\") ?>)"
              "<? endif ?>"
              "<? if exists(\"searchDept\") ?>"
              "   OR (dept_number ~* <? value(\"searchString\") ?>)"
              "   OR (dept_name   ~* <? value(\"searchString\") ?>)"
              "<? endif ?>"
              "<? if exists(\"searchMgr\") ?>"
              "   OR (m.emp_code   ~* <? value(\"searchString\") ?>)"
              "   OR (m.emp_number ~* <? value(\"searchString\") ?>)"
              "<? endif ?>"
              "<? if exists(\"searchNumber\") ?>"
              "   OR (e.emp_number ~* <? value(\"searchString\") ?>)"
              "<? endif ?>"
              "<? if exists(\"searchShift\") ?>"
              "   OR (shift_number ~* <? value(\"searchString\") ?>)"
              "   OR (shift_name   ~* <? value(\"searchString\") ?>)"
              "<? endif ?>"
              "      )"  // end searchString
              "<? if exists(\"activeOnly\") ?>"
              "   AND e.emp_active "
              "<? endif ?>"
              "ORDER BY emp_code");

  MetaSQLQuery mql(sql);
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _emp->populate(q, true);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
