/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "version.h"

QString _Name        = "xTuple ERP: %1 Edition";
QString _Version     = "4.9.2";
QString _dbVersion   = "4.9.2";
QString _Copyright   = "Copyright (c) 1999-2015, OpenMFG, LLC.";
QString _ConnAppName = "xTuple ERP (qt-client)";

#ifdef __USEALTVERSION__
#include "altVersion.cpp"
#else
QString _Build     = "%1 %2";
#endif

/*: Please translate this Version string to the base version of the application
    you are translating. This is a hack to embed the application version number
    into the translation file so the Update Manager can find
    the best translation file for a given version of the application.
 */
static QString _translationFileVersionPlaceholder = QT_TRANSLATE_NOOP("xTuple", "Version");
