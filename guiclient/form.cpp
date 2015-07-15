/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "form.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "errorReporter.h"
#include "guiErrorCheck.h"

form::form(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
}

form::~form()
{
  // no need to delete child widgets, Qt does it all for us
}

void form::languageChange()
{
  retranslateUi(this);
}

enum SetResponse form::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("form_id", &valid);
  if (valid)
  {
    _formid = param.toInt();
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
      _key->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void form::sSave()
{
  XSqlQuery formSave;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_name->text().trimmed().isEmpty(), _name,
                          tr("You must enter a valid Name for this Form "
                             "before continuing"))
         << GuiErrorCheck(_report->id() == -1, _report,
                          tr("You must select a Report for this Form "
                             "before continuing"))
     ;

  if (_mode == cNew)
  {
    formSave.prepare( "SELECT form_id "
               "FROM form "
               "WHERE (form_name=:form_name);" );
    formSave.bindValue(":form_name", _name->text());
    formSave.exec();
    if (formSave.first())
      errors << GuiErrorCheck(true, _name,
                              tr( "A Form has already been defined with the selected Name.\n"
                                  "You may not create duplicate Forms." ) );
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Form"), errors))
    return;

  if (_mode == cNew)
  {
    formSave.exec("SELECT NEXTVAL('form_form_id_seq') AS _form_id");
    if (formSave.first())
      _formid = formSave.value("_form_id").toInt();
    else if (formSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, formSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    formSave.prepare( "INSERT INTO form "
               "(form_id, form_name, form_descrip, form_report_name, form_key) "
               "VALUES "
               "(:form_id, :form_name, :form_descrip, :form_report_name, :form_key);" );

  }
  else if (_mode == cEdit)
    formSave.prepare( "UPDATE form "
               "SET form_name=:form_name, form_descrip=:form_descrip,"
               "    form_report_name=:form_report_name, form_key=:form_key "
               "WHERE (form_id=:form_id);" );

  formSave.bindValue(":form_id", _formid);
  formSave.bindValue(":form_name", _name->text());
  formSave.bindValue(":form_descrip", _descrip->text());
  formSave.bindValue(":form_report_name", _report->code());

  if (_key->currentIndex() == 0)
    formSave.bindValue(":form_key", "Cust");
  else if (_key->currentIndex() == 1)
    formSave.bindValue(":form_key", "Item");
  else if (_key->currentIndex() == 2)
    formSave.bindValue(":form_key", "ItSt");
  else if (_key->currentIndex() == 3)
    formSave.bindValue(":form_key", "PO");
  else if (_key->currentIndex() == 4)
    formSave.bindValue(":form_key", "SO");
  else if (_key->currentIndex() == 5)
    formSave.bindValue(":form_key", "Vend");
  else if (_key->currentIndex() == 6)
    formSave.bindValue(":form_key", "WO");
  else if (_key->currentIndex() == 7)
    formSave.bindValue(":form_key", "SASC");
  else if (_key->currentIndex() == 8)
    formSave.bindValue(":form_key", "PES");
  else if (_key->currentIndex() == 9)
    formSave.bindValue(":form_key", "RA");
  else
    formSave.bindValue(":form_key", "");

  formSave.exec();
  if (formSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, formSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_formid);
}

void form::populate()
{
  XSqlQuery formpopulate;
  formpopulate.prepare( "SELECT * "
      	     "FROM form "
             "WHERE (form_id=:form_id);" );
  formpopulate.bindValue(":form_id", _formid);
  formpopulate.exec();
  if (formpopulate.first())
  {
    _name->setText(formpopulate.value("form_name").toString());
    _descrip->setText(formpopulate.value("form_descrip").toString());
    _report->setCode(formpopulate.value("form_report_name").toString());
  
    if (formpopulate.value("form_key").toString() == "Cust")
      _key->setCurrentIndex(0);
    else if (formpopulate.value("form_key").toString() == "Item")
      _key->setCurrentIndex(1);
    else if (formpopulate.value("form_key").toString() == "ItSt")
      _key->setCurrentIndex(2);
    else if (formpopulate.value("form_key").toString() == "PO")
      _key->setCurrentIndex(3);
    else if (formpopulate.value("form_key").toString() == "SO")
      _key->setCurrentIndex(4);
    else if (formpopulate.value("form_key").toString() == "Vend")
      _key->setCurrentIndex(5);
    else if (formpopulate.value("form_key").toString() == "WO")
      _key->setCurrentIndex(6);
    else if (formpopulate.value("form_key").toString() == "SASC")
      _key->setCurrentIndex(7);
    else if (formpopulate.value("form_key").toString() == "PES")
      _key->setCurrentIndex(8);
    else if (formpopulate.value("form_key").toString() == "RA")
      _key->setCurrentIndex(9);
    else
      _key->setCurrentIndex(-1);
  }
  else if (formpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, formpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
