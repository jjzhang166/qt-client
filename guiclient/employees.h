/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EMPLOYEES_H
#define EMPLOYEES_H

#include "display.h"
#include <parameter.h>

class employees : public display
{
    Q_OBJECT

public:
    employees(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);

public slots:
    virtual void sDelete();
    virtual void sEdit();
    virtual void sNew();
    virtual void sView();
    virtual void sOpen();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
};

#endif // EMPLOYEES_H
