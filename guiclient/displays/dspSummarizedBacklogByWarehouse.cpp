/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedBacklogByWarehouse.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "guiclient.h"
#include "dspInventoryAvailabilityBySalesOrder.h"
#include "salesOrder.h"
#include "printPackingList.h"
#include "storedProcErrorLookup.h"
#include "mqlutil.h"
#include "errorReporter.h"

dspSummarizedBacklogByWarehouse::dspSummarizedBacklogByWarehouse(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspSummarizedBacklogByWarehouse", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Summarized Backlog by Site"));
  setListLabel(tr("Backlog"));
  setReportName("SummarizedBacklogByWarehouse");
  setMetaSQLOptions("summarizedBacklogByWarehouse", "detail");
  setUseAltId(true);

  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));

  _customerType->setType(ParameterGroup::CustomerType);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Order#/Shipment#"),         _itemColumn, Qt::AlignLeft, true, "cohead_number");
  list()->addColumn(tr("Customer/Ship Via"),            -1, Qt::AlignLeft,  true, "cust_name");
  list()->addColumn(tr("Hold Type/Shipped"),   _orderColumn*2, Qt::AlignRight, true, "f_holdtype");
  list()->addColumn(tr("Ordered/Shipped"),     _dateColumn, Qt::AlignRight, true, "cohead_orderdate");
  list()->addColumn(tr("Scheduled"),           _dateColumn, Qt::AlignRight, true, "scheddate");
  list()->addColumn(tr("Pack Date"),           _dateColumn, Qt::AlignRight, true, "cohead_packdate");
  if (_privileges->check("ViewCustomerPrices"))
    list()->addColumn(tr("Sales"),  _moneyColumn, Qt::AlignRight, true, "sales");
  if (_privileges->check("ViewCosts"))
    list()->addColumn(tr("Cost"),   _moneyColumn, Qt::AlignRight, true, "cost");
  if (_privileges->check("ViewCustomerPrices") && _privileges->check("ViewCosts"))
    list()->addColumn(tr("Margin"), _moneyColumn, Qt::AlignRight, true, "margin");
  list()->addColumn(tr("Sales Rep"),           _itemColumn, Qt::AlignRight, true, "salesrep_name");
  list()->addColumn(tr("Time Received"),       _dateColumn, Qt::AlignRight, false, "cohead_created");
  list()->addColumn(tr("Pack List Batch"),     _dateColumn, Qt::AlignRight, false, "packed");

  list()->setRootIsDecorated(true);
  list()->setDragString("soheadid=");

  if ( (!_privileges->check("ViewCustomerPrices")) && (!_privileges->check("ViewCosts")) )
    _showPrices->setEnabled(false);
  sHandlePrices(_showPrices->isChecked());

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  sFillList();
}

void dspSummarizedBacklogByWarehouse::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspSummarizedBacklogByWarehouse::sHandlePrices(bool pShowPrices)
{
  if(pShowPrices){
    list()->showColumn("sales");
    list()->showColumn("cost");
    list()->showColumn("margin");
  }
  else{
    list()->hideColumn("sales");
    list()->hideColumn("cost");
    list()->hideColumn("margin");
  }
}

bool dspSummarizedBacklogByWarehouse::setParams(ParameterList &params)
{
  _warehouse->appendValue(params);
  _customerType->appendValue(params);

  if (_dates->allValid())
    _dates->appendValue(params);
  else
    return false;

  if(_showPrices->isChecked())
    params.append("showPrices");

  sHandlePrices(_showPrices->isChecked());

  params.append("none",   tr("None"));
  params.append("credit", tr("Credit"));
  params.append("pack",   tr("Pack"));
  params.append("return", tr("Return"));
  params.append("ship",   tr("Ship"));
  params.append("other",  tr("Other"));
  params.append("yes", tr("Yes"));
  params.append("no", tr("No"));

  return true;
}

void dspSummarizedBacklogByWarehouse::sInventoryAvailabilityBySalesOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", list()->id());
      
  dspInventoryAvailabilityBySalesOrder *newdlg = new dspInventoryAvailabilityBySalesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSummarizedBacklogByWarehouse::sEdit()
{
  salesOrder::editSalesOrder(list()->id(), false);
}

void dspSummarizedBacklogByWarehouse::sView()
{
  salesOrder::viewSalesOrder(list()->id());
}

void dspSummarizedBacklogByWarehouse::sDelete()
{
  XSqlQuery dspDelete;
  if ( QMessageBox::question(this, tr("Delete Sales Order?"),
                             tr("<p>Are you sure that you want to completely "
			     "delete the selected Sales Order?"),
			     QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    dspDelete.prepare("SELECT deleteSo(:sohead_id) AS result;");
    dspDelete.bindValue(":sohead_id", list()->id());
    dspDelete.exec();
    if (dspDelete.first())
    {
      int result = dspDelete.value("result").toInt();
      switch (result)
      {
        case 0:
          omfgThis->sSalesOrdersUpdated(-1);
          break;

        case -1:
          if (QMessageBox::question(this, tr("Cannot Delete Sales Order"),
				    storedProcErrorLookup("deleteSO", result) +
				       tr("<br>Would you like to Close the "
					  "selected Sales Order instead?" ),
				    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
          {
            dspDelete.prepare( "UPDATE coitem "
                       "SET coitem_status='C' "
                       "WHERE ((coitem_status<>'X')"
		       "   AND (coitem_cohead_id=:sohead_id));" );
            dspDelete.bindValue(":sohead_id", list()->id());
            dspDelete.exec();
        if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Sales Order Information"),
                                      dspDelete, __FILE__, __LINE__))
        {
          return;
        }
      
            sFillList();
          }

          break;

        default:
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Sales Order Information"),
                               storedProcErrorLookup("deleteSO", result),
                               __FILE__, __LINE__);
	  return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Sales Order Information"),
                                  dspDelete, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void dspSummarizedBacklogByWarehouse::sPrintPackingList()
{
  ParameterList params;
  if (list()->altId() > 0)
    params.append("shiphead_id", list()->altId());
  else
    params.append("sohead_id", list()->id());

  printPackingList newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspSummarizedBacklogByWarehouse::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if (list()->id() != -1)
  {
    menuItem = pMenu->addAction(tr("Inventory Availability by Sales Order..."), this, SLOT(sInventoryAvailabilityBySalesOrder()));
    if (!_privileges->check("ViewInventoryAvailability"))
      menuItem->setEnabled(false);

    pMenu->addSeparator();
  
    menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
    if (!_privileges->check("MaintainSalesOrders"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
    if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
    if (!_privileges->check("MaintainSalesOrders"))
      menuItem->setEnabled(false);

  }

  if (list()->altId() > -1 ||
      (list()->id() != -1 && list()->currentItem()->text(2) != tr("Pack") &&
       list()->currentItem()->text(2) != tr("Credit")))
  {
    if (list()->id() != -1)
      pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Print Packing List..."), this, SLOT(sPrintPackingList()));
    if (!_privileges->check("PrintPackingLists"))
      menuItem->setEnabled(false);
  }
}

void dspSummarizedBacklogByWarehouse::sFillList()
{
  XSqlQuery dspFillList;
  ParameterList params;
  if (setParams(params))
  {
    display::sFillList();

//    if (list()->topLevelItemCount())
//    {
      MetaSQLQuery totm = mqlLoad("summarizedBacklogByWarehouse", "totals");
      dspFillList = totm.toQuery(params);
      if (dspFillList.first())
        _totalSalesOrders->setText(dspFillList.value("totalorders").toString());
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                    dspFillList, __FILE__, __LINE__))
      {
        return;
      }

      MetaSQLQuery cntm = mqlLoad("summarizedBacklogByWarehouse", "counts");
      dspFillList = cntm.toQuery(params);
      if (dspFillList.first())
        _totalLineItems->setText(dspFillList.value("totalitems").toString());
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                    dspFillList, __FILE__, __LINE__))
      {
        return;
      }

      MetaSQLQuery qtym = mqlLoad("summarizedBacklogByWarehouse", "qtys");
      dspFillList = qtym.toQuery(params);
      if (dspFillList.first())
        _totalQty->setText(dspFillList.value("f_totalqty").toString());
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Order Information"),
                                    dspFillList, __FILE__, __LINE__))
      {
        return;
      }
//    }
//    else
//    {
//      _totalSalesOrders->clear();
//      _totalLineItems->clear();
//      _totalQty->clear();
//    }
  }
}

