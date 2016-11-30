/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSERIALPORTINFOPROTO_H__
#define __QSERIALPORTINFOPROTO_H__

#include <QtScript>
#include <QString>
#include <QStringList>

void setupQSerialPortInfoProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050100
#include <QSerialPortInfo>

Q_DECLARE_METATYPE(QSerialPortInfo)
Q_DECLARE_METATYPE(QSerialPortInfo*)

QScriptValue constructQSerialPortInfo(QScriptContext *context, QScriptEngine *engine);

class QSerialPortInfoProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSerialPortInfoProto(QObject *parent);

    Q_INVOKABLE QString description() const;
    Q_INVOKABLE bool    hasProductIdentifier() const;
    Q_INVOKABLE bool    hasVendorIdentifier() const;
    Q_INVOKABLE bool    isBusy() const;
    Q_INVOKABLE bool    isNull() const;
    Q_INVOKABLE QString manufacturer() const;
    Q_INVOKABLE QString portName() const;
    Q_INVOKABLE quint16 productIdentifier() const;
    Q_INVOKABLE QString serialNumber() const;
    Q_INVOKABLE void    swap(QSerialPortInfo & other);
    Q_INVOKABLE QString systemLocation() const;
    Q_INVOKABLE quint16 vendorIdentifier() const;
    Q_INVOKABLE QString toString() const;
};
#endif

#endif
