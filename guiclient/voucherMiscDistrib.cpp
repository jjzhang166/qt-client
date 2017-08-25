/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "voucherMiscDistrib.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "guiErrorCheck.h"
#include "errorReporter.h"

voucherMiscDistrib::voucherMiscDistrib(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save,            SIGNAL(clicked()),     this, SLOT(sSave()));
  connect(_taxCode,         SIGNAL(newID(int)),    this, SLOT(sCheck()));
  connect(_accountSelected, SIGNAL(toggled(bool)), this, SLOT(sHandleSelection()));
  connect(_expcatSelected,  SIGNAL(toggled(bool)), this, SLOT(sHandleSelection()));
  connect(_freightSelected, SIGNAL(toggled(bool)), this, SLOT(sHandleSelection()));
  connect(_taxSelected,     SIGNAL(toggled(bool)), this, SLOT(sHandleSelection()));
  
  _account->setType(GLCluster::cRevenue | GLCluster::cExpense |
                    GLCluster::cAsset | GLCluster::cLiability);

  _freightCostElement->populate("SELECT costelem_id, costelem_type "
                                "FROM costelem "
                                "WHERE ( (costelem_active)"
                                " AND (NOT costelem_sys)"
                                " AND (costelem_po) ) "
                                "ORDER BY costelem_type;");

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

  _freightSelected->setVisible(_miscvoucher);
  _freightGroup->setVisible(_miscvoucher);
 
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

      if (_metrics->value("DefaultFreightCostElement").toInt() > 0)
        _freightCostElement->setId(_metrics->value("DefaultFreightCostElement").toInt());
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      sCheck();
    }
  }

  param = pParams.value("vend_id", &valid);
  if (valid && _miscvoucher)
    _origVoucher->setExtraClause("vohead_posted AND NOT COALESCE(vohead_misc, false)");

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
    if (vpopulateVoucher.value("vodist_expcat_id").toInt() != -1)
    {
      _expcatSelected->setChecked(true);
      _expcat->setId(vpopulateVoucher.value("vodist_expcat_id").toInt());
    }
    if (vpopulateVoucher.value("vodist_costelem_id").toInt() != -1)
    {
      _freightSelected->setChecked(true);
      _origVoucher->setId(vpopulateVoucher.value("vodist_freight_vohead_id").toInt());
      _freightDistQty->setChecked(vpopulateVoucher.value("vodist_freight_dist_method").toString() == "Q");
      _freightDistVal->setChecked(vpopulateVoucher.value("vodist_freight_dist_method").toString() == "V");
      _freightDistWgt->setChecked(vpopulateVoucher.value("vodist_freight_dist_method").toString() == "W");
    }
    if (vpopulateVoucher.value("vodist_tax_id").toInt() != -1)
    {
      _taxSelected->setChecked(true);
      _taxCode->setId(vpopulateVoucher.value("vodist_tax_id").toInt());
    }
    if (vpopulateVoucher.value("vodist_taxtype_id").toInt() > 0)
    {
      _taxType->setId(vpopulateVoucher.value("vodist_taxtype_id").toInt());
    }
  }
}

void voucherMiscDistrib::sSave()
{
  XSqlQuery saveVoucher;
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_accountSelected->isChecked() && !_account->isValid(),
                           _account,
                           tr("<p>You must select an Account to post this "
                              "Miscellaneous Distribution to."))
         << GuiErrorCheck(_expcatSelected->isChecked() && !_expcat->isValid(),
                           _expcat,
                           tr("<p>You must select an Expense Category to post this "
                              "Miscellaneous Distribution to."))
         << GuiErrorCheck(_taxSelected->isChecked() && !_taxCode->isValid(),
                           _taxCode,
                           tr("<p>You must select an Tax Code to post this "
                              "Miscellaneous Distribution to."))
         << GuiErrorCheck(_freightSelected->isChecked() && !_origVoucher->isValid(),
                           _origVoucher,
                           tr("<p>You must define the original Voucher, item cost element, "
                              "and method to distribute this freight expense to."))
  ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Voucher Distribution"), errors))
    return;

  if (_mode == cNew)
  {
    saveVoucher.exec("SELECT NEXTVAL('vodist_vodist_id_seq') AS _vodistid;");
    if (saveVoucher.first())
      _vodistid = saveVoucher.value("_vodistid").toInt();
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Voucher Information"),
                           saveVoucher, __FILE__, __LINE__);
      return;
    }

    saveVoucher.prepare( "INSERT INTO vodist "
               "( vodist_id, vodist_vohead_id, vodist_poitem_id,"
               "  vodist_costelem_id, vodist_accnt_id, vodist_amount, vodist_discountable,"
               "  vodist_expcat_id, vodist_tax_id, vodist_notes, vodist_taxtype_id, "
               "  vodist_freight_vohead_id, vodist_freight_dist_method ) "
               "VALUES "
               "( :vodist_id, :vodist_vohead_id, -1,"
               "  :vodist_costelem_id, :vodist_accnt_id, :vodist_amount, :vodist_discountable,"
               "  :vodist_expcat_id, :vodist_tax_id, :vodist_notes, :vodist_taxtype_id, "
               "  :vodist_voucher, :vodist_method );" );
  }
  else if (_mode == cEdit)
    saveVoucher.prepare( "UPDATE vodist "
               "SET vodist_accnt_id=:vodist_accnt_id,"
               "    vodist_amount=:vodist_amount,"
               "    vodist_discountable=:vodist_discountable,"
               "    vodist_expcat_id=:vodist_expcat_id, "
               "    vodist_tax_id = :vodist_tax_id, "
               "    vodist_notes=:vodist_notes, "
               "    vodist_taxtype_id=:vodist_taxtype_id, "
               "    vodist_costelem_id=:vodist_costelem_id, "
               "    vodist_freight_vohead_id=:vodist_voucher, "
               "    vodist_freight_dist_method=:vodist_method "
               "WHERE (vodist_id=:vodist_id);" );
  
  saveVoucher.bindValue(":vodist_id", _vodistid);
  saveVoucher.bindValue(":vodist_vohead_id", _voheadid);
  saveVoucher.bindValue(":vodist_amount", _amount->localValue());
  saveVoucher.bindValue(":vodist_discountable", QVariant(_discountable->isChecked()));
  saveVoucher.bindValue(":vodist_notes", _notes->toPlainText().trimmed());
  if(_taxType->isValid())
    saveVoucher.bindValue(":vodist_taxtype_id", _taxType->id());

  if(_accountSelected->isChecked())
  {
    saveVoucher.bindValue(":vodist_accnt_id", _account->id());
    saveVoucher.bindValue(":vodist_expcat_id", -1);
    saveVoucher.bindValue(":vodist_tax_id", -1);
    saveVoucher.bindValue(":vodist_costelem_id", -1);
  }
  else if (_expcatSelected->isChecked())
  {
    saveVoucher.bindValue(":vodist_accnt_id", -1);
    saveVoucher.bindValue(":vodist_expcat_id", _expcat->id());
    saveVoucher.bindValue(":vodist_tax_id", -1);
    saveVoucher.bindValue(":vodist_costelem_id", -1);
  }
  else if (_freightSelected->isChecked())
  {
    saveVoucher.bindValue(":vodist_accnt_id", -1);
    saveVoucher.bindValue(":vodist_expcat_id", -1);
    saveVoucher.bindValue(":vodist_tax_id", -1);
    saveVoucher.bindValue(":vodist_costelem_id", _freightCostElement->id());
    saveVoucher.bindValue(":vodist_voucher", _origVoucher->id());
    if (_freightDistQty->isChecked())
      saveVoucher.bindValue(":vodist_method", "Q");
    else if (_freightDistVal->isChecked())
      saveVoucher.bindValue(":vodist_method", "V");
    else if (_freightDistWgt->isChecked())
      saveVoucher.bindValue(":vodist_method", "W");
  }
  else
  {
    saveVoucher.bindValue(":vodist_accnt_id", -1);
    saveVoucher.bindValue(":vodist_expcat_id", -1);
    saveVoucher.bindValue(":vodist_costelem_id", -1);
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
	_freightSelected->setEnabled(false);
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
	_freightSelected->setEnabled(false);
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

void voucherMiscDistrib::sHandleSelection()
{
  bool onoff = _accountSelected->isChecked() || _expcatSelected->isChecked()
              || _freightSelected->isChecked();
  _taxTypeLit->setEnabled(onoff);
  _taxType->setEnabled(onoff);
  _discountable->setEnabled(onoff);
  _discountable->setChecked(onoff);
}
