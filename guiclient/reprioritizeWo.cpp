/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reprioritizeWo.h"

#include <QMessageBox>
#include <QVariant>
#include "errorReporter.h"

reprioritizeWo::reprioritizeWo(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_reprioritize, SIGNAL(clicked()), this, SLOT(sReprioritize()));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sPopulateWoInfo(int)));

  _captive = false;

  _wo->setType(cWoOpen | cWoExploded | cWoReleased | cWoIssued);

  if (_preferences->boolean("XCheckBox/forgetful"))
    _changeChildren->setChecked(true);
}

reprioritizeWo::~reprioritizeWo()
{
  // no need to delete child widgets, Qt does it all for us
}

void reprioritizeWo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse reprioritizeWo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setReadOnly(true);
  }

  return NoError;
}

void reprioritizeWo::sReprioritize()
{
  XSqlQuery reprioritizeReprioritize;

  reprioritizeReprioritize.prepare("SELECT reprioritizeWo(:wo_id, :newPriority, :reprioritizeChildren);");
  reprioritizeReprioritize.bindValue(":wo_id", _wo->id());
  reprioritizeReprioritize.bindValue(":newPriority", _new->value());
  reprioritizeReprioritize.bindValue(":reprioritizeChildren", QVariant(_changeChildren->isChecked()));
  reprioritizeReprioritize.exec();

  omfgThis->sWorkOrdersUpdated(_wo->id(), true);

  if (_captive)
    close();
  else
  {
    _wo->setId(-1);
    _close->setText(tr("&Close"));
    _wo->setFocus();
  }
}


void reprioritizeWo::sPopulateWoInfo(int pWoid)
{
  XSqlQuery reprioritizePopulateWoInfo;
  if (pWoid != -1)
  {
    reprioritizePopulateWoInfo.prepare( "SELECT wo_priority "
               "FROM wo "
               "WHERE (wo_id=:wo_id);" );
    reprioritizePopulateWoInfo.bindValue(":wo_id", pWoid);
    reprioritizePopulateWoInfo.exec();
    if (reprioritizePopulateWoInfo.first())
    {
      _current->setText(reprioritizePopulateWoInfo.value("wo_priority").toString());
      _new->setValue(reprioritizePopulateWoInfo.value("wo_priority").toInt());
      return;
    }
    else
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Information"),
                         reprioritizePopulateWoInfo, __FILE__, __LINE__);
  }

  _current->clear();
  _new->setValue(1);
}

