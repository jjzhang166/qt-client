/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSSLELLIPTICCURVEPROTO_H__
#define __QSSLELLIPTICCURVEPROTO_H__

#include <QScriptEngine>

void setupQSslEllipticCurveProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QSslEllipticCurve>

Q_DECLARE_METATYPE(QSslEllipticCurve*)
//Q_DECLARE_METATYPE(QSslEllipticCurve) // Already set in qsslellipticcurve.h

QScriptValue constructQSslEllipticCurve(QScriptContext *context, QScriptEngine *engine);

class QSslEllipticCurveProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSslEllipticCurveProto(QObject *parent);

    Q_INVOKABLE bool    isTlsNamedCurve() const;
    Q_INVOKABLE bool    isValid() const;
    Q_INVOKABLE QString longName() const;
    Q_INVOKABLE QString shortName() const;

};

#endif
#endif
