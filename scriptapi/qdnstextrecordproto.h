/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDNSTEXTRECORDPROTO_H__
#define __QDNSTEXTRECORDPROTO_H__

#include <QtScript>

void setupQDnsTextRecordProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000

#include <QDnsTextRecord>

Q_DECLARE_METATYPE(QDnsTextRecord*)
Q_DECLARE_METATYPE(QDnsTextRecord)

QScriptValue constructQDnsTextRecord(QScriptContext *context, QScriptEngine *engine);

class QDnsTextRecordProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDnsTextRecordProto(QObject *parent);
    Q_INVOKABLE virtual ~QDnsTextRecordProto();

    Q_INVOKABLE QString             name() const;
    Q_INVOKABLE void                swap(QDnsTextRecord &other);
    Q_INVOKABLE quint32             timeToLive() const;
    Q_INVOKABLE QList<QByteArray>   values() const;

    Q_INVOKABLE QString             toString() const;

};

#endif

#endif // __QDNSTEXTRECORDPROTO_H__
