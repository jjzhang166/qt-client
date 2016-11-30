/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "assignItemToPlannerCode.h"

#include <QMessageBox>
#include <QVariant>

#include "guiErrorCheck.h"

assignItemToPlannerCode::assignItemToPlannerCode(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

    // signals and slots connections
  connect(_item, SIGNAL(valid(bool)), _buttonBox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sAssign()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  _plannerCode->setAllowNull(true);
  _plannerCode->setType(XComboBox::PlannerCodes);
}

assignItemToPlannerCode::~assignItemToPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void assignItemToPlannerCode::languageChange()
{
  retranslateUi(this);
}

void assignItemToPlannerCode::sAssign()
{
  XSqlQuery assignAssign;
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_item->isValid(), _item,
                          tr("You must select a valid Item before continuing.") )
         << GuiErrorCheck(!_plannerCode->isValid(), _plannerCode,
                          tr("You must select a Planner Code to assign before continuing.") )
  ;

  if (GuiErrorCheck::reportErrors(this, tr("Missing Information"), errors))
      return;

  QString sql( "UPDATE itemsite "
               "SET itemsite_plancode_id=:plancode_id "
               "WHERE ( (itemsite_item_id=:item_id)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ");";

  assignAssign.prepare(sql);
  _warehouse->bindValue(assignAssign);
  assignAssign.bindValue(":plancode_id", _plannerCode->id());
  assignAssign.bindValue(":item_id", _item->id());
  assignAssign.exec();

  accept();
}
