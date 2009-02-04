/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "version.h"

QString _Name = "xTuple ERP: %1 Edition";

#ifndef __USEALTVERSION__
QString _Version   = "3.2.1";
QString _dbVersion = "3.2.1";
#else
#include "../altVersion.cpp"
#endif
QString _Copyright = "Copyright (c) 1999-2009, xTuple, LLC.";

