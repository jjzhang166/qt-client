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

#include "accountingPeriods.h"

#include <QMenu>
#include <QSqlError>

#include <openreports.h>

#include "accountingPeriod.h"
#include "storedProcErrorLookup.h"
#include "unpostedGLTransactions.h"

accountingPeriods::accountingPeriods(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_period, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem *)));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_closePeriod, SIGNAL(clicked()), this, SLOT(sClosePeriod()));
  connect(_freezePeriod, SIGNAL(clicked()), this, SLOT(sFreezePeriod()));
  
  _period->addColumn(tr("Name"),            -1, Qt::AlignLeft,   true, "period_name");
  _period->addColumn(tr("Start"),  _dateColumn, Qt::AlignCenter, true, "period_start");
  _period->addColumn(tr("End"),    _dateColumn, Qt::AlignCenter, true, "period_end");
  _period->addColumn(tr("Qtr"),  _statusColumn, Qt::AlignCenter, true, "qtr");
  _period->addColumn(tr("Year"),   _dateColumn, Qt::AlignCenter, true, "year");
  _period->addColumn(tr("Closed"), _ynColumn+3, Qt::AlignCenter, true, "closed");
  _period->addColumn(tr("Frozen"), _ynColumn+3, Qt::AlignCenter, true, "frozen");

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

  connect(_period, SIGNAL(valid(bool)), _closePeriod, SLOT(setEnabled(bool)));
  connect(_period, SIGNAL(valid(bool)), _freezePeriod, SLOT(setEnabled(bool)));

  sFillList();
}

accountingPeriods::~accountingPeriods()
{
  // no need to delete child widgets, Qt does it all for us
}

void accountingPeriods::languageChange()
{
  retranslateUi(this);
}

void accountingPeriods::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
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

    menuItem = pMenu->insertItem(tr("Freeze..."), this, SLOT(sFreezePeriod()), 0);
#if 0
    if (!_privileges->check("MaintainItemMasters"))
      pMenu->setItemEnabled(menuItem, FALSE);
#endif
  }
  else if (altId == 2)
  {
    menuItem = pMenu->insertItem(tr("Thaw..."), this, SLOT(sThawPeriod()), 0);
#if 0
    if (!_privileges->check("MaintainItemMasters"))
      pMenu->setItemEnabled(menuItem, FALSE);
#endif
  }
}

void accountingPeriods::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  accountingPeriod newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void accountingPeriods::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("period_id", _period->id());

  accountingPeriod newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void accountingPeriods::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("period_id", _period->id());

  accountingPeriod newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void accountingPeriods::sDelete()
{
  q.prepare("SELECT deleteAccountingPeriod(:period_id) AS result;");
  q.bindValue(":period_id", _period->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteAccountingPeriod",
						       result));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void accountingPeriods::sClosePeriod()
{
  q.prepare("SELECT closeAccountingPeriod(:period_id) AS result;");
  q.bindValue(":period_id", _period->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("closeAccountingPeriod", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void accountingPeriods::sOpenPeriod()
{
  bool reallyOpen = false;

  q.prepare("SELECT COUNT(gltrans_sequence) AS count "
            "FROM gltrans, period "
            "WHERE ( (NOT gltrans_posted) "
            "AND (gltrans_date BETWEEN period_start AND period_end) "
            "AND (period_id=:period_id) );");
  q.bindValue(":period_id", _period->id());
  q.exec();
  if (q.first())
  {
    if (q.value("count").toInt() <= 0)
      reallyOpen = true;
    else
    {
      ParameterList params;

      unpostedGLTransactions newdlg(this, "", true);
      params.append("period_id", _period->id());
      newdlg.set(params);

      reallyOpen = (newdlg.exec() == XDialog::Accepted);
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (reallyOpen)
  {
    q.prepare("SELECT openAccountingPeriod(:period_id) AS result;");
    q.bindValue(":period_id", _period->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("openAccountingPeriod", result),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void accountingPeriods::sFreezePeriod()
{
  q.prepare("SELECT freezeAccountingPeriod(:period_id) AS result;");
  q.bindValue(":period_id", _period->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("freezeAccountingPeriod", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void accountingPeriods::sThawPeriod()
{
  q.prepare("SELECT thawAccountingPeriod(:period_id) AS result;");
  q.bindValue(":period_id", _period->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("thawAccountingPeriod", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void accountingPeriods::sPrint()
{
  orReport report("AccountingPeriodsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void accountingPeriods::sFillList()
{
  _period->clear();
  _period->populate( "SELECT period_id,"
                     "       CASE WHEN (NOT period_closed) THEN 0"
                     "            WHEN ( (period_closed) AND (NOT period_freeze) ) THEN 1"
                     "            WHEN ( (period_closed) AND (period_freeze) ) THEN 2"
                     "            ELSE 0"
                     "       END,"
                     "       period_name,"
                     "       period_start, period_end,"
                     "       COALESCE(to_char(period_quarter,'9'),'?') AS qtr,"
                     "       COALESCE(to_char(EXTRACT(year FROM yearperiod_end),'9999'),'?') AS year,"
                     "       formatBoolYN(period_closed) AS closed,"
                     "       formatBoolYN(period_freeze) AS frozen "
                     "  FROM period LEFT OUTER JOIN yearperiod "
                     "    ON (period_yearperiod_id=yearperiod_id) "
                     " ORDER BY period_start;", TRUE );
}

