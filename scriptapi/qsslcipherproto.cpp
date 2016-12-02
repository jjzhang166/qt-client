/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslcipherproto.h"
#include <QScriptValueIterator>

#if QT_VERSION < 0x050000
void setupQSslCipherProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue QListQSslCipherToScriptValue(QScriptEngine *engine, const QList<QSslCipher> &list)
{
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < list.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(list.at(i)));
  }
  return newArray;
}
void QListQSslCipherFromScriptValue(const QScriptValue &obj, QList<QSslCipher> &list)
{
  list = QList<QSslCipher>();
  QScriptValueIterator it(obj);

  while (it.hasNext()) {
    it.next();
    if (it.flags() & QScriptValue::SkipInEnumeration)
      continue;
    QSslCipher item = qscriptvalue_cast<QSslCipher>(it.value());
    list.insert(it.name().toInt(), item);
  }
}

void setupQSslCipherProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QSslCipherProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslCipher*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSslCipher>(), proto);

  QScriptValue constructor = engine->newFunction(constructQSslCipher, proto);
  engine->globalObject().setProperty("QSslCipher",  constructor);

  qScriptRegisterMetaType(engine, QListQSslCipherToScriptValue, QListQSslCipherFromScriptValue);
}

QScriptValue constructQSslCipher(QScriptContext *context,
                                    QScriptEngine  *engine)
{
  QSslCipher *obj = 0;
  if (context->argumentCount() == 1) {
    QScriptValue arg = context->argument(0);
    if (arg.isString()) {
      obj = new QSslCipher(arg.toString());
    } else {
      obj = new QSslCipher(qscriptvalue_cast<QSslCipher>(arg));
    }
  }
  else if (context->argumentCount() == 2) {
    obj = new QSslCipher(context->argument(0).toString(), (QSsl::SslProtocol)context->argument(1).toInt32());
  }
  else {
    obj = new QSslCipher();
  }
  return engine->toScriptValue(obj);
}

QSslCipherProto::QSslCipherProto(QObject *parent)
    : QObject(parent)
{
}
QSslCipherProto::~QSslCipherProto()
{
}

QString QSslCipherProto::authenticationMethod() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->authenticationMethod();
  return QString();
}

QString QSslCipherProto::encryptionMethod() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->encryptionMethod();
  return QString();
}

bool QSslCipherProto::isNull() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

QString QSslCipherProto::keyExchangeMethod() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->keyExchangeMethod();
  return QString();
}

QString QSslCipherProto::name() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

QSsl::SslProtocol QSslCipherProto::protocol() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->protocol();
  return QSsl::SslProtocol();
}

QString QSslCipherProto::protocolString() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->protocolString();
  return QString();
}

int QSslCipherProto::supportedBits() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->supportedBits();
  return 0;
}

void QSslCipherProto::swap(QSslCipher & other)
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    item->swap(other);
}

int QSslCipherProto::usedBits() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->usedBits();
  return 0;
}

#endif
