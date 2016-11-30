/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QCRYPTOGRAPHICHASHPROTO_H__
#define __QCRYPTOGRAPHICHASHPROTO_H__

#include <QScriptEngine>

void setupQCryptographicHashProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QCryptographicHash>

Q_DECLARE_METATYPE(QCryptographicHash*)
Q_DECLARE_METATYPE(enum QCryptographicHash::Algorithm)

QScriptValue constructQCryptographicHash(QScriptContext *context, QScriptEngine *engine);

class QCryptographicHashProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QCryptographicHashProto(QObject *parent);
    virtual ~QCryptographicHashProto();

    Q_INVOKABLE void        addData(const char * data, int length);
    Q_INVOKABLE void        addData(const QByteArray & data);
    Q_INVOKABLE bool        addData(QIODevice * device);
    Q_INVOKABLE void        reset();
    Q_INVOKABLE QByteArray  result() const;

};

#endif
#endif
