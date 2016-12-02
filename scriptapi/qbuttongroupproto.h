/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QBUTTONGROUP_H__
#define __QBUTTONGROUP_H__

#include <QObject>
#include <QtScript>
#include <QButtonGroup>

Q_DECLARE_METATYPE(QButtonGroup*)

void setupQButtonGroupProto(QScriptEngine *engine);
QScriptValue constructQButtonGroup(QScriptContext *context, QScriptEngine *engine);

class QButtonGroupProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QButtonGroupProto(QObject *parent);

    Q_INVOKABLE  void addButton(QAbstractButton *button);
    Q_INVOKABLE  QList<QAbstractButton *> buttons() const;
    Q_INVOKABLE  bool exclusive();
    Q_INVOKABLE  void removeButton(QAbstractButton *button);
    Q_INVOKABLE  void setExclusive(bool exclusive);
};

#endif
