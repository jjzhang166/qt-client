/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QNETWORKACCESSMANAGERPROTO_H__
#define __QNETWORKACCESSMANAGERPROTO_H__

#include <QScriptEngine>

void setupQNetworkAccessManagerProto(QScriptEngine *engine);

#include <QAbstractNetworkCache>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkConfiguration>
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QNetworkReply>
#include <QObject>
#include <QStringList>
#include <QtScript>

class QByteArray;

Q_DECLARE_METATYPE(QNetworkAccessManager*)

QScriptValue constructQNetworkAccessManager(QScriptContext *context, QScriptEngine *engine);

class QNetworkAccessManagerProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QNetworkAccessManagerProto(QObject *parent);
    virtual ~QNetworkAccessManagerProto();

    Q_INVOKABLE QNetworkConfiguration                           activeConfiguration() const;
    Q_INVOKABLE QAbstractNetworkCache                          *cache() const;
#if QT_VERSION >= 0x050000
    Q_INVOKABLE void                                            clearAccessCache();
#endif
    Q_INVOKABLE QNetworkConfiguration                           configuration() const;
#if QT_VERSION >= 0x050000
    Q_INVOKABLE void                                            connectToHost(const QString & hostName, quint16 port = 80);
    Q_INVOKABLE void                                            connectToHostEncrypted(const QString & hostName, quint16 port = 443, const QSslConfiguration & sslConfiguration = QSslConfiguration::defaultConfiguration());
#endif
    Q_INVOKABLE QNetworkCookieJar                              *cookieJar() const;
    Q_INVOKABLE QNetworkReply                                  *deleteResource(const QNetworkRequest & request);
    Q_INVOKABLE QNetworkReply                                  *get(const QNetworkRequest & request);
    Q_INVOKABLE QNetworkReply                                  *head(const QNetworkRequest & request);
    Q_INVOKABLE QNetworkAccessManager::NetworkAccessibility     networkAccessible() const;
    Q_INVOKABLE QNetworkReply                                  *post(const QNetworkRequest & request, QIODevice * data);
    Q_INVOKABLE QNetworkReply                                  *post(const QNetworkRequest & request, const QByteArray & data);
    Q_INVOKABLE QNetworkReply                                  *post(const QNetworkRequest & request, QHttpMultiPart * multiPart);
    Q_INVOKABLE QNetworkProxy                                   proxy() const;
    Q_INVOKABLE QNetworkProxyFactory                           *proxyFactory() const;
    Q_INVOKABLE QNetworkReply                                  *put(const QNetworkRequest & request, QIODevice * data);
    Q_INVOKABLE QNetworkReply                                  *put(const QNetworkRequest & request, const QByteArray & data);
    Q_INVOKABLE QNetworkReply                                  *put(const QNetworkRequest & request, QHttpMultiPart * multiPart);
    Q_INVOKABLE QNetworkReply                                  *sendCustomRequest(const QNetworkRequest & request, const QByteArray & verb, QIODevice * data = 0);
    Q_INVOKABLE void                                            setCache(QAbstractNetworkCache * cache);
    Q_INVOKABLE void                                            setConfiguration(const QNetworkConfiguration & config);
    Q_INVOKABLE void                                            setCookieJar(QNetworkCookieJar * cookieJar);
    Q_INVOKABLE void                                            setNetworkAccessible(QNetworkAccessManager::NetworkAccessibility accessible);
    Q_INVOKABLE void                                            setProxy(const QNetworkProxy & proxy);
    Q_INVOKABLE void                                            setProxyFactory(QNetworkProxyFactory * factory);
#if QT_VERSION >= 0x050000
    Q_INVOKABLE QStringList                                     supportedSchemes() const;
#endif

};

#endif
