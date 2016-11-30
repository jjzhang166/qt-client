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
#include "qudpsocketproto.h"

#define DEBUG false

#if QT_VERSION < 0x050000
void setupQUdpSocketProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

QScriptValue QUdpSockettoScriptValue(QScriptEngine *engine, QUdpSocket* const &item)
{ return engine->newQObject(item); }

void QUdpSocketfromScriptValue(const QScriptValue &obj, QUdpSocket* &item)
{
  item = qobject_cast<QUdpSocket*>(obj.toQObject());
}

void setupQUdpSocketProto(QScriptEngine *engine)
{
 qScriptRegisterMetaType(engine, QUdpSockettoScriptValue, QUdpSocketfromScriptValue);

  QScriptValue proto = engine->newQObject(new QUdpSocketProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QUdpSocket*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQUdpSocket, proto);
  engine->globalObject().setProperty("QUdpSocket",  constructor);
}

QScriptValue constructQUdpSocket(QScriptContext * context, QScriptEngine  *engine)
{
  QUdpSocket *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isQObject())
  {
    if (DEBUG) qDebug("constructQUdpSocket(): calling QUdpSocket(QObject)");
    obj = new QUdpSocket(context->argument(0).toQObject());
  }
  else {
    if (DEBUG) qDebug("constructQUdpSocket(): calling QUdpSocket()");
    obj = new QUdpSocket();
  }
  return engine->toScriptValue(obj);
}

QUdpSocketProto::QUdpSocketProto(QObject *parent)
    : QAbstractSocketProto(parent)
{
}

bool QUdpSocketProto::hasPendingDatagrams() const
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    return item->hasPendingDatagrams();
  return false;
}

bool QUdpSocketProto::joinMulticastGroup(const QHostAddress & groupAddress)
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    return item->joinMulticastGroup(groupAddress);
  return false;
}

bool QUdpSocketProto::joinMulticastGroup(const QHostAddress & groupAddress, const QNetworkInterface & iface)
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    return item->joinMulticastGroup(groupAddress, iface);
  return false;
}

bool QUdpSocketProto::leaveMulticastGroup(const QHostAddress & groupAddress)
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    return item->leaveMulticastGroup(groupAddress);
  return false;
}

bool QUdpSocketProto::leaveMulticastGroup(const QHostAddress & groupAddress, const QNetworkInterface & iface)
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    return item->leaveMulticastGroup(groupAddress, iface);
  return false;
}

QNetworkInterface QUdpSocketProto::multicastInterface() const
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    return item->multicastInterface();
  return QNetworkInterface();
}

qint64 QUdpSocketProto::pendingDatagramSize() const
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    return item->pendingDatagramSize();
  return 0;
}

qint64 QUdpSocketProto::readDatagram(char * data, qint64 maxSize, QHostAddress * address, quint16 * port)
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    return item->readDatagram(data, maxSize, address, port);
  return 0;
}

void QUdpSocketProto::setMulticastInterface(const QNetworkInterface & iface)
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    item->setMulticastInterface(iface);
}

qint64 QUdpSocketProto::writeDatagram(const char * data, qint64 size, const QHostAddress & address, quint16 port)
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    return item->writeDatagram(data, size, address, port);
  return 0;
}

qint64 QUdpSocketProto::writeDatagram(const QByteArray & datagram, const QHostAddress & host, quint16 port)
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    return item->writeDatagram(datagram, host, port);
  return 0;
}

QString QUdpSocketProto::toString() const
{
  QUdpSocket *item = qscriptvalue_cast<QUdpSocket*>(thisObject());
  if (item)
    return QString("QUdpSocket()");
  return QString("QUdpSocket(unknown)");
}
#endif

