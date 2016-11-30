/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBSOCKETCORSAUTHENTICATORPROTO_H__
#define __QWEBSOCKETCORSAUTHENTICATORPROTO_H__

#include <QtScript>

void setupQWebSocketCorsAuthenticatorProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000

#include <QWebSocketCorsAuthenticator>

Q_DECLARE_METATYPE(QWebSocketCorsAuthenticator*)
//Q_DECLARE_METATYPE(QWebSocketCorsAuthenticator) // Is Q_DECLARE_PRIVATE() in qwebsocketcorsauthenticator.h

QScriptValue constructQWebSocketCorsAuthenticator(QScriptContext *context, QScriptEngine *engine);

class QWebSocketCorsAuthenticatorProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QWebSocketCorsAuthenticatorProto(QObject *parent);
    Q_INVOKABLE virtual ~QWebSocketCorsAuthenticatorProto();

    Q_INVOKABLE bool                        allowed() const;
    Q_INVOKABLE QString                     origin() const;
    Q_INVOKABLE void                        setAllowed(bool allowed);
    Q_INVOKABLE void                        swap(QWebSocketCorsAuthenticator &other);
//  Q_INVOKABLE QWebSocketCorsAuthenticator &operator=(QWebSocketCorsAuthenticator &&other);
//  Q_INVOKABLE QWebSocketCorsAuthenticator &operator=(const QWebSocketCorsAuthenticator &other);

    Q_INVOKABLE QString                     toString() const;
};

#endif

#endif
