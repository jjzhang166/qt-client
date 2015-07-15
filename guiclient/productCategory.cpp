/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "productCategory.h"

#include <QVariant>
#include <QMessageBox>

productCategory::productCategory(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_category, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

productCategory::~productCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void productCategory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse productCategory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("prodcat_id", &valid);
  if (valid)
  {
    _prodcatid = param.toInt();
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

      _category->setEnabled(false);
      _description->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void productCategory::sCheck()
{
  XSqlQuery productCheck;
  _category->setText(_category->text().trimmed());
  if ( (_mode == cNew) && (_category->text().length()) )
  {
    productCheck.prepare( "SELECT prodcat_id "
               "FROM prodcat "
               "WHERE (UPPER(prodcat_code)=UPPER(:prodcat_code));" );
    productCheck.bindValue(":prodcat_code", _category->text());
    productCheck.exec();
    if (productCheck.first())
    {
      _prodcatid = productCheck.value("prodcat_id").toInt();
      _mode = cEdit;
      populate();

      _category->setEnabled(false);
    }
  }
}

void productCategory::sSave()
{
  XSqlQuery productSave;
  if (_category->text().trimmed().isEmpty())
  {
    QMessageBox::critical(this, tr("Missing Category"),
			  tr("You must name this Category before saving it."));
    _category->setFocus();
    return;
  }

  if (_mode == cEdit)
  {
    productSave.prepare( "SELECT prodcat_id "
               "FROM prodcat "
               "WHERE ( (prodcat_id<>:prodcat_id)"
               " AND (prodcat_code=:prodcat_code) );");
    productSave.bindValue(":prodcat_id", _prodcatid);
    productSave.bindValue(":prodcat_code", _category->text());
    productSave.exec();
    if (productSave.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Product Category"),
                             tr( "A Product Category with the entered code already exists."
                                 "You may not create a Product Category with this code." ) );
      _category->setFocus();
      return;
    }

    productSave.prepare( "UPDATE prodcat "
               "SET prodcat_code=:prodcat_code, prodcat_descrip=:prodcat_descrip "
               "WHERE (prodcat_id=:prodcat_id);" );
    productSave.bindValue(":prodcat_id", _prodcatid);
    productSave.bindValue(":prodcat_code", _category->text().toUpper());
    productSave.bindValue(":prodcat_descrip", _description->text());
    productSave.exec();
  }
  else if (_mode == cNew)
  {
    productSave.prepare( "SELECT prodcat_id "
               "FROM prodcat "
               "WHERE (prodcat_code=:prodcat_code);");
    productSave.bindValue(":prodcat_code", _category->text().trimmed());
    productSave.exec();
    if (productSave.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Product Category"),
                             tr( "A Product Category with the entered code already exists.\n"
                                 "You may not create a Product Category with this code." ) );
      _category->setFocus();
      return;
    }

    productSave.exec("SELECT NEXTVAL('prodcat_prodcat_id_seq') AS prodcat_id;");
    if (productSave.first())
      _prodcatid = productSave.value("prodcat_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    productSave.prepare( "INSERT INTO prodcat "
               "( prodcat_id, prodcat_code, prodcat_descrip ) "
               "VALUES "
               "( :prodcat_id, :prodcat_code, :prodcat_descrip );" );
    productSave.bindValue(":prodcat_id", _prodcatid);
    productSave.bindValue(":prodcat_code", _category->text().toUpper());
    productSave.bindValue(":prodcat_descrip", _description->text());
    productSave.exec();
  }

  done(_prodcatid);
}

void productCategory::populate()
{
  XSqlQuery productpopulate;
  productpopulate.prepare( "SELECT prodcat_code, prodcat_descrip "
             "FROM prodcat "
             "WHERE (prodcat_id=:prodcat_id);" );
  productpopulate.bindValue(":prodcat_id", _prodcatid);
  productpopulate.exec();
  if (productpopulate.first())
  {
    _category->setText(productpopulate.value("prodcat_code").toString());
    _description->setText(productpopulate.value("prodcat_descrip").toString());
  }
}

