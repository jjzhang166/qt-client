/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "metrics.h"
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QVariant>
#include "xsqlquery.h"
#include <QMessageBox>

Parameters::Parameters(QObject * parent)
  : QObject(parent)
{
  _dirty = false;
}

void Parameters::load()
{
  _values.clear();

  XSqlQuery q;
  q.prepare(_readSql);
  q.bindValue(":username", _username);
  q.exec();
  while (q.next())
    _values[q.value("key").toString()] = q.value("value").toString();
  if (q.lastError().type() != QSqlError::NoError) {
    QMessageBox::critical(0, tr("Error loading %1").arg(metaObject()->className()),
                         q.lastError().text());
    return;
  }

  _dirty = false;

  emit loaded();
}

void Parameters::sSetDirty(const QString &note)
{
    if(note == _notifyName)
        _dirty = true;
}

QString Parameters::value(const char *pName)
{
  return value(QString(pName));
}

QString Parameters::value(const QString &pName)
{
  MetricMap::iterator it = _values.find(pName);
  if (it == _values.end())
    return QString::null;
  else
    return it.value();
}

bool Parameters::boolean(const char *pName)
{
  return boolean(QString(pName));
}

bool Parameters::boolean(const QString &pName)
{
  MetricMap::iterator it = _values.find(pName);
  if (it == _values.end())
    return false;
  else if (it.value() == "t")
    return true;

  return false;
}

void Parameters::set(const char *pName, bool pValue)
{
  set(pName, QString(pValue ? "t" : "f"));
}

void Parameters::set(const QString &pName, bool pValue)
{
  set(pName, QString(pValue ? "t" : "f"));
}

void Parameters::set(const char *pName, int pValue)
{
  set(QString(pName), pValue);
}

void Parameters::set(const QString &pName, int pValue)
{
  set(QString(pName), QString::number(pValue));
}

void Parameters::set(const char *pName, const QString &pValue)
{
  set(QString(pName), pValue);
}

void Parameters::set(const QString &pName, const QString &pValue)
{
  MetricMap::iterator it = _values.find(pName);
  if (it != _values.end())
  {
    if (it.value() == pValue)
      return;
    else
      it.value() = pValue;
  }
  else
    _values[pName] = pValue;

  _set(pName, pValue);
}

void Parameters::_set(const QString &pName, QVariant pValue)
{
  XSqlQuery q;
  q.prepare(_setSql);
  q.bindValue(":username", _username);
  q.bindValue(":name", pName);
  q.bindValue(":value", pValue);
  q.exec();

  _dirty = true;
}

QString Parameters::parent(const QString &pValue)
{
  for (MetricMap::iterator it = _values.begin(); it != _values.end(); it++)
    if (it.value() == pValue)
      return it.key();

  return QString::null;
}


Metrics::Metrics()
{
  _notifyName = "metricsUpdated";
  _readSql = "SELECT metric_name AS key, metric_value AS value FROM metric;";
  _setSql  = "SELECT setMetric(:name, :value);";

  load();
}


Preferences::Preferences(const QString &pUsername)
{
  _notifyName = "preferencesUpdated";
  _readSql  = "SELECT usrpref_name AS key, usrpref_value AS value "
              "FROM usrpref "
              "WHERE (usrpref_username=:username);";
  _setSql   = "SELECT setUserPreference(:username, :name, :value);";
  _username = pUsername;

  load();
}

void Preferences::remove(const QString &pPrefName)
{
  XSqlQuery q;
  q.prepare("SELECT deleteUserPreference(:prefname);");
  q.bindValue(":prefname", pPrefName);
  q.exec();

  _dirty = true;
}


Privileges::Privileges()
{
  _notifyName = "usrprivUpdated";
  QString user;
  XSqlQuery userq("SELECT getEffectiveXtUser() AS user;");
  if (userq.lastError().type() != QSqlError::NoError)
    userq.exec("SELECT CURRENT_USER AS user;");
  if (userq.first())
    user = userq.value("user").toString();

  _readSql = QString("SELECT priv_name AS key, TEXT('t') AS value "
             "  FROM usrpriv, priv "
             " WHERE((usrpriv_priv_id=priv_id)"
             "   AND (usrpriv_username='%1')) "
             " UNION "
             "SELECT priv_name AS key, TEXT('t') AS value "
             "  FROM priv, grppriv, usrgrp"
             " WHERE((usrgrp_grp_id=grppriv_grp_id)"
             "   AND (grppriv_priv_id=priv_id)"
             "   AND (usrgrp_username='%1'));").arg(user);

  QSqlDatabase::database().driver()->subscribeToNotification("usrprivUpdated");
  QObject::connect(QSqlDatabase::database().driver(), SIGNAL(notification(const QString&)),
           this, SLOT(sSetDirty(const QString &)));

  load();
}

bool Privileges::check(const QString &pName)
{
    if (pName == "#superuser")
      return isDba();

    if(_dirty)
      load();

    MetricMap::iterator it = _values.find(pName);
    if (it != _values.end())
      return true;

    if (pName.contains(" ")) {
      QStringList privlist = pName.split(' ', QString::SkipEmptyParts);
      foreach (QString priv, privlist) {
        if (check(priv))
          return true;
      }
    }

    if (pName.contains("+")) {
      bool anded = true;
      QStringList privlist = pName.split('+', QString::SkipEmptyParts);
      foreach (QString priv, privlist) {
        anded = anded && check(priv);
      }
      return anded;
    }

 return false;
}

bool Privileges::isDba()
{
  XSqlQuery su("SELECT isDBA() AS issuper;");
  su.exec();
  if (su.first())
    return su.value("issuper").toBool();
  else if (su.lastError().type() != QSqlError::NoError)
    qWarning("SQL error in Privileges::isDba(): %s",
             qPrintable(su.lastError().text()));

  return false;
}
