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

#include "selectedPayments.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "selectPayment.h"
#include "storedProcErrorLookup.h"

selectedPayments::selectedPayments(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_selectedBankAccount, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  QString base;
  q.exec("SELECT currConcat(baseCurrID()) AS base;");
  if (q.first())
    base = q.value("base").toString();

  _apselect->addColumn(tr("Bank Accnt."), _itemColumn, Qt::AlignLeft  );
  _apselect->addColumn(tr("Vendor"),      -1,          Qt::AlignLeft  );
  _apselect->addColumn(tr("Doc. Type"),  _orderColumn, Qt::AlignRight );
  _apselect->addColumn(tr("Doc. #"),     _orderColumn, Qt::AlignRight );
  _apselect->addColumn(tr("Inv. #"),     _orderColumn, Qt::AlignRight );
  _apselect->addColumn(tr("P/O #"),      _orderColumn, Qt::AlignRight );
  _apselect->addColumn(tr("Selected"),   _moneyColumn, Qt::AlignRight );
  _apselect->addColumn(tr("Currency"),   _currencyColumn, Qt::AlignLeft );
  _apselect->addColumn(tr("Running\n(%1)").arg(base), _moneyColumn, Qt::AlignRight );
  if (omfgThis->singleCurrency())
  {
    _apselect->hideColumn(7);
    _apselect->headerItem()->setText(8, tr("Running"));
  }

  connect(omfgThis, SIGNAL(paymentsUpdated(int, int, bool)), this, SLOT(sFillList(int)));

  sFillList();
}

selectedPayments::~selectedPayments()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectedPayments::languageChange()
{
  retranslateUi(this);
}

void selectedPayments::setParams(ParameterList & params)
{
  if (_selectedBankAccount->isChecked())
    params.append("bankaccntid", _bankaccnt->id());
}

void selectedPayments::sPrint()
{
  orReport report("SelectedPaymentsList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void selectedPayments::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("apopen_id", _apselect->id());

  selectPayment newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void selectedPayments::sClear()
{
  q.prepare("SELECT clearPayment(:apselect_id) AS result;");
  q.bindValue(":apselect_id", _apselect->altId());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("clearPayment", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sPaymentsUpdated(_bankaccnt->id(), _apselect->altId(), TRUE);
}

void selectedPayments::sFillList(int pBankaccntid)
{
  if ( (pBankaccntid == -1) || (pBankaccntid == _bankaccnt->id()) )
    sFillList();
}

void selectedPayments::sFillList()
{
  _apselect->clear();

  QString sql( "SELECT apopen_id, apselect_id,"
               "       (bankaccnt_name || '-' || bankaccnt_descrip) AS f_bank,"
               "       (vend_number || '-' || vend_name) AS f_vendor,"
               "       CASE WHEN (apopen_doctype='V') THEN :voucher"
               "            When (apopen_doctype='D') THEN :debitMemo"
               "       END AS doctype,"
               "       apopen_docnumber, apopen_ponumber, apselect_amount,"
	       "       apopen_invcnumber,"
	       "       currToBase(apselect_curr_id, apselect_amount, "
	       "		  CURRENT_DATE) AS apselect_amount_base, "
	       "       currConcat(apselect_curr_id) AS currAbbr "
               "FROM apopen, apselect, vend, bankaccnt "
               "WHERE ( (apopen_vend_id=vend_id)"
               "  AND   (apselect_apopen_id=apopen_id)"
               "  AND   (apselect_bankaccnt_id=bankaccnt_id)" 
	       "<? if exists(\"bankaccntid\") ?>"
	       "  AND   (bankaccnt_id=<? value(\"bankaccntid\") ?>)"
	       "<? endif ?>"
	       " ) "
	       "ORDER BY bankaccnt_name, vend_number, apopen_docnumber;" );
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  XTreeWidgetItem *last = 0;
  double running = 0;
  while (q.next())
  {
    running += q.value("apselect_amount_base").toDouble();

    last = new XTreeWidgetItem(_apselect, last,
			       q.value("apopen_id").toInt(),
			       q.value("apselect_id").toInt(),
			       q.value("f_bank"),
			       q.value("f_vendor"),
			       q.value("doctype"),
			       q.value("apopen_docnumber"),
			       q.value("apopen_invcnumber"),
			       q.value("apopen_ponumber"),
			       formatMoney(q.value("apselect_amount").toDouble()),
			       q.value("currAbbr"),
			       formatMoney(running) );
  }
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
