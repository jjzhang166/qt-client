/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qjsondocumentproto.h"

#if QT_VERSION < 0x050000
void setupQJsonDocumentProto(QScriptEngine *engine)
{
  // do nothing
}
#else
#include <QJsonDocument>

void setupQJsonDocumentProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QJsonDocumentProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QJsonDocument*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQJsonDocument,
                                                 proto);
  engine->globalObject().setProperty("QJsonDocument",  constructor);
}

QScriptValue constructQJsonDocument(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QJsonDocument *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isVariant())
    obj = new QJsonDocument(context->argument(0).toVariant().toJsonDocument());
  else
    obj = new QJsonDocument();
  return engine->toScriptValue(obj);
}

QJsonDocumentProto::QJsonDocumentProto(QObject *parent)
    : QObject(parent)
{
}

QJsonDocumentProto::~QJsonDocumentProto()
{
}

QJsonArray QJsonDocumentProto::array() const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->array();
  return QJsonArray();
}

bool QJsonDocumentProto::isArray() const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->isArray();
  return false;
}

bool QJsonDocumentProto::isEmpty() const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

bool QJsonDocumentProto::isNull() const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

bool QJsonDocumentProto::isObject() const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->isObject();
  return false;
}

QJsonObject QJsonDocumentProto::object() const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->object();
  return QJsonObject();
}

const char * QJsonDocumentProto::rawData(int * size) const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->rawData(size);
  return 0;
}

void QJsonDocumentProto::setArray(const QJsonArray & array)
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    item->setArray(array);
}

void QJsonDocumentProto::setObject(const QJsonObject & object)
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    item->setObject(object);
}

QByteArray QJsonDocumentProto::toBinaryData() const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->toBinaryData();
  return QByteArray();
}

QByteArray QJsonDocumentProto::toJson(QJsonDocument::JsonFormat format) const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->toJson(format);
  return QByteArray();
}

QVariant QJsonDocumentProto::toVariant() const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->toVariant();
  return QVariant();
}

bool QJsonDocumentProto::operator!=(const QJsonDocument & other) const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->operator!=(other);
  return false;
}

QJsonDocument & QJsonDocumentProto::operator=(const QJsonDocument & other)
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->operator=(other);
  return *(new QJsonDocument());
}

bool QJsonDocumentProto::operator==(const QJsonDocument & other) const
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->operator==(other);
  return false;
}

QJsonDocument QJsonDocumentProto::fromBinaryData(const QByteArray & data, QJsonDocument::DataValidation validation)
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->fromBinaryData(data, validation);
  return QJsonDocument();
}

QJsonDocument QJsonDocumentProto::fromJson(const QByteArray & json, QJsonParseError * error)
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->fromJson(json, error);
  return QJsonDocument();
}

QJsonDocument QJsonDocumentProto::fromRawData(const char * data, int size, QJsonDocument::DataValidation validation)
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->fromRawData(data, size, validation);
  return QJsonDocument();
}

QJsonDocument QJsonDocumentProto::fromVariant(const QVariant & variant)
{
  QJsonDocument *item = qscriptvalue_cast<QJsonDocument*>(thisObject());
  if (item)
    return item->fromVariant(variant);
  return QJsonDocument();
}
#endif
