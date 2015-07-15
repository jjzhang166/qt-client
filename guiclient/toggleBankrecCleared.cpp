/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "toggleBankrecCleared.h"

#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

#include <metasql.h>
#include <parameter.h>

#include "mqlutil.h"

toggleBankrecCleared::toggleBankrecCleared(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  
  _bankaccntid = -1;
  _transtype = "";
  _bankrecid = -1;
  _sourceid = -1;
  _source = "";
  _baseamount = 0.0;

  if (!_metrics->boolean("CashBasedTax"))
  {
    _transdate->setEnabled(false);
  }
  
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

toggleBankrecCleared::~toggleBankrecCleared()
{
  // no need to delete child widgets, Qt does it all for us
}

void toggleBankrecCleared::languageChange()
{
  retranslateUi(this);
}

enum SetResponse toggleBankrecCleared::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("bankaccntid", &valid);
  if (valid)
    _bankaccntid = param.toInt();
  
  param = pParams.value("bankrecid", &valid);
  if (valid)
    _bankrecid = param.toInt();
  
  param = pParams.value("sourceid", &valid);
  if (valid)
    _sourceid = param.toInt();
  
  param = pParams.value("source", &valid);
  if (valid)
    _source = param.toString();
  
  param = pParams.value("transtype", &valid);
  if (valid)
    _transtype = param.toString();
  
  populate();

  return NoError;
}

void toggleBankrecCleared::sSave()
{
  XSqlQuery reconcileToggleCleared;
  reconcileToggleCleared.prepare("SELECT bankrec_id FROM bankrec WHERE (bankrec_id=:bankrecid)"
                                 "  AND (:transdate BETWEEN bankrec_opendate AND bankrec_enddate);");
  reconcileToggleCleared.bindValue(":bankrecid", _bankrecid);
  reconcileToggleCleared.bindValue(":transdate", _transdate->date());
  reconcileToggleCleared.exec();
  if (!reconcileToggleCleared.first())
  {
    QMessageBox::critical( this, tr("Date Outside Range"),
                          tr("The Effective Date must be in the date range "
                             "of this Bank Reconciliation period.") );
    return;
  }
  if (reconcileToggleCleared.lastError().type() != QSqlError::NoError)
  {
    systemError(this, reconcileToggleCleared.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  reconcileToggleCleared.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid, :currrate, :baseamount, :transdate) AS cleared");
  reconcileToggleCleared.bindValue(":bankrecid", _bankrecid);
  reconcileToggleCleared.bindValue(":sourceid", _sourceid);
  reconcileToggleCleared.bindValue(":source", _source);
  reconcileToggleCleared.bindValue(":currrate", _exchrate->toDouble());
  reconcileToggleCleared.bindValue(":baseamount", _baseamount);
  reconcileToggleCleared.bindValue(":transdate", _transdate->date());
  reconcileToggleCleared.exec();
  if (reconcileToggleCleared.lastError().type() != QSqlError::NoError)
  {
    systemError(this, reconcileToggleCleared.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  QDialog::accept();
}

void toggleBankrecCleared::populate()
{
  if (_transtype == "receipt")
    populateReceipt();
  else
    populateCheck();
}

void toggleBankrecCleared::populateReceipt()
{
  ParameterList params;
  params.append("bankaccntid", _bankaccntid);
  params.append("bankrecid", _bankrecid);
  params.append("sourceid", _sourceid);
  params.append("source", _source);
  MetaSQLQuery mrcp = mqlLoad("bankrec", "receipts");
  XSqlQuery rcp = mrcp.toQuery(params);
  if (rcp.first())
  {
    _docnumber->setText(rcp.value("docnumber").toString());
    _transdate->setDate(rcp.value("f_date").toDate());
    _exchrate->setDouble(rcp.value("doc_exchrate").toDouble());
    _baseamount = rcp.value("base_amount").toDouble();
  }
  if (rcp.lastError().type() != QSqlError::NoError)
  {
    systemError(this, rcp.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void toggleBankrecCleared::populateCheck()
{
  ParameterList params;
  params.append("bankaccntid", _bankaccntid);
  params.append("bankrecid", _bankrecid);
  params.append("sourceid", _sourceid);
  params.append("source", _source);
  MetaSQLQuery mchk = mqlLoad("bankrec", "checks");
  XSqlQuery chk = mchk.toQuery(params);
  if (chk.first())
  {
    _docnumber->setText(chk.value("doc_number").toString());
    _transdate->setDate(chk.value("transdate").toDate());
    _exchrate->setDouble(chk.value("doc_exchrate").toDouble());
    _baseamount = chk.value("base_amount").toDouble();
  }
  if (chk.lastError().type() != QSqlError::NoError)
  {
    systemError(this, chk.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}