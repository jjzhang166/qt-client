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

#include "dspSalesOrdersByCustomerPO.h"

#include <QMenu>
#include <QMessageBox>

#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "returnAuthorization.h"
#include "salesOrder.h"

dspSalesOrdersByCustomerPO::dspSalesOrdersByCustomerPO(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_so, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_dates, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_poNumber, SIGNAL(lostFocus()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Starting Order Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Ending Order Date:"));

  _so->addColumn(tr("Cust #"),      _itemColumn,  Qt::AlignLeft   );
  _so->addColumn(tr("Customer"),    _itemColumn,  Qt::AlignLeft   );
  _so->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft   );
  _so->addColumn(tr("Ordered"),     _dateColumn,  Qt::AlignRight  );
  _so->addColumn(tr("Scheduled"),   _dateColumn,  Qt::AlignRight  );
  _so->addColumn(tr("Status"),      _itemColumn,  Qt::AlignCenter );
  _so->addColumn(tr("Ship-to"),     -1,           Qt::AlignLeft   );
  _so->addColumn(tr("Cust. P/O #"), 200,          Qt::AlignLeft   );

  _poNumber->setFocus();
  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())  );
}

dspSalesOrdersByCustomerPO::~dspSalesOrdersByCustomerPO()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSalesOrdersByCustomerPO::languageChange()
{
  retranslateUi(this);
}

void dspSalesOrdersByCustomerPO::sPopulateMenu(QMenu *menuThis)
{
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

void dspSalesOrdersByCustomerPO::sEditOrder()
{
  salesOrder::editSalesOrder(_so->id(), false);
}

void dspSalesOrdersByCustomerPO::sViewOrder()
{
  salesOrder::viewSalesOrder(_so->id());
}

void dspSalesOrdersByCustomerPO::sCreateRA()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("sohead_id", _so->id());

  returnAuthorization *newdlg = new returnAuthorization();
  if (newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
  else
    QMessageBox::critical(this, tr("Could Not Open Window"),
			  tr("The new Return Authorization could not be created"));
}

void dspSalesOrdersByCustomerPO::sDspShipmentStatus()
{
  ParameterList params;
  params.append("sohead_id", _so->id());
  params.append("run");

  dspSalesOrderStatus *newdlg = new dspSalesOrderStatus();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSalesOrdersByCustomerPO::sDspShipments()
{
  ParameterList params;
  params.append("sohead_id", _so->id());
  params.append("run");

  dspShipmentsBySalesOrder *newdlg = new dspShipmentsBySalesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSalesOrdersByCustomerPO::sFillList()
{
  _so->clear();
  if (_poNumber->text().stripWhiteSpace().length() == 0)    
    return;

  if (_dates->allValid()) 
  {
    QString sql( "SELECT cohead_id, cust_number, cust_name, cohead_number,"
                 "       formatDate(cohead_orderdate),"
                 "       formatDate(MIN(soitem.coitem_scheddate)),"
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
                 "       END,"
                 "       cohead_shiptoname, cohead_custponumber "
                 "FROM cohead, coitem as soitem, cust "
                 "WHERE ((soitem.coitem_cohead_id=cohead_id)"
                 " AND (cohead_cust_id=cust_id)"
                 " AND (cohead_orderdate BETWEEN :startDate AND :endDate)" );

    sql += " AND (cohead_custponumber ~* :poNumber)";

    sql += ") "
           "GROUP BY cohead_id, cust_number, cust_name, cohead_number, cohead_orderdate,"
           "         cohead_shiptoname, cohead_custponumber "
           "ORDER BY cohead_number;";

    q.prepare(sql);
    _dates->bindValue(q);
    q.bindValue(":noLines", tr("No Lines"));
    q.bindValue(":closed", tr("Closed"));
    q.bindValue(":open", tr("Open"));
    q.bindValue(":partial", tr("Partial"));
    q.bindValue(":poNumber", _poNumber->text());
    q.exec();
    _so->populate(q);
  }
  else
    _so->clear();
}

