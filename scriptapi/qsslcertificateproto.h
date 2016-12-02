/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSSLCERTIFICATEPROTO_H__
#define __QSSLCERTIFICATEPROTO_H__

#include <QScriptEngine>

void setupQSslCertificateProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QSsl>
#include <QSslCertificate>
#include <QSslCertificateExtension>
#include <QSslError>
#include <QSslKey>

Q_DECLARE_METATYPE(QSslCertificate*)
//Q_DECLARE_METATYPE(QSslCertificate) //Do not need. Already declared by qsslcertificate.h
Q_DECLARE_METATYPE(enum QSslCertificate::SubjectInfo)

QScriptValue constructQSslCertificate(QScriptContext *context, QScriptEngine *engine);

class QSslCertificateProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSslCertificateProto(QObject *parent);
    virtual ~QSslCertificateProto();

    Q_INVOKABLE void                                                clear();
    Q_INVOKABLE QByteArray                                          digest(QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md5) const;
    Q_INVOKABLE QString                                             effectiveDate() const;
    Q_INVOKABLE QString                                             expiryDate() const;
    Q_INVOKABLE QList<QSslCertificateExtension>                     extensions() const;
    Q_INVOKABLE bool                                                isBlacklisted() const;
    Q_INVOKABLE bool                                                isNull() const;
    Q_INVOKABLE bool                                                isSelfSigned() const;
    Q_INVOKABLE QStringList                                         issuerInfo(QSslCertificate::SubjectInfo subject) const;
    Q_INVOKABLE QStringList                                         issuerInfo(const QByteArray & attribute) const;
    Q_INVOKABLE QList<QByteArray>                                   issuerInfoAttributes() const;
    Q_INVOKABLE QSslKey                                             publicKey() const;
    Q_INVOKABLE QByteArray                                          serialNumber() const;
    Q_INVOKABLE QMultiMap<QSsl::AlternativeNameEntryType, QString>  subjectAlternativeNames() const;
    Q_INVOKABLE QStringList                                         subjectInfo(QSslCertificate::SubjectInfo subject) const;
    Q_INVOKABLE QStringList                                         subjectInfo(const QByteArray & attribute) const;
    Q_INVOKABLE QList<QByteArray>                                   subjectInfoAttributes() const;
    Q_INVOKABLE void                                                swap(QSslCertificate & other);
    Q_INVOKABLE QByteArray                                          toDer() const;
    Q_INVOKABLE QByteArray                                          toPem() const;
    Q_INVOKABLE QString                                             toText() const;
    Q_INVOKABLE QByteArray                                          version() const;

};

#endif
#endif
