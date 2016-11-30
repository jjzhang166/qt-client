/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslcertificateproto.h"
#include <QScriptValueIterator>

#if QT_VERSION < 0x050000
void setupQSslCertificateProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue SubjectInfoToScriptValue(QScriptEngine *engine, const QSslCertificate::SubjectInfo &item)
{
  return engine->newVariant(item);
}
void SubjectInfoFromScriptValue(const QScriptValue &obj, QSslCertificate::SubjectInfo &item)
{
  item = (QSslCertificate::SubjectInfo)obj.toInt32();
}

QScriptValue QListQSslCertificatetoScriptValue(QScriptEngine *engine, const QList<QSslCertificate> &list)
{
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < list.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(list.at(i)));
  }
  return newArray;
}
void QListQSslCertificatefromScriptValue(const QScriptValue &obj, QList<QSslCertificate> &list)
{
  list = QList<QSslCertificate>();
  QScriptValueIterator it(obj);

  while (it.hasNext()) {
    it.next();
    if (it.flags() & QScriptValue::SkipInEnumeration)
      continue;
    QSslCertificate item = qscriptvalue_cast<QSslCertificate>(it.value());
    list.insert(it.name().toInt(), item);
  }
}

QScriptValue fromDataForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QByteArray data = context->argument(0).toVariant().toByteArray();
    return engine->toScriptValue(QSslCertificate::fromData(data));
  } else if (context->argumentCount() == 2) {
    QByteArray data = context->argument(0).toVariant().toByteArray();
    QSsl::EncodingFormat format = (QSsl::EncodingFormat)context->argument(1).toInt32();
    return engine->toScriptValue(QSslCertificate::fromData(data, format));
  } else {
    return engine->undefinedValue();
  }
}

// TODO: Can't qscriptvalue_cast<QIODevice*>(context->argument(0));
/*
QScriptValue fromDeviceForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QIODevice *device = qscriptvalue_cast<QIODevice*>(context->argument(0));
    return engine->toScriptValue(QSslCertificate::fromDevice(device));
  } else if (context->argumentCount() == 2) {
    QIODevice *device = qscriptvalue_cast<QIODevice*>(context->argument(0));
    QSsl::EncodingFormat format = (QSsl::EncodingFormat)context->argument(1).toInt32();
    return engine->toScriptValue(QSslCertificate::fromDevice(device, format));
  } else {
    return engine->undefinedValue();
  }
}
*/

QScriptValue fromPathForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QString path = context->argument(0).toString();
    return engine->toScriptValue(QSslCertificate::fromPath(path));
  } else if (context->argumentCount() == 2) {
    QString path = context->argument(0).toString();
    QSsl::EncodingFormat format = (QSsl::EncodingFormat)context->argument(1).toInt32();
    return engine->toScriptValue(QSslCertificate::fromPath(path, format));
  } else if (context->argumentCount() == 3) {
    QString path = context->argument(0).toString();
    QSsl::EncodingFormat format = (QSsl::EncodingFormat)context->argument(1).toInt32();
    QRegExp::PatternSyntax syntax = (QRegExp::PatternSyntax)context->argument(2).toInt32();
    return engine->toScriptValue(QSslCertificate::fromPath(path, format, syntax));
  } else {
    return engine->undefinedValue();
  }
}

// TODO: Something is wrong with how we expose QIODevice
/*
QScriptValue importPkcs12ForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 3) {
    QIODevice *device = qscriptvalue_cast<QIODevice*>(context->argument(0));
    QSslKey *key = qscriptvalue_cast<QSslKey*>(context->argument(1));
    QSslCertificate *certificate = qscriptvalue_cast<QSslCertificate*>(context->argument(2));
    return engine->toScriptValue(QSslCertificate::importPkcs12(device, key, certificate));
  } else if (context->argumentCount() == 4) {
    QIODevice *device = qscriptvalue_cast<QIODevice*>(context->argument(0));
    QSslKey *key = qscriptvalue_cast<QSslKey*>(context->argument(1));
    QSslCertificate *certificate = qscriptvalue_cast<QSslCertificate*>(context->argument(2));
    QList<QSslCertificate> *caCertificates = qscriptvalue_cast<QList<QSslCertificate>*>(context->argument(3));
    return engine->toScriptValue(QSslCertificate::importPkcs12(device, key, certificate, caCertificates));
  } else if (context->argumentCount() == 5) {
    QIODevice *device = qscriptvalue_cast<QIODevice*>(context->argument(0));
    QSslKey *key = qscriptvalue_cast<QSslKey*>(context->argument(1));
    QSslCertificate *certificate = qscriptvalue_cast<QSslCertificate*>(context->argument(2));
    QList<QSslCertificate> *caCertificates = qscriptvalue_cast<QList<QSslCertificate>*>(context->argument(3));
    QByteArray passPhrase = context->argument(4).toVariant().toByteArray();
    return engine->toScriptValue(QSslCertificate::importPkcs12(device, key, certificate, caCertificates, passPhrase));
  } else {
    return engine->toScriptValue(false);
  }
}
*/

QScriptValue verifyForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 2) {
    QList<QSslCertificate> certificateChain = qscriptvalue_cast< QList<QSslCertificate> >(context->argument(0));
    QString hostName = context->argument(1).toString();
    return engine->toScriptValue(QSslCertificate::verify(certificateChain, hostName));
  } else {
    return engine->undefinedValue();
  }
}

void setupQSslCertificateProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QSslCertificateProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslCertificate*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSslCertificate>(), proto);

  QScriptValue constructor = engine->newFunction(constructQSslCertificate, proto);
  engine->globalObject().setProperty("QSslCertificate",  constructor);

  qScriptRegisterMetaType(engine, QListQSslCertificatetoScriptValue, QListQSslCertificatefromScriptValue);

  qScriptRegisterMetaType(engine, SubjectInfoToScriptValue, SubjectInfoFromScriptValue);
  constructor.setProperty("Organization",               QScriptValue(engine, QSslCertificate::Organization), permanent);
  constructor.setProperty("CommonName",                 QScriptValue(engine, QSslCertificate::CommonName), permanent);
  constructor.setProperty("LocalityName",               QScriptValue(engine, QSslCertificate::LocalityName), permanent);
  constructor.setProperty("OrganizationalUnitName",     QScriptValue(engine, QSslCertificate::OrganizationalUnitName), permanent);
  constructor.setProperty("CountryName",                QScriptValue(engine, QSslCertificate::CountryName), permanent);
  constructor.setProperty("StateOrProvinceName",        QScriptValue(engine, QSslCertificate::StateOrProvinceName), permanent);
  constructor.setProperty("DistinguishedNameQualifier", QScriptValue(engine, QSslCertificate::DistinguishedNameQualifier), permanent);
  constructor.setProperty("SerialNumber",               QScriptValue(engine, QSslCertificate::SerialNumber), permanent);
  constructor.setProperty("EmailAddress",               QScriptValue(engine, QSslCertificate::EmailAddress), permanent);

  QScriptValue fromData = engine->newFunction(fromDataForJS);
  constructor.setProperty("fromData", fromData);
  // TODO: Can't qscriptvalue_cast<QIODevice*>(context->argument(0));
  /*
  QScriptValue fromDevice = engine->newFunction(fromDeviceForJS);
  constructor.setProperty("fromDevice", fromDevice);
  */
  QScriptValue fromPath = engine->newFunction(fromPathForJS);
  constructor.setProperty("fromPath", fromPath);
  // TODO: Something is wrong with how we expose QIODevice
  /*
  QScriptValue importPkcs12 = engine->newFunction(importPkcs12ForJS);
  constructor.setProperty("importPkcs12", importPkcs12);
  */
  QScriptValue verify = engine->newFunction(verifyForJS);
  constructor.setProperty("verify", verify);
}

#include <QSslCertificate>
QScriptValue constructQSslCertificate(QScriptContext *context, QScriptEngine *engine)
{
  QSslCertificate *obj = 0;
  QString cert;
  if (context->argumentCount() == 1)
  {
    cert = context->argument(0).toString();
    obj = new QSslCertificate(cert.toLocal8Bit(), QSsl::Pem);
  }
  else
    context->throwError(QScriptContext::UnknownError,
                        "No SSL Certificate provided to QSslCertificate");

  return engine->toScriptValue(obj);
}

QSslCertificateProto::QSslCertificateProto(QObject *parent)
    : QObject(parent)
{
}

QSslCertificateProto::~QSslCertificateProto()
{
}

void QSslCertificateProto::clear()
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    item->clear();
}

QByteArray QSslCertificateProto::digest(QCryptographicHash::Algorithm algorithm) const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->digest(algorithm);
  return QByteArray();
}

QString QSslCertificateProto::effectiveDate() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->effectiveDate().toString();
  return QString();
}

QString QSslCertificateProto::expiryDate() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->expiryDate().toString();
  return QString();
}

QList<QSslCertificateExtension> QSslCertificateProto::extensions() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->extensions();
  return QList<QSslCertificateExtension>();
}

bool QSslCertificateProto::isBlacklisted() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->isBlacklisted();
  return false;
}

bool QSslCertificateProto::isNull() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

bool QSslCertificateProto::isSelfSigned() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->isSelfSigned();
  return false;
}

QStringList QSslCertificateProto::issuerInfo(QSslCertificate::SubjectInfo subject) const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->issuerInfo(subject);
  return QStringList();
}

QStringList QSslCertificateProto::issuerInfo(const QByteArray & attribute) const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->issuerInfo(attribute);
  return QStringList();
}

QList<QByteArray> QSslCertificateProto::issuerInfoAttributes() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->issuerInfoAttributes();
  return QList<QByteArray>();
}

QSslKey QSslCertificateProto::publicKey() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->publicKey();
  return QSslKey();
}

QByteArray QSslCertificateProto::serialNumber() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->serialNumber();
  return QByteArray();
}

QMultiMap<QSsl::AlternativeNameEntryType, QString> QSslCertificateProto::subjectAlternativeNames() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->subjectAlternativeNames();
  return QMultiMap<QSsl::AlternativeNameEntryType, QString>();
}

QStringList QSslCertificateProto::subjectInfo(QSslCertificate::SubjectInfo subject) const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->subjectInfo(subject);
  return QStringList();
}

QStringList QSslCertificateProto::subjectInfo(const QByteArray & attribute) const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->subjectInfo(attribute);
  return QStringList();
}

QList<QByteArray> QSslCertificateProto::subjectInfoAttributes() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->subjectInfoAttributes();
  return QList<QByteArray>();
}

void QSslCertificateProto::swap(QSslCertificate & other)
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    item->swap(other);
}

QByteArray QSslCertificateProto::toDer() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->toDer();
  return QByteArray();
}

QByteArray QSslCertificateProto::toPem() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->toPem();
  return QByteArray();
}

QString QSslCertificateProto::toText() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->toText();
  return QString();
}

QByteArray QSslCertificateProto::version() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->version();
  return QByteArray();
}

#endif
