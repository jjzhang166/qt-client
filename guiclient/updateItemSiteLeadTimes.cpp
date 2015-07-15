/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateItemSiteLeadTimes.h"

updateItemSiteLeadTimes::updateItemSiteLeadTimes(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));

  _classCode->setType(ParameterGroup::ClassCode);
}

updateItemSiteLeadTimes::~updateItemSiteLeadTimes()
{
  // no need to delete child widgets, Qt does it all for us
}

void updateItemSiteLeadTimes::languageChange()
{
  retranslateUi(this);
}

void updateItemSiteLeadTimes::sUpdate()
{
  XSqlQuery updateUpdate;
  QString sql( "SELECT updateItemSiteLeadTime(itemsite_id, :leadTimePad) AS result "
               "FROM itemsite, item "
               "WHERE ((itemsite_item_id=item_id)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  sql += ");";

  updateUpdate.prepare(sql);
  updateUpdate.bindValue(":leadTimePad", _leadTimePad->value());
  _warehouse->bindValue(updateUpdate);
  _classCode->bindValue(updateUpdate);
  updateUpdate.exec();

  accept();
}
