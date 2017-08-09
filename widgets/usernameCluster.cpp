/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QtScript>

#include "usernamecluster.h"

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

QScriptValue UsernameLineEdittoScriptValue(QScriptEngine *engine, UsernameLineEdit* const &item)
{
  return engine->newQObject(item);
}

void UsernameLineEditfromScriptValue(const QScriptValue &obj, UsernameLineEdit* &item)
{
  item = qobject_cast<UsernameLineEdit*>(obj.toQObject());
}

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

  return engine->toScriptValue(obj);
}

void setupUsernameLineEdit(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, UsernameLineEdittoScriptValue, UsernameLineEditfromScriptValue);

  QScriptValue widget = engine->newFunction(constructUsernameLineEdit);

  widget.setProperty("UsersAll",     QScriptValue(engine, UsernameLineEdit::UsersAll),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UsersActive",  QScriptValue(engine, UsernameLineEdit::UsersActive),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UsersInactive",QScriptValue(engine, UsernameLineEdit::UsersInactive),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("UsernameLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

QScriptValue UsernameClustertoScriptValue(QScriptEngine *engine, UsernameCluster* const &item)
{
  return engine->newQObject(item);
}

void UsernameClusterfromScriptValue(const QScriptValue &obj, UsernameCluster* &item)
{
  item = qobject_cast<UsernameCluster*>(obj.toQObject());
}

QScriptValue constructUsernameCluster(QScriptContext *context,
                                       QScriptEngine  *engine)
{
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
}

void setupUsernameCluster(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, UsernameClustertoScriptValue, UsernameClusterfromScriptValue);

  QScriptValue widget = engine->newFunction(constructUsernameCluster);

  engine->globalObject().setProperty("UsernameCluster", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
