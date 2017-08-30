/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "guiErrorCheck.h"
#include "scriptapi_internal.h"

#include <QList>
#include <QMessageBox>
#include <QWidget>

GuiErrorCheck::GuiErrorCheck(bool phasError, QWidget *pwidget, QString pmsg) :
  hasError(phasError),
  widget(pwidget),
  msg(pmsg)
{
  if (! widget)
  {
    msg = "Bug: GuiErrorCheck created with empty widget"; // no tr
    hasError = true;
  }
}

GuiErrorCheck::GuiErrorCheck() :
  hasError(false),
  widget(0),
  msg(QString())
{
}

GuiErrorCheck::~GuiErrorCheck()
{
  // no need to delete child widgets, Qt does it all for us
}

bool GuiErrorCheck::reportErrors(QWidget *parent, QString title,
                                 QList<GuiErrorCheck> list)
{
  for (int i = 0; i < list.length(); i++)
  {
    if (list[i].hasError)
    {
      QMessageBox::critical(parent, title, QString("<p>%1").arg(list[i].msg));
      if (list[i].widget)
        list[i].widget->setFocus();
      return true;
    }
  }

  return false;
}

// Script exposure

QScriptValue QListGuiErrorChecktoScriptValue(QScriptEngine *engine, const QList<GuiErrorCheck> &list)
{
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < list.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(list.at(i)));
  }
  return newArray;
}
void QListGuiErrorCheckfromScriptValue(const QScriptValue &obj, QList<GuiErrorCheck> &list)
{
  list = QList<GuiErrorCheck>();
  QScriptValueIterator it(obj);

  while (it.hasNext()) {
    it.next();
    if (it.flags() & QScriptValue::SkipInEnumeration)
      continue;
    GuiErrorCheck item = qscriptvalue_cast<GuiErrorCheck>(it.value());
    list.insert(it.name().toInt(), item);
  }
}

// Static functions
static QScriptValue guierrorcheck_reporterrors(QScriptContext *context, QScriptEngine * /*engine*/ )
{
  bool result = false;
  QWidget *widget           = qscriptvalue_cast<QWidget*>(context->argument(0));
  QList<GuiErrorCheck> list = qscriptvalue_cast< QList<GuiErrorCheck> >(context->argument(2));

  if (context->argumentCount() == 3 &&
      widget &&
      context->argument(1).isString() &&
      !list.isEmpty())
    result = GuiErrorCheck::reportErrors(widget,
                                         context->argument(1).toString(),
                                         list);
  else
    context->throwError(QScriptContext::UnknownError, "Could not find an appropriate GuiErrorCheck::reportErrors()");

  return QScriptValue(result);
}

void setupGuiErrorCheck(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QListGuiErrorChecktoScriptValue, QListGuiErrorCheckfromScriptValue);

  QScriptValue constructor = engine->newFunction(constructGuiErrorCheck);
  engine->globalObject().setProperty("GuiErrorCheck", constructor, CTORPROPFLAGS);

  constructor.setProperty("reportErrors", engine->newFunction(guierrorcheck_reporterrors), STATICPROPFLAGS);
}

QScriptValue constructGuiErrorCheck(QScriptContext *context, QScriptEngine *engine)
{
  GuiErrorCheck obj;

  if (context->argumentCount() == 3    &&
      context->argument(0).isBoolean() &&
      qscriptvalue_cast<QWidget*>(context->argument(1)) &&
      context->argument(2).isString()) {
    obj = GuiErrorCheck(context->argument(0).toBoolean(),
                        qscriptvalue_cast<QWidget*>(context->argument(1)),
                        context->argument(2).toString());
  }
  else {
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate GuiErrorCheck constructor");
  }

  return engine->toScriptValue(obj);
}
