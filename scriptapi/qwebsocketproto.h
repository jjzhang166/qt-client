/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBSOCKETPROTO_H__
#define __QWEBSOCKETPROTO_H__

#include <QtScript>

void setupQWebSocketProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000

#include <QWebSocket>
class QMaskGenerator;

Q_DECLARE_METATYPE(QWebSocket*)
//Q_DECLARE_METATYPE(QWebSocket) // Is Q_DISABLE_COPY() in qwebsocket.h

QScriptValue constructQWebSocket(QScriptContext *context, QScriptEngine *engine);

class QWebSocketProto :public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QWebSocketProto(QObject *parent);
    Q_INVOKABLE virtual ~QWebSocketProto();

    Q_INVOKABLE void                          abort();
    Q_INVOKABLE QWebSocketProtocol::CloseCode closeCode() const;
    Q_INVOKABLE QString                       closeReason() const;
    Q_INVOKABLE QAbstractSocket::SocketError  error() const;
    Q_INVOKABLE QString                       errorString() const;
    Q_INVOKABLE bool                          flush();
    Q_INVOKABLE void                          ignoreSslErrors(const QList<QSslError> & errors);
    Q_INVOKABLE bool                          isValid() const;
    Q_INVOKABLE QHostAddress                  localAddress() const;
    Q_INVOKABLE quint16                       localPort() const;
    Q_INVOKABLE const QMaskGenerator         *maskGenerator() const;
    Q_INVOKABLE QString                       origin() const;
    Q_INVOKABLE QAbstractSocket::PauseModes   pauseMode() const;
    Q_INVOKABLE QHostAddress                  peerAddress() const;
    Q_INVOKABLE QString                       peerName() const;
    Q_INVOKABLE quint16                       peerPort() const;
    Q_INVOKABLE QNetworkProxy                 proxy() const;
    Q_INVOKABLE qint64                        readBufferSize() const;
    Q_INVOKABLE QUrl                          requestUrl() const;
    Q_INVOKABLE QString                       resourceName() const;
    Q_INVOKABLE void                          resume();
    Q_INVOKABLE qint64                        sendBinaryMessage(const QByteArray & data);
    Q_INVOKABLE qint64                        sendTextMessage(const QString & message);
    Q_INVOKABLE void                          setMaskGenerator(const QMaskGenerator * maskGenerator);
    Q_INVOKABLE void                          setPauseMode(QAbstractSocket::PauseModes pauseMode);
    Q_INVOKABLE void                          setProxy(const QNetworkProxy & networkProxy);
    Q_INVOKABLE void                          setReadBufferSize(qint64 size);
    Q_INVOKABLE void                          setSslConfiguration(const QSslConfiguration & sslConfiguration);
    Q_INVOKABLE QSslConfiguration             sslConfiguration() const;
    Q_INVOKABLE QAbstractSocket::SocketState  state() const;
    Q_INVOKABLE QWebSocketProtocol::Version   version() const;

    Q_INVOKABLE QString                       toString() const;

  public slots:

    void close(QWebSocketProtocol::CloseCode closeCode = QWebSocketProtocol::CloseCodeNormal, const QString & reason = QString());
    void ignoreSslErrors();
    void open(const QUrl & url);
    void ping(const QByteArray & payload = QByteArray());

  signals:
    void    aboutToClose();
    void    binaryFrameReceived(const QByteArray & frame, bool isLastFrame);
    void    binaryMessageReceived(const QByteArray & message);
    void    bytesWritten(qint64 bytes);
    void    connected();
    void    disconnected();
    void    error(QAbstractSocket::SocketError error);
    void    pong(quint64 elapsedTime, const QByteArray & payload);
    void    proxyAuthenticationRequired(const QNetworkProxy & proxy, QAuthenticator * authenticator);
    void    readChannelFinished();
    void    sslErrors(const QList<QSslError> & errors);
    void    stateChanged(QAbstractSocket::SocketState state);
    void    textFrameReceived(const QString & frame, bool isLastFrame);
    void    textMessageReceived(const QString & message);

};

#endif

#endif
