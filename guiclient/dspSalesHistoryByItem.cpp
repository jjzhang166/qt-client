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

#include "dspSalesHistoryByItem.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include "salesHistoryInformation.h"

#define UNITPRICE_COL	6
#define EXTPRICE_COL	7
#define UNITCOST_COL	(8 - (_privileges->check("ViewCustomerPrices") ? 0 : 2))
#define EXTCOST_COL	(9 - (_privileges->check("ViewCustomerPrices") ? 0 : 2))

/*
 *  Constructs a dspSalesHistoryByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSalesHistoryByItem::dspSalesHistoryByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sohist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));
  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));

  _item->setType(ItemLineEdit::cSold);
  _customerType->setType(CustomerType);

  _sohist->addColumn(tr("Customer"),   -1,           Qt::AlignLeft   );
  _sohist->addColumn(tr("S/O #"),      _orderColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Ord. Date"),  _dateColumn,  Qt::AlignCenter );
  _sohist->addColumn(tr("Invoice #"),  _orderColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Invc. Date"), _dateColumn,  Qt::AlignCenter );
  _sohist->addColumn(tr("Shipped"),    _qtyColumn,   Qt::AlignRight  );
  if (_privileges->check("ViewCustomerPrices"))
  {
    _sohist->addColumn( tr("Unit Price"), _priceColumn,    Qt::AlignRight );
    _sohist->addColumn( tr("Ext. Price"), _bigMoneyColumn, Qt::AlignRight );
  }
  if (_privileges->check("ViewCosts"))
  {
    _sohist->addColumn( tr("Unit Cost"), _costColumn, Qt::AlignRight );
    _sohist->addColumn( tr("Ext. Cost"), _costColumn, Qt::AlignRight );
  }

  _showCosts->setEnabled(_privileges->check("ViewCosts"));
  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices"));

  sHandleParams();

  _item->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSalesHistoryByItem::~dspSalesHistoryByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSalesHistoryByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSalesHistoryByItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    _item->setItemsiteid(param.toInt());

  param = pParams.value("custtype_id", &valid);
  if (valid)
    _customerType->setId(param.toInt());

  param = pParams.value("custtype_pattern", &valid);
  if (valid)
    _customerType->setPattern(param.toString());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
  else
    _warehouse->setAll();

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspSalesHistoryByItem::sHandleParams()
{
  if (_showPrices->isChecked())
  {
    _sohist->showColumn(UNITPRICE_COL);
    _sohist->showColumn(EXTPRICE_COL);
  }
  else
  {
    _sohist->hideColumn(UNITPRICE_COL);
    _sohist->hideColumn(EXTPRICE_COL);
  }

  if (_showCosts->isChecked())
  {
    _sohist->showColumn(UNITCOST_COL);
    _sohist->showColumn(EXTCOST_COL);
  }
  else
  {
    _sohist->hideColumn(UNITCOST_COL);
    _sohist->hideColumn(EXTCOST_COL);
  }
}

void dspSalesHistoryByItem::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("EditSalesHistory"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspSalesHistoryByItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohist_id", _sohist->id());

  salesHistoryInformation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspSalesHistoryByItem::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("sohist_id", _sohist->id());

  salesHistoryInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspSalesHistoryByItem::sPrint()
{
  if (isVisible())
  {
    if (!_dates->allValid())
    {
        QMessageBox::warning( this, tr("Enter Valid Dates"),
                              tr("Please enter a valid Start and End Date.") );
        _dates->setFocus();
        return;
    }
  }

  ParameterList params;
  _warehouse->appendValue(params);
  _customerType->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _item->id());

  if (_showCosts->isChecked())
    params.append("showCosts");

  if (_showPrices->isChecked())
    params.append("showPrices");

  orReport report("SalesHistoryByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSalesHistoryByItem::sFillList()
{
  _sohist->clear();

  if (!checkParameters())
    return;

  QString sql( "SELECT cohist_id, cust_name, cohist_ordernumber,"
               "       formatDate(cohist_orderdate) AS f_orderdate,"
               "       CASE WHEN (cohist_invcnumber='-1') THEN 'Credit'"
               "            ELSE TEXT(cohist_invcnumber)"
               "       END AS invoicenumber,"
               "       formatDate(cohist_invcdate, 'Return') AS f_invcdate,"
               "       cohist_qtyshipped, formatQty(cohist_qtyshipped) AS f_shipped " );

  sql += ", formatSalesPrice(cohist_unitprice) AS f_price,"
	 "  round(cohist_qtyshipped * cohist_unitprice, 2) AS extprice,"
	 "  formatMoney(round(cohist_qtyshipped * cohist_unitprice, 2)) AS f_extprice ";

  sql += ", formatSalesPrice(cohist_unitcost) AS f_cost,"
	 "  (cohist_qtyshipped * cohist_unitcost) AS extcost,"
	 "  formatMoney(cohist_qtyshipped * cohist_unitcost) AS f_extcost ";

  sql += "FROM cohist, cust, itemsite, item "
         "WHERE ( (cohist_itemsite_id=itemsite_id)"
         " AND (cohist_cust_id=cust_id)"
         " AND (itemsite_item_id=item_id)"
         " AND (item_id=:item_id)"
         " AND (cohist_invcdate BETWEEN :startDate AND :endDate) ";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_customerType->isSelected())
    sql += " AND (cust_custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ :custtype_pattern)))";

  sql += ") "
         "ORDER BY cohist_invcdate, cust_number; ";

  q.prepare(sql);
  _customerType->bindValue(q);
  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    double totalSales = 0.0;
    double totalUnits = 0.0;
    double totalCost  = 0.0;
    XTreeWidgetItem *last = 0;

    do
    {
      last = new XTreeWidgetItem(_sohist, last,
				 q.value("cohist_id").toInt(),
				 q.value("cust_name"),
				 q.value("cohist_ordernumber"),
				 q.value("f_orderdate"),
				 q.value("invoicenumber"),
				 q.value("f_invcdate"),
				 q.value("f_shipped"),
				 q.value("f_price"),
				 q.value("f_extprice"),
				 q.value("f_cost"),
				 q.value("f_extcost"));
 
      totalUnits += q.value("cohist_qtyshipped").toDouble();
      totalSales += q.value("extprice").toDouble();
      totalCost  += q.value("extcost").toDouble();
    }
    while (q.next());

    XTreeWidgetItem *totals = new XTreeWidgetItem(_sohist, last, -1,
						  QVariant(tr("Totals")));
    totals->setText(5, formatQty(totalUnits));
    totals->setText(EXTPRICE_COL, formatMoney(totalSales));
    totals->setText(EXTCOST_COL, formatMoney(totalCost));
  }
}

bool dspSalesHistoryByItem::checkParameters()
{
  if (isVisible())
  {
    if (!_item->isValid())
    {
      QMessageBox::warning( this, tr("Enter Item Number"),
                            tr("Please enter a valid Item Number.") );
      _item->setFocus();
      return FALSE;
    }

    if (!_dates->startDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
      return FALSE;
    }

    if (!_dates->endDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
      return FALSE;
    }
  }

  return TRUE;
}

