/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBSECURITYORIGINPROTO_H__
#define __QWEBSECURITYORIGINPROTO_H__

#include <QScriptEngine>

void setupQWebSecurityOriginProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QWebSecurityOrigin>

Q_DECLARE_METATYPE(QWebSecurityOrigin*)
//Q_DECLARE_METATYPE(QWebSecurityOrigin)
Q_DECLARE_METATYPE(enum QWebSecurityOrigin::SubdomainSetting)

QScriptValue constructQWebSecurityOrigin(QScriptContext *context, QScriptEngine *engine);

class QWebSecurityOriginProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QWebSecurityOriginProto(QObject *parent);
    virtual ~QWebSecurityOriginProto();

    Q_INVOKABLE void                  addAccessWhitelistEntry(const QString & scheme, const QString & host, QWebSecurityOrigin::SubdomainSetting subdomainSetting);
    Q_INVOKABLE qint64                databaseQuota() const;
    Q_INVOKABLE qint64                databaseUsage() const;
    // TODO: QWebDatabase and therefore QList<QWebDatabase> are not exposed yet.
    //Q_INVOKABLE QList<QWebDatabase>   databases() const;
    Q_INVOKABLE QString               host() const;
    Q_INVOKABLE int                   port() const;
    Q_INVOKABLE void                  removeAccessWhitelistEntry(const QString & scheme, const QString & host, QWebSecurityOrigin::SubdomainSetting subdomainSetting);
    Q_INVOKABLE QString               scheme() const;
    Q_INVOKABLE void                  setApplicationCacheQuota(qint64 quota);
    Q_INVOKABLE void                  setDatabaseQuota(qint64 quota);

};

#endif
#endif
