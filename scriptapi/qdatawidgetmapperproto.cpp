/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdatawidgetmapperproto.h"

QDataWidgetMapperProto::QDataWidgetMapperProto(QObject *parent)
  : QObject(parent)
{
}

void QDataWidgetMapperProto::addMapping(QWidget *widget, int section)
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, section);
}

void QDataWidgetMapperProto::addMapping(QWidget *widget, int section, const QByteArray &propertyName)
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, section, propertyName);
}

void QDataWidgetMapperProto::clearMapping()
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    item->clearMapping();
}

QAbstractItemDelegate *QDataWidgetMapperProto::itemDelegate() const
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    return item->itemDelegate();
  return 0;
}

QByteArray QDataWidgetMapperProto::mappedPropertyName(QWidget *widget) const
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    return item->mappedPropertyName(widget);
  return QByteArray();
}

int QDataWidgetMapperProto::mappedSection(QWidget *widget) const
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    return item->mappedSection(widget);
  return 0;
}

QWidget *QDataWidgetMapperProto::mappedWidgetAt(int section) const
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    return item->mappedWidgetAt(section);
  return 0;
}

QAbstractItemModel *QDataWidgetMapperProto::model() const
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    return item->model();
  return 0;
}

void QDataWidgetMapperProto::removeMapping(QWidget *widget)
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    item->removeMapping(widget);
}

void QDataWidgetMapperProto::setItemDelegate(QAbstractItemDelegate *delegate)
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    item->setItemDelegate(delegate);
}

void QDataWidgetMapperProto::setModel(QAbstractItemModel *model)
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    item->setModel(model);
}

void QDataWidgetMapperProto::setRootIndex(const QModelIndex &index)
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    item->setRootIndex(index);
}

QModelIndex QDataWidgetMapperProto::rootIndex() const
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    return item->rootIndex();
  return QModelIndex();
}

QScriptValue QDataWidgetMappertoScriptValue(QScriptEngine *engine, QDataWidgetMapper* const &item)
{
  return engine->newQObject(item);
}

void QDataWidgetMapperfromScriptValue(const QScriptValue &obj, QDataWidgetMapper* &item)
{
  item = qobject_cast<QDataWidgetMapper*>(obj.toQObject());
}

QScriptValue constructQDataWidgetMapper(QScriptContext *context, QScriptEngine *engine)
{
  QDataWidgetMapper *obj = 0;
  if (context->argumentCount() == 0)
    obj = new QDataWidgetMapper();
  else if (context->argument(0).isQObject())
    obj = new QDataWidgetMapper(context->argument(0).toQObject());
  else
      context->throwError(QScriptContext::UnknownError,
                          "Could not find an appropriate QDataWidgetMapper constructor");

  return engine->toScriptValue(obj);
}

void setupQDataWidgetMapperProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QDataWidgetMappertoScriptValue, QDataWidgetMapperfromScriptValue);

  if (! engine->globalObject().property("QDataWidgetMapper").isFunction())
  {
    QScriptValue ctor = engine->newFunction(constructQDataWidgetMapper);
    QScriptValue meta = engine->newQMetaObject(&QDataWidgetMapper::staticMetaObject, ctor);

    engine->globalObject().setProperty("QDataWidgetMapper", meta,
                                       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  }
}

QString QDataWidgetMapperProto::toString() const
{
  QDataWidgetMapper *item = qscriptvalue_cast<QDataWidgetMapper*>(thisObject());
  if (item)
    return QString("QDataWidgetMapper()");
  return QString("QDataWidgetMapper(unknown)");
}
