/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSQLDRIVERPROTO_H__
#define __QSQLDRIVERPROTO_H__

#include <QScriptEngine>

void setupQSqlDriverProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlIndex>
#include <QSqlRecord>

Q_DECLARE_METATYPE(QSqlDriver*)
Q_DECLARE_METATYPE(enum QSqlDriver::DbmsType)
Q_DECLARE_METATYPE(enum QSqlDriver::DriverFeature)
Q_DECLARE_METATYPE(enum QSqlDriver::IdentifierType)
Q_DECLARE_METATYPE(enum QSqlDriver::NotificationSource)
Q_DECLARE_METATYPE(enum QSqlDriver::StatementType)

QScriptValue constructQSqlDriver(QScriptContext *context, QScriptEngine *engine);

class QSqlDriverProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSqlDriverProto(QObject *parent);
    virtual ~QSqlDriverProto();

    Q_INVOKABLE bool                            beginTransaction();
    Q_INVOKABLE void                            close();
    Q_INVOKABLE bool                            commitTransaction();
    Q_INVOKABLE QSqlResult *                    createResult() const;
    Q_INVOKABLE QSqlDriver::DbmsType            dbmsType() const;
    Q_INVOKABLE QString                         escapeIdentifier(const QString & identifier, QSqlDriver::IdentifierType type) const;
    Q_INVOKABLE QString                         formatValue(const QSqlField & field, bool trimStrings = false) const;
    Q_INVOKABLE QVariant                        handle() const;
    Q_INVOKABLE bool                            hasFeature(QSqlDriver::DriverFeature feature) const;
    Q_INVOKABLE bool                            isIdentifierEscaped(const QString & identifier, QSqlDriver::IdentifierType type) const;
    Q_INVOKABLE bool                            isOpen() const;
    Q_INVOKABLE bool                            isOpenError() const;
    Q_INVOKABLE QSqlError                       lastError() const;
    //Q_INVOKABLE QSql::NumericalPrecisionPolicy  numericalPrecisionPolicy() const;
    Q_INVOKABLE bool                            open(
                                                    const QString & db,
                                                    const QString & user = QString(),
                                                    const QString & password = QString(),
                                                    const QString & host = QString(),
                                                    int port = -1,
                                                    const QString & options = QString()
                                                );
    Q_INVOKABLE QSqlIndex                       primaryIndex(const QString & tableName) const;
    Q_INVOKABLE QSqlRecord                      record(const QString & tableName) const;
    Q_INVOKABLE bool                            rollbackTransaction();
    //Q_INVOKABLE void                            setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy);
    Q_INVOKABLE QString                         sqlStatement(
                                                    QSqlDriver::StatementType type,
                                                    const QString & tableName,
                                                    const QSqlRecord & rec,
                                                    bool preparedStatement
                                                ) const;
    Q_INVOKABLE QString                         stripDelimiters(const QString & identifier, QSqlDriver::IdentifierType type) const;
    Q_INVOKABLE bool                            subscribeToNotification(const QString & name);
    Q_INVOKABLE QStringList                     subscribedToNotifications() const;
    Q_INVOKABLE QStringList                     tables(QSql::TableType tableType) const;
    Q_INVOKABLE bool                            unsubscribeFromNotification(const QString & name);

  signals:
    void    notification(const QString & name);
    void    notification(const QString & name, QSqlDriver::NotificationSource source, const QVariant & payload);

};

#endif
#endif
