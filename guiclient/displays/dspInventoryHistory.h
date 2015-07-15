/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __DSPINVENTORYHISTORY_H__
#define __DSPINVENTORYHISTORY_H__

#include "guiclient.h"
#include "display.h"
#include <parameter.h>

class dspInventoryHistory : public display
{
    Q_OBJECT

public:
    dspInventoryHistory(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);

    virtual bool setParams(ParameterList & params);
    Q_INVOKABLE virtual void setItemId(int itemId);
    Q_INVOKABLE virtual void setStartDate(QDate startDate);

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sEditTransInfo();
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * pItem, int);
    virtual void sViewTransInfo();
    virtual void sViewWOInfo();
};

#endif // __DSPINVENTORYHISTORY_H__
