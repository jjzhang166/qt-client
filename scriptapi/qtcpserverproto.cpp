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
#include "qtcpserverproto.h"
#include "qhostaddressproto.h"

#define DEBUG false

#if QT_VERSION < 0x050000
void setupQTcpServerProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

QScriptValue QTcpServertoScriptValue(QScriptEngine *engine, QTcpServer* const &item)
{ return engine->newQObject(item); }

void QTcpServerfromScriptValue(const QScriptValue &obj, QTcpServer* &item)
{
  item = qobject_cast<QTcpServer*>(obj.toQObject());
}

void setupQTcpServerProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QTcpServertoScriptValue, QTcpServerfromScriptValue);

  QScriptValue proto = engine->newQObject(new QTcpServerProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QTcpServer*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQTcpServer, proto);
  engine->globalObject().setProperty("QTcpServer",  constructor);

}

QScriptValue constructQTcpServer(QScriptContext *context, QScriptEngine  *engine)
{
  QTcpServer *obj = 0;
  if (context->argumentCount() == 1) {
    obj = new QTcpServer(context->argument(0).toQObject());
  }
  else {
    obj = new QTcpServer();
  }

  return engine->toScriptValue(obj);
}

QTcpServerProto::QTcpServerProto(QObject *parent) : QObject(parent)
{
}

void QTcpServerProto::close()
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    item->close();
}

QString QTcpServerProto::errorString() const
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

bool QTcpServerProto::hasPendingConnections() const
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->hasPendingConnections();
  return false;
}

bool QTcpServerProto::isListening() const
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->isListening();
  return false;
}

bool QTcpServerProto::listen(const QHostAddress & address, quint16 port)
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->listen(address, port);
  return false;
}

QTcpSocket *QTcpServerProto::nextPendingConnection()
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->nextPendingConnection();
  return new QTcpSocket();
}

void QTcpServerProto::pauseAccepting()
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    item->pauseAccepting();
}

QNetworkProxy QTcpServerProto::proxy() const
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->proxy();
  return QNetworkProxy();
}

void QTcpServerProto::resumeAccepting()
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    item->resumeAccepting();
}

QHostAddress QTcpServerProto::serverAddress() const
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->serverAddress();
  return QHostAddress();
}

QAbstractSocket::SocketError QTcpServerProto::serverError() const
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->serverError();
  return QAbstractSocket::SocketError();
}

quint16 QTcpServerProto::serverPort() const
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->serverPort();
  return quint16();
}

void QTcpServerProto::setMaxPendingConnections(int numConnections)
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    item->setMaxPendingConnections(numConnections);
}

void QTcpServerProto::setProxy(const QNetworkProxy & networkProxy)
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    item->setProxy(networkProxy);
}

bool QTcpServerProto::setSocketDescriptor(qintptr socketDescriptor)
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->setSocketDescriptor(socketDescriptor);
  return false;
}

qintptr QTcpServerProto::socketDescriptor() const
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->socketDescriptor();
  return qintptr();
}

bool QTcpServerProto::waitForNewConnection(int msec, bool * timedOut)
{
  QTcpServer *item = qscriptvalue_cast<QTcpServer*>(thisObject());
  if (item)
    return item->waitForNewConnection(msec, timedOut);
  return false;
}
#endif
