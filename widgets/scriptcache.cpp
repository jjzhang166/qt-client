/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptcache.h"

#include <QSqlDatabase>
#include <QSqlDriver>

ScriptCache::ScriptCache(QObject *parent)
  : QObject(parent)
{
  _tablesToWatch << "pkghead" << "script" << "pkgscript";

  QSqlDatabase db = QSqlDatabase::database();
  foreach (QString tableName, _tablesToWatch)
  {
    if (! db.driver()->subscribedToNotifications().contains(tableName))
      db.driver()->subscribeToNotification(tableName);
  }
  QObject::connect(db.driver(), SIGNAL(notification(const QString&)), this, SLOT(sNotified(const QString &)));
  if (parent)
    connect(parent, SIGNAL(dbConnectionLost()), this, SLOT(sDbConnectionLost()));
}

ScriptCache::~ScriptCache()
{
  clear();
}

void ScriptCache::clear()
{
  _scriptsById.clear();
  _idsByName.clear();
}

void ScriptCache::sDbConnectionLost()
{
  clear();
}

void ScriptCache::sNotified(const QString &pNotification)
{
  if (_tablesToWatch.contains(pNotification))
    clear();
}
