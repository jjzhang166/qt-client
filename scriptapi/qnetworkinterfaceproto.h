/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QNETWORKINTERFACEPROTO_H__
#define __QNETWORKINTERFACEPROTO_H__
#include <QtScript>

void setupQNetworkInterfaceProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QNetworkInterface>
#include <QHostAddress>
QScriptValue constructQNetworkInterface(QScriptContext *context, QScriptEngine *engine);

Q_DECLARE_METATYPE(QNetworkInterface*)
Q_DECLARE_METATYPE(QList<QNetworkAddressEntry>)
Q_DECLARE_METATYPE(enum QNetworkInterface::InterfaceFlag)
Q_DECLARE_METATYPE(QNetworkInterface::InterfaceFlags)

class QNetworkInterfaceProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QNetworkInterfaceProto(QObject *parent);
    Q_INVOKABLE QList<QNetworkAddressEntry>       addressEntries() const;
    Q_INVOKABLE QNetworkInterface::InterfaceFlags flags() const;
    Q_INVOKABLE QString                           hardwareAddress() const;
    Q_INVOKABLE QString                           humanReadableName() const;
    Q_INVOKABLE int                               index() const;
    Q_INVOKABLE bool                              isValid() const;
    Q_INVOKABLE QString                           name() const;
    Q_INVOKABLE void                              swap(QNetworkInterface & other);
    // added custom
    Q_INVOKABLE QString                           toString() const;
};
#endif

#endif
