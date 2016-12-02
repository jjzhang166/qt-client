/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslerrorproto.h"

#include <QList>
#include <QScriptValueIterator>

#if QT_VERSION < 0x050000
void setupQSslErrorProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
/*
QScriptValue QSslErrortoScriptValue(QScriptEngine *engine, QSslError* const &item)
{ return engine->newQObject(item); }

void QSslErrorfromScriptValue(const QScriptValue &obj, QSslError* &item)
{
  item = qobject_cast<QSslError*>(obj.toQObject());
}
*/
QScriptValue SslErrorToScriptValue(QScriptEngine *engine, const QSslError::SslError &item)
{
  return engine->newVariant(item);
}
void SslErrorFromScriptValue(const QScriptValue &obj, QSslError::SslError &item)
{
  item = (QSslError::SslError)obj.toInt32();
}

QScriptValue QListQSslErrortoScriptValue(QScriptEngine *engine, const QList<QSslError> &list)
{
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < list.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(list.at(i)));
  }
  return newArray;
}
void QListQSslErrorfromScriptValue(const QScriptValue &obj, QList<QSslError> &list)
{
  list = QList<QSslError>();
  QScriptValueIterator it(obj);

  while (it.hasNext()) {
    it.next();
    if (it.flags() & QScriptValue::SkipInEnumeration)
      continue;
    QSslError item = qscriptvalue_cast<QSslError>(it.value());
    list.insert(it.name().toInt(), item);
  }
}

void setupQSslErrorProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QListQSslErrortoScriptValue, QListQSslErrorfromScriptValue);
  //qScriptRegisterMetaType(engine, QSslErrortoScriptValue, QSslErrorfromScriptValue);
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QSslErrorProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslError*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSslError>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQSslError, proto);
  engine->globalObject().setProperty("QSslError",  constructor);

  qScriptRegisterMetaType(engine, SslErrorToScriptValue, SslErrorFromScriptValue);
  constructor.setProperty("NoError", QScriptValue(engine, QSslError::NoError), permanent);
  constructor.setProperty("UnableToGetIssuerCertificate", QScriptValue(engine, QSslError::UnableToGetIssuerCertificate), permanent);
  constructor.setProperty("UnableToDecryptCertificateSignature", QScriptValue(engine, QSslError::UnableToDecryptCertificateSignature), permanent);
  constructor.setProperty("UnableToDecodeIssuerPublicKey", QScriptValue(engine, QSslError::UnableToDecodeIssuerPublicKey), permanent);
  constructor.setProperty("CertificateSignatureFailed", QScriptValue(engine, QSslError::CertificateSignatureFailed), permanent);
  constructor.setProperty("CertificateNotYetValid", QScriptValue(engine, QSslError::CertificateNotYetValid), permanent);
  constructor.setProperty("CertificateExpired", QScriptValue(engine, QSslError::CertificateExpired), permanent);
  constructor.setProperty("InvalidNotBeforeField", QScriptValue(engine, QSslError::InvalidNotBeforeField), permanent);
  constructor.setProperty("InvalidNotAfterField", QScriptValue(engine, QSslError::InvalidNotAfterField), permanent);
  constructor.setProperty("SelfSignedCertificate", QScriptValue(engine, QSslError::SelfSignedCertificate), permanent);
  constructor.setProperty("SelfSignedCertificateInChain", QScriptValue(engine, QSslError::SelfSignedCertificateInChain), permanent);
  constructor.setProperty("UnableToGetLocalIssuerCertificate", QScriptValue(engine, QSslError::UnableToGetLocalIssuerCertificate), permanent);
  constructor.setProperty("UnableToVerifyFirstCertificate", QScriptValue(engine, QSslError::UnableToVerifyFirstCertificate), permanent);
  constructor.setProperty("CertificateRevoked", QScriptValue(engine, QSslError::CertificateRevoked), permanent);
  constructor.setProperty("InvalidCaCertificate", QScriptValue(engine, QSslError::InvalidCaCertificate), permanent);
  constructor.setProperty("PathLengthExceeded", QScriptValue(engine, QSslError::PathLengthExceeded), permanent);
  constructor.setProperty("InvalidPurpose", QScriptValue(engine, QSslError::InvalidPurpose), permanent);
  constructor.setProperty("CertificateUntrusted", QScriptValue(engine, QSslError::CertificateUntrusted), permanent);
  constructor.setProperty("CertificateRejected", QScriptValue(engine, QSslError::CertificateRejected), permanent);
  constructor.setProperty("SubjectIssuerMismatch", QScriptValue(engine, QSslError::SubjectIssuerMismatch), permanent);
  constructor.setProperty("AuthorityIssuerSerialNumberMismatch", QScriptValue(engine, QSslError::AuthorityIssuerSerialNumberMismatch), permanent);
  constructor.setProperty("NoPeerCertificate", QScriptValue(engine, QSslError::NoPeerCertificate), permanent);
  constructor.setProperty("HostNameMismatch", QScriptValue(engine, QSslError::HostNameMismatch), permanent);
  constructor.setProperty("UnspecifiedError", QScriptValue(engine, QSslError::UnspecifiedError), permanent);
  constructor.setProperty("NoSslSupport", QScriptValue(engine, QSslError::NoSslSupport), permanent);
  constructor.setProperty("CertificateBlacklisted", QScriptValue(engine, QSslError::CertificateBlacklisted), permanent);
}

QScriptValue constructQSslError(QScriptContext *context, QScriptEngine  *engine)
{
  QSslError *obj = 0;
  QSslError::SslError sslError;
  QString cert;
  if (context->argumentCount() == 2) {
    sslError = static_cast<QSslError::SslError>(context->argument(0).toInt32());
    cert = context->argument(1).toString();
    obj = new QSslError(sslError, QSslCertificate(cert.toLocal8Bit(), QSsl::Pem));
  }
  else if (context->argumentCount() == 1) {
    QScriptValue arg = context->argument(0);
    if (arg.isString() or arg.isNumber()) {
      sslError = static_cast<QSslError::SslError>(context->argument(0).toInt32());
      obj = new QSslError(sslError);
    } else {
      obj = new QSslError(qscriptvalue_cast<QSslError>(context->argument(0)));
    }
  }
  else {
    obj = new QSslError();
  }

  return engine->toScriptValue(obj);
}

QSslErrorProto::QSslErrorProto(QObject *parent)
    : QObject(parent)
{
}
QSslErrorProto::~QSslErrorProto()
{
}

QSslCertificate QSslErrorProto::certificate() const
{
  QSslError *item = qscriptvalue_cast<QSslError*>(thisObject());
  if (item)
    return item->certificate();
  return QSslCertificate();
}

QSslError::SslError QSslErrorProto::error() const
{
  QSslError *item = qscriptvalue_cast<QSslError*>(thisObject());
  if (item)
    return item->error();
  return QSslError::SslError();
}

QString QSslErrorProto::errorString() const
{
  QSslError *item = qscriptvalue_cast<QSslError*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

void QSslErrorProto::swap(QSslError & other)
{
  QSslError *item = qscriptvalue_cast<QSslError*>(thisObject());
  if (item)
    item->swap(other);
}

QString QSslErrorProto::toString() const
{
  QSslError *item = qscriptvalue_cast<QSslError*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

#endif
