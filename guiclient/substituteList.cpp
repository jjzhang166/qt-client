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

#include "substituteList.h"

#include <QVariant>

/*
 *  Constructs a substituteList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
substituteList::substituteList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    QButtonGroup * showByButtonGroup = new QButtonGroup(this);
    showByButtonGroup->addButton(_byLeadTime);
    showByButtonGroup->addButton(_byDays);
    showByButtonGroup->addButton(_byDate);

    // signals and slots connections
    connect(_byDays, SIGNAL(toggled(bool)), _days, SLOT(setEnabled(bool)));
    connect(_byDate, SIGNAL(toggled(bool)), _date, SLOT(setEnabled(bool)));
    connect(_subs, SIGNAL(valid(bool)), _select, SLOT(setEnabled(bool)));
    connect(_subs, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
    connect(showByButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
    connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
    connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
    init();

    //If not multi-warehouse hide whs control
    if (!_metrics->boolean("MultiWhs"))
    {
      _warehouseLit->hide();
      _warehouse->hide();
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
substituteList::~substituteList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void substituteList::languageChange()
{
    retranslateUi(this);
}


void substituteList::init()
{
  _subs->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft  );
  _subs->addColumn(tr("Description"),  -1,          Qt::AlignLeft  );
  _subs->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight );
  _subs->addColumn(tr("Norm. QOH"),    _qtyColumn,  Qt::AlignRight );
  _subs->addColumn(tr("Availability"), _qtyColumn,  Qt::AlignRight );
  _subs->addColumn(tr("Norm. Avail."), _qtyColumn,  Qt::AlignRight );
}

enum SetResponse substituteList::set( ParameterList &pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT womatl_itemsite_id,"
               "       bomitem_id, bomitem_subtype "
               "FROM womatl, bomitem "
               "WHERE ( (womatl_bomitem_id=bomitem_id)"
               " AND (womatl_id=:womatl_id) );" );
    q.bindValue(":womatl_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _item->setItemsiteid(q.value("womatl_itemsite_id").toInt());
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      
      _bomitemid = q.value("bomitem_id").toInt();
      _itemsiteid = q.value("womatl_itemsite_id").toInt();
      _source = q.value("bomitem_subtype").toString();

      sFillList();
    }
  }

  _byLeadTime->setChecked(pParams.inList("byLeadTime"));

  param = pParams.value("byDays", &valid);
  if (valid)
  {
   _byDays->setChecked(TRUE);
   _days->setValue(param.toInt());
  }

  param = pParams.value("byDate", &valid);
  if (valid)
  {
   _byDate->setChecked(TRUE);
   _date->setDate(param.toDate());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void substituteList::sSelect()
{
  if (_sub.findFirst("item_id", _subs->id()) != -1)
  {
    _uomratio = _sub.value("uomratio").toDouble();

    done(_subs->id());
  }
  else
    reject();
}

void substituteList::sFillList()
{
  _subs->clear();

  QString sql;

  if (_source == "I")
  {
    sql = "SELECT item_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
          "       formatQty(itemsite_qtyonhand) AS f_qoh,"
          "       formatQty(itemsite_qtyonhand * uomratio) AS f_normqoh,"
          "       formatQty(available) AS f_available,"
          "       formatqty(available * uomratio) AS f_normavailable,"
          "       uomratio "
          "FROM ( SELECT item_id, item_number, item_descrip1, item_descrip2,"
          "       itemsite_qtyonhand, itemsub_uomratio AS uomratio, itemsub_rank AS rank,";

    if (_byLeadTime->isChecked())
      sql += " qtyAvailable(itemsite_id, itemsite_leadtime) AS available ";
    else if (_byDays->isChecked())
      sql += " qtyAvailable(itemsite_id, :days) AS available ";
    else if (_byDate->isChecked())
      sql += " qtyAvailable(itemsite_id, (:date - CURRENT_DATE)) AS available ";

    sql += "FROM item, itemsite, itemsub "
           "WHERE ( (itemsite_item_id=item_id)"
           " AND (itemsite_item_id=itemsub_sub_item_id)"
           " AND (itemsite_warehous_id=:warehous_id)"
           " AND (itemsub_parent_item_id=:item_id) ) ) AS data "
           "ORDER BY rank";
  }
  else if (_source == "B")
  {
    sql = "SELECT item_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
          "       formatQty(itemsite_qtyonhand) AS f_qoh,"
          "       formatQty(itemsite_qtyonhand * uomratio) AS f_normqoh,"
          "       formatQty(available) AS f_available,"
          "       formatqty(available * uomratio) AS f_normavailable,"
          "       uomratio "
          "FROM ( SELECT item_id, item_number, item_descrip1, item_descrip2,"
          "       itemsite_qtyonhand, bomitemsub_uomratio AS uomratio, bomitemsub_rank AS rank,";

    if (_byLeadTime->isChecked())
      sql += " qtyAvailable(itemsite_id, itemsite_leadtime) AS available ";
    else if (_byDays->isChecked())
      sql += " qtyAvailable(itemsite_id, :days) AS available ";
    else if (_byDate->isChecked())
      sql += " qtyAvailable(itemsite_id, (:date - CURRENT_DATE)) AS available ";

    sql += "FROM item, itemsite, bomitemsub "
           "WHERE ( (itemsite_item_id=item_id)"
           " AND (itemsite_item_id=bomitemsub_item_id)"
           " AND (bomitemsub_bomitem_id=:bomitem_id)"
           " AND (itemsite_warehous_id=:warehous_id)"
           " AND (itemsite_id<>:itemsite_id) ) "
           "UNION "
           "SELECT item_id, item_number, item_descrip1, item_descrip2,"
           "       si.itemsite_qtyonhand, 1/bomitemsub_uomratio AS uomratio, 0 AS rank,";

    if (_byLeadTime->isChecked())
      sql += " qtyAvailable(si.itemsite_id, si.itemsite_leadtime) AS available ";
    else if (_byDays->isChecked())
      sql += " qtyAvailable(si.itemsite_id, :days) AS available ";
    else if (_byDate->isChecked())
      sql += " qtyAvailable(si.itemsite_id, (:date - CURRENT_DATE)) AS available ";

    sql += "FROM item, itemsite bi, bomitem, itemsite si, bomitemsub "
           "WHERE ( (bi.itemsite_item_id=item_id)"
           " AND (bi.itemsite_item_id=bomitem_item_id)"
           " AND (bomitem_id=:bomitem_id)"
           " AND (bi.itemsite_warehous_id=:warehous_id)"
           " AND (si.itemsite_id=:itemsite_id)"
           " AND (si.itemsite_item_id=bomitemsub_item_id)"
           " AND (bomitemsub_bomitem_id=:bomitem_id) ) "
           " ) AS data "
           "ORDER BY rank";
  }

  _sub.prepare(sql);
  _sub.bindValue(":warehous_id", _warehouse->id());
  _sub.bindValue(":item_id", _item->id());
  _sub.bindValue(":bomitem_id", _bomitemid);
  _sub.bindValue(":itemsite_id", _itemsiteid);

  if (_byDays->isChecked())
    _sub.bindValue(":days", _days->value());

  if (_byDate->isChecked())
    _sub.bindValue(":date", _date->date());

  _sub.exec();
  XTreeWidgetItem *last = 0;
  while (_sub.next())
    last = new XTreeWidgetItem(_subs, last, _sub.value("item_id").toInt(),
			       _sub.value("item_number"),
			       _sub.value("itemdescrip"),
			       _sub.value("f_qoh"),
			       _sub.value("f_normqoh"),
			       _sub.value("f_available"),
			       _sub.value("f_normavailable") );
}
