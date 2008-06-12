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

#include "dspBacklogBySalesOrder.h"

#include <QVariant>
#include <QWorkspace>
#include <QStatusBar>
#include <QMenu>
#include <parameter.h>
#include <openreports.h>
#include "inputManager.h"
#include "salesOrderList.h"
#include "dspRunningAvailability.h"

/*
 *  Constructs a dspBacklogBySalesOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspBacklogBySalesOrder::dspBacklogBySalesOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_salesOrder, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_salesOrderList, SIGNAL(clicked()), this, SLOT(sSalesOrderList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_soitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_salesOrder, SIGNAL(requestList()), this, SLOT(sSalesOrderList()));

#ifndef Q_WS_MAC
  _salesOrderList->setMaximumWidth(25);
#endif

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _salesOrder, SLOT(setId(int)));

  _soitem->addColumn(tr("#"),           _seqColumn,  Qt::AlignCenter );
  _soitem->addColumn(tr("Item"),        _itemColumn, Qt::AlignLeft   );
  _soitem->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _soitem->addColumn(tr("Whs."),        _whsColumn,  Qt::AlignCenter );
  _soitem->addColumn(tr("Ordered"),     _qtyColumn,  Qt::AlignRight  );
  _soitem->addColumn(tr("Shipped"),     _qtyColumn,  Qt::AlignRight  );
  _soitem->addColumn(tr("Balance"),     _qtyColumn,  Qt::AlignRight  );
  _soitem->addColumn(tr("At Shipping"), _qtyColumn,  Qt::AlignRight  );
  _soitem->addColumn(tr("Available"),   _qtyColumn,  Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspBacklogBySalesOrder::~dspBacklogBySalesOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspBacklogBySalesOrder::languageChange()
{
  retranslateUi(this);
}

void dspBacklogBySalesOrder::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sRunningAvailability()), 0);
}

void dspBacklogBySalesOrder::sPrint()
{
  ParameterList params;
  params.append("sohead_id", _salesOrder->id());

  orReport report("BacklogBySalesOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBacklogBySalesOrder::sSalesOrderList()
{
  ParameterList params;
  params.append("sohead_id", _salesOrder->id());
  params.append("soType", cSoOpen);

  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  _salesOrder->setId(newdlg.exec());
}

void dspBacklogBySalesOrder::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _soitem->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspBacklogBySalesOrder::sFillList()
{
  _soitem->clear();

  if (_salesOrder->isValid())
  {
    q.prepare( "SELECT cohead_number,"
               "       formatDate(cohead_orderdate) AS orderdate,"
               "       cohead_custponumber,"
               "       cust_name, cust_phone "
               "FROM cohead, cust "
               "WHERE ( (cohead_cust_id=cust_id)"
               " AND (cohead_id=:sohead_id) );" );
    q.bindValue(":sohead_id", _salesOrder->id());
    q.exec();
    if (q.first())
    {
      _orderDate->setText(q.value("orderdate").toString());
      _poNumber->setText(q.value("cohead_custponumber").toString());
      _custName->setText(q.value("cust_name").toString());
      _custPhone->setText(q.value("cust_phone").toString());
    }

    q.prepare( "SELECT coitem_id, itemsite_id,"
               "       coitem_linenumber, item_number, f_description, warehous_code,"
               "       formatQty(coitem_qtyord) AS f_ordered,"
               "       formatQty(coitem_qtyshipped) AS f_shipped,"
               "       formatQty(noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned)) AS f_balance,"
               "       formatQty(atshipping) AS f_atshipping,"
               "       formatQty(available) AS f_available,"
               "       (available <= reorderlevel) AS reorder,"
               "       (available < 0) AS stockout "
               "FROM ( SELECT coitem_id, itemsite_id, coitem_linenumber, item_number,"
               "              (item_descrip1 || ' ' || item_descrip2) AS f_description, warehous_code,"
               "              coitem_qtyord, coitem_qtyshipped, coitem_qtyreturned,"
               "              SUM(coship_qty) AS atshipping,"
               "              CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
               "              qtyAvailable(itemsite_id, coitem_scheddate) AS available "
               "       FROM itemsite, item, warehous,"
               "            coitem LEFT OUTER JOIN"
               "            ( coship JOIN"
               "              cosmisc ON ( (coship_cosmisc_id=cosmisc_id) AND (NOT cosmisc_shipped) )"
               "            ) ON (coship_coitem_id=coitem_id) "
               "       WHERE ((coitem_itemsite_id=itemsite_id)"
               "        AND (coitem_status <> 'X')"
               "        AND (itemsite_item_id=item_id)"
               "        AND (itemsite_warehous_id=warehous_id)"
               "        AND (coitem_cohead_id=:sohead_id)) "
               "       GROUP BY coitem_id, itemsite_id, coitem_linenumber, item_number,"
               "                item_descrip1, item_descrip2, warehous_code,"
               "                coitem_qtyord, coitem_qtyshipped, coitem_qtyreturned,"
               "                reorderlevel, coitem_scheddate "
               "       ORDER BY coitem_linenumber ) AS data;" );
    q.bindValue(":sohead_id", _salesOrder->id());
    q.exec();
    XTreeWidgetItem *last = 0;
    while (q.next())
    {
      last = new XTreeWidgetItem(_soitem, last,
				 q.value("coitem_id").toInt(), q.value("itemsite_id").toInt(),
				 q.value("coitem_linenumber"), q.value("item_number"),
				 q.value("f_description"), q.value("warehous_code"),
				 q.value("f_ordered"), q.value("f_shipped"),
				 q.value("f_balance"), q.value("f_atshipping"),
				 q.value("f_available") );

      if (q.value("stockout").toBool())
        last->setTextColor(8, "red");
      else if (q.value("reorder").toBool())
        last->setTextColor(8, "orange");
    }
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

