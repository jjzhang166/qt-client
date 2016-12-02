/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
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

#if QT_VERSION < 0x050000
QScriptValue FormattingOptionToScriptValue(QScriptEngine *engine, const QUrl::FormattingOption &item)
{
  return engine->newVariant(item);
}
void FormattingOptionFromScriptValue(const QScriptValue &obj, QUrl::FormattingOption &item)
{
  item = (QUrl::FormattingOption)obj.toInt32();
}

QScriptValue ParsingModeToScriptValue(QScriptEngine *engine, const QUrl::ParsingMode &item)
{
  return engine->newVariant(item);
}
void ParsingModeFromScriptValue(const QScriptValue &obj, QUrl::ParsingMode &item)
{
  item = (QUrl::ParsingMode)obj.toInt32();
}

#else
QScriptValue ComponentFormattingOptionToScriptValue(QScriptEngine *engine, const QUrl::ComponentFormattingOption &item)
{
  return engine->newVariant(item);
}
void ComponentFormattingOptionFromScriptValue(const QScriptValue &obj, QUrl::ComponentFormattingOption &item)
{
  item = (QUrl::ComponentFormattingOption)obj.toInt32();
}

QScriptValue ComponentFormattingOptionsToScriptValue(QScriptEngine *engine, const QUrl::ComponentFormattingOptions &item)
{
  return QScriptValue(engine, (int)item);
}
void ComponentFormattingOptionsFromScriptValue(const QScriptValue &obj, QUrl::ComponentFormattingOptions &item)
{
  item = (QUrl::ComponentFormattingOptions)obj.toInt32();
}

QScriptValue FormattingOptionsToScriptValue(QScriptEngine *engine, const QUrl::FormattingOptions &item)
{
    return QScriptValue(engine, (int)item);
}
void FormattingOptionsFromScriptValue(const QScriptValue &obj, QUrl::FormattingOptions &item)
{
  item = (QUrl::FormattingOptions)obj.toInt32();
}

QScriptValue ParsingModeToScriptValue(QScriptEngine *engine, const QUrl::ParsingMode &item)
{
  return engine->newVariant(item);
}
void ParsingModeFromScriptValue(const QScriptValue &obj, QUrl::ParsingMode &item)
{
  item = (QUrl::ParsingMode)obj.toInt32();
}

QScriptValue UrlFormattingOptionToScriptValue(QScriptEngine *engine, const QUrl::UrlFormattingOption &item)
{
  return engine->newVariant(item);
}
void UrlFormattingOptionFromScriptValue(const QScriptValue &obj, QUrl::UrlFormattingOption &item)
{
  item = (QUrl::UrlFormattingOption)obj.toInt32();
}

QScriptValue UserInputResolutionOptionToScriptValue(QScriptEngine *engine, const QUrl::UserInputResolutionOption &item)
{
  return engine->newVariant(item);
}
void UserInputResolutionOptionFromScriptValue(const QScriptValue &obj, QUrl::UserInputResolutionOption &item)
{
  item = (QUrl::UserInputResolutionOption)obj.toInt32();
}

QScriptValue UserInputResolutionOptionsToScriptValue(QScriptEngine *engine, const QUrl::UserInputResolutionOptions &item)
{
    return QScriptValue(engine, (int)item);
}
void UserInputResolutionOptionsFromScriptValue(const QScriptValue &obj, QUrl::UserInputResolutionOptions &item)
{
  item = (QUrl::UserInputResolutionOptions)obj.toInt32();
}

#endif

void setupQUrlProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue urlproto = engine->newQObject(new QUrlProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QUrl*>(), urlproto);
  engine->setDefaultPrototype(qMetaTypeId<QUrl>(), urlproto);

  QScriptValue constructor = engine->newFunction(constructQUrl, urlproto);
  engine->globalObject().setProperty("QUrl", constructor);

  #if QT_VERSION < 0x050000
  qScriptRegisterMetaType(engine, FormattingOptionToScriptValue, FormattingOptionFromScriptValue);
  constructor.setProperty("None", QScriptValue(engine, QUrl::None), permanent);
  constructor.setProperty("RemoveScheme", QScriptValue(engine, QUrl::RemoveScheme), permanent);
  constructor.setProperty("RemovePassword", QScriptValue(engine, QUrl::RemovePassword), permanent);
  constructor.setProperty("RemoveUserInfo", QScriptValue(engine, QUrl::RemoveUserInfo), permanent);
  constructor.setProperty("RemovePort", QScriptValue(engine, QUrl::RemovePort), permanent);
  constructor.setProperty("RemoveAuthority", QScriptValue(engine, QUrl::RemoveAuthority), permanent);
  constructor.setProperty("RemovePath", QScriptValue(engine, QUrl::RemovePath), permanent);
  constructor.setProperty("RemoveQuery", QScriptValue(engine, QUrl::RemoveQuery), permanent);
  constructor.setProperty("RemoveFragment", QScriptValue(engine, QUrl::RemoveFragment), permanent);
  constructor.setProperty("StripTrailingSlash", QScriptValue(engine, QUrl::StripTrailingSlash), permanent);
  qScriptRegisterMetaType(engine, ParsingModeToScriptValue, ParsingModeFromScriptValue);
  constructor.setProperty("TolerantMode", QScriptValue(engine, QUrl::TolerantMode), permanent);
  constructor.setProperty("StrictMode", QScriptValue(engine, QUrl::StrictMode), permanent);
  #else
  qScriptRegisterMetaType(engine, ComponentFormattingOptionToScriptValue, ComponentFormattingOptionFromScriptValue);
  constructor.setProperty("PrettyDecoded", QScriptValue(engine, QUrl::PrettyDecoded), permanent);
  constructor.setProperty("EncodeSpaces", QScriptValue(engine, QUrl::EncodeSpaces), permanent);
  constructor.setProperty("EncodeUnicode", QScriptValue(engine, QUrl::EncodeUnicode), permanent);
  constructor.setProperty("EncodeDelimiters", QScriptValue(engine, QUrl::EncodeDelimiters), permanent);
  constructor.setProperty("EncodeReserved", QScriptValue(engine, QUrl::EncodeReserved), permanent);
  constructor.setProperty("DecodeReserved", QScriptValue(engine, QUrl::DecodeReserved), permanent);
  constructor.setProperty("FullyEncoded", QScriptValue(engine, QUrl::FullyEncoded), permanent);
  constructor.setProperty("FullyDecoded", QScriptValue(engine, QUrl::FullyDecoded), permanent);
  qScriptRegisterMetaType(engine, ComponentFormattingOptionsToScriptValue, ComponentFormattingOptionsFromScriptValue);
  qScriptRegisterMetaType(engine, FormattingOptionsToScriptValue, FormattingOptionsFromScriptValue);
  qScriptRegisterMetaType(engine, ParsingModeToScriptValue, ParsingModeFromScriptValue);
  constructor.setProperty("TolerantMode", QScriptValue(engine, QUrl::TolerantMode), permanent);
  constructor.setProperty("StrictMode", QScriptValue(engine, QUrl::StrictMode), permanent);
  constructor.setProperty("DecodedMode", QScriptValue(engine, QUrl::DecodedMode), permanent);
  qScriptRegisterMetaType(engine, UrlFormattingOptionToScriptValue, UrlFormattingOptionFromScriptValue);
  constructor.setProperty("None", QScriptValue(engine, QUrl::None), permanent);
  constructor.setProperty("RemoveScheme", QScriptValue(engine, QUrl::RemoveScheme), permanent);
  constructor.setProperty("RemovePassword", QScriptValue(engine, QUrl::RemovePassword), permanent);
  constructor.setProperty("RemoveUserInfo", QScriptValue(engine, QUrl::RemoveUserInfo), permanent);
  constructor.setProperty("RemovePort", QScriptValue(engine, QUrl::RemovePort), permanent);
  constructor.setProperty("RemoveAuthority", QScriptValue(engine, QUrl::RemoveAuthority), permanent);
  constructor.setProperty("RemovePath", QScriptValue(engine, QUrl::RemovePath), permanent);
  constructor.setProperty("RemoveQuery", QScriptValue(engine, QUrl::RemoveQuery), permanent);
  constructor.setProperty("RemoveFragment", QScriptValue(engine, QUrl::RemoveFragment), permanent);
  constructor.setProperty("RemoveFilename", QScriptValue(engine, QUrl::RemoveFilename), permanent);
  constructor.setProperty("PreferLocalFile", QScriptValue(engine, QUrl::PreferLocalFile), permanent);
  constructor.setProperty("StripTrailingSlash", QScriptValue(engine, QUrl::StripTrailingSlash), permanent);
  constructor.setProperty("NormalizePathSegments", QScriptValue(engine, QUrl::NormalizePathSegments), permanent);
  qScriptRegisterMetaType(engine, UserInputResolutionOptionToScriptValue, UserInputResolutionOptionFromScriptValue);
  constructor.setProperty("DefaultResolution", QScriptValue(engine, QUrl::DefaultResolution), permanent);
  constructor.setProperty("AssumeLocalFile", QScriptValue(engine, QUrl::AssumeLocalFile), permanent);
  qScriptRegisterMetaType(engine, UserInputResolutionOptionsToScriptValue, UserInputResolutionOptionsFromScriptValue);
  #endif
}

QScriptValue constructQUrl(QScriptContext *context, QScriptEngine *engine)
{
  QUrl *url = 0;

  if (context->argumentCount() == 2)
  {
    url = new QUrl(context->argument(0).toString(),
                   (QUrl::ParsingMode)(context->argument(2).toInt32()));
  }
  else if (context->argumentCount() == 1 && context->argument(0).isString())
  {
    url = new QUrl(context->argument(0).toString());
  }
  else if (context->argumentCount() == 1 && context->argument(0).isVariant() &&
           context->argument(0).toVariant().type() == QVariant::Url)
  {
    url = new QUrl(context->argument(0).toVariant().toUrl());
  }
  else if (context->argumentCount() == 1) // && argument(0) is not a string
  {
    // url = new QUrl(dynamic_cast<QUrl>(context->argument(0).toObject()));
    context->throwError(QScriptContext::UnknownError,
                        "QUrl(QUrl &other) constructor is not yet supported");
  }
  else
    url = new QUrl();

  return engine->toScriptValue(url);
}

// QUrlProto itself
QUrlProto::QUrlProto(QObject *parent) : QObject(parent)
{
}
QUrlProto::~QUrlProto()
{
}

/*
 * There were major changes to QUrl in Qt 5.x. QUrlQuery was added and sever function moved there.
 * Several function now have parameter defaults. The versioner are split below into one large chunk
 * to keep each versions code together.
 */
// ############################################################################
// Qt 4.8 version:
// ############################################################################
#if QT_VERSION < 0x050000

void QUrlProto::addEncodedQueryItem(const QByteArray &key,
                                    const QByteArray &value)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->addEncodedQueryItem(key, value);
}

void QUrlProto::addQueryItem(const QString &key, const QString &value)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->addQueryItem(key, value);
}

QList<QByteArray> QUrlProto::allEncodedQueryItemValues(const QByteArray &key) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->allEncodedQueryItemValues(key);
  return QList<QByteArray>();
}

QStringList QUrlProto::allQueryItemValues(const QString &key) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->allQueryItemValues(key);
  return QStringList();
}

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
  if (item)
    item->clear();
}

QByteArray QUrlProto::encodedFragment() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->encodedFragment();
  return QByteArray();
}

QByteArray QUrlProto::encodedHost() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->encodedHost();
  return QByteArray();
}

QByteArray QUrlProto::encodedPassword() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->encodedPassword();
  return QByteArray();
}

QByteArray QUrlProto::encodedPath() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->encodedPath();
  return QByteArray();
}

QByteArray QUrlProto::encodedQuery() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->encodedQuery();
  return QByteArray();
}

QByteArray QUrlProto::encodedQueryItemValue(const QByteArray &key) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->encodedQueryItemValue(key);
  return QByteArray();
}

QList<QPair<QByteArray, QByteArray> > QUrlProto::encodedQueryItems() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->encodedQueryItems();
  return QList<QPair<QByteArray, QByteArray> >();
}

QByteArray QUrlProto::encodedUserName() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->encodedUserName();
  return QByteArray();
}

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

bool QUrlProto::hasEncodedQueryItem(const QByteArray &key) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->hasEncodedQueryItem(key);
  return false;
}

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
    return item->hasQueryItem(key);
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
    return item->queryItemValue(key);
  return QString();
}

QList<QPair<QString, QString> > QUrlProto::queryItems() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->queryItems();
  return QList<QPair<QString, QString> >();
}

char QUrlProto::queryPairDelimiter() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->queryPairDelimiter();
  return '\0';
}

char QUrlProto::queryValueDelimiter() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->queryValueDelimiter();
  return '\0';
}

void QUrlProto::removeAllEncodedQueryItems(const QByteArray &key)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->removeAllEncodedQueryItems(key);
}

void QUrlProto::removeAllQueryItems(const QString &key)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->removeAllQueryItems(key);
}

void QUrlProto::removeEncodedQueryItem(const QByteArray &key)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->removeEncodedQueryItem(key);
}

void QUrlProto::removeQueryItem(const QString &key)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->removeQueryItem(key);
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
  if (item)
    item->setQueryDelimiters(valueDelimiter, pairDelimiter);
}

void QUrlProto::setQueryItems(const QList<QPair<QString, QString> > &query)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setQueryItems(query);
}

void QUrlProto::setQueryItems(const QVariantMap &map)
{
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

    item->setQueryItems(query);
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

#else
// ############################################################################
// Qt 5.5.1 version
// ############################################################################
QUrl QUrlProto::adjusted(QUrl::FormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->adjusted(options);
  return QUrl();
}

QString QUrlProto::authority(QUrl::ComponentFormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->authority(options);
  return QString();
}

void QUrlProto::clear()
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->clear();
}

QString QUrlProto::errorString() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

QString QUrlProto::fileName(QUrl::ComponentFormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->fileName(options);
  return QString();
}

QString QUrlProto::fragment(QUrl::ComponentFormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->fragment(options);
  return QString();
}

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

QString QUrlProto::host(QUrl::ComponentFormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->host(options);
  return QString();
}

bool QUrlProto::isEmpty() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

bool QUrlProto::isLocalFile() const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->isLocalFile();
  return false;
}

bool QUrlProto::isParentOf(const QUrl & childUrl) const
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

bool QUrlProto::matches(const QUrl & url, QUrl::FormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->matches(url, options);
  return false;
}

QString QUrlProto::password(QUrl::ComponentFormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->password(options);
  return QString();
}

QString QUrlProto::path(QUrl::ComponentFormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->path(options);
  return QString();
}

int QUrlProto::port(int defaultPort) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->port(defaultPort);
  return 0;
}

QString QUrlProto::query(QUrl::ComponentFormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->query(options);
  return QString();
}

QUrl QUrlProto::resolved(const QUrl & relative) const
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

void QUrlProto::setAuthority(const QString & authority, QUrl::ParsingMode mode)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setAuthority(authority, mode);
}

void QUrlProto::setFragment(const QString & fragment, QUrl::ParsingMode mode)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setFragment(fragment, mode);
}

void QUrlProto::setHost(const QString & host, QUrl::ParsingMode mode)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setHost(host, mode);
}

void QUrlProto::setPassword(const QString & password, QUrl::ParsingMode mode)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setPassword(password, mode);
}

void QUrlProto::setPath(const QString & path, QUrl::ParsingMode mode)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setPath(path, mode);
}

void QUrlProto::setPort(int port)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setPort(port);
}

void QUrlProto::setQuery(const QString & query, QUrl::ParsingMode mode)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setQuery(query, mode);
}

void QUrlProto::setQuery(const QUrlQuery & query)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setQuery(query);
}

void QUrlProto::setScheme(const QString & scheme)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setScheme(scheme);
}

void QUrlProto::setUrl(const QString & url, QUrl::ParsingMode parsingMode)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setUrl(url, parsingMode);
}

void QUrlProto::setUserInfo(const QString & userInfo, QUrl::ParsingMode mode)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setUserInfo(userInfo, mode);
}

void QUrlProto::setUserName(const QString & userName, QUrl::ParsingMode mode)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->setUserName(userName, mode);
}

void QUrlProto::swap(QUrl & other)
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    item->swap(other);
}

QString QUrlProto::toDisplayString(QUrl::FormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->toDisplayString(options);
  return QString();
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
  return QString();
}

QString QUrlProto::topLevelDomain(QUrl::ComponentFormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->topLevelDomain(options);
  return QString();
}

QString QUrlProto::url(QUrl::FormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->url(options);
  return QString();
}

QString QUrlProto::userInfo(QUrl::ComponentFormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->userInfo(options);
  return QString();
}

QString QUrlProto::userName(QUrl::ComponentFormattingOptions options) const
{
  QUrl *item = qscriptvalue_cast<QUrl*>(thisObject());
  if (item)
    return item->userName(options);
  return QString();
}

#endif
