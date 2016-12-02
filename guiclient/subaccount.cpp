/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "subaccount.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include "errorReporter.h"

subaccount::subaccount(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _number->setMaxLength(_metrics->value("GLSubaccountSize").toInt());
  _cachedNumber = "";
}

subaccount::~subaccount()
{
  // no need to delete child widgets, Qt does it all for us
}

void subaccount::languageChange()
{
  retranslateUi(this);
}

enum SetResponse subaccount::set(const ParameterList &pParams )
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("subaccnt_id", &valid);
  if (valid)
  {
    _subaccntid = param.toInt();
    populate();
  }
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _number->setEnabled(false);
      _descrip->setEnabled(false);
      _close->setText(tr("&Close"));
    }
  }
  return NoError;
}

void subaccount::sSave()
{
  XSqlQuery subaccountSave;
  if (_number->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Sub Account"),
                            tr("You must enter a valid Number.") );
      return;
  }
  
  subaccountSave.prepare("SELECT subaccnt_id"
            "  FROM subaccnt"
            " WHERE((subaccnt_id != :subaccnt_id)"
            "   AND (subaccnt_number=:subaccnt_number))");
  subaccountSave.bindValue(":subaccnt_id", _subaccntid);
  subaccountSave.bindValue(":subaccnt_number", _number->text());
  subaccountSave.exec();
  if(subaccountSave.first())
  {
    QMessageBox::critical(this, tr("Duplicate Sub Account Number"),
      tr("A Sub Account Number already exists for the one specified.") );
    return;
  }

  if (_mode == cNew)
  {
    subaccountSave.exec("SELECT NEXTVAL('subaccnt_subaccnt_id_seq') AS subaccnt_id;");
    if (subaccountSave.first())
      _subaccntid = subaccountSave.value("subaccnt_id").toInt();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sub Account Information"),
                                  subaccountSave, __FILE__, __LINE__))
    {
      return;
    }
    
    subaccountSave.prepare( "INSERT INTO subaccnt "
               "( subaccnt_id, subaccnt_number, subaccnt_descrip ) "
               "VALUES "
               "( :subaccnt_id, :subaccnt_number, :subaccnt_descrip );" );
  }
  else if (_mode == cEdit)
  {
    if (_number->text() != _cachedNumber &&
        QMessageBox::question(this, tr("Change All Accounts?"),
                              tr("<p>The old Subaccount Number %1 might be "
                                 "used by existing Accounts. Would you like to "
                                 "change all accounts that use it to Subaccount"
                                 " Number %2?<p>If you answer 'No' then change "
                                 "the Number back to %3 and Save again.")
                                .arg(_cachedNumber)
                                .arg(_number->text())
                                .arg(_cachedNumber),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    subaccountSave.prepare( "UPDATE subaccnt "
               "SET subaccnt_number=:subaccnt_number,"
               "    subaccnt_descrip=:subaccnt_descrip "
               "WHERE (subaccnt_id=:subaccnt_id);" );
  }
  
  subaccountSave.bindValue(":subaccnt_id", _subaccntid);
  subaccountSave.bindValue(":subaccnt_number", _number->text());
  subaccountSave.bindValue(":subaccnt_descrip", _descrip->toPlainText());
  subaccountSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Sub Account Information"),
                                subaccountSave, __FILE__, __LINE__))
  {
    return;
  }
  
  done(_subaccntid);
}

void subaccount::populate()
{
  XSqlQuery subaccountpopulate;
  subaccountpopulate.prepare( "SELECT * "
             "FROM subaccnt "
             "WHERE (subaccnt_id=:subaccnt_id);" );
  subaccountpopulate.bindValue(":subaccnt_id", _subaccntid);
  subaccountpopulate.exec();
  if (subaccountpopulate.first())
  {
    _number->setText(subaccountpopulate.value("subaccnt_number").toString());
    _descrip->setText(subaccountpopulate.value("subaccnt_descrip").toString());
    _cachedNumber = subaccountpopulate.value("subaccnt_number").toString();
  }
}
