/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "arOpenItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "printArOpenItem.h"
#include "storedProcErrorLookup.h"
#include "taxDetail.h"
#include "currcluster.h"

arOpenItem::arOpenItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _commprcnt = 0.0;

  _save = _buttonBox->button(QDialogButtonBox::Save);
  _save->setDisabled(true);

  connect(_buttonBox,      SIGNAL(accepted()),                 this, SLOT(sSave()));
  connect(_buttonBox,      SIGNAL(rejected()),                 this, SLOT(sClose()));
  connect(_cust,           SIGNAL(newId(int)),                this, SLOT(sPopulateCustInfo(int)));
  connect(_cust,           SIGNAL(valid(bool)),               _save, SLOT(setEnabled(bool)));
  connect(_terms,          SIGNAL(newID(int)),                this, SLOT(sPopulateDueDate()));
  connect(_docDate,        SIGNAL(newDate(const QDate&)),     this, SLOT(sPopulateDueDate()));
  connect(_taxLit,         SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_amount,         SIGNAL(valueChanged()),            this, SLOT(sCalculateCommission()));
  connect(_docNumber,      SIGNAL(textEdited(QString)),       this, SLOT(sReleaseNumber()));

  _last = -1;
  _aropenid = -1;
  _seqiss = 0;

  _arapply->addColumn(tr("Type"),            _dateColumn, Qt::AlignCenter,true, "doctype");
  _arapply->addColumn(tr("Doc. #"),                   -1, Qt::AlignLeft,  true, "docnumber");
  _arapply->addColumn(tr("Apply Date"),      _dateColumn, Qt::AlignCenter,true, "arapply_postdate");
  _arapply->addColumn(tr("Dist. Date"),      _dateColumn, Qt::AlignCenter,true, "arapply_distdate");
  _arapply->addColumn(tr("Amount"),         _moneyColumn, Qt::AlignRight, true, "arapply_applied");
  _arapply->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,  true, "currabbr");
  _arapply->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight, true, "baseapplied");

  _printOnPost->setVisible(false);

  if (omfgThis->singleCurrency())
      _arapply->hideColumn("currabbr");

  _cust->setType(CLineEdit::ActiveCustomers);
  _terms->setType(XComboBox::ARTerms);
  _salesrep->setType(XComboBox::SalesReps);

  _altSalescatid->setType(XComboBox::SalesCategoriesActive);

  _rsnCode->setType(XComboBox::ReasonCodes);

  _journalNumber->setEnabled(FALSE);

  _altAccntid->setType(GLCluster::cRevenue | GLCluster::cExpense);
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
  XSqlQuery aret;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("docType", &valid);
  if (valid)
  {
    if (param.toString() == "creditMemo")
    {
      setWindowTitle(windowTitle() + tr(" - Enter Misc. Credit Memo"));
      _docType->setCurrentIndex(0);
      _rsnCode->setType(XComboBox::ARCMReasonCodes);
    }
    else if (param.toString() == "debitMemo")
    {
      setWindowTitle(windowTitle() + tr(" - Enter Misc. Debit Memo"));
      _docType->setCurrentIndex(1);
      _rsnCode->setType(XComboBox::ARDMReasonCodes);
    }
    else if (param.toString() == "invoice")
      _docType->setCurrentIndex(2);
    else if (param.toString() == "customerDeposit")
      _docType->setCurrentIndex(3);
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

      aret.exec("SELECT fetchARMemoNumber() AS number;");
      if (aret.first())
      {
        _docNumber->setText(aret.value("number").toString());
        _seqiss = aret.value("number").toInt();
      }
      else if (aret.lastError().type() != QSqlError::NoError)
      {
	systemError(this, aret.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _paid->clear();
      _save->setText(tr("Post"));
      _printOnPost->setVisible(true);
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
      _useAltPrepaid->setEnabled(FALSE);
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
      _terms->setType(XComboBox::Terms);
      _salesrep->setEnabled(FALSE);
      _commissionDue->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);
      _useAltPrepaid->setEnabled(FALSE);
      _altPrepaid->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
    else
      return UndefinedError;
  }
  
  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _cust->setId(param.toInt());
    populate();
  }

  param = pParams.value("aropen_id", &valid);
  if (valid)
  {
    _aropenid = param.toInt();
    populate();
  }

  return NoError;
}

void arOpenItem::sSave()
{
  XSqlQuery arSave;
  QString storedProc;

  if (_mode == cNew)
  {
    if (!_docDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot Save Receivable Memo"),
                             tr("You must enter a date for this Receivable Memo before you may save it") );
      _docDate->setFocus();
      return;
    }

    if (!_dueDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot Save Receivable Memo"),
                             tr("You must enter a date for this Receivable Memo before you may save it") );
      _dueDate->setFocus();
      return;
    }

    if (_amount->isZero())
    {
      QMessageBox::critical( this, tr("Cannot Save Receivable Memo"),
                             tr("You must enter an amount for this Receivable Memo before you may save it") );
      _amount->setFocus();
      return;
    }
    
    if(_tax->localValue() > _amount->localValue())
    {
      QMessageBox::critical( this, tr("Cannot Save Receivable Memo"),
                             tr("The tax amount may not be greater than the total Receivable Memo amount.") );
      return;
    }

    if (_useAltPrepaid->isChecked())
    {
      if(_altSalescatidSelected->isChecked() && !_altSalescatid->isValid())
      {
        QMessageBox::critical( this, tr("Cannot Save Receivable Memo"),
                               tr("You must choose a valid Alternate Sales Category for this Receivable Memo before you may save it") );
        return;
      }

      if(_altAccntidSelected->isChecked() && !_altAccntid->isValid())
      {
        QMessageBox::critical( this, tr("Cannot Save Receivable Memo"),
                               tr("You must choose a valid Alternate Prepaid Account Number for this Receivable Memo before you may save it.") );
        return;
      }
    }

    if (_docType->currentIndex() == 0)
    {
      arSave.prepare( "SELECT createARCreditMemo( :aropen_id, :cust_id, :aropen_docnumber, :aropen_ordernumber,"
                 "                           :aropen_docdate, :aropen_amount, :aropen_notes, :aropen_rsncode_id,"
                 "                           :aropen_salescat_id, :aropen_accnt_id, :aropen_duedate,"
                 "                           :aropen_terms_id, :aropen_salesrep_id, :aropen_commission_due,"
                 "                           NULL, :curr_id ) AS result;" );
      storedProc = "createARCreditMemo";
    }
    else if (_docType->currentIndex() == 1)
    {
      arSave.prepare( "SELECT createARDebitMemo( :aropen_id,:cust_id, NULL, :aropen_docnumber, :aropen_ordernumber,"
                 "                          :aropen_docdate, :aropen_amount, :aropen_notes, :aropen_rsncode_id,"
                 "                          :aropen_salescat_id, :aropen_accnt_id, :aropen_duedate,"
                 "                          :aropen_terms_id, :aropen_salesrep_id, :aropen_commission_due, "
                 "                          :curr_id ) AS result;" );
      storedProc = "createARDebitMemo";
    }

    arSave.bindValue(":cust_id", _cust->id());
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

    arSave.prepare( "UPDATE aropen "
               "SET aropen_duedate=:aropen_duedate,"
               "    aropen_terms_id=:aropen_terms_id, aropen_salesrep_id=:aropen_salesrep_id,"
               "    aropen_amount=:aropen_amount,"
               "    aropen_commission_due=:aropen_commission_due, aropen_notes=:aropen_notes,"
               "    aropen_rsncode_id=:aropen_rsncode_id, "
	       "    aropen_curr_id=:curr_id "
               "WHERE (aropen_id=:aropen_id);" );
  }

  if (_aropenid != -1)
    arSave.bindValue(":aropen_id", _aropenid);
  arSave.bindValue(":aropen_docdate", _docDate->date());
  arSave.bindValue(":aropen_duedate", _dueDate->date());
  arSave.bindValue(":aropen_docnumber", _docNumber->text());
  arSave.bindValue(":aropen_ordernumber", _orderNumber->text());
  arSave.bindValue(":aropen_terms_id", _terms->id());

  if (_salesrep->isValid())
    arSave.bindValue(":aropen_salesrep_id", _salesrep->id());

  arSave.bindValue(":aropen_amount", _amount->localValue());
  arSave.bindValue(":aropen_commission_due", _commissionDue->baseValue());
  arSave.bindValue(":aropen_notes",          _notes->toPlainText());
  arSave.bindValue(":aropen_rsncode_id", _rsnCode->id());
  arSave.bindValue(":curr_id", _amount->id());
  if(_useAltPrepaid->isChecked() && _altSalescatidSelected->isChecked())
    arSave.bindValue(":aropen_salescat_id", _altSalescatid->id());
  else
    arSave.bindValue(":aropen_salescat_id", -1);
  if(_useAltPrepaid->isChecked() && _altAccntidSelected->isChecked())
    arSave.bindValue(":aropen_accnt_id", _altAccntid->id());
  else
    arSave.bindValue(":aropen_accnt_id", -1);

  switch (_docType->currentIndex())
  {
    case 0:
      arSave.bindValue(":aropen_doctype", "C");
      break;

    case 1:
      arSave.bindValue(":aropen_doctype", "D");
      break;

    case 2:
      arSave.bindValue(":aropen_doctype", "I");
      break;

    case 3:
      arSave.bindValue(":aropen_doctype", "R");
      break;
  }

  if (arSave.exec())
  {
    if (_mode == cEdit)
      done(_aropenid);
    else
    {
      arSave.first();
      if (arSave.lastError().type() != QSqlError::NoError)
      {
	systemError(this, arSave.lastError().databaseText(), __FILE__, __LINE__);
        reset();
	return;
      }
      _last = arSave.value("result").toInt();
      if (_last < 0)
      {
	systemError(this, storedProc.isEmpty() ?
			    tr("Saving Credit Memo Failed: %1").arg(_last) :
			    storedProcErrorLookup(storedProc, _last),
		    __FILE__, __LINE__);
        reset();
	return;
      }
      if(_printOnPost->isChecked())
        sPrintOnPost(_last);
      reset();
    }
  }
  else if (arSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, arSave.lastError().databaseText(), __FILE__, __LINE__);
    if (_mode == cNew)
      reset();
    return;
  }
}

void arOpenItem::sClose()
{
  if (_mode == cNew)
  {
    if(_seqiss)
      sReleaseNumber();

    if(_last != -1)
    {
      done(_last);
      return;
    }
  }

  reject();
}

void arOpenItem::sReleaseNumber()
{
  XSqlQuery arReleaseNumber;
  if(_seqiss)
  {
    arReleaseNumber.prepare("SELECT releaseARMemoNumber(:docNumber);");
    arReleaseNumber.bindValue(":docNumber", _seqiss);
    arReleaseNumber.exec();
    if (arReleaseNumber.lastError().type() != QSqlError::NoError)
    {
      systemError(this, arReleaseNumber.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void arOpenItem::sPopulateCustInfo(int pCustid)
{
  if ( (pCustid != -1) && (_mode == cNew) )
  {
    XSqlQuery c;
    c.prepare( "SELECT cust_terms_id, cust_salesrep_id, cust_curr_id, cust_commprcnt "
               "FROM custinfo "
               "WHERE (cust_id=:cust_id);" );
    c.bindValue(":cust_id", pCustid);
    c.exec();
    if (c.first())
    {
      _terms->setId(c.value("cust_terms_id").toInt());
      _salesrep->setId(c.value("cust_salesrep_id").toInt());
      _amount->setId(c.value("cust_curr_id").toInt());
      _tax->setId(c.value("cust_curr_id").toInt());
      _commprcnt = c.value("cust_commprcnt").toDouble();
    }
    else if (c.lastError().type() != QSqlError::NoError)
    {
      systemError(this, c.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void arOpenItem::populate()
{
  XSqlQuery arpopulate;
  arpopulate.prepare( "SELECT aropen_cust_id, aropen_docdate, aropen_duedate,"
             "       aropen_doctype, aropen_docnumber,"
             "       aropen_ordernumber, aropen_journalnumber,"
             "       aropen_amount, aropen_amount,"
             "       aropen_paid, "
             "       (aropen_amount - aropen_paid) AS f_balance,"
             "       aropen_terms_id, aropen_salesrep_id,"
             "       aropen_commission_due, cust_commprcnt,"
             "       aropen_notes, aropen_rsncode_id, aropen_salescat_id, "
             "       aropen_accnt_id, aropen_curr_id, "
             "       COALESCE(SUM(taxhist_tax),0) AS tax, "
             "       CASE WHEN (aropen_doctype = 'D' OR "
             "                 (aropen_doctype='C' AND cmhead_id IS NULL)) THEN "
             "         TRUE "
             "       ELSE "
             "         FALSE "
             "       END AS showTax "
             "FROM aropen "
             "  JOIN custinfo ON (cust_id=aropen_cust_id) "
             "  LEFT OUTER JOIN aropentax ON (aropen_id=taxhist_parent_id) "
             "  LEFT OUTER JOIN cmhead ON ((aropen_doctype='C') "
             "                         AND (aropen_docnumber=cmhead_number)) "
             "WHERE (aropen_id=:aropen_id) "
             "GROUP BY aropen_cust_id, aropen_docdate, aropen_duedate,      "
             "  aropen_doctype, aropen_docnumber, aropen_ordernumber, aropen_journalnumber,  "
             "  aropen_amount, aropen_amount, aropen_paid, f_balance, aropen_terms_id, "
             "  aropen_salesrep_id, aropen_commission_due, cust_commprcnt, aropen_notes, aropen_rsncode_id, "
             "  aropen_salescat_id, aropen_accnt_id, aropen_curr_id, cmhead_id;" );
  arpopulate.bindValue(":aropen_id", _aropenid);
  arpopulate.exec();
  if (arpopulate.first())
  {
    _cust->setId(arpopulate.value("aropen_cust_id").toInt());
    _docDate->setDate(arpopulate.value("aropen_docdate").toDate(), true);
    _dueDate->setDate(arpopulate.value("aropen_duedate").toDate());
    _docNumber->setText(arpopulate.value("aropen_docnumber").toString());
    _orderNumber->setText(arpopulate.value("aropen_ordernumber").toString());
    _journalNumber->setText(arpopulate.value("aropen_journalnumber").toString());
    _amount->set(arpopulate.value("aropen_amount").toDouble(),
		 arpopulate.value("aropen_curr_id").toInt(),
		 arpopulate.value("aropen_docdate").toDate(), false);
    _paid->setLocalValue(arpopulate.value("aropen_paid").toDouble());
    _balance->setLocalValue(arpopulate.value("f_balance").toDouble());
    _terms->setId(arpopulate.value("aropen_terms_id").toInt());
    _salesrep->setId(arpopulate.value("aropen_salesrep_id").toInt());
    _commissionDue->setBaseValue(arpopulate.value("aropen_commission_due").toDouble());
    _commprcnt = arpopulate.value("cust_commprcnt").toDouble();
    _notes->setText(arpopulate.value("aropen_notes").toString());
    if (arpopulate.value("showTax").toBool())
      _tax->setLocalValue(arpopulate.value("tax").toDouble());
    else
    {
      _taxLit->hide();
      _tax->hide();
    }

    if(!arpopulate.value("aropen_rsncode_id").isNull() && arpopulate.value("aropen_rsncode_id").toInt() != -1)
      _rsnCode->setId(arpopulate.value("aropen_rsncode_id").toInt());

    if(!arpopulate.value("aropen_accnt_id").isNull() && arpopulate.value("aropen_accnt_id").toInt() != -1)
    {
      _useAltPrepaid->setChecked(TRUE);
      _altAccntidSelected->setChecked(TRUE);
      _altAccntid->setId(arpopulate.value("aropen_accnt_id").toInt());
    }

    if(!arpopulate.value("aropen_salescat_id").isNull() && arpopulate.value("aropen_salescat_id").toInt() != -1)
    {
      _useAltPrepaid->setChecked(TRUE);
      _altSalescatidSelected->setChecked(TRUE);
      _altSalescatid->setId(arpopulate.value("aropen_salescat_id").toInt());
    }

    QString docType = arpopulate.value("aropen_doctype").toString();
    if (docType == "C")
      _docType->setCurrentIndex(0);
    else if (docType == "D")
      _docType->setCurrentIndex(1);
    else if (docType == "I")
      _docType->setCurrentIndex(2);
    else if (docType == "R")
      _docType->setCurrentIndex(3);

    _cAmount = arpopulate.value("aropen_amount").toDouble();

    if ( (docType == "I") || (docType == "D") )
    {
      arpopulate.prepare( "SELECT arapply_id, arapply_source_aropen_id,"
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
                 "       END AS docnumber, arapply_distdate,"
                 "       arapply_postdate, arapply_applied, "
                 "       currConcat(arapply_curr_id) AS currabbr,"
                 "       currToBase(arapply_curr_id, arapply_applied, arapply_postdate) AS baseapplied,"
                 "       'curr' AS arapply_applied_xtnumericrole,"
                 "       'curr' AS baseapplied_xtnumericrole "
                 "FROM arapply "
                 "WHERE (arapply_target_aropen_id=:aropen_id) "
                 "ORDER BY arapply_postdate;" );

      arpopulate.bindValue(":creditMemo", tr("Credit Memo"));
      arpopulate.bindValue(":cashdeposit", tr("Cash Deposit"));
      arpopulate.bindValue(":check", tr("Check"));
      arpopulate.bindValue(":certifiedCheck", tr("Certified Check"));
      arpopulate.bindValue(":masterCard", tr("Master Card"));
      arpopulate.bindValue(":visa", tr("Visa"));
      arpopulate.bindValue(":americanExpress", tr("American Express"));
      arpopulate.bindValue(":discoverCard", tr("Discover Card"));
      arpopulate.bindValue(":otherCreditCard", tr("Other Credit Card"));
      arpopulate.bindValue(":cash", tr("Cash"));
      arpopulate.bindValue(":wireTransfer", tr("Wire Transfer"));
      arpopulate.bindValue(":other", tr("Other"));
    }
    else if (docType == "C" || docType == "R")
    {
      arpopulate.prepare( "SELECT arapply_id, arapply_target_aropen_id,"
                 "       CASE WHEN (arapply_target_doctype = 'I') THEN :invoice"
                 "            WHEN (arapply_target_doctype = 'D') THEN :debitMemo"
                 "            WHEN (arapply_target_doctype = 'K') THEN :apcheck"
                 "            WHEN (arapply_target_doctype = 'R') THEN :cashreceipt"
                 "            ELSE :other"
                 "       END AS doctype,"
                 "       arapply_target_docnumber AS docnumber,"
                 "       arapply_distdate, arapply_postdate, arapply_applied,"
                 "       currConcat(arapply_curr_id) AS currabbr,"
                 "       currToBase(arapply_curr_id, arapply_applied, arapply_postdate) AS baseapplied,"
                 "       'curr' AS arapply_applied_xtnumericrole,"
                 "       'curr' AS baseapplied_xtnumericrole "
                 "FROM arapply "
                 "WHERE (arapply_source_aropen_id=:aropen_id) "
                 "ORDER BY arapply_postdate;" );

      arpopulate.bindValue(":invoice", tr("Invoice"));
      arpopulate.bindValue(":debitMemo", tr("Debit Memo"));
      arpopulate.bindValue(":apcheck", tr("A/P Check"));
      arpopulate.bindValue(":cashreceipt", tr("Cash Receipt"));
    }

    arpopulate.bindValue(":error", tr("Error"));
    arpopulate.bindValue(":aropen_id", _aropenid);
    arpopulate.exec();
    _arapply->populate(arpopulate, TRUE);
    if (arpopulate.lastError().type() != QSqlError::NoError)
	systemError(this, arpopulate.lastError().databaseText(), __FILE__, __LINE__);
  }
  else if (arpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, arpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void arOpenItem::reset()
{
  _aropenid = -1;
  _cust->setId(-1);
  _docDate->clear();
  _dueDate->clear();
  _docNumber->clear();
  _orderNumber->clear();
  _journalNumber->clear();
  _terms->setId(-1);
  _salesrep->setId(-1);
  _tax->clear();
  _commissionDue->clear();
  _amount->clear();
  _paid->clear();
  _balance->clear();
  _rsnCode->setId(-1);
  _useAltPrepaid->setChecked(false);
  _notes->clear();
  _arapply->clear();

  _cust->setFocus();

  ParameterList params;
  params.append("mode", "new");
  set(params);
}

void arOpenItem::sCalculateCommission()
{
  _commissionDue->setBaseValue(_amount->baseValue() * _commprcnt);
}

void arOpenItem::sPopulateDueDate()
{
  XSqlQuery arPopulateDueDate;
  if ( (_terms->isValid()) && (_docDate->isValid()) && (!_dueDate->isValid()) )
  {
    arPopulateDueDate.prepare("SELECT determineDueDate(:terms_id, :docDate) AS duedate;");
    arPopulateDueDate.bindValue(":terms_id", _terms->id());
    arPopulateDueDate.bindValue(":docDate", _docDate->date());
    arPopulateDueDate.exec();
    if (arPopulateDueDate.first())
      _dueDate->setDate(arPopulateDueDate.value("duedate").toDate());
  }
}

void arOpenItem::sPrintOnPost(int temp_id)
{
  ParameterList params;
  params.append("aropen_id", temp_id);

  printArOpenItem newdlg(this, "", true);
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void arOpenItem::sTaxDetail()
{
  XSqlQuery ar;
  if (_aropenid == -1)
  {
    if (!_docDate->isValid() || !_dueDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot set tax amounts"),
                             tr("You must enter document and due dates for this Receivable Memo before you may set tax amounts.") );
      _docDate->setFocus();
      return;
    }
    
    if (_amount->isZero())
    {
      QMessageBox::critical( this, tr("Cannot set tax amounts"),
                             tr("You must enter an amount for this Receivable Memo before you may set tax amounts.") );
      _amount->setFocus();
      return;
    }

    ar.prepare("SELECT nextval('aropen_aropen_id_seq') AS result;");
    ar.exec();
    if (ar.first())
      _aropenid = ar.value("result").toInt();
    else if (ar.lastError().type() != QSqlError::NoError)
    {
      systemError(this, ar.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
      return;
    
    ar.prepare("INSERT INTO aropen "
      "( aropen_id, aropen_docdate, aropen_duedate, aropen_doctype, "
      "  aropen_docnumber, aropen_curr_id, aropen_open, aropen_posted, aropen_amount ) "
      "VALUES "
      "( :aropen_id, :docDate, :dueDate, :docType, :docNumber, :currId, true, false, :amount ); ");
    ar.bindValue(":aropen_id",_aropenid);
    ar.bindValue(":docDate", _docDate->date());
    ar.bindValue(":dueDate", _dueDate->date());
    ar.bindValue(":amount", _amount->localValue());
    if (_docType->currentIndex())
      ar.bindValue(":docType", "D" );
    else
      ar.bindValue(":docType", "C" );
    ar.bindValue(":docNumber", _docNumber->text());
    ar.bindValue(":currId", _amount->id());
    ar.exec();
    if (ar.lastError().type() != QSqlError::NoError)
    {
      systemError(this, ar.lastError().databaseText(), __FILE__, __LINE__);
      reset();
      return;
    }
  }
  
  taxDetail newdlg(this, "", true);
  ParameterList params;

  params.append("curr_id", _tax->id());
  params.append("date",    _tax->effective());
  if (!_docType->currentIndex())
    params.append("sense",-1);
  if (_mode != cNew)
    params.append("readOnly");

  ar.exec("SELECT getadjustmenttaxtypeid() as taxtype;");
  if(ar.first())
    params.append("taxtype_id", ar.value("taxtype").toInt());  
   
  params.append("order_type", "AR");
  params.append("order_id", _aropenid);
  params.append("display_type", "A");
  params.append("subtotal", _amount->localValue());
  params.append("adjustment");
  if (!_docType->currentIndex())
    params.append("sense",-1);
  if (newdlg.set(params) == NoError)  
  {
    newdlg.exec();
    XSqlQuery taxq;
    taxq.prepare( "SELECT SUM(taxhist_tax) AS tax "
      "FROM aropentax "
      "WHERE (taxhist_parent_id=:aropen_id);" );
    taxq.bindValue(":aropen_id", _aropenid);
    taxq.exec();
    if (taxq.first())
    {
      if (!_docType->currentIndex())
        _tax->setLocalValue(taxq.value("tax").toDouble() * -1);
      else
        _tax->setLocalValue(taxq.value("tax").toDouble());
    }
    else if (taxq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

