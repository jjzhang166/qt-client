/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "accountingYearPeriods.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>

#include <openreports.h>

#include "accountingYearPeriod.h"
#include "storedProcErrorLookup.h"

accountingYearPeriods::accountingYearPeriods(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

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
  QAction *menuItem;
  int altId = ((XTreeWidgetItem *)pSelected)->altId();

  if (altId == 0)
  {
    menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainAccountingPeriods"));
  }

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  if (altId == 0)
  {
    menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
    menuItem->setEnabled(_privileges->check("MaintainAccountingPeriods"));
  }

  pMenu->addSeparator();

  if (altId == 0)
  {
    menuItem = pMenu->addAction(tr("Close..."), this, SLOT(sClosePeriod()));
  }
  else if (altId == 1)
  {
    menuItem = pMenu->addAction(tr("Open..."), this, SLOT(sOpenPeriod()));
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
  XSqlQuery deleteAccountingYear;
  deleteAccountingYear.prepare("SELECT deleteAccountingYearPeriod(:period_id) AS result;");
  deleteAccountingYear.bindValue(":period_id", _period->id());
  deleteAccountingYear.exec();
  if (deleteAccountingYear.first())
  {
    int result = deleteAccountingYear.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteAccountingYearPeriod", result), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (deleteAccountingYear.lastError().type() != QSqlError::NoError)
  {
    systemError(this, deleteAccountingYear.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountingYearPeriods::sClosePeriod()
{
  XSqlQuery closeAccountingYear;
  closeAccountingYear.prepare("SELECT closeAccountingYearPeriod(:period_id) AS result;");
  closeAccountingYear.bindValue(":period_id", _period->id());
  closeAccountingYear.exec();
  if (closeAccountingYear.first())
  {
    int result = closeAccountingYear.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("closeAccountingYearPeriod", result), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (closeAccountingYear.lastError().type() != QSqlError::NoError)
  {
    systemError(this, closeAccountingYear.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountingYearPeriods::sOpenPeriod()
{
  XSqlQuery openAccountingYear;
  openAccountingYear.prepare("SELECT openAccountingYearPeriod(:period_id) AS result;");
  openAccountingYear.bindValue(":period_id", _period->id());
  openAccountingYear.exec();
  if (openAccountingYear.first())
  {
    int result = openAccountingYear.value("result").toInt();
    if (result < 0)
    {
      systemError(this,
                  storedProcErrorLookup("openAccountingYearPeriod", result),
                  __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (openAccountingYear.lastError().type() != QSqlError::NoError)
  {
    systemError(this, openAccountingYear.lastError().databaseText(), __FILE__, __LINE__);
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
  _period->populate("SELECT yearperiod_id,"
                    "       CASE WHEN (NOT yearperiod_closed) THEN 0"
                    "            ELSE 1"
                    "       END,"
                    "       yearperiod_start, yearperiod_end,"
                    "       formatBoolYN(yearperiod_closed) AS closed"
                    "  FROM yearperiod "
                    " ORDER BY yearperiod_start;", TRUE );
}
