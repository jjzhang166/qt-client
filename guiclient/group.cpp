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

#include "group.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

/*
 *  Constructs a group as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
group::group(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_addAll, SIGNAL(clicked()), this, SLOT(sAddAll()));
  connect(_revoke, SIGNAL(clicked()), this, SLOT(sRevoke()));
  connect(_revokeAll, SIGNAL(clicked()), this, SLOT(sRevokeAll()));
  connect(_module, SIGNAL(activated(const QString&)), this, SLOT(sModuleSelected(const QString&)));
  connect(_available, SIGNAL(valid(bool)), _add, SLOT(setEnabled(bool)));
  connect(_granted, SIGNAL(itemSelected(int)), this, SLOT(sRevoke()));
  connect(_granted, SIGNAL(valid(bool)), _revoke, SLOT(setEnabled(bool)));
  connect(_available, SIGNAL(itemSelected(int)), this, SLOT(sAdd()));


  _available->addColumn("Available Privileges", -1, Qt::AlignLeft);
  _granted->addColumn("Granted Privileges", -1, Qt::AlignLeft);

  q.exec( "SELECT DISTINCT priv_module "
          "FROM priv "
          "ORDER BY priv_module;" );
  while (q.next())
    _module->insertItem(q.value("priv_module").toString());

}

/*
 *  Destroys the object and frees any allocated resources
 */
group::~group()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void group::languageChange()
{
  retranslateUi(this);
}

enum SetResponse group::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("grp_id", &valid);
  if (valid)
  {
    _grpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      q.exec("SELECT NEXTVAL('grp_grp_id_seq') AS grp_id;");
      if (q.first())
        _grpid = q.value("grp_id").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      _mode = cNew;
      q.exec("BEGIN;");
      q.prepare( "INSERT INTO grp "
                 "( grp_id, grp_name, grp_descrip)"
                 "VALUES( :grp_id, :grp_id, '' );" );
      q.bindValue(":grp_id", _grpid);
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        q.exec("ROLLBACK;");
        return UndefinedError;
      }
      
      _module->setCurrentItem(0);
      sModuleSelected(_module->text(0));
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  return NoError;
}

void group::sClose()
{
  if(cNew == _mode)
  {
    q.prepare("ROLLBACK;");
  }

  reject();
}

void group::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ((_mode == cNew) && (_name->text().length() != 0))
  {
    q.prepare( "SELECT grp_id "
               "  FROM grp "
               " WHERE (UPPER(grp_name)=UPPER(:grp_name));" );
    q.bindValue(":grp_name", _name->text());
    q.exec();
    if (q.first())
    {
      _grpid = q.value("grp_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void group::sSave()
{
  _name->setText(_name->text().stripWhiteSpace().upper());
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Name"),
                              tr("You must enter a valid Name for this Group.") );
    _name->setFocus();
    return;
  }

  q.prepare( "UPDATE grp "
             "   SET grp_name=:grp_name,"
             "       grp_descrip=:grp_descrip "
             " WHERE(grp_id=:grp_id);" );
  q.bindValue(":grp_id", _grpid);
  q.bindValue(":grp_name", _name->text());
  q.bindValue(":grp_descrip", _description->text().stripWhiteSpace());
  q.exec();

  q.exec("COMMIT;");
  _mode = cEdit;

  done(_grpid);
}

void group::sModuleSelected(const QString &pModule)
{
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
    XTreeWidgetItem *granted = NULL;
    XTreeWidgetItem *available = NULL;

//  Insert each priv into either the available or granted list
    XSqlQuery grppriv;
    grppriv.prepare( "SELECT priv_id "
                     "FROM priv, grppriv "
                     "WHERE ( (grppriv_priv_id=priv_id)"
                     " AND (grppriv_grp_id=:grp_id)"
                     " AND (priv_module=:priv_module) );" );
    grppriv.bindValue(":grp_id", _grpid);
    grppriv.bindValue(":priv_module", _module->currentText());
    grppriv.exec();
    do
    {
      if (grppriv.findFirst("priv_id", privs.value("priv_id").toInt()) == -1)
        available = new XTreeWidgetItem(_available, available, privs.value("priv_id").toInt(), privs.value("priv_name"));
      else
        granted = new XTreeWidgetItem(_granted, granted, privs.value("priv_id").toInt(), privs.value("priv_name"));
    }
    while (privs.next());
  }
}

void group::sAdd()
{
  q.prepare("SELECT grantPrivGroup(:grp_id, :priv_id) AS result;");
  q.bindValue(":grp_id", _grpid);
  q.bindValue(":priv_id", _available->id());
  q.exec();

  sModuleSelected(_module->currentText());
}

void group::sAddAll()
{
  q.prepare("SELECT grantAllModulePrivGroup(:grp_id, :module) AS result;");
  q.bindValue(":grp_id", _grpid);
  q.bindValue(":module", _module->currentText());
  q.exec();

  sModuleSelected(_module->currentText());
}

void group::sRevoke()
{
  q.prepare("SELECT revokePrivGroup(:grp_id, :priv_id) AS result;");
  q.bindValue(":grp_id", _grpid);
  q.bindValue(":priv_id", _granted->id());
  q.exec();

  sModuleSelected(_module->currentText());
}

void group::sRevokeAll()
{
  q.prepare("SELECT revokeAllModulePrivGroup(:grp_id, :module) AS result;");
  q.bindValue(":grp_id", _grpid);
  q.bindValue(":module", _module->currentText());
  q.exec();

  sModuleSelected(_module->currentText());
}

void group::populate()
{
  q.prepare( "SELECT grp_name, grp_descrip "
             "  FROM grp "
             " WHERE(grp_id=:grp_id);" );
  q.bindValue(":grp_id", _grpid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("grp_name"));
    _description->setText(q.value("grp_descrip"));

    q.prepare( "SELECT priv_module "
               "FROM grppriv, priv "
               "WHERE ( (grppriv_priv_id=priv_id)"
               " AND (grppriv_id=:grp_id) ) "
               "ORDER BY priv_module "
               "LIMIT 1;" );
    q.bindValue(":grp_id", _grpid);
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

