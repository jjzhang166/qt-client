/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qtcpsocketproto.h"

#if QT_VERSION < 0x050000
void setupQTcpSocketProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

QScriptValue QTcpSockettoScriptValue(QScriptEngine *engine, QTcpSocket* const &item)
{ return engine->newQObject(item); }

void QTcpSocketfromScriptValue(const QScriptValue &obj, QTcpSocket* &item)
{
  item = qobject_cast<QTcpSocket*>(obj.toQObject());
}

void setupQTcpSocketProto(QScriptEngine *engine)
{
 qScriptRegisterMetaType(engine, QTcpSockettoScriptValue, QTcpSocketfromScriptValue);

  QScriptValue proto = engine->newQObject(new QTcpSocketProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QTcpSocket*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQTcpSocket,
                                                 proto);
  engine->globalObject().setProperty("QTcpSocket",  constructor);
}

QScriptValue constructQTcpSocket(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QTcpSocket *obj = 0;
  obj = new QTcpSocket();
  return engine->toScriptValue(obj);
}

QTcpSocketProto::QTcpSocketProto(QObject *parent)
    : QAbstractSocketProto(parent)
{
}
#endif
