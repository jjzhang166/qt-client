/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "classCode.h"

#include <QVariant>
#include <QMessageBox>

#include "errorReporter.h"
#include "guiErrorCheck.h"

#include "classCodeTax.h"


classCode::classCode(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSaveAndDone()));
  connect(_classCode, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_newTax, SIGNAL(clicked()), this, SLOT(sNewTax()));
  connect(_editTax, SIGNAL(clicked()), this, SLOT(sEditTax()));
  connect(_deleteTax, SIGNAL(clicked()), this, SLOT(sDeleteTax()));

  _classcodetax->addColumn(tr("Tax Type"), _itemColumn, Qt::AlignLeft, true, "taxtype_name");
  _classcodetax->addColumn(tr("Tax Zone"),          -1, Qt::AlignLeft, true, "taxzone");

}

classCode::~classCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void classCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse classCode::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    _classcodeid = param.toInt();
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

      _classCode->setEnabled(false);
      _description->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void classCode::sSaveAndDone()
{
  if (sSave())
    done(_classcodeid);
}

bool classCode::sSave()
{
  XSqlQuery classSave;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_classCode->text().length() == 0, _classCode,
                          tr("You must enter a valid Class Code "
                             "before continuing"))
     ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Class Code"), errors))
    return false;

  if (_mode == cEdit)
    classSave.prepare( "UPDATE classcode "
               "SET classcode_code=:classcode_code, classcode_descrip=:classcode_descrip "
               "WHERE (classcode_id=:classcode_id);" );
  else if (_mode == cNew)
  {
    classSave.exec("SELECT NEXTVAL('classcode_classcode_id_seq') AS classcode_id");
    if (classSave.first())
      _classcodeid = classSave.value("classcode_id").toInt();
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Class Code at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__),
                         classSave, __FILE__, __LINE__);
      return false;
    }
 
    classSave.prepare( "INSERT INTO classcode "
               "( classcode_id, classcode_code, classcode_descrip ) "
               "VALUES "
               "( :classcode_id, :classcode_code, :classcode_descrip );" );
  }

  classSave.bindValue(":classcode_id", _classcodeid);
  classSave.bindValue(":classcode_code", _classCode->text());
  classSave.bindValue(":classcode_descrip", _description->text());
  classSave.exec();
  if(ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Class Code at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__),
                         classSave, __FILE__, __LINE__))
      return false;

  _mode = cEdit;
  return true;

}

void classCode::sCheck()
{
  XSqlQuery classCheck;
  _classCode->setText(_classCode->text().trimmed());
  if ( (_mode == cNew) && (_classCode->text().length()) )
  {
    classCheck.prepare( "SELECT classcode_id "
               "FROM classcode "
               "WHERE (UPPER(classcode_code)=UPPER(:classcode_code));" );
    classCheck.bindValue(":classcode_code", _classCode->text());
    classCheck.exec();
    if (classCheck.first())
    {
      _classcodeid = classCheck.value("classcode_id").toInt();
      _mode = cEdit;
      populate();

      _classCode->setEnabled(false);
    }
  }
}

void classCode::populate()
{
  XSqlQuery classpopulate;
  classpopulate.prepare( "SELECT classcode_code, classcode_descrip "
             "FROM classcode "
             "WHERE (classcode_id=:classcode_id);" );
  classpopulate.bindValue(":classcode_id", _classcodeid);
  classpopulate.exec();
  if (classpopulate.first())
  {
    _classCode->setText(classpopulate.value("classcode_code"));
    _description->setText(classpopulate.value("classcode_descrip"));
  }

  sFillListTax();
}

void classCode::sFillListTax()
{ 
  XSqlQuery fillListItemtax;
  fillListItemtax.prepare("SELECT classcodetax_id, taxtype_name,"
            "       COALESCE(taxzone_code,:any) AS taxzone"
            "  FROM classcodetax JOIN taxtype ON (classcodetax_taxtype_id=taxtype_id)"
            "       LEFT OUTER JOIN taxzone ON (classcodetax_taxzone_id=taxzone_id)"
            " WHERE (classcodetax_classcode_id=:classcode_id)"
            " ORDER BY taxtype_name;");
  fillListItemtax.bindValue(":classcode_id", _classcodeid);
  fillListItemtax.bindValue(":any", tr("Any"));
  fillListItemtax.exec();
  _classcodetax->populate(fillListItemtax);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Information"),
                                fillListItemtax, __FILE__, __LINE__))
    return;
}

void classCode::sNewTax()
{
  if (!sSave())
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("classcode_id", _classcodeid);

  classCodeTax newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  sFillListTax();
}

void classCode::sEditTax()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("classcodetax_id", _classcodetax->id());

  classCodeTax newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  sFillListTax();
}
void classCode::sDeleteTax()
{
  XSqlQuery deleteItemtax;
  deleteItemtax.prepare("DELETE FROM classcodetax"
            " WHERE (classcodetax_id=:classcodetax_id);");
  deleteItemtax.bindValue(":classcodetax_id", _classcodetax->id());
  deleteItemtax.exec();
  sFillListTax();
}
