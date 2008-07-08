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
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
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

#include "dspBacklogByParameterList.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "salesOrder.h"
#include "salesOrderItem.h"
#include "printPackingList.h"

#define	AMOUNT_COL	8

dspBacklogByParameterList::dspBacklogByParameterList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));
  connect(_soitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _soitem->setSelectionMode(QAbstractItemView::ExtendedSelection);
  _soitem->setRootIsDecorated(TRUE);
  _soitem->addColumn(tr("S/O #/Line #"),         _itemColumn,  Qt::AlignRight  );
  _soitem->addColumn(tr("Customer/Item Number"), -1,           Qt::AlignLeft   );
  _soitem->addColumn(tr("Order"),                _dateColumn,  Qt::AlignRight  );
  _soitem->addColumn(tr("Ship/Sched."),          _dateColumn,  Qt::AlignCenter );
  _soitem->addColumn(tr("UOM"),                  _uomColumn,   Qt::AlignCenter );
  _soitem->addColumn(tr("Ordered"),              _qtyColumn,   Qt::AlignRight  );
  _soitem->addColumn(tr("Shipped"),              _qtyColumn,   Qt::AlignRight  );
  _soitem->addColumn(tr("Balance"),              _qtyColumn,   Qt::AlignRight  );
  if (_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"))
    _soitem->addColumn(tr("Amount (base)"),           _moneyColumn, Qt::AlignRight  );

  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"));

  if (! _showPrices->isChecked())
    _soitem->hideColumn(AMOUNT_COL);
}

dspBacklogByParameterList::~dspBacklogByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBacklogByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspBacklogByParameterList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("custtype_id", &valid);
  if (valid)
  {
    _parameter->setType(CustomerType);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("custtype_pattern", &valid);
  if (valid)
  {
    _parameter->setType(CustomerType);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("custtype", &valid);
  if (valid)
    _parameter->setType(CustomerType);

  param = pParams.value("custgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(CustomerGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("custgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(CustomerGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("custgrp", &valid);
  if (valid)
    _parameter->setType(CustomerGroup);

  param = pParams.value("prodcat_id", &valid);
  if (valid)
  {
    _parameter->setType(ProductCategory);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("prodcat_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ProductCategory);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("prodcat", &valid);
  if (valid)
    _parameter->setType(ProductCategory);

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  switch (_parameter->type())
  {
    case CustomerType:
      setCaption(tr("Backlog by Customer Type"));
      break;

    case CustomerGroup:
      setCaption(tr("Backlog by Customer Group"));
      break;

    case ProductCategory:
      setCaption(tr("Backlog by Product Category"));
      break;

    default:
      break;
  }

  return NoError;
}

void dspBacklogByParameterList::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
    _soitem->showColumn(AMOUNT_COL);
  else
    _soitem->hideColumn(AMOUNT_COL);
}

void dspBacklogByParameterList::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _parameter->appendValue(params);
  _dates->appendValue(params);

  if (_showPrices->isChecked())
    params.append("showPrices");

  if (_parameter->isAll())
  {
    switch (_parameter->type())
    {
      case CustomerType:
        params.append("custtype");
        break;

      case CustomerGroup:
        params.append("custgrp");
        break;

      case ProductCategory:
        params.append("prodcat");
        break;

      default:
        break;
    }
  }

  orReport report("BacklogByParameterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBacklogByParameterList::sEditOrder()
{
  salesOrder::editSalesOrder(_soitem->id(), false);
}

void dspBacklogByParameterList::sViewOrder()
{
  salesOrder::viewSalesOrder(_soitem->id());
}

void dspBacklogByParameterList::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("soitem_id", _soitem->altId());
      
  salesOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByParameterList::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", _soitem->altId());
      
  salesOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByParameterList::sPrintPackingList()
{
  QList <QTreeWidgetItem*> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("sohead_id", ((XTreeWidgetItem*)(selected[i]))->id());

    printPackingList newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspBacklogByParameterList::sAddToPackingListBatch()
{
  QList <QTreeWidgetItem*> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.prepare("SELECT addToPackingListBatch(:sohead_id) AS result;");
    q.bindValue(":sohead_id", ((XTreeWidgetItem*)(selected[i]))->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void dspBacklogByParameterList::sPopulateMenu(QMenu *pMenu)
{
  QList <QTreeWidgetItem*> selected = _soitem->selectedItems();

  int menuItem;

  if (selected.size() == 1)
  {
    menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
    if (!_privileges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);
    if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
      pMenu->setItemEnabled(menuItem, FALSE);

    if (_soitem->altId() != -1)
    {
      pMenu->insertSeparator();

      menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
      if (!_privileges->check("MaintainSalesOrders"))
        pMenu->setItemEnabled(menuItem, FALSE);

      menuItem = pMenu->insertItem(tr("View Item..."), this, SLOT(sViewItem()), 0);
      if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
        pMenu->setItemEnabled(menuItem, FALSE);
    }
  }

  if (_soitem->id() > 0)
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Print Packing List..."), this, SLOT(sPrintPackingList()), 0);
    if (!_privileges->check("PrintPackingLists"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Add to Packing List Batch..."), this, SLOT(sAddToPackingListBatch()), 0);
    if (!_privileges->check("MaintainPackingListBatch"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspBacklogByParameterList::sFillList()
{
  _soitem->clear();

  if (_dates->allValid())
  {
    QString sql( "SELECT cohead_id, coitem_id, cohead_number, "
		 "       coitem_linenumber, cust_name,"
                 "       formatDate(cohead_orderdate) AS f_orderdate,"
                 "       formatDate((SELECT MIN(coitem_scheddate) "
		 "                   FROM coitem "
		 "                   WHERE (coitem_cohead_id=cohead_id))) AS f_shipdate,"
                 "       formatDate(coitem_scheddate) AS f_scheddate,"
                 "       item_number, uom_name,"
                 "       formatQty(coitem_qtyord) AS f_ordered,"
                 "       formatQty(coitem_qtyshipped) AS f_shipped,"
                 "       formatQty(noNeg(coitem_qtyord - coitem_qtyshipped + "
		 "                       coitem_qtyreturned)) AS f_balance, "
                 "       formatMoney(round((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) * (currtobase(cohead_curr_id,coitem_price,current_date) / coitem_price_invuomratio),2)) AS f_backlog,"
                 "       round((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) * (currtobase(cohead_curr_id,coitem_price,current_date) / coitem_price_invuomratio),2) AS backlog "
                 "FROM cohead, coitem, itemsite, item, cust, uom "
                 "WHERE ((coitem_cohead_id=cohead_id)"
                 " AND (cohead_cust_id=cust_id)"
                 " AND (coitem_itemsite_id=itemsite_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (coitem_qty_uom_id=uom_id)"
                 " AND (coitem_status NOT IN ('C','X'))"
                 " AND (coitem_scheddate BETWEEN <? value(\"startDate\") ?>"
		 "                             AND <? value(\"endDate\") ?>)"
		 "<? if exists(\"warehous_id\") ?>"
		 " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>) "
		 "<? endif ?>"
		 "<? if exists(\"custtype_id\") ?>"
		 " AND (cust_custtype_id=<? value(\"custtype_id\") ?>)"
		 "<? elseif exists(\"custtype_pattern\") ?>"
		 " AND (cust_custtype_id IN (SELECT custtype_id "
		 "      FROM custtype "
		 "      WHERE (custtype_code ~ <? value(\"custtype_pattern\") ?>)))"
		 "<? elseif exists(\"by_custgrp\" ?>"
		 " AND (cust_id IN (SELECT DISTINCT custgrpitem_cust_id "
		 "                  FROM custgrpitem))"
		 "<? elseif exists(\"custgrp_id\") ?>"
		 " AND (cust_id IN (SELECT DISTINCT custgrpitem_cust_id"
		 "      FROM custgrpitem"
		 "      WHERE (custgrpitem_custgrp_id=<? value(\"custgrp_id\") ?>)))"
		 "<? elseif exists(\"custgrp_pattern\") ?>"
		 " AND (cust_id IN (SELECT DISTINCT custgrpitem_cust_id"
		 "      FROM custgrp, custgrpitem"
		 "      WHERE ((custgrpitem_custgrp_id=custgrp_id)"
		 "        AND (custgrp_name ~ <? value(\"custgrp_pattern\") ?>) )))"
		 "<? elseif exists(\"prodcat_id\") ?>"
		 " AND (item_prodcat_id=<? value(\"prodcat_id\") ?>)"
		 "<? elseif exists(\"prodcat_pattern\") ?>"
		 " AND (item_prodcat_id IN (SELECT DISTINCT prodcat_id"
		 "      FROM prodcat"
		 "      WHERE (prodcat_code ~ <? value(\"prodcat_pattern\") ?>)))"
		 "<? endif ?>"
		 " ) "
		 "ORDER BY coitem_scheddate, cohead_number, "
		 "         coitem_linenumber DESC;" );

    ParameterList params;
    if (_warehouse->isSelected())
      params.append("warehous_id", _warehouse->id());

    params.append("startDate", _dates->startDate());
    params.append("endDate",   _dates->endDate());

    if (_parameter->type() == CustomerType)
    {
      if (_parameter->isSelected())
        params.append("custtype_id", _parameter->id());
      else if (_parameter->isPattern())
        params.append("custtype_pattern", _parameter->pattern());
    }
    else if (_parameter->type() == CustomerGroup)
    {
      if (_parameter->isAll())
        params.append("by_custgrp");
      else if (_parameter->isSelected())
        params.append("custgrp_id", _parameter->id());
      else if (_parameter->isPattern())
        params.append("custgrp_pattern", _parameter->pattern());
    }
    else if (_parameter->type() == ProductCategory)
    {
      if (_parameter->isSelected())
        params.append("prodcat_id", _parameter->id());
      else if (_parameter->isPattern())
        params.append("prodcat_pattern", _parameter->pattern());
    }

    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);
    if (q.first())
    {
      XTreeWidgetItem *head = NULL;
      int soheadid        = -1;
      double totalBacklog = 0.0;

      do
      {
        if (soheadid != q.value("cohead_id").toInt())
        {
          soheadid = q.value("cohead_id").toInt();
          head = new XTreeWidgetItem( _soitem, head, soheadid, -1,
                                    q.value("cohead_number"), q.value("cust_name"),
                                    q.value("f_orderdate"), q.value("f_shipdate") );
        }

        new XTreeWidgetItem( head, soheadid, q.value("coitem_id").toInt(),
                           q.value("coitem_linenumber"), q.value("item_number"),
                           "", q.value("f_scheddate"),
                           q.value("uom_name"), q.value("f_ordered"),
                           q.value("f_shipped"), q.value("f_balance"),
                           q.value("f_backlog") );

        totalBacklog += q.value("backlog").toDouble();
      }
      while (q.next());

      if (_showPrices->isChecked())
      {
        XTreeWidgetItem *totals = new XTreeWidgetItem(_soitem, head, -1, -1, "", tr("Total Backlog") );
        totals->setText(AMOUNT_COL, formatMoney(totalBacklog));
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
