/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scripttoolbox.h"

#include <QBoxLayout>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QIODevice>
#include <QLayout>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QSqlError>
#include <QStackedLayout>
#include <QTabWidget>
#include <QTextStream>
#include <QUrl>
#include <QWidget>
#include <qwebview.h>

#include <parameter.h>
#include <metasql.h>
#include <openreports.h>

#include "creditCard.h"
#include "creditcardprocessor.h"
#include "mqlutil.h"
#include "scriptquery.h"
#include "xdialog.h"
#include "xmainwindow.h"
#include "xtreewidget.h"
#include "xuiloader.h"

QWidget *ScriptToolbox::_lastWindow = 0;

ScriptToolbox::ScriptToolbox(QScriptEngine * engine)
  : QObject(engine)
{
  _engine = engine;
}

ScriptToolbox::~ScriptToolbox()
{
}

QObject * ScriptToolbox::executeQuery(const QString & query)
{
  ParameterList params;
  ScriptQuery * sq = new ScriptQuery(_engine);
  MetaSQLQuery mql(query);
  sq->setQuery(mql.toQuery(params));
  return sq;
}

QObject * ScriptToolbox::executeQuery(const QString & query, const ParameterList & params)
{
  ScriptQuery * sq = new ScriptQuery(_engine);
  MetaSQLQuery mql(query);
  sq->setQuery(mql.toQuery(params));
  return sq;
}

QObject * ScriptToolbox::executeDbQuery(const QString & group, const QString & name)
{
  ParameterList params;
  ScriptQuery * sq = new ScriptQuery(_engine);
  MetaSQLQuery mql = mqlLoad(group, name);
  sq->setQuery(mql.toQuery(params));
  return sq;
}

QObject * ScriptToolbox::executeDbQuery(const QString & group, const QString & name, const ParameterList & params)
{
  ScriptQuery * sq = new ScriptQuery(_engine);
  MetaSQLQuery mql = mqlLoad(group, name);
  sq->setQuery(mql.toQuery(params));
  return sq;
}

QObject * ScriptToolbox::executeBegin()
{
  ParameterList params;
  ScriptQuery * sq = new ScriptQuery(_engine);
  MetaSQLQuery mql("BEGIN;");
  sq->setQuery(mql.toQuery(params));
  return sq;
}

QObject * ScriptToolbox::executeCommit()
{
  ParameterList params;
  ScriptQuery * sq = new ScriptQuery(_engine);
  MetaSQLQuery mql("COMMIT;");
  sq->setQuery(mql.toQuery(params));
  return sq;
}

QObject * ScriptToolbox::executeRollback()
{
  ParameterList params;
  ScriptQuery * sq = new ScriptQuery(_engine);
  MetaSQLQuery mql("ROLLBACK;");
  sq->setQuery(mql.toQuery(params));
  return sq;
}

QObject * ScriptToolbox::qtyVal()
{
  QValidator * val = omfgThis->qtyVal();
  return val;
}

QObject * ScriptToolbox::TransQtyVal()
{
  QValidator * val = omfgThis->transQtyVal();
  return val;
}

QObject * ScriptToolbox::qtyPerVal()
{
  QValidator * val = omfgThis->qtyPerVal();
  return val;
}

QObject * ScriptToolbox::percentVal()
{
  QValidator * val = omfgThis->percentVal();
  return val;
}

QObject * ScriptToolbox::moneyVal()
{
  QValidator * val = omfgThis->moneyVal();
  return val;
}

QObject * ScriptToolbox::negMoneyVal()
{
  QValidator * val = omfgThis->negMoneyVal();
  return val;
}

QObject * ScriptToolbox::priceVal()
{
  QValidator * val = omfgThis->priceVal();
  return val;
}
QObject * ScriptToolbox::costVal()
{
  QValidator * val = omfgThis->costVal();
  return val;
}

QObject * ScriptToolbox::ratioVal()
{
  QValidator * val = omfgThis->ratioVal();
  return val;
}

QObject * ScriptToolbox::weightVal()
{
  QValidator * val = omfgThis->weightVal();
  return val;
}

QObject * ScriptToolbox::runTimeVal()
{
  QValidator * val = omfgThis->runTimeVal();
  return val;
}

QObject * ScriptToolbox::orderVal()
{
  QValidator * val = omfgThis->orderVal();
  return val;
}

QObject * ScriptToolbox::dayVal()
{
  QValidator * val = omfgThis->dayVal();
  return val;
}

QObject * ScriptToolbox::createGridLayout()
{
  QGridLayout * layout = new QGridLayout;
  return layout;
}

QObject * ScriptToolbox::widgetGetLayout(QWidget * w)
{
  QObject * p = w->parentWidget();
  while(p)
  {
    QList<QLayout*> list = p->findChildren<QLayout*>();
    for (int i = 0; i < list.size(); ++i)
    {
      if (list.at(i)->indexOf(w) != -1)
        return list.at(i);
    }
  }
  return NULL;
}

void ScriptToolbox::layoutGridAddLayout(QObject * parent, QObject * child, int row, int column, int alignment)
{
  QGridLayout * parentLayout = qobject_cast<QGridLayout*>(parent);
  QGridLayout * childLayout = qobject_cast<QGridLayout*>(child);

  if(parentLayout)
    parentLayout->addLayout(childLayout, row, column, (Qt::Alignment)alignment);
}

//TODO add an insert/add layout function for every insert/add widget function

void ScriptToolbox::layoutBoxInsertWidget(QObject * obj, int index, QWidget * widget, int stretch, int alignment)
{
  QBoxLayout * layout = qobject_cast<QBoxLayout*>(obj);
  if(layout && widget)
    layout->insertWidget(index, widget, stretch, (Qt::Alignment)alignment);
}

void ScriptToolbox::layoutGridAddWidget(QObject * obj, QWidget * widget, int row, int column, int alignment)
{
  QGridLayout * layout = qobject_cast<QGridLayout*>(obj);
  if(layout && widget)
    layout->addWidget(widget, row, column, (Qt::Alignment)alignment);
}

void ScriptToolbox::layoutGridAddWidget(QObject * obj, QWidget * widget, int fromRow, int fromColumn, int rowSpan, int columnSpan, int alignment)
{
  QGridLayout * layout = qobject_cast<QGridLayout*>(obj);
  if(layout && widget)
    layout->addWidget(widget, fromRow, fromColumn, rowSpan, columnSpan, (Qt::Alignment)alignment);
}

void ScriptToolbox::layoutStackedInsertWidget(QObject * obj, int index, QWidget * widget)
{
  QStackedLayout * layout = qobject_cast<QStackedLayout*>(obj);
  if(layout && widget)
    layout->insertWidget(index, widget);
}

QObject * ScriptToolbox::menuAddAction(QObject * menu, const QString & text, const bool enabled)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * act = 0;
  if(m)
  {
    act = m->addAction(text);
    act->setEnabled(enabled);
  }
  return act;
}

QObject * ScriptToolbox::menuAddMenu(QObject * menu, const QString & text, const QString & name)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QMenu * nm = 0;
  if(m)
  {
    nm = m->addMenu(text);
    if(!name.isEmpty())
      nm->setObjectName(name);
  }
  return nm;
}

int ScriptToolbox::tabCount(QWidget * tab)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  if(tw)
    return tw->count();
  return 0;
}

QObject * ScriptToolbox::menuAddSeparator(QObject * menu)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * na = 0;
  if(m)
    na = m->addSeparator();
  return na;
}

QObject * ScriptToolbox::menuInsertAction(QObject * menu, QObject * before, const QString & name, const bool enabled)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * ba = qobject_cast<QAction*>(before);
  QAction * na = new QAction(name,m);
  na->setEnabled(enabled);
  if(m && ba && na)
  {
    m->insertAction(ba, na);
    return na;
  }
  return 0;
}

QObject * ScriptToolbox::menuInsertMenu(QObject * menu, QObject * before, const QString & name)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * ba = qobject_cast<QAction*>(before);
  QMenu * nm = new QMenu(name);

  if (!ba && nm)
  {
    QMenu * bm =  qobject_cast<QMenu*>(before);
    if (bm)
      ba = bm->menuAction();
  }
  if (ba && nm)
    m->insertMenu(ba, nm);
  return nm;
}

QObject * ScriptToolbox::menuInsertSeparator(QObject * menu, QObject * before)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * ba = qobject_cast<QAction*>(before);
  if (!ba)
  {
    QMenu * bm = qobject_cast<QMenu*>(before);
    ba = bm->menuAction();
  }
  QAction * na = new QAction(m);
  if(m && ba && na)
  {
    na->setSeparator(true);
    m->insertAction(ba, na);
    return na;
  }
  return 0;
}

void ScriptToolbox::menuRemove(QObject * menu, QObject * action)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * act = qobject_cast<QAction*>(action);
  if (!act)
  {
    QMenu * ma = qobject_cast<QMenu*>(action);
    act = ma->menuAction();
  }
  if(m && act)
    m->removeAction(act);
}

int ScriptToolbox::menuActionCount(QObject * menu)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  if (m)
    return m->actions().count();
  return 0;
}

QWidget * ScriptToolbox::tabWidget(QWidget * tab, int idx)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  QWidget * w = 0;
  if(tw)
    w = tw->widget(idx);
  return w;
}

int ScriptToolbox::tabInsertTab(QWidget * tab, int idx, QWidget * page, const QString & text)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  int i = -1;
  if(tw)
    i = tw->insertTab(idx, page, text);
  return i;
}

int ScriptToolbox::tabTabIndex(QWidget * tab, QWidget * page)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  if(tw)
    return tw->indexOf(page);
  return 0;
}

void ScriptToolbox::tabRemoveTab(QWidget * tab, int idx)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  if(tw)
    tw->removeTab(idx);
}

void ScriptToolbox::tabSetTabEnabled(QWidget * tab, int idx, bool enable)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  if(tw)
    tw->setTabEnabled(idx, enable);
}

void ScriptToolbox::tabSetTabText(QWidget * tab, int idx, const QString & text)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  if(tw)
    tw->setTabText(idx, text);
}

QString ScriptToolbox::tabtabText(QWidget * tab, int idx)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  QString str;
  if(tw)
    str = tw->tabText(idx);
  return str;
}

QWidget * ScriptToolbox::createWidget(const QString & className, QWidget * parent, const QString & name)
{
  XUiLoader ui;
  return ui.createWidget(className, parent, name);
}

QObject * ScriptToolbox::createLayout(const QString & className, QWidget * parent, const QString & name)
{
  XUiLoader ui;
  return ui.createLayout(className, parent, name);
}

QWidget * ScriptToolbox::loadUi(const QString & screenName, QWidget * parent)
{
  if(screenName.isEmpty())
    return 0;

  q.prepare("SELECT *"
            "  FROM uiform"
            " WHERE((uiform_name=:uiform_name)"
            "   AND (uiform_enabled))"
            " ORDER BY uiform_order"
            " LIMIT 1;");
  q.bindValue(":uiform_name", screenName);
  q.exec();
  if(!q.first())
  {
    QMessageBox::critical(0, tr("Could Not Create Form"),
      tr("Could not create the required form. Either an error occurred or the specified form does not exist.") );
    return 0;
  }

  XUiLoader loader;
  QByteArray ba = q.value("uiform_source").toByteArray();
  QBuffer uiFile(&ba);
  if(!uiFile.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(0, tr("Could not load file"),
        tr("There was an error loading the UI Form from the database."));
    return 0;
  }
  QWidget *ui = loader.load(&uiFile, parent);
  uiFile.close();

  return ui;
}

QWidget * ScriptToolbox::lastWindow() const
{
  return _lastWindow;
}

bool ScriptToolbox::printReport(const QString & name, const ParameterList & params, const QString & pdfFilename)
{
  orReport report(name, params);
  if(report.isValid())
  {
      if(!pdfFilename.isEmpty()) 
      {
          report.exportToPDF(pdfFilename);
      }
      else
      {
          report.print();
      }
  }
  else
  {
    report.reportError(NULL);
    return false;
  } 
  return true;
}

bool ScriptToolbox::coreDisconnect(QObject * sender, const QString & signal, QObject * receiver, const QString & method)
{
  return QObject::disconnect(sender, QString("2%1").arg(signal).toUtf8().data(), receiver, QString("1%1").arg(method).toUtf8().data());
}

QString ScriptToolbox::fileDialog(QWidget * parent, const QString & caption, const QString & dir, const QString & filter, int fileModeSel, int acceptModeSel)
{
  QStringList filters;
  QString path;
  QFileDialog::FileMode	 _fileMode;
  QFileDialog::AcceptMode _acceptMode;

  switch (fileModeSel)
  {
  case 1:
   _fileMode = QFileDialog::ExistingFile;
   break;
  case 2:
   _fileMode = QFileDialog::Directory;
   break;
  case 3:
   _fileMode = QFileDialog::DirectoryOnly;
   break;
  case 4:
   _fileMode = QFileDialog::ExistingFiles;
   break;
  default:
   _fileMode = QFileDialog::AnyFile;
  }

  switch (acceptModeSel)
  {
  case 1:
   _acceptMode = QFileDialog::AcceptSave;
   break;
  default:
   _acceptMode = QFileDialog::AcceptOpen;
  }

  QFileDialog newdlg(parent, caption, dir);
  newdlg.setFileMode(_fileMode);
  newdlg.setAcceptMode(_acceptMode);
  if (! filter.isEmpty())
  {
    filters << filter << tr("Any Files (*)");
    newdlg.setFilters(filters);
  }
  if (newdlg.exec())  path = newdlg.selectedFiles().join(", ");
  else path = "";

  return path;
}

void ScriptToolbox::openUrl(const QString & fileUrl)
{
    //If url scheme is missing, we'll assume it is "file" for now.
    QUrl url(fileUrl);
    if (url.scheme().isEmpty())
      url.setScheme("file");
    QDesktopServices::openUrl(url);
}

bool ScriptToolbox::copyFile(const QString & oldName, const QString & newName)
{
   return QFile::copy(oldName, newName);
}

QString ScriptToolbox::getFileName(const QString & path)
{
    QFileInfo fi(path);
    return fi.fileName();
}

bool ScriptToolbox::renameFile(const QString & oldName, const QString & newName)
{
   return QFile::rename(oldName, newName);
}

bool ScriptToolbox::removeFile(const QString & name)
{
   return QFile::remove(name);
}

bool ScriptToolbox::fileExists(const QString & name)
{
   return QFile::exists(name);
}

QString ScriptToolbox::textStreamRead(const QString & name)
{
  QFile file(name);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return "error";
  QTextStream stream(&file);
  QString fileText = tr(file.readAll());
  file.close();
  return fileText;
}

bool ScriptToolbox::textStreamWrite(const QString & pName, const QString & WriteText)
{
   QString name = pName;
   if(name.isEmpty() || WriteText.isEmpty()) return false;
   QFileInfo fi(name);
   if(fi.suffix().isEmpty())
      name += ".txt";
   QFile file(name);
   if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
   QTextStream out(&file);
   out << WriteText;
   file.close();
   return true;
}

bool fileExists(const QString & name)
{
   QFile file(name);
   return file.exists();
}

QString ScriptToolbox::getHomeDir()
{
   return QDir::homePath();
}

QString ScriptToolbox::getCurrentDir()
{
   return QDir::currentPath();
}

QString ScriptToolbox::rootPath()
{
   return QDir::rootPath();
}


bool ScriptToolbox::makePath(const QString & mkPath, const QString & rootPath)
{
   QDir dir(rootPath);

   return dir.mkpath(mkPath);
}

bool ScriptToolbox::removePath(const QString & rmPath, const QString & rootPath)
{
   QDir dir(rootPath);

   return dir.rmpath(rmPath);
}

void ScriptToolbox::listProperties(const QScriptValue &obj) const
{
  qWarning("Properties of %s:", qPrintable(obj.toString()));
  QScriptValue tmp = obj;
  while (tmp.isObject())
  {
    QScriptValueIterator prop(tmp);
    while (prop.hasNext())
    {
      prop.next();
      qWarning("  %s\t= %s",
               qPrintable(prop.name()), qPrintable(prop.value().toString()));
    }
    tmp = tmp.prototype();
    if (! (tmp.isNull() || tmp.isUndefined()))
      qWarning(" Prototype %s of %s:",
               qPrintable(tmp.toString()), qPrintable(obj.toString()));
  }
  qWarning("End of %s", qPrintable(obj.toString()));
}

int ScriptToolbox::messageBox(const QString & type, QWidget * parent, const QString & title, const QString & text, int buttons, int defaultButton)
{
  int btn;
  if(type == "critical")
    btn = QMessageBox::critical(parent, title, text, buttons, defaultButton);
  else if(type == "information")
    btn = QMessageBox::information(parent, title, text, buttons, defaultButton);
  else if(type == "question")
    btn = QMessageBox::question(parent, title, text, buttons, defaultButton);
  else //if(type == "warning")
    btn = QMessageBox::warning(parent, title, text, buttons, defaultButton);
  return btn;
}

QScriptValue ScriptToolbox::variantToScriptValue(QScriptEngine * engine, QVariant var)
{
  if(var.isNull())
    return engine->nullValue();

  switch(var.type())
  {
    case QVariant::Invalid:
      return engine->nullValue();
    case QVariant::Bool:
      return QScriptValue(engine, var.toBool());
    case QVariant::Int:
      return QScriptValue(engine, var.toInt());
    case QVariant::UInt:
      return QScriptValue(engine, var.toUInt());
    case QVariant::Double:
      return QScriptValue(engine, var.toDouble());
    case QVariant::Char:
      return QScriptValue(engine, var.toChar().unicode());
    case QVariant::String:
      return QScriptValue(engine, var.toString());
    case QVariant::LongLong:
      return QScriptValue(engine, qsreal(var.toLongLong()));
    case QVariant::ULongLong:
      return QScriptValue(engine, qsreal(var.toULongLong()));
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
      return engine->newDate(var.toDateTime());
    case QVariant::RegExp:
      return engine->newRegExp(var.toRegExp());
/*
 * Would be ideal to have these as well but I don't know if they are really necessary
    case QVariant::StringList:
    case QVariant::List:

    case QVariant::Map:
*/
    default:
      return engine->newVariant(var);
  }

  // If we are not doing an explicity conversion just pass the variant back
  // and see what happens
  return engine->newVariant(var);
}

void ScriptToolbox::setLastWindow(QWidget * lw)
{
  _lastWindow = lw;
}

QWidget *ScriptToolbox::openWindow(QString name, QWidget *parent, Qt::WindowModality modality, Qt::WindowFlags flags)
{
  QWidget *returnVal = 0;

  XSqlQuery screenq;
  screenq.prepare("SELECT * "
                  "FROM uiform "
                  "WHERE (uiform_name=:uiform_name);");
  screenq.bindValue(":uiform_name", name);
  screenq.exec();
  if (screenq.first())
  {
    XUiLoader loader;
    QByteArray ba = screenq.value("uiform_source").toByteArray();
    QBuffer uiFile(&ba);
    if (!uiFile.open(QIODevice::ReadOnly))
    {
      QMessageBox::critical(0, tr("Could not load UI"),
                            tr("<p>There was an error loading the UI Form "
                               "from the database."));
      return 0;
    }
    QWidget *ui = loader.load(&uiFile);
    uiFile.close();

    if (ui->inherits("QDialog"))
    {
      flags |= Qt::Dialog;
      if (modality == Qt::NonModal)
        modality = Qt::WindowModal;
    }

    XMainWindow *window = new XMainWindow(parent,
                                          screenq.value("uiform_name").toString(),
                                          flags);

    window->setCentralWidget(ui);
    window->setWindowTitle(ui->windowTitle());
    window->setWindowModality(modality);

    if (ui->inherits("QDialog"))
    {
      QDialog *innerdlg = qobject_cast<QDialog*>(ui);

      connect(innerdlg, SIGNAL(finished(int)), window, SLOT(close()));
      connect(innerdlg, SIGNAL(accepted()),    window, SLOT(close()));
      connect(innerdlg, SIGNAL(rejected()),    window, SLOT(close()));

      // alternative to creating mydialog object:
      // for each property of mydialog (including functions)
      //   add a property to _engine's mywindow property
      if (engine(window))
      {
        QScriptValue mydialog = engine(window)->newQObject(innerdlg);
        engine(window)->globalObject().setProperty("mydialog", mydialog);
      }
      else
        qWarning("Could not find the script engine to embed a dialog inside "
               "a placeholder window");

      omfgThis->handleNewWindow(window);
      returnVal = ui;
    }
    else
    {
      omfgThis->handleNewWindow(window);
      returnVal = window;
    }
    _lastWindow = window;
  }
  else if (screenq.lastError().type() != QSqlError::None)
  {
    systemError(0, screenq.lastError().databaseText(), __FILE__, __LINE__);
    return 0;
  }

  return returnVal;
}

void ScriptToolbox::addColumnXTreeWidget(QWidget * tree, const QString & pString, int pWidth, int pAlignment, bool pVisible, const QString pEditColumn, const QString pDisplayColumn)
{
  XTreeWidget *xt = qobject_cast<XTreeWidget*>(tree);
  if(xt)
    xt->addColumn(pString, pWidth, pAlignment, pVisible, pEditColumn, pDisplayColumn);
}

void ScriptToolbox::populateXTreeWidget(QWidget * tree, QObject * pSql, bool pUseAltId)
{
  XTreeWidget *xt = qobject_cast<XTreeWidget*>(tree);
  ScriptQuery *sq = qobject_cast<ScriptQuery*>(pSql);
  if(xt)
    xt->populate(sq->query(), pUseAltId);
}

void ScriptToolbox::loadQWebView(QWidget * webView, const QString & url)
{
  QUrl p = QUrl(url);
  QWebView *wv = qobject_cast<QWebView*>(webView);
  if(wv)
    wv->load(p);
}

// ParameterList Conversion functions
QScriptValue ParameterListtoScriptValue(QScriptEngine *engine, const ParameterList &params)
{
  QScriptValue obj = engine->newObject();
  for(int i = 0; i < params.count(); i++)
  {
    obj.setProperty(params.name(i), ScriptToolbox::variantToScriptValue(engine, params.value(i)));
  }

  return obj;
}

void ParameterListfromScriptValue(const QScriptValue &obj, ParameterList &params)
{
  QScriptValueIterator it(obj);
  while (it.hasNext())
  {
    it.next();
    if(it.flags() & QScriptValue::SkipInEnumeration)
      continue;
    params.append(it.name(), it.value().toVariant());
  }
}

QScriptValue XSqlQuerytoScriptValue(QScriptEngine *engine, const XSqlQuery &qry)
{
  ScriptQuery * sq = new ScriptQuery(engine);
  sq->setQuery(qry);
  QScriptValue obj = engine->newQObject(sq);
  return obj;
}

void XSqlQueryfromScriptValue(const QScriptValue &obj, XSqlQuery &qry)
{
  ScriptQuery * sq = qobject_cast<ScriptQuery*>(obj.toQObject());
  if(sq)
  {
    qry = sq->query();
  }
}

QScriptValue SetResponsetoScriptValue(QScriptEngine *engine, const enum SetResponse &sr)
{
  return QScriptValue(engine, (int)sr);
}

void SetResponsefromScriptValue(const QScriptValue &obj, enum SetResponse &sr)
{
  sr = (enum SetResponse)obj.toInt32();
}

QScriptValue ParameterGroupStatestoScriptValue(QScriptEngine *engine, const enum ParameterGroup::ParameterGroupStates &en)
{
  return QScriptValue(engine, (int)en);
}

void ParameterGroupStatesfromScriptValue(const QScriptValue &obj, enum ParameterGroup::ParameterGroupStates &en)
{
  en = (enum ParameterGroup::ParameterGroupStates)obj.toInt32();
}

QScriptValue ParameterGroupTypestoScriptValue(QScriptEngine *engine, const enum ParameterGroup::ParameterGroupTypes &en)
{
  return QScriptValue(engine, (int)en);
}

void ParameterGroupTypesfromScriptValue(const QScriptValue &obj, enum ParameterGroup::ParameterGroupTypes &en)
{
  en = (enum ParameterGroup::ParameterGroupTypes)obj.toInt32();
}

QScriptValue QtWindowModalitytoScriptValue(QScriptEngine *engine, const enum Qt::WindowModality &en)
{
  return QScriptValue(engine, (int)en);
}

void QtWindowModalityfromScriptValue(const QScriptValue &obj, enum Qt::WindowModality &en)
{
  en = (enum Qt::WindowModality)obj.toInt32();
}

QScriptValue SaveFlagstoScriptValue(QScriptEngine *engine, const enum SaveFlags &en)
{
  return QScriptValue(engine, (int)en);
}

void SaveFlagsfromScriptValue(const QScriptValue &obj, enum SaveFlags &en)
{
  if (obj.isNumber())
    en = (enum SaveFlags)obj.toInt32();
  else if (obj.isString())
  {
    if (obj.toString() == "CHECK")
      en = CHECK;
    else if (obj.toString() == "CHANGEONE")
      en = CHANGEONE;
    else if (obj.toString() == "CHANGEALL")
      en = CHANGEALL;
    else
      qWarning("string %s could not be converted to SaveFlags",
               qPrintable(obj.toString()));
  }
  else
    qWarning("object %s could not be converted to SaveFlags",
             qPrintable(obj.toString()));
}

int ScriptToolbox::saveCreditCard(QWidget *parent,
                                  int custId,
                                  QString ccName, 
                                  QString ccAddress1, 
                                  QString ccAddress2,
                                  QString ccCity, 
                                  QString ccState, 
                                  QString ccZip, 
                                  QString ccCountry,
                                  QString ccNumber,
                                  QString ccType,
                                  QString ccExpireMonth,
                                  QString ccExpireYear,
                                  int ccId,
                                  bool ccActive )
{
  return creditCard::saveCreditCard(parent,
                                    custId,
                                    ccName,
                                    ccAddress1,
                                    ccAddress2,
                                    ccCity,
                                    ccState,
                                    ccZip,
                                    ccCountry,
                                    ccNumber,
                                    ccType,
                                    ccExpireMonth,
                                    ccExpireYear,
                                    ccId,
                                    ccActive);
}

QObject *ScriptToolbox::getCreditCardProcessor()
{
  return CreditCardProcessor::getProcessor();
}
