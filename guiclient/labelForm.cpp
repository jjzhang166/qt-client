/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "labelForm.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

labelForm::labelForm(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
}

labelForm::~labelForm()
{
    // no need to delete child widgets, Qt does it all for us
}

void labelForm::languageChange()
{
    retranslateUi(this);
}

enum SetResponse labelForm::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("labelform_id", &valid);
  if (valid)
  {
    _labelformid = param.toInt();
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
      _labelsPerPage->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void labelForm::sSave()
{
  XSqlQuery labelSave;
  if (_name->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Format Name is Invalid"),
                          tr("<p>You must enter a valid name for this Label Format.") );
    _name->setFocus();
    return;
  }

  else if (_report->currentIndex() == -1)
  {
    QMessageBox::warning( this, tr("Report Name is Invalid"),
                          tr("<p>You must enter a select Report for this Label Format.") );
    _report->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    labelSave.prepare( "SELECT labelform_id "
         	     "FROM labelform "
               "WHERE (labelform_name=:labelform_name);" );
    labelSave.bindValue(":labelform_name", _name->text());
    labelSave.exec();
    if (labelSave.first())
    {
      QMessageBox::warning( this, tr("Format Name is Invalid"),
                            tr("<p>This Label Form Name already exists.") );
      _name->setFocus();
      return;
    }
    else if (labelSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, labelSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    labelSave.exec("SELECT NEXTVAL('labelform_labelform_id_seq') AS _labelform_id");
    if (labelSave.first())
      _labelformid = labelSave.value("_labelform_id").toInt();
    else if (labelSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, labelSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    labelSave.prepare( "INSERT INTO labelform "
               "(labelform_id, labelform_name, labelform_report_name, labelform_perpage) "
               "VALUES "
               "(:labelform_id, :labelform_name, :labelform_report_name, :labelform_perpage);" );

  }
  if (_mode == cEdit)
    labelSave.prepare( "UPDATE labelform "
               "SET labelform_name=:labelform_name, labelform_report_name=:labelform_report_name,"
               "    labelform_perpage=:labelform_perpage "
               "WHERE (labelform_id=:labelform_id);" );

  labelSave.bindValue(":labelform_id", _labelformid);
  labelSave.bindValue(":labelform_name", _name->text());
  labelSave.bindValue(":labelform_report_name", _report->code());
  labelSave.bindValue(":labelform_perpage", _labelsPerPage->value());
  labelSave.exec();
  if (labelSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, labelSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_labelformid);
}

void labelForm::populate()
{
  XSqlQuery labelpopulate;
  labelpopulate.prepare( "SELECT * "
       	     "FROM labelform "
	     "WHERE (labelform_id=:labelform_id);" );
  labelpopulate.bindValue(":labelform_id", _labelformid);
  labelpopulate.exec();
  if (labelpopulate.first())
  {
    _name->setText(labelpopulate.value("labelform_name").toString());
    _report->setCode(labelpopulate.value("labelform_report_name").toString());
    _labelsPerPage->setValue(labelpopulate.value("labelform_perpage").toInt());
  }
  else if (labelpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, labelpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
