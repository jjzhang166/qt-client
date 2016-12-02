/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postCheck.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"
#include "errorReporter.h"

postCheck::postCheck(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));

  _captive = false;

  _check->setAllowNull(true);

  _bankaccnt->setType(XComboBox::APBankAccounts);
  sHandleBankAccount(_bankaccnt->id());
}

postCheck::~postCheck()
{
  // no need to delete child widgets, Qt does it all for us
}

void postCheck::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postCheck::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("check_id", &valid);
  if (valid)
  {
    populate(param.toInt());
    _bankaccnt->setEnabled(false);
    _check->setEnabled(false);
  }

  return NoError;
}

void postCheck::sPost()
{
  XSqlQuery postPost;
  postPost.prepare( "SELECT checkhead_bankaccnt_id,"
	     "       postCheck(checkhead_id, NULL) AS result "
             "FROM checkhead "
             "WHERE ((checkhead_id=:checkhead_id)"
             " AND  (NOT checkhead_posted) );" );
  postPost.bindValue(":checkhead_id", _check->id());
  postPost.exec();
  if (postPost.first())
  {
    int result = postPost.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Check Information"),
                             storedProcErrorLookup("postCheck", result),
                             __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(postPost.value("checkhead_bankaccnt_id").toInt(), _check->id(), true);

    if (_captive)
      accept();
    else
    {
      sHandleBankAccount(_bankaccnt->id());
      _close->setText(tr("&Close"));
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Check Information"),
                                postPost, __FILE__, __LINE__))
  {
    return;
  }
}

void postCheck::sHandleBankAccount(int pBankaccntid)
{
  XSqlQuery postHandleBankAccount;
  postHandleBankAccount.prepare( "SELECT checkhead_id,"
	     "     (CASE WHEN(checkhead_number=-1) THEN TEXT('Unspecified')"
             "           ELSE TEXT(checkhead_number) "
             "      END || '-' || checkrecip_name) "
             "FROM checkhead LEFT OUTER JOIN"
	     "     checkrecip ON ((checkhead_recip_id=checkrecip_id)"
	     "                AND (checkhead_recip_type=checkrecip_type))"
             "     JOIN bankaccnt ON (checkhead_bankaccnt_id=bankaccnt_id)  "
             " WHERE ((NOT checkhead_void)"
             "  AND  (NOT checkhead_posted)"
             "  AND  (checkhead_bankaccnt_id=:bankaccnt_id) ) "
             "ORDER BY checkhead_number;" );
  postHandleBankAccount.bindValue(":bankaccnt_id", pBankaccntid);
  postHandleBankAccount.exec();
  _check->populate(postHandleBankAccount);
  _check->setNull();
}

void postCheck::populate(int pcheckid)
{
  XSqlQuery postpopulate;
  postpopulate.prepare( "SELECT checkhead_bankaccnt_id "
             "FROM checkhead "
             "WHERE (checkhead_id=:check_id);" );
  postpopulate.bindValue(":check_id", pcheckid);
  postpopulate.exec();
  if (postpopulate.first())
  {
    _bankaccnt->setId(postpopulate.value("checkhead_bankaccnt_id").toInt());
    _check->setId(pcheckid);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Check Information"),
                                postpopulate, __FILE__, __LINE__))
  {
    return;
  }
}
