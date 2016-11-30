/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsqlerrorproto.h"

QScriptValue QSqlErrorTypetoScriptValue(QScriptEngine *engine, const enum QSqlError::ErrorType &p)
{
  return QScriptValue(engine, (int)p);
}
void QSqlErrorTypefromScriptValue(const QScriptValue &obj, enum QSqlError::ErrorType &p)
{
  p = (enum QSqlError::ErrorType)obj.toInt32();
}

void setupQSqlErrorProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QSqlErrorProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSqlError*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSqlError>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQSqlError, proto);

  qScriptRegisterMetaType(engine, QSqlErrorTypetoScriptValue,	QSqlErrorTypefromScriptValue);
  constructor.setProperty("NoError",         QScriptValue(engine, QSqlError::NoError),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("ConnectionError", QScriptValue(engine, QSqlError::ConnectionError), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("StatementError",  QScriptValue(engine, QSqlError::StatementError),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("TransactionError",QScriptValue(engine, QSqlError::TransactionError),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("UnknownError",    QScriptValue(engine, QSqlError::UnknownError),    QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("QSqlError",  constructor, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

QScriptValue constructQSqlError(QScriptContext *context, QScriptEngine  *engine)
{
  QSqlError *obj = 0;

  if (context->argumentCount() == 1) {
    obj = new QSqlError(context->argument(0).toString());
  } else if (context->argumentCount() == 2) {
    obj = new QSqlError(context->argument(0).toString(), context->argument(1).toString());
  } else if (context->argumentCount() == 3) {
    obj = new QSqlError(context->argument(0).toString(), context->argument(1).toString(), (enum QSqlError::ErrorType)context->argument(2).toInt32());
  } else if (context->argumentCount() == 4) {
#if QT_VERSION >= 0x050000
    obj = new QSqlError(context->argument(0).toString(), context->argument(1).toString(), (enum QSqlError::ErrorType)context->argument(2).toInt32(), context->argument(3).toString());
#else
    obj = new QSqlError(context->argument(0).toString(), context->argument(1).toString(), (enum QSqlError::ErrorType)context->argument(2).toInt32(), context->argument(3).toInt32());
#endif
  } else {
    obj = new QSqlError();
  }

  return engine->toScriptValue(obj);
}

QSqlErrorProto::QSqlErrorProto(QObject *parent) : QObject(parent)
{
}
QSqlErrorProto::~QSqlErrorProto()
{
}

QString QSqlErrorProto::databaseText() const
{
  QSqlError *item = qscriptvalue_cast<QSqlError*>(thisObject());
  if (item) {
    return item->databaseText();
  }
  return QString();
}

QString QSqlErrorProto::driverText() const
{
  QSqlError *item = qscriptvalue_cast<QSqlError*>(thisObject());
  if (item) {
    return item->driverText();
  }
  return QString();
}

bool QSqlErrorProto::isValid() const
{
  QSqlError *item = qscriptvalue_cast<QSqlError*>(thisObject());
  if (item) {
    return item->isValid();
  }
  return false;
}

#if QT_VERSION >= 0x050000
QString QSqlErrorProto::nativeErrorCode() const
{
  QSqlError *item = qscriptvalue_cast<QSqlError*>(thisObject());
  if (item) {
    return item->nativeErrorCode();
  }
  return QString();
}
#endif

QString QSqlErrorProto::text() const
{
  QSqlError *item = qscriptvalue_cast<QSqlError*>(thisObject());
  if (item) {
    return item->text();
  }
  return QString();
}

QSqlError::ErrorType QSqlErrorProto::type() const
{
  QSqlError *item = qscriptvalue_cast<QSqlError*>(thisObject());
  if (item) {
    return item->type();
  }
  return QSqlError::ErrorType();
}

QString QSqlErrorProto::toString() const
{
  QSqlError *item = qscriptvalue_cast<QSqlError*>(thisObject());
  if (item)
    return item->text();
  return QString("QSqlError(unknown)");
}
