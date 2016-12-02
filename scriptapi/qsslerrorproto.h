/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSSLERRORPROTO_H__
#define __QSSLERRORPROTO_H__

#include <QScriptEngine>

void setupQSslErrorProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QSslCertificate>
#include <QSslError>

Q_DECLARE_METATYPE(QSslError*)
Q_DECLARE_METATYPE(QSslError) //Do not need? Already declared by qsslcertificate.h
Q_DECLARE_METATYPE(enum QSslError::SslError)

QScriptValue constructQSslError(QScriptContext *context, QScriptEngine *engine);

class QSslErrorProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSslErrorProto(QObject *parent);
    virtual ~QSslErrorProto();

    Q_INVOKABLE QSslCertificate       certificate() const;
    Q_INVOKABLE QSslError::SslError   error() const;
    Q_INVOKABLE QString               errorString() const;
    Q_INVOKABLE void                  swap(QSslError & other);

    Q_INVOKABLE QString               toString() const;

};

#endif
#endif
