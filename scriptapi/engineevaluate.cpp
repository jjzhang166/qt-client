/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "engineevaluate.h"
#include <QtScript>

QScriptValue engineEvaluateForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).isString()) {

    context->setActivationObject(context->parentContext()->activationObject());
    context->setThisObject(context->parentContext()->thisObject());

    QString script = context->argument(0).toString();
    QString scriptname = context->argument(1).toString();
    QScriptValue result = engine->evaluate(script, scriptname, 1);

    if (engine->hasUncaughtException()) {
      qWarning() << "Uncaught exception in script: " << scriptname
                 << " at line: "
                 << engine->uncaughtExceptionLineNumber() << ":"
                 << result.toString();
      return engine->toScriptValue(false);
    } else {
      return result;
    }
  } else {
    context->throwError(QScriptContext::UnknownError,
                          "The engineEvaluate() method takes two arguments.");
    return engine->toScriptValue(false);
  }
}

void setupEngineEvaluate(QScriptEngine *engine)
{
  QScriptValue engineEvaluate = engine->newFunction(engineEvaluateForJS);
  engine->globalObject().setProperty("engineEvaluate", engineEvaluate);
}
