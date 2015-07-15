/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSALESORDERS_H
#define DSPSALESORDERS_H

#include "display.h"

#include "ui_dspSalesOrders.h"

class dspSalesOrders : public display, public Ui::dspSalesOrders
{
    Q_OBJECT

public:
    dspSalesOrders(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);

    virtual bool checkSitePrivs(int orderid);
    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPopulateMenu(QMenu * menuThis, QTreeWidgetItem*, int);
    virtual void sNew();
    virtual void sEditOrder();
    virtual void sViewOrder();
    virtual void sCopyOrder();
    virtual void sCreateRA();
    virtual void sDspShipmentStatus();
    virtual void sDspShipments();

protected slots:
    virtual void languageChange();

};

#endif // DSPSALESORDERS_H
