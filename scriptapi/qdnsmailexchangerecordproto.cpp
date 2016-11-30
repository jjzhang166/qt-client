/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdnsmailexchangerecordproto.h"

#if QT_VERSION < 0x050000
void setupQDnsMailExchangeRecordProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

QScriptValue QListQDnsMailExchangeRecordToScriptValue(QScriptEngine *engine, const QList<QDnsMailExchangeRecord> &list)
{
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < list.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(list.at(i)));
  }
  return newArray;
}
void QListQDnsMailExchangeRecordFromScriptValue(const QScriptValue &obj, QList<QDnsMailExchangeRecord> &list)
{
  list = QList<QDnsMailExchangeRecord>();

  if (obj.isArray()) {
    QScriptValueIterator it(obj);

    while (it.hasNext()) {
      it.next();
      if (it.flags() & QScriptValue::SkipInEnumeration)
        continue;

      if (it.value().isString()) {
        QDnsMailExchangeRecord item = qscriptvalue_cast<QDnsMailExchangeRecord>(it.value());
        list.insert(it.name().toInt(), item);
      }
    }
  }
}

void setupQDnsMailExchangeRecordProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDnsMailExchangeRecordProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDnsMailExchangeRecord*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDnsMailExchangeRecord>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDnsMailExchangeRecord, proto);
  engine->globalObject().setProperty("QDnsMailExchangeRecord",  constructor);

  qScriptRegisterMetaType(engine, QListQDnsMailExchangeRecordToScriptValue, QListQDnsMailExchangeRecordFromScriptValue);
}

QScriptValue constructQDnsMailExchangeRecord(QScriptContext *context, QScriptEngine *engine)
{
  QDnsMailExchangeRecord *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isObject()) {
    obj = new QDnsMailExchangeRecord(qscriptvalue_cast<QDnsMailExchangeRecord>(context->argument(0)));
  } else {
    obj = new QDnsMailExchangeRecord();
  }

  return engine->toScriptValue(obj);
}

QDnsMailExchangeRecordProto::QDnsMailExchangeRecordProto(QObject *parent) : QObject(parent)
{
}
QDnsMailExchangeRecordProto::~QDnsMailExchangeRecordProto()
{
}

QString QDnsMailExchangeRecordProto::exchange() const
{
  QDnsMailExchangeRecord *item = qscriptvalue_cast<QDnsMailExchangeRecord*>(thisObject());
  if (item)
    return item->exchange();
  return QString();
}

QString QDnsMailExchangeRecordProto::name() const
{
  QDnsMailExchangeRecord *item = qscriptvalue_cast<QDnsMailExchangeRecord*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

quint16 QDnsMailExchangeRecordProto::preference() const
{
  QDnsMailExchangeRecord *item = qscriptvalue_cast<QDnsMailExchangeRecord*>(thisObject());
  if (item)
    return item->preference();
  return quint16();
}

void QDnsMailExchangeRecordProto::swap(QDnsMailExchangeRecord &other)
{
  QDnsMailExchangeRecord *item = qscriptvalue_cast<QDnsMailExchangeRecord*>(thisObject());
  if (item)
    item->swap(other);
}

quint32 QDnsMailExchangeRecordProto::timeToLive() const
{
  QDnsMailExchangeRecord *item = qscriptvalue_cast<QDnsMailExchangeRecord*>(thisObject());
  if (item)
    return item->timeToLive();
  return quint32();
}

QString QDnsMailExchangeRecordProto::toString() const
{
  QDnsMailExchangeRecord *item = qscriptvalue_cast<QDnsMailExchangeRecord*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

#endif
