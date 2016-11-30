/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
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

#include "errorReporter.h"
#include "printArOpenItem.h"
#include "storedProcErrorLookup.h"
#include "taxDetail.h"
#include "currcluster.h"
#include "guiErrorCheck.h"

arOpenItem::arOpenItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _commprcnt = 0.0;

  _save = _buttonBox->button(QDialogButtonBox::Save);
  _save->setDisabled(true);

  connect(_buttonBox,      SIGNAL(accepted()),                this, SLOT(sSave()));
  connect(_buttonBox,      SIGNAL(rejected()),                this, SLOT(sClose()));
  connect(_cust,           SIGNAL(newId(int)),                this, SLOT(sPopulateCustInfo(int)));
  connect(_cust,           SIGNAL(valid(bool)),               _save, SLOT(setEnabled(bool)));
  connect(_terms,          SIGNAL(newID(int)),                this, SLOT(sPopulateDueDate()));
  connect(_docDate,        SIGNAL(newDate(const QDate&)),     this, SLOT(sPopulateDueDate()));
  connect(_taxLit,         SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_amount,         SIGNAL(valueChanged()),            this, SLOT(sCalculateCommission()));
  connect(_amount,         SIGNAL(editingFinished()),         this, SLOT(sDetermineTaxAmount()));
  connect(_taxzone,        SIGNAL(newID(int)),                this, SLOT(sDetermineTaxAmount()));
  connect(_docNumber,      SIGNAL(textEdited(QString)),       this, SLOT(sReleaseNumber()));

  _last = -1;
  _aropenid = -1;
  _seqiss = 0;

  _docType->append(0, tr("Credit Memo"),      "C");
  _docType->append(1, tr("Debit Memo"),       "D");
  _docType->append(2, tr("Invoice"),          "I");
  _docType->append(3, tr("Customer Deposit"), "R");

  _arapply->addColumn(tr("Type"),            _dateColumn, Qt::AlignCenter,true, "doctype");
  _arapply->addColumn(tr("Doc. #"),                   -1, Qt::AlignLeft,  true, "docnumber");
  _arapply->addColumn(tr("Apply Date"),      _dateColumn, Qt::AlignCenter,true, "arapply_postdate");
  _arapply->addColumn(tr("Dist. Date"),      _dateColumn, Qt::AlignCenter,true, "arapply_distdate");
  _arapply->addColumn(tr("Amount"),         _moneyColumn, Qt::AlignRight, true, "arapply_applied");
  _arapply->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency(), "currabbr");
  _arapply->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight, true, "baseapplied");

  _printOnPost->setVisible(false);

  _cust->setType(CLineEdit::ActiveCustomers);
  _terms->setType(XComboBox::ARTerms);
  _salesrep->setType(XComboBox::SalesReps);

  _altSalescatid->setType(XComboBox::SalesCategoriesActive);

  _rsnCode->setType(XComboBox::ReasonCodes);

  _journalNumber->setEnabled(false);

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
      _docType->setCode("C");
      _rsnCode->setType(XComboBox::ARCMReasonCodes);
    }
    else if (param.toString() == "debitMemo")
    {
      setWindowTitle(windowTitle() + tr(" - Enter Misc. Debit Memo"));
      _docType->setCode("D");
      _rsnCode->setType(XComboBox::ARDMReasonCodes);
    }
    else if (param.toString() == "invoice")
      _docType->setCode("I");
    else if (param.toString() == "customerDeposit")
      _docType->setCode("R");
    else
      return UndefinedError;
//  ToDo - better error return types

    _docType->setEnabled(false);
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
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving A/R Information"),
                                  aret, __FILE__, __LINE__))
    {
      return UndefinedError;
    }

      _paid->clear();
      _save->setText(tr("Post"));
      _printOnPost->setVisible(true);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _cust->setReadOnly(true);
      _docDate->setEnabled(false);
      _docType->setEnabled(false);
      _docNumber->setEnabled(false);
      _orderNumber->setEnabled(false);
      _journalNumber->setEnabled(false);
      _amount->setEnabled(false);
      _taxzone->setEnabled(false);
      _terms->setEnabled(false);
      _useAltPrepaid->setEnabled(false);
      _altPrepaid->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _cust->setReadOnly(true);
      _docDate->setEnabled(false);
      _dueDate->setEnabled(false);
      _docType->setEnabled(false);
      _docNumber->setEnabled(false);
      _orderNumber->setEnabled(false);
      _journalNumber->setEnabled(false);
      _amount->setEnabled(false);
      _terms->setEnabled(false);
      _terms->setType(XComboBox::Terms);
      _salesrep->setEnabled(false);
      _commissionDue->setEnabled(false);
      _rsnCode->setEnabled(false);
      _taxzone->setEnabled(false);
      _useAltPrepaid->setEnabled(false);
      _altPrepaid->setEnabled(false);
      _notes->setReadOnly(true);
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

    QList<GuiErrorCheck>errors;
    errors<<GuiErrorCheck(!_docDate->isValid(), _docDate,
                            tr("You must enter a document date for this Receivable Memo before you may save it."))
          <<GuiErrorCheck(!_dueDate->isValid(), _dueDate,
                           tr("You must enter a due date for this Receivable Memo before you may save it."))
          <<GuiErrorCheck(_amount->isZero(), _amount,
                          tr("You must enter an amount for this Receivable Memo before you may save it."))
          <<GuiErrorCheck(_tax->localValue() > _amount->localValue(), _tax,
                          tr("The tax amount may not be greater than the total Receivable Memo amount."));


    if (_useAltPrepaid->isChecked())
    {
      errors<<GuiErrorCheck(_altSalescatidSelected->isChecked() && !_altSalescatid->isValid(), _altSalescatid,
                           tr("You must choose a valid Alternate Sales Category for this Receivable Memo before you may save it."))
            <<GuiErrorCheck(_altAccntidSelected->isChecked() && !_altAccntid->isValid(), _altAccntid,
                          tr("You must choose a valid Alternate Prepaid Account Number for this Receivable Memo before you may save it."));

    }

    if(GuiErrorCheck::reportErrors(this,tr("Cannot Save Receivable Memo"),errors))
        return;

    if (_docType->code() == "C")
    {
      arSave.prepare( "SELECT createARCreditMemo( :aropen_id, :cust_id, :aropen_docnumber, :aropen_ordernumber,"
                 "                           :aropen_docdate, :aropen_amount, :aropen_notes, :aropen_rsncode_id,"
                 "                           :aropen_salescat_id, :aropen_accnt_id, :aropen_duedate,"
                 "                           :aropen_terms_id, :aropen_salesrep_id, :aropen_commission_due,"
                 "                           NULL, :curr_id, NULL, NULL, :taxzone ) AS result;" );
      storedProc = "createARCreditMemo";
    }
    else if (_docType->code() == "D")
    {
      arSave.prepare( "SELECT createARDebitMemo( :aropen_id,:cust_id, NULL, :aropen_docnumber, :aropen_ordernumber,"
                 "                          :aropen_docdate, :aropen_amount, :aropen_notes, :aropen_rsncode_id,"
                 "                          :aropen_salescat_id, :aropen_accnt_id, :aropen_duedate,"
                 "                          :aropen_terms_id, :aropen_salesrep_id, :aropen_commission_due, "
                 "                          :curr_id, :taxzone ) AS result;" );
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
               "    aropen_curr_id=:curr_id, "
               "    aropen_taxzone_id=:taxzone "
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
  if (_taxzone->isValid())
    arSave.bindValue(":taxzone", _taxzone->id());

  if(_useAltPrepaid->isChecked() && _altSalescatidSelected->isChecked())
    arSave.bindValue(":aropen_salescat_id", _altSalescatid->id());
  else
    arSave.bindValue(":aropen_salescat_id", -1);
  if(_useAltPrepaid->isChecked() && _altAccntidSelected->isChecked())
    arSave.bindValue(":aropen_accnt_id", _altAccntid->id());
  else
    arSave.bindValue(":aropen_accnt_id", -1);

  arSave.bindValue(":aropen_doctype", _docType->code());

  if (arSave.exec())
  {
    if (_mode == cEdit)
      done(_aropenid);
    else
    {
      arSave.first();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting A/R %1M").arg(_docType->code()),
                                    arSave, __FILE__, __LINE__))
      {
        reset();
        return;
      }
      _last = arSave.value("result").toInt();
      if (_last < 0)
      {
        if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting A/R %1M").arg(_docType->code()),
                                    arSave, __FILE__, __LINE__))
        {
          reset();
        }
        return;
      }
      if(_printOnPost->isChecked())
        sPrintOnPost(_last);
      reset();
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting A/R %1M").arg(_docType->code()),
                                arSave, __FILE__, __LINE__))
  {
    if (_mode == cNew)
      reset();
    return;
  }
}

void arOpenItem::sClose()
{
  XSqlQuery deleteARDocument;
  if (_mode == cNew)
  {
    if(_aropenid != -1){
      deleteARDocument.prepare("DELETE FROM aropen WHERE aropen_id = :aropenid;");
      deleteARDocument.bindValue(":aropenid", _aropenid);
      deleteARDocument.exec();
    }

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
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Cancelling A/R %1M").arg(_docType->code()),
                                  arReleaseNumber, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void arOpenItem::sPopulateCustInfo(int pCustid)
{
  if ( (pCustid != -1) && (_mode == cNew) )
  {
    XSqlQuery c;
    c.prepare( "SELECT cust_terms_id, cust_salesrep_id, cust_curr_id, "
               "       cust_taxzone_id, cust_commprcnt "
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
      _taxzone->setId(c.value("cust_taxzone_id").toInt());
      _commprcnt = c.value("cust_commprcnt").toDouble();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving A/R Information"),
                                  c, __FILE__, __LINE__))
    {
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
             "       aropen_accnt_id, aropen_curr_id, aropen_taxzone_id, "
             "       COALESCE(SUM(taxhist_tax),0) AS tax, "
             "       CASE WHEN (aropen_doctype = 'D' OR "
             "                 (aropen_doctype='C' AND cmhead_id IS NULL)) THEN "
             "         true "
             "       ELSE "
             "         false "
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
             "  aropen_salescat_id, aropen_accnt_id, aropen_curr_id, aropen_taxzone_id, cmhead_id;" );
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
    _taxzone->setId(arpopulate.value("aropen_taxzone_id").toInt());
    if (arpopulate.value("showTax").toBool())
      _tax->setLocalValue(arpopulate.value("tax").toDouble());
    else
    {
      _taxzoneLit->hide();
      _taxzone->hide();
      _taxLit->hide();
      _tax->hide();
    }

    if(!arpopulate.value("aropen_rsncode_id").isNull() && arpopulate.value("aropen_rsncode_id").toInt() != -1)
      _rsnCode->setId(arpopulate.value("aropen_rsncode_id").toInt());

    if(!arpopulate.value("aropen_accnt_id").isNull() && arpopulate.value("aropen_accnt_id").toInt() != -1)
    {
      _useAltPrepaid->setChecked(true);
      _altAccntidSelected->setChecked(true);
      _altAccntid->setId(arpopulate.value("aropen_accnt_id").toInt());
    }

    if(!arpopulate.value("aropen_salescat_id").isNull() && arpopulate.value("aropen_salescat_id").toInt() != -1)
    {
      _useAltPrepaid->setChecked(true);
      _altSalescatidSelected->setChecked(true);
      _altSalescatid->setId(arpopulate.value("aropen_salescat_id").toInt());
    }

    QString docType = arpopulate.value("aropen_doctype").toString();
    _docType->setCode(docType);

    _cAmount = arpopulate.value("aropen_amount").toDouble();

    if ( (docType == "I") || (docType == "D") )
    {
      arpopulate.prepare( "SELECT arapply_id, arapply_source_aropen_id,"
                 "       CASE WHEN (arapply_source_doctype = 'C') THEN :creditMemo"
                 "            WHEN (arapply_source_doctype = 'R') THEN :cashdeposit"
                 "            ELSE getFundsTypeName(arapply_fundstype)"
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
    _arapply->populate(arpopulate, true);
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving A/R Information"),
                                  arpopulate, __FILE__, __LINE__);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Next Period Number"),
                                arpopulate, __FILE__, __LINE__))
  {
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

bool arOpenItem::sInitializeMemo()
{
  XSqlQuery ar;
  ar.prepare("SELECT nextval('aropen_aropen_id_seq') AS result;");
  ar.exec();
  if (ar.first())
    _aropenid = ar.value("result").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Initializing Memo"),
                                ar, __FILE__, __LINE__))
    return false;
  else
    return false;

  ar.prepare("INSERT INTO aropen "
    "( aropen_id, aropen_docdate, aropen_duedate, aropen_doctype, "
    "  aropen_docnumber, aropen_curr_id, aropen_open, aropen_posted, aropen_amount ) "
    "VALUES "
    "( :aropen_id, :docDate, :dueDate, :docType, :docNumber, :currId, true, false, :amount ); ");
  ar.bindValue(":aropen_id",_aropenid);
  ar.bindValue(":docDate", _docDate->date());
  ar.bindValue(":dueDate", _dueDate->date());
  ar.bindValue(":amount", _amount->localValue());
  ar.bindValue(":docType", _docType->code() );
  ar.bindValue(":docNumber", _docNumber->text());
  ar.bindValue(":currId", _amount->id());
  ar.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Initializing Memo"),
                                ar, __FILE__, __LINE__))
  {
    reset();
    return false;
  }

  return true;
}

void arOpenItem::sTaxDetail()
{
  XSqlQuery ar;
  if (_aropenid == -1)
  {
    QList<GuiErrorCheck>errors;
    errors<<GuiErrorCheck(!_docDate->isValid(), _docDate,
                            tr("You must enter a document date for this Receivable Memo before you may set tax amounts."))
          <<GuiErrorCheck(!_dueDate->isValid(), _dueDate,
                            tr("You must enter a due date for this Receivable Memo before you may set tax amounts."))
          <<GuiErrorCheck(_amount->isZero(), _amount,
                           tr("You must enter an amount for this Receivable Memo before you may set tax amounts."));

    if(GuiErrorCheck::reportErrors(this,tr("Cannot Set Tax Amounts"),errors))
        return;


    if (!sInitializeMemo())
      return;
  }

  taxDetail newdlg(this, "", true);
  ParameterList params;

  params.append("curr_id", _tax->id());
  params.append("date",    _tax->effective());
  if (_docType->code() == "C")
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
      if (_docType->code() == "C")
        _tax->setLocalValue(taxq.value("tax").toDouble() * -1);
      else
        _tax->setLocalValue(taxq.value("tax").toDouble());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Cannot set tax amounts"),
                                  taxq, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void arOpenItem::sDetermineTaxAmount()
{
  if (_mode != cNew)
    return;

  XSqlQuery ar;
  if (_aropenid == -1)
  {
    if (!_docDate->isValid() || !_dueDate->isValid() || _amount->isZero())
      return;
    if (!sInitializeMemo())
      return;
  }
  ar.prepare( "SELECT updatememotax(:source, :doctype, :aropen_id, :taxzone, :date, :curr, :amount) AS tax;" );

  ar.bindValue(":source",    "AR");
  ar.bindValue(":doctype",   _docType->code());
  ar.bindValue(":aropen_id", _aropenid);
  ar.bindValue(":taxzone",   _taxzone->id());
  ar.bindValue(":date",      _tax->effective());
  ar.bindValue(":curr",      _tax->id());
  ar.bindValue(":amount",    _amount->localValue());
  ar.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Returning AR Open Tax"),
                           ar, __FILE__, __LINE__))
    return;

  if (ar.first())
    _tax->setLocalValue(ar.value("tax").toDouble());
}
