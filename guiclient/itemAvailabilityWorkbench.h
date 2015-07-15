/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMAVAILABILITYWORKBENCH_H
#define ITEMAVAILABILITYWORKBENCH_H

#include "guiclient.h"
#include "xwidget.h"

#include "dspCostedIndentedBOM.h"
#include "dspInventoryAvailability.h"
#include "dspInventoryHistory.h"
#include "dspInventoryLocator.h"
#include "dspPoItemsByItem.h"
#include "dspPoItemReceivingsByItem.h"
#include "dspPricesByCustomer.h"
#include "dspQuotesByItem.h"
#include "dspRunningAvailability.h"
#include "dspSalesHistory.h"
#include "dspSalesOrdersByItem.h"
#include "dspSingleLevelWhereUsed.h"
#include "item.h"

#include <parameter.h>

#include "ui_itemAvailabilityWorkbench.h"

class itemAvailabilityWorkbench : public XWidget, public Ui::itemAvailabilityWorkbench
{
    Q_OBJECT

public:
    itemAvailabilityWorkbench(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~itemAvailabilityWorkbench();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sFillList();
    virtual void sHandleButtons();

protected slots:
    virtual void languageChange();

protected:
  dspCostedIndentedBOM *_dspCostedIndentedBOM;
  dspInventoryAvailability *_dspInventoryAvailability;
  dspInventoryHistory *_dspInventoryHistory;
  dspInventoryLocator *_dspInventoryLocator;
  dspPoItemsByItem *_dspPoItemsByItem;
  dspPoItemReceivingsByItem *_dspPoItemReceivingsByItem;
  dspPricesByCustomer *_dspPricesByCustomer;
  dspQuotesByItem *_dspQuotesByItem;
  dspRunningAvailability *_dspRunningAvailability;
  dspSalesHistory *_dspSalesHistory;
  dspSalesOrdersByItem *_dspSalesOrdersByItem;
  dspSingleLevelWhereUsed *_dspSingleLevelWhereUsed;
  item *_itemMaster;

};

#endif // ITEMAVAILABILITYWORKBENCH_H
