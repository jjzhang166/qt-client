/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "apOpenItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "printApOpenItem.h"
#include "taxDetail.h"

apOpenItem::apOpenItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_docDate,        SIGNAL(newDate(const QDate&)),          this, SLOT(sPopulateDueDate()));
  connect(_buttonBox,      SIGNAL(accepted()),                     this, SLOT(sSave()));
  connect(_buttonBox,      SIGNAL(rejected()),                     this, SLOT(sClose()));
  connect(_terms,          SIGNAL(newID(int)),                     this, SLOT(sPopulateDueDate()));
  connect(_vend,           SIGNAL(newId(int)),                     this, SLOT(sPopulateVendInfo(int)));
  connect(_amount,         SIGNAL(editingFinished()),              this, SLOT(sDetermineTaxAmount()));
  connect(_taxzone,        SIGNAL(newID(int)),                     this, SLOT(sDetermineTaxAmount()));
  connect(_taxLit,         SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_docNumber,      SIGNAL(textEdited(QString)),            this, SLOT(sReleaseNumber()));
  connect(_usePrepaid,     SIGNAL(toggled(bool)),                  this, SLOT(sToggleAccount()));
  connect(_amount,         SIGNAL(valueChanged()),                 this, SLOT(sCalcBalance()));

  _cAmount = 0.0;
  _apopenid = -1;
  _seqiss = 0;

  _docType->append(0, tr("Credit Memo"), "C");
  _docType->append(1, tr("Debit Memo"),  "D");
  _docType->append(2, tr("Voucher"),     "V");

  _apapply->addColumn( tr("Type"),        _dateColumn, Qt::AlignCenter,true, "doctype");
  _apapply->addColumn( tr("Doc. #"),               -1, Qt::AlignLeft,  true, "docnumber");
  _apapply->addColumn( tr("Apply Date"),  _dateColumn, Qt::AlignCenter,true, "apapply_postdate");
  _apapply->addColumn( tr("Amount"),     _moneyColumn, Qt::AlignRight, true, "apapply_amount");
  _apapply->addColumn( tr("Currency"),_currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency(), "currabbr");

  _printOnPost->setVisible(false);

  _terms->setType(XComboBox::APTerms);
  _journalNumber->setEnabled(false);

  _accntId->setType(GLCluster::cRevenue | GLCluster::cExpense);
  sToggleAccount();
}

apOpenItem::~apOpenItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void apOpenItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse apOpenItem::set(const ParameterList &pParams)
{
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
      _status->setEnabled(false);
    }
    else if (param.toString() == "debitMemo")
    {
      setWindowTitle(windowTitle() + tr(" - Enter Misc. Debit Memo"));
      _docType->setCode("D");
    }
    else if (param.toString() == "voucher")
      _docType->setCode("V");
    else
      return UndefinedError;
//  ToDo - better error return types

    _docType->setEnabled(false);
  }

  param = pParams.value("mode", &valid);
  XSqlQuery setOpenItem;
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      setOpenItem.exec("SELECT fetchAPMemoNumber() AS number;");
      if (setOpenItem.first())
      {
        _docNumber->setText(setOpenItem.value("number").toString());
        _seqiss = setOpenItem.value("number").toInt();
      }

      _paid->clear();
      _buttonBox->button(QDialogButtonBox::Save)->setText(tr("Post"));
      _printOnPost->setVisible(true);
      populateStatus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _vend->setReadOnly(true);
      _docDate->setEnabled(false);
      _dueDate->setEnabled(false);
      _docType->setEnabled(false);
      _docNumber->setEnabled(false);
      _poNumber->setEnabled(false);
      _journalNumber->setEnabled(false);
      _amount->setEnabled(false);
      _terms->setEnabled(false);
      _notes->setReadOnly(false);
      _usePrepaid->setEnabled(false);
      _taxzone->setEnabled(false);
      _accntId->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _vend->setReadOnly(true);
      _docDate->setEnabled(false);
      _dueDate->setEnabled(false);
      _docType->setEnabled(false);
      _docNumber->setEnabled(false);
      _poNumber->setEnabled(false);
      _journalNumber->setEnabled(false);
      _amount->setEnabled(false);
      _terms->setEnabled(false);
      _terms->setType(XComboBox::Terms);
      _notes->setReadOnly(true);
      _usePrepaid->setEnabled(false);
      _accntId->setEnabled(false);
      _status->setEnabled(false);
      _taxzone->setEnabled(false);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vend->setId(param.toInt());

  param = pParams.value("apopen_id", &valid);
  if (valid)
  {
    _apopenid = param.toInt();
    populate();
  }

  return NoError;
}

void apOpenItem::sSave()
{
  XSqlQuery saveOpenItem;
  if (_mode == cNew)
  {
    QList<GuiErrorCheck> errors;
    errors << GuiErrorCheck(!_docDate->isValid(), _docDate,
                            tr("<p>You must enter a Document Date for this A/P Memo "
                               "before you may save it"))
    << GuiErrorCheck(!_dueDate->isValid(), _dueDate,
                     tr("<p>You must enter a Due Date for this A/P Memo "
                        "before you may save it"))
    << GuiErrorCheck(_amount->isZero(), _amount,
                     tr("<p>You must enter an amount for this A/P Memo "
                        "before you may save it"))
    << GuiErrorCheck(_tax->localValue() > _amount->localValue(), _tax,
                     tr("The tax amount may not be greater than the total A/P Memo amount."))
    << GuiErrorCheck(!_usePrepaid->isChecked() && !_accntId->isValid(), _accntId,
                     tr("<p>You must choose a valid Distribution "
                        "Account Number for this A/P Memo before you "
                        "may save it."))
    ;
    if (GuiErrorCheck::reportErrors(this, tr("Cannot Save A/P Memo"), errors))
      return;
    
    QString tmpFunctionName;
    QString queryStr;

    if (_docType->code() == "C")
      tmpFunctionName = "createAPCreditMemo";
    else if (_docType->code() == "D")
      tmpFunctionName = "createAPDebitMemo";
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Cannot Save A/P Memo"),
                           tr("Internal Error: _docType has an invalid document type of %1")
                           .arg(_docType->currentIndex()), __FILE__, __LINE__);
      return;
    }

    queryStr = "SELECT " + tmpFunctionName + "( :apopen_id, :vend_id, " +
		(_journalNumber->text().isEmpty() ?
		      QString("fetchJournalNumber('AP-MISC')") : _journalNumber->text()) +
	       ", :apopen_docnumber, :apopen_ponumber, :apopen_docdate,"
	       "  :apopen_amount, :apopen_notes, :apopen_accnt_id,"
	       "  :apopen_duedate, :apopen_terms_id, :curr_id, :apopen_taxzone_id ) AS result;";
	
    saveOpenItem.prepare(queryStr);
    saveOpenItem.bindValue(":vend_id", _vend->id());
    saveOpenItem.bindValue(":apopen_docdate", _docDate->date());
  }
  else if (_mode == cEdit)
  {
    if (_cAmount != _amount->localValue())
      if ( QMessageBox::warning( this, tr("A/P Open Amount Changed"),
                                 tr( "<p>You are changing the open amount of "
                                    "this A/P Open Item.  If you do not post a "
                                    "G/L Transaction to distribute this change "
                                    "then the A/P Open Item total will be out "
                                    "of balance with the A/P Trial Balance(s). "
                                    "Are you sure that you want to save this "
                                    "change?" ),
                                 tr("Yes"), tr("No"), QString::null ) == 1 )
        return;

    saveOpenItem.prepare( "UPDATE apopen "
	       "SET apopen_doctype=:apopen_doctype,"
               "    apopen_ponumber=:apopen_ponumber, apopen_docnumber=:apopen_docnumber,"
               "    apopen_amount=:apopen_amount,"
               "    apopen_terms_id=:apopen_terms_id, "
	       "    apopen_notes=:apopen_notes, "
               "    apopen_curr_id=:curr_id, "
	       "    apopen_status = :apopen_status, "
               "    apopen_taxzone_id = :apopen_taxzone_id "
               "WHERE (apopen_id=:apopen_id);" );
  }

  if (_apopenid != -1)
    saveOpenItem.bindValue(":apopen_id", _apopenid);
  saveOpenItem.bindValue(":apopen_docnumber", _docNumber->text());
  saveOpenItem.bindValue(":apopen_duedate", _dueDate->date());
  saveOpenItem.bindValue(":apopen_ponumber", _poNumber->text());
  saveOpenItem.bindValue(":apopen_amount", _amount->localValue());
  saveOpenItem.bindValue(":apopen_notes",   _notes->toPlainText());
  saveOpenItem.bindValue(":curr_id", _amount->id());
  saveOpenItem.bindValue(":apopen_terms_id", _terms->id());
  if (_taxzone->isValid())
    saveOpenItem.bindValue(":apopen_taxzone_id", _taxzone->id());
  QString temp;
  if (_status->id() == 1)
    temp = "O" ;
  else
	temp = "H" ;
  saveOpenItem.bindValue(":apopen_status", temp);
  if(!_usePrepaid->isChecked())
    saveOpenItem.bindValue(":apopen_accnt_id", _accntId->id());
  else
    saveOpenItem.bindValue(":apopen_accnt_id", -1);

  saveOpenItem.bindValue(":apopen_doctype", _docType->code());

  saveOpenItem.exec();
  if (saveOpenItem.first())
  {
    if (_mode == cNew)
    {
      if (saveOpenItem.value("result").toInt() == -1)
      {
        QMessageBox::critical( this, tr("Cannot Create A/P Memo"),
                               tr( "<p>The A/P Memo cannot be created as there "
                                  "are missing A/P Account Assignments for the "
                                  "selected Vendor. You must create an A/P "
                                  "Account Assignment for the selected "
                                  "Vendor's Vendor Type before you may create "
                                  "this A/P Memo." ) );
        return;
      }
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Cannot Create A/P Memo"),
                                saveOpenItem, __FILE__, __LINE__))
  {
    return;
  }

  if (_mode == cEdit)
  {
    if(_printOnPost->isChecked())
      sPrintOnPost(_apopenid);
    done(_apopenid);
  }
  else
  {
    if(_printOnPost->isChecked())
    {
      if (_docType->code() == "C")
//    Credit Memo function returns journal# not apopenid so we have to go find it
      {
        XSqlQuery getCMid;
        getCMid.prepare("SELECT apopen_id AS result FROM apopen "
                        "WHERE ((apopen_doctype = 'C') "
                        " AND (apopen_docnumber = :docnumber) "
                        " AND (apopen_journalnumber = :journal));");
        getCMid.bindValue(":docnumber", _docNumber->text());
        getCMid.bindValue(":journal", saveOpenItem.value("result").toInt());
        getCMid.exec();
        if (ErrorReporter::error(QtCriticalMsg, this, tr("Returning apopenid"),
                           getCMid, __FILE__, __LINE__))
        {
          return;
        }
        if (getCMid.first())
        {
          sPrintOnPost(getCMid.value("result").toInt());
        }
      }
      else
      {
//      The Debit Memo returns apopenid so is fine
        sPrintOnPost(saveOpenItem.value("result").toInt());
      }
    }
    done(saveOpenItem.value("result").toInt());
  }
}

void apOpenItem::sClose()
{
  XSqlQuery deleteOpenItem;
  if (_mode == cNew) {
//  Handle new placeholder documents that get orphaned when document is cancelled (#23873)
    if (_apopenid != -1){
      deleteOpenItem.prepare("DELETE FROM apopen WHERE apopen_id = :apopenid;");
      deleteOpenItem.bindValue(":apopenid", _apopenid);
      deleteOpenItem.exec();
    }
    sReleaseNumber();
  }

  reject();
}

void apOpenItem::sReleaseNumber()
{
  XSqlQuery releaseOpenItem;
  if(_seqiss)
  {
    releaseOpenItem.prepare("SELECT releaseAPMemoNumber(:docNumber);");
    releaseOpenItem.bindValue(":docNumber", _seqiss);
    releaseOpenItem.exec();
  }
}

void apOpenItem::sToggleAccount()
{
  _accntId->setEnabled(!_usePrepaid->isChecked());
}

void apOpenItem::populate()
{
  populateStatus();
  XSqlQuery populateOpenItem;
  populateOpenItem.prepare( "SELECT apopen_vend_id, apopen_docdate, apopen_duedate,"
             "       apopen_doctype, apopen_docnumber,"
             "       apopen_ponumber, apopen_journalnumber,"
             "       apopen_amount,   apopen_paid, "
             "       (apopen_amount - apopen_paid) AS f_balance,"
             "       apopen_terms_id, apopen_notes, apopen_accnt_id, "
             "       apopen_curr_id, apopen_taxzone_id, "
			 "       CASE WHEN apopen_status ='O' THEN 1 "
             "         ELSE CASE WHEN apopen_status = 'H' THEN 2 "
             "         END "
             "       END AS status_id, apopen_status, "
             "       (SELECT COALESCE(SUM(taxhist_tax),0) "
             "        FROM apopentax "
             "        WHERE (taxhist_parent_id=apopen_id)) AS tax, "
             "       CASE WHEN (apopen_doctype IN ('D', 'C')) THEN true "
             "            ELSE false "
             "       END AS showTax "
             "FROM apopen "
             "WHERE ( (apopen_id=:apopen_id)"
             "  AND   (apopen_void = false) );" );
  populateOpenItem.bindValue(":apopen_id", _apopenid);
  populateOpenItem.exec();
  if (populateOpenItem.first())
  {
    _vend->setId(populateOpenItem.value("apopen_vend_id").toInt());
    _docDate->setDate(populateOpenItem.value("apopen_docdate").toDate(), true);
    _dueDate->setDate(populateOpenItem.value("apopen_duedate").toDate());
    _docNumber->setText(populateOpenItem.value("apopen_docnumber").toString());
    _poNumber->setText(populateOpenItem.value("apopen_ponumber").toString());
    _journalNumber->setText(populateOpenItem.value("apopen_journalnumber").toString());
    _amount->setId(populateOpenItem.value("apopen_curr_id").toInt());
    _amount->setLocalValue(populateOpenItem.value("apopen_amount").toDouble());
    _paid->setLocalValue(populateOpenItem.value("apopen_paid").toDouble());
    _balance->setLocalValue(populateOpenItem.value("f_balance").toDouble());
    _terms->setId(populateOpenItem.value("apopen_terms_id").toInt());
    _taxzone->setId(populateOpenItem.value("apopen_taxzone_id").toInt());
    if (populateOpenItem.value("apopen_status").toString() == "C")
    {
      QString status;
      status = " SELECT DISTINCT "
            " CASE WHEN apopen_status ='C' THEN 0 "
            " END AS status_id, "
            " CASE WHEN apopen_status ='C' THEN TEXT('Closed') "
            " END AS status, "
            " CASE WHEN apopen_status ='C' THEN TEXT('Closed') "
            " END AS status "
            " FROM apopen "
            " WHERE apopen_status <> '' " ;
          _status->populate(status, -1);
          _status->setEnabled(false);
    }
    _status->setId(populateOpenItem.value("status_id").toInt());
	
    XSqlQuery selectpayment;
    selectpayment.prepare("SELECT * FROM apselect WHERE apselect_apopen_id = :apopen_id;");
    selectpayment.bindValue(":apopen_id", _apopenid);
    selectpayment.exec();
    if (selectpayment.first())
      _status->setEnabled(false);
    _notes->setText(populateOpenItem.value("apopen_notes").toString());
    if (populateOpenItem.value("showTax").toBool())
      _tax->setLocalValue(populateOpenItem.value("tax").toDouble());
    else
    {
      _taxLit->hide();
      _tax->hide();
    }

    if(!populateOpenItem.value("apopen_accnt_id").isNull() && populateOpenItem.value("apopen_accnt_id").toInt() != -1)
    {
      _usePrepaid->setChecked(false);
      _accntId->setId(populateOpenItem.value("apopen_accnt_id").toInt());
    }

    QString docType = populateOpenItem.value("apopen_doctype").toString();
    _docType->setCode(docType);

    _cAmount = _amount->localValue();

    if ( (docType == "V") || (docType == "D") )
    {
      populateOpenItem.prepare( "SELECT apapply_id, apapply_source_apopen_id,"
                 "       CASE WHEN (apapply_source_doctype='C') THEN :creditMemo"
                 "            WHEN (apapply_source_doctype='K') THEN :check"
                 "            ELSE :other"
                 "       END AS doctype,"
                 "       apapply_source_docnumber AS docnumber,"
                 "       apapply_postdate, apapply_amount,"
		 "       currConcat(apapply_curr_id) AS currabbr,"
                 "       'curr' AS apapply_amount_xtnumericrole "
                 "FROM apapply "
                 "WHERE (apapply_target_apopen_id=:apopen_id) "
                 "ORDER BY apapply_postdate;" );

      populateOpenItem.bindValue(":creditMemo", tr("Credit Memo"));
      populateOpenItem.bindValue(":check", tr("Check"));
    }
    else if (docType == "C")
    {
      populateOpenItem.prepare( "SELECT apapply_id, apapply_target_apopen_id,"
                 "       CASE WHEN (apapply_target_doctype='V') THEN :voucher"
                 "            WHEN (apapply_target_doctype='D') THEN :debitMemo"
                 "            ELSE :other"
                 "       END AS doctype,"
                 "       apapply_target_docnumber AS docnumber,"
                 "       apapply_postdate, apapply_amount,"
		 "       currConcat(apapply_curr_id) AS currabbr,"
                 "       'curr' AS apapply_amount_xtnumericrole "
                 "FROM apapply "
                 "WHERE (apapply_source_apopen_id=:apopen_id) "
                 "ORDER BY apapply_postdate;" );

      populateOpenItem.bindValue(":voucher", tr("Voucher"));
      populateOpenItem.bindValue(":debitMemo", tr("Debit Memo"));
    }

    populateOpenItem.bindValue(":apopen_id", _apopenid);
    populateOpenItem.bindValue(":other", tr("Other"));
    populateOpenItem.exec();
    _apapply->populate(populateOpenItem, true);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Information"),
                                  populateOpenItem, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void apOpenItem::sPopulateVendInfo(int vend_id)
{
  XSqlQuery vendor;
  vendor.prepare("SELECT vend_curr_id,"
                 "       vend_terms_id, "
                 "       vend_taxzone_id "
                 "  FROM vendinfo"
                 " WHERE(vend_id = :vend_id);");
  vendor.bindValue(":vend_id", vend_id);
  vendor.exec();
  if (vendor.first())
  {
    _amount->setId(vendor.value("vend_curr_id").toInt());
    _tax->setId(vendor.value("vend_curr_id").toInt());
    _taxzone->setId(vendor.value("vend_taxzone_id").toInt());
    _terms->setId(vendor.value("vend_terms_id").toInt());
  }
}

void apOpenItem::sPopulateDueDate()
{
  if ( (_terms->isValid()) && (_docDate->isValid()) && (!_dueDate->isValid()) )
  {
    XSqlQuery dueq;
    dueq.prepare("SELECT determineDueDate(:terms_id, :docDate) AS duedate;");
    dueq.bindValue(":terms_id", _terms->id());
    dueq.bindValue(":docDate", _docDate->date());
    dueq.exec();
    if (dueq.first())
      _dueDate->setDate(dueq.value("duedate").toDate());
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Setting Due Date"),
                                  dueq, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void apOpenItem::sPrintOnPost(int temp_id)
{
  ParameterList params;
  params.append("apopen_id", temp_id);

  printApOpenItem newdlg(this, "", true);
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

bool apOpenItem::sInitializeMemo()
{
  XSqlQuery ap;
  ap.prepare("SELECT nextval('apopen_apopen_id_seq') AS result;");
  ap.exec();
  if (ap.first())
    _apopenid = ap.value("result").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Initializing Memo"),
                                  ap, __FILE__, __LINE__))
    return false;
  else
    return false;
    
  ap.prepare("INSERT INTO apopen "
    "( apopen_id, apopen_docdate, apopen_duedate, apopen_doctype, "
    "  apopen_docnumber, apopen_curr_id, apopen_posted, apopen_amount ) "
    "VALUES "
    "( :apopen_id, :docDate, :dueDate, :docType, :docNumber, :currId, false, 0 ); ");
  ap.bindValue(":apopen_id",_apopenid);
  ap.bindValue(":docDate", _docDate->date());
  ap.bindValue(":dueDate", _dueDate->date());
  ap.bindValue(":docType", _docType->code() );
  ap.bindValue(":docNumber", _docNumber->text());
  ap.bindValue(":currId", _amount->id());
  ap.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Initializing Memo"),
                                  ap, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

void apOpenItem::sTaxDetail()
{
  XSqlQuery ap;
  if (_apopenid == -1)
  {

    QList<GuiErrorCheck>errors;
    errors<<GuiErrorCheck(!_docDate->isValid(), _docDate,
                            tr("You must enter a document date for this A/P Memo before you may set tax amounts."))
           <<GuiErrorCheck(!_dueDate->isValid(), _dueDate,
                            tr("You must enter a due date for this A/P Memo before you may set tax amounts."))
           <<GuiErrorCheck(_amount->isZero(), _amount,
                            tr("You must enter an amount for this Payable Memo before you may set tax amounts."));

    if(GuiErrorCheck::reportErrors(this,tr("Cannot Set Tax Amounts"),errors))
        return;

    if (!sInitializeMemo())
      return;
  }
  
  taxDetail newdlg(this, "", true);
  ParameterList params;

  params.append("curr_id", _tax->id());
  params.append("date",    _tax->effective());
  if (_mode != cNew)
    params.append("readOnly");

  ap.exec("SELECT getadjustmenttaxtypeid() as taxtype;");
  if(ap.first())
    params.append("taxtype_id", ap.value("taxtype").toInt());  
   
  params.append("order_type", "AP");
  params.append("order_id", _apopenid);
  params.append("display_type", "A");
  params.append("subtotal", _amount->localValue());
  params.append("adjustment");
  if (_docType->code() == "D")
    params.append("sense",-1);
  if (newdlg.set(params) == NoError)  
  {
    newdlg.exec();
    XSqlQuery taxq;
    taxq.prepare( "SELECT SUM(taxhist_tax) AS tax "
      "FROM apopentax "
      "WHERE (taxhist_parent_id=:apopen_id);" );
    taxq.bindValue(":apopen_id", _apopenid);
    taxq.exec();
    if (taxq.first())
    {
      if (_docType->code() == "D")
        _tax->setLocalValue(taxq.value("tax").toDouble() * -1);
      else
        _tax->setLocalValue(taxq.value("tax").toDouble());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Setting Tax Amounts"),
                                  taxq, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void apOpenItem::sDetermineTaxAmount()
{
  if (_mode != cNew)
    return;

  XSqlQuery ap;
  if (_apopenid == -1)
  {
    if (!_docDate->isValid() || !_dueDate->isValid() || _amount->isZero())
      return;
    if (!sInitializeMemo())
      return;
  }
  ap.prepare( "SELECT updatememotax(:source, :doctype, :apopen_id, :taxzone, :date, :curr, :amount) AS tax;" );

  ap.bindValue(":source",    "AP");
  ap.bindValue(":doctype",   _docType->code());
  ap.bindValue(":apopen_id", _apopenid);
  ap.bindValue(":taxzone",   _taxzone->id());
  ap.bindValue(":date",      _tax->effective());
  ap.bindValue(":curr",      _tax->id());
  ap.bindValue(":amount",    _amount->localValue());
  ap.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Returning AP Open Tax"),
                           ap, __FILE__, __LINE__))
    return;

  if (ap.first())
    _tax->setLocalValue(ap.value("tax").toDouble());
}
 
void apOpenItem::populateStatus()
{
  QString status;
  status = "SELECT 1 AS status_id, TEXT('Open') AS status, TEXT('Open') AS status "
           "UNION "
           "SELECT 2 AS status_id, TEXT('On Hold') AS status, TEXT('On Hold') AS status;";
  _status->populate(status, -1);
}

void apOpenItem::sCalcBalance()
{
  _balance->setLocalValue(_amount->localValue() - _paid->localValue());
}
