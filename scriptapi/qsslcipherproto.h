/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSSCIPHERPROTO_H__
#define __QSSCIPHERPROTO_H__

#include <QScriptEngine>

void setupQSslCipherProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QSsl>
#include <QSslCipher>

Q_DECLARE_METATYPE(QSslCipher*)
Q_DECLARE_METATYPE(QSslCipher)

QScriptValue constructQSslCipher(QScriptContext *context, QScriptEngine *engine);

class QSslCipherProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSslCipherProto(QObject *parent);
    virtual ~QSslCipherProto();

    Q_INVOKABLE QString             authenticationMethod() const;
    Q_INVOKABLE QString             encryptionMethod() const;
    Q_INVOKABLE bool                isNull() const;
    Q_INVOKABLE QString             keyExchangeMethod() const;
    Q_INVOKABLE QString             name() const;
    Q_INVOKABLE QSsl::SslProtocol   protocol() const;
    Q_INVOKABLE QString             protocolString() const;
    Q_INVOKABLE int                 supportedBits() const;
    Q_INVOKABLE void                swap(QSslCipher & other);
    Q_INVOKABLE int                 usedBits() const;

};

#endif
#endif
