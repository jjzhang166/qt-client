/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customCommand.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "customCommandArgument.h"
#include "errorReporter.h"

customCommand::customCommand(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_accept, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _mode = cNew;
  _cmdid = -1;
  _saved = false;
  
  _arguments->addColumn(tr("Order"), _whsColumn, Qt::AlignCenter,true, "cmdarg_order");
  _arguments->addColumn(tr("Argument"),      -1, Qt::AlignLeft,  true, "cmdarg_arg");
  
  _module->clear();
  int i = 0;
  _module->append(i++, "Products");
  _module->append(i++, "Inventory");
  if (_metrics->value("Application") != "PostBooks")
    _module->append(i++, "Schedule");
  _module->append(i++, "Purchase");
  _module->append(i++, "Manufacture");
  _module->append(i++, "CRM");
  _module->append(i++, "Sales");
  _module->append(i++, "Accounting");
  _module->append(i++, "System");
}

customCommand::~customCommand()
{
  // no need to delete child widgets, Qt does it all for us
}

void customCommand::languageChange()
{
  retranslateUi(this);
}

enum SetResponse customCommand::set( const ParameterList & pParams )
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if(valid)
  {
    QString mode = param.toString();
    if("new" == mode)
      setMode(cNew);
    else if("edit" == mode)
      setMode(cEdit);
    else if("view" == mode)
      setMode(cView);
  }

  // after setMode because populate() may change mode
  param = pParams.value("cmd_id", &valid);
  if(valid)
  {
    _cmdid = param.toInt();
    populate();
  }
  
  return NoError;
}

void customCommand::setMode(const int pmode)
{
  XSqlQuery custometMode;
  switch (pmode)
  {
    case cNew:
      custometMode.prepare("SELECT nextval('cmd_cmd_id_seq') AS result;");
      custometMode.exec();
      if(custometMode.first())
        _cmdid = custometMode.value("result").toInt();
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Custom Command Information"),
                                    custometMode, __FILE__, __LINE__))
      {
        return;
      }
      /* fallthru */

    case cEdit:
      _module->setEnabled(true);
      _title->setEnabled(true);
      _privname->setEnabled(true);
      _name->setEnabled(true);
      _description->setEnabled(true);
      _executable->setEnabled(true);
      _accept->show();
      _close->setText(tr("&Cancel"));

      if (pmode == cNew)
        _module->setFocus();
      else
        _accept->setFocus();
      break;

    case cView:
    default:
      _module->setEnabled(false);
      _title->setEnabled(false);
      _privname->setEnabled(false);
      _name->setEnabled(false);
      _description->setEnabled(false);
      _executable->setEnabled(false);
      _accept->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
      break;
  }
  _mode = pmode;
}

void customCommand::sSave()
{
  XSqlQuery customSave;
  if(!save())
    return;

  // make sure the custom privs get updated
  customSave.exec("SELECT updateCustomPrivs();");
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Custom Command"),
                                customSave, __FILE__, __LINE__))
  {
    return;
  }

  accept();
}
bool customCommand::save()
{
  XSqlQuery customave;
  if(_title->text().trimmed().isEmpty())
  {
    QMessageBox::warning( this, tr("Cannot Save"),
      tr("You must enter in a Menu Label for this command.") );
    return false;
  }

  if(_executable->text().trimmed().isEmpty())
  {
    QMessageBox::warning( this, tr("Cannot Save"),
      tr("You must enter in a program to execute.") );
    return false;
  }

  if(_privname->text().trimmed().contains(QRegExp("\\s")))
  {
    QMessageBox::warning( this, tr("Cannot Save"),
      tr("Priv Name may not contain spaces.") );
    return false;
  }

  if((cNew == _mode) && !_saved)
    customave.prepare("INSERT INTO cmd"
              "      (cmd_id, cmd_module, cmd_title, cmd_privname,"
              "       cmd_name, cmd_descrip, cmd_executable) "
              "VALUES(:cmd_id, :cmd_module, :cmd_title, :cmd_privname,"
              "       :cmd_name, :cmd_descrip, :cmd_executable);");
  else if(cEdit == _mode || ((cNew == _mode) && _saved))
    customave.prepare("UPDATE cmd"
              "   SET cmd_module=:cmd_module,"
              "       cmd_title=:cmd_title,"
              "       cmd_privname=:cmd_privname,"
              "       cmd_name=:cmd_name,"
              "       cmd_descrip=:cmd_descrip,"
              "       cmd_executable=:cmd_executable "
              " WHERE (cmd_id=:cmd_id);");

  customave.bindValue(":cmd_id", _cmdid);
  customave.bindValue(":cmd_module", _module->code());
  customave.bindValue(":cmd_title", _title->text());
  customave.bindValue(":cmd_privname", _privname->text().trimmed());
  if(!_name->text().isEmpty())
    customave.bindValue(":cmd_name", _name->text());
  customave.bindValue(":cmd_descrip", _description->toPlainText());
  customave.bindValue(":cmd_executable", _executable->text());
  customave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Custom Command"),
                                customave, __FILE__, __LINE__))
  {
    return false;
  }
  _saved = true;
  return true;
}

void customCommand::reject()
{
  if(cNew == _mode)
  {
    QSqlQuery query;
    query.prepare("DELETE FROM cmdarg WHERE (cmdarg_cmd_id=:cmd_id);");
    query.bindValue(":cmd_id", _cmdid);
    query.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Rejecting Custom Command"),
                                  query, __FILE__, __LINE__))
    {
      return;
    }
    query.prepare("DELETE FROM cmd WHERE (cmd_id=:cmd_id);");
    query.bindValue(":cmd_id", _cmdid);
    query.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Rejecting Custom Command"),
                                  query, __FILE__, __LINE__))
    {
      return;
    }
  }

  XDialog::reject();
}

void customCommand::sNew()
{
  if(!save())
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("cmd_id", _cmdid);

  customCommandArgument newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void customCommand::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmdarg_id", _arguments->id());

  customCommandArgument newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void customCommand::sDelete()
{
  XSqlQuery customDelete;
  customDelete.prepare("DELETE FROM cmdarg WHERE (cmdarg_id=:cmdarg_id);");
  customDelete.bindValue(":cmdarg_id", _arguments->id());
  if(customDelete.exec())
    sFillList();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Custom Command Argument"),
                                customDelete, __FILE__, __LINE__))
  {
    return;
  }
}

void customCommand::populate()
{
  XSqlQuery custompopulate;
  custompopulate.prepare("SELECT cmd.*, COALESCE(pkghead_indev,true) AS editable "
            "  FROM cmd, pg_class, pg_namespace "
            "  LEFT OUTER JOIN pkghead ON (nspname=pkghead_name) "
            " WHERE ((cmd.tableoid=pg_class.oid)"
            "   AND (relnamespace=pg_namespace.oid) "
            "   AND  (cmd_id=:cmd_id));");
  custompopulate.bindValue(":cmd_id", _cmdid);
  custompopulate.exec();
  if(custompopulate.first())
  {
    _module->setCode(custompopulate.value("cmd_module").toString());
    if (_module->id() < 0)
    {
      _module->append(_module->count(), custompopulate.value("cmd_module").toString());
      _module->setCode(custompopulate.value("cmd_module").toString());
    }
    _title->setText(custompopulate.value("cmd_title").toString());
    _oldPrivname = custompopulate.value("cmd_privname").toString();
    _privname->setText(custompopulate.value("cmd_privname").toString());
    _name->setText(custompopulate.value("cmd_name").toString());
    _executable->setText(custompopulate.value("cmd_executable").toString());
    _description->setText(custompopulate.value("cmd_descrip").toString());
    if (!custompopulate.value("editable").toBool())
      setMode(cView);

    sFillList();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Custom Command Information"),
                                custompopulate, __FILE__, __LINE__))
  {
    return;
  }
}

void customCommand::sFillList()
{
  XSqlQuery customFillList;
  customFillList.prepare("SELECT cmdarg_id, cmdarg_order, cmdarg_arg"
            "  FROM cmdarg"
            " WHERE (cmdarg_cmd_id=:cmd_id)"
            " ORDER BY cmdarg_order, cmdarg_id;");
  customFillList.bindValue(":cmd_id", _cmdid);
  customFillList.exec();
  _arguments->populate(customFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Custom Command Information"),
                                customFillList, __FILE__, __LINE__))
  {
    return;
  }
}
