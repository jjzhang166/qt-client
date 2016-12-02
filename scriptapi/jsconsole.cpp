/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

// @See: http://www.shibu.jp/techmemo/qtscript.html#adding-functions-and-objects-to-qtscript-engine
// @See: http://stackoverflow.com/a/32407007/251019

#include "jsconsole.h"

#include <QDebug>

QScriptValue consoleErrorForJS(QScriptContext* context, QScriptEngine* engine)
{
    QStringList list;
    for(int i=0; i<context->argumentCount(); ++i)
    {
        QScriptValue param(context->argument(i));
        list.append(param.toString());
    }
    QStringList backtrace = context->backtrace();
    qCritical() << qPrintable("\n" + list.join(" ") + "\nStacktrace:\n" + backtrace.join("\n"));
    return engine->undefinedValue();
}

QScriptValue consoleInfoForJS(QScriptContext* context, QScriptEngine* engine)
{
    QStringList list;
    for(int i=0; i<context->argumentCount(); ++i)
    {
        QScriptValue param(context->argument(i));
        list.append(param.toString());
    }
#if QT_VERSION >= 0x050000
    qInfo() << qPrintable("\n" + list.join(" "));
#else
    qDebug() << qPrintable("\n" + list.join(" "));
#endif
    return engine->undefinedValue();
}

QScriptValue consoleLogForJS(QScriptContext* context, QScriptEngine* engine)
{
    QStringList list;
    for(int i=0; i<context->argumentCount(); ++i)
    {
        QScriptValue param(context->argument(i));
        list.append(param.toString());
    }
    // TODO: Use qInfo(), but that does not currently show in the Database Log window.
    qDebug() << qPrintable("\n" + list.join(" "));
    return engine->undefinedValue();
}

QScriptValue consoleWarnForJS(QScriptContext* context, QScriptEngine* engine)
{
    QStringList list;
    for(int i=0; i<context->argumentCount(); ++i)
    {
        QScriptValue param(context->argument(i));
        list.append(param.toString());
    }
    qWarning() << qPrintable("\n" + list.join(" "));
    return engine->undefinedValue();
}

void setupJSConsole(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new JSConsole(engine));
  engine->globalObject().setProperty("console",  proto);

  QScriptValue consoleError = engine->newFunction(consoleErrorForJS);
  proto.setProperty("error",  consoleError);
  QScriptValue consoleInfo = engine->newFunction(consoleInfoForJS);
  proto.setProperty("info",  consoleInfo);
  QScriptValue consoleLog = engine->newFunction(consoleLogForJS);
  proto.setProperty("log",  consoleLog);
  QScriptValue consoleWarn = engine->newFunction(consoleWarnForJS);
  proto.setProperty("warn",  consoleWarn);

  // Overload Qt Script print() function as it does not appear to be working
  // consistently in Qt 5.5.1.
  engine->globalObject().setProperty("print",  consoleLog);
}

JSConsole::JSConsole(QObject *parent) : QObject(parent)
{
}
JSConsole::~JSConsole()
{
}
