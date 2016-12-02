/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "prepareCheckRun.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"
#include "errorReporter.h"

prepareCheckRun::prepareCheckRun(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  
  connect(_prepare, SIGNAL(clicked()), this, SLOT(sPrint()));
  
  _checkDate->setDate(omfgThis->dbDate());
}

prepareCheckRun::~prepareCheckRun()
{
  // no need to delete child widgets, Qt does it all for us
}

void prepareCheckRun::languageChange()
{
  retranslateUi(this);
}

enum SetResponse prepareCheckRun::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
  {
    _bankaccnt->setId(param.toInt());
    if (_bankaccnt->isValid())
      _bankaccnt->setEnabled(false);
  }
    
  return NoError;
}

void prepareCheckRun::sPrint()
{
  XSqlQuery preparePrint;
  if(!_bankaccnt->isValid())
  {
    QMessageBox::warning(this, tr("No Bank Account"), tr("You must select a Bank Account before you may continue."));
    return;
  }

  preparePrint.prepare("SELECT createChecks(:bankaccnt_id, :checkDate) AS result;");
  preparePrint.bindValue(":bankaccnt_id", _bankaccnt->id());
  preparePrint.bindValue(":checkDate", _checkDate->date());
  preparePrint.exec();
  if (preparePrint.first())
  {
    int result = preparePrint.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Preparing Check Run for Printing"),
                             storedProcErrorLookup("createChecks", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Preparing Check Run for Printing"),
                                preparePrint, __FILE__, __LINE__))
  {
    return;
  }

  omfgThis->sChecksUpdated(_bankaccnt->id(), -1, true);

  accept();
}
