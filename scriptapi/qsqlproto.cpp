/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsqlproto.h"

QScriptValue QSqlLocationtoScriptValue(QScriptEngine *engine, const enum QSql::Location &p)
{
  return QScriptValue(engine, (int)p);
}
void QSqlLocationfromScriptValue(const QScriptValue &obj, enum QSql::Location &p)
{
  p = (enum QSql::Location)obj.toInt32();
}

QScriptValue QSqlNumericalPrecisionPolicytoScriptValue(QScriptEngine *engine, const enum QSql::NumericalPrecisionPolicy &p)
{
  return QScriptValue(engine, (int)p);
}
void QSqlNumericalPrecisionPolicyfromScriptValue(const QScriptValue &obj, enum QSql::NumericalPrecisionPolicy &p)
{
  p = (enum QSql::NumericalPrecisionPolicy)obj.toInt32();
}

QScriptValue QSqlParamTypeFlagtoScriptValue(QScriptEngine *engine, const enum QSql::ParamTypeFlag &p)
{
  return QScriptValue(engine, (int)p);
}
void QSqlParamTypeFlagfromScriptValue(const QScriptValue &obj, enum QSql::ParamTypeFlag &p)
{
  p = (enum QSql::ParamTypeFlag)obj.toInt32();
}

QScriptValue QSqlTableTypetoScriptValue(QScriptEngine *engine, const enum QSql::TableType &p)
{
  return QScriptValue(engine, (int)p);
}
void QSqlTableTypefromScriptValue(const QScriptValue &obj, enum QSql::TableType &p)
{
  p = (enum QSql::TableType)obj.toInt32();
}

void setupQSqlProto(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  qScriptRegisterMetaType(engine, QSqlLocationtoScriptValue, QSqlLocationfromScriptValue);
  widget.setProperty("BeforeFirstRow",         QScriptValue(engine, QSql::BeforeFirstRow),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AfterLastRow",         QScriptValue(engine, QSql::AfterLastRow),         QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, QSqlNumericalPrecisionPolicytoScriptValue, QSqlNumericalPrecisionPolicyfromScriptValue);
  widget.setProperty("LowPrecisionInt32",         QScriptValue(engine, QSql::LowPrecisionInt32),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LowPrecisionInt64",         QScriptValue(engine, QSql::LowPrecisionInt64),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LowPrecisionDouble",         QScriptValue(engine, QSql::LowPrecisionDouble),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("HighPrecision",         QScriptValue(engine, QSql::HighPrecision),         QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, QSqlParamTypeFlagtoScriptValue, QSqlParamTypeFlagfromScriptValue);
  widget.setProperty("In",         QScriptValue(engine, QSql::In),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Out",         QScriptValue(engine, QSql::Out),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("InOut",         QScriptValue(engine, QSql::InOut),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Binary",         QScriptValue(engine, QSql::Binary),         QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, QSqlTableTypetoScriptValue, QSqlTableTypefromScriptValue);
  widget.setProperty("Tables",         QScriptValue(engine, QSql::Tables),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SystemTables",         QScriptValue(engine, QSql::SystemTables),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Views",         QScriptValue(engine, QSql::Views),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AllTables",         QScriptValue(engine, QSql::AllTables),         QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("QSql", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
