/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qnetworkaccessmanagerproto.h"

#include <QByteArray>
QScriptValue QNetworkAccessManagertoScriptValue(QScriptEngine *engine, QNetworkAccessManager* const &item)
{
  return engine->newQObject(item);
}

void QNetworkAccessManagerfromScriptValue(const QScriptValue &obj, QNetworkAccessManager* &item)
{
  item = qobject_cast<QNetworkAccessManager*>(obj.toQObject());
}

void setupQNetworkAccessManagerProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QNetworkAccessManagertoScriptValue, QNetworkAccessManagerfromScriptValue);
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QNetworkAccessManagerProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QNetworkAccessManager*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQNetworkAccessManager, proto);
  engine->globalObject().setProperty("QNetworkAccessManager",  constructor);
  proto.setProperty("UnknownAccessibility", QScriptValue(engine, QNetworkAccessManager::UnknownAccessibility),  permanent);
  proto.setProperty("NotAccessible",        QScriptValue(engine, QNetworkAccessManager::NotAccessible),         permanent);
  proto.setProperty("Accessible",           QScriptValue(engine, QNetworkAccessManager::Accessible),            permanent);
}
QScriptValue constructQNetworkAccessManager(QScriptContext *context, QScriptEngine *engine)
{
  QNetworkAccessManager *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QNetworkAccessManager(context->argument(0).toQObject());
  else
    obj = new QNetworkAccessManager();
  return engine->toScriptValue(obj);
}

QNetworkAccessManagerProto::QNetworkAccessManagerProto(QObject *parent)
    : QObject(parent)
{
}

QNetworkAccessManagerProto::~QNetworkAccessManagerProto()
{
}

QNetworkConfiguration QNetworkAccessManagerProto::activeConfiguration() const
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->activeConfiguration();
  return QNetworkConfiguration();
}

QAbstractNetworkCache *QNetworkAccessManagerProto::cache() const
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->cache();
  return 0;
}

#if QT_VERSION >= 0x050000
void QNetworkAccessManagerProto::clearAccessCache()
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    item->clearAccessCache();
}
#endif

QNetworkConfiguration QNetworkAccessManagerProto::configuration() const
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->configuration();
  return QNetworkConfiguration();
}

#if QT_VERSION >= 0x050000
void QNetworkAccessManagerProto::connectToHost(const QString & hostName, quint16 port)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    item->connectToHost(hostName, port);
}

void QNetworkAccessManagerProto::connectToHostEncrypted(const QString & hostName, quint16 port, const QSslConfiguration & sslConfiguration)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    item->connectToHostEncrypted(hostName, port, sslConfiguration);
}
#endif

QNetworkCookieJar *QNetworkAccessManagerProto::cookieJar() const
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->cookieJar();
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::deleteResource(const QNetworkRequest & request)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->deleteResource(request);
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::get(const QNetworkRequest & request)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->get(request);
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::head(const QNetworkRequest & request)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->head(request);
  return 0;
}

QNetworkAccessManager::NetworkAccessibility QNetworkAccessManagerProto::networkAccessible() const
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->networkAccessible();
  return QNetworkAccessManager::UnknownAccessibility;
}

QNetworkReply *QNetworkAccessManagerProto::post(const QNetworkRequest & request, QIODevice * data)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->post(request, data);
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::post(const QNetworkRequest & request, const QByteArray & data)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->post(request, data);
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::post(const QNetworkRequest & request, QHttpMultiPart * multiPart)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->post(request, multiPart);
  return 0;
}

QNetworkProxy QNetworkAccessManagerProto::proxy() const
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->proxy();
  return QNetworkProxy();
}

QNetworkProxyFactory *QNetworkAccessManagerProto::proxyFactory() const
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->proxyFactory();
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::put(const QNetworkRequest & request, QIODevice * data)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->put(request, data);
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::put(const QNetworkRequest & request, const QByteArray & data)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->put(request, data);
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::put(const QNetworkRequest & request, QHttpMultiPart * multiPart)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->put(request, multiPart);
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::sendCustomRequest(const QNetworkRequest & request, const QByteArray & verb, QIODevice * data)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->sendCustomRequest(request, verb, data);
  return 0;
}

void QNetworkAccessManagerProto::setCache(QAbstractNetworkCache * cache)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    item->setCache(cache);
}

void QNetworkAccessManagerProto::setConfiguration(const QNetworkConfiguration & config)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    item->setConfiguration(config);
}

void QNetworkAccessManagerProto::setCookieJar(QNetworkCookieJar * cookieJar)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    item->setCookieJar(cookieJar);
}

void QNetworkAccessManagerProto::setNetworkAccessible(QNetworkAccessManager::NetworkAccessibility accessible)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    item->setNetworkAccessible(accessible);
}

void QNetworkAccessManagerProto::setProxy(const QNetworkProxy & proxy)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    item->setProxy(proxy);
}

void QNetworkAccessManagerProto::setProxyFactory(QNetworkProxyFactory * factory)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    item->setProxyFactory(factory);
}

#if QT_VERSION >= 0x050000
QStringList QNetworkAccessManagerProto::supportedSchemes() const
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->supportedSchemes();
  return QStringList();
}
#endif
