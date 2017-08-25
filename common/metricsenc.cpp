/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "metricsenc.h"
#include "xsqlquery.h"

#include <QtScript>

#include "errorReporter.h"

Parametersenc::Parametersenc(const QString &key, QObject *parent)
  : Parameters(parent),
    _key(key)
{
}

void Parametersenc::load()
{
  _values.clear();

  XSqlQuery q;
  q.prepare(_readSql);
  q.bindValue(":username", _username);
  q.bindValue(":key",      _key);
  q.exec();
  while (q.next())
    _values[q.value("key").toString()] = q.value("value").toString();
  if (ErrorReporter::error(QtCriticalMsg, 0,
                           tr("Error loading %1").arg(metaObject()->className()),
                           q, __FILE__, __LINE__))
    return;

  _dirty = false;

  emit loaded();
}

void Parametersenc::_set(const QString &pName, QVariant pValue)
{
  XSqlQuery q;
  q.prepare(_setSql);
  q.bindValue(":username", _username);
  q.bindValue(":name", pName);
  q.bindValue(":value", pValue);
  q.bindValue(":key", _key);
  q.exec();

  _dirty = true;
}

Metricsenc::Metricsenc(const QString &pKey, QObject *parent)
  : Parametersenc(pKey, parent)
{
  _notifyName = "metricsencUpdated";
  _readSql = "SELECT metricenc_name AS key,"
             "       decrypt(setbytea(metricenc_value), setbytea(:key), 'bf') AS value"
             "  FROM metricenc;";
  _setSql  = "SELECT setMetricEnc(:name, :value, :key);";

  load();
}
