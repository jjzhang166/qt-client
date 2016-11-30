/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdnslookupproto.h"

#if QT_VERSION < 0x050000
void setupQDnsLookupProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

QScriptValue DnsLookupErrorToScriptValue(QScriptEngine *engine, const QDnsLookup::Error &item)
{
  return engine->newVariant(item);
}
void DnsLookupErrorFromScriptValue(const QScriptValue &obj, QDnsLookup::Error &item)
{
  item = (QDnsLookup::Error)obj.toInt32();
}

QScriptValue DnsLookupTypeToScriptValue(QScriptEngine *engine, const QDnsLookup::Type &item)
{
  return engine->newVariant(item);
}
void DnsLookupTypeFromScriptValue(const QScriptValue &obj, QDnsLookup::Type &item)
{
  item = (QDnsLookup::Type)obj.toInt32();
}

void setupQDnsLookupProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QDnsLookupProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDnsLookup*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QDnsLookup>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDnsLookup, proto);
  engine->globalObject().setProperty("QDnsLookup",  constructor);

  qScriptRegisterMetaType(engine, DnsLookupErrorToScriptValue, DnsLookupErrorFromScriptValue);
  constructor.setProperty("NoError", QScriptValue(engine, QDnsLookup::NoError), permanent);
  constructor.setProperty("ResolverError", QScriptValue(engine, QDnsLookup::ResolverError), permanent);
  constructor.setProperty("OperationCancelledError", QScriptValue(engine, QDnsLookup::OperationCancelledError), permanent);
  constructor.setProperty("InvalidRequestError", QScriptValue(engine, QDnsLookup::InvalidRequestError), permanent);
  constructor.setProperty("InvalidReplyError", QScriptValue(engine, QDnsLookup::InvalidReplyError), permanent);
  constructor.setProperty("ServerFailureError", QScriptValue(engine, QDnsLookup::ServerFailureError), permanent);
  constructor.setProperty("ServerRefusedError", QScriptValue(engine, QDnsLookup::ServerRefusedError), permanent);
  constructor.setProperty("NotFoundError", QScriptValue(engine, QDnsLookup::NotFoundError), permanent);

  qScriptRegisterMetaType(engine, DnsLookupTypeToScriptValue, DnsLookupTypeFromScriptValue);
  constructor.setProperty("A", QScriptValue(engine, QDnsLookup::A), permanent);
  constructor.setProperty("AAAA", QScriptValue(engine, QDnsLookup::AAAA), permanent);
  constructor.setProperty("ANY", QScriptValue(engine, QDnsLookup::ANY), permanent);
  constructor.setProperty("CNAME", QScriptValue(engine, QDnsLookup::CNAME), permanent);
  constructor.setProperty("MX", QScriptValue(engine, QDnsLookup::MX), permanent);
  constructor.setProperty("NS", QScriptValue(engine, QDnsLookup::NS), permanent);
  constructor.setProperty("PTR", QScriptValue(engine, QDnsLookup::PTR), permanent);
  constructor.setProperty("SRV", QScriptValue(engine, QDnsLookup::SRV), permanent);
  constructor.setProperty("TXT", QScriptValue(engine, QDnsLookup::TXT), permanent);
}

QScriptValue constructQDnsLookup(QScriptContext *context, QScriptEngine *engine)
{
  QDnsLookup *obj = 0;
  if (context->argumentCount() == 2 &&
      context->argument(0).isNumber() &&
      context->argument(1).isString()) {

    obj = new QDnsLookup((QDnsLookup::Type)context->argument(0).toInt32(),
                         context->argument(1).toString());
  } else if (context->argumentCount() == 3 &&
             context->argument(0).isNumber() &&
             context->argument(1).isString() &&
             context->argument(2).isObject()) {

    obj = new QDnsLookup((QDnsLookup::Type)context->argument(0).toInt32(),
                         context->argument(1).toString(),
                         QHostAddress(context->argument(2).toString()));
  } else {
    obj = new QDnsLookup();
  }

  return engine->toScriptValue(obj);
}

QDnsLookupProto::QDnsLookupProto(QObject *parent) : QObject(parent)
{
}
QDnsLookupProto::~QDnsLookupProto()
{
}

QList<QDnsDomainNameRecord> QDnsLookupProto::canonicalNameRecords() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->canonicalNameRecords();
  return QList<QDnsDomainNameRecord>();
}

QDnsLookup::Error QDnsLookupProto::error() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->error();
  return QDnsLookup::Error();
}

QString QDnsLookupProto::errorString() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

QList<QDnsHostAddressRecord> QDnsLookupProto::hostAddressRecords() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->hostAddressRecords();
  return QList<QDnsHostAddressRecord>();
}

bool QDnsLookupProto::isFinished() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->isFinished();
  return false;
}

QList<QDnsMailExchangeRecord> QDnsLookupProto::mailExchangeRecords() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->mailExchangeRecords();
  return QList<QDnsMailExchangeRecord>();
}

QString QDnsLookupProto::name() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

QList<QDnsDomainNameRecord> QDnsLookupProto::nameServerRecords() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->nameServerRecords();
  return QList<QDnsDomainNameRecord>();
}

QHostAddress QDnsLookupProto::nameserver() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->nameserver();
  return QHostAddress();
}

QList<QDnsDomainNameRecord> QDnsLookupProto::pointerRecords() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->pointerRecords();
  return QList<QDnsDomainNameRecord>();
}

QList<QDnsServiceRecord> QDnsLookupProto::serviceRecords() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->serviceRecords();
  return QList<QDnsServiceRecord>();
}

void QDnsLookupProto::setName(const QString &name)
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    item->setName(name);
}

void QDnsLookupProto::setNameserver(const QHostAddress &nameserver)
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    item->setNameserver(nameserver);
}

void QDnsLookupProto::setType(QDnsLookup::Type type)
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    item->setType(type);
}

QList<QDnsTextRecord> QDnsLookupProto::textRecords() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->textRecords();
  return QList<QDnsTextRecord>();
}

QDnsLookup::Type QDnsLookupProto::type() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->type();
  return QDnsLookup::Type();
}

QString QDnsLookupProto::toString() const
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

// Slots

void QDnsLookupProto::abort()
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    item->abort();
}

void QDnsLookupProto::lookup()
{
  QDnsLookup *item = qscriptvalue_cast<QDnsLookup*>(thisObject());
  if (item)
    item->lookup();
}
#endif
