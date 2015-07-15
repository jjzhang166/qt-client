/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateOUTLevels.h"

#include <QMessageBox>

updateOUTLevels::updateOUTLevels(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _daysGroupInt = new QButtonGroup(this);
  _daysGroupInt->addButton(_leadTime);
  _daysGroupInt->addButton(_fixedDays);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
  connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
  connect(_fixedDays, SIGNAL(toggled(bool)), _days, SLOT(setEnabled(bool)));
  connect(_leadTime, SIGNAL(toggled(bool)), _leadTimePad, SLOT(setEnabled(bool)));
  
  _plannerCode->setType(ParameterGroup::PlannerCode);
}

updateOUTLevels::~updateOUTLevels()
{
  // no need to delete child widgets, Qt does it all for us
}

void updateOUTLevels::languageChange()
{
  retranslateUi(this);
}

void updateOUTLevels::sUpdate()
{
  XSqlQuery updateUpdate;
  if (_periods->topLevelItemCount() > 0)
  {
    QString sql;

    if (_leadTime->isChecked())
      sql = QString( "SELECT updateOUTLevel(itemsite_id, (itemsite_leadtime + :leadTimePad), '{%1}') AS result "
                     "FROM itemsite, plancode "
                     "WHERE ( (itemsite_plancode_id=plancode_id)" )
            .arg(_periods->periodString());

    else if (_fixedDays->isChecked())
      sql = QString( "SELECT updateOUTLevel(itemsite_id, :days, '{%1}') AS result "
                     "FROM itemsite, plancode "
                     "WHERE ( (itemsite_plancode_id=plancode_id)" )
            .arg(_periods->periodString());

    if (_warehouse->isChecked())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    if (_plannerCode->isSelected())
      sql += " AND (plancode_id=:plancode_id)";
    else if (_plannerCode->isPattern())
      sql += " AND (plancode_code ~ :plancode_pattern)";

    sql += ");";

    updateUpdate.prepare(sql);
    updateUpdate.bindValue(":leadTimePad", _leadTimePad->value());
    updateUpdate.bindValue(":days", _days->value());
    _warehouse->bindValue(updateUpdate);
    _plannerCode->bindValue(updateUpdate);
    updateUpdate.exec();

    accept();
  }
  else
  {
    QMessageBox::critical( this, tr("Incomplete Data"),
                           tr("You must select at least one Period to continue.") );
    _periods->setFocus();
    return;
  }
}
