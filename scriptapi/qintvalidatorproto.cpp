/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qintvalidatorproto.h"

#define DEBUG false

QScriptValue QIntValidatorToScriptValue(QScriptEngine *engine, QIntValidator* const &in)
 { return engine->newQObject(in); }

void QIntValidatorFromScriptValue(const QScriptValue &object, QIntValidator* &out)
 { out = qobject_cast<QIntValidator*>(object.toQObject()); }

void setupQIntValidatorProto(QScriptEngine *engine)
{
  //QScriptValue proto = engine->newQObject(new QIntValidatorProto(engine));
  //engine->setDefaultPrototype(qMetaTypeId<QIntValidator*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QIntValidator>(),  proto);

  qScriptRegisterMetaType(engine, QIntValidatorToScriptValue, QIntValidatorFromScriptValue);

  //QScriptValue constructor = engine->newFunction(constructQIntValidator,
  //                                               proto);
  //engine->globalObject().setProperty("QIntValidator", constructor);
}

QScriptValue constructQIntValidator(QScriptContext *context,
                            QScriptEngine  *engine)
{
  QIntValidator *obj = 0;

  if (DEBUG)
  {
    qDebug("constructQIntValidator() entered");
    for (int i = 0; i < context->argumentCount(); i++)
      qDebug("context->argument(%d) = %s", i,
             qPrintable(context->argument(i).toString()));
  }

  if (context->argumentCount() >= 3)
    obj = new QIntValidator(context->argument(0).toNumber(),
                    context->argument(1).toNumber(),
                    context->argument(2).toQObject());
  else if (context->argumentCount() == 1)
    obj = new QIntValidator(context->argument(0).toQObject());
  else
    context->throwError(QScriptContext::UnknownError,
                        "QIntValidator() constructor is not yet supported");

  return engine->toScriptValue(obj);
}

QIntValidatorProto::QIntValidatorProto(QObject *parent)
    : QObject(parent)
{
}

