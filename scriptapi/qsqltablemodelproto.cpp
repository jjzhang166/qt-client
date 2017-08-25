/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsqltablemodelproto.h"

QScriptValue EditStrategyToScriptValue(QScriptEngine *engine,
                                       QSqlTableModel::EditStrategy const &item)
{
  return QScriptValue(engine, (int)item);
}

void EditStrategyFromScriptValue(const QScriptValue &obj, QSqlTableModel::EditStrategy &item)
{
  item = (QSqlTableModel::EditStrategy)(obj.toInt32());
}

QScriptValue QSqlTableModelToScriptValue(QScriptEngine *engine, QSqlTableModel* const &item)
{
  return engine->newQObject(item);
}

void QSqlTableModelFromScriptValue(const QScriptValue &obj, QSqlTableModel* &item)
{
  item = qobject_cast<QSqlTableModel*>(obj.toQObject());
}

QScriptValue constructQSqlTableModel(QScriptContext *context, QScriptEngine *engine)
{
#if QT_VERSION >= 0x050000
  QSqlTableModel *model = 0;

  if (context->argumentCount() == 0)
    model = new QSqlTableModel();
  else if (context->argumentCount() == 1 && context->argument(0).isQObject())
    model = new QSqlTableModel(context->argument(0).toQObject());
  /* QSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
  else if (context->argumentCount() == 2 && context->argument(0).isQObject() &&
           context->argument(1).isQObject())
    model = new QSqlTableModel(context->argument(0).toQObject(),
                               qscriptvalue_cast<QSqlDatabase>(context->argument(1)));
   */
  else
    context->throwError(QScriptContext::UnknownError,
                        QString("Could not find an appropriate QSqlTableModel constructor"));

  return engine->toScriptValue(model);
#else
  Q_UNUSED(context); Q_UNUSED(engine); return QScriptValue();
#endif
}

void setupQSqlTableModelProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, EditStrategyToScriptValue,   EditStrategyFromScriptValue);
  qScriptRegisterMetaType(engine, QSqlTableModelToScriptValue, QSqlTableModelFromScriptValue);

  if (! engine->globalObject().property("QSqlTableModel").isFunction())
  {
    QScriptValue ctor = engine->newFunction(constructQSqlTableModel);
    QScriptValue meta = engine->newQMetaObject(&QSqlTableModel::staticMetaObject, ctor);

    engine->globalObject().setProperty("QSqlTableModel", meta,
                                       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  }
}

QSqlTableModelProto::QSqlTableModelProto(QObject *parent)
    : QObject(parent)
{
}

void QSqlTableModelProto::clear()
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    item->clear();
}

QVariant QSqlTableModelProto::data(const QModelIndex &idx, int role) const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->data(idx, role);
  return QVariant();
}

QSqlDatabase QSqlTableModelProto::database() const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->database();
  return QSqlDatabase();
}

QSqlTableModel::EditStrategy QSqlTableModelProto::editStrategy() const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->editStrategy();
  return QSqlTableModel::EditStrategy();
}

int QSqlTableModelProto::fieldIndex(const QString &fieldName) const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->fieldIndex(fieldName);
  return 0;
}

QString QSqlTableModelProto::filter() const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->filter();
  return QString();
}

Qt::ItemFlags QSqlTableModelProto::flags(const QModelIndex &index) const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->flags(index);
  return Qt::ItemFlags();
}

QVariant QSqlTableModelProto::headerData(int section, Qt::Orientation orientation, int role) const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->headerData(section, orientation, role);
  return QVariant();
}

bool QSqlTableModelProto::insertRecord(int row, const QSqlRecord &record)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->insertRecord(row, record);
  return false;
}

bool QSqlTableModelProto::insertRows(int row, int count, const QModelIndex &parent)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->insertRows(row, count, parent);
  return false;
}

bool QSqlTableModelProto::isDirty() const
{
#if QT_VERSION >= 0x050000
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->isDirty();
#endif
  return true;
}

bool QSqlTableModelProto::isDirty(const QModelIndex &index) const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->isDirty(index);
  return true;
}

QSqlIndex QSqlTableModelProto::primaryKey() const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->primaryKey();
  return QSqlIndex();
}

QSqlRecord QSqlTableModelProto::record() const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->record();
  return QSqlRecord();
}

QSqlRecord QSqlTableModelProto::record(int row) const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->record(row);
  return QSqlRecord();
}

bool QSqlTableModelProto::removeColumns(int column, int count, const QModelIndex &parent)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->removeColumns(column, count, parent);
  return false;
}

bool QSqlTableModelProto::removeRows(int row, int count, const QModelIndex &parent)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->removeRows(row, count, parent);
  return false;
}

void QSqlTableModelProto::revertRow(int row)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    item->revertRow(row);
}

int QSqlTableModelProto::rowCount(const QModelIndex &parent) const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->rowCount(parent);
  return 0;
}

bool QSqlTableModelProto::setData(const QModelIndex &index, const QVariant &value, int role)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->setData(index, value, role);
  return false;
}

void QSqlTableModelProto::setEditStrategy(QSqlTableModel::EditStrategy strategy)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    item->setEditStrategy(strategy);
}

void QSqlTableModelProto::setFilter(const QString &filter)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    item->setFilter(filter);
}

bool QSqlTableModelProto::setRecord(int row, const QSqlRecord &record)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->setRecord(row, record);
  return false;
}

void QSqlTableModelProto::setSort(int column, Qt::SortOrder order)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    item->setSort(column, order);
}

void QSqlTableModelProto::setTable(const QString &tableName)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    item->setTable(tableName);
}

void QSqlTableModelProto::sort(int column, Qt::SortOrder order)
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    item->sort(column, order);
}

QString QSqlTableModelProto::tableName() const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return item->tableName();
  return QString();
}

QString QSqlTableModelProto::toString() const
{
  QSqlTableModel *item = qscriptvalue_cast<QSqlTableModel*>(thisObject());
  if (item)
    return QString("QSqlTableModel()");
  return QString("QSqlTableModel(unknown)");
}
