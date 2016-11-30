/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QABSTRACTSOCKETPROTO_H__
#define __QABSTRACTSOCKETPROTO_H__
#include <QObject>
#include <QString>
#include <QtScript>

void setupQAbstractSocketProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QIODevice>
#include <QAbstractSocket>
#include <QNetworkProxy>
#include <QHostAddress>
#include "qiodeviceproto.h"

QScriptValue constructQAbstractSocket(QScriptContext *context, QScriptEngine *engine);

Q_DECLARE_METATYPE(enum QAbstractSocket::BindFlag)
Q_DECLARE_METATYPE(QAbstractSocket::BindMode)
Q_DECLARE_METATYPE(enum QAbstractSocket::NetworkLayerProtocol)
Q_DECLARE_METATYPE(enum QAbstractSocket::PauseMode)
Q_DECLARE_METATYPE(QAbstractSocket::PauseModes)
//Q_DECLARE_METATYPE(enum QAbstractSocket::SocketError) // Already declared in qabstractsocket.h
Q_DECLARE_METATYPE(enum QAbstractSocket::SocketOption)
//Q_DECLARE_METATYPE(enum QAbstractSocket::SocketState) // Already declared in qabstractsocket.h
Q_DECLARE_METATYPE(enum QAbstractSocket::SocketType)

class QAbstractSocketProto : public QIODeviceProto
{
  Q_OBJECT

  public:
    QAbstractSocketProto(QObject *parent);
    // public functions
    Q_INVOKABLE void                         abort();
    Q_INVOKABLE bool                         bind(const QHostAddress & address, quint16 port = 0, QAbstractSocket::BindMode mode = QAbstractSocket::DefaultForPlatform);
    Q_INVOKABLE bool                         bind(quint16 port = 0, QAbstractSocket::BindMode mode = QAbstractSocket::DefaultForPlatform);
    Q_INVOKABLE void                         connectToHost(const QString & hostName, quint16 port, QIODevice::OpenMode openMode = QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::AnyIPProtocol);
    Q_INVOKABLE void                         connectToHost(const QHostAddress & address, quint16 port, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    Q_INVOKABLE void                         disconnectFromHost();
    Q_INVOKABLE QAbstractSocket::SocketError error() const;
    Q_INVOKABLE bool                         flush();
    Q_INVOKABLE bool                         isValid() const;
    Q_INVOKABLE QHostAddress                 localAddress() const;
    Q_INVOKABLE quint16                      localPort() const;
    Q_INVOKABLE QAbstractSocket::PauseModes  pauseMode() const;
    Q_INVOKABLE QHostAddress                 peerAddress() const;
    Q_INVOKABLE QString                      peerName() const;
    Q_INVOKABLE quint16                      peerPort() const;
    Q_INVOKABLE QNetworkProxy                proxy() const;
    Q_INVOKABLE qint64                       readBufferSize() const;
    Q_INVOKABLE void                         resume();
    Q_INVOKABLE void                         setPauseMode(QAbstractSocket::PauseModes pauseMode);
    Q_INVOKABLE void                         setProxy(const QNetworkProxy & networkProxy);
    Q_INVOKABLE void                         setReadBufferSize(qint64 size);
    Q_INVOKABLE bool                         setSocketDescriptor(qintptr socketDescriptor, QAbstractSocket::SocketState socketState = QAbstractSocket::ConnectedState, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    Q_INVOKABLE void                         setSocketOption(QAbstractSocket::SocketOption option, const QVariant & value);
    Q_INVOKABLE qintptr                      socketDescriptor() const;
    Q_INVOKABLE QVariant                     socketOption(QAbstractSocket::SocketOption option);
    Q_INVOKABLE QAbstractSocket::SocketType  socketType() const;
    Q_INVOKABLE QAbstractSocket::SocketState state() const;
    Q_INVOKABLE bool                         waitForConnected(int msecs = 30000);
    Q_INVOKABLE bool                         waitForDisconnected(int msecs = 30000);
    // reimplemented public functions
    Q_INVOKABLE bool                         atEnd() const;
    Q_INVOKABLE qint64                       bytesAvailable() const;
    Q_INVOKABLE qint64                       bytesToWrite() const;
    Q_INVOKABLE bool                         canReadLine() const;
    Q_INVOKABLE void                         close();
    Q_INVOKABLE bool                         isSequential() const;
    Q_INVOKABLE bool                         waitForBytesWritten(int msecs = 30000);
    Q_INVOKABLE bool                         waitForReadyRead(int msecs = 30000);
    // added custom
    Q_INVOKABLE QString                      toString() const;

  signals:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void hostFound();
    void proxyAuthenticationRequired(const QNetworkProxy & proxy, QAuthenticator * authenticator);
    void stateChanged(QAbstractSocket::SocketState socketState);

};
#endif

#endif
