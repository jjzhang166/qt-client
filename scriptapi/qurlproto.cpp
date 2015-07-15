/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qurlproto.h"

#include <QByteArray>
#include <QList>
#include <QSslConfiguration>
#include <QStringList>
#include <QUrl>
#include <QVariant>

#define DEBUG false

// support functions
void setupQUrlProto(QScriptEngine *engine)
{
  if (DEBUG) qDebug("setupQUrlProto entered");

  QScriptValue urlproto = engine->newQObject(new QUrlProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QUrl*>(), urlproto);

  QScriptValue urlConstructor = engine->newFunction(constructQUrl, urlproto);
  engine->globalObject().setProperty("QUrl", urlConstructor);
}

QScriptValue constructQUrl(QScriptContext *context, QScriptEngine *engine)
{
  if (DEBUG) qDebug("constructQUrl called");
  QUrl *url = 0;

  if (context->argumentCount() == 2)
  {
    if (DEBUG) qDebug("qurl(2 args)");
    url = new QUrl(context->argument(0).toString(),
                   (QUrl::ParsingMode)(context->argument(2).toInt32()));
  }
  else if (context->argumentCount() == 1 && context->argument(0).isString())
  {
    if (DEBUG) qDebug("qurl(1 string arg)");
    url = new QUrl(context->argument(0).toString());
  }
  else if (context->argumentCount() == 1 && context->argument(0).isVariant() &&
           context->argument(0).toVariant().type() == QVariant::Url)
  {
    url = new QUrl(context->argument(0).toVariant().toUrl());
  }
  else if (context->argumentCount() == 1) // && argument(0) is not a string
  {
    if (DEBUG) qDebug("qurl(1 arg that isn't a string)");
    // url = new QUrl(dynamic_cast<QUrl>(context->argument(0).toObject()));
    context->throwError(QScriptContext::UnknownError,
                        "QUrl(QUrl &other) constructor is not yet supported");
  }
  else
    url = new QUrl();

  if (DEBUG) qDebug("end result: %s", qPrintable(url->toString()));

  return engine->toScriptValue(url);
}

// QUrlProto itself
QUrlProto::QUrlProto(QObject *parent)
  : QObject(parent)
{
  // Set this instance's QUrlQuery member for Qt5+
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  _query = new QUrlQuery();
#endif
}

// addEncodedQueryItem removed from both QtUrl and QtUrlQuery classes in Qt5
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
void QUrlProto::addEncodedQueryItem(const QByteArray &key,
                                    const QByteArray &value)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->addEncodedQueryItem(key, value);
}
#endif

void QUrlProto::addQueryItem(const QString &key, const QString &value)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());

  if (item) {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
     _query->addQueryItem(key, value);
     item->setQuery(*_query);
#else
     item->addQueryItem(key, value);
#endif
  }
}

// RETURN AND PARAM TYPES CHANGED FOR Qt5
// I did not bother with Qt4 backwards compatibility directives - true backwards compatibility
// would require varying the return/param types of several methods since QStrings are now used over QByteArrays
#if QT_VERSION >= 0x050000
QStringList QUrlProto::allEncodedQueryItemValues(const QString &key) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());

  if (item) {
      return _query->allQueryItemValues(key,QUrl::FullyEncoded);
  }

  return QStringList();
}
#else
QList<QByteArray> QUrlProto::allEncodedQueryItemValues(const QByteArray &key) const {
   QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->allEncodedQueryItemValues(key);
  return QList<QByteArray>();
 }
#endif
#if QT_VERSION >= 0x050000
QStringList QUrlProto::allQueryItemValues(const QString &key) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());

  if (item) {
      return _query->allQueryItemValues(key);
  }

  return QStringList();
}
#else
QStringList QUrlProto::allQueryItemValues(const QString &key) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item) {
  return item->allQueryItemValues(key);
  }

  return QStringList();
}
#endif

QString QUrlProto::authority() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->authority();
  return QString();
}

void QUrlProto::clear()
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item) {
    item->clear();
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    _query->clear();
#endif
  }
}


// NOTE: The return types of many of the following methods have been changed from QByteArray to QString for Qt5
// This is to accommodate the new return type of the QUrl getter methods (since QStrings can now be encodings)
#if QT_VERSION >= 0x050000
QString QUrlProto::encodedFragment() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item) {
    return item->fragment(QUrl::FullyEncoded);
  }
  return QString();
}
#else
QByteArray QUrlProto::encodedFragment() const
{
   QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
   if (item) {
   return item->encodedFragment();
   }
  return QByteArray();
}
#endif
#if QT_VERSION >= 0x050000
QString QUrlProto::encodedHost() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->host(QUrl::FullyEncoded);
  return QString();
}
#else
QByteArray QUrlProto::encodedHost() const
{
   QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
   if (item) {
    return item->encodedHost();
   }
  return QByteArray();
}
#endif
#if QT_VERSION >= 0x050000
QString QUrlProto::encodedPassword() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->password(QUrl::FullyEncoded);
  return QString();
}
#else
QByteArray QUrlProto::encodedPassword() const
{
   QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
   if (item) {
    return item->encodedPassword();
   }
  return QByteArray();
}
#endif
#if QT_VERSION >= 0x050000
QString QUrlProto::encodedPath() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->path(QUrl::FullyEncoded);
  return QString();
}
#else
QByteArray QUrlProto::encodedPath() const
{
   QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
   if (item) {
    return item->encodedPath();
   }
  return QByteArray();
}
#endif
#if QT_VERSION >= 0x050000
QString QUrlProto::encodedQuery() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->query(QUrl::FullyEncoded);
  return QString();
}
#else
QByteArray QUrlProto::encodedQuery() const
{
   QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
   if (item) {
    return item->encodedQuery();
   }
  return QByteArray();
}
#endif
#if QT_VERSION >= 0x050000
QString QUrlProto::encodedQueryItemValue(const QString &key) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item) 
    return _query->queryItemValue(key, QUrl::FullyEncoded);
 
  return QString();
}
#else
QByteArray QUrlProto::encodedQueryItemValue(const QByteArray &key) const
{
   QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item) {
    return item->encodedQueryItemValue(key);
  }
  return QByteArray();
}
#endif
#if QT_VERSION >= 0x050000
QList<QPair<QString, QString> > QUrlProto::encodedQueryItems() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item) 
    return _query->queryItems(QUrl::FullyEncoded);
  
  return QList<QPair<QString, QString> >();
}
#else
QList<QPair<QByteArray, QByteArray> > QUrlProto::encodedQueryItems() const
{
   QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item) {
    return item->encodedQueryItems();
  }
  return QList<QPair<QByteArray, QByteArray> >();
}
#endif
#if QT_VERSION >= 0x050000
QString QUrlProto::encodedUserName() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->userName(QUrl::FullyEncoded);
  return QString();
}
#else
QByteArray QUrlProto::encodedUserName() const
{
   QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
   if (item) {
    return item->encodedUserName();
   }
  return QByteArray();
}
#endif

QString QUrlProto::errorString() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

QString QUrlProto::fragment() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->fragment();
  return QString();
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
bool QUrlProto::hasEncodedQueryItem(const QByteArray &key) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->hasEncodedQueryItem(key);
  return false;
}
#endif

bool QUrlProto::hasFragment() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->hasFragment();
  return false;
}

bool QUrlProto::hasQuery() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->hasQuery();
  return false;
}

bool QUrlProto::hasQueryItem(const QString &key) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    return _query->hasQueryItem(key);
#else
    return item->hasQueryItem(key);
#endif
  return false;
}

QString QUrlProto::host() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->host();
  return QString();
}

bool QUrlProto::isEmpty() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->isEmpty();
  return true;
}

bool QUrlProto::isParentOf(const QUrl &childUrl) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->isParentOf(childUrl);
  return false;
}

bool QUrlProto::isRelative() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->isRelative();
  return false;
}

bool QUrlProto::isValid() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

QString QUrlProto::password() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->password();
  return QString();
}

QString QUrlProto::path() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->path();
  return QString();
}

int QUrlProto::port() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->port();
  return 0;
}

int QUrlProto::port(int defaultPort) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->port(defaultPort);
  return 0;
}

QString QUrlProto::queryItemValue(const QString &key) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    return _query->queryItemValue(key);
#else
    return item->queryItemValue(key);
#endif
  return QString();
}

QList<QPair<QString, QString> > QUrlProto::queryItems() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    return _query->queryItems();
#else    
    return item->queryItems();
#endif
  return QList<QPair<QString, QString> >();
}

char QUrlProto::queryPairDelimiter() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    return _query->queryPairDelimiter().toLatin1();
#else
    return item->queryPairDelimiter();
#endif
  return '\0';
}

char QUrlProto::queryValueDelimiter() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    return _query->queryValueDelimiter().toLatin1();
#else
    return item->queryValueDelimiter();
#endif
  return '\0';
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
void QUrlProto::removeAllEncodedQueryItems(const QByteArray &key)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->removeAllEncodedQueryItems(key);
}
#endif

void QUrlProto::removeAllQueryItems(const QString &key)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item) {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    _query->removeAllQueryItems(key);
    item->setQuery(*_query);
#else
    item->removeAllQueryItems(key);
#endif
  }
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
void QUrlProto::removeEncodedQueryItem(const QByteArray &key)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->removeEncodedQueryItem(key);
}
#endif

void QUrlProto::removeQueryItem(const QString &key)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item) {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    _query->removeQueryItem(key);
    item->setQuery(*_query);
#else
    item->removeQueryItem(key);
#endif
  }
}


QUrl QUrlProto::resolved(const QUrl &relative) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->resolved(relative);
  return QUrl();
}

QString QUrlProto::scheme() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->scheme();
  return QString();
}

void QUrlProto::setAuthority(const QString &authority)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setAuthority(authority);
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
void QUrlProto::setEncodedFragment(const QByteArray &fragment)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setEncodedFragment(fragment);
}

void QUrlProto::setEncodedHost(const QByteArray &host)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setEncodedHost(host);
}

void QUrlProto::setEncodedPassword(const QByteArray &password)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setEncodedPassword(password);
}

void QUrlProto::setEncodedPath(const QByteArray &path)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setEncodedPath(path);
}

void QUrlProto::setEncodedQuery(const QByteArray &query)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setEncodedQuery(query);
}

void QUrlProto::setEncodedQueryItems(const QList<QPair<QByteArray, QByteArray> > &query)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setEncodedQueryItems(query);
}

void QUrlProto::setEncodedUrl(const QByteArray &encodedUrl)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setEncodedUrl(encodedUrl);
}

void QUrlProto::setEncodedUrl(const QByteArray &encodedUrl, QUrl::ParsingMode parsingMode)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setEncodedUrl(encodedUrl, parsingMode);
}

void QUrlProto::setEncodedUserName(const QByteArray &userName)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setEncodedUserName(userName);
}
#endif

void QUrlProto::setFragment(const QString &fragment)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setFragment(fragment);
}

void QUrlProto::setHost(const QString &host)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setHost(host);
}

void QUrlProto::setPassword(const QString &password)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setPassword(password);
}

void QUrlProto::setPath(const QString &path)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setPath(path);
}

void QUrlProto::setPort(int port)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setPort(port);
}

void QUrlProto::setQueryDelimiters(char valueDelimiter, char pairDelimiter)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item) {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    _query->setQueryDelimiters(valueDelimiter, pairDelimiter);
    item->setQuery(*_query);
#else
    item->setQueryDelimiters(valueDelimiter, pairDelimiter);
#endif
  }
}

void QUrlProto::setQueryItems(const QList<QPair<QString, QString> > &query)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item) {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    _query->setQueryItems(query);
    item->setQuery(*_query);
#else
    item->setQueryItems(query);
#endif
  }
}

void QUrlProto::setQueryItems(const QVariantMap &map)
{
  if (DEBUG) qDebug("setQueryItems(const QVariantMap &map) entered");
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
  {
    QList<QPair<QString, QString> > query;
    QMapIterator<QString, QVariant> i(map);
    while (i.hasNext())
    {
      i.next();
      query.append(qMakePair(i.key(), i.value().toString()));
    }

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    _query->setQueryItems(query);
    item->setQuery(*_query);
#else
    item->setQueryItems(query);
#endif
  }
}


void QUrlProto::setScheme(const QString &scheme)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setScheme(scheme);
}

void QUrlProto::setUrl(const QString &url)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setUrl(url);
}

void QUrlProto::setUrl(const QString &url, QUrl::ParsingMode parsingMode)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setUrl(url, parsingMode);
}

void QUrlProto::setUserInfo(const QString &userInfo)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setUserInfo(userInfo);
}

void QUrlProto::setUserName(const QString &userName)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setUserName(userName);
}

QByteArray QUrlProto::toEncoded(QUrl::FormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->toEncoded(options);
  return QByteArray();
}

QString QUrlProto::toLocalFile() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->toLocalFile();
  return QString();
}

QString QUrlProto::toString(QUrl::FormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->toString(options);
  return QString("[QUrl(no data available)]");
}

QString QUrlProto::userInfo() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->userInfo();
  return QString();
}

QString QUrlProto::userName() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->userName();
  return QString();
}
