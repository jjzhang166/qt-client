/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "uiform.h"

#include <QBuffer>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QSqlError>
#include <QTextStream>
#include <QStatusBar>
#include <QVariant>
#include <QDesignerComponents>

#include "customCommand.h"
#include "package.h"
#include "scriptEditor.h"
#include "storedProcErrorLookup.h"
#include "xTupleDesigner.h"
#include "xuiloader.h"
#include "errorReporter.h"

uiform::uiform(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_cmdDelete,    SIGNAL(clicked()), this, SLOT(sCmdDelete()));
  connect(_cmdEdit,      SIGNAL(clicked()), this, SLOT(sCmdEdit()));
  connect(_cmdNew,       SIGNAL(clicked()), this, SLOT(sCmdNew()));
  connect(_edit,         SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_export,       SIGNAL(clicked()), this, SLOT(sExport()));
  connect(_import,       SIGNAL(clicked()), this, SLOT(sImport()));
  connect(_name,       SIGNAL(editingFinished()), this, SLOT(sFillList()));
  connect(_save,         SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_scriptDelete, SIGNAL(clicked()), this, SLOT(sScriptDelete()));
  connect(_scriptEdit,   SIGNAL(clicked()), this, SLOT(sScriptEdit()));
  connect(_scriptNew,    SIGNAL(clicked()), this, SLOT(sScriptNew()));

  _script->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft,  true, "script_name");
  _script->addColumn(tr("Description"),   -1, Qt::AlignLeft,  true, "script_notes");
  _script->addColumn(tr("Order"),  _ynColumn, Qt::AlignCenter,true, "script_order");
  _script->addColumn(tr("Enabled"),_ynColumn, Qt::AlignCenter,true, "script_enabled");
  _script->addColumn(tr("Package"),_ynColumn, Qt::AlignCenter,false,"nspname");

  _commands->addColumn(tr("Module"),_itemColumn, Qt::AlignCenter,true, "cmd_module");
  _commands->addColumn(tr("Menu Label"),     -1, Qt::AlignLeft,  true, "cmd_title");
  _commands->addColumn(tr("Package"), _ynColumn, Qt::AlignCenter,false,"nspname");

  _package->populate("SELECT pkghead_id, pkghead_name, pkghead_name "
                     "FROM   pkghead "
                     "ORDER BY pkghead_name;");
  _package->setEnabled(package::userHasPriv(cEdit));

  _uiformid      = -1;
  _changed       = false;
  _pkgheadidOrig = -1;
  _source        = QString::null;
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
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      setMode(cNew);
    else if (param.toString() == "edit")
      setMode(cEdit);
    else if (param.toString() == "view")
      setMode(cView);
  }

  // follow setMode because populate() might change it
  param = pParams.value("uiform_id", &valid);
  if (valid)
  {
    _uiformid = param.toInt();
    populate();
  }

  return NoError;
}

void uiform::setMode(const int pmode)
{
  switch (pmode)
  {
    case cNew:
    case cEdit:
      _name->setEnabled(true);
      _grade->setEnabled(true);
      _notes->setReadOnly(false);
      _import->setEnabled(true);
      _enabled->setEnabled(true);
      _close->setText(tr("&Cancel"));
      _save->show();
      break;

    case cView:
    default:
      _name->setEnabled(false);
      _grade->setEnabled(false);
      _notes->setReadOnly(true);
      _import->setEnabled(false);
      _enabled->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
      break;
  }
  _mode = pmode;
}

void uiform::close()
{
  if (cView != _mode && changed())
  {
    switch (QMessageBox::question(this, tr("Save first?"),
                      tr("The screen appears to have changed. Do you "
                         "want to save your changes?"),
                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                      QMessageBox::Yes))
    {
      case QMessageBox::Cancel:
        return;
      case QMessageBox::No:
        break;
      case QMessageBox::Yes:
      default:
        sSave();
        return;
    }
  }
  XWidget::close();
}

void uiform::sSave()
{
  XSqlQuery uiformSave;
  if (_name->text().isEmpty())
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

  if (_mode == cNew && _uiformid == -1)
  {
    uiformSave.exec("SELECT NEXTVAL('uiform_uiform_id_seq') AS _uiform_id");
    if (uiformSave.first())
      _uiformid = uiformSave.value("_uiform_id").toInt();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving UI Form Information"),
                                  uiformSave, __FILE__, __LINE__))
    {
      return;
    }

    uiformSave.prepare( "INSERT INTO uiform "
               "(uiform_id, uiform_name, uiform_notes, uiform_order, uiform_enabled, uiform_source) "
               "VALUES "
               "(:uiform_id, :uiform_name, :uiform_notes, :uiform_order, :uiform_enabled, :uiform_source);" );

  }
  else if (_mode != cView)
    uiformSave.prepare( "UPDATE uiform "
               "SET uiform_name=:uiform_name, uiform_notes=:uiform_notes,"
               "    uiform_order=:uiform_order, uiform_enabled=:uiform_enabled,"
               "    uiform_source=:uiform_source "
               "WHERE (uiform_id=:uiform_id);" );

  uiformSave.bindValue(":uiform_id", _uiformid);
  uiformSave.bindValue(":uiform_name", _name->text());
  uiformSave.bindValue(":uiform_order", _grade->value());
  uiformSave.bindValue(":uiform_enabled", QVariant(_enabled->isChecked()));
  uiformSave.bindValue(":uiform_source", _source);
  uiformSave.bindValue(":uiform_notes", _notes->text());

  uiformSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving UI Form Information"),
                                uiformSave, __FILE__, __LINE__))
  {
    return;
  }

  if (_package->id() != _pkgheadidOrig &&
      QMessageBox::question(this, tr("Move to different package?"),
                            tr("Do you want to move this screen "
                               "to the %1 package?").arg(_package->code()),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
  {
    uiformSave.prepare("SELECT moveuiform(:formid, :oldpkgid,"
              "                  :newpkgid) AS result;");
    uiformSave.bindValue(":formid",   _uiformid);
    uiformSave.bindValue(":oldpkgid", _pkgheadidOrig);
    uiformSave.bindValue(":newpkgid", _package->id());
    uiformSave.exec();
    if (uiformSave.first())
    {
      int result = uiformSave.value("result").toInt();
      if (result >= 0)
        _pkgheadidOrig = _package->id();
      else
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                             tr("%1: <p>The screen was saved to its original location but "
                                "could not be moved: %2")
                             .arg(windowTitle())
                             .arg(storedProcErrorLookup("moveuiform", result)),__FILE__,__LINE__);
      }
    }
    else if (uiformSave.lastError().type() != QSqlError::NoError)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                           tr("%1: <p>The screen was saved to its original location but "
                              "could not be moved: <pre>%2</pre>")
                           .arg(windowTitle())
                           .arg(uiformSave.lastError().databaseText()),__FILE__,__LINE__);
    }
  }

  _changed = false;
  close();
}

void uiform::populate()
{
  XSqlQuery uiformpopulate;
  uiformpopulate.prepare( "SELECT uiform.*, COALESCE(pkghead_id, -1) AS pkghead_id,"
             "       COALESCE(pkghead_indev,true) AS editable "
      	     "  FROM uiform, pg_class, pg_namespace "
             "  LEFT OUTER JOIN pkghead ON (nspname=pkghead_name) "
             " WHERE ((uiform.tableoid=pg_class.oid)"
             "   AND (relnamespace=pg_namespace.oid) "
             "   AND  (uiform_id=:uiform_id));" );
  uiformpopulate.bindValue(":uiform_id", _uiformid);
  uiformpopulate.exec();
  if (uiformpopulate.first())
  {
    _name->setText(uiformpopulate.value("uiform_name").toString());
    _grade->setValue(uiformpopulate.value("uiform_order").toInt());
    _enabled->setChecked(uiformpopulate.value("uiform_enabled").toBool());
    _source = uiformpopulate.value("uiform_source").toString();
    _notes->setText(uiformpopulate.value("uiform_notes").toString());
    _pkgheadidOrig = uiformpopulate.value("pkghead_id").toInt();
    _package->setId(_pkgheadidOrig);

    if (!uiformpopulate.value("editable").toBool())
      setMode(cView);

    sFillList();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving UI Form Information"),
                                uiformpopulate, __FILE__, __LINE__))
  {
    return;
  }
}

void uiform::sImport()
{
  QSettings settings("xTuple.com", "xTupleDesigner");
  QString path = settings.value("LastDirectory").toString();
  
  QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), path, tr("UI (*.ui)"));
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
  _changed = true;
  
  QFileInfo fi(filename);
  settings.setValue("LastDirectory", fi.path());
}

void uiform::sEdit()
{
  static bool xdinit = false;
  QWidget *ui;
  QSize size;
  if(!xdinit)
  {
    QDesignerComponents::initializeResources();
    xdinit = true;
  }
  xTupleDesigner *designer = new xTupleDesigner(this, "xTupleDesigner", Qt::Window);
  designer->setFormEnabled(_enabled->isChecked());
  designer->setFormId(_uiformid);
  designer->setOrder(_grade->value());
  if (_source.isEmpty())
    designer->setSource(new QFile(":newForm.ui"));
  else
  {
    designer->setSource(new QBuffer(new QByteArray(_source.toUtf8()), this));

    // Create the form independently, get size, then apply size to widget in designer
    QByteArray ba;
    ba.append(_source.toUtf8());
    QBuffer uiFile(&ba);
    XUiLoader loader;
    ui = loader.load(&uiFile);
    if (ui)
      size = ui->size();
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                           tr("%1: Could not load .ui (%2)")
                           .arg(windowTitle())
                           .arg(uiFile.errorString()),__FILE__,__LINE__);
      return;
    }
  }

  connect(designer, SIGNAL(formEnabledChanged(bool)),_enabled,SLOT(setChecked(bool)));
  connect(designer, SIGNAL(formIdChanged(int)),      this,    SLOT(setFormId(int)));
  connect(designer, SIGNAL(notesChanged(QString)),   _notes,  SLOT(setText(QString)));
  connect(designer, SIGNAL(sourceChanged(QString)),  this,    SLOT(setSource(QString)));

  omfgThis->handleNewWindow(designer);
  // Set to default UI size
  ui = designer->findChild<QWidget*>(_name->text());
  if (ui)
  {
    designer->resize(size);
    // Make adjustments for margins
    size.setHeight(size.height()+(designer->height()-ui->height()));
    size.setWidth(size.width()+(designer->width()-ui->width()));
    designer->resize(size);
  }
}

void uiform::sExport()
{
  QSettings settings("xTuple.com", "xTupleDesigner");
  QString path = settings.value("LastDirectory").toString();
  
  QString filename = QFileDialog::getSaveFileName( this, tr("Save File"), path, tr("UI (*.ui)"));
  if(filename.isNull())
    return;

  (void)saveFile(_source, filename);

  QFileInfo fi(filename);
  settings.setValue("LastDirectory", fi.path());
}

bool uiform::saveFile(const QString &source, QString &filename)
{
  QFileInfo fi(filename);
  if(fi.suffix().isEmpty())
    filename += ".ui";

  QFile file(filename);
  if(!file.open(QIODevice::WriteOnly))
  {
    QMessageBox::critical(0, tr("Could not export file"), file.errorString());
    return false;
  }

  QTextStream ts(&file);
  ts.setCodec("UTF-8");
  ts << source;
  file.close();

  return true;
}

void uiform::sScriptNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("script_name", _name->text());

  scriptEditor *newdlg = new scriptEditor(this, "", Qt::Window);
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg, Qt::ApplicationModal);
  connect(newdlg, SIGNAL(destroyed()), this, SLOT(sFillList()));
}

void uiform::sScriptEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("script_id", _script->id());

  scriptEditor *newdlg = new scriptEditor(this, "", Qt::Window);
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg, Qt::ApplicationModal);
  connect(newdlg, SIGNAL(destroyed()), this, SLOT(sFillList()));
}

void uiform::sScriptDelete()
{
  XSqlQuery uiformScriptDelete;
  if ( QMessageBox::warning(this, tr("Delete Script?"),
                            tr("<p>Are you sure that you want to completely "
			       "delete the selected script?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    uiformScriptDelete.prepare( "DELETE FROM script "
               "WHERE (script_id=:script_id);" );
    uiformScriptDelete.bindValue(":script_id", _script->id());
    uiformScriptDelete.exec();
  }

  sFillList();
}

void uiform::sCmdNew()
{
  ParameterList params;
  params.append("mode", "new");

  customCommand newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void uiform::sCmdEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmd_id", _commands->id());

  customCommand newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void uiform::sCmdDelete()
{
  XSqlQuery uiformCmdDelete;
  if ( QMessageBox::warning(this, tr("Delete Command?"),
                            tr("<p>Are you sure that you want to completely "
			       "delete the selected command?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    uiformCmdDelete.prepare("BEGIN; "
              "DELETE FROM cmdarg WHERE (cmdarg_cmd_id=:cmd_id); "
              "DELETE FROM cmd WHERE (cmd_id=:cmd_id); "
              "SELECT updateCustomPrivs(); "
              "COMMIT; ");
    uiformCmdDelete.bindValue(":cmd_id", _commands->id());
    if(uiformCmdDelete.exec())
      sFillList();
    else
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Selected Command"),
                         uiformCmdDelete, __FILE__, __LINE__);
  }
}

void uiform::sFillList()
{
  XSqlQuery uiformFillList;
  uiformFillList.prepare( "SELECT script_id, script_name, script_notes,"
             "       script_order, script_enabled,"
             "       CASE WHEN nspname = 'public' THEN ''"
             "            ELSE nspname END AS nspname"
             "  FROM script, pg_class, pg_namespace"
             " WHERE ((script.tableoid=pg_class.oid)"
             "   AND  (relnamespace=pg_namespace.oid)"
             "   AND  (script_name=:name))"
             " ORDER BY script_name, script_order, script_id;" );
  uiformFillList.bindValue(":name", _name->text());
  uiformFillList.exec();
  _script->populate(uiformFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving UI Form Information"),
                                uiformFillList, __FILE__, __LINE__))
  {
    return;
  }

  uiformFillList.prepare("SELECT DISTINCT cmd_id, cmd_module, cmd_title,"
            "       CASE WHEN nspname = 'public' THEN ''"
            "            ELSE nspname END AS nspname"
            "  FROM cmd JOIN cmdarg ON (cmdarg_cmd_id=cmd_id), pg_class, pg_namespace"
            "  WHERE((cmd.tableoid=pg_class.oid)"
            "    AND (relnamespace=pg_namespace.oid)"
            "    AND (cmd_module IN ('Products','Inventory','Schedule','Purchase', "
            "                        'Manufacture','CRM','Sales','Accounting','System'))"
            "    AND (cmdarg_arg='uiform='||:name)) "
            " ORDER BY cmd_module, cmd_title;");
  uiformFillList.bindValue(":name", _name->text());
  uiformFillList.exec();
  _commands->populate(uiformFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving UI Form Information"),
                                uiformFillList, __FILE__, __LINE__))
  {
    return;
  }
}

void uiform::setFormId(int p)
{
  _uiformid = p;
  _changed = true;
}

void uiform::setSource(QString p)
{
  _source = p;
  _changed = true;
}

bool uiform::changed()
{
  return _changed;
}
