/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customerType.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

customerType::customerType(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _custtypeid = -1;

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(close()));
  connect(_code, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  
  _charass->setType("CT");
}

customerType::~customerType()
{
  // no need to delete child widgets, Qt does it all for us
}

void customerType::languageChange()
{
    retranslateUi(this);
}

enum SetResponse customerType::set(const ParameterList &pParams)
{
  XSqlQuery customeret;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("custtype_id", &valid);
  if (valid)
  {
    _custtypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      
      customeret.exec("SELECT NEXTVAL('custtype_custtype_id_seq') AS custtype_id;");
      if (customeret.first())
      {
        _custtypeid = customeret.value("custtype_id").toInt();
        _charass->setId(_custtypeid);
      }
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(false);
      _description->setEnabled(false);
      _charass->setReadOnly(true);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void customerType::sCheck()
{
  XSqlQuery customerCheck;
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length()))
  {
    customerCheck.prepare( "SELECT custtype_id"
               "  FROM custtype "
               " WHERE((UPPER(custtype_code)=UPPER(:custtype_code))"
               "   AND (custtype_id != :custtype_id));" );
    customerCheck.bindValue(":custtype_code", _code->text());
    customerCheck.bindValue(":custtype_id", _custtypeid);
    customerCheck.exec();
    if (customerCheck.first())
    {
      _custtypeid = customerCheck.value("custtype_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(false);
    }
  }
}

void customerType::sSave()
{
  XSqlQuery customerSave;
  if (_code->text().trimmed().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Customer Type Code"),
                              tr("You must enter a valid Code for this Customer Type before creating it.")  );
    _code->setFocus();
    return;
  }

  customerSave.prepare("SELECT custtype_id"
            "  FROM custtype"
            " WHERE((custtype_id != :custtype_id)"
            "   AND (custtype_code=:custtype_name))");
  customerSave.bindValue(":custtype_id", _custtypeid);
  customerSave.bindValue(":custtype_name", _code->text().trimmed());
  customerSave.exec();
  if(customerSave.first())
  {
    QMessageBox::critical(this, tr("Cannot Save Customer Type"),
                          tr("You have entered a duplicate Code for this Customer Type. "
                             "Please select a different Code before saving."));
    _code->setFocus();
    return;
  }


  if (_mode == cNew)
  {
    customerSave.prepare( "INSERT INTO custtype "
               "(custtype_id, custtype_code, custtype_descrip, custtype_char) "
               "VALUES "
               "(:custtype_id, :custtype_code, :custtype_descrip, :custtype_char);" );
  }
  else if (_mode == cEdit)
    customerSave.prepare( "UPDATE custtype "
               "SET custtype_code=:custtype_code,"
               "    custtype_descrip=:custtype_descrip, custtype_char=:custtype_char "
               "WHERE (custtype_id=:custtype_id);" );

  customerSave.bindValue(":custtype_id", _custtypeid);
  customerSave.bindValue(":custtype_code", _code->text().trimmed());
  customerSave.bindValue(":custtype_descrip", _description->text().trimmed());
  customerSave.bindValue(":custtype_char",  QVariant(_characteristicGroup->isChecked()));
  customerSave.exec();
  if (customerSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, customerSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_custtypeid);
}

void customerType::populate()
{
  XSqlQuery customerpopulate;
  customerpopulate.prepare( "SELECT custtype_code, custtype_descrip, custtype_char "
              "FROM custtype "
              "WHERE (custtype_id=:custtype_id);" );
  customerpopulate.bindValue(":custtype_id", _custtypeid);
  customerpopulate.exec();
  if (customerpopulate.first())
  {
    _code->setText(customerpopulate.value("custtype_code").toString());
    _description->setText(customerpopulate.value("custtype_descrip").toString());
    _characteristicGroup->setChecked(customerpopulate.value("custtype_char").toBool());
    _charass->setId(_custtypeid);
  }
  else if (customerpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, customerpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
