/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */
#include "scriptapi_internal.h"
#include "qabstractsocketproto.h"

#define DEBUG false

#if QT_VERSION < 0x050000
void setupQAbstractSocketProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

QScriptValue QAbstractSockettoScriptValue(QScriptEngine *engine, QAbstractSocket* const &item)
{
  return engine->newQObject(item);
}
void QAbstractSocketfromScriptValue(const QScriptValue &obj, QAbstractSocket* &item)
{
  item = qobject_cast<QAbstractSocket*>(obj.toQObject());
}

QScriptValue BindFlagtoScriptValue(QScriptEngine *engine, const enum QAbstractSocket::BindFlag &p)
{
  return QScriptValue(engine, (int)p);
}
void BindFlagfromScriptValue(const QScriptValue &obj, enum QAbstractSocket::BindFlag &p)
{
  p = (enum QAbstractSocket::BindFlag)obj.toInt32();
}

QScriptValue NetworkLayerProtocoltoScriptValue(QScriptEngine *engine, const enum QAbstractSocket::NetworkLayerProtocol &p)
{
  return QScriptValue(engine, (int)p);
}
void NetworkLayerProtocolfromScriptValue(const QScriptValue &obj, enum QAbstractSocket::NetworkLayerProtocol &p)
{
  p = (enum QAbstractSocket::NetworkLayerProtocol)obj.toInt32();
}

QScriptValue PauseModetoScriptValue(QScriptEngine *engine, const enum QAbstractSocket::PauseMode &p)
{
  return QScriptValue(engine, (int)p);
}
void PauseModefromScriptValue(const QScriptValue &obj, enum QAbstractSocket::PauseMode &p)
{
  p = (enum QAbstractSocket::PauseMode)obj.toInt32();
}

QScriptValue SocketErrortoScriptValue(QScriptEngine *engine, const enum QAbstractSocket::SocketError &p)
{
  return QScriptValue(engine, (int)p);
}
void SocketErrorfromScriptValue(const QScriptValue &obj, enum QAbstractSocket::SocketError &p)
{
  p = (enum QAbstractSocket::SocketError)obj.toInt32();
}

QScriptValue SocketOptiontoScriptValue(QScriptEngine *engine, const enum QAbstractSocket::SocketOption &p)
{
  return QScriptValue(engine, (int)p);
}
void SocketOptionfromScriptValue(const QScriptValue &obj, enum QAbstractSocket::SocketOption &p)
{
  p = (enum QAbstractSocket::SocketOption)obj.toInt32();
}

QScriptValue SocketStatetoScriptValue(QScriptEngine *engine, const enum QAbstractSocket::SocketState &p)
{
  return QScriptValue(engine, (int)p);
}
void SocketStatefromScriptValue(const QScriptValue &obj, enum QAbstractSocket::SocketState &p)
{
  p = (enum QAbstractSocket::SocketState)obj.toInt32();
}

QScriptValue SocketTypetoScriptValue(QScriptEngine *engine, const enum QAbstractSocket::SocketType &p)
{
  return QScriptValue(engine, (int)p);
}
void SocketTypefromScriptValue(const QScriptValue &obj, enum QAbstractSocket::SocketType &p)
{
  p = (enum QAbstractSocket::SocketType)obj.toInt32();
}

void setupQAbstractSocketProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QAbstractSockettoScriptValue, QAbstractSocketfromScriptValue);

  QScriptValue proto = engine->newQObject(new QAbstractSocketProto(engine));
  QScriptValue constructor = engine->newFunction(constructQAbstractSocket, proto);
  engine->globalObject().setProperty("QAbstractSocket",  constructor);

    // enum QAbstractSocket::BindFlag
  qScriptRegisterMetaType(engine,               BindFlagtoScriptValue, BindFlagfromScriptValue);
  constructor.setProperty("ShareAddress",       QScriptValue(engine,   QAbstractSocket::ShareAddress),       ENUMPROPFLAGS);
  constructor.setProperty("DontShareAddress",   QScriptValue(engine,   QAbstractSocket::DontShareAddress),   ENUMPROPFLAGS);
  constructor.setProperty("ReuseAddressHint",   QScriptValue(engine,   QAbstractSocket::ReuseAddressHint),   ENUMPROPFLAGS);
  constructor.setProperty("DefaultForPlatform", QScriptValue(engine,   QAbstractSocket::DefaultForPlatform), ENUMPROPFLAGS);

  // enum QAbstractSocket::NetworkLayerProtocol
  qScriptRegisterMetaType(engine,                        NetworkLayerProtocoltoScriptValue, NetworkLayerProtocolfromScriptValue);
  constructor.setProperty("IPv4Protocol",                QScriptValue(engine,               QAbstractSocket::IPv4Protocol),                ENUMPROPFLAGS);
  constructor.setProperty("IPv6Protocol",                QScriptValue(engine,               QAbstractSocket::IPv6Protocol),                ENUMPROPFLAGS);
  constructor.setProperty("AnyIPProtocol",               QScriptValue(engine,               QAbstractSocket::AnyIPProtocol),               ENUMPROPFLAGS);
  constructor.setProperty("UnknownNetworkLayerProtocol", QScriptValue(engine,               QAbstractSocket::UnknownNetworkLayerProtocol), ENUMPROPFLAGS);

  // enum QAbstractSocket::PauseMode
  qScriptRegisterMetaType(engine,             PauseModetoScriptValue,  PauseModefromScriptValue);
  constructor.setProperty("PauseNever",       QScriptValue(engine,     QAbstractSocket::PauseNever),       ENUMPROPFLAGS);
  constructor.setProperty("PauseOnSslErrors", QScriptValue(engine,     QAbstractSocket::PauseOnSslErrors), ENUMPROPFLAGS);

  // enum QAbstractSocket::SocketError
  qScriptRegisterMetaType(engine, SocketErrortoScriptValue, SocketErrorfromScriptValue);
  constructor.setProperty("ConnectionRefusedError",           QScriptValue(engine, QAbstractSocket::ConnectionRefusedError),           ENUMPROPFLAGS);
  constructor.setProperty("RemoteHostClosedError",            QScriptValue(engine, QAbstractSocket::RemoteHostClosedError),            ENUMPROPFLAGS);
  constructor.setProperty("HostNotFoundError",                QScriptValue(engine, QAbstractSocket::HostNotFoundError),                ENUMPROPFLAGS);
  constructor.setProperty("SocketAccessError",                QScriptValue(engine, QAbstractSocket::SocketAccessError),                ENUMPROPFLAGS);
  constructor.setProperty("SocketResourceError",              QScriptValue(engine, QAbstractSocket::SocketResourceError),              ENUMPROPFLAGS);
  constructor.setProperty("SocketTimeoutError",               QScriptValue(engine, QAbstractSocket::SocketTimeoutError),               ENUMPROPFLAGS);
  constructor.setProperty("DatagramTooLargeError",            QScriptValue(engine, QAbstractSocket::DatagramTooLargeError),            ENUMPROPFLAGS);
  constructor.setProperty("NetworkError",                     QScriptValue(engine, QAbstractSocket::NetworkError),                     ENUMPROPFLAGS);
  constructor.setProperty("AddressInUseError",                QScriptValue(engine, QAbstractSocket::AddressInUseError),                ENUMPROPFLAGS);
  constructor.setProperty("SocketAddressNotAvailableError",   QScriptValue(engine, QAbstractSocket::SocketAddressNotAvailableError),   ENUMPROPFLAGS);
  constructor.setProperty("UnsupportedSocketOperationError",  QScriptValue(engine, QAbstractSocket::UnsupportedSocketOperationError),  ENUMPROPFLAGS);
  constructor.setProperty("ProxyAuthenticationRequiredError", QScriptValue(engine, QAbstractSocket::ProxyAuthenticationRequiredError), ENUMPROPFLAGS);
  constructor.setProperty("SslHandshakeFailedError",          QScriptValue(engine, QAbstractSocket::SslHandshakeFailedError),          ENUMPROPFLAGS);
  constructor.setProperty("UnfinishedSocketOperationError",   QScriptValue(engine, QAbstractSocket::UnfinishedSocketOperationError),   ENUMPROPFLAGS);
  constructor.setProperty("ProxyConnectionRefusedError",      QScriptValue(engine, QAbstractSocket::ProxyConnectionRefusedError),      ENUMPROPFLAGS);
  constructor.setProperty("ProxyConnectionClosedError",       QScriptValue(engine, QAbstractSocket::ProxyConnectionClosedError),       ENUMPROPFLAGS);
  constructor.setProperty("ProxyConnectionTimeoutError",      QScriptValue(engine, QAbstractSocket::ProxyConnectionTimeoutError),      ENUMPROPFLAGS);
  constructor.setProperty("ProxyNotFoundError",               QScriptValue(engine, QAbstractSocket::ProxyNotFoundError),               ENUMPROPFLAGS);
  constructor.setProperty("ProxyProtocolError",               QScriptValue(engine, QAbstractSocket::ProxyProtocolError),               ENUMPROPFLAGS);
  constructor.setProperty("OperationError",                   QScriptValue(engine, QAbstractSocket::OperationError),                   ENUMPROPFLAGS);
  constructor.setProperty("SslInternalError",                 QScriptValue(engine, QAbstractSocket::SslInternalError),                 ENUMPROPFLAGS);
  constructor.setProperty("SslInvalidUserDataError",          QScriptValue(engine, QAbstractSocket::SslInvalidUserDataError),          ENUMPROPFLAGS);
  constructor.setProperty("TemporaryError",                   QScriptValue(engine, QAbstractSocket::TemporaryError),                   ENUMPROPFLAGS);
  constructor.setProperty("UnknownSocketError",               QScriptValue(engine, QAbstractSocket::UnknownSocketError),               ENUMPROPFLAGS);

  // enum QAbstractSocket::SocketOption
  qScriptRegisterMetaType(engine, SocketOptiontoScriptValue, SocketOptionfromScriptValue);
  constructor.setProperty("LowDelayOption",                QScriptValue(engine, QAbstractSocket::LowDelayOption),                ENUMPROPFLAGS);
  constructor.setProperty("KeepAliveOption",               QScriptValue(engine, QAbstractSocket::KeepAliveOption),               ENUMPROPFLAGS);
  constructor.setProperty("MulticastTtlOption",            QScriptValue(engine, QAbstractSocket::MulticastTtlOption),            ENUMPROPFLAGS);
  constructor.setProperty("MulticastLoopbackOption",       QScriptValue(engine, QAbstractSocket::MulticastLoopbackOption),       ENUMPROPFLAGS);
  constructor.setProperty("TypeOfServiceOption",           QScriptValue(engine, QAbstractSocket::TypeOfServiceOption),           ENUMPROPFLAGS);
  constructor.setProperty("SendBufferSizeSocketOption",    QScriptValue(engine, QAbstractSocket::SendBufferSizeSocketOption),    ENUMPROPFLAGS);
  constructor.setProperty("ReceiveBufferSizeSocketOption", QScriptValue(engine, QAbstractSocket::ReceiveBufferSizeSocketOption), ENUMPROPFLAGS);

  // enum QAbstractSocket::SocketState
  qScriptRegisterMetaType(engine, SocketStatetoScriptValue, SocketStatefromScriptValue);
  constructor.setProperty("UnconnectedState", QScriptValue(engine, QAbstractSocket::UnconnectedState), ENUMPROPFLAGS);
  constructor.setProperty("HostLookupState",  QScriptValue(engine, QAbstractSocket::HostLookupState),  ENUMPROPFLAGS);
  constructor.setProperty("ConnectingState",  QScriptValue(engine, QAbstractSocket::ConnectingState),  ENUMPROPFLAGS);
  constructor.setProperty("ConnectedState",   QScriptValue(engine, QAbstractSocket::ConnectedState),   ENUMPROPFLAGS);
  constructor.setProperty("BoundState",       QScriptValue(engine, QAbstractSocket::BoundState),       ENUMPROPFLAGS);
  constructor.setProperty("ClosingState",     QScriptValue(engine, QAbstractSocket::ClosingState),     ENUMPROPFLAGS);
  constructor.setProperty("ListeningState",   QScriptValue(engine, QAbstractSocket::ListeningState),   ENUMPROPFLAGS);

  // enum QAbstractSocket::SocketType
  qScriptRegisterMetaType(engine,              SocketTypetoScriptValue,  SocketTypefromScriptValue);
  constructor.setProperty("TcpSocket",         QScriptValue(engine,      QAbstractSocket::TcpSocket),         ENUMPROPFLAGS);
  constructor.setProperty("UdpSocket",         QScriptValue(engine,      QAbstractSocket::UdpSocket),         ENUMPROPFLAGS);
  constructor.setProperty("UnknownSocketType", QScriptValue(engine,      QAbstractSocket::UnknownSocketType), ENUMPROPFLAGS);
}

QScriptValue constructQAbstractSocket(QScriptContext *context, QScriptEngine  *engine)
{
  QAbstractSocket *object = 0;
  if (context->argumentCount() == 2)
  {
    if (DEBUG) qDebug("qabstractsocket(2 args)");
    object = new QAbstractSocket((QAbstractSocket::SocketType)context->argument(0).toInt32(), context->argument(1).toQObject());
  }
  else
  {
    if (DEBUG) qDebug("qabstractsocket(unknown)");
    context->throwError(QScriptContext::UnknownError, "QAbstractSocket unknown constructor");
  }
  return engine->toScriptValue(object);
}

QAbstractSocketProto::QAbstractSocketProto(QObject *parent)
    : QIODeviceProto(parent)
{
}

void QAbstractSocketProto::abort()
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    item->abort();
}

bool QAbstractSocketProto::bind(const QHostAddress & address, quint16 port, QAbstractSocket::BindMode mode)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->bind(address, port, mode);
  return false;
}

bool QAbstractSocketProto::bind(quint16 port, QAbstractSocket::BindMode mode)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->bind(port, mode);
  return false;
}

void QAbstractSocketProto::connectToHost(const QString & hostName, quint16 port, QIODevice::OpenMode openMode, QAbstractSocket::NetworkLayerProtocol protocol)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->connectToHost(hostName, port, openMode, protocol);
}

void QAbstractSocketProto::connectToHost(const QHostAddress & address, quint16 port, QIODevice::OpenMode openMode)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->connectToHost(address, port, openMode);
}

void QAbstractSocketProto::disconnectFromHost()
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->disconnectFromHost();

}

QAbstractSocket::SocketError QAbstractSocketProto::error() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->error();
  return QAbstractSocket::UnknownSocketError;
}

bool QAbstractSocketProto::flush()
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->flush();
  return false;
}

bool QAbstractSocketProto::isValid() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

QHostAddress QAbstractSocketProto::localAddress() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->localAddress();
  return QHostAddress();
}

quint16 QAbstractSocketProto::localPort() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->localPort();
  return quint16();
}

QAbstractSocket::PauseModes QAbstractSocketProto::pauseMode() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->pauseMode();
  return QAbstractSocket::PauseNever;
}

QHostAddress QAbstractSocketProto::peerAddress() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->peerAddress();
  return QHostAddress();
}

QString QAbstractSocketProto::peerName() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->peerName();
  return QString();
}

quint16 QAbstractSocketProto::peerPort() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->peerPort();
  return quint16();
}

QNetworkProxy QAbstractSocketProto::proxy() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->proxy();
  return QNetworkProxy();
}

qint64 QAbstractSocketProto::readBufferSize() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->readBufferSize();
  return 0;
}

void QAbstractSocketProto::resume()
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    item->resume();
}

void QAbstractSocketProto::setPauseMode(QAbstractSocket::PauseModes pauseMode)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    item->setPauseMode(pauseMode);
}

void QAbstractSocketProto::setProxy(const QNetworkProxy & networkProxy)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    item->setProxy(networkProxy);
}

void QAbstractSocketProto::setReadBufferSize(qint64 size)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    item->setReadBufferSize(size);
}

bool QAbstractSocketProto::setSocketDescriptor(qintptr socketDescriptor, QAbstractSocket::SocketState socketState, QIODevice::OpenMode openMode)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->setSocketDescriptor(socketDescriptor, socketState, openMode);
  return false;
}

void QAbstractSocketProto::setSocketOption(QAbstractSocket::SocketOption option, const QVariant & value)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    item->setSocketOption(option, value);
}

qintptr QAbstractSocketProto::socketDescriptor() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->socketDescriptor();
  return qintptr();
}

QVariant QAbstractSocketProto::socketOption(QAbstractSocket::SocketOption option)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->socketOption(option);
  return QVariant();
}

QAbstractSocket::SocketType QAbstractSocketProto::socketType() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->socketType();
  return QAbstractSocket::UnknownSocketType;
}

QAbstractSocket::SocketState QAbstractSocketProto::state() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->state();
  return QAbstractSocket::UnconnectedState;
}

bool QAbstractSocketProto::waitForConnected(int msecs)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->waitForConnected(msecs);
  return false;
}

bool QAbstractSocketProto::waitForDisconnected(int msecs)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->waitForDisconnected(msecs);
  return false;
}

bool QAbstractSocketProto::atEnd() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->atEnd();
  return false;
}

qint64 QAbstractSocketProto::bytesAvailable() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->bytesAvailable();
  return 0;
}

qint64 QAbstractSocketProto::bytesToWrite() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->bytesToWrite();
  return 0;
}

bool QAbstractSocketProto::canReadLine() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->canReadLine();
  return false;
}

void QAbstractSocketProto::close()
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    item->close();
}

bool QAbstractSocketProto::isSequential() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->isSequential();
  return false;
}

bool QAbstractSocketProto::waitForBytesWritten(int msecs)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->waitForBytesWritten(msecs);
  return false;
}

bool QAbstractSocketProto::waitForReadyRead(int msecs)
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return item->waitForReadyRead(msecs);
  return false;
}

QString QAbstractSocketProto::toString() const
{
  QAbstractSocket *item = qscriptvalue_cast<QAbstractSocket*>(thisObject());
  if (item)
    return QString("QAbstractSocketProto()");
  return QString("QAbstractSocketProto(unknown)");
}
#endif
