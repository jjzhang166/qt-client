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

#include "apOpenItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include <QSqlError>

/*
 *  Constructs a apOpenItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
apOpenItem::apOpenItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_vend, SIGNAL(newId(int)), this, SLOT(sPopulateVendInfo(int)));
  connect(_amount, SIGNAL(idChanged(int)), _paid, SLOT(setId(int)));
  connect(_amount, SIGNAL(idChanged(int)), _balance, SLOT(setId(int)));
  connect(_amount, SIGNAL(effectiveChanged(const QDate&)), _paid, SLOT(setEffective(const QDate&)));
  connect(_amount, SIGNAL(effectiveChanged(const QDate&)), _balance, SLOT(setEffective(const QDate&)));
  connect(_docDate, SIGNAL(newDate(const QDate&)), _amount, SLOT(setEffective(const QDate&)));

  _cAmount = 0.0;

  _apapply->addColumn( tr("Type"),        _dateColumn, Qt::AlignCenter,true, "doctype");
  _apapply->addColumn( tr("Doc. #"),               -1, Qt::AlignLeft,  true, "docnumber");
  _apapply->addColumn( tr("Apply Date"),  _dateColumn, Qt::AlignCenter,true, "apapply_postdate");
  _apapply->addColumn( tr("Amount"),     _moneyColumn, Qt::AlignRight, true, "apapply_amount");
  _apapply->addColumn( tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currabbr");

  if (omfgThis->singleCurrency())
      _apapply->hideColumn(4);

  _terms->setType(XComboBox::APTerms);
  _journalNumber->setEnabled(FALSE);
}

/*
 *  Destroys the object and frees any allocated resources
 */
apOpenItem::~apOpenItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void apOpenItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse apOpenItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  
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
    else if (param.toString() == "voucher")
      _docType->setCurrentItem(2);
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

      q.exec("SELECT fetchAPMemoNumber() AS number;");
      if (q.first())
        _docNumber->setText(q.value("number").toString());
//  ToDo

      _paid->clear();
      _save->setText(tr("Post"));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _vend->setReadOnly(TRUE);
      _docDate->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _docType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);
      _journalNumber->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _notes->setReadOnly(FALSE);
      _altPrepaid->setEnabled(FALSE);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _vend->setReadOnly(TRUE);
      _docDate->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _docType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);
      _journalNumber->setEnabled(FALSE);
      _amount->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _altPrepaid->setEnabled(FALSE);
      _save->hide();
      _close->setText(tr("&Close"));
    }
  }
  
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
  if (_mode == cNew)
  {
    if (!_docDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot Save A/P Memo"),
                             tr("You must enter a date for this A/P Memo before you may save it") );
      _docDate->setFocus();
      return;
    }

    if (!_dueDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot Save A/P Memo"),
                             tr("You must enter a date for this A/P Memo before you may save it") );
      _dueDate->setFocus();
      return;
    }

    if (_amount->isZero())
    {
      QMessageBox::critical( this, tr("Cannot Save A/P Memo"),
                             tr("You must enter an amount for this A/P Memo before you may save it") );
      _amount->setFocus();
      return;
    }

    if (_altPrepaid->isChecked() && (!_altAccntid->isValid()))
    {
      QMessageBox::critical( this, tr("Cannot Save A/P Memo"),
        tr("You must choose a valid Alternate Prepaid Account Number for this A/P Memo before you may save it.") );
      return;
    }

    QString tmpFunctionName;
    QString queryStr;

    if (_docType->currentItem() == 0)
      tmpFunctionName = "createAPCreditMemo";
    else if (_docType->currentItem() == 1)
      tmpFunctionName = "createAPDebitMemo";
    else
    {
      systemError(this,
		  tr("Internal Error: _docType has an invalid document type %1")
		  .arg(_docType->currentItem()), __FILE__, __LINE__);
      return;
    }

    queryStr = "SELECT " + tmpFunctionName + "( :vend_id, " +
		(_journalNumber->text().isEmpty() ?
		      QString("fetchJournalNumber('AP-MISC')") : _journalNumber->text()) +
	       ", :apopen_docnumber, :apopen_ponumber, :apopen_docdate,"
	       "  :apopen_amount, :apopen_notes, :apopen_accnt_id,"
	       "  :apopen_duedate, :apopen_terms_id, :curr_id ) AS result;";
    q.prepare(queryStr);
    q.bindValue(":vend_id", _vend->id());
    q.bindValue(":apopen_docdate", _docDate->date());
  }
  else if (_mode == cEdit)
  {
    if (_cAmount != _amount->localValue())
      if ( QMessageBox::warning( this, tr("A/P Open Amount Changed"),
                                 tr( "You are changing the open amount of this A/P Open Item.  If you do not post a G/L Transaction\n"
                                     "to distribute this change then the A/P Open Item total will be out of balance with the\n"
                                     "A/P Trial Balance(s).\n"
                                     "Are you sure that you want to save this change?" ),
                                 tr("Yes"), tr("No"), QString::null ) == 1 )
        return;

    q.prepare( "UPDATE apopen "
	       "SET apopen_doctype=:apopen_doctype,"
               "    apopen_ponumber=:apopen_ponumber, apopen_docnumber=:apopen_docnumber,"
               "    apopen_amount=:apopen_amount,"
               "    apopen_terms_id=:apopen_terms_id, "
	       "    apopen_notes=:apopen_notes, "
	       "    apopen_curr_id=:curr_id "
               "WHERE (apopen_id=:apopen_id);" );
    q.bindValue(":apopen_id", _apopenid);
  }

  q.bindValue(":apopen_docnumber", _docNumber->text());
  q.bindValue(":apopen_duedate", _dueDate->date());
  q.bindValue(":apopen_ponumber", _poNumber->text());
  q.bindValue(":apopen_amount", _amount->localValue());
  q.bindValue(":apopen_notes", _notes->text());
  q.bindValue(":curr_id", _amount->id());
  q.bindValue(":apopen_terms_id", _terms->id());
  if(_altPrepaid->isChecked())
    q.bindValue(":apopen_accnt_id", _altAccntid->id());
  else
    q.bindValue(":apopen_accnt_id", -1);

  switch (_docType->currentItem())
  {
    case 0:
      q.bindValue(":apopen_doctype", "C");
      break;

    case 1:
      q.bindValue(":apopen_doctype", "D");
      break;

    case 2:
      q.bindValue(":apopen_doctype", "V");
      break;
  }

  q.exec();
  if (q.first())
  {
    if (_mode == cNew)
    {
      if (q.value("result").toInt() == -1)
      {
        QMessageBox::critical( this, tr("Cannot Create A/P Memo"),
                               tr( "The A/P Memo cannot be created as there are missing A/P Account Assignments for the selected Vendor.\n"
                                   "You must create an A/P Account Assignment for the selected Vendor's Vendor Type before\n"
                                   "you may create this A/P Memo." ) );
        return;
      }
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cEdit)
    done(_apopenid);
  else
    done(q.value("result").toInt());
}

void apOpenItem::sClose()
{
  if (_mode == cNew)
  {
    q.prepare("SELECT releaseAPMemoNumber(:docNumber);");
    q.bindValue(":docNumber", _docNumber->text().toInt());
    q.exec();
  }

  reject();
}

void apOpenItem::populate()
{
  q.prepare( "SELECT apopen_vend_id, apopen_docdate, apopen_duedate,"
             "       apopen_doctype, apopen_docnumber,"
             "       apopen_ponumber, apopen_journalnumber,"
             "       apopen_amount,   apopen_paid, "
             "       (apopen_amount - apopen_paid) AS f_balance,"
             "       apopen_terms_id, apopen_notes, apopen_accnt_id, "
	     "       apopen_curr_id "
             "FROM apopen "
             "WHERE (apopen_id=:apopen_id);" );
  q.bindValue(":apopen_id", _apopenid);
  q.exec();
  if (q.first())
  {
    _vend->setId(q.value("apopen_vend_id").toInt());
    _docDate->setDate(q.value("apopen_docdate").toDate(), true);
    _dueDate->setDate(q.value("apopen_duedate").toDate());
    _docNumber->setText(q.value("apopen_docnumber").toString());
    _poNumber->setText(q.value("apopen_ponumber").toString());
    _journalNumber->setText(q.value("apopen_journalnumber").toString());
    _amount->setId(q.value("apopen_curr_id").toInt());
    _amount->setLocalValue(q.value("apopen_amount").toDouble());
    _paid->setLocalValue(q.value("apopen_paid").toDouble());
    _balance->setLocalValue(q.value("f_balance").toDouble());
    _terms->setId(q.value("apopen_terms_id").toInt());
    _notes->setText(q.value("apopen_notes").toString());

    if(!q.value("apopen_accnt_id").isNull() && q.value("apopen_accnt_id").toInt() != -1)
    {
      _altPrepaid->setChecked(true);
      _altAccntid->setId(q.value("apopen_accnt_id").toInt());
    }

    QString docType = q.value("apopen_doctype").toString();
    if (docType == "C")
      _docType->setCurrentItem(0);
    else if (docType == "D")
      _docType->setCurrentItem(1);
    else if (docType == "V")
      _docType->setCurrentItem(2);

    _cAmount = _amount->localValue();

    if ( (docType == "V") || (docType == "D") )
    {
      q.prepare( "SELECT apapply_id, apapply_source_apopen_id,"
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

      q.bindValue(":creditMemo", tr("C/M"));
      q.bindValue(":check", tr("Check"));
    }
    else if (docType == "C")
    {
      q.prepare( "SELECT apapply_id, apapply_target_apopen_id,"
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

      q.bindValue(":voucher", tr("Voucher"));
      q.bindValue(":debitMemo", tr("D/M"));
    }

    q.bindValue(":apopen_id", _apopenid);
    q.bindValue(":other", tr("Other"));
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    _apapply->populate(q, TRUE);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void apOpenItem::sPopulateVendInfo(int vend_id)
{
  XSqlQuery vendor;
  vendor.prepare("SELECT vend_curr_id,"
                 "       vend_terms_id "
                 "  FROM vend"
                 " WHERE(vend_id = :vend_id);");
  vendor.bindValue(":vend_id", vend_id);
  vendor.exec();
  if (vendor.first())
  {
    _amount->setId(vendor.value("vend_curr_id").toInt());
    _terms->setId(vendor.value("vend_terms_id").toInt());
  }
}

