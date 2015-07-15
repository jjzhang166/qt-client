/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateOUTLevelByItem.h"

#include <QMessageBox>
#include <parameter.h>

updateOUTLevelByItem::updateOUTLevelByItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
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
  connect(_item, SIGNAL(valid(bool)), _update, SLOT(setEnabled(bool)));
}

updateOUTLevelByItem::~updateOUTLevelByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void updateOUTLevelByItem::languageChange()
{
  retranslateUi(this);
}

void updateOUTLevelByItem::sUpdate()
{
  XSqlQuery updateUpdate;
  if (_periods->topLevelItemCount() > 0)
  {
    QString sql;

    if (_leadTime->isChecked())
      sql = QString( "SELECT updateOUTLevel(itemsite_id, (itemsite_leadtime + :leadTimePad), '{%1}') AS result "
                     "FROM itemsite "
                     "WHERE ( (itemsite_item_id=:item_id)" )
            .arg(_periods->periodString());

    else if (_fixedDays->isChecked())
      sql = QString( "SELECT updateOUTLevel(itemsite_id, :days, '{%1}') AS result "
                     "FROM itemsite "
                     "WHERE ( (itemsite_item_id=:item_id)" )
            .arg(_periods->periodString());

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += ");";

    updateUpdate.prepare(sql);
    _warehouse->bindValue(updateUpdate);
    updateUpdate.bindValue(":leadTimePad", _leadTimePad->value());
    updateUpdate.bindValue(":days", _days->value());
    updateUpdate.bindValue(":item_id", _item->id());
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
