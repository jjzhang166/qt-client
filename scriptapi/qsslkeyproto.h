/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSSLKEYPROTO_H__
#define __QSSLKEYPROTO_H__

#include <QScriptEngine>

void setupQSslKeyProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QSslKey>
#include <QTextCodec>

Q_DECLARE_METATYPE(QSslKey*)
Q_DECLARE_METATYPE(QSslKey)

QScriptValue constructQSslKey(QScriptContext *context, QScriptEngine *engine);

class QSslKeyProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSslKeyProto(QObject *parent);
    virtual ~QSslKeyProto();

    Q_INVOKABLE QSsl::KeyAlgorithm  algorithm() const;
    Q_INVOKABLE void                clear();
    Q_INVOKABLE bool                isNull() const;
    Q_INVOKABLE int                 length() const;
    Q_INVOKABLE void                swap(QSslKey & other);
    Q_INVOKABLE QByteArray          toDer(const QByteArray & passPhrase = QByteArray()) const;
    Q_INVOKABLE QByteArray          toPem(const QByteArray & passPhrase = QByteArray()) const;
    Q_INVOKABLE QSsl::KeyType       type() const;

};

#endif
#endif
