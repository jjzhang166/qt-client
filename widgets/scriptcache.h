/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QDebug>
#include <QWidget>
#include <QScriptEngine>
#include <QScriptEngineDebugger>
#include <QScriptValue>
#include <QSqlDatabase>
#include <QSqlDriver>

#include "guiclientinterface.h"
#include "include.h"
#include "qtsetup.h"
#include "widgets.h"
#include "xsqlquery.h"

class ScriptCache : public QObject
{
  Q_OBJECT

  public:
    ScriptCache(QObject *parent = 0);
    virtual ~ScriptCache();

    QHash<int, QPair<QString, QString> > _scriptsById;
    QHash<QString, QList<int> >          _idsByName;
    QStringList                          _tablesToWatch;

  public slots:
    virtual void clear();
    virtual void sDbConnectionLost();
    virtual void sNotified(const QString &pNotification);
};
