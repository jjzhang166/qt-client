/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBankrecHistory.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>
#include "guiclient.h"

dspBankrecHistory::dspBankrecHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _bankaccnt->populate("SELECT bankaccnt_id,"
                       "       (bankaccnt_name || '-' || bankaccnt_descrip) "
                       "FROM bankaccnt "
                       "ORDER BY bankaccnt_name;");

  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sBankaccntChanged()));
  connect(_bankrec,   SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_print,      SIGNAL(clicked()), this, SLOT(sPrint()));

  _details->addColumn(tr("Date"),       _dateColumn, Qt::AlignCenter,true, "gltrans_date");
  _details->addColumn(tr("Doc Number/Notes"),    -1, Qt::AlignLeft,  true, "gltrans_docnumber");
  _details->addColumn(tr("Amount"), _bigMoneyColumn, Qt::AlignRight, true, "amount");

  sBankaccntChanged();
}

dspBankrecHistory::~dspBankrecHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBankrecHistory::languageChange()
{
  retranslateUi(this);
}

void dspBankrecHistory::sPrint()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id());
  params.append("bankrec_id", _bankrec->id());

  orReport report("BankrecHistory", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBankrecHistory::sBankaccntChanged()
{
  XSqlQuery bq;
  bq.prepare( "SELECT bankrec_id, (formatDate(bankrec_opendate) || '-' || formatDate(bankrec_enddate)) "
             "FROM bankrec "
             "WHERE (bankrec_bankaccnt_id=:bankaccnt_id) "
             "ORDER BY bankrec_opendate, bankrec_enddate; ");
  bq.bindValue(":bankaccnt_id", _bankaccnt->id());
  bq.exec();
  _bankrec->populate(bq);

  sFillList();
}

void dspBankrecHistory::sFillList()
{
  q.prepare( "SELECT bankrec_username, bankrec_created,"
             "       bankrec_opendate,"
             "       bankrec_enddate,"
             "       bankrec_openbal,"
             "       bankrec_endbal "
             "FROM bankrec "
             "WHERE (bankrec_id=:bankrecid);" );
  q.bindValue(":bankrecid", _bankrec->id());
  q.exec();
  if(q.first())
  {
    _poster->setText(q.value("bankrec_username").toString());
    _postdate->setDate(q.value("bankrec_created").toDate());

    XSqlQuery brq;
    brq.prepare("SELECT -1 AS gltrans_id, 0 AS seq,"
                "       CAST(:opendate AS DATE) AS gltrans_date,"
                "       :opening AS gltrans_docnumber, :openbal AS amount,"
                "       'curr' AS amount_xtnumericrole "
                "UNION "
                "SELECT -1 AS gltrans_id, 2 AS seq,"
                "       CAST(:enddate AS DATE) AS gltrans_date,"
                "       :ending AS gltrans_docnumber, :endbal AS amount,"
                "       'curr' AS amount_xtnumericrole "
                "UNION "
                "SELECT gltrans_id, 1 AS seq, gltrans_date,"
                "       gltrans_docnumber,"
                "       (currtolocal(bankaccnt_curr_id,gltrans_amount,gltrans_date) * -1) AS amount,"
                "       'curr' AS amount_xtnumericrole "
                "FROM gltrans, bankrecitem "
                "  JOIN bankrec ON (bankrecitem_bankrec_id=bankrec_id) "
                "  JOIN bankaccnt ON (bankaccnt_id=bankrec_bankaccnt_id) "
                "WHERE ((bankrecitem_bankrec_id=:bankrecid)"
                "  AND (bankrecitem_source='GL')"
                "  AND (bankrecitem_source_id=gltrans_id) ) "
                "ORDER BY seq, gltrans_date, gltrans_id;" );
    brq.bindValue(":bankrecid", _bankrec->id());
    brq.bindValue(":opendate", q.value("bankrec_opendate").toDate());
    brq.bindValue(":openbal",  q.value("bankrec_openbal").toDouble());
    brq.bindValue(":opening",  tr("Opening Balance"));
    brq.bindValue(":enddate",  q.value("bankrec_enddate").toDate());
    brq.bindValue(":endbal",   q.value("bankrec_endbal").toDouble());
    brq.bindValue(":ending",   tr("Ending Balance"));
    brq.exec();
    _details->populate(brq);
    if (brq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, brq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
