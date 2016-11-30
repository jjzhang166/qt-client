/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shippingForm.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include "errorReporter.h"

shippingForm::shippingForm(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _shipformid = -1;

  connect(_name,	SIGNAL(editingFinished()),	this, SLOT(sCheck()));
  connect(_buttonBox,	SIGNAL(accepted()),	this, SLOT(sSave()));
}

shippingForm::~shippingForm()
{
  // no need to delete child widgets, Qt does it all for us
}

void shippingForm::languageChange()
{
  retranslateUi(this);
}

enum SetResponse shippingForm::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("shipform_id", &valid);
  if (valid)
  {
    _shipformid = param.toInt();
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
      _report->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void shippingForm::sCheck()
{
  XSqlQuery shippingCheck;
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) || (_name->text().length()))
  {
    shippingCheck.prepare( "SELECT shipform_id"
               "  FROM shipform"
               " WHERE((UPPER(shipform_name)=UPPER(:shipform_name))"
               "   AND (shipform_id != :shipform_id));" );
    shippingCheck.bindValue(":shipform_name", _name->text());
    shippingCheck.bindValue(":shipform_id", _shipformid);
    shippingCheck.exec();
    if (shippingCheck.first())
    {
      _shipformid = shippingCheck.value("shipform_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Shipping Form Information"),
                                  shippingCheck, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void shippingForm::sSave()
{
  XSqlQuery shippingSave;
  if (_name->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Format Name is Invalid"),
                          tr("You must enter a valid name for this Bill of Lading Format.") );
    _name->setFocus();
    return;
  }

  sCheck();

  if (_report->id() == -1)
  {
    QMessageBox::warning( this, tr("Report Name is Invalid"),
                          tr("You must enter a select report for this Bill of Lading Format.") );
    _report->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    shippingSave.exec("SELECT NEXTVAL('shipform_shipform_id_seq') AS shipform_id;");
    if (shippingSave.first())
      _shipformid = shippingSave.value("shipform_id").toInt();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Shipping Form Information"),
                                  shippingSave, __FILE__, __LINE__))
    {
      return;
    }

    shippingSave.prepare( "INSERT INTO shipform "
               "(shipform_id, shipform_name, shipform_report_name) "
               "VALUES "
               "(:shipform_id, :shipform_name, :shipform_report_name);" );

  }
  if (_mode == cEdit)
    shippingSave.prepare( "UPDATE shipform "
               "SET shipform_name=:shipform_name, shipform_report_name=:shipform_report_name "
               "WHERE (shipform_id=:shipform_id);" );

  shippingSave.bindValue(":shipform_id", _shipformid);
  shippingSave.bindValue(":shipform_name", _name->text());
  shippingSave.bindValue(":shipform_report_name", _report->code());
  shippingSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Shipping Form Information"),
                                shippingSave, __FILE__, __LINE__))
  {
    return;
  }

  done(_shipformid);
}

void shippingForm::populate()
{
  XSqlQuery shippingpopulate;
  shippingpopulate.prepare( "SELECT * "
       	     "FROM shipform "
	     "WHERE (shipform_id=:shipform_id);" );
  shippingpopulate.bindValue(":shipform_id", _shipformid);
  shippingpopulate.exec();
  if (shippingpopulate.first())
  {
    _name->setText(shippingpopulate.value("shipform_name").toString());
    _report->setCode(shippingpopulate.value("shipform_report_name").toString());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Shipping Form Information"),
                                shippingpopulate, __FILE__, __LINE__))
  {
    return;
  }
}
