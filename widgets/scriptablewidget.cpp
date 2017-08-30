/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptablewidget.h"

#include <QDebug>
#include <QWidget>
#include <QScriptEngine>
#include <QScriptEngineDebugger>
#include <QScriptValue>
#include <QSqlDatabase>
#include <QSqlDriver>

#include "guiclientinterface.h"
#include "include.h"
#include "qtsetup.h"
#include "scriptcache.h"
#include "setupscriptapi.h"
#include "widgets.h"
#include "xsqlquery.h"

#define DEBUG false

GuiClientInterface *ScriptableWidget::_guiClientInterface = 0;
ScriptCache        *ScriptableWidget::_cache              = 0;

ScriptableWidget::ScriptableWidget(QWidget *self)
  : _debugger(0),
    _engine(0),
    _scriptLoaded(false)
{
  _self = (self ? self : dynamic_cast<QWidget *>(this));
  if (! _cache)
    _cache = new ScriptCache(_guiClientInterface);
}

ScriptableWidget::~ScriptableWidget()
{
}

QScriptEngine *ScriptableWidget::engine()
{
  QWidget *w = _self;
  if (w && ! _engine)
  {
    _engine = new QScriptEngine(w);
    if (_x_preferences && _x_preferences->boolean("EnableScriptDebug"))
    {
      _debugger = new QScriptEngineDebugger(w);
      _debugger->attachTo(_engine);
    }

    setupQt(_engine);
    setupInclude(_engine);
    setupScriptApi(_engine);
    setupWidgetsScriptApi(_engine, _guiClientInterface);
    QScriptValue mywidget = _engine->newQObject(w);
    _engine->globalObject().setProperty("mywidget",  mywidget);
  }

  return _engine;
}

void ScriptableWidget::loadScript(const QStringList &list)
{
  qDebug() << "Looking for scripts" << list;
  QString widgetName = list.last();

  // assume a coherent cache has the whole list if it has the last
  if (! _cache->_idsByName.contains(widgetName))
  {
    _cache->_idsByName.insert(widgetName, QList<int>());

    // make one query to get all relevant scripts, using a JSON object
    // to enforce load order: key = order, value = name
    QStringList pair;
    for (int i = 0; i < list.length(); i++)
    {
      pair.append(QString("\"%1\": \"%2\"").arg(i).arg(list.at(i)));
    }

    XSqlQuery q;
    q.prepare("WITH jsonlist AS (SELECT *"
              "                    FROM json_each_text(:jsonlist))"
              "SELECT script_id, script_name, script_source"
              "  FROM script"
              "  JOIN jsonlist ON script_name = value"
              " WHERE script_enabled"
              " ORDER BY key, script_order;");
    q.bindValue(":jsonlist", "{" + pair.join(", ") + "}");
    q.exec();
    while (q.next())
    {
      _cache->_scriptsById.insert(q.value("script_id").toInt(),
                               qMakePair(q.value("script_name").toString(),
                                         q.value("script_source").toString()));
      _cache->_idsByName[widgetName].append(q.value("script_id").toInt());
    }
    if (DEBUG) qDebug() << _cache->_idsByName[widgetName];
  }

  if (_cache->_idsByName[widgetName].isEmpty())
    return;

  if (! engine())
  {
    qDebug() << "could not initialize engine" << list;
    return;
  }

  foreach(int id, _cache->_idsByName[widgetName])
  {
    QPair<QString, QString> script = _cache->_scriptsById.value(id);
    if (DEBUG) qDebug() << "evaluating" << id << script.first;
    QScriptValue result = engine()->evaluate(script.second, script.first);
    if (engine()->hasUncaughtException())
    {
      qDebug() << "uncaught exception at line"
               << engine()->uncaughtExceptionLineNumber()
               << ":" << result.toString();
    }
  }
}

void ScriptableWidget::loadScript(const QString& oName)
{
  loadScript(QStringList(oName));
}

void ScriptableWidget::loadScriptEngine()
{
  QWidget *w = _self;
  if (_scriptLoaded || ! w)
    return;
  _scriptLoaded = true;

  QStringList scriptList;

  // load scripts by class heirarchy name
  const QMetaObject *m = w->metaObject();
  while (m)
  {
    scriptList.prepend(m->className());
    m = m->superClass();
  }

  // load scripts by object name
  QStringList parts = w->objectName().split(" ");
  QStringList search_parts;
  QString oName;
  while(!parts.isEmpty())
  {
    search_parts.append(parts.takeFirst());
    oName = search_parts.join(" ");
    scriptList.append(oName);
  }

  scriptList.removeDuplicates();
  loadScript(scriptList);
}
