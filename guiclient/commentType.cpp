/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "commentType.h"

#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

#include "storedProcErrorLookup.h"

commentType::commentType(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  XSqlQuery commentcommentType;
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_addAll, SIGNAL(clicked()), this, SLOT(sAddAll()));
  connect(_revoke, SIGNAL(clicked()), this, SLOT(sRevoke()));
  connect(_revokeAll, SIGNAL(clicked()), this, SLOT(sRevokeAll()));
  connect(_module, SIGNAL(activated(const QString&)), this, SLOT(sModuleSelected(const QString&)));
  connect(_granted, SIGNAL(itemSelected(int)), this, SLOT(sRevoke()));
  connect(_available, SIGNAL(itemSelected(int)), this, SLOT(sAdd()));

  _available->addColumn("Available Sources", -1, Qt::AlignLeft);
  _granted->addColumn("Granted Sources", -1, Qt::AlignLeft);
  
  commentcommentType.exec( "SELECT DISTINCT source_module "
          "FROM source "
          "ORDER BY source_module;" );
  for (int i = 0; commentcommentType.next(); i++)
    _module->insertItem(i, commentcommentType.value("source_module").toString());
}

commentType::~commentType()
{
  // no need to delete child widgets, Qt does it all for us
}

void commentType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse commentType::set(const ParameterList &pParams)
{
  XSqlQuery commentet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cmnttype_id", &valid);
  if (valid)
  {
    _cmnttypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      commentet.exec("SELECT NEXTVAL('cmnttype_cmnttype_id_seq') AS cmnttype_id");
      if (commentet.first())
        _cmnttypeid = commentet.value("cmnttype_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
      }

      _module->setCurrentIndex(0);
      sModuleSelected(_module->itemText(0));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(false);
      _description->setEnabled(false);
      _editable->setEnabled(false);
      _order->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void commentType::sSave()
{
  XSqlQuery commentSave;
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Cannot Save Comment Type"),
                              tr("You must enter a valid Comment Type before saving this Item Type.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    commentSave.prepare( "INSERT INTO cmnttype "
               "( cmnttype_id, cmnttype_name, cmnttype_descrip, cmnttype_editable, cmnttype_order ) "
               "VALUES "
               "( :cmnttype_id, :cmnttype_name, :cmnttype_descrip, :cmnttype_editable, :cmnttype_order );" );
  }
  else if (_mode == cEdit)
    commentSave.prepare( "UPDATE cmnttype "
               "SET cmnttype_name=:cmnttype_name,"
               "    cmnttype_descrip=:cmnttype_descrip,"
               "    cmnttype_editable=:cmnttype_editable,"
               "    cmnttype_order=:cmnttype_order "
               "WHERE (cmnttype_id=:cmnttype_id);" );

  commentSave.bindValue(":cmnttype_id", _cmnttypeid);
  commentSave.bindValue(":cmnttype_name", _name->text());
  commentSave.bindValue(":cmnttype_descrip", _description->text());
  commentSave.bindValue(":cmnttype_editable", _editable->isChecked());
  commentSave.bindValue(":cmnttype_order", _order->value());
  commentSave.exec();

  done(_cmnttypeid);
}

void commentType::sCheck()
{
  XSqlQuery commentCheck;
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    commentCheck.prepare( "SELECT cmnttype_id "
               "FROM cmnttype "
               "WHERE (UPPER(cmnttype_name)=UPPER(:cmnttype_name));" );
    commentCheck.bindValue(":cmnttype_name", _name->text());
    commentCheck.exec();
    if (commentCheck.first())
    {
      _cmnttypeid = commentCheck.value("cmnttype_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
  }
}

void commentType::populate()
{
  XSqlQuery commentpopulate;
  commentpopulate.prepare( "SELECT * "
             "FROM cmnttype "
             "WHERE (cmnttype_id=:cmnttype_id);" );
  commentpopulate.bindValue(":cmnttype_id", _cmnttypeid);
  commentpopulate.exec();
  if (commentpopulate.first())
  {
    _name->setText(commentpopulate.value("cmnttype_name"));
    _description->setText(commentpopulate.value("cmnttype_descrip"));
    _editable->setChecked(commentpopulate.value("cmnttype_editable").toBool());
    _order->setValue(commentpopulate.value("cmnttype_order").toInt());
    if(commentpopulate.value("cmnttype_sys").toBool())
    {
      _name->setEnabled(false);
      if(_name->text() == "ChangeLog")
        _editable->setEnabled(false);
    }
    
    commentpopulate.prepare( "SELECT source_module "
               "FROM cmnttypesource, source "
               "WHERE ( (cmnttypesource_source_id=source_id)"
               " AND (cmnttypesource_cmnttype_id=:cmnttype_id) ) "
               "ORDER BY source_module "
               "LIMIT 1;" );
    commentpopulate.bindValue(":cmnttype_id", _cmnttypeid);
    commentpopulate.exec();
    if (commentpopulate.first())
    {
      for (int counter = 0; counter < _module->count(); counter++)
      {
        if (_module->itemText(counter) == commentpopulate.value("source_module").toString())
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
}

void commentType::sModuleSelected(const QString &pModule)
{
  XTreeWidgetItem *granted = NULL;
  XTreeWidgetItem *available = NULL;

  _available->clear();
  _granted->clear();

  XSqlQuery sources;
  sources.prepare( "SELECT source_id, source_descrip "
                   "FROM source "
                   "WHERE (source_module=:source_module) "
                   "ORDER BY source_descrip;" );
  sources.bindValue(":source_module", pModule);
  sources.exec();
  if (sources.first())
  {
    granted = NULL;
    available = NULL;

//  Insert each source into either the available or granted list
    XSqlQuery cmnttypesource;
    cmnttypesource.prepare( "SELECT source_id "
                            "FROM source, cmnttypesource "
                            "WHERE ( (cmnttypesource_source_id=source_id)"
                            " AND (cmnttypesource_cmnttype_id=:cmnttype_id)"
                            " AND (source_module=:source_module) );" );
    cmnttypesource.bindValue(":cmnttype_id", _cmnttypeid);
    cmnttypesource.bindValue(":source_module", _module->currentText());
    cmnttypesource.exec();

    do
    {
      if (cmnttypesource.findFirst("source_id", sources.value("source_id").toInt()) == -1)
        available = new XTreeWidgetItem(_available, available, sources.value("source_id").toInt(), sources.value("source_descrip"));
      else
      {
        granted = new XTreeWidgetItem(_granted, granted, sources.value("source_id").toInt(), sources.value("source_descrip"));
      }
    }
    while (sources.next());
  }
}

void commentType::sAdd()
{
  XSqlQuery commentAdd;
  commentAdd.prepare("SELECT grantCmnttypeSource(:cmnttype_id, :source_id) AS result;");
  commentAdd.bindValue(":cmnttype_id", _cmnttypeid);
  commentAdd.bindValue(":source_id", _available->id());
  commentAdd.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (commentAdd.lastError().type() != QSqlError::NoError)
  {
    systemError(this, commentAdd.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

void commentType::sAddAll()
{
  XSqlQuery commentAddAll;
  commentAddAll.prepare("SELECT grantAllModuleCmnttypeSource(:cmnttype_id, :module) AS result;");
  commentAddAll.bindValue(":cmnttype_id", _cmnttypeid);
  commentAddAll.bindValue(":module", _module->currentText());
  commentAddAll.exec();
  if (commentAddAll.first())
  {
    int result = commentAddAll.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("grantAllModuleCmnttypeSource", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (commentAddAll.lastError().type() != QSqlError::NoError)
  {
    systemError(this, commentAddAll.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

void commentType::sRevoke()
{
  XSqlQuery commentRevoke;
  commentRevoke.prepare("SELECT revokeCmnttypeSource(:cmnttype_id, :source_id) AS result;");
  commentRevoke.bindValue(":cmnttype_id", _cmnttypeid);
  commentRevoke.bindValue(":source_id", _granted->id());
  commentRevoke.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (commentRevoke.lastError().type() != QSqlError::NoError)
  {
    systemError(this, commentRevoke.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

void commentType::sRevokeAll()
{
  XSqlQuery commentRevokeAll;
  commentRevokeAll.prepare("SELECT revokeAllModuleCmnttypeSource(:cmnttype_id, :module) AS result;");
  commentRevokeAll.bindValue(":cmnttype_id", _cmnttypeid);
  commentRevokeAll.bindValue(":module", _module->currentText());
  commentRevokeAll.exec();
  if (commentRevokeAll.first())
  {
    int result = commentRevokeAll.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("revokeAllModuleCmnttypeSource", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (commentRevokeAll.lastError().type() != QSqlError::NoError)
  {
    systemError(this, commentRevokeAll.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

