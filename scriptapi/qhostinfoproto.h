/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QHOSTINFOPROTO_H__
#define __QHOSTINFOPROTO_H__

#include <QtScript>
#include <QString>
#include <QList>

void setupQHostInfoProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QHostAddress>
#include <QHostInfo>

Q_DECLARE_METATYPE(QHostInfo*)
Q_DECLARE_METATYPE(enum QHostInfo::HostInfoError)

QScriptValue constructQHostInfo(QScriptContext *context, QScriptEngine *engine);

class QHostInfoProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QHostInfoProto(QObject *parent);

    Q_INVOKABLE QList<QHostAddress>	     addresses() const;
    Q_INVOKABLE QHostInfo::HostInfoError error() const;
    Q_INVOKABLE QString                  errorString() const;
    Q_INVOKABLE QString                  hostName() const;
    Q_INVOKABLE int                      lookupId() const;
    Q_INVOKABLE void                     setAddresses(const QList<QHostAddress> & addresses);
    Q_INVOKABLE void                     setError(QHostInfo::HostInfoError error);
    Q_INVOKABLE void                     setErrorString(const QString & str);
    Q_INVOKABLE void                     setHostName(const QString & hostName);
    Q_INVOKABLE void                     setLookupId(int id);
    Q_INVOKABLE QString                  toString() const;
};
#endif

#endif
