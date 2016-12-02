/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDNSMAILEXCHANGERECORDPROTO_H__
#define __QDNSMAILEXCHANGERECORDPROTO_H__

#include <QtScript>

void setupQDnsMailExchangeRecordProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000

#include <QDnsMailExchangeRecord>

Q_DECLARE_METATYPE(QDnsMailExchangeRecord*)
Q_DECLARE_METATYPE(QDnsMailExchangeRecord)

QScriptValue constructQDnsMailExchangeRecord(QScriptContext *context, QScriptEngine *engine);

class QDnsMailExchangeRecordProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDnsMailExchangeRecordProto(QObject *parent);
    Q_INVOKABLE virtual ~QDnsMailExchangeRecordProto();

    Q_INVOKABLE QString   exchange() const;
    Q_INVOKABLE QString   name() const;
    Q_INVOKABLE quint16   preference() const;
    Q_INVOKABLE void      swap(QDnsMailExchangeRecord &other);
    Q_INVOKABLE quint32   timeToLive() const;

    Q_INVOKABLE QString   toString() const;

};

#endif

#endif // __QDNSMAILEXCHANGERECORDPROTO_H__

