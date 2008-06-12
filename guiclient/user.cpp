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

#include "user.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <qmd5.h>

user::user(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_addAll, SIGNAL(clicked()), this, SLOT(sAddAll()));
  connect(_revoke, SIGNAL(clicked()), this, SLOT(sRevoke()));
  connect(_revokeAll, SIGNAL(clicked()), this, SLOT(sRevokeAll()));
  connect(_module, SIGNAL(activated(const QString&)), this, SLOT(sModuleSelected(const QString&)));
  connect(_granted, SIGNAL(itemSelected(int)), this, SLOT(sRevoke()));
  connect(_available, SIGNAL(itemSelected(int)), this, SLOT(sAdd()));
  connect(_username, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_enhancedAuth, SIGNAL(toggled(bool)), this, SLOT(sEnhancedAuthUpdate()));
  connect(_grantedGroup, SIGNAL(itemSelected(int)), this, SLOT(sRevokeGroup()));
  connect(_availableGroup, SIGNAL(itemSelected(int)), this, SLOT(sAddGroup()));
  connect(_addGroup, SIGNAL(clicked()), this, SLOT(sAddGroup()));
  connect(_revokeGroup, SIGNAL(clicked()), this, SLOT(sRevokeGroup()));

  _available->addColumn("Available Privileges", -1, Qt::AlignLeft);
  _granted->addColumn("Granted Privileges", -1, Qt::AlignLeft);

  _availableGroup->addColumn("Available Groups", -1, Qt::AlignLeft);
  _grantedGroup->addColumn("Granted Groups", -1, Qt::AlignLeft);

  _locale->setType(XComboBox::Locales);

  q.exec( "SELECT DISTINCT priv_module "
          "FROM priv "
          "ORDER BY priv_module;" );
  while (q.next())
    _module->insertItem(q.value("priv_module").toString());

  _authCache = false;
  if(_evaluation == true)
  {
    _enhancedAuth->setEnabled(false);
    _passwd->setEnabled(false);
    _verify->setEnabled(false);
  }
  
  if (!_metrics->boolean("Routings"))
  {
    _woTimeClockOnly->setChecked(FALSE);
    _woTimeClockOnly->hide();
  }
}

user::~user()
{
  // no need to delete child widgets, Qt does it all for us
}

void user::languageChange()
{
  retranslateUi(this);
}

enum SetResponse user::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("username", &valid);
  if (valid)
  {
    _cUsername = param.toString();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _module->setCurrentItem(0);
      sModuleSelected(_module->text(0));

      if (_cUsername.isEmpty())
        _username->setFocus();
      else
      {
        _username->setEnabled(false);
        _username->setText(_cUsername);
        _active->setFocus();
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _username->setEnabled(FALSE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void user::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM usrpriv "
               "WHERE (usrpriv_username=:username);" );
    q.bindValue(":username", _cUsername);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  reject();
}

void user::sSave()
{
  if (_passwd->text() != _verify->text())
  {
    QMessageBox::warning( this, tr("Password do not Match"),
                          tr( "The entered password and verify do not match\n"
                              "Please enter both again carefully." ));

    _passwd->clear();
    _verify->clear();
    _passwd->setFocus();

    return;
  }

  QString passwd = _passwd->text();
  if(_enhancedAuth->isChecked())
  {
    passwd = passwd + "OpenMFG" + _username->text();
    passwd = QMd5(passwd);
  }

  if (_mode == cNew)
  {
    q.prepare( "SELECT usesysid"
               "  FROM pg_user"
               " WHERE (usename=:username);" );
    q.bindValue(":username", _username->text());
    q.exec();
    if (!q.first())
    {
      q.prepare( QString( "SELECT createUser(:username, :createUsers); "
                          "ALTER USER %1 WITH PASSWORD :password;" )
                 .arg(_username->text()) );
      q.bindValue(":username", _username->text());
      q.bindValue(":createUsers", QVariant(_createUsers->isChecked(), 0));
      q.bindValue(":password", passwd);
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    else
      q.exec( QString("ALTER GROUP openmfg ADD USER %1;")
              .arg(_username->text()) );

    q.prepare( "INSERT INTO usr "
               "( usr_username, usr_propername,"
               "  usr_email, usr_initials, usr_locale_id,"
               "  usr_agent, usr_active,"
	       "  usr_window ) "
               "VALUES "
               "( :usr_username, :usr_propername,"
               "  :usr_email, :usr_initials, :usr_locale_id,"
               "  :usr_agent, :usr_active,"
	       "  :usr_window );" );
  }
  else if (_mode == cEdit)
  {
    if (_passwd->text() != "        ")
    {
      q.prepare( QString( "ALTER USER %1 WITH PASSWORD :password;")
                 .arg(_username->text()) );
      q.bindValue(":password", passwd);
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    if(_createUsers->isEnabled())
    {
      q.prepare("SELECT setUserCanCreateUsers(:username, :createUsers);");
      q.bindValue(":username", _username->text());
      q.bindValue(":createUsers", QVariant(_createUsers->isChecked(), 0));
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    
    q.prepare( "UPDATE usr "
               "SET usr_propername=:usr_propername, usr_initials=:usr_initials,"
	       "    usr_email=:usr_email, usr_locale_id=:usr_locale_id,"
	       "    usr_agent=:usr_agent, usr_active=:usr_active,"
	       "    usr_window=:usr_window "
               "WHERE (usr_username=:usr_username);" );
  }

  q.bindValue(":usr_username", _username->text().stripWhiteSpace().lower());
  q.bindValue(":usr_propername", _properName->text());
  q.bindValue(":usr_email", _email->text());
  q.bindValue(":usr_initials", _initials->text());
  q.bindValue(":usr_locale_id", _locale->id());
  q.bindValue(":usr_agent", QVariant(_agent->isChecked(), 0));
  q.bindValue(":usr_active", QVariant(_active->isChecked(), 0));
  // keep synchronized with the select below, GUIClient, and main
  q.bindValue(":usr_window", _woTimeClockOnly->isChecked() ? "woTimeClock" : "");
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT setUserPreference(:username, 'DisableExportContents', :value) AS result");
  q.bindValue(":username", _username->text().stripWhiteSpace().lower());
  q.bindValue(":value", (_exportContents->isChecked() ? "t" : "f"));
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT setUserPreference(:username, 'UseEnhancedAuthentication', :value) AS result");
  q.bindValue(":username", _username->text().stripWhiteSpace().lower());
  q.bindValue(":value", (_enhancedAuth->isChecked() ? "t" : "f"));
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void user::sModuleSelected(const QString &pModule)
{
  XTreeWidgetItem *granted = NULL;
  XTreeWidgetItem *available = NULL;

  _availableGroup->clear();
  _grantedGroup->clear();
  XSqlQuery groups;
  groups.prepare("SELECT grp_id, grp_name, usrgrp_id"
                 "  FROM grp LEFT OUTER JOIN usrgrp"
                 "    ON (usrgrp_grp_id=grp_id AND usrgrp_username=:username);");
  groups.bindValue(":username", _cUsername);
  groups.exec();
  while(groups.next())
  {
    if (groups.value("usrgrp_id").toInt() == 0)
      available = new XTreeWidgetItem(_availableGroup, available, groups.value("grp_id").toInt(), groups.value("grp_name"));
    else
      granted = new XTreeWidgetItem(_grantedGroup, granted, groups.value("grp_id").toInt(), groups.value("grp_name"));
  }

  _available->clear();
  _granted->clear();

  XSqlQuery privs;
  privs.prepare( "SELECT priv_id, priv_name "
                 "FROM priv "
                 "WHERE (priv_module=:priv_module) "
                 "ORDER BY priv_name;" );
  privs.bindValue(":priv_module", pModule);
  privs.exec();
  if (privs.first())
  {
    granted = NULL;
    available = NULL;

//  Insert each priv into either the available or granted list
    XSqlQuery usrpriv;
    usrpriv.prepare( "SELECT priv_id "
                     "FROM priv, usrpriv "
                     "WHERE ( (usrpriv_priv_id=priv_id)"
                     " AND (usrpriv_username=:username)"
                     " AND (priv_module=:priv_module) );" );
    usrpriv.bindValue(":username", _cUsername);
    usrpriv.bindValue(":priv_module", _module->currentText());
    usrpriv.exec();

    XSqlQuery grppriv;
    grppriv.prepare("SELECT priv_id"
                    "  FROM priv, grppriv, usrgrp"
                    " WHERE((usrgrp_grp_id=grppriv_grp_id)"
                    "   AND (grppriv_priv_id=priv_id)"
                    "   AND (usrgrp_username=:username)"
                    "   AND (priv_module=:priv_module));");
    grppriv.bindValue(":username", _cUsername);
    grppriv.bindValue(":priv_module", _module->currentText());
    grppriv.exec();

    do
    {
      if (usrpriv.findFirst("priv_id", privs.value("priv_id").toInt()) == -1 && grppriv.findFirst("priv_id", privs.value("priv_id").toInt()) == -1)
        available = new XTreeWidgetItem(_available, available, privs.value("priv_id").toInt(), privs.value("priv_name"));
      else
      {
        granted = new XTreeWidgetItem(_granted, granted, privs.value("priv_id").toInt(), privs.value("priv_name"));
        if(usrpriv.findFirst("priv_id", privs.value("priv_id").toInt()) == -1)
          granted->setTextColor(Qt::gray);
      }
    }
    while (privs.next());
  }
}

void user::sAdd()
{
  q.prepare("SELECT grantPriv(:username, :priv_id) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":priv_id", _available->id());
  q.exec();

  sModuleSelected(_module->currentText());
}

void user::sAddAll()
{
  q.prepare("SELECT grantAllModulePriv(:username, :module) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":module", _module->currentText());
  q.exec();

  sModuleSelected(_module->currentText());
}

void user::sRevoke()
{
  q.prepare("SELECT revokePriv(:username, :priv_id) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":priv_id", _granted->id());
  q.exec();

  sModuleSelected(_module->currentText());
}

void user::sRevokeAll()
{
  q.prepare("SELECT revokeAllModulePriv(:username, :module) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":module", _module->currentText());
  q.exec();

  sModuleSelected(_module->currentText());
}

void user::sAddGroup()
{
  q.prepare("SELECT grantGroup(:username, :grp_id) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":grp_id", _availableGroup->id());
  q.exec();

  sModuleSelected(_module->currentText());
}

void user::sRevokeGroup()
{
  q.prepare("SELECT revokeGroup(:username, :grp_id) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":grp_id", _grantedGroup->id());
  q.exec();

  sModuleSelected(_module->currentText());
}

void user::sCheck()
{
  _cUsername = _username->text().stripWhiteSpace();
  if (_cUsername.length() > 0)
  {
    q.prepare( "SELECT * "
               "FROM usr "
               "WHERE (usr_username=:username);" );
    q.bindValue(":username", _cUsername);
    q.exec();
    if (q.first())
    {
      populate();
      _mode = cEdit;
      _username->setEnabled(FALSE);
      _properName->setFocus();
    }
    else
    {
      q.prepare( "SELECT userId(:username) AS userid,"
                 "       userCanCreateUsers(CURRENT_USER) AS createusers;" );
      q.bindValue(":username", _cUsername);
      q.exec();
      if (!q.first())
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        reject();
        return;
      }

      if ( (q.value("userid").toInt() == -1) &&
           (!q.value("createusers").toBool()) )
      {
        QMessageBox::warning( this, tr("Cannot Create System User"),
                              tr( "A User with the entered username does not exist in the system and you do not have privilege to create a new system User.\n"
                                  "You may create the OpenMFG User but the new User will not be able to log into OpenMFG." ) );
      }
    }
  }
}

void user::populate()
{
  q.prepare( "SELECT *, userCanCreateUsers(usr_username) AS createusers,"
             "       userCanCreateUsers(CURRENT_USER) AS enablecreateusers,"
             "       emp_id "
             "FROM usr LEFT OUTER JOIN emp ON (usr_id=emp_usr_id) "
             "WHERE (usr_username=:usr_username);" );
  q.bindValue(":usr_username", _cUsername);
  q.exec();
  if (q.first())
  {
    _username->setText(q.value("usr_username"));
    _active->setChecked(q.value("usr_active").toBool());
    _properName->setText(q.value("usr_propername"));
    _initials->setText(q.value("usr_initials"));
    _email->setText(q.value("usr_email"));
    //_passwd->setText(q.value("usr_passwd"));
    //_verify->setText(q.value("usr_passwd"));
    _locale->setId(q.value("usr_locale_id").toInt());
    _agent->setChecked(q.value("usr_agent").toBool());
    _createUsers->setChecked(q.value("createusers").toBool());
    _createUsers->setEnabled(q.value("enablecreateusers").toBool());
    // keep synchronized with the insert/update above, GUIClient, and main
    _woTimeClockOnly->setChecked(q.value("usr_window").toString()==("woTimeClock"));
    _employee->setId(q.value("emp_id").toInt());

    _passwd->setText("        ");
    _verify->setText("        ");

    q.prepare( "SELECT usrpref_value "
               "  FROM usrpref "
               " WHERE ( (usrpref_name = 'DisableExportContents') "
               "   AND (usrpref_username=:username) ); ");
    q.bindValue(":username", _cUsername);
    q.exec();
    if(q.first())
      _exportContents->setChecked(q.value("usrpref_value").toString()=="t");
    else
      _exportContents->setChecked(FALSE);

    q.prepare( "SELECT usrpref_value "
               "  FROM usrpref "
               " WHERE ( (usrpref_name = 'UseEnhancedAuthentication') "
               "   AND (usrpref_username=:username) ); ");
    q.bindValue(":username", _cUsername);
    q.exec();
    _authCache = false;
    if(q.first())
      _authCache = (q.value("usrpref_value").toString()=="t");
    _enhancedAuth->setChecked(_authCache);

    q.prepare( "SELECT priv_module "
               "FROM usrpriv, priv "
               "WHERE ( (usrpriv_priv_id=priv_id)"
               " AND (usrpriv_username=:username) ) "
               "ORDER BY priv_module "
               "LIMIT 1;" );
    q.bindValue(":username", _cUsername);
    q.exec();
    if (q.first())
    {
      for (int counter = 0; counter < _module->count(); counter++)
      {
        if (_module->text(counter) == q.value("priv_module").toString())
        {
          _module->setCurrentItem(counter);
          sModuleSelected(_module->text(counter));
        }
      }
    }
    else
    {
      _module->setCurrentItem(0);
      sModuleSelected(_module->text(0));
    }
  }
}

void user::sEnhancedAuthUpdate()
{
  if((_mode == cEdit) && (_authCache != _enhancedAuth->isChecked()) && (_passwd->text() == "        "))
    QMessageBox::information( this, tr("Enhanced Authentication"),
      tr("You have changed this user's Enhanced Authentication option.\n"
         "The password must be updated in order for this change to take\n"
         "full effect.") );
}
