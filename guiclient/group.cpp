/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "group.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "storedProcErrorLookup.h"

#define DEBUG false

group::group(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  XSqlQuery groupgroup;
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_addAll, SIGNAL(clicked()), this, SLOT(sAddAll()));
  connect(_revoke, SIGNAL(clicked()), this, SLOT(sRevoke()));
  connect(_revokeAll, SIGNAL(clicked()), this, SLOT(sRevokeAll()));
  connect(_module, SIGNAL(activated(const QString&)), this, SLOT(sModuleSelected(const QString&)));
  connect(_available, SIGNAL(valid(bool)), _add, SLOT(setEnabled(bool)));
  connect(_granted, SIGNAL(itemSelected(int)), this, SLOT(sRevoke()));
  connect(_granted, SIGNAL(valid(bool)), _revoke, SLOT(setEnabled(bool)));
  connect(_available, SIGNAL(itemSelected(int)), this, SLOT(sAdd()));


  _available->addColumn(tr("Available Privileges"), -1, Qt::AlignLeft, true, "priv_name");
  _available->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "priv_descrip");
  _granted->addColumn(tr("Granted Privileges"), -1, Qt::AlignLeft, true, "priv_name");
  _granted->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "priv_descrip");

  groupgroup.exec( "SELECT DISTINCT priv_module "
          "FROM priv "
          "ORDER BY priv_module;" );
  for (int i = 0; groupgroup.next(); i++)
    _module->append(i, groupgroup.value("priv_module").toString());

  _trapClose = false;
}

group::~group()
{
  // no need to delete child widgets, Qt does it all for us
}

void group::languageChange()
{
  retranslateUi(this);
}

enum SetResponse group::set(const ParameterList &pParams)
{
  XSqlQuery groupet;
  XDialog::set(pParams);
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
      groupet.exec("SELECT NEXTVAL('grp_grp_id_seq') AS grp_id;");
      if (groupet.first())
        _grpid = groupet.value("grp_id").toInt();
      else if (groupet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, groupet.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      _mode = cNew;
      _trapClose = true;
      groupet.exec("BEGIN;");
      groupet.prepare( "INSERT INTO grp "
                 "( grp_id, grp_name, grp_descrip)"
                 "VALUES( :grp_id, :grp_id, '' );" );
      groupet.bindValue(":grp_id", _grpid);
      groupet.exec();
      if (groupet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, groupet.lastError().databaseText(), __FILE__, __LINE__);
        groupet.exec("ROLLBACK;");
        _trapClose = false;
        return UndefinedError;
      }

      _module->setCurrentIndex(0);
      sModuleSelected(_module->itemText(0));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setEnabled(false);
      groupet.exec("BEGIN;");
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(false);
      _description->setEnabled(false);
      _addAll->setEnabled(false);
      _revokeAll->setEnabled(false);
      disconnect(_available, SIGNAL(itemSelected(int)), this, SLOT(sAdd()));
      disconnect(_available, SIGNAL(valid(bool)), _add, SLOT(setEnabled(bool)));
      disconnect(_granted, SIGNAL(itemSelected(int)), this, SLOT(sRevoke()));
      disconnect(_granted, SIGNAL(valid(bool)), _revoke, SLOT(setEnabled(bool)));
      _save->hide();
      _close->setText(tr("&Close"));
    }
  }

  return NoError;
}

/* override reject() instead of closeEvent() because the QDialog docs
   say the Esc key calls reject and the close event cannot be ignored
 */
void group::reject()
{
  if (DEBUG)
    qDebug("group::reject() called with _trapClose = %d", _trapClose);

  if(_trapClose)
  {
    XSqlQuery endtxn;
    switch(QMessageBox::question(this, tr("Save?"),
                                 tr("<p>Do you wish to save your changes?"),
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                 QMessageBox::Yes))
    {
      case QMessageBox::Yes:
        sSave();
        return;
//        endtxn.exec("COMMIT;");
//        break;

      case QMessageBox::No:
        endtxn.exec("ROLLBACK;");
        break;

      case QMessageBox::Cancel:
        return;

      default:
        break;

    }
  }
  
  XDialog::reject();
}

void group::sCheck()
{
  XSqlQuery groupCheck;
  _name->setText(_name->text().trimmed().toUpper());
  if ((_mode == cNew) && (_name->text().length() != 0))
  {
    groupCheck.prepare( "SELECT grp_id "
               "  FROM grp "
               " WHERE (UPPER(grp_name)=UPPER(:grp_name));" );
    groupCheck.bindValue(":grp_name", _name->text());
    groupCheck.exec();
    if (groupCheck.first())
    {
      XSqlQuery groupet;
      groupet.exec("ROLLBACK;");
      groupet.exec("BEGIN;");
      _grpid = groupCheck.value("grp_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
  }
}

void group::sSave()
{
  XSqlQuery groupSave;
  _name->setText(_name->text().trimmed().toUpper());

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_name->text().trimmed().isEmpty(), _name,
                          tr("You must enter a valid Name for this Group "
                             "before continuing"))
     ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Group"), errors))
    return;

  groupSave.prepare( "UPDATE grp "
             "   SET grp_name=:grp_name,"
             "       grp_descrip=:grp_descrip "
             " WHERE(grp_id=:grp_id);" );
  groupSave.bindValue(":grp_id", _grpid);
  groupSave.bindValue(":grp_name", _name->text());
  groupSave.bindValue(":grp_descrip", _description->text().trimmed());
  groupSave.exec();
  if (groupSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, groupSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  groupSave.exec("COMMIT;");
  _trapClose = false;
  _mode = cEdit;

  done(_grpid);
}

void group::sModuleSelected(const QString &pModule)
{
  XSqlQuery avail;
  avail.prepare( "SELECT priv_id, priv_name, priv_descrip "
                 "FROM priv "
                 "WHERE ((priv_module=:priv_module) "
                 "   AND (priv_id NOT IN (SELECT grppriv_priv_id"
                 "                        FROM grppriv"
                 "                        WHERE (grppriv_grp_id=:grpid)"
                 "                       ))) "
                 "ORDER BY priv_name;" );
  avail.bindValue(":priv_module", pModule);
  avail.bindValue(":grpid", _grpid);
  avail.exec();
  _available->populate(avail);
  if (avail.lastError().type() != QSqlError::NoError)
  {
    systemError(this, avail.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery grppriv;
  grppriv.prepare( "SELECT priv_id, priv_name, priv_descrip "
                   "FROM priv, grppriv "
                   "WHERE ((grppriv_priv_id=priv_id)"
                   "   AND (grppriv_grp_id=:grp_id)"
                   "   AND (priv_module=:priv_module))"
                   "ORDER BY priv_name;" );
  grppriv.bindValue(":grp_id", _grpid);
  grppriv.bindValue(":priv_module", _module->currentText());
  grppriv.exec();
  _granted->populate(grppriv);
  if (grppriv.lastError().type() != QSqlError::NoError)
  {
    systemError(this, grppriv.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void group::sAdd()
{
  XSqlQuery groupAdd;
  groupAdd.prepare("SELECT grantPrivGroup(:grp_id, :priv_id) AS result;");
  groupAdd.bindValue(":grp_id", _grpid);
  groupAdd.bindValue(":priv_id", _available->id());
  groupAdd.exec();
  if (groupAdd.first())
  {
    int result = groupAdd.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("grantPrivGroup", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (groupAdd.lastError().type() != QSqlError::NoError)
  {
    systemError(this, groupAdd.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
  _trapClose = true;
}

void group::sAddAll()
{
  XSqlQuery groupAddAll;
  groupAddAll.prepare("SELECT grantAllModulePrivGroup(:grp_id, :module) AS result;");
  groupAddAll.bindValue(":grp_id", _grpid);
  groupAddAll.bindValue(":module", _module->currentText());
  groupAddAll.exec();
  if (groupAddAll.first())
  {
    int result = groupAddAll.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("grantAllModulePrivGroup", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (groupAddAll.lastError().type() != QSqlError::NoError)
  {
    systemError(this, groupAddAll.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
  _trapClose = true;
}

void group::sRevoke()
{
  XSqlQuery groupRevoke;
  groupRevoke.prepare("SELECT revokePrivGroup(:grp_id, :priv_id) AS result;");
  groupRevoke.bindValue(":grp_id", _grpid);
  groupRevoke.bindValue(":priv_id", _granted->id());
  groupRevoke.exec();
  if (groupRevoke.first())
  {
    int result = groupRevoke.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("revokePrivGroup", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (groupRevoke.lastError().type() != QSqlError::NoError)
  {
    systemError(this, groupRevoke.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
  _trapClose = true;
}

void group::sRevokeAll()
{
  XSqlQuery groupRevokeAll;
  groupRevokeAll.prepare("SELECT revokeAllModulePrivGroup(:grp_id, :module) AS result;");
  groupRevokeAll.bindValue(":grp_id", _grpid);
  groupRevokeAll.bindValue(":module", _module->currentText());
  groupRevokeAll.exec();
  if (groupRevokeAll.first())
  {
    int result = groupRevokeAll.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("revokeAllModulePrivGroup", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (groupRevokeAll.lastError().type() != QSqlError::NoError)
  {
    systemError(this, groupRevokeAll.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
  _trapClose = true;
}

void group::populate()
{
  XSqlQuery grouppopulate;
  grouppopulate.prepare( "SELECT grp_name, grp_descrip "
             "  FROM grp "
             " WHERE(grp_id=:grp_id);" );
  grouppopulate.bindValue(":grp_id", _grpid);
  grouppopulate.exec();
  if (grouppopulate.first())
  {
    _name->setText(grouppopulate.value("grp_name"));
    _description->setText(grouppopulate.value("grp_descrip"));

    grouppopulate.prepare( "SELECT priv_module "
               "FROM grppriv, priv "
               "WHERE ( (grppriv_priv_id=priv_id)"
               " AND (grppriv_id=:grp_id) ) "
               "ORDER BY priv_module "
               "LIMIT 1;" );
    grouppopulate.bindValue(":grp_id", _grpid);
    grouppopulate.exec();
    if (grouppopulate.first())
    {
      for (int counter = 0; counter < _module->count(); counter++)
      {
        if (_module->itemText(counter) == grouppopulate.value("priv_module").toString())
        {
          _module->setCurrentIndex(counter);
          sModuleSelected(_module->itemText(counter));
        }
      }
    }
    else
    {
      _module->setCurrentIndex(0);
      sModuleSelected(_module->itemText(0));
    }
  }
  else if (grouppopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, grouppopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
