/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shippingChargeType.h"

#include <QVariant>
#include <QMessageBox>

shippingChargeType::shippingChargeType(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _shipchrgid = -1;

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

shippingChargeType::~shippingChargeType()
{
  // no need to delete child widgets, Qt does it all for us
}

void shippingChargeType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse shippingChargeType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("shipchrg_id", &valid);
  if (valid)
  {
    _shipchrgid = param.toInt();
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
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(false);
      _description->setEnabled(false);
      _customerFreight->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void shippingChargeType::sCheck()
{
  XSqlQuery shippingCheck;
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().trimmed().length()))
  {
    shippingCheck.prepare( "SELECT shipchrg_id "
               "FROM shipchrg "
               "WHERE (UPPER(shipchrg_name)=UPPER(:shipchrg_name));" );
    shippingCheck.bindValue(":shipchrg_name", _name->text());
    shippingCheck.exec();
    if (shippingCheck.first())
    {
      _shipchrgid = shippingCheck.value("shipchrg_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
  }
}

void shippingChargeType::sSave()
{
  XSqlQuery shippingSave;
  if (_name->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Shipping Charge"),
                            tr("You must enter a valid Name.") );
      return;
  }

  shippingSave.prepare( "SELECT shipchrg_id "
             "FROM shipchrg "
             "WHERE ( (shipchrg_id<>:shipchrg_id)"
             " AND (UPPER(shipchrg_name)=UPPER(:shipchrg_name)) );" );
  shippingSave.bindValue(":shipchrg_id", _shipchrgid);
  shippingSave.bindValue(":shipchrg_name", _name->text());
  shippingSave.exec();
  if (shippingSave.first())
  {
    QMessageBox::critical( this, tr("Cannot Save Shipping Charge Type"),
                           tr( "The new Shipping Charge Type information cannot be saved as the new Shipping Charge Type that you\n"
                               "entered conflicts with an existing Shipping Charge Type.  You must uniquely name this Shipping Charge Type\n"
                               "before you may save it." ) );
    return;
  }

  if (_mode == cNew)
  {
    shippingSave.exec("SELECT NEXTVAL('shipchrg_shipchrg_id_seq') AS shipchrg_id;");
    if (shippingSave.first())
      _shipchrgid = shippingSave.value("shipchrg_id").toInt();

    shippingSave.prepare( "INSERT INTO shipchrg "
               "(shipchrg_id, shipchrg_name, shipchrg_descrip, shipchrg_custfreight) "
               "VALUES "
               "(:shipchrg_id, :shipchrg_name, :shipchrg_descrip, :shipchrg_custfreight);" );
  }
  else if (_mode == cEdit)
  {
    shippingSave.prepare( "UPDATE shipchrg "
               "SET shipchrg_name=:shipchrg_name, shipchrg_descrip=:shipchrg_descrip,"
               "    shipchrg_custfreight=:shipchrg_custfreight "
               "WHERE (shipchrg_id=:shipchrg_id);" );
  }

  shippingSave.bindValue(":shipchrg_id", _shipchrgid);
  shippingSave.bindValue(":shipchrg_name", _name->text().trimmed());
  shippingSave.bindValue(":shipchrg_descrip", _description->text().trimmed());
  shippingSave.bindValue(":shipchrg_custfreight", QVariant(_customerFreight->isChecked()));
  shippingSave.exec();

  done(_shipchrgid);
}

void shippingChargeType::populate()
{
  XSqlQuery shippingpopulate;
  shippingpopulate.prepare( "SELECT shipchrg_name, shipchrg_descrip, shipchrg_custfreight "
             "FROM shipchrg "
             "WHERE (shipchrg_id=:shipchrg_id);" );
  shippingpopulate.bindValue(":shipchrg_id", _shipchrgid);
  shippingpopulate.exec();
  if (shippingpopulate.first()) 
  {
    _name->setText(shippingpopulate.value("shipchrg_name").toString());
    _description->setText(shippingpopulate.value("shipchrg_descrip").toString());
    _customerFreight->setChecked(shippingpopulate.value("shipchrg_custfreight").toBool());
  }
}
