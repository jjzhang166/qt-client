/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebsocketserverproto.h"

#if QT_VERSION < 0x050000
void setupQWebSocketServerProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue SslModeToScriptValue(QScriptEngine *engine, const QWebSocketServer::SslMode &item)
{
  return engine->newVariant(item);
}
void SslModeFromScriptValue(const QScriptValue &obj, QWebSocketServer::SslMode &item)
{
  item = (QWebSocketServer::SslMode)obj.toInt32();
}

QScriptValue QWebSocketServerToScriptValue(QScriptEngine *engine, QWebSocketServer* const &item)
{
  return engine->newQObject(item);
}
void QWebSocketServerFromScriptValue(const QScriptValue &obj, QWebSocketServer* &item)
{
  item = qobject_cast<QWebSocketServer*>(obj.toQObject());
}

void setupQWebSocketServerProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QWebSocketServerToScriptValue, QWebSocketServerFromScriptValue);
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QWebSocketServerProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebSocketServer*>(), proto);
  // Not allowed. Is Q_DISABLE_COPY() in qwebsocketserver.h
  //engine->setDefaultPrototype(qMetaTypeId<QWebSocketServer>(), proto);

  QScriptValue constructor = engine->newFunction(constructQWebSocketServer, proto);
  engine->globalObject().setProperty("QWebSocketServer",  constructor);

  qScriptRegisterMetaType(engine, SslModeToScriptValue, SslModeFromScriptValue);
  constructor.setProperty("SecureMode",    QScriptValue(engine, QWebSocketServer::SecureMode),    permanent);
  constructor.setProperty("NonSecureMode", QScriptValue(engine, QWebSocketServer::NonSecureMode), permanent);
}

QScriptValue constructQWebSocketServer(QScriptContext *context, QScriptEngine *engine)
{
  QWebSocketServer *obj = 0;
  if (context->argumentCount() == 3)
  {
    obj = new QWebSocketServer(context->argument(0).toString(),
                               (QWebSocketServer::SslMode)context->argument(1).toInt32(),
                               context->argument(2).toQObject());
  }
  else if (context->argumentCount() == 2)
  {
    obj = new QWebSocketServer(context->argument(0).toString(),
                               (QWebSocketServer::SslMode)context->argument(1).toInt32());
  }
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QWebSocketServer constructor");

  return engine->toScriptValue(obj);
}

QWebSocketServerProto::QWebSocketServerProto(QObject *parent)
    : QObject(parent)
{
}

QWebSocketServerProto::~QWebSocketServerProto()
{
}

void QWebSocketServerProto::close()
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    item->close();
}

QWebSocketProtocol::CloseCode QWebSocketServerProto::error() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->error();
  return QWebSocketProtocol::CloseCodeProtocolError;
}

QString QWebSocketServerProto::errorString() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

bool QWebSocketServerProto::hasPendingConnections() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->hasPendingConnections();
  return false;
}

bool QWebSocketServerProto::isListening() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->isListening();
  return false;
}

bool QWebSocketServerProto::listen(const QHostAddress & address, quint16 port)
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->listen(address, port);
  return false;
}

int QWebSocketServerProto::maxPendingConnections() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->maxPendingConnections();
  return 0;
}

QWebSocket *QWebSocketServerProto::nextPendingConnection()
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->nextPendingConnection();
  return 0;
}

void QWebSocketServerProto::pauseAccepting()
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    item->pauseAccepting();
}

QNetworkProxy QWebSocketServerProto::proxy() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->proxy();
  return QNetworkProxy();
}

void QWebSocketServerProto::resumeAccepting()
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    item->resumeAccepting();
}

QWebSocketServer::SslMode QWebSocketServerProto::secureMode() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->secureMode();
  return QWebSocketServer::NonSecureMode;
}

QHostAddress QWebSocketServerProto::serverAddress() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->serverAddress();
  return QHostAddress();
}

QString QWebSocketServerProto::serverName() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->serverName();
  return QString();
}

quint16 QWebSocketServerProto::serverPort() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->serverPort();
  return 0;
}

QUrl QWebSocketServerProto::serverUrl() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->serverUrl();
  return QUrl();
}

void QWebSocketServerProto::setMaxPendingConnections(int numConnections)
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    item->setMaxPendingConnections(numConnections);
}

void QWebSocketServerProto::setProxy(const QNetworkProxy &networkProxy)
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    item->setProxy(networkProxy);
}

void QWebSocketServerProto::setServerName(const QString &serverName)
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    item->setServerName(serverName);
}

bool QWebSocketServerProto::setSocketDescriptor(int socketDescriptor)
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->setSocketDescriptor(socketDescriptor);
  return false;
}

void QWebSocketServerProto::setSslConfiguration(const QSslConfiguration & sslConfiguration)
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    item->setSslConfiguration(sslConfiguration);
}

int QWebSocketServerProto::socketDescriptor() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->socketDescriptor();
  return 0;
}

QSslConfiguration QWebSocketServerProto::sslConfiguration() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->sslConfiguration();
  return QSslConfiguration();
}

QList<QWebSocketProtocol::Version> QWebSocketServerProto::supportedVersions() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return item->supportedVersions();
  return QList<QWebSocketProtocol::Version>();
}

QString QWebSocketServerProto::toString() const
{
  QWebSocketServer *item = qscriptvalue_cast<QWebSocketServer*>(thisObject());
  if (item)
    return "QWebSocketServer(" + item->serverAddress().toString() + "," + item->secureMode() + ")";
  return QString();
}

#endif
