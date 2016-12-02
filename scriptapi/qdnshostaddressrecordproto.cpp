/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdnshostaddressrecordproto.h"

#if QT_VERSION < 0x050000
void setupQDnsHostAddressRecordProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

QScriptValue QListQDnsHostAddressRecordToScriptValue(QScriptEngine *engine, const QList<QDnsHostAddressRecord> &list)
{
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < list.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(list.at(i)));
  }
  return newArray;
}
void QListQDnsHostAddressRecordFromScriptValue(const QScriptValue &obj, QList<QDnsHostAddressRecord> &list)
{
  list = QList<QDnsHostAddressRecord>();

  if (obj.isArray()) {
    QScriptValueIterator it(obj);

    while (it.hasNext()) {
      it.next();
      if (it.flags() & QScriptValue::SkipInEnumeration)
        continue;

      if (it.value().isString()) {
        QDnsHostAddressRecord item = qscriptvalue_cast<QDnsHostAddressRecord>(it.value());
        list.insert(it.name().toInt(), item);
      }
    }
  }
}

void setupQDnsHostAddressRecordProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDnsHostAddressRecordProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDnsHostAddressRecord*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDnsHostAddressRecord>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDnsHostAddressRecord, proto);
  engine->globalObject().setProperty("QDnsHostAddressRecord",  constructor);

  qScriptRegisterMetaType(engine, QListQDnsHostAddressRecordToScriptValue, QListQDnsHostAddressRecordFromScriptValue);
}

QScriptValue constructQDnsHostAddressRecord(QScriptContext *context, QScriptEngine *engine)
{
  QDnsHostAddressRecord *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isObject()) {
    obj = new QDnsHostAddressRecord(qscriptvalue_cast<QDnsHostAddressRecord>(context->argument(0)));
  } else {
    obj = new QDnsHostAddressRecord();
  }

  return engine->toScriptValue(obj);
}

QDnsHostAddressRecordProto::QDnsHostAddressRecordProto(QObject *parent) : QObject(parent)
{
}
QDnsHostAddressRecordProto::~QDnsHostAddressRecordProto()
{
}

QString QDnsHostAddressRecordProto::name() const
{
  QDnsHostAddressRecord *item = qscriptvalue_cast<QDnsHostAddressRecord*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

void QDnsHostAddressRecordProto::swap(QDnsHostAddressRecord &other)
{
  QDnsHostAddressRecord *item = qscriptvalue_cast<QDnsHostAddressRecord*>(thisObject());
  if (item)
    item->swap(other);
}

quint32 QDnsHostAddressRecordProto::timeToLive() const
{
  QDnsHostAddressRecord *item = qscriptvalue_cast<QDnsHostAddressRecord*>(thisObject());
  if (item)
    return item->timeToLive();
  return quint32();
}

QHostAddress QDnsHostAddressRecordProto::value() const
{
  QDnsHostAddressRecord *item = qscriptvalue_cast<QDnsHostAddressRecord*>(thisObject());
  if (item)
    return item->value();
  return QHostAddress();
}

QString QDnsHostAddressRecordProto::toString() const
{
  QDnsHostAddressRecord *item = qscriptvalue_cast<QDnsHostAddressRecord*>(thisObject());
  if (item)
    return item->value().toString();
  return QString();
}

#endif
