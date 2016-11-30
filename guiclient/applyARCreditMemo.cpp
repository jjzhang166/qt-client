/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "applyARCreditMemo.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>

#include "arCreditMemoApplication.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

applyARCreditMemo::applyARCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_apply,            SIGNAL(clicked()),      this, SLOT(sApply()));
  connect(_applyLineBalance, SIGNAL(clicked()),      this, SLOT(sApplyLineBalance()));
  connect(_applyToBalance,   SIGNAL(clicked()),      this, SLOT(sApplyBalance()));
  connect(_available,        SIGNAL(idChanged(int)), this, SLOT(sPriceGroup()));
  connect(_clear,            SIGNAL(clicked()),      this, SLOT(sClear()));
  connect(_buttonBox,        SIGNAL(accepted()),     this, SLOT(sPost()));
  connect(_buttonBox,        SIGNAL(rejected()),     this, SLOT(reject()));
  connect(_searchDocNum,     SIGNAL(textChanged(const QString&)), this, SLOT(sSearchDocNumChanged(const QString&)));

  _buttonBox->button(QDialogButtonBox::Save)->setText(tr("Post"));

  _captive = false;

  _aropen->addColumn(tr("Doc. Type"),   _docTypeColumn, Qt::AlignCenter, true, "doctype");
  _aropen->addColumn(tr("Doc. Number"), -1,             Qt::AlignCenter, true, "aropen_docnumber");
  _aropen->addColumn(tr("Doc. Date"),   _dateColumn,    Qt::AlignCenter, true, "aropen_docdate");
  _aropen->addColumn(tr("Due Date"),    _dateColumn,    Qt::AlignCenter, true, "aropen_duedate");
  _aropen->addColumn(tr("Open"),        _moneyColumn,   Qt::AlignRight,  true, "balance");
  _aropen->addColumn(tr("Currency"),	_currencyColumn,Qt::AlignLeft,   !omfgThis->singleCurrency(), "balance_curr");
  _aropen->addColumn(tr("Applied"),     _moneyColumn,   Qt::AlignRight,  true, "applied");
  _aropen->addColumn(tr("Currency"),	_currencyColumn,Qt::AlignLeft,   !omfgThis->singleCurrency(), "applied_curr");
  _aropen->addColumn(tr("All Pending"), _moneyColumn,   Qt::AlignRight,  true, "pending");
  _aropen->addColumn(tr("Currency"),	_currencyColumn,Qt::AlignLeft,   !omfgThis->singleCurrency(), "balance_curr");

  _cust->setReadOnly(true);

  if(_metrics->boolean("HideApplyToBalance"))
    _applyToBalance->hide();

  sPriceGroup();
  adjustSize();
}

applyARCreditMemo::~applyARCreditMemo()
{
  // no need to delete child widgets, Qt does it all for us
}

void applyARCreditMemo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse applyARCreditMemo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("aropen_id", &valid);
  if (valid)
  {
    _captive = true;
    _aropenid = param.toInt();
    populate();
  }

  return NoError;
}

void applyARCreditMemo::sPost()
{
  XSqlQuery applyPost;
  populate(); // repeat in case someone else has updated applications

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  applyPost.exec("BEGIN;");
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting A/R CM"),
                                applyPost, __FILE__, __LINE__))
  {
    return;
  }

  applyPost.prepare("SELECT postARCreditMemoApplication(:aropen_id) AS result;");
  applyPost.bindValue(":aropen_id", _aropenid);
  applyPost.exec();
  if (applyPost.first())
  {
    QString msg;
    int result = applyPost.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting A/R CM"),
                                      applyPost, __FILE__, __LINE__);
      return;
    }
  }
  else if (applyPost.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting A/R CM"),
                                  applyPost, __FILE__, __LINE__);
    return;
  }
  applyPost.exec("COMMIT;");
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting A/R CM"),
                                applyPost, __FILE__, __LINE__))
  {
    return;
  }

  accept();
}

void applyARCreditMemo::sApplyBalance()
{
  XSqlQuery applyApplyBalance;
  applyApplyBalance.prepare("SELECT applyARCreditMemoToBalance(:aropen_id) AS result;");
  applyApplyBalance.bindValue(":aropen_id", _aropenid);
  applyApplyBalance.exec();
  if (applyApplyBalance.first())
  {
    int result = applyApplyBalance.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying A/R CM"),
                                      applyApplyBalance, __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying A/R CM"),
                                applyApplyBalance, __FILE__, __LINE__))
  {
    return;
  }

  populate();
}

void applyARCreditMemo::sApplyLineBalance()
{
  XSqlQuery applyApplyLineBalance;
  applyApplyLineBalance.prepare("SELECT applyARCreditMemoToBalance(:sourceAropenid, :targetAropenid) AS result;");
  applyApplyLineBalance.bindValue(":sourceAropenid", _aropenid);
  applyApplyLineBalance.bindValue(":targetAropenid", _aropen->id());
  applyApplyLineBalance.exec();
  if (applyApplyLineBalance.first())
  {
    int result = applyApplyLineBalance.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying A/R CM"),
                                      applyApplyLineBalance, __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying A/R CM"),
                                applyApplyLineBalance, __FILE__, __LINE__))
  {
    return;
  }

  populate();
}

void applyARCreditMemo::sApply()
{
  ParameterList params;
  params.append("sourceAropenid", _aropenid);
  params.append("targetAropenid", _aropen->id());

  arCreditMemoApplication newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    populate();
}

void applyARCreditMemo::sClear()
{
  XSqlQuery applyClear;
  applyClear.prepare( "DELETE FROM arcreditapply "
             "WHERE ( (arcreditapply_source_aropen_id=:sourceAropenid) "
             " AND (arcreditapply_target_aropen_id=:targetAropenid) );" );
  applyClear.bindValue(":sourceAropenid", _aropenid);
  applyClear.bindValue(":targetAropenid", _aropen->id());
  applyClear.exec();
  ErrorReporter::error(QtCriticalMsg, this, tr("Error Clearing Application of A/R CM"),
                                applyClear, __FILE__, __LINE__);

  populate();
}

void applyARCreditMemo::sClose()
{
  XSqlQuery applyClose;
  applyClose.prepare( "DELETE FROM arcreditapply "
             "WHERE (arcreditapply_source_aropen_id=:sourceAropenid);" );
  applyClose.bindValue(":sourceAropenid", _aropenid);
  applyClose.exec();
  ErrorReporter::error(QtCriticalMsg, this, tr("Error Clearing Application of A/R CM"),
                                applyClose, __FILE__, __LINE__);

  reject();
}

void applyARCreditMemo::populate()
{
  XSqlQuery applypopulate;
  applypopulate.prepare( "SELECT aropen_cust_id, aropen_docnumber, aropen_docdate,"
             "       (aropen_amount - aropen_paid - COALESCE(prepared,0.0) - COALESCE(cashapplied,0.0)) AS available,"
             "       COALESCE(SUM(currToCurr(arcreditapply_curr_id, "
             "				     aropen_curr_id, "
             "				     arcreditapply_amount, "
             "				     current_date)), 0) AS f_applied, "
             "	     aropen_curr_id "
             "FROM aropen LEFT OUTER JOIN arcreditapply ON (arcreditapply_source_aropen_id=aropen_id) "
             "       LEFT OUTER JOIN (SELECT aropen_id AS prepared_aropen_id,"
             "                               COALESCE(SUM(checkitem_amount + checkitem_discount),0) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN aropen ON (checkitem_aropen_id=aropen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void))"
             "                         GROUP BY aropen_id) AS sub1"
             "         ON (prepared_aropen_id=aropen_id)"
             "       LEFT OUTER JOIN (SELECT aropen_id AS cash_aropen_id,"
             "                               SUM(cashrcptitem_amount + cashrcptitem_discount) * -1.0 AS cashapplied"
             "                          FROM cashrcpt JOIN cashrcptitem ON (cashrcptitem_cashrcpt_id=cashrcpt_id)"
             "                                     JOIN aropen ON (cashrcptitem_aropen_id=aropen_id)"
             "                         WHERE ((NOT cashrcpt_posted)"
             "                           AND  (NOT cashrcpt_void))"
             "                         GROUP BY aropen_id ) AS sub2"
             "         ON (cash_aropen_id=aropen_id)"
             "WHERE (aropen_id=:aropen_id) "
             "GROUP BY aropen_cust_id, aropen_docnumber, aropen_docdate,"
             "         aropen_amount, aropen_paid, aropen_curr_id, prepared, cashapplied;" );
  applypopulate.bindValue(":aropen_id", _aropenid);
  applypopulate.exec();
  if (applypopulate.first())
  {
    _available->set(applypopulate.value("available").toDouble(),
		    applypopulate.value("aropen_curr_id").toInt(),
		    applypopulate.value("aropen_docdate").toDate(), false);
    _cust->setId(applypopulate.value("aropen_cust_id").toInt());
    _applied->setLocalValue(applypopulate.value("f_applied").toDouble());
    _balance->setLocalValue(_available->localValue() - _applied->localValue());
    _docNumber->setText(applypopulate.value("aropen_docnumber").toString());
    _docDate->setDate(applypopulate.value("aropen_docdate").toDate(), true);
  }
  else
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving A/R CM Information"),
                                applypopulate, __FILE__, __LINE__);

  MetaSQLQuery mql = mqlLoad("arOpenApplications", "detail");
  ParameterList params;
  params.append("cust_id",          _cust->id());
  params.append("debitMemo",        tr("Debit Memo"));
  params.append("invoice",          tr("Invoice"));
  params.append("source_aropen_id", _aropenid);
  applypopulate = mql.toQuery(params);
  _aropen->populate(applypopulate);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving A/R CM Information"),
                                applypopulate, __FILE__, __LINE__))
  {
    return;
  }
}

void applyARCreditMemo::sPriceGroup()
{
  if (! omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_available->currAbbr()));
}

void applyARCreditMemo::sSearchDocNumChanged(const QString &pTarget)
{
  _aropen->clearSelection();

  if (pTarget.isEmpty())
    return;

  int i;
  for (i = 0; i < _aropen->topLevelItemCount(); i++)
  {
    if (_aropen->topLevelItem(i)->text(1).startsWith(pTarget))
      break;
  }

  if (i < _aropen->topLevelItemCount())
  {
    _aropen->setCurrentItem(_aropen->topLevelItem(i));
    _aropen->scrollToItem(_aropen->topLevelItem(i));
  }
}
