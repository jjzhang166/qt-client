/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#include "xwebsync.h"
#include <QObject>

#if QT_VERSION < 0x050000
void setupXWebSync(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
void setupXWebSync(QScriptEngine *engine)
{
  QScriptValue constructor = engine->newFunction(constructXWebSync);
  QScriptValue metaObject = engine->newQMetaObject(&XWebSync::staticMetaObject, constructor);
  engine->globalObject().setProperty("XWebSync", metaObject);
}

QScriptValue constructXWebSync(QScriptContext *context, QScriptEngine *engine)
{
  QObject *parent = context->argument(0).toQObject();
  XWebSync *object = new XWebSync(parent);
  return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}

XWebSync::XWebSync(QObject *parent) :
  QObject(parent), d_ptr(new XWebSyncPrivate)
{
}
XWebSync::~XWebSync()
{
}

QString XWebSync::getData() const
{
  Q_D(const XWebSync);
  return d->data;
}

QString XWebSync::getQuery() const
{
  Q_D(const XWebSync);
  return d->query;
}

QString XWebSync::getTitle() const
{
  Q_D(const XWebSync);
  return d->title;
}

void XWebSync::sendClientMessage(const QString &message)
{
  emit receivedClientMessage(message);
}

void XWebSync::sendServerMessage(const QString &message)
{
  emit receivedServerMessage(message);
}

void XWebSync::setData(const QString &data)
{
  Q_D(XWebSync);
  if (d->data != data) {
      d->data = data;
      emit dataChanged(d->data);
  }
}

void XWebSync::setQuery(const QString &query)
{
  Q_D(XWebSync);
  if (d->query != query) {
      d->query = query;
      emit queryChanged(d->query);
  }
}

void XWebSync::setTitle(const QString &title)
{
  Q_D(XWebSync);
  if (d->title != title) {
      d->title = title;
      emit titleChanged(d->title);
  }
}

#endif
