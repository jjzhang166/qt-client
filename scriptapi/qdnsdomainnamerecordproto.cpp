/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdnsdomainnamerecordproto.h"

#if QT_VERSION < 0x050000
void setupQDnsDomainNameRecordProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

QScriptValue QListQDnsDomainNameRecordToScriptValue(QScriptEngine *engine, const QList<QDnsDomainNameRecord> &list)
{
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < list.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(list.at(i)));
  }
  return newArray;
}
void QListQDnsDomainNameRecordFromScriptValue(const QScriptValue &obj, QList<QDnsDomainNameRecord> &list)
{
  list = QList<QDnsDomainNameRecord>();

  if (obj.isArray()) {
    QScriptValueIterator it(obj);

    while (it.hasNext()) {
      it.next();
      if (it.flags() & QScriptValue::SkipInEnumeration)
        continue;

      if (it.value().isString()) {
        QDnsDomainNameRecord item = qscriptvalue_cast<QDnsDomainNameRecord>(it.value());
        list.insert(it.name().toInt(), item);
      }
    }
  }
}

void setupQDnsDomainNameRecordProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDnsDomainNameRecordProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDnsDomainNameRecord*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDnsDomainNameRecord>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDnsDomainNameRecord, proto);
  engine->globalObject().setProperty("QDnsDomainNameRecord",  constructor);

  qScriptRegisterMetaType(engine, QListQDnsDomainNameRecordToScriptValue, QListQDnsDomainNameRecordFromScriptValue);
}

QScriptValue constructQDnsDomainNameRecord(QScriptContext *context, QScriptEngine *engine)
{
  QDnsDomainNameRecord *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isObject()) {
    obj = new QDnsDomainNameRecord(qscriptvalue_cast<QDnsDomainNameRecord>(context->argument(0)));
  } else {
    obj = new QDnsDomainNameRecord();
  }

  return engine->toScriptValue(obj);
}

QDnsDomainNameRecordProto::QDnsDomainNameRecordProto(QObject *parent) : QObject(parent)
{
}
QDnsDomainNameRecordProto::~QDnsDomainNameRecordProto()
{
}

QString QDnsDomainNameRecordProto::name() const
{
  QDnsDomainNameRecord *item = qscriptvalue_cast<QDnsDomainNameRecord*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

void QDnsDomainNameRecordProto::swap(QDnsDomainNameRecord &other)
{
  QDnsDomainNameRecord *item = qscriptvalue_cast<QDnsDomainNameRecord*>(thisObject());
  if (item)
    item->swap(other);
}

quint32 QDnsDomainNameRecordProto::timeToLive() const
{
  QDnsDomainNameRecord *item = qscriptvalue_cast<QDnsDomainNameRecord*>(thisObject());
  if (item)
    return item->timeToLive();
  return quint32();
}

QString QDnsDomainNameRecordProto::value() const
{
  QDnsDomainNameRecord *item = qscriptvalue_cast<QDnsDomainNameRecord*>(thisObject());
  if (item)
    return item->value();
  return QString();
}

QString QDnsDomainNameRecordProto::toString() const
{
  QDnsDomainNameRecord *item = qscriptvalue_cast<QDnsDomainNameRecord*>(thisObject());
  if (item)
    return item->value();
  return QString();
}

#endif
