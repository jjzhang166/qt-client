/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "cashReceiptMiscDistrib.h"

#include <metasql.h>
#include <errorReporter.h>

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "guiErrorCheck.h"

cashReceiptMiscDistrib::cashReceiptMiscDistrib(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  _account->setType(GLCluster::cRevenue | GLCluster::cExpense |
                    GLCluster::cAsset | GLCluster::cLiability);
  adjustSize();
}

cashReceiptMiscDistrib::~cashReceiptMiscDistrib()
{
  // no need to delete child widgets, Qt does it all for us
}

void cashReceiptMiscDistrib::languageChange()
{
  retranslateUi(this);
}

enum SetResponse cashReceiptMiscDistrib::set(const ParameterList &pParams)
{

  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cashrcpt_id", &valid);
  if (valid)
    _cashrcptid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("effective", &valid);
  if (valid)
    _amount->setEffective(param.toDate());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
  }

  param = pParams.value("cashrcptmisc_id", &valid);
  if (valid)
  {
    _cashrcptmiscid = param.toInt();
    if (_mode == cEdit)
      populate();
  }

  param = pParams.value("custSelector", &valid);
  if (valid && param.toString() == "G")
    setGroup(pParams);
  if (valid && param.toString() == "C")
  {
    _custSelector->setVisible(false);
    _custSelLit->setVisible(false);
  }

  return NoError;
}

void cashReceiptMiscDistrib::populate()
{
  XSqlQuery cashpopulate;
  cashpopulate.prepare( "SELECT cashrcptmisc_accnt_id, cashrcptmisc_tax_id, cashrcptmisc_notes,"
             "       cashrcptmisc_amount, cashrcpt_curr_id, cashrcpt_distdate "
             "FROM cashrcptmisc JOIN cashrcpt ON (cashrcptmisc_cashrcpt_id = cashrcpt_id) "
             "WHERE (cashrcptmisc_id=:cashrcptmisc_id);" );
  cashpopulate.bindValue(":cashrcptmisc_id", _cashrcptmiscid);
  cashpopulate.exec();
  if (cashpopulate.first())
  {
    _account->setId(cashpopulate.value("cashrcptmisc_accnt_id").toInt());
    _amount->set(cashpopulate.value("cashrcptmisc_amount").toDouble(),
    		 cashpopulate.value("cashrcpt_curr_id").toInt(),
		 cashpopulate.value("cashrcpt_distdate").toDate(), false);
    _notes->setText(cashpopulate.value("cashrcptmisc_notes").toString());
    if (cashpopulate.value("cashrcptmisc_tax_id").toInt() > 0)
    {
      _taxCodeSelected->setChecked(true);
      _taxCode->setId(cashpopulate.value("cashrcptmisc_tax_id").toInt());
    }
  }
}

void cashReceiptMiscDistrib::sSave()
{
  XSqlQuery cashSave;

  QList<GuiErrorCheck>errors;
  errors<<GuiErrorCheck(!_account->isValid() && !_taxCode->isValid(), _account,
                        tr("You must select an Account or Tax Code to post this Miscellaneous Distribution to."))
       <<GuiErrorCheck(_amount->isZero(), _amount,
                       tr("You must enter an amount for this Miscellaneous Distribution."));

  if(GuiErrorCheck::reportErrors(this,tr("Cannot Save Transaction"),errors))
      return;

  if (_mode == cNew)
    cashSave.prepare( "INSERT INTO cashrcptmisc "
               "( cashrcptmisc_cashrcpt_id,"
               "  cashrcptmisc_accnt_id, cashrcptmisc_amount,"
               "  cashrcptmisc_tax_id, cashrcptmisc_notes, "
               "  cashrcptmisc_cust_id ) "
               "VALUES "
               "( :cashrcptmisc_cashrcpt_id,"
               "  :cashrcptmisc_accnt_id, :cashrcptmisc_amount,"
               "  :cashrcptmisc_tax_id, :cashrcptmisc_notes, "
               "  :cashrcptmisc_cust_id ) "
               " RETURNING cashrcptmisc_id AS cashrcptmiscid;" );
  else if (_mode == cEdit)
    cashSave.prepare( "UPDATE cashrcptmisc "
               "SET cashrcptmisc_accnt_id=:cashrcptmisc_accnt_id,"
               "    cashrcptmisc_tax_id=:cashrcptmisc_tax_id, "
               "    cashrcptmisc_amount=:cashrcptmisc_amount, "
               "    cashrcptmisc_cust_id=:cashrcptmisc_cust_id, "
               "    cashrcptmisc_notes=:cashrcptmisc_notes "
               "WHERE (cashrcptmisc_id=:cashrcptmisc_id) "
               " RETURNING cashrcptmisc_id AS cashrcptmiscid;" );

  cashSave.bindValue(":cashrcptmisc_id", _cashrcptmiscid);
  cashSave.bindValue(":cashrcptmisc_cashrcpt_id", _cashrcptid);
  if (_account->isValid())
    cashSave.bindValue(":cashrcptmisc_accnt_id", _account->id());
  if (_taxCodeSelected->isChecked() && _taxCode->isValid())
    cashSave.bindValue(":cashrcptmisc_tax_id", _taxCode->id());
  cashSave.bindValue(":cashrcptmisc_amount", _amount->localValue());
  cashSave.bindValue(":cashrcptmisc_notes",       _notes->toPlainText().trimmed());
  if (_custSelector->id() > 0)
    cashSave.bindValue(":cashrcptmisc_cust_id", _custSelector->id());
  cashSave.exec();
  if (cashSave.first())
    _cashrcptmiscid = cashSave.value("cashrcptmiscid").toInt();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Cash Receipt"),
                           cashSave, __FILE__, __LINE__))
      return;

  done(_cashrcptmiscid);
}

//cash_receipt_by_customer_group
void cashReceiptMiscDistrib::showCustomers(int group, int customer)
{
  if (group==0)
    return;

  //Set Up Customer widget
  XSqlQuery query;
  query.prepare("SELECT custinfo.cust_id,custinfo.cust_name FROM custinfo, custgrpitem "
                "WHERE custgrpitem.custgrpitem_cust_id = custinfo.cust_id "
                "AND (custgrpitem_custgrp_id =:group);");
  query.bindValue(":group", group);
  query.exec();
  if (query.first())
    _custSelector->populate(query);
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Populating Customer Info"),
                                query, __FILE__, __LINE__))
    return;

  if (_mode == cEdit)
    _custSelector->setId(customer);
}

void cashReceiptMiscDistrib::setGroup(const ParameterList &pParams)
{
  if (_mode == cNew)
  {
    QString sql = "SELECT cashrcpt_custgrp_id FROM cashrcpt WHERE cashrcpt_id=<? value('cashrcpt_id') ?>";
    XSqlQuery query;
    MetaSQLQuery mql(sql);
    query = mql.toQuery(pParams);
    if (query.first())
    {
      _custgrp = query.value("cashrcpt_custgrp_id").toInt();
      showCustomers(_custgrp, 0);
    }
  }
  if (_mode == cEdit)
  {
    QString sql = "SELECT cashrcptmisc_cust_id, cashrcpt_custgrp_id FROM cashrcpt "
                  " JOIN cashrcptmisc ON cashrcptmisc_cashrcpt_id=cashrcpt_id "
                  " WHERE cashrcptmisc_id=<? value('cashrcptmisc_id') ?>";
    _cashmisc = pParams.value("cashrcptmisc_id").toInt();
    XSqlQuery query;
    MetaSQLQuery mql(sql);
    query = mql.toQuery(pParams);
    if (query.first())
    {
      _custgrp = query.value("cashrcpt_custgrp_id").toInt();
      showCustomers(_custgrp, query.value("cashrcptmisc_cust_id").toInt());
    }
  }
}

