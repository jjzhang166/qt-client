/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSite.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include <QDebug>

#include <metasql.h>
#include "mqlutil.h"

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "storedProcErrorLookup.h"

itemSite::itemSite(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sCheckItemsite()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_planningType, SIGNAL(activated(int)), this, SLOT(sHandlePlanningType()));
  connect(_poSupply, SIGNAL(toggled(bool)), this, SLOT(sHandlePOSupplied(bool)));
  connect(_createPo, SIGNAL(toggled(bool)), this, SLOT(sHandleCreatePO(bool)));
  connect(_createSoPr, SIGNAL(toggled(bool)), this, SLOT(sHandleCreateSOPR(bool)));
  connect(_createPr, SIGNAL(toggled(bool)), this, SLOT(sHandleCreateWOPR(bool)));
  connect(_woSupply, SIGNAL(toggled(bool)), this, SLOT(sHandleWOSupplied(bool)));
  connect(_item, SIGNAL(typeChanged(const QString&)), this, SLOT(sCacheItemType(const QString&)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sCheckItemsite()));
  connect(_controlMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(sHandleControlMethod()));
  connect(_controlMethod, SIGNAL(activated(int)), this, SLOT(sHandleControlMethod()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sFillRestricted()));
  connect(_toggleRestricted, SIGNAL(clicked()), this, SLOT(sToggleRestricted()));
  connect(_useDefaultLocation, SIGNAL(toggled(bool)), this, SLOT(sDefaultLocChanged()));
  connect(_locationControl, SIGNAL(toggled(bool)), this, SLOT(sDefaultLocChanged()));
  connect(_costJob, SIGNAL(toggled(bool)), this, SLOT(sHandleJobCost()));

  _planningType->append(0, "None", "N");

  _mpsTimeFenceLit->hide();
  _mpsTimeFence->hide();
  _mpsTimeFenceDaysLit->hide();

  if(_metrics->value("Application") == "PostBooks")
  {
    _planningType->setCurrentIndex(0);
    _planningType->hide();
    _planningTypeLit->hide();
    _createPlannedTransfers->hide();

    _orderGroupLit->hide();
    _orderGroup->hide();
    _orderGroupDaysLit->hide();
    _orderGroupFirst->hide();
  }
  else
    _planningType->append(1, "MRP", "M");

  if (!_metrics->boolean("MultiWhs"))
  {
    _createPlannedTransfers->hide();
  }

  if (_metrics->boolean("EnableDropShipments"))
	_dropShip->setEnabled(false);
  else
	_dropShip->hide();

  sHandlePlanningType();
  
  _itemsiteid = -1;
  _itemType = 0;
  _qohCache = 0;
  _costcatid = -1;

  _captive = false;
  _updates = true;
    
  _reorderLevel->setValidator(omfgThis->qtyVal());
  _orderUpToQty->setValidator(omfgThis->qtyVal());
  _minimumOrder->setValidator(omfgThis->qtyVal());
  _maximumOrder->setValidator(omfgThis->qtyVal());
  _orderMultiple->setValidator(omfgThis->qtyVal());
  _safetyStock->setValidator(omfgThis->qtyVal());
  _orderGroup->setMinimum(1);
    
  _restricted->addColumn(tr("Location"), _itemColumn, Qt::AlignLeft, true, "location_name" );
  _restricted->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "location_descrip" );
  _restricted->addColumn(tr("Allowed"), _dateColumn, Qt::AlignCenter, true, "allowed" );

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  else
  {
    _warehouse->setAllowNull(true);
    _warehouse->setNull();
  }

  _controlMethod->append(0, tr("None"),     "N");
  _controlMethod->append(1, tr("Regular"),  "R");
  _controlMethod->append(2, tr("Lot #"),    "L");
  _controlMethod->append(3, tr("Serial #"), "S");

  //Default to Regular control  
  _controlMethod->setCode("R");

  //If not lot serial control, remove options
  if (!_metrics->boolean("LotSerialControl"))
  {
    _controlMethod->removeItem(3);
    _controlMethod->removeItem(2);
    _autoNumberGroup->hide();
    _tab->removeTab(_tab->indexOf(_expirationTab));
  }
  
  //If routings disabled, hide options
  if (!_metrics->boolean("Routings"))
  {
    _disallowBlankWIP->hide();
  }
  
  _costAvg->setVisible(_metrics->boolean("AllowAvgCostMethod"));
  _costStd->setVisible(_metrics->boolean("AllowStdCostMethod"));
  _costStd->setVisible(_metrics->boolean("AllowJobCostMethod"));
  
  adjustSize();
  
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
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  bool     check;
    
  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = true;
    _itemsiteid = param.toInt();
    emit newId(_itemsiteid);
	
    _item->setReadOnly(true);

    populate();
  }
    
  param = pParams.value("item_id", &valid);
  if (valid)
  {
    check = true;
	
    _item->setId(param.toInt());
    _item->setReadOnly(true);
  }
  else
    check = false;
    
  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _captive = true;
    _warehouse->setId(param.toInt());
    _warehouse->setEnabled(false);
  }
  else if (check)
    check = false;
    
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
          emit newId(_itemsiteid);
          populate();
          
          _item->setReadOnly(true);
          _warehouse->setEnabled(false);
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
          _tab->setTabEnabled(_tab->indexOf(_expirationTab),false);
        } 
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
        _tab->setTabEnabled(_tab->indexOf(_expirationTab),false);
      }
      if (_mode == cNew)
      {
        XSqlQuery newItemsiteid("SELECT NEXTVAL('itemsite_itemsite_id_seq') AS _itemsite_id");
        if (newItemsiteid.first())
        {
          _itemsiteid = newItemsiteid.value("_itemsite_id").toInt();
          emit newId(_itemsiteid);
        }
        else if (newItemsiteid.lastError().type() != QSqlError::NoError)
        {
          systemError(this, newItemsiteid.lastError().databaseText(), __FILE__, __LINE__);
        }
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(true);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _item->setReadOnly(true);
      _warehouse->setEnabled(false);
      _useParameters->setEnabled(false);
      _useParametersOnManual->setEnabled(false);
      _reorderLevel->setEnabled(false);
      _orderUpToQty->setEnabled(false);
      _minimumOrder->setEnabled(false);
      _maximumOrder->setEnabled(false);
      _orderMultiple->setEnabled(false);
      _safetyStock->setEnabled(false);
      _abcClass->setEnabled(false);
      _autoUpdateABCClass->setEnabled(false);
      _cycleCountFreq->setEnabled(false);
      _leadTime->setEnabled(false);
      _eventFence->setEnabled(false);
      _active->setEnabled(false);
      _poSupply->setEnabled(false);
      _woSupply->setEnabled(false);
      _createPr->setEnabled(false);
      _createSoPr->setEnabled(false);
      _createPo->setEnabled(false);
      _dropShip->setEnabled(false);
      _createWo->setEnabled(false);
      _sold->setEnabled(false);
      _soldRanking->setEnabled(false);
      _stocked->setEnabled(false);
      _controlMethod->setEnabled(false);
      _perishable->setEnabled(false);
      _locationControl->setEnabled(false);
      _disallowBlankWIP->setEnabled(false);
      _useDefaultLocation->setEnabled(false);
      _location->setEnabled(false);
      _locations->setEnabled(false);
      _recvlocations->setEnabled(false);
      _issuelocations->setEnabled(false);
      _locations_dist->setEnabled(false);
      _recvlocations_dist->setEnabled(false);
      _issuelocations_dist->setEnabled(false);
      _miscLocation->setEnabled(false);
      _miscLocationName->setEnabled(false);
      _locationComments->setEnabled(false);
      _plannerCode->setEnabled(false);
      _costcat->setEnabled(false);
      _eventFence->setEnabled(false);
      _notes->setReadOnly(true);
      _orderGroup->setEnabled(false);
      _orderGroupFirst->setEnabled(false);
      _mpsTimeFence->setEnabled(false);
      _planningType->setEnabled(false);
      _createPlannedTransfers->setEnabled(false);
      _woCostGroup->setEnabled(false);
      _costing->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
      _comments->setReadOnly(true);
    }
    emit newMode(_mode);
  }

  return NoError;
}

bool itemSite::sSave()
{
  XSqlQuery itemSave;
  
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_warehouse->id() == -1, _warehouse,
                          tr("<p>You must select a Site for this "
                             "Item Site before creating it."))
         << GuiErrorCheck(!_costNone->isChecked() && !_costAvg->isChecked() &&
                          !_costStd->isChecked() && !_costJob->isChecked(), _costNone,
                          tr("<p>You must select a Cost Method for this "
                             "Item Site before you may save it."))
         << GuiErrorCheck(_costAvg->isChecked() && _qohCache < 0, _costAvg,
                          tr("<p>You can not change an Item Site to "
                             "Average Costing when it has a negative Qty. On Hand."))
         << GuiErrorCheck(_costcat->id() == -1, _costcat,
                          tr("<p>You must select a Cost Category for this "
                             "Item Site before you may save it."))
         << GuiErrorCheck(_plannerCode->id() == -1, _plannerCode,
                          tr("<p>You must select a Planner Code for this "
                             "Item Site before you may save it."))
         << GuiErrorCheck(_plannerCode->id() == -1, _plannerCode,
                          tr("<p>You must select a Planner Code for this "
                             "Item Site before you may save it."))
         << GuiErrorCheck(_mode == cEdit && _costcat->id() != _costcatid && _qohCache != 0, _costcat,
                          tr("This Item Site has a quantity on hand and Cost Category cannot be changed."))
         << GuiErrorCheck(!_active->isChecked() && _qohCache != 0, _active,
                          tr("This Item Site has a quantity on hand and must be marked as active."))
  ;
  

  bool isLocationControl = _locationControl->isChecked();
  bool isLotSerial = ((_controlMethod->code() == "L") || (_controlMethod->code() == "S"));
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
      errors << GuiErrorCheck(true, _locationControl,
                              tr("<p>You have indicated that this Item Site "
                                 "should be Multiple Location Controlled "
                                 "but there are no non-restrictive Locations "
                                 "in the selected Site nor restrictive Locations "
                                 "that will accept the selected Item."
                                 "<p>You must first create at least one valid "
                                 "Location for this Item Site before it may be "
                                 "Multiple Location Controlled."));
    }
  }
    
  if(_active->isChecked())
  {
    itemSave.prepare("SELECT item_id "
                     "FROM item "
                     "WHERE ((item_id=:item_id)"
                     "  AND  (item_active)) "
                     "LIMIT 1; ");
    itemSave.bindValue(":item_id", _item->id());
    itemSave.exec();
    if (!itemSave.first())         
    {
      errors << GuiErrorCheck(true, _active,
                              tr("This Item Site refers to an inactive Item and must be marked as inactive."));
    }
  }

  if (_locationControl->isChecked() && _disallowBlankWIP->isChecked())
  {
    itemSave.prepare("SELECT EXISTS(SELECT boohead_id"
                     "              FROM boohead"
                     "              WHERE ((COALESCE(boohead_final_location_id, -1) = -1)"
                     "                 AND (boohead_rev_id=getActiveRevId('BOO', :item_id))"
                     "                 AND (boohead_item_id=:item_id))"
                     "        UNION SELECT booitem_id"
                     "              FROM booitem JOIN"
                     "                   boohead ON (booitem_item_id=boohead_item_id"
                     "                           AND booitem_rev_id=boohead_rev_id)"
                     "              WHERE ((COALESCE(booitem_wip_location_id, -1) = -1)"
                     "                 AND (boohead_rev_id=getActiveRevId('BOO', :item_id))"
                     "                 AND (boohead_item_id=:item_id))"
                     "       ) AS isBlank;");
    itemSave.bindValue(":item_id", _item->id());
    itemSave.exec();
    if (itemSave.first())
    {
      if (itemSave.value("isBlank").toBool() &&
          QMessageBox::question(this, tr("Save anyway?"),
                                tr("<p>You have selected to disallow blank WIP "
                                   "locations but the active Bill Of Operations "
                                   "for this Item is either missing a Final "
                                   "Location or contains an Operation without "
                                   "a WIP Location. Are you sure you want to "
                                   "save this Item Site?<p>If you say 'Yes' "
                                   "then you should fix the Bill Of Operations."),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No) == QMessageBox::No)
        return false;
    }
    else if (itemSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }
    
  if(!_active->isChecked())
  {
    itemSave.prepare("SELECT coitem_id "
                     "FROM coitem "
                     "WHERE ((coitem_itemsite_id=:itemsite_id)"
                     "  AND  (coitem_status NOT IN ('X','C'))) "
                     "UNION "
                     "SELECT wo_id "
                     "FROM wo "
                     "WHERE ((wo_itemsite_id=:itemsite_id)"
                     "  AND  (wo_status<>'C')) "
                     "UNION "
                     "SELECT womatl_id "
                     "FROM womatl, wo "
                     "WHERE ((womatl_itemsite_id=:itemsite_id)"
                     "  AND  (wo_id=womatl_wo_id)"
                     "  AND  (wo_status<>'C')) "
                     "UNION "
                     "SELECT poitem_id "
                     "FROM poitem "
                     "WHERE ((poitem_itemsite_id=:itemsite_id)"
                     "  AND  (poitem_status<>'C')) "
                     "LIMIT 1; ");
    itemSave.bindValue(":itemsite_id", _itemsiteid);
    itemSave.exec();
    if (itemSave.first())         
    { 
      errors << GuiErrorCheck(true, _active,
                              tr("This Item Site is used in an active order and must be marked as active."));
    }
    
    if (_metrics->boolean("MultiWhs"))
    {
      itemSave.prepare("SELECT raitem_id "
                       "FROM raitem "
                       "WHERE ((raitem_itemsite_id=:itemsite_id)"
                       "  AND  (raitem_status<>'C')) "
                       "UNION "
                       "SELECT planord_id "
                       "FROM planord "
                       "WHERE (planord_itemsite_id=:itemsite_id)"
                       "UNION "
                       "SELECT planreq_id "
                       "FROM planreq "
                       "WHERE (planreq_itemsite_id=:itemsite_id)"
                       "LIMIT 1; ");
      itemSave.bindValue(":itemsite_id", _itemsiteid);
      itemSave.exec();
      if (itemSave.first())         
      { 
        errors << GuiErrorCheck(true, _active,
                                tr("This Item Site is used in an active order and must be marked as active."));
      }
    }
  }
  
  int _supplyItemsiteId = -1;
  if (_createPlannedTransfers->isChecked())
  {
    itemSave.prepare("SELECT itemsite_id "
                     "FROM itemsite "
                     "WHERE ( (itemsite_item_id=:item_id)"
                     "  AND   (itemsite_warehous_id=:warehous_id) ); ");
    itemSave.bindValue(":item_id", _item->id());
    itemSave.bindValue(":warehous_id", _suppliedFromSite->id());
    itemSave.exec();
    if (itemSave.first())
    {
      if (itemSave.value("itemsite_id").toInt() == _itemsiteid)
      { 
        errors << GuiErrorCheck(true, _suppliedFromSite,
                                tr("The Supplied From Site must be different from this Site."));
      }
      else
        _supplyItemsiteId = itemSave.value("itemsite_id").toInt();
    }
    else
    { 
      errors << GuiErrorCheck(true, _suppliedFromSite,
                              tr("Cannot find Supplied From Item Site."));
    }
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Item Site"), errors))
    return false;

  if (!isLotSerial)
  {
    itemSave.prepare("SELECT EXISTS(SELECT 1 "
                     "              FROM itemsite "
                     "              WHERE ((itemsite_item_id = :itemsite_item_id) "
                     "                 AND (itemsite_warehous_id <> :itemsite_warehous_id) "
                     "                 AND (itemsite_controlmethod IN ('L','S'))));");
    itemSave.bindValue(":itemsite_warehous_id", _warehouse->id());
    itemSave.bindValue(":itemsite_item_id", _item->id());
    itemSave.exec();
    if (itemSave.first())
    {
      if(itemSave.value("exists").toBool())
      {
        if ( QMessageBox::question(this, tr("Confirm Lot/Serial?"),
                                 tr( "<p>This Item Site has not been marked as lot/serial "
				    "enabled, yet other sites for item %1 are.\n "
				    "Do you wish to continue and save the Item Site? ").arg(_item->number()),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
           return true;
      }
    }
  }
  
  XSqlQuery newItemSite;
    
  if (_mode == cNew)
  {
    newItemSite.prepare( "INSERT INTO itemsite "
                         "( itemsite_id, itemsite_item_id, itemsite_warehous_id, itemsite_qtyonhand,"
                         "  itemsite_useparams, itemsite_useparamsmanual, itemsite_reorderlevel,"
                         "  itemsite_ordertoqty, itemsite_minordqty, itemsite_maxordqty, itemsite_multordqty,"
                         "  itemsite_safetystock, itemsite_cyclecountfreq,"
                         "  itemsite_leadtime, itemsite_eventfence, itemsite_plancode_id, itemsite_costcat_id,"
                         "  itemsite_poSupply, itemsite_woSupply, itemsite_createpr, itemsite_createwo,"
						 "  itemsite_dropship, itemsite_createsopr, itemsite_createsopo,"
                         "  itemsite_sold, itemsite_soldranking,"
                         "  itemsite_stocked, itemsite_planning_type, itemsite_supply_itemsite_id,"
                         "  itemsite_controlmethod, itemsite_perishable, itemsite_active,"
                         "  itemsite_loccntrl, itemsite_location_id, itemsite_location,"
                         "  itemsite_recvlocation_id, itemsite_issuelocation_id,"
                         "  itemsite_location_dist, itemsite_recvlocation_dist, itemsite_issuelocation_dist,"
                         "  itemsite_location_comments, itemsite_notes,"
                         "  itemsite_abcclass, itemsite_autoabcclass,"
                         "  itemsite_freeze, itemsite_datelastused, itemsite_ordergroup, itemsite_ordergroup_first,"
                         "  itemsite_mps_timefence,"
                         "  itemsite_disallowblankwip, "
                         "  itemsite_costmethod, itemsite_value, itemsite_cosdefault, "
                         "  itemsite_warrpurc, itemsite_autoreg, itemsite_lsseq_id) "
                         "VALUES "
                         "( :itemsite_id, :itemsite_item_id, :itemsite_warehous_id, 0.0,"
                         "  :itemsite_useparams, :itemsite_useparamsmanual, :itemsite_reorderlevel,"
                         "  :itemsite_ordertoqty, :itemsite_minordqty, :itemsite_maxordqty, :itemsite_multordqty,"
                         "  :itemsite_safetystock, :itemsite_cyclecountfreq,"
                         "  :itemsite_leadtime, :itemsite_eventfence, :itemsite_plancode_id, :itemsite_costcat_id,"
                         "  :itemsite_poSupply, :itemsite_woSupply, :itemsite_createpr, :itemsite_createwo,"
						 "  :itemsite_dropship, :itemsite_createsopr, :itemsite_createsopo,"
                         "  :itemsite_sold, :itemsite_soldranking,"
                         "  :itemsite_stocked, :itemsite_planning_type, :itemsite_supply_itemsite_id,"
                         "  :itemsite_controlmethod, :itemsite_perishable, :itemsite_active,"
                         "  :itemsite_loccntrl, :itemsite_location_id, :itemsite_location,"
                         "  :itemsite_recvlocation_id, :itemsite_issuelocation_id,"
                         "  :itemsite_location_dist, :itemsite_recvlocation_dist, :itemsite_issuelocation_dist,"
                         "  :itemsite_location_comments, :itemsite_notes,"
                         "  :itemsite_abcclass, :itemsite_autoabcclass,"
                         "  false, startOfTime(), :itemsite_ordergroup, :itemsite_ordergroup_first,"
                         "  :itemsite_mps_timefence,"
                         "  :itemsite_disallowblankwip, "
                         "  :itemsite_costmethod, 0, :itemsite_cosdefault, "
                         "  :itemsite_warrpurc, :itemsite_autoreg, :itemsite_lsseq_id  );" );
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
                                 "should be Multiple Location Controlled and "
                                 "there is existing quantity on hand."
                                 "<p>You must select a default location "
                                 "for the on hand balance to be relocated to." ) );
      return false;
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
        return true;
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
                         "    itemsite_poSupply=:itemsite_poSupply, itemsite_woSupply=:itemsite_woSupply,"
						 "    itemsite_createsopr=:itemsite_createsopr, itemsite_createsopo=:itemsite_createsopo,"
						 "    itemsite_dropship=:itemsite_dropship,"
                         "    itemsite_createpr=:itemsite_createpr, itemsite_createwo=:itemsite_createwo,"
                         "    itemsite_sold=:itemsite_sold, itemsite_soldranking=:itemsite_soldranking,"
                         "    itemsite_stocked=:itemsite_stocked, itemsite_planning_type=:itemsite_planning_type,"
                         "    itemsite_supply_itemsite_id=:itemsite_supply_itemsite_id,"
                         "    itemsite_controlmethod=:itemsite_controlmethod, itemsite_active=:itemsite_active,"
                         "    itemsite_perishable=:itemsite_perishable,"
                         "    itemsite_loccntrl=:itemsite_loccntrl, itemsite_location_id=:itemsite_location_id,"
                         "    itemsite_location=:itemsite_location, itemsite_location_comments=:itemsite_location_comments,"
                         "    itemsite_recvlocation_id=:itemsite_recvlocation_id, itemsite_issuelocation_id=:itemsite_issuelocation_id,"
                         "    itemsite_location_dist=:itemsite_location_dist,"
                         "    itemsite_recvlocation_dist=:itemsite_recvlocation_dist, itemsite_issuelocation_dist=:itemsite_issuelocation_dist,"
                         "    itemsite_abcclass=:itemsite_abcclass, itemsite_autoabcclass=:itemsite_autoabcclass,"
                         "    itemsite_notes=:itemsite_notes,"
                         "    itemsite_ordergroup=:itemsite_ordergroup,"
                         "    itemsite_ordergroup_first=:itemsite_ordergroup_first,"
                         "    itemsite_mps_timefence=:itemsite_mps_timefence,"
                         "    itemsite_disallowblankwip=:itemsite_disallowblankwip, "
                         "    itemsite_warrpurc=:itemsite_warrpurc, itemsite_autoreg=:itemsite_autoreg, "
                         "    itemsite_costmethod=:itemsite_costmethod, itemsite_cosdefault=:itemsite_cosdefault, "
                         "    itemsite_warehous_id=:itemsite_warehous_id, "
                         "    itemsite_lsseq_id=:itemsite_lsseq_id "
                         "WHERE (itemsite_id=:itemsite_id);" );
  }

  newItemSite.bindValue(":itemsite_id", _itemsiteid);
  newItemSite.bindValue(":itemsite_item_id", _item->id());
  newItemSite.bindValue(":itemsite_warehous_id", _warehouse->id());

  newItemSite.bindValue(":itemsite_useparams", QVariant(_useParameters->isChecked()));
  newItemSite.bindValue(":itemsite_reorderlevel", _reorderLevel->toDouble());
  newItemSite.bindValue(":itemsite_ordertoqty", _orderUpToQty->toDouble());
  newItemSite.bindValue(":itemsite_minordqty", _minimumOrder->toDouble());
  newItemSite.bindValue(":itemsite_maxordqty", _maximumOrder->toDouble());
  newItemSite.bindValue(":itemsite_multordqty", _orderMultiple->toDouble());
  newItemSite.bindValue(":itemsite_useparamsmanual", QVariant(_useParametersOnManual->isChecked()));

  newItemSite.bindValue(":itemsite_safetystock", _safetyStock->toDouble());
  newItemSite.bindValue(":itemsite_cyclecountfreq", _cycleCountFreq->value());
  newItemSite.bindValue(":itemsite_plancode_id", _plannerCode->id());
  newItemSite.bindValue(":itemsite_costcat_id", _costcat->id());
    
  newItemSite.bindValue(":itemsite_active", QVariant(_active->isChecked()));
  newItemSite.bindValue(":itemsite_planning_type", _planningType->code());
  if (_createPlannedTransfers->isChecked())
    newItemSite.bindValue(":itemsite_supply_itemsite_id", _supplyItemsiteId);
  newItemSite.bindValue(":itemsite_poSupply", QVariant(_poSupply->isChecked()));
  newItemSite.bindValue(":itemsite_woSupply", QVariant(_woSupply->isChecked()));
  newItemSite.bindValue(":itemsite_createsopr", QVariant(_createSoPr->isChecked()));
  newItemSite.bindValue(":itemsite_createsopo", QVariant(_createPo->isChecked()));
  newItemSite.bindValue(":itemsite_createpr", QVariant(_createPr->isChecked()));
  newItemSite.bindValue(":itemsite_dropship", QVariant(_dropShip->isChecked() && _createPo->isChecked()));
  newItemSite.bindValue(":itemsite_createwo", QVariant(_createWo->isChecked()));
  newItemSite.bindValue(":itemsite_sold", QVariant(_sold->isChecked()));
  newItemSite.bindValue(":itemsite_stocked", QVariant(_stocked->isChecked()));
  newItemSite.bindValue(":itemsite_perishable", QVariant(_perishable->isChecked()));
  newItemSite.bindValue(":itemsite_loccntrl", QVariant(_locationControl->isChecked()));
  newItemSite.bindValue(":itemsite_disallowblankwip", QVariant((_locationControl->isChecked() && _disallowBlankWIP->isChecked())));
    
  newItemSite.bindValue(":itemsite_leadtime", _leadTime->value());
  newItemSite.bindValue(":itemsite_eventfence", _eventFence->value());
  newItemSite.bindValue(":itemsite_soldranking", _soldRanking->value());
    
  newItemSite.bindValue(":itemsite_location_comments", _locationComments->text().trimmed());
  newItemSite.bindValue(":itemsite_notes", _notes->toPlainText().trimmed());
  newItemSite.bindValue(":itemsite_abcclass", _abcClass->currentText());
  newItemSite.bindValue(":itemsite_autoabcclass", QVariant(_autoUpdateABCClass->isChecked()));
    
  newItemSite.bindValue(":itemsite_ordergroup", _orderGroup->value());
  newItemSite.bindValue(":itemsite_ordergroup_first", QVariant(_orderGroupFirst->isChecked()));
  newItemSite.bindValue(":itemsite_mps_timefence", _mpsTimeFence->value());
  newItemSite.bindValue(":itemsite_autolsnum", _autoNumberGroup->isChecked());

  if (_useDefaultLocation->isChecked())
  {
    if (_location->isChecked())
    {
      newItemSite.bindValue(":itemsite_location_id", _locations->id());
      newItemSite.bindValue(":itemsite_recvlocation_id", _recvlocations->id());
      newItemSite.bindValue(":itemsite_issuelocation_id", _issuelocations->id());
      newItemSite.bindValue(":itemsite_location_dist", QVariant(_locations_dist->isChecked()));
      newItemSite.bindValue(":itemsite_recvlocation_dist", QVariant(_recvlocations_dist->isChecked()));
      newItemSite.bindValue(":itemsite_issuelocation_dist", QVariant(_issuelocations_dist->isChecked()));
      newItemSite.bindValue(":itemsite_location", "");
    }
    else if (_miscLocation->isChecked())
    {
      newItemSite.bindValue(":itemsite_location_id", -1);
      newItemSite.bindValue(":itemsite_recvlocation_id", -1);
      newItemSite.bindValue(":itemsite_issuelocation_id", -1);
      newItemSite.bindValue(":itemsite_location_dist", false);
      newItemSite.bindValue(":itemsite_recvlocation_dist", false);
      newItemSite.bindValue(":itemsite_issuelocation_dist", false);
      newItemSite.bindValue(":itemsite_location", _miscLocationName->text().trimmed());
    }
  }
  else
  {
    newItemSite.bindValue(":itemsite_location_id", -1);
    newItemSite.bindValue(":itemsite_recvlocation_id", -1);
    newItemSite.bindValue(":itemsite_issuelocation_id", -1);
    newItemSite.bindValue(":itemsite_location_dist", false);
    newItemSite.bindValue(":itemsite_recvlocation_dist", false);
    newItemSite.bindValue(":itemsite_issuelocation_dist", false);
    newItemSite.bindValue(":itemsite_location", "");
  }
    
  newItemSite.bindValue(":itemsite_controlmethod", _controlMethod->code());

  if(_costNone->isChecked())
    newItemSite.bindValue(":itemsite_costmethod", "N");
  else if(_costAvg->isChecked())
    newItemSite.bindValue(":itemsite_costmethod", "A");
  else if(_costStd->isChecked())
    newItemSite.bindValue(":itemsite_costmethod", "S");
  else if(_costJob->isChecked())
    newItemSite.bindValue(":itemsite_costmethod", "J");

  newItemSite.bindValue(":itemsite_warrpurc", QVariant(_purchWarranty->isChecked()));
  newItemSite.bindValue(":itemsite_autoreg", QVariant(_autoRegister->isChecked()));

  if (_woCostGroup->isChecked())
  {    
    if (_todate->isChecked())
      newItemSite.bindValue(":itemsite_cosdefault", QString("D"));
    else 
      newItemSite.bindValue(":itemsite_cosdefault", QString("P"));
  }
  if (_sequence->isValid())
    newItemSite.bindValue(":itemsite_lsseq_id", _sequence->id());
  
  newItemSite.exec();
  if (newItemSite.lastError().type() != QSqlError::NoError)
  {
    systemError(this, newItemSite.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
    
  omfgThis->sItemsitesUpdated();
  emit saved(_itemsiteid);
  
  if ((_captive) || (!_metrics->boolean("MultiWhs")))
    accept();
  else
  {
    _warehouse->setNull();
    clear();
  }
  return true;
}

void itemSite::sCheckItemsite()
{
  int whsCache;
  if ( (_item->isValid()) &&
       (_updates) &&
       (_warehouse->id() != -1) )
  {
    _updates = false;
	
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
      emit newMode(_mode);
      emit newId(_itemsiteid);
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
      emit newMode(_mode);
      clear();
    }

    _active->setFocus();

    _updates = true;
  }
}

void itemSite::sHandleJobCost()
{
  if (_costJob->isChecked() && _itemType == 'M')
  {
    _safetyStock->setEnabled(false);
    _abcClass->setEnabled(false);
    _autoUpdateABCClass->setEnabled(false);
    _cycleCountFreq->setEnabled(false);
    _useParameters->setEnabled(false);
    _woSupply->setChecked(true);
    _woSupply->setEnabled(false);
    _createWo->setChecked(true);
    _sold->setChecked(true);
    _sold->setEnabled(false);
    _stocked->setChecked(false);
    _autoUpdateABCClass->setChecked(false);
    _cycleCountFreq->setValue(0);
    _inventory->setEnabled(false);
    _poSupply->setChecked(false);
    _poSupply->setEnabled(false);
    _locationControl->setChecked(false);
    _useDefaultLocation->setChecked(false);
    _tab->setTabEnabled(_tab->indexOf(_restrictedLocations),false);
    _useParameters->setChecked(false);
    _planningType->setCurrentIndex(0);
    _safetyStock->setDouble(0);
    _createPlannedTransfers->setChecked(false);
    _tab->setTabEnabled(_tab->indexOf(_planningTab),false);
  }
  else if (_costJob->isChecked() && (_itemType == 'P' || _itemType == 'O'))
  {
    _safetyStock->setEnabled(false);
    _abcClass->setEnabled(false);
    _autoUpdateABCClass->setEnabled(false);
    _cycleCountFreq->setEnabled(false);
    _useParameters->setEnabled(false);
    _woSupply->setChecked(false);
    _woSupply->setEnabled(false);
    _createWo->setChecked(false);
    _sold->setChecked(true);
    _sold->setEnabled(false);
    _stocked->setChecked(false);
    _autoUpdateABCClass->setChecked(false);
    _cycleCountFreq->setValue(0);
    _inventory->setEnabled(false);
    _poSupply->setChecked(true);
    _poSupply->setEnabled(false);
    _createPr->setChecked(true);
    _createSoPr->setChecked(true);
    _locationControl->setChecked(false);
    _useDefaultLocation->setChecked(false);
    _tab->setTabEnabled(_tab->indexOf(_restrictedLocations),false);
    _useParameters->setChecked(false);
    _planningType->setCurrentIndex(0);
    _safetyStock->setDouble(0);
    _createPlannedTransfers->setChecked(false);
    _tab->setTabEnabled(_tab->indexOf(_planningTab),false);
  }
  else
  {
    _inventory->setEnabled(true);
    sCacheItemType(_itemType);
  }
}

void itemSite::sHandlePlanningType()
{
  if (_planningType->code() == "M" || _planningType->code() == "S")
  {
    _createPlannedTransfers->setEnabled(true);
    _orderGroup->setEnabled(true);
    _orderGroupFirst->setEnabled(true);

    if (_planningType->code() == "S")
      _mpsTimeFence->setEnabled(true);
    else
      _mpsTimeFence->setEnabled(false);
  }
  else
  {
    _createPlannedTransfers->setEnabled(false);
    _orderGroup->setEnabled(false);
    _orderGroupFirst->setEnabled(false);
    _mpsTimeFence->setEnabled(false);
  }

} 

void itemSite::sHandlePOSupplied(bool pSupplied)
{
  if ( (pSupplied) &&
       ( (_itemType == 'P') || (_itemType == 'O') ) )
  {
    _createPr->setEnabled(true);
	_createSoPr->setEnabled(true);
	_createPo->setEnabled(true);
  }
  else
  {
    _createPr->setEnabled(false);
    _createPr->setChecked(false);
	_createSoPr->setEnabled(false);
    _createSoPr->setChecked(false);
	_createPo->setEnabled(false);
    _createPo->setChecked(false);
	_dropShip->setChecked(false);
  }
}

void itemSite::sHandleCreatePO(bool pCreate)
{
  if (pCreate)
  {
    _dropShip->setEnabled(true);
	_createSoPr->setChecked(false);
	
	XSqlQuery source;
    source.prepare("SELECT * FROM itemsrc "
	               "WHERE itemsrc_item_id = :item_id "
				   "LIMIT 1;");
	source.bindValue(":item_id", _item->id());
    source.exec();
	if ( !(source.first()) )
	  QMessageBox::warning( this, tr("No Item Source found"),
        tr("Purchase Orders cannot be automatically "
           "created for this Item as there are no Item "
           "Sources for it.  You must create one or "
           "more Item Sources for this Item before "
           "the application can automatically create "
           "Purchase Orders for it.") );
  }
  else
	_dropShip->setChecked(false);
}

void itemSite::sHandleCreateSOPR(bool pCreate)
{
  if (pCreate)
  {
  	_createPo->setChecked(false);

	XSqlQuery source;
    source.prepare("SELECT * FROM itemsrc "
	               "WHERE itemsrc_item_id = :item_id "
				   "LIMIT 1;");
	source.bindValue(":item_id", _item->id());
    source.exec();
	if ( !(source.first()) )
	  QMessageBox::warning( this, tr("No Item Source found"),
        tr("Purchase Orders cannot be automatically "
           "created for this Item as there are no Item "
           "Sources for it.  You must create one or "
           "more Item Sources for this Item before "
           "the application can automatically create "
           "Purchase Orders for it.") );
  }
}

void itemSite::sHandleCreateWOPR(bool pCreate)
{
  if (pCreate)
  {
  	XSqlQuery source;
    source.prepare("SELECT * FROM itemsrc "
	               "WHERE itemsrc_item_id = :item_id "
				   "LIMIT 1;");
	source.bindValue(":item_id", _item->id());
    source.exec();
	if ( !(source.first()) )
	  QMessageBox::warning( this, tr("No Item Source found"),
        tr("Purchase Orders cannot be automatically "
           "created for this Item as there are no Item "
           "Sources for it.  You must create one or "
           "more Item Sources for this Item before "
           "the application can automatically create "
           "Purchase Orders for it.") );
  }
} 

void itemSite::sHandleWOSupplied(bool pSupplied)
{
  if ( (pSupplied) &&
       ( (_itemType == 'M') ) )
    _createWo->setEnabled(true);
  else
  {
    _createWo->setEnabled(false);
    _createWo->setChecked(false);
  }
} 

void itemSite::sHandleControlMethod()
{
  if (_controlMethod->code() == "N" || _itemType == 'R' || _itemType == 'K')
  {
    _costNone->setChecked(true);
    _costNone->setEnabled(true);
    _costAvg->setEnabled(false);
    _costStd->setEnabled(false);
    _costJob->setEnabled(false);
  }
  else
  {
    if(_costStd->isVisibleTo(this) && !_costAvg->isChecked() && !_costJob->isChecked())
      _costStd->setChecked(true);
    else if (_costAvg->isVisibleTo(this) && !_costStd->isChecked() && !_costJob->isChecked())
      _costAvg->setChecked(true);
    else if (_costJob->isVisibleTo(this) && !_costAvg->isChecked() && !_costStd->isChecked())
      _costJob->setChecked(true);
    _costNone->setEnabled(false);
    _costAvg->setEnabled(true);
    _costStd->setEnabled(true);
  }
  if( (_itemType == 'M' || _itemType == 'P' || _itemType == 'O') &&
     _controlMethod->code() != "N" &&
     _costJob->isVisibleTo(this) &&
     _qohCache == 0)
    _costJob->setEnabled(true);
  else
  {
    _costJob->setChecked(false);
    _costJob->setEnabled(false);
  }

  if ( (_controlMethod->code() == "L") ||
       (_controlMethod->code() == "S") )  
  {
    _autoNumberGroup->setEnabled(true);
    _perishable->setEnabled(true);
    _tab->setTabEnabled(_tab->indexOf(_expirationTab),true);
  }
  else
  {
    _autoNumberGroup->setEnabled(false);
    _perishable->setEnabled(false);
    _tab->setTabEnabled(_tab->indexOf(_expirationTab),false);
  }
  if (_costJob->isChecked())
    sHandleJobCost();
}

void itemSite::sCacheItemType(const QString &pItemType)
{
    _itemType = pItemType[0].toLatin1();
    sCacheItemType(_itemType);
}

void itemSite::sCacheItemType(char pItemType)
{
  _itemType = pItemType;

  if (_itemType == 'L')
  {
    _planningType->setCurrentIndex(1);
    _planningType->setEnabled(false);
  }
  else if (_itemType != 'P' && _itemType != 'M' && _itemType != 'F' && _itemType != 'B' &&
           _itemType != 'C' && _itemType != 'Y' && _itemType != 'O' && _itemType != 'A')
  {
    _planningType->setCurrentIndex(0);
    _planningType->setEnabled(false);
  }
  else
  {
    _planningType->setEnabled(true);
  }

  if (_controlMethod->code() == "N" || _itemType == 'R' || _itemType == 'K')
  {
    _costNone->setChecked(true);
    _costNone->setEnabled(true);
    _costAvg->setEnabled(false);
    _costStd->setEnabled(false);
    _costJob->setEnabled(false);
  }
  else
  {
    if(_costStd->isVisibleTo(this) && !_costAvg->isChecked())
      _costStd->setChecked(true);
    else
      _costAvg->setChecked(true);
    _costNone->setEnabled(false);
    _costAvg->setEnabled(true);
    _costStd->setEnabled(true);
    _costJob->setEnabled(false);
  }

  if ( (_itemType == 'B') || (_itemType == 'F') || (_itemType == 'R') ||
           (_itemType == 'L') || (_itemType == 'K'))
  {  
    _safetyStock->setEnabled(false);
    _abcClass->setEnabled(false);
    _autoUpdateABCClass->setEnabled(false);
    _cycleCountFreq->setEnabled(false);

    if(_itemType=='L')
    {
      _orderGroup->setEnabled(true);
      _orderGroupFirst->setEnabled(true);
      _mpsTimeFence->setEnabled(true);
    }
    else
    {
      _orderGroup->setEnabled(false);
      _orderGroupFirst->setEnabled(false);
      _mpsTimeFence->setEnabled(false);
    }

    _poSupply->setChecked((_itemType!='L') && (_itemType!='K'));
    _poSupply->setEnabled(false);
    _woSupply->setChecked((_itemType!='L') && (_itemType!='K'));
    _woSupply->setEnabled(false);
    _createPr->setChecked(false);
    _createPr->setEnabled(false);
    _createSoPr->setChecked(false);
    _createSoPr->setEnabled(false);
    _createPo->setChecked(false);
    _createPo->setEnabled(false);
    _createWo->setEnabled(false);

    if((_itemType == 'R') || (_itemType == 'K'))
    {
      _sold->setEnabled(true);
      _controlMethod->setCode("N");
    }
    else
    {
      _sold->setChecked(false);
      _sold->setEnabled(false);
      _controlMethod->setCode("R");
    }
	
    _stocked->setChecked(false);
    _stocked->setEnabled(false);
	
    _useDefaultLocation->setChecked(false);
    _useDefaultLocation->setEnabled(false);
	
    _locationControl->setChecked(false);
    _locationControl->setEnabled(false);
	
    _controlMethod->setEnabled(false);
  }
  else
  {
    _safetyStock->setEnabled(true);
    _abcClass->setEnabled(true);
    _autoUpdateABCClass->setEnabled(true);
    _cycleCountFreq->setEnabled(true);
    _leadTime->setEnabled(true);
    _orderGroup->setEnabled(true);
    _orderGroupFirst->setEnabled(true);
    _mpsTimeFence->setEnabled(true);
	
    _poSupply->setEnabled(true);
    _woSupply->setEnabled(true);
 
    if ( (_itemType == 'O') || (_itemType == 'P') )
    {
      _createPr->setEnabled(_poSupply->isChecked());
      _createSoPr->setEnabled(_poSupply->isChecked());
      _createPo->setEnabled(_poSupply->isChecked());
    }
    else
    {
      _createPr->setChecked(false);
      _createPr->setEnabled(false);
	  _createSoPr->setChecked(false);
      _createSoPr->setEnabled(false);
	  _createPo->setChecked(false);
      _createPo->setEnabled(false);
    }

    if ( (_itemType == 'M') )
      _createWo->setEnabled(_woSupply->isChecked());
    else
    {
      _createWo->setChecked(false);
      _createWo->setEnabled(false);
    }
    
    _sold->setEnabled(true);
    _stocked->setEnabled(true);
    _useDefaultLocation->setEnabled(true);
    _locationControl->setEnabled(true);
    _controlMethod->setEnabled(true);
  }

  if ( (_itemType == 'B') || (_itemType == 'F') || (_itemType == 'R') ||
           (_itemType == 'L') || (_itemType == 'K'))
    _woCostGroup->setEnabled(false);
  _tab->setTabEnabled(_tab->indexOf(_planningTab),(_itemType!='K'));
  _tab->setTabEnabled(_tab->indexOf(_restrictedLocations),(_itemType!='K'));

  sHandleControlMethod();
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
    _recvlocations->populate(query);
    _issuelocations->populate(query);
    sDefaultLocChanged();
    sFillRestricted();
}

void itemSite::populate()
{
  XSqlQuery itemsite;
  itemsite.prepare( "SELECT itemsite.*,"
                    "       item_sold, item_type, supplysite.itemsite_warehous_id AS supplywarehousid "
                    "FROM itemsite JOIN item ON (itemsite.itemsite_item_id=item_id) "
                    "              LEFT OUTER JOIN itemsite supplysite ON (itemsite.itemsite_supply_itemsite_id=supplysite.itemsite_id) "
                    "WHERE (itemsite.itemsite_id=:itemsite_id);" );
  itemsite.bindValue(":itemsite_id", _itemsiteid);
  itemsite.exec();
  if (itemsite.first())
  {
    _updates = false;

    _item->setId(itemsite.value("itemsite_item_id").toInt());
    _warehouse->setId(itemsite.value("itemsite_warehous_id").toInt());
    _warehouse->setEnabled(itemsite.value("itemsite_warehous_id").isNull() ||
			   itemsite.value("itemsite_warehous_id").toInt() <= 0);
    populateLocations();

    _active->setChecked(itemsite.value("itemsite_active").toBool());

    _qohCache = itemsite.value("itemsite_qtyonhand").toDouble();
    sHandleControlMethod();

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
    _orderGroupFirst->setChecked(itemsite.value("itemsite_ordergroup_first").toBool());
    _mpsTimeFence->setValue(itemsite.value("itemsite_mps_timefence").toInt());
    _controlMethod->setCode(itemsite.value("itemsite_controlmethod").toString());

    _wasLotSerial = ((itemsite.value("itemsite_controlmethod").toString() == "L") ||
        	     (itemsite.value("itemsite_controlmethod").toString() == "S") );

    if ( (_controlMethod->code() == "L") ||
         (_controlMethod->code() == "S") )
    {
      _perishable->setEnabled(true);
      _perishable->setChecked(itemsite.value("itemsite_perishable").toBool());
      _sequence->setId(itemsite.value("itemsite_lsseq_id").toInt());
    }
    else
      _perishable->setEnabled(false);

    if(itemsite.value("itemsite_costmethod").toString() == "N")
      _costNone->setChecked(true);
    else if(itemsite.value("itemsite_costmethod").toString() == "A")
      _costAvg->setChecked(true);
    else if(itemsite.value("itemsite_costmethod").toString() == "S")
      _costStd->setChecked(true);
    else if(itemsite.value("itemsite_costmethod").toString() == "J")
      _costJob->setChecked(true);

    _costcat->setId(itemsite.value("itemsite_costcat_id").toInt());
    _costcatid = itemsite.value("itemsite_costcat_id").toInt();
    _plannerCode->setId(itemsite.value("itemsite_plancode_id").toInt());

    for (int pcounter = 0; pcounter < _planningType->count(); pcounter++)
    {
      _planningType->setCurrentIndex(pcounter);
      if (QString(itemsite.value("itemsite_planning_type").toString()[0]) == _planningType->code())
        break;
    }
    sHandlePlanningType();
    
    if (!itemsite.value("supplywarehousid").isNull())
    {
      _suppliedFromSite->setId(itemsite.value("supplywarehousid").toInt());
      _createPlannedTransfers->setChecked(true);
    }
    else
      _createPlannedTransfers->setChecked(false);

    _poSupply->setChecked(itemsite.value("itemsite_poSupply").toBool());
    _woSupply->setChecked(itemsite.value("itemsite_woSupply").toBool());
    _sold->setChecked(itemsite.value("itemsite_sold").toBool());
    _soldRanking->setValue(itemsite.value("itemsite_soldranking").toInt());
    _stocked->setChecked(itemsite.value("itemsite_stocked").toBool());
    _notes->setText(itemsite.value("itemsite_notes").toString());

    if ( (itemsite.value("item_type").toString() == "P") ||
         (itemsite.value("item_type").toString() == "O")    )
    {
      _createPr->setChecked(itemsite.value("itemsite_createpr").toBool());
      _createSoPr->setChecked(itemsite.value("itemsite_createsopr").toBool());
      _createPo->setChecked(itemsite.value("itemsite_createsopo").toBool());
      _dropShip->setChecked(itemsite.value("itemsite_dropship").toBool());
    }
    else
    {
      _createPr->setEnabled(false);
      _createSoPr->setEnabled(false);
      _createPo->setEnabled(false);
      _dropShip->setEnabled(false);
    }

    if ( (itemsite.value("item_type").toString() == "M") )
      _createWo->setChecked(itemsite.value("itemsite_createwo").toBool());
    else
      _createWo->setEnabled(false);

    if (itemsite.value("itemsite_loccntrl").toBool())
      _locationControl->setChecked(true);
    else
      _locationControl->setChecked(false);
    _wasLocationControl = itemsite.value("itemsite_loccntrl").toBool();
    _disallowBlankWIP->setChecked(itemsite.value("itemsite_disallowblankwip").toBool());

    if (itemsite.value("itemsite_location_id").toInt() == -1)
    {
      if (!itemsite.value("itemsite_loccntrl").toBool())
      {
        if (itemsite.value("itemsite_location").toString().length())
        {
          _useDefaultLocation->setChecked(true);
          _miscLocation->setChecked(true);
          _miscLocationName->setText(itemsite.value("itemsite_location").toString());
        }
        else
          _useDefaultLocation->setChecked(false);
      }
      else
        _useDefaultLocation->setChecked(false);
    }
    else
    {
      _useDefaultLocation->setChecked(true);
      _location->setChecked(true);
      _locations->setId(itemsite.value("itemsite_location_id").toInt());
      _recvlocations->setId(itemsite.value("itemsite_recvlocation_id").toInt());
      _issuelocations->setId(itemsite.value("itemsite_issuelocation_id").toInt());
      _locations_dist->setChecked(itemsite.value("itemsite_location_dist").toBool());
      _recvlocations_dist->setChecked(itemsite.value("itemsite_recvlocation_dist").toBool());
      _issuelocations_dist->setChecked(itemsite.value("itemsite_issuelocation_dist").toBool());
    }
	
    _locationComments->setText(itemsite.value("itemsite_location_comments").toString());

    for (int counter = 0; counter < _abcClass->count(); counter++)
      if (_abcClass->itemText(counter) == itemsite.value("itemsite_abcclass").toString())
        _abcClass->setCurrentIndex(counter);

    _autoUpdateABCClass->setChecked(itemsite.value("itemsite_autoabcclass").toBool());
	
    sHandlePOSupplied(itemsite.value("itemsite_poSupply").toBool());
    sHandleWOSupplied(itemsite.value("itemsite_woSupply").toBool());
    _comments->setId(_itemsiteid);
    
    
    _purchWarranty->setChecked(itemsite.value("itemsite_warrpurc").toBool());
    _autoRegister->setChecked(itemsite.value("itemsite_autoreg").toBool());

    if (itemsite.value("itemsite_cosdefault").toString() == "D")
    {
      _woCostGroup->setChecked(true);
      _todate->setChecked(true);
      _proportional->setChecked(false);
    }
    else if (itemsite.value("itemsite_cosdefault").toString() == "P")
    {
      _woCostGroup->setChecked(true);
      _todate->setChecked(false);
      _proportional->setChecked(true);
    }

    _updates = true;
  }
  else if (itemsite.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemsite.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  emit populated();
}

void itemSite::clear()
{
  XSqlQuery newItemsiteid("SELECT NEXTVAL('itemsite_itemsite_id_seq') AS _itemsite_id");
  if (newItemsiteid.first())
  {
    _itemsiteid = newItemsiteid.value("_itemsite_id").toInt();
    emit newId(_itemsiteid);
  }
  else if (newItemsiteid.lastError().type() != QSqlError::NoError)
  {
    systemError(this, newItemsiteid.lastError().databaseText(), __FILE__, __LINE__);
  }
  if (_item->id() != -1)
    _item->setFocus();
  else
    _active->setFocus();
    
  _active->setChecked(true);
  _useParameters->setChecked(false);
  _useParametersOnManual->setChecked(false);
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
  _orderGroupFirst->setChecked(false);
  _mpsTimeFence->setValue(0);
    
  sCacheItemType(_itemType);
    
  _locationControl->setChecked(false);
  _useDefaultLocation->setChecked(false);
  _miscLocationName->clear();
  _locationComments->clear();
    
  _costcat->setId(-1);
  
  _purchWarranty->setChecked(false);
  _autoRegister->setChecked(false);
  _tab->setTabEnabled(_tab->indexOf(_expirationTab),false);
    
  populateLocations();
}


void itemSite::sFillRestricted()
{
  XSqlQuery itemFillRestricted;
  int locationid = _restricted->id();
  itemFillRestricted.prepare("SELECT location_id, COALESCE(locitem_id, -1),"
            "       formatLocationName(location_id) AS location_name, firstLine(location_descrip) AS location_descrip,"
            "       (locitem_id IS NOT NULL) AS allowed"
            "  FROM location LEFT OUTER JOIN locitem"
            "         ON (locitem_location_id=location_id AND locitem_item_id=:item_id)"
            " WHERE ((location_restrict)"
            "   AND  (location_warehous_id=:warehouse_id) ) "
            "ORDER BY location_name; ");
  itemFillRestricted.bindValue(":warehouse_id", _warehouse->id());
  itemFillRestricted.bindValue(":item_id", _item->id());
  itemFillRestricted.exec();
  _restricted->populate(itemFillRestricted, locationid, true);
}


void itemSite::sToggleRestricted()
{
  XSqlQuery itemToggleRestricted;
  XTreeWidgetItem * locitem = static_cast<XTreeWidgetItem*>(_restricted->currentItem());
  if(0 == locitem)
    return;

  if(-1 != locitem->altId())
  {
    itemToggleRestricted.prepare("DELETE FROM locitem WHERE (locitem_id=:locitem_id); ");
    itemToggleRestricted.bindValue(":locitem_id", locitem->altId());
    itemToggleRestricted.exec();
  }
  else
  {
    itemToggleRestricted.prepare("INSERT INTO locitem(locitem_location_id, locitem_item_id) VALUES (:location_id, :item_id);");
    itemToggleRestricted.bindValue(":location_id", locitem->id());
    itemToggleRestricted.bindValue(":item_id", _item->id());
    itemToggleRestricted.exec();
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
  else if (whsq.lastError().type() != QSqlError::NoError)
  {
    systemError(pparent, whsq.lastError().databaseText(), __FILE__, __LINE__);
    return -100;
  }
  else
  {
    QMessageBox::warning(pparent, tr("No Site"),
		       tr("<p>The desired Item Site cannot be created as "
			  "there is no Site with the internal ID %1.")
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
	    else if (isq.lastError().type() != QSqlError::NoError)
	    {
	      systemError(pparent, isq.lastError().databaseText(), __FILE__, __LINE__);
	      return -100;
	    }
            return -1; // user cancelled
	  }
	}
	return itemsiteid;
      } // end if successfully copied an itemsite
      else if (isq.lastError().type() != QSqlError::NoError)
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
	isq.prepare("UPDATE itemsite SET itemsite_active = true "
		  "WHERE itemsite_id=:itemsiteid;");
	isq.bindValue(":itemsiteid", itemsiteid);
	isq.exec();
	if (isq.lastError().type() != QSqlError::NoError)
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
  else if (isq.lastError().type() != QSqlError::NoError)
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
    if ( (_controlMethod->code() == "L") ||
        (_controlMethod->code() == "S") )
    {
      _locations_dist->setEnabled(false);
      _recvlocations_dist->setEnabled(false);
      _issuelocations_dist->setEnabled(false);
      _locations_dist->setChecked(false);
      _recvlocations_dist->setChecked(false);
      _issuelocations_dist->setChecked(false);
    }
    else
    {
      _locations_dist->setEnabled(_location->isChecked());
      _recvlocations_dist->setEnabled(_location->isChecked());
      _issuelocations_dist->setEnabled(_location->isChecked());
    }
    _miscLocation->setEnabled(!_locationControl->isChecked());
    _miscLocationName->setEnabled(!_locationControl->isChecked());
  }
  else
  {
    _miscLocation->setEnabled(false);
    _miscLocationName->setEnabled(false);
  }
}

