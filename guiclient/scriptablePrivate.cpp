/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptablePrivate.h"

#include <QWidget>
#include <QScriptEngine>
#include <QScriptEngineDebugger>

#include "include.h"
#include "scripttoolbox.h"
#include "qeventproto.h"
#include "parameterlistsetup.h"

ScriptablePrivate::ScriptablePrivate(QWidget *parent, QWidget *self)
  : ScriptableWidget(self),
    _showMe(0),
    _rememberPos(0),
    _rememberSize(0),
    _shown(false)
{
  ScriptToolbox::setLastWindow(parent);
}

ScriptablePrivate::~ScriptablePrivate()
{
  if(_showMe)
    delete _showMe;
  if(_rememberPos)
    delete _rememberPos;
  if(_rememberSize)
    delete _rememberSize;
}

QScriptEngine *ScriptablePrivate::engine()
{
  QScriptEngine *engine = ScriptableWidget::engine();
  omfgThis->loadScriptGlobals(engine);
  QScriptValue mywidget = engine->globalObject().property("mywidget");

  engine->globalObject().setProperty("mywindow", mywidget);

  // mydialog is required for backwards compatibility.
  // we had problems when we first started scripting [qx]dialogs.
  // i don't know if these remain. gjm
  if (_self->inherits("QDialog"))
    _engine->globalObject().setProperty("mydialog", mywidget);

  return engine;
}

enum SetResponse ScriptablePrivate::callSet(const ParameterList & params)
{
  loadScriptEngine();

  enum SetResponse returnValue = NoError;
  if (_engine)
  {
    QScriptValue tmp = _engine->globalObject()
                           .property("set")
                           .call(QScriptValue(),
                                 QScriptValueList() << ParameterListtoScriptValue(_engine, params));
    returnValue = (enum SetResponse)tmp.toInt32();
  }

  return returnValue;
}

void ScriptablePrivate::callShowEvent(QEvent *event)
{
  if (_engine)
  {
    _engine->globalObject().property("showEvent")
                           .call(QScriptValue(),
                                 QScriptValueList() << _engine->toScriptValue(event));
  }
}

void ScriptablePrivate::callCloseEvent(QEvent *event)
{
  if (_engine)
  {
    _engine->globalObject().property("closeEvent")
                           .call(QScriptValue(),
                                 QScriptValueList() << _engine->toScriptValue(event));
  }
}

