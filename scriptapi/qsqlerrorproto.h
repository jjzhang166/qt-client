/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSQLERRORPROTO_H__
#define __QSQLERRORPROTO_H__

#include <QSqlError>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QSqlError*)
Q_DECLARE_METATYPE(QSqlError)
Q_DECLARE_METATYPE(enum QSqlError::ErrorType);

void setupQSqlErrorProto(QScriptEngine *engine);

QScriptValue QSqlErrorTypetoScriptValue(QScriptEngine *engine, const enum QSqlError::ErrorType &p);
void QSqlErrorTypefromScriptValue(const QScriptValue &obj, enum QSqlError::ErrorType &p);

QScriptValue constructQSqlError(QScriptContext *context, QScriptEngine *engine);

class QSqlErrorProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSqlErrorProto(QObject *parent);
    virtual ~QSqlErrorProto ();

    Q_INVOKABLE QString                 databaseText() const;
    Q_INVOKABLE QString                 driverText() const;
    Q_INVOKABLE bool                    isValid() const;

#if QT_VERSION >= 0x050000
    Q_INVOKABLE QString                 nativeErrorCode() const;
#endif

    Q_INVOKABLE QString                 text() const;
    Q_INVOKABLE QSqlError::ErrorType    type() const;

    Q_INVOKABLE QString                 toString() const;

};

#endif
