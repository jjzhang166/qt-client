/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __WEBCHANNELTRANSPORT_H__
#define __WEBCHANNELTRANSPORT_H__

#include <QtScript>

void setupWebChannelTransport(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QJsonDocument>
#include <QJsonObject>
#include <QtWebChannel/QWebChannelAbstractTransport>
#include <QWebSocket>

QScriptValue constructWebChannelTransport(QScriptContext *context, QScriptEngine *engine);

class WebChannelTransport : public QWebChannelAbstractTransport
{
    Q_OBJECT

public:
    explicit WebChannelTransport(QWebSocket *socket);
    virtual ~WebChannelTransport();

    void sendMessage(const QJsonObject &message) Q_DECL_OVERRIDE;

private slots:
    void textMessageReceived(const QString &message);

private:
    QWebSocket *m_socket;
};

Q_DECLARE_METATYPE(WebChannelTransport*)

#endif

#endif
