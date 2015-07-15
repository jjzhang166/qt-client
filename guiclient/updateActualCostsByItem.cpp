/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateActualCostsByItem.h"

#include <QVariant>
#include <QSqlError>

updateActualCostsByItem::updateActualCostsByItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_item, SIGNAL(valid(bool)), _update, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_selectAll, SIGNAL(clicked()), this, SLOT(sSelectAll()));
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));

  if (!_metrics->boolean("Routings"))
  {
    _directLabor->hide();
    _lowerDirectLabor->hide();
    _overhead->hide();
    _lowerOverhead->hide();
    _machOverhead->hide();
    _lowerMachOverhead->hide();
  }
  else if (_metrics->value("TrackMachineOverhead") != "M")
  {
    _machOverhead->setEnabled(false);
    _machOverhead->setChecked(true);
    _lowerMachOverhead->setEnabled(false);
    _lowerMachOverhead->setChecked(true);
  }

  _captive = false;
  _updateActual = true;
}

updateActualCostsByItem::~updateActualCostsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void updateActualCostsByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse updateActualCostsByItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(true);
    _captive = true;
  }

  // default cost type is "actual"
  param = pParams.value("costtype", &valid);
  if (valid && param.toString() == "standard")
  {
    _updateActual = false;
    setWindowTitle(tr("Update Standard Costs By Item"));
    _rollUp->setText(tr("&Roll Up Standard Costs"));
  }

  return NoError;
}

void updateActualCostsByItem::sSelectAll()
{
  _lowerMaterial->setChecked(true);
  _user->setChecked(true);
  _lowerUser->setChecked(true);
  if (_metrics->boolean("Routings"))
  {
    _directLabor->setChecked(true);
    _lowerDirectLabor->setChecked(true);
    _overhead->setChecked(true);
    _lowerOverhead->setChecked(true);
    if (_metrics->value("TrackMachineOverhead") == "M")
    {
      _machOverhead->setChecked(true);
      _lowerMachOverhead->setChecked(true);
    }
  }
}

void updateActualCostsByItem::sUpdate()
{
  XSqlQuery sql;
  sql.prepare("SELECT doUpdateCosts(:item_id, true, :lowMaterial, :dirLabor, "
              "         :lowDirLabor, :overhead, :lowOverhead, :machOverhead, "
              "         :lowMachOverhead, :lowUser, :rollUp, :updateActual)");
  sql.bindValue(":item_id",         _item->id());
  sql.bindValue(":lowMaterial",     _lowerMaterial->isChecked()     ? "t" : "f");
  sql.bindValue(":dirLabor",        _directLabor->isChecked()       ? "t" : "f");
  sql.bindValue(":lowDirLabor",     _lowerDirectLabor->isChecked()  ? "t" : "f");
  sql.bindValue(":overhead",        _overhead->isChecked()          ? "t" : "f");
  sql.bindValue(":lowOverhead",     _lowerOverhead->isChecked()     ? "t" : "f");
  sql.bindValue(":machOverhead",    (_machOverhead->isChecked() ||
      ((_metrics->value("TrackMachineOverhead") != "M") && _metrics->boolean("Routings"))) ? "t" : "f");
  sql.bindValue(":lowMachOverhead", (_lowerMachOverhead->isChecked() ||
      ((_metrics->value("TrackMachineOverhead") != "M") && _metrics->boolean("Routings"))) ? "t" : "f");
  sql.bindValue(":lowUser",         _lowerUser->isChecked()         ? "t" : "f");
  sql.bindValue(":rollUp",          _rollUp->isChecked()            ? "t" : "f");
  sql.bindValue(":updateActual",    _updateActual                   ? "t" : "f" );

  sql.exec();
  if (sql.lastError().type() != QSqlError::NoError)
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__));
    return;
  }

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));
    _item->setId(-1);
    _item->setFocus();
  }
}
