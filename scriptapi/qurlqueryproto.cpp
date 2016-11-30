/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qurlqueryproto.h"

#if QT_VERSION < 0x050000
void setupQUrlQueryProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue defaultQueryPairDelimiterForJS(QScriptContext* /*context*/, QScriptEngine* engine)
{
  return engine->toScriptValue(QUrlQuery::defaultQueryPairDelimiter());
}

QScriptValue defaultQueryValueDelimiterForJS(QScriptContext* /*context*/, QScriptEngine* engine)
{
  return engine->toScriptValue(QUrlQuery::defaultQueryValueDelimiter());
}

void setupQUrlQueryProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QUrlQueryProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QUrlQuery*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QUrlQuery>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQUrlQuery, proto);
  engine->globalObject().setProperty("QUrlQuery", constructor);

  QScriptValue defaultQueryPairDelimiter = engine->newFunction(defaultQueryPairDelimiterForJS);
  constructor.setProperty("defaultQueryPairDelimiter", defaultQueryPairDelimiter);
  QScriptValue defaultQueryValueDelimiter = engine->newFunction(defaultQueryValueDelimiterForJS);
  constructor.setProperty("defaultQueryValueDelimiter", defaultQueryValueDelimiter);
}

QScriptValue constructQUrlQuery(QScriptContext *context, QScriptEngine  *engine)
{
  QUrlQuery *obj = 0;
  if (context->argumentCount() == 1) {
    obj = new QUrlQuery(context->argument(0).toString());
  } else {
    obj = new QUrlQuery();
  }

  return engine->toScriptValue(obj);
}

QUrlQueryProto::QUrlQueryProto(QObject *parent) : QObject(parent)
{
}
QUrlQueryProto::~QUrlQueryProto()
{
}

void QUrlQueryProto::addQueryItem(const QString & key, const QString & value)
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    item->addQueryItem(key, value);
}

QStringList QUrlQueryProto::allQueryItemValues(const QString & key, QUrl::ComponentFormattingOptions encoding) const
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    return item->allQueryItemValues(key, encoding);
  return QStringList();
}

void QUrlQueryProto::clear()
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    item->clear();
}

bool QUrlQueryProto::hasQueryItem(const QString & key) const
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    return item->hasQueryItem(key);
  return false;
}

bool QUrlQueryProto::isEmpty() const
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

QString QUrlQueryProto::query(QUrl::ComponentFormattingOptions encoding) const
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    return item->query(encoding);
  return QString();
}

QString QUrlQueryProto::queryItemValue(const QString & key, QUrl::ComponentFormattingOptions encoding) const
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    return item->queryItemValue(key, encoding);
  return QString();
}

QList<QPair<QString, QString> > QUrlQueryProto::queryItems(QUrl::ComponentFormattingOptions encoding) const
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    return item->queryItems(encoding);
  return QList<QPair<QString, QString> >();
}

QChar QUrlQueryProto::queryPairDelimiter() const
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    return item->queryPairDelimiter();
  return QChar();
}

QChar QUrlQueryProto::queryValueDelimiter() const
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    return item->queryValueDelimiter();
  return QChar();
}

void QUrlQueryProto::removeAllQueryItems(const QString & key)
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    item->removeAllQueryItems(key);
}

void QUrlQueryProto::removeQueryItem(const QString & key)
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    item->removeQueryItem(key);
}

void QUrlQueryProto::setQuery(const QString & queryString)
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    item->setQuery(queryString);
}

void QUrlQueryProto::setQueryDelimiters(QChar valueDelimiter, QChar pairDelimiter)
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    item->setQueryDelimiters(valueDelimiter, pairDelimiter);
}

void QUrlQueryProto::setQueryItems(const QList<QPair<QString, QString> > & query)
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    item->setQueryItems(query);
}

void QUrlQueryProto::swap(QUrlQuery & other)
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    item->swap(other);
}

QString QUrlQueryProto::toString(QUrl::ComponentFormattingOptions encoding) const
{
  QUrlQuery *item = qscriptvalue_cast<QUrlQuery*>(thisObject());
  if (item)
    return item->toString(encoding);
  return QString();
}

#endif
