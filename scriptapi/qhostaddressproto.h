/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QHOSTADDRESSPROTO_H__
#define __QHOSTADDRESSPROTO_H__

#include <QScriptEngine>

void setupQHostAddressProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QAbstractSocket>
#include <QHostAddress>
#include <QScriptable>
#include <QString>

Q_DECLARE_METATYPE(QHostAddress*)
Q_DECLARE_METATYPE(QHostAddress)
Q_DECLARE_METATYPE(enum QHostAddress::SpecialAddress)

QScriptValue constructQHostAddress(QScriptContext *context, QScriptEngine *engine);

class QHostAddressProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QHostAddressProto(QObject *parent);
    ~QHostAddressProto();

    Q_INVOKABLE void                                    clear();
    Q_INVOKABLE bool                                    isInSubnet(const QHostAddress & subnet, int netmask) const;
    Q_INVOKABLE bool                                    isInSubnet(const QPair<QHostAddress, int> & subnet) const;
    Q_INVOKABLE bool                                    isLoopback() const;
    Q_INVOKABLE bool                                    isNull() const;
    Q_INVOKABLE QAbstractSocket::NetworkLayerProtocol   protocol() const;
    Q_INVOKABLE QString                                 scopeId() const;
    Q_INVOKABLE void                                    setAddress(quint32 ip4Addr);
    Q_INVOKABLE void                                    setAddress(quint8 * ip6Addr);
    Q_INVOKABLE void                                    setAddress(const quint8 * ip6Addr);
    Q_INVOKABLE void                                    setAddress(const Q_IPV6ADDR & ip6Addr);
    Q_INVOKABLE void                                    setAddress(const sockaddr * sockaddr);
    Q_INVOKABLE bool                                    setAddress(const QString & address);
    Q_INVOKABLE void                                    setScopeId(const QString & id);
    Q_INVOKABLE quint32                                 toIPv4Address() const;
    Q_INVOKABLE quint32                                 toIPv4Address(bool * ok) const;
    Q_INVOKABLE Q_IPV6ADDR                              toIPv6Address() const;
    Q_INVOKABLE QString                                 toString() const;

};

#endif
#endif
