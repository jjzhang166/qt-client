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

#include "relocateInventory.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "storedProcErrorLookup.h"

relocateInventory::relocateInventory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_move, SIGNAL(clicked()), this, SLOT(sMove()));

  _captive = FALSE;

  _item->setType(ItemLineEdit::cLocationControlled);
  _qty->setValidator(omfgThis->transQtyVal());

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _source->addColumn(tr("Location"),_itemColumn, Qt::AlignLeft, true, "location");
  _source->addColumn(tr("Lot/Serial #"),     -1, Qt::AlignLeft, true, "lotserial");
  _source->addColumn(tr("Qty."),     _qtyColumn, Qt::AlignRight,true, "itemloc_qty");

  _target->addColumn(tr("Location"),     -1, Qt::AlignLeft,  true, "locationname");
  _target->addColumn(tr("Qty."), _qtyColumn, Qt::AlignRight, true, "qty");

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate());
}

relocateInventory::~relocateInventory()
{
  // no need to delete child widgets, Qt does it all for us
}

void relocateInventory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse relocateInventory::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);
    _source->setFocus();
  }

  int locid;
  param = pParams.value("source_location_id", &valid);
  if (valid)
  {
    locid = param.toInt();
    for (int i = 0; i < _source->topLevelItemCount(); i++)
    {
      XTreeWidgetItem* cursor = (XTreeWidgetItem*)(_source->topLevelItem(i));
      if (cursor->altId() == locid)
      {
        _source->setCurrentItem(cursor);
        _source->scrollToItem(cursor);
        _source->setEnabled(false);
      }
    }
  }

  param = pParams.value("target_location_id", &valid);
  if (valid)
  {
    locid = param.toInt();
    for (int i = 0; i < _source->topLevelItemCount(); i++)
    {
      XTreeWidgetItem* cursor = (XTreeWidgetItem*)(_source->topLevelItem(i));
      if (cursor->id() == locid)
      {
        _target->setCurrentItem(cursor);
        _target->scrollToItem(cursor);
        _target->setEnabled(false);
      }
    }
  }

  param = pParams.value("itemloc_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    XSqlQuery itemsiteid;
    itemsiteid.prepare( "SELECT itemloc_itemsite_id "
                        "FROM itemloc "
                        "WHERE (itemloc_id=:itemloc_id);" );
    itemsiteid.bindValue(":itemloc_id", param.toInt());
    itemsiteid.exec();
    if (itemsiteid.first())
    {
      _item->setItemsiteid(itemsiteid.value("itemloc_itemsite_id").toInt());
      _item->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);

      for (int i = 0; i < _source->topLevelItemCount(); i++)
      {
	XTreeWidgetItem* cursor = (XTreeWidgetItem*)(_source->topLevelItem(i));
        if (cursor->id() == param.toInt())
        {
          _source->setCurrentItem(cursor);
          _source->scrollToItem(cursor);
        }
      }

      _source->setEnabled(FALSE);
    }
  }

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _qty->setText(param.toString());
    _qty->setEnabled(false);
  }

  return NoError;
}

void relocateInventory::sMove()
{
  struct {
    bool        condition;
    QString     msg;
    QWidget     *widget;
  } error[] = {
    { ! _item->isValid(),
      tr("You must select an Item before posting this transaction."), _item },
    { _qty->text().length() == 0 || _qty->toDouble() <= 0,
      tr("<p>You must enter a positive Quantity before posting this Transaction."),
      _qty },
    { _source->currentItem() == 0,
      tr("<p>You must select a Source Location before relocating Inventory."),
      _source },
    { _target->currentItem() == 0,
      tr("<p>You must select a Target Location before relocating Inventory."),
      _target },
    { _source->currentItem() && _target->currentItem() &&
      ((XTreeWidgetItem *)_source->currentItem())->id() ==
        ((XTreeWidgetItem *)_target->currentItem())->id(),
      tr("<p>Please select different Locations for the Source and Target."),
      _target },
    { true, "", NULL }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Post Transaction"),
                          error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  XSqlQuery relocate;
  relocate.prepare( "SELECT relocateInventory(:source, :target, itemsite_id,"
                    ":qty, :comments, :date) AS result "
                    "FROM itemsite "
                    "WHERE ( (itemsite_item_id=:item_id)"
                    " AND (itemsite_warehous_id=:warehous_id));" );
  relocate.bindValue(":source", _source->id());
  relocate.bindValue(":target", _target->id());
  relocate.bindValue(":qty", _qty->toDouble());
  relocate.bindValue(":comments", _notes->text().stripWhiteSpace());
  relocate.bindValue(":item_id", _item->id());
  relocate.bindValue(":warehous_id", _warehouse->id());
  relocate.bindValue(":date",           _transDate->date());
  relocate.exec();

  if (relocate.first())
  {
    int result = relocate.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("relocateInventory", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (relocate.lastError().type() != QSqlError::None)
  {
    systemError(this, relocate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));

    sFillList();
    _qty->clear();
    _notes->clear();
    _item->setFocus();
  }
}

void relocateInventory::sFillList()
{
  if (_item->isValid())
  {
    XSqlQuery query;
    query.prepare( "SELECT itemloc.*,"
                   "       CASE WHEN (itemloc_location_id=-1) THEN :undefined"
                   "            ELSE formatLocationName(itemloc_location_id)"
                   "       END AS location,"
                   "       formatlotserialnumber(itemloc_ls_id) AS lotserial,"
                   "       'qty' AS itemloc_qty_xtnumericrole "
                   "FROM itemloc, itemsite "
                   "WHERE ( (itemloc_itemsite_id=itemsite_id)"
                   " AND (itemsite_item_id=:item_id)"
                   " AND (itemsite_warehous_id=:warehous_id) ) "
                   "ORDER BY location;" );
    query.bindValue(":undefined", tr("Undefined"));
    query.bindValue(":item_id", _item->id());
    query.bindValue(":warehous_id", _warehouse->id());
    query.exec();
    _source->populate(query, true);
    if (query.lastError().type() != QSqlError::None)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    query.prepare( "SELECT location_id,"
                   "       formatLocationName(location_id) AS locationname,"
                   "       ( SELECT COALESCE(SUM(itemloc_qty), 0)"
                   "         FROM itemloc, itemsite"
                   "         WHERE ( (itemloc_location_id=location_id)"
                   "         AND (itemloc_itemsite_id=itemsite_id)"
                   "         AND (itemsite_item_id=:item_id)"
                   "         AND (itemsite_warehous_id=location_warehous_id))) AS qty,"
                   "       'qty' AS qty_xtnumericrole "
                   "FROM location, itemsite "
                   "WHERE ( (itemsite_warehous_id=:warehous_id)"
                   " AND (location_warehous_id=:warehous_id)"
                   " AND (itemsite_item_id=:item_id)"
                   " AND  (validLocation(location_id, itemsite_id)) ) "
                   "ORDER BY locationname;" );
    query.bindValue(":warehous_id", _warehouse->id());
    query.bindValue(":item_id", _item->id());
    query.exec();
    _target->populate(query);
    if (query.lastError().type() != QSqlError::None)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _source->clear();
    _target->clear();
  }
}
