/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "rejectCode.h"

#include <QVariant>
#include <QMessageBox>

rejectCode::rejectCode(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_code, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

rejectCode::~rejectCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void rejectCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse rejectCode::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("rjctcode_id", &valid);
  if (valid)
  {
    _rjctcodeid = param.toInt();
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

void rejectCode::sSave()
{
  XSqlQuery rejectSave;
  if (_code->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Reject Code"),
                              tr("You must enter a valid code for this Reject Code.") );
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    if (sCheck())
    {
      QMessageBox::warning( this, tr("Cannot Save Reject Code"),
                            tr("This Reject code already exists.  You have been placed in edit mode.") );
      return;
    }

    rejectSave.exec("SELECT NEXTVAL('rjctcode_rjctcode_id_seq') AS rjctcode_id");
    if (rejectSave.first())
      _rjctcodeid = rejectSave.value("rjctcode_id").toInt();

    rejectSave.prepare( "INSERT INTO rjctcode "
               "(rjctcode_id, rjctcode_code, rjctcode_descrip) "
               "VALUES "
               "(:rjctcode_id, :rjctcode_code, :rjctcode_descrip);" );
  }
  else if (_mode == cEdit)
    rejectSave.prepare("SELECT rjctcode_id"
              "  FROM rjctcode"
              " WHERE((rjctcode_id != :rjctcode_id)"
              " AND (rjctcode_code = :rjctcode_code));");

  rejectSave.bindValue(":rjctcode_id", _rjctcodeid); 
  rejectSave.bindValue(":rjctcode_code", _code->text());
  rejectSave.exec();
  if (rejectSave.first())
  {
    QMessageBox::warning( this, tr("Cannot Save Reject Code"),
                          tr("You may not rename this Reject code with the entered name as it is in use by another Reject code.") );
    return;
  }

  rejectSave.prepare( "UPDATE rjctcode "
             "SET rjctcode_code=:rjctcode_code,"
             "    rjctcode_descrip=:rjctcode_descrip "
             "WHERE (rjctcode_id=:rjctcode_id);" );

  rejectSave.bindValue(":rjctcode_id", _rjctcodeid);
  rejectSave.bindValue(":rjctcode_code", _code->text());
  rejectSave.bindValue(":rjctcode_descrip", _description->text().trimmed());
  rejectSave.exec();

  done(_rjctcodeid);
}

bool rejectCode::sCheck()
{
  XSqlQuery rejectCheck;
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length()))
  {
    rejectCheck.prepare( "SELECT rjctcode_id "
               "FROM rjctcode "
               "WHERE (UPPER(rjctcode_code)=UPPER(:rjctcode_code));" );
    rejectCheck.bindValue(":rjctcode_code", _code->text());
    rejectCheck.exec();
    if (rejectCheck.first())
    {
      _rjctcodeid = rejectCheck.value("rjctcode_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(false);
      return true;
    }
  }
  return false;
}

void rejectCode::populate()
{
  XSqlQuery rejectpopulate;
  rejectpopulate.prepare( "SELECT rjctcode_code, rjctcode_descrip "
             "FROM rjctcode "
             "WHERE (rjctcode_id=:rjctcode_id);" );
  rejectpopulate.bindValue(":rjctcode_id", _rjctcodeid);
  rejectpopulate.exec();
  if (rejectpopulate.first())
  {
    _code->setText(rejectpopulate.value("rjctcode_code").toString());
    _description->setText(rejectpopulate.value("rjctcode_descrip").toString());
  }
} 
