/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qnetworkrequestproto.h"

#include <QByteArray>
#include <QList>
#include <QNetworkRequest>
#include <QUrl>
#include <QVariant>

#define DEBUG false

QScriptValue NetworkRequestAttributeToScriptValue(QScriptEngine *engine, const QNetworkRequest::Attribute &item)
{
  return engine->newVariant(item);
}
void NetworkRequestAttributeFromScriptValue(const QScriptValue &obj, QNetworkRequest::Attribute &item)
{
  item = (QNetworkRequest::Attribute)obj.toInt32();
}

QScriptValue NetworkRequestCacheLoadControlToScriptValue(QScriptEngine *engine, const QNetworkRequest::CacheLoadControl &item)
{
  return engine->newVariant(item);
}
void NetworkRequestCacheLoadControlFromScriptValue(const QScriptValue &obj, QNetworkRequest::CacheLoadControl &item)
{
  item = (QNetworkRequest::CacheLoadControl)obj.toInt32();
}

QScriptValue NetworkRequestKnownHeadersToScriptValue(QScriptEngine *engine, const QNetworkRequest::KnownHeaders &item)
{
  return engine->newVariant(item);
}
void NetworkRequestKnownHeadersFromScriptValue(const QScriptValue &obj, QNetworkRequest::KnownHeaders &item)
{
  item = (QNetworkRequest::KnownHeaders)obj.toInt32();
}

QScriptValue NetworkRequestLoadControlToScriptValue(QScriptEngine *engine, const QNetworkRequest::LoadControl &item)
{
  return engine->newVariant(item);
}
void NetworkRequestLoadControlFromScriptValue(const QScriptValue &obj, QNetworkRequest::LoadControl &item)
{
  item = (QNetworkRequest::LoadControl)obj.toInt32();
}

QScriptValue NetworkRequestPriorityToScriptValue(QScriptEngine *engine, const QNetworkRequest::Priority &item)
{
  return engine->newVariant(item);
}
void NetworkRequestPriorityFromScriptValue(const QScriptValue &obj, QNetworkRequest::Priority &item)
{
  item = (QNetworkRequest::Priority)obj.toInt32();
}

void setupQNetworkRequestProto(QScriptEngine *engine)
{
  if (DEBUG) qDebug("setupQNetworkRequestProto entered");

  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue netreqproto = engine->newQObject(new QNetworkRequestProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QNetworkRequest*>(), netreqproto);
  QScriptValue constructor = engine->newFunction(constructQNetworkRequest,  netreqproto);
  engine->globalObject().setProperty("QNetworkRequest", constructor);

  qScriptRegisterMetaType(engine, NetworkRequestAttributeToScriptValue, NetworkRequestAttributeFromScriptValue);
  constructor.setProperty("HttpStatusCodeAttribute", QScriptValue(engine, QNetworkRequest::HttpStatusCodeAttribute), permanent);
  constructor.setProperty("HttpReasonPhraseAttribute", QScriptValue(engine, QNetworkRequest::HttpReasonPhraseAttribute), permanent);
  constructor.setProperty("RedirectionTargetAttribute", QScriptValue(engine, QNetworkRequest::RedirectionTargetAttribute), permanent);
  constructor.setProperty("ConnectionEncryptedAttribute", QScriptValue(engine, QNetworkRequest::ConnectionEncryptedAttribute), permanent);
  constructor.setProperty("CacheLoadControlAttribute", QScriptValue(engine, QNetworkRequest::CacheLoadControlAttribute), permanent);
  constructor.setProperty("CacheSaveControlAttribute", QScriptValue(engine, QNetworkRequest::CacheSaveControlAttribute), permanent);
  constructor.setProperty("SourceIsFromCacheAttribute", QScriptValue(engine, QNetworkRequest::SourceIsFromCacheAttribute), permanent);
  constructor.setProperty("DoNotBufferUploadDataAttribute", QScriptValue(engine, QNetworkRequest::DoNotBufferUploadDataAttribute), permanent);
  constructor.setProperty("HttpPipeliningAllowedAttribute", QScriptValue(engine, QNetworkRequest::HttpPipeliningAllowedAttribute), permanent);
  constructor.setProperty("HttpPipeliningWasUsedAttribute", QScriptValue(engine, QNetworkRequest::HttpPipeliningWasUsedAttribute), permanent);
  constructor.setProperty("CustomVerbAttribute", QScriptValue(engine, QNetworkRequest::CustomVerbAttribute), permanent);
  constructor.setProperty("CookieLoadControlAttribute", QScriptValue(engine, QNetworkRequest::CookieLoadControlAttribute), permanent);
  constructor.setProperty("CookieSaveControlAttribute", QScriptValue(engine, QNetworkRequest::CookieSaveControlAttribute), permanent);
  constructor.setProperty("AuthenticationReuseAttribute", QScriptValue(engine, QNetworkRequest::AuthenticationReuseAttribute), permanent);
#if QT_VERSION >= 0x050000
  constructor.setProperty("BackgroundRequestAttribute", QScriptValue(engine, QNetworkRequest::BackgroundRequestAttribute), permanent);
  // Not needed //constructor.setProperty("SpdyAllowedAttribute", QScriptValue(engine, QNetworkRequest::SpdyAllowedAttribute), permanent);
  // Not needed //constructor.setProperty("SpdyWasUsedAttribute", QScriptValue(engine, QNetworkRequest::SpdyWasUsedAttribute), permanent);
  constructor.setProperty("EmitAllUploadProgressSignalsAttribute", QScriptValue(engine, QNetworkRequest::EmitAllUploadProgressSignalsAttribute), permanent);
#endif
  // Not in Qt 5.5 //constructor.setProperty("FollowRedirectsAttribute", QScriptValue(engine, QNetworkRequest::FollowRedirectsAttribute), permanent);
  constructor.setProperty("User", QScriptValue(engine, QNetworkRequest::User), permanent);
  constructor.setProperty("UserMax", QScriptValue(engine, QNetworkRequest::UserMax), permanent);

  qScriptRegisterMetaType(engine, NetworkRequestCacheLoadControlToScriptValue, NetworkRequestCacheLoadControlFromScriptValue);
  constructor.setProperty("AlwaysNetwork", QScriptValue(engine, QNetworkRequest::AlwaysNetwork), permanent);
  constructor.setProperty("PreferNetwork", QScriptValue(engine, QNetworkRequest::PreferNetwork), permanent);
  constructor.setProperty("PreferCache", QScriptValue(engine, QNetworkRequest::PreferCache), permanent);
  constructor.setProperty("AlwaysCache", QScriptValue(engine, QNetworkRequest::AlwaysCache), permanent);

  qScriptRegisterMetaType(engine, NetworkRequestKnownHeadersToScriptValue, NetworkRequestKnownHeadersFromScriptValue);
  constructor.setProperty("ContentDispositionHeader", QScriptValue(engine, QNetworkRequest::ContentDispositionHeader), permanent);
  constructor.setProperty("ContentTypeHeader", QScriptValue(engine, QNetworkRequest::ContentTypeHeader), permanent);
  constructor.setProperty("ContentLengthHeader", QScriptValue(engine, QNetworkRequest::ContentLengthHeader), permanent);
  constructor.setProperty("LocationHeader", QScriptValue(engine, QNetworkRequest::LocationHeader), permanent);
  constructor.setProperty("LastModifiedHeader", QScriptValue(engine, QNetworkRequest::LastModifiedHeader), permanent);
  constructor.setProperty("CookieHeader", QScriptValue(engine, QNetworkRequest::CookieHeader), permanent);
  constructor.setProperty("SetCookieHeader", QScriptValue(engine, QNetworkRequest::SetCookieHeader), permanent);
#if QT_VERSION >= 0x050000
  constructor.setProperty("UserAgentHeader", QScriptValue(engine, QNetworkRequest::UserAgentHeader), permanent);
  constructor.setProperty("ServerHeader", QScriptValue(engine, QNetworkRequest::ServerHeader), permanent);
#endif

  qScriptRegisterMetaType(engine, NetworkRequestLoadControlToScriptValue, NetworkRequestLoadControlFromScriptValue);
  constructor.setProperty("Automatic", QScriptValue(engine, QNetworkRequest::Automatic), permanent);
  constructor.setProperty("Manual", QScriptValue(engine, QNetworkRequest::Manual), permanent);

  qScriptRegisterMetaType(engine, NetworkRequestPriorityToScriptValue, NetworkRequestPriorityFromScriptValue);
  constructor.setProperty("HighPriority", QScriptValue(engine, QNetworkRequest::HighPriority), permanent);
  constructor.setProperty("NormalPriority", QScriptValue(engine, QNetworkRequest::NormalPriority), permanent);
  constructor.setProperty("LowPriority", QScriptValue(engine, QNetworkRequest::LowPriority), permanent);
}

QScriptValue constructQNetworkRequest(QScriptContext *context,
                                      QScriptEngine  *engine)
{
  if (DEBUG) qDebug("constructQNetworkRequest called");
  QNetworkRequest *req = 0;

  if (context->argumentCount() > 0)
    context->throwError(QScriptContext::UnknownError,
                        "QNetworkRequest() constructors with "
                        "arguments are not supported");
  else
    req = new QNetworkRequest();

  return engine->toScriptValue(req);
}

QNetworkRequestProto::QNetworkRequestProto(QObject *parent)
  : QObject(parent)
{
}

QVariant QNetworkRequestProto::attribute(QNetworkRequest::Attribute code, const QVariant &defaultValue) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->attribute(code, defaultValue);
  return QVariant();
}

bool QNetworkRequestProto::hasRawHeader(const QByteArray &headerName) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->hasRawHeader(headerName);
  return false;
}

QVariant QNetworkRequestProto::header(QNetworkRequest::KnownHeaders header) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->header(header);
  return QVariant();
}

QByteArray QNetworkRequestProto::rawHeader(const QByteArray &headerName) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->rawHeader(headerName);
  return QByteArray();
}

QList<QByteArray> QNetworkRequestProto::rawHeaderList() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->rawHeaderList();
  return QList<QByteArray>();
}

void QNetworkRequestProto::setAttribute(QNetworkRequest::Attribute code, const QVariant &value)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setAttribute(code, value);
}

void QNetworkRequestProto::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setHeader(header, value);
}

void QNetworkRequestProto::setRawHeader(const QByteArray &headerName, const QByteArray &headerValue)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setRawHeader(headerName, headerValue);
}

#ifndef QT_NO_OPENSSL
void QNetworkRequestProto::setSslConfiguration(const QSslConfiguration &config)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setSslConfiguration(config);
}

QSslConfiguration QNetworkRequestProto::sslConfiguration() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->sslConfiguration();
  return QSslConfiguration();
}
#endif

void QNetworkRequestProto::setUrl(const QUrl &url)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setUrl(url);
}

QUrl QNetworkRequestProto::url() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->url();
  return QUrl();
}

QString QNetworkRequestProto::toString() const
{
  return QString("[QNetworkRequest(url=%1)]")
            .arg(url().toString(QUrl::RemovePassword));
}
