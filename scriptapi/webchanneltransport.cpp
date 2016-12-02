/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "webchanneltransport.h"

#if QT_VERSION < 0x050000
void setupWebChannelTransport(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
void setupWebChannelTransport(QScriptEngine *engine)
{
  QScriptValue constructor = engine->newFunction(constructWebChannelTransport);
  QScriptValue metaObject = engine->newQMetaObject(&WebChannelTransport::staticMetaObject, constructor);
  engine->globalObject().setProperty("WebChannelTransport", metaObject);
}

QScriptValue constructWebChannelTransport(QScriptContext *context, QScriptEngine *engine)
{
  QWebSocket *socket = qscriptvalue_cast<QWebSocket*>(context->argument(0));
  WebChannelTransport *object = new WebChannelTransport(socket);
  return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}

/*!
    Construct the transport object and wrap the given socket.

    The socket is also set as the parent of the transport object.
*/
WebChannelTransport::WebChannelTransport(QWebSocket *socket)
: QWebChannelAbstractTransport(socket)
, m_socket(socket)
{
    connect(socket, &QWebSocket::textMessageReceived,
            this, &WebChannelTransport::textMessageReceived);
}

/*!
    Destroys the WebChannelTransport.
*/
WebChannelTransport::~WebChannelTransport()
{

}

/*!
    Serialize the JSON message and send it as a text message via the WebSocket to the client.
*/
void WebChannelTransport::sendMessage(const QJsonObject &message)
{
    QJsonDocument doc(message);
    m_socket->sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

/*!
    Deserialize the stringified JSON messageData and emit messageReceived.
*/
void WebChannelTransport::textMessageReceived(const QString &messageData)
{
    QJsonParseError error;
    QJsonDocument message = QJsonDocument::fromJson(messageData.toUtf8(), &error);
    if (error.error) {
        qWarning() << "Failed to parse text message as JSON object:" << messageData
                   << "Error is:" << error.errorString();
        return;
    } else if (!message.isObject()) {
        qWarning() << "Received JSON message that is not an object: " << messageData;
        return;
    }
    emit messageReceived(message.object(), this);
}

#endif
