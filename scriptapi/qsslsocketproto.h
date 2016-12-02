/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSSLSOCKETPROTO_H__
#define __QSSLSOCKETPROTO_H__

#include <QScriptEngine>

void setupQSslSocketProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QIODevice>
#include <QScriptable>
#include <QSsl>
#include <QSslCertificate>
#include <QSslCipher>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslSocket>
#include "qtcpsocketproto.h"
class QSslSocket;

Q_DECLARE_METATYPE(QSslSocket*)
Q_DECLARE_METATYPE(enum QSslSocket::PeerVerifyMode)
Q_DECLARE_METATYPE(enum QSslSocket::SslMode)

QScriptValue constructQSslSocket(QScriptContext *context, QScriptEngine *engine);

class QSslSocketProto : public QTcpSocketProto
{
  Q_OBJECT

  public:
    QSslSocketProto(QObject *parent);
    virtual ~QSslSocketProto();

    Q_INVOKABLE void                        abort();
    Q_INVOKABLE void                        addCaCertificate(const QSslCertificate & certificate);
    Q_INVOKABLE bool                        addCaCertificates(const QString & path, QSsl::EncodingFormat format = QSsl::Pem, QRegExp::PatternSyntax syntax = QRegExp::FixedString);
    Q_INVOKABLE void                        addCaCertificates(const QList<QSslCertificate> & certificates);
    Q_INVOKABLE void                        connectToHostEncrypted(const QString & hostName, quint16 port, QIODevice::OpenMode mode = QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::AnyIPProtocol);
    Q_INVOKABLE void                        connectToHostEncrypted(const QString & hostName, quint16 port, const QString & sslPeerName, QIODevice::OpenMode mode = QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::AnyIPProtocol);
    Q_INVOKABLE qint64                      encryptedBytesAvailable() const;
    Q_INVOKABLE qint64                      encryptedBytesToWrite() const;
    Q_INVOKABLE bool                        flush();
    Q_INVOKABLE void                        ignoreSslErrors(const QList<QSslError> & errors);
    Q_INVOKABLE bool                        isEncrypted() const;
    Q_INVOKABLE QSslCertificate             localCertificate() const;
    Q_INVOKABLE QList<QSslCertificate>      localCertificateChain() const;
    Q_INVOKABLE QSslSocket::SslMode         mode() const;
    Q_INVOKABLE QSslCertificate             peerCertificate() const;
    Q_INVOKABLE QList<QSslCertificate>      peerCertificateChain() const;
    Q_INVOKABLE int                         peerVerifyDepth() const;
    Q_INVOKABLE QSslSocket::PeerVerifyMode  peerVerifyMode() const;
    Q_INVOKABLE QString                     peerVerifyName() const;
    Q_INVOKABLE QSslKey                     privateKey() const;
    Q_INVOKABLE QSsl::SslProtocol           protocol() const;
    Q_INVOKABLE QSslCipher                  sessionCipher() const;
    Q_INVOKABLE QSsl::SslProtocol           sessionProtocol() const;
    Q_INVOKABLE void                        setLocalCertificate(const QSslCertificate & certificate);
    Q_INVOKABLE void                        setLocalCertificate(const QString & path, QSsl::EncodingFormat format = QSsl::Pem);
    Q_INVOKABLE void                        setLocalCertificateChain(const QList<QSslCertificate> & localChain);
    Q_INVOKABLE void                        setPeerVerifyDepth(int depth);
    Q_INVOKABLE void                        setPeerVerifyMode(QSslSocket::PeerVerifyMode mode);
    Q_INVOKABLE void                        setPeerVerifyName(const QString & hostName);
    Q_INVOKABLE void                        setPrivateKey(const QSslKey & key);
    Q_INVOKABLE void                        setPrivateKey(const QString & fileName, QSsl::KeyAlgorithm algorithm = QSsl::Rsa, QSsl::EncodingFormat format = QSsl::Pem, const QByteArray & passPhrase = QByteArray());
    Q_INVOKABLE void                        setProtocol(QSsl::SslProtocol protocol);
    Q_INVOKABLE void                        setSslConfiguration(const QSslConfiguration & configuration);
    Q_INVOKABLE QSslConfiguration           sslConfiguration() const;
    Q_INVOKABLE QList<QSslError>            sslErrors() const;
    Q_INVOKABLE bool                        waitForEncrypted(int msecs = 30000);

  // Reimplemented Public Functions.
    Q_INVOKABLE bool      atEnd() const;
    Q_INVOKABLE qint64    bytesAvailable() const;
    Q_INVOKABLE qint64    bytesToWrite() const;
    Q_INVOKABLE bool      canReadLine() const;
    Q_INVOKABLE void      close();
    Q_INVOKABLE void      resume();
    Q_INVOKABLE void      setReadBufferSize(qint64 size);
    Q_INVOKABLE bool      setSocketDescriptor(qintptr socketDescriptor, QAbstractSocket::SocketState state = QAbstractSocket::ConnectedState, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    Q_INVOKABLE void      setSocketOption(QAbstractSocket::SocketOption option, const QVariant & value);
    Q_INVOKABLE QVariant  socketOption(QAbstractSocket::SocketOption option);
    Q_INVOKABLE bool      waitForBytesWritten(int msecs = 30000);
    Q_INVOKABLE bool      waitForConnected(int msecs = 30000);
    Q_INVOKABLE bool      waitForDisconnected(int msecs = 30000);
    Q_INVOKABLE bool      waitForReadyRead(int msecs = 30000);

  public slots:
    void  ignoreSslErrors();
    void  startClientEncryption();
    void  startServerEncryption();

  signals:
    void  encrypted();
    void  encryptedBytesWritten(qint64 written);
    void  modeChanged(QSslSocket::SslMode mode);
    void  peerVerifyError(const QSslError & error);
    void  preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator * authenticator);
    void  sslErrors(const QList<QSslError> & errors);

    // QAbstractSocket signals:
    void  connected();
    void  disconnected();
    void  error(QAbstractSocket::SocketError socketError);
    void  hostFound();
    void  proxyAuthenticationRequired(const QNetworkProxy & proxy, QAuthenticator * authenticator);
    void  stateChanged(QAbstractSocket::SocketState socketState);

    // QIODevice signals:
    void  aboutToClose();
    void  bytesWritten(qint64 bytes);
    void  readChannelFinished();
    void  readyRead();

    // QObject signals:
    void  destroyed(QObject * obj = 0);
    void  objectNameChanged(const QString & objectName);

};

#endif
#endif
