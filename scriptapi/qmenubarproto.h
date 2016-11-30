/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QMENUBARPROTO_H__
#define __QMENUBARPROTO_H__

#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QtScript>
#include <QWidget>

#include "qactionproto.h"

Q_DECLARE_METATYPE(QMenuBar*)

void setupQMenuBarProto(QScriptEngine *engine);
QScriptValue constructQMenuBar(QScriptContext *context, QScriptEngine *engine);

class QMenuBarProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QMenuBarProto(QObject *parent);

    Q_INVOKABLE QAction *actionAt(const QPoint &pt)	const;
    Q_INVOKABLE QRect    actionGeometry(QAction *act)	const;
    Q_INVOKABLE QAction *activeAction()	                const;
    Q_INVOKABLE QAction *addAction(const QString &text);
    Q_INVOKABLE QAction *addAction(const QString &text, const QObject *receiver, const char *member);
    Q_INVOKABLE void     addAction(QAction *action);
    Q_INVOKABLE QAction *addMenu(QMenu *menu);
    Q_INVOKABLE QMenu   *addMenu(const QString &title);
    Q_INVOKABLE QMenu   *addMenu(const QIcon &icon, const QString &title);
    Q_INVOKABLE QAction *addSeparator();
    Q_INVOKABLE void     clear();
    Q_INVOKABLE QWidget *cornerWidget(Qt::Corner corner = Qt::TopRightCorner) const;
    Q_INVOKABLE QAction *insertMenu(QAction *before, QMenu *menu);
    Q_INVOKABLE QAction *insertSeparator(QAction *before);
    Q_INVOKABLE bool     isDefaultUp()                const;
    Q_INVOKABLE bool     isNativeMenuBar()	        const;
    Q_INVOKABLE void     setActiveAction(QAction *act);
    Q_INVOKABLE void     setCornerWidget(QWidget * widget, Qt::Corner corner = Qt::TopRightCorner);
    Q_INVOKABLE void     setDefaultUp(bool choice);
    Q_INVOKABLE void     setNativeMenuBar(bool nativeMenuBar);
    Q_INVOKABLE QString  toString()                     const;

};

#endif
