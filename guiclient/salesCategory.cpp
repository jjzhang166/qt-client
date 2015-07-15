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
#include "salesCategory.h"

#include <QVariant>
#include <QMessageBox>

salesCategory::salesCategory(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _salescatid = -1;

  // signals and slots connections
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_category, SIGNAL(editingFinished()), this, SLOT(sCheck()));

  _sales->setType(GLCluster::cRevenue);
  _prepaid->setType(GLCluster::cRevenue);
  _araccnt->setType(GLCluster::cAsset);
}

salesCategory::~salesCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesCategory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesCategory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("salescat_id", &valid);
  if (valid)
  {
    _salescatid = param.toInt();
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
      _salescatid = -1;
      _category->clear();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _category->setEnabled(false);
      _active->setEnabled(false);
      _description->setEnabled(false);
      _sales->setReadOnly(true);
      _prepaid->setReadOnly(true);
      _araccnt->setReadOnly(true);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void salesCategory::sCheck()
{
  XSqlQuery salesCheck;
  _category->setText(_category->text().trimmed().toUpper());
  if ((_mode == cNew) && (_category->text().length() != 0))
  {
    salesCheck.prepare( "SELECT salescat_id "
               "FROM salescat "
               "WHERE (UPPER(salescat_name)=:salescat_name);" );
    salesCheck.bindValue(":salescat_name", _category->text().trimmed());
    salesCheck.exec();
    if (salesCheck.first())
    {
      _salescatid = salesCheck.value("salescat_id").toInt();
      _mode = cEdit;
      populate();

      _category->setEnabled(false);
    }
  }
}

void salesCategory::sSave()
{
  XSqlQuery salesSave;
  QList<GuiErrorCheck> errors;

  errors << GuiErrorCheck(_category->text().trimmed().isEmpty(), _category,
                          tr("<p>You must specify a name for the Sales Category."));

  salesSave.prepare("SELECT salescat_id"
            "  FROM salescat"
            " WHERE((salescat_name=:salescat_name)"
            "   AND (salescat_id != :salescat_id))");
  salesSave.bindValue(":salescat_id", _salescatid);
  salesSave.bindValue(":salescat_name", _category->text().trimmed());
  salesSave.exec();
  if(salesSave.first())
  {
    errors << GuiErrorCheck(true, _category,
                            tr("<p>You cannot specify a duplicate name for the Sales Category."));
  }

  if (_metrics->boolean("InterfaceARToGL"))
  {
    errors << GuiErrorCheck(!_sales->isValid(), _sales,
                            tr("<p>You must select a Sales Account Number for this Sales Category before you may save it."))
           << GuiErrorCheck(!_prepaid->isValid(), _prepaid,
                            tr("<p>You must select a Prepaid Account Number for this Sales Category before you may save it."))
           << GuiErrorCheck(!_araccnt->isValid(), _araccnt,
                            tr("<p>You must select an A/R Account Number for this Sales Category before you may save it."))
           ;
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Sales Category"), errors))
    return;

  if ( (_mode == cNew) || (_mode == cCopy) )
  {
    salesSave.exec("SELECT NEXTVAL('salescat_salescat_id_seq') AS salescat_id");
    if (salesSave.first())
      _salescatid = salesSave.value("salescat_id").toInt();

    salesSave.prepare( "INSERT INTO salescat"
               "( salescat_id, salescat_name, salescat_active, salescat_descrip,"
               "  salescat_sales_accnt_id, salescat_prepaid_accnt_id, salescat_ar_accnt_id ) "
               "VALUES "
               "( :salescat_id, :salescat_name, :salescat_active, :salescat_descrip,"
               "  :salescat_sales_accnt_id, :salescat_prepaid_accnt_id, :salescat_ar_accnt_id );" );
  }
  else if (_mode == cEdit)
    salesSave.prepare( "UPDATE salescat "
               "SET salescat_name=:salescat_name, salescat_active=:salescat_active,"
               "    salescat_descrip=:salescat_descrip,"
               "    salescat_sales_accnt_id=:salescat_sales_accnt_id,"
               "    salescat_prepaid_accnt_id=:salescat_prepaid_accnt_id,"
               "    salescat_ar_accnt_id=:salescat_ar_accnt_id "
               "WHERE (salescat_id=:salescat_id);" );

  salesSave.bindValue(":salescat_id", _salescatid);
  salesSave.bindValue(":salescat_name", _category->text().trimmed());
  salesSave.bindValue(":salescat_active", QVariant(_active->isChecked()));
  salesSave.bindValue(":salescat_descrip", _description->text().trimmed());
  salesSave.bindValue(":salescat_sales_accnt_id", _sales->id());
  salesSave.bindValue(":salescat_prepaid_accnt_id", _prepaid->id());
  salesSave.bindValue(":salescat_ar_accnt_id", _araccnt->id());
  salesSave.exec();

  done(_salescatid);
}

void salesCategory::populate()
{
  XSqlQuery salespopulate;
  salespopulate.prepare( "SELECT * "
             "FROM salescat "
             "WHERE (salescat_id=:salescat_id);" );
  salespopulate.bindValue(":salescat_id", _salescatid);
  salespopulate.exec();
  if (salespopulate.first())
  {
    if (_mode != cCopy)
    {
      _category->setText(salespopulate.value("salescat_name").toString());
      _description->setText(salespopulate.value("salescat_descrip").toString());
      _active->setChecked(salespopulate.value("salescat_active").toBool());
    }
    else
      _active->setChecked(true);

    _sales->setId(salespopulate.value("salescat_sales_accnt_id").toInt());
    _prepaid->setId(salespopulate.value("salescat_prepaid_accnt_id").toInt());
    _araccnt->setId(salespopulate.value("salescat_ar_accnt_id").toInt());
  }
}
 
