/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "reconcileBankaccount.h"

#include <QApplication>
#include <QCursor>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <parameter.h>

#include "bankAdjustment.h"
#include "storedProcErrorLookup.h"

reconcileBankaccount::reconcileBankaccount(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    connect(_addAdjustment, SIGNAL(clicked()), this, SLOT(sAddAdjustment()));
    connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sBankaccntChanged()));
    connect(_cancel,	SIGNAL(clicked()),  this, SLOT(sCancel()));
    connect(_checks,   SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(sChecksToggleCleared()));
    connect(_endBal,	SIGNAL(lostFocus()), this, SLOT(populate()));
    connect(_openBal,	SIGNAL(lostFocus()), this, SLOT(populate()));
    connect(_receipts,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(sReceiptsToggleCleared()));
    connect(_reconcile,	SIGNAL(clicked()), this, SLOT(sReconcile()));
    connect(_save,	SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_update,	SIGNAL(clicked()), this, SLOT(populate()));

    _receipts->addColumn(tr("Cleared"),   _ynColumn * 2, Qt::AlignCenter );
    _receipts->addColumn(tr("Date"),        _dateColumn, Qt::AlignCenter );
    _receipts->addColumn(tr("Doc. Number"), _itemColumn, Qt::AlignLeft   );
    _receipts->addColumn(tr("Notes"),                -1, Qt::AlignLeft   );
    _receipts->addColumn(tr("Amount"),  _bigMoneyColumn, Qt::AlignRight  );
    
    _checks->addColumn(tr("Cleared"),   _ynColumn * 2, Qt::AlignCenter );
    _checks->addColumn(tr("Date"),        _dateColumn, Qt::AlignCenter );
    _checks->addColumn(tr("Doc. Number"), _itemColumn, Qt::AlignLeft   );
    _checks->addColumn(tr("Notes"),                -1, Qt::AlignLeft   );
    _checks->addColumn(tr("Amount"),  _bigMoneyColumn, Qt::AlignRight  );
    
    _bankrecid = -1;	// do this before _bankaccnt->populate()
    
    _bankaccnt->populate("SELECT bankaccnt_id,"
			 "       (bankaccnt_name || '-' || bankaccnt_descrip) "
			 "FROM bankaccnt "
			 "ORDER BY bankaccnt_name;");
    _currency->setLabel(_currencyLit);

    if (!_privileges->check("MaintainBankAdjustments"))
      _addAdjustment->setEnabled(FALSE);

    connect(omfgThis, SIGNAL(bankAdjustmentsUpdated(int, bool)), this, SLOT(populate()));
    connect(omfgThis, SIGNAL(checksUpdated(int, int, bool)), this, SLOT(populate()));
    connect(omfgThis, SIGNAL(cashReceiptsUpdated(int, bool)), this, SLOT(populate()));
}

reconcileBankaccount::~reconcileBankaccount()
{
    // no need to delete child widgets, Qt does it all for us
}

void reconcileBankaccount::languageChange()
{
    retranslateUi(this);
}

void reconcileBankaccount::sCancel()
{
  if(_bankrecid != -1)
  {
    q.prepare("SELECT count(*) AS num"
	      "  FROM bankrec"
	      " WHERE (bankrec_id=:bankrecid); ");
    q.bindValue(":bankrecid", _bankrecid);
    q.exec();
    if (q.first() && q.value("num").toInt() > 0)
    {
      if (QMessageBox::question(this, tr("Cancel Bank Reconciliation?"),
				tr("<p>Are you sure you want to Cancel this Bank "
				   "Reconciliation and delete all of the Cleared "
				   "notations for this time period?"),
				 QMessageBox::Yes,
				 QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
	return;
      }

      q.prepare( "SELECT deleteBankReconciliation(:bankrecid) AS result;" );
      q.bindValue(":bankrecid", _bankrecid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("deleteBankReconciliation", result),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }
  close();
}

bool reconcileBankaccount::sSave(bool closeWhenDone)
{
  q.prepare("SELECT count(*) AS num"
            "  FROM bankrec"
            " WHERE (bankrec_id=:bankrecid); ");
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.first() && q.value("num").toInt() > 0)
    q.prepare("UPDATE bankrec"
              "   SET bankrec_bankaccnt_id=:bankaccntid,"
              "       bankrec_opendate=:startDate,"
              "       bankrec_enddate=:endDate,"
              "       bankrec_openbal=:openbal,"
              "       bankrec_endbal=:endbal "
              " WHERE (bankrec_id=:bankrecid); ");
  else if (q.value("num").toInt() == 0)
    q.prepare("INSERT INTO bankrec "
              "(bankrec_id, bankrec_bankaccnt_id,"
              " bankrec_opendate, bankrec_enddate,"
              " bankrec_openbal, bankrec_endbal) "
              "VALUES "
              "(:bankrecid, :bankaccntid,"
              " :startDate, :endDate,"
              " :openbal, :endbal); ");
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  q.bindValue(":bankrecid", _bankrecid);
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":startDate", _startDate->date());
  q.bindValue(":endDate", _endDate->date());
  q.bindValue(":openbal", _openBal->localValue());
  q.bindValue(":endbal", _endBal->localValue());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, tr("<p>There was an error creating records to reconcile "
			 "this account: <br><pre>%1</pre>")
			.arg(q.lastError().databaseText()), __FILE__, __LINE__);
    return false;
  }

  if (closeWhenDone)
    return close();

  return true;
}

void reconcileBankaccount::sReconcile()
{
  if(_bankrecid == -1)
  {
    QMessageBox::critical( this, tr("Cannot Reconcile Account"),
      tr("<p>There was an error trying to reconcile this account. "
         "Please contact your Systems Administrator.") );
    return;
  }

  if (!_startDate->isValid())
  {
    QMessageBox::warning( this, tr("Missing Opening Date"),
      tr("<p>No Opening Date was specified for this reconciliation. Please specify an Opening Date.") );
    _startDate->setFocus();
    return;
  }

  if (!_endDate->isValid())
  {
    QMessageBox::warning( this, tr("Missing Ending Date"),
      tr("<p>No Ending Date was specified for this reconciliation. Please specify an Ending Date.") );
    _endDate->setFocus();
    return;
  }

  double begBal = _openBal->localValue();
  double endBal = _endBal->localValue();

  // calculate cleared balance
  q.prepare("SELECT round(:endBal - (:begBal + COALESCE(SUM(amount),0.0)), 2) AS diff_value"
            "  FROM ( SELECT currToLocal(bankaccnt_curr_id, gltrans_amount * -1, gltrans_date) AS amount"
            "           FROM bankaccnt, gltrans, bankrecitem"
            "          WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "            AND (bankrecitem_source='GL')"
            "            AND (bankrecitem_source_id=gltrans_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (NOT gltrans_rec)"
            "            AND (bankaccnt_id=:bankaccntid) ) "
            "          UNION ALL"
            "         SELECT CASE WHEN(bankadjtype_iscredit=true) THEN (bankadj_amount * -1) ELSE bankadj_amount END AS amount"
            "           FROM bankadj, bankadjtype, bankrecitem"
            "          WHERE ( (bankrecitem_source='AD')"
            "            AND (bankrecitem_source_id=bankadj_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (bankadj_bankadjtype_id=bankadjtype_id)"
            "            AND (NOT bankadj_posted)"
            "            AND (bankadj_bankaccnt_id=:bankaccntid) ) ) AS data;");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.bindValue(":endBal", endBal);
  q.bindValue(":begBal", begBal);
  q.bindValue(":curr_id",   _currency->id());
  q.bindValue(":effective", _startDate->date());
  q.bindValue(":expires",   _endDate->date());
  q.exec();
  if(!q.exec() || !q.first())
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(q.value("diff_value").toDouble() != 0.0)
  {
    QMessageBox::critical( this, tr("Balances Do Not Match"),
      tr("The cleared amounts do not balance with the specified\n"
         "beginning and ending balances.\n"
         "Please correct this before continuing.") );
    return;
  }

  if (! sSave(false))
    return;

  q.prepare("SELECT postBankReconciliation(:bankrecid) AS result;");
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("postBankReconciliation", result),
		  __FILE__, __LINE__);
      return;
    }
    _bankrecid = -1;
    close();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

/* 
   Note that the SELECTs here are UNIONs of the gltrans table (in the base
   currency) and the bankadj table (in the bank account's currency).
*/
void reconcileBankaccount::populate()
{
  qApp->setOverrideCursor(Qt::waitCursor);

  double begBal = _openBal->localValue();
  double endBal = _endBal->localValue();

  int currid = -1;

  // fill receipts list
  currid = _receipts->id();
  _receipts->clear();
  q.prepare("SELECT gltrans_id AS id, 1 AS altid,"
            "       jrnluse_use AS use, gltrans_journalnumber AS jrnlnum,"
            "       formatDate(COALESCE(date(jrnluse_date), gltrans_date)) AS f_jrnldate,"
            "       COALESCE(bankrecitem_cleared, FALSE) AS cleared,"
            "       formatDate(gltrans_date) AS f_date,"
            "       gltrans_docnumber AS docnumber,"
            "       gltrans_notes AS notes,"
            "       formatMoney(currToLocal(bankaccnt_curr_id, gltrans_amount, gltrans_date) * -1) AS f_amount,"
            "       currToLocal(bankaccnt_curr_id, gltrans_amount, gltrans_date) * -1 AS amount,"
            "       COALESCE(date(jrnluse_date), gltrans_date) AS jrnldate,"
            "       gltrans_date AS sortdate "
            "  FROM (bankaccnt CROSS JOIN gltrans) LEFT OUTER JOIN bankrecitem "
            "    ON ((bankrecitem_source='GL') AND (bankrecitem_source_id=gltrans_id)"
            "        AND (bankrecitem_bankrec_id=:bankrecid)) "
            "       LEFT OUTER JOIN jrnluse ON (jrnluse_number=gltrans_journalnumber AND jrnluse_use='C/R')"
            " WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "   AND (NOT gltrans_rec)"
            "   AND (gltrans_amount < 0)"
            "   AND (bankaccnt_id=:bankaccntid) ) "
            " UNION ALL "
            "SELECT bankadj_id AS id, 2 AS altid,"
            "       '' AS use, NULL AS jrnlnum, formatDate(bankadj_date) AS f_jrnldate,"
            "       COALESCE(bankrecitem_cleared, FALSE) AS cleared,"
            "       formatDate(bankadj_date) AS f_date,"
            "       bankadj_docnumber AS docnumber,"
            "       bankadjtype_name AS notes,"
            "       formatMoney(CASE WHEN(bankadjtype_iscredit=true) THEN (bankadj_amount * -1) ELSE bankadj_amount END) AS f_amount,"
            "       (CASE WHEN(bankadjtype_iscredit=true) THEN (bankadj_amount * -1) ELSE bankadj_amount END) AS amount,"
            "       bankadj_date AS jrnldate,"
            "       bankadj_date AS sortdate "
            "  FROM (bankadjtype CROSS JOIN bankadj) "
            "               LEFT OUTER JOIN bankrecitem ON ((bankrecitem_source='AD') "
            "                 AND (bankrecitem_source_id=bankadj_id) "
            "                 AND (bankrecitem_bankrec_id=:bankrecid)) "
            " WHERE ( (((bankadjtype_iscredit=false) AND (bankadj_amount > 0)) OR ((bankadjtype_iscredit=true) AND (bankadj_amount < 0))) "
            "   AND (bankadj_bankadjtype_id=bankadjtype_id) "
            "   AND (NOT bankadj_posted) "
            "   AND (bankadj_bankaccnt_id=:bankaccntid) ) "
            "ORDER BY jrnldate, jrnlnum, sortdate; ");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  int jrnlnum = 0;
  XTreeWidgetItem * parent = 0;
  XTreeWidgetItem * lastChild = 0;
  XTreeWidgetItem * last = 0;
  bool cleared = TRUE;
  double amount = 0.0;
  bool amountNull = true;
  while (q.next())
  {
    if(q.value("use").toString() == "C/R")
    {
      if(q.value("jrnlnum").toInt() != jrnlnum || (0 == parent))
      {
        if(parent != 0)
        {
          parent->setText(0, (cleared ? tr("Yes") : tr("No")));
          parent->setText(4, amountNull ? tr("?????") : formatMoney(amount));
        }
        jrnlnum = q.value("jrnlnum").toInt();
        last = new XTreeWidgetItem( _receipts, last,
          jrnlnum, 3, "", q.value("f_jrnldate"), q.value("jrnlnum"));
        parent = last;
        cleared = true;
        amount = 0.0;
	amountNull = true;
        lastChild = 0;
      }
      cleared = (cleared && q.value("cleared").toBool());
      amount += q.value("amount").toDouble();
      amountNull = q.value("amount").isNull();
      
      lastChild = new XTreeWidgetItem( parent, lastChild,
        q.value("id").toInt(), q.value("altid").toInt(),
        (q.value("cleared").toBool() ? tr("Yes") : tr("No")),
        q.value("f_date"), q.value("docnumber"),
        q.value("notes"),
	q.value("f_amount").isNull() ? tr("?????") : q.value("f_amount") );
    }
    else
    {
      if(parent != 0)
      {
        parent->setText(0, (cleared ? tr("Yes") : tr("No")));
        parent->setText(4, formatMoney(amount));
      }
      parent = 0;
      cleared = true;
      amount = 0.0;
      amountNull = true;
      lastChild = 0;
      last = new XTreeWidgetItem( _receipts, last,
        q.value("id").toInt(), q.value("altid").toInt(),
        (q.value("cleared").toBool() ? tr("Yes") : tr("No")),
        q.value("f_date"), q.value("docnumber"),
        q.value("notes"),
	q.value("f_amount").isNull() ? tr("?????") : q.value("f_amount") );
    }
  }
  if(parent != 0)
  {
    parent->setText(0, (cleared ? tr("Yes") : tr("No")));
    parent->setText(4, amountNull ? tr("?????") : formatMoney(amount));
  }
  if(currid != -1)
    _receipts->setCurrentItem(_receipts->topLevelItem(currid));
  if(_receipts->currentItem())
    _receipts->scrollToItem(_receipts->currentItem());


  // fill receipts cleared value
  q.prepare("SELECT formatMoney(COALESCE(SUM(amount),0.0)) AS cleared_amount"
            "  FROM ( SELECT currToLocal(bankaccnt_curr_id, gltrans_amount * -1, gltrans_date) AS amount"
            "           FROM bankaccnt, gltrans, bankrecitem"
            "          WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "            AND (bankrecitem_source='GL')"
            "            AND (bankrecitem_source_id=gltrans_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (NOT gltrans_rec)"
            "            AND (gltrans_amount < 0)"
            "            AND (bankaccnt_id=:bankaccntid) ) "
            "          UNION ALL"
            "         SELECT CASE WHEN(bankadjtype_iscredit=true) THEN (bankadj_amount * -1) ELSE bankadj_amount END AS amount"
            "           FROM bankrecitem, bankadj, bankadjtype "
            "          WHERE ( (bankrecitem_source='AD')"
            "            AND (bankrecitem_source_id=bankadj_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (bankadj_bankadjtype_id=bankadjtype_id)"
            "            AND (NOT bankadj_posted)"
            "            AND (((bankadjtype_iscredit=false) AND (bankadj_amount > 0)) OR (bankadjtype_iscredit=true AND (bankadj_amount < 0))) "
            "            AND (bankadj_bankaccnt_id=:bankaccntid) ) ) AS data;");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.first())
    _clearedReceipts->setText(q.value("cleared_amount").toString());
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // fill checks list
  currid = _checks->id();
  _checks->clear();
  q.prepare("SELECT gltrans_id AS id, 1 AS altid,"
            "       formatBoolYN(COALESCE(bankrecitem_cleared, FALSE)) AS f_cleared,"
            "       formatDate(gltrans_date) AS f_date,"
            "       gltrans_docnumber AS docnumber,"
            "       gltrans_notes AS notes,"
            "       formatMoney(currToLocal(bankaccnt_curr_id, gltrans_amount, gltrans_date)) AS f_amount,"
            "       gltrans_date AS sortdate "
            "  FROM (bankaccnt CROSS JOIN gltrans) LEFT OUTER JOIN bankrecitem "
            "    ON ((bankrecitem_source='GL') AND (bankrecitem_source_id=gltrans_id)"
            "        AND (bankrecitem_bankrec_id=:bankrecid)) "
            " WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "   AND (NOT gltrans_rec)"
            "   AND (gltrans_amount > 0)"
            "   AND (bankaccnt_id=:bankaccntid) ) "
            " UNION ALL "
            "SELECT bankadj_id AS id, 2 AS altid,"
            "       formatBoolYN(COALESCE(bankrecitem_cleared, FALSE)) AS f_cleared,"
            "       formatDate(bankadj_date) AS f_date,"
            "       bankadj_docnumber AS docnumber,"
            "       bankadjtype_name AS notes,"
            "       formatMoney(CASE WHEN(bankadjtype_iscredit=false) THEN (bankadj_amount * -1) ELSE bankadj_amount END) AS f_amount,"
            "       bankadj_date AS sortdate "
            "  FROM (bankadjtype CROSS JOIN bankadj) "
            "               LEFT OUTER JOIN bankrecitem ON ((bankrecitem_source='AD') "
            "                 AND (bankrecitem_source_id=bankadj_id) "
            "                 AND (bankrecitem_bankrec_id=:bankrecid)) "
            " WHERE ( (((bankadjtype_iscredit=true) AND (bankadj_amount > 0)) OR ((bankadjtype_iscredit=false) AND (bankadj_amount < 0))) "
            "   AND (bankadj_bankadjtype_id=bankadjtype_id) "
            "   AND (NOT bankadj_posted) "
            "   AND (bankadj_bankaccnt_id=:bankaccntid) ) "
            "ORDER BY sortdate; ");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  XTreeWidgetItem* lastCheckItem = 0;
  while (q.next())
  {
    lastCheckItem = new XTreeWidgetItem( _checks, lastCheckItem,
      q.value("id").toInt(), q.value("altid").toInt(),
      q.value("f_cleared"), q.value("f_date"), q.value("docnumber"),
      q.value("notes"),
      q.value("f_amount").isNull() ? tr("?????") : q.value("f_amount") );
  }
  if(currid != -1)
    _checks->setCurrentItem(_checks->topLevelItem(currid));
  if(_checks->currentItem())
    _checks->scrollToItem(_checks->currentItem());

  // fill checks cleared value
  q.prepare("SELECT formatMoney(COALESCE(SUM(amount),0.0)) AS cleared_amount"
            "  FROM ( SELECT currToLocal(bankaccnt_curr_id, gltrans_amount, gltrans_date) AS amount"
            "           FROM bankaccnt, gltrans, bankrecitem"
            "          WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "            AND (bankrecitem_source='GL')"
            "            AND (bankrecitem_source_id=gltrans_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (NOT gltrans_rec)"
            "            AND (gltrans_amount > 0)"
            "            AND (bankaccnt_id=:bankaccntid) ) "
            "          UNION ALL"
            "         SELECT CASE WHEN(bankadjtype_iscredit=false) THEN (bankadj_amount * -1) ELSE bankadj_amount END AS amount"
            "           FROM bankadj, bankadjtype, bankrecitem"
            "          WHERE ( (bankrecitem_source='AD')"
            "            AND (bankrecitem_source_id=bankadj_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (bankadj_bankadjtype_id=bankadjtype_id)"
            "            AND (NOT bankadj_posted)"
            "            AND (((bankadjtype_iscredit=true) AND (bankadj_amount > 0)) OR ((bankadjtype_iscredit=false) AND (bankadj_amount < 0)))"
            "            AND (bankadj_bankaccnt_id=:bankaccntid) ) ) AS data;");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.first())
    _clearedChecks->setText(q.value("cleared_amount").toString());
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // calculate cleared balance
  q.prepare("SELECT formatMoney(COALESCE(SUM(amount),0.0) + :begBal) AS cleared_amount,"
            "       formatMoney(:endBal) AS end_amount,"
            "       formatMoney(:endBal - (:begBal + (COALESCE(SUM(amount),0.0)))) AS diff_amount,"
            "       round(:endBal - (:begBal + COALESCE(SUM(amount),0.0)), 2) AS diff_value"
            "  FROM ( SELECT currToLocal(bankaccnt_curr_id, gltrans_amount * -1, gltrans_date) AS amount"
            "           FROM bankaccnt, gltrans, bankrecitem"
            "          WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "            AND (bankrecitem_source='GL')"
            "            AND (bankrecitem_source_id=gltrans_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (NOT gltrans_rec)"
            "            AND (bankaccnt_id=:bankaccntid) ) "
            "          UNION ALL"
            "         SELECT CASE WHEN(bankadjtype_iscredit=true) THEN (bankadj_amount * -1) ELSE bankadj_amount END AS amount"
            "           FROM bankadj, bankadjtype, bankrecitem"
            "          WHERE ( (bankrecitem_source='AD')"
            "            AND (bankrecitem_source_id=bankadj_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (bankadj_bankadjtype_id=bankadjtype_id)"
            "            AND (NOT bankadj_posted)"
            "            AND (bankadj_bankaccnt_id=:bankaccntid) ) ) AS data;");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.bindValue(":endBal", endBal);
  q.bindValue(":begBal", begBal);
  q.bindValue(":curr_id",   _currency->id());
  q.bindValue(":effective", _startDate->date());
  q.bindValue(":expires",   _endDate->date());
  q.exec();
  bool enableRec = FALSE;
  if(q.first())
  {
    _clearBal->setText(q.value("cleared_amount").toString());
    _endBal2->setText(q.value("end_amount").toString());
    _diffBal->setText(q.value("diff_amount").toString());
    if(q.value("diff_value").toDouble() == 0.0)
    {
      _diffBal->setPaletteForegroundColor(QColor("black"));
      if(_startDate->isValid() && _endDate->isValid())
        enableRec = TRUE;
    }
    else
      _diffBal->setPaletteForegroundColor(QColor("red"));
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  //_reconcile->setEnabled(enableRec);

  qApp->restoreOverrideCursor();
}

void reconcileBankaccount::sAddAdjustment()
{
  ParameterList params;
  params.append("mode", "new");

  params.append("bankaccnt_id", _bankaccnt->id());

  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void reconcileBankaccount::sReceiptsToggleCleared()
{
  XTreeWidgetItem *item = (XTreeWidgetItem*)_receipts->currentItem();
  XTreeWidgetItem *child = 0;
  bool setto = true;

  if(0 == item)
    return;

  _receipts->scrollToItem(item);
  if(item->altId() == 3)
  {
    setto = item->text(0) == tr("No");
    for (int i = 0; i < item->childCount(); i++)
    {
      child = item->child(i);
      if(child->text(0) != (setto ? tr("Yes") : tr("No")))
      {
        q.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid) AS cleared");
        q.bindValue(":bankrecid", _bankrecid);
        q.bindValue(":sourceid", child->id());
        if(child->altId()==1)
          q.bindValue(":source", "GL");
        else if(child->altId()==2)
          q.bindValue(":source", "AD");
        q.exec();
        if(q.first())
          child->setText(0, (q.value("cleared").toBool() ? tr("Yes") : tr("No") ));
	else if (q.lastError().type() != QSqlError::None)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
    }
    item->setText(0, (setto ? tr("Yes") : tr("No")));
  }
  else
  {
    q.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid) AS cleared");
    q.bindValue(":bankrecid", _bankrecid);
    q.bindValue(":sourceid", item->id());
    if(item->altId()==1)
      q.bindValue(":source", "GL");
    else if(item->altId()==2)
      q.bindValue(":source", "AD");
    q.exec();
    if(q.first())
    {
      item->setText(0, (q.value("cleared").toBool() ? tr("Yes") : tr("No") ));

      item = (XTreeWidgetItem*)item->parent();
      if(item != 0 && item->altId() == 3)
      {
        setto = true;
	for (int i = 0; i < item->childCount(); i++)
        {
          setto = (setto && (item->child(i)->text(0) == tr("Yes")));
        }
        item->setText(0, (setto ? tr("Yes") : tr("No")));
      }
    }
    else
    {
      populate();
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }
}

void reconcileBankaccount::sChecksToggleCleared()
{
  XTreeWidgetItem *item = (XTreeWidgetItem*)_checks->currentItem();

  if(0 == item)
    return;

  _checks->scrollToItem(item);

  q.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid) AS cleared");
  q.bindValue(":bankrecid", _bankrecid);
  q.bindValue(":sourceid", item->id());
  if(item->altId()==1)
    q.bindValue(":source", "GL");
  else if(item->altId()==2)
    q.bindValue(":source", "AD");
  q.exec();
  if(q.first())
    item->setText(0, (q.value("cleared").toBool() ? tr("Yes") : tr("No") ));
  else
  {
    populate();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void reconcileBankaccount::sBankaccntChanged()
{
  XSqlQuery accntq;
  accntq.prepare("SELECT bankaccnt_curr_id "
            "FROM bankaccnt WHERE bankaccnt_id = :accntId;");
  accntq.bindValue(":accntId", _bankaccnt->id());
  accntq.exec();
  if (accntq.first())
    _currency->setId(accntq.value("bankaccnt_curr_id").toInt());
  else if (accntq.lastError().type() != QSqlError::None)
  {
    systemError(this, accntq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accntq.prepare("SELECT * FROM bankrec "
		 "WHERE ((bankrec_bankaccnt_id=:accntId)"
		 "  AND  (NOT bankrec_posted));");
  accntq.bindValue(":accntId", _bankaccnt->id());
  accntq.exec();
  if (accntq.first())
  {
    _bankrecid = accntq.value("bankrec_id").toInt();
    _startDate->setDate(accntq.value("bankrec_opendate").toDate());
    _endDate->setDate(accntq.value("bankrec_enddate").toDate());
    _openBal->setLocalValue(accntq.value("bankrec_openbal").toDouble());
    _endBal->setLocalValue(accntq.value("bankrec_endbal").toDouble());
  }
  else if (accntq.lastError().type() != QSqlError::None)
  {
    systemError(this, accntq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    accntq.prepare("SELECT NEXTVAL('bankrec_bankrec_id_seq') AS bankrec_id");
    accntq.exec();
    if (accntq.first())
      _bankrecid = accntq.value("bankrec_id").toInt();
    else if (accntq.lastError().type() != QSqlError::None)
    {
      systemError(this, accntq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  populate();
}
