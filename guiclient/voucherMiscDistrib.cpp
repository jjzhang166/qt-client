/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "voucherMiscDistrib.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

voucherMiscDistrib::voucherMiscDistrib(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxCode, SIGNAL(newID(int)), this, SLOT(sCheck()));
  
  _account->setType(GLCluster::cRevenue | GLCluster::cExpense |
                    GLCluster::cAsset | GLCluster::cLiability);

  adjustSize();
}

voucherMiscDistrib::~voucherMiscDistrib()
{
  // no need to delete child widgets, Qt does it all for us
}

void voucherMiscDistrib::languageChange()
{
  retranslateUi(this);
}

enum SetResponse voucherMiscDistrib::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vohead_id", &valid);
  if (valid)
    _voheadid = param.toInt();
 
  _miscvoucher = pParams.inList("voucher_type");
 
  param = pParams.value("curr_id", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("curr_effective", &valid);
  if (valid)
    _amount->setEffective(param.toDate());

  param = pParams.value("vodist_id", &valid);
  if (valid)
  {
    _vodistid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      param = pParams.value("vend_id", &valid);
      if (valid)
        sPopulateVendorInfo(param.toInt());
      param = pParams.value("amount", &valid);
      if (valid)
        _amount->setLocalValue(param.toDouble());
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      sCheck();
    }
  }

  return NoError;
}

void voucherMiscDistrib::populate()
{
  XSqlQuery vpopulateVoucher;
  vpopulateVoucher.prepare( "SELECT * "
             "FROM vodist "
             "WHERE (vodist_id=:vodist_id);" ) ;
  vpopulateVoucher.bindValue(":vodist_id", _vodistid);
  vpopulateVoucher.exec();
  if (vpopulateVoucher.first())
  {
    _account->setId(vpopulateVoucher.value("vodist_accnt_id").toInt());
    _amount->setLocalValue(vpopulateVoucher.value("vodist_amount").toDouble());
    _discountable->setChecked(vpopulateVoucher.value("vodist_discountable").toBool());
    _notes->setText(vpopulateVoucher.value("vodist_notes").toString());
    if(vpopulateVoucher.value("vodist_expcat_id").toInt() != -1)
    {
      _expcatSelected->setChecked(true);
      _expcat->setId(vpopulateVoucher.value("vodist_expcat_id").toInt());
    }
    if(vpopulateVoucher.value("vodist_tax_id").toInt() != -1)
    {
	   _taxSelected->setChecked(true);
	   _taxCode->setId(vpopulateVoucher.value("vodist_tax_id").toInt());
    }
  }
}

void voucherMiscDistrib::sSave()
{
  XSqlQuery saveVoucher;
  if (_accountSelected->isChecked() && !_account->isValid())
  {
    QMessageBox::warning( this, tr("Select Account"),
                          tr("You must select an Account to post this Miscellaneous Distribution to.") );
    _account->setFocus();
    return;
  }

  if (_expcatSelected->isChecked() && !_expcat->isValid())
  {
    QMessageBox::warning( this, tr("Select Expense Category"),
                          tr("You must select an Expense Category to post this Miscellaneous Distribution to.") );
    _account->setFocus();
    return;
  }

  if (_taxSelected->isChecked() && !_taxCode->isValid())
  {
    QMessageBox::warning( this, tr("Select Tax Code"),
                          tr("You must select a Tax Code to post this Miscellaneous Distribution to.") );
    _account->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    saveVoucher.exec("SELECT NEXTVAL('vodist_vodist_id_seq') AS _vodistid;");
    if (saveVoucher.first())
      _vodistid = saveVoucher.value("_vodistid").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    saveVoucher.prepare( "INSERT INTO vodist "
               "( vodist_id, vodist_vohead_id, vodist_poitem_id,"
               "  vodist_costelem_id, vodist_accnt_id, vodist_amount, vodist_discountable,"
               "  vodist_expcat_id, vodist_tax_id, vodist_notes ) "
               "VALUES "
               "( :vodist_id, :vodist_vohead_id, -1,"
               "  -1, :vodist_accnt_id, :vodist_amount, :vodist_discountable,"
               "  :vodist_expcat_id, :vodist_tax_id, :vodist_notes );" );
  }
  else if (_mode == cEdit)
    saveVoucher.prepare( "UPDATE vodist "
               "SET vodist_accnt_id=:vodist_accnt_id,"
               "    vodist_amount=:vodist_amount,"
               "    vodist_discountable=:vodist_discountable,"
               "    vodist_expcat_id=:vodist_expcat_id, "
               "    vodist_tax_id = :vodist_tax_id, "
			   "    vodist_notes=:vodist_notes "
               "WHERE (vodist_id=:vodist_id);" );
  
  saveVoucher.bindValue(":vodist_id", _vodistid);
  saveVoucher.bindValue(":vodist_vohead_id", _voheadid);
  saveVoucher.bindValue(":vodist_amount", _amount->localValue());
  saveVoucher.bindValue(":vodist_discountable", QVariant(_discountable->isChecked()));
  saveVoucher.bindValue(":vodist_notes", _notes->toPlainText().trimmed());
  if(_accountSelected->isChecked())
  {
    saveVoucher.bindValue(":vodist_accnt_id", _account->id());
    saveVoucher.bindValue(":vodist_expcat_id", -1);
    saveVoucher.bindValue(":vodist_tax_id", -1);
  }
  else if (_expcatSelected->isChecked())
  {
    saveVoucher.bindValue(":vodist_accnt_id", -1);
    saveVoucher.bindValue(":vodist_expcat_id", _expcat->id());
    saveVoucher.bindValue(":vodist_tax_id", -1);
  }
  else
  {
    saveVoucher.bindValue(":vodist_accnt_id", -1);
    saveVoucher.bindValue(":vodist_expcat_id", -1);
    saveVoucher.bindValue(":vodist_tax_id", _taxCode->id()); 
  }
  saveVoucher.exec();

  done(_vodistid);
}

void voucherMiscDistrib::sCheck()     
{
  XSqlQuery checkVoucher;
 if(_taxSelected->isChecked() && _mode == cEdit)
 {
  checkVoucher.prepare( "SELECT vodist_id, vodist_vohead_id, vodist_amount "
              "FROM vodist " 
              "WHERE (vodist_id=:vodist_id) "
			  "   AND (vodist_poitem_id=-1)"
              "   AND (vodist_tax_id=:tax_id);" );
  
  checkVoucher.bindValue(":vodist_id", _vodistid);
  checkVoucher.bindValue(":tax_id", _taxCode->id());
  checkVoucher.exec();
  if (checkVoucher.first())
  {
    _amount->setFocus();
	_taxSelected->setEnabled(false);
	_taxCode->setEnabled(false);
	_expcatSelected->setEnabled(false);
	_accountSelected->setEnabled(false);
  }
 }
else if(_taxSelected->isChecked() && _mode == cNew)
{
   checkVoucher.prepare( "SELECT vodist_id, vodist_amount "
              "FROM vodist " 
              "WHERE (vodist_vohead_id=:vodist_vohead_id) "
			  "   AND (vodist_poitem_id=-1)"
              "   AND (vodist_tax_id=:tax_id);" );
  
  checkVoucher.bindValue(":vodist_vohead_id", _voheadid);
  checkVoucher.bindValue(":tax_id", _taxCode->id());
  checkVoucher.exec();
  if (checkVoucher.first())
  {
    _vodistid=checkVoucher.value("vodist_id").toInt();
    _amount->setLocalValue(checkVoucher.value("vodist_amount").toDouble());
	_amount->setFocus();
	_mode=cEdit;
	_taxSelected->setEnabled(false);
	_taxCode->setEnabled(false);
	_expcatSelected->setEnabled(false);
	_accountSelected->setEnabled(false);
  }
 }
}

void voucherMiscDistrib::sPopulateVendorInfo(int pVendid)
{
  XSqlQuery populateVoucher;
  populateVoucher.prepare("SELECT COALESCE(vend_accnt_id, -1) AS vend_accnt_id,"
                          "       COALESCE(vend_expcat_id, -1) AS vend_expcat_id,"
                          "       COALESCE(vend_tax_id, -1) AS vend_tax_id "
                          "FROM vendinfo "
                          "WHERE (vend_id=:vend_id);" );
  populateVoucher.bindValue(":vend_id", pVendid);
  populateVoucher.exec();
  if (populateVoucher.first())
  {
    if(populateVoucher.value("vend_accnt_id").toInt() != -1)
    {
      _accountSelected->setChecked(true);
      _account->setId(populateVoucher.value("vend_accnt_id").toInt());
      _amount->setFocus();
    }
    if(populateVoucher.value("vend_expcat_id").toInt() != -1)
    {
      _expcatSelected->setChecked(true);
      _expcat->setId(populateVoucher.value("vend_expcat_id").toInt());
      _amount->setFocus();
    }
    if(populateVoucher.value("vend_tax_id").toInt() != -1)
    {
      _taxSelected->setChecked(true);
      _taxCode->setId(populateVoucher.value("vend_tax_id").toInt());
      _amount->setFocus();
    }
  }
}

