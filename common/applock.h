/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __APPLOCK_H__
#define __APPLOCK_H__

#include <QMetaType>
#include <QObject>
#include <QtScript>

class AppLockPrivate;
class QScriptEngine;

void  setupAppLock(QScriptEngine *engine);

class AppLock : public QObject
{
  Q_OBJECT

  public:
    enum AcquireMode { Silent, Interactive };

    AppLock(QObject *parent = 0);
    AppLock(QString table, int id, QObject *parent = 0);
    ~AppLock();

    Q_INVOKABLE bool    acquire(AcquireMode mode = Silent);
    Q_INVOKABLE bool    acquire(QString table, int id, AcquireMode mode = Silent);
    Q_INVOKABLE bool    holdsLock()   const;
    Q_INVOKABLE bool    isLockedOut() const;
    Q_INVOKABLE QString lastError()   const;
    Q_INVOKABLE bool    release();
    Q_INVOKABLE QString toString()    const;

  private:
    AppLockPrivate *_p;
};

Q_DECLARE_METATYPE(AppLock *)
Q_DECLARE_METATYPE(enum AppLock::AcquireMode)

void setupAppLockProto(QScriptEngine *engine);
QScriptValue constructAppLock(QScriptContext *context, QScriptEngine *engine);
class AppLockProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    AppLockProto(QObject *parent);

    Q_INVOKABLE bool    acquire(AppLock::AcquireMode mode);
    Q_INVOKABLE bool    acquire(QString table, int id, AppLock::AcquireMode mode);
    Q_INVOKABLE bool    holdsLock()   const;
    Q_INVOKABLE bool    isLockedOut() const;
    Q_INVOKABLE QString lastError()   const;
    Q_INVOKABLE bool    release();
    Q_INVOKABLE QString toString()    const;
};

#endif
