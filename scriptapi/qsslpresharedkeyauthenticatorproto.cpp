/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslpresharedkeyauthenticatorproto.h"

#if QT_VERSION < 0x050000
void setupQSslPreSharedKeyAuthenticatorProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
void setupQSslPreSharedKeyAuthenticatorProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QSslPreSharedKeyAuthenticatorProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslPreSharedKeyAuthenticator*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSslPreSharedKeyAuthenticator>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQSslPreSharedKeyAuthenticator, proto);
  engine->globalObject().setProperty("QSslPreSharedKeyAuthenticator", constructor);
}

QScriptValue constructQSslPreSharedKeyAuthenticator(QScriptContext *context, QScriptEngine *engine)
{

  QSslPreSharedKeyAuthenticator *obj = 0;
  if (context->argumentCount() == 1) {
    QSslPreSharedKeyAuthenticator sslPSK = qscriptvalue_cast<QSslPreSharedKeyAuthenticator>(context->argument(0));
    obj = new QSslPreSharedKeyAuthenticator(sslPSK);
  } else {
    obj = new QSslPreSharedKeyAuthenticator();
  }

  return engine->toScriptValue(obj);
}

QSslPreSharedKeyAuthenticatorProto::QSslPreSharedKeyAuthenticatorProto(QObject *parent)
    : QObject(parent)
{
}
QSslPreSharedKeyAuthenticatorProto::~QSslPreSharedKeyAuthenticatorProto()
{
}

QByteArray QSslPreSharedKeyAuthenticatorProto::identity() const
{
  QSslPreSharedKeyAuthenticator *item = qscriptvalue_cast<QSslPreSharedKeyAuthenticator*>(thisObject());
  if (item)
    return item->identity();
  return QByteArray();
}

QByteArray QSslPreSharedKeyAuthenticatorProto::identityHint() const
{
  QSslPreSharedKeyAuthenticator *item = qscriptvalue_cast<QSslPreSharedKeyAuthenticator*>(thisObject());
  if (item)
    return item->identityHint();
  return QByteArray();
}

int QSslPreSharedKeyAuthenticatorProto::maximumIdentityLength() const
{
  QSslPreSharedKeyAuthenticator *item = qscriptvalue_cast<QSslPreSharedKeyAuthenticator*>(thisObject());
  if (item)
    return item->maximumIdentityLength();
  return 0;
}

int QSslPreSharedKeyAuthenticatorProto::maximumPreSharedKeyLength() const
{
  QSslPreSharedKeyAuthenticator *item = qscriptvalue_cast<QSslPreSharedKeyAuthenticator*>(thisObject());
  if (item)
    return item->maximumPreSharedKeyLength();
  return 0;
}

QByteArray QSslPreSharedKeyAuthenticatorProto::preSharedKey() const
{
  QSslPreSharedKeyAuthenticator *item = qscriptvalue_cast<QSslPreSharedKeyAuthenticator*>(thisObject());
  if (item)
    return item->preSharedKey();
  return QByteArray();
}

void QSslPreSharedKeyAuthenticatorProto::setIdentity(const QByteArray & identity)
{
  QSslPreSharedKeyAuthenticator *item = qscriptvalue_cast<QSslPreSharedKeyAuthenticator*>(thisObject());
  if (item)
    item->setIdentity(identity);
}

void QSslPreSharedKeyAuthenticatorProto::setPreSharedKey(const QByteArray & preSharedKey)
{
  QSslPreSharedKeyAuthenticator *item = qscriptvalue_cast<QSslPreSharedKeyAuthenticator*>(thisObject());
  if (item)
    item->setPreSharedKey(preSharedKey);
}

void QSslPreSharedKeyAuthenticatorProto::swap(QSslPreSharedKeyAuthenticator & authenticator)
{
  QSslPreSharedKeyAuthenticator *item = qscriptvalue_cast<QSslPreSharedKeyAuthenticator*>(thisObject());
  if (item)
    item->swap(authenticator);
}

#endif
