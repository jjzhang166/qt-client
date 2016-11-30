/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateActualCostsByClassCode.h"

#include <QSqlError>
#include <QVariant>
#include "errorReporter.h"

updateActualCostsByClassCode::updateActualCostsByClassCode(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_selectAll, SIGNAL(clicked()), this, SLOT(sSelectAll()));

  _classCode->setType(ParameterGroup::ClassCode);

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

  _updateActual = true;
}

updateActualCostsByClassCode::~updateActualCostsByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void updateActualCostsByClassCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse updateActualCostsByClassCode::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  // default cost type is "actual"
  param = pParams.value("costtype", &valid);
  if (valid)
  { 
    if (param.toString() == "standard")
    {
      _updateActual = false;
      setWindowTitle(tr("Update Standard Costs By Class Code"));
      _rollUp->setText(tr("&Roll Up Standard Costs"));
    }
    else if (param.toString() != "actual")
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                           tr("%1: A System Error occurred. \n"
                              "Illegal parameter value '%2' for 'costtype'")
                           .arg(windowTitle())
                           .arg(param.toString()),__FILE__,__LINE__);
      return UndefinedError;
    }
  }

  return NoError;
}

void updateActualCostsByClassCode::sSelectAll()
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

void updateActualCostsByClassCode::sUpdate()
{
  XSqlQuery updateUpdate;
  QString sql = "SELECT doUpdateCosts(item_id, true, :lowMaterial, :directLabor, "
                "       :lowDirectLabor, :overhead, :lowOverhead, "
                "       :machOverhead, :lowMachOverhead, :lowUser, :rollUp, "
                "       :updateActual ) "
                "  FROM item, classcode "
                " WHERE(item_classcode_id = classcode_id) ";

  if (_classCode->isSelected())
    sql += "  AND (item_classcode_id=:classcode_id);";
  else if (_classCode->isPattern())
    sql += "  AND (classcode_code ~ :classcode_pattern);";
  else
    sql = "SELECT doUpdateCosts(:lowMaterial, :directLabor, "
          "        :lowDirectLabor, :overhead, :lowOverhead, "
          "        :machOverhead, :lowMachOverhead, :lowUser, :rollUp, "
          "        :updateActual);";

  updateUpdate.prepare(sql);
  updateUpdate.bindValue(":lowMaterial",     _lowerMaterial->isChecked()     ? "t" : "f" );
  updateUpdate.bindValue(":directLabor",     _directLabor->isChecked()       ? "t" : "f" );
  updateUpdate.bindValue(":lowDirectLabor",  _lowerDirectLabor->isChecked()  ? "t" : "f" );
  updateUpdate.bindValue(":overhead",        _overhead->isChecked()          ? "t" : "f" );
  updateUpdate.bindValue(":lowOverhead",     _lowerOverhead->isChecked()     ? "t" : "f" );
  updateUpdate.bindValue(":machOverhead",    (_machOverhead->isChecked() ||
      ((_metrics->value("TrackMachineOverhead") != "M") && _metrics->boolean("Routings"))) ? "t" : "f");
  updateUpdate.bindValue(":lowMachOverhead", (_lowerMachOverhead->isChecked() ||
      ((_metrics->value("TrackMachineOverhead") != "M") && _metrics->boolean("Routings"))) ? "t" : "f");
  updateUpdate.bindValue(":lowUser",         _lowerUser->isChecked()         ? "t" : "f" );
  updateUpdate.bindValue(":rollUp",          _rollUp->isChecked()            ? "t" : "f" );
  updateUpdate.bindValue(":updateActual",    _updateActual                   ? "t" : "f" );
  _classCode->bindValue(updateUpdate);

  updateUpdate.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Cost Information"),
                                updateUpdate, __FILE__, __LINE__))
  {
    return;
  }

  accept();
}
