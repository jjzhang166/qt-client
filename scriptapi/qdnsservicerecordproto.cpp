/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdnsservicerecordproto.h"

#if QT_VERSION < 0x050000
void setupQDnsServiceRecordProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

QScriptValue QListQDnsServiceRecordToScriptValue(QScriptEngine *engine, const QList<QDnsServiceRecord> &list)
{
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < list.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(list.at(i)));
  }
  return newArray;
}
void QListQDnsServiceRecordFromScriptValue(const QScriptValue &obj, QList<QDnsServiceRecord> &list)
{
  list = QList<QDnsServiceRecord>();

  if (obj.isArray()) {
    QScriptValueIterator it(obj);

    while (it.hasNext()) {
      it.next();
      if (it.flags() & QScriptValue::SkipInEnumeration)
        continue;

      if (it.value().isString()) {
        QDnsServiceRecord item = qscriptvalue_cast<QDnsServiceRecord>(it.value());
        list.insert(it.name().toInt(), item);
      }
    }
  }
}

void setupQDnsServiceRecordProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDnsServiceRecordProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDnsServiceRecord*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDnsServiceRecord>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDnsServiceRecord, proto);
  engine->globalObject().setProperty("QDnsServiceRecord",  constructor);

  qScriptRegisterMetaType(engine, QListQDnsServiceRecordToScriptValue, QListQDnsServiceRecordFromScriptValue);
}

QScriptValue constructQDnsServiceRecord(QScriptContext *context, QScriptEngine *engine)
{
  QDnsServiceRecord *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isObject()) {
    obj = new QDnsServiceRecord(qscriptvalue_cast<QDnsServiceRecord>(context->argument(0)));
  } else {
    obj = new QDnsServiceRecord();
  }

  return engine->toScriptValue(obj);
}

QDnsServiceRecordProto::QDnsServiceRecordProto(QObject *parent) : QObject(parent)
{
}
QDnsServiceRecordProto::~QDnsServiceRecordProto()
{
}

QString QDnsServiceRecordProto::name() const
{
  QDnsServiceRecord *item = qscriptvalue_cast<QDnsServiceRecord*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

quint16 QDnsServiceRecordProto::port() const
{
  QDnsServiceRecord *item = qscriptvalue_cast<QDnsServiceRecord*>(thisObject());
  if (item)
    return item->port();
  return quint16();
}

quint16 QDnsServiceRecordProto::priority() const
{
  QDnsServiceRecord *item = qscriptvalue_cast<QDnsServiceRecord*>(thisObject());
  if (item)
    return item->priority();
  return quint16();
}

void QDnsServiceRecordProto::swap(QDnsServiceRecord &other)
{
  QDnsServiceRecord *item = qscriptvalue_cast<QDnsServiceRecord*>(thisObject());
  if (item)
    item->swap(other);
}

QString QDnsServiceRecordProto::target() const
{
  QDnsServiceRecord *item = qscriptvalue_cast<QDnsServiceRecord*>(thisObject());
  if (item)
    return item->target();
  return QString();
}

quint32 QDnsServiceRecordProto::timeToLive() const
{
  QDnsServiceRecord *item = qscriptvalue_cast<QDnsServiceRecord*>(thisObject());
  if (item)
    return item->timeToLive();
  return quint32();
}

quint16 QDnsServiceRecordProto::weight() const
{
  QDnsServiceRecord *item = qscriptvalue_cast<QDnsServiceRecord*>(thisObject());
  if (item)
    return item->weight();
  return quint16();
}

QString QDnsServiceRecordProto::toString() const
{
  QDnsServiceRecord *item = qscriptvalue_cast<QDnsServiceRecord*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

#endif
