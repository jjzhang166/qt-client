/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customerGroups.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include "customerGroup.h"
#include "guiclient.h"

customerGroups::customerGroups(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _custgrp->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "custgrp_name");
  _custgrp->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "custgrp_descrip");
  
  if (_privileges->check("MaintainCustomerGroups"))
  {
    connect(_custgrp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_custgrp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_custgrp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_custgrp, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

customerGroups::~customerGroups()
{
  // no need to delete child widgets, Qt does it all for us
}

void customerGroups::languageChange()
{
  retranslateUi(this);
}

void customerGroups::sDelete()
{
  if (QMessageBox::question(this, tr("Delete?"),
                            tr("Are you sure you want to delete this Customer Group?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;
  
  XSqlQuery customerDelete;
  customerDelete.prepare( "DELETE FROM custgrpitem "
             "WHERE (custgrpitem_custgrp_id=:custgrp_id); "
             "DELETE FROM custgrp "
             "WHERE (custgrp_id=:custgrp_id);" );
  customerDelete.bindValue(":custgrp_id", _custgrp->id());
  customerDelete.exec();

  sFillList();
}

void customerGroups::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  customerGroup newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void customerGroups::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("custgrp_id", _custgrp->id());

  customerGroup newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void customerGroups::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("custgrp_id", _custgrp->id());

  customerGroup newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void customerGroups::sFillList()
{
  _custgrp->populate( "SELECT custgrp_id, custgrp_name, custgrp_descrip "
                      "FROM custgrp "
                      "ORDER BY custgrp_name;" );
}
