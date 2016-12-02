/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qcryptographichashproto.h"

#if QT_VERSION < 0x050000
void setupQCryptographicHashProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue AlgorithmToScriptValue(QScriptEngine *engine, const QCryptographicHash::Algorithm &item)
{
  return engine->newVariant(item);
}
void AlgorithmFromScriptValue(const QScriptValue &obj, QCryptographicHash::Algorithm &item)
{
  item = (QCryptographicHash::Algorithm)obj.toInt32();
}

QScriptValue hashForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 2) {
    QByteArray data = context->argument(0).toVariant().toByteArray();
    QCryptographicHash::Algorithm method = (QCryptographicHash::Algorithm)context->argument(1).toInt32();
    return engine->toScriptValue(QCryptographicHash::hash(data, method));
  } else {
    return engine->undefinedValue();
  }
}

void setupQCryptographicHashProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QCryptographicHashProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QCryptographicHash*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQCryptographicHash, proto);
  engine->globalObject().setProperty("QCryptographicHash", constructor);

  qScriptRegisterMetaType(engine, AlgorithmToScriptValue, AlgorithmFromScriptValue);
  constructor.setProperty("Md4", QScriptValue(engine, QCryptographicHash::Md4), permanent);
  constructor.setProperty("Md5", QScriptValue(engine, QCryptographicHash::Md5), permanent);
  constructor.setProperty("Sha1", QScriptValue(engine, QCryptographicHash::Sha1), permanent);
  constructor.setProperty("Sha224", QScriptValue(engine, QCryptographicHash::Sha224), permanent);
  constructor.setProperty("Sha256", QScriptValue(engine, QCryptographicHash::Sha256), permanent);
  constructor.setProperty("Sha384", QScriptValue(engine, QCryptographicHash::Sha384), permanent);
  constructor.setProperty("Sha512", QScriptValue(engine, QCryptographicHash::Sha512), permanent);
  constructor.setProperty("Sha3_224", QScriptValue(engine, QCryptographicHash::Sha3_224), permanent);
  constructor.setProperty("Sha3_256", QScriptValue(engine, QCryptographicHash::Sha3_256), permanent);
  constructor.setProperty("Sha3_384", QScriptValue(engine, QCryptographicHash::Sha3_384), permanent);
  constructor.setProperty("Sha3_512", QScriptValue(engine, QCryptographicHash::Sha3_512), permanent);

  QScriptValue hash = engine->newFunction(hashForJS);
  constructor.setProperty("hash", hash);
}

QScriptValue constructQCryptographicHash(QScriptContext *context, QScriptEngine  *engine)
{
  QCryptographicHash *obj = 0;
  if (context->argumentCount() == 1) {
    QCryptographicHash::Algorithm method = (QCryptographicHash::Algorithm)context->argument(0).toInt32();
    obj = new QCryptographicHash(method);
  } else {
    context->throwError(QScriptContext::UnknownError,
                        "No Algorithm provided to QCryptographicHash");
  }

  return engine->toScriptValue(obj);
}

QCryptographicHashProto::QCryptographicHashProto(QObject *parent)
    : QObject(parent)
{
}
QCryptographicHashProto::~QCryptographicHashProto()
{
}

void QCryptographicHashProto::addData(const char * data, int length)
{
  QCryptographicHash *item = qscriptvalue_cast<QCryptographicHash*>(thisObject());
  if (item)
    item->addData(data, length);
}

void QCryptographicHashProto::addData(const QByteArray & data)
{
  QCryptographicHash *item = qscriptvalue_cast<QCryptographicHash*>(thisObject());
  if (item)
    item->addData(data);
}

bool QCryptographicHashProto::addData(QIODevice * device)
{
  QCryptographicHash *item = qscriptvalue_cast<QCryptographicHash*>(thisObject());
  if (item)
    return item->addData(device);
  return false;
}

void QCryptographicHashProto::reset()
{
  QCryptographicHash *item = qscriptvalue_cast<QCryptographicHash*>(thisObject());
  if (item)
    item->reset();
}

QByteArray QCryptographicHashProto::result() const
{
  QCryptographicHash *item = qscriptvalue_cast<QCryptographicHash*>(thisObject());
  if (item)
    return item->result();
  return QByteArray();
}

#endif
