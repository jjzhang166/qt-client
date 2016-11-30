/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDNSHOSTADDRESSRECORDPROTO_H__
#define __QDNSHOSTADDRESSRECORDPROTO_H__

#include <QtScript>

void setupQDnsHostAddressRecordProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000

#include <QDnsHostAddressRecord>
#include <QHostAddress>

Q_DECLARE_METATYPE(QDnsHostAddressRecord*)
Q_DECLARE_METATYPE(QDnsHostAddressRecord)

QScriptValue constructQDnsHostAddressRecord(QScriptContext *context, QScriptEngine *engine);

class QDnsHostAddressRecordProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDnsHostAddressRecordProto(QObject *parent);
    Q_INVOKABLE virtual ~QDnsHostAddressRecordProto();

    Q_INVOKABLE QString         name() const;
    Q_INVOKABLE void            swap(QDnsHostAddressRecord &other);
    Q_INVOKABLE quint32         timeToLive() const;
    Q_INVOKABLE QHostAddress    value() const;

    Q_INVOKABLE QString         toString() const;

};

#endif

#endif // __QDNSHOSTADDRESSRECORDPROTO_H__
