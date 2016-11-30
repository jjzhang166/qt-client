/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSQLQUERYPROTO_H__
#define __QSQLQUERYPROTO_H__

#include <QObject>
#include <QScriptEngine>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlResult>
#include <QString>
#include <QVariant>
#include <QtScript>

#include "qsqldatabaseproto.h"

Q_DECLARE_METATYPE(QSqlQuery*)
Q_DECLARE_METATYPE(QSqlQuery)
Q_DECLARE_METATYPE(enum QSqlQuery::BatchExecutionMode)

void setupQSqlQueryProto(QScriptEngine *engine);
QScriptValue constructQSqlQuery(QScriptContext *context, QScriptEngine *engine);

class QSqlQueryProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSqlQueryProto(QObject *parent);
    virtual ~QSqlQueryProto();

    Q_INVOKABLE void                            addBindValue(const QVariant &val, QSql::ParamType paramType = QSql::In);
    Q_INVOKABLE int                             at() const;
    Q_INVOKABLE void                            bindValue(const QString &placeholder, const QVariant &val, QSql::ParamType paramType = QSql::In);
    Q_INVOKABLE void                            bindValue(int pos, const QVariant &val, QSql::ParamType paramType = QSql::In);
    Q_INVOKABLE QVariant                        boundValue(const QString &placeholder) const;
    Q_INVOKABLE QVariant                        boundValue(int pos) const;
    Q_INVOKABLE QMap<QString, QVariant>         boundValues() const;
    Q_INVOKABLE void                            clear();
    Q_INVOKABLE const QSqlDriver*               driver() const;
    Q_INVOKABLE bool                            exec(const QString &query);
    Q_INVOKABLE bool                            exec();
    Q_INVOKABLE bool                            execBatch(QSqlQuery::BatchExecutionMode mode = QSqlQuery::ValuesAsRows);
    Q_INVOKABLE QString                         executedQuery() const;
    Q_INVOKABLE void                            finish();
    Q_INVOKABLE bool                            first();
    Q_INVOKABLE bool                            isActive() const;
    Q_INVOKABLE bool                            isForwardOnly() const;
    Q_INVOKABLE bool                            isNull(int field) const;
#if QT_VERSION >= 0x050000
    Q_INVOKABLE bool                            isNull(const QString &name) const;
#endif
    Q_INVOKABLE bool                            isSelect() const;
    Q_INVOKABLE bool                            isValid() const;
    Q_INVOKABLE bool                            last();
    Q_INVOKABLE QSqlError                       lastError() const;
    Q_INVOKABLE QVariant                        lastInsertId() const;
    Q_INVOKABLE QString                         lastQuery() const;
    Q_INVOKABLE bool                            next();
    Q_INVOKABLE bool                            nextResult();
    Q_INVOKABLE int                             numRowsAffected() const;
    Q_INVOKABLE QSql::NumericalPrecisionPolicy  numericalPrecisionPolicy() const;
    Q_INVOKABLE bool                            prepare(const QString &query);
    Q_INVOKABLE bool                            previous();
    Q_INVOKABLE QSqlRecord                      record() const;
    Q_INVOKABLE const QSqlResult*               result() const;
    Q_INVOKABLE bool                            seek(int index, bool relative = false);
    Q_INVOKABLE void                            setForwardOnly(bool forward);
    Q_INVOKABLE void                            setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy);
    Q_INVOKABLE int                             size() const;
    Q_INVOKABLE QVariant                        value(int index) const;

#if QT_VERSION >= 0x050000
    Q_INVOKABLE QVariant                        value(const QString &name) const;
#endif

    Q_INVOKABLE QString                         toString() const;

};

#endif
