/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "expenseCategory.h"

#include <QVariant>
#include <QMessageBox>

expenseCategory::expenseCategory(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _expcatid = -1;

  // signals and slots connections
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_category, SIGNAL(editingFinished()), this, SLOT(sCheck()));

  _expense->setType(GLCluster::cExpense | GLCluster::cAsset | GLCluster::cLiability);
  _purchasePrice->setType(GLCluster::cAsset | GLCluster::cExpense);
  _liability->setType(GLCluster::cLiability);
  _freight->setType(GLCluster::cExpense);

}

expenseCategory::~expenseCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void expenseCategory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse expenseCategory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("expcat_id", &valid);
  if (valid)
  {
    _expcatid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;
      _category->clear();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _category->setEnabled(false);
      _active->setEnabled(false);
      _description->setEnabled(false);
      _expense->setReadOnly(true);
      _purchasePrice->setReadOnly(true);
      _liability->setReadOnly(true);
      _freight->setReadOnly(true);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void expenseCategory::sCheck()
{
  XSqlQuery expenseCheck;
  _category->setText(_category->text().trimmed().toUpper());
  if ( ((_mode == cNew) || (_mode == cCopy)) && (_category->text().length() != 0) )
  {
    expenseCheck.prepare( "SELECT expcat_id "
               "FROM expcat "
               "WHERE (UPPER(expcat_code)=:expcat_code);" );
    expenseCheck.bindValue(":expcat_code", _category->text().trimmed());
    expenseCheck.exec();
    if (expenseCheck.first())
    {
      _expcatid = expenseCheck.value("expcat_id").toInt();
      _mode = cEdit;
      populate();

      _category->setEnabled(false);
    }
  }
}

void expenseCategory::sSave()
{
  XSqlQuery expenseSave;
  QList<GuiErrorCheck> errors;

  errors << GuiErrorCheck(_category->text().trimmed().isEmpty(), _category,
                         tr("<p>You must specify a name."));

  if (_mode == cEdit)
  {
    expenseSave.prepare( "SELECT expcat_id "
               "  FROM expcat "
               " WHERE((UPPER(expcat_code)=UPPER(:expcat_code))"
               "   AND (expcat_id != :expcat_id));" );
    expenseSave.bindValue(":expcat_id", _expcatid);
    expenseSave.bindValue(":expcat_code", _category->text().trimmed());
    expenseSave.exec();
    if (expenseSave.first())
    {
      errors << GuiErrorCheck(true, _category,
                             tr("<p>The name you have specified is already in use."));
    }
  }

  if (_metrics->boolean("InterfaceAPToGL"))
  {
    errors << GuiErrorCheck(!_expense->isValid(), _expense,
                            tr("<p>You must select a Expense Account Number for this Expense Category before you may save it."))
           << GuiErrorCheck(!_purchasePrice->isValid(), _purchasePrice,
                            tr("<p>You must select a Purchase Price Variance Account Number for this Expense Category before you may save it."))
           << GuiErrorCheck(!_liability->isValid(), _liability,
                            tr("<p>You must select a P/O Liability Clearing Account Number for this Expense Category before you may save it."))
           << GuiErrorCheck(!_freight->isValid(), _freight,
                            tr("<p>You must select a Freight Receiving Account Number for this Expense Category before you may save it."))
           ;
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Expense Category"), errors))
    return;

  if ( (_mode == cNew) || (_mode == cCopy) )
  {
    expenseSave.exec("SELECT NEXTVAL('expcat_expcat_id_seq') AS expcat_id");
    if (expenseSave.first())
      _expcatid = expenseSave.value("expcat_id").toInt();

    expenseSave.prepare( "INSERT INTO expcat"
               "( expcat_id, expcat_code, expcat_active, expcat_descrip,"
               "  expcat_exp_accnt_id, expcat_purchprice_accnt_id,"
               "  expcat_liability_accnt_id, expcat_freight_accnt_id ) "
               "VALUES "
               "( :expcat_id, :expcat_code, :expcat_active, :expcat_descrip,"
               "  :expcat_exp_accnt_id, :expcat_purchprice_accnt_id,"
               "  :expcat_liability_accnt_id, :expcat_freight_accnt_id );" );
  }
  else if (_mode == cEdit)
    expenseSave.prepare( "UPDATE expcat "
               "SET expcat_code=:expcat_code, expcat_active=:expcat_active,"
               "    expcat_descrip=:expcat_descrip,"
               "    expcat_exp_accnt_id=:expcat_exp_accnt_id,"
               "    expcat_purchprice_accnt_id=:expcat_purchprice_accnt_id,"
               "    expcat_liability_accnt_id=:expcat_liability_accnt_id,"
               "    expcat_freight_accnt_id=:expcat_freight_accnt_id "
               "WHERE (expcat_id=:expcat_id);" );

  expenseSave.bindValue(":expcat_id", _expcatid);
  expenseSave.bindValue(":expcat_code", _category->text().trimmed());
  expenseSave.bindValue(":expcat_active", QVariant(_active->isChecked()));
  expenseSave.bindValue(":expcat_descrip", _description->text().trimmed());
  expenseSave.bindValue(":expcat_exp_accnt_id", _expense->id());
  expenseSave.bindValue(":expcat_purchprice_accnt_id", _purchasePrice->id());
  expenseSave.bindValue(":expcat_liability_accnt_id", _liability->id());
  expenseSave.bindValue(":expcat_freight_accnt_id", _freight->id());
  expenseSave.exec();

  done(_expcatid);
}

void expenseCategory::populate()
{
  XSqlQuery expensepopulate;
  expensepopulate.prepare( "SELECT * "
             "FROM expcat "
             "WHERE (expcat_id=:expcat_id);" );
  expensepopulate.bindValue(":expcat_id", _expcatid);
  expensepopulate.exec();
  if (expensepopulate.first())
  {
    if (_mode != cCopy)
    {
      _category->setText(expensepopulate.value("expcat_code").toString());
      _description->setText(expensepopulate.value("expcat_descrip").toString());
      _active->setChecked(expensepopulate.value("expcat_active").toBool());
    }
    else
      _active->setChecked(true);

    _expense->setId(expensepopulate.value("expcat_exp_accnt_id").toInt());
    _purchasePrice->setId(expensepopulate.value("expcat_purchprice_accnt_id").toInt());
    _liability->setId(expensepopulate.value("expcat_liability_accnt_id").toInt());
    _freight->setId(expensepopulate.value("expcat_freight_accnt_id").toInt());
  }
}
 
