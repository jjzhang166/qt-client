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

#include "itemAvailabilityWorkbench.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>
#include <openreports.h>
#include "rptRunningAvailability.h"
#include "rptInventoryAvailabilityByItem.h"
#include "rptCostedIndentedBOM.h"
#include "rptInventoryHistoryByItem.h"
#include "rptInventoryLocator.h"
#include "rptSingleLevelWhereUsed.h"
#include "item.h"
#include "bom.h"
#include "boo.h"
#include "adjustmentTrans.h"
#include "transferTrans.h"
#include "scrapTrans.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "dspItemCostSummary.h"
#include "maintainItemCosts.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "workOrder.h"
#include "purchaseOrder.h"
#include "purchaseRequest.h"
#include "dspRunningAvailability.h"
#include "dspOrders.h"
#include "dspAllocations.h"
#include "dspInventoryHistoryByItem.h"
#include "createCountTagsByItem.h"
#include "enterMiscCount.h"
#include "firmPlannedOrder.h"
#include "countTag.h"
#include "transactionInformation.h"
#include "relocateInventory.h"
#include "reassignLotSerial.h"

/*
 *  Constructs a itemAvailabilityWorkbench as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
itemAvailabilityWorkbench::itemAvailabilityWorkbench(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
  connect(_item, SIGNAL(newId(int)), _invhistWarehouse, SLOT(findItemSites(int)));
  connect(_item, SIGNAL(newId(int)), _invWarehouse, SLOT(findItemSites(int)));
  connect(_item, SIGNAL(newId(int)), _itemlocWarehouse, SLOT(findItemSites(int)));
  connect(_item, SIGNAL(valid(bool)), _availPrint, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _costedPrint, SLOT(setEnabled(bool)));
  connect(_byDate, SIGNAL(toggled(bool)), _date, SLOT(setEnabled(bool)));
  connect(_byDays, SIGNAL(toggled(bool)), _days, SLOT(setEnabled(bool)));
  connect(_byDates, SIGNAL(toggled(bool)), _endDate, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _histPrint, SLOT(setEnabled(bool)));
  connect(_showReorder, SIGNAL(toggled(bool)), _ignoreReorderAtZero, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _invhistQuery, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _invQuery, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _itemlocQuery, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _locPrint, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _runPrint, SLOT(setEnabled(bool)));
  connect(_byDates, SIGNAL(toggled(bool)), _startDate, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _wherePrint, SLOT(setEnabled(bool)));
  connect(_invQuery, SIGNAL(clicked()), this, SLOT(sFillListAvail()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillListCosted()));
  connect(_costsGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sFillListCosted()));
  connect(_costsGroup, SIGNAL(toggled(bool)), this, SLOT(sFillListCosted()));
  connect(_invhistQuery, SIGNAL(clicked()), this, SLOT(sFillListInvhist()));
  connect(_itemlocQuery, SIGNAL(clicked()), this, SLOT(sFillListItemloc()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sFillListRunning()));
  connect(_showPlanned, SIGNAL(toggled(bool)), this, SLOT(sFillListRunning()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillListWhereUsed()));
  connect(_effective, SIGNAL(newDate(const QDate&)), this, SLOT(sFillListWhereUsed()));
  connect(_showReorder, SIGNAL(toggled(bool)), this, SLOT(sHandleShowReorder(bool)));
  connect(_invAvailability, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuAvail(QMenu*,QTreeWidgetItem*)));
  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuCosted(QMenu*,QTreeWidgetItem*)));
  connect(_invhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuHistory(QMenu*,QTreeWidgetItem*)));
  connect(_itemloc, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuLocation(QMenu*,QTreeWidgetItem*)));
  connect(_availability, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuRunning(QMenu*,QTreeWidgetItem*)));
  connect(_whereused, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuWhereUsed(QMenu*,QTreeWidgetItem*)));
  connect(_availPrint, SIGNAL(clicked()), this, SLOT(sPrintAvail()));
  connect(_costedPrint, SIGNAL(clicked()), this, SLOT(sPrintCosted()));
  connect(_histPrint, SIGNAL(clicked()), this, SLOT(sPrintHistory()));
  connect(_locPrint, SIGNAL(clicked()), this, SLOT(sPrintLocation()));
  connect(_runPrint, SIGNAL(clicked()), this, SLOT(sPrintRunning()));
  connect(_wherePrint, SIGNAL(clicked()), this, SLOT(sPrintWhereUsed()));

  //if (_metrics->boolean("AllowInactiveBomItems"))
  //  _item->setType(ItemLineEdit::cGeneralComponents);
  //else
  //  _item->setType(ItemLineEdit::cGeneralComponents | ItemLineEdit::cActive);
  
  // Running Availability
  _availability->addColumn(tr("Order Type/#"),       _itemColumn, Qt::AlignLeft  );
  _availability->addColumn(tr("Source/Destination"), -1,          Qt::AlignLeft  );
  _availability->addColumn(tr("Due Date"),           _dateColumn, Qt::AlignLeft  );
  _availability->addColumn(tr("Ordered"),            _qtyColumn,  Qt::AlignRight );
  _availability->addColumn(tr("Received"),           _qtyColumn,  Qt::AlignRight );
  _availability->addColumn(tr("Balance"),            _qtyColumn,  Qt::AlignRight );
  _availability->addColumn(tr("Running Avail."),     _qtyColumn,  Qt::AlignRight );

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillListRunning()));
  
  // Inventory Availability
  _invAvailability->addColumn(tr("Whs."),         -1,         Qt::AlignCenter );
  _invAvailability->addColumn(tr("LT"),           _whsColumn, Qt::AlignCenter );
  _invAvailability->addColumn(tr("QOH"),          _qtyColumn, Qt::AlignRight  );
  _invAvailability->addColumn(tr("Allocated"),    _qtyColumn, Qt::AlignRight  );
  _invAvailability->addColumn(tr("Unallocated"),  _qtyColumn, Qt::AlignRight  );
  _invAvailability->addColumn(tr("On Order"),     _qtyColumn, Qt::AlignRight  );
  _invAvailability->addColumn(tr("Reorder Lvl."), _qtyColumn, Qt::AlignRight  );
  _invAvailability->addColumn(tr("OUT Level"),    _qtyColumn, Qt::AlignRight  );
  _invAvailability->addColumn(tr("Available"),    _qtyColumn, Qt::AlignRight  );

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillListAvail()));
                                                                     
  // Costed BOM
  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Seq #"),         80,           Qt::AlignCenter );
  _bomitem->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignLeft   );
  _bomitem->addColumn(tr("Description"),   -1,           Qt::AlignLeft   );
  _bomitem->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Ext. Qty. Per"), _qtyColumn,   Qt::AlignRight  );
  _bomitem->addColumn(tr("Scrap %"),       _prcntColumn, Qt::AlignRight  );
  _bomitem->addColumn(tr("Effective"),     _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Expires"),       _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Unit Cost"),     _costColumn,  Qt::AlignRight  );
  _bomitem->addColumn(tr("Ext'd Cost"),    _priceColumn, Qt::AlignRight  );
  _bomitem->setIndentation(10);
  
  // Invhist
  _invhist->setRootIsDecorated(TRUE);
  _invhist->addColumn(tr("Time"),        (_dateColumn + 30),   Qt::AlignLeft   );
  _invhist->addColumn(tr("User"),        _orderColumn,         Qt::AlignCenter );
  _invhist->addColumn(tr("Type"),        _transColumn,         Qt::AlignCenter );
  _invhist->addColumn(tr("Whs."),        _whsColumn,           Qt::AlignCenter );
  _invhist->addColumn(tr("Order #/Location-Lot/Serial #"), -1, Qt::AlignLeft   );
  _invhist->addColumn(tr("UOM"),         _uomColumn,           Qt::AlignCenter );
  _invhist->addColumn(tr("Trans-Qty"),   _qtyColumn,           Qt::AlignRight  );
  _invhist->addColumn(tr("From Area"),_orderColumn,        Qt::AlignLeft   );
  _invhist->addColumn(tr("QOH Before"),  _qtyColumn,           Qt::AlignRight  );
  _invhist->addColumn(tr("To Area"), _orderColumn,         Qt::AlignLeft   );
  _invhist->addColumn(tr("QOH After"),   _qtyColumn,           Qt::AlignRight  );

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  _transType->append(cTransTransfers, tr("Transfers")              );
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentItem(0);
  
  // Itemloc
  _itemloc->addColumn(tr("Whs."),         _whsColumn,   Qt::AlignCenter );
  _itemloc->addColumn(tr("Location"),     200,          Qt::AlignLeft   );
  _itemloc->addColumn(tr("Netable"),      _orderColumn, Qt::AlignCenter );
  _itemloc->addColumn(tr("Lot/Serial #"), -1,           Qt::AlignLeft   );
  _itemloc->addColumn(tr("Expiration"),   _dateColumn,  Qt::AlignCenter );
  _itemloc->addColumn(tr("Qty."),         _qtyColumn,   Qt::AlignRight  );
  
  // Where Used
  _effective->setNullString(tr("Now"));
  _effective->setNullDate(omfgThis->startOfTime());
  _effective->setAllowNullDate(TRUE);
  _effective->setNull();
  
  _whereused->addColumn(tr("Seq #"),       40,           Qt::AlignCenter );
  _whereused->addColumn(tr("Parent Item"), _itemColumn,  Qt::AlignLeft   );
  _whereused->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _whereused->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignLeft   );
  _whereused->addColumn(tr("Qty. Per"),    _qtyColumn,   Qt::AlignRight  );
  _whereused->addColumn(tr("Scrap %"),     _prcntColumn, Qt::AlignRight  );
  _whereused->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter );
  _whereused->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter );
  
//  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillListWhereUsed(int, bool)));
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillListWhereUsed()));
  
  // General
  _item->setFocus();

  if(!_privleges->check("ViewBOMs"))
    _tab->removeTab(5);
  if(!_privleges->check("ViewQOH"))
    _tab->removeTab(4);
  if(!_privleges->check("ViewInventoryHistory"))
    _tab->removeTab(3);
  if(!_privleges->check("ViewBOMs"))
    _tab->removeTab(2);
  if(!_privleges->check("ViewInventoryAvailability"))
  {
    _tab->removeTab(1);
    _tab->removeTab(0);
  }
  if(!_privleges->check("ViewCosts"))
  {
    _costsGroup->setEnabled(false);
    _costsGroup->setChecked(false);
  }
  else
    _costsGroup->setChecked(true);
    
  //If not OpenMFG, hide show planned option
  if (_metrics->value("Application") != "OpenMFG")
    _showPlanned->hide();
    
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  
  //If not Serial Control, hide lot control
  if (!_metrics->boolean("LotSerialControl"))
    _tab->removePage(_tab->page(4));
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemAvailabilityWorkbench::~itemAvailabilityWorkbench()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemAvailabilityWorkbench::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemAvailabilityWorkbench::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  return NoError;
}

void itemAvailabilityWorkbench::sFillListWhereUsed()
{
  if ((_item->isValid()) && (_effective->isValid()))
  {
    QString sql( "SELECT bomitem_parent_item_id, item_id, bomitem_seqnumber,"
                 "       item_number, (item_descrip1 || ' ' || item_descrip2),"
                 "       item_invuom, formatQtyper(bomitem_qtyper),"
                 "       formatScrap(bomitem_scrap),"
                 "       formatDate(bomitem_effective, 'Always'),"
                 "       formatDate(bomitem_expires, 'Never') "
                 "FROM bomitem, item "
                 "WHERE ( (bomitem_parent_item_id=item_id)"
                 " AND (bomitem_item_id=:item_id)" );

    if (_effective->isNull())
      sql += "AND (CURRENT_DATE BETWEEN bomitem_effective AND (bomitem_expires-1))";
    else
      sql += " AND (:effective BETWEEN bomitem_effective AND (bomitem_expires-1))";

    sql += ") ORDER BY item_number";

    q.prepare(sql);
    q.bindValue(":item_id", _item->id());
    q.bindValue(":effective", _effective->date());
    q.exec();

    //if (pLocal)
    //  _whereused->populate(q, TRUE, pItemid);
    //else
      _whereused->populate(q, TRUE);
  }
  else
    _whereused->clear();
}

void itemAvailabilityWorkbench::sFillListItemloc()
{
    if (_item->isValid())
  {
    QString sql( "SELECT itemloc_id, 1 AS type, warehous_code,"
                 "       CASE WHEN (location_id IS NULL) THEN :na"
                 "            ELSE (formatLocationName(location_id) || '-' || firstLine(location_descrip))"
                 "       END AS locationname,"
                 "       CASE WHEN (location_id IS NULL) THEN :na"
                 "            WHEN (location_netable) THEN :yes"
                 "            ELSE :no"
                 "       END AS netable,"
                 "       CASE WHEN (itemsite_controlmethod NOT IN ('L', 'S')) THEN :na"
                 "            ELSE itemloc_lotserial"
                 "       END AS lotserial,"
                 "       CASE WHEN (itemsite_perishable) THEN formatDate(itemloc_expiration)"
                 "            ELSE :na"
                 "       END AS f_expiration,"
                 "       CASE WHEN (itemsite_perishable) THEN (itemloc_expiration <= CURRENT_DATE)"
                 "            ELSE FALSE"
                 "       END AS expired,"
                 "       formatQty(itemloc_qty) AS f_qoh "
                 "FROM itemsite, warehous,"
                 "     itemloc LEFT OUTER JOIN location ON (itemloc_location_id=location_id) "
                 "WHERE ( ( (itemsite_loccntrl) OR (itemsite_controlmethod IN ('L', 'S')) )"
                 " AND (itemloc_itemsite_id=itemsite_id)"
                 " AND (itemsite_warehous_id=warehous_id)"
                 " AND (itemsite_item_id=:item_id)" );

    if (_itemlocWarehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += ") "
           "UNION SELECT itemsite_id, 2 AS type, warehous_code,"
           "             :na AS locationname,"
           "             :na AS netable,"
           "             :na AS lotserial,"
           "             :na AS f_expiration,"
           "             FALSE  AS expired,"
           "             formatQty(itemsite_qtyonhand) AS f_qoh "
           "FROM itemsite, warehous "
           "WHERE ( (NOT itemsite_loccntrl)"
           " AND (itemsite_controlmethod NOT IN ('L', 'S'))"
           " AND (itemsite_warehous_id=warehous_id)"
           " AND (itemsite_item_id=:item_id)";

    if (_itemlocWarehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += ") "
           "ORDER BY warehous_code, locationname, lotserial;";

    q.prepare(sql);
    q.bindValue(":yes", tr("Yes"));
    q.bindValue(":no", tr("No"));
    q.bindValue(":na", tr("N/A"));
    q.bindValue(":undefined", tr("Undefined"));
    q.bindValue(":item_id", _item->id());
    _itemlocWarehouse->bindValue(q);
    q.exec();

    _itemloc->clear();
    XTreeWidgetItem *last = 0;
    while (q.next())
    {
      last = new XTreeWidgetItem( _itemloc, last,
                                               q.value("itemloc_id").toInt(), q.value("type").toInt(),
                                               q.value("warehous_code"), q.value("locationname"),
                                               q.value("netable"), q.value("lotserial"),
                                               q.value("f_expiration"), q.value("f_qoh") );
      if (q.value("expired").toBool())
        last->setTextColor("red");
    }
  }
  else
    _itemloc->clear();
}

void itemAvailabilityWorkbench::sFillListInvhist()
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

  if (_item->isValid())
  {
    QString sql( "SELECT invhist_id,"
                 "       invhist_transdate, formatDateTime(invhist_transdate) AS transdate,"
                 "       invhist_user, invhist_transtype, whs1.warehous_code AS warehous_code,"
                 "       formatQty(invhist_invqty) AS transqty,"
                 "       invhist_invuom,"
                 "       CASE WHEN (invhist_ordtype NOT LIKE '') THEN (invhist_ordtype || '-' || invhist_ordnumber)"
                 "            ELSE invhist_ordnumber"
                 "       END AS ordernumber,"
                 "       formatQty(invhist_qoh_before) AS qohbefore,"
                 "       formatQty(invhist_qoh_after) AS qohafter,"
                 "       invhist_posted,"
                 "       0 AS invdetail_id, '' AS locationname,"
                 "       '' AS detailqty,"
                 "       '' AS locationqtybefore, '' AS locationqtyafter,"
                 "       CASE WHEN (invhist_transtype='TW') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='IB') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='IM') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='IT') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='RB') THEN 'WIP'"
                 "            WHEN (invhist_transtype='RM') THEN 'WIP'"
                 "            WHEN (invhist_transtype='RP') THEN 'PURCH'"
                 "            WHEN (invhist_transtype='RS') THEN 'SHIP'"
                 "            WHEN (invhist_transtype='SH') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='SI') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='SV') THEN whs1.warehous_code"
                 "            ELSE ''"
                 "       END AS locfrom,"
                 "       CASE WHEN (invhist_transtype='TW') THEN whs2.warehous_code"
                 "            WHEN (invhist_transtype='AD') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='CC') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='IB') THEN 'WIP'"
                 "            WHEN (invhist_transtype='IM') THEN 'WIP'"
                 "            WHEN (invhist_transtype='NN') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='RB') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='RM') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='RP') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='RS') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='RT') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='RX') THEN whs1.warehous_code"
                 "            WHEN (invhist_transtype='SH') THEN 'SHIP'"
                 "            WHEN (invhist_transtype='SI') THEN 'SCRAP'"
                 "            WHEN (invhist_transtype='SV') THEN 'SHIP'"
                 "            ELSE ''"
                 "       END AS locto "
                 "FROM itemsite, item, warehous AS whs1, invhist LEFT OUTER JOIN warehous AS whs2 ON (invhist_xfer_warehous_id=whs2.warehous_id) "
                 "WHERE ( (NOT invhist_hasdetail)"
                 " AND (invhist_itemsite_id=itemsite_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (itemsite_warehous_id=whs1.warehous_id)"
                 " AND (itemsite_item_id=:item_id)"
                 " AND (DATE(invhist_transdate) BETWEEN :startDate AND :endDate)"
                 " AND (transType(invhist_transtype, :transactionType))" );

    if (_invhistWarehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += " ) "
           "UNION SELECT invhist_id,"
           "             invhist_transdate, formatDateTime(invhist_transdate) AS transdate,"
           "             invhist_user, invhist_transtype, whs1.warehous_code AS warehous_code,"
           "             formatQty(invhist_invqty) AS transqty,"
           "             invhist_invuom,"
           "             CASE WHEN (invhist_ordtype NOT LIKE '') THEN (invhist_ordtype || '-' || invhist_ordnumber)"
           "                  ELSE invhist_ordnumber"
           "             END AS ordernumber,"
           "             formatQty(invhist_qoh_before) AS qohbefore,"
           "             formatQty(invhist_qoh_after) AS qohafter,"
           "             invhist_posted,"
           "             invdetail_id,"
           "             CASE WHEN (invdetail_location_id=-1) THEN invdetail_lotserial"
           "                  WHEN (invdetail_lotserial IS NULL) THEN formatLocationName(invdetail_location_id)"
           "                  ELSE (formatLocationName(invdetail_location_id) || '-' || invdetail_lotserial)"
           "             END AS locationname,"
           "             formatQty(invdetail_qty) AS detailqty,"
           "             formatQty(invdetail_qty_before) AS locationqtybefore,"
           "             formatQty(invdetail_qty_after) AS locationqtyafter,"
           "             CASE WHEN (invhist_transtype='TW') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='IB') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='IM') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='IT') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='RB') THEN 'WIP'"
           "                  WHEN (invhist_transtype='RM') THEN 'WIP'"
           "                  WHEN (invhist_transtype='RP') THEN 'PURCH'"
           "                  WHEN (invhist_transtype='RS') THEN 'SHIP'"
           "                  WHEN (invhist_transtype='SH') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='SI') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='SV') THEN whs1.warehous_code"
           "                  ELSE ''"
           "             END AS locfrom,"
           "             CASE WHEN (invhist_transtype='TW') THEN whs2.warehous_code"
           "                  WHEN (invhist_transtype='AD') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='CC') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='IB') THEN 'WIP'"
           "                  WHEN (invhist_transtype='IM') THEN 'WIP'"
           "                  WHEN (invhist_transtype='NN') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='RB') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='RM') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='RP') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='RS') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='RT') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='RX') THEN whs1.warehous_code"
           "                  WHEN (invhist_transtype='SH') THEN 'SHIP'"
           "                  WHEN (invhist_transtype='SI') THEN 'SCRAP'"
           "                  WHEN (invhist_transtype='SV') THEN 'SHIP'"
           "                  ELSE ''"
           "             END AS locto "
           "FROM itemsite, item, warehous AS whs1, invdetail, invhist LEFT OUTER JOIN warehous AS whs2 ON (invhist_xfer_warehous_id=whs2.warehous_id) "
           "WHERE ( (invhist_hasdetail)"
           " AND (invhist_itemsite_id=itemsite_id)"
           " AND (itemsite_item_id=item_id)"
           " AND (itemsite_warehous_id=whs1.warehous_id)"
           " AND (invdetail_invhist_id=invhist_id)"
           " AND (itemsite_item_id=:item_id)"
           " AND (DATE(invhist_transdate) BETWEEN :startDate AND :endDate)"
           " AND (transType(invhist_transtype, :transactionType))";

    if (_invhistWarehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += ") "
           "ORDER BY invhist_transdate DESC, invhist_transtype, locationname;";

    q.prepare(sql);
    _dates->bindValue(q);
    q.bindValue(":item_id", _item->id());
    q.bindValue(":transactionType", _transType->id());
    _invhistWarehouse->bindValue(q);
    q.exec();
    if (q.first())
    {
      XTreeWidgetItem *parentItem = NULL;
      int           invhistid   = 0;

      do
      {
        if (q.value("invhist_id").toInt() != invhistid)
        {
          invhistid = q.value("invhist_id").toInt();

          parentItem = new XTreeWidgetItem( _invhist, parentItem,
                                          q.value("invhist_id").toInt(), q.value("invdetail_id").toInt(),
                                          q.value("transdate"), q.value("invhist_user"),
                                          q.value("invhist_transtype"), q.value("warehous_code"),
                                          q.value("ordernumber"), q.value("invhist_invuom"),
                                          q.value("transqty") );

          if (q.value("invhist_posted").toBool())
          {
            parentItem->setText(7, q.value("locfrom").toString());
            parentItem->setText(8, q.value("qohbefore").toString());
            parentItem->setText(9, q.value("locto").toString());
            parentItem->setText(10, q.value("qohafter").toString());
          }
          else
            parentItem->setTextColor("orange");
        }

        if (q.value("invdetail_id").toInt())
        {
          XTreeWidgetItem *child = new XTreeWidgetItem( parentItem, q.value("invhist_id").toInt(), q.value("invdetail_id").toInt(),
                                                    "", "", "", "",
                                                    q.value("locationname"), "",
                                                    q.value("detailqty") );

          if (q.value("invhist_posted").toBool())
          {
            child->setText(8, q.value("locationqtybefore").toString());
            child->setText(10, q.value("locationqtyafter").toString());
          }
          else
            child->setTextColor("orange");
        }
      }
      while (q.next());
    }
  }
}

void itemAvailabilityWorkbench::sFillListCosted()
{
  double totalCosts = 0;

  _bomitem->clear();

  if (_item->isValid() && (_item->itemType() == "M" || _item->itemType() == "B" || _item->itemType() == "F") )
  {
    q.prepare("SELECT indentedBOM(:item_id) AS bomwork_set_id;");
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
    {
      int _worksetid = q.value("bomwork_set_id").toInt();

      QString sql( "SELECT bomwork_id, item_id, bomwork_parent_id,"
                   "       bomwork_seqnumber, item_number, item_invuom,"
                   "       (item_descrip1 || ' ' || item_descrip2) AS itemdescription,"
                   "       formatQtyPer(bomwork_qtyper) AS qtyper,"
                   "       formatScrap(bomwork_scrap) AS scrap,"
                   "       formatDate(bomwork_effective, 'Always') AS effective,"
                   "       formatDate(bomwork_expires, 'Never') AS expires," );

      if(_costsGroup->isChecked())
      {
        if (_useStandardCosts->isChecked())
          sql += " formatCost(bomwork_stdunitcost) AS f_unitcost,"
                 " formatCost(bomwork_qtyper * (1 + bomwork_scrap) * bomwork_stdunitcost) AS f_extendedcost,"
                 " (bomwork_qtyper * (1 + bomwork_scrap) * bomwork_stdunitcost) AS extendedcost,";
        else if (_useActualCosts->isChecked())
          sql += " formatCost(bomwork_actunitcost) AS f_unitcost,"
                 " formatCost(bomwork_qtyper * (1 + bomwork_scrap) * bomwork_actunitcost) AS f_extendedcost,"
                 " (bomwork_qtyper * (1 + bomwork_scrap) * bomwork_actunitcost) AS extendedcost,";
      }
      else
      {
          sql += " '' AS f_unitcost,"
                 " '' AS f_extendedcost,"
                 " 0 AS extendedcost,";
      }

      sql += " bomwork_level "
             "FROM bomwork, item "
             "WHERE ((bomwork_item_id=item_id)"
             " AND (bomwork_set_id=:bomwork_set_id)"
             " AND (CURRENT_DATE BETWEEN bomwork_effective AND (bomwork_expires - 1))) "

             "UNION SELECT -1 AS bomwork_id, -1 AS item_id, -1 AS bomwork_parent_id,"
             "             99999 AS bomwork_seqnumber, costelem_type AS item_number, '' AS item_invuom,"
             "             '' AS itemdescription,"
             "             '' AS qtyper, '' AS scrap, '' AS effective, '' AS expires,";

      if(_costsGroup->isChecked())
      {
        if (_useStandardCosts->isChecked())
          sql += " formatCost(itemcost_stdcost) AS f_unitcost,"
                 " formatCost(itemcost_stdcost) AS f_extendedcost,"
                 " itemcost_stdcost AS extendedcost,";
        else if (_useActualCosts->isChecked())
          sql += " formatCost(currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE)) AS f_unitcost,"
                 " formatCost(currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE)) AS f_extendedcost,"
                 " currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE) AS extendedcost,";
      }
      else
      {
          sql += " '' AS f_unitcost,"
                 " '' AS f_extendedcost,"
                 " 0 AS extendedcost,";
      }

      sql += " -1 AS bomwork_level "
             "FROM itemcost, costelem "
             "WHERE ( (itemcost_costelem_id=costelem_id)"
             " AND (NOT itemcost_lowlevel)"
             " AND (itemcost_item_id=:item_id) ) "

             "ORDER BY bomwork_level, bomwork_seqnumber, item_number;";

      q.prepare(sql);
      q.bindValue(":bomwork_set_id", _worksetid);
      q.bindValue(":item_id", _item->id());
      q.exec();
      XTreeWidgetItem *last = NULL;

      while (q.next())
      {
        if (q.value("bomwork_parent_id").toInt() == -1)
        {
          if (q.value("bomwork_id").toInt() == -1)
          {
            last = new XTreeWidgetItem( _bomitem, last, -1, -1,
                                      "", q.value("item_number"),
                                      "", "", "", "", "" );
            last->setText(7, "");
            last->setText(8, q.value("f_unitcost").toString());
            last->setText(9, q.value("f_extendedcost").toString());
          }
          else
          {
            last = new XTreeWidgetItem( _bomitem, last, q.value("bomwork_id").toInt(), q.value("item_id").toInt(),
                                      q.value("bomwork_seqnumber"), q.value("item_number"),
                                      q.value("itemdescription"), q.value("item_invuom"),
                                      q.value("qtyper"), q.value("scrap"),
                                      q.value("effective"), q.value("expires"),
                                      q.value("f_unitcost"), q.value("f_extendedcost") );
          }

          totalCosts += q.value("extendedcost").toDouble();
        }
        else
        {
          XTreeWidgetItem *cursor = _bomitem->topLevelItem(0);
          if (cursor)
          {
            do
            {
              if (cursor->id() == q.value("bomwork_parent_id").toInt())
              {
                XTreeWidgetItem *sibling = NULL;
                XTreeWidgetItem *child;

                sibling = cursor->child(cursor->childCount() - 1);

                child = new XTreeWidgetItem( cursor, sibling, q.value("bomwork_id").toInt(), q.value("item_id").toInt(),
                                           q.value("bomwork_seqnumber"), q.value("item_number"),
                                           q.value("itemdescription"), q.value("item_invuom"),
                                           q.value("qtyper"), q.value("scrap"),
                                           q.value("effective"), q.value("expires"),
                                           q.value("f_unitcost"), q.value("f_extendedcost") );
                cursor->setExpanded(TRUE);

                break;
              }
            }
            while ((cursor = _bomitem->topLevelItem(_bomitem->indexOfTopLevelItem(cursor)+1)) != NULL);
          }
        }
      }
 
      if(_costsGroup->isChecked())
      {
        last = new XTreeWidgetItem(_bomitem, last, -1, -1);
        last->setText(1, tr("Total Cost"));
        last->setText(9, formatCost(totalCosts));

        q.prepare( "SELECT formatCost(actcost(:item_id)) AS actual,"
                   "       formatCost(stdcost(:item_id)) AS standard;" );
        q.bindValue(":item_id", _item->id());
        q.exec();
        if (q.first())
        {
          last = new XTreeWidgetItem(_bomitem, last, -1, -1);
          last->setText(1, tr("Actual Cost"));
          last->setText(9, q.value("actual").toString());
  
          last = new XTreeWidgetItem( _bomitem, last, -1, -1);
          last->setText(1, tr("Standard Cost"));
          last->setText(9, q.value("standard").toString());
        }
      }

      q.prepare( "DELETE FROM bomwork "
                 "WHERE (bomwork_set_id=:bomwork_set_id);" );
      q.bindValue(":bomwork_set_id", _worksetid);
      q.exec();
    }
  }
}


void itemAvailabilityWorkbench::sFillListRunning()
{
  _availability->clear();

  if (_item->isValid())
  {
    q.prepare( "SELECT item_type, item_sold,"
               "       itemsite_id, itemsite_qtyonhand,"
               "       CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
               "       itemsite_qtyonhand, formatQty(itemsite_qtyonhand) AS f_qoh,"
               "       formatQty(CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END) AS f_reorderlevel,"
               "       formatQty(CASE WHEN(itemsite_useparams) THEN itemsite_ordertoqty ELSE 0.0 END) AS f_ordertoqty,"
               "       formatQty(CASE WHEN(itemsite_useparams) THEN itemsite_multordqty ELSE 0.0 END) AS f_multorderqty "
               "FROM item, itemsite "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=:warehous_id)"
               " AND (item_id=:item_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.exec();
    if (q.first())
    {
      _qoh->setText(q.value("f_qoh").toString());
      _reorderLevel->setText(q.value("f_reorderlevel").toString());
      _orderMultiple->setText(q.value("f_multorderqty").toString());
      _orderToQty->setText(q.value("f_ordertoqty").toString());

      bool    primed              = FALSE;
      int     itemsiteid          = q.value("itemsite_id").toInt();
      double  reorderLevel        = q.value("reorderlevel").toDouble();
      double  runningAvailability = q.value("itemsite_qtyonhand").toDouble();
      QString itemType            = q.value("item_type").toString();
      bool    itemSold            = q.value("item_sold").toBool();
      QString sql;

      if (itemType == "M")
      {
        primed = TRUE;
        sql = "SELECT wo_id AS orderid, -1 AS altorderid,"
              "       ('W/O-' || formatWoNumber(wo_id)) AS ordernumber,"
              "       1 AS sequence,"
              "       '' AS item_number,"
              "       formatDate(wo_duedate) AS duedate,"
              "       wo_duedate AS r_duedate,"
              "       (wo_duedate < CURRENT_DATE) AS late,"
              "       formatQty(wo_qtyord) AS f_qtyordered,"
              "       formatQty(wo_qtyrcv) AS f_qtyreceived,"
              "       formatQty(noNeg(wo_qtyord - wo_qtyrcv)) AS f_balance,"
              "       noNeg(wo_qtyord - wo_qtyrcv) AS balance "
              "FROM wo "
              "WHERE ( (wo_status<>'C')"
              " AND (wo_itemsite_id=:itemsite_id) ) ";
      }

      if ( (itemType == "C") || (itemType == "Y") )
      {
        if (primed)
          sql += "UNION ";
        else
          primed = TRUE;
       
        sql = "SELECT wo_id AS orderid, -1 AS altorderid,"
              "       ('W/O-' || formatWoNumber(wo_id)) AS ordernumber,"
              "       1 AS sequence,"
              "       item_number,"
              "       formatDate(wo_duedate) AS duedate,"
              "       wo_duedate AS r_duedate,"
              "       (wo_duedate < CURRENT_DATE) AS late,"
              "       formatQty(wo_qtyord * brddist_stdqtyper) AS f_qtyordered,"
              "       formatQty(wo_qtyrcv * brddist_stdqtyper) AS f_qtyreceived,"
              "       formatQty(noNeg((wo_qtyord - wo_qtyrcv) * brddist_stdqtyper)) AS f_balance,"
              "       noNeg((wo_qtyord - wo_qtyrcv) * brddist_stdqtyper) AS balance "
              "FROM brddist, wo, itemsite, item "
              "WHERE ( (wo_status<>'C')"
              " AND (brddist_wo_id=wo_id)"
              " AND (wo_itemsite_id=itemsite_id) ) "
              " AND (itemsite_item_id=item_id)"
              " AND (brddist_itemsite_id=:itemsite_id)";
      }

      if ( (itemType == "P") || (itemType == "M") || (itemType == "C") || (itemType == "O") )
      {
        if (primed)
          sql += "UNION ";
        else
          primed = TRUE;
       
        sql += "SELECT wo_id AS orderid, womatl_id AS altorderid,"
               "       ('W/O-' || formatWoNumber(wo_id)) AS ordernumber,"
               "       2 AS sequence,"
               "       item_number,"
               "       formatDate(womatl_duedate) AS duedate,"
               "       womatl_duedate AS r_duedate,"
               "       (FALSE) AS late,"
               "       formatQty(womatl_qtyreq) AS f_qtyordered,"
               "       formatQty(womatl_qtyiss) AS f_qtyreceived,"
               "       formatQty((noNeg(womatl_qtyreq - womatl_qtyiss) * -1)) AS f_balance,"
               "       (noNeg(womatl_qtyreq - womatl_qtyiss) * -1) AS balance "
               "FROM womatl, wo, itemsite, item "
               "WHERE ( (wo_status<>'C')"
               " AND (wo_itemsite_id=itemsite_id)"
               " AND (womatl_wo_id=wo_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (womatl_itemsite_id=:itemsite_id) ) ";
      }

      if ( (itemType == "P") || (itemType == "O") )
      {
        if (primed)
          sql += "UNION ";
        else
          primed = TRUE;
       

        sql += "SELECT pohead_id AS orderid, poitem_id AS altorderid,"
               "       ('P/O-' || pohead_number) AS ordernumber,"
               "       1 AS sequence,"
               "       '' AS item_number,"
               "       formatDate(poitem_duedate) AS duedate,"
               "       poitem_duedate AS r_duedate,"
               "       (poitem_duedate < CURRENT_DATE) AS late,"
               "       formatQty(poitem_qty_ordered * poitem_invvenduomratio) AS f_qtyordered,"
               "       formatQty(poitem_qty_received * poitem_invvenduomratio) AS f_qtyreceived,"
               "       formatQty(noNeg(poitem_qty_ordered - poitem_qty_received) * poitem_invvenduomratio) AS f_balance,"
               "       (noNeg(poitem_qty_ordered - poitem_qty_received) * poitem_invvenduomratio) AS balance "
               "FROM pohead, poitem "
               "WHERE ( (poitem_pohead_id=pohead_id)"
               " AND (poitem_status <> 'C')"
               " AND (poitem_itemsite_id=:itemsite_id) ) ";
      }

      if (itemSold)
      {
        if (primed)
          sql += "UNION ";
        else
          primed = TRUE;
       

        sql += "SELECT cohead_id AS orderid, coitem_id AS altorderid,"
               "       ('S/O-' || TEXT(cohead_number)) AS ordernumber,"
               "       2 AS sequence,"
               "       cust_name AS item_number,"
               "       formatDate(coitem_scheddate) AS duedate,"
               "       coitem_scheddate AS r_duedate,"
               "       (coitem_scheddate < CURRENT_DATE) AS late,"
               "       formatQty(coitem_qtyord) AS f_qtyordered,"
               "       formatQty(coitem_qtyshipped - coitem_qtyreturned + qtyAtShipping(coitem_id)) AS f_qtyreceived,"
               "       formatQty((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned - qtyAtShipping(coitem_id)) * -1)) AS f_balance,"
               "       (noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned - qtyAtShipping(coitem_id)) * -1) AS balance "
               "FROM coitem, cohead, cust "
               "WHERE ( (coitem_status='O')"
               " AND (cohead_cust_id=cust_id)"
               " AND (coitem_cohead_id=cohead_id)"
               " AND (coitem_itemsite_id=:itemsite_id) ) ";
      }

      if (_showPlanned->isChecked())
      {
        if (primed)
          sql += "UNION ";
        else
          primed = TRUE;
       
        sql += "SELECT planord_id AS orderid, -1 AS altorderid,"
               "       CASE WHEN (planord_firm) THEN :firmPo"
               "            ELSE :plannedPo"
               "       END AS ordernumber,"
               "       1 AS sequence,"
               "       '' AS item_number,"
               "       formatDate(planord_duedate) AS duedate,"
               "       planord_duedate AS r_duedate,"
               "       FALSE AS late,"
               "       formatQty(planord_qty) AS f_qtyordered,"
               "       '' AS f_qtyreceived,"
               "       formatQty(planord_qty) AS f_balance,"
               "       planord_qty AS balance "
               "FROM planord "
               "WHERE ( (planord_type='P')"
               " AND (planord_itemsite_id=:itemsite_id) ) "

               "UNION SELECT planord_id AS orderid, -1 AS altorderid,"
               "             CASE WHEN (planord_firm) THEN :firmWo"
               "                  ELSE :plannedWo"
               "             END AS ordernumber,"
               "             1 AS sequence,"
               "             '' AS item_number,"
               "             formatDate(planord_duedate) AS duedate,"
               "             planord_duedate AS r_duedate,"
               "             FALSE AS late,"
               "             formatQty(planord_qty) AS f_qtyordered,"
               "             '' AS f_qtyreceived,"
               "             formatQty(planord_qty) AS f_balance,"
               "             planord_qty AS balance "
               "FROM planord "
               "WHERE ( (planord_type='W')"
               " AND (planord_itemsite_id=:itemsite_id) ) "

               "UNION SELECT planreq_id AS orderid, -1 AS altorderid,"
               "             CASE WHEN (planord_firm) THEN :firmWoReq"
               "                  ELSE :plannedWoReq"
               "             END AS ordernumber,"
               "             1 AS sequence,"
               "             item_number,"
               "             formatDate(planord_startdate) AS duedate,"
               "             planord_startdate AS r_duedate,"
               "             FALSE AS late,"
               "             formatQty(planreq_qty) AS f_qtyordered,"
               "             '' AS f_qtyreceived,"
               "             formatQty(planreq_qty * -1) AS f_balance,"
               "             (planreq_qty * -1) AS balance "
               "FROM planreq, planord, itemsite, item "
               "WHERE ( (planreq_source='P')"
               " AND (planreq_source_id=planord_id)"
               " AND (planord_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (planreq_itemsite_id=:itemsite_id) ) ";
      }

      sql += "ORDER BY r_duedate, sequence";

      q.prepare(sql);
      q.bindValue(":itemsite_id", itemsiteid);
      q.bindValue(":firmPo", tr("Planned P/O (firmed)"));
      q.bindValue(":plannedPo", tr("Planned P/O"));
      q.bindValue(":firmWo", tr("Planned W/O (firmed)"));
      q.bindValue(":plannedWo", tr("Planned W/O"));
      q.bindValue(":firmWoReq", tr("Planned W/O Req. (firmed)"));
      q.bindValue(":plannedWoReq", tr("Planned W/O Req."));
      q.exec();
      if (q.first())
      {
        XTreeWidgetItem *last = 0;
        do
        {
          runningAvailability += q.value("balance").toDouble();

          last = new XTreeWidgetItem( _availability, last,
			                           q.value("orderid").toInt(), q.value("altorderid").toInt(),
                                                   q.value("ordernumber"), q.value("item_number"),
                                                   q.value("duedate"), q.value("f_qtyordered"),
                                                   q.value("f_qtyreceived"), q.value("f_balance"),
                                                   formatQty(runningAvailability) );

          if (q.value("late").toBool())
            last->setTextColor(2, "red");

          if (runningAvailability < 0.0)
            last->setTextColor(6, "red");
          else if (runningAvailability < reorderLevel)
            last->setTextColor(6, "orange");

          if ((last->text(0).contains("Planned P/O")) ||  (last->text(0).contains("Planned W/O")))
            last->setTextColor("blue");
        }
        while (q.next());
      }
    }
  }
  else
  {
    _qoh->setText("0.00");
    _reorderLevel->setText("0.00");
    _orderMultiple->setText("0.00");
    _orderToQty->setText("0.00");
  }
}

void itemAvailabilityWorkbench::sHandleShowReorder( bool pValue )
{
  if (pValue)
    _showShortages->setChecked(TRUE);
}


void itemAvailabilityWorkbench::sFillListAvail()
{
  _invAvailability->clear();

  if ((_byDate->isChecked()) && (!_date->isValid()))
  {
    QMessageBox::critical( this, tr("Enter Valid Date"),
                           tr( "You have choosen to view Inventory Availabilty as of a given date but have not.\n"
                               "indicated the date.  Please enter a valid date." ) );
    _date->setFocus();
    return;
  }

  QString sql( "SELECT itemsite_id, itemtype, warehous_id, warehous_code, itemsite_leadtime,"
               "       formatQty(qoh) AS f_qoh,"
               "       formatQty(noNeg(qoh - allocated)) AS f_unallocated,"
               "       formatQty(allocated) AS f_allocated,"
               "       formatQty(ordered) AS f_ordered,"
               "       formatQty(reorderlevel) AS f_reorderlevel,"
               "       formatQty(outlevel) AS f_outlevel,"
               "       (qoh - allocated + ordered) AS available,"
               "       formatQty(qoh - allocated + ordered) AS f_available,"
               "       ((qoh - allocated + ordered) < 0) AS stockout,"
               "       ((qoh - allocated + ordered) <= reorderlevel) AS reorder "
               "FROM ( SELECT itemsite_id,"
               "              CASE WHEN (item_type IN ('P', 'O')) THEN 1"
               "                   WHEN (item_type IN ('M')) THEN 2"
               "                   ELSE 0"
               "              END AS itemtype,"
               "              warehous_id, warehous_code, itemsite_leadtime,"
               "              CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
               "              CASE WHEN(itemsite_useparams) THEN itemsite_ordertoqty ELSE 0.0 END AS outlevel,"
               "              itemsite_qtyonhand AS qoh," );

  if (_leadTime->isChecked())
    sql += " qtyAllocated(itemsite_id, itemsite_leadtime) AS allocated,"
           " qtyOrdered(itemsite_id, itemsite_leadtime) AS ordered ";

  else if (_byDays->isChecked())
    sql += " qtyAllocated(itemsite_id, :days) AS allocated,"
           " qtyOrdered(itemsite_id, :days) AS ordered ";

  else if (_byDate->isChecked())
    sql += " qtyAllocated(itemsite_id, (:date - CURRENT_DATE)) AS allocated,"
           " qtyOrdered(itemsite_id, (:date - CURRENT_DATE)) AS ordered ";

  else if (_byDates->isChecked())
    sql += " qtyAllocated(itemsite_id, :startDate, :endDate) AS allocated,"
           " qtyOrdered(itemsite_id, :startDate, :endDate) AS ordered ";


   sql += "FROM itemsite, warehous, item "
          "WHERE ( (itemsite_active)"
          " AND (itemsite_warehous_id=warehous_id)"
          " AND (itemsite_item_id=item_id)"
          " AND (item_id=:item_id)";

  if (_invWarehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") ) AS data ";

  if (_showReorder->isChecked())
  {
    sql += "WHERE ( ((qoh - allocated + ordered) <= reorderlevel) ";

    if (_ignoreReorderAtZero->isChecked())
      sql += " AND (NOT ( ((qoh - allocated + ordered) = 0) AND (reorderlevel = 0)) ) ) ";
    else
      sql += ") ";
  }
  else if (_showShortages->isChecked())
    sql += "WHERE ((qoh - allocated + ordered) < 0) ";

  sql += "ORDER BY warehous_code DESC;";

  q.prepare(sql);
  q.bindValue(":days", _days->value());
  q.bindValue(":date", _date->date());
  q.bindValue(":startDate", _startDate->date());
  q.bindValue(":endDate", _endDate->date());
  q.bindValue(":item_id", _item->id());
  _invWarehouse->bindValue(q);
  q.exec();
  XTreeWidgetItem *last = 0;
  while (q.next())
  {
    last = new XTreeWidgetItem( _invAvailability, last,
                                             q.value("itemsite_id").toInt(), q.value("itemtype").toInt(),
                                             q.value("warehous_code"), q.value("itemsite_leadtime"),
                                             q.value("f_qoh"), q.value("f_allocated"),
                                             q.value("f_unallocated"), q.value("f_ordered"),
                                             q.value("f_reorderlevel"), q.value("f_outlevel"),
                                             q.value("f_available") );

    if (_byDates->isChecked())
      last->setTextColor(2, "grey");

    if (q.value("stockout").toBool())
      last->setTextColor(8, "red");
    else if (q.value("reorder").toBool())
      last->setTextColor(8, "orange");
  }
}

void itemAvailabilityWorkbench::sPrintRunning()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("warehous_id", _warehouse->id());
  params.append("print");

  if (_showPlanned->isChecked())
    params.append("showPlanned");

  rptRunningAvailability newdlg(this, "", TRUE);
  newdlg.set(params);
}

void itemAvailabilityWorkbench::sPrintAvail()
{
  ParameterList params;
  _invWarehouse->appendValue(params);
  params.append("item_id", _item->id());
  params.append("print");

  if (_showReorder->isChecked())
    params.append("showReorder");

  if (_ignoreReorderAtZero->isChecked())
    params.append("ignoreReorderAtZero");

  if (_showShortages->isChecked())
    params.append("showShortages");

  if (_leadTime->isChecked())
    params.append("byLeadTime");
  else if (_byDays->isChecked())
    params.append("byDays", _days->value());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byDates");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  rptInventoryAvailabilityByItem newdlg(this, "", TRUE);
  newdlg.set(params);
}

void itemAvailabilityWorkbench::sPrintCosted()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("print");

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if (_useActualCosts->isChecked())
    params.append("useActualCosts");

  rptCostedIndentedBOM newdlg(this, "", TRUE);
  newdlg.set(params);
}

void itemAvailabilityWorkbench::sPrintHistory()
{
  ParameterList params;
  _invhistWarehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _item->id());
  params.append("transType", _transType->id());
  params.append("print");

  rptInventoryHistoryByItem newdlg(this, "", TRUE);
  newdlg.set(params);

}

void itemAvailabilityWorkbench::sPrintLocation()
{
  ParameterList params;

  params.append("item_id", _item->id());
  _itemlocWarehouse->appendValue(params);
  params.append("print");

  rptInventoryLocator newdlg(this, "", TRUE);
  newdlg.set(params);
}

void itemAvailabilityWorkbench::sPrintWhereUsed()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("print");

  if(!_effective->isNull())
    params.append("effective", _effective->date());

  rptSingleLevelWhereUsed newdlg(this, "", TRUE);
  newdlg.set(params);
}

void itemAvailabilityWorkbench::sPopulateMenuRunning( QMenu * pMenu, QTreeWidgetItem * pSelected )
{
  if (pSelected->text(0) == tr("Planned W/O (firmed)") || pSelected->text(0) == tr("Planned W/O") ||
      pSelected->text(0) == tr("Planned P/O (firmed)") || pSelected->text(0) == tr("Planned P/O") )
  {
    if (pSelected->text(0) == tr("Planned W/O (firmed)") || pSelected->text(0) == tr("Planned P/O (firmed)") )
      pMenu->insertItem(tr("Soften Order..."), this, SLOT(sSoftenOrder()), 0);
    else
      pMenu->insertItem(tr("Firm Order..."), this, SLOT(sFirmOrder()), 0);

    pMenu->insertItem(tr("Release Order..."), this, SLOT(sReleaseOrder()), 0);
    pMenu->insertItem(tr("Delete Order..."), this, SLOT(sDeleteOrder()), 0);
  }

  else if (pSelected->text(0).contains("W/O") && !(pSelected->text(0) == tr("Planned W/O Req. (firmed)") || pSelected->text(0) == tr("Planned W/O Req.")))
    pMenu->insertItem(tr("View Work Order Details..."), this, SLOT(sViewWOInfo()), 0);
}

void itemAvailabilityWorkbench::sPopulateMenuAvail( QMenu *pMenu, QTreeWidgetItem * selected )
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Inventory History..."), this, SLOT(sViewHistory()), 0);
  if (!_privleges->check("ViewInventoryHistory"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
  if (selected->text(3).remove(',').toDouble() == 0.0)
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
  if (selected->text(5).remove(',').toDouble() == 0.0)
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sRunningAvailability()), 0);

  pMenu->insertSeparator();

  if (((XTreeWidgetItem *)selected)->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Create P/R..."), this, SLOT(sCreatePR()), 0);
    if (!_privleges->check("MaintainPurchaseRequests"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Create P/O..."), this, SLOT(sCreatePO()), 0);
    if (!_privleges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();
  }
  else if (((XTreeWidgetItem *)selected)->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Create W/O..."), this, SLOT(sCreateWO()), 0);
    if (!_privleges->check("MaintainWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();
  }

  menuItem = pMenu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
  if (!_privleges->check("IssueCountTags"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Enter Misc. Inventory Count..."), this, SLOT(sEnterMiscCount()), 0);
  if (!_privleges->check("EnterMiscCounts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  pMenu->insertItem(tr("View Substitute Availability..."), this, SLOT(sViewSubstituteAvailability()), 0);
}

void itemAvailabilityWorkbench::sPopulateMenuCosted( QMenu * pMenu, QTreeWidgetItem * pSelected )
{
  if(!_privleges->check("ViewCosts"))
    return;
 
  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    pMenu->insertItem(tr("Maintain Item Costs..."), this, SLOT(sMaintainItemCosts()), 0);

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    pMenu->insertItem(tr("View Item Costing..."), this, SLOT(sViewItemCosting()), 0);
}

void itemAvailabilityWorkbench::sPopulateMenuHistory( QMenu * pMenu, QTreeWidgetItem * pItem )
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()), 0);
  menuItem = pMenu->insertItem(tr("Edit Transaction Information..."), this, SLOT(sEditTransInfo()), 0);

  if ( (pItem->text(3).length()) &&
       ( (pItem->text(2) == "RM") || (pItem->text(2) == "IM") ) )
  {
    QString orderNumber = _invhist->currentItem()->text(4);
    int sep1            = orderNumber.find('-');
    int sep2            = orderNumber.find('-', (sep1 + 1));
    int mainNumber      = orderNumber.mid((sep1 + 1), ((sep2 - sep1) - 1)).toInt();
    int subNumber       = orderNumber.right((orderNumber.length() - sep2) - 1).toInt();

    if ( (mainNumber) && (subNumber) )
    {
      menuItem = pMenu->insertItem(tr("View Work Order Information..."), this, SLOT(sViewWOInfoHistory()), 0);
      if ((!_privleges->check("MaintainWorkOrders")) && (!_privleges->check("ViewWorkOrders")))
        pMenu->setItemEnabled(menuItem, FALSE);
    }
  }
}

void itemAvailabilityWorkbench::sPopulateMenuLocation( QMenu * pMenu, QTreeWidgetItem * pSelected )
{
  int menuItem;

  if (((XTreeWidgetItem *)pSelected)->altId() == -1)
  {
    menuItem = pMenu->insertItem(tr("Relocate..."), this, SLOT(sRelocateInventory()), 0);
    if (!_privleges->check("RelocateInventory"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Reassign Lot/Serial #..."), this, SLOT(sReassignLotSerial()), 0);
    if (!_privleges->check("ReassignLotSerial"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void itemAvailabilityWorkbench::sPopulateMenuWhereUsed( QMenu *menu, QTreeWidgetItem * )
{
  int menuItem;

  menuItem = menu->insertItem(tr("Edit Bill of Materials..."), this, SLOT(sEditBOM()), 0);
  if (!_privleges->check("MaintainBOMs"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Edit Bill of Operations..."), this, SLOT(sEditBOO()), 0);
  if (!_privleges->check("MaintainBOOs"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Edit Item Master..."), this, SLOT(sEditItem()), 0);
  if (!_privleges->check("MaintainItemMasters"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("View Item Inventory History..."), this, SLOT(sViewInventoryHistory()), 0);
  if (!_privleges->check("ViewInventoryHistory"))
    menu->setItemEnabled(menuItem, FALSE);
}

void itemAvailabilityWorkbench::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byRange");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byRange");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _invAvailability->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemAvailabilityWorkbench::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _invAvailability->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _invAvailability->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());

  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemAvailabilityWorkbench::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());

  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemAvailabilityWorkbench::sViewSubstituteAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());

  if (_leadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sSoftenOrder()
{
  q.prepare( "UPDATE planord "
             "SET planord_firm=FALSE "
             "WHERE (planord_id=:planord_id);" );
  q.bindValue(":planord_id", _availability->id());
  q.exec();

  sFillListRunning();
}

void itemAvailabilityWorkbench::sFirmOrder()
{
  ParameterList params;
  params.append("planord_id", _availability->id());

  firmPlannedOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sFillListRunning();
}

void itemAvailabilityWorkbench::sReleaseOrder()
{
  // TODO
  if (_availability->currentItem()->text(0) == tr("Planned W/O (firmed)") || _availability->currentItem()->text(0) == tr("Planned W/O"))
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _availability->id());

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
#if 0
    if (newdlg.exec() != QDialog::Rejected)
    {
      sDeleteOrder();
      sFillListRunning();
    }
#endif
  }
  else if (_availability->currentItem()->text(0) == tr("Planned P/O (firmed)") || _availability->currentItem()->text(0) == tr("Planned P/O"))
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _availability->id());

    purchaseRequest newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
      sFillListRunning();
  }
}

void itemAvailabilityWorkbench::sDeleteOrder()
{
  q.prepare( "SELECT deletePlannedOrder(:planord_id, FALSE);" );
  q.bindValue(":planord_id", _availability->id());
  q.exec();

  sFillListRunning();
}

void itemAvailabilityWorkbench::sMaintainItemCosts()
{
  ParameterList params;
  params.append("item_id", _bomitem->altId());

  maintainItemCosts *newdlg  = new maintainItemCosts();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sViewItemCosting()
{
  ParameterList params;
  params.append( "item_id", _bomitem->altId() );
  params.append( "run",     TRUE              );

  dspItemCostSummary *newdlg = new dspItemCostSummary();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(2));

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
  else
  {
    transactionInformation newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void itemAvailabilityWorkbench::sEditTransInfo()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("invhist_id", _invhist->id());

  transactionInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemAvailabilityWorkbench::sViewWOInfo()
{
  QString orderNumber = _availability->currentItem()->text(0);
  int sep1            = orderNumber.find('-');
  int sep2            = orderNumber.find('-', (sep1 + 1));
  int mainNumber      = orderNumber.mid((sep1 + 1), ((sep2 - sep1) - 1)).toInt();
  int subNumber       = orderNumber.right((orderNumber.length() - sep2) - 1).toInt();

  q.prepare( "SELECT wo_id "
             "FROM wo "
             "WHERE ( (wo_number=:wo_number)"
             " AND (wo_subnumber=:wo_subnumber) );" );
  q.bindValue(":wo_number", mainNumber);
  q.bindValue(":wo_subnumber", subNumber);
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("wo_id", q.value("wo_id"));

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void itemAvailabilityWorkbench::sViewWOInfoHistory()
{
  QString orderNumber = _invhist->currentItem()->text(4);
  int sep1            = orderNumber.find('-');
  int sep2            = orderNumber.find('-', (sep1 + 1));
  int mainNumber      = orderNumber.mid((sep1 + 1), ((sep2 - sep1) - 1)).toInt();
  int subNumber       = orderNumber.right((orderNumber.length() - sep2) - 1).toInt();

  q.prepare( "SELECT wo_id "
             "FROM wo "
             "WHERE ( (wo_number=:wo_number)"
             " AND (wo_subnumber=:wo_subnumber) );" );
  q.bindValue(":wo_number", mainNumber);
  q.bindValue(":wo_subnumber", subNumber);
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("wo_id", q.value("wo_id"));

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void itemAvailabilityWorkbench::sRelocateInventory()
{
  ParameterList params;
  params.append("itemloc_id", _itemloc->id());

  relocateInventory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec())
    sFillListItemloc();
}

void itemAvailabilityWorkbench::sReassignLotSerial()
{
  ParameterList params;
  params.append("itemloc_id", _itemloc->id());

  reassignLotSerial newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillListItemloc();
}

void itemAvailabilityWorkbench::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _whereused->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sEditBOO()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _whereused->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sEditItem()
{
  item::editItem(_whereused->id());
}

void itemAvailabilityWorkbench::sViewInventoryHistory()
{
  ParameterList params;
  params.append("item_id", _whereused->altId());
  params.append("warehous_id", -1);
  params.append("run");

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}


