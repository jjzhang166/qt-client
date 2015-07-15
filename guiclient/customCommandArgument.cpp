/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customCommandArgument.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

customCommandArgument::customCommandArgument(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_accept, SIGNAL(clicked()), this, SLOT(sSave()));

  _mode = cNew;
  _cmdargid = -1;
  _cmdid = -1;
}

customCommandArgument::~customCommandArgument()
{
  // no need to delete child widgets, Qt does it all for us
}

void customCommandArgument::languageChange()
{
  retranslateUi(this);
}

enum SetResponse customCommandArgument::set( const ParameterList & pParams )
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("cmd_id", &valid);
  if(valid)
    _cmdid = param.toInt();
  
  param = pParams.value("cmdarg_id", &valid);
  if(valid)
  {
    _cmdargid = param.toInt();
    populate();
  }
  
  param = pParams.value("mode", &valid);
  if(valid)
  {
    QString mode = param.toString();
    if("new" == mode)
      _mode = cNew;
    else if("edit" == mode)
      _mode = cEdit;
  }
  
  return NoError;
}

void customCommandArgument::sSave()
{
  XSqlQuery customSave;
  if(_argument->text().trimmed().isEmpty())
  {
    QMessageBox::warning(this, tr("No Argument Specified"),
                      tr("You must specify an argument in order to save.") );
    return;
  }

  if(cNew == _mode)
    customSave.prepare("INSERT INTO cmdarg"
              "      (cmdarg_cmd_id, cmdarg_order, cmdarg_arg) "
              "VALUES(:cmd_id, :order, :argument);");
  else if(cEdit == _mode)
    customSave.prepare("UPDATE cmdarg"
              "   SET cmdarg_order=:order,"
              "       cmdarg_arg=:argument"
              " WHERE (cmdarg_id=:cmdarg_id); ");

  customSave.bindValue(":cmd_id", _cmdid);
  customSave.bindValue(":cmdarg_id", _cmdargid);
  customSave.bindValue(":order", _order->value());
  customSave.bindValue(":argument", _argument->text());

  if(customSave.exec())
    accept();
  else if (customSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, customSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customCommandArgument::populate()
{
  XSqlQuery custompopulate;
  custompopulate.prepare("SELECT cmdarg_cmd_id, cmdarg_order, cmdarg_arg"
            "  FROM cmdarg"
            " WHERE (cmdarg_id=:cmdarg_id);");
  custompopulate.bindValue(":cmdarg_id", _cmdargid);
  custompopulate.exec();
  if(custompopulate.first())
  {
    _cmdid = custompopulate.value("cmdarg_cmd_id").toInt();
    _order->setValue(custompopulate.value("cmdarg_order").toInt());
    _argument->setText(custompopulate.value("cmdarg_arg").toString());
  }
  else if (custompopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, custompopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
