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

#include "dspItemSitesByItem.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include "inputManager.h"
#include "itemSite.h"
#include "dspInventoryAvailabilityByItem.h"
#include "createCountTagsByItem.h"
#include "dspInventoryLocator.h"
#include "guiclient.h"

/*
 *  Constructs a dspItemSitesByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemSitesByItem::dspItemSitesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_itemsite, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_item, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setId(int)));

  _itemsite->addColumn(tr("Whs."),          _whsColumn,   Qt::AlignCenter );
  _itemsite->addColumn(tr("QOH"),           -1,           Qt::AlignRight  );
  _itemsite->addColumn(tr("Loc. Cntrl."),   _orderColumn, Qt::AlignCenter );
  _itemsite->addColumn(tr("Cntrl. Meth."),  _itemColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Sold Ranking"),  _itemColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Last Cnt'd"),    _dateColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Last Used"),     _dateColumn,  Qt::AlignCenter );
  _itemsite->setDragString("itemsiteid=");
  
  connect(omfgThis, SIGNAL(itemsitesUpdated()), SLOT(sFillList()));

  _item->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemSitesByItem::~dspItemSitesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemSitesByItem::languageChange()
{
  retranslateUi(this);
}

void dspItemSitesByItem::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

//  Ack - HackHackHack
  params.append("placeHolder", -1);

  if (_showInactive->isChecked())
    params.append("showInactive");

  orReport report("ItemSitesByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemSitesByItem::sPopulateMenu(QMenu *menu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  menuItem = menu->insertItem(tr("View Item Site..."), this, SLOT(sViewItemsite()), 0);
  if ((!_privileges->check("MaintainItemSites")) && (!_privileges->check("ViewItemSites")))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Edit Item Site..."), this, SLOT(sEditItemsite()), 0);
  if (!_privileges->check("MaintainItemSites"))
    menu->setItemEnabled(menuItem, FALSE);

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("View Inventory Availability..."), this, SLOT(sViewInventoryAvailability()), 0);
  if (!_privileges->check("ViewInventoryAvailability"))
    menu->setItemEnabled(menuItem, FALSE);

  if (((XTreeWidgetItem *)pSelected)->altId() == 1)
  {
    menuItem = menu->insertItem(tr("View Location/Lot/Serial # Detail..."), this, SLOT(sViewLocationLotSerialDetail()), 0);
    if (!_privileges->check("ViewQOH"))
      menu->setItemEnabled(menuItem, FALSE);
  }

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
  if (!_privileges->check("IssueCountTags"))
    menu->setItemEnabled(menuItem, FALSE);
}

void dspItemSitesByItem::sViewItemsite()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByItem::sEditItemsite()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByItem::sViewInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("byLeadTime");
  params.append("run");

  dspInventoryAvailabilityByItem *newdlg = new dspInventoryAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSitesByItem::sViewLocationLotSerialDetail()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("run");

  dspInventoryLocator *newdlg = new dspInventoryLocator();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSitesByItem::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByItem::sFillList()
{
  QString sql( "SELECT itemsite_id,"
               "       CASE WHEN ( (itemsite_loccntrl) OR"
               "                   (itemsite_controlmethod IN ('L', 'S')) ) THEN 1"
               "            ELSE 0"
               "       END,"
               "       warehous_code,"
               "       formatQty(itemsite_qtyonhand),"
               "       formatBoolYN(itemsite_loccntrl),"
               "       CASE WHEN itemsite_controlmethod='R' THEN :regular"
               "            WHEN itemsite_controlmethod='N' THEN :none"
               "            WHEN itemsite_controlmethod='L' THEN :lot"
               "            WHEN itemsite_controlmethod='S' THEN :serial"
               "       END,"
               "       CASE WHEN (itemsite_sold) THEN TEXT(itemsite_soldranking)"
               "            ELSE :na"
               "       END,"
               "       formatDate(itemsite_datelastcount, 'Never'),"
               "       formatDate(itemsite_datelastused, 'Never') "
               "FROM itemsite, warehous "
               "WHERE ( (itemsite_warehous_id=warehous_id)"
               " AND (itemsite_item_id=:item_id)" );

  if (!_showInactive->isChecked())
    sql += " AND (itemsite_active)";

  sql += " )"
         "ORDER BY warehous_code;";

  q.prepare(sql);
  q.bindValue(":regular", tr("Regular"));
  q.bindValue(":none", tr("None"));
  q.bindValue(":lot", tr("Lot #"));
  q.bindValue(":serial", tr("Serial #"));
  q.bindValue(":na", tr("N/A"));
  q.bindValue(":item_id", _item->id());
  q.exec();
  _itemsite->populate(q, TRUE);
}

