/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSSLCONFIGURATIONPROTO_H__
#define __QSSLCONFIGURATIONPROTO_H__

#include <QScriptEngine>

void setupQSslConfigurationProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QSslCipher>
#include <QSslConfiguration>
#include <QSslEllipticCurve>
#include <QSslKey>

Q_DECLARE_METATYPE(QSslConfiguration*)
//Q_DECLARE_METATYPE(QSslConfiguration) // Already set in qsslconfiguration.h
Q_DECLARE_METATYPE(enum QSslConfiguration::NextProtocolNegotiationStatus)

QScriptValue constructQSslConfiguration(QScriptContext *context, QScriptEngine *engine);

class QSslConfigurationProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSslConfigurationProto(QObject *parent);
    virtual ~QSslConfigurationProto();

    Q_INVOKABLE QList<QByteArray>                                 allowedNextProtocols() const;
    Q_INVOKABLE QList<QSslCertificate>                            caCertificates() const;
    Q_INVOKABLE QList<QSslCipher>                                 ciphers() const;
    Q_INVOKABLE QVector<QSslEllipticCurve>                        ellipticCurves() const;
    Q_INVOKABLE bool                                              isNull() const;
    Q_INVOKABLE QSslCertificate                                   localCertificate() const;
    Q_INVOKABLE QList<QSslCertificate>                            localCertificateChain() const;
    Q_INVOKABLE QByteArray                                        nextNegotiatedProtocol() const;
    Q_INVOKABLE QSslConfiguration::NextProtocolNegotiationStatus  nextProtocolNegotiationStatus() const;
    Q_INVOKABLE QSslCertificate                                   peerCertificate() const;
    Q_INVOKABLE QList<QSslCertificate>                            peerCertificateChain() const;
    Q_INVOKABLE int                                               peerVerifyDepth() const;
    Q_INVOKABLE QSslSocket::PeerVerifyMode                        peerVerifyMode() const;
    Q_INVOKABLE QSslKey                                           privateKey() const;
    Q_INVOKABLE QSsl::SslProtocol                                 protocol() const;
    Q_INVOKABLE QSslCipher                                        sessionCipher() const;
    Q_INVOKABLE QSsl::SslProtocol                                 sessionProtocol() const;
    Q_INVOKABLE QByteArray                                        sessionTicket() const;
    Q_INVOKABLE int                                               sessionTicketLifeTimeHint() const;
    Q_INVOKABLE void                                              setAllowedNextProtocols(const QList<QByteArray> & protocols);
    Q_INVOKABLE void                                              setCaCertificates(const QList<QSslCertificate> & certificates);
    Q_INVOKABLE void                                              setCiphers(const QList<QSslCipher> & ciphers);
    Q_INVOKABLE void                                              setEllipticCurves(const QVector<QSslEllipticCurve> & curves);
    Q_INVOKABLE void                                              setLocalCertificate(const QSslCertificate & certificate);
    Q_INVOKABLE void                                              setLocalCertificateChain(const QList<QSslCertificate> & localChain);
    Q_INVOKABLE void                                              setPeerVerifyDepth(int depth);
    Q_INVOKABLE void                                              setPeerVerifyMode(QSslSocket::PeerVerifyMode mode);
    Q_INVOKABLE void                                              setPrivateKey(const QSslKey & key);
    Q_INVOKABLE void                                              setProtocol(QSsl::SslProtocol protocol);
    Q_INVOKABLE void                                              setSessionTicket(const QByteArray & sessionTicket);
    Q_INVOKABLE void                                              setSslOption(QSsl::SslOption option, bool on);
    Q_INVOKABLE void                                              swap(QSslConfiguration & other);
    Q_INVOKABLE bool                                              testSslOption(QSsl::SslOption option) const;

};

#endif
#endif
