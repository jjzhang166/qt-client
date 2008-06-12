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

//  warehouseCluster.cpp
//  Created 03/12/2003 JSL
//  Copyright (c) 2003-2008, OpenMFG, LLC

#include <metasql.h>
#include <parameter.h>
#include <xsqlquery.h>

#include "OpenMFGWidgets.h"
#include "warehouseCluster.h"

WComboBox::WComboBox(QWidget *parent, const char *name) : XComboBox(parent, name)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  setMinimumWidth(80);

  setType(AllActive);
}

void WComboBox::setAllowNull(bool pAllowNull)
{
  _allowNull = pAllowNull;
  setType(_type);
}

WComboBox::WComboBoxTypes WComboBox::type()
{
  return _type;
}

void WComboBox::setType(WComboBoxTypes pType)
{
  _type = pType;

  int warehousid = ((_x_preferences) ? _x_preferences->value("PreferredWarehouse").toInt() : -1);

  QString whss("SELECT warehous_id, warehous_code, warehous_code "
             "FROM whsinfo "
             "WHERE (true"
             "<? if exists(\"active\") ?>  AND (warehous_active)  <? endif ?>"
             "<? if exists(\"shipping\") ?>AND (warehous_shipping)<? endif ?>"
             "<? if exists(\"transit\") ?>"
             "  AND (warehous_transit) "
             "<? elseif exists(\"nottransit\") ?>"
             "  AND (NOT warehous_transit)"
             "<? endif ?>"
             ") "
             "ORDER BY warehous_code;" );

  ParameterList whsp;

  switch (_type)
  {
    case AllActive:
      whsp.append("active");
      break;

    case Sold:
    case NonTransit:
      whsp.append("nottransit");
      whsp.append("active");
      break;

    case Shipping:
      whsp.append("shipping");
      whsp.append("active");
      break;

    case Supply:
      clear();
      return; // yes, the previous version just had "clear(); return;"
      break;

    case Transit:
      whsp.append("transit");
      whsp.append("active");
      break;

    case All:
    default:
      break;

  }

  MetaSQLQuery whsm(whss);
  XSqlQuery whsq = whsm.toQuery(whsp);
  populate(whsq, warehousid);
}

void WComboBox::findItemsites(int pItemID)
{
  if (pItemID != -1)
  {
    QString iss("SELECT warehous_id, warehous_code, warehous_code "
               "FROM whsinfo, itemsite "
               "WHERE ((itemsite_warehous_id=warehous_id)"
               "  AND  (itemsite_item_id=<? value(\"itemid\") ?>) "
               "<? if exists(\"active\") ?>  AND (warehous_active)  <? endif ?>"
               "<? if exists(\"shipping\") ?>AND (warehous_shipping)<? endif ?>"
               "<? if exists(\"transit\") ?>"
               "  AND (warehous_transit) "
               "<? elseif exists(\"nottransit\") ?>"
               "  AND (NOT warehous_transit)"
               "<? endif ?>"
               "<? if exists(\"active\") ?>  AND (itemsite_active)  <? endif ?>"
               "<? if exists(\"soldIS\") ?>  AND (itemsite_sold)    <? endif ?>"
               "<? if exists(\"supplyIS\") ?>AND (itemsite_supply)  <? endif ?>"
               ") "
               "ORDER BY warehous_code;" );
    ParameterList isp;
    isp.append("itemid", pItemID);

    switch (_type)
    {
      case AllActive:
        isp.append("active");
        break;

      case NonTransit:
        isp.append("nottransit");
        isp.append("active");
        break;

      case Sold:
        isp.append("active");
        isp.append("nottransit");
        isp.append("soldIS");
        break;

      /* TODO: ? previous version of this function didn't have Shipping case
      case Shipping:
        isp.append("shipping");
        isp.append("active");
        break;
      */

      case Supply:
        isp.append("active");
        isp.append("supplyIS");
        break;

      case Transit:
        isp.append("transit");
        isp.append("active");
        break;

      case All:
      default:
        break;
    }

    MetaSQLQuery ism(iss);
    XSqlQuery isq = ism.toQuery(isp);
    populate(isq, ((_x_preferences) ?
                    _x_preferences->value("PreferredWarehouse").toInt() : -1));

    if (currentItem() == -1)
      setCurrentItem(0);

  }
  else
    clear();
}

void WComboBox::setId(int pId)
{
  XComboBox::setId(pId);
}
