/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "releasePlannedOrdersByPlannerCode.h"

#include <QVariant>
#include <QMessageBox>

releasePlannedOrdersByPlannerCode::releasePlannedOrdersByPlannerCode(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_release, SIGNAL(clicked()), this, SLOT(sRelease()));

  _plannerCode->setType(ParameterGroup::PlannerCode);
  
// TODO Always append to existing unreleased Transfer Orders
  _appendTransferOrder->hide();
  _appendTransferOrder->setChecked(true);
}

releasePlannedOrdersByPlannerCode::~releasePlannedOrdersByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void releasePlannedOrdersByPlannerCode::languageChange()
{
  retranslateUi(this);
}

void releasePlannedOrdersByPlannerCode::sRelease()
{
  XSqlQuery releaseRelease;
  if (!_cutoffDate->isValid())
  {
    QMessageBox::critical( this, tr("Enter Cutoff Date"),
                           tr("Please enter a valid Cutoff Date.") );
    _cutoffDate->setFocus();
    return;
  }

  QString sql( "SELECT releasePlannedOrder(planord_id, true, :appendTransferOrder) "
               "FROM planord, itemsite "
               "WHERE ( (planord_itemsite_id=itemsite_id)"
               " AND (planord_startdate<=:cutOffDate)" );

  if (_warehouse->isChecked())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_firmedOnly->isChecked())
    sql += " AND (planord_firm)";

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  sql += ");";

  releaseRelease.prepare(sql);
  releaseRelease.bindValue(":cutOffDate", _cutoffDate->date());
//  releaseRelease.bindValue(":appendTransferOrder",	QVariant(_appendTransferOrder->isChecked()));
  releaseRelease.bindValue(":appendTransferOrder",	true);
  _warehouse->bindValue(releaseRelease);
  _plannerCode->bindValue(releaseRelease);
  releaseRelease.exec();

  accept();
}
