/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QAPPLICATIONPROTO_H__
#define __QAPPLICATIONPROTO_H__

#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QObject>
#include <QPalette>
#include <QSessionManager>
#include <QtScript>

#include "qcoreapplicationproto.h"

class QString;

Q_DECLARE_METATYPE(QApplication*)

void setupQApplicationProto(QScriptEngine *engine);
QScriptValue constructQApplication(QScriptContext *context, QScriptEngine *engine);

class QApplicationProto : public QCoreApplicationProto
{
  Q_OBJECT

  public:
    QApplicationProto(QObject *parent);

    Q_INVOKABLE bool            notify(QObject *receiver, QEvent *e);

#ifndef QT_NO_SESSIONMANAGER
    Q_INVOKABLE bool            isSessionRestored() const;
    Q_INVOKABLE QString         sessionId()         const;
    Q_INVOKABLE QString         sessionKey()        const;
#endif

    Q_INVOKABLE QWidget        *activeModalWidget();
    Q_INVOKABLE QWidget        *activePopupWidget();
    Q_INVOKABLE QWidget        *activeWindow();
    Q_INVOKABLE void            alert(QWidget * widget, int msec = 0);
    Q_INVOKABLE QWidgetList     allWidgets();
    Q_INVOKABLE void            beep();
    Q_INVOKABLE void            changeOverrideCursor(const QCursor &cursor);
    Q_INVOKABLE QClipboard     *clipboard();
    Q_INVOKABLE int             colorSpec();
    Q_INVOKABLE QDesktopWidget *desktop();
    Q_INVOKABLE bool            desktopSettingsAware();
    Q_INVOKABLE int             exec();
    Q_INVOKABLE QWidget        *focusWidget();
    Q_INVOKABLE QFont           font();
    Q_INVOKABLE QFont           font(const QWidget * widget);
    Q_INVOKABLE QFont           font(const char * className);
    Q_INVOKABLE QFontMetrics    fontMetrics();
    Q_INVOKABLE bool            isEffectEnabled(Qt::UIEffect effect);
    Q_INVOKABLE bool            isLeftToRight();
    Q_INVOKABLE bool            isRightToLeft();
    Q_INVOKABLE Qt::KeyboardModifiers   keyboardModifiers();
    Q_INVOKABLE Qt::MouseButtons        mouseButtons();
    Q_INVOKABLE QCursor        *overrideCursor();
    Q_INVOKABLE QPalette        palette();
    Q_INVOKABLE QPalette        palette(const QWidget *widget);
    Q_INVOKABLE QPalette        palette(const char *className);
    Q_INVOKABLE Qt::KeyboardModifiers   queryKeyboardModifiers();
    Q_INVOKABLE void            restoreOverrideCursor();
    Q_INVOKABLE void            setActiveWindow(QWidget *active);
    Q_INVOKABLE void            setColorSpec(int spec);
    Q_INVOKABLE void            setDesktopSettingsAware(bool on);
    Q_INVOKABLE void            setDoubleClickInterval(int ms);
    Q_INVOKABLE void            setEffectEnabled(Qt::UIEffect effect, bool enable = true);
    Q_INVOKABLE void            setFont(const QFont &font, const char *className = 0);
    Q_INVOKABLE void            setOverrideCursor(const QCursor &cursor);
    Q_INVOKABLE void            setPalette(const QPalette &palette, const char *className = 0);
    Q_INVOKABLE void            setQuitOnLastWindowClosed(bool quit);
    Q_INVOKABLE void            setStyle(QStyle *style);
    Q_INVOKABLE QStyle         *setStyle(const QString &style);
    Q_INVOKABLE QStyle         *style();
    Q_INVOKABLE QWidget        *topLevelAt(const QPoint &point);
    Q_INVOKABLE QWidget        *topLevelAt(int x, int y);
    Q_INVOKABLE QWidgetList     topLevelWidgets();
    Q_INVOKABLE QWidget        *widgetAt(const QPoint &point);
    Q_INVOKABLE QWidget        *widgetAt(int x, int y);

    Q_INVOKABLE QString        toString()          const;
};

#endif
