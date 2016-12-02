/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSQLPROTO_H__
#define __QSQLPROTO_H__

#include <QScriptEngine>

#if QT_VERSION >= 0x050000
#include <QSql>
#else
#include <QtSql>
#endif

void setupQSqlProto(QScriptEngine *engine);

Q_DECLARE_METATYPE(enum QSql::Location)
Q_DECLARE_METATYPE(enum QSql::NumericalPrecisionPolicy)
Q_DECLARE_METATYPE(enum QSql::ParamTypeFlag)
Q_DECLARE_METATYPE(enum QSql::TableType)

#endif
