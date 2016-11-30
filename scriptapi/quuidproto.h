/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QUUIDPROTO_H__
#define __QUUIDPROTO_H__

#include <QScriptEngine>

void setupQUuidProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QUuid>

Q_DECLARE_METATYPE(QUuid*)
Q_DECLARE_METATYPE(QUuid)
Q_DECLARE_METATYPE(enum QUuid::Variant)
Q_DECLARE_METATYPE(enum QUuid::Version)

QScriptValue constructQUuid(QScriptContext *context, QScriptEngine *engine);

class QUuidProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QUuidProto(QObject *parent);
    virtual ~QUuidProto();

    Q_INVOKABLE bool            isNull() const;
    Q_INVOKABLE QByteArray      toByteArray() const;
    Q_INVOKABLE QByteArray      toRfc4122() const;
    Q_INVOKABLE QString         toString() const;
    Q_INVOKABLE QUuid::Variant  variant() const;
    Q_INVOKABLE QUuid::Version  version() const;

};

#endif
#endif
