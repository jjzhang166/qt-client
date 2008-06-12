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

#include "dspSalesOrderStatus.h"

#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <openreports.h>

#include "inputManager.h"
#include "salesOrderList.h"

dspSalesOrderStatus::dspSalesOrderStatus(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_so, SIGNAL(newId(int)), this, SLOT(sFillList(int)));
  connect(_soList, SIGNAL(clicked()), this, SLOT(sSalesOrderList()));
  connect(_so, SIGNAL(requestList()), this, SLOT(sSalesOrderList()));

#ifndef Q_WS_MAC
  _soList->setMaximumWidth(25);
#endif

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _so, SLOT(setId(int)));

  _soitem->addColumn(tr("#"),                         _seqColumn,  Qt::AlignCenter );
  _soitem->addColumn(tr("Item"),                      _itemColumn, Qt::AlignLeft   );
  _soitem->addColumn(tr("Description"),               -1,          Qt::AlignLeft   );
  _soitem->addColumn(tr("Whs."),                      _whsColumn,  Qt::AlignCenter );
  _soitem->addColumn(tr("Ordered"),                   _qtyColumn,  Qt::AlignRight  );
  _soitem->addColumn(tr("Shipped"),                   _qtyColumn,  Qt::AlignRight  );
  _soitem->addColumn(tr("Returned"),                  _qtyColumn,  Qt::AlignRight  );
  _soitem->addColumn(tr("Invoiced"),		      _qtyColumn,  Qt::AlignRight  );
  _soitem->addColumn(tr("Balance/Close Date (User)"), 175,         Qt::AlignRight  );
  _soitem->addColumn(tr("Child Ord. #/Status"),       _itemColumn, Qt::AlignCenter );

  _so->setFocus();
}

dspSalesOrderStatus::~dspSalesOrderStatus()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSalesOrderStatus::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSalesOrderStatus::set(const ParameterList &pParams)
{ 
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
    _so->setId(param.toInt());

  return NoError;
}

void dspSalesOrderStatus::sPrint()
{
  ParameterList params;
  params.append("sohead_id", _so->id());

  orReport report("SalesOrderStatus", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSalesOrderStatus::sSalesOrderList()
{
  ParameterList params;
  params.append("sohead_id", _so->id());
  params.append("soType", (cSoOpen | cSoClosed));
  
  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  _so->setId(newdlg.exec());
}

void dspSalesOrderStatus::sFillList(int pSoheadid)
{
  if (pSoheadid != -1)
  {
    q.prepare( "SELECT cohead_number,"
               "       cohead_orderdate,"
               "       cohead_custponumber,"
               "       cust_name, cust_phone "
               "FROM cohead, cust "
               "WHERE ( (cohead_cust_id=cust_id)"
               " AND (cohead_id=:sohead_id) );" );
    q.bindValue(":sohead_id", pSoheadid);
    q.exec();
    if (q.first())
    {
      _orderDate->setDate(q.value("cohead_orderdate").toDate());
      _poNumber->setText(q.value("cohead_custponumber").toString());
      _custName->setText(q.value("cust_name").toString());
      _custPhone->setText(q.value("cust_phone").toString());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT MAX(lastupdated) AS lastupdated "
               "  FROM (SELECT cohead_lastupdated AS lastupdated "
               "          FROM cohead "
               "         WHERE (cohead_id=:sohead_id) "
               "         UNION "
               "        SELECT coitem_lastupdated AS lastupdated "
               "          FROM coitem "
               "         WHERE (coitem_cohead_id=:sohead_id) ) AS data; " );
    q.bindValue(":sohead_id", pSoheadid);
    q.exec();
    if (q.first())
      _lastUpdated->setText(formatDate(q.value("lastupdated").toDate()));
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT coitem_id, coitem_linenumber, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2),"
               "       warehous_code,"
               "       formatQty(coitem_qtyord),"
               "       formatQty(coitem_qtyshipped),"
               "       formatQty(coitem_qtyreturned),"
	       "       formatQty(SUM(COALESCE(cobill_qty, 0))),"
               "       CASE WHEN (coitem_status='C') THEN ( formatDate(coitem_closedate) || ' (' || coitem_close_username || ')' )"
               "            ELSE formatQty(noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned))"
               "       END,"
               "       CASE WHEN (coitem_order_id=-1) THEN ''"
               "            WHEN (coitem_order_type='W') THEN ( SELECT (formatWoNumber(wo_id) || '/' || wo_status)"
               "                                                FROM wo"
               "                                                WHERE (wo_id=coitem_order_id) )"
               "            ELSE ''"
               "       END "
               "FROM itemsite, item, warehous, coitem LEFT OUTER JOIN "
	       "     cobill ON (coitem_id=cobill_coitem_id AND "
	       "                cobill_invcitem_id IS NOT NULL) "
               "WHERE ( (coitem_itemsite_id=itemsite_id)"
               " AND (coitem_status <> 'X')"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (coitem_cohead_id=:sohead_id) ) "
	       "GROUP BY coitem_id, coitem_linenumber, item_number, "
	       "         item_descrip1, item_descrip2, warehous_code, "
	       "         coitem_qtyord, coitem_qtyshipped, coitem_status, "
	       "         coitem_closedate, coitem_close_username, "
	       "         coitem_qtyreturned, coitem_order_id, "
	       "         coitem_order_type "
               "ORDER BY coitem_linenumber;" );
    q.bindValue(":sohead_id", pSoheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _soitem->populate(q);
  }
  else
  {
    _orderDate->clear();
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
    _soitem->clear();
  }
}
