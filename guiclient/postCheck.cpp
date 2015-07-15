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

postCheck::postCheck(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));

  _captive = FALSE;

  _check->setAllowNull(TRUE);

  _bankaccnt->setType(XComboBox::APBankAccounts);
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
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("check_id", &valid);
  if (valid)
  {
    populate(param.toInt());
    _bankaccnt->setEnabled(FALSE);
    _check->setEnabled(FALSE);
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
      systemError(this, storedProcErrorLookup("postCheck", result),
		  __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(postPost.value("checkhead_bankaccnt_id").toInt(), _check->id(), TRUE);

    if (_captive)
      accept();
    else
    {
      sHandleBankAccount(_bankaccnt->id());
      _close->setText(tr("&Close"));
    }
  }
  else if (postPost.lastError().type() != QSqlError::NoError)
  {
    systemError(this, postPost.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void postCheck::sHandleBankAccount(int pBankaccntid)
{
  XSqlQuery postHandleBankAccount;
  postHandleBankAccount.prepare( "SELECT checkhead_id,"
	     "       (TEXT(checkhead_number) || '-' || checkrecip_name) "
             "FROM checkhead LEFT OUTER JOIN"
	     "     checkrecip ON ((checkhead_recip_id=checkrecip_id)"
	     "                AND (checkhead_recip_type=checkrecip_type))"
             "     JOIN bankaccnt ON (checkhead_bankaccnt_id=bankaccnt_id)  "
             " WHERE ((NOT checkhead_void)"
             "  AND  (NOT checkhead_posted)"
             "  AND  CASE WHEN (bankaccnt_prnt_check) THEN checkhead_printed ELSE 1=1 END" 
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
  else if (postpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, postpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
