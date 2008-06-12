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

#include "itemSite.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "distributeInitialQOH.h"
#include "storedProcErrorLookup.h"

itemSite::itemSite(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sCheckItemsite()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_supply, SIGNAL(toggled(bool)), this, SLOT(sHandleSupplied(bool)));
  connect(_item, SIGNAL(typeChanged(const QString&)), this, SLOT(sCacheItemType(const QString&)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sCheckItemsite()));
  connect(_controlMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(sHandleControlMethod()));
  connect(_controlMethod, SIGNAL(activated(int)), this, SLOT(sHandleControlMethod()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sFillRestricted()));
  connect(_toggleRestricted, SIGNAL(clicked()), this, SLOT(sToggleRestricted()));
  connect(_useDefaultLocation, SIGNAL(toggled(bool)), this, SLOT(sDefaultLocChanged()));
  connect(_locationControl, SIGNAL(toggled(bool)), this, SLOT(sDefaultLocChanged()));

  _itemType = 0;

  _captive = FALSE;
  _updates = TRUE;
    
  _reorderLevel->setValidator(omfgThis->qtyVal());
  _orderUpToQty->setValidator(omfgThis->qtyVal());
  _minimumOrder->setValidator(omfgThis->qtyVal());
  _maximumOrder->setValidator(omfgThis->qtyVal());
  _orderMultiple->setValidator(omfgThis->qtyVal());
  _safetyStock->setValidator(omfgThis->qtyVal());
    
  _restricted->addColumn(tr("Location"), _itemColumn, Qt::AlignLeft );
  _restricted->addColumn(tr("Description"), -1, Qt::AlignLeft );
  _restricted->addColumn(tr("Allowed"), _ynColumn, Qt::AlignCenter );

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  else
  {
    _warehouse->setAllowNull(TRUE);
    _warehouse->setNull();
  }
  
  //If not lot serial control, remove options
  if (!_metrics->boolean("LotSerialControl"))
  {
    _controlMethod->removeItem(3);
    _controlMethod->removeItem(2);
    _perishable->hide();
    _tab->removeTab(_tab->indexOf(_expirationTab));
  }
  
  //If routings disabled, hide options
  if (!_metrics->boolean("Routings"))
  {
    _disallowBlankWIP->hide();
  }
  
  
  //If not OpenMFG, hide inapplicable controls
  if (_metrics->value("Application") != "OpenMFG")
  {
    _orderGroupLit->hide();
    _orderGroup->hide();
    _orderGroupDaysLit->hide();
    _mpsTimeFenceLit->hide();
    _mpsTimeFence->hide();
    _mpsTimeFenceDaysLit->hide();
  }
  
  //These things will be implemented at the site level later
  _costing->hide();
  _costingLit->hide();
  _planningType->hide();
  _planningTypeLit->hide();
}

itemSite::~itemSite()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemSite::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemSite::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  bool     check;
    
  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _itemsiteid = param.toInt();
	
    _item->setReadOnly(TRUE);

    populate();
  }
    
  param = pParams.value("item_id", &valid);
  if (valid)
  {
    check = TRUE;
	
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }
  else
    check = FALSE;
    
  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _warehouse->setId(param.toInt());
    _warehouse->setEnabled(FALSE);
  }
  else if (check)
    check = FALSE;
    
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      if (check)
      {
        XSqlQuery itemsiteid;
        itemsiteid.prepare( "SELECT itemsite_id "
                            "FROM itemsite "
                            "WHERE ( (itemsite_item_id=:item_id)"
                            " AND (itemsite_warehous_id=:warehous_id) );" );
        itemsiteid.bindValue(":item_id", _item->id());
        itemsiteid.bindValue(":warehous_id", _warehouse->id());
        itemsiteid.exec();
        if (itemsiteid.first())
        {
          _mode = cEdit;
          
          _itemsiteid = itemsiteid.value("itemsite_id").toInt();
          populate();
          
          _item->setReadOnly(TRUE);
          _warehouse->setEnabled(FALSE);
        }
        else
        {
          _mode = cNew;
          _reorderLevel->setDouble(0.0);
          _orderUpToQty->setDouble(0.0);
          _minimumOrder->setDouble(0.0);
          _maximumOrder->setDouble(0.0);
          _orderMultiple->setDouble(0.0);
          _safetyStock->setDouble(0.0);
          _cycleCountFreq->setValue(0);
          _leadTime->setValue(0);
          _eventFence->setValue(_metrics->value("DefaultEventFence").toInt());
          _tab->setTabEnabled(_tab->indexOf(_expirationTab),FALSE);
        } 
      }
    }
    else if (param.toString() == "edit")
    {
	_mode = cEdit;
	
	_item->setReadOnly(TRUE);
    }
    else if (param.toString() == "view")
    {
	_mode = cView;

	_item->setReadOnly(TRUE);
	_warehouse->setEnabled(FALSE);
	_useParameters->setEnabled(FALSE);
	_useParametersOnManual->setEnabled(FALSE);
	_reorderLevel->setEnabled(FALSE);
	_orderUpToQty->setEnabled(FALSE);
	_minimumOrder->setEnabled(FALSE);
	_maximumOrder->setEnabled(FALSE);
	_orderMultiple->setEnabled(FALSE);
	_safetyStock->setEnabled(FALSE);
	_abcClass->setEnabled(FALSE);
	_autoUpdateABCClass->setEnabled(FALSE);
	_cycleCountFreq->setEnabled(FALSE);
	_leadTime->setEnabled(FALSE);
	_eventFence->setEnabled(FALSE);
	_active->setEnabled(FALSE);
	_supply->setEnabled(FALSE);
	_createPr->setEnabled(FALSE);
	_createWo->setEnabled(FALSE);
	_sold->setEnabled(FALSE);
	_soldRanking->setEnabled(FALSE);
	_stocked->setEnabled(FALSE);
	_controlMethod->setEnabled(FALSE);
	_perishable->setEnabled(FALSE);
	_locationControl->setEnabled(FALSE);
	_disallowBlankWIP->setEnabled(FALSE);
	_useDefaultLocation->setEnabled(FALSE);
	_location->setEnabled(FALSE);
	_locations->setEnabled(FALSE);
	_miscLocation->setEnabled(FALSE);
	_miscLocationName->setEnabled(FALSE);
	_locationComments->setEnabled(FALSE);
	_plannerCode->setEnabled(FALSE);
	_costcat->setEnabled(FALSE);
	_eventFence->setEnabled(FALSE);
	_notes->setReadOnly(TRUE);
	_orderGroup->setEnabled(FALSE);
	_mpsTimeFence->setEnabled(FALSE);
	_close->setText(tr("&Close"));
	_save->hide();
	_comments->setReadOnly(TRUE);
	
	_close->setFocus();
    }
  }

  return NoError;
}

void itemSite::sSave()
{
  if (_warehouse->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Save Item Site"),
                           tr( "<p>You must select a Warehouse for this "
			      "Item Site before creating it." ) );
    _warehouse->setFocus();
    return;
  }
    
  if (_costcat->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Save Item Site"),
                           tr("<p>You must select a Cost Category for this "
			      "Item Site before you may save it.") );
    _costcat->setFocus();
    return;
  } 
    
  if (_plannerCode->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Save Item Site"),
                           tr("<p>You must select a Planner Code for this "
			      "Item Site before you may save it.") );
    _plannerCode->setFocus();
    return;
  } 
  
  if (_stocked->isChecked() && _reorderLevel->toDouble() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Item Site"),
                           tr("<p>You must set a reorder level "
			      "for a stocked item before you may save it.") );
    _reorderLevel->setFocus();
    return;
  }

  bool isLocationControl = _locationControl->isChecked();
  bool isLotSerial = (((_controlMethod->currentItem() == 2) || (_controlMethod->currentItem() == 3)) ? TRUE : FALSE);
  if ( ( (_mode == cNew) && (isLocationControl) ) ||
       ( (_mode == cEdit) && (isLocationControl) && (!_wasLocationControl) ) )
  {
    XSqlQuery locationid;
    locationid.prepare( "SELECT location_id "
                        "FROM location "
                        "WHERE ((location_warehous_id=:warehous_id)"
                        " AND ( (NOT location_restrict) OR"
                        "       ( (location_restrict) AND"
                        "         (location_id IN ( SELECT locitem_location_id"
                        "                           FROM locitem"
                        "                           WHERE (locitem_item_id=:item_id) ) ) ) )) "
                        "LIMIT 1;" );
    locationid.bindValue(":warehous_id", _warehouse->id());
    locationid.bindValue(":item_id", _item->id());
    locationid.exec();
    if (!locationid.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Item Site"),
                             tr( "<p>You have indicated that this Item Site "
				"should be multiply located but there are no "
                                 "non-restrictive Locations in the selected "
				 "Warehouse nor restrictive Locations that "
				 "will accept the selected Item."
				 "<p>You must first create at least one valid "
				 "Location for this Item Site before it may be "
				 "multiply located." ) );
      return;
    }
  }
    
  XSqlQuery newItemSite;
    
  if (_mode == cNew)
  {
    XSqlQuery newItemsiteid("SELECT NEXTVAL('itemsite_itemsite_id_seq') AS _itemsite_id");
    if (newItemsiteid.first())
      _itemsiteid = newItemsiteid.value("_itemsite_id").toInt();
    else if (newItemsiteid.lastError().type() != QSqlError::None)
    {
      systemError(this, newItemsiteid.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
	
    newItemSite.prepare( "INSERT INTO itemsite "
                         "( itemsite_id, itemsite_item_id, itemsite_warehous_id, itemsite_qtyonhand,"
                         "  itemsite_useparams, itemsite_useparamsmanual, itemsite_reorderlevel,"
                         "  itemsite_ordertoqty, itemsite_minordqty, itemsite_maxordqty, itemsite_multordqty,"
                         "  itemsite_safetystock, itemsite_cyclecountfreq,"
                         "  itemsite_leadtime, itemsite_eventfence, itemsite_plancode_id, itemsite_costcat_id,"
                         "  itemsite_supply, itemsite_createpr, itemsite_createwo, "
                         "  itemsite_sold, itemsite_soldranking,"
                         "  itemsite_stocked,"
                         "  itemsite_controlmethod, itemsite_perishable, itemsite_active,"
                         "  itemsite_loccntrl, itemsite_location_id, itemsite_location,"
                         "  itemsite_location_comments, itemsite_notes,"
                         "  itemsite_abcclass, itemsite_autoabcclass,"
                         "  itemsite_freeze, itemsite_datelastused, itemsite_ordergroup,"
                         "  itemsite_mps_timefence,"
                         "  itemsite_disallowblankwip, "
                         "  itemsite_warrpurc, itemsite_autoreg) "
                         "VALUES "
                         "( :itemsite_id, :itemsite_item_id, :itemsite_warehous_id, 0.0,"
                         "  :itemsite_useparams, :itemsite_useparamsmanual, :itemsite_reorderlevel,"
                         "  :itemsite_ordertoqty, :itemsite_minordqty, :itemsite_maxordqty, :itemsite_multordqty,"
                         "  :itemsite_safetystock, :itemsite_cyclecountfreq,"
                         "  :itemsite_leadtime, :itemsite_eventfence, :itemsite_plancode_id, :itemsite_costcat_id,"
                         "  :itemsite_supply, :itemsite_createpr, :itemsite_createwo, "
                         "  :itemsite_sold, :itemsite_soldranking,"
                         "  :itemsite_stocked,"
                         "  :itemsite_controlmethod, :itemsite_perishable, :itemsite_active,"
                         "  :itemsite_loccntrl, :itemsite_location_id, :itemsite_location,"
                         "  :itemsite_location_comments, :itemsite_notes,"
                         "  :itemsite_abcclass, :itemsite_autoabcclass,"
                         "  FALSE, startOfTime(), :itemsite_ordergroup,"
                         "  :itemsite_mps_timefence,"
                         "  :itemsite_disallowblankwip, "
                         "  :itemsite_warrpurc, :itemsite_autoreg  );" );
  }
  else if (_mode == cEdit)
  {
    int state = 0;
    if ( (_wasLocationControl) && (isLocationControl) )        // -
      state = 10;
    else if ( (!_wasLocationControl) && (!isLocationControl) ) // _
      state = 20;
    else if ( (!_wasLocationControl) && (isLocationControl) )  // _|-
      state = 30;
    else if ( (_wasLocationControl) && (!isLocationControl) )  // -|_
      state = 40;

    if ( (_wasLotSerial) && (isLotSerial) )                    // -
      state += 1;
    else if ( (!_wasLotSerial) && (!isLotSerial) )             // _
      state += 2;
    else if ( (!_wasLotSerial) && (isLotSerial) )              // _|-
      state += 3;
    else if ( (_wasLotSerial) && (!isLotSerial) )              // -|_
      state += 4;

    XSqlQuery query;
    if ( ( (state == 31) || (state == 32) || (state == 33) || (state == 34) ) &&
       (_qohCache > 0) && ( (!_location->isChecked() || (!_location->isEnabled()) ) ) )
    {
      QMessageBox::critical( this, tr("Cannot Save Item Site"),
                             tr( "<p>You have indicated that this Item Site "
				"should be mutiply located and there is existing quantity on hand."
				 "<p>You must select a default location for the on hand balance to be relocated to." ) );
      return;
    }
    
    if ( (state == 24) || (state == 42) || (state == 44) )
    {
      if ( QMessageBox::question(this, tr("Delete Inventory Detail Records?"),
                                 tr( "<p>You have indicated that detailed "
				    "inventory records for this Item Site "
				    "should no longer be kept. All of the "
				    "detailed inventory records will be "
				    "deleted. "
				    "Are you sure that you want to do this?" ),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
        return;
    }
    
    if (_qohCache != 0.0)
    {
//  Handle Lot/Serial distribution
      if ( (state == 13) || (state == 23) || (state == 33) || (state == 43) )
        QMessageBox::warning(this, tr("Assign Lot Numbers"),
                             tr("<p>You should now use the Reassign Lot/Serial # window to assign Lot/Serial numbers.") );
    }

    newItemSite.prepare( "UPDATE itemsite "
                         "SET itemsite_useparams=:itemsite_useparams, itemsite_useparamsmanual=:itemsite_useparamsmanual,"
                         "    itemsite_reorderlevel=:itemsite_reorderlevel, itemsite_ordertoqty=:itemsite_ordertoqty,"
                         "    itemsite_minordqty=:itemsite_minordqty, itemsite_maxordqty=:itemsite_maxordqty, itemsite_multordqty=:itemsite_multordqty,"
                         "    itemsite_safetystock=:itemsite_safetystock, itemsite_cyclecountfreq=:itemsite_cyclecountfreq,"
                         "    itemsite_leadtime=:itemsite_leadtime, itemsite_eventfence=:itemsite_eventfence,"
                         "    itemsite_plancode_id=:itemsite_plancode_id, itemsite_costcat_id=:itemsite_costcat_id,"
                         "    itemsite_supply=:itemsite_supply, itemsite_createpr=:itemsite_createpr, itemsite_createwo=:itemsite_createwo, "
                         "    itemsite_sold=:itemsite_sold, itemsite_soldranking=:itemsite_soldranking,"
                         "    itemsite_stocked=:itemsite_stocked,"
                         "    itemsite_controlmethod=:itemsite_controlmethod, itemsite_active=:itemsite_active,"
                         "    itemsite_perishable=:itemsite_perishable,"
                         "    itemsite_loccntrl=:itemsite_loccntrl, itemsite_location_id=:itemsite_location_id,"
                         "    itemsite_location=:itemsite_location, itemsite_location_comments=:itemsite_location_comments,"
                         "    itemsite_abcclass=:itemsite_abcclass, itemsite_autoabcclass=:itemsite_autoabcclass,"
                         "    itemsite_notes=:itemsite_notes,"
                         "    itemsite_ordergroup=:itemsite_ordergroup,"
                         "    itemsite_mps_timefence=:itemsite_mps_timefence,"
                         "    itemsite_disallowblankwip=:itemsite_disallowblankwip, "
                         "    itemsite_warrpurc=:itemsite_warrpurc, itemsite_autoreg=:itemsite_autoreg, "
                         "    itemsite_warehous_id=:itemsite_warehous_id "
                         "WHERE (itemsite_id=:itemsite_id);" );
  }

  newItemSite.bindValue(":itemsite_id", _itemsiteid);
  newItemSite.bindValue(":itemsite_item_id", _item->id());
  newItemSite.bindValue(":itemsite_warehous_id", _warehouse->id());

  newItemSite.bindValue(":itemsite_useparams", QVariant(_useParameters->isChecked(), 0));
  newItemSite.bindValue(":itemsite_reorderlevel", _reorderLevel->toDouble());
  newItemSite.bindValue(":itemsite_ordertoqty", _orderUpToQty->toDouble());
  newItemSite.bindValue(":itemsite_minordqty", _minimumOrder->toDouble());
  newItemSite.bindValue(":itemsite_maxordqty", _maximumOrder->toDouble());
  newItemSite.bindValue(":itemsite_multordqty", _orderMultiple->toDouble());
  newItemSite.bindValue(":itemsite_useparamsmanual", QVariant(_useParametersOnManual->isChecked(), 0));

  newItemSite.bindValue(":itemsite_safetystock", _safetyStock->toDouble());
  newItemSite.bindValue(":itemsite_cyclecountfreq", _cycleCountFreq->value());
  newItemSite.bindValue(":itemsite_plancode_id", _plannerCode->id());
  newItemSite.bindValue(":itemsite_costcat_id", _costcat->id());
    
  newItemSite.bindValue(":itemsite_active", QVariant(_active->isChecked(), 0));
  newItemSite.bindValue(":itemsite_supply", QVariant(_supply->isChecked(), 0));
  newItemSite.bindValue(":itemsite_createpr", QVariant(_createPr->isChecked(), 0));
  newItemSite.bindValue(":itemsite_createwo", QVariant(_createWo->isChecked(), 0));
  newItemSite.bindValue(":itemsite_sold", QVariant(_sold->isChecked(), 0));
  newItemSite.bindValue(":itemsite_stocked", QVariant(_stocked->isChecked(), 0));
  newItemSite.bindValue(":itemsite_perishable", QVariant(_perishable->isChecked(), 0));
  newItemSite.bindValue(":itemsite_loccntrl", QVariant(_locationControl->isChecked(), 0));
  newItemSite.bindValue(":itemsite_disallowblankwip", QVariant((_locationControl->isChecked() && _disallowBlankWIP->isChecked()), 0));
    
  newItemSite.bindValue(":itemsite_leadtime", _leadTime->value());
  newItemSite.bindValue(":itemsite_eventfence", _eventFence->value());
  newItemSite.bindValue(":itemsite_soldranking", _soldRanking->value());
    
  newItemSite.bindValue(":itemsite_location_comments", _locationComments->text().stripWhiteSpace());
  newItemSite.bindValue(":itemsite_notes", _notes->text().stripWhiteSpace());
  newItemSite.bindValue(":itemsite_abcclass", _abcClass->currentText());
  newItemSite.bindValue(":itemsite_autoabcclass", QVariant(_autoUpdateABCClass->isChecked(), 0));
    
  newItemSite.bindValue(":itemsite_ordergroup", _orderGroup->value());
  newItemSite.bindValue(":itemsite_mps_timefence", _mpsTimeFence->value());

  if (_useDefaultLocation->isChecked())
  {
    if (_location->isChecked())
    {
      newItemSite.bindValue(":itemsite_location_id", _locations->id());
      newItemSite.bindValue(":itemsite_location", "");
    }
    else if (_miscLocation->isChecked())
    {
      newItemSite.bindValue(":itemsite_location_id", -1);
      newItemSite.bindValue(":itemsite_location", _miscLocationName->text().stripWhiteSpace());
    }
  }
  else
  {
    newItemSite.bindValue(":itemsite_location_id", -1);
    newItemSite.bindValue(":itemsite_location", "");
  }
    
  if (_controlMethod->currentItem() == 0)
    newItemSite.bindValue(":itemsite_controlmethod", "N");
  else if (_controlMethod->currentItem() == 1)
    newItemSite.bindValue(":itemsite_controlmethod", "R");
  else if (_controlMethod->currentItem() == 2)
    newItemSite.bindValue(":itemsite_controlmethod", "L");
  else if (_controlMethod->currentItem() == 3)
    newItemSite.bindValue(":itemsite_controlmethod", "S");
    
  newItemSite.bindValue(":itemsite_warrpurc", QVariant(_purchWarranty->isChecked(), 0));
  newItemSite.bindValue(":itemsite_autoreg", QVariant(_autoRegister->isChecked(), 0));
    
  newItemSite.exec();
  if (newItemSite.lastError().type() != QSqlError::None)
  {
    systemError(this, newItemSite.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
    
  omfgThis->sItemsitesUpdated();
    
  if ((_captive) || (!_metrics->boolean("MultiWhs")))
    accept();
  else
  {
    _warehouse->setNull();
    clear();
  }
}

void itemSite::sCheckItemsite()
{
  int whsCache;
  if ( (_item->isValid()) &&
       (_updates) &&
       (_warehouse->id() != -1) )
  {
    _updates = FALSE;
	
    XSqlQuery query;
    query.prepare( "SELECT itemsite_id "
                   "FROM itemsite "
                   "WHERE ( (itemsite_item_id=:item_id)"
                   " AND (COALESCE(itemsite_warehous_id,:warehous_id)=:warehous_id) );" );
    query.bindValue(":item_id", _item->id());
    query.bindValue(":warehous_id", _warehouse->id());
    query.exec();
    if (query.first())
    {
      _mode = cEdit;
      _itemsiteid = query.value("itemsite_id").toInt();
      whsCache=_warehouse->id();
      disconnect(_warehouse, SIGNAL(newID(int)), this, SLOT(sCheckItemsite()));
      populate();
      if (_warehouse->id() == -1)
      {
        _warehouse->setId(whsCache);
        populateLocations();
      }
      connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sCheckItemsite()));
    }
    else
    { 
      _mode = cNew;
      clear();
    }

    _active->setFocus();

    _updates = TRUE;
  }
}

void itemSite::sHandleSupplied(bool pSupplied)
{
  if ( (pSupplied) &&
       ( (_itemType == 'P') || (_itemType == 'O') ) )
    _createPr->setEnabled(TRUE);
  else
  {
    _createPr->setEnabled(FALSE);
    _createPr->setChecked(FALSE);
  }

  if ( (pSupplied) &&
       ( (_itemType == 'M') ) )
    _createWo->setEnabled(TRUE);
  else
  {
    _createWo->setEnabled(FALSE);
    _createWo->setChecked(FALSE);
  }
} 


void itemSite::sHandleControlMethod()
{
  if ( (_controlMethod->currentItem() == 2) ||
       (_controlMethod->currentItem() == 3) )  
  {
    _perishable->setEnabled(TRUE);
    _tab->setTabEnabled(_tab->indexOf(_expirationTab),TRUE);
  }
  else
  {
    _perishable->setEnabled(FALSE);
    _tab->setTabEnabled(_tab->indexOf(_expirationTab),FALSE);
  }
}

void itemSite::sCacheItemType(const QString &pItemType)
{
    _itemType = pItemType[0].toAscii();
    sCacheItemType(_itemType);
}

void itemSite::sCacheItemType(char pItemType)
{
  _itemType = pItemType;

  if ( (_itemType == 'B') || (_itemType == 'F') || (_itemType == 'R') ||
	   (_itemType == 'L') || (_itemType == 'J') )
  {  
    _safetyStock->setEnabled(FALSE);
    _abcClass->setEnabled(FALSE);
    _autoUpdateABCClass->setEnabled(FALSE);
    _cycleCountFreq->setEnabled(FALSE);
    _leadTime->setEnabled(_itemType == 'J');
	_useParameters->setEnabled(!_itemType == 'J');

    if(_itemType=='L')
    {
      _orderGroup->setEnabled(TRUE);
      _mpsTimeFence->setEnabled(TRUE);
    }
    else
    {
      _orderGroup->setEnabled(FALSE);
      _mpsTimeFence->setEnabled(FALSE);
    }

    _supply->setChecked((_itemType!='L'));
    _supply->setEnabled(FALSE);
    _createPr->setChecked(FALSE);
    _createPr->setEnabled(FALSE);
    _createWo->setChecked(_itemType == 'J');
    _createWo->setEnabled(FALSE);

    if((_itemType == 'R') || (_itemType == 'J'))
      _sold->setEnabled(TRUE);
    else
    {
      _sold->setChecked(FALSE);
      _sold->setEnabled(FALSE);
    }
	
    _stocked->setChecked(FALSE);
    _stocked->setEnabled(FALSE);
	
    _useDefaultLocation->setChecked(FALSE);
    _useDefaultLocation->setEnabled(FALSE);
	
    _locationControl->setChecked(FALSE);
    _locationControl->setEnabled(FALSE);
	
    _controlMethod->setCurrentItem(1);
    _controlMethod->setEnabled(FALSE);
  }
  else
  {
    _safetyStock->setEnabled(TRUE);
    _abcClass->setEnabled(TRUE);
    _autoUpdateABCClass->setEnabled(TRUE);
    _cycleCountFreq->setEnabled(TRUE);
    _leadTime->setEnabled(TRUE);
    _orderGroup->setEnabled(TRUE);
    _mpsTimeFence->setEnabled(TRUE);
	
    _supply->setEnabled(TRUE);
    
    if ( (_itemType == 'O') || (_itemType == 'P') )
      _createPr->setEnabled(_supply->isChecked());
    else
    {
      _createPr->setChecked(FALSE);
      _createPr->setEnabled(FALSE);
    }
    
    if ( (_itemType == 'M') )
      _createWo->setEnabled(_supply->isChecked());
    else
    {
      _createWo->setChecked(FALSE);
      _createWo->setEnabled(FALSE);
    }
    
    _sold->setEnabled(TRUE);
    _stocked->setEnabled(TRUE);
    _useDefaultLocation->setEnabled(TRUE);
    _locationControl->setEnabled(TRUE);
    _controlMethod->setEnabled(TRUE);
  }
}

void itemSite::populateLocations()
{
    XSqlQuery query;
    query.prepare( "SELECT location_id, formatLocationName(location_id) AS locationname "
                   "FROM location "
                   "WHERE ( (location_warehous_id=:warehous_id)"
                   " AND (NOT location_restrict) ) "
		       
                   "UNION SELECT location_id, formatLocationName(location_id) AS locationname "
                   "FROM location, locitem "
                   "WHERE ( (location_warehous_id=:warehous_id)"
                   " AND (location_restrict)"
                   " AND (locitem_location_id=location_id)"
                   " AND (locitem_item_id=:item_id) ) "
		       
                   "ORDER BY locationname;" );
    query.bindValue(":warehous_id", _warehouse->id());
    query.bindValue(":item_id", _item->id());
    query.exec();
    _locations->populate(query);
    sDefaultLocChanged();
    sFillRestricted();
}

void itemSite::populate()
{
  XSqlQuery itemsite;
  itemsite.prepare( "SELECT itemsite_item_id, itemsite_warehous_id, itemsite_qtyonhand,"
                    "       item_sold, item_type, itemsite_active, itemsite_controlmethod,"
                    "       itemsite_perishable, itemsite_useparams, itemsite_useparamsmanual,"
                    "       itemsite_reorderlevel,"
                    "       itemsite_ordertoqty,"
                    "       itemsite_minordqty,"
                    "       itemsite_maxordqty,"
                    "       itemsite_multordqty,"
                    "       itemsite_safetystock,"
                    "       itemsite_leadtime, itemsite_eventfence, itemsite_plancode_id,"
                    "       itemsite_supply, itemsite_createpr, itemsite_createwo, itemsite_sold,"
                    "       itemsite_soldranking, itemsite_stocked, itemsite_abcclass, itemsite_autoabcclass,"
                    "       itemsite_loccntrl, itemsite_location_id, itemsite_location,"
                    "       itemsite_location_comments,"
                    "       itemsite_cyclecountfreq,"
                    "       itemsite_costcat_id, itemsite_notes,"
                    "       itemsite_ordergroup, itemsite_mps_timefence,"
                    "       itemsite_disallowblankwip, "
                    "       itemsite_warrpurc, itemsite_autoreg "
                    "FROM itemsite, item "
                    "WHERE ( (itemsite_item_id=item_id)"
                    " AND (itemsite_id=:itemsite_id) );" );
  itemsite.bindValue(":itemsite_id", _itemsiteid);
  itemsite.exec();
  if (itemsite.first())
  {
    _updates = FALSE;

    _item->setId(itemsite.value("itemsite_item_id").toInt());
    _warehouse->setId(itemsite.value("itemsite_warehous_id").toInt());
    _warehouse->setEnabled(itemsite.value("itemsite_warehous_id").isNull() ||
			   itemsite.value("itemsite_warehous_id").toInt() <= 0);
    populateLocations();

    _active->setChecked(itemsite.value("itemsite_active").toBool());

    _qohCache = itemsite.value("itemsite_qtyonhand").toDouble();

    _useParameters->setChecked(itemsite.value("itemsite_useparams").toBool());
    _useParametersOnManual->setChecked(itemsite.value("itemsite_useparamsmanual").toBool());

    _reorderLevel->setDouble(itemsite.value("itemsite_reorderlevel").toDouble());
    _orderUpToQty->setDouble(itemsite.value("itemsite_ordertoqty").toDouble());
    _minimumOrder->setDouble(itemsite.value("itemsite_minordqty").toDouble());
    _maximumOrder->setDouble(itemsite.value("itemsite_maxordqty").toDouble());
    _orderMultiple->setDouble(itemsite.value("itemsite_multordqty").toDouble());
    _safetyStock->setDouble(itemsite.value("itemsite_safetystock").toDouble());

    _cycleCountFreq->setValue(itemsite.value("itemsite_cyclecountfreq").toInt());
    _leadTime->setValue(itemsite.value("itemsite_leadtime").toInt());
    _eventFence->setValue(itemsite.value("itemsite_eventfence").toInt());

    _orderGroup->setValue(itemsite.value("itemsite_ordergroup").toInt());
    _mpsTimeFence->setValue(itemsite.value("itemsite_mps_timefence").toInt());

    if (itemsite.value("itemsite_controlmethod").toString() == "N")
    {
      _controlMethod->setCurrentItem(0);
      _wasLotSerial = FALSE;
    }
    else if (itemsite.value("itemsite_controlmethod").toString() == "R")
    {
      _controlMethod->setCurrentItem(1);
      _wasLotSerial = FALSE;
    }
    else if (itemsite.value("itemsite_controlmethod").toString() == "L")
    {
      _controlMethod->setCurrentItem(2);
      _wasLotSerial = TRUE;
    }
    else if (itemsite.value("itemsite_controlmethod").toString() == "S")
    {
      _controlMethod->setCurrentItem(3);
      _wasLotSerial = TRUE;
    }

    if ( (_controlMethod->currentItem() == 2) ||
         (_controlMethod->currentItem() == 3) )
    {
      _perishable->setEnabled(TRUE);
      _perishable->setChecked(itemsite.value("itemsite_perishable").toBool());
    }
    else
      _perishable->setEnabled(FALSE);

    _costcat->setId(itemsite.value("itemsite_costcat_id").toInt());
    _plannerCode->setId(itemsite.value("itemsite_plancode_id").toInt());
    _supply->setChecked(itemsite.value("itemsite_supply").toBool());
    _sold->setChecked(itemsite.value("itemsite_sold").toBool());
    _soldRanking->setValue(itemsite.value("itemsite_soldranking").toInt());
    _stocked->setChecked(itemsite.value("itemsite_stocked").toBool());
    _notes->setText(itemsite.value("itemsite_notes").toString());

    if ( (itemsite.value("item_type").toString() == "P") ||
         (itemsite.value("item_type").toString() == "O")    )
      _createPr->setChecked(itemsite.value("itemsite_createpr").toBool());
    else
      _createPr->setEnabled(FALSE);

    if ( (itemsite.value("item_type").toString() == "M") )
      _createWo->setChecked(itemsite.value("itemsite_createwo").toBool());
    else
      _createWo->setEnabled(FALSE);

    if (itemsite.value("itemsite_loccntrl").toBool())
      _locationControl->setChecked(TRUE);
    else
      _locationControl->setChecked(FALSE);
    _wasLocationControl = itemsite.value("itemsite_loccntrl").toBool();
    _disallowBlankWIP->setChecked(itemsite.value("itemsite_disallowblankwip").toBool());

    if (itemsite.value("itemsite_location_id").toInt() == -1)
    {
      if (!itemsite.value("itemsite_loccntrl").toBool())
      {
        if (itemsite.value("itemsite_location").toString().length())
        {
          _useDefaultLocation->setChecked(TRUE);
          _miscLocation->setChecked(TRUE);
          _miscLocationName->setText(itemsite.value("itemsite_location").toString());
        }
        else
          _useDefaultLocation->setChecked(FALSE);
      }
      else
        _useDefaultLocation->setChecked(FALSE);
    }
    else
    {
      _useDefaultLocation->setChecked(TRUE);
      _location->setChecked(TRUE);
      _locations->setId(itemsite.value("itemsite_location_id").toInt());
    }
	
    _locationComments->setText(itemsite.value("itemsite_location_comments").toString());

    for (int counter = 0; counter < _abcClass->count(); counter++)
      if (_abcClass->text(counter) == itemsite.value("itemsite_abcclass").toString())
        _abcClass->setCurrentItem(counter);

    _autoUpdateABCClass->setChecked(itemsite.value("itemsite_autoabcclass").toBool());
	
    sHandleSupplied(itemsite.value("itemsite_supply").toBool());
    sCacheItemType(_itemType);
    _comments->setId(_itemsiteid);
    
    
    _purchWarranty->setChecked(itemsite.value("itemsite_warrpurc").toBool());
    _autoRegister->setChecked(itemsite.value("itemsite_autoreg").toBool());

    _updates = TRUE;
  }
}

void itemSite::clear()
{
  if (_item->id() != -1)
    _item->setFocus();
  else
    _active->setFocus();
    
  _active->setChecked(TRUE);
  _useParameters->setChecked(FALSE);
  _useParametersOnManual->setChecked(FALSE);
  _reorderLevel->setText("0.00");
  _orderUpToQty->setText("0.00");
  _minimumOrder->setText("0.00");
  _maximumOrder->setText("0.00");
  _orderMultiple->setText("0.00");
  _safetyStock->setText("0.00");
    
  _cycleCountFreq->setValue(0);
  _leadTime->setValue(0);
  _eventFence->setValue(_metrics->value("DefaultEventFence").toInt());

  _orderGroup->setValue(1);
  _mpsTimeFence->setValue(0);
    
  _controlMethod->setCurrentItem(1);
  _supply->setChecked((_itemType!='L'));
  _sold->setChecked(!(_itemType == 'B' || _itemType == 'F' || _itemType == 'L'));
  _stocked->setChecked(FALSE);
    
  _locationControl->setChecked(FALSE);
  _useDefaultLocation->setChecked(FALSE);
  _miscLocationName->clear();
  _locationComments->clear();
    
  _costcat->setId(-1);
  
  _purchWarranty->setChecked(FALSE);
  _autoRegister->setChecked(FALSE);
  _tab->setTabEnabled(_tab->indexOf(_expirationTab),FALSE);
    
  populateLocations();
}


void itemSite::sFillRestricted()
{
  int locationid = _restricted->id();
  q.prepare("SELECT location_id, COALESCE(locitem_id, -1),"
            "       location_name, firstLine(location_descrip),"
            "       formatBoolYN(locitem_id IS NOT NULL)"
            "  FROM location LEFT OUTER JOIN locitem"
            "         ON (locitem_location_id=location_id AND locitem_item_id=:item_id)"
            " WHERE ((location_restrict)"
            "   AND  (location_warehous_id=:warehouse_id) ) "
            "ORDER BY location_name; ");
  q.bindValue(":warehouse_id", _warehouse->id());
  q.bindValue(":item_id", _item->id());
  q.exec();
  _restricted->populate(q, locationid, true);
}


void itemSite::sToggleRestricted()
{
  XTreeWidgetItem * locitem = static_cast<XTreeWidgetItem*>(_restricted->currentItem());
  if(0 == locitem)
    return;

  if(-1 != locitem->altId())
  {
    q.prepare("DELETE FROM locitem WHERE (locitem_id=:locitem_id); ");
    q.bindValue(":locitem_id", locitem->altId());
    q.exec();
  }
  else
  {
    q.prepare("INSERT INTO locitem(locitem_location_id, locitem_item_id) VALUES (:location_id, :item_id);");
    q.bindValue(":location_id", locitem->id());
    q.bindValue(":item_id", _item->id());
    q.exec();
  }

  sFillRestricted();
}

int itemSite::createItemSite(QWidget* pparent, int pitemsiteid, int pwhsid, bool peditResult)
{
  QString noactiveis = tr("<p>There is no active Item Site for this Item "
			  "at %1. Shipping or receiving this Item will "
			  "fail if there is no Item Site. Please have an "
			  "administrator create one before trying to ship "
			  "this Order.");
  QString whs;
  XSqlQuery whsq;
  whsq.prepare("SELECT warehous_code "
	       "FROM whsinfo "
	       "WHERE (warehous_id=:whsid);");
  whsq.bindValue(":whsid",	pwhsid);
  whsq.exec();
  if (whsq.first())
    whs = whsq.value("warehous_code").toString();
  else if (whsq.lastError().type() != QSqlError::None)
  {
    systemError(pparent, whsq.lastError().databaseText(), __FILE__, __LINE__);
    return -100;
  }
  else
  {
    QMessageBox::warning(pparent, tr("No Warehouse"),
		       tr("<p>The desired Item Site cannot be created as "
			  "there is no Warehouse with the internal ID %1.")
			    .arg(whs));
    return -99;
  }

  // use the s(ource) itemsite_item_id to see if the d(est) itemsite exists
  XSqlQuery isq;
  isq.prepare("SELECT COALESCE(d.itemsite_id, -1) AS itemsite_id,"
	      "       COALESCE(d.itemsite_active, false) AS itemsite_active "
	      "FROM itemsite s LEFT OUTER JOIN"
	      "     itemsite d ON (d.itemsite_item_id=s.itemsite_item_id"
	      "                    AND d.itemsite_warehous_id=:whsid) "
	      "WHERE (s.itemsite_id=:itemsiteid);");

  isq.bindValue(":itemsiteid",	pitemsiteid);
  isq.bindValue(":whsid",	pwhsid);
  isq.exec();
  if (isq.first())
  {
    int itemsiteid = isq.value("itemsite_id").toInt();
    if (itemsiteid > 0 && isq.value("itemsite_active").toBool())
      return itemsiteid;
    else if (! _privileges->check("MaintainItemSites"))
    {
      QMessageBox::warning(pparent, tr("No Active Item Site"),
			   noactiveis.arg(whs));
      return 0;	// not fatal - toitem trigger should log an event
    }
    else if (itemsiteid < 0)
    {
      isq.prepare("SELECT copyItemSite(:itemsiteid, :whsid) AS result;");
      isq.bindValue(":itemsiteid", pitemsiteid);
      isq.bindValue(":whsid",	 pwhsid);
      isq.exec();
      if (isq.first())
      {
	itemsiteid = isq.value("result").toInt();
	if (itemsiteid < 0)
	{
	  systemError(pparent, storedProcErrorLookup("copyItemSite", itemsiteid),
		      __FILE__, __LINE__);
	  return itemsiteid;
	}
	if (peditResult)
	{
	  itemSite newdlg(pparent, "", true);
	  ParameterList params;
	  params.append("mode",		"edit");
	  params.append("itemsite_id",	itemsiteid);
	  if (newdlg.set(params) != NoError || newdlg.exec() != XDialog::Accepted)
	  {
	    isq.prepare("SELECT deleteItemSite(:itemsiteid) AS result;");
	    isq.bindValue(":itemsiteid", itemsiteid);
	    isq.exec();
	    if (isq.first())
	    {
	      int result = isq.value("result").toInt();
	      if (result < 0)
	      {
		systemError(pparent, storedProcErrorLookup("deleteItemsite", result), __FILE__, __LINE__);
		return result;
	      }
	    }
	    else if (isq.lastError().type() != QSqlError::None)
	    {
	      systemError(pparent, isq.lastError().databaseText(), __FILE__, __LINE__);
	      return -100;
	    }
	  }
	}
	return itemsiteid;
      } // end if successfully copied an itemsite
      else if (isq.lastError().type() != QSqlError::None)
      {
	systemError(pparent, isq.lastError().databaseText(), __FILE__, __LINE__);
	return -100;
      }
    }
    else if (! isq.value("itemsite_active").toBool())
    {
      if (QMessageBox::question(pparent, tr("Inactive Item Site"),
			      tr("<p>The Item Site for this Item at %1 is "
				 "inactive. Would you like to make it active?")
				.arg(whs),
			      QMessageBox::Yes | QMessageBox::Default,
			      QMessageBox::No) == QMessageBox::Yes)
      {
	isq.prepare("UPDATE itemsite SET itemsite_active = TRUE "
		  "WHERE itemsite_id=:itemsiteid;");
	isq.bindValue(":itemsiteid", itemsiteid);
	isq.exec();
	if (isq.lastError().type() != QSqlError::None)
	{
	  systemError(pparent, isq.lastError().databaseText(), __FILE__, __LINE__);
	  return -100;
	}
	return itemsiteid;
      }
      else
      {
	QMessageBox::warning(pparent, tr("No Active Item Site"),
			     noactiveis.arg(whs));
	return -98;
      }
    }
  }
  else if (isq.lastError().type() != QSqlError::None)
  {
    systemError(pparent, isq.lastError().databaseText(), __FILE__, __LINE__);
    return -100;
  }

  systemError(pparent, tr("<p>There was a problem checking or creating an "
		       "Item Site for this Transfer Order Item."),
		      __FILE__, __LINE__);
  return -90;	// catchall: we didn't successfully find/create an itemsite
}


void itemSite::sDefaultLocChanged()
{
  if (_useDefaultLocation->isChecked())
  {
    _location->setChecked(_locationControl->isChecked());
    _location->setEnabled(_locationControl->isChecked());
    _locations->setEnabled(_locationControl->isChecked());
    _miscLocation->setChecked(!_locationControl->isChecked());
    _miscLocation->setEnabled(!_locationControl->isChecked());
    _miscLocationName->setEnabled(!_locationControl->isChecked());
  }
  else
  {
    _location->setEnabled(FALSE);
    _locations->setEnabled(FALSE);
    _miscLocation->setEnabled(FALSE);
    _miscLocationName->setEnabled(FALSE);
  }
}
