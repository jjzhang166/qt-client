/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPRETURNAUTHORIZATIONS_H
#define DSPRETURNAUTHORIZATIONS_H

#include "guiclient.h"
#include "display.h"

class dspReturnAuthorizations : public display
{
    Q_OBJECT

public:
    dspReturnAuthorizations(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);

    virtual bool checkSitePrivs(int orderid);
    virtual bool setParams(ParameterList&);

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sPopulateMenu(QMenu * menuThis, QTreeWidgetItem*, int);
    virtual void sEditRA();
    virtual void sViewRA();

};

#endif // DSPRETURNAUTHORIZATIONS_H
