/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebsocketcorsauthenticatorproto.h"

#if QT_VERSION < 0x050000
void setupQWebSocketCorsAuthenticatorProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
void setupQWebSocketCorsAuthenticatorProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QWebSocketCorsAuthenticatorProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebSocketCorsAuthenticator*>(), proto);
  // Not allowed. Is Q_DECLARE_PRIVATE() in qwebsocketcorsauthenticator.h
  //engine->setDefaultPrototype(qMetaTypeId<QWebSocketCorsAuthenticator>(), proto);

  engine->globalObject().setProperty("QWebSocketCorsAuthenticator",  proto);
}

QScriptValue constructQWebSocketCorsAuthenticator(QScriptContext *context, QScriptEngine *engine)
{
  QWebSocketCorsAuthenticator *obj = 0;
  if (context->argumentCount() >= 1)
  {
    QScriptValue arg = context->argument(0);
    if (arg.isString()) {
      obj = new QWebSocketCorsAuthenticator(arg.toString());
    } else {
      obj = new QWebSocketCorsAuthenticator(arg.property("origin").toString());
      obj->setAllowed(arg.property("allowed").toBool());
    }
  }
  else {
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QWebSocketCorsAuthenticator constructor");
  }

  return engine->toScriptValue(obj);
}

QWebSocketCorsAuthenticatorProto::QWebSocketCorsAuthenticatorProto(QObject *parent)
    : QObject(parent)
{
}

QWebSocketCorsAuthenticatorProto::~QWebSocketCorsAuthenticatorProto()
{
}

bool QWebSocketCorsAuthenticatorProto::allowed() const
{
  QWebSocketCorsAuthenticator *item = qscriptvalue_cast<QWebSocketCorsAuthenticator*>(thisObject());
  if (item)
    return item->allowed();
  return false;
}

QString QWebSocketCorsAuthenticatorProto::origin() const
{
  QWebSocketCorsAuthenticator *item = qscriptvalue_cast<QWebSocketCorsAuthenticator*>(thisObject());
  if (item)
    return item->origin();
  return QString();
}

void QWebSocketCorsAuthenticatorProto::setAllowed(bool allowed)
{
  QWebSocketCorsAuthenticator *item = qscriptvalue_cast<QWebSocketCorsAuthenticator*>(thisObject());
  if (item)
    item->setAllowed(allowed);
}

void QWebSocketCorsAuthenticatorProto::swap(QWebSocketCorsAuthenticator &other)
{
  QWebSocketCorsAuthenticator *item = qscriptvalue_cast<QWebSocketCorsAuthenticator*>(thisObject());
  if (item)
    item->swap(other);
}

/*
QWebSocketCorsAuthenticator QWebSocketCorsAuthenticatorProto::&operator=(QWebSocketCorsAuthenticator &&other)
{
  QWebSocketCorsAuthenticator *item = qscriptvalue_cast<QWebSocketCorsAuthenticator*>(thisObject());
  if (item)
    return item->operator=(other);
  return QWebSocktCorsAuthenticator();
}

QWebSocketCorsAuthenticator QWebSocketCorsAuthenticatorProto::&operator=(const QWebSocketCorsAuthenticator &other)
{
  QWebSocketCorsAuthenticator *item = qscriptvalue_cast<QWebSocketCorsAuthenticator*>(thisObject());
  if (item)
    return item->&operator=(other);
  return QWebSocktCorsAuthenticator();
}
*/

QString QWebSocketCorsAuthenticatorProto::toString() const
{
  QWebSocketCorsAuthenticator *item = qscriptvalue_cast<QWebSocketCorsAuthenticator*>(thisObject());
  if (item)
    return item->origin();
  return QString();
}

#endif
