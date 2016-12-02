/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDNSSERVICERECORDPROTO_H__
#define __QDNSSERVICERECORDPROTO_H__

#include <QtScript>

void setupQDnsServiceRecordProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000

#include <QDnsServiceRecord>

Q_DECLARE_METATYPE(QDnsServiceRecord*)
Q_DECLARE_METATYPE(QDnsServiceRecord)

QScriptValue constructQDnsServiceRecord(QScriptContext *context, QScriptEngine *engine);

class QDnsServiceRecordProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDnsServiceRecordProto(QObject *parent);
    Q_INVOKABLE virtual ~QDnsServiceRecordProto();

    Q_INVOKABLE QString   name() const;
    Q_INVOKABLE quint16   port() const;
    Q_INVOKABLE quint16   priority() const;
    Q_INVOKABLE void      swap(QDnsServiceRecord &other);
    Q_INVOKABLE QString   target() const;
    Q_INVOKABLE quint32   timeToLive() const;
    Q_INVOKABLE quint16   weight() const;

    Q_INVOKABLE QString   toString() const;

};

#endif

#endif // __QDNSSERVICERECORDPROTO_H__
