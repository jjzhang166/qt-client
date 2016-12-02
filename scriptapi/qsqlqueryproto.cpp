/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsqlqueryproto.h"

#include <QString>

QScriptValue BatchExecutionModeToScriptValue(QScriptEngine *engine, const QSqlQuery::BatchExecutionMode &item)
{
  return engine->newVariant(item);
}
void BatchExecutionModeFromScriptValue(const QScriptValue &obj, QSqlQuery::BatchExecutionMode &item)
{
  item = (QSqlQuery::BatchExecutionMode)obj.toInt32();
}

void setupQSqlQueryProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QSqlQueryProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSqlQuery*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSqlQuery>(),  proto);
  QScriptValue constructor = engine->newFunction(constructQSqlQuery, proto);

  qScriptRegisterMetaType(engine, BatchExecutionModeToScriptValue, BatchExecutionModeFromScriptValue);
  constructor.setProperty("ValuesAsRows", QScriptValue(engine, QSqlQuery::ValuesAsRows), permanent);
  constructor.setProperty("ValuesAsColumns", QScriptValue(engine, QSqlQuery::ValuesAsColumns), permanent);

  engine->globalObject().setProperty("QSqlQuery",  constructor);
}

QScriptValue constructQSqlQuery(QScriptContext *context, QScriptEngine  *engine)
{
  QSqlQuery *obj = 0;
  if (context->argumentCount() == 0) {
    obj = new QSqlQuery();
  } else if (context->argumentCount() == 1) {
    if (context->argument(0).isString()) {
      obj = new QSqlQuery(context->argument(0).toString());
    } else if (context->argument(0).isObject()) {
      // TODO: Need to expose QSqlResult for this to work.
      /*
      QSqlResult *resultItem = qscriptvalue_cast<QSqlResult*>(context->argument(0));
      if (resultItem) {
        obj = new QSqlQuery(resultItem);
      }
      */

      // TODO: This qscriptvalue_cast does not return a valid QSqlDatabase object.
      //QSqlDatabase *dbItem = qscriptvalue_cast<QSqlDatabase*>(context->argument(0));
      // Because the above qscriptvalue_cast does not work, we jump through hoops to get
      // to the database connection.
      QScriptValue connectionNameFunction = context->argument(0).property("connectionName");
      if (connectionNameFunction.isFunction()) {
        QScriptValue connectionName = connectionNameFunction.call(context->argument(0));
        QSqlDatabase dbItem = QSqlDatabase::database(connectionName.toString());
        if (dbItem.isValid()) {
          obj = new QSqlQuery(dbItem);
        } else {
          context->throwError(QScriptContext::UnknownError,
            "Could not construct a QSqlQuery with the supplied QSqlDatabase.");
        }
      } else {
        QSqlQuery *queryItem = qscriptvalue_cast<QSqlQuery*>(context->argument(0));
        if (queryItem) {
          obj = new QSqlQuery(*(queryItem));
        } else {
          context->throwError(QScriptContext::UnknownError,
            "Could not find an appropriate QSqlQueryconstructor");
        }
      }
    } else {
      context->throwError(QScriptContext::UnknownError,
        "Could not find an appropriate QSqlQueryconstructor");
    }
  } else if (context->argumentCount() == 2) {
    // TODO: This qscriptvalue_cast does not return a valid QSqlDatabase object.
    //QSqlDatabase *dbItem = qscriptvalue_cast<QSqlDatabase*>(context->argument(0));
    // Because the above qscriptvalue_cast does not work, we jump through hoops to get
    // to the database connection.
    QScriptValue connectionNameFunction = context->argument(1).property("connectionName");
    if (connectionNameFunction.isFunction()) {
      QScriptValue connectionName = connectionNameFunction.call(context->argument(1));
      QSqlDatabase dbItem = QSqlDatabase::database(connectionName.toString());
      if (context->argument(0).isString() && dbItem.isValid()) {
        obj = new QSqlQuery(context->argument(0).toString(), dbItem);
      } else {
        context->throwError(QScriptContext::UnknownError,
          "Could not construct a QSqlQuery with the supplied QSqlDatabase.");
      }
    } else {
      context->throwError(QScriptContext::UnknownError,
        "Could not find an appropriate QSqlQueryconstructor");
    }
  } else {
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QSqlQueryconstructor");
  }

  return engine->toScriptValue(obj);
}

QSqlQueryProto::QSqlQueryProto(QObject *parent) : QObject(parent)
{
}
QSqlQueryProto::~QSqlQueryProto()
{
}

void QSqlQueryProto::addBindValue(const QVariant &val, QSql::ParamType paramType)
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    item->addBindValue(val, paramType);
}

int QSqlQueryProto::at() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->at();
  return -1;
}

void QSqlQueryProto::bindValue(const QString &placeholder, const QVariant &val, QSql::ParamType paramType)
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    item->bindValue(placeholder, val, paramType);
}

void QSqlQueryProto::bindValue(int pos, const QVariant &val, QSql::ParamType paramType)
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    item->bindValue(pos, val, paramType);
}

QVariant QSqlQueryProto::boundValue(const QString &placeholder) const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->boundValue(placeholder);
  return QVariant();
}

QVariant QSqlQueryProto::boundValue(int pos) const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->boundValue(pos);
  return QVariant();
}

QMap<QString, QVariant> QSqlQueryProto::boundValues() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->boundValues();
  return QMap<QString, QVariant>();
}

void QSqlQueryProto::clear()
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    item->clear();
}

const QSqlDriver* QSqlQueryProto::driver() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->driver();
  return 0;
}

bool QSqlQueryProto::exec(const QString &query)
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->exec(query);
  return false;
}

bool QSqlQueryProto::exec()
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->exec();
  return false;
}

bool QSqlQueryProto::execBatch(QSqlQuery::BatchExecutionMode mode)
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->execBatch(mode);
  return false;
}

QString QSqlQueryProto::executedQuery() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->executedQuery();
  return QString();
}

void QSqlQueryProto::finish()
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    item->finish();
}

bool QSqlQueryProto::first()
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->first();
  return false;
}

bool QSqlQueryProto::isActive() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->isActive();
  return false;
}

bool QSqlQueryProto::isForwardOnly() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->isForwardOnly();
  return true;
}

bool QSqlQueryProto::isNull(int field) const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->isNull(field);
  return true;
}

#if QT_VERSION >= 0x050000
bool QSqlQueryProto::isNull(const QString &name) const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->isNull(name);
  return false;
}
#endif

bool QSqlQueryProto::isSelect() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->isSelect();
  return false;
}

bool QSqlQueryProto::isValid() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

bool QSqlQueryProto::last()
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->last();
  return false;
}

QSqlError QSqlQueryProto::lastError() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->lastError();
  return QSqlError(QString(), QString(), QSqlError::UnknownError);
}

QVariant QSqlQueryProto::lastInsertId() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->lastInsertId();
  return QVariant();
}

QString QSqlQueryProto::lastQuery() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->lastQuery();
  return QString();
}

bool QSqlQueryProto::next()
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->next();
  return false;
}

bool QSqlQueryProto::nextResult()
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->nextResult();
  return false;
}

int QSqlQueryProto::numRowsAffected() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->numRowsAffected();
  return -1;
}

QSql::NumericalPrecisionPolicy QSqlQueryProto::numericalPrecisionPolicy() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->numericalPrecisionPolicy();
  return QSql::NumericalPrecisionPolicy();
}

bool QSqlQueryProto::prepare(const QString &query)
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->prepare(query);
  return false;
}

bool QSqlQueryProto::previous()
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->previous();
  return false;
}

QSqlRecord QSqlQueryProto::record() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->record();
  return QSqlRecord();
}

const QSqlResult* QSqlQueryProto::result() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->result();
  return 0;
}

bool QSqlQueryProto::seek(int index, bool relative)
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->seek(index, relative);
  return false;
}

void QSqlQueryProto::setForwardOnly(bool forward)
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    item->setForwardOnly(forward);
}

void QSqlQueryProto::setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy)
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    item->setNumericalPrecisionPolicy(precisionPolicy);
}

int QSqlQueryProto::size() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->size();
  return 0;
}

QVariant QSqlQueryProto::value(int index) const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->value(index);
  return QVariant();
}

#if QT_VERSION >= 0x050000
QVariant QSqlQueryProto::value(const QString &name) const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->value(name);
  return QVariant();
}
#endif

QString QSqlQueryProto::toString() const
{
  QSqlQuery *item = qscriptvalue_cast<QSqlQuery*>(thisObject());
  if (item)
    return item->lastQuery();
  return QString("QSqlQuery(unknown)");
}
