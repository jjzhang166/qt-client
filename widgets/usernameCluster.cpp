/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "usernamecluster.h"

#include <QMenu>
#include <QtScript>

UsernameLineEdit::UsernameLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "usr", "usr_id", "usr_username", "usr_propername", 0, 0, pName, "usr_active"),
    _username(0)
{
  setTitles(tr("User Name"), tr("User Names"));
  setUiName("user");
  setShowInactive(true);
  if (_x_preferences && !_x_preferences->boolean("ClusterButtons"))
  {
    menu()->removeAction(_infoAct);
    menu()->removeAction(menu()->actions().at(2));
  }
}

UsernameLineEdit::~UsernameLineEdit()
{
  if (_username)
    delete _username;
}

void UsernameLineEdit::setType(enum Type pType)
{
  _type = pType;

  qWarning() << "UsernameLineEdit::setType is deprecated and should not be used.";
}

void UsernameLineEdit::setUsername(const QString & pUsername)
{
  setNumber(pUsername);
}

const QString & UsernameLineEdit::username()
{
  if(hasFocus())
    sParse();
  if (_username)
    delete _username;
  _username = new QString(text());
  return *_username;
}

///////////////////////////////////////

UsernameCluster::UsernameCluster(QWidget * parent, const char * name)
  : VirtualCluster(parent, name)
{
  addNumberWidget(new UsernameLineEdit(this, name));
}

void UsernameCluster::setUsername(const QString & pUsername)
{
  static_cast<UsernameLineEdit* >(_number)->setUsername(pUsername);
}

// script exposure ////////////////////////////////////////////////////////////

QScriptValue constructUsernameLineEdit(QScriptContext *context,
                                       QScriptEngine  *engine)
{
  UsernameLineEdit *obj = 0;

  if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameLineEdit(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else if (context->argumentCount() >= 2 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameLineEdit(qscriptvalue_cast<QWidget*>(context->argument(0)),
                               qPrintable(context->argument(1).toString()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate UsernameLineEdit constructor");

#if QT_VERSION >= 0x050000
  return engine->toScriptValue(obj);
#else
  Q_UNUSED(engine); return QScriptValue();
#endif
}

void setupUsernameLineEdit(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags ro = QScriptValue::ReadOnly | QScriptValue::Undeletable;
  QScriptValue widget = engine->globalObject().property("UsernameLineEdit");
  if (! widget.isFunction()) {
    widget = engine->newFunction(constructUsernameLineEdit);
    engine->globalObject().setProperty("UsernameLineEdit", widget, ro);
  }

  widget.setProperty("UsersAll",     QScriptValue(engine, UsernameLineEdit::UsersAll),      ro);
  widget.setProperty("UsersActive",  QScriptValue(engine, UsernameLineEdit::UsersActive),   ro);
  widget.setProperty("UsersInactive",QScriptValue(engine, UsernameLineEdit::UsersInactive), ro);
}

QScriptValue constructUsernameCluster(QScriptContext *context,
                                       QScriptEngine  *engine)
{
#if QT_VERSION >= 0x050000
  UsernameCluster *obj = 0;

  if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameCluster(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else if (context->argumentCount() >= 2 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameCluster(qscriptvalue_cast<QWidget*>(context->argument(0)),
                               qPrintable(context->argument(1).toString()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate UsernameCluster constructor");

  return engine->toScriptValue(obj);
#else
  Q_UNUSED(context); Q_UNUSED(engine); return QScriptValue();
#endif
}

void setupUsernameCluster(QScriptEngine *engine)
{
  if (! engine->globalObject().property("UsernameCluster").isFunction())
  {
    QScriptValue ctor = engine->newFunction(constructUsernameCluster);
    QScriptValue meta = engine->newQMetaObject(&UsernameCluster::staticMetaObject, ctor);

    engine->globalObject().setProperty("UsernameCluster", meta,
                                       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  }
}
