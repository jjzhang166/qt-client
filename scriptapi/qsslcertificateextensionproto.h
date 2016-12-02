/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef QSSLCERTIFICATEEXTENSIONPROTO_H
#define QSSLCERTIFICATEEXTENSIONPROTO_H

#include <QScriptEngine>

void setupQSslCertificateExtensionProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QSslCertificateExtension>

Q_DECLARE_METATYPE(QSslCertificateExtension*)
Q_DECLARE_METATYPE(QSslCertificateExtension)

QScriptValue constructQSslCertificateExtension(QScriptContext *context, QScriptEngine *engine);

class QSslCertificateExtensionProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSslCertificateExtensionProto(QObject *parent);
    virtual ~QSslCertificateExtensionProto();

    Q_INVOKABLE bool      isCritical() const;
    Q_INVOKABLE bool      isSupported() const;
    Q_INVOKABLE QString   name() const;
    Q_INVOKABLE QString   oid() const;
    Q_INVOKABLE void      swap(QSslCertificateExtension & other);
    Q_INVOKABLE QVariant  value() const;

};
#endif
#endif
