/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPOsByVendor.h"

#include "guiclient.h"
#include "purchaseOrder.h"

dspPOsByVendor::dspPOsByVendor(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspPOsByVendor", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Purchase Orders by Vendor"));
  setListLabel(tr("Purchase Orders"));
  setReportName("POsByVendor");
  setMetaSQLOptions("purchaseOrders", "detail");
  setNewVisible(true);

  list()->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight,  true,  "pohead_number"  );
  list()->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true,  "warehousecode" );
  list()->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter, true,  "poitem_status" );
  list()->addColumn(tr("Vendor"),      -1,           Qt::AlignLeft,   true,  "vend_number"   );
  list()->addColumn(tr("Date"),        _dateColumn,  Qt::AlignCenter, true,  "sortDate" );

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   true);

  _descrip->setEnabled(_searchDescrip->isChecked());
}

void dspPOsByVendor::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPOsByVendor::setParams(ParameterList &params)
{
  _dates->appendValue(params);
  _warehouse->appendValue(params);
  _vend->appendValue(params);

  if(_showClosed->isChecked())
    params.append("showClosed");

  if(_byReceiptDate->isChecked())
    params.append("byReceiptDate");
  else if(_byDueDate->isChecked())
    params.append("byDueDate");
  else //if(_byOrderDate->isChecked())
    params.append("byOrderDate");

  if(_searchDescrip->isChecked())
    params.append("descrip_pattern", _descrip->text());

  params.append("closed",	tr("Closed"));
  params.append("unposted",	tr("Unposted"));
  params.append("partial",	tr("Partial"));
  params.append("received",	tr("Received"));
  params.append("open",		tr("Open"));

  return true;
}

void dspPOsByVendor::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;

  if (dynamic_cast<XTreeWidgetItem*>(pSelected) &&
      dynamic_cast<XTreeWidgetItem*>(pSelected)->rawValue("poitem_status").toString() == "U")
  {
    menuItem = pMenu->addAction(tr("Edit Order..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  }

  menuItem = pMenu->addAction(tr("View Order..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                       _privileges->check("ViewPurchaseOrders"));
}

void dspPOsByVendor::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  _vend->appendValue(params);
  
  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByVendor::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByVendor::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

