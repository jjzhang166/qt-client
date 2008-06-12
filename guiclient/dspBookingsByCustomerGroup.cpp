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

#include "dspBookingsByCustomerGroup.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>
#include <QWorkspace>
#include <openreports.h>

#include "salesHistoryInformation.h"

dspBookingsByCustomerGroup::dspBookingsByCustomerGroup(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sohist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrice(bool)));

  _customerGroup->setType(CustomerGroup);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _sohist->addColumn(tr("S/O #"),       _orderColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Invoice #"),   _orderColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Ord. Date"),   _dateColumn,  Qt::AlignCenter );
  _sohist->addColumn(tr("Invc. Date"),  _dateColumn,  Qt::AlignCenter );
  _sohist->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _sohist->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _sohist->addColumn(tr("Shipped"),     _qtyColumn,   Qt::AlignRight  );

  if (_privileges->check("ViewCustomerPrices"))
  {
    _sohist->addColumn(tr("Unit Price"),  _priceColumn, Qt::AlignRight  );
    _sohist->addColumn(tr("Total"),       _moneyColumn, Qt::AlignRight  );
  }

  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices"));
  _showCosts->setEnabled(_privileges->check("ViewCosts"));

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _showPrices->setChecked(_privileges->check("ViewCustomerPrices"));
    _showCosts->setChecked(_privileges->check("ViewCosts"));
  }

  sHandlePrice(_showPrices->isChecked());
}

dspBookingsByCustomerGroup::~dspBookingsByCustomerGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBookingsByCustomerGroup::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspBookingsByCustomerGroup::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("custgrp_id", &valid);
  if (valid)
    _customerGroup->setId(param.toInt());

  param = pParams.value("custgrp_pattern", &valid);
  if (valid)
    _customerGroup->setPattern(param.toString());

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

void dspBookingsByCustomerGroup::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("EditSalesHistory"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspBookingsByCustomerGroup::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohist_id", _sohist->id());

  salesHistoryInformation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspBookingsByCustomerGroup::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("sohist_id", _sohist->id());

  salesHistoryInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspBookingsByCustomerGroup::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _customerGroup->appendValue(params);
  _dates->appendValue(params);

  if (_showCosts->isChecked())
    params.append("showCosts");

  if (_showPrices->isChecked())
    params.append("showPrices");

  orReport report("BookingsByCustomerGroup", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBookingsByCustomerGroup::sHandlePrice(bool pShowPrice)
{
  if (pShowPrice)
  {
    _sohist->showColumn(7);
    _sohist->showColumn(8);
  }
  else
  {
    _sohist->hideColumn(7);
    _sohist->hideColumn(8);
  }
}

void dspBookingsByCustomerGroup::sFillList()
{
  _sohist->clear();

  if (!checkParameters())
    return;

  QString sql( "SELECT cohist_id, cohist_ordernumber, cohist_invcnumber,"
               "       formatDate(cohist_orderdate) AS f_orderdate,"
               "       formatDate(cohist_invcdate) AS f_invcdate,"
               "       item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
               "       cohist_qtyshipped AS shipped,"
               "       formatQty(cohist_qtyshipped) AS f_shipped,"
               "       formatSalesPrice(cohist_unitprice) AS f_price,"
               "       round(cohist_qtyshipped * cohist_unitprice,2) AS extended,"
               "       formatMoney(round(cohist_qtyshipped * cohist_unitprice)) AS f_extended "
               "FROM cohist, itemsite, item, cust, custgrpitem, custgrp "
               "WHERE ( (cohist_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (cohist_cust_id=cust_id)"
               " AND (custgrpitem_custgrp_id=custgrp_id)"
               " AND (custgrpitem_cust_id=cust_id)"
               " AND (cohist_invcdate BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_customerGroup->isSelected())
    sql += " AND (custgrp_id=:custgrp_id)";
  else if (_customerGroup->isPattern())
    sql += " AND (custgrp_name ~ :custgrp_pattern)";

  sql += ") "
         "ORDER BY cohist_invcdate, item_number";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _customerGroup->bindValue(q);
  _dates->bindValue(q);
  q.exec();
  if (q.first())
  {
    double        totalUnits = 0.0;
    double        totalSales = 0.0;

    XTreeWidgetItem *last  = 0;
    do
    {
      last = new XTreeWidgetItem( _sohist, last, q.value("cohist_id").toInt(),
			       q.value("cohist_ordernumber"), q.value("cohist_invcnumber"),
			       q.value("f_orderdate"), q.value("f_invcdate"),
			       q.value("item_number"), q.value("description"),
			       q.value("f_shipped"), q.value("f_price"),
			       q.value("f_extended") );

       totalUnits += q.value("shipped").toDouble();
       totalSales += q.value("extended").toDouble();
    }
    while (q.next());

    XTreeWidgetItem *totals = new XTreeWidgetItem(_sohist, last, -1);
    totals->setText(5, tr("Totals"));
    totals->setText(6, formatQty(totalUnits));
    totals->setText(8, formatMoney(totalSales));
  }
}

bool dspBookingsByCustomerGroup::checkParameters()
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
