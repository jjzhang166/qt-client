/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XWEBSYNC_H__
#define __XWEBSYNC_H__

#include <QtScript>

void setupXWebSync(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QObject>
#include "xwebsync_p.h"

class XWebSyncPrivate;

QScriptValue constructXWebSync(QScriptContext *context, QScriptEngine *engine);

class XWebSync : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString data READ getData WRITE setData NOTIFY dataChanged)
  Q_PROPERTY(QString query READ getQuery WRITE setQuery NOTIFY queryChanged)
  Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)

  public:
    explicit XWebSync(QObject *parent);
    ~XWebSync();

    Q_INVOKABLE QString getData() const;
    Q_INVOKABLE QString getQuery() const;
    Q_INVOKABLE QString getTitle() const;
    Q_INVOKABLE void    setData(const QString &data);
    Q_INVOKABLE void    setQuery(const QString &query);
    Q_INVOKABLE void    setTitle(const QString &title);

  public slots:
    Q_INVOKABLE void sendClientMessage(const QString &message);
    Q_INVOKABLE void sendServerMessage(const QString &message);

  signals:
    void dataChanged(const QString &data);
    void executeRequest();
    void queryChanged(const QString &query);
    void receivedClientMessage(const QString &message);
    void receivedServerMessage(const QString &message);
    void titleChanged(const QString &title);

  private:
    Q_DISABLE_COPY(XWebSync)
    Q_DECLARE_PRIVATE(XWebSync)
    QScopedPointer<XWebSyncPrivate> d_ptr;
};

Q_DECLARE_METATYPE(XWebSync*)

#endif

#endif
