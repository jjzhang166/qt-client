/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebsocketproto.h"

#if QT_VERSION < 0x050000
void setupQWebSocketProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

#include <QMaskGenerator>

QScriptValue QWebSocketToScriptValue(QScriptEngine *engine, QWebSocket* const &item)
{
  return engine->newQObject(item);
}
void QWebSocketFromScriptValue(const QScriptValue &obj, QWebSocket* &item)
{
  item = qobject_cast<QWebSocket*>(obj.toQObject());
}

void setupQWebSocketProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QWebSocketToScriptValue, QWebSocketFromScriptValue);

  QScriptValue proto = engine->newQObject(new QWebSocketProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebSocket*>(), proto);
  // Not allowed. Is Q_DISABLE_COPY() in qwebsocket.h
  //engine->setDefaultPrototype(qMetaTypeId<QWebSocket>(), proto);

  QScriptValue constructor = engine->newFunction(constructQWebSocket, proto);
  engine->globalObject().setProperty("QWebSocket",  constructor);
}

QScriptValue constructQWebSocket(QScriptContext *context, QScriptEngine *engine)
{
  Q_UNUSED(engine);
  QWebSocket *obj = 0;
  if (context->argumentCount() == 3)
    obj = new QWebSocket(context->argument(0).toString(), (QWebSocketProtocol::Version)context->argument(1).toInt32(), context->argument(2).toQObject());
  else if (context->argumentCount() == 2)
    obj = new QWebSocket(context->argument(0).toString(), (QWebSocketProtocol::Version)context->argument(1).toInt32());
  else if (context->argumentCount() == 1)
    obj = new QWebSocket(context->argument(0).toString());
  else
    obj = new QWebSocket();
  return engine->toScriptValue(obj);
}

QWebSocketProto::QWebSocketProto(QObject *parent)
  : QObject(parent)
{
}

QWebSocketProto::~QWebSocketProto()
{
}

void QWebSocketProto::abort()
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->abort();
}

QWebSocketProtocol::CloseCode QWebSocketProto::closeCode() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->closeCode();
  return QWebSocketProtocol::CloseCodeProtocolError;
}

QString QWebSocketProto::closeReason() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->closeReason();
  return QString();
}

QAbstractSocket::SocketError QWebSocketProto::error() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->error();
  return QAbstractSocket::UnknownSocketError;
}

QString QWebSocketProto::errorString() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->errorString();
  return QString("QWebSocketProto::errorString() - could not cast");
}

bool QWebSocketProto::flush()
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->flush();
  return false;
}

void QWebSocketProto::ignoreSslErrors(const QList<QSslError> & errors)
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->ignoreSslErrors(errors);
}

bool QWebSocketProto::isValid() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

QHostAddress QWebSocketProto::localAddress() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->localAddress();
  return QHostAddress();
}

quint16 QWebSocketProto::localPort() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->localPort();
  return 0;
}

const QMaskGenerator *QWebSocketProto::maskGenerator() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->maskGenerator();
  return 0;
}

QString QWebSocketProto::origin() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->origin();
  return QString();
}

QAbstractSocket::PauseModes QWebSocketProto::pauseMode() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->pauseMode();
  return QAbstractSocket::PauseNever;
}

QHostAddress QWebSocketProto::peerAddress() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->peerAddress();
  return QHostAddress();
}

QString QWebSocketProto::peerName() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->peerName();
  return QString();
}

quint16 QWebSocketProto::peerPort() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->peerPort();
  return 0;
}

QNetworkProxy QWebSocketProto::proxy() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->proxy();
  return QNetworkProxy();
}

qint64 QWebSocketProto::readBufferSize() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->readBufferSize();
  return 0;
}

QUrl QWebSocketProto::requestUrl() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->requestUrl();
  return QUrl();
}

QString QWebSocketProto::resourceName() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->resourceName();
  return QString();
}

void QWebSocketProto::resume()
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->resume();
}

qint64 QWebSocketProto::sendBinaryMessage(const QByteArray & data)
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->sendBinaryMessage(data);
  return 0;
}

qint64 QWebSocketProto::sendTextMessage(const QString & message)
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->sendTextMessage(message);
  return 0;
}

void QWebSocketProto::setMaskGenerator(const QMaskGenerator * maskGenerator)
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->setMaskGenerator(maskGenerator);
}

void QWebSocketProto::setPauseMode(QAbstractSocket::PauseModes pauseMode)
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->setPauseMode(pauseMode);
}

void QWebSocketProto::setProxy(const QNetworkProxy & networkProxy)
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->setProxy(networkProxy);
}

void QWebSocketProto::setReadBufferSize(qint64 size)
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->setReadBufferSize(size);
}

void QWebSocketProto::setSslConfiguration(const QSslConfiguration & sslConfiguration)
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->setSslConfiguration(sslConfiguration);
}

QSslConfiguration QWebSocketProto::sslConfiguration() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->sslConfiguration();
  return QSslConfiguration();
}

QAbstractSocket::SocketState QWebSocketProto::state() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->state();
  return QAbstractSocket::UnconnectedState;
}

QWebSocketProtocol::Version QWebSocketProto::version() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return item->version();
  return QWebSocketProtocol::VersionUnknown;
}

QString QWebSocketProto::toString() const
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    return "QWebSocket(" + item->localAddress().toString() + "<->" + item->peerAddress().toString() + ")";
  return QString("QWebSocket(unknown)");
}

void QWebSocketProto::close(QWebSocketProtocol::CloseCode closeCode, const QString &reason)
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->close(closeCode, reason);
}

void QWebSocketProto::ignoreSslErrors()
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->ignoreSslErrors();
}

void QWebSocketProto::open(const QUrl &url)
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->open(url);
}

void QWebSocketProto::ping(const QByteArray &payload)
{
  QWebSocket *item = qscriptvalue_cast<QWebSocket*>(thisObject());
  if (item)
    item->ping(payload);
}

#endif
