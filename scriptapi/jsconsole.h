/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef JSCONSOLE_H
#define JSCONSOLE_H

#include <QObject>
#include <QtScript>

void setupJSConsole(QScriptEngine *engine);

class JSConsole : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    JSConsole(QObject *parent = 0);
    Q_INVOKABLE virtual ~JSConsole();
};

#endif // JSCONSOLE_H
