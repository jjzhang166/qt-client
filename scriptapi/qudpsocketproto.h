/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QUDPSOCKETPROTO_H__
#define __QUDPSOCKETPROTO_H__
#include <QObject>
#include <QtScript>

void setupQUdpSocketProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QIODevice>
#include <QAbstractSocket>
#include <QUdpSocket>
#include <QNetworkInterface>
#include "qabstractsocketproto.h"

QScriptValue constructQUdpSocket(QScriptContext *context, QScriptEngine *engine);

Q_DECLARE_METATYPE(QUdpSocket*)

class QUdpSocketProto : public QAbstractSocketProto
{
  Q_OBJECT

  public:
    QUdpSocketProto(QObject *parent);
    
    Q_INVOKABLE bool              hasPendingDatagrams() const;
    Q_INVOKABLE bool              joinMulticastGroup(const QHostAddress & groupAddress);
    Q_INVOKABLE bool              joinMulticastGroup(const QHostAddress & groupAddress, const QNetworkInterface & iface);
    Q_INVOKABLE bool              leaveMulticastGroup(const QHostAddress & groupAddress);
    Q_INVOKABLE bool              leaveMulticastGroup(const QHostAddress & groupAddress, const QNetworkInterface & iface);
    Q_INVOKABLE QNetworkInterface multicastInterface() const;
    Q_INVOKABLE qint64            pendingDatagramSize() const;
    Q_INVOKABLE qint64            readDatagram(char * data, qint64 maxSize, QHostAddress * address = 0, quint16 * port = 0);
    Q_INVOKABLE void              setMulticastInterface(const QNetworkInterface & iface);
    Q_INVOKABLE qint64            writeDatagram(const char * data, qint64 size, const QHostAddress & address, quint16 port);
    Q_INVOKABLE qint64            writeDatagram(const QByteArray & datagram, const QHostAddress & host, quint16 port);
    // added custom
    Q_INVOKABLE QString           toString() const;
};
#endif

#endif
