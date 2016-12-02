/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "package.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "mqlutil.h"
#include "errorReporter.h"
#include "storedProcErrorLookup.h"

#define DEBUG false

// TODO: XDialog should have a default implementation that returns false
bool package::userHasPriv(const int pMode)
{
  bool retval = false;
  bool canmaintain = false;

  if (pMode == cNew || pMode == cEdit)
  {
    XSqlQuery su;
    su.exec("SELECT isDBA() as rolsuper;");
    if (su.first())
      canmaintain = su.value("rolsuper").toBool();
    else if (ErrorReporter::error(QtCriticalMsg, 0, tr("Error Retrieving Privilege Information"),
                                  su, __FILE__, __LINE__))
    {
      return false;
    }
  }

  switch (pMode)
  {
    case cView:
      retval = _privileges->check("ViewPackages") || canmaintain;
      break;
    case cNew:
    case cEdit:
      retval = canmaintain;
      break;
    default:
      retval = false;
      break;
  }
  return retval;
}

// TODO: this code really belongs in XDialog
void package::setVisible(bool visible)
{
  if (! visible)
    XDialog::setVisible(false);

  else if (! userHasPriv(_mode))
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: Insufficient privileges to view this window ")
                         .arg(windowTitle()),__FILE__,__LINE__);
    reject();
  }
  else
    XDialog::setVisible(true);
}

package::package(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _rec->addColumn(tr("Type"),       -1, Qt::AlignLeft, true, "pkgitem_type");
  _rec->addColumn(tr("Name"),       -1, Qt::AlignLeft, true, "pkgitem_name");
  _rec->addColumn(tr("Description"),-1, Qt::AlignLeft, true, "pkgitem_descrip");

  _req->addColumn(tr("Package"),    -1, Qt::AlignLeft, true, "pkghead_name");
  _req->addColumn(tr("Description"),-1, Qt::AlignLeft, true, "pkghead_descrip");
  _req->addColumn(tr("Version"),    -1, Qt::AlignRight,true, "pkghead_version");

  _dep->addColumn(tr("Package"),    -1, Qt::AlignLeft, true, "pkghead_name");
  _dep->addColumn(tr("Description"),-1, Qt::AlignLeft, true, "pkghead_descrip");
  _dep->addColumn(tr("Version"),    -1, Qt::AlignRight,true, "pkghead_version");

  connect(_save,                  SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_showSystemDetails, SIGNAL(toggled(bool)), this, SLOT(populate()));

  _mode              = cNew;
  _pkgheadid         = -1;
  _priorEnabledState = false;
}

package::~package()
{
  // no need to delete child widgets, Qt does it all for us
}

void package::languageChange()
{
  retranslateUi(this);
}

enum SetResponse package::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("pkghead_id", &valid);
  if (valid)
  {
    _pkgheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setEnabled(false);
      _version->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(false);
      _version->setEnabled(false);
      _description->setEnabled(false);
      _developer->setEnabled(false);
      _notes->setEnabled(false);
      _enabled->setEnabled(false);
      _indev->setEnabled(false);

      _save->hide();
      _close->setText(tr("&Close"));
    }
  }

  return NoError;
}

void package::sCheck()
{
  XSqlQuery packageCheck;
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    packageCheck.prepare( "SELECT pkghead_id "
               "FROM pkghead "
               "WHERE (UPPER(pkghead_name)=UPPER(:pkghead_name));" );
    packageCheck.bindValue(":pkghead_code", _name->text());
    packageCheck.exec();
    if (packageCheck.first())
    {
      _pkgheadid = packageCheck.value("pkghead_id").toInt();
      _mode = cEdit;
      populate();
    }
  }
}

void package::sSave()
{
  XSqlQuery packageSave;
  if (_mode == cNew)
  {
    packageSave.exec("SELECT NEXTVAL('pkghead_pkghead_id_seq') AS _pkghead_id");
    if (packageSave.first())
      _pkgheadid = packageSave.value("_pkghead_id").toInt();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Package Information"),
                                  packageSave, __FILE__, __LINE__))
    {
      return;
    }

    packageSave.prepare( "INSERT INTO pkghead ("
               "  pkghead_id, pkghead_name, pkghead_descrip,"
               "  pkghead_version, pkghead_developer, pkghead_notes,"
               "  pkghead_indev"
               ") VALUES ("
               "  :pkghead_id, :pkghead_name, :pkghead_descrip, "
               "  :pkghead_version, :pkghead_developer, :pkghead_notes,"
               "  :pkghead_indev);");
  }
  else if (_mode == cEdit)
  {
    packageSave.prepare( "SELECT pkghead_id "
               "FROM pkghead "
               "WHERE ( (UPPER(pkghead_name)=UPPER(:pkghead_name))"
               " AND (pkghead_id<>:pkghead_id) );" );
    packageSave.bindValue(":pkghead_id",   _pkgheadid);
    packageSave.bindValue(":pkghead_name", _name->text());
    packageSave.exec();
    if (packageSave.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Package"),
                            tr("<p>You may not rename this Package to %1 as "
                               "this value is used by a different Package.") );
      return;
    }

    packageSave.prepare( "UPDATE pkghead "
               "SET pkghead_name=:pkghead_name,"
               "    pkghead_descrip=:pkghead_descrip,"
               "    pkghead_version=:pkghead_version,"
               "    pkghead_developer=:pkghead_developer,"
               "    pkghead_notes=:pkghead_notes,"
               "    pkghead_indev=:pkghead_indev "
               "WHERE (pkghead_id=:pkghead_id);" );
  }

  packageSave.bindValue(":pkghead_id",       _pkgheadid);
  packageSave.bindValue(":pkghead_name",     _name->text());
  packageSave.bindValue(":pkghead_descrip",  _description->text());
  packageSave.bindValue(":pkghead_version",  _version->text());
  packageSave.bindValue(":pkghead_developer",_developer->text());
  packageSave.bindValue(":pkghead_notes",    _notes->toPlainText());
  packageSave.bindValue(":pkghead_indev",    _indev->isChecked());
  packageSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Package Information"),
                                packageSave, __FILE__, __LINE__))
  {
    return;
  }

  if (DEBUG) qDebug("_enabled->isChecked: %d\tprior state: %d",
                    _enabled->isChecked(), _priorEnabledState);
  if (_enabled->isChecked() != _priorEnabledState)
  {
    XSqlQuery eq;
    QString funcname;
    if (_enabled->isChecked())
    {
      eq.prepare("SELECT enablePackage(:id) AS result;");
      funcname = "enablePackage";
    }
    else
    {
      eq.prepare("SELECT disablePackage(:id) AS result;");
      funcname = "disablePackage";
    }
    eq.bindValue(":id", _pkgheadid);
    eq.exec();
    if (eq.first())
    {
      int result = eq.value("result").toInt();
      if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Package Information"),
                               storedProcErrorLookup("funcname", result),
                               __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Package Information"),
                                  eq, __FILE__, __LINE__))
    {
      return;
    }
  }

  done(_pkgheadid);
}

void package::populate()
{
  XSqlQuery packagepopulate;
  if (DEBUG)    qDebug("package::populate() entered");

  packagepopulate.prepare("SELECT *, packageIsEnabled(pkghead_name) AS enabled "
            "FROM pkghead WHERE (pkghead_id=:pkghead_id);");
  packagepopulate.bindValue(":pkghead_id", _pkgheadid);
  packagepopulate.exec();
  if (packagepopulate.first())
  {
    if (DEBUG)    qDebug("package::populate() select pkghead succeeded");
    _name->setText(packagepopulate.value("pkghead_name").toString());
    _description->setText(packagepopulate.value("pkghead_descrip").toString());
    if (DEBUG)    qDebug("package::populate() select pkghead half done");
    _version->setText(packagepopulate.value("pkghead_version").toString());
    _developer->setText(packagepopulate.value("pkghead_developer").toString());
    _notes->setText(packagepopulate.value("pkghead_notes").toString());
    _enabled->setChecked(packagepopulate.value("enabled").toBool());
    _priorEnabledState = _enabled->isChecked();
    _indev->setChecked(packagepopulate.value("pkghead_indev").toBool());
    if (DEBUG)    qDebug("package::populate() select pkghead complete");
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Package Information"),
                                packagepopulate, __FILE__, __LINE__))
  {
    return;
  }

  ParameterList params;
  params.append("pkgname", _name->text());
  params.append("script",  tr("Script"));
  params.append("cmd",     tr("Custom Command"));
  params.append("function",tr("Stored Procedure"));
  params.append("trigger", tr("Trigger"));
  params.append("image",   tr("Image"));
  params.append("metasql", tr("MetaSQL"));
  params.append("priv",    tr("Privilege"));
  params.append("report",  tr("Report"));
  params.append("schema",  tr("Schema"));
  params.append("table",   tr("Table"));
  params.append("uiform",  tr("Screen"));
  params.append("view",    tr("View"));
  params.append("sequence",tr("Sequence"));
  params.append("index",   tr("Index"));
  if (_showSystemDetails->isChecked())
    params.append("showsystemdetails");

  MetaSQLQuery itemmql = mqlLoad("package", "items");
  packagepopulate = itemmql.toQuery(params);

  packagepopulate.exec();
  if (DEBUG)    qDebug("package::populate() select pkgitem exec'ed");
  _rec->populate(packagepopulate);
  if (DEBUG)    qDebug("package::populate() populate pkgitem done");
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Package Information"),
                                packagepopulate, __FILE__, __LINE__))
  {
    return;
  }

  // TODO: make this recursive?
  packagepopulate.prepare("SELECT pkghead.* "
            "FROM pkgdep, pkghead "
            "WHERE ((pkgdep_pkghead_id=pkghead_id)"
            "  AND  (pkgdep_parent_pkghead_id=:pkghead_id));");
  packagepopulate.bindValue(":pkghead_id", _pkgheadid);
  packagepopulate.exec();
  if (DEBUG)    qDebug("package::populate() select pkgdep exec'ed");
  _dep->populate(packagepopulate);
  if (DEBUG)    qDebug("package::populate() populate pkgdep done");
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Package Info"),
                           packagepopulate, __FILE__, __LINE__))
  {
    return;
  }

  // TODO: make this recursive?
  packagepopulate.prepare("SELECT * "
            "FROM pkgdep, pkghead "
            "WHERE ((pkgdep_parent_pkghead_id=pkghead_id)"
            "  AND  (pkgdep_pkghead_id=:pkghead_id));");
  packagepopulate.bindValue(":pkghead_id", _pkgheadid);
  packagepopulate.exec();
  if (DEBUG)    qDebug("package::populate() select pkgdep exec'ed");
  _req->populate(packagepopulate);
  if (DEBUG)    qDebug("package::populate() populate pkgdep done");
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Package Info"),
                           packagepopulate, __FILE__, __LINE__))
  {
    return;
  }

}
