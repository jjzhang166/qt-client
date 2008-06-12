/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "uiform.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QTextStream>
#include <QFileDialog>
#include <QFile>

#include "scriptEditor.h"
#include "customCommand.h"

uiform::uiform(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_import, SIGNAL(clicked()), this, SLOT(sImport()));
  connect(_export, SIGNAL(clicked()), this, SLOT(sExport()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sFillList()));
  connect(_cmdNew, SIGNAL(clicked()), this, SLOT(sCmdNew()));
  connect(_cmdEdit, SIGNAL(clicked()), this, SLOT(sCmdEdit()));
  connect(_cmdDelete, SIGNAL(clicked()), this, SLOT(sCmdDelete()));
  connect(_scriptNew, SIGNAL(clicked()), this, SLOT(sScriptNew()));
  connect(_scriptEdit, SIGNAL(clicked()), this, SLOT(sScriptEdit()));
  connect(_scriptDelete, SIGNAL(clicked()), this, SLOT(sScriptDelete()));

  _script->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft );
  _script->addColumn(tr("Description"), -1,          Qt::AlignLeft );
  _script->addColumn(tr("Order"),       _ynColumn,   Qt::AlignCenter );
  _script->addColumn(tr("Enabled"),     _ynColumn,   Qt::AlignCenter );

  _commands->addColumn(tr("Module"), _itemColumn, Qt::AlignCenter );
  _commands->addColumn(tr("Menu Label"), -1,     Qt::AlignLeft );
}

uiform::~uiform()
{
  // no need to delete child widgets, Qt does it all for us
}

void uiform::languageChange()
{
  retranslateUi(this);
}

enum SetResponse uiform::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("uiform_id", &valid);
  if (valid)
  {
    _uiformid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _order->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _import->setEnabled(FALSE);
      _enabled->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void uiform::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::warning( this, tr("UI Form Name is Invalid"),
                          tr("<p>You must enter a valid name for this UI Form.") );
    _name->setFocus();
    return;
  }

  if (_source.length() == 0)
  {
    QMessageBox::warning( this, tr("UI Form Source is Empty"),
                          tr("<p>You must enter some source for this UI Form.") );
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('uiform_uiform_id_seq') AS _uiform_id");
    if (q.first())
      _uiformid = q.value("_uiform_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO uiform "
               "(uiform_id, uiform_name, uiform_notes, uiform_order, uiform_enabled, uiform_source) "
               "VALUES "
               "(:uiform_id, :uiform_name, :uiform_notes, :uiform_order, :uiform_enabled, :uiform_source);" );

  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE uiform "
               "SET uiform_name=:uiform_name, uiform_notes=:uiform_notes,"
               "    uiform_order=:uiform_order, uiform_enabled=:uiform_enabled,"
               "    uiform_source=:uiform_source "
               "WHERE (uiform_id=:uiform_id);" );

  q.bindValue(":uiform_id", _uiformid);
  q.bindValue(":uiform_name", _name->text());
  q.bindValue(":uiform_order", _order->value());
  q.bindValue(":uiform_enabled", QVariant(_enabled->isChecked(), 0));
  q.bindValue(":uiform_source", _source);
  q.bindValue(":uiform_notes", _notes->text());

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_uiformid);
}

void uiform::populate()
{
  q.prepare( "SELECT * "
      	     "  FROM uiform "
             " WHERE (uiform_id=:uiform_id);" );
  q.bindValue(":uiform_id", _uiformid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("uiform_name").toString());
    _order->setValue(q.value("uiform_order").toInt());
    _enabled->setChecked(q.value("uiform_enabled").toBool());
    _source = q.value("uiform_source").toString();
    _notes->setText(q.value("uiform_notes").toString());

    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void uiform::sImport()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), QString::null, tr("UI (*.ui)"));
  if(filename.isNull())
    return;

  QFile file(filename);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QMessageBox::critical(this, tr("Could not import file"), file.errorString());
    return;
  }
  QTextStream ts(&file);
  _source=ts.readAll();
  file.close();
}

void uiform::sExport()
{
  QString filename = QFileDialog::getSaveFileName( this, tr("Save File"), QString::null, tr("UI (*.ui)"));
  if(filename.isNull())
    return;

  QFileInfo fi(filename);
  if(fi.suffix().isEmpty())
    filename += ".ui";

  QFile file(filename);
  if(!file.open(QIODevice::WriteOnly))
  {
    QMessageBox::critical(this, tr("Could not export file"), file.errorString());
    return;
  }

  QTextStream ts(&file);
  ts.setCodec("UTF-8");
  ts << _source;
  file.close();
}

void uiform::sScriptNew()
{
  ParameterList params;
  params.append("mode", "new");

  scriptEditor newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void uiform::sScriptEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("script_id", _script->id());

  scriptEditor newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void uiform::sScriptDelete()
{
  q.prepare( "DELETE FROM script "
             "WHERE (script_id=:script_id);" );
  q.bindValue(":script_id", _script->id());
  q.exec();

  sFillList();
}

void uiform::sCmdNew()
{
  ParameterList params;
  params.append("mode", "new");

  customCommand newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void uiform::sCmdEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmd_id", _commands->id());

  customCommand newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void uiform::sCmdDelete()
{
  q.prepare("BEGIN; "
            "DELETE FROM cmdarg WHERE (cmdarg_cmd_id=:cmd_id); "
            "DELETE FROM cmd WHERE (cmd_id=:cmd_id); "
            "SELECT updateCustomPrivs(); "
            "COMMIT; ");
  q.bindValue(":cmd_id", _commands->id());
  if(q.exec())
    sFillList();
  else
    systemError( this, tr("A System Error occurred at customCommands::%1")
                             .arg(__LINE__) );
}

void uiform::sFillList()
{
  q.prepare( "SELECT script_id, script_name, script_notes,"
             "       script_order, formatBoolYN(script_enabled)"
             "  FROM script"
             " WHERE(script_name=:name)"
             " ORDER BY script_name, script_order, script_id;" );
  q.bindValue(":name", _name->text());
  q.exec();
  _script->populate(q);

  q.prepare("SELECT DISTINCT cmd_id, cmd_module, cmd_title"
            "  FROM cmd JOIN cmdarg ON (cmdarg_cmd_id=cmd_id)"
            "  WHERE((cmd_module IN ('CRM', 'Products','Inventory','Schedule','Purchase', "
            "                        'Manufacture','CRM','Sales','Accounting','System'))"
            "    AND (cmdarg_arg='uiform='||:name)) "
            " ORDER BY cmd_module, cmd_title;");
  q.bindValue(":name", _name->text());
  q.exec();
  _commands->populate(q);

}

