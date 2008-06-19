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

#include "lotSerial.h"
#include "characteristicAssignment.h"
#include "lotSerialRegistration.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include <comments.h>

lotSerial::lotSerial(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_lotSerial, SIGNAL(valid(bool)), this, SLOT(populate()));
    connect(_notes, SIGNAL(textChanged()), this, SLOT(sChanged()));
    connect(_deleteChar,  SIGNAL(clicked()), this, SLOT(sDeleteCharass()));
    connect(_editChar,    SIGNAL(clicked()), this, SLOT(sEditCharass()));
    connect(_newChar,     SIGNAL(clicked()), this, SLOT(sNewCharass()));
    connect(_deleteReg,  SIGNAL(clicked()), this, SLOT(sDeleteReg()));
    connect(_editReg,    SIGNAL(clicked()), this, SLOT(sEditReg()));
    connect(_newReg,     SIGNAL(clicked()), this, SLOT(sNewReg()));

    
    _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft );
    _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft );
    
    _reg->addColumn(tr("Number")      ,        40,  Qt::AlignLeft );
    _reg->addColumn(tr("CRM Account#"),        80,  Qt::AlignLeft );
    _reg->addColumn(tr("Name"        ),	      -1,   Qt::AlignLeft );
    _reg->addColumn(tr("First Name"  ),        80,  Qt::AlignLeft );
    _reg->addColumn(tr("Last Name"   ),        80,  Qt::AlignLeft );
    _reg->addColumn(tr("Phone"       ),        80,  Qt::AlignLeft );
  
    _changed=false;
}

lotSerial::~lotSerial()
{
    // no need to delete child widgets, Qt does it all for us
}

void lotSerial::languageChange()
{
    retranslateUi(this);
}

void lotSerial::populate()
{
  if (_changed)
  {
    if (QMessageBox::question(this, tr("Save changes?"),
                              tr("<p>Notes were changed without saving. "
                                 "If you continue your changes will be lost. "
                                 "Would you like an opportunity to save your changes first?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::Yes)
    {
      disconnect(_lotSerial, SIGNAL(valid(bool)), this, SLOT(populate()));
      _item->setId(_itemidCache);
      _lotSerial->setId(_lsidCache);
      connect(_lotSerial, SIGNAL(valid(bool)), this, SLOT(populate()));
      return;
    }
  }

  q.prepare( "SELECT ls_item_id,ls_notes "
             "FROM ls "
             "WHERE (ls_id=:ls_id );" );
  q.bindValue(":ls_id", _lotSerial->id());
  q.exec();
  if (q.first())
  {
    _lsidCache=_lotSerial->id();
    if (_item->id() == -1)
      _item->setId(q.value("ls_item_id").toInt());
    _itemidCache=_item->id();
    _notes->setText(q.value("ls_notes").toString());
    _changed=false;
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void lotSerial::sSave()
{
  q.prepare("UPDATE ls SET"
            " ls_notes=:notes "
            "WHERE (ls_id=:ls_id);");
  q.bindValue(":notes",_notes->text());
  q.bindValue(":ls_id", _lotSerial->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _notes->clear();
  _changed=false;
  _item->setId(-1);
}

void lotSerial::sChanged()
{
  _changed=true;
}

void lotSerial::sNewCharass()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("ls_id", _lotSerial->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void lotSerial::sEditCharass()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void lotSerial::sDeleteCharass()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void lotSerial::sNewReg()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("ls_id", _lotSerial->id());

  lotSerialRegistration newdlg(this, "", TRUE);
  newdlg.set(params);
  
  newdlg.exec();
  sFillList();
}

void lotSerial::sEditReg()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("lsreg_id", _reg->id());

  lotSerialRegistration newdlg(this, "", TRUE);
  newdlg.set(params);
  
  newdlg.exec();
  sFillList();
}

void lotSerial::sDeleteReg()
{
  q.prepare( "DELETE FROM lsreg "
             "WHERE (lsreg_id=:lsreg_id);"
             "DELETE FROM charass "
             "WHERE ((charass_target_type='LSR') "
             "AND (charass_target_id=:lsreg_id))" );
  q.bindValue(":lsreg_id", _reg->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}


void lotSerial::sFillList()
{
  q.prepare( "SELECT charass_id, char_name, charass_value "
             "FROM charass, char "
             "WHERE ((charass_target_type='LS')"
             " AND   (charass_char_id=char_id)"
             " AND   (charass_target_id=:ls_id) ) "
             "ORDER BY char_name;" );
  q.bindValue(":ls_id", _lotSerial->id());
  q.exec();
  _charass->clear();
  _charass->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  q.prepare( "SELECT lsreg_id,lsreg_number,crmacct_number,crmacct_name,"
             "  cntct_first_name,cntct_last_name,cntct_phone "
             "FROM lsreg "
             "  LEFT OUTER JOIN crmacct ON (lsreg_crmacct_id=crmacct_id), "
             "  cntct "
             "WHERE ((lsreg_cntct_id=cntct_id) "
             "AND (lsreg_ls_id=:ls_id));");
  q.bindValue(":ls_id", _lotSerial->id());
  q.exec();
  _reg->clear();
  _reg->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
