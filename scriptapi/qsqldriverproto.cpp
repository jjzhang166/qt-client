/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsqldriverproto.h"

#if QT_VERSION < 0x050000
void setupQSqlDriverProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue DbmsTypeToScriptValue(QScriptEngine *engine, const QSqlDriver::DbmsType &item)
{
  return engine->newVariant(item);
}
void DbmsTypeFromScriptValue(const QScriptValue &obj, QSqlDriver::DbmsType &item)
{
  item = (QSqlDriver::DbmsType)obj.toInt32();
}

QScriptValue DriverFeatureToScriptValue(QScriptEngine *engine, const QSqlDriver::DriverFeature &item)
{
  return engine->newVariant(item);
}
void DriverFeatureFromScriptValue(const QScriptValue &obj, QSqlDriver::DriverFeature &item)
{
  item = (QSqlDriver::DriverFeature)obj.toInt32();
}

QScriptValue IdentifierTypeToScriptValue(QScriptEngine *engine, const QSqlDriver::IdentifierType &item)
{
  return engine->newVariant(item);
}
void IdentifierTypeFromScriptValue(const QScriptValue &obj, QSqlDriver::IdentifierType &item)
{
  item = (QSqlDriver::IdentifierType)obj.toInt32();
}

QScriptValue NotificationSourceToScriptValue(QScriptEngine *engine, const QSqlDriver::NotificationSource &item)
{
  return engine->newVariant(item);
}
void NotificationSourceFromScriptValue(const QScriptValue &obj, QSqlDriver::NotificationSource &item)
{
  item = (QSqlDriver::NotificationSource)obj.toInt32();
}

QScriptValue StatementTypeToScriptValue(QScriptEngine *engine, const QSqlDriver::StatementType &item)
{
  return engine->newVariant(item);
}
void StatementTypeFromScriptValue(const QScriptValue &obj, QSqlDriver::StatementType &item)
{
  item = (QSqlDriver::StatementType)obj.toInt32();
}

void setupQSqlDriverProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QSqlDriverProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSqlDriver*>(), proto);
  engine->globalObject().setProperty("QSqlDriver", proto);

  qScriptRegisterMetaType(engine, DbmsTypeToScriptValue, DbmsTypeFromScriptValue);
  proto.setProperty("UnknownDbms", QScriptValue(engine, QSqlDriver::UnknownDbms), permanent);
  proto.setProperty("MSSqlServer", QScriptValue(engine, QSqlDriver::MSSqlServer), permanent);
  proto.setProperty("MySqlServer", QScriptValue(engine, QSqlDriver::MySqlServer), permanent);
  proto.setProperty("PostgreSQL", QScriptValue(engine, QSqlDriver::PostgreSQL), permanent);
  proto.setProperty("Oracle", QScriptValue(engine, QSqlDriver::Oracle), permanent);
  proto.setProperty("Sybase", QScriptValue(engine, QSqlDriver::Sybase), permanent);
  proto.setProperty("SQLite", QScriptValue(engine, QSqlDriver::SQLite), permanent);
  proto.setProperty("Interbase", QScriptValue(engine, QSqlDriver::Interbase), permanent);
  proto.setProperty("DB2", QScriptValue(engine, QSqlDriver::DB2), permanent);

  qScriptRegisterMetaType(engine, DriverFeatureToScriptValue, DriverFeatureFromScriptValue);
  proto.setProperty("Transactions", QScriptValue(engine, QSqlDriver::Transactions), permanent);
  proto.setProperty("QuerySize", QScriptValue(engine, QSqlDriver::QuerySize), permanent);
  proto.setProperty("BLOB", QScriptValue(engine, QSqlDriver::BLOB), permanent);
  proto.setProperty("Unicode", QScriptValue(engine, QSqlDriver::Unicode), permanent);
  proto.setProperty("PreparedQueries", QScriptValue(engine, QSqlDriver::PreparedQueries), permanent);
  proto.setProperty("NamedPlaceholders", QScriptValue(engine, QSqlDriver::NamedPlaceholders), permanent);
  proto.setProperty("PositionalPlaceholders", QScriptValue(engine, QSqlDriver::PositionalPlaceholders), permanent);
  proto.setProperty("LastInsertId", QScriptValue(engine, QSqlDriver::LastInsertId), permanent);
  proto.setProperty("BatchOperations", QScriptValue(engine, QSqlDriver::BatchOperations), permanent);
  proto.setProperty("SimpleLocking", QScriptValue(engine, QSqlDriver::SimpleLocking), permanent);
  proto.setProperty("LowPrecisionNumbers", QScriptValue(engine, QSqlDriver::LowPrecisionNumbers), permanent);
  proto.setProperty("EventNotifications", QScriptValue(engine, QSqlDriver::EventNotifications), permanent);
  proto.setProperty("FinishQuery", QScriptValue(engine, QSqlDriver::FinishQuery), permanent);
  proto.setProperty("MultipleResultSets", QScriptValue(engine, QSqlDriver::MultipleResultSets), permanent);
  proto.setProperty("CancelQuery", QScriptValue(engine, QSqlDriver::CancelQuery), permanent);

  qScriptRegisterMetaType(engine, IdentifierTypeToScriptValue, IdentifierTypeFromScriptValue);
  proto.setProperty("FieldName", QScriptValue(engine, QSqlDriver::FieldName), permanent);
  proto.setProperty("TableName", QScriptValue(engine, QSqlDriver::TableName), permanent);

  qScriptRegisterMetaType(engine, NotificationSourceToScriptValue, NotificationSourceFromScriptValue);
  proto.setProperty("UnknownSource", QScriptValue(engine, QSqlDriver::UnknownSource), permanent);
  proto.setProperty("SelfSource", QScriptValue(engine, QSqlDriver::SelfSource), permanent);
  proto.setProperty("OtherSource", QScriptValue(engine, QSqlDriver::OtherSource), permanent);

  qScriptRegisterMetaType(engine, StatementTypeToScriptValue, StatementTypeFromScriptValue);
  proto.setProperty("WhereStatement", QScriptValue(engine, QSqlDriver::WhereStatement), permanent);
  proto.setProperty("SelectStatement", QScriptValue(engine, QSqlDriver::SelectStatement), permanent);
  proto.setProperty("UpdateStatement", QScriptValue(engine, QSqlDriver::UpdateStatement), permanent);
  proto.setProperty("InsertStatement", QScriptValue(engine, QSqlDriver::InsertStatement), permanent);
  proto.setProperty("DeleteStatement", QScriptValue(engine, QSqlDriver::DeleteStatement), permanent);
}

// QSqlDriver is an abstract class. I can only be returned, not created.
/*
QScriptValue constructQSqlDriver(QScriptContext *context, QScriptEngine  *engine)
{
  QSqlDriver *obj = 0;
  QObject *parent = 0;
  if (context->argumentCount() == 1)
  {
    parent = context->argument(0).toQObject();
    obj = new QSqlDriver(parent);
  } else {
    parent = new QObject();
    obj = new QSqlDriver(parent);
  }

  return engine->toScriptValue(obj);
}
*/

QSqlDriverProto::QSqlDriverProto(QObject *parent) : QObject(parent)
{
}
QSqlDriverProto::~QSqlDriverProto()
{
}

bool QSqlDriverProto::beginTransaction()
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->beginTransaction();
  return false;
}

void QSqlDriverProto::close()
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    item->close();
}

bool QSqlDriverProto::commitTransaction()
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->commitTransaction();
  return false;
}

QSqlResult * QSqlDriverProto::createResult() const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->createResult();
  return 0;
}

QSqlDriver::DbmsType QSqlDriverProto::dbmsType() const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->dbmsType();
  return QSqlDriver::DbmsType();
}

QString QSqlDriverProto::escapeIdentifier(const QString & identifier, QSqlDriver::IdentifierType type) const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->escapeIdentifier(identifier, type);
  return QString();
}

QString QSqlDriverProto::formatValue(const QSqlField & field, bool trimStrings) const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->formatValue(field, trimStrings);
  return QString();
}

QVariant QSqlDriverProto::handle() const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->handle();
  return QVariant();
}

bool QSqlDriverProto::hasFeature(QSqlDriver::DriverFeature feature) const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->hasFeature(feature);
  return false;
}

bool QSqlDriverProto::isIdentifierEscaped(const QString & identifier, QSqlDriver::IdentifierType type) const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->isIdentifierEscaped(identifier, type);
  return false;
}

bool QSqlDriverProto::isOpen() const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->isOpen();
  return false;
}

bool QSqlDriverProto::isOpenError() const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->isOpenError();
  return false;
}

QSqlError QSqlDriverProto::lastError() const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->lastError();
  return QSqlError();
}

/*
QSql::NumericalPrecisionPolicy QSqlDriverProto::numericalPrecisionPolicy() const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->numericalPrecisionPolicy();
  return QSql::NumericalPrecisionPolicy();
}
*/

bool QSqlDriverProto::open(const QString & db,
                           const QString & user,
                           const QString & password,
                           const QString & host,
                           int port,
                           const QString & options)
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->open(db, user, password, host, port, options);
  return false;
}

QSqlIndex QSqlDriverProto::primaryIndex(const QString & tableName) const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->primaryIndex(tableName);
  return QSqlIndex();
}

QSqlRecord QSqlDriverProto::record(const QString & tableName) const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->record(tableName);
  return QSqlRecord();
}

bool QSqlDriverProto::rollbackTransaction()
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->rollbackTransaction();
  return false;
}

/*
void QSqlDriverProto::setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy)
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    item->setNumericalPrecisionPolicy(precisionPolicy);
}
*/

QString QSqlDriverProto::sqlStatement(QSqlDriver::StatementType type,
                                      const QString & tableName,
                                      const QSqlRecord & rec,
                                      bool preparedStatement
                                     ) const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->sqlStatement(type, tableName, rec, preparedStatement);
  return QString();
}

QString QSqlDriverProto::stripDelimiters(const QString & identifier, QSqlDriver::IdentifierType type) const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->stripDelimiters(identifier, type);
  return QString();
}

bool QSqlDriverProto::subscribeToNotification(const QString & name)
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->subscribeToNotification(name);
  return false;
}

QStringList QSqlDriverProto::subscribedToNotifications() const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->subscribedToNotifications();
  return QStringList();
}

QStringList QSqlDriverProto::tables(QSql::TableType tableType) const
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->tables(tableType);
  return QStringList();
}

bool QSqlDriverProto::unsubscribeFromNotification(const QString & name)
{
  QSqlDriver *item = qscriptvalue_cast<QSqlDriver*>(thisObject());
  if (item)
    return item->unsubscribeFromNotification(name);
  return false;
}

#endif
