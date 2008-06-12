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

#include "createCountTagsByItem.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a createCountTagsByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
createCountTagsByItem::createCountTagsByItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
  connect(_item, SIGNAL(valid(bool)), _create, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
  connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
  connect(_byLocation, SIGNAL(toggled(bool)), _location, SLOT(setEnabled(bool)));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLocations()));

  _freeze->setEnabled(_privileges->check("FreezeInventory"));
  _item->setType(ItemLineEdit::cPurchased
                 | ItemLineEdit::cManufactured
                 | ItemLineEdit::cBreeder
                 | ItemLineEdit::cCoProduct
                 | ItemLineEdit::cByProduct
                 | ItemLineEdit::cCosting
                 | ItemLineEdit::cTooling
                 | ItemLineEdit::cOutsideProcess);

  _captive = FALSE;
  
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  sPopulateLocations();
}

/*
 *  Destroys the object and frees any allocated resources
 */
createCountTagsByItem::~createCountTagsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void createCountTagsByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse createCountTagsByItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(TRUE);
    _warehouse->setEnabled(FALSE);

    _priority->setFocus();
  }

  return NoError;
}

void createCountTagsByItem::sCreate()
{
  q.prepare( "SELECT createCountTag(itemsite_id, :comments, :priority, :freeze, :location_id) AS _invcnt_id "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":comments", _comments->text());
  q.bindValue(":priority", QVariant(_priority->isChecked(), 0));
  q.bindValue(":freeze", QVariant(_freeze->isChecked(), 0));
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  if(_byLocation->isChecked())
    q.bindValue(":location_id", _location->id());
  q.exec();
  if (!q.first())
    systemError( this, tr("A System Error occurred at createCountTagsByItem::%1.")
                       .arg(__LINE__) );

  if (_captive)
    done(q.value("_invcnt_id").toInt());
  else
  {
    _item->setId(-1);
    _comments->clear();
    _item->setFocus();
  }
}


void createCountTagsByItem::sPopulateLocations()
{
  q.prepare("SELECT itemsite_loccntrl"
            "  FROM itemsite"
            " WHERE ((itemsite_item_id=:item_id)"
            "   AND  (itemsite_warehous_id=:warehous_id));");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if(q.first() && q.value("itemsite_loccntrl").toBool())
  {
    q.prepare( "SELECT location_id,"
               "       formatLocationName(location_id) AS locationname"
               "  FROM location, itemsite"
               " WHERE ((validLocation(location_id, itemsite_id))"
               "   AND  (itemsite_warehous_id=:warehous_id)"
               "   AND  (itemsite_item_id=:item_id)) "
               " ORDER BY locationname;");
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.exec();
    _location->populate(q);  
    _byLocation->setEnabled(true);
  }
  else
  {
    _location->clear();
    _byLocation->setChecked(false);
    _location->setEnabled(false);
    _byLocation->setEnabled(false);
  }
}

