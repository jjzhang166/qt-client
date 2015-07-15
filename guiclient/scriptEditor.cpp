/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptEditor.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QScriptEngine>
#include <QSettings>
#include <QTextStream>
#include <QVariant>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "jsHighlighter.h"
#include "package.h"
#include "storedProcErrorLookup.h"

#define DEBUG false

static QString lastSaveDir = QString();

scriptEditor::scriptEditor(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);
  setWindowModality(Qt::WindowModal);

  connect(_export,          SIGNAL(clicked()), this, SLOT(sSaveFile()));
  connect(_find,            SIGNAL(clicked()), this, SLOT(sFind()));
  connect(_findText,SIGNAL(editingFinished()), this, SLOT(sFindSignal()));
  connect(_import,          SIGNAL(clicked()), this, SLOT(sImport()));
  connect(_line,    SIGNAL(editingFinished()), this, SLOT(sGoto()));
  connect(_save,            SIGNAL(clicked()), this, SLOT(sSave()));

  _highlighter = new JSHighlighter(_source->document());
  _document = _source->document();
  _document->setDefaultFont(QFont("Courier"));

  _package->populate("SELECT pkghead_id, pkghead_name, pkghead_name "
                     "FROM   pkghead "
                     "ORDER BY pkghead_name;");

  connect(_document, SIGNAL(blockCountChanged(int)), this, SLOT(sBlockCountChanged(int)));
  connect(_document, SIGNAL(modificationChanged(bool)), this, SLOT(setWindowModified(bool)));
  connect(_source, SIGNAL(cursorPositionChanged()), this, SLOT(sPositionChanged()));

  _package->setEnabled(package::userHasPriv(cEdit));

  _scriptid      = -1;
  _pkgheadidOrig = -1;
  _findCnt = 0;
  
  QSettings settings("xTuple.com", "scriptEditor");
  lastSaveDir = settings.value("LastDirectory").toString();
}

scriptEditor::~scriptEditor()
{
  QSettings settings("xTuple.com", "scriptEditor");
  settings.setValue("LastDirectory",lastSaveDir);
  // no need to delete child widgets, Qt does it all for us
}

void scriptEditor::languageChange()
{
  retranslateUi(this);
}

void scriptEditor::closeEvent(QCloseEvent *event)
{
  if (_document->isModified())
  {
    switch (QMessageBox::question(this, tr("Save Changes?"),
                                  tr("Do you want to save your changes?"),
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
    {
      case QMessageBox::Yes:
        if (sSave())
          event->accept();
        else
          event->ignore();
        break;
      case QMessageBox::No:
        event->accept();
        break;
      case QMessageBox::Cancel:
      default:
        event->ignore();
        break;
    }
  }
  else
    event->accept();
}

enum SetResponse scriptEditor::set(const ParameterList &pParams)
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
  param = pParams.value("script_id", &valid);
  if (valid)
  {
    _scriptid = param.toInt();
    populate();
  }
  
  param = pParams.value("script_name", &valid);
  if (valid)
  {
    _name->setText(param.toString());
    _name->setEnabled(false);
  }

  return NoError;
}

void scriptEditor::setMode(const int pmode)
{
  if (DEBUG)
    qDebug("scriptEditor::setMode(%d)", pmode);
  switch (pmode)
  {
    case cNew:
    case cEdit:
      if (DEBUG) qDebug("scriptEditor::setMode(%d) case new/edit", pmode);
      _name->setEnabled(true);
      _order->setEnabled(true);
      _notes->setReadOnly(false);
      _source->setReadOnly(false);
      _enabled->setEnabled(true);
      _save->show();
      if (pmode == cNew)
        _name->setFocus();
      else
        _save->setFocus();
      break;

    case cView:
    default:
      if (DEBUG) qDebug("scriptEditor::setMode(%d) case view/default", pmode);
      _name->setEnabled(false);
      _order->setEnabled(false);
      _notes->setReadOnly(true);
      _source->setReadOnly(true);
      _enabled->setEnabled(false);
      _save->hide();
      _import->setEnabled(false);
      _package->setEnabled(false);
      _close->setFocus();
  };

  _mode = pmode;
}

bool scriptEditor::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_name->text().trimmed().isEmpty(), _name,
                          tr("<p>You must enter a valid name for this Script."))
  ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Account"), errors))
    return false;

  QScriptEngine engine;
  if (!engine.canEvaluate(_source->toPlainText()) || _source->toPlainText().length() == 0)
  {
    if (QMessageBox::question(this, windowTitle(),
                          tr("<p>The script appears incomplete are you sure you want to save?"),
                             QMessageBox::Yes,
                             QMessageBox::No  | QMessageBox::Default) != QMessageBox::Yes)
    return false;
  }

  QMessageBox save;
  save.setText("How do you want to save your changes?");
  QPushButton *cancel = save.addButton(QMessageBox::Cancel);
  QPushButton *db     = save.addButton(tr("Database only"),    QMessageBox::AcceptRole);
  QPushButton *file   = save.addButton(tr("File only"),        QMessageBox::AcceptRole);
  QPushButton *both   = save.addButton(tr("Database and File"),QMessageBox::AcceptRole);

  QString _importFileName;      // TODO: make this a private class member?
  save.setDefaultButton(_importFileName.isEmpty() ? db : both);
  save.setEscapeButton((QAbstractButton*)cancel);

  save.exec();
  if (save.clickedButton() == (QAbstractButton*)db && sSaveToDB())
  {
    close();
    return true;
  }
  else if (save.clickedButton() == (QAbstractButton*)file && sSaveFile())
  {
    close();
    return true;
  }
  else if (save.clickedButton() == (QAbstractButton*)both
           && sSaveFile() && sSaveToDB())
  {
    close();
    return true;
  }
  else if (save.clickedButton() == (QAbstractButton*)cancel)
    return false;
  else
  {
    qWarning("scriptEditor::sSave() bug - unknown button clicked");
    return false;
  }

  return false;
}
  
bool scriptEditor::sSaveToDB()
{
  XSqlQuery saveq;
  if (_mode == cNew)
    saveq.prepare( "INSERT INTO script "
               "(script_id, script_name, script_notes, script_order, script_enabled, script_source) "
               "VALUES "
               "(DEFAULT, :script_name, :script_notes, :script_order, :script_enabled, :script_source) "
               "RETURNING script_id;" );

  else if (_mode == cEdit)
    saveq.prepare( "UPDATE script "
               "SET script_name=:script_name, script_notes=:script_notes,"
               "    script_order=:script_order, script_enabled=:script_enabled,"
               "    script_source=:script_source "
               "WHERE (script_id=:script_id) "
               "RETURNING script_id;" );

  saveq.bindValue(":script_id", _scriptid);
  saveq.bindValue(":script_name", _name->text());
  saveq.bindValue(":script_order", _order->value());
  saveq.bindValue(":script_enabled", QVariant(_enabled->isChecked()));
  saveq.bindValue(":script_source", _source->toPlainText());
  saveq.bindValue(":script_notes", _notes->text());

  saveq.exec();
  if (saveq.first())
    _scriptid = saveq.value("script_id").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving"),
                                saveq, __FILE__, __LINE__))
    return false;
  else
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving"),
                         tr("The script was not saved properly"),
                         __FILE__, __LINE__);
    return false;
  }

  _document->setModified(false);
  if (_package->id() != _pkgheadidOrig &&
      QMessageBox::question(this, tr("Move to different package?"),
                            tr("Do you want to move this script "
                               "to the %1 package?").arg(_package->code()),
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
  {
    XSqlQuery moveq;
    moveq.prepare("SELECT movescript(:scriptid, :oldpkgid,"
                  "                  :newpkgid) AS result;");
    moveq.bindValue(":scriptid", _scriptid);
    moveq.bindValue(":oldpkgid", _pkgheadidOrig);
    moveq.bindValue(":newpkgid", _package->id());
    moveq.exec();
    if (moveq.first())
    {
      int result = moveq.value("result").toInt();
      if (result >= 0)
        _pkgheadidOrig = _package->id();
      else
        ErrorReporter::error(QtCriticalMsg, this, tr("Saved but Not Moved"),
                             tr("<p>The script was saved to its original location but "
                                "could not be moved."),
                             storedProcErrorLookup("movescript", result), __FILE__, __LINE__);
    }
    else // don't return immediately after reporting this error
      ErrorReporter::error(QtCriticalMsg, this, tr("Saved but Not Moved"),
                           tr("<p>The script was saved to its original location but "
                              "could not be moved."),
                           moveq, __FILE__, __LINE__);
  }

  setMode(cEdit);
  return true;
}

void scriptEditor::populate()
{
  XSqlQuery getq;
  getq.prepare( "SELECT script.*, COALESCE(pkghead_id, -1) AS pkghead_id, "
            "        COALESCE(pkghead_indev,true) AS editable "
      	     "  FROM script, pg_class, pg_namespace "
             "  LEFT OUTER JOIN pkghead ON (nspname=pkghead_name) "
             " WHERE ((script.tableoid=pg_class.oid)"
             "    AND (relnamespace=pg_namespace.oid) "
             "    AND (script_id=:script_id));" );
  getq.bindValue(":script_id", _scriptid);
  getq.exec();
  if (getq.first())
  {
    _name->setText(getq.value("script_name").toString());
    _order->setValue(getq.value("script_order").toInt());
    _enabled->setChecked(getq.value("script_enabled").toBool());
    _source->setText(getq.value("script_source").toString());
    _notes->setText(getq.value("script_notes").toString());
    _pkgheadidOrig = getq.value("pkghead_id").toInt();
    _package->setId(_pkgheadidOrig);

    setWindowTitle("[*]" + tr("Script Editor - %1").arg(_name->text()));
    if (DEBUG)
      qDebug("scriptEditor::populate() editable = %d",
             getq.value("editable").toBool());
    if (!getq.value("editable").toBool())
      setMode(cView);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Script"),
                                getq, __FILE__, __LINE__))
    return;
}

void scriptEditor::sImport()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                  lastSaveDir, tr("Script (*.script *.js)"));
  if(filename.isNull())
    return;

  QFile file(filename);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QMessageBox::critical(this, tr("Could not import file"), file.errorString());
    return;
  }
  QTextStream ts(&file);
  _source->setText(ts.readAll());
  file.close();
  
  QFileInfo fi(filename);
  lastSaveDir = fi.absolutePath();
}

bool scriptEditor::sSaveFile()
{
  QString filename = QFileDialog::getSaveFileName(this, tr("Save Script File"),
                                                  lastSaveDir +
                                                  QDir::separator() +
                                                  _name->text().trimmed() +
                                                  ".js",
                                                  tr("Script (*.script *.js)"));
  if(filename.isNull())
    return false;

  return saveFile(_source->toPlainText(), filename);
}

bool scriptEditor::saveFile(const QString &source,
                            QString &filename) // input/output
{
  QFileInfo fi(filename);
  if(fi.suffix().isEmpty())
    filename += ".js";

  QFile file(filename);
  if(!file.open(QIODevice::WriteOnly))
  {
    QMessageBox::critical(0, tr("Could not export file"), file.errorString());
    return false;
  }

  QTextStream ts(&file);
  ts.setCodec("UTF-8");
#ifdef Q_OS_WIN
  // bug #15457 Exporting script on windows does not write UTF BOM
  ts.setGenerateByteOrderMark(true);
#endif
  ts << source;
  file.close();

  lastSaveDir = fi.absolutePath();
  return true;
}

void scriptEditor::sGoto()
{
  QTextCursor cursor = QTextCursor(_document->findBlockByNumber(_line->value() - 1));
  _source->setTextCursor(cursor);
  _source->ensureCursorVisible();
  _source->setFocus();
}

/** Called from a timer set in sFindSignal. This function checks the _findCnt and if it's greater than 0 it
    will call sFind. This is done to prevent the calling of sFind twice under some circumstances.
 */
void scriptEditor::sFindDo()
{
  if(_findCnt > 0)
    sFind();
}

/** Called when _findText emits editingFinished. Increments count and triggers 1ms singleshot timer to call sFindDo.
    This part of a solution to prevent double-finding when the user edits a text and then clicks the find button
    which was causing sFind to be called twice.
 */
void scriptEditor::sFindSignal()
{
  _findCnt++;
  QTimer::singleShot(1, this, SLOT(sFindDo()));
}

/** Called by the find button and sFindDo function. It decrements a _findCnt value down to 0 so sFindDo can evaluate
    if it needs to call this function after an initial editingFinished signal from the _findText widget is emitted.
 */
void scriptEditor::sFind()
{
  _findCnt --;
  if(_findCnt < 0)
    _findCnt = 0;

  if (_findText->text().isEmpty())
    return;

  QTextCursor oldposition = _source->textCursor();
  bool found = _source->find(_findText->text());
  if (!found && ! oldposition.atStart())
  {
    int answer = QMessageBox::question(this, tr("Continue Search?"),
                                       tr("<p>'%1' was not found. "
                                          "Start search from the beginning?")
                                         .arg(_findText->text()),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::Yes);
    if (answer == QMessageBox::Yes)
    {
      QTextCursor newposition = oldposition;
      newposition.movePosition(QTextCursor::Start);
      _source->setTextCursor(newposition);
      found = _source->find(_findText->text());
    }
    else
      found = true;     // a lie to prevent a useless message
  }
  if (!found)
  {
    QMessageBox::information(this, tr("Not Found"),
                             tr("<p>'%1' was not found.")
                               .arg(_findText->text()));
    _source->setTextCursor(oldposition);
  }
  _source->setFocus();
}

void scriptEditor::sBlockCountChanged(const int p)
{
  _line->setMaximum(p);
}

void scriptEditor::sPositionChanged()
{
  _line->setValue(_source->textCursor().blockNumber() + 1);
}
