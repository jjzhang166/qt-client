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

#include "user.h"
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
  _assigned->addColumn(tr("Username"), 100, Qt::AlignLeft, true, "usr_username");
  _assigned->addColumn(tr("Proper Name"), -1, Qt::AlignLeft, true, "usr_propername");

  groupgroup.exec( "SELECT DISTINCT priv_module "
          "FROM priv "
          "ORDER BY priv_module;" );
  for (int i = 0; groupgroup.next(); i++)
    _module->append(i, groupgroup.value("priv_module").toString());

  connect(_assigned, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu *, QTreeWidgetItem *)));

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

void group::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  QAction *menuItem;

  menuItem = pMenu->addAction("Maintain User...", this, SLOT(sEditUser()));
  if (!_privileges->check("MaintainUsers"))
    menuItem->setEnabled(false);
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
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Group Information"),
                                    groupet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }

      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

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
        rollback.exec();
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Group Information"),
                             groupet, __FILE__, __LINE__);
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
                          tr("You must enter a valid Name for this Role "
                             "before continuing"))
     ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Role"), errors))
    return;

  groupSave.prepare( "UPDATE grp "
             "   SET grp_name=:grp_name,"
             "       grp_descrip=:grp_descrip "
             " WHERE(grp_id=:grp_id);" );
  groupSave.bindValue(":grp_id", _grpid);
  groupSave.bindValue(":grp_name", _name->text());
  groupSave.bindValue(":grp_descrip", _description->text().trimmed());
  groupSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Group"),
                                groupSave, __FILE__, __LINE__))
  {
    return;
  }

  groupSave.exec("COMMIT;");
  _trapClose = false;
  _mode = cEdit;

  done(_grpid);
}

void group::sEditUser()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("username", _assigned->selectedItems().first()->text(0));

  user newdlg(this);
  newdlg.set(params);

  newdlg.exec();
  populateAssigned();
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
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Group Information"),
                                avail, __FILE__, __LINE__))
  {
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
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Group Information"),
                                grppriv, __FILE__, __LINE__))
  {
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
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Granting Privilege"),
                             storedProcErrorLookup("grantPrivGroup", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Granting Privilege "),
                                groupAdd, __FILE__, __LINE__))
  {
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
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Granting All Privileges"),
                             storedProcErrorLookup("grantAllModulePrivGroup", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Granting All Privileges"),
                                groupAddAll, __FILE__, __LINE__))
  {
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
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Revoking Privilege"),
                             storedProcErrorLookup("revokePrivGroup", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Revoking Privilege"),
                                groupRevoke, __FILE__, __LINE__))
  {
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
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Revoking All Privileges"),
                             storedProcErrorLookup("revokeAllModulePrivGroup", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Revoking All Privileges"),
                                groupRevokeAll, __FILE__, __LINE__))
  {
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
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Role Information"),
                                grouppopulate, __FILE__, __LINE__))
  {
    return;
  }
  
  populateAssigned();
}

void group::populateAssigned()
{

  XSqlQuery assignedpopulate;
  assignedpopulate.prepare( "SELECT usr_username, usr_propername FROM usr "
                            " JOIN usrgrp ON (usrgrp_username=usr_username) "
                            " WHERE (usrgrp_grp_id=:grp_id);" );
  assignedpopulate.bindValue(":grp_id", _grpid);
  assignedpopulate.exec();
  if (assignedpopulate.lastError().type() != QSqlError::NoError)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("User Role Assignment"),
                         assignedpopulate.lastError(), __FILE__, __LINE__);
    return;
  }
  _assigned->populate(assignedpopulate, false);
}
