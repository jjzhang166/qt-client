/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qobjectproto.h"
QScriptValue QObjectPointerToScriptValue(QScriptEngine *engine, QObject* const &item)
{
  return engine->newQObject(item);
}
void QObjectPointerFromScriptValue(const QScriptValue &obj, QObject* &item)
{
  item = qobject_cast<QObject*>(obj.toQObject());
}

// TODO: Should connect be support in Qt Script?
/*
QScriptValue connectForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 3) {
    return context->throwError("QObject.connect() with three arguments is not supported in Qt Script.");
  } else if (context->argumentCount() == 4) {
    QObject *sender = context->argument(0).toQObject();

    if (context->argument(1).isString()) {
      QString argSignal = context->argument(1).toString();
      QByteArray baSignal = argSignal.toLatin1();
      const char *signal = baSignal.data();
      if (context->argument(3).isString()) {
        QObject *receiver = context->argument(2).toQObject();
        QString argMethod = context->argument(3).toString();
        QByteArray baMethod = argMethod.toLatin1();
        const char *method = baMethod.data();
        return engine->toScriptValue(QObject::connect(sender, signal, receiver, method));
      } else {
        return context->throwError("QObject.connect() with a PointerToMemberFunction or QMetaMethod arguments are not supported in Qt Script.");
      }
    } else {
      return context->throwError("QObject.connect() with a PointerToMemberFunction or QMetaMethod arguments are not supported in Qt Script.");
    }
  } else if (context->argumentCount() == 5) {
    QObject *sender = context->argument(0).toQObject();

    if (context->argument(1).isString()) {
      QString argSignal = context->argument(1).toString();
      QByteArray baSignal = argSignal.toLatin1();
      const char *signal = baSignal.data();
      if (context->argument(3).isString()) {
        QObject *receiver = context->argument(2).toQObject();
        QString argMethod = context->argument(3).toString();
        QByteArray baMethod = argMethod.toLatin1();
        const char *method = baMethod.data();
        Qt::ConnectionType type = (Qt::ConnectionType)context->argument(4).toInt32();
        return engine->toScriptValue(QObject::connect(sender, signal, receiver, method, type));
      } else {
        return context->throwError("QObject.connect() with a PointerToMemberFunction or QMetaMethod arguments are not supported in Qt Script.");
      }
    } else {
      return context->throwError("QObject.connect() with a PointerToMemberFunction or QMetaMethod arguments are not supported in Qt Script.");
    }
  }

  return engine->undefinedValue();
}
*/

QScriptValue disconnectForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 4) {
    QObject *sender = context->argument(0).toQObject();

    if (context->argument(1).isString()) {
      QString argSignal = context->argument(1).toString();
      QByteArray baSignal = argSignal.toLatin1();
      const char *signal = baSignal.data();
      if (context->argument(3).isString()) {
        QObject *receiver = context->argument(2).toQObject();
        QString argMethod = context->argument(3).toString();
        QByteArray baMethod = argMethod.toLatin1();
        const char *method = baMethod.data();
        return engine->toScriptValue(QObject::disconnect(sender, signal, receiver, method));
      } else {
        return context->throwError("QObject.disconnect() with a PointerToMemberFunction or QMetaMethod arguments are not supported in Qt Script.");
      }
    } else {
      return context->throwError("QObject.disconnect() with a PointerToMemberFunction or QMetaMethod arguments are not supported in Qt Script.");
    }
  } else {
    return context->throwError("QObject.disconnect() must be called with exactly four arguments.");
  }

  return engine->undefinedValue();
}

void setupQObjectProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QObjectPointerToScriptValue, QObjectPointerFromScriptValue);

  QScriptValue pointerProto = engine->newQObject(new QObjectProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QObject*>(), pointerProto);

  QScriptValue constructor = engine->newFunction(constructQObject, pointerProto);
  engine->globalObject().setProperty("QObject", constructor);

  // TODO: Should connect be support in Qt Script?
  /*
  QScriptValue connect = engine->newFunction(connectForJS);
  constructor.setProperty("connect", connect);
  */
  QScriptValue disconnect = engine->newFunction(disconnectForJS);
  constructor.setProperty("disconnect", disconnect);
}

QScriptValue constructQObject(QScriptContext *context, QScriptEngine  *engine)
{
  QObject *obj = 0;
  if (context->argumentCount() == 1) {
    obj = new QObject(context->argument(0).toQObject());
  } else {
    obj = new QObject();
  }

  return engine->toScriptValue(obj);
}

QObjectProto::QObjectProto(QObject *parent) : QObject(parent)
{
  setObjectName(QString("QObject"));
}
QObjectProto::~QObjectProto()
{
}

bool QObjectProto::blockSignals(bool block)
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->blockSignals(block);
  return false;
}

const QObjectList& QObjectProto::children() const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->children();
  return *(new QObjectList());
}

#if QT_VERSION >= 0x050000
QMetaObject::Connection QObjectProto::connect(const QObject * sender, const char * signal, const char * method, Qt::ConnectionType type) const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->connect(sender, signal, method, type);
  return QMetaObject::Connection();
}
#endif

bool QObjectProto::disconnect(const char * signal, const QObject * receiver, const char * method) const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->disconnect(signal, receiver, method);
  return false;
}

bool QObjectProto::disconnect(const QObject * receiver, const char * method) const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->disconnect(receiver, method);
  return false;
}

void QObjectProto::dumpObjectInfo()
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    item->dumpObjectInfo();
}

void QObjectProto::dumpObjectTree()
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    item->dumpObjectTree();
}

QList<QByteArray> QObjectProto::dynamicPropertyNames() const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->dynamicPropertyNames();
  return QList<QByteArray>();
}

bool QObjectProto::event(QEvent * e)
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->event(e);
  return false;
}

bool QObjectProto::eventFilter(QObject * watched, QEvent * event)
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->eventFilter(watched, event);
  return false;
}

// TODO: Does not work. `T` does not have a type
/*
T QObjectProto::findChild(const QString & name = QString(), Qt::FindChildOptions options) const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->findChild(name, options);
  return T();
}

QList<T> QObjectProto::findChildren(const QString & name = QString(), Qt::FindChildOptions options) const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->findChildren(name, options);
  return QList<T>();
}

QList<T> QObjectProto::findChildren(const QRegExp & regExp, Qt::FindChildOptions options) const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->findChildren(regExp, options);
  return QList<T>();
}

QList<T> QObjectProto::findChildren(const QRegularExpression & re, Qt::FindChildOptions options) const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->findChildren(re, options);
  return QList<T>();
}
*/

bool QObjectProto::inherits(const char * className) const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->inherits(className);
  return false;
}

void QObjectProto::installEventFilter(QObject * filterObj)
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    item->installEventFilter(filterObj);
}

bool QObjectProto::isWidgetType() const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->isWidgetType();
  return false;
}

#if QT_VERSION >= 0x050000
bool QObjectProto::isWindowType() const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->isWindowType();
  return false;
}
#endif

void QObjectProto::killTimer(int id)
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    item->killTimer(id);
}

// TODO: Does not work. `virtual const QMetaObject* QObjectProto::metaObject() const` cannot be overloaded
/*
virtual const QMetaObject* QObjectProto::metaObject() const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->metaObject();
  return virtual();
}
*/

void QObjectProto::moveToThread(QThread * targetThread)
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    item->moveToThread(targetThread);
}

QString QObjectProto::objectName() const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->objectName();
  return QString();
}

QObject* QObjectProto::parent() const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->parent();
  return new QObject();
}

QVariant QObjectProto::property(const char * name) const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->property(name);
  return QVariant();
}

void QObjectProto::removeEventFilter(QObject * obj)
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    item->removeEventFilter(obj);
}

void QObjectProto::setObjectName(const QString & name)
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    item->setObjectName(name);
}

void QObjectProto::setParent(QObject * parent)
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    item->setParent(parent);
}

bool QObjectProto::setProperty(const char * name, const QVariant & value)
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->setProperty(name, value);
  return false;
}

bool QObjectProto::signalsBlocked() const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->signalsBlocked();
  return false;
}

#if QT_VERSION >= 0x050000
int QObjectProto::startTimer(int interval, Qt::TimerType timerType)
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->startTimer(interval, timerType);
  return 0;
}
#endif

QThread* QObjectProto::thread() const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->thread();
  return new QThread();
}

void QObjectProto::deleteLater()
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    item->deleteLater();
}

QString QObjectProto::toString() const
{
  QObject *item = qscriptvalue_cast<QObject*>(thisObject());
  if (item)
    return item->objectName();
  return QString("QObject(unknown)");
}
