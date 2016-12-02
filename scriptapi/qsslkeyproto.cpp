/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslkeyproto.h"

#if QT_VERSION < 0x050000
void setupQSslKeyProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
void setupQSslKeyProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QSslKeyProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslKey*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSslKey>(), proto);

  QScriptValue constructor = engine->newFunction(constructQSslKey, proto);
  engine->globalObject().setProperty("QSslKey",  constructor);
}

QScriptValue constructQSslKey(QScriptContext *context,
                                    QScriptEngine  *engine)
{
  QSslKey *obj = 0;
  QString key;
  QSsl::KeyAlgorithm algorithm;
  QSsl::EncodingFormat encoding;
  QSsl::KeyType type;
  QByteArray passPhrase;

  if (context->argumentCount() == 1) { // Handle `const QSslKey & other`.
    QSslKey paramKey = qscriptvalue_cast<QSslKey>(context->argument(0));
    obj = new QSslKey(paramKey);
  } else if (context->argumentCount() >= 2) {
    QScriptValue arg = context->argument(0);
    // TODO: How to check is this is a QByteArray or QIODevice*???
    key = arg.toString();
    algorithm = static_cast<QSsl::KeyAlgorithm>(context->argument(1).toInt32());

    if (key.length() > 0) { // Handle `const QByteArray & encoded`.
      QByteArray encoded = arg.toVariant().toByteArray();
      if (context->argumentCount() == 2) {
        obj = new QSslKey(encoded, algorithm);
      } else if (context->argumentCount() == 3) {
        encoding = static_cast<QSsl::EncodingFormat>(context->argument(2).toInt32());
        obj = new QSslKey(encoded, algorithm, encoding);
      } else if (context->argumentCount() == 4) {
        encoding = static_cast<QSsl::EncodingFormat>(context->argument(2).toInt32());
        type = static_cast<QSsl::KeyType>(context->argument(3).toInt32());
        obj = new QSslKey(encoded, algorithm, encoding, type);
      } else if (context->argumentCount() == 5) {
        encoding = static_cast<QSsl::EncodingFormat>(context->argument(2).toInt32());
        type = static_cast<QSsl::KeyType>(context->argument(3).toInt32());
        passPhrase = context->argument(4).toVariant().toByteArray();
        obj = new QSslKey(encoded, algorithm, encoding, type, passPhrase);
      }
    }
    // TODO: Something is wrong with how we expose QIODevice
    /*
    else { // Handle `QIODevice * device`.
      QIODevice *device = qscriptvalue_cast<QIODevice*>(arg);
      if (context->argumentCount() == 2) {
        obj = new QSslKey(device, algorithm);
      } else if (context->argumentCount() == 3) {
        encoding = static_cast<QSsl::EncodingFormat>(context->argument(2).toInt32());
        obj = new QSslKey(device, algorithm, encoding);
      } else if (context->argumentCount() == 4) {
        encoding = static_cast<QSsl::EncodingFormat>(context->argument(2).toInt32());
        type = static_cast<QSsl::KeyType>(context->argument(3).toInt32());
        obj = new QSslKey(device, algorithm, encoding, type);
      } else if (context->argumentCount() == 5) {
        encoding = static_cast<QSsl::EncodingFormat>(context->argument(2).toInt32());
        type = static_cast<QSsl::KeyType>(context->argument(3).toInt32());
        passPhrase = context->argument(4).toVariant().toByteArray();
        obj = new QSslKey(device, algorithm, encoding, type, passPhrase);
      }
    }
    */
  } else {
    context->throwError(QScriptContext::UnknownError,
                        "No SSL Key provided to QSslKey");
  }

  return engine->toScriptValue(obj);
}

QSslKeyProto::QSslKeyProto(QObject *parent) : QObject(parent)
{
}
QSslKeyProto::~QSslKeyProto()
{
}

QSsl::KeyAlgorithm QSslKeyProto::algorithm() const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->algorithm();
  return QSsl::KeyAlgorithm();
}

void QSslKeyProto::clear()
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    item->clear();
}

bool QSslKeyProto::isNull() const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

int QSslKeyProto::length() const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->length();
  return 0;
}

void QSslKeyProto::swap(QSslKey & other)
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    item->swap(other);
}

QByteArray QSslKeyProto::toDer(const QByteArray & passPhrase) const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->toDer(passPhrase);
  return QByteArray();
}

QByteArray QSslKeyProto::toPem(const QByteArray & passPhrase) const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->toPem(passPhrase);
  return QByteArray();
}

QSsl::KeyType QSslKeyProto::type() const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->type();
  return QSsl::KeyType();
}
#endif
