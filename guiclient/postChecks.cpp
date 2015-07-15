/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postChecks.h"

#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "storedProcErrorLookup.h"

postChecks::postChecks(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));

  _numberOfChecks->setPrecision(0);

  _bankaccnt->setAllowNull(TRUE);
  _bankaccnt->setType(XComboBox::APBankAccounts);

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);

}

postChecks::~postChecks()
{
  // no need to delete child widgets, Qt does it all for us
}

void postChecks::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postChecks::set(const ParameterList & pParams )
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
    _bankaccnt->setId(param.toInt());

  return NoError;
}

void postChecks::sPost()
{
  XSqlQuery postPost;
  postPost.prepare("SELECT postChecks(:bankaccnt_id) AS result;");
  postPost.bindValue(":bankaccnt_id", _bankaccnt->id());
  postPost.exec();
  if (postPost.first())
  {
    int result = postPost.value("result").toInt();
    if (result < 0)
      systemError(this, storedProcErrorLookup("postChecks", result), __FILE__, __LINE__);

    omfgThis->sChecksUpdated(_bankaccnt->id(), -1, TRUE);

    if (_printJournal->isChecked())
    {
    ParameterList params;
    params.append("source", "A/P");
    params.append("sourceLit", tr("A/P"));
    params.append("startJrnlnum", result);
    params.append("endJrnlnum", result);

    if (_metrics->boolean("UseJournals"))
    {
      params.append("title",tr("Journal Series"));
      params.append("table", "sltrans");
    }
    else
    {
      params.append("title",tr("General Ledger Series"));
      params.append("gltrans", true);
      params.append("table", "gltrans");
    }

    orReport report("GLSeries", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
    }

    accept();
  }
  else if (postPost.lastError().type() != QSqlError::NoError)
  {
    systemError(this, postPost.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void postChecks::sHandleBankAccount(int pBankaccntid)
{
  XSqlQuery postHandleBankAccount;
  postHandleBankAccount.prepare( "SELECT COUNT(*) AS numofchecks "
             "FROM checkhead "
             "JOIN bankaccnt ON (bankaccnt_id=checkhead_bankaccnt_id) "
             "WHERE ( (NOT checkhead_void)"
             " AND (NOT checkhead_posted)"
             " AND CASE WHEN (bankaccnt_prnt_check) THEN checkhead_printed ELSE 1=1 END"
             " AND (checkhead_bankaccnt_id=:bankaccnt_id) );" );
  postHandleBankAccount.bindValue(":bankaccnt_id", pBankaccntid);
  postHandleBankAccount.exec();
  if (postHandleBankAccount.first())
    _numberOfChecks->setDouble(postHandleBankAccount.value("numofchecks").toDouble());
  else if (postHandleBankAccount.lastError().type() != QSqlError::NoError)
  {
    systemError(this, postHandleBankAccount.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
