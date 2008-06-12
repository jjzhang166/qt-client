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

#include "dspBriefSalesHistoryBySalesRep.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>
#include <openreports.h>

#include "salesHistoryInformation.h"

dspBriefSalesHistoryBySalesRep::dspBriefSalesHistoryBySalesRep(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sohist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));

  _salesrep->setType(XComboBox::SalesRepsActive);
  _productCategory->setType(ProductCategory);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _sohist->addColumn(tr("Customer"),    -1,           Qt::AlignLeft  );
  _sohist->addColumn(tr("S/O #"),      _orderColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Invoice #"),  _orderColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Ord. Date"),  _dateColumn,  Qt::AlignCenter );
  _sohist->addColumn(tr("Invc. Date"), _dateColumn,  Qt::AlignCenter );
  _sohist->addColumn( tr("Ext. Price"), _moneyColumn, Qt::AlignRight );
  _sohist->addColumn( tr("Ext. Cost"), _costColumn, Qt::AlignRight );

  _showCosts->setEnabled(_privileges->check("ViewCosts"));
  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices"));

  sHandleParams();

  _salesrep->setFocus();
}

dspBriefSalesHistoryBySalesRep::~dspBriefSalesHistoryBySalesRep()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBriefSalesHistoryBySalesRep::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspBriefSalesHistoryBySalesRep::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("salesrep_id", &valid);
  if (valid)
    _salesrep->setId(param.toInt());

  param = pParams.value("prodcat_id", &valid);
  if (valid)
    _productCategory->setId(param.toInt());

  param = pParams.value("prodcat_pattern", &valid);
  if (valid)
    _productCategory->setPattern(param.toString());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

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

void dspBriefSalesHistoryBySalesRep::sHandleParams()
{
  if (_showPrices->isChecked())
    _sohist->showColumn(5);
  else
    _sohist->hideColumn(5);

  if (_showCosts->isChecked())
    _sohist->showColumn(6);
  else
    _sohist->hideColumn(6);
}

void dspBriefSalesHistoryBySalesRep::sPopulateMenu(QMenu *)
{
}

void dspBriefSalesHistoryBySalesRep::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr("Please enter a valid Start and End Date.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  params.append("salesrep_id", _salesrep->id());

  _productCategory->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_showCosts->isChecked())
    params.append("showCosts");

  if (_showPrices->isChecked())
    params.append("showPrices");

  orReport report("SalesHistoryBySalesRep", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBriefSalesHistoryBySalesRep::sFillList()
{
  _sohist->clear();

  if (!checkParameters())
    return;

  QString sql( "SELECT cust_id, cust_name, cohist_ordernumber,"
               "       CASE WHEN (cohist_invcnumber='-1') THEN 'Credit'"
               "            ELSE TEXT(cohist_invcnumber)"
               "       END AS invoicenumber,"
               "       formatDate(cohist_orderdate) AS f_orderdate,"
               "       formatDate(cohist_invcdate, 'Return') AS f_invcdate,"
	       "       SUM(round(cohist_qtyshipped * cohist_unitprice,2)) AS extprice,"
	       "       formatMoney(SUM(round(cohist_qtyshipped * cohist_unitprice,2))) AS f_extprice,"
	       "       SUM(cohist_qtyshipped * cohist_unitcost) AS extcost,"
	       "       formatCost(SUM(cohist_qtyshipped * cohist_unitcost)) AS f_extcost "
	       "FROM cohist, cust, itemsite, item, prodcat "
	       "WHERE ( (cohist_itemsite_id=itemsite_id)"
	       " AND (cohist_cust_id=cust_id)"
	       " AND (itemsite_item_id=item_id)"
	       " AND (item_prodcat_id=prodcat_id)"
	       " AND (cohist_salesrep_id=:salesrep_id)"
	       " AND (cohist_invcdate BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_productCategory->isSelected())
    sql += " AND (prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += " AND (prodcat_code ~ :prodcat_pattern)";

  sql += ") "
         "GROUP BY cust_id, cust_name, cohist_ordernumber, cohist_invcnumber,"
         "         cohist_orderdate, cohist_invcdate "
         "ORDER BY cohist_invcdate;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _productCategory->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":salesrep_id", _salesrep->id());
  q.exec();
  if (q.first())
  {
    double totalSales = 0.0;
    double totalCosts = 0.0;

    XTreeWidgetItem *last  = 0;
    do
    {
      last = new XTreeWidgetItem(_sohist, 0, q.value("cust_id").toInt(),
				 q.value("cust_name"),
				 q.value("cohist_ordernumber"),
				 q.value("invoicenumber"),
				 q.value("f_orderdate"),
				 q.value("f_invcdate"),
				 q.value("f_extprice"),
				 q.value("f_extcost"));
 
      totalSales += q.value("extprice").toDouble();
      totalCosts += q.value("extcost").toDouble();
    }
    while (q.next());

    if ( (_showPrices->isChecked()) || (_showCosts->isChecked()) )
    {
      XTreeWidgetItem *totals = new XTreeWidgetItem(_sohist, last, -1, QVariant(tr("Total Sales")));

      totals->setText(5, formatMoney(totalSales));
      totals->setText(6, formatCost(totalCosts));
    }
  }
}

bool dspBriefSalesHistoryBySalesRep::checkParameters()
{
  if (isVisible())
  {
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
