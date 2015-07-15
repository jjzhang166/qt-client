/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxAdjustment.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

taxAdjustment::taxAdjustment(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxcode, SIGNAL(newID(int)), this, SLOT(sCheck()));

  _taxhistid = -1;
  _sense = 1;
}

taxAdjustment::~taxAdjustment()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxAdjustment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxAdjustment::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("order_id", &valid);
  if (valid)
    _orderid = param.toInt();

  param = pParams.value("order_type", &valid);
  if (valid)
  {
    if (param.toString() == "I")
      _table = "invcheadtax";
    else if (param.toString() == "B")
      _table = "cobmisctax";
    else if (param.toString() == "CM")
      _table = "cmheadtax";
    else if (param.toString() == "AR")
      _table = "aropentax";
    else if (param.toString() == "AP")
      _table = "apopentax";
    else
      _table = param.toString();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
  }

  param = pParams.value("date", &valid);
   if (valid)
     _amount->setEffective(param.toDate());

  param = pParams.value("curr_id", &valid);
   if (valid)
     _amount->setId(param.toInt());

   param = pParams.value("sense", &valid);
   if (valid)
    _sense = param.toInt();

  return NoError;
}

void taxAdjustment::sSave()
{
  XSqlQuery taxSave;
  if (_taxcode->id() == -1)
  {
    QMessageBox::critical( this, tr("Tax Code Required"),
                          tr( "<p>You must select a tax code "
                          " before you may save." ) );
    return;
  }

  QString sql;
  if (_mode == cNew)
    sql = QString( "INSERT into %1 (taxhist_basis,taxhist_percent,taxhist_amount,taxhist_docdate, taxhist_tax_id, taxhist_tax, taxhist_taxtype_id, taxhist_parent_id  ) "
                   "VALUES (0, 0, 0, :date, :taxcode_id, :amount, getadjustmenttaxtypeid(), :order_id) ").arg(_table);

  else if (_mode == cEdit)
    sql = QString( "UPDATE taxhist "
                   "SET taxhist_tax=:amount, taxhist_docdate=:date "
                   "WHERE (taxhist_id=:taxhist_id) ");
  taxSave.prepare(sql);
  taxSave.bindValue(":taxhist_id", _taxhistid);
  taxSave.bindValue(":taxcode_id", _taxcode->id());
  taxSave.bindValue(":amount", _amount->localValue() * _sense);
  taxSave.bindValue(":order_id", _orderid);
  taxSave.bindValue(":date", _amount->effective());
  taxSave.exec();
  if (taxSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  done(_orderid);
}

void taxAdjustment::sCheck()
{
  XSqlQuery taxCheck;
  QString table;

   taxCheck.prepare( "SELECT taxhist_id, taxhist_tax_id, taxhist_tax "
              "FROM taxhist "
              " JOIN pg_class ON (pg_class.oid=taxhist.tableoid) "
              "WHERE ((taxhist_parent_id=:order_id) "
              " AND (taxhist_taxtype_id=getadjustmenttaxtypeid()) "
              " AND (taxhist_tax_id=:tax_id) "
              " AND (relname=:table) );" );
  taxCheck.bindValue(":order_id", _orderid);
  taxCheck.bindValue(":tax_id", _taxcode->id());
  taxCheck.bindValue(":table", _table);

  taxCheck.exec();
  if (taxCheck.first())
  {
    _taxhistid=taxCheck.value("taxhist_id").toInt();
    _amount->setLocalValue(taxCheck.value("taxhist_tax").toDouble() * _sense);
    _amount->setFocus();
    _mode=cEdit;
  }
  else
  {
    _taxhistid=-1;
    _amount->clear();
    _amount->setFocus();
    _mode=cNew;
  }
}

