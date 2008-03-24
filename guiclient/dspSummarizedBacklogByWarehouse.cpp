/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "dspSummarizedBacklogByWarehouse.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "dspInventoryAvailabilityBySalesOrder.h"
#include "salesOrder.h"
#include "salesOrderItem.h"
#include "rescheduleSoLineItems.h"
#include "printPackingList.h"
#include "storedProcErrorLookup.h"

dspSummarizedBacklogByWarehouse::dspSummarizedBacklogByWarehouse(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_so, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _customerType->setType(CustomerType);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _so->addColumn(tr("S/O #/Shipped"),       _itemColumn, Qt::AlignRight  );
  _so->addColumn(tr("Customer/Ship Via"),            -1, Qt::AlignLeft   );
  _so->addColumn(tr("Hold Type/Ship #"), _orderColumn*2, Qt::AlignRight  );
  _so->addColumn(tr("Ordered/Shipped"),     _dateColumn, Qt::AlignRight  );
  _so->addColumn(tr("Scheduled"),           _dateColumn, Qt::AlignRight  );
  _so->addColumn(tr("Pack Date"),           _dateColumn, Qt::AlignRight  );
  if (_privleges->check("ViewCustomerPrices") ||
      _privleges->check("MaintainCustomerPrices"))
  {
    _so->addColumn(tr("Sales"),  _moneyColumn, Qt::AlignRight);
    _so->addColumn(tr("Cost"),   _moneyColumn, Qt::AlignRight);
    _so->addColumn(tr("Margin"), _moneyColumn, Qt::AlignRight);
  }

  _so->setRootIsDecorated(TRUE);
  _so->setDragString("soheadid=");

  if ( (!_privleges->check("ViewCustomerPrices")) && (!_privleges->check("MaintainCustomerPrices")) )
    _showPrices->setEnabled(FALSE);
  sHandlePrices(_showPrices->isChecked());

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  sFillList();
}

dspSummarizedBacklogByWarehouse::~dspSummarizedBacklogByWarehouse()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSummarizedBacklogByWarehouse::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedBacklogByWarehouse::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
  {
    _so->showColumn(6);
    _so->showColumn(7);
    _so->showColumn(8);
  }
  else
  {
    _so->hideColumn(6);
    _so->hideColumn(7);
    _so->hideColumn(8);
  }

  sFillList();
}

bool dspSummarizedBacklogByWarehouse::setParams(ParameterList &params)
{
  _warehouse->appendValue(params);
  _customerType->appendValue(params);

  if (_dates->allValid())
    _dates->appendValue(params);
  else
    return false;

  if (_showPrices->isChecked())
    params.append("showPrices");

  if (_shipDate->isChecked())
    params.append("orderByShipDate");
  else if (_packDate->isChecked())
    params.append("orderByPackDate");
  else if (_orderNumber->isChecked())
    params.append("orderByOrderNumber");

  params.append("none",   tr("None"));
  params.append("credit", tr("Credit"));
  params.append("pack",   tr("Pack"));
  params.append("return", tr("Return"));
  params.append("ship",   tr("Ship"));
  params.append("other",  tr("Other"));

  return true;
}

void dspSummarizedBacklogByWarehouse::sPrint()
{
  ParameterList params;
  if (setParams(params))
  {
    orReport report("SummarizedBacklogByWarehouse", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
}

void dspSummarizedBacklogByWarehouse::sInventoryAvailabilityBySalesOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", _so->id());
      
  dspInventoryAvailabilityBySalesOrder *newdlg = new dspInventoryAvailabilityBySalesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSummarizedBacklogByWarehouse::sEdit()
{
  salesOrder::editSalesOrder(_so->id(), false);
}

void dspSummarizedBacklogByWarehouse::sView()
{
  salesOrder::viewSalesOrder(_so->id());
}

void dspSummarizedBacklogByWarehouse::sReschedule()
{
  ParameterList params;
  params.append("sohead_id", _so->id());
      
  rescheduleSoLineItems newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void dspSummarizedBacklogByWarehouse::sDelete()
{
  if ( QMessageBox::question(this, tr("Delete Sales Order?"),
                             tr("<p>Are you sure that you want to completely "
			     "delete the selected Sales Order?"),
			     QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteSo(:sohead_id) AS result;");
    q.bindValue(":sohead_id", _so->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
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
            q.prepare( "UPDATE coitem "
                       "SET coitem_status='C' "
                       "WHERE ((coitem_status<>'X')"
		       "   AND (coitem_cohead_id=:sohead_id));" );
            q.bindValue(":sohead_id", _so->id());
            q.exec();
	    if (q.lastError().type() != QSqlError::None)
	    {
	      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	      return;
	    }
      
            sFillList();
          }

          break;

        default:
          systemError(this, storedProcErrorLookup("deleteSO", result),
		      __FILE__, __LINE__);
	  return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void dspSummarizedBacklogByWarehouse::sPrintPackingList()
{
  ParameterList params;
  if (_so->altId() > 0)
    params.append("cosmisc_id", _so->altId());
  else
    params.append("sohead_id", _so->id());

  printPackingList newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspSummarizedBacklogByWarehouse::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if (_so->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Inventory Availability by Sales Order..."), this, SLOT(sInventoryAvailabilityBySalesOrder()), 0);
    if (!_privleges->check("ViewInventoryAvailability"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();
  
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    if (!_privleges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
    if ((!_privleges->check("MaintainSalesOrders")) && (!_privleges->check("ViewSalesOrders")))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Reschedule..."), this, SLOT(sReschedule()), 0);
    if (!_privleges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
    if (!_privleges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

  }

  if (_so->altId() > -1 ||
      (_so->id() != -1 && _so->currentItem()->text(2) != tr("Pack") &&
       _so->currentItem()->text(2) != tr("Credit")))
  {
    if (_so->id() != -1)
      pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Print Packing List..."), this, SLOT(sPrintPackingList()), 0);
    if (!_privleges->check("PrintPackingLists"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspSummarizedBacklogByWarehouse::sFillList()
{
  _so->clear();

  XTreeWidgetItem *orderLine  = NULL;

  ParameterList params;
  if (setParams(params))
  {
    QString sql( "SELECT cohead_id, cohead_holdtype, cohead_number, cust_name,"
		 "       CASE WHEN (cohead_holdtype='N') THEN <? value(\"none\") ?>"
		 "            WHEN (cohead_holdtype='C') THEN <? value(\"credit\") ?>"
		 "            WHEN (cohead_holdtype='S') THEN <? value(\"ship\") ?>"
		 "            WHEN (cohead_holdtype='P') THEN <? value(\"pack\") ?>"
		 "            WHEN (cohead_holdtype='R') THEN <? value(\"return\") ?>"
		 "            ELSE <? value(\"other\") ?>"
		 "       END AS f_holdtype,"
		 "       formatDate(cohead_orderdate) AS f_orderdate,"
		 "       formatDate(MIN(coitem_scheddate)) AS f_scheddate,"
		 "       formatDate(cohead_packdate) AS f_packdate,"
		 "       formatMoney( SUM( round((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) *"
		 "                         (coitem_price / coitem_price_invuomratio), 2) ) ) AS f_sales,"
		 "       formatCost(SUM((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) * stdcost(item_id) ) ) AS f_cost,"
		 "       formatMoney( SUM( (noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) *"
		 "                         ((coitem_price / coitem_price_invuomratio) - stdcost(item_id)) ) ) AS f_margin,"
		 "       SUM( round((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) *"
		 "            (coitem_price / coitem_price_invuomratio), 2) ) AS sales,"
		 "       SUM((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) * stdcost(item_id) ) AS cost,"
		 "       SUM((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) *"
		 "            ((coitem_price / coitem_price_invuomratio) - stdcost(item_id)) ) AS margin,"
		 "       MIN(coitem_scheddate) AS scheddate,"
		 "       COALESCE(cosmisc_id, -1) AS cosmisc_id, "
		 "       formatShipmentNumber(cosmisc_id) AS cosmisc_number, "
		 "       CASE WHEN (cosmisc_shipped IS NULL) THEN 0"
		 "            WHEN (cosmisc_shipped) THEN 1"
		 "            WHEN (NOT cosmisc_shipped) THEN 2"
		 "       END AS shipstatus,"
		 "       COALESCE(cosmisc_shipvia, '') AS shipvia,"
		 "       CASE WHEN (cosmisc_shipdate IS NULL) THEN ''"
		 "            ELSE formatDate(cosmisc_shipdate)"
		 "       END AS shipdate,"
		 "       ( (COALESCE(cobmisc_cohead_id,0) > 0)"
		 "       AND (SUM(noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned)) > 0)"
		 "       ) AS overbilled "
		 "FROM coitem, itemsite, item, cust, cohead "
                 "  LEFT OUTER JOIN cosmisc ON (cosmisc_cohead_id=cohead_id) "
                 "  LEFT OUTER JOIN (SELECT DISTINCT cobmisc_cohead_id FROM cobmisc) AS cobmisc ON (cobmisc_cohead_id=cohead_id) "
		 "WHERE ( (coitem_cohead_id=cohead_id)"
		 " AND (cohead_cust_id=cust_id)"
		 " AND (coitem_itemsite_id=itemsite_id)"
		 " AND (itemsite_item_id=item_id)"
		 " AND (coitem_status NOT IN ('C','X'))"
		 " AND (coitem_scheddate BETWEEN <? value(\"startDate\") ?>"
		 "                           AND <? value(\"endDate\") ?>)"
		 "<? if exists(\"warehous_id\") ?>"
		 " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
		 "<? endif ?>"
		 "<? if exists(\"custtype_id\") ?>"
		 " AND (cust_custtype_id=<? value(\"custtype_id\") ?>)"
		 "<? elseif exists(\"custtype_pattern\") ?>"
		 " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype"
		 "  WHERE (custtype_code ~ <? value(\"custtype_pattern\") ?>)))"
		 "<? endif ?>"
		 ") "
		 "GROUP BY cohead_id, cohead_number, cust_name,"
		 "         cohead_holdtype, cohead_orderdate, cohead_packdate,"
		 "         cosmisc_shipped, cosmisc_shipvia, cosmisc_shipdate,"
		 "         cosmisc_id, cobmisc_cohead_id "
		 " ORDER BY "
		 "<? if exists(\"orderByShipDate\") ?>scheddate,"
		 "<? elseif exists(\"orderByPackDate\") ?>cohead_packdate,"
		 "<? endif ?>"
		 "          cohead_number, cosmisc_shipped;");

    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);
    if (q.first())
    {
      double        totalSales  = 0.0;
      double        totalCost   = 0.0;
      double        totalMargin = 0.0;
      int           coheadid    = -1;
      bool          unshipped   = FALSE;
      bool          overbilled  = FALSE;

      do
      {
        if ( (coheadid != q.value("cohead_id").toInt()) || (!orderLine) )
        {
          coheadid = q.value("cohead_id").toInt();
          unshipped = FALSE;
          overbilled = FALSE;

          orderLine = new XTreeWidgetItem( _so, orderLine,
					 q.value("cohead_id").toInt(),
					 -1,
                                         q.value("cohead_number"), q.value("cust_name"),
                                         q.value("f_holdtype"), q.value("f_orderdate"),
                                         q.value("f_scheddate"), q.value("f_packdate"),
                                         q.value("f_sales"), q.value("f_cost"),
                                         q.value("f_margin") );

          totalSales  += q.value("sales").toDouble();
          totalCost   += q.value("cost").toDouble();
          totalMargin += q.value("margin").toDouble();
        }

        if (q.value("overbilled").toBool())
        {
          overbilled = TRUE;
          orderLine->setTextColor("red");
        }

        if (q.value("shipstatus").toInt())
        {
          XTreeWidgetItem *shipLine = new XTreeWidgetItem(orderLine, -1,
						 q.value("cosmisc_id").toInt());

          if (q.value("shipstatus").toInt() == 1)
          {
            shipLine->setText(0, tr("Yes"));
            shipLine->setTextColor("green");

            if (!unshipped && !overbilled)
              orderLine->setTextColor("green");
          }
          else if (q.value("shipstatus").toInt() == 2)
          {
            shipLine->setText(0, tr("No"));
            shipLine->setTextColor("blue");
            if(!overbilled)
              orderLine->setTextColor("blue");
            unshipped = TRUE;
          }

          shipLine->setText(1, q.value("shipvia"));
	  shipLine->setText(2, q.value("cosmisc_number"));
          shipLine->setText(3, q.value("shipdate"));
        }
      }
      while (q.next());

      if (_showPrices->isChecked())
        new XTreeWidgetItem( _so, orderLine, -1,
                           "", "", tr("Total Backlog"), "", "", "",
                           formatMoney(totalSales),
                           formatCost(totalCost),
                           formatMoney(totalMargin) );

      QString tots = "SELECT COUNT(cohead_id) AS totalorders "
		     "FROM ( SELECT DISTINCT cohead_id "
		     "       FROM cohead, coitem, itemsite, cust "
		     "       WHERE ( (coitem_cohead_id=cohead_id)"
		     "        AND (coitem_itemsite_id=itemsite_id)"
		     "        AND (cohead_cust_id=cust_id)"
		     "        AND (coitem_status NOT IN ('C','X'))"
		     " AND (coitem_scheddate BETWEEN <? value(\"startDate\") ?>"
		     "                           AND <? value(\"endDate\") ?>)"
		     "<? if exists(\"warehous_id\") ?>"
		     " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
		     "<? endif ?>"
		     "<? if exists(\"custtype_id\") ?>"
		     " AND (cust_custtype_id=<? value(\"custtype_id\") ?>)"
		     "<? elseif exists(\"custtype_pattern\") ?>"
		     " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype"
		     "  WHERE (custtype_code ~ <? value(\"custtype_pattern\") ?>)))"
		     "<? endif ?>"
		     ") ) AS data;";

      MetaSQLQuery totm(tots);
      q = totm.toQuery(params);
      if (q.first())
        _totalSalesOrders->setText(q.value("totalorders").toString());
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      QString cnts = "SELECT COUNT(coitem.*) AS totalitems "
		     "FROM cohead, coitem, itemsite, cust "
		     "WHERE ( (coitem_cohead_id=cohead_id)"
		     " AND (coitem_itemsite_id=itemsite_id)"
		     " AND (cohead_cust_id=cust_id)"
		     " AND (coitem_status NOT IN ('C','X'))"
		     " AND (coitem_scheddate BETWEEN <? value(\"startDate\") ?>"
		     "                           AND <? value(\"endDate\") ?>)"
		     "<? if exists(\"warehous_id\") ?>"
		     " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
		     "<? endif ?>"
		     "<? if exists(\"custtype_id\") ?>"
		     " AND (cust_custtype_id=<? value(\"custtype_id\") ?>)"
		     "<? elseif exists(\"custtype_pattern\") ?>"
		     " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype"
		     "  WHERE (custtype_code ~ <? value(\"custtype_pattern\") ?>)))"
		     "<? endif ?>"
		     ");";

      MetaSQLQuery cntm(cnts);
      q = cntm.toQuery(params);
      if (q.first())
        _totalLineItems->setText(q.value("totalitems").toString());
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      QString qtys = "SELECT formatQty(SUM(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned)) AS f_totalqty "
		     "FROM cohead, coitem, itemsite, item, cust "
		     "WHERE ( (coitem_cohead_id=cohead_id)"
		     " AND (coitem_itemsite_id=itemsite_id)"
		     " AND (itemsite_item_id=item_id)"
		     " AND (cohead_cust_id=cust_id)"
		     " AND (coitem_status NOT IN ('C','X'))"
		     " AND (coitem_scheddate BETWEEN <? value(\"startDate\") ?>"
		     "                           AND <? value(\"endDate\") ?>)"
		     "<? if exists(\"warehous_id\") ?>"
		     " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
		     "<? endif ?>"
		     "<? if exists(\"custtype_id\") ?>"
		     " AND (cust_custtype_id=<? value(\"custtype_id\") ?>)"
		     "<? elseif exists(\"custtype_pattern\") ?>"
		     " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype"
		     "  WHERE (custtype_code ~ <? value(\"custtype_pattern\") ?>)))"
		     "<? endif ?>"
		     ");";

      MetaSQLQuery qtym(qtys);
      q = qtym.toQuery(params);
      if (q.first())
        _totalQty->setText(q.value("f_totalqty").toString());
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    else
    {
      _totalSalesOrders->clear();
      _totalLineItems->clear();
      _totalQty->clear();
    }
  }
}

