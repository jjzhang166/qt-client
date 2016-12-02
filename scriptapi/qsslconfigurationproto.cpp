/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslconfigurationproto.h"
#include <QSslCipher>

#if QT_VERSION < 0x050000
void setupQSslConfigurationProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue NextProtocolNegotiationStatusToScriptValue(QScriptEngine *engine, const QSslConfiguration::NextProtocolNegotiationStatus &item)
{
  return engine->newVariant(item);
}
void NextProtocolNegotiationStatusFromScriptValue(const QScriptValue &obj, QSslConfiguration::NextProtocolNegotiationStatus &item)
{
  item = (QSslConfiguration::NextProtocolNegotiationStatus)obj.toInt32();
}

QScriptValue defaultConfigurationForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  QSslConfiguration defaultConfiguration = QSslConfiguration::defaultConfiguration();
  return engine->toScriptValue(defaultConfiguration);
}

QScriptValue setDefaultConfigurationForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QSslConfiguration configuration = qscriptvalue_cast<QSslConfiguration>(context->argument(0));
    QSslConfiguration::setDefaultConfiguration(configuration);
  }
  return engine->undefinedValue();
}

// TODO: Something is wrong with how we expose QSslCipher.
/*
QScriptValue supportedCiphersForJS(QScriptContext* context, QScriptEngine* engine)
{
  QList<QSslCipher> ciphers = QSslConfiguration::supportedCiphers();
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < ciphers.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(ciphers.at(i)));
  }
  return newArray;
}
*/

QScriptValue supportedEllipticCurvesForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  QVector<QSslEllipticCurve> curves = QSslConfiguration::supportedEllipticCurves();
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < curves.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(curves.at(i)));
  }
  return newArray;
}

QScriptValue systemCaCertificatesForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  QList<QSslCertificate> certificates = QSslConfiguration::systemCaCertificates();
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < certificates.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(certificates.at(i)));
  }
  return newArray;
}

void setupQSslConfigurationProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QSslConfigurationProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslConfiguration*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSslConfiguration>(), proto);

  QScriptValue constructor = engine->newFunction(constructQSslConfiguration, proto);

  qScriptRegisterMetaType(engine, NextProtocolNegotiationStatusToScriptValue, NextProtocolNegotiationStatusFromScriptValue);
  constructor.setProperty("NextProtocolNegotiationNone", QScriptValue(engine, QSslConfiguration::NextProtocolNegotiationNone), permanent);
  constructor.setProperty("NextProtocolNegotiationNegotiated", QScriptValue(engine, QSslConfiguration::NextProtocolNegotiationNegotiated), permanent);
  constructor.setProperty("NextProtocolNegotiationUnsupported", QScriptValue(engine, QSslConfiguration::NextProtocolNegotiationUnsupported), permanent);

  QScriptValue defaultConfiguration = engine->newFunction(defaultConfigurationForJS);
  constructor.setProperty("defaultConfiguration", defaultConfiguration);
  QScriptValue setDefaultConfiguration = engine->newFunction(setDefaultConfigurationForJS);
  constructor.setProperty("setDefaultConfiguration", setDefaultConfiguration);
  // TODO: Something is wrong with how we expose QSslCipher.
  /*
  QScriptValue supportedCiphers = engine->newFunction(supportedCiphersForJS);
  constructor.setProperty("supportedCiphers", supportedCiphers);
  */
  QScriptValue supportedEllipticCurves = engine->newFunction(supportedEllipticCurvesForJS);
  constructor.setProperty("supportedEllipticCurves", supportedEllipticCurves);
  QScriptValue systemCaCertificates = engine->newFunction(systemCaCertificatesForJS);
  constructor.setProperty("systemCaCertificates", systemCaCertificates);

  engine->globalObject().setProperty("QSslConfiguration",  constructor);
}

#include <QSslConfiguration>
QScriptValue constructQSslConfiguration(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QSslConfiguration *obj = 0;
  obj = new QSslConfiguration();
  return engine->toScriptValue(obj);
}

QSslConfigurationProto::QSslConfigurationProto(QObject *parent)
    : QObject(parent)
{
}

QSslConfigurationProto::~QSslConfigurationProto()
{
}

QList<QByteArray> QSslConfigurationProto::allowedNextProtocols() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->allowedNextProtocols();
  return QList<QByteArray>();
}

QList<QSslCertificate> QSslConfigurationProto::caCertificates() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->caCertificates();
  return QList<QSslCertificate>();
}

QList<QSslCipher> QSslConfigurationProto::ciphers() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->ciphers();
  return QList<QSslCipher>();
}

QVector<QSslEllipticCurve> QSslConfigurationProto::ellipticCurves() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->ellipticCurves();
  return QVector<QSslEllipticCurve>();
}

bool QSslConfigurationProto::isNull() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

QSslCertificate QSslConfigurationProto::localCertificate() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->localCertificate();
  return QSslCertificate();
}

QList<QSslCertificate> QSslConfigurationProto::localCertificateChain() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->localCertificateChain();
  return QList<QSslCertificate>();
}

QByteArray QSslConfigurationProto::nextNegotiatedProtocol() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->nextNegotiatedProtocol();
  return QByteArray();
}

QSslConfiguration::NextProtocolNegotiationStatus QSslConfigurationProto::nextProtocolNegotiationStatus() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->nextProtocolNegotiationStatus();
  return QSslConfiguration::NextProtocolNegotiationStatus();
}

QSslCertificate QSslConfigurationProto::peerCertificate() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->peerCertificate();
  return QSslCertificate();
}

QList<QSslCertificate> QSslConfigurationProto::peerCertificateChain() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->peerCertificateChain();
  return QList<QSslCertificate>();
}

int QSslConfigurationProto::peerVerifyDepth() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->peerVerifyDepth();
  return 0;
}

QSslSocket::PeerVerifyMode QSslConfigurationProto::peerVerifyMode() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->peerVerifyMode();
  return QSslSocket::PeerVerifyMode();
}

QSslKey QSslConfigurationProto::privateKey() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->privateKey();
  return QSslKey();
}

QSsl::SslProtocol QSslConfigurationProto::protocol() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->protocol();
  return QSsl::SslProtocol();
}

QSslCipher QSslConfigurationProto::sessionCipher() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->sessionCipher();
  return QSslCipher();
}

QSsl::SslProtocol QSslConfigurationProto::sessionProtocol() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->sessionProtocol();
  return QSsl::SslProtocol();
}

QByteArray QSslConfigurationProto::sessionTicket() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->sessionTicket();
  return QByteArray();
}

int QSslConfigurationProto::sessionTicketLifeTimeHint() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->sessionTicketLifeTimeHint();
  return 0;
}

void QSslConfigurationProto::setAllowedNextProtocols(const QList<QByteArray> & protocols)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setAllowedNextProtocols(protocols);
}

void QSslConfigurationProto::setCaCertificates(const QList<QSslCertificate> & certificates)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setCaCertificates(certificates);
}

void QSslConfigurationProto::setCiphers(const QList<QSslCipher> & ciphers)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setCiphers(ciphers);
}

void QSslConfigurationProto::setEllipticCurves(const QVector<QSslEllipticCurve> & curves)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setEllipticCurves(curves);
}

void QSslConfigurationProto::setLocalCertificate(const QSslCertificate & certificate)
{
  QScriptValue scriptObj = thisObject();
  scriptObj.setProperty("_localCertificate", qPrintable(QString(certificate.toPem())));
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(scriptObj);
  if (item)
    item->setLocalCertificate(certificate);
}

void QSslConfigurationProto::setLocalCertificateChain(const QList<QSslCertificate> & localChain)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setLocalCertificateChain(localChain);
}

void QSslConfigurationProto::setPeerVerifyDepth(int depth)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setPeerVerifyDepth(depth);
}

void QSslConfigurationProto::setPeerVerifyMode(QSslSocket::PeerVerifyMode mode)
{
  QScriptValue scriptObj = thisObject();
  scriptObj.setProperty("_peerVerifyMode", mode);
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(scriptObj);
  if (item)
    item->setPeerVerifyMode(mode);
}

void QSslConfigurationProto::setPrivateKey(const QSslKey & key)
{
  QScriptValue scriptObj = thisObject();
  scriptObj.setProperty("_privateKey", qPrintable(QString(key.toPem())));
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(scriptObj);
  if (item)
    item->setPrivateKey(key);
}

void QSslConfigurationProto::setProtocol(QSsl::SslProtocol protocol)
{
  QScriptValue scriptObj = thisObject();
  scriptObj.setProperty("_protocol", protocol);
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(scriptObj);
  if (item)
    item->setProtocol(protocol);
}

void QSslConfigurationProto::setSessionTicket(const QByteArray & sessionTicket)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setSessionTicket(sessionTicket);
}

void QSslConfigurationProto::setSslOption(QSsl::SslOption option, bool on)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setSslOption(option, on);
}

void QSslConfigurationProto::swap(QSslConfiguration & other)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->swap(other);
}

bool QSslConfigurationProto::testSslOption(QSsl::SslOption option) const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->testSslOption(option);
  return false;
}

#endif
