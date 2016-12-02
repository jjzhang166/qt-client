/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDNSDOMAINNAMERECORDPROTO_H__
#define __QDNSDOMAINNAMERECORDPROTO_H__

#include <QtScript>

void setupQDnsDomainNameRecordProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000

#include <QDnsDomainNameRecord>

Q_DECLARE_METATYPE(QDnsDomainNameRecord*)
Q_DECLARE_METATYPE(QDnsDomainNameRecord)

QScriptValue constructQDnsDomainNameRecord(QScriptContext *context, QScriptEngine *engine);

class QDnsDomainNameRecordProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDnsDomainNameRecordProto(QObject *parent);
    Q_INVOKABLE virtual ~QDnsDomainNameRecordProto();

    Q_INVOKABLE QString   name() const;
    Q_INVOKABLE void      swap(QDnsDomainNameRecord &other);
    Q_INVOKABLE quint32   timeToLive() const;
    Q_INVOKABLE QString   value() const;

    Q_INVOKABLE QString   toString() const;

};

#endif

#endif // __QDNSDOMAINNAMERECORDPROTO_H__
