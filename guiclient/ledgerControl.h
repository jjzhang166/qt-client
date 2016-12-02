/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef LEDGERCONTROL_H
#define LEDGERCONTROL_H

#include "guiclient.h"
#include "display.h"
#include <parameter.h>

class ledgerControl : public display
{
    Q_OBJECT

  public:
    ledgerControl(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);

    Q_INVOKABLE virtual bool setParams(ParameterList &params);
};

#endif

