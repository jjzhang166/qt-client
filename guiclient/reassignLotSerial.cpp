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

#include "reassignLotSerial.h"

#include "math.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "storedProcErrorLookup.h"

reassignLotSerial::reassignLotSerial(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sFillList()));
    connect(_reassign, SIGNAL(clicked()), this, SLOT(sReassign()));

    _captive = FALSE;

    _item->setType(ItemLineEdit::cLotSerialControlled);
    _qty->setValidator(omfgThis->qtyPerVal());

    _source->addColumn(tr("Location"),     _itemColumn, Qt::AlignLeft   );
    _source->addColumn(tr("Lot/Serial #"), -1,          Qt::AlignLeft   );
    _source->addColumn(tr("Expires"),      _dateColumn, Qt::AlignCenter );
    _source->addColumn(tr("Warranty"), _dateColumn, Qt::AlignCenter);
    _source->addColumn(tr("Qty."),         _qtyColumn,  Qt::AlignRight  );
    
    //If not multi-warehouse hide whs control
    if (!_metrics->boolean("MultiWhs"))
    {
      _warehouseLit->hide();
      _warehouse->hide();
    }
}

reassignLotSerial::~reassignLotSerial()
{
    // no need to delete child widgets, Qt does it all for us
}

void reassignLotSerial::languageChange()
{
    retranslateUi(this);
}

enum SetResponse reassignLotSerial::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("itemloc_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT itemloc_itemsite_id "
               "FROM itemloc "
               "WHERE (itemloc_id=:itemloc_id);" );
    q.bindValue(":itemloc_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _captive = TRUE;

      _item->setItemsiteid(q.value("itemloc_itemsite_id").toInt());
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);

      for (int i = 0; i < _source->topLevelItemCount(); i++)
      {
        if (((XTreeWidget*)(_source->topLevelItem(i)))->id() == param.toInt())
          _source->setCurrentItem(_source->topLevelItem(i));
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  return NoError;
}

void reassignLotSerial::sReassign()
{
  if (_source->currentItem() == 0)
  {
    QMessageBox::critical( this, tr("Select Source Location"),
                           tr("You must select a Source Location before reassigning its Lot/Serial #.") );
    _source->setFocus();
    return;
  }

  if (_qty->toDouble() == 0)
  {
    QMessageBox::critical( this, tr("Enter Quantity to Reassign"),
                           tr("You must enter a quantity to reassign.") );
    _qty->setFocus();
    return;
  }

  QDoubleValidator* qtyVal = (QDoubleValidator*)(_qty->validator());
  q.prepare("SELECT reassignLotSerial(:source, CAST (:qty AS NUMERIC(100,:decimals)), "
	    "                         :lotNumber, :expirationDate, :warrantyDate) AS result;");
  q.bindValue(":source", _source->id());
  q.bindValue(":qty", _qty->toDouble());
  q.bindValue(":decimals", qtyVal->decimals());
  q.bindValue(":lotNumber", _lotNumber->text());

  if (_expirationDate->isEnabled())
    q.bindValue(":expirationDate", _expirationDate->date());
  else
    q.bindValue(":expirationDate", omfgThis->startOfTime());

  if (_warrantyDate->isEnabled())
    q.bindValue(":warrantyDate", _warrantyDate->date());

  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("reassignLotSerial", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));

    sFillList();

    if (_qty->isEnabled())
      _qty->clear();

    _qty->setFocus();
    _lotNumber->clear();
    _expirationDate->setNull();
    _warrantyDate->setNull();
  }
}

void reassignLotSerial::sFillList()
{
  if (_item->isValid())
  {
    q.prepare( "SELECT itemsite_id, itemsite_perishable, itemsite_controlmethod, itemsite_warrpurc "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.exec();
    if (q.first())
    {
      if (q.value("itemsite_controlmethod").toString() == "S")
      {
        _qty->setEnabled(FALSE);
        _qty->setText("1.0");
      }
      else if (q.value("itemsite_controlmethod").toString() == "L")
        _qty->setEnabled(TRUE);
      else
      {
        _source->clear();
        return;
      }

      int itemsiteid = q.value("itemsite_id").toInt();
      _expirationDate->setEnabled(q.value("itemsite_perishable").toBool());
      _warrantyDate->setEnabled(q.value("itemsite_warrpurc").toBool());

      q.prepare( "SELECT itemloc_id, formatLocationName(itemloc_location_id) AS locationname, ls_number,"
                 "       formatDate(itemloc_expiration, :never), formatDate(itemloc_warrpurc, :never), formatQtyPer(itemloc_qty) "
                 "FROM itemloc, itemsite, ls "
                 "WHERE ( (itemloc_itemsite_id=itemsite_id)"
                 " AND (itemsite_id=:itemsite_id)"
                 " AND (ls_id=itemloc_ls_id) ) "
                 "ORDER BY locationname;" );
      q.bindValue(":never", tr("Never"));
      q.bindValue(":itemsite_id", itemsiteid);
      q.exec();
      _source->populate(q);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _source->clear();
}
