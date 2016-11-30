/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QTCPSOCKETPROTO_H__
#define __QTCPSOCKETPROTO_H__
#include <QObject>
#include <QString>
#include <QtScript>

void setupQTcpSocketProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QIODevice>
#include <QAbstractSocket>
#include <QTcpSocket>
#include "qabstractsocketproto.h"

QScriptValue constructQTcpSocket(QScriptContext *context, QScriptEngine *engine);

Q_DECLARE_METATYPE(QTcpSocket*)

class QTcpSocketProto : public QAbstractSocketProto
{
  Q_OBJECT

  public:
    QTcpSocketProto(QObject *parent);
};
#endif

#endif
