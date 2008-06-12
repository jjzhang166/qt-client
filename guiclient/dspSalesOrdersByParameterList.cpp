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

#include "dspSalesOrdersByParameterList.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "returnAuthorization.h"
#include "salesOrder.h"

dspSalesOrdersByParameterList::dspSalesOrdersByParameterList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_so, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_dates, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_parameter, SIGNAL(updated()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Starting Order Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Ending Order Date:"));

  _so->addColumn(tr("Customer"),    _itemColumn,  Qt::AlignLeft   );
  _so->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft   );
  _so->addColumn(tr("Ordered"),     _dateColumn,  Qt::AlignRight  );
  _so->addColumn(tr("Scheduled"),   _dateColumn,  Qt::AlignRight  );
  _so->addColumn(tr("Status"),      _itemColumn,  Qt::AlignCenter );
  _so->addColumn(tr("Ship-to"),     -1,           Qt::AlignLeft   );
  _so->addColumn(tr("Cust. P/O #"), 200,          Qt::AlignLeft   );

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())  );
}

dspSalesOrdersByParameterList::~dspSalesOrdersByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSalesOrdersByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSalesOrdersByParameterList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("custtype", &valid);
  if (valid)
    _parameter->setType(CustomerType);

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

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (_parameter->type() == CustomerType)
    setCaption(tr("Sales Order Lookup by Customer Type"));

  return NoError;
}

void dspSalesOrdersByParameterList::sPopulateMenu(QMenu *menuThis)
{
  if(_so->altId() == -1)
    return;

  menuThis->insertItem(tr("Edit..."), this, SLOT(sEditOrder()), 0);
  menuThis->insertItem(tr("View..."), this, SLOT(sViewOrder()), 0);
  menuThis->insertSeparator();
  menuThis->insertItem(tr("Shipment Status..."), this, SLOT(sDspShipmentStatus()), 0);
  menuThis->insertItem(tr("Shipments..."), this, SLOT(sDspShipments()), 0);

  if (_privileges->check("MaintainReturns"))
  {
    menuThis->insertSeparator();
    menuThis->insertItem(tr("Create Return Authorization..."), this, SLOT(sCreateRA()));
  }
}

void dspSalesOrdersByParameterList::sEditOrder()
{
  salesOrder::editSalesOrder(_so->altId(), false);
}

void dspSalesOrdersByParameterList::sViewOrder()
{
  salesOrder::viewSalesOrder(_so->altId());
}

void dspSalesOrdersByParameterList::sCreateRA()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("sohead_id", _so->altId());

  returnAuthorization *newdlg = new returnAuthorization();
  if (newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
  else
    QMessageBox::critical(this, tr("Could Not Open Window"),
			  tr("The new Return Authorization could not be created"));
}

void dspSalesOrdersByParameterList::sDspShipmentStatus()
{
  ParameterList params;
  params.append("sohead_id", _so->altId());
  params.append("run");

  dspSalesOrderStatus *newdlg = new dspSalesOrderStatus();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSalesOrdersByParameterList::sDspShipments()
{
  ParameterList params;
  params.append("sohead_id", _so->altId());
  params.append("run");

  dspShipmentsBySalesOrder *newdlg = new dspShipmentsBySalesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSalesOrdersByParameterList::sFillList()
{
  _so->clear();
  if (_dates->allValid()) 
  {
    QString sql( "SELECT cust_id, cohead_id, cust_number, cohead_number,"
                 "       formatDate(cohead_orderdate) AS f_orderdate,"
                 "       formatDate(MIN(soitem.coitem_scheddate)) AS f_scheddate,"
                 "       CASE"
                 "        WHEN ( (SELECT COUNT(*)"
                 "                FROM coitem"
                 "                WHERE ((coitem_status<>'X') AND (coitem_cohead_id=cohead_id))) = 0) THEN :noLines"
                 "        WHEN ( ( (SELECT COUNT(*)"
                 "                  FROM coitem"
                 "                  WHERE ((coitem_status='C')"
                 "                   AND (coitem_cohead_id=cohead_id))) > 0) AND"
                 "               ( (SELECT COUNT(*)"
                 "                  FROM coitem"
                 "                  WHERE ((coitem_status NOT IN ('C','X'))"
                 "                   AND (coitem_cohead_id=cohead_id))) = 0) ) THEN :closed"
                 "        WHEN ( ( (SELECT COUNT(*)"
                 "                  FROM coitem"
                 "                  WHERE ((coitem_status='C')"
                 "                   AND (coitem_cohead_id=cohead_id))) = 0) AND"
                 "               ( (SELECT COUNT(*)"
                 "                  FROM coitem"
                 "                  WHERE ((coitem_status NOT IN ('C','X'))"
                 "                   AND (coitem_cohead_id=cohead_id))) > 0) ) THEN :open"
                 "        ELSE :partial"
                 "       END AS f_status,"
                 "       cohead_shiptoname, cohead_custponumber "
                 "FROM cohead, coitem as soitem, cust "
                 "WHERE ( (cohead_cust_id=cust_id)"
                 " AND (soitem.coitem_cohead_id=cohead_id)"
                 " AND (cohead_orderdate BETWEEN :startDate AND :endDate)" );

    if (_parameter->isSelected())
    {
      if (_parameter->type() == CustomerType)
        sql += " AND (cust_custtype_id=:custtype_id)";
    }
    else if (_parameter->isPattern())
    {
      if (_parameter->type() == CustomerType)
        sql += " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ :custtype_pattern)))";
    }

    sql += ") "
           "GROUP BY cust_id, cust_number, cohead_id, cohead_number, cohead_orderdate,"
           "         cohead_shiptoname, cohead_custponumber "
           "ORDER BY cust_number, cohead_number;";

    q.prepare(sql);
    _dates->bindValue(q);
    _parameter->bindValue(q);
    q.bindValue(":noLines", tr("No Lines"));
    q.bindValue(":closed", tr("Closed"));
    q.bindValue(":open", tr("Open"));
    q.bindValue(":partial", tr("Partial"));
    q.exec();
    int custid = -1;
    XTreeWidgetItem *lastcust = 0;
    XTreeWidgetItem *lastorder = 0;
    while(q.next())
    {
      if(q.value("cust_id").toInt() != custid)
      {
        custid = q.value("cust_id").toInt();
        lastcust = new XTreeWidgetItem(_so, lastcust, custid, -1, q.value("cust_number"));
        lastorder = 0;
      }

      lastorder = new XTreeWidgetItem(lastcust, lastorder, custid, q.value("cohead_id").toInt(),
                                "", q.value("cohead_number"), q.value("f_orderdate"),
                                q.value("f_scheddate"), q.value("f_status"),
                                q.value("cohead_shiptoname"), q.value("cohead_custponumber") );
    }
  }
}

