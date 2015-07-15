/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureIM.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include "guiclient.h"
#include "errorReporter.h"
#include "storedProcErrorLookup.h"

configureIM::configureIM(QWidget* parent, const char* name, bool /*modal*/, Qt::WindowFlags fl)
    : XAbstractConfigure(parent, fl)
{
  XSqlQuery configureconfigureIM;
  setupUi(this);

  if (name)
    setObjectName(name);

  //Inventory
  //Disable multi-warehouse if PostBooks
  if (_metrics->value("Application") == "PostBooks")
    _multiWhs->hide();
  else
  {
    configureconfigureIM.exec("SELECT * "
	   "FROM whsinfo");
    if (configureconfigureIM.size() > 1)
    {
      _multiWhs->setCheckable(false);
      _multiWhs->setTitle("Multiple Sites");
    }
    else
      _multiWhs->setChecked(_metrics->boolean("MultiWhs"));
      
    if (_metrics->value("TONumberGeneration") == "M")
      _toNumGeneration->setCurrentIndex(0); 
    else if (_metrics->value("TONumberGeneration") == "A")
      _toNumGeneration->setCurrentIndex(1);
    else if (_metrics->value("TONumberGeneration") == "O")
      _toNumGeneration->setCurrentIndex(2);

    _toNextNum->setValidator(omfgThis->orderVal());
    configureconfigureIM.exec( "SELECT orderseq_number AS tonumber "
	    "FROM orderseq "
	    "WHERE (orderseq_name='ToNumber');" );
    if (configureconfigureIM.first())
      _toNextNum->setText(configureconfigureIM.value("tonumber").toString());
    else if (configureconfigureIM.lastError().type() != QSqlError::NoError)
      systemError(this, configureconfigureIM.lastError().databaseText(), __FILE__, __LINE__);

    _enableToShipping->setChecked(_metrics->boolean("EnableTOShipping"));
    _transferOrderChangeLog->setChecked(_metrics->boolean("TransferOrderChangeLog"));
  }

  _eventFence->setValue(_metrics->value("DefaultEventFence").toInt());
  _itemSiteChangeLog->setChecked(_metrics->boolean("ItemSiteChangeLog"));
  _warehouseChangeLog->setChecked(_metrics->boolean("WarehouseChangeLog"));
  
  if (_metrics->boolean("PostCountTagToDefault"))
    _postToDefault->setChecked(true);
  else
    _doNotPost->setChecked(true);

  _defaultTransWhs->setId(_metrics->value("DefaultTransitWarehouse").toInt());

  QString countSlipAuditing = _metrics->value("CountSlipAuditing");
  if (countSlipAuditing == "N")
    _noSlipChecks->setChecked(true);
  else if (countSlipAuditing == "W")
    _checkOnUnpostedWarehouse->setChecked(true);
  else if (countSlipAuditing == "A")
    _checkOnUnposted->setChecked(true);
  else if (countSlipAuditing == "X")
    _checkOnAllWarehouse->setChecked(true);
  else if (countSlipAuditing == "B")
    _checkOnAll->setChecked(true);
    
  QString avgCostingMethod = _metrics->value("CountAvgCostMethod");
  if (avgCostingMethod == "STD")
    _useStdCost->setChecked(true);
  else if (avgCostingMethod == "ACT")
  {
    _useAvgCost->setChecked(true);
    _useActCost->setChecked(true);
  }
  else if (avgCostingMethod == "AVG")
    _useAvgCost->setChecked(true);
  else
    _useStdCost->setChecked(true);
    
  if(_metrics->value("Application") != "PostBooks")
  {
    configureconfigureIM.exec("SELECT DISTINCT itemsite_controlmethod "
	      "FROM itemsite "
	      "WHERE (itemsite_controlmethod IN ('L','S'));");
    if (configureconfigureIM.first())
    {
      _lotSerial->setChecked(true);
      _lotSerial->setEnabled(false);
    }
    else
      _lotSerial->setChecked(_metrics->boolean("LotSerialControl"));
  }
  else
    _lotSerial->hide();
  _setDefLoc->setChecked(_metrics->boolean("SetDefaultLocations"));

// Shipping and Receiving
  QString metric = _metrics->value("ShipmentNumberGeneration");
  if (metric == "A")
    _shipmentNumGeneration->setCurrentIndex(0);

  _nextShipmentNum->setValidator(omfgThis->orderVal());
  configureconfigureIM.exec("SELECT setval('shipment_number_seq', nextval('shipment_number_seq') -1); "
         "SELECT currval('shipment_number_seq') AS shipment_number;");
  if (configureconfigureIM.first())
    _nextShipmentNum->setText(configureconfigureIM.value("shipment_number"));
  else if (configureconfigureIM.lastError().type() != QSqlError::NoError)
    systemError(this, configureconfigureIM.lastError().databaseText(), __FILE__, __LINE__);

  _kitInheritCOS->setChecked(_metrics->boolean("KitComponentInheritCOS"));
  _disallowReceiptExcess->setChecked(_metrics->boolean("DisallowReceiptExcessQty"));
  _warnIfReceiptDiffers->setChecked(_metrics->boolean("WarnIfReceiptQtyDiffers"));
  _recordPpvOnReceipt->setChecked(_metrics->boolean("RecordPPVonReceipt"));

  _tolerance->setValidator(omfgThis->percentVal());
  _tolerance->setText(_metrics->value("ReceiptQtyTolerancePct"));

  _costAvg->setChecked(_metrics->boolean("AllowAvgCostMethod"));
  _costStd->setChecked(_metrics->boolean("AllowStdCostMethod"));
  _costJob->setChecked(_metrics->boolean("AllowJobCostMethod"));

  configureconfigureIM.prepare("SELECT count(*) AS result FROM itemsite WHERE(itemsite_costmethod='A');");
  configureconfigureIM.exec();
  if(configureconfigureIM.first() && configureconfigureIM.value("result").toInt() > 0)
  {
    _costAvg->setChecked(true);
    _costAvg->setEnabled(false);
  }

  configureconfigureIM.prepare("SELECT count(*) AS result FROM itemsite WHERE(itemsite_costmethod='S');");
  configureconfigureIM.exec();
  if(configureconfigureIM.first() && configureconfigureIM.value("result").toInt() > 0)
  {
    _costStd->setChecked(true);
    _costStd->setEnabled(false);
  }

  if(!_costAvg->isChecked() && !_costStd->isChecked())
    _costStd->isChecked();

  _asOfQOH->setChecked(_metrics->boolean("EnableAsOfQOH"));
  
  // Jobs at this time should always be checked and disabled
  // when this is changed in the future this should be replaced with
  // similar code checks as for Avg and Std cost
  _costJob->setChecked(true);
  _costJob->setEnabled(false);

  // Receipt Cost Override
  _receiptCostOverride->setChecked(_metrics->boolean("AllowReceiptCostOverride"));
  connect(_receiptCostOverride, SIGNAL(toggled(bool)), this, SLOT(sReceiptCostOverrideWarning()));

  this->setWindowTitle("Inventory Configuration");

  // Requires Consolidated shipping package
  _shipByGroup->hide();

  adjustSize();
}

configureIM::~configureIM()
{
    // no need to delete child widgets, Qt does it all for us
}

void configureIM::languageChange()
{
    retranslateUi(this);
}

bool configureIM::sSave()
{
  XSqlQuery configureSave;
  emit saving();

  if(!_costAvg->isChecked() && !_costStd->isChecked())
  { 
    QMessageBox::warning(this, tr("No Cost selected"),
                         tr("<p>You must have checked Standard Cost, "
                            "Average Cost or both before saving."));
    return false;
  }

  // Inventory
  _metrics->set("DefaultEventFence", _eventFence->value());
  _metrics->set("ItemSiteChangeLog", _itemSiteChangeLog->isChecked());
  _metrics->set("WarehouseChangeLog", _warehouseChangeLog->isChecked());
  _metrics->set("PostCountTagToDefault", _postToDefault->isChecked());
  _metrics->set("MultiWhs", ((!_multiWhs->isCheckable()) || (_multiWhs->isChecked())));
  _metrics->set("LotSerialControl", _lotSerial->isChecked());
  _metrics->set("SetDefaultLocations", _setDefLoc->isChecked());
  _metrics->set("AllowAvgCostMethod", _costAvg->isChecked());
  _metrics->set("AllowStdCostMethod", _costStd->isChecked());
  _metrics->set("AllowJobCostMethod", _costJob->isChecked());
  _metrics->set("AllowReceiptCostOverride", _receiptCostOverride->isChecked());

  if (_toNumGeneration->currentIndex() == 0)
    _metrics->set("TONumberGeneration", QString("M"));
  else if (_toNumGeneration->currentIndex() == 1)
    _metrics->set("TONumberGeneration", QString("A"));
  else if (_toNumGeneration->currentIndex() == 2)
    _metrics->set("TONumberGeneration", QString("O"));

  if (_metrics->boolean("MultiWhs"))
  {
    configureSave.prepare("SELECT setNextNumber('ToNumber', :next) AS result;");
    configureSave.bindValue(":next", _toNextNum->text());
    configureSave.exec();
    if (configureSave.first())
    {
      int result = configureSave.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("setNextNumber", result), __FILE__, __LINE__);
        _toNextNum->setFocus();
        return false;
      }
    }
    else if (configureSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, configureSave.lastError().databaseText(), __FILE__, __LINE__);
      _toNextNum->setFocus();
      return false;
    }

    _metrics->set("DefaultTransitWarehouse", _defaultTransWhs->id());
    _metrics->set("EnableTOShipping",	     _enableToShipping->isChecked());
    _metrics->set("TransferOrderChangeLog",  _transferOrderChangeLog->isChecked());
  }

  if (_noSlipChecks->isChecked())
    _metrics->set("CountSlipAuditing", QString("N"));
  else if (_checkOnUnpostedWarehouse->isChecked())
    _metrics->set("CountSlipAuditing", QString("W"));
  else if (_checkOnUnposted->isChecked())
    _metrics->set("CountSlipAuditing", QString("A"));
  else if (_checkOnAllWarehouse->isChecked())
    _metrics->set("CountSlipAuditing", QString("X"));
  else if (_checkOnAll->isChecked())
    _metrics->set("CountSlipAuditing", QString("B"));
    
  if (_useStdCost->isChecked())
    _metrics->set("CountAvgCostMethod", QString("STD"));
  else if (_useActCost->isChecked())
    _metrics->set("CountAvgCostMethod", QString("ACT"));
  else if (_useAvgCost->isChecked())
    _metrics->set("CountAvgCostMethod", QString("AVG"));

  //Shipping and Receiving
  const char *numberGenerationTypes[] = { "A" };

  _metrics->set("ShipmentNumberGeneration", QString(numberGenerationTypes[_shipmentNumGeneration->currentIndex()]));

  if (! _shipformCopies->save())
    return false;

  _metrics->set("KitComponentInheritCOS", _kitInheritCOS->isChecked());
  _metrics->set("DisallowReceiptExcessQty", _disallowReceiptExcess->isChecked());
  _metrics->set("WarnIfReceiptQtyDiffers", _warnIfReceiptDiffers->isChecked());
  _metrics->set("ReceiptQtyTolerancePct", _tolerance->text());
  _metrics->set("RecordPPVonReceipt", _recordPpvOnReceipt->isChecked());

  configureSave.prepare("SELECT setval('shipment_number_seq', :shipmentnumber);");
  configureSave.bindValue(":shipmentnumber", _nextShipmentNum->text().toInt());
  configureSave.exec();
  if (configureSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, configureSave.lastError().databaseText(), __FILE__, __LINE__);
    _nextShipmentNum->setFocus();
    return false;
  }


  if(_asOfQOH->isChecked() && !_metrics->boolean("EnableAsOfQOH"))
  {
    if(QMessageBox::question(this, tr("Enable As-Of QOH Reporting"),
                          tr("<p>Enabling As-Of QOH reporting requires some processing to occur "
                             "for it to work correctly. This may take some time. Would you like to continue?"),
                          QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
      XSqlQuery qq;
      qq.exec("SELECT buildInvbal(itemsite_id)"
              "  FROM itemsite;");
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Enabing As-Of Reporting"),
                               qq, __FILE__, __LINE__))
        return false;

    }
    else
      return false;
  }
  _metrics->set("EnableAsOfQOH", _asOfQOH->isChecked());

  return true;
}

void configureIM::sReceiptCostOverrideWarning()
{
  if (!_receiptCostOverride->isChecked())
    QMessageBox::warning(this, tr("Receipt Cost Override Warning"),
                         tr("Unposted or uninvoiced receipts could be negatively affected if this feature is disabled."));
}
