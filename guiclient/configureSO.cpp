/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureSO.h"

#include <QSqlError>
#include <QMessageBox>
#include <errorReporter.h>
#include <guiErrorCheck.h>

configureSO::configureSO(QWidget* parent, const char* name, bool /*modal*/, Qt::WindowFlags fl)
    : XAbstractConfigure(parent, fl)
{
  XSqlQuery configureconfigureSO;
  setupUi(this);

  if (name)
    setObjectName(name);

  connect(_creditLimit,          SIGNAL(editingFinished()), this, SLOT(sEditCreditLimit()));
  connect(_askUpdatePrice, SIGNAL(toggled(bool)), _ignoreCustDisc, SLOT(setEnabled(bool)));

  _nextSoNumber->setValidator(omfgThis->orderVal());
  _nextQuNumber->setValidator(omfgThis->orderVal());
  _nextRaNumber->setValidator(omfgThis->orderVal());
  _nextCmNumber->setValidator(omfgThis->orderVal());
  _nextInNumber->setValidator(omfgThis->orderVal());
  _creditLimit->setValidator(omfgThis->moneyVal());

  _orderNumGeneration->setMethod(_metrics->value("CONumberGeneration"));
  _quoteNumGeneration->setMethod(_metrics->value("QUNumberGeneration"));
  _creditMemoNumGeneration->setMethod(_metrics->value("CMNumberGeneration"));
  _invoiceNumGeneration->setMethod(_metrics->value("InvcNumberGeneration"));

  QString metric;
  metric = _metrics->value("InvoiceDateSource");
  if (metric == "scheddate")
    _invcScheddate->setChecked(true);
  else if (metric == "shipdate")
    _invcShipdate->setChecked(true);
  else
    _invcCurrdate->setChecked(true);

  configureconfigureSO.exec( "SELECT sonumber.orderseq_number AS sonumber,"
          "       qunumber.orderseq_number AS qunumber,"
          "       cmnumber.orderseq_number AS cmnumber,"
          "       innumber.orderseq_number AS innumber "
          "FROM orderseq AS sonumber,"
          "     orderseq AS qunumber,"
          "     orderseq AS cmnumber,"
          "     orderseq AS innumber "
          "WHERE ( (sonumber.orderseq_name='SoNumber')"
          " AND (qunumber.orderseq_name='QuNumber')"
          " AND (cmnumber.orderseq_name='CmNumber')"
          " AND (innumber.orderseq_name='InvcNumber') );" );
  if (configureconfigureSO.first())
  {
    _nextSoNumber->setText(configureconfigureSO.value("sonumber"));
    _nextQuNumber->setText(configureconfigureSO.value("qunumber"));
    _nextCmNumber->setText(configureconfigureSO.value("cmnumber"));
    _nextInNumber->setText(configureconfigureSO.value("innumber"));
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving SO Setting Information"),
                                configureconfigureSO, __FILE__, __LINE__))
  {
    return;
  }

  _allowDiscounts->setChecked(_metrics->boolean("AllowDiscounts"));
  _allowASAP->setChecked(_metrics->boolean("AllowASAPShipSchedules"));
  _customerChangeLog->setChecked(_metrics->boolean("CustomerChangeLog"));
  _salesOrderChangeLog->setChecked(_metrics->boolean("SalesOrderChangeLog"));
  _restrictCreditMemos->setChecked(_metrics->boolean("RestrictCreditMemos"));
  _autoSelectForBilling->setChecked(_metrics->boolean("AutoSelectForBilling"));
  _saveAndAdd->setChecked(_metrics->boolean("AlwaysShowSaveAndAdd"));
  _firmAndAdd->setChecked(_metrics->boolean("FirmSalesOrderPackingList"));
  _priceOverride->setChecked(_metrics->boolean("DisableSalesOrderPriceOverride"));
  _autoAllocateCM->setChecked(_metrics->boolean("AutoAllocateCreditMemos"));
  _hideSOMiscChrg->setChecked(_metrics->boolean("HideSOMiscCharge"));
  _enableSOShipping->setChecked(_metrics->boolean("EnableSOShipping"));
  _printSO->setChecked(_metrics->boolean("DefaultPrintSOOnSave"));
  _creditCheckSO->setChecked(_metrics->boolean("CreditCheckSOOnSave"));
  _lineItemsSO->setChecked(_metrics->boolean("DefaultSOLineItemsTab"));
  _enablePromiseDate->setChecked(_metrics->boolean("UsePromiseDate"));
  _calcFreight->setChecked(_metrics->boolean("CalculateFreight"));
  _includePkgWeight->setChecked(_metrics->boolean("IncludePackageWeight"));
  _quoteafterSO->setChecked(_metrics->boolean("ShowQuotesAfterSO"));
  _itemPricingPrecedence->setChecked(_metrics->boolean("ItemPricingPrecedence"));
  _useListPriceSchedules->setChecked(_metrics->boolean("AllowListPriceSchedules"));
  _wholesalePriceCosting->setChecked(_metrics->boolean("WholesalePriceCosting"));
  _long30Markups->setChecked(_metrics->boolean("Long30Markups"));

  _shipform->setId(_metrics->value("DefaultShipFormId").toInt());
  _shipvia->setId(_metrics->value("DefaultShipViaId").toInt());
  _shipchrg->setId(_metrics->value("DefaultShipChrgId").toInt());
  _sellingWarehouse->setId(_metrics->value("DefaultSellingWarehouseId").toInt());

  if (_metrics->value("DefaultBalanceMethod") == "B")
    _balanceMethod->setCurrentIndex(0);
  else if (_metrics->value("DefaultBalanceMethod") == "O")
    _balanceMethod->setCurrentIndex(1);

  _custtype->setId(_metrics->value("DefaultCustType").toInt());
  _salesrep->setId(_metrics->value("DefaultSalesRep").toInt());
  _terms->setId(_metrics->value("DefaultTerms").toInt());

  _partial->setChecked(_metrics->boolean("DefaultPartialShipments"));
  _backorders->setChecked(_metrics->boolean("DefaultBackOrders"));
  _freeFormShiptos->setChecked(_metrics->boolean("DefaultFreeFormShiptos"));

  _creditLimit->setText(_metrics->value("SOCreditLimit"));
  _creditRating->setText(_metrics->value("SOCreditRate"));

  if (_metrics->value("soPriceEffective") == "OrderDate")
    _priceOrdered->setChecked(true);
  else if (_metrics->value("soPriceEffective") == "ScheduleDate")
    _priceScheduled->setChecked(true);

  if(_metrics->value("UpdatePriceLineEdit").toInt() == 1)
    _dontUpdatePrice->setChecked(true);
  else if (_metrics->value("UpdatePriceLineEdit").toInt() == 2)
    _askUpdatePrice->setChecked(true);
  else if(_metrics->value("UpdatePriceLineEdit").toInt() == 3)
    _updatePrice->setChecked(true);
  _ignoreCustDisc->setChecked(_askUpdatePrice->isChecked() && _metrics->boolean("IgnoreCustDisc"));

  this->setWindowTitle("Sales Configuration");

  //Set status of Returns Authorization and Reservations based on context
  if(_metrics->value("Application") == "PostBooks")
  {
    _authNumGenerationLit->setVisible(false);
    _returnAuthorizationNumGeneration->setVisible(false);
    _nextRaNumberLit->setVisible(false);
    _nextRaNumber->setVisible(false);
    _tab->removeTab(_tab->indexOf(_returns));
    _enableReturns->setChecked(false);
    _enableReservations->hide();
    _enableReservations->setChecked(false);
    _requireReservations->hide();
    _requireReservations->setChecked(false);
    _locationGroup->hide();
    _locationGroup->setChecked(false);
    _lowest->setChecked(false);
    _highest->setChecked(false);
    _alpha->setChecked(false);
    _manualReservations->setChecked(false);
  }
  else
  {
    configureconfigureSO.exec("SELECT rahead_id FROM rahead LIMIT 1;");
    if (configureconfigureSO.first())
      _enableReturns->setCheckable(false);
    else
      _enableReturns->setChecked(_metrics->boolean("EnableReturnAuth"));

    configureconfigureSO.exec( "SELECT ranumber.orderseq_number AS ranumber "
            "FROM orderseq AS ranumber "
            "WHERE (ranumber.orderseq_name='RaNumber');" );
    if (configureconfigureSO.first())
    {
      _nextRaNumber->setText(configureconfigureSO.value("ranumber"));
    }
    else
      _nextRaNumber->setText("10000");

    metric = _metrics->value("RANumberGeneration");
    if (metric == "M")
      _returnAuthorizationNumGeneration->setCurrentIndex(0);
    else if (metric == "A")
      _returnAuthorizationNumGeneration->setCurrentIndex(1);
    else if (metric == "O")
      _returnAuthorizationNumGeneration->setCurrentIndex(2);

    metric = _metrics->value("DefaultRaDisposition");
    if (metric == "C")
      _disposition->setCurrentIndex(0);
    else if (metric == "R")
      _disposition->setCurrentIndex(1);
    else if (metric == "P")
      _disposition->setCurrentIndex(2);
    else if (metric == "V")
      _disposition->setCurrentIndex(3);
    else if (metric == "M")
      _disposition->setCurrentIndex(4);
    else
      _disposition->setCurrentIndex(5);

    metric = _metrics->value("DefaultRaTiming");
    if (metric == "I")
      _timing->setCurrentIndex(0);
    else if (metric == "R")
      _timing->setCurrentIndex(1);
    else
      _timing->setCurrentIndex(2);

    metric = _metrics->value("DefaultRaCreditMethod");
    if (metric == "N")
      _creditBy->setCurrentIndex(0);
    else if (metric == "M")
      _creditBy->setCurrentIndex(1);
    else if (metric == "K")
      _creditBy->setCurrentIndex(2);
    else if (metric == "C")
      _creditBy->setCurrentIndex(3);
    else
      _creditBy->setCurrentIndex(4);

    _returnAuthChangeLog->setChecked(_metrics->boolean("ReturnAuthorizationChangeLog"));
    _printRA->setChecked(_metrics->boolean("DefaultPrintRAOnSave"));
    _closeRA->setChecked(_metrics->boolean("CloseRAOnCredit"));

    _enableReservations->setChecked(_metrics->boolean("EnableSOReservations"));
    _requireReservations->setChecked(_metrics->boolean("RequireSOReservations"));

    _locationGroup->setChecked(_metrics->boolean("EnableSOReservationsByLocation"));
    _manualReservations->setChecked(_metrics->boolean("SOManualReservations"));
    if(_metrics->value("SOReservationLocationMethod").toInt() == 1)
      _lowest->setChecked(true);
    else if (_metrics->value("SOReservationLocationMethod").toInt() == 2)
      _highest->setChecked(true);
    else if(_metrics->value("SOReservationLocationMethod").toInt() == 3)
      _alpha->setChecked(true);
  }
  
  _ssosCust->setType(CLineEdit::ActiveCustomers);
  _enableSSOS->setChecked(_metrics->boolean("SSOSEnabled"));
  if (_enableSSOS->isChecked())
  {
    _ssosCust->setId(_metrics->value("SSOSDefaultCustId").toInt());
    _ssosSaleType->setId(_metrics->value("SSOSDefaultSaleTypeId").toInt());
    _ssosRequireInv->setChecked(_metrics->boolean("SSOSRequireInv"));
    _ssosPrintSOAck->setChecked(_metrics->boolean("SSOSPrintSOAck"));
    _ssosPrintPackList->setChecked(_metrics->boolean("SSOSPrintPackList"));
    _ssosPrintInvoice->setChecked(_metrics->boolean("SSOSPrintInvoice"));
  }
  else
    _ssosGroup->setEnabled(false);
  
  adjustSize();
}

configureSO::~configureSO()
{
  // no need to delete child widgets, Qt does it all for us
}

void configureSO::languageChange()
{
  retranslateUi(this);
}

bool configureSO::sSave()
{
  XSqlQuery configureSave;
  emit saving();

  const char *dispositionTypes[] = { "C", "R", "P", "V", "M", "" };
  const char *timingTypes[] = { "I", "R", "" };
  const char *creditMethodTypes[] = { "N", "M", "K", "C", "" };

  if ( (_metrics->boolean("EnableSOReservationsByLocation")) &&
       (!_locationGroup->isChecked()) )
  {
    if (QMessageBox::warning(this, tr("Reserve by Location Disabled"),
                             tr("<p>All existing Sales Order location reservations will be removed. Are you sure you want to continue?"),
                             QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      return false;
    }
    else
    {
      configureSave.prepare("DELETE FROM reserve "
                " WHERE (reserve_demand_type='SO');");
      configureSave.exec();
    }	
  }

  _metrics->set("ShowQuotesAfterSO", _quoteafterSO->isChecked());
  _metrics->set("AllowDiscounts", _allowDiscounts->isChecked());
  _metrics->set("AllowASAPShipSchedules", _allowASAP->isChecked());
  _metrics->set("CustomerChangeLog", _customerChangeLog->isChecked());
  _metrics->set("SalesOrderChangeLog", _salesOrderChangeLog->isChecked());
  _metrics->set("RestrictCreditMemos", _restrictCreditMemos->isChecked());
  _metrics->set("AutoSelectForBilling", _autoSelectForBilling->isChecked());
  _metrics->set("AlwaysShowSaveAndAdd", _saveAndAdd->isChecked());
  _metrics->set("FirmSalesOrderPackingList", _firmAndAdd->isChecked());
  _metrics->set("DisableSalesOrderPriceOverride", _priceOverride->isChecked());
  _metrics->set("AutoAllocateCreditMemos", _autoAllocateCM->isChecked());
  _metrics->set("HideSOMiscCharge", _hideSOMiscChrg->isChecked());
  _metrics->set("EnableSOShipping", _enableSOShipping->isChecked());
  _metrics->set("CONumberGeneration",   _orderNumGeneration->methodCode());
  _metrics->set("QUNumberGeneration",   _quoteNumGeneration->methodCode());
  _metrics->set("CMNumberGeneration",   _creditMemoNumGeneration->methodCode());
  _metrics->set("InvcNumberGeneration", _invoiceNumGeneration->methodCode());
  _metrics->set("DefaultShipFormId", _shipform->id());
  _metrics->set("DefaultShipViaId", _shipvia->id());
  _metrics->set("DefaultShipChrgId", _shipchrg->id());
  _metrics->set("DefaultSellingWarehouseId", _sellingWarehouse->id());
  _metrics->set("DefaultCustType", _custtype->id());
  _metrics->set("DefaultSalesRep", _salesrep->id());
  _metrics->set("DefaultTerms", _terms->id());
  _metrics->set("DefaultPartialShipments", _partial->isChecked());
  _metrics->set("DefaultBackOrders", _backorders->isChecked());
  _metrics->set("DefaultFreeFormShiptos", _freeFormShiptos->isChecked());
  _metrics->set("DefaultPrintSOOnSave", _printSO->isChecked());
  _metrics->set("CreditCheckSOOnSave", _creditCheckSO->isChecked());
  _metrics->set("DefaultSOLineItemsTab", _lineItemsSO->isChecked());
  _metrics->set("UsePromiseDate", _enablePromiseDate->isChecked());
  _metrics->set("CalculateFreight", _calcFreight->isChecked());
  _metrics->set("IncludePackageWeight", _includePkgWeight->isChecked());
  _metrics->set("EnableReturnAuth", _enableReturns->isChecked());
  _metrics->set("EnableSOReservations", _enableReservations->isChecked());
  _metrics->set("RequireSOReservations", _enableReservations->isChecked() && _requireReservations->isChecked());
  _metrics->set("ItemPricingPrecedence", _itemPricingPrecedence->isChecked());
  _metrics->set("AllowListPriceSchedules", _useListPriceSchedules->isChecked());
  _metrics->set("WholesalePriceCosting", _wholesalePriceCosting->isChecked());
  _metrics->set("Long30Markups", _long30Markups->isChecked());

  _metrics->set("EnableSOReservationsByLocation", _enableReservations->isChecked() && _locationGroup->isChecked());
  _metrics->set("SOManualReservations", _enableReservations->isChecked() && _locationGroup->isChecked() && _manualReservations->isChecked());
  //SOReservationLocationMethod are three Options Either
  // Lowest quantity first,
  // Highest quantity first,
  // Alpha by Location Name
  if(_lowest->isChecked())
    _metrics->set("SOReservationLocationMethod", 1);
  else if (_highest->isChecked())
    _metrics->set("SOReservationLocationMethod", 2);
  else if(_alpha->isChecked())
    _metrics->set("SOReservationLocationMethod", 3);

  _metrics->set("SOCreditLimit", _creditLimit->text());
  _metrics->set("SOCreditRate", _creditRating->text());

  if (_priceOrdered->isChecked())
    _metrics->set("soPriceEffective", QString("OrderDate"));
  else if (_priceScheduled->isChecked())
    _metrics->set("soPriceEffective", QString("ScheduleDate"));
  else
    _metrics->set("soPriceEffective", QString("CurrentDate"));

  //UpdatePriceLineEdit are three Options Either 
  // Don't Update price
  // Ask to Update Price,
  // Update Price
  if(_dontUpdatePrice->isChecked())
    _metrics->set("UpdatePriceLineEdit", 1);
  else if (_askUpdatePrice->isChecked())
    _metrics->set("UpdatePriceLineEdit", 2);
  else if(_updatePrice->isChecked())
    _metrics->set("UpdatePriceLineEdit", 3);
  _metrics->set("IgnoreCustDisc", _askUpdatePrice->isChecked() && _ignoreCustDisc->isChecked());

  if(_invcScheddate->isChecked())
    _metrics->set("InvoiceDateSource", QString("scheddate"));
  else if(_invcShipdate->isChecked())
    _metrics->set("InvoiceDateSource", QString("shipdate"));
  else
    _metrics->set("InvoiceDateSource", QString("currdate"));

  if (! _invoiceCopies->save())
    return false;
  if (! _creditMemoCopies->save())
    return false;

  switch (_balanceMethod->currentIndex())
  {
  case 0:
    _metrics->set("DefaultBalanceMethod", QString("B"));
    break;

  case 1:
    _metrics->set("DefaultBalanceMethod", QString("O"));
    break;
  }

  configureSave.prepare( "SELECT setNextSoNumber(:sonumber), setNextQuNumber(:qunumber),"
             "       setNextCmNumber(:cmnumber), setNextInvcNumber(:innumber);" );
  configureSave.bindValue(":sonumber", _nextSoNumber->text().toInt());
  configureSave.bindValue(":qunumber", _nextQuNumber->text().toInt());
  configureSave.bindValue(":cmnumber", _nextCmNumber->text().toInt());
  configureSave.bindValue(":innumber", _nextInNumber->text().toInt());
  configureSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving SO Setting Information"),
                                configureSave, __FILE__, __LINE__))
  {
    return false;
  }

  if (_enableReturns->isChecked() || !_enableReturns->isCheckable())
  {
    _metrics->set("DefaultRaDisposition", QString(dispositionTypes[_disposition->currentIndex()]));
    _metrics->set("DefaultRaTiming", QString(timingTypes[_timing->currentIndex()]));
    _metrics->set("DefaultRaCreditMethod", QString(creditMethodTypes[_creditBy->currentIndex()]));
    _metrics->set("ReturnAuthorizationChangeLog", _returnAuthChangeLog->isChecked());
    _metrics->set("DefaultPrintRAOnSave", _printRA->isChecked());
    _metrics->set("CloseRAOnCredit", _closeRA->isChecked());
    _metrics->set("RANumberGeneration", _returnAuthorizationNumGeneration->methodCode());

    configureSave.prepare( "SELECT setNextRaNumber(:ranumber);" );
    configureSave.bindValue(":ranumber", _nextRaNumber->text().toInt());
    configureSave.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving SO Setting Information"),
                                  configureSave, __FILE__, __LINE__))
    {
      return false;
    }
  }
  _metrics->set("EnableReturnAuth", (_enableReturns->isChecked() || !_enableReturns->isCheckable()));
  
  if (_enableSSOS->isChecked())
  {

    QList<GuiErrorCheck>errors;
    errors<<GuiErrorCheck(!_ssosCust->isValid(), _ssosCust,
                          tr("<p>You must select a Cash Customer # if Simple S/O is enabled."));

    if(GuiErrorCheck::reportErrors(this,tr("Unable To Apply Sales Settings"),errors))
        return false;
    
    _metrics->set("SSOSEnabled", true);
    _metrics->set("SSOSDefaultCustId", _ssosCust->id());
    _metrics->set("SSOSDefaultSaleTypeId", _ssosSaleType->id());
    _metrics->set("SSOSRequireInv", _ssosRequireInv->isChecked());
    _metrics->set("SSOSPrintSOAck", _ssosPrintSOAck->isChecked());
    _metrics->set("SSOSPrintPackList", _ssosPrintPackList->isChecked());
    _metrics->set("SSOSPrintInvoice", _ssosPrintInvoice->isChecked());
  }
  else
  {
    _metrics->set("SSOSEnabled", false);
    _metrics->set("SSOSDefaultCustId", -1);
    _metrics->set("SSOSDefaultSaleTypeId", -1);
    _metrics->set("SSOSRequireInv", false);
    _metrics->set("SSOSPrintSOAck", false);
    _metrics->set("SSOSPrintPackList", false);
    _metrics->set("SSOSPrintInvoice", false);
  }

  return true;
}

void configureSO::sEditCreditLimit()
{
  _creditLimit->setDouble((double)qRound(_creditLimit->toDouble()));
}
