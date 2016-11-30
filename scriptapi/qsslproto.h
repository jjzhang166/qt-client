/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSSLPROTO_H__
#define __QSSLPROTO_H__

#include <QScriptEngine>

void setupQSslProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QSsl>

Q_DECLARE_METATYPE(enum QSsl::AlternativeNameEntryType)
Q_DECLARE_METATYPE(enum QSsl::EncodingFormat)
Q_DECLARE_METATYPE(enum QSsl::KeyAlgorithm)
Q_DECLARE_METATYPE(enum QSsl::KeyType)
Q_DECLARE_METATYPE(enum QSsl::SslOption)
Q_DECLARE_METATYPE(enum QSsl::SslProtocol)

#endif
#endif
