/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "costingElements.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include "userCostingElement.h"

costingElements::costingElements(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _costelem->addColumn(tr("Costing Element"), -1, Qt::AlignLeft, true, "costelem_type");

  sFillList();
}

costingElements::~costingElements()
{
  // no need to delete child widgets, Qt does it all for us
}

void costingElements::languageChange()
{
  retranslateUi(this);
}

void costingElements::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  userCostingElement newdlg(this, "", true);
  newdlg.set(params);
 
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void costingElements::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("costelem_id", _costelem->id());

  userCostingElement newdlg(this, "", true);
  newdlg.set(params);
 
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void costingElements::sDelete()
{
  XSqlQuery costingDelete;
  costingDelete.prepare( "SELECT itemcost_id "
             "FROM itemcost "
             "WHERE (itemcost_costelem_id=:costelem_id) "
             "LIMIT 1;" );
  costingDelete.bindValue(":costelem_id", _costelem->id());
  costingDelete.exec();
  if (costingDelete.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Selected User Costing Element"),
                           tr( "<p>The selected User Costing Element cannot be "
                              "deleted as it is being used by an existing Item "
                              "Cost. You must first delete all Item Costs "
                              "assigned to the selected User Costing Element "
                              "before you may delete it." ) );
    return;
  }

  costingDelete.prepare( "SELECT costhist_id "
             "FROM costhist "
             "WHERE (costhist_costelem_id=:costelem_id) "
             "LIMIT 1;" );
  costingDelete.bindValue(":costelem_id", _costelem->id());
  costingDelete.exec();
  if (costingDelete.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Selected User Costing Element"),
                           tr( "<p>The selected User Costing Element cannot be "
                              "deleted as it is there is Costing History "
                              "assigned to it. You may only deactivate the "
                              "selected User Costing Element." ) );
    return;
  }

  costingDelete.prepare( "DELETE FROM costelem "
             "WHERE (costelem_id=:costelem_id);" );
  costingDelete.bindValue(":costelem_id", _costelem->id());
  costingDelete.exec();

  sFillList();
}

void costingElements::sFillList()
{
  _costelem->populate( "SELECT costelem_id, costelem_type "
                       "FROM costelem "
                       "WHERE (NOT costelem_sys) "
                       "ORDER BY costelem_type" );
}
