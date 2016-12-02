/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qapplicationproto.h"

#include <QRgb>
#include <QString>
#include <QStyle>

QScriptValue QApplicationtoScriptValue(QScriptEngine *engine, QApplication* const &item)
{ return engine->newQObject(item); }

void QApplicationfromScriptValue(const QScriptValue &obj, QApplication* &item)
{
  item = qobject_cast<QApplication*>(obj.toQObject());
}

void setupQApplicationProto(QScriptEngine *engine)
{
 qScriptRegisterMetaType(engine, QApplicationtoScriptValue, QApplicationfromScriptValue);

  QScriptValue proto = engine->newQObject(new QApplicationProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QApplication*>(), proto);
  /*
  QScriptValue constructor = engine->newFunction(constructQApplication,
                                                 proto);
  engine->globalObject().setProperty("QApplication", constructor);
  */
  engine->globalObject().setProperty("QApplication", engine->newQObject(qApp));
}

QScriptValue constructQApplication(QScriptContext  *context,
                                    QScriptEngine  *engine)
{
  Q_UNUSED(engine);
  context->throwError(QScriptContext::UnknownError,
                      QString("Cannot construct a QApplication in a script!"));
  return QScriptValue();
}

QApplicationProto::QApplicationProto(QObject *parent)
    : QCoreApplicationProto(parent)
{
}

bool QApplicationProto::notify(QObject *receiver, QEvent *e)
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->notify(receiver, e);
  return false;
}

#ifndef QT_NO_SESSIONMANAGER
bool QApplicationProto::isSessionRestored() const
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->isSessionRestored();
  return false;
}

QString QApplicationProto::sessionId() const
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->sessionId();
  return QString();
}

QString QApplicationProto::sessionKey() const
{
  QApplication *item = qscriptvalue_cast<QApplication*>(thisObject());
  if (item)
    return item->sessionKey();
  return QString();
}
#endif

QWidget *QApplicationProto::activeModalWidget()
{
  return qApp->activeModalWidget();
}

QWidget *QApplicationProto::activePopupWidget()
{
  return qApp->activePopupWidget();
}

QWidget *QApplicationProto::activeWindow()
{
  return qApp->activeWindow();
}

void QApplicationProto::alert(QWidget *widget, int msec)
{
  qApp->alert(widget, msec);
}

QWidgetList QApplicationProto::allWidgets()
{
  return qApp->allWidgets();
}

void QApplicationProto::beep()
{
  qApp->beep();
}

void QApplicationProto::changeOverrideCursor(const QCursor &cursor)
{
  qApp->changeOverrideCursor(cursor);
}

QClipboard *QApplicationProto::clipboard()
{
  return qApp->clipboard();
}

int QApplicationProto::colorSpec()
{
  return qApp->colorSpec();
}

QDesktopWidget *QApplicationProto::desktop()
{
  return qApp->desktop();
}

bool QApplicationProto::desktopSettingsAware()
{
  return qApp->desktopSettingsAware();
}

int QApplicationProto::exec()
{
  return qApp->exec();
}

QWidget *QApplicationProto::focusWidget()
{
  return qApp->focusWidget();
}

QFont QApplicationProto::font()
{
  return qApp->font();
}

QFont QApplicationProto::font(const QWidget *widget)
{
  return qApp->font(widget);
}

QFont QApplicationProto::font(const char *className)
{
  return qApp->font(className);
}

QFontMetrics QApplicationProto::fontMetrics()
{
  return qApp->fontMetrics();
}

bool QApplicationProto::isEffectEnabled(Qt::UIEffect effect)
{
  return qApp->isEffectEnabled(effect);
}

bool QApplicationProto::isLeftToRight()
{
  return qApp->isLeftToRight();
}

bool QApplicationProto::isRightToLeft()
{
  return qApp->isRightToLeft();
}

Qt::KeyboardModifiers QApplicationProto::keyboardModifiers()
{
  return qApp->keyboardModifiers();
}

Qt::MouseButtons QApplicationProto::mouseButtons()
{
  return qApp->mouseButtons();
}

QCursor *QApplicationProto::overrideCursor()
{
  return qApp->overrideCursor();
}

QPalette QApplicationProto::palette()
{
  return qApp->palette();
}

QPalette QApplicationProto::palette(const QWidget *widget)
{
  return qApp->palette(widget);
}

QPalette QApplicationProto::palette(const char *className)
{
  return qApp->palette(className);
}

Qt::KeyboardModifiers QApplicationProto::queryKeyboardModifiers()
{
  return qApp->queryKeyboardModifiers();
}

void QApplicationProto::restoreOverrideCursor()
{
  qApp->restoreOverrideCursor();
}

void QApplicationProto::setActiveWindow(QWidget *active)
{
  qApp->setActiveWindow(active);
}

void QApplicationProto::setColorSpec(int spec)
{
  qApp->setColorSpec(spec);
}

void QApplicationProto::setDesktopSettingsAware(bool on)
{
  qApp->setDesktopSettingsAware(on);
}

void QApplicationProto::setDoubleClickInterval(int ms)
{
  qApp->setDoubleClickInterval(ms);
}

void QApplicationProto::setEffectEnabled(Qt::UIEffect effect, bool enable)
{
  qApp->setEffectEnabled(effect, enable);
}

void QApplicationProto::setFont(const QFont &font, const char *className)
{
  qApp->setFont(font, className);
}

void QApplicationProto::setOverrideCursor(const QCursor &cursor)
{
  qApp->setOverrideCursor(cursor);
}

void QApplicationProto::setPalette(const QPalette &palette, const char *className)
{
  qApp->setPalette(palette, className);
}

void QApplicationProto::setQuitOnLastWindowClosed(bool quit)
{
  qApp->setQuitOnLastWindowClosed(quit);
}

void QApplicationProto::setStyle(QStyle *style)
{
  qApp->setStyle(style);
}

QStyle *QApplicationProto::setStyle(const QString &style)
{
  return qApp->setStyle(style);
}

QStyle *QApplicationProto::style()
{
  return qApp->style();
}

QWidget *QApplicationProto::topLevelAt(const QPoint &point)
{
  return qApp->topLevelAt(point);
}

QWidget *QApplicationProto::topLevelAt(int x, int y)
{
  return qApp->topLevelAt(x, y);
}

QWidgetList QApplicationProto::topLevelWidgets()
{
  return qApp->topLevelWidgets();
  return QWidgetList();
}

QWidget *QApplicationProto::widgetAt(const QPoint &point)
{
  return qApp->widgetAt(point);
}

QWidget *QApplicationProto::widgetAt(int x, int y)
{
  return qApp->widgetAt(x, y);
}

QString QApplicationProto::toString() const
{
  return qApp->arguments().join(" ");
}

