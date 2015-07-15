/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendorType.h"

#include <QVariant>
#include <QMessageBox>

vendorType::vendorType(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(close()));
  connect(_code, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

vendorType::~vendorType()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendorType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse vendorType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vendtype_id", &valid);
  if (valid)
  {
    _vendtypeid = param.toInt();
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
      _code->setEnabled(false);
      _description->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void vendorType::sCheck()
{
  XSqlQuery vendorCheck;
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length()))
  {
    vendorCheck.prepare( "SELECT vendtype_id "
               "FROM vendtype "
               "WHERE (UPPER(vendtype_code)=UPPER(:venttype_code));" );
    vendorCheck.bindValue(":vendtype_code", _code->text());
    vendorCheck.exec();
    if (vendorCheck.first())
    {
      _vendtypeid = vendorCheck.value("vendtype_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(false);
    }
  }
}

void vendorType::sSave()
{
  XSqlQuery vendorSave;
  if (_code->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Vendor Type Code"),
                              tr("You must enter a valid Code for this Vendor Type before creating it.")  );
    _code->setFocus();
    return;
  }

  vendorSave.prepare("SELECT vendtype_id"
            "  FROM vendtype"
            " WHERE((vendtype_id != :vendtype_id)"
            "   AND (vendtype_code=:vendtype_code))");
  vendorSave.bindValue(":vendtype_id", _vendtypeid);
  vendorSave.bindValue(":vendtype_code", _code->text().trimmed());
  vendorSave.exec();
  if(vendorSave.first())
  {
    QMessageBox::critical( this, tr("Duplicate Entry"),
      tr("The Code you have entered for this Vendor Type is already in the system.") );
    return;
  }

  if (_mode == cNew)
  {
    vendorSave.exec("SELECT NEXTVAL('vendtype_vendtype_id_seq') AS _vendtype_id;");
    if (vendorSave.first())
      _vendtypeid = vendorSave.value("_vendtype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    vendorSave.prepare( "INSERT INTO vendtype "
               "(vendtype_id, vendtype_code, vendtype_descrip) "
               "VALUES "
               "(:vendtype_id, :vendtype_code, :vendtype_descrip);" );
  }
  else if (_mode == cEdit)
    vendorSave.prepare( "UPDATE vendtype "
               "SET vendtype_code=:vendtype_code,"
               "    vendtype_descrip=:vendtype_descrip "
               "WHERE (vendtype_id=:vendtype_id);" );

  vendorSave.bindValue(":vendtype_id", _vendtypeid);
  vendorSave.bindValue(":vendtype_code", _code->text().trimmed());
  vendorSave.bindValue(":vendtype_descrip", _description->text().trimmed());
  vendorSave.exec();

  done(_vendtypeid);
}

void vendorType::populate()
{
  XSqlQuery vendorpopulate;
  vendorpopulate.prepare( "SELECT vendtype_code, vendtype_descrip "
             "FROM vendtype "
             "WHERE (vendtype_id=:vendtype_id);" );
  vendorpopulate.bindValue(":vendtype_id", _vendtypeid);
  vendorpopulate.exec();
  if (vendorpopulate.first())
  {
    _code->setText(vendorpopulate.value("vendtype_code"));
    _description->setText(vendorpopulate.value("vendtype_descrip"));
  }
}
