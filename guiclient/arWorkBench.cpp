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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "arWorkBench.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include <stdlib.h>
#include <metasql.h>

#include "arOpenItem.h"
#include "applyARCreditMemo.h"
#include "cashReceipt.h"
#include "creditMemo.h"
#include "creditcardprocessor.h"
#include "dspInvoiceInformation.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"

arWorkBench::arWorkBench(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_viewAropen, SIGNAL(clicked()), this, SLOT(sViewAropen()));
  connect(_editAropen, SIGNAL(clicked()), this, SLOT(sEditAropen()));
  connect(_applyAropenCM, SIGNAL(clicked()), this, SLOT(sApplyAropenCM()));
  connect(_editAropenCM, SIGNAL(clicked()), this, SLOT(sEditAropenCM()));
  connect(_viewAropenCM, SIGNAL(clicked()), this, SLOT(sViewAropenCM()));
  connect(_ccRefundCM,   SIGNAL(clicked()), this, SLOT(sCCRefundCM()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_newCashrcpt, SIGNAL(clicked()), this, SLOT(sNewCashrcpt()));
  connect(_editCashrcpt, SIGNAL(clicked()), this, SLOT(sEditCashrcpt()));
  connect(_viewCashrcpt, SIGNAL(clicked()), this, SLOT(sViewCashrcpt()));
  connect(_deleteCashrcpt, SIGNAL(clicked()), this, SLOT(sDeleteCashrcpt()));
  connect(_postCashrcpt, SIGNAL(clicked()), this, SLOT(sPostCashrcpt()));
  connect(_preauth, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(sgetCCAmount()));
  connect(_postPreauth, SIGNAL(clicked()), this, SLOT(sPostPreauth()));
  connect(_voidPreauth, SIGNAL(clicked()), this, SLOT(sVoidPreauth()));
  connect(_aropen, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)),
          this, SLOT(sPopulateAropenMenu(QMenu*)));
  connect(_aropenCM, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)),
          this, SLOT(sPopulateAropenCMMenu(QMenu*)));
  connect(_cashrcpt, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)),
          this, SLOT(sPopulateCashRcptMenu(QMenu*)));
  connect(_preauth, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)),
          this, SLOT(sPopulatePreauthMenu(QMenu*)));

  _cashrcpt->addColumn(tr("Dist. Date"), _dateColumn,     Qt::AlignCenter );
  _cashrcpt->addColumn(tr("Amount"),     _bigMoneyColumn, Qt::AlignRight  );
  _cashrcpt->addColumn(tr("Currency"),   _currencyColumn, Qt::AlignLeft   );
  if (_privleges->check("MaintainCashReceipts"))
  {
    connect(_cashrcpt, SIGNAL(valid(bool)), _editCashrcpt, SLOT(setEnabled(bool)));
    connect(_cashrcpt, SIGNAL(valid(bool)), _deleteCashrcpt, SLOT(setEnabled(bool)));
    connect(_cashrcpt, SIGNAL(valid(bool)), _postCashrcpt, SLOT(setEnabled(bool)));
  }
  else
  {
    _newCashrcpt->setEnabled(FALSE);
    connect(_cashrcpt, SIGNAL(itemSelected(int)), _viewCashrcpt, SLOT(animateClick()));
  }
  if(_privleges->check("PostCashReceipts"))
    connect(_cashrcpt, SIGNAL(itemSelected(int)), _editCashrcpt, SLOT(animateClick()));
  connect(omfgThis, SIGNAL(cashReceiptsUpdated(int, bool)), this, SLOT(sFillList()));
                                                                       
  _aropenCM->addColumn( tr("Type"),     _ynColumn,    Qt::AlignCenter );
  _aropenCM->addColumn( tr("Doc. #"),   _itemColumn,  Qt::AlignCenter );
  _aropenCM->addColumn( tr("Amount"),   _moneyColumn, Qt::AlignRight  );
  _aropenCM->addColumn( tr("Applied"),  _moneyColumn, Qt::AlignRight  );
  _aropenCM->addColumn( tr("Balance"),  _moneyColumn, Qt::AlignRight  );
  _aropenCM->addColumn( tr("Currency"), _currencyColumn, Qt::AlignLeft );
  
  _aropen->addColumn(tr("Doc. Type"), -1,              Qt::AlignCenter );
  _aropen->addColumn(tr("Doc. #"),    _orderColumn,    Qt::AlignRight  );
  _aropen->addColumn(tr("Order #"),   _orderColumn,    Qt::AlignRight  );
  _aropen->addColumn(tr("Doc. Date"), _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Due Date"),  _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Amount"),    _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Paid"),      _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Balance"),   _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn( tr("Currency"), _currencyColumn, Qt::AlignLeft   );
  
  _preauth->addColumn(tr("Order-Seq."),  150, Qt::AlignRight  );
  _preauth->addColumn(tr("Amount"),   _bigMoneyColumn, Qt::AlignRight  );
  _preauth->addColumn(tr("Currency"), _currencyColumn, Qt::AlignLeft   );

  if(_privleges->check("EditAROpenItem"))
  {
    connect(_aropen, SIGNAL(valid(bool)), _editAropen, SLOT(setEnabled(bool)));
    connect(_aropen, SIGNAL(itemSelected(int)), _editAropen, SLOT(animateClick()));
    connect(_aropenCM, SIGNAL(valid(bool)), _editAropenCM, SLOT(setEnabled(bool)));
    connect(_aropenCM, SIGNAL(itemSelected(int)), _editAropenCM, SLOT(animateClick()));
  }
  else
  {
    connect(_aropen, SIGNAL(itemSelected(int)), _viewAropen, SLOT(animateClick()));
    connect(_aropenCM, SIGNAL(itemSelected(int)), _viewAropenCM, SLOT(animateClick()));
  }
  if (_privleges->check("ApplyARMemos"))
    connect(_aropenCM, SIGNAL(valid(bool)), _applyAropenCM, SLOT(setEnabled(bool)));

  if (omfgThis->singleCurrency())
  {
    _cashrcpt->hideColumn(2);
    _preauth->hideColumn(2);
    _aropen->hideColumn(8);
    _aropenCM->hideColumn(4);
  }

  if (_metrics->boolean("CCAccept") && _privleges->check("ProcessCreditCards"))
    connect(_aropenCM, SIGNAL(valid(bool)), _ccRefundCM, SLOT(setEnabled(bool)));
  else
  {
    _ccRefundCM->setEnabled(false);
    _preauthLit->setEnabled(false);
    _preauth->setEnabled(false);
    _postPreauth->setEnabled(false);
    _CCAmountLit->setEnabled(false);
    _CCAmount->setEnabled(false);
  }

  if(_metrics->boolean("EnableCustomerDeposits"))
    _aropenCMLit->setText(tr("A/R Open Credit Memos and Deposits"));
}

arWorkBench::~arWorkBench()
{
  // no need to delete child widgets, Qt does it all for us
}

void arWorkBench::languageChange()
{
  retranslateUi(this);
}

enum SetResponse arWorkBench::set( const ParameterList & pParams )
{
  QVariant param;
  bool    valid;
  
  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  return NoError;
}

void arWorkBench::sFillList()
{
  _CCAmount->clear();

  XSqlQuery currQ;
  currQ.prepare("SELECT cust_curr_id FROM cust WHERE (cust_id=:cust_id)");
  currQ.bindValue(":cust_id", _cust->id());
  currQ.exec();
  if (currQ.first())
    _aropenTotal->setId(currQ.value("cust_curr_id").toInt());
  else if (currQ.lastError().type() != QSqlError::NoError)
    systemError(this, currQ.lastError().databaseText(), __FILE__, __LINE__);

  sFillCashrcptList();
  sFillAropenCMList();
  sFillAropenList();
  sFillPreauthList();
}

void arWorkBench::sFillAropenList()
{
  q.prepare("SELECT aropen_id,"
             "       CASE WHEN (aropen_doctype='D') THEN 0"
             "            WHEN (aropen_doctype='I') THEN 1"
             "            ELSE -1"
             "       END AS altid,"
             "       CASE WHEN (aropen_doctype='D') THEN :debitMemo"
             "            WHEN (aropen_doctype='I') THEN :invoice"
             "            ELSE :other"
             "       END AS f_doctype,"
             "       aropen_docnumber, aropen_ordernumber,"
             "       formatDate(aropen_docdate) AS f_docdate,"
             "       formatDate(aropen_duedate) AS f_duedate,"
             "       formatMoney(aropen_amount) AS f_amount,"
             "       formatMoney(aropen_paid) AS f_paid,"
             "       formatMoney(aropen_amount - aropen_paid) AS balance,"
             "       currConcat(aropen_curr_id) AS currAbbr "
             "FROM aropen "
             "WHERE ( (aropen_open)"
             " AND (NOT (aropen_doctype IN ('C', 'R'))) "
             " AND (aropen_cust_id=:cust_id) ) "
             "ORDER BY aropen_docdate;" );
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":debitMemo", tr("D/M"));
  q.bindValue(":invoice", tr("Invoice"));
  q.bindValue(":other", tr("Other"));
  q.exec();
  _aropen->clear();
  _aropen->populate(q, true);
  
  q.prepare("SELECT SUM(CASE WHEN (aropen_doctype IN ('C', 'R')) THEN"
            "           (currToBase(aropen_curr_id, aropen_amount - aropen_paid, aropen_docdate) * -1)"
            "      WHEN (aropen_doctype IN ('I', 'D')) THEN"
            "           currToBase(aropen_curr_id, aropen_amount - aropen_paid, aropen_docdate)"
            "      ELSE currToBase(aropen_curr_id, aropen_amount - aropen_paid, aropen_docdate)"
            "       END) AS total_balance "
            "FROM aropen "
            "WHERE ( (aropen_open)"
            " AND (aropen_cust_id=:cust_id) ) " );
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  if(q.first())
  {
    _aropenTotal->setBaseValue(q.value("total_balance").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}

void arWorkBench::sEditAropen()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void arWorkBench::sViewAropen()
{
  ParameterList params;
  XTreeWidgetItem * item = static_cast<XTreeWidgetItem*>(_aropen->currentItem());
  if(_aropen->altId() == 1 && item)
  {
    params.append("invoiceNumber", item->text(1));
    dspInvoiceInformation * newdlg = new dspInvoiceInformation();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
  {
    params.append("mode", "view");
    params.append("aropen_id", _aropen->id());
    arOpenItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void arWorkBench::sFillAropenCMList()
{
  q.prepare( "SELECT aropen_id,"
             "       CASE WHEN (aropen_doctype='C') THEN 0"
             "            WHEN (aropen_doctype='R') THEN 1"
             "            ELSE -1"
             "       END,"
             "       CASE WHEN(aropen_doctype='C') THEN :creditmemo"
             "            WHEN(aropen_doctype='R') THEN :cashdeposit"
             "            ELSE aropen_doctype"
             "       END,"
             "       aropen_docnumber,"
             "       formatMoney(aropen_amount),"
             "       formatMoney(aropen_paid + COALESCE(prepared,0.0)),"
             "       formatMoney(aropen_amount - aropen_paid - COALESCE(prepared,0.0)),"
             "       currConcat(aropen_curr_id) "
             "FROM aropen "
 	         "       LEFT OUTER JOIN (SELECT aropen_id AS prepared_aropen_id,"
             "                               SUM(currToCurr(checkitem_curr_id, aropen_curr_id, checkitem_amount + checkitem_discount, checkitem_docdate)) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN aropen ON (checkitem_aropen_id=aropen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void))"
             "                         GROUP BY aropen_id) AS sub1"
             "         ON (prepared_aropen_id=aropen_id)"
             "WHERE ( (aropen_doctype IN ('C', 'R'))"
             " AND (aropen_open)"
             " AND ((aropen_amount - aropen_paid - COALESCE(prepared,0.0)) > 0.0)"
             " AND (aropen_cust_id=:cust_id) ) "
             "ORDER BY aropen_docnumber;" );
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":creditmemo", tr("C/M"));
  q.bindValue(":cashdeposit", tr("C/D"));
  q.exec();
  _aropenCM->clear();
  _aropenCM->populate(q, true);
}

void arWorkBench::sEditAropenCM()
{
  ParameterList params;
  params.append("mode", "edit");

  q.prepare("SELECT 1 AS type, cmhead_id AS id "
            "FROM cmhead "
            "WHERE (cmhead_number=:docnum) "
            "UNION "
            "SELECT 2 AS type, aropen_id AS id "
            "FROM aropen "
            "WHERE ((aropen_docnumber=:docnum)"
            "  AND (aropen_doctype IN ('C','R')) "
            ") ORDER BY type LIMIT 1;");
  q.bindValue(":docnum", _aropenCM->currentItem()->text(1));
  q.exec();
  if (q.first())
  {
    if (q.value("type").toInt() == 1)
    {
      params.append("cmhead_id", q.value("id"));
      creditMemo* newdlg = new creditMemo();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (q.value("type").toInt() == 2)
    {
      params.append("aropen_id", q.value("id"));
      arOpenItem newdlg(this, "", true);
      newdlg.set(params);
      if (newdlg.exec() != QDialog::Rejected)
        sFillList();
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    QMessageBox::information(this, tr("Credit Memo Not Found"),
                             tr("<p>The Credit Memo #%1 could not be found.")
                             .arg(_aropenCM->currentItem()->text(1)));
    return;
  }
}

void arWorkBench::sViewAropenCM()
{
  ParameterList params;
  params.append("mode", "view");

  q.prepare("SELECT 1 AS type, cmhead_id AS id "
            "FROM cmhead "
            "WHERE (cmhead_number=:docnum) "
            "UNION "
            "SELECT 2 AS type, aropen_id AS id "
            "FROM aropen "
            "WHERE ((aropen_docnumber=:docnum)"
            "  AND (aropen_doctype IN ('C','R')) "
            ") ORDER BY type LIMIT 1;");
  q.bindValue(":docnum", _aropenCM->currentItem()->text(1));
  q.exec();
  if (q.first())
  {
    if (q.value("type").toInt() == 1)
    {
      params.append("cmhead_id", q.value("id"));
      creditMemo* newdlg = new creditMemo();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (q.value("type").toInt() == 2)
    {
      params.append("aropen_id", q.value("id"));
      arOpenItem newdlg(this, "", true);
      newdlg.set(params);
      if (newdlg.exec() != QDialog::Rejected)
        sFillList();
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    QMessageBox::information(this, tr("Credit Memo Not Found"),
                             tr("<p>The Credit Memo #%1 could not be found.")
                             .arg(_aropenCM->currentItem()->text(1)));
    return;
  }
}

void arWorkBench::sApplyAropenCM()
{
  ParameterList params;
  params.append("aropen_id", _aropenCM->id());

  applyARCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void arWorkBench::sFillCashrcptList()
{
  q.prepare("SELECT cashrcpt_id, formatDate(cashrcpt_distdate),"
            "       formatMoney(cashrcpt_amount), currConcat(cashrcpt_curr_id) "
            "FROM cashrcpt "
            "WHERE (cashrcpt_cust_id=:cust_id) "
            "ORDER BY cashrcpt_distdate;" );
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  _cashrcpt->clear();
  _cashrcpt->populate(q);
}

void arWorkBench::sNewCashrcpt()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void arWorkBench::sEditCashrcpt()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cashrcpt_id", _cashrcpt->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void arWorkBench::sViewCashrcpt()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cashrcpt_id", _cashrcpt->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void arWorkBench::sDeleteCashrcpt()
{
  q.prepare("SELECT deleteCashrcpt(:cashrcpt_id) AS result;");
  q.bindValue(":cashrcpt_id", _cashrcpt->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteCashrcpt", result));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillCashrcptList();
}

void arWorkBench::sPostCashrcpt()
{
  int journalNumber = -1;

  q.exec("SELECT fetchJournalNumber('C/R') AS journalnumber;");
  if (q.first())
    journalNumber = q.value("journalnumber").toInt();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT postCashReceipt(:cashrcpt_id, :journalNumber) AS result;");
  q.bindValue(":cashrcpt_id", _cashrcpt->id());
  q.bindValue(":journalNumber", journalNumber);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("postCashReceipt", result),
                  __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void arWorkBench::sFillPreauthList()
{
  int ccValidDays = _metrics->value("CCValidDays").toInt();
  if(ccValidDays < 1)
    ccValidDays = 7;
  q.prepare("SELECT ccpay_id,"
            " TEXT(ccpay_order_number) || '-' || TEXT(ccpay_order_number_seq) as ordnum, "
            "        formatMoney(ccpay_amount) AS f_amount,"
            "        currConcat(ccpay_curr_id) AS currAbbr "
            "FROM ccpay "
            " WHERE ( (ccpay_status = 'A')"
            "   AND   (date_part('day', CURRENT_TIMESTAMP - ccpay_transaction_datetime) < :ccValidDays)"
            " AND (ccpay_cust_id = :cust_id) )"
            " ORDER BY ccpay_transaction_datetime;" );
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":ccValidDays", ccValidDays);
  q.exec();
  _preauth->clear();
  _preauth->populate(q);
  
}

void arWorkBench::sgetCCAmount()
{
  q.prepare("SELECT ccpay_amount, ccpay_curr_id "
             "FROM ccpay "
             " WHERE (ccpay_id = :ccpay_id);");
  q.bindValue(":ccpay_id", _preauth->id());
  if (q.exec() && q.first())
  {
    /* _CCAmount->id() defaults to customer's currency
       if CC payment is in either customer's currency or base
       set _CCAmount appropriately
       but handle it if it somehow happens to be in a 3rd currency
     */
    int ccpayCurrId = q.value("ccpay_curr_id").toInt(); 
    if (ccpayCurrId == _CCAmount->baseId())
      _CCAmount->setBaseValue(q.value("ccpay_amount").toDouble());
    else if (ccpayCurrId != _CCAmount->id())
    {
      _CCAmount->setId(ccpayCurrId);
      _CCAmount->setLocalValue(q.value("ccpay_amount").toDouble());
    }
    else
      _CCAmount->setLocalValue(q.value("ccpay_amount").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  else
  {
    _CCAmount->clear();
  }
}

void arWorkBench::sPostPreauth()
{
  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (! cardproc->errorMsg().isEmpty())
  {
    QMessageBox::warning( this, tr("Credit Card Error"), cardproc->errorMsg() );
    _CCAmount->setFocus();
    return;
  }

  _postPreauth->setEnabled(false);
  _voidPreauth->setEnabled(false);
  int ccpayid   = _preauth->id();
  QString ordernum;
  int returnVal = cardproc->chargePreauthorized(-2,
						_CCAmount->localValue(),
						_CCAmount->id(),
						ordernum, ordernum, ccpayid);
  if (returnVal < 0)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
			  cardproc->errorMsg());
  else if (returnVal > 0)
    QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			 cardproc->errorMsg());
  else if (! cardproc->errorMsg().isEmpty())
    QMessageBox::information(this, tr("Credit Card Processing Note"),
			 cardproc->errorMsg());
  else
    _CCAmount->clear();

  sFillCashrcptList();
  sFillAropenCMList();
  sFillAropenList();
  sFillPreauthList();

  _voidPreauth->setEnabled(true);
  _postPreauth->setEnabled(true);
}

void arWorkBench::sVoidPreauth()
{
  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (! cardproc->errorMsg().isEmpty())
  {
    QMessageBox::warning( this, tr("Credit Card Error"), cardproc->errorMsg() );
    _CCAmount->setFocus();
    return;
  }

  _postPreauth->setEnabled(false);
  _voidPreauth->setEnabled(false);
  int ccpayid   = _preauth->id();
  QString ordernum;
  int returnVal = cardproc->voidPrevious(ccpayid);
  if (returnVal < 0)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
			  cardproc->errorMsg());
  else if (returnVal > 0)
    QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			 cardproc->errorMsg());
  else if (! cardproc->errorMsg().isEmpty())
    QMessageBox::information(this, tr("Credit Card Processing Note"),
			 cardproc->errorMsg());
  else
    _CCAmount->clear();

  sFillCashrcptList();
  sFillAropenCMList();
  sFillAropenList();
  sFillPreauthList();

  _voidPreauth->setEnabled(true);
  _postPreauth->setEnabled(true);
}

void arWorkBench::sPopulateAropenMenu(QMenu *pMenu)
{
  int menuItem;

  if (_aropen->altId() == 0)
  {
    menuItem = pMenu->insertItem(tr("View Apply-To Debit Memo..."), this, SLOT(sViewAropen()), 0);
    if (! _privleges->check("MaintainARMemos") &&
        ! _privleges->check("ViewARMemos"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  else if (_aropen->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("View Apply-To Invoice..."), this, SLOT(sViewAropen()), 0);
    if (! _privleges->check("MaintainMiscInvoices") &&
        ! _privleges->check("ViewMiscInvoices"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void arWorkBench::sPopulateAropenCMMenu(QMenu *pMenu)
{
  int menuItem;

  if (_aropenCM->altId() == 0)
  {
    menuItem = pMenu->insertItem(tr("Apply Credit Memo..."), this, SLOT(sApplyAropenCM()), 0);
    if (! _privleges->check("ApplyARMemos"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Edit Credit Memo..."), this, SLOT(sEditAropenCM()), 0);
    if (! _privleges->check("EditAROpenItem"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Credit Memo..."), this, SLOT(sViewAropenCM()), 0);
    if (! _privleges->check("EditAROpenItem") &&
        ! _privleges->check("ViewAROpenItems"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  /* show cash receipts here???
  else if (_aropenCM->altId() == 1)
  {
  }
  */
}

void arWorkBench::sPopulateCashRcptMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Cash Receipt..."), this, SLOT(sEditCashrcpt()), 0);
  if (! _privleges->check("MaintainCashReceipts") &&
      ! _privleges->check("ViewCashReceipts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Cash Receipt..."), this, SLOT(sViewCashrcpt()), 0);
  if (! _privleges->check("ViewCashReceipts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete Cash Receipt..."), this, SLOT(sDeleteCashrcpt()), 0);
  if (! _privleges->check("MaintainCashReceipts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Post Cash Receipt..."), this, SLOT(sPostCashrcpt()), 0);
  if (! _privleges->check("PostCashReceipts"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void arWorkBench::sPopulatePreauthMenu(QMenu*)
{
  /*
  id = ccpay_id
  column 0 = ccpay_order_number || ccpay_order_number_seq
  */
}

void arWorkBench::sCCRefundCM()
{
  if (_aropenCM->altId() < 0)
  {
    QMessageBox::warning(this, tr("Cannot Refund by Credit Card"),
			 tr("<p>The application cannot refund this "
			    "transaction using a credit card."));
    return;
  }
  
  int     ccardid = -1;
  double  total   =  0.0;
  double  tax     =  0.0;
  double  freight =  0.0;
  double  duty    =  0.0;
  int     currid  = -1;
  bool    taxexempt = false;
  QString docnum;
  QString refnum;
  int     ccpayid = -1;

  q.prepare("SELECT cmhead_id "
	    "FROM cmhead "
	    "WHERE (cmhead_number=:cmheadnumber);");
  q.bindValue(":cmheadnumber", _aropenCM->currentItem()->text(1));
  q.exec();
  if (q.first())
  {
    ParameterList ccp;
    ccp.append("cmhead_id", q.value("cmhead_id"));
    MetaSQLQuery ccm = mqlLoad(":/so/creditMemoCreditCard.mql");
    XSqlQuery ccq = ccm.toQuery(ccp);
    if (ccq.first())
    {
      ccardid = ccq.value("ccard_id").toInt();
      total   = ccq.value("total").toDouble();
      tax     = ccq.value("tax_in_cmcurr").toDouble();
      taxexempt = ccq.value("cmhead_tax_id").isNull();
      freight = ccq.value("cmhead_freight").toDouble();
      currid  = ccq.value("cmhead_curr_id").toInt();
      docnum  = ccq.value("cmhead_number").toString();
      refnum  = ccq.value("cohead_number").toString();
      ccpayid = ccq.value("ccpay_id").toInt();
    }
    else if (ccq.lastError().type() != QSqlError::None)
    {
      systemError(this, ccq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      QMessageBox::critical(this, tr("Credit Card Processing Error"),
			    tr("Could not find a Credit Card to use for "
			       "this Credit transaction."));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else // cmhead not found - maybe it's just an open item
  {
    q.prepare("SELECT ccard_id, aropen_amount - aropen_paid AS balance,"
	      "       aropen_curr_id, aropen_docnumber "
	      "FROM aropen, ccard "
	      "WHERE ((aropen_cust_id=ccard_cust_id)"
	      "  AND  (ccard_active)"
	      "  AND  (aropen_open)"
	      "  AND  (aropen_id=:aropenid));");
    q.bindValue(":aropenid", _aropenCM->id());
    q.exec();

    if (q.first())
    {
      ccardid = q.value("ccard_id").toInt();
      total   = q.value("balance").toDouble();
      currid  = q.value("aropen_curr_id").toInt();
      docnum  = q.value("aropen_docnumber").toString();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      QMessageBox::critical(this, tr("Credit Card Processing Error"),
			    tr("Could not find a Credit Card to use for "
			       "this Credit transaction."));
      return;
    }
  }

  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (! cardproc)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
			  CreditCardProcessor::errorMsg());
  else
  {
    int refid = _aropenCM->id();
    int returnVal = cardproc->credit(ccardid, -2, total, tax, taxexempt,
				     freight, duty, currid,
				     docnum, refnum, ccpayid, "aropen", refid);
    if (returnVal < 0)
      QMessageBox::critical(this, tr("Credit Card Processing Error"),
			    cardproc->errorMsg());
    else if (returnVal > 0)
      QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			   cardproc->errorMsg());
    else if (! cardproc->errorMsg().isEmpty())
      QMessageBox::information(this, tr("Credit Card Processing Note"),
			   cardproc->errorMsg());
  }

  sFillList();
}
