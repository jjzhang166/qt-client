/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBSOCKETSERVERPROTO_H__
#define __QWEBSOCKETSERVERPROTO_H__

#include <QScriptEngine>

void setupQWebSocketServerProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QNetworkProxy>
#include <QScriptable>
#include <QUrl>
#include <QWebSocketServer>

class QWebSocket;

Q_DECLARE_METATYPE(QWebSocketServer*)
//Q_DECLARE_METATYPE(QWebSocketServer) // Is Q_DISABLE_COPY() in qwebsocketserver.h
Q_DECLARE_METATYPE(enum QWebSocketServer::SslMode)

QScriptValue constructQWebSocketServer(QScriptContext *context, QScriptEngine *engine);

class QWebSocketServerProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QWebSocketServerProto(QObject *parent);
    virtual ~QWebSocketServerProto();

    Q_INVOKABLE void                                close();
    Q_INVOKABLE QWebSocketProtocol::CloseCode       error() const;
    Q_INVOKABLE QString                             errorString() const;
    Q_INVOKABLE bool                                hasPendingConnections() const;
    Q_INVOKABLE bool                                isListening() const;
    Q_INVOKABLE bool                                listen(const QHostAddress & address = QHostAddress::Any, quint16 port = 0);
    Q_INVOKABLE int                                 maxPendingConnections() const;
    Q_INVOKABLE QWebSocket                         *nextPendingConnection();
    Q_INVOKABLE void                                pauseAccepting();
    Q_INVOKABLE QNetworkProxy                       proxy() const;
    Q_INVOKABLE void                                resumeAccepting();
    Q_INVOKABLE QWebSocketServer::SslMode           secureMode() const;
    Q_INVOKABLE QHostAddress                        serverAddress() const;
    Q_INVOKABLE QString                             serverName() const;
    Q_INVOKABLE quint16                             serverPort() const;
    Q_INVOKABLE QUrl                                serverUrl() const;
    Q_INVOKABLE void                                setMaxPendingConnections(int numConnections);
    Q_INVOKABLE void                                setProxy(const QNetworkProxy & networkProxy);
    Q_INVOKABLE void                                setServerName(const QString & serverName);
    Q_INVOKABLE bool                                setSocketDescriptor(int socketDescriptor);
    Q_INVOKABLE void                                setSslConfiguration(const QSslConfiguration & sslConfiguration);
    Q_INVOKABLE int                                 socketDescriptor() const;
    Q_INVOKABLE QSslConfiguration                   sslConfiguration() const;
    Q_INVOKABLE QList<QWebSocketProtocol::Version>  supportedVersions() const;

    Q_INVOKABLE QString toString() const;

  signals:
    void    acceptError(QAbstractSocket::SocketError socketError);
    void    closed();
    void    newConnection();
    void    originAuthenticationRequired(QWebSocketCorsAuthenticator * authenticator);
    void    peerVerifyError(const QSslError & error);
    void    serverError(QWebSocketProtocol::CloseCode closeCode);
    void    sslErrors(const QList<QSslError> & errors);

};

#endif
#endif
