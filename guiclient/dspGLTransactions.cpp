/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspGLTransactions.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "glTransactionDetail.h"
#include "dspGLSeries.h"
#include "invoice.h"
#include "purchaseOrder.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "dspShipmentsByShipment.h"
#include "apOpenItem.h"
#include "arOpenItem.h"
#include "salesOrder.h"
#include "dspWoHistoryByNumber.h"
#include "transactionInformation.h"

dspGLTransactions::dspGLTransactions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _gltrans->addColumn(tr("Date"),      _dateColumn,    Qt::AlignCenter, true, "gltrans_date");
  _gltrans->addColumn(tr("Source"),    _orderColumn,   Qt::AlignCenter, true, "gltrans_source");
  _gltrans->addColumn(tr("Doc. Type"), _docTypeColumn, Qt::AlignCenter, true, "gltrans_doctype");
  _gltrans->addColumn(tr("Doc. #"),    _orderColumn,   Qt::AlignCenter, true, "docnumber");
  _gltrans->addColumn(tr("Reference"), -1,             Qt::AlignLeft,   true, "notes");
  _gltrans->addColumn(tr("Account"),   _itemColumn,    Qt::AlignLeft,   true, "account");
  _gltrans->addColumn(tr("Debit"),     _moneyColumn,   Qt::AlignRight,  true, "debit");
  _gltrans->addColumn(tr("Credit"),    _moneyColumn,   Qt::AlignRight,  true, "credit");
  _gltrans->addColumn(tr("Posted"),    _ynColumn,      Qt::AlignCenter, true, "gltrans_posted");
  _gltrans->addColumn(tr("Username"),  _userColumn,    Qt::AlignLeft,   true, "gltrans_username");
  _gltrans->addColumn(tr("Running Total"), _moneyColumn, Qt::AlignRight,false,"running");

  _beginningBalance->setPrecision(omfgThis->moneyVal());

  _beginningBalanceLit->setVisible(_selectedAccount->isChecked() && _showRunningTotal->isChecked());
  _beginningBalance->setVisible(_selectedAccount->isChecked() && _showRunningTotal->isChecked());
  if (_selectedAccount->isChecked() && _showRunningTotal->isChecked())
    _gltrans->showColumn("running");
  else
    _gltrans->hideColumn("running");
}

dspGLTransactions::~dspGLTransactions()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspGLTransactions::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspGLTransactions::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
  {
    _selectedAccount->setChecked(TRUE);
    _account->setId(param.toInt());
  }

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("period_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT period_start, period_end "
               "FROM period "
               "WHERE (period_id=:period_id);" );
    q.bindValue(":period_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _dates->setStartDate(q.value("period_start").toDate());
      _dates->setEndDate(q.value("period_end").toDate());
    }
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspGLTransactions::sPopulateMenu(QMenu * menuThis, QTreeWidgetItem* pItem)
{
  menuThis->insertItem(tr("View..."), this, SLOT(sViewTrans()), 0);
  menuThis->insertItem(tr("View GL Series..."), this, SLOT(sViewSeries()), 0);

  XTreeWidgetItem * item = (XTreeWidgetItem*)pItem;
  if(0 == item)
    return;

  if(item->rawValue("gltrans_doctype").toString() == "VO")
    menuThis->insertItem(tr("View Voucher..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "IN")
    menuThis->insertItem(tr("View Invoice..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "PO")
    menuThis->insertItem(tr("View Purchase Order..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "SH")
    menuThis->insertItem(tr("View Shipment..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "CM")
    menuThis->insertItem(tr("View Credit Memo..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "DM")
    menuThis->insertItem(tr("View Debit Memo..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "SO")
    menuThis->insertItem(tr("View Sales Order..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "WO")
    menuThis->insertItem(tr("View WO History..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_source").toString() == "I/M")
    menuThis->insertItem(tr("View Inventory History..."), this, SLOT(sViewDocument()));
}

bool dspGLTransactions::setParams(ParameterList &params)
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"),
      tr("You must specify a valid date range."));
    _dates->setFocus();
    return false;
  }

  _dates->appendValue(params);

  if (_selectedAccount->isChecked())
  {
    if (! _account->isValid())
    {
      QMessageBox::warning(this, tr("Invalid Account"),
        tr("You must specify a valid Account or select All Accounts."));
      _account->setFocus();
      return false;
    }
    params.append("accnt_id", _account->id());
    if (_showRunningTotal->isChecked())
    {
      double beginning = 0;
      QDate  periodStart = _dates->startDate();
      XSqlQuery begq;
      begq.prepare("SELECT "
                   "  CASE WHEN accnt_type IN ('A','E') THEN "
                   "    trialbal_beginning * -1 "
                   "  ELSE trialbal_beginning END AS trialbal_beginning,"
                   "  period_start "
                   "FROM trialbal "
                   "  JOIN accnt ON (trialbal_accnt_id=accnt_id), "
                   "  period "
                   "WHERE ((trialbal_period_id=period_id)"
                   "  AND  (trialbal_accnt_id=:accnt_id)"
                   "  AND  (:start BETWEEN period_start AND period_end));");
      begq.bindValue(":accnt_id", _account->id());
      begq.bindValue(":start",    _dates->startDate());
      begq.exec();
      if (begq.first())
      {
        beginning   = begq.value("trialbal_beginning").toDouble();
        periodStart = begq.value("period_start").toDate();
      }
      else if (begq.lastError().type() != QSqlError::None)
      {
	systemError(this, begq.lastError().databaseText(), __FILE__, __LINE__);
	return false;
      }
      XSqlQuery glq;
      glq.prepare("SELECT CASE WHEN accnt_type IN ('A','E') THEN "
                  "         COALESCE(SUM(gltrans_amount),0) * -1"
                  "       ELSE COALESCE(SUM(gltrans_amount),0) END AS glamount "
                  "FROM gltrans "
                  "  JOIN accnt ON (gltrans_accnt_id=accnt_id) "
                  "WHERE ((gltrans_date BETWEEN :periodstart AND date :querystart - interval '1 day')"
                  "  AND  (gltrans_accnt_id=:accnt_id)) "
                  "GROUP BY accnt_type;");
      glq.bindValue(":periodstart", periodStart);
      glq.bindValue(":querystart",  _dates->startDate());
      glq.bindValue(":accnt_id",    _account->id());
      glq.exec();
      if (glq.first())
        beginning   += glq.value("glamount").toDouble();
      else if (glq.lastError().type() != QSqlError::None)
      {
	systemError(this, glq.lastError().databaseText(), __FILE__, __LINE__);
	return false;
      }

      params.append("beginningBalance", beginning);
    }
  }

  if (_selectedSource->isChecked())
    params.append("source", _source->currentText());

  return true;
}

void dspGLTransactions::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("GLTransactions", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspGLTransactions::sFillList()
{
  MetaSQLQuery mql("SELECT gltrans.*,"
                   "       CASE WHEN(gltrans_docnumber='Misc.' AND"
                   "              invhist_docnumber IS NOT NULL) THEN"
                   "              (gltrans_docnumber || ' - ' || invhist_docnumber)"
                   "            ELSE gltrans_docnumber"
                   "       END AS docnumber,"
                   "       firstLine(gltrans_notes) AS notes,"
                   "       (formatGLAccount(accnt_id) || ' - ' || accnt_descrip) AS account,"
                   "       CASE WHEN (gltrans_amount < 0) THEN ABS(gltrans_amount)"
                   "            ELSE NULL"
                   "       END AS debit,"
                   "       CASE WHEN (gltrans_amount > 0) THEN gltrans_amount"
                   "            ELSE NULL"
                   "       END AS credit,"
                   "       CASE WHEN accnt_type IN ('A','E') THEN "
                   "         gltrans_amount * -1 "
                   "       ELSE gltrans_amount END AS running,"
                   "       'curr' AS debit_xtnumericrole,"
                   "       'curr' AS credit_xtnumericrole,"
                   "       'curr' AS running_xtnumericrole,"
                   "       0 AS running_xtrunningrole,"
                   "       <? value(\"beginningBalance\") ?> AS running_xtrunninginit "
                   "FROM gltrans JOIN accnt ON (gltrans_accnt_id=accnt_id) "
                   "     LEFT OUTER JOIN invhist ON (gltrans_misc_id=invhist_id"
                   "                            AND gltrans_docnumber='Misc.') "
                   "WHERE ((gltrans_date BETWEEN <? value(\"startDate\") ?>"
                   "                         AND <? value(\"endDate\") ?>)"
                   "<? if exists(\"accnt_id\") ?>"
                   " AND (gltrans_accnt_id=<? value(\"accnt_id\") ?>)"
                   "<? endif ?>"
                   "<? if exists(\"source\") ?>"
                   " AND (gltrans_source=<? value(\"source\") ?>)"
                   "<? endif ?>"
                   ") "
                   "ORDER BY gltrans_created"
                   "<? if not exists(\"beginningBalance\") ?> DESC <? endif ?>,"
                   "   gltrans_sequence, gltrans_amount;");
  ParameterList params;
  if (! setParams(params))
    return;

  _beginningBalanceLit->setVisible(_selectedAccount->isChecked() && _showRunningTotal->isChecked());
  _beginningBalance->setVisible(_selectedAccount->isChecked() && _showRunningTotal->isChecked());
  if (_selectedAccount->isChecked() && _showRunningTotal->isChecked())
  {
    _gltrans->showColumn("running");
    _beginningBalance->setDouble(params.value("beginningBalance").toDouble());
  }
  else
    _gltrans->hideColumn("running");

  q = mql.toQuery(params);
  _gltrans->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspGLTransactions::sViewTrans()
{
  ParameterList params;

  params.append("gltrans_id", _gltrans->id());

  glTransactionDetail newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspGLTransactions::sViewSeries()
{
  q.prepare("SELECT gltrans_date, gltrans_journalnumber"
            "  FROM gltrans"
            " WHERE (gltrans_id=:gltrans_id)");
  q.bindValue(":gltrans_id", _gltrans->id());
  q.exec();
  if(!q.first())
    return;

  ParameterList params;

  params.append("startDate", q.value("gltrans_date").toDate());
  params.append("endDate", q.value("gltrans_date").toDate());
  params.append("journalnumber", q.value("gltrans_journalnumber").toString());

  dspGLSeries *newdlg = new dspGLSeries();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspGLTransactions::sViewDocument()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_gltrans->currentItem();
  if(0 == item)
    return;

  ParameterList params;
  if(item->rawValue("gltrans_doctype").toString() == "VO")
  {
    q.prepare("SELECT vohead_id, vohead_misc"
              "  FROM vohead"
              " WHERE (vohead_number=:vohead_number)");
    q.bindValue(":vohead_number", item->rawValue("docnumber").toString());
    q.exec();
    if(!q.first())
      return;

    params.append("vohead_id", q.value("vohead_id").toInt());
    params.append("mode", "view");
    
    if(q.value("vohead_misc").toBool())
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }

  }
  else if(item->rawValue("gltrans_doctype").toString() == "IN")
  {
    q.prepare("SELECT invchead_id"
              "  FROM invchead"
              " WHERE (invchead_invcnumber=:invchead_invcnumber)");
    q.bindValue(":invchead_invcnumber", item->rawValue("docnumber").toString());
    q.exec();
    if(!q.first())
      return;

    invoice::viewInvoice(q.value("invchead_id").toInt());
  }
  else if(item->rawValue("gltrans_doctype").toString() == "PO")
  {
    q.prepare("SELECT pohead_id"
              "  FROM pohead"
              " WHERE (pohead_number=:pohead_number)");
    q.bindValue(":pohead_number", item->rawValue("docnumber").toString());
    q.exec();
    if(!q.first())
      return;

    params.append("pohead_id", q.value("pohead_id").toInt());
    params.append("mode", "view");

    purchaseOrder *newdlg = new purchaseOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(item->rawValue("gltrans_doctype").toString() == "SH")
  {
    q.prepare("SELECT shiphead_id"
              "  FROM shiphead"
              " WHERE (shiphead_number=:shiphead_number)");
    q.bindValue(":shiphead_number", item->rawValue("docnumber").toString());
    q.exec();
    if(!q.first())
      return;

    params.append("shiphead_id", q.value("shiphead_id").toInt());

    dspShipmentsByShipment *newdlg = new dspShipmentsByShipment();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if( (item->rawValue("gltrans_doctype").toString() == "CM") || (item->rawValue("gltrans_doctype").toString() == "DM") )
  {
    if(item->rawValue("gltrans_source").toString() == "A/P")
    {
      q.prepare("SELECT apopen_id"
                "  FROM apopen"
                " WHERE (apopen_docnumber=:docnumber)");
      q.bindValue(":docnumber", item->rawValue("docnumber").toString());
      q.exec();
      if(!q.first())
        return;

      params.append("mode", "view");
      params.append("apopen_id", q.value("apopen_id").toInt());
      apOpenItem newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();
    }
    else if(item->rawValue("gltrans_source").toString() == "A/R")
    {
      q.prepare("SELECT aropen_id"
                "  FROM aropen"
                " WHERE (aropen_docnumber=:docnumber)");
      q.bindValue(":docnumber", item->rawValue("docnumber").toString());
      q.exec();
      if(!q.first())
        return;

      params.append("mode", "view");
      params.append("aropen_id", q.value("aropen_id").toInt());
      arOpenItem newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();
    }
  }
  else if(item->rawValue("gltrans_doctype").toString() == "SO")
  {
    QStringList docnumber = item->rawValue("docnumber").toString().split("-");
    q.prepare("SELECT cohead_id"
              "  FROM cohead"
              " WHERE (cohead_number=:docnumber)");
    q.bindValue(":docnumber", docnumber[0]);
    q.exec();
    if(q.first())
      salesOrder::viewSalesOrder(q.value("cohead_id").toInt());
  }
  else if(item->rawValue("gltrans_doctype").toString() == "WO")
  {
    QStringList docnumber = item->rawValue("docnumber").toString().split("-");
    params.append("wo_number", docnumber[0]);

    dspWoHistoryByNumber *newdlg = new dspWoHistoryByNumber();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(item->rawValue("gltrans_source").toString() == "I/M")
  {
    q.prepare("SELECT gltrans_misc_id"
              "  FROM gltrans"
              " WHERE (gltrans_id=:gltrans_id)");
    q.bindValue(":gltrans_id", item->id());
    q.exec();
    if(!q.first())
      return;

    params.append("mode", "view");
    params.append("invhist_id", q.value("gltrans_misc_id").toInt());

    transactionInformation newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}
