/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSSLPRESHAREKEYAUTHENTICATORPROTO_H__
#define __QSSLPRESHAREKEYAUTHENTICATORPROTO_H__

#include <QScriptEngine>

void setupQSslPreSharedKeyAuthenticatorProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QSslPreSharedKeyAuthenticator>

//Q_DECLARE_METATYPE(QSslPreSharedKeyAuthenticator*) // Already set in qsslpresharedkeyauthenticator.h
//Q_DECLARE_METATYPE(QSslPreSharedKeyAuthenticator) // Already set in qsslpresharedkeyauthenticator.h

QScriptValue constructQSslPreSharedKeyAuthenticator(QScriptContext *context, QScriptEngine *engine);

class QSslPreSharedKeyAuthenticatorProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSslPreSharedKeyAuthenticatorProto(QObject *parent);
    virtual ~QSslPreSharedKeyAuthenticatorProto();

    Q_INVOKABLE QByteArray  identity() const;
    Q_INVOKABLE QByteArray  identityHint() const;
    Q_INVOKABLE int         maximumIdentityLength() const;
    Q_INVOKABLE int         maximumPreSharedKeyLength() const;
    Q_INVOKABLE QByteArray  preSharedKey() const;
    Q_INVOKABLE void        setIdentity(const QByteArray & identity);
    Q_INVOKABLE void        setPreSharedKey(const QByteArray & preSharedKey);
    Q_INVOKABLE void        swap(QSslPreSharedKeyAuthenticator & authenticator);

};

#endif
#endif
