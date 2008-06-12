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

#include "createCountTagsByParameterList.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a createCountTagsByParameterList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
createCountTagsByParameterList::createCountTagsByParameterList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLocations()));
  connect(_byLocation, SIGNAL(toggled(bool)), _location, SLOT(setEnabled(bool)));

  _parameter->setType(ClassCode);

  _freeze->setEnabled(_privileges->check("FreezeInventory"));

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  if (_preferences->boolean("XCheckBox/forgetful"))
    _priority->setChecked(true);

  sPopulateLocations();
}

/*
 *  Destroys the object and frees any allocated resources
 */
createCountTagsByParameterList::~createCountTagsByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void createCountTagsByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse createCountTagsByParameterList::set(const ParameterList & pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("classcode", &valid);
  if (valid)
    _parameter->setType(ClassCode);

  param = pParams.value("plancode", &valid);
  if (valid)
    _parameter->setType(PlannerCode);

  switch (_parameter->type())
  {
    case ClassCode:
      setCaption(tr("Create Count Tags by Class Code"));
      break;

    case PlannerCode:
      setCaption(tr("Create Count Tags by Planner Code"));
      break;

    default:
      break;
  }

  return NoError;
}

void createCountTagsByParameterList::sCreate()
{
  QString sql;
//---------------Class Code--------------------------------------  
  if ((_parameter->type() == ClassCode) && _parameter->isSelected())
  {
    sql =  "SELECT createCountTag(itemsite_id, :comments, :priority, :freeze, :location_id) "
           "FROM ( SELECT itemsite_id";
    if(_byLocation->isChecked())
          sql += "      FROM itemsite, item, itemloc";
    else
          sql += "       FROM itemsite, item";
          sql +=  "       WHERE ( (itemsite_item_id=item_id)"
                "        AND (itemsite_warehous_id=:warehous_id)"
                "        AND (item_classcode_id=:classcode_id)";
    if(_byLocation->isChecked())
    {
       sql += "      AND (itemloc_location_id=:location_id)"
              "      AND (itemloc_itemsite_id = itemsite_id)"
              "      AND (validLocation(:location_id, itemsite_id))";
    }
    if(_ignoreZeroBalance->isChecked())
      sql += "      AND (itemsite_qtyonhand<>0.0)";
      sql += " )"
           "       ORDER BY item_number ) AS data;";
  }
//----------------Class Code Pattern-------------------------------  
  else if ((_parameter->type() == ClassCode) && _parameter->isPattern())
  {
    sql =  "SELECT createCountTag(itemsite_id, :comments, :priority, :freeze, :location_id) "
           "FROM ( SELECT itemsite_id";
    if(_byLocation->isChecked())
          sql += "      FROM itemsite, item, itemloc";
    else
          sql += "       FROM itemsite, item";
          sql += "       WHERE ( (itemsite_item_id=item_id)"
           "        AND (itemsite_warehous_id=:warehous_id)"
           "        AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";
    if(_byLocation->isChecked())
    {
       sql += "      AND (itemloc_location_id=:location_id)"
              "      AND (itemloc_itemsite_id = itemsite_id)"
              "      AND (validLocation(:location_id, itemsite_id))";
    }
    if(_ignoreZeroBalance->isChecked())
      sql += "      AND (itemsite_qtyonhand<>0.0)";
      sql += " )"
           "       ORDER BY item_number ) AS data;";
  }
//-----------------Planner Code-------------------------------------
  else if ((_parameter->type() == PlannerCode) && _parameter->isSelected())
  {
    sql =  "SELECT createCountTag(itemsite_id, :comments, :priority, :freeze, :location_id) "
           "FROM ( SELECT itemsite_id";
    if(_byLocation->isChecked())
          sql += "      FROM itemsite, item, itemloc";
    else
          sql += "       FROM itemsite, item";
    sql +=       "       WHERE ( (itemsite_item_id=item_id)"
           "        AND (itemsite_warehous_id=:warehous_id)"           
           "        AND (itemsite_plancode_id=:plancode_id)";
    if(_byLocation->isChecked())
    {
       sql += "      AND (itemloc_location_id=:location_id)"
              "      AND (itemloc_itemsite_id = itemsite_id)"
              "      AND (validLocation(:location_id, itemsite_id))";
    }
    if(_ignoreZeroBalance->isChecked())
      sql += "      AND (itemsite_qtyonhand<>0.0)";
      sql += " )"
           "       ORDER BY item_number ) AS data;";
  }
//----------------Planner Code Pattern-------------------------------
  else if ((_parameter->type() == PlannerCode) && _parameter->isPattern())
  {
    sql =  "SELECT createCountTag(itemsite_id, :comments, :priority, :freeze, :location_id) "
           "FROM ( SELECT itemsite_id";
    if(_byLocation->isChecked())
          sql += "      FROM itemsite, item, itemloc";
    else
          sql += "       FROM itemsite, item";
          sql+=  "       WHERE ( (itemsite_item_id=item_id)"
           "        AND (itemsite_warehous_id=:warehous_id)"   
           "        AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";
    if(_byLocation->isChecked())
    {
       sql += "      AND (itemloc_location_id=:location_id)"
              "      AND (itemloc_itemsite_id = itemsite_id)"
              "      AND (validLocation(:location_id, itemsite_id))";
    }
    if(_ignoreZeroBalance->isChecked())
      sql += "      AND (itemsite_qtyonhand<>0.0)";
      sql += " )"
           "       ORDER BY item_number ) AS data;";
  }
  else
//----------------Warehouse ------------------------------------------
  {
    sql =  "SELECT createCountTag(itemsite_id, :comments, :priority, :freeze, :location_id) "
           "FROM ( SELECT itemsite_id";
    if(_byLocation->isChecked())
          sql += "      FROM itemsite, item, itemloc";
    else
          sql += "       FROM itemsite, item";
          sql+=  "       WHERE ( (itemsite_item_id=item_id)"
           "        AND (itemsite_warehous_id=:warehous_id))"; 
    if(_byLocation->isChecked())
    {
       sql += "      AND (itemloc_location_id=:location_id)"
              "      AND (itemloc_itemsite_id = itemsite_id)"
              "      AND (validLocation(:location_id, itemsite_id))";
    }
    if(_ignoreZeroBalance->isChecked())
      sql += "      AND (itemsite_qtyonhand<>0.0)";
    sql += "       ORDER BY item_number ) AS data;";
  }
  q.prepare(sql);
  q.bindValue(":comments", _comments->text());
  q.bindValue(":priority", QVariant(_priority->isChecked(), 0));
  q.bindValue(":freeze", QVariant(_freeze->isChecked(), 0));
  q.bindValue(":warehous_id", _warehouse->id());
  if(_byLocation->isChecked())
    q.bindValue(":location_id", _location->id());
  _parameter->bindValue(q);
  if (!q.exec())
    systemError( this, tr("A System Error occurred at createCountTagsByParameterList::%1.")
                       .arg(__LINE__) );
  else if (!q.first())
    QMessageBox::information( this, tr("No Count Tags Created"),
      tr("No Item Sites matched the criteria you specified.") );

  accept();
}

void createCountTagsByParameterList::sPopulateLocations()
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

