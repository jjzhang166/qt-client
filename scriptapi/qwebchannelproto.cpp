/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebchannelproto.h"

#if QT_VERSION < 0x050000
void setupQWebChannelProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
void setupQWebChannelProto(QScriptEngine *engine)
{
  QScriptValue constructor = engine->newFunction(constructQWebChannel);
  QScriptValue metaObject = engine->newQMetaObject(&QWebChannel::staticMetaObject, constructor);
  engine->globalObject().setProperty("QWebChannel", metaObject);
}

QScriptValue constructQWebChannel(QScriptContext *context, QScriptEngine *engine)
{
  QObject *parent = context->argument(0).toQObject();
  QWebChannel *object = new QWebChannel(parent);
  return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}

#endif
