/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qmenubarproto.h"

#define DEBUG false

void setupQMenuBarProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QMenuBarProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QMenuBar*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QMenu>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQMenuBar,
                                                 proto);
  engine->globalObject().setProperty("QMenuBar", constructor);
}

QScriptValue constructQMenuBar(QScriptContext *context,
                            QScriptEngine  *engine)
{
  QMenuBar *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isQObject())
    obj = new QMenuBar(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else
    obj = new QMenuBar();
  return engine->toScriptValue(obj);
}

QMenuBarProto::QMenuBarProto(QObject *parent)
    : QObject(parent)
{
}

QAction *QMenuBarProto::actionAt(const QPoint &pt) const
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->actionAt(pt);
  return 0;
}

QRect QMenuBarProto::actionGeometry(QAction *act) const
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->actionGeometry(act);
  return QRect();
}

QAction *QMenuBarProto::activeAction() const
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->activeAction();
  return 0;
}

QAction *QMenuBarProto::addAction(const QString &text)
{
  if (DEBUG) qDebug("addAction(QString = %s)", qPrintable(text));
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->addAction(text);
  return 0;
}

QAction *QMenuBarProto::addAction(const QString &text, const QObject *receiver, const char *member)
{
  if (DEBUG) qDebug("addAction(QString = %s, QObject, char* = %s)", qPrintable(text), member);
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->addAction(text, receiver, member);
  return 0;
}

void QMenuBarProto::addAction(QAction *action)
{
  // Can't use addAction here because of ambiguity problem
  if (DEBUG) qDebug("addAction(QAction)");
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    item->addAction(action);
}

QAction *QMenuBarProto::addMenu(QMenu *menu)
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->addMenu(menu);
  return 0;
}

QMenu *QMenuBarProto::addMenu(const QString &title)
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->addMenu(title);
  return 0;
}

QMenu *QMenuBarProto::addMenu(const QIcon &icon, const QString &title)
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->addMenu(icon, title);
  return 0;
}

QAction *QMenuBarProto::addSeparator()
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->addSeparator();
  return 0;
}

void QMenuBarProto::clear()
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    item->clear();
}

QWidget *QMenuBarProto::cornerWidget(Qt::Corner corner) const
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->cornerWidget(corner);
  return 0;
}

QAction *QMenuBarProto::insertMenu(QAction *before, QMenu *menu)
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->insertMenu(before, menu);
  return 0;
}

QAction *QMenuBarProto::insertSeparator(QAction *before)
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->insertSeparator(before);
  return 0;
}

bool QMenuBarProto::isDefaultUp() const
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->isDefaultUp();
  return false;
}

bool QMenuBarProto::isNativeMenuBar() const
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return item->isNativeMenuBar();
  return false;
}

void QMenuBarProto::setActiveAction(QAction *act)
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    item->setActiveAction(act);
}

void QMenuBarProto::setCornerWidget(QWidget * widget, Qt::Corner corner)
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    item->setCornerWidget(widget, corner);
}


void QMenuBarProto::setDefaultUp(bool choice)
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    item->setDefaultUp(choice);
}

void QMenuBarProto::setNativeMenuBar(bool nativeMenuBar)
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    item->setNativeMenuBar(nativeMenuBar);
}

QString QMenuBarProto::toString() const
{
  QMenuBar *item = qscriptvalue_cast<QMenuBar*>(thisObject());
  if (item)
    return QString("[QMenuBar named %1]")
                  .arg(item->objectName());
  return QString("QMenuBar(unknown)");
}
