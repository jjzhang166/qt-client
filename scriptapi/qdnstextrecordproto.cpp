/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdnstextrecordproto.h"

#if QT_VERSION < 0x050000
void setupQDnsTextRecordProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

QScriptValue QListQDnsTextRecordToScriptValue(QScriptEngine *engine, const QList<QDnsTextRecord> &list)
{
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < list.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(list.at(i)));
  }
  return newArray;
}
void QListQDnsTextRecordFromScriptValue(const QScriptValue &obj, QList<QDnsTextRecord> &list)
{
  list = QList<QDnsTextRecord>();

  if (obj.isArray()) {
    QScriptValueIterator it(obj);

    while (it.hasNext()) {
      it.next();
      if (it.flags() & QScriptValue::SkipInEnumeration)
        continue;

      if (it.value().isString()) {
        QDnsTextRecord item = qscriptvalue_cast<QDnsTextRecord>(it.value());
        list.insert(it.name().toInt(), item);
      }
    }
  }
}

void setupQDnsTextRecordProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDnsTextRecordProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDnsTextRecord*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDnsTextRecord>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDnsTextRecord, proto);
  engine->globalObject().setProperty("QDnsTextRecord",  constructor);

  qScriptRegisterMetaType(engine, QListQDnsTextRecordToScriptValue, QListQDnsTextRecordFromScriptValue);
}

QScriptValue constructQDnsTextRecord(QScriptContext *context, QScriptEngine *engine)
{
  QDnsTextRecord *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isObject()) {
    obj = new QDnsTextRecord(qscriptvalue_cast<QDnsTextRecord>(context->argument(0)));
  } else {
    obj = new QDnsTextRecord();
  }

  return engine->toScriptValue(obj);
}

QDnsTextRecordProto::QDnsTextRecordProto(QObject *parent) : QObject(parent)
{
}
QDnsTextRecordProto::~QDnsTextRecordProto()
{
}

QString QDnsTextRecordProto::name() const
{
  QDnsTextRecord *item = qscriptvalue_cast<QDnsTextRecord*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

void QDnsTextRecordProto::swap(QDnsTextRecord &other)
{
  QDnsTextRecord *item = qscriptvalue_cast<QDnsTextRecord*>(thisObject());
  if (item)
    item->swap(other);
}

quint32 QDnsTextRecordProto::timeToLive() const
{
  QDnsTextRecord *item = qscriptvalue_cast<QDnsTextRecord*>(thisObject());
  if (item)
    return item->timeToLive();
  return quint32();
}

QList<QByteArray> QDnsTextRecordProto::values() const
{
  QDnsTextRecord *item = qscriptvalue_cast<QDnsTextRecord*>(thisObject());
  if (item)
    return item->values();
  return QList<QByteArray>();
}

QString QDnsTextRecordProto::toString() const
{
  QDnsTextRecord *item = qscriptvalue_cast<QDnsTextRecord*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

#endif
