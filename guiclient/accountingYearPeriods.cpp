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

#include "accountingYearPeriods.h"

#include <QMenu>
#include <QSqlError>

#include <openreports.h>

#include "accountingYearPeriod.h"
#include "storedProcErrorLookup.h"

accountingYearPeriods::accountingYearPeriods(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    connect(_period, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_closePeriod, SIGNAL(clicked()), this, SLOT(sClosePeriod()));

    _period->addColumn(tr("Start"),  _dateColumn, Qt::AlignCenter, true, "yearperiod_start");
    _period->addColumn(tr("End"),    _dateColumn, Qt::AlignCenter, true, "yearperiod_end");
    _period->addColumn(tr("Closed"), -1         , Qt::AlignCenter, true, "closed");

    if (_privileges->check("MaintainAccountingPeriods"))
    {
      connect(_period, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_period, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_period, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
      _new->setEnabled(FALSE);
      connect(_period, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }

    sFillList();
}

accountingYearPeriods::~accountingYearPeriods()
{
    // no need to delete child widgets, Qt does it all for us
}

void accountingYearPeriods::languageChange()
{
    retranslateUi(this);
}

void accountingYearPeriods::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;
  int altId = ((XTreeWidgetItem *)pSelected)->altId();

  if (altId == 0)
  {
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainAccountingPeriods"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  if (altId == 0)
  {
    menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
    if (!_privileges->check("MaintainAccountingPeriods"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  pMenu->insertSeparator();

  if (altId == 0)
  {
    menuItem = pMenu->insertItem(tr("Close..."), this, SLOT(sClosePeriod()), 0);
#if 0
    if (!_privileges->check("MaintainItemMasters"))
      pMenu->setItemEnabled(menuItem, FALSE);
#endif
  }
  else if (altId == 1)
  {
    menuItem = pMenu->insertItem(tr("Open..."), this, SLOT(sOpenPeriod()), 0);
#if 0
    if (!_privileges->check("MaintainItemMasters"))
      pMenu->setItemEnabled(menuItem, FALSE);
#endif

#if 0
    if (!_privileges->check("MaintainItemMasters"))
      pMenu->setItemEnabled(menuItem, FALSE);
#endif
  }
}

void accountingYearPeriods::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  accountingYearPeriod newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void accountingYearPeriods::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("yearperiod_id", _period->id());

  accountingYearPeriod newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void accountingYearPeriods::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("yearperiod_id", _period->id());

  accountingYearPeriod newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void accountingYearPeriods::sDelete()
{
  q.prepare("SELECT deleteAccountingYearPeriod(:period_id) AS result;");
  q.bindValue(":period_id", _period->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteAccountingYearPeriod", result), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountingYearPeriods::sClosePeriod()
{
  q.prepare("SELECT closeAccountingYearPeriod(:period_id) AS result;");
  q.bindValue(":period_id", _period->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("closeAccountingYearPeriod", result), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountingYearPeriods::sOpenPeriod()
{
  q.prepare("SELECT openAccountingYearPeriod(:period_id) AS result;");
  q.bindValue(":period_id", _period->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this,
                  storedProcErrorLookup("openAccountingYearPeriod", result),
                  __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountingYearPeriods::sPrint()
{
  orReport report("AccountingYearPeriodsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void accountingYearPeriods::sFillList()
{
  _period->clear();
  _period->populate( "SELECT yearperiod_id,"
                     "       CASE WHEN (NOT yearperiod_closed) THEN 0"
                     "            ELSE 1"
                     "       END,"
                     "       *, "
                     "       formatBoolYN(yearperiod_closed) AS closed "
                     "FROM yearperiod "
                     "ORDER BY yearperiod_start;", TRUE );
}
