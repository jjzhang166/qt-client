/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qbuttongroupproto.h"

#include <QScriptEngine>
#include <QButtonGroup>
#include <QAbstractButton>

QScriptValue QButtonGrouptoScriptValue(QScriptEngine *engine, QButtonGroup* const &item)
{
  return engine->newQObject(item);
}

void QButtonGroupfromScriptValue(const QScriptValue &obj, QButtonGroup* &item)
{
  item = qobject_cast<QButtonGroup*>(obj.toQObject());
}

void setupQButtonGroupProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QButtonGrouptoScriptValue, QButtonGroupfromScriptValue);

  QScriptValue proto = engine->newQObject(new QButtonGroupProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QButtonGroup*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQButtonGroup, proto);

  engine->globalObject().setProperty("QButtonGroup", constructor);
}

QScriptValue constructQButtonGroup(QScriptContext *context, QScriptEngine *engine)
{
  QButtonGroup *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isObject())
  {
    obj = new QButtonGroup(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  }
  else
    obj = new QButtonGroup();
  return engine->toScriptValue(obj);
}

QButtonGroupProto::QButtonGroupProto(QObject *parent)
    : QObject(parent)
{
}

void QButtonGroupProto::addButton(QAbstractButton *button)
{
  QButtonGroup *item = qscriptvalue_cast<QButtonGroup*>(thisObject());
  if (item)
    item->addButton(button);
}

QList<QAbstractButton *> QButtonGroupProto::buttons() const
{
  QButtonGroup *item = qscriptvalue_cast<QButtonGroup*>(thisObject());
  if (item)
    return item->buttons();

  return QList<QAbstractButton *>();
}

void QButtonGroupProto::removeButton(QAbstractButton *button)
{
  QButtonGroup *item = qscriptvalue_cast<QButtonGroup*>(thisObject());
  if (item)
    item->removeButton(button);
}

bool QButtonGroupProto::exclusive()
{
  QButtonGroup *item = qscriptvalue_cast<QButtonGroup*>(thisObject());
  if (item)
  {
    return item->exclusive();
  }
  return false;
}

void QButtonGroupProto::setExclusive(bool exclusive)
{
  QButtonGroup *item = qscriptvalue_cast<QButtonGroup*>(thisObject());
  if (item)
  {
    item->setExclusive(exclusive);
  }
}

