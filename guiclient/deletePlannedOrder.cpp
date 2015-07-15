/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "deletePlannedOrder.h"

#include <QVariant>

#include "plCluster.h"

deletePlannedOrder::deletePlannedOrder(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_planord, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  _captive = false;
}

deletePlannedOrder::~deletePlannedOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void deletePlannedOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse deletePlannedOrder::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("planord_id", &valid);
  if (valid)
  {
    _planord->setId(param.toInt());
    _planord->setReadOnly(true);
  }

  return NoError;
}

void deletePlannedOrder::sDelete()
{
  XSqlQuery deleteDelete;
  deleteDelete.prepare( "SELECT deletePlannedOrder(:planord_id, :deleteChildren);" );
  deleteDelete.bindValue(":planord_id", _planord->id());
  deleteDelete.bindValue(":deleteChildren", QVariant(_deleteChildren->isChecked()));
  deleteDelete.exec();

  if (_captive)
    accept();
  else
  {
    _planord->setId(-1);
    _close->setText(tr("&Close"));
    _planord->setFocus();
  }
}

