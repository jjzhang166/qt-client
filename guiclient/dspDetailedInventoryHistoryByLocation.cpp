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

#include "dspDetailedInventoryHistoryByLocation.h"

#include <QMessageBox>
#include <QMenu>

#include <openreports.h>
#include <parameter.h>

#include "adjustmentTrans.h"
#include "transferTrans.h"
#include "scrapTrans.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "countTag.h"

/*
 *  Constructs a dspDetailedInventoryHistoryByLocation as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspDetailedInventoryHistoryByLocation::dspDetailedInventoryHistoryByLocation(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_invhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_location, SIGNAL(newID(int)), this, SLOT(sPopulateLocationInfo(int)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sPopulateLocations()));

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  _transType->append(cTransTransfers, tr("Transfers")              );
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentItem(0);

  _invhist->addColumn(tr("Date"),         (_dateColumn + 30),  Qt::AlignRight  );
  _invhist->addColumn(tr("Type"),         _transColumn,        Qt::AlignCenter );
  _invhist->addColumn(tr("Order #"),      _orderColumn,        Qt::AlignLeft   );
  _invhist->addColumn(tr("Item Number"),  _itemColumn,         Qt::AlignLeft   );
  _invhist->addColumn(tr("Lot/Serial #"), -1,                  Qt::AlignLeft   );
  _invhist->addColumn(tr("UOM"),          _uomColumn,          Qt::AlignCenter );
  _invhist->addColumn(tr("Trans-Qty"),    _qtyColumn,          Qt::AlignRight  );
  _invhist->addColumn(tr("Qty. Before"),  _qtyColumn,          Qt::AlignRight  );
  _invhist->addColumn(tr("Qty. After"),   _qtyColumn,          Qt::AlignRight  );

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   TRUE);
  
  sPopulateLocations();
}

dspDetailedInventoryHistoryByLocation::~dspDetailedInventoryHistoryByLocation()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspDetailedInventoryHistoryByLocation::languageChange()
{
  retranslateUi(this);
}

void dspDetailedInventoryHistoryByLocation::sPopulateLocations()
{
  if (_warehouse->isAll())
    _location->populate( "SELECT location_id,"
                         "       CASE WHEN (LENGTH(location_descrip) > 0) THEN (warehous_code || '-' || formatLocationName(location_id) || '-' || location_descrip)"
                         "            ELSE (warehous_code || '-' || formatLocationName(location_id))"
                         "       END AS locationname "
                         "FROM location, warehous "
                         "WHERE (location_warehous_id=warehous_id) "
                         "ORDER BY locationname;" );
  else
  {
    q.prepare( "SELECT location_id, "
               "       CASE WHEN (LENGTH(location_descrip) > 0) THEN (formatLocationName(location_id) || '-' || location_descrip)"
               "            ELSE formatLocationName(location_id)"
               "       END AS locationname "
               "FROM location "
               "WHERE (location_warehous_id=:warehous_id) "
               "ORDER BY locationname;" );
    _warehouse->bindValue(q);
    q.exec();
    _location->populate(q);
  }
}

void dspDetailedInventoryHistoryByLocation::sPopulateLocationInfo(int pLocationid)
{
  q.prepare( "SELECT formatBoolYN(location_netable) AS netable,"
             "       formatBoolYN(location_restrict) AS restricted "
             "FROM location, warehous "
             "WHERE ( (location_warehous_id=warehous_id)"
             " AND (location_id=:location_id) );" );
  q.bindValue(":location_id", pLocationid);
  q.exec();
  if (q.first())
  {
    _netable->setText(q.value("netable").toString());
    _restricted->setText(q.value("restricted").toString());
  }
}

void dspDetailedInventoryHistoryByLocation::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("location_id", _location->id());
  params.append("transType", _transType->id());

  orReport report("DetailedInventoryHistoryByLocation", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspDetailedInventoryHistoryByLocation::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(1));

  ParameterList params;
  params.append("mode", "view");
  params.append("invhist_id", _invhist->id());

  if (transType == "AD")
  {
    adjustmentTrans *newdlg = new adjustmentTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "TW")
  {
    transferTrans *newdlg = new transferTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "SI")
  {
    scrapTrans *newdlg = new scrapTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "EX")
  {
    expenseTrans *newdlg = new expenseTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "RX")
  {
    materialReceiptTrans *newdlg = new materialReceiptTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "CC")
  {
    countTag newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspDetailedInventoryHistoryByLocation::sPopulateMenu(QMenu *menuThis)
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(1));

  if ( (transType == "AD") ||
       (transType == "TW") ||
       (transType == "SI") ||
       (transType == "EX") ||
       (transType == "RX") ||
       (transType == "CC") )
    menuThis->insertItem(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()), 0);
}

void dspDetailedInventoryHistoryByLocation::sFillList()
{
  _invhist->clear();

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  q.prepare( "SELECT invhist_id,"
             "       formatDateTime(invhist_transdate) AS transdate,"
             "       invhist_transtype, (invhist_ordtype || '-' || invhist_ordnumber) AS ordernumber,"
             "       invhist_invuom,"
             "       item_number, formatlotserialnumber(invdetail_ls_id) AS lotserial,"
             "       formatQty(invdetail_qty) AS transqty,"
             "       formatQty(invdetail_qty_before) AS qohbefore,"
             "       formatQty(invdetail_qty_after) AS qohafter,"
             "       invhist_posted "
             "FROM invdetail, invhist, itemsite, item "
             "WHERE ( (invdetail_invhist_id=invhist_id)"
             " AND (invhist_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (invdetail_location_id=:location_id)"
             " AND (DATE(invhist_transdate) BETWEEN :startDate AND :endDate)"
             " AND (transType(invhist_transtype, :transType)) ) "
             "ORDER BY invhist_transdate DESC, invhist_transtype;" );
  _dates->bindValue(q);
  q.bindValue(":location_id", _location->id());
  q.bindValue(":transType", _transType->id());
  q.exec();

  XTreeWidgetItem *last = 0;
  while (q.next())
  {
    last = new XTreeWidgetItem( _invhist, last, q.value("invhist_id").toInt(),
			       q.value("transdate"),
			       q.value("invhist_transtype"),
			       q.value("ordernumber"),
			       q.value("item_number"),
			       q.value("lotserial"),
			       q.value("invhist_invuom"),
			       q.value("transqty") );

    if (q.value("invhist_posted").toBool())
    {
      last->setText(7, q.value("qohbefore").toString());
      last->setText(8, q.value("qohafter").toString());
    }
    else
      last->setTextColor("orange");
  }
}

