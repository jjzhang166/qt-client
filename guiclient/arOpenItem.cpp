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

#include "arOpenItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

arOpenItem::arOpenItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustInfo(int)));

  _last = -1;

  _arapply->addColumn(tr("Type"),        _dateColumn, Qt::AlignCenter,true, "doctype");
  _arapply->addColumn(tr("Doc. #"),               -1, Qt::AlignLeft,  true, "docnumber");
  _arapply->addColumn(tr("Apply Date"),  _dateColumn, Qt::AlignCenter,true, "arapply_postdate");
  _arapply->addColumn(tr("Amount"),     _moneyColumn, Qt::AlignRight, true, "arapply_applied");
  _arapply->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currabbr");

  if (omfgThis->singleCurrency())
      _arapply->hideColumn("currabbr");

  _terms->setType(XComboBox::ARTerms);
  _salesrep->setType(XComboBox::SalesReps);

  _altSalescatid->setType(XComboBox::SalesCategories);

  _rsnCode->setType(XComboBox::ReasonCodes);

  _journalNumber->setEnabled(FALSE);
  _commissionPaid->setEnabled(FALSE);
}

arOpenItem::~arOpenItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void arOpenItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse arOpenItem::set( const ParameterList &pParams )
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("aropen_id", &valid);
  if (valid)
  {
    _aropenid = param.toInt();
    populate();
  }

  param = pParams.value("docType", &valid);
  if (valid)
  {
    if (param.toString() == "creditMemo")
    {
      setCaption(caption() + tr(" - Enter Misc. Credit Memo"));
      _docType->setCurrentItem(0);
    }
    else if (param.toString() == "debitMemo")
    {
      setCaption(caption() + tr(" - Enter Misc. Debit Memo"));
      _docType->setCurrentItem(1);
    }
    else if (param.toString() == "invoice")
      _docType->setCurrentItem(2);
    else if (param.toString() == "customerDeposit")
      _docType->setCurrentItem(3);
    else
      return UndefinedError;
//  ToDo - better error return types

    _docType->setEnabled(FALSE);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT fetchARMemoNumber() AS number;");
      if (q.first())
        _docNumber->setText(q.value("number").toString());
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _paid->clear();
      _commissionPaid->clear();
      _save->setText(tr("Post"));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _cust->setReadOnly(TRUE);
      _docDate->setEnabled(FALSE);
      _docType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _orderNumber->setEnabled(FALSE);
      _journalNumber->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _altPrepaid->setEnabled(FALSE);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _cust->setReadOnly(TRUE);
      _docDate->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _docType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _orderNumber->setEnabled(FALSE);
      _journalNumber->setEnabled(FALSE);
      _amount->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _salesrep->setEnabled(FALSE);
      _commissionDue->setEnabled(FALSE);
      _commissionPaid->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);
      _altPrepaid->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _save->hide();

      _close->setText(tr("&Close"));
    }
    else
      return UndefinedError;
  }
  
  return NoError;
}

void arOpenItem::sSave()
{
  QString storedProc;
  if (_mode == cNew)
  {
    if (!_docDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot Save A/R Memo"),
                             tr("You must enter a date for this A/R Memo before you may save it") );
      _docDate->setFocus();
      return;
    }

    if (!_dueDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot Save A/R Memo"),
                             tr("You must enter a date for this A/R Memo before you may save it") );
      _dueDate->setFocus();
      return;
    }

    if (_amount->isZero())
    {
      QMessageBox::critical( this, tr("Cannot Save A/R Memo"),
                             tr("You must enter an amount for this A/R Memo before you may save it") );
      _amount->setFocus();
      return;
    }

    if (_altPrepaid->isChecked())
    {
      if(_altSalescatidSelected->isChecked() && !_altSalescatid->isValid())
      {
        QMessageBox::critical( this, tr("Cannot Save A/R Memo"),
                               tr("You must choose a valid Alternate Sales Category for this A/R Memo before you may save it") );
        return;
      }

      if(_altAccntidSelected->isChecked() && !_altAccntid->isValid())
      {
        QMessageBox::critical( this, tr("Cannot Save A/R Memo"),
                               tr("You must choose a valid Alternate Prepaid Account Number for this A/R Memo before you may save it.") );
        return;
      }
    }

    if (_docType->currentItem() == 0)
    {
      q.prepare( "SELECT createARCreditMemo( :cust_id, :aropen_docnumber, :aropen_ordernumber,"
                 "                           :aropen_docdate, :aropen_amount, :aropen_notes, :aropen_rsncode_id,"
                 "                           :aropen_salescat_id, :aropen_accnt_id, :aropen_duedate,"
                 "                           :aropen_terms_id, :aropen_salesrep_id, :aropen_commission_due,"
		 "                           :curr_id ) AS result;" );
      storedProc = "createARCreditMemo";
    }
    else if (_docType->currentItem() == 1)
    {
      q.prepare( "SELECT createARDebitMemo( :cust_id, :aropen_docnumber, :aropen_ordernumber,"
                 "                          :aropen_docdate, :aropen_amount, :aropen_notes, :aropen_rsncode_id,"
                 "                           :aropen_salescat_id, :aropen_accnt_id, :aropen_duedate,"
                 "                           :aropen_terms_id, :aropen_salesrep_id, :aropen_commission_due, "
		 "                           :curr_id ) AS result;" );
      storedProc = "createARCreditMemo";
    }

    q.bindValue(":cust_id", _cust->id());
  }
  else if (_mode == cEdit)
  {
    if (_cAmount != _amount->localValue())
      if ( QMessageBox::warning( this, tr("A/R Open Amount Changed"),
                                 tr( "You are changing the open amount of this A/R Open Item.  If you do not post a G/L Transaction\n"
                                     "to distribute this change then the A/R Open Item total will be out of balance with the\n"
                                     "A/R Trial Balance(s).\n"
                                     "Are you sure that you want to save this change?" ),
                                 tr("Yes"), tr("No"), QString::null ) == 1 )
        return;

    q.prepare( "UPDATE aropen "
               "SET aropen_duedate=:aropen_duedate,"
               "    aropen_terms_id=:aropen_terms_id, aropen_salesrep_id=:aropen_salesrep_id,"
               "    aropen_amount=:aropen_amount,"
               "    aropen_commission_due=:aropen_commission_due, aropen_notes=:aropen_notes,"
               "    aropen_rsncode_id=:aropen_rsncode_id, "
	       "    aropen_curr_id=:curr_id "
               "WHERE (aropen_id=:aropen_id);" );
  }

  q.bindValue(":aropen_id", _aropenid);
  q.bindValue(":aropen_docdate", _docDate->date());
  q.bindValue(":aropen_duedate", _dueDate->date());
  q.bindValue(":aropen_docnumber", _docNumber->text());
  q.bindValue(":aropen_ordernumber", _orderNumber->text());
  q.bindValue(":aropen_terms_id", _terms->id());
  q.bindValue(":aropen_salesrep_id", _salesrep->id());
  q.bindValue(":aropen_amount", _amount->localValue());
  q.bindValue(":aropen_commission_due", _commissionDue->baseValue());
  q.bindValue(":aropen_notes", _notes->text());
  q.bindValue(":aropen_rsncode_id", _rsnCode->id());
  q.bindValue(":curr_id", _amount->id());
  if(_altPrepaid->isChecked() && _altSalescatidSelected->isChecked())
    q.bindValue(":aropen_salescat_id", _altSalescatid->id());
  else
    q.bindValue(":aropen_salescat_id", -1);
  if(_altPrepaid->isChecked() && _altAccntidSelected->isChecked())
    q.bindValue(":aropen_accnt_id", _altAccntid->id());
  else
    q.bindValue(":aropen_accnt_id", -1);

  switch (_docType->currentItem())
  {
    case 0:
      q.bindValue(":aropen_doctype", "C");
      break;

    case 1:
      q.bindValue(":aropen_doctype", "D");
      break;

    case 2:
      q.bindValue(":aropen_doctype", "I");
      break;

    case 3:
      q.bindValue(":aropen_doctype", "R");
      break;
  }

  if (q.exec())
  {
    if (_mode == cEdit)
      done(_aropenid);
    else
    {
      q.first();
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      _last = q.value("result").toInt();
      if (_last < 0)
      {
	systemError(this, storedProc.isEmpty() ?
			    tr("Saving Credit Memo Failed: %1").arg(_last) :
			    storedProcErrorLookup(storedProc, _last),
		    __FILE__, __LINE__);
	return;
      }
      reset();
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void arOpenItem::sClose()
{
  if (_mode == cNew)
  {
    q.prepare("SELECT releaseARMemoNumber(:docNumber);");
    q.bindValue(":docNumber", _docNumber->text().toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if(_last != -1)
    {
      done(_last);
      return;
    }
  }

  reject();
}

void arOpenItem::sPopulateCustInfo(int pCustid)
{
  if ( (pCustid != -1) && (_mode == cNew) )
  {
    XSqlQuery c;
    c.prepare( "SELECT cust_terms_id, cust_salesrep_id, cust_curr_id "
               "FROM cust "
               "WHERE (cust_id=:cust_id);" );
    c.bindValue(":cust_id", pCustid);
    c.exec();
    if (c.first())
    {
      _terms->setId(c.value("cust_terms_id").toInt());
      _salesrep->setId(c.value("cust_salesrep_id").toInt());
      _amount->setId(c.value("cust_curr_id").toInt());
    }
    else if (c.lastError().type() != QSqlError::None)
    {
      systemError(this, c.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void arOpenItem::populate()
{
  q.prepare( "SELECT aropen_cust_id, aropen_docdate, aropen_duedate,"
             "       aropen_doctype, aropen_docnumber,"
             "       aropen_ordernumber, aropen_journalnumber,"
             "       aropen_amount, aropen_amount,"
             "       aropen_paid, "
             "       (aropen_amount - aropen_paid) AS f_balance,"
             "       aropen_terms_id, aropen_salesrep_id,"
             "       aropen_commission_due,"
             "       aropen_notes, aropen_rsncode_id, aropen_salescat_id, "
	     "       aropen_accnt_id, aropen_curr_id "
             "FROM aropen "
             "WHERE (aropen_id=:aropen_id);" );
  q.bindValue(":aropen_id", _aropenid);
  q.exec();
  if (q.first())
  {
    _cust->setId(q.value("aropen_cust_id").toInt());
    _docDate->setDate(q.value("aropen_docdate").toDate(), true);
    _dueDate->setDate(q.value("aropen_duedate").toDate());
    _docNumber->setText(q.value("aropen_docnumber").toString());
    _orderNumber->setText(q.value("aropen_ordernumber").toString());
    _journalNumber->setText(q.value("aropen_journalnumber").toString());
    _amount->set(q.value("aropen_amount").toDouble(),
		 q.value("aropen_curr_id").toInt(),
		 q.value("aropen_docdate").toDate(), false);
    _paid->setLocalValue(q.value("aropen_paid").toDouble());
    _balance->setLocalValue(q.value("f_balance").toDouble());
    _terms->setId(q.value("aropen_terms_id").toInt());
    _salesrep->setId(q.value("aropen_salesrep_id").toInt());
    _commissionDue->setBaseValue(q.value("aropen_commission_due").toDouble());
    _notes->setText(q.value("aropen_notes").toString());

    if(!q.value("aropen_rsncode_id").isNull() && q.value("aropen_rsncode_id").toInt() != -1)
      _rsnCode->setId(q.value("aropen_rsncode_id").toInt());

    if(!q.value("aropen_accnt_id").isNull() && q.value("aropen_accnt_id").toInt() != -1)
    {
      _altPrepaid->setChecked(TRUE);
      _altAccntidSelected->setChecked(TRUE);
      _altAccntid->setId(q.value("aropen_accnt_id").toInt());
    }

    if(!q.value("aropen_salescat_id").isNull() && q.value("aropen_salescat_id").toInt() != -1)
    {
      _altPrepaid->setChecked(TRUE);
      _altSalescatidSelected->setChecked(TRUE);
      _altSalescatid->setId(q.value("aropen_salescat_id").toInt());
    }

    QString docType = q.value("aropen_doctype").toString();
    if (docType == "C")
      _docType->setCurrentItem(0);
    else if (docType == "D")
      _docType->setCurrentItem(1);
    else if (docType == "I")
      _docType->setCurrentItem(2);
    else if (docType == "R")
      _docType->setCurrentItem(3);

    _cAmount = q.value("aropen_amount").toDouble();

    if ( (docType == "I") || (docType == "D") )
    {
      q.prepare( "SELECT arapply_id, arapply_source_aropen_id,"
                 "       CASE WHEN (arapply_source_doctype = 'C') THEN :creditMemo"
                 "            WHEN (arapply_source_doctype = 'R') THEN :cashdeposit"
                 "            WHEN (arapply_fundstype='C') THEN :check"
                 "            WHEN (arapply_fundstype='T') THEN :certifiedCheck"
                 "            WHEN (arapply_fundstype='M') THEN :masterCard"
                 "            WHEN (arapply_fundstype='V') THEN :visa"
                 "            WHEN (arapply_fundstype='A') THEN :americanExpress"
                 "            WHEN (arapply_fundstype='D') THEN :discoverCard"
                 "            WHEN (arapply_fundstype='R') THEN :otherCreditCard"
                 "            WHEN (arapply_fundstype='K') THEN :cash"
                 "            WHEN (arapply_fundstype='W') THEN :wireTransfer"
                 "            WHEN (arapply_fundstype='O') THEN :other"
                 "       END AS doctype,"
                 "       CASE WHEN (arapply_source_doctype IN ('C','R')) THEN arapply_source_docnumber"
                 "            WHEN (arapply_source_doctype = 'K') THEN arapply_refnumber"
                 "            ELSE :other"
                 "       END AS docnumber,"
                 "       arapply_postdate, arapply_applied, "
		 "       currConcat(arapply_curr_id) AS currabbr,"
                 "       'curr' AS arapply_applied_xtnumericrole "
                 "FROM arapply "
                 "WHERE (arapply_target_aropen_id=:aropen_id) "
                 "ORDER BY arapply_postdate;" );

      q.bindValue(":creditMemo", tr("Credit Memo"));
      q.bindValue(":cashdeposit", tr("Cash Deposit"));
      q.bindValue(":check", tr("Check"));
      q.bindValue(":certifiedCheck", tr("Certified Check"));
      q.bindValue(":masterCard", tr("Master Card"));
      q.bindValue(":visa", tr("Visa"));
      q.bindValue(":americanExpress", tr("American Express"));
      q.bindValue(":discoverCard", tr("Discover Card"));
      q.bindValue(":otherCreditCard", tr("Other Credit Card"));
      q.bindValue(":cash", tr("Cash"));
      q.bindValue(":wireTransfer", tr("Wire Transfer"));
      q.bindValue(":other", tr("Other"));
    }
    else if (docType == "C" || docType == "R")
    {
      q.prepare( "SELECT arapply_id, arapply_target_aropen_id,"
                 "       CASE WHEN (arapply_target_doctype = 'I') THEN :invoice"
                 "            WHEN (arapply_target_doctype = 'D') THEN :debitMemo"
                 "            WHEN (arapply_target_doctype = 'K') THEN :apcheck"
                 "            ELSE :other"
                 "       END AS doctype,"
                 "       arapply_target_docnumber AS docnumber,"
                 "       arapply_postdate, arapply_applied,"
                 "       currConcat(arapply_curr_id) AS currabbr,"
                 "       'curr' AS arapply_applied_xtnumericrole "
                 "FROM arapply "
                 "WHERE (arapply_source_aropen_id=:aropen_id) "
                 "ORDER BY arapply_postdate;" );

      q.bindValue(":invoice", tr("Invoice"));
      q.bindValue(":debitMemo", tr("Debit Memo"));
      q.bindValue(":apcheck", tr("A/P Check"));
    }

    q.bindValue(":error", tr("Error"));
    q.bindValue(":aropen_id", _aropenid);
    q.exec();
    _arapply->populate(q, TRUE);
    if (q.lastError().type() != QSqlError::NoError)
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void arOpenItem::reset()
{
  _cust->setId(-1);
  _docDate->clear();
  _dueDate->clear();
  _docNumber->clear();
  _orderNumber->clear();
  _journalNumber->clear();
  _terms->setId(-1);
  _salesrep->setId(-1);
  _commissionDue->clear();
  _commissionPaid->clear();
  _amount->clear();
  _paid->clear();
  _balance->clear();
  _rsnCode->setId(-1);
  _altPrepaid->setChecked(false);
  _notes->clear();
  _arapply->clear();

  _cust->setFocus();

  ParameterList params;
  params.append("mode", "new");
  set(params);
}
