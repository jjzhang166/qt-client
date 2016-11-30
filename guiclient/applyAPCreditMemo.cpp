/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "applyAPCreditMemo.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>

#include "apCreditMemoApplication.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "storedProcErrorLookup.h"
#include "mqlutil.h"

applyAPCreditMemo::applyAPCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_apply,          SIGNAL(clicked()), this, SLOT(sApply()));
  connect(_applyToBalance, SIGNAL(clicked()), this, SLOT(sApplyBalance()));
  connect(_available, SIGNAL(idChanged(int)), this, SLOT(sPriceGroup()));
  connect(_clear,          SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_buttonBox,      SIGNAL(accepted()), this, SLOT(sPost()));
  connect(_buttonBox,      SIGNAL(rejected()), this, SLOT(sClose()));

  _buttonBox->button(QDialogButtonBox::Save)->setText(tr("Post"));

  _captive = false;

  _apopen->addColumn(tr("Doc. Type"),   _docTypeColumn,  Qt::AlignCenter,true, "doctype");
  _apopen->addColumn(tr("Doc. Number"), -1,              Qt::AlignCenter,true, "apopen_docnumber");
  _apopen->addColumn(tr("Doc. Date"),   _dateColumn,     Qt::AlignCenter,true, "apopen_docdate");
  _apopen->addColumn(tr("Due Date"),    _dateColumn,     Qt::AlignCenter,true, "apopen_duedate");
  _apopen->addColumn(tr("Open"),        _moneyColumn,    Qt::AlignRight, true, "openamount");
  _apopen->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency(), "opencurrabbr");
  _apopen->addColumn(tr("Applied"),     _moneyColumn,    Qt::AlignRight, true, "apcreditapply_amount");
  _apopen->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency(), "appliedcurrabbr");

  if (_privileges->check("ApplyAPMemos"))
      connect(_apopen, SIGNAL(valid(bool)), _apply, SLOT(setEnabled(bool)));

  _vend->setReadOnly(true);
  sPriceGroup();
  adjustSize();
}

applyAPCreditMemo::~applyAPCreditMemo()
{
  // no need to delete child widgets, Qt does it all for us
}

void applyAPCreditMemo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse applyAPCreditMemo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("apopen_id", &valid);
  if (valid)
  {
    _captive = true;
    _apopenid = param.toInt();
    populate();
  }

  return NoError;
}

void applyAPCreditMemo::sPost()
{
  XSqlQuery applyPost;

  applyPost.prepare("SELECT postAPCreditMemoApplication(:apopen_id) AS result;");
  applyPost.bindValue(":apopen_id", _apopenid);
  applyPost.exec();
  if (ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error posting"), applyPost,
                           __FILE__, __LINE__))
    return;

  accept();
}

void applyAPCreditMemo::sApplyBalance()
{
  XSqlQuery applyApplyBalance;
  applyApplyBalance.prepare("SELECT applyAPCreditMemoToBalance(:apopen_id) AS result;");
  applyApplyBalance.bindValue(":apopen_id", _apopenid);
  applyApplyBalance.exec();
  if (applyApplyBalance.first())
  {
    int result = applyApplyBalance.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying A/P Credit Memo"),
                             storedProcErrorLookup("applyAPCreditMemoToBalance", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying A/P Credit Memo"),
                                applyApplyBalance, __FILE__, __LINE__))
  {
    return;
  }

  populate();
}

void applyAPCreditMemo::sApply()
{
  ParameterList params;
  params.append("sourceApopenid", _apopenid);
  params.append("targetApopenid", _apopen->id());

  apCreditMemoApplication newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    populate();
}

void applyAPCreditMemo::sClear()
{
  XSqlQuery applyClear;
  applyClear.prepare( "DELETE FROM apcreditapply "
             "WHERE ( (apcreditapply_source_apopen_id=:sourceApopenid) "
             " AND (apcreditapply_target_apopen_id=:targetApopenid) );" );
  applyClear.bindValue(":sourceApopenid", _apopenid);
  applyClear.bindValue(":targetApopenid", _apopen->id());
  applyClear.exec();
  ErrorReporter::error(QtCriticalMsg, this, tr("Error Clearing Application of A/P CM"),
                                applyClear, __FILE__, __LINE__);

  populate();
}

void applyAPCreditMemo::sClose()
{
  XSqlQuery applyClose;
  applyClose.prepare( "DELETE FROM apcreditapply "
             "WHERE (apcreditapply_source_apopen_id=:sourceApopenid);" );
  applyClose.bindValue(":sourceApopenid", _apopenid);
  applyClose.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Cancelling Application of A/P CM"),
                                applyClose, __FILE__, __LINE__))
  {
    return;
  }

  reject();
}

void applyAPCreditMemo::populate()
{
  XSqlQuery applypopulate;
  applypopulate.prepare( "SELECT apopen_vend_id, apopen_docnumber, apopen_docdate,"
             "       (apopen_amount - apopen_paid) AS available, apopen_curr_id, "
             "       COALESCE(SUM(currToCurr(apcreditapply_curr_id,"
	     "				apopen_curr_id, apcreditapply_amount, "
	     "				CURRENT_DATE)), 0) AS f_applied "
             "FROM apopen LEFT OUTER JOIN apcreditapply ON (apcreditapply_source_apopen_id=apopen_id) "
             "WHERE (apopen_id=:apopen_id) "
             "GROUP BY apopen_vend_id, apopen_docnumber, apopen_docdate,"
             "         apopen_curr_id, apopen_amount, apopen_paid;" );
  applypopulate.bindValue(":apopen_id", _apopenid);
  applypopulate.exec();
  if (applypopulate.first())
  {
    _vend->setId(applypopulate.value("apopen_vend_id").toInt());
    _docDate->setDate(applypopulate.value("apopen_docdate").toDate(), true);
    _available->setId(applypopulate.value("apopen_curr_id").toInt());
    _available->setLocalValue(applypopulate.value("available").toDouble());
    _applied->setLocalValue(applypopulate.value("f_applied").toDouble());
    _balance->setLocalValue(_available->localValue() - _applied->localValue());
    _docNumber->setText(applypopulate.value("apopen_docnumber").toString());
  
    _cachedAmount = applypopulate.value("available").toDouble();
  }
  else (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving A/P Information"),
                                applypopulate, __FILE__, __LINE__));


  MetaSQLQuery mql = mqlLoad("applyAPMemo", "details");
  ParameterList params;
  params.append("parentApopenid", _apopenid);
  params.append("vend_id", _vend->id());
  params.append("voucher", tr("Voucher"));
  params.append("debitMemo", tr("Debit Memo"));

  applypopulate = mql.toQuery(params);
  _apopen->populate(applypopulate);
  ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving A/P Information"),
                                applypopulate, __FILE__, __LINE__);
}

void applyAPCreditMemo::sPriceGroup()
{
  if (! omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_available->currAbbr()));
}
