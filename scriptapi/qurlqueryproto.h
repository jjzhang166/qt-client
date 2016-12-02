/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QURLQUERYPROTO_H__
#define __QURLQUERYPROTO_H__

#include <QScriptEngine>

void setupQUrlQueryProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QUrlQuery>

Q_DECLARE_METATYPE(QUrlQuery*)
Q_DECLARE_METATYPE(QUrlQuery)

QScriptValue constructQUrlQuery(QScriptContext *context, QScriptEngine *engine);

class QUrlQueryProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QUrlQueryProto(QObject *parent);
    virtual ~QUrlQueryProto();

    Q_INVOKABLE void                              addQueryItem(const QString & key, const QString & value);
    Q_INVOKABLE QStringList                       allQueryItemValues(const QString & key, QUrl::ComponentFormattingOptions encoding = QUrl::PrettyDecoded) const;
    Q_INVOKABLE void                              clear();
    Q_INVOKABLE bool                              hasQueryItem(const QString & key) const;
    Q_INVOKABLE bool                              isEmpty() const;
    Q_INVOKABLE QString                           query(QUrl::ComponentFormattingOptions encoding = QUrl::PrettyDecoded) const;
    Q_INVOKABLE QString                           queryItemValue(const QString & key, QUrl::ComponentFormattingOptions encoding = QUrl::PrettyDecoded) const;
    Q_INVOKABLE QList<QPair<QString, QString> >   queryItems(QUrl::ComponentFormattingOptions encoding = QUrl::PrettyDecoded) const;
    Q_INVOKABLE QChar                             queryPairDelimiter() const;
    Q_INVOKABLE QChar                             queryValueDelimiter() const;
    Q_INVOKABLE void                              removeAllQueryItems(const QString & key);
    Q_INVOKABLE void                              removeQueryItem(const QString & key);
    Q_INVOKABLE void                              setQuery(const QString & queryString);
    Q_INVOKABLE void                              setQueryDelimiters(QChar valueDelimiter, QChar pairDelimiter);
    Q_INVOKABLE void                              setQueryItems(const QList<QPair<QString, QString> > & query);
    Q_INVOKABLE void                              swap(QUrlQuery & other);
    Q_INVOKABLE QString                           toString(QUrl::ComponentFormattingOptions encoding = QUrl::PrettyDecoded) const;

};

#endif
#endif
