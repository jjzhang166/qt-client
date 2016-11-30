/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebsecurityoriginproto.h"
#include <QScriptValueIterator>

#if QT_VERSION < 0x050000
void setupQWebSecurityOriginProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
/*
QScriptValue QWebSecurityOriginToScriptValue(QScriptEngine *engine, QWebSecurityOrigin const &item)
{
  QScriptValue obj = engine->newObject();
  QUrl url = QUrl();
  url.setHost(item.host());
  url.setPort(item.port());
  url.setScheme(item.scheme());
  obj.setProperty("_url", qPrintable(url.toString()));
  return obj;
}
void QWebSecurityOriginFromScriptValue(const QScriptValue &obj, QWebSecurityOrigin &item)
{
  QString url = obj.property("_url").toString();
  QUrl newUrl = QUrl(url);
  item = QWebSecurityOrigin(newUrl);
}
*/

QScriptValue QWebSecurityOriginPointerToScriptValue(QScriptEngine *engine, QWebSecurityOrigin* const &item)
{
  QScriptValue obj = engine->newObject();
  QUrl url = QUrl();
  url.setHost(item->host());
  url.setPort(item->port());
  url.setScheme(item->scheme());
  obj.setProperty("_url", qPrintable(url.toString()));
  return obj;
}
void QWebSecurityOriginPointerFromScriptValue(const QScriptValue &obj, QWebSecurityOrigin* &item)
{
  QString url = obj.property("_url").toString();
  QUrl newUrl = QUrl(url);
  item = new QWebSecurityOrigin(newUrl);
}

QScriptValue SubdomainSettingToScriptValue(QScriptEngine *engine, const QWebSecurityOrigin::SubdomainSetting &item)
{
  return engine->newVariant(item);
}
void SubdomainSettingFromScriptValue(const QScriptValue &obj, QWebSecurityOrigin::SubdomainSetting &item)
{
  item = (QWebSecurityOrigin::SubdomainSetting)obj.toInt32();
}

// TODO: error: no matching function for call to 'QWebSecurityOrigin::QWebSecurityOrigin()'
// FROM line: `QWebSecurityOrigin item = qscriptvalue_cast<QWebSecurityOrigin>(it.value());`
/*
QScriptValue QListQWebSecurityOriginToScriptValue(QScriptEngine *engine, const QList<QWebSecurityOrigin> &list)
{
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < list.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(list.at(i)));
  }
  return newArray;
}
void QListQWebSecurityOriginFromScriptValue(const QScriptValue &obj, QList<QWebSecurityOrigin> &list)
{
  list = QList<QWebSecurityOrigin>();
  QScriptValueIterator it(obj);

  while (it.hasNext()) {
    it.next();
    if (it.flags() & QScriptValue::SkipInEnumeration)
      continue;
    QWebSecurityOrigin item = qscriptvalue_cast<QWebSecurityOrigin>(it.value());
    list.insert(it.name().toInt(), item);
  }
}
*/

QScriptValue addLocalSchemeForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QWebSecurityOrigin::addLocalScheme(context->argument(0).toString());
  }

  return engine->undefinedValue();
}

//QScriptValue allOriginsForJS(QScriptContext* /*context*/, QScriptEngine* engine)
//{
//  return engine->toScriptValue(QWebSecurityOrigin::allOrigins());
//}

QScriptValue localSchemesForJS(QScriptContext* /*context*/, QScriptEngine* engine)
{
  return engine->toScriptValue(QWebSecurityOrigin::localSchemes());
}

QScriptValue removeLocalSchemeForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QWebSecurityOrigin::removeLocalScheme(context->argument(0).toString());
  }

  return engine->undefinedValue();
}

void setupQWebSecurityOriginProto(QScriptEngine *engine)
{
  //qScriptRegisterMetaType(engine, QWebSecurityOriginToScriptValue, QWebSecurityOriginFromScriptValue);
  qScriptRegisterMetaType(engine, QWebSecurityOriginPointerToScriptValue, QWebSecurityOriginPointerFromScriptValue);
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  // TODO:
  //qScriptRegisterMetaType(engine, QListQWebSecurityOriginToScriptValue, QListQWebSecurityOriginFromScriptValue);

  QScriptValue proto = engine->newQObject(new QWebSecurityOriginProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebSecurityOrigin*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QWebSecurityOrigin>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQWebSecurityOrigin, proto);
  engine->globalObject().setProperty("QWebSecurityOrigin", constructor);

  qScriptRegisterMetaType(engine, SubdomainSettingToScriptValue, SubdomainSettingFromScriptValue);
  constructor.setProperty("AllowSubdomains", QScriptValue(engine, QWebSecurityOrigin::AllowSubdomains), permanent);
  constructor.setProperty("DisallowSubdomains", QScriptValue(engine, QWebSecurityOrigin::DisallowSubdomains), permanent);

  QScriptValue addLocalScheme = engine->newFunction(addLocalSchemeForJS);
  constructor.setProperty("addLocalScheme", addLocalScheme);
  //QScriptValue allOrigins = engine->newFunction(allOriginsForJS);
  //constructor.setProperty("allOrigins", allOrigins);
  QScriptValue localSchemes = engine->newFunction(localSchemesForJS);
  constructor.setProperty("localSchemes", localSchemes);
  QScriptValue removeLocalScheme = engine->newFunction(removeLocalSchemeForJS);
  constructor.setProperty("removeLocalScheme", removeLocalScheme);
}

QScriptValue constructQWebSecurityOrigin(QScriptContext *context, QScriptEngine  *engine)
{
  QWebSecurityOrigin *obj = 0;
  if (context->argumentCount() == 1) {
    QUrl url = qscriptvalue_cast<QUrl>(context->argument(0));
    if (url.toString().length() > 0) {
      obj = new QWebSecurityOrigin(url);
    } else {
      // TODO: error: no matching function for call to 'QWebSecurityOrigin::QWebSecurityOrigin()'
      // The argument must be a QWebSecurityOrigin.
      //QWebSecurityOrigin other = qscriptvalue_cast<QWebSecurityOrigin>(context->argument(0));
      //obj = new QWebSecurityOrigin(other);
    }
  } else {
    context->throwError(QScriptContext::UnknownError,
                        "No QUrl or QWebSecurityOrigin argument provided to the QWebSecurityOriginconstructor");
  }

  return engine->toScriptValue(obj);
}

QWebSecurityOriginProto::QWebSecurityOriginProto(QObject *parent) : QObject(parent)
{
}
QWebSecurityOriginProto::~QWebSecurityOriginProto()
{
}

void QWebSecurityOriginProto::addAccessWhitelistEntry(const QString & scheme, const QString & host, QWebSecurityOrigin::SubdomainSetting subdomainSetting)
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    item->addAccessWhitelistEntry(scheme, host, subdomainSetting);
}

qint64 QWebSecurityOriginProto::databaseQuota() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->databaseQuota();
  return 0;
}

qint64 QWebSecurityOriginProto::databaseUsage() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->databaseUsage();
  return 0;
}

// TODO: QWebDatabase and therefore QList<QWebDatabase> are not exposed yet.
/*
QList<QWebDatabase> QWebSecurityOriginProto::databases() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->databases();
  return QList<QWebDatabase>();
}
*/

QString QWebSecurityOriginProto::host() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->host();
  return QString();
}

int QWebSecurityOriginProto::port() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->port();
  return 0;
}

void QWebSecurityOriginProto::removeAccessWhitelistEntry(const QString & scheme, const QString & host, QWebSecurityOrigin::SubdomainSetting subdomainSetting)
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    item->removeAccessWhitelistEntry(scheme, host, subdomainSetting);
}

QString QWebSecurityOriginProto::scheme() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->scheme();
  return QString();
}

void QWebSecurityOriginProto::setApplicationCacheQuota(qint64 quota)
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    item->setApplicationCacheQuota(quota);
}

void QWebSecurityOriginProto::setDatabaseQuota(qint64 quota)
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    item->setDatabaseQuota(quota);
}

#endif
