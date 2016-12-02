/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qjsonvalueproto.h"
#include "qjsonobjectproto.h"

#if QT_VERSION < 0x050000
void setupQJsonValueProto(QScriptEngine *engine)
{
  Q_UNUSED(engine); // do nothing
}

#else
QScriptValue QJsonValueToScriptValue(QScriptEngine *engine, QJsonValue* const &in)
{
  QJsonObject obj = in->toObject();
  return QJsonObjectToScriptValue(engine, &obj);
}

void QJsonValueFromScriptValue(const QScriptValue &obj, QJsonValue* &out)
{
  out = dynamic_cast<QJsonValue*>(obj.toQObject());
}

void setupQJsonValueProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QJsonValueToScriptValue, QJsonValueFromScriptValue);
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QJsonValueProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QJsonValue*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QJsonValue>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQJsonValue,
                                                 proto);
  engine->globalObject().setProperty("QJsonValue",  constructor);
  proto.setProperty("Undefined", QScriptValue(engine, QJsonValue::Undefined),  permanent);
}

QScriptValue constructQJsonValue(QScriptContext *context, QScriptEngine *engine)
{
  Q_UNUSED(context);
  QJsonValue *obj = new QJsonValue();

  if (context->argumentCount() >= 1)
    *obj = QJsonValue::fromVariant(context->argument(0).toVariant());
  return engine->toScriptValue(obj);
}

QJsonValueProto::QJsonValueProto(QObject *parent)
    : QObject(parent)
{
}

QJsonValueProto::~QJsonValueProto()
{
}

bool QJsonValueProto::isArray() const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->isArray();
  return false;
}

bool QJsonValueProto::isBool() const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->isBool();
  return false;
}

bool QJsonValueProto::isDouble() const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->isDouble();
  return false;
}

bool QJsonValueProto::isNull() const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

bool QJsonValueProto::isObject() const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->isObject();
  return false;
}

bool QJsonValueProto::isString() const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->isString();
  return false;
}

bool QJsonValueProto::isUndefined() const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->isUndefined();
  return false;
}

QJsonArray QJsonValueProto::toArray(const QJsonArray & defaultValue) const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->toArray(defaultValue);
  return QJsonArray();
}

QJsonArray QJsonValueProto::toArray() const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->toArray();
  return QJsonArray();
}

bool QJsonValueProto::toBool(bool defaultValue) const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->toBool(defaultValue);
  return false;
}

double QJsonValueProto::toDouble(double defaultValue) const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->toDouble(defaultValue);
  return double();
}

int QJsonValueProto::toInt(int defaultValue) const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->toInt(defaultValue);
  return 0;
}

QJsonObject QJsonValueProto::toObject(const QJsonObject & defaultValue) const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->toObject(defaultValue);
  return QJsonObject();
}

QJsonObject QJsonValueProto::toObject() const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->toObject();
  return QJsonObject();
}

QString QJsonValueProto::toString(const QString & defaultValue) const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->toString(defaultValue);
  return QString();
}

QVariant QJsonValueProto::toVariant() const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->toVariant();
  return QVariant();
}

QJsonValue::Type QJsonValueProto::type() const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->type();
  return QJsonValue::Undefined;
}

bool QJsonValueProto::operator!=(const QJsonValue & other) const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->operator!=(other);
  return false;
}

QJsonValue & QJsonValueProto::operator=(const QJsonValue & other)
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->operator=(other);
  return *(new QJsonValue());
}

bool QJsonValueProto::operator==(const QJsonValue & other) const
{
  QJsonValue *item = qscriptvalue_cast<QJsonValue*>(thisObject());
  if (item)
    return item->operator==(other);
  return false;
}

QJsonValue QJsonValueProto::fromVariant(const QVariant & variant)
{
  return QJsonValue::fromVariant(variant);
}

#endif
