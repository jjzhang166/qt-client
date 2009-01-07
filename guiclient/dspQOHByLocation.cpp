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

#include "dspQOHByLocation.h"

#include <QMenu>
#include <QVariant>
#include <QSqlError>

#include <openreports.h>
#include <parameter.h>

#include "inputManager.h"
#include "relocateInventory.h"

dspQOHByLocation::dspQOHByLocation(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemloc, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sPopulateLocations()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  omfgThis->inputManager()->notify(cBCLocation, this, this, SLOT(set(int)));

  _location->setAllowNull(TRUE);

  _itemloc->setRootIsDecorated(TRUE);
  _itemloc->addColumn(tr("Site"),         _itemColumn, Qt::AlignLeft,   true,  "warehous_code"   );
  _itemloc->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _itemloc->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true,  "f_descrip"   );
  _itemloc->addColumn(tr("Lot/Serial #"), 150,         Qt::AlignLeft,   true,  "f_lotserial"   );
  _itemloc->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  _itemloc->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight,  true,  "qty"  );
  _itemloc->addColumn(tr("Reserved"),     _qtyColumn,  Qt::AlignRight,  false, "reservedqty"  );
  
  if(_metrics->boolean("EnableSOReservationsByLocation"))
    _itemloc->showColumn(6);

  sPopulateLocations();
}

dspQOHByLocation::~dspQOHByLocation()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspQOHByLocation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspQOHByLocation::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("location_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT location_warehous_id "
               "FROM location "
               "WHERE (location_id=:location_id);" );
    q.bindValue(":location_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _warehouse->setId(q.value("location_warehous_id").toInt());
      _location->setId(param.toInt());
    }
  }
  
  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspQOHByLocation::sPrint()
{
  ParameterList params;
  params.append("location_id", _location->id());
  _warehouse->appendValue(params);

  orReport report("QOHByLocation", params);
  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
  }
}

void dspQOHByLocation::sPopulateLocations()
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
    q.bindValue(":warehous_id", _warehouse->id());
    q.exec();
    _location->populate(q);
  }
}

void dspQOHByLocation::sRelocate()
{
  ParameterList params;
  params.append("itemloc_id", _itemloc->id());

  relocateInventory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec())
    sFillList();
}

void dspQOHByLocation::sPopulateMenu(QMenu *menu)
{
  int menuItem;

  if (_itemloc->id() != -1)
  {
    menuItem = menu->insertItem(tr("Relocate..."), this, SLOT(sRelocate()), 0);
    if (!_privileges->check("RelocateInventory"))
      menu->setItemEnabled(menuItem, FALSE);
  }
}

void dspQOHByLocation::sFillList()
{
  _itemloc->clear();

  if (_location->id() != -1)
  {
    q.prepare( "SELECT formatBoolYN(location_netable) AS netable,"
               "       formatBoolYN(location_restrict) AS restricted "
               "FROM location, warehous "
               "WHERE ( (location_warehous_id=warehous_id)"
               " AND (location_id=:location_id));" );
    q.bindValue(":location_id", _location->id());
    q.exec();
    if (q.first())
    {
      _netable->setText(q.value("netable").toString());
      _restricted->setText(q.value("restricted").toString());
    }

    QString sql( "SELECT itemloc_id, warehous_code, item_number,"
                 "       f_descrip, f_lotserial, uom_name,"
                 "       qty, reservedqty,"
                 "       'qty' AS qty_xtnumericrole,"
                 "       'qty' AS reservedqty_xtnumericrole,"
                 "       level AS xtindentrole "
                 "FROM ( "
                 "SELECT itemloc_id, 0 AS level, item_number AS sortkey, warehous_code, item_number,"
                 "       (item_descrip1 || ' ' || item_descrip2) AS f_descrip,"
                 "       formatlotserialnumber(itemloc_ls_id) AS f_lotserial, uom_name,"
                 "       itemloc_qty AS qty,"
                 "       CASE WHEN (:reservations) THEN qtyReservedLocation(itemloc_id)"
                 "            ELSE 0"
                 "       END AS reservedqty "
                 "FROM itemloc, itemsite, warehous, item, uom "
                 "WHERE ( (itemloc_itemsite_id=itemsite_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (item_inv_uom_id=uom_id)"
                 " AND (itemsite_warehous_id=warehous_id)"
                 " AND (itemloc_location_id=:location_id) ) "
                 "UNION "
                 "SELECT -1 AS itemloc_id, 0 AS level, item_number AS sortkey, warehous_code, item_number,"
                 "       (item_descrip1 || ' ' || item_descrip2) AS f_descrip,"
                 "       :na AS f_lotserial, uom_name,"
                 "       itemsite_qtyonhand AS qty,"
                 "       0 AS reservedqty "
                 "FROM itemsite, warehous, item, uom "
                 "WHERE ((itemsite_item_id=item_id)"
                 " AND (item_inv_uom_id=uom_id)"
                 " AND (itemsite_warehous_id=warehous_id)"
                 " AND (NOT itemsite_loccntrl)"
                 " AND (itemsite_location_id=:location_id)) " );

    if(_metrics->boolean("EnableSOReservationsByLocation"))
      sql +=     "UNION "
                 "SELECT itemloc_id, 1 AS level, item_number AS sortkey, '' AS warehous_code, '' AS item_number,"
                 "       (itemlocrsrv_source || '-' || formatSOItemNumber(itemlocrsrv_source_id)) AS f_descrip,"
                 "       '' AS f_lotserial, '' AS uom_name,"
                 "       NULL AS qty,"
                 "       itemlocrsrv_qty AS reservedqty "
                 "FROM itemlocrsrv, itemloc, itemsite, item "
                 "WHERE ( (itemlocrsrv_itemloc_id=itemloc_id) "
                 "  AND   (itemsite_id=itemloc_itemsite_id) "
                 "  AND   (item_id=itemsite_item_id) "
                 "  AND   (itemloc_location_id=:location_id) ) ";
               
    sql +=       "    ) AS data "
                 "ORDER BY sortkey, itemloc_id, level;";
                 
    q.prepare(sql);
    q.bindValue(":location_id", _location->id());
    q.bindValue(":na", tr("N/A"));
    if(_metrics->boolean("EnableSOReservationsByLocation"))
      q.bindValue(":reservations", true);
    else
      q.bindValue(":reservations", false);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (q.first())
    {
      _itemloc->populate(q);
    }
  }
  else
  {
    _netable->clear();
    _restricted->clear();
    _itemloc->clear();
  }
}
