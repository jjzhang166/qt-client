/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDNSLOOKUPPROTO_H__
#define __QDNSLOOKUPPROTO_H__

#include <QtScript>

void setupQDnsLookupProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000

#include <QDnsDomainNameRecord>
#include <QDnsHostAddressRecord>
#include <QDnsLookup>
#include <QDnsMailExchangeRecord>
#include <QDnsServiceRecord>
#include <QDnsTextRecord>
#include <QHostAddress>

Q_DECLARE_METATYPE(QDnsLookup*)
//Q_DECLARE_METATYPE(QDnsLookup)

Q_DECLARE_METATYPE(enum QDnsLookup::Error)
Q_DECLARE_METATYPE(enum QDnsLookup::Type)

QScriptValue constructQDnsLookup(QScriptContext *context, QScriptEngine *engine);

class QDnsLookupProto : public QObject, public QScriptable
{
  Q_OBJECT

  Q_PROPERTY(QDnsLookup::Error    error           READ error)
  Q_PROPERTY(QString              errorString     READ errorString)
  Q_PROPERTY(QString              name            READ name           WRITE setName)
  Q_PROPERTY(QHostAddress         nameserver      READ nameserver     WRITE setNameserver)
  Q_PROPERTY(QDnsLookup::Type     type            READ type           WRITE setType)

  public:
    QDnsLookupProto(QObject *parent);
    Q_INVOKABLE virtual ~QDnsLookupProto();

    Q_INVOKABLE QList<QDnsDomainNameRecord>     canonicalNameRecords() const;
    Q_INVOKABLE QDnsLookup::Error               error() const;
    Q_INVOKABLE QString                         errorString() const;
    Q_INVOKABLE QList<QDnsHostAddressRecord>    hostAddressRecords() const;
    Q_INVOKABLE bool                            isFinished() const;
    Q_INVOKABLE QList<QDnsMailExchangeRecord>   mailExchangeRecords() const;
    Q_INVOKABLE QString                         name() const;
    Q_INVOKABLE QList<QDnsDomainNameRecord>     nameServerRecords() const;
    Q_INVOKABLE QHostAddress                    nameserver() const;
    Q_INVOKABLE QList<QDnsDomainNameRecord>     pointerRecords() const;
    Q_INVOKABLE QList<QDnsServiceRecord>        serviceRecords() const;
    Q_INVOKABLE void                            setName(const QString &name);
    Q_INVOKABLE void                            setNameserver(const QHostAddress &nameserver);
    Q_INVOKABLE void                            setType(QDnsLookup::Type type);
    Q_INVOKABLE QList<QDnsTextRecord>           textRecords() const;
    Q_INVOKABLE QDnsLookup::Type                type() const;

    Q_INVOKABLE QString                         toString() const;

  public slots:
    void  abort();
    void  lookup();

  signals:
    void  finished();
    void  nameChanged(const QString &name);
    void  nameserverChanged(const QHostAddress &nameserver);
    void  typeChanged(QDnsLookup::Type type);

};

#endif

#endif // __QDNSLOOKUPPROTO_H__

