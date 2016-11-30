/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslsocketproto.h"

#if QT_VERSION < 0x050000
void setupQSslSocketProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue addDefaultCaCertificateForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QScriptValue toPem = context->argument(0).property(QString("toPem"));
    QString certificate;
    if (toPem.isFunction()) {
      certificate = toPem.call(context->argument(0)).toString();
    } else {
      certificate = context->argument(0).toString();
    }
    QSslCertificate newCert = QSslCertificate(certificate.toLocal8Bit(), QSsl::Pem);
    QSslSocket::addDefaultCaCertificate(newCert);
  }
  return engine->undefinedValue();
}

QScriptValue sslLibraryBuildVersionNumberForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QSslSocket::sslLibraryBuildVersionNumber());
}

QScriptValue sslLibraryBuildVersionStringForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QSslSocket::sslLibraryBuildVersionString());
}

QScriptValue sslLibraryVersionNumberForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QSslSocket::sslLibraryVersionNumber());
}

QScriptValue sslLibraryVersionStringForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QSslSocket::sslLibraryVersionString());
}

QScriptValue supportsSslForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QSslSocket::supportsSsl());
}

QScriptValue PeerVerifyModeToScriptValue(QScriptEngine *engine, const QSslSocket::PeerVerifyMode &item)
{
  return engine->newVariant(item);
}
void PeerVerifyModeFromScriptValue(const QScriptValue &obj, QSslSocket::PeerVerifyMode &item)
{
  item = (QSslSocket::PeerVerifyMode)obj.toInt32();
}

QScriptValue SslModeToScriptValue(QScriptEngine *engine, const QSslSocket::SslMode &item)
{
  return engine->newVariant(item);
}
void SslModeFromScriptValue(const QScriptValue &obj, QSslSocket::SslMode &item)
{
  item = (QSslSocket::SslMode)obj.toInt32();
}

void setupQSslSocketProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QSslSocketProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslSocket*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQSslSocket, proto);
  engine->globalObject().setProperty("QSslSocket",  constructor);

  QScriptValue addDefaultCaCertificate = engine->newFunction(addDefaultCaCertificateForJS);
  constructor.setProperty("addDefaultCaCertificate", addDefaultCaCertificate);
  QScriptValue sslLibraryBuildVersionNumber = engine->newFunction(sslLibraryBuildVersionNumberForJS);
  constructor.setProperty("sslLibraryBuildVersionNumber", sslLibraryBuildVersionNumber);
  QScriptValue sslLibraryBuildVersionString = engine->newFunction(sslLibraryBuildVersionStringForJS);
  constructor.setProperty("sslLibraryBuildVersionString", sslLibraryBuildVersionString);
  QScriptValue sslLibraryVersionNumber = engine->newFunction(sslLibraryVersionNumberForJS);
  constructor.setProperty("sslLibraryVersionNumber", sslLibraryVersionNumber);
  QScriptValue sslLibraryVersionString = engine->newFunction(sslLibraryVersionStringForJS);
  constructor.setProperty("sslLibraryVersionString", sslLibraryVersionString);
  QScriptValue supportsSsl = engine->newFunction(supportsSslForJS);
  constructor.setProperty("supportsSsl", supportsSsl);

  qScriptRegisterMetaType(engine, PeerVerifyModeToScriptValue, PeerVerifyModeFromScriptValue);
  constructor.setProperty("VerifyNone", QScriptValue(engine, QSslSocket::VerifyNone), permanent);
  constructor.setProperty("QueryPeer", QScriptValue(engine, QSslSocket::QueryPeer), permanent);
  constructor.setProperty("VerifyPeer", QScriptValue(engine, QSslSocket::VerifyPeer), permanent);
  constructor.setProperty("AutoVerifyPeer", QScriptValue(engine, QSslSocket::AutoVerifyPeer), permanent);

  qScriptRegisterMetaType(engine, SslModeToScriptValue, SslModeFromScriptValue);
  constructor.setProperty("UnencryptedMode", QScriptValue(engine, QSslSocket::UnencryptedMode), permanent);
  constructor.setProperty("SslClientMode", QScriptValue(engine, QSslSocket::SslClientMode), permanent);
  constructor.setProperty("SslServerMode", QScriptValue(engine, QSslSocket::SslServerMode), permanent);

}

QScriptValue constructQSslSocket(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QSslSocket *obj = 0;
  obj = new QSslSocket();

  return engine->toScriptValue(obj);
}

QSslSocketProto::QSslSocketProto(QObject *parent) : QTcpSocketProto(parent)
{
}

QSslSocketProto::~QSslSocketProto()
{
}


void QSslSocketProto::abort()
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->abort();
}

void QSslSocketProto::addCaCertificate(const QSslCertificate & certificate)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->addCaCertificate(certificate);
}

bool QSslSocketProto::addCaCertificates(const QString & path, QSsl::EncodingFormat format, QRegExp::PatternSyntax syntax)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->addCaCertificates(path, format, syntax);
  return false;
}

void QSslSocketProto::addCaCertificates(const QList<QSslCertificate> & certificates)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->addCaCertificates(certificates);
}

void QSslSocketProto::connectToHostEncrypted(const QString & hostName, quint16 port, QIODevice::OpenMode mode, QAbstractSocket::NetworkLayerProtocol protocol)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->connectToHostEncrypted(hostName, port, mode, protocol);
}

void QSslSocketProto::connectToHostEncrypted(const QString & hostName, quint16 port, const QString & sslPeerName, QIODevice::OpenMode mode, QAbstractSocket::NetworkLayerProtocol protocol)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->connectToHostEncrypted(hostName, port, sslPeerName, mode, protocol);
}

qint64 QSslSocketProto::encryptedBytesAvailable() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->encryptedBytesAvailable();
  return 0;
}

qint64 QSslSocketProto::encryptedBytesToWrite() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->encryptedBytesToWrite();
  return 0;
}

bool QSslSocketProto::flush()
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->flush();
  return false;
}

void QSslSocketProto::ignoreSslErrors(const QList<QSslError> & errors)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->ignoreSslErrors(errors);
}

bool QSslSocketProto::isEncrypted() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->isEncrypted();
  return false;
}

QSslCertificate QSslSocketProto::localCertificate() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->localCertificate();
  return QSslCertificate();
}

QList<QSslCertificate> QSslSocketProto::localCertificateChain() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->localCertificateChain();
  return QList<QSslCertificate>();
}

QSslSocket::SslMode QSslSocketProto::mode() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->mode();
  return QSslSocket::SslMode();
}

QSslCertificate QSslSocketProto::peerCertificate() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->peerCertificate();
  return QSslCertificate();
}

QList<QSslCertificate> QSslSocketProto::peerCertificateChain() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->peerCertificateChain();
  return QList<QSslCertificate>();
}

int QSslSocketProto::peerVerifyDepth() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->peerVerifyDepth();
  return 0;
}

QSslSocket::PeerVerifyMode QSslSocketProto::peerVerifyMode() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->peerVerifyMode();
  return QSslSocket::PeerVerifyMode();
}

QString QSslSocketProto::peerVerifyName() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->peerVerifyName();
  return QString();
}

QSslKey QSslSocketProto::privateKey() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->privateKey();
  return QSslKey();
}

QSsl::SslProtocol QSslSocketProto::protocol() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->protocol();
  return QSsl::SslProtocol();
}

QSslCipher QSslSocketProto::sessionCipher() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->sessionCipher();
  return QSslCipher();
}

QSsl::SslProtocol QSslSocketProto::sessionProtocol() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->sessionProtocol();
  return QSsl::SslProtocol();
}

void QSslSocketProto::setLocalCertificate(const QSslCertificate & certificate)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setLocalCertificate(certificate);
}

void QSslSocketProto::setLocalCertificate(const QString & path, QSsl::EncodingFormat format)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setLocalCertificate(path, format);
}

void QSslSocketProto::setLocalCertificateChain(const QList<QSslCertificate> & localChain)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setLocalCertificateChain(localChain);
}

void QSslSocketProto::setPeerVerifyDepth(int depth)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setPeerVerifyDepth(depth);
}

void QSslSocketProto::setPeerVerifyMode(QSslSocket::PeerVerifyMode mode)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setPeerVerifyMode(mode);
}

void QSslSocketProto::setPeerVerifyName(const QString & hostName)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setPeerVerifyName(hostName);
}

void QSslSocketProto::setPrivateKey(const QSslKey & key)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setPrivateKey(key);
}

void QSslSocketProto::setPrivateKey(const QString & fileName, QSsl::KeyAlgorithm algorithm, QSsl::EncodingFormat format, const QByteArray & passPhrase)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setPrivateKey(fileName, algorithm, format, passPhrase);
}

void QSslSocketProto::setProtocol(QSsl::SslProtocol protocol)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setProtocol(protocol);
}

void QSslSocketProto::setSslConfiguration(const QSslConfiguration & configuration)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setSslConfiguration(configuration);
}

QSslConfiguration QSslSocketProto::sslConfiguration() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->sslConfiguration();
  return QSslConfiguration();
}

QList<QSslError> QSslSocketProto::sslErrors() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->sslErrors();
  return QList<QSslError>();
}

bool QSslSocketProto::waitForEncrypted(int msecs)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->waitForEncrypted(msecs);
  return false;
}

// Reimplemented Public Functions.

bool QSslSocketProto::atEnd() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->atEnd();
  return false;
}

qint64 QSslSocketProto::bytesAvailable() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->bytesAvailable();
  return 0;
}

qint64 QSslSocketProto::bytesToWrite() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->bytesToWrite();
  return 0;
}

bool QSslSocketProto::canReadLine() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->canReadLine();
  return false;
}

void QSslSocketProto::close()
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->close();
}

void QSslSocketProto::resume()
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->resume();
}

void QSslSocketProto::setReadBufferSize(qint64 size)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setReadBufferSize(size);
}

bool QSslSocketProto::setSocketDescriptor(qintptr socketDescriptor, QAbstractSocket::SocketState state, QIODevice::OpenMode openMode)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->setSocketDescriptor(socketDescriptor, state, openMode);
  return false;
}

void QSslSocketProto::setSocketOption(QAbstractSocket::SocketOption option, const QVariant & value)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    item->setSocketOption(option, value);
}

QVariant QSslSocketProto::socketOption(QAbstractSocket::SocketOption option)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->socketOption(option);
  return QVariant();
}

bool QSslSocketProto::waitForBytesWritten(int msecs)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->waitForBytesWritten(msecs);
  return false;
}

bool QSslSocketProto::waitForConnected(int msecs)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->waitForConnected(msecs);
  return false;
}

bool QSslSocketProto::waitForDisconnected(int msecs)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->waitForDisconnected(msecs);
  return false;
}

bool QSslSocketProto::waitForReadyRead(int msecs)
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->waitForReadyRead(msecs);
  return false;
}

// public slots:

void  QSslSocketProto::ignoreSslErrors()
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->ignoreSslErrors();
}

void  QSslSocketProto::startClientEncryption()
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->startClientEncryption();
}

void  QSslSocketProto::startServerEncryption()
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return item->startServerEncryption();
}

#endif
