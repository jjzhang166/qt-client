/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesOrderItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "itemCharacteristicDelegate.h"
#include "itemSourceList.h"
#include "maintainItemCosts.h"
#include "openPurchaseOrder.h"
#include "priceList.h"
#include "reserveSalesOrderItem.h"
#include "storedProcErrorLookup.h"
#include "taxDetail.h"
#include "woMaterialItem.h"
#include "xdoublevalidator.h"

#define cNewQuote   (0x20 | cNew)
#define cEditQuote  (0x20 | cEdit)
#define cViewQuote  (0x20 | cView)

#define ISQUOTE(mode) (((mode) & 0x20) == 0x20)
#define ISORDER(mode) (!ISQUOTE(mode))

#define iDontUpdate   1
#define iAskToUpdate  2
#define iJustUpdate   3

salesOrderItem::salesOrderItem(QWidget *parent, const char *name, Qt::WindowFlags fl)
  : XDialog(parent, name, fl)
{
  setupUi(this);

  connect(_item,              SIGNAL(newId(int)),                   this, SLOT(sPopulateItemInfo(int)));
  connect(_item,              SIGNAL(newId(int)),                   this, SLOT(sPopulateItemSources(int)));
  connect(_item,              SIGNAL(newId(int)),                   this, SLOT(sPopulateItemSubs(int)));
  connect(_item,              SIGNAL(newId(int)),                   this, SLOT(sPopulateHistory()));
  connect(_item,              SIGNAL(newId(int)),                   this, SLOT(sDetermineAvailability()));
  connect(_listPrices,        SIGNAL(clicked()),                    this, SLOT(sListPrices()));
  connect(_netUnitPrice,      SIGNAL(idChanged(int)),               this, SLOT(sPriceGroup()));
  connect(_netUnitPrice,      SIGNAL(valueChanged()),               this, SLOT(sCalculateExtendedPrice()));
  connect(_qtyOrdered,        SIGNAL(editingFinished()),            this, SLOT(sDetermineAvailability()));
  connect(_qtyOrdered,        SIGNAL(editingFinished()),            this, SLOT(sDeterminePrice()));
  connect(_qtyOrdered,        SIGNAL(editingFinished()),            this, SLOT(sCalcUnitCost()));
  connect(_save,              SIGNAL(clicked()),                    this, SLOT(sSaveClicked()));
  connect(_scheduledDate,     SIGNAL(newDate(const QDate &)),       this, SLOT(sHandleScheduleDate()));
  connect(_showAvailability,  SIGNAL(toggled(bool)),                this, SLOT(sDetermineAvailability()));
  connect(_showIndented,      SIGNAL(toggled(bool)),                this, SLOT(sDetermineAvailability()));
  connect(_warehouse,         SIGNAL(newID(int)),                   this, SLOT(sPopulateItemsiteInfo()));
  connect(_warehouse,         SIGNAL(newID(int)),                   this, SLOT(sDetermineAvailability()));
  connect(_warehouse,         SIGNAL(newID(int)),                   this, SLOT(sPopulateItemSubs(int)));
  connect(_subs,              SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateSubMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_next,              SIGNAL(clicked()),                    this, SLOT(sNext()));
  connect(_prev,              SIGNAL(clicked()),                    this, SLOT(sPrev()));
  connect(_notes,             SIGNAL(textChanged()),                this, SLOT(sChanged()));
  connect(_promisedDate,      SIGNAL(newDate(const QDate &)),       this, SLOT(sChanged()));
  connect(_scheduledDate,     SIGNAL(newDate(const QDate &)),       this, SLOT(sChanged()));
  connect(_netUnitPrice,      SIGNAL(valueChanged()),               this, SLOT(sChanged()));
  connect(_qtyOrdered,        SIGNAL(editingFinished()),            this, SLOT(sChanged()));
  connect(_warehouse,         SIGNAL(newID(int)),                   this, SLOT(sChanged()));
  connect(_item,              SIGNAL(newId(int)),                   this, SLOT(sChanged()));
  connect(_qtyUOM,            SIGNAL(newID(int)),                   this, SLOT(sChanged()));
  connect(_cancel,            SIGNAL(clicked()),                    this, SLOT(sCancel()));
  connect(_extendedPrice,     SIGNAL(valueChanged()),               this, SLOT(sLookupTax()));
  connect(_taxLit,            SIGNAL(leftClickedURL(QString)),      this, SLOT(sTaxDetail()));
  connect(_taxtype,           SIGNAL(newID(int)),                   this, SLOT(sLookupTax()));
  connect(_qtyUOM,            SIGNAL(newID(int)),                   this, SLOT(sQtyUOMChanged()));
  connect(_priceUOM,          SIGNAL(newID(int)),                   this, SLOT(sPriceUOMChanged()));
  connect(_inventoryButton,   SIGNAL(toggled(bool)),                this, SLOT(sHandleButton()));
  connect(_itemSourcesButton, SIGNAL(toggled(bool)),                this, SLOT(sHandleButton()));
  connect(_supplyOrderButton, SIGNAL(toggled(bool)),                this, SLOT(sHandleButton()));
  connect(_substitutesButton, SIGNAL(toggled(bool)),                this, SLOT(sHandleButton()));
  connect(_historyCostsButton,SIGNAL(toggled(bool)),                this, SLOT(sHandleButton()));
  connect(_historyCostsButton,SIGNAL(toggled(bool)),                this, SLOT(sPopulateHistory()));
  connect(_historyDates,      SIGNAL(updated()),                    this, SLOT(sPopulateHistory()));

#ifndef Q_WS_MAC
  _listPrices->setMaximumWidth(25);
  _subItemList->setMaximumWidth(25);
#endif

  _leadTime              = 999;
  _shiptoid              = -1;
  _preferredWarehouseid  = -1;
  _modified              = false;
  _canceling             = false;
  _partialsaved          = false;
  _error                 = false;
  _stocked               = false;
  _originalQtyOrd        = 0.0;
  _updateItemsite        = false;
  _updatePrice           = true;
  _qtyinvuomratio        = 1.0;
  _priceinvuomratio      = 1.0;
  _priceRatio            = 1.0;
  _invuomid              = -1;
  _invIsFractional       = false;
  _qtyreserved           = 0.0;
  _priceType             = "N";  // default to nominal
  _priceMode             = "D";  // default to discount

  _authNumber->hide();
  _authNumberLit->hide();
  _authLineNumber->hide();
  _authLineNumberLit->hide();

  _availabilityLastItemid      = -1;
  _availabilityLastWarehousid  = -1;
  _availabilityLastSchedDate   = QDate();
  _availabilityLastShow        = false;
  _availabilityQtyOrdered      = 0.0;

  _charVars << -1 << -1 << -1 << 0 << -1 << omfgThis->dbDate();

  //  Configure some Widgets
  _item->setType(ItemLineEdit::cSold | ItemLineEdit::cActive);
  _item->addExtraClause( QString("(itemsite_active)") );  // ItemLineEdit::cActive doesn't compare against the itemsite record
  _item->addExtraClause( QString("(itemsite_sold)") );    // ItemLineEdit::cSold doesn't compare against the itemsite record

  _taxtype->setEnabled(_privileges->check("OverrideTax"));

  _availability->addColumn(tr("#"),             _seqColumn, Qt::AlignCenter,true, "seqnumber");
  _availability->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,  true, "item_number");
  _availability->addColumn(tr("Description"),           -1, Qt::AlignLeft,  true, "item_descrip");
  _availability->addColumn(tr("UOM"),           _uomColumn, Qt::AlignCenter,true, "uom_name");
  _availability->addColumn(tr("Pend. Alloc."),  _qtyColumn, Qt::AlignRight, true, "pendalloc");
  _availability->addColumn(tr("Total Alloc."),  _qtyColumn, Qt::AlignRight, true, "totalalloc");
  _availability->addColumn(tr("On Order"),      _qtyColumn, Qt::AlignRight, true, "ordered");
  _availability->addColumn(tr("Available QOH"), _qtyColumn, Qt::AlignRight, true, "availableqoh");
  _availability->addColumn(tr("Availability"),  _qtyColumn, Qt::AlignRight, true, "totalavail");

  _itemsrcp->addColumn(tr("Vendor #"),    _itemColumn, Qt::AlignLeft, true, "vend_number");
  _itemsrcp->addColumn(tr("Vendor Name"),          -1, Qt::AlignLeft, true, "vend_name");
  _itemsrcp->addColumn(tr("Description"),          -1, Qt::AlignLeft, true, "itemsrc_descrip");
  _itemsrcp->addColumn(tr("Qty Break"),    _qtyColumn, Qt::AlignRight,true, "itemsrcp_qtybreak");
  _itemsrcp->addColumn(tr("Base Price"), _moneyColumn, Qt::AlignRight,true, "price_base");

  _subs->addColumn(tr("Site"),             _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _subs->addColumn(tr("Item Number"),     _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _subs->addColumn(tr("Description"),              -1,  Qt::AlignLeft,   true,  "itemdescrip"   );
  _subs->addColumn(tr("LT"),               _whsColumn,  Qt::AlignCenter, true,  "leadtime" );
  _subs->addColumn(tr("Available QOH"),    _qtyColumn,  Qt::AlignRight,  true,  "availableqoh"  );
  _subs->addColumn(tr("Allocated"),        _qtyColumn,  Qt::AlignRight,  true,  "allocated"  );
  _subs->addColumn(tr("On Order"),         _qtyColumn,  Qt::AlignRight,  true,  "ordered"  );
  _subs->addColumn(tr("Reorder Lvl."),     _qtyColumn,  Qt::AlignRight,  true,  "reorderlevel"  );
  _subs->addColumn(tr("Available"),        _qtyColumn,  Qt::AlignRight,  true,  "available"  );

  _historyDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _historyDates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _historyCosts->addColumn(tr("P/O #"),        _orderColumn, Qt::AlignRight, true, "ponumber");
  _historyCosts->addColumn(tr("Vendor"),       120,          Qt::AlignLeft,  true, "vend_name");
  _historyCosts->addColumn(tr("Due Date"),     _dateColumn,  Qt::AlignCenter,true, "duedate");
  _historyCosts->addColumn(tr("Recv. Date"),   _dateColumn,  Qt::AlignCenter,true, "recvdate");
  _historyCosts->addColumn(tr("Vend. Item #"), _itemColumn,  Qt::AlignLeft,  true, "venditemnumber");
  _historyCosts->addColumn(tr("Description"),  -1,           Qt::AlignLeft,  true, "venditemdescrip");
  _historyCosts->addColumn(tr("Rcvd/Rtnd"),    _qtyColumn,   Qt::AlignRight, true, "sense");
  _historyCosts->addColumn(tr("Qty."),         _qtyColumn,   Qt::AlignRight, true, "qty");
  if (_privileges->check("ViewCosts"))
  {
    _historyCosts->addColumn(tr("Purch. Cost"),_priceColumn, Qt::AlignRight,true, "purchcost");
    _historyCosts->addColumn(tr("Recv. Cost"), _priceColumn, Qt::AlignRight,true, "recvcost");
    _historyCosts->addColumn(tr("Value"),      _priceColumn, Qt::AlignRight,true, "value");
  }

  _historySales->addColumn(tr("Customer"),            -1,              Qt::AlignLeft,   true,  "cust_name"   );
  _historySales->addColumn(tr("Doc. #"),              _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  _historySales->addColumn(tr("Invoice #"),           _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"   );
  _historySales->addColumn(tr("Ord. Date"),           _dateColumn,     Qt::AlignCenter, true,  "cohist_orderdate" );
  _historySales->addColumn(tr("Invc. Date"),          _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  _historySales->addColumn(tr("Item Number"),         _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  _historySales->addColumn(tr("Description"),         -1,              Qt::AlignLeft,   true,  "itemdescription"   );
  _historySales->addColumn(tr("Shipped"),             _qtyColumn,      Qt::AlignRight,  true,  "cohist_qtyshipped"  );
  if (_privileges->check("ViewCustomerPrices"))
  {
    _historySales->addColumn(tr("Unit Price"),        _priceColumn,    Qt::AlignRight,  true,  "cohist_unitprice" );
    _historySales->addColumn(tr("Ext. Price"),        _bigMoneyColumn, Qt::AlignRight,  true,  "extprice" );
    _historySales->addColumn(tr("Currency"),          _currencyColumn, Qt::AlignRight,  true,  "currAbbr" );
    _historySales->addColumn(tr("Base Unit Price"),   _bigMoneyColumn, Qt::AlignRight,  true,  "baseunitprice" );
    _historySales->addColumn(tr("Base Ext. Price"),   _bigMoneyColumn, Qt::AlignRight,  true,  "baseextprice" );
  }
  if (_privileges->check("ViewCosts"))
  {
    _historySales->addColumn(tr("Unit Cost"),         _costColumn,     Qt::AlignRight,  true,  "cohist_unitcost" );
    _historySales->addColumn(tr("Ext. Cost"),         _bigMoneyColumn, Qt::AlignRight,  true,  "extcost" );
  }

  _woIndentedList->addColumn(tr("Order#"),          _orderColumn,   Qt::AlignLeft      , true,   "wonumber");
  _woIndentedList->addColumn(tr("Item#"),           _itemColumn,    Qt::AlignLeft      , true,   "wodata_itemnumber" );
  _woIndentedList->addColumn(tr("Description"),      -1,            Qt::AlignLeft      , true,   "wodata_descrip");
  _woIndentedList->addColumn(tr("Status"),          _statusColumn,  Qt::AlignCenter    , true,   "wodata_status");
  _woIndentedList->addColumn(tr("Cust. Price"),     _priceColumn,   Qt::AlignRight     , true,   "wodata_custprice");
  _woIndentedList->addColumn(tr("List Price"),      _priceColumn,   Qt::AlignRight     , true,   "wodata_listprice");
  _woIndentedList->addColumn(tr("Ord/Req."),        _qtyColumn,     Qt::AlignRight     , true,   "qtyordreq");
  _woIndentedList->addColumn(tr("UOM"),             _uomColumn,     Qt::AlignLeft      , true,   "wodata_qtyuom");
  if (_metrics->boolean("Routings"))
  {
    _woIndentedList->addColumn(tr("Setup Remain."),           _qtyColumn,     Qt::AlignRight     , false,  "wodata_setup");
    _woIndentedList->addColumn(tr("Run Remain."),             _qtyColumn,     Qt::AlignRight     , false,  "wodata_run");
  }
  _woIndentedList->addColumn(tr("Start Date"),      _dateColumn,    Qt::AlignCenter    , false,  "wodata_startdate");
  _woIndentedList->addColumn(tr("Due Date"),        _dateColumn,    Qt::AlignCenter    , true,   "wodata_duedate");

  _itemchar = new QStandardItemModel(0, 3, this);
  _itemchar->setHeaderData( CHAR_ID, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
  _itemchar->setHeaderData( CHAR_VALUE, Qt::Horizontal, tr("Value"), Qt::DisplayRole);
  _itemchar->setHeaderData( CHAR_PRICE, Qt::Horizontal, tr("Price"), Qt::DisplayRole);

  _itemcharView->hideColumn(CHAR_PRICE);
  _baseUnitPriceLit->hide();
  _baseUnitPrice->hide();

  _itemcharView->setModel(_itemchar);
  ItemCharacteristicDelegate *delegate = new ItemCharacteristicDelegate(this);
  _itemcharView->setItemDelegate(delegate);

  if (!_metrics->boolean("UsePromiseDate"))
  {
    _promisedDateLit->hide();
    _promisedDate->hide();
  }

  if (!_preferences->boolean("ClusterButtons"))
    _subItemList->hide();

  _qtyOrdered->setValidator(omfgThis->qtyVal());
  _supplyOrderQty->setValidator(omfgThis->qtyVal());
  _discountFromCust->setValidator(omfgThis->negPercentVal());

  _shippedToDate->setPrecision(omfgThis->qtyVal());
  _discountFromListPrice->setValidator(omfgThis->percentVal());
  _markupFromUnitCost->setValidator(omfgThis->percentVal());
  _onHand->setPrecision(omfgThis->qtyVal());
  _allocated->setPrecision(omfgThis->qtyVal());
  _unallocated->setPrecision(omfgThis->qtyVal());
  _onOrder->setPrecision(omfgThis->qtyVal());
  _available->setPrecision(omfgThis->qtyVal());

  if (_metrics->boolean("EnableSOReservations"))
  {
    _reserved->setPrecision(omfgThis->qtyVal());
    _reservable->setPrecision(omfgThis->qtyVal());
  }
  else
  {
    _reserved->hide();
    _reservable->hide();
    _reserveOnSave->setChecked(false);
  }
  _reserveOnSave->hide();

  //  Disable the Discount Percent stuff if we don't allow them
  if ((!_metrics->boolean("AllowDiscounts")) && (!_privileges->check("OverridePrice")))
  {
    _netUnitPrice->setEnabled(FALSE);
    _discountFromCust->setEnabled(FALSE);
    _unitCost->setEnabled(FALSE);
    _markupFromUnitCost->setEnabled(FALSE);
  }
  if (!_privileges->check("ViewCosts"))
  {
    _unitCost->hide();
    _unitCostLit->hide();
    _markupFromUnitCost->hide();
    _markupFromUnitCostLit->hide();
  }

  if (!_privileges->check("ShowMarginsOnSalesOrder"))
  {
    _margin->hide();
    _marginLit->hide();
  }
  
  if ((_metrics->boolean("DisableSalesOrderPriceOverride")) || (!_privileges->check("OverridePrice")))
  {
    _netUnitPrice->setEnabled(false);
  }

  _supplyOrderType = "";
  _supplyOrderId = -1;
  _supplyOrderQtyCache = 0.0;
  _supplyOrderQtyOrderedCache = 0.0;
  _supplyOrderQtyOrderedInvCache = 0.0;
  _supplyOrderDueDateCache = QDate();
  _supplyOrderScheduledDateCache = QDate();
  _supplyOrderDropShipCache = false;
  _supplyOverridePriceCache = 0.0;
  _itemsrc = -1;
  _taxzoneid   = -1;
  _initialMode = -1;

  sPriceGroup();

  if (!_metrics->boolean("EnableReturnAuth"))
    _warranty->hide();

  if (_metrics->value("soPriceEffective") == "ScheduleDate")
  {
    connect(_scheduledDate, SIGNAL(newDate(QDate)), this, SLOT(sHandleScheduleDate()));
    connect(_item,          SIGNAL(newId(int)),     this, SLOT(sHandleScheduleDate()));
  }

    adjustSize();

  _inventoryButton->setEnabled(_showAvailability->isChecked());
  _itemSourcesButton->setEnabled(_showAvailability->isChecked());
  _supplyOrderButton->setEnabled(_showAvailability->isChecked());
  _substitutesButton->setEnabled(_showAvailability->isChecked());
  _availability->setEnabled(_showAvailability->isChecked());
  _showIndented->setEnabled(_showAvailability->isChecked());

  _altCosAccnt->setType(GLCluster::cRevenue | GLCluster::cExpense);
  _altRevAccnt->setType(GLCluster::cRevenue);

  // TO DO **** Fix tab order issues and offer alternate means for "Express Tab Order"  ****
}

salesOrderItem::~salesOrderItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesOrderItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesOrderItem:: set(const ParameterList &pParams)
{
  XSqlQuery setSales;
  XDialog::set(pParams);
  QVariant  param;
  bool      valid;

  _prev->setEnabled(true);
  _next->setEnabled(true);
  _next->setText(tr("Next"));

  param = pParams.value("sohead_id", &valid);
  if (valid)
    _soheadid = param.toInt();
  else
    _soheadid = -1;

  param = pParams.value("taxzone_id", &valid);
  if (valid)
    _taxzoneid = param.toInt();

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _custid = param.toInt();
    setSales.prepare("SELECT COALESCE(cust_preferred_warehous_id, -1) AS preferredwarehousid, "
              "(cust_number || '-' || cust_name) as f_name "
              "  FROM custinfo"
              " WHERE (cust_id=:cust_id); ");
    setSales.bindValue(":cust_id", _custid);
    setSales.exec();
    if (setSales.first())
    {
      if (setSales.value("preferredwarehousid").toInt() != -1)
        _preferredWarehouseid = setSales.value("preferredwarehousid").toInt();
      _custName = setSales.value("f_name").toString();
    }
    else if (setSales.lastError().type() != QSqlError::NoError)
    {
      systemError(this, setSales.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
    _charVars.replace(CUST_ID, param.toInt());
  }

  param = pParams.value("shipto_id", &valid);
  if (valid)
  {
    _shiptoid = param.toInt();
    _charVars.replace(SHIPTO_ID, param.toInt());
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    int _warehouseid = param.toInt();
    _warehouse->setId(-1);
    _warehouse->setId(_warehouseid);
    _preferredWarehouseid = _warehouseid;
  }

  param = pParams.value("orderNumber", &valid);
  if (valid)
    _orderNumber->setText(param.toString());

  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _netUnitPrice->setId(param.toInt());
    _tax->setId(param.toInt());
    _charVars.replace(CURR_ID, param.toInt());
  }

  param = pParams.value("orderDate", &valid);
  if (valid)
  {
    _netUnitPrice->setEffective(param.toDate());
    _charVars.replace(EFFECTIVE, param.toDate());
  }

  param = pParams.value("shipDate", &valid);
  if (valid)
    _scheduledDate->setDate(param.toDate());

  if (_metrics->boolean("AllowASAPShipSchedules") && !_scheduledDate->isValid())
    _scheduledDate->setDate(QDate::currentDate());
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
    QDate asOf;

    if (_metrics->value("soPriceEffective") == "ScheduleDate")
      asOf = _scheduledDate->date();
    else if (_metrics->value("soPriceEffective") == "OrderDate")
      asOf = _netUnitPrice->effective();
    else
      asOf = omfgThis->dbDate();

    if (param.toString() == "new")
    {
      _mode = cNew;

      _save->setEnabled(false);
      _next->setText(tr("New"));
      _comments->setType(Comments::SalesOrderItem);
      _comments->setReadOnly(false);
      _item->setReadOnly(false);
      _warehouse->setEnabled(true);
      _cancel->setEnabled(false);
      _supplyOrderType = "";
      _supplyOrderId = -1;
      _itemsrc = -1;

      _item->addExtraClause( QString("(item_id IN (SELECT custitem FROM custitem(%1, %2, '%3') ) )").arg(_custid).arg(_shiptoid).arg(asOf.toString(Qt::ISODate)) );

      prepare();

      connect(_netUnitPrice,      SIGNAL(editingFinished()),    this,         SLOT(sCalculateDiscountPrcnt()));
      connect(_discountFromCust,  SIGNAL(editingFinished()),    this,         SLOT(sCalculateFromDiscount()));
      connect(_unitCost,          SIGNAL(editingFinished()),    this,         SLOT(sCalculateFromMarkup()));
      connect(_markupFromUnitCost,SIGNAL(editingFinished()),    this,         SLOT(sCalculateFromMarkup()));
      connect(_item,              SIGNAL(valid(bool)),          _listPrices,  SLOT(setEnabled(bool)));
      connect(_createSupplyOrder, SIGNAL(toggled(bool)),        this,         SLOT(sHandleSupplyOrder()));

      setSales.prepare("SELECT count(*) AS cnt"
                "  FROM coitem"
                " WHERE (coitem_cohead_id=:sohead_id);");
      setSales.bindValue(":sohead_id", _soheadid);
      setSales.exec();
      if (!setSales.first() || setSales.value("cnt").toInt() == 0)
        _prev->setEnabled(false);
      if (setSales.lastError().type() != QSqlError::NoError)
      {
        systemError(this, setSales.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      
      if (_metrics->boolean("EnableSOReservations"))
        _reserveOnSave->show();
    }
    else if (param.toString() == "newQuote")
    {
      _mode = cNewQuote;
      //  TODO - quotes different from sales orders?
      //      _item->setType(ItemLineEdit::cSold | ItemLineEdit::cItemActive);
      //      _item->clearExtraClauseList();

      setWindowTitle(tr("Quote Item"));

      _save->setEnabled(FALSE);
      _next->setText(tr("New"));
      _comments->setType(Comments::QuoteItem);
      _comments->setReadOnly(true);
      _cancel->hide();
      _sub->hide();
      _subItem->hide();
      _subItemList->hide();
      _item->setReadOnly(false);
      _warehouse->setEnabled(true);
      _supplyOrderId = -1;
      _itemsrc = -1;
      _warranty->hide();
      _tabs->removeTab(_tabs->indexOf(_costofsalesTab));

      _item->addExtraClause( QString("(item_id IN (SELECT custitem FROM custitem(%1, %2, '%3') ) )").arg(_custid).arg(_shiptoid).arg(asOf.toString(Qt::ISODate)) );

      prepare();

      connect(_netUnitPrice,      SIGNAL(editingFinished()),    this,         SLOT(sCalculateDiscountPrcnt()));
      connect(_discountFromCust,  SIGNAL(editingFinished()),    this,         SLOT(sCalculateFromDiscount()));
      connect(_unitCost,          SIGNAL(editingFinished()),    this,         SLOT(sCalculateFromMarkup()));
      connect(_markupFromUnitCost,SIGNAL(editingFinished()),    this,         SLOT(sCalculateFromMarkup()));
      connect(_item,              SIGNAL(valid(bool)),          _listPrices,  SLOT(setEnabled(bool)));

      setSales.prepare("SELECT count(*) AS cnt"
                "  FROM quitem"
                " WHERE (quitem_quhead_id=:sohead_id);");
      setSales.bindValue(":sohead_id", _soheadid);
      setSales.exec();
      if (!setSales.first() || setSales.value("cnt").toInt() == 0)
        _prev->setEnabled(false);
      if (setSales.lastError().type() != QSqlError::NoError)
      {
        systemError(this, setSales.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _item->setReadOnly(TRUE);
      _listPrices->setEnabled(TRUE);
      _comments->setType(Comments::SalesOrderItem);

      connect(_qtyOrdered,        SIGNAL(editingFinished()),    this, SLOT(sCalculateExtendedPrice()));
      connect(_netUnitPrice,      SIGNAL(editingFinished()),    this, SLOT(sCalculateDiscountPrcnt()));
      connect(_discountFromCust,  SIGNAL(editingFinished()),    this, SLOT(sCalculateFromDiscount()));
      connect(_unitCost,          SIGNAL(editingFinished()),    this, SLOT(sCalculateFromMarkup()));
      connect(_markupFromUnitCost,SIGNAL(editingFinished()),    this, SLOT(sCalculateFromMarkup()));
      connect(_createSupplyOrder, SIGNAL(toggled(bool)),        this, SLOT(sHandleSupplyOrder()));
      
      if (_metrics->boolean("EnableSOReservations"))
        _reserveOnSave->show();
    }
    else if (param.toString() == "editQuote")
    {
      _mode = cEditQuote;
      _item->setType(ItemLineEdit::cSold | ItemLineEdit::cItemActive);
      _item->clearExtraClauseList();

      setWindowTitle(tr("Quote Item"));

      _item->setReadOnly(TRUE);
      _listPrices->setEnabled(TRUE);
      _comments->setType(Comments::QuoteItem);
      _cancel->hide();
      _sub->hide();
      _subItem->hide();
      _subItemList->hide();
      _warranty->hide();
      _tabs->removeTab(_tabs->indexOf(_costofsalesTab));

      connect(_qtyOrdered,        SIGNAL(editingFinished()),  this, SLOT(sCalculateExtendedPrice()));
      connect(_netUnitPrice,      SIGNAL(editingFinished()),  this, SLOT(sCalculateDiscountPrcnt()));
      connect(_discountFromCust,  SIGNAL(editingFinished()),  this, SLOT(sCalculateFromDiscount()));
      connect(_unitCost,          SIGNAL(editingFinished()),  this, SLOT(sCalculateFromMarkup()));
      connect(_markupFromUnitCost,SIGNAL(editingFinished()),  this, SLOT(sCalculateFromMarkup()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _comments->setType(Comments::SalesOrderItem);
      _sub->setEnabled(false);
      _subItem->setEnabled(false);
      _supplyWarehouse->setEnabled(FALSE);
      _supplyOverridePrice->setEnabled(FALSE);
    }
    else if (param.toString() == "viewQuote")
    {
      _mode = cViewQuote;
      _item->setType(ItemLineEdit::cSold | ItemLineEdit::cItemActive);
      _item->clearExtraClauseList();

      setWindowTitle(tr("Quote Item"));

      _cancel->hide();
      _sub->hide();
      _subItem->hide();
      _comments->setType(Comments::QuoteItem);
      _warranty->hide();
      _tabs->removeTab(_tabs->indexOf(_costofsalesTab));
    }
  }

  if (_initialMode == -1)
    _initialMode = _mode;

  bool viewMode = (cView == _mode || cViewQuote == _mode);
  if (viewMode)
  {
    _item->setReadOnly(viewMode);
    _customerPN->setEnabled(!viewMode);
    _qtyOrdered->setEnabled(!viewMode);
    _netUnitPrice->setEnabled(!viewMode);
    _discountFromCust->setEnabled(!viewMode);
    _unitCost->setEnabled(!viewMode);
    _markupFromUnitCost->setEnabled(!viewMode);
    _scheduledDate->setEnabled(!viewMode);
    _createSupplyOrder->setEnabled(!viewMode);
    _notes->setEnabled(!viewMode);
    _comments->setReadOnly(viewMode);
    _taxtype->setEnabled(!viewMode);
    _itemcharView->setEnabled(!viewMode);
    _promisedDate->setEnabled(!viewMode);
    _qtyUOM->setEnabled(!viewMode);
    _priceUOM->setEnabled(!viewMode);
    _warranty->setEnabled(!viewMode);
    _listPrices->setEnabled(!viewMode);
    _altCosAccnt->setEnabled(!viewMode);
    _altRevAccnt->setEnabled(!viewMode);

    _subItemList->setVisible(!viewMode);
    _save->setVisible(!viewMode);
  }

  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _soitemid = param.toInt();
    populate();

    if (ISQUOTE(_mode))
      setSales.prepare("SELECT a.quitem_id AS id"
                "  FROM quitem AS a, quitem as b"
                " WHERE ((a.quitem_quhead_id=b.quitem_quhead_id)"
                "   AND  (b.quitem_id=:id))"
                " ORDER BY a.quitem_linenumber "
                " LIMIT 1;");
    else
      setSales.prepare("SELECT a.coitem_id AS id"
                "  FROM coitem AS a, coitem AS b"
                " WHERE ((a.coitem_cohead_id=b.coitem_cohead_id)"
                "   AND  (b.coitem_id=:id))"
                " ORDER BY a.coitem_linenumber, a.coitem_subnumber"
                " LIMIT 1;");
    setSales.bindValue(":id", _soitemid);
    setSales.exec();
    if (!setSales.first() || setSales.value("id").toInt() == _soitemid)
      _prev->setEnabled(false);
    if (setSales.lastError().type() != QSqlError::NoError)
    {
      systemError(this, setSales.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    if (ISQUOTE(_mode))
      setSales.prepare("SELECT a.quitem_id AS id"
                "  FROM quitem AS a, quitem as b"
                " WHERE ((a.quitem_quhead_id=b.quitem_quhead_id)"
                "   AND  (b.quitem_id=:id))"
                " ORDER BY a.quitem_linenumber DESC"
                " LIMIT 1;");
    else
      setSales.prepare("SELECT a.coitem_id AS id"
                "  FROM coitem AS a, coitem AS b"
                " WHERE ((a.coitem_cohead_id=b.coitem_cohead_id)"
                "   AND  (b.coitem_id=:id))"
                " ORDER BY a.coitem_linenumber DESC, a.coitem_subnumber DESC"
                " LIMIT 1;");
    setSales.bindValue(":id", _soitemid);
    setSales.exec();
    if (setSales.first() && setSales.value("id").toInt() == _soitemid)
    {
      if (cView == _initialMode || cViewQuote == _initialMode)
        _next->setEnabled(false);
      else
        _next->setText(tr("New"));
    }
    if (setSales.lastError().type() != QSqlError::NoError)
    {
      systemError(this, setSales.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    // See if this is linked to a Return Authorization and handle
    if ((ISORDER(_mode)) && (_metrics->boolean("EnableReturnAuth")))
    {
      setSales.prepare("SELECT rahead_number,raitem_linenumber "
                "FROM raitem,rahead "
                "WHERE ((raitem_new_coitem_id=:coitem_id) "
                "AND (rahead_id=raitem_rahead_id));");
      setSales.bindValue(":coitem_id",_soitemid);
      setSales.exec();
      if (setSales.first())
      {
        _authNumber->show();
        _authNumberLit->show();
        _authLineNumber->show();
        _authLineNumberLit->show();
        _authNumber->setText(setSales.value("rahead_number").toString());
        _authLineNumber->setText(setSales.value("raitem_linenumber").toString());
        _qtyOrdered->setEnabled(FALSE);
        _qtyUOM->setEnabled(FALSE);
        _priceUOM->setEnabled(FALSE);
      }
    }
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  // If not multi-warehouse and a sales order hide whs control
  // Leave the warehouse controls available on Quotes as it is
  // possible to create quote line items without an itemsite
  // and the user needs a means to come back and specify the
  // warehouse after an itemsite is created.
  if (ISORDER(_mode) && !_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
    _supplyWarehouseLit->hide();
    _supplyWarehouse->hide();
  }

  _modified = false;

  sHandleButton();

  return NoError;
}

/** \return one of isOrder, isQuote, ...
 */
int salesOrderItem::modeType() const
{
  if (ISQUOTE(_mode))
    return 1;
  else
    return 2;
}

void salesOrderItem::prepare()
{
  XSqlQuery salesprepare;
  if (_mode == cNew)
  {
    //  Grab the next coitem_id
    salesprepare.exec("SELECT NEXTVAL('coitem_coitem_id_seq') AS _coitem_id");
    if (salesprepare.first())
    {
      _soitemid = salesprepare.value("_coitem_id").toInt();
      _comments->setId(_soitemid);
    }
    else if (salesprepare.lastError().type() != QSqlError::NoError)
    {
      systemError(this, salesprepare.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    salesprepare.prepare( "SELECT (COALESCE(MAX(coitem_linenumber), 0) + 1) AS _linenumber "
               "FROM coitem "
               "WHERE (coitem_cohead_id=:coitem_id)" );
    salesprepare.bindValue(":coitem_id", _soheadid);
    salesprepare.exec();
    if (salesprepare.first())
      _lineNumber->setText(salesprepare.value("_linenumber").toString());
    else if (salesprepare.lastError().type() != QSqlError::NoError)
    {
      systemError(this, salesprepare.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (!_scheduledDate->isValid())
    {
      salesprepare.prepare( "SELECT MIN(coitem_scheddate) AS scheddate "
                 "FROM coitem "
                 "WHERE (coitem_cohead_id=:cohead_id);" );
      salesprepare.bindValue(":cohead_id", _soheadid);
      salesprepare.exec();
      if (salesprepare.first())
        _scheduledDate->setDate(salesprepare.value("scheddate").toDate());
      else if (salesprepare.lastError().type() != QSqlError::NoError)
      {
        systemError(this, salesprepare.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }
  else if (_mode == cNewQuote)
  {
    //  Grab the next quitem_id
    salesprepare.exec("SELECT NEXTVAL('quitem_quitem_id_seq') AS _quitem_id");
    if (salesprepare.first())
      _soitemid = salesprepare.value("_quitem_id").toInt();
    else if (salesprepare.lastError().type() != QSqlError::NoError)
    {
      systemError(this, salesprepare.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    salesprepare.prepare( "SELECT (COALESCE(MAX(quitem_linenumber), 0) + 1) AS n_linenumber "
               "FROM quitem "
               "WHERE (quitem_quhead_id=:quhead_id)" );
    salesprepare.bindValue(":quhead_id", _soheadid);
    salesprepare.exec();
    if (salesprepare.first())
      _lineNumber->setText(salesprepare.value("n_linenumber").toString());
    else if (salesprepare.lastError().type() != QSqlError::NoError)
    {
      systemError(this, salesprepare.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (!_scheduledDate->isValid())
    {
      salesprepare.prepare( "SELECT MIN(quitem_scheddate) AS scheddate "
                 "FROM quitem "
                 "WHERE (quitem_quhead_id=:quhead_id);" );
      salesprepare.bindValue(":quhead_id", _soheadid);
      salesprepare.exec();
      if (salesprepare.first())
        _scheduledDate->setDate(salesprepare.value("scheddate").toDate());
      else if (salesprepare.lastError().type() != QSqlError::NoError)
      {
        systemError(this, salesprepare.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }
  _modified = false;
  _partialsaved = false;
}

void salesOrderItem::clear()
{
  if (_supplyOrderId > -1)
  {
    disconnect(_woIndentedList,    SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateWoMenu(QMenu*, QTreeWidgetItem*)));
    disconnect(_woIndentedList,    SIGNAL(itemSelected(int)),            _supplyWoEdit, SLOT(animateClick()));
    disconnect(_woIndentedList,    SIGNAL(valid(bool)),                  _supplyWoEdit, SLOT(setEnabled(bool)));
    disconnect(_woIndentedList,    SIGNAL(valid(bool)),                  _supplyWoDelete, SLOT(setEnabled(bool)));
    disconnect(_supplyWoNewMatl,   SIGNAL(clicked()),                    this, SLOT(sNewWoMatl()));
    disconnect(_supplyWoEdit,      SIGNAL(clicked()),                    this, SLOT(sEditWoMatl()));
    disconnect(_supplyWoDelete,    SIGNAL(clicked()),                    this, SLOT(sDeleteWoMatl()));
    disconnect(_supplyRollupPrices,SIGNAL(toggled(bool)),                this, SLOT(sRollupPrices()));
    disconnect(_supplyOrderQty,    SIGNAL(editingFinished()),            this, SLOT(sHandleSupplyOrder()));
    //  disconnect(_supplyOrderDueDate,SIGNAL(newDate(const QDate &)),       this, SLOT(sHandleSupplyOrder()));
    disconnect(_supplyOverridePrice,SIGNAL(editingFinished()),           this, SLOT(sHandleSupplyOrder()));
    disconnect(_supplyDropShip,    SIGNAL(toggled(bool)),                this, SLOT(sHandleSupplyOrder()));
  }

  _supplyOrderType = "";
  _supplyOrderId = -1;
  _createSupplyOrder->setChecked(FALSE);
  _item->setReadOnly(FALSE);
  _warehouse->setEnabled(TRUE);
  _item->setId(-1);
  _customerPN->clear();
  _qtyOrdered->clear();
  _qtyUOM->clear();
  _priceUOM->clear();
  _netUnitPrice->clear();
  _extendedPrice->clear();
//  _scheduledDate->clear();
  _promisedDate->clear();
  _unitCost->clear();
  _listPrice->clear();
  _customerPrice->clear();
  _discountFromListPrice->clear();
  _markupFromUnitCost->clear();
  _discountFromCust->clear();
  _margin->clear();
  _shippedToDate->clear();
  _onHand->clear();
  _allocated->clear();
  _unallocated->clear();
  _onOrder->clear();
  _available->clear();
  _reserved->clear();
  _reservable->clear();
  _itemsrcp->clear();
  _subs->clear();
  _historyCosts->clear();
  _historySales->clear();
  _leadtime->clear();
  _itemchar->removeRows(0, _itemchar->rowCount());
  _notes->clear();
  _sub->setChecked(false);
  _subItem->clear();
  _subItem->setEnabled(false);
  _subItemList->setEnabled(false);
  _comments->setId(-1);
  _warehouse->clear();  // are these two _warehouse steps necessary?
  _warehouse->setType(WComboBox::Sold);
  _originalQtyOrd  = 0.0;
  _qtyOrderedCache   = 0.0;
  _priceUOMCache   = -1;
  _modified        = false;
  _partialsaved    = false;
  _updateItemsite  = false;
  _baseUnitPrice->clear();
  _itemcharView->setEnabled(TRUE);
  _itemsrc = -1;
  _altRevAccnt->clear();
}

void salesOrderItem::sSaveClicked()
{
  sSave(false);
}

void salesOrderItem::sSave(bool pPartial)
{
  XSqlQuery salesSave;
  _save->setFocus();

  _error = true;
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_warehouse->isValid(), _warehouse,
                          tr("<p>You must select a valid Site before saving this Sales Order Item."))
         << GuiErrorCheck(!(_qtyOrdered->toDouble() > 0), _qtyOrdered,
                          tr("<p>You must enter a valid Quantity Ordered before saving this Sales Order Item."))
         << GuiErrorCheck((_qtyOrdered->toDouble() != (double)qRound(_qtyOrdered->toDouble()) &&
                           _qtyOrdered->validator()->inherits("QIntValidator")), _qtyOrdered,
                          tr("This UOM for this Item does not allow fractional quantities. Please fix the quantity."))
         << GuiErrorCheck(_netUnitPrice->isEmpty(), _netUnitPrice,
                          tr("<p>You must enter a Price before saving this Sales Order Item."))
         << GuiErrorCheck(_sub->isChecked() && !_subItem->isValid(), _subItem,
                          tr("<p>You must enter a Substitute Item before saving this Sales Order Item."))
         << GuiErrorCheck(!_scheduledDate->isValid(), _scheduledDate,
                          tr("<p>You must enter a valid Schedule Date before saving this Sales Order Item."))
         << GuiErrorCheck(_createSupplyOrder->isChecked() &&
                          _item->itemType() == "M" &&
                          _supplyWarehouse->id() == -1, _supplyWarehouse,
                          tr("<p>Before an Order may be created, a valid Supplied at Site must be selected."))
         << GuiErrorCheck(!_createSupplyOrder->isChecked() &&
                          _costmethod == "J", _createSupplyOrder,
                          tr("<p>You must create a supply order for this Job Costed Item before saving this Sales Order Item."))
  ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Sales Order Item"), errors))
    return;
  
  // Make sure Qty Ordered/Qty UOM does not result in an invalid fractional Inv Qty
  if (!pPartial && !_invIsFractional)
  {
    if (qAbs((_qtyOrdered->toDouble() * _qtyinvuomratio) - (double)qRound(_qtyOrdered->toDouble() * _qtyinvuomratio)) > 0.00001)
    {
      if (QMessageBox::question(this, tr("Change Qty Ordered?"),
                                tr("This Qty Ordered/Qty UOM will result "
                                   "in a fractional Inventory Qty for "
                                   "this Item.  This Item does not allow "
                                   "fractional quantities.\n"
                                   "Do you want to change the Qty Ordered?"),
                                QMessageBox::Yes | QMessageBox::Default,
                                QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
      {
        if (_qtyOrdered->validator()->inherits("QIntValidator"))
          _qtyOrdered->setDouble((double)qRound(_qtyOrdered->toDouble() * _qtyinvuomratio + 0.5) / _qtyinvuomratio, 0);
        else
          _qtyOrdered->setDouble((double)qRound(_qtyOrdered->toDouble() * _qtyinvuomratio + 0.5) / _qtyinvuomratio, 2);
        _qtyOrderedCache = _qtyOrdered->toDouble();
        _qtyOrdered->setFocus();
        return;
      }
    }
  }

  _error = false;

  QDate promiseDate;

  if (_metrics->boolean("UsePromiseDate"))
  {
    if (_promisedDate->isValid())
    {
      if (_promisedDate->isNull())
        promiseDate = omfgThis->endOfTime();
      else
        promiseDate = _promisedDate->date();
    }
  }
  else
    promiseDate = omfgThis->endOfTime();

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");  // In case of failure along the way
  salesSave.exec("BEGIN;");

  if (_mode == cNew && !_partialsaved)
  {
    salesSave.prepare( "INSERT INTO coitem "
               "( coitem_id, coitem_cohead_id, coitem_linenumber, coitem_itemsite_id,"
               "  coitem_status, coitem_scheddate, coitem_promdate,"
               "  coitem_qtyord, coitem_qty_uom_id, coitem_qty_invuomratio,"
               "  coitem_qtyshipped, coitem_qtyreturned,"
               "  coitem_unitcost, coitem_custprice, coitem_pricemode,"
               "  coitem_price, coitem_price_uom_id, coitem_price_invuomratio,"
               "  coitem_order_type, coitem_order_id,"
               "  coitem_custpn, coitem_memo, coitem_substitute_item_id,"
               "  coitem_prcost, coitem_taxtype_id, coitem_warranty,"
               "  coitem_cos_accnt_id, coitem_rev_accnt_id) "
               "  SELECT :soitem_id, :soitem_sohead_id, :soitem_linenumber, itemsite_id,"
               "       'O', :soitem_scheddate, :soitem_promdate,"
               "       :soitem_qtyord, :qty_uom_id, :qty_invuomratio, 0, 0,"
               "       :soitem_unitcost, :soitem_custprice, :soitem_pricemode,"
               "       :soitem_price, :price_uom_id, :price_invuomratio,"
               "       :soitem_order_type, :soitem_order_id,"
               "       :soitem_custpn, :soitem_memo, :soitem_substitute_item_id,"
               "       :soitem_prcost, :soitem_taxtype_id, :soitem_warranty, "
               "       :soitem_cos_accnt_id, :soitem_rev_accnt_id "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    salesSave.bindValue(":soitem_id", _soitemid);
    salesSave.bindValue(":soitem_sohead_id", _soheadid);
    salesSave.bindValue(":soitem_linenumber", _lineNumber->text().toInt());
    salesSave.bindValue(":soitem_scheddate", _scheduledDate->date());
    salesSave.bindValue(":soitem_promdate", promiseDate);
    salesSave.bindValue(":soitem_qtyord", _qtyOrdered->toDouble());
    salesSave.bindValue(":qty_uom_id", _qtyUOM->id());
    salesSave.bindValue(":qty_invuomratio", _qtyinvuomratio);
    salesSave.bindValue(":soitem_custprice", _customerPrice->localValue());
    salesSave.bindValue(":soitem_unitcost", _unitCost->localValue());
    salesSave.bindValue(":soitem_pricemode", _priceMode);
    salesSave.bindValue(":soitem_price", _netUnitPrice->localValue());
    salesSave.bindValue(":price_uom_id", _priceUOM->id());
    salesSave.bindValue(":price_invuomratio", _priceinvuomratio);
    salesSave.bindValue(":soitem_prcost", _supplyOverridePrice->localValue());
    salesSave.bindValue(":soitem_custpn", _customerPN->text());
    salesSave.bindValue(":soitem_memo", _notes->toPlainText());
    salesSave.bindValue(":item_id", _item->id());
    if (_sub->isChecked())
      salesSave.bindValue(":soitem_substitute_item_id", _subItem->id());
    salesSave.bindValue(":warehous_id", _warehouse->id());
    if (_taxtype->isValid())
      salesSave.bindValue(":soitem_taxtype_id", _taxtype->id());
    if (_altCosAccnt->isValid())
      salesSave.bindValue(":soitem_cos_accnt_id", _altCosAccnt->id());
    if (_altRevAccnt->isValid())
      salesSave.bindValue(":soitem_rev_accnt_id", _altRevAccnt->id());
    salesSave.bindValue(":soitem_warranty",QVariant(_warranty->isChecked()));
// setting supply order info will cause trigger to create order
//    salesSave.bindValue(":soitem_order_type", _supplyOrderType);
//    salesSave.bindValue(":soitem_order_id", _supplyOrderId);
    salesSave.exec();
    if (salesSave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if ( (_mode == cEdit) || ((_mode == cNew) && _partialsaved) )
  {
    salesSave.prepare("UPDATE coitem "
                      "SET coitem_itemsite_id=(SELECT itemsite_id"
                      "                        FROM itemsite"
                      "                        WHERE (itemsite_item_id=:item_id)"
                      "                          AND (itemsite_warehous_id=:warehous_id)),"
                      "    coitem_scheddate=:soitem_scheddate,"
                      "    coitem_promdate=:soitem_promdate,"
                      "    coitem_qtyord=:soitem_qtyord,"
                      "    coitem_qty_uom_id=:qty_uom_id,"
                      "    coitem_qty_invuomratio=:qty_invuomratio,"
                      "    coitem_unitcost=:soitem_unitcost,"
                      "    coitem_custprice=:soitem_custprice,"
                      "    coitem_pricemode=:soitem_pricemode,"
                      "    coitem_price=:soitem_price,"
                      "    coitem_price_uom_id=:price_uom_id,"
                      "    coitem_price_invuomratio=:price_invuomratio,"
                      "    coitem_memo=:soitem_memo,"
                      "    coitem_order_type=:soitem_order_type,"
                      "    coitem_order_id=:soitem_order_id,"
                      "    coitem_substitute_item_id=:soitem_substitute_item_id,"
                      "    coitem_prcost=:soitem_prcost,"
                      "    coitem_taxtype_id=:soitem_taxtype_id, "
                      "    coitem_cos_accnt_id=:soitem_cos_accnt_id, "
                      "    coitem_rev_accnt_id=:soitem_rev_accnt_id, "
                      "    coitem_warranty=:soitem_warranty, "
                      "    coitem_custpn=:custpn "
                      "WHERE (coitem_id=:soitem_id);" );
    salesSave.bindValue(":item_id", _item->id());
    salesSave.bindValue(":warehous_id", _warehouse->id());
    salesSave.bindValue(":soitem_scheddate", _scheduledDate->date());
    salesSave.bindValue(":soitem_promdate", promiseDate);
    salesSave.bindValue(":soitem_qtyord", _qtyOrdered->toDouble());
    salesSave.bindValue(":qty_uom_id", _qtyUOM->id());
    salesSave.bindValue(":qty_invuomratio", _qtyinvuomratio);
    salesSave.bindValue(":soitem_unitcost", _unitCost->localValue());
    salesSave.bindValue(":soitem_custprice", _customerPrice->localValue());
    salesSave.bindValue(":soitem_price", _netUnitPrice->localValue());
    salesSave.bindValue(":soitem_pricemode", _priceMode);
    salesSave.bindValue(":price_uom_id", _priceUOM->id());
    salesSave.bindValue(":price_invuomratio", _priceinvuomratio);
    if (_supplyOrderId > -1)
    {
      salesSave.bindValue(":soitem_order_type", _supplyOrderType);
      salesSave.bindValue(":soitem_order_id", _supplyOrderId);
      salesSave.bindValue(":soitem_prcost", _supplyOverridePrice->localValue());
    }
    salesSave.bindValue(":soitem_memo", _notes->toPlainText());
    salesSave.bindValue(":soitem_id", _soitemid);
    if (_sub->isChecked())
      salesSave.bindValue(":soitem_substitute_item_id", _subItem->id());
    if (_taxtype->isValid())
      salesSave.bindValue(":soitem_taxtype_id", _taxtype->id());
    if (_altCosAccnt->isValid())
      salesSave.bindValue(":soitem_cos_accnt_id", _altCosAccnt->id());
    if (_altRevAccnt->isValid())
      salesSave.bindValue(":soitem_rev_accnt_id", _altRevAccnt->id());
    salesSave.bindValue(":soitem_warranty",QVariant(_warranty->isChecked()));
    salesSave.bindValue(":custpn", _customerPN->text());

    salesSave.exec();
    if (salesSave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    //  Check to see if a Reservations need changes
    if (_qtyOrdered->toDouble() < _qtyreserved)
    {
      salesSave.prepare("SELECT unreserveSoLineQty(:soitem_id) AS result;");
      salesSave.bindValue(":soitem_id", _soitemid);
      salesSave.exec();
      if (salesSave.first())
      {
        int result = salesSave.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("unreservedSoLineQty", result) +
                      tr("<br>Line Item %1").arg(""),
                      __FILE__, __LINE__);
        }
        // setup for re-reserving the new qty
        _reserveOnSave->setChecked(true);
      }
      else if (salesSave.lastError().type() != QSqlError::NoError)
      {
        systemError(this, tr("Line Item %1\n").arg("") +
                    salesSave.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
  }
  else if (_mode == cNewQuote && !_partialsaved)
  {
    salesSave.prepare( "INSERT INTO quitem "
               "( quitem_id, quitem_quhead_id, quitem_linenumber, quitem_itemsite_id,"
               "  quitem_item_id, quitem_scheddate, quitem_promdate, quitem_qtyord,"
               "  quitem_qty_uom_id, quitem_qty_invuomratio,"
               "  quitem_unitcost, quitem_custprice, quitem_price, quitem_pricemode,"
               "  quitem_price_uom_id, quitem_price_invuomratio,"
               "  quitem_custpn, quitem_memo, quitem_createorder, "
               "  quitem_order_warehous_id, quitem_prcost, quitem_taxtype_id, "
               "  quitem_dropship, quitem_itemsrc_id ) "
               "VALUES(:quitem_id, :quitem_quhead_id, :quitem_linenumber,"
               "       (SELECT itemsite_id FROM itemsite WHERE ((itemsite_item_id=:item_id) AND (itemsite_warehous_id=:warehous_id))),"
               "       :item_id, :quitem_scheddate, :quitem_promdate, :quitem_qtyord,"
               "       :qty_uom_id, :qty_invuomratio,"
               "       :quitem_unitcost, :quitem_custprice, :quitem_price, :quitem_pricemode,"
               "       :price_uom_id, :price_invuomratio,"
               "       :quitem_custpn, :quitem_memo, :quitem_createorder, "
               "       :quitem_order_warehous_id, :quitem_prcost, :quitem_taxtype_id, "
               "       :quitem_dropship, :quitem_itemsrc_id);" );
    salesSave.bindValue(":quitem_id", _soitemid);
    salesSave.bindValue(":quitem_quhead_id", _soheadid);
    salesSave.bindValue(":quitem_linenumber", _lineNumber->text().toInt());
    salesSave.bindValue(":quitem_scheddate", _scheduledDate->date());
    salesSave.bindValue(":quitem_promdate", promiseDate);
    salesSave.bindValue(":quitem_qtyord", _qtyOrdered->toDouble());
    salesSave.bindValue(":qty_uom_id", _qtyUOM->id());
    salesSave.bindValue(":qty_invuomratio", _qtyinvuomratio);
    salesSave.bindValue(":quitem_unitcost", _unitCost->localValue());
    salesSave.bindValue(":quitem_custprice", _customerPrice->localValue());
    salesSave.bindValue(":quitem_price", _netUnitPrice->localValue());
    salesSave.bindValue(":quitem_pricemode", _priceMode);
    salesSave.bindValue(":price_uom_id", _priceUOM->id());
    salesSave.bindValue(":price_invuomratio", _priceinvuomratio);
    salesSave.bindValue(":quitem_custpn", _customerPN->text());
    salesSave.bindValue(":quitem_memo", _notes->toPlainText());
    salesSave.bindValue(":quitem_createorder", QVariant(_createSupplyOrder->isChecked()));
    salesSave.bindValue(":quitem_order_warehous_id", _supplyWarehouse->id());
    salesSave.bindValue(":quitem_prcost", _supplyOverridePrice->localValue());
    salesSave.bindValue(":item_id", _item->id());
    salesSave.bindValue(":warehous_id", _warehouse->id());
    if (_taxtype->isValid())
      salesSave.bindValue(":quitem_taxtype_id", _taxtype->id());
    salesSave.bindValue(":quitem_dropship", QVariant(_supplyDropShip->isChecked()));
    if (_itemsrc > 0)
      salesSave.bindValue(":quitem_itemsrc_id", _itemsrc);
    salesSave.exec();
    if (salesSave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
          systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if ( (_mode == cEditQuote) || ((_mode == cNewQuote) && _partialsaved) )
  {
    salesSave.prepare("UPDATE quitem "
                      "SET quitem_itemsite_id=(SELECT itemsite_id"
                      "                        FROM itemsite"
                      "                        WHERE (itemsite_item_id=:item_id)"
                      "                          AND (itemsite_warehous_id=:warehous_id)),"
                      "    quitem_scheddate=:quitem_scheddate,"
                      "    quitem_promdate=:quitem_promdate,"
                      "    quitem_qtyord=:quitem_qtyord,"
                      "    quitem_qty_uom_id=:qty_uom_id,"
                      "    quitem_qty_invuomratio=:qty_invuomratio,"
                      "    quitem_unitcost=:quitem_unitcost,"
                      "    quitem_custprice=:quitem_custprice,"
                      "    quitem_price=:quitem_price,"
                      "    quitem_pricemode=:quitem_pricemode,"
                      "    quitem_price_uom_id=:price_uom_id,"
                      "    quitem_price_invuomratio=:price_invuomratio,"
                      "    quitem_memo=:quitem_memo,"
                      "    quitem_createorder=:quitem_createorder,"
                      "    quitem_order_warehous_id=:quitem_order_warehous_id,"
                      "    quitem_prcost=:quitem_prcost,"
                      "    quitem_taxtype_id=:quitem_taxtype_id,"
                      "    quitem_dropship=:quitem_dropship,"
                      "    quitem_itemsrc_id=:quitem_itemsrc_id, "
                      "    quitem_custpn=:custpn "
                      "WHERE (quitem_id=:quitem_id);" );
    salesSave.bindValue(":item_id", _item->id());
    salesSave.bindValue(":warehous_id", _warehouse->id());
    salesSave.bindValue(":quitem_scheddate", _scheduledDate->date());
    salesSave.bindValue(":quitem_promdate", promiseDate);
    salesSave.bindValue(":quitem_qtyord", _qtyOrdered->toDouble());
    salesSave.bindValue(":qty_uom_id", _qtyUOM->id());
    salesSave.bindValue(":qty_invuomratio", _qtyinvuomratio);
    salesSave.bindValue(":quitem_unitcost", _unitCost->localValue());
    salesSave.bindValue(":quitem_custprice", _customerPrice->localValue());
    salesSave.bindValue(":quitem_price", _netUnitPrice->localValue());
    salesSave.bindValue(":quitem_pricemode", _priceMode);
    salesSave.bindValue(":price_uom_id", _priceUOM->id());
    salesSave.bindValue(":price_invuomratio", _priceinvuomratio);
    salesSave.bindValue(":quitem_memo", _notes->toPlainText());
    salesSave.bindValue(":quitem_createorder", QVariant(_createSupplyOrder->isChecked()));
    salesSave.bindValue(":quitem_order_warehous_id", _supplyWarehouse->id());
    salesSave.bindValue(":quitem_prcost", _supplyOverridePrice->localValue());
    salesSave.bindValue(":quitem_id", _soitemid);
    if (_taxtype->isValid())
      salesSave.bindValue(":quitem_taxtype_id", _taxtype->id());
    salesSave.bindValue(":quitem_dropship", QVariant(_supplyDropShip->isChecked()));
    if (_itemsrc > 0)
      salesSave.bindValue(":quitem_itemsrc_id", _itemsrc);
    salesSave.bindValue(":custpn", _customerPN->text());
    salesSave.exec();
    if (salesSave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
          systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (_updateItemsite)
    {
      salesSave.prepare("UPDATE quitem "
                "   SET quitem_itemsite_id=(SELECT itemsite_id FROM itemsite WHERE ((itemsite_item_id=quitem_item_id) AND (itemsite_warehous_id=:warehous_id)))"
                " WHERE (quitem_id=:quitem_id);" );
      salesSave.bindValue(":warehous_id", _warehouse->id());
      salesSave.bindValue(":quitem_id", _soitemid);
      salesSave.exec();
      if (salesSave.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
          systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }

  // Update supply order characteristics
  if ( (_mode != cView) && (_mode != cViewQuote) )
  {
//    if (_supplyOrderId != -1 && !_item->isConfigured())
    if (_supplyOrderId != -1)
    {
      // Update Supply Order Characteristics
      if (_itemchar->rowCount() > 0)
      {
        bool changed = false;
        QModelIndex idx1, idx2;
        XSqlQuery chgq;
        chgq.prepare("SELECT (charass_value=:value) AS same"
                     "  FROM charass"
                     " WHERE ((charass_char_id=:id)"
                     "    AND (charass_target_id=:target)"
                     "    AND (charass_target_type='SI'));");
        for (int i = 0; i < _itemchar->rowCount(); i++)
        {
          idx1 = _itemchar->index(i, CHAR_ID);
          idx2 = _itemchar->index(i, CHAR_VALUE);
          chgq.bindValue(":id", _itemchar->data(idx1, Qt::UserRole));
          chgq.bindValue(":value", _itemchar->data(idx2, Qt::DisplayRole));
          chgq.bindValue(":target", _soitemid);
          chgq.exec();
          if (chgq.first())
          {
            if (! chgq.value("same").toBool())
            {
              changed = true;
              break;
            }
          }
          else if (_itemchar->data(idx2, Qt::DisplayRole) != "")
          {
            changed = true;
            break;
          }
          else if (chgq.lastError().type() != QSqlError::NoError)
          {
            rollback.exec();
            ErrorReporter::error(QtCriticalMsg, this, tr("Error Checking Characteristics"),
                                 chgq, __FILE__, __LINE__);
            return;
          }
        }
        
        bool applychange = false;
        if (changed)
        {
          if (_mode == cNew)
            applychange = true;
          else if (QMessageBox::question(this, tr("Change Characteristics?"),
                                         tr("<p>Should the characteristics for the "
                                            "associated supply order be updated?"),
                                         QMessageBox::Yes | QMessageBox::Default,
                                         QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
            applychange = true;
        }
        
        if (applychange)
        {
          salesSave.prepare("SELECT updateCharAssignment(:target_type, :target_id, :char_id, :char_value) AS result;");
          
          for (int i = 0; i < _itemchar->rowCount(); i++)
          {
            idx1 = _itemchar->index(i, CHAR_ID);
            idx2 = _itemchar->index(i, CHAR_VALUE);
            if (_supplyOrderType == "P")
              salesSave.bindValue(":target_type", "PI");
            else
              salesSave.bindValue(":target_type", "W");
            salesSave.bindValue(":target_id", _supplyOrderId);
            salesSave.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
            salesSave.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
            salesSave.exec();
            if (salesSave.first())
            {
              int result = salesSave.value("result").toInt();
              if (result < 0)
              {
                rollback.exec();
                ErrorReporter::error(QtCriticalMsg, this, tr("Error with Characteristics"),
                                     storedProcErrorLookup("updateCharAssignment", result),
                                     __FILE__, __LINE__);
                return;
              }
            }
            else if (salesSave.lastError().type() != QSqlError::NoError)
            {
              rollback.exec();
              ErrorReporter::error(QtCriticalMsg, this, tr("Error with Characteristics"),
                                   salesSave, __FILE__, __LINE__);
              return;
            }
          }
        }
      }
      
      _qtyOrderedCache = _qtyOrdered->toDouble();
    }

    QString type = "SI";
    if (_mode & 0x20)
      type = "QI";

    QModelIndex idx1, idx2, idx3;
    if (_item->isConfigured())
      salesSave.prepare("SELECT updateCharAssignment(:target_type, :target_id, :char_id, :char_value, CAST(:char_price AS NUMERIC)) AS result;");
    else
      salesSave.prepare("SELECT updateCharAssignment(:target_type, :target_id, :char_id, :char_value) AS result;");

    for (int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx1 = _itemchar->index(i, CHAR_ID);
      idx2 = _itemchar->index(i, CHAR_VALUE);
      idx3 = _itemchar->index(i, CHAR_PRICE);
      salesSave.bindValue(":target_type", type);
      salesSave.bindValue(":target_id", _soitemid);
      salesSave.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
      salesSave.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
      salesSave.bindValue(":char_price", _itemchar->data(idx3, Qt::DisplayRole));
      salesSave.exec();
      if (salesSave.first())
      {
        int result = salesSave.value("result").toInt();
        if (result < 0)
        {
          rollback.exec();
          systemError(this, storedProcErrorLookup("updateCharAssignment", result),
                      __FILE__, __LINE__);
          return;
        }
      }
      else if (salesSave.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
          systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    if ( (_supplyOrderType == "W") && (_supplyOrderStatus->text() == "O") && _item->isConfigured() )
    {
      XSqlQuery explodeq;
      explodeq.prepare( "SELECT explodeWo(:wo_id, true) AS result;" );
      explodeq.bindValue(":wo_id", _supplyOrderId);
      explodeq.exec();
      if (explodeq.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, explodeq.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }

  salesSave.exec("COMMIT;");

  if (!pPartial)
  {
    if (_mode == cNew)
      omfgThis->sSalesOrdersUpdated(_soheadid);
    else if (_mode == cNewQuote)
      omfgThis->sQuotesUpdated(_soheadid);
  }
  
  _modified = false;

  if ( (pPartial) && ((cNew == _mode) || (cNewQuote == _mode)) )
  {
    _item->setReadOnly(TRUE);
    _warehouse->setEnabled(FALSE);
    _partialsaved = true;
    return;
  }

  if (_metrics->boolean("EnableSOReservations") && _reserveOnSave->isChecked())
    sReserveStock();

  if ( (!_canceling) && (cNew == _mode || cNewQuote == _mode) )
  {
    clear();
    prepare();
    _prev->setEnabled(true);
    _item->setFocus();
  }
}

void salesOrderItem::sPopulateItemsiteInfo()
{
  if (_item->isValid() && _warehouse->isValid())
  {
    XSqlQuery itemsite;
    itemsite.prepare("SELECT itemsite_leadtime, itemsite_costmethod,"
                     "       itemsite_createsopo, itemsite_createsopr,"
                     "       itemsite_createwo, itemsite_dropship,"
                     "       itemsite_stocked,"
                     "       itemCost(:item_id, :cust_id, :shipto_id, :qty, :qtyUOM, :priceUOM,"
                     "                :curr_id, :effective, :asof, :warehous_id) AS unitcost "
                     "FROM itemsite JOIN item ON (item_id=itemsite_item_id) "
                     "WHERE ( (itemsite_warehous_id=:warehous_id)"
                     "  AND   (itemsite_item_id=:item_id) );" );
    // TODO: why JOIN item if we don't use any of its fields?
    itemsite.bindValue(":cust_id", _custid);
    itemsite.bindValue(":shipto_id", _shiptoid);
    itemsite.bindValue(":qty", _qtyOrdered->toDouble());
    itemsite.bindValue(":qtyUOM", _qtyUOM->id());
    itemsite.bindValue(":priceUOM", _priceUOM->id());
    itemsite.bindValue(":curr_id", _customerPrice->id());
    itemsite.bindValue(":effective", _customerPrice->effective());
    if (_metrics->value("soPriceEffective") == "OrderDate")
      itemsite.bindValue(":asof", _netUnitPrice->effective());
    else if (_metrics->value("soPriceEffective") == "ScheduleDate" && _scheduledDate->isValid())
      itemsite.bindValue(":asof", _scheduledDate->date());
    else
      itemsite.bindValue(":asof", omfgThis->dbDate());
    itemsite.bindValue(":warehous_id", _warehouse->id());
    itemsite.bindValue(":item_id", _item->id());
    itemsite.exec();
    if (itemsite.first())
    {
      _leadTime    = itemsite.value("itemsite_leadtime").toInt();
      _stocked     = itemsite.value("itemsite_stocked").toBool();
      _costmethod  = itemsite.value("itemsite_costmethod").toString();
      _unitCost->setBaseValue(itemsite.value("unitcost").toDouble() * _priceinvuomratio);

      if (itemsite.value("itemsite_createwo").toBool())
      {
        _supplyOrderType = "W";
        _createSupplyOrder->setTitle(tr("Create Work Order"));
      }
      else if (itemsite.value("itemsite_createsopo").toBool())
      {
        _supplyOrderType = "P";
        _createSupplyOrder->setTitle(tr("Create Purchase Order"));
        if (_metrics->boolean("EnableDropShipments"))
        {
          _supplyDropShip->setEnabled(true);
          _supplyDropShip->setChecked(itemsite.value("itemsite_dropship").toBool());
          _supplyOrderDropShipCache = itemsite.value("itemsite_dropship").toBool();
        }
        else
        {
          _supplyDropShip->setEnabled(false);
          _supplyDropShip->setChecked(false);
          _supplyOrderDropShipCache = false;
        }
      }
      else if (itemsite.value("itemsite_createsopr").toBool())
      {
        _supplyOrderType = "R";
        _createSupplyOrder->setTitle(tr("Create Purchase Request"));
        if (_metrics->boolean("EnableDropShipments"))
        {
          _supplyDropShip->setEnabled(true);
          _supplyDropShip->setChecked(itemsite.value("itemsite_dropship").toBool());
          _supplyOrderDropShipCache = itemsite.value("itemsite_dropship").toBool();
        }
        else
        {
          _supplyDropShip->setEnabled(false);
          _supplyDropShip->setChecked(false);
          _supplyOrderDropShipCache = false;
        }
      }
      else
      {
        _createSupplyOrder->setEnabled(FALSE);
        _supplyOrderType = "";
        _createSupplyOrder->setTitle(tr("Create Supply Order"));
      }

      if (_item->isConfigured() && _costmethod == "J" && _supplyOrderType == "W")
      {
        _tabs->setCurrentIndex(_tabs->indexOf(_itemCharacteristicsTab));
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Itemsite"),
                                  itemsite, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void salesOrderItem::sListPrices()
{
  ParameterList params;
  params.append("cust_id", _custid);
  params.append("shipto_id", _shiptoid);
  params.append("item_id", _item->id());
  params.append("warehous_id", _warehouse->id());
  params.append("qty", _qtyOrdered->toDouble() * _qtyinvuomratio);
  params.append("curr_id", _netUnitPrice->id());
  params.append("effective", _netUnitPrice->effective());
  if (_metrics->value("soPriceEffective") == "OrderDate")
    params.append("asof", _netUnitPrice->effective());
  else if (_metrics->value("soPriceEffective") == "ScheduleDate" &&
           _scheduledDate->isValid())
    params.append("asof", _scheduledDate->date());
  else
    params.append("asof", omfgThis->dbDate());

  priceList newdlg(this);
  newdlg.set(params);
  if ( (newdlg.exec() == XDialog::Accepted) &&
       (_privileges->check("OverridePrice")) &&
       (!_metrics->boolean("DisableSalesOrderPriceOverride")) )
  {
    _netUnitPrice->setLocalValue(newdlg._selectedPrice * (_priceinvuomratio / _priceRatio));
    sCalculateDiscountPrcnt();
  }
}

void salesOrderItem::sRecalcPrice()
{
  sDeterminePrice(true);
}

void salesOrderItem::sDeterminePrice()
{
  XSqlQuery salesDeterminePrice;
  sDeterminePrice(false);
}

void salesOrderItem::sDeterminePrice(bool force)
{
  XSqlQuery salesDeterminePrice;
  // Determine if we can or should update the price
  if ( _mode == cView ||
       _mode == cViewQuote ||
       !_item->isValid() ||
       _qtyOrdered->text().isEmpty() ||
       _qtyUOM->id() < 0 ||
       _priceUOM->id() < 0 ||
       (
         !force && _qtyOrdered->toDouble() == _qtyOrderedCache && (
           _metrics->value("soPriceEffective") != "ScheduleDate" || (
             !_scheduledDate->isValid() ||
             _scheduledDate->date() == _scheduledDateCache) ) ) )
  {
    sCheckSupplyOrder();
    return;
  }

  double  charTotal  =0;
  bool    dateChanged =(_scheduledDateCache != _scheduledDate->date());
  bool    qtyChanged =(_qtyOrderedCache != _qtyOrdered->toDouble());
  bool    priceUOMChanged =(_priceUOMCache != _priceUOM->id());
  QDate   asOf;

  if (_metrics->value("soPriceEffective") == "ScheduleDate")
    asOf = _scheduledDate->date();
  else if (_metrics->value("soPriceEffective") == "OrderDate")
    asOf = _netUnitPrice->effective();
  else
    asOf = omfgThis->dbDate();

  // Okay, we'll update customer price for sure, but how about net unit price?
  if ( _mode == cEdit ||
       _mode == cEditQuote)
  {
    if ( _customerPrice->localValue() != _netUnitPrice->localValue() && (
           _metrics->boolean("IgnoreCustDisc") || (
             _metrics->value("UpdatePriceLineEdit").toInt() == iDontUpdate &&
             !force) ) )
      _updatePrice = false;
    else if ( _metrics->value("UpdatePriceLineEdit").toInt() != iJustUpdate)
    {
      QString token(tr("Characteristic"));
      if (dateChanged)
          token=tr("Scheduled Date");
      if (qtyChanged)
        token=tr("Item quantity");
      if (priceUOMChanged)
        token=tr("Price UOM");
      if (QMessageBox::question(this, tr("Update Price?"),
                                tr("<p>The %1 has changed. Do you want to update the Price?").arg(token),
                                QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape) == QMessageBox::No)
        _updatePrice = false;
    }
  }
  // Go get the new price information
  // For configured items, update characteristic pricing
  if ( _item->isConfigured() )
  {
    disconnect(_itemchar, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(sRecalcPrice()));
    _charVars.replace(QTY, _qtyOrdered->toDouble() * _qtyinvuomratio);

    QModelIndex idx1, idx2, idx3;
    salesDeterminePrice.prepare("SELECT itemcharprice(:item_id,:char_id,:value,:cust_id,:shipto_id,:qty,:curr_id,:effective,:asof)::numeric(16,4) AS price;");

    for (int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx1 = _itemchar->index(i, CHAR_ID);
      idx2 = _itemchar->index(i, CHAR_VALUE);
      idx3 = _itemchar->index(i, CHAR_PRICE);
      salesDeterminePrice.bindValue(":item_id", _item->id());
      salesDeterminePrice.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
      salesDeterminePrice.bindValue(":value", _itemchar->data(idx2, Qt::DisplayRole));
      salesDeterminePrice.bindValue(":cust_id", _custid);
      salesDeterminePrice.bindValue(":shipto_id", _shiptoid);
      salesDeterminePrice.bindValue(":qty", _qtyOrdered->toDouble() * _qtyinvuomratio);
      salesDeterminePrice.bindValue(":curr_id", _customerPrice->id());
      salesDeterminePrice.bindValue(":effective", _customerPrice->effective());
      salesDeterminePrice.bindValue(":asof", asOf);
      salesDeterminePrice.exec();
      if (salesDeterminePrice.first())
      {
        _itemchar->setData(idx3, salesDeterminePrice.value("price").toString(), Qt::DisplayRole);
        _itemchar->setData(idx3, QVariant(_charVars), Qt::UserRole);
      }
      else if (salesDeterminePrice.lastError().type() != QSqlError::NoError)
      {
        systemError(this, salesDeterminePrice.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    connect(_itemchar, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(sRecalcPrice()));

    // Total up price for configured item characteristics
    QModelIndex idx;

    for (int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx        = _itemchar->index(i, CHAR_PRICE);
      charTotal += _itemchar->data(idx, Qt::DisplayRole).toDouble();
    }
  }
  // Now get item price information
  XSqlQuery itemprice;
  itemprice.prepare( "SELECT * FROM "
                     "itemIpsPrice(:item_id, :cust_id, :shipto_id, :qty, :qtyUOM, :priceUOM,"
                     "             :curr_id, :effective, :asof, :warehouse);" );
  itemprice.bindValue(":cust_id", _custid);
  itemprice.bindValue(":shipto_id", _shiptoid);
  itemprice.bindValue(":qty", _qtyOrdered->toDouble());
  itemprice.bindValue(":qtyUOM", _qtyUOM->id());
  itemprice.bindValue(":priceUOM", _priceUOM->id());
  itemprice.bindValue(":item_id", _item->id());
  itemprice.bindValue(":curr_id", _customerPrice->id());
  itemprice.bindValue(":effective", _customerPrice->effective());
  itemprice.bindValue(":asof", asOf);
  itemprice.bindValue(":warehouse", _warehouse->id());
  itemprice.exec();
  if (itemprice.first())
  {
    if (itemprice.value("itemprice_price").toDouble() == -9999.0)
    {
      if (!_updatePrice)
      {
        sCheckSupplyOrder();
        return;
      }

      // User expected an update, so let them know and reset
      QMessageBox::critical(this, tr("Customer Cannot Buy at Quantity"),
                            tr("<p>This item is marked as exclusive and "
                                 "no qualifying price schedule was found. "
                                 "You may click on the price list button "
                                 "(...) next to the Unit Price to determine "
                                 "if there is a minimum quantity the selected "
                                 "Customer may purchase." ) );
      _originalQtyOrd = 0.0;

      _customerPrice->clear();
      _netUnitPrice->clear();

      if (qtyChanged)
      {
        _qtyOrdered->clear();
        _qtyOrdered->setFocus();
      }
      else
      {
        _scheduledDate->clear();
        _scheduledDate->setFocus();
      }
    }
    else
    {
      double price = itemprice.value("itemprice_price").toDouble();
      _priceType = itemprice.value("itemprice_type").toString();
      if (_priceType == "N" || _priceType == "D" || _priceType == "P")  // nominal, discount, or list price
        _priceMode = "D";
      else  // markup or list cost
        _priceMode = "M";

      _baseUnitPrice->setLocalValue(price);
      _customerPrice->setLocalValue(price + charTotal);
      if (_updatePrice) // Configuration or user said they also want net unit price updated
        _netUnitPrice->setLocalValue(price + charTotal);

      sCalculateDiscountPrcnt();
      _qtyOrderedCache = _qtyOrdered->toDouble();
      _priceUOMCache = _priceUOM->id();
      _scheduledDateCache = _scheduledDate->date();
    }
  }
  else if (itemprice.lastError().type() != QSqlError::NoError)
            systemError(this, itemprice.lastError().databaseText(), __FILE__, __LINE__);

  sCheckSupplyOrder();
}

void salesOrderItem::sPopulateItemInfo(int pItemid)
{
  XSqlQuery salesPopulateItemInfo;
  _itemchar->removeRows(0, _itemchar->rowCount());
  if (pItemid != -1)
  {
    XSqlQuery uom;
    uom.prepare("SELECT uom_id, uom_name"
                "  FROM item"
                "  JOIN uom ON (item_inv_uom_id=uom_id)"
                " WHERE(item_id=:item_id)"
                " UNION "
                "SELECT uom_id, uom_name"
                "  FROM item"
                "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
                "  JOIN uom ON (itemuomconv_to_uom_id=uom_id)"
                " WHERE((itemuomconv_from_uom_id=item_inv_uom_id)"
                "   AND (item_id=:item_id))"
                " UNION "
                "SELECT uom_id, uom_name"
                "  FROM item"
                "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
                "  JOIN uom ON (itemuomconv_from_uom_id=uom_id)"
                " WHERE((itemuomconv_to_uom_id=item_inv_uom_id)"
                "   AND (item_id=:item_id))"
                " ORDER BY uom_name;");
    uom.bindValue(":item_id", _item->id());
    uom.exec();
    _qtyUOM->populate(uom);
    _priceUOM->populate(uom);

    //  Grab the price for this item/customer/qty
    salesPopulateItemInfo.prepare( "SELECT item_type, item_config, uom_name,"
               "       item_inv_uom_id, item_price_uom_id,"
               "       iteminvpricerat(item_id) AS invpricerat,"
               "       item_listprice, item_fractional,"
               "       itemsite_createsopo, itemsite_dropship, itemsite_costmethod,"
               "       getItemTaxType(item_id, :taxzone) AS taxtype_id "
               "FROM item JOIN uom ON (item_inv_uom_id=uom_id)"
               "LEFT OUTER JOIN itemsite ON ((itemsite_item_id=item_id) AND (itemsite_warehous_id=:warehous_id)) "
               "WHERE (item_id=:item_id);" );
    salesPopulateItemInfo.bindValue(":item_id", pItemid);
    salesPopulateItemInfo.bindValue(":warehous_id", _warehouse->id());
    salesPopulateItemInfo.bindValue(":taxzone", _taxzoneid);
    salesPopulateItemInfo.exec();
    if (salesPopulateItemInfo.first())
    {
      if (salesPopulateItemInfo.value("itemsite_createsopo").toBool())
      {
        XSqlQuery itemsrc;
        itemsrc.prepare("SELECT itemsrc_id, itemsrc_item_id "
                        "FROM itemsrc "
                        "WHERE (itemsrc_item_id = :item_id) "
                        "  AND (itemsrc_active) "
                        "LIMIT 1;");
        itemsrc.bindValue(":item_id", _item->id());
        itemsrc.exec();
        if (itemsrc.first())
          ;   // do nothing
        else if (itemsrc.lastError().type() != QSqlError::NoError)
        {
            systemError(this, itemsrc.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
        else
        {
          QMessageBox::warning( this, tr("Cannot Create P/O"),
                                tr("<p> Purchase Orders cannot be automatically "
                                     "created for this Item as there are no Item "
                                     "Sources for it.  You must create one or "
                                     "more Item Sources for this Item before "
                                     "the application can automatically create "
                                     "Purchase Orders for it." ) );
          _createSupplyOrder->setEnabled(FALSE);
        }
      }

      if (_mode == cNew)
        sDeterminePrice();

      _priceRatio        = salesPopulateItemInfo.value("invpricerat").toDouble(); // Always ratio from default price uom
      _invuomid          = salesPopulateItemInfo.value("item_inv_uom_id").toInt();
      _invIsFractional   = salesPopulateItemInfo.value("item_fractional").toBool();
      _priceinvuomratio  = _priceRatio; // the ration from the currently selected price uom
      _qtyinvuomratio    = 1.0;

      _qtyUOM->setId(salesPopulateItemInfo.value("item_inv_uom_id").toInt());
      _priceUOM->setId(salesPopulateItemInfo.value("item_price_uom_id").toInt());

      _listPrice->setBaseValue(salesPopulateItemInfo.value("item_listprice").toDouble());
      _taxtype->setId(salesPopulateItemInfo.value("taxtype_id").toInt());

      sFindSellingWarehouseItemsites(_item->id());
      sPopulateItemsiteInfo();
      sCalculateDiscountPrcnt();

    }
    else if (salesPopulateItemInfo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, salesPopulateItemInfo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    _charVars.replace(ITEM_ID, _item->id());
    disconnect( _itemchar,  SIGNAL(itemChanged(QStandardItem *)), this, SLOT(sRecalcPrice()));
    disconnect( _itemchar,  SIGNAL(itemChanged(QStandardItem *)), this, SLOT(sRecalcAvailability()));

    // Populate customer part number if any
    salesPopulateItemInfo.prepare( "SELECT itemalias_number "
              "FROM itemalias"
              " JOIN item ON (item_id=itemalias_item_id)"
              " LEFT OUTER JOIN crmacct ON (itemalias_crmacct_id=crmacct_id)"
              "WHERE (item_id=:item_id)"
              "  AND (crmacct_cust_id=:cust_id OR itemalias_crmacct_id IS NULL);" );
    salesPopulateItemInfo.bindValue(":item_id", _item->id());
    salesPopulateItemInfo.bindValue(":cust_id", _custid);
    salesPopulateItemInfo.exec();
    if (salesPopulateItemInfo.first())
    {
       _customerPN->setText(salesPopulateItemInfo.value("itemalias_number").toString());
    }

    // Populate Characteristics
    salesPopulateItemInfo.prepare("SELECT char_id, char_name, "
              " CASE WHEN char_type < 2 THEN "
              "   charass_value "
              " ELSE "
              "   formatDate(charass_value::date) "
              "END AS f_charass_value, "
              " charass_value, charass_price "
              "FROM ("
              "SELECT "
              "  char_id, "
              "  char_type, "
              "  char_name,"
              "  char_order, "
              "  COALESCE(si.charass_value,i2.charass_value) AS charass_value,"
              "  COALESCE(si.charass_price,itemcharprice(:item_id,char_id,COALESCE(si.charass_value,i2.charass_value),:cust_id,:shipto_id,:qty,:curr_id,:effective),0)::numeric(16,4) AS charass_price "
              "FROM "
              "  (SELECT DISTINCT "
              "    char_id,"
              "    char_type, "
              "    char_name, "
              "    char_order "
              "   FROM charass, char"
              "   WHERE ((charass_char_id=char_id)"
              "   AND (charass_target_type='I')"
              "   AND (charass_target_id=:item_id) ) ) AS data"
              "  LEFT OUTER JOIN charass  si ON ((:coitem_id=si.charass_target_id)"
              "                              AND (:sotype=si.charass_target_type)"
              "                              AND (si.charass_char_id=char_id))"
              "  LEFT OUTER JOIN item     i1 ON (i1.item_id=:item_id)"
              "  LEFT OUTER JOIN charass  i2 ON ((i1.item_id=i2.charass_target_id)"
              "                              AND ('I'=i2.charass_target_type)"
              "                              AND (i2.charass_char_id=char_id)"
              "                              AND (i2.charass_default))) data2 "
              "ORDER BY char_order, char_name; ");
    salesPopulateItemInfo.bindValue(":coitem_id", _soitemid);
    salesPopulateItemInfo.bindValue(":cust_id", _custid);
    salesPopulateItemInfo.bindValue(":shipto_id", _shiptoid);
    salesPopulateItemInfo.bindValue(":qty", _qtyOrdered->toDouble() * _qtyinvuomratio);
    salesPopulateItemInfo.bindValue(":item_id", _item->id());
    salesPopulateItemInfo.bindValue(":curr_id", _customerPrice->id());
    salesPopulateItemInfo.bindValue(":effective", _customerPrice->effective());
    if (_mode & 0x20)
      salesPopulateItemInfo.bindValue(":sotype", "QI");
    else
      salesPopulateItemInfo.bindValue(":sotype", "SI");
    salesPopulateItemInfo.bindValue(":soitem_id", _soitemid);
    salesPopulateItemInfo.exec();
    int         row = 0;
    QModelIndex idx;
    while (salesPopulateItemInfo.next())
    {
      _itemchar->insertRow(_itemchar->rowCount());
      idx = _itemchar->index(row, CHAR_ID);
      _itemchar->setData(idx, salesPopulateItemInfo.value("char_name"), Qt::DisplayRole);
      _itemchar->setData(idx, salesPopulateItemInfo.value("char_id"), Qt::UserRole);
      idx = _itemchar->index(row, CHAR_VALUE);
      _itemchar->setData(idx, salesPopulateItemInfo.value("f_charass_value"), Qt::DisplayRole);
      _itemchar->setData(idx, _item->id(), Xt::IdRole);
      _itemchar->setData(idx, salesPopulateItemInfo.value("f_charass_value"), Qt::UserRole);
      idx = _itemchar->index(row, CHAR_PRICE);
      _itemchar->setData(idx, salesPopulateItemInfo.value("charass_price"), Qt::DisplayRole);
      _itemchar->setData(idx, QVariant(_charVars), Qt::UserRole);
      row++;
    }
    if (salesPopulateItemInfo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, salesPopulateItemInfo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    // Setup widgets and signals needed to handle configuration
    if (_item->isConfigured())
    {
      connect(_itemchar,  SIGNAL(itemChanged(QStandardItem *)), this, SLOT(sRecalcPrice()));
      connect(_itemchar,  SIGNAL(itemChanged(QStandardItem *)), this, SLOT(sRecalcAvailability()));
      _itemcharView->showColumn(CHAR_PRICE);
      _baseUnitPriceLit->show();
      _baseUnitPrice->setVisible(TRUE);
    }
    else
    {
      disconnect( _itemchar,  SIGNAL(itemChanged(QStandardItem *)), this, SLOT(sRecalcPrice()));
      disconnect( _itemchar,  SIGNAL(itemChanged(QStandardItem *)), this, SLOT(sRecalcAvailability()));
      _itemcharView->hideColumn(CHAR_PRICE);
      _baseUnitPriceLit->hide();
      _baseUnitPrice->setVisible(FALSE);
    }
  }
}

void salesOrderItem::sRecalcAvailability()
{
  sDetermineAvailability(TRUE);
}

void salesOrderItem::sDetermineAvailability()
{
  sDetermineAvailability(FALSE);
}

void salesOrderItem::sDetermineAvailability( bool p )
{
  if (  (_item->id()==_availabilityLastItemid) &&
        (_warehouse->id()==_availabilityLastWarehousid) &&
        (_scheduledDate->date()==_availabilityLastSchedDate) &&
        (_showAvailability->isChecked()==_availabilityLastShow) &&
        (_showIndented->isChecked()==_availabilityLastShowIndent) &&
        ((_qtyOrdered->toDouble() * _qtyinvuomratio)==_availabilityQtyOrdered) &&
        (!p) )
    return;

  _availabilityLastItemid      = _item->id();
  _availabilityLastWarehousid  = _warehouse->id();
  _availabilityLastSchedDate   = _scheduledDate->date();
  _availabilityLastShow        = _showAvailability->isChecked();
  _availabilityLastShowIndent  = _showIndented->isChecked();
  _availabilityQtyOrdered      = (_qtyOrdered->toDouble() * _qtyinvuomratio);

  _availability->clear();

  if (_item->isValid() && _warehouse->isValid() && _scheduledDate->isValid() && _showAvailability->isChecked())
  {
    XSqlQuery availability;
    QString sql = "SELECT itemsite_id,"
                  "       availableqoh,"
                  "       allocated,"
                  "       (noNeg(availableqoh - allocated)) AS unallocated,"
                  "       ordered,"
                  "       (availableqoh - allocated + ordered) AS available,"
                  "       reserved,"
                  "       reservable,"
                  "       itemsite_leadtime "
                  "FROM ( SELECT itemsite_id, qtyAvailable(itemsite_id) AS availableqoh,"
                  "              qtyAllocated(itemsite_id, DATE(<? value('date') ?>)) AS allocated,"
                  "              qtyOrdered(itemsite_id, DATE(<? value('date') ?>)) AS ordered, "
                  "<? if exists('includeReservations') ?>"
                  "              COALESCE((SELECT coitem_qtyreserved"
                  "                        FROM coitem"
                  "                        WHERE coitem_id=<? value('soitem_id') ?>), 0.0) AS reserved,"
                  "              (qtyAvailable(itemsite_id) - qtyreserved(itemsite_id)) AS reservable,"
                  "<? else ?>"
                  "              0.0 AS reserved,"
                  "              0.0 AS reservable,"
                  "<? endif ?>"
                  "              itemsite_leadtime "
                  "       FROM itemsite, item "
                  "       WHERE ((itemsite_item_id=item_id)"
                  "        AND (item_id=<? value('item_id') ?>)"
                  "        AND (itemsite_warehous_id=<? value('warehous_id') ?>)) ) AS data;";
    ParameterList params;
    params.append("date", _scheduledDate->date());
    params.append("item_id", _item->id());
    params.append("warehous_id", _warehouse->id());
    params.append("soitem_id", _soitemid);
    params.append("includeReservations", (ISORDER(_mode) && _metrics->boolean("EnableSOReservations")));
    MetaSQLQuery mql(sql);
    availability = mql.toQuery(params);
    if (availability.first())
    {
      _onHand->setDouble(availability.value("availableqoh").toDouble());
      _allocated->setDouble(availability.value("allocated").toDouble());
      _unallocated->setDouble(availability.value("unallocated").toDouble());
      _onOrder->setDouble(availability.value("ordered").toDouble());
      _available->setDouble(availability.value("available").toDouble());
      _reserved->setDouble(availability.value("reserved").toDouble());
      _reservable->setDouble(availability.value("reservable").toDouble());
      _leadtime->setText(availability.value("itemsite_leadtime").toString());

      QString stylesheet;
      if (availability.value("available").toDouble() < _availabilityQtyOrdered)
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
      _available->setStyleSheet(stylesheet);

      if ((_item->itemType() == "M") || (_item->itemType() == "K"))
      {
        if (_showIndented->isChecked())
        {
          QString sql(
            "SELECT itemsite_id, reorderlevel,"
            "       bomdata_bomwork_level,"
            "       bomdata_bomwork_id,"
            "       bomdata_bomwork_parent_id,"
            "       bomdata_bomwork_seqnumber AS seqnumber,"
            "       bomdata_item_number AS item_number,"
            "       bomdata_itemdescription AS item_descrip,"
            "       bomdata_uom_name AS uom_name,"
            "       pendalloc,"
            "       ordered,"
            "       availableqoh, "
            "       (totalalloc + pendalloc) AS totalalloc,"
            "       (availableqoh + ordered - (totalalloc + pendalloc)) AS totalavail,"
            "       'qty' AS pendalloc_xtnumericrole,"
            "       'qty' AS ordered_xtnumericrole,"
            "       'qty' AS availableqoh_xtnumericrole,"
            "       'qty' AS totalalloc_xtnumericrole,"
            "       'qty' AS totalavail_xtnumericrole,"
            "       CASE WHEN availableqoh < pendalloc THEN 'error'"
            "            WHEN (availableqoh + ordered - (totalalloc + pendalloc)) < 0  THEN 'error'"
            "            WHEN (availableqoh + ordered - (totalalloc + pendalloc)) < reorderlevel THEN 'warning'"
            "       END AS qtforegroundrole,"
            "       bomdata_bomwork_level - 1 AS xtindentrole "
            "  FROM ( SELECT itemsite_id,"
            "                CASE WHEN(itemsite_useparams)"
            "                     THEN itemsite_reorderlevel"
            "                     ELSE 0.0"
            "                     END AS reorderlevel,"
            "                ib.*, "
            "                ((bomdata_qtyfxd::NUMERIC + bomdata_qtyper::NUMERIC * :qty) * (1 + bomdata_scrap::NUMERIC))"
            "                                       AS pendalloc,"
            "                (qtyAllocated(itemsite_id, DATE(:schedDate)) -"
            "                             ((bomdata_qtyfxd::NUMERIC + bomdata_qtyper::NUMERIC * :origQtyOrd) *"
            "                              (1 + bomdata_scrap::NUMERIC)))"
            "                                       AS totalalloc,"
            "                qtyAvailable(itemsite_id) AS availableqoh,"
            "                qtyOrdered(itemsite_id, DATE(:schedDate))"
            "                                                AS ordered"
            "           FROM indentedBOM(:item_id, "
            "                            getActiveRevId('BOM', :item_id),"
            "                            0,0) ib LEFT OUTER JOIN"
            "                itemsite ON ((itemsite_item_id=bomdata_item_id)"
            "                         AND (itemsite_warehous_id=:warehous_id))"
            "          WHERE (bomdata_item_id > 0)");

          if (_item->isConfigured())  // For configured items limit to bomitems associated with selected characteristic values
          {
            QModelIndex charidx;
            QModelIndex valueidx;

            sql +=  "        AND ((bomdata_char_id IS NULL) ";

            for (int i = 0; i < _itemchar->rowCount(); i++)
            {
              charidx  = _itemchar->index(i, CHAR_ID);
              valueidx = _itemchar->index(i, CHAR_VALUE);
              sql     += QString(" OR ((bomdata_char_id=%1) AND (bomdata_value='%2'))")
                         .arg(_itemchar->data(charidx, Qt::UserRole).toString())
                         .arg(_itemchar->data(valueidx, Qt::DisplayRole).toString().replace("'", "''"));
            }

            sql +=  " ) ";
          }

          sql += "       ) AS data "
                 "ORDER BY bomworkSequence(bomdata_bomwork_id);";

          availability.prepare(sql);
          availability.bindValue(":item_id",        _item->id());
          availability.bindValue(":warehous_id",    _warehouse->id());
          availability.bindValue(":qty",            _availabilityQtyOrdered);
          availability.bindValue(":schedDate",      _scheduledDate->date());
          availability.bindValue(":origQtyOrd",     _originalQtyOrd);
          availability.exec();
          _availability->populate(availability);
          if (availability.lastError().type() != QSqlError::NoError)
          {
            systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
          _availability->expandAll();
        }
        else
        {
          int     itemsiteid = availability.value("itemsite_id").toInt();
          QString sql("SELECT itemsiteid, reorderlevel,"
                      "       bomitem_seqnumber AS seqnumber, item_number, "
                      "       item_descrip, uom_name,"
                      "       pendalloc, "
                      "       ordered, "
                      "       availableqoh, "
                      "       (totalalloc + pendalloc) AS totalalloc,"
                      "       (availableqoh + ordered - (totalalloc + pendalloc)) AS totalavail,"
                      "       'qty' AS pendalloc_xtnumericrole,"
                      "       'qty' AS ordered_xtnumericrole,"
                      "       'qty' AS availableqoh_xtnumericrole,"
                      "       'qty' AS totalalloc_xtnumericrole,"
                      "       'qty' AS totalavail_xtnumericrole,"
                      "       CASE WHEN availableqoh < pendalloc THEN 'error'"
                      "            WHEN (availableqoh + ordered - (totalalloc + pendalloc)) < 0  THEN 'error'"
                      "            WHEN (availableqoh + ordered - (totalalloc + pendalloc)) < reorderlevel THEN 'warning'"
                      "       END AS qtforegroundrole "
                      "FROM ( SELECT cs.itemsite_id AS itemsiteid,"
                      "              CASE WHEN(cs.itemsite_useparams) THEN cs.itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
                      "              bomitem_seqnumber, item_number,"
                      "              (item_descrip1 || ' ' || item_descrip2) AS item_descrip, uom_name,"
                      "              itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, (bomitem_qtyfxd + bomitem_qtyper * :qty) * (1 + bomitem_scrap)) AS pendalloc,"
                      "              (qtyAllocated(cs.itemsite_id, DATE(:schedDate)) - itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, (bomitem_qtyfxd + bomitem_qtyper * :origQtyOrd) * (1 + bomitem_scrap))) AS totalalloc,"
                      "              qtyAvailable(cs.itemsite_id) AS availableqoh,"
                      "              qtyOrdered(cs.itemsite_id, DATE(:schedDate)) AS ordered "
                      "       FROM item, bomitem LEFT OUTER JOIN"
                      "            itemsite AS cs ON ((cs.itemsite_warehous_id=:warehous_id)"
                      "                           AND (cs.itemsite_item_id=bomitem_item_id)),"
                      "            uom,"
                      "            itemsite AS ps "
                      "       WHERE ( (bomitem_item_id=item_id)"
                      "        AND (item_inv_uom_id=uom_id)"
                      "        AND (bomitem_parent_item_id=ps.itemsite_item_id)"
                      "        AND (bomitem_rev_id=getActiveRevId('BOM',bomitem_parent_item_id))"
                      "        AND (:schedDate BETWEEN bomitem_effective AND (bomitem_expires-1))");

          if (_item->isConfigured())  // For configured items limit to bomitems associated with selected characteristic values
          {
            QModelIndex charidx;
            QModelIndex valueidx;

            sql +=  "        AND ((bomitem_char_id IS NULL) ";

            for (int i = 0; i < _itemchar->rowCount(); i++)
            {
              charidx  = _itemchar->index(i, CHAR_ID);
              valueidx = _itemchar->index(i, CHAR_VALUE);
              sql     += QString(" OR ((bomitem_char_id=%1) AND (bomitem_value='%2'))").arg(_itemchar->data(charidx, Qt::UserRole).toString()).arg(_itemchar->data(valueidx, Qt::DisplayRole).toString().replace("'", "''"));
            }

            sql +=  " ) ";
          }
          sql +=  "        AND (ps.itemsite_id=:itemsite_id) ) ) AS data "
                  "ORDER BY bomitem_seqnumber;";
          availability.prepare(sql);
          availability.bindValue(":itemsite_id", itemsiteid);
          availability.bindValue(":warehous_id", _warehouse->id());
          availability.bindValue(":qty", _availabilityQtyOrdered);
          availability.bindValue(":schedDate", _scheduledDate->date());
          availability.bindValue(":origQtyOrd", _originalQtyOrd);
          availability.exec();
          _availability->populate(availability);
          if (availability.lastError().type() != QSqlError::NoError)
          {
            systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
        }
      }
      else
        _availability->setEnabled(FALSE);
    }
    else if (availability.lastError().type() != QSqlError::NoError)
    {
            systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _onHand->clear();
    _allocated->clear();
    _unallocated->clear();
    _onOrder->clear();
    _available->clear();
    _leadtime->clear();
  }
}

void salesOrderItem::sPopulateItemSources(int pItemid)
{
  XSqlQuery priceq;
  MetaSQLQuery mql = mqlLoad("itemSources", "prices");
  ParameterList params;
  params.append("item_id", pItemid);
  params.append("nominal",tr("Nominal"));
  params.append("discount",tr("Discount"));
  params.append("price", tr("Price"));
  params.append("fixed", tr("Fixed"));
  params.append("percent", tr("Percent"));
  params.append("mixed", tr("Mixed"));

  priceq = mql.toQuery(params);
  _itemsrcp->populate(priceq);
}

void salesOrderItem::sPopulateItemSubs(int pItemid)
{
  if (_item->isValid() && _warehouse->isValid())
  {
    XSqlQuery subq;
    MetaSQLQuery mql = mqlLoad("substituteAvailability", "detail");
    ParameterList params;
    params.append("item_id", pItemid);
    params.append("warehous_id", _warehouse->id());
    params.append("byDate", true);
    if (_scheduledDate->isValid())
      params.append("date", _scheduledDate->date());
    else
      params.append("date", omfgThis->dbDate());
    
    subq = mql.toQuery(params);
    _subs->populate(subq);
  }
}

void salesOrderItem::sPopulateSubMenu(QMenu *menu, QTreeWidgetItem*, int)
{
  if ( !_partialsaved && ((_mode == cNew) || (_mode == cNewQuote)) )
  menu->addAction(tr("Substitute..."), this, SLOT(sSubstitute()));
}

void salesOrderItem::sSubstitute()
{
  int _itemsiteid = _subs->id();
  _sub->setChecked(true);
  _subItem->setId(_item->id());
  _item->setItemsiteid(_itemsiteid);
}

void salesOrderItem::sReserveStock()
{
  if (_metrics->boolean("SOManualReservations"))
  {
    ParameterList params;
    params.append("soitem_id", _soitemid);
    
    reserveSalesOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else
  {
    XSqlQuery reserveSales;
    reserveSales.prepare("SELECT reserveSoLineBalance(:soitem_id) AS result;");
    reserveSales.bindValue(":soitem_id", _soitemid);
    reserveSales.exec();
    if (reserveSales.first())
    {
      int result = reserveSales.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("reserveSoLineBalance", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (reserveSales.lastError().type() != QSqlError::NoError)
    {
      systemError(this, reserveSales.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void salesOrderItem::sPopulateHistory()
{
  if (_historyCostsButton->isChecked())
  {
    XSqlQuery historyq;
    MetaSQLQuery historymql = mqlLoad("receivings", "detail");
    ParameterList params;
    params.append("item_id", _item->id());
    params.append("warehous_id", _warehouse->id());
    params.append("startDate", _historyDates->startDate());
    params.append("endDate", _historyDates->endDate());
    params.append("received", tr("Received"));
    params.append("returned", tr("Returned"));
    params.append("unvouchered", tr("Not Vouchered"));
    params.append("nonInv",   tr("NonInv - "));
    params.append("na",       tr("N/A"));
    historyq = historymql.toQuery(params);
    _historyCosts->populate(historyq);
  }
  else
  {
    XSqlQuery historyq;
    MetaSQLQuery historymql = mqlLoad("salesHistory", "detail");
    ParameterList params;
    params.append("cust_id", _custid);
    params.append("item_id", _item->id());
    params.append("warehous_id", _warehouse->id());
    params.append("startDate", _historyDates->startDate());
    params.append("endDate", _historyDates->endDate());
    historyq = historymql.toQuery(params);
    _historySales->populate(historyq);
  }
}

void salesOrderItem::sCalculateDiscountPrcnt()
{
  double  netUnitPrice = _netUnitPrice->baseValue();
  double  charTotal    = 0;

  if (netUnitPrice == 0.0)
  {
    _discountFromListPrice->setDouble(100.0);
    _markupFromUnitCost->setDouble(100.0);
    _discountFromCust->setDouble(100.0);
  }
  else
  {
    if (_listPrice->isZero())
      _discountFromListPrice->setText(tr("N/A"));
    else
      _discountFromListPrice->setDouble((1.0 - (netUnitPrice / _listPrice->baseValue())) * 100.0);
    
    if (_unitCost->isZero())
      _markupFromUnitCost->setText(tr("N/A"));
    else if (_metrics->boolean("Long30Markups"))
      _markupFromUnitCost->setDouble((1.0 - (_unitCost->baseValue() / netUnitPrice)) * 100.0);
    else
      _markupFromUnitCost->setDouble(((netUnitPrice / _unitCost->baseValue()) - 1.0) * 100.0);
    
    if (_customerPrice->isZero())
      _discountFromCust->setText(tr("N/A"));
    else
      _discountFromCust->setDouble((1.0 - (netUnitPrice / _customerPrice->baseValue())) * 100.0);
  }

  if (_item->isConfigured())  // Total up price for configured item characteristics
  {
    QModelIndex idx;

    for (int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx        = _itemchar->index(i, CHAR_PRICE);
      charTotal += _itemchar->data(idx, Qt::DisplayRole).toDouble();
    }
    _baseUnitPrice->setLocalValue(_netUnitPrice->localValue() - charTotal);
  }

  _margin->setLocalValue(((_netUnitPrice->localValue() - _unitCost->localValue()) / _priceinvuomratio) * (_qtyOrdered->toDouble() * _qtyinvuomratio));

  sCalculateExtendedPrice();
}

void salesOrderItem::sCalculateExtendedPrice()
{
  _extendedPrice->setLocalValue(((_qtyOrdered->toDouble() * _qtyinvuomratio) / _priceinvuomratio) * _netUnitPrice->localValue());
}

void salesOrderItem::sCheckSupplyOrder()
{
//  QMessageBox::critical(this, tr("Debug"),
//                        tr("sCheckSupplyOrder called."));
  if ( (_item->isValid()) &&
      (_warehouse->isValid()) &&
      (_scheduledDate->isValid()) &&
      (_qtyOrdered->toDouble() > 0.0) &&
      (_supplyOrderType != "") )
  {
    if (_createSupplyOrder->isChecked())
      sHandleSupplyOrder();
    else
      if (((_mode == cNew || _mode == cNewQuote) && !_stocked) || _costmethod == "J" || _supplyOrderId > -1)
        _createSupplyOrder->setChecked(true);
  }
}

void salesOrderItem::sHandleSupplyOrder()
{
  /*
  if (_createSupplyOrder->isChecked())
    QMessageBox::critical(this, tr("Debug"),
                          tr("sHandleSupplyOrder called with _createSupplyOrder checked."));
  else
    QMessageBox::critical(this, tr("Debug"),
                          tr("sHandleSupplyOrder called with _createSupplyOrder unchecked."));
  if (_supplyOrderId == -1)
    QMessageBox::critical(this, tr("Debug"),
                          tr("sHandleSupplyOrder called with _supplyOrderId=-1."));
  else
    QMessageBox::critical(this, tr("Debug"),
                          tr("sHandleSupplyOrder called with _supplyOrderId set."));
  */
  
  if (ISQUOTE(_mode))
    return;
  
  XSqlQuery ordq;
  if (_createSupplyOrder->isChecked() && ISORDER(_mode))
  { // createSupplyOrder is checked
    
    double valqty = 0.0;
    ordq.prepare( "SELECT validateOrderQty(itemsite_id, :qty, TRUE) AS qty "
                  "FROM itemsite "
                  "WHERE ((itemsite_item_id=:item_id)"
                  " AND (itemsite_warehous_id=:warehous_id));" );
    ordq.bindValue(":qty", _qtyOrdered->toDouble() * _qtyinvuomratio);
    ordq.bindValue(":item_id", _item->id());
    ordq.bindValue(":warehous_id", (_item->itemType() == "M") ? _supplyWarehouse->id() : _warehouse->id());
    ordq.exec();
    if (ordq.first())
      valqty = ordq.value("qty").toDouble();
    else if (ordq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
      
    if (_supplyOrderId == -1)
    { // supply order does not exist
      // first save the sales order item
      sSave(true);
      if (_modified)  // catch an error saving
      {
        _createSupplyOrder->setChecked(false);
        return;
      }
      
      // check _supplyOrderType to determine type of order
      if (_supplyOrderType == "W")
      {
        ordq.prepare( "SELECT createWo(:orderNumber, itemsite_id, :qty, itemsite_leadtime, :dueDate,"
                     ":comments, :parent_type, :parent_id ) AS result, itemsite_id "
                     "FROM itemsite "
                     "WHERE ( (itemsite_item_id=:item_id)"
                     " AND (itemsite_warehous_id=:warehous_id) );" );
        ordq.bindValue(":orderNumber", _orderNumber->text().toInt());
        ordq.bindValue(":qty", valqty);
        ordq.bindValue(":dueDate", _scheduledDate->date());
        ordq.bindValue(":comments", _custName + "\n" + _notes->toPlainText());
        ordq.bindValue(":item_id", _item->id());
        ordq.bindValue(":warehous_id", _supplyWarehouse->id());
        ordq.bindValue(":parent_type", QString("S"));
        ordq.bindValue(":parent_id", _soitemid);
        ordq.exec();
        if (ordq.first())
        {
          int _woid = ordq.value("result").toInt();
          if ((_woid > 0) && _item->isConfigured())
          {
            XSqlQuery implodeq;
            implodeq.prepare( "SELECT implodeWo(:wo_id, true) AS result;" );
            implodeq.bindValue(":wo_id", _woid);
            implodeq.exec();
          }
        }
        else if (ordq.lastError().type() != QSqlError::NoError)
        {
          systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
      }
      else if (_supplyOrderType == "P")
      {
        int   itemsrcid  = _itemsrc;
        int   poheadid   = -1;
        
        if (itemsrcid==-1)
        {
          ordq.prepare("SELECT itemsrc_id FROM itemsrc "
                       "WHERE ((itemsrc_item_id=:item_id) AND (itemsrc_default) AND (itemsrc_active)) ");
          ordq.bindValue(":item_id", _item->id());
          ordq.exec();
          if (ordq.first())
          {
            itemsrcid=(ordq.value("itemsrc_id").toInt());
          }
          else if (ordq.lastError().type() != QSqlError::NoError)
          {
            systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
          else
          {
            ParameterList itemSourceParams;
            itemSourceParams.append("item_id", _item->id());
            itemSourceParams.append("qty", valqty);
            itemSourceList newdlg(omfgThis, "", TRUE);
            newdlg.set(itemSourceParams);
            itemsrcid = newdlg.exec();
          }
        }
        
        ordq.prepare( "SELECT itemsrc_vend_id, vend_name  "
                      "FROM itemsrc LEFT OUTER JOIN vendinfo ON (vend_id = itemsrc_vend_id) "
                      "WHERE (itemsrc_id=:itemsrc_id);" );
        ordq.bindValue(":itemsrc_id", itemsrcid);
        ordq.exec();
        if (!ordq.first())
        {
          systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
        else
        {
          int vendid = ordq.value("itemsrc_vend_id").toInt();
          QString vendname = ordq.value("vend_name").toString();
          ordq.prepare( "SELECT pohead_id "
                        "FROM pohead "
                        "WHERE (pohead_vend_id = :vend_id)"
                        "  AND (pohead_status = 'U')"
                        "  AND (pohead_dropship = :drop_ship) "
                        "  AND (NOT pohead_dropship OR (pohead_cohead_id = :sohead_id));" );
          ordq.bindValue(":drop_ship", _supplyDropShip->isChecked());
          ordq.bindValue(":vend_id", vendid);
          ordq.bindValue(":sohead_id", _soheadid);
          ordq.exec();
          if (ordq.first())
          {
            if(QMessageBox::question( this, tr("Purchase Order Exists"),
                                     tr("An Unreleased Purchase Order already exists for this Vendor.\n"
                                        "Click Yes to use an existing Purchase Order\n"
                                        "otherwise a new one will be created."),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
            {
              ParameterList openPurchaseOrderParams;
              openPurchaseOrderParams.append("vend_id", vendid);
              openPurchaseOrderParams.append("vend_name", vendname);
              openPurchaseOrderParams.append("drop_ship", _supplyDropShip->isChecked());
              openPurchaseOrderParams.append("sohead_id", _soheadid);
              openPurchaseOrder newdlg(omfgThis, "", TRUE);
              newdlg.set(openPurchaseOrderParams);
              poheadid = newdlg.exec();
              if (poheadid == XDialog::Rejected)
                return;
            }
          }
        }

        ordq.prepare("SELECT createPurchaseToSale(:soitem_id, :itemsrc_id, :drop_ship,"
                     "                            :qty, :duedate, :price, :pohead_id) AS result;");
        ordq.bindValue(":soitem_id", _soitemid);
        ordq.bindValue(":itemsrc_id", itemsrcid);
        ordq.bindValue(":drop_ship", _supplyDropShip->isChecked());
        ordq.bindValue(":qty", valqty);
        ordq.bindValue(":duedate", _scheduledDate->date());
        if (_supplyOverridePrice->localValue() > 0.00)
          ordq.bindValue(":price", _supplyOverridePrice->localValue());
        ordq.bindValue(":pohead_id", poheadid);
        ordq.exec();
      }
      else if (_supplyOrderType == "R")
      {
        ordq.prepare("SELECT createPr(:orderNumber, 'S', :soitem_id) AS result;");
        ordq.bindValue(":orderNumber", _orderNumber->text().toInt());
        ordq.bindValue(":soitem_id", _soitemid);
        ordq.exec();
      }
      
      if (ordq.first())
      {
        _supplyOrderId = ordq.value("result").toInt();
        if (_supplyOrderId < 0)
        {
          QString procname;
          if (_supplyOrderType == "W")
            procname = "createWo";
          else if (_supplyOrderType == "R")
            procname = "createPr";
          else if (_supplyOrderType == "P")
            procname = "createPurchaseToSale";
          else
            procname = "unnamed stored procedure";
          systemError(this, storedProcErrorLookup(procname, _supplyOrderId),
                      __FILE__, __LINE__);
          return;
        }
        // save the sales order item again to capture the supply order id
        sSave(true);
        
        ordq.prepare("INSERT INTO charass"
                     "      (charass_target_type, charass_target_id,"
                     "       charass_char_id, charass_value) "
                     "SELECT :ordertype, :orderid, charass_char_id, charass_value"
                     "  FROM charass"
                     " WHERE ((charass_target_type='SI')"
                     "   AND  (charass_target_id=:soitem_id));");
        ordq.bindValue(":ordertype", _supplyOrderType);
        ordq.bindValue(":orderid", _supplyOrderId);
        ordq.bindValue(":soitem_id", _soitemid);
        ordq.exec();
        if (ordq.lastError().type() != QSqlError::NoError)
        {
          systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
      }
      else if (ordq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }  // end supply order does not exist
    else
    {  // supply order exists
      // first process any potential changes
      
      double qtyordinv = (_qtyOrdered->toDouble() * _qtyinvuomratio);
      if ( (qtyordinv != _supplyOrderQtyOrderedInvCache) ||
           (_supplyOrderQty->toDouble() != _supplyOrderQtyCache) )
      { // Qty ordered change
        if (_supplyOrderType == "W")
        { // WO qty change
          if (qtyordinv != _supplyOrderQtyOrderedInvCache)
          { // qty ordered changed
            bool applychange = false;
            if (_mode == cNew)
              applychange = true;
            else if (QMessageBox::question(this, tr("Change Work Order Quantity?"),
                                           tr("<p>The Quantity Ordered for this Line Item has changed "
                                              "from %1 to %2."
                                              "<p>Should the W/O quantity for this Line Item be changed?")
                                           .arg(_supplyOrderQtyOrderedInvCache).arg(qtyordinv),
                                           QMessageBox::No | QMessageBox::Escape,
                                           QMessageBox::Yes  | QMessageBox::Default) == QMessageBox::Yes)
              applychange = true;
            if (applychange)
            {
              ordq.prepare("SELECT changeWoQty(:wo_id, :qty, TRUE) AS result;");
              ordq.bindValue(":wo_id", _supplyOrderId);
              ordq.bindValue(":qty", valqty);
              ordq.exec();
              if (ordq.first())
              {
                int result = ordq.value("result").toInt();
                if (result < 0)
                {
                  systemError(this, storedProcErrorLookup("changeWoQty", result), __FILE__, __LINE__);
                  return;
                }
              }
              else if (ordq.lastError().type() != QSqlError::NoError)
              {
                systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
                return;
              }
            }
          } // end qty ordered changed
          else if (_supplyOrderQty->toDouble() != _supplyOrderQtyCache)
          { // supply ord qty changed
            ordq.prepare("SELECT changeWoQty(:wo_id, :qty, TRUE) AS result;");
            ordq.bindValue(":wo_id", _supplyOrderId);
            ordq.bindValue(":qty", _supplyOrderQty->toDouble());
            ordq.exec();
            if (ordq.first())
            {
              int result = ordq.value("result").toInt();
              if (result < 0)
              {
                systemError(this, storedProcErrorLookup("changeWoQty", result), __FILE__, __LINE__);
                return;
              }
            }
            else if (ordq.lastError().type() != QSqlError::NoError)
            {
              systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
              return;
            }
          } // end supply ord qty changed
        } // end WO qty change
        else if (_supplyOrderType == "P")
        { // PO qty change
          if (_supplyOrderStatus->text() == "C")
          {
            QMessageBox::critical(this, tr("Cannot Update Supply Order"),
                                  tr("The Purchase Order Item this Sales Order Item is linked to is closed.\n"
                                     "The quantity may not be updated."));
            _qtyOrdered->setDouble(_supplyOrderQtyOrderedCache);
            return;
          }
          
          if (qtyordinv != _supplyOrderQtyOrderedInvCache)
          { // qty ordered changed
            bool applychange = false;
            if (_mode == cNew)
              applychange = true;
            else if (QMessageBox::question(this, tr("Change P/O Quantity?"),
                                           tr("<p>The Quantity Ordered for this Line Item has changed "
                                              "from %1 to %2."
                                              "<p>Should the P/O quantity for this Line Item be changed?")
                                           .arg(_supplyOrderQtyOrderedInvCache).arg(qtyordinv),
                                           QMessageBox::Yes | QMessageBox::Default,
                                           QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
              applychange = true;
            if (applychange)
            {
              ordq.prepare("SELECT changePoitemQty(:poitem_id, :qty, true) AS result;");
              ordq.bindValue(":poitem_id", _supplyOrderId);
              ordq.bindValue(":qty", valqty);
              ordq.exec();
              if (ordq.first())
              {
                bool result = ordq.value("result").toBool();
                if (!result)
                {
                  systemError(this, tr("changePoQty failed"), __FILE__, __LINE__);
                  return;
                }
              }
              else if (ordq.lastError().type() != QSqlError::NoError)
              {
                systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
                return;
              }
            }
          } // end qty ordered changed
          else if (_supplyOrderQty->toDouble() != _supplyOrderQtyCache)
          { // supply ord qty changed
            ordq.prepare("SELECT changePoitemQty(:poitem_id, :qty, true) AS result;");
            ordq.bindValue(":poitem_id", _supplyOrderId);
            ordq.bindValue(":qty", _supplyOrderQty->toDouble());
            ordq.exec();
            if (ordq.first())
            {
              bool result = ordq.value("result").toBool();
              if (!result)
              {
                systemError(this, tr("changePoQty failed"), __FILE__, __LINE__);
                return;
              }
            }
            else if (ordq.lastError().type() != QSqlError::NoError)
            {
              systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
              return;
            }
          } // end supply ord qty changed
        } // end PO qty change
        else if (_supplyOrderType == "R")
        { // PR qty change
          if (qtyordinv != _supplyOrderQtyOrderedInvCache)
          { // qty ordered changed
            bool applychange = false;
            if (_mode == cNew)
              applychange = true;
            else if (QMessageBox::question(this, tr("Change P/R Quantity?"),
                                           tr("<p>The Supply Order Quantity for this Line Item has changed "
                                              "from %1 to %2."
                                              "<p>Should the P/R quantity for this Line Item be changed?")
                                           .arg(_supplyOrderQtyOrderedInvCache).arg(qtyordinv),
                                           QMessageBox::Yes | QMessageBox::Default,
                                           QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
              applychange = true;
            if (applychange)
            {
              ordq.prepare("SELECT changePrQty(:pr_id, :qty) AS result;");
              ordq.bindValue(":pr_id", _supplyOrderId);
              ordq.bindValue(":qty", valqty);
              ordq.exec();
              if (ordq.first())
              {
                bool result = ordq.value("result").toBool();
                if (!result)
                {
                  systemError(this, tr("changePrQty failed"), __FILE__, __LINE__);
                  return;
                }
              }
              else if (ordq.lastError().type() != QSqlError::NoError)
              {
                systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
                return;
              }
            }
          } // end qty ordered changed
          else if (_supplyOrderQty->toDouble() != _supplyOrderQtyCache)
          { // supply ord qty changed
            ordq.prepare("SELECT changePrQty(:pr_id, :qty) AS result;");
            ordq.bindValue(":pr_id", _supplyOrderId);
            ordq.bindValue(":qty", _supplyOrderQty->toDouble());
            ordq.exec();
            if (ordq.first())
            {
              bool result = ordq.value("result").toBool();
              if (!result)
              {
                systemError(this, tr("changePrQty failed"), __FILE__, __LINE__);
                return;
              }
            }
            else if (ordq.lastError().type() != QSqlError::NoError)
            {
              systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
              return;
            }
          } // end supply ord qty changed
        } // end PR qty change
      } // end Qty Ordered change
      
      if ( (_scheduledDate->date() != _supplyOrderScheduledDateCache) ||
           (_supplyOrderDueDate->date() != _supplyOrderDueDateCache) )
      { // Scheduled date change
        if (_supplyOrderType == "W")
        { // WO date change
          if (_scheduledDate->date() != _supplyOrderScheduledDateCache)
          { // scheduled date changed
            bool applychange = false;
            if (_mode == cNew)
              applychange = true;
            else if (QMessageBox::question(this, tr("Reschedule Work Order?"),
                                           tr("<p>The Scheduled Date for this Line Item has changed."
                                              "<p>Should the W/O for this Line Item be rescheduled?"),
                                           QMessageBox::Yes | QMessageBox::Default,
                                           QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
              applychange = true;
            if (applychange)
            {
              ordq.prepare("SELECT changeWoDates(:wo_id, wo_startdate + (:dueDate-wo_duedate), :dueDate, TRUE) AS result "
                           "FROM wo "
                           "WHERE (wo_id=:wo_id);");
              ordq.bindValue(":wo_id", _supplyOrderId);
              ordq.bindValue(":dueDate", _scheduledDate->date());
              ordq.exec();
              if (ordq.first())
              {
                int result = ordq.value("result").toInt();
                if (result < 0)
                {
                  systemError(this, storedProcErrorLookup("changeWoDates", result),
                              __FILE__, __LINE__);
                  return;
                }
              }
              else if (ordq.lastError().type() != QSqlError::NoError)
              {
                systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
                return;
              }
            }
          } // end scheduled date changed
          else if (_supplyOrderDueDate->date() != _supplyOrderDueDateCache)
          { // supply ord due date changed
            ordq.prepare("SELECT changeWoDates(:wo_id, wo_startdate + (:dueDate-wo_duedate), :dueDate, TRUE) AS result "
                         "FROM wo "
                         "WHERE (wo_id=:wo_id);");
            ordq.bindValue(":wo_id", _supplyOrderId);
            ordq.bindValue(":dueDate", _supplyOrderDueDate->date());
            ordq.exec();
            if (ordq.first())
            {
              int result = ordq.value("result").toInt();
              if (result < 0)
              {
                systemError(this, storedProcErrorLookup("changeWoDates", result),
                            __FILE__, __LINE__);
                return;
              }
            }
            else if (ordq.lastError().type() != QSqlError::NoError)
            {
              systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
              return;
            }
          } // end supply ord due date changed
        }  // end WO date change
        else if (_supplyOrderType == "P")
        { // PO date change
          if (_supplyOrderStatus->text() == "C")
          {
            QMessageBox::critical(this, tr("Cannot Update Supply Order"),
                                  tr("The Purchase Order Item this Sales Order Item is linked to is closed.\n"
                                     "The due date may not be updated."));
            _scheduledDate->setDate(_supplyOrderScheduledDateCache);
            return;
          }
            
          if (_scheduledDate->date() != _supplyOrderScheduledDateCache)
          { // scheduled date changed
            bool applychange = false;
            if (_mode == cNew)
              applychange = true;
            else if (QMessageBox::question(this, tr("Reschedule P/O?"),
                                           tr("<p>The Scheduled Date for this Line Item has changed."
                                              "<p>Should the P/O for this Line Item be rescheduled?"),
                                           QMessageBox::Yes | QMessageBox::Default,
                                           QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
              applychange = true;
            if (applychange)
            {
              ordq.prepare("SELECT changePoitemDueDate(:poitem_id, :dueDate, true) AS result;");
              ordq.bindValue(":poitem_id", _supplyOrderId);
              ordq.bindValue(":dueDate", _scheduledDate->date());
              ordq.exec();
              if (ordq.first())
              {
                int result = ordq.value("result").toInt();
                if (result < 0)
                {
                  systemError(this, storedProcErrorLookup("changePoDate", result), __FILE__, __LINE__);
                  return;
                }
              }
              else if (ordq.lastError().type() != QSqlError::NoError)
              {
                systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
                return;
              }
            }
          } // end scheduled date changed
          else if (_supplyOrderDueDate->date() != _supplyOrderDueDateCache)
          { // supply ord due date changed
            ordq.prepare("SELECT changePoitemDueDate(:poitem_id, :dueDate, true) AS result;");
            ordq.bindValue(":poitem_id", _supplyOrderId);
            ordq.bindValue(":dueDate", _supplyOrderDueDate->date());
            ordq.exec();
            if (ordq.first())
            {
              int result = ordq.value("result").toInt();
              if (result < 0)
              {
                systemError(this, storedProcErrorLookup("changePoDate", result), __FILE__, __LINE__);
                return;
              }
            }
            else if (ordq.lastError().type() != QSqlError::NoError)
            {
              systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
              return;
            }
          } // end supply ord due date changed
        } // end PO date change
        else if (_supplyOrderType == "R")
        { // PR date change
          if (_scheduledDate->date() != _supplyOrderScheduledDateCache)
          { // scheduled date changed
            bool applychange = false;
            if (_mode == cNew)
              applychange = true;
            else if (QMessageBox::question(this, tr("Reschedule P/R?"),
                                           tr("<p>The Scheduled Date for this Line Item has changed."
                                              "<p>Should the P/R for this Line Item be rescheduled?"),
                                           QMessageBox::Yes | QMessageBox::Default,
                                           QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
              applychange = true;
            if (applychange)
            {
              ordq.prepare("SELECT changePrDate(:pr_id, :dueDate) AS result;");
              ordq.bindValue(":pr_id", _supplyOrderId);
              ordq.bindValue(":dueDate", _scheduledDate->date());
              ordq.exec();
              if (ordq.first())
              {
                int result = ordq.value("result").toInt();
                if (result < 0)
                {
                  systemError(this, storedProcErrorLookup("changePrDate", result), __FILE__, __LINE__);
                  return;
                }
              }
              else if (ordq.lastError().type() != QSqlError::NoError)
              {
                systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
                return;
              }
            }
          } // end scheduled date changed
          else if (_supplyOrderDueDate->date() != _supplyOrderDueDateCache)
          { // supply ord due date changed
            ordq.prepare("SELECT changePrDate(:pr_id, :dueDate) AS result;");
            ordq.bindValue(":pr_id", _supplyOrderId);
            ordq.bindValue(":dueDate", _supplyOrderDueDate->date());
            ordq.exec();
            if (ordq.first())
            {
              int result = ordq.value("result").toInt();
              if (result < 0)
              {
                systemError(this, storedProcErrorLookup("changePrDate", result), __FILE__, __LINE__);
                return;
              }
            }
            else if (ordq.lastError().type() != QSqlError::NoError)
            {
              systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
              return;
            }
          } // end supply ord due date changed
        } // end PR date change
      } // end Scheduled date change
      
      if ( (_supplyDropShip->isChecked() && !_supplyOrderDropShipCache) ||
           (!_supplyDropShip->isChecked() && _supplyOrderDropShipCache) )
      { // drop ship change
        if (_supplyOrderType == "P")
        { // PO drop ship change
          if (_supplyOrderStatus->text() == "C")
          {
            QMessageBox::critical(this, tr("Cannot Update Supply Order"),
                                  tr("The Purchase Order Item this Sales Order Item is linked to is closed.\n"
                                     "The drop ship may not be updated."));
            _supplyDropShip->setChecked(_supplyOrderDropShipCache);
            return;
          }

           XSqlQuery sto;
		  sto.prepare( "SELECT cohead_shiptoaddress1 FROM cohead WHERE cohead_id=:cohead_id" );
			sto.bindValue(":cohead_id", _soheadid);
			sto.exec();
			if (sto.first()){

          if ( _supplyDropShip->isChecked() && _shiptoid < 1){
		  if(sto.value("cohead_shiptoaddress1").toString().isEmpty())
			  //removes the error if Free-Form Ship-To was used
			{
			 QMessageBox::critical(this, tr("Cannot Update Supply Order"),
					tr("<p>You must enter a valid Ship-To Address (or #) before saving this Sales Order Item."));
			 return;
		  }}}
          
          if (QMessageBox::question(this, tr("Drop Ship P/O?"),
                                    tr("<p>The Drop Ship for this Line Item has changed."
                                       "<p>Should the P/O Drop Ship for this Line Item be changed?"),
                                    QMessageBox::Yes | QMessageBox::Default,
                                    QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
          {
            ordq.prepare("SELECT changePurchaseDropShip(:coitem_id, :poitem_id, :dropship) AS result;");
            ordq.bindValue(":coitem_id", _soitemid);
            ordq.bindValue(":poitem_id", _supplyOrderId);
            ordq.bindValue(":dropship",QVariant(_supplyDropShip->isChecked()));
            ordq.exec();
            if (ordq.first())
            {
              _supplyOrderId = ordq.value("result").toInt();
              if (_supplyOrderId < 0)
              {
                systemError(this, storedProcErrorLookup("changePurchaseDropShip", _supplyOrderId), __FILE__, __LINE__);
                return;
              }
              // save the sales order item again to capture the supply order id
              sSave(true);
            }
            else if (ordq.lastError().type() != QSqlError::NoError)
            {
              systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
              return;
            }
          }
        } // end PO drop ship change
      } // end drop ship changed

      if (_supplyOverridePrice->localValue() != _supplyOverridePriceCache)
      { // override price change
        if (_supplyOrderType == "P")
        { // PO override price change
          if (_supplyOrderStatus->text() == "C")
          {
            QMessageBox::critical(this, tr("Cannot Update Supply Order"),
                                  tr("The Purchase Order Item this Sales Order Item is linked to is closed.\n"
                                     "The override price may not be updated."));
            _supplyOverridePrice->setLocalValue(_supplyOverridePriceCache);
            return;
          }
          
          if (QMessageBox::question(this, tr("Override Price P/O?"),
                                    tr("<p>The Override Price for this Line Item has changed."
                                       "<p>Should the P/O Price for this Line Item be changed?"),
                                    QMessageBox::Yes | QMessageBox::Default,
                                    QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
          {
            ordq.prepare("UPDATE poitem SET poitem_unitprice=:unitprice "
                         "WHERE (poitem_id=:poitem_id);");
            ordq.bindValue(":poitem_id", _supplyOrderId);
            ordq.bindValue(":unitprice", _supplyOverridePrice->localValue());
            ordq.exec();
            if (ordq.lastError().type() != QSqlError::NoError)
            {
              systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
              return;
            }
          }
        } // end PO override price change
      } // end override price changed
    
    }  // end supply order exists
    
    // Populate Supply Order info
    sPopulateOrderInfo();
    
  } // end createSupplyOrder is checked
  else
  { // createSupplyOrder is not checked
    if (_supplyOrderId != -1)
    { // supply order exists
      if (_supplyOrderType == "W")
      {  // work order
        if (QMessageBox::question(this, tr("Delete Work Order"),
                                  tr("<p>You are requesting to delete the Work "
                                       "Order created for this Sales Order Item.\n"
                                       "Are you sure you want to do this?"),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
        {
          ordq.prepare("SELECT deleteWo(:wo_id, TRUE) AS result;");
          ordq.bindValue(":wo_id", _supplyOrderId);
          ordq.exec();
          if (ordq.first())
          {
            int result = ordq.value("result").toInt();
            if (result < 0)
            {
              systemError(this, storedProcErrorLookup("deleteWo", result),
                          __FILE__, __LINE__);
              _createSupplyOrder->setChecked(true);
              return;
            }
            else
            {
              if ((cNew == _mode) || (cNewQuote == _mode))
              {
                _item->setReadOnly(FALSE);
                _warehouse->setEnabled(TRUE);
              }
              _supplyOrderId = -1;
              _itemcharView->setEnabled(TRUE);
            }
          }
          else if (ordq.lastError().type() != QSqlError::NoError)
          {
              systemError(this, ordq.lastError().databaseText(),
                        __FILE__, __LINE__);
            _createSupplyOrder->setChecked(true);
            return;
          }
          omfgThis->sWorkOrdersUpdated(-1, TRUE);
        }
        else
          _createSupplyOrder->setChecked(true);
      }  // end work order
      else if (_supplyOrderType == "P")
      {  // purchase order
        if (QMessageBox::question(this, tr("Delete Purchase Order Item"),
                                  tr("<p>You are requesting to delete the "
                                     "Purchase Order Item created for this Sales "
                                     "Order Item. The associated Purchase Order "
                                     "will also be deleted if no other Purchase "
                                     "Order Item exists for that Purchase Order.\n"
                                     "Are you sure you want to do this?"),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
        {
          ordq.prepare("SELECT deletepoitem(:poitemid) AS result;");
          ordq.bindValue(":poitemid", _supplyOrderId);
          ordq.exec();
          if (ordq.first())
          {
            int result = ordq.value("result").toInt();
            if (result < 0)
            {
              systemError(this, storedProcErrorLookup("deletePoitem", result),
                          __FILE__, __LINE__);
              _createSupplyOrder->setChecked(true);
              return;
            }
            else
            {
              if ((cNew == _mode) || (cNewQuote == _mode))
              {
                _item->setReadOnly(FALSE);
                _warehouse->setEnabled(TRUE);
              }
              _supplyOrderId = -1;
            }
          }
          else if (ordq.lastError().type() != QSqlError::NoError)
          {
            systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
            _createSupplyOrder->setChecked(true);
            return;
          }
        }
        else
          _createSupplyOrder->setChecked(true);
      }  // end purchase order
      else if (_supplyOrderType == "R")
      {  // purchase request
        if (QMessageBox::question(this, tr("Delete Purchase Request"),
                                  tr("<p>You are requesting to delete the "
                                     "Purchase Request created for this Sales "
                                     "Order Item.\n"
                                     "Are you sure you want to do this?"),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
        {
          ordq.prepare("SELECT deletePr(:pr_id) AS result;");
          ordq.bindValue(":pr_id", _supplyOrderId);
          ordq.exec();
          if (ordq.first())
          {
            bool result = ordq.value("result").toBool();
            if (!result)
            {
              systemError(this, tr("deletePr failed"), __FILE__, __LINE__);
              _createSupplyOrder->setChecked(true);
              return;
            }
            else
            {
              if ((cNew == _mode) || (cNewQuote == _mode))
              {
                _item->setReadOnly(FALSE);
                _warehouse->setEnabled(TRUE);
              }
              _supplyOrderId = -1;
            }
          }
          else if (ordq.lastError().type() != QSqlError::NoError)
          {
            systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
        }
        else
          _createSupplyOrder->setChecked(true);
      }  // end purchase request
    }  // end supply order exists

    if (_supplyOrderId == -1)
    {
      _supplyOrder->clear();
      _supplyOrderLine->clear();
      _supplyOrderStatus->clear();
      _supplyOrderQtyCache = 0.0;
      _supplyOrderQtyOrderedCache = 0.0;
      _supplyOrderQtyOrderedInvCache = 0.0;
      _supplyOrderQty->clear();
      _supplyOrderDueDateCache = QDate();
      _supplyOrderScheduledDateCache = _scheduledDate->date();
      _supplyOrderDropShipCache = false;
      _supplyOrderDueDate->clear();
      _supplyDropShip->setChecked(false);
      _supplyRollupPrices->setChecked(false);
      _supplyOverridePrice->clear();
      _supplyOverridePriceCache = 0.0;
      _woIndentedList->clear();
    }
  }  // end createSupplyOrder is not checked
}

void salesOrderItem::sPopulateOrderInfo()
{
  if (ISQUOTE(_mode))
    return;
  
  if (!_createSupplyOrder->isChecked())
  {
    QMessageBox::critical(this, tr("Debug"),
                          tr("sPopulateOrderInfo called with _createSupplyOrder unchecked."));
    return;
  }

  if (_supplyOrderId == -1)
  {
    QMessageBox::critical(this, tr("Debug"),
                          tr("sPopulateOrderInfo called with _supplyOrderId = -1."));
    return;
  }
  
//  QMessageBox::critical(this, tr("Debug"),
//                        tr("sPopulateOrderInfo called with _supplyOrderType = %1").arg(_supplyOrderType));
//  QMessageBox::critical(this, tr("Debug"),
//                        tr("sPopulateOrderInfo called with _supplyOrderId = %1").arg(_supplyOrderId));

  XSqlQuery ordq;
  
  // Populate Supply Order
  if (_supplyOrderType == "W")
  {
    ordq.prepare( "SELECT wo.*,"
                 "       CASE WHEN wo_status IN ('R', 'I', 'C') THEN true"
                 "            ELSE false END AS orderlocked,"
                 "       warehous_id, warehous_code "
                 "FROM wo, itemsite, whsinfo "
                 "WHERE ((wo_itemsite_id=itemsite_id)"
                 " AND (itemsite_warehous_id=warehous_id)"
                 " AND (wo_id=:wo_id));" );
    ordq.bindValue(":wo_id", _supplyOrderId);
    ordq.exec();
    if (ordq.first())
    {
      _createSupplyOrder->setTitle(tr("Create Work Order"));
      
      _supplyOrder->setText(ordq.value("wo_number").toString());
      _supplyOrderQty->setDouble(ordq.value("wo_qtyord").toDouble());
      _supplyOrderDueDate->setDate(ordq.value("wo_duedate").toDate());
      _supplyOrderStatus->setText(ordq.value("wo_status").toString());
      
      if (ordq.value("orderlocked").toBool())
        _createSupplyOrder->setEnabled(FALSE);
      
      if (_item->isConfigured() && (ordq.value("wo_status").toString() != "O"))
        _itemcharView->setEnabled(FALSE);
      
      _supplyWarehouse->clear();
      _supplyWarehouse->append(ordq.value("warehous_id").toInt(), ordq.value("warehous_code").toString());
      _supplyWarehouse->setEnabled(FALSE);

      _supplyOrderLit->show();
      _supplyOrderLineLit->hide();
      _supplyOrderStatusLit->show();
      _supplyOrderQtyLit->show();
      _supplyOrderDueDateLit->show();
      _supplyOrder->show();
      _supplyOrderLine->hide();
      _supplyOrderStatus->show();
      _supplyOrderQty->show();
      _supplyOrderDueDate->show();
      if (!_item->isConfigured() && (ordq.value("wo_status").toString() == "E"))
        _supplyRollupPrices->setEnabled(true);
      else
        _supplyRollupPrices->setEnabled(false);
      if (_metrics->boolean("MultiWhs"))
      {
        _supplyWarehouseLit->show();
        _supplyWarehouse->show();
      }
      else
      {
        _supplyWarehouseLit->hide();
        _supplyWarehouse->hide();
      }

      _supplyOrderStack->setCurrentWidget(_workOrderPage);
      sFillWoIndentedList();
    }
    else
    {
      _supplyOrderId = -1;
      _createSupplyOrder->setChecked(FALSE);
    }
  }
  else if (_supplyOrderType == "P")
  {
    ordq.prepare("SELECT pohead_number, pohead_dropship, poitem.*"
                 "  FROM poitem JOIN pohead ON (pohead_id = poitem_pohead_id)"
                 " WHERE (poitem_id = :poitem_id);");
    ordq.bindValue(":poitem_id", _supplyOrderId);
    ordq.exec();
    if (ordq.first())
    {
      _createSupplyOrder->setTitle(tr("Create Purchase Order"));
      
      _supplyOrder->setText(ordq.value("pohead_number").toString());
      _supplyOrderLine->setText(ordq.value("poitem_linenumber").toString());
      _supplyOrderStatus->setText(ordq.value("poitem_status").toString());
      _supplyOrderQty->setDouble(ordq.value("poitem_qty_ordered").toDouble());
      _supplyOrderDueDate->setDate(ordq.value("poitem_duedate").toDate());
      _supplyDropShip->setChecked(ordq.value("pohead_dropship").toBool());
      _supplyOverridePrice->setLocalValue(ordq.value("poitem_unitprice").toDouble());
      _itemsrc = ordq.value("poitem_itemsrc_id").toInt();
      
      _supplyOrderLit->show();
      _supplyOrderLineLit->show();
      _supplyOrderStatusLit->show();
      _supplyOrderQtyLit->show();
      _supplyOrderDueDateLit->show();
      _supplyWarehouseLit->hide();
      _supplyOrder->show();
      _supplyOrderLine->show();
      _supplyOrderStatus->show();
      _supplyOrderQty->show();
      _supplyOrderDueDate->show();
      _supplyWarehouse->hide();
      _supplyDropShip->setVisible(_metrics->boolean("EnableDropShipments"));
      
      _supplyOrderStack->setCurrentWidget(_purchaseOrderPage);
    }
    else
    {
      // report an error if one occurred, but always reset supply order info
      ErrorReporter::error(QtCriticalMsg, this, tr("Getting P/O Items"),
                                  ordq, __FILE__, __LINE__);
      _supplyOrderId = -1;
      _createSupplyOrder->setChecked(FALSE);
    }
  }
  else if (_supplyOrderType == "R")
  {
    ordq.prepare( "SELECT * "
                 "FROM pr "
                 "WHERE (pr_id=:pr_id);" );
    ordq.bindValue(":pr_id", _supplyOrderId);
    ordq.exec();
    if (ordq.first())
    {
      _createSupplyOrder->setTitle(tr("Create Purchase Request"));
      
      _supplyOrder->setText(ordq.value("pr_number").toString());
      _supplyOrderStatus->setText(ordq.value("pr_status").toString());
      _supplyOrderQty->setDouble(ordq.value("pr_qtyreq").toDouble());
      _supplyOrderDueDate->setDate(ordq.value("pr_duedate").toDate());
      _supplyOrderStatus->setText(ordq.value("pr_status").toString());
      
      if ((ordq.value("pr_status").toString() == "R") || (ordq.value("pr_status").toString() == "C"))
        _createSupplyOrder->setEnabled(FALSE);

      _supplyOrderLit->show();
      _supplyOrderLineLit->hide();
      _supplyOrderStatusLit->show();
      _supplyOrderQtyLit->show();
      _supplyOrderDueDateLit->show();
      _supplyWarehouseLit->hide();
      _supplyOrder->show();
      _supplyOrderLine->hide();
      _supplyOrderStatus->show();
      _supplyOrderQty->show();
      _supplyOrderDueDate->show();
      _supplyWarehouse->hide();
      
//      _supplyOrderStack->setCurrentWidget(_purchaseRequestPage);
      _supplyOrderStack->setCurrentWidget(_purchaseOrderPage);
    }
    else
    {
      _supplyOrderId = -1;
      _createSupplyOrder->setChecked(FALSE);
    }
  }
  
  if (_costmethod == "J")
    _createSupplyOrder->setEnabled(false);
  
  _supplyOrderQtyCache = _supplyOrderQty->toDouble();
  _supplyOrderDueDateCache = _supplyOrderDueDate->date();
  _supplyOrderDropShipCache = _supplyDropShip->isChecked();
  _supplyOverridePriceCache = _supplyOverridePrice->localValue();
  _supplyOrderQtyOrderedCache = _qtyOrdered->toDouble();
  _supplyOrderQtyOrderedInvCache = _qtyOrdered->toDouble() * _qtyinvuomratio;
  _supplyOrderScheduledDateCache = _scheduledDate->date();

  if (_mode == cNew || _mode == cEdit)
  {
    _supplyWoNewMatl->setEnabled(true);
    connect(_woIndentedList,    SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateWoMenu(QMenu*, QTreeWidgetItem*)));
    connect(_woIndentedList,    SIGNAL(itemSelected(int)),            _supplyWoEdit, SLOT(animateClick()));
    connect(_woIndentedList,    SIGNAL(valid(bool)),                  _supplyWoEdit, SLOT(setEnabled(bool)));
    connect(_woIndentedList,    SIGNAL(valid(bool)),                  _supplyWoDelete, SLOT(setEnabled(bool)));
    connect(_supplyWoNewMatl,   SIGNAL(clicked()),                    this, SLOT(sNewWoMatl()));
    connect(_supplyWoEdit,      SIGNAL(clicked()),                    this, SLOT(sEditWoMatl()));
    connect(_supplyWoDelete,    SIGNAL(clicked()),                    this, SLOT(sDeleteWoMatl()));
    connect(_supplyRollupPrices,SIGNAL(toggled(bool)),                this, SLOT(sRollupPrices()));
    connect(_supplyOrderQty,    SIGNAL(editingFinished()),            this, SLOT(sHandleSupplyOrder()));
    //  connect(_supplyOrderDueDate,SIGNAL(newDate(const QDate &)),       this, SLOT(sHandleSupplyOrder()));
    connect(_supplyOverridePrice,SIGNAL(editingFinished()),           this, SLOT(sHandleSupplyOrder()));
    connect(_supplyDropShip,    SIGNAL(toggled(bool)),                this, SLOT(sHandleSupplyOrder()));
  }
  else
  {
    _supplyWoNewMatl->setEnabled(false);
  }
}

void salesOrderItem::sRollupPrices()
{
  if (ISQUOTE(_mode))
    return;
  
  if (_supplyOrderId != -1 && _supplyOrderType == "W")
  {
    if (_supplyRollupPrices->isChecked())
    {
      XSqlQuery ordq;
      if (_metrics->boolean("Routings"))
        ordq.prepare("SELECT SUM(COALESCE(price, 0.0)) AS price "
                     "FROM ( "
                     "SELECT (womatl_price * womatl_qtyreq / wo_qtyord) AS price "
                     "FROM womatl JOIN wo ON (wo_id=womatl_wo_id) "
                     "WHERE (womatl_wo_id = :wo_id) "
                     "UNION "
                     "SELECT wooper_price AS price "
                     "FROM xtmfg.wooper JOIN wo ON (wo_id=wooper_wo_id) "
                     "WHERE (wooper_wo_id = :wo_id) "
                     " ) AS data;");
      else
        ordq.prepare("SELECT SUM(COALESCE(price, 0.0)) AS price "
                     "FROM ( "
                     "SELECT (womatl_price * womatl_qtyreq / wo_qtyord) AS price "
                     "FROM womatl JOIN wo ON (wo_id=womatl_wo_id) "
                     "WHERE (womatl_wo_id = :wo_id) "
                     " ) AS data;");
      ordq.bindValue(":wo_id", _supplyOrderId);
      ordq.exec();
      if (ordq.first())
      {
        _netUnitPrice->setLocalValue(ordq.value("price").toDouble());
      }
      else if (ordq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, ordq.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    else
      sDeterminePrice(true);
  }
}

void salesOrderItem::sFillWoIndentedList()
{
  XSqlQuery workFillList;
  _woIndentedList->clear();
  //The wodata_id_type column is used to indicate the source of the wodata_id
  //there are three different tables used wo, womatl and womatlvar
  //wodata_id_type = 1 = wo_id
  //wodata_id_type = 2 = womatl_id
  //wodata_id_type = 3 = womatlvar_id
  QString sql(
              "     SELECT wodata_id, "
              "           wodata_id_type, "
              "           CASE WHEN wodata_id_type = 1 THEN "
              "                  wodata_number || '-' || wodata_subnumber "
              "                WHEN wodata_id_type = 3 THEN "
              "                  wodata_subnumber::text "
              "           END AS wonumber, "
              "           wodata_itemnumber, "
              "           wodata_descrip, "
              "           wodata_status, "
              "           wodata_startdate, "
              "           wodata_duedate, "
              "           wodata_adhoc,    "
              "           wodata_itemsite_id, "
              "           wodata_qoh AS qoh, "
              "           wodata_short AS short, "
              "           wodata_qtyper AS qtyper, "
              "           wodata_qtyiss AS qtyiss,    "
              "           wodata_qtyrcv AS qtyrcv,  "
              "           wodata_qtyordreq AS qtyordreq, "
              "           wodata_qtyuom, "
              "           wodata_scrap AS scrap, "
              "           wodata_setup, "
              "           wodata_run, "
              "           wodata_notes, "
              "           wodata_ref, "
              "           wodata_custprice, wodata_listprice, "
              "           CASE WHEN (wodata_status = 'C') THEN 'gray' "
              "                WHEN (wodata_qoh = 0) THEN 'warning' "
              "                WHEN (wodata_qoh < 0) THEN 'error' "
              "           END AS qoh_qtforegroundrole, "
              "           CASE WHEN (wodata_status = 'C') THEN 'gray' "
              "                WHEN (wodata_qtyiss = 0) THEN 'warning' "
              "           END AS qtyiss_qtforegroundrole, "
              "           CASE WHEN (wodata_status = 'C') THEN 'gray' "
              "                WHEN (wodata_short > 0) THEN 'error' "
              "           END AS short_qtforegroundrole, "
              "           CASE WHEN (wodata_status = 'C') THEN 'gray' "
              "                WHEN (wodata_startdate <= current_date) THEN 'error' "
              "           END AS wodata_startdate_qtforegroundrole,   "
              "           CASE WHEN (wodata_status = 'C') THEN 'gray' "
              "                WHEN (wodata_duedate <= current_date) THEN 'error' "
              "           END AS wodata_duedate_qtforegroundrole,   "
              "           CASE WHEN (wodata_status = 'C') THEN 'gray' "
              "                WHEN (wodata_id_type = 3) THEN 'emphasis' "
              "                WHEN (wodata_id_type = 1) THEN 'altemphasis' "
              "           ELSE null END AS qtforegroundrole, "
              "           'qty' AS qoh_xtnumericrole, "
              "           'qtyper' AS qty_per_xtnumericrole, "
              "           'qty' AS qtyiss_xtnumericrole, "
              "           'qty' AS qtyrcv_xtnumericrole, "
              "           'qty' AS qtyordreq_xtnumericrole, "
              "           'qty' AS short_xtnumericrole, "
              "           'qty' AS setup_xtnumericrole,"
              "           'qty' AS run_xtnumericrole,"
              "           'scrap' AS scrap_xtnumericrole, "
              "           wodata_level AS xtindentrole "
              "    FROM indentedwo(:wo_id, :showops, :showmatl, :showindent) ");
  workFillList.prepare(sql);
  workFillList.bindValue(":wo_id", _supplyOrderId);
  if (_metrics->boolean("Routings"))
    workFillList.bindValue(":showops", true);
  else
    workFillList.bindValue(":showops", false);
  workFillList.bindValue(":showmatl", true);
  workFillList.bindValue(":showindent", false);
  workFillList.exec();
  if (workFillList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, workFillList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _woIndentedList->populate(workFillList, true);
  _woIndentedList->expandAll();
}

void salesOrderItem::sPopulateWoMenu(QMenu *pMenu,  QTreeWidgetItem *selected)
{
  QString  status(selected->text(3));
  QAction *menuItem;
  
  //Check if row is a work order and id is vaild
  if(_woIndentedList->altId() == 1 && _woIndentedList->id() > -1)
  {
    if (_mode != cView)
    {
      if (status == "O" ||status == "E" || status == "R" || status == "I")
      {
        menuItem = pMenu->addAction(tr("New Material..."), this, SLOT(sNewWoMatl()));
        if (!_privileges->check("MaintainWoMaterials"))
          menuItem->setEnabled(false);
      }
    }
  }
  
  //Check a womatl row is selected and the id is vaild
  if(_woIndentedList->altId() == 2 && _woIndentedList->id() > -1)
  {
    if (_mode != cView)
    {
      if (status == "O" || status == "E" || status == "R" || status == "I")
      {
        menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEditWoMatl()));
        if (!_privileges->check("MaintainWoMaterials"))
          menuItem->setEnabled(false);
      }
    }
    
    menuItem = pMenu->addAction(tr("View..."), this, SLOT(sViewWoMatl()));
    
    if (_mode != cView)
    {
      if (status == "O" || status == "E")
      {
        menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDeleteWoMatl()));
        if (!_privileges->check("MaintainWoMaterials"))
          menuItem->setEnabled(false);
      }
    }
  }
}

void salesOrderItem::sNewWoMatl()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("showPrice", true);
  params.append("wo_id", _supplyOrderId);
  
  woMaterialItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
//  int currentId = _woIndentedList->id();
//  int currentAltId = _woIndentedList->altId();
  omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), TRUE);
  sFillWoIndentedList();
//  _woIndentedList->setId(currentId,currentAltId);
  if (_supplyRollupPrices->isChecked())
    sRollupPrices();
}

void salesOrderItem::sEditWoMatl()
{
  if(_woIndentedList->altId() == 2 && _woIndentedList->id() > -1)
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("showPrice", true);
    params.append("womatl_id", _woIndentedList->id());
    
    woMaterialItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
//    int currentId = _woIndentedList->id();
//    int currentAltId = _woIndentedList->altId();
    omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), TRUE);
    sFillWoIndentedList();
//    _woIndentedList->setId(currentId,currentAltId);
    if (_supplyRollupPrices->isChecked())
      sRollupPrices();
  }
}

void salesOrderItem::sViewWoMatl()
{
  if(_woIndentedList->altId() == 2 && _woIndentedList->id() > -1)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("showPrice", true);
    params.append("womatl_id", _woIndentedList->id());
    woMaterialItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void salesOrderItem::sDeleteWoMatl()
{
  if(_woIndentedList->altId() == 2 && _woIndentedList->id() > -1)
  {
    XSqlQuery workDeleteMatl;
    int womatlid = _woIndentedList->id();
    if (_woIndentedList->currentItem()->data(7, Qt::UserRole).toMap().value("raw").toDouble() > 0)
    {
      QMessageBox::critical( this, tr("W/O Material Requirement cannot be Deleted"),
                            tr("<p>This W/O Material Requirement cannot be "
                               "deleted as it has material issued to it. "
                               "You must return this material to stock before "
                               "you can delete this Material Requirement." ) );
      return;
    }
    
    workDeleteMatl.prepare("SELECT deleteWoMaterial(:womatl_id);");
    workDeleteMatl.bindValue(":womatl_id", womatlid);
    workDeleteMatl.exec();
    if (workDeleteMatl.first())
    {
      int result = workDeleteMatl.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("deleteWoMaterial", result), __FILE__, __LINE__);
        return;
      }
    }
    else if (workDeleteMatl.lastError().type() != QSqlError::NoError)
    {
      systemError(this, workDeleteMatl.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    workDeleteMatl.prepare("SELECT womatl_wo_id AS woid "
                           "FROM womatl "
                           "WHERE (womatl_id=:womatl_id) ");
    workDeleteMatl.bindValue(":womatl_id", womatlid);
    workDeleteMatl.exec();
    if (workDeleteMatl.first())
      omfgThis->sWorkOrderMaterialsUpdated(workDeleteMatl.value("woid").toInt(), womatlid, TRUE);
    
    omfgThis->sWorkOrdersUpdated(_woIndentedList->id(), TRUE);
    sFillWoIndentedList();
    if (_supplyRollupPrices->isChecked())
      sRollupPrices();
  }
}

void salesOrderItem::sCalculateFromDiscount()
{
  if (_customerPrice->isZero())
    _discountFromCust->setText(tr("N/A"));
  else
  {
    if (_updatePrice)
    {
//      if (_priceMode == "M")  // markup
//        _netUnitPrice->setLocalValue(_customerPrice->localValue() +
//                                    (_customerPrice->localValue() * _discountFromCust->toDouble() / 100.0));
//      else  // discount
        _netUnitPrice->setLocalValue(_customerPrice->localValue() -
                                    (_customerPrice->localValue() * _discountFromCust->toDouble() / 100.0));
    }
    sCalculateDiscountPrcnt();
  }
}

void salesOrderItem::sCalculateFromMarkup()
{
    if (_unitCost->isZero())
      _markupFromUnitCost->setText(tr("N/A"));
    else
    {
        if (_updatePrice)
        {
            if (_metrics->boolean("Long30Markups"))
                _netUnitPrice->setLocalValue(_unitCost->localValue() /
                                             (1.0 - (_markupFromUnitCost->toDouble() / 100.0)));
            else
                _netUnitPrice->setLocalValue(_unitCost->localValue() +
                                             (_unitCost->localValue() * _markupFromUnitCost->toDouble() / 100.0));
        }
        sCalculateDiscountPrcnt();
    }
}

void salesOrderItem::populate()
{
  if (_mode == cNew || _mode == cNewQuote)
    return;

  XSqlQuery item;
  QString sql;
  sql = "<? if exists('isSalesOrder') ?>"
          "SELECT itemsite_leadtime, warehous_id, warehous_code, "
          "       item_id, uom_name, iteminvpricerat(item_id) AS invpricerat, item_listprice,"
          "       item_inv_uom_id, item_fractional,"
          "       coitem_status, coitem_cohead_id,"
          "       COALESCE(coitem_order_type, '') AS coitem_order_type,"
          "       COALESCE(coitem_order_id, -1) AS coitem_order_id,"
          "       coitem_custpn, coitem_memo, NULL AS quitem_createorder,"
          "       NULL AS quitem_order_warehous_id,"
          "       formatSoLineNumber(coitem_id) AS linenumber,"
          "       coitem_qtyord AS qtyord,"
          "       coitem_qty_uom_id AS qty_uom_id,"
          "       coitem_qty_invuomratio AS qty_invuomratio,"
          "       coitem_qtyshipped AS qtyshipped,"
          "       coitem_scheddate,"
          "       coitem_custprice, coitem_unitcost,"
          "       coitem_price, coitem_pricemode,"
          "       coitem_price_uom_id AS price_uom_id,"
          "       coitem_price_invuomratio AS price_invuomratio,"
          "       coitem_promdate AS promdate,"
          "       coitem_substitute_item_id, coitem_prcost,"
          "       qtyAtShipping(coitem_id) AS qtyatshipping,"
          "       coitem_taxtype_id,"
          "       coitem_cos_accnt_id, coitem_rev_accnt_id, "
          "       coitem_warranty, coitem_qtyreserved, locale_qty_scale, "
          "       cohead_number AS ordnumber "
          "FROM coitem, whsinfo, itemsite, item, uom, cohead, locale "
          "LEFT OUTER JOIN usr ON (usr_username = getEffectiveXtUser()) "
          "WHERE ( (coitem_itemsite_id=itemsite_id)"
          " AND (itemsite_warehous_id=warehous_id)"
          " AND (itemsite_item_id=item_id)"
          " AND (item_inv_uom_id=uom_id)"
          " AND (cohead_id=coitem_cohead_id)"
          " AND (coitem_id=<? value('id') ?>) "
          " AND (locale_id = usr_locale_id));"
          "<? else ?>"
            "SELECT itemsite_leadtime, COALESCE(warehous_id, -1) AS warehous_id, "
            "       warehous_code,"
            "       item_id, uom_name, iteminvpricerat(item_id) AS invpricerat, item_listprice,"
            "       item_inv_uom_id, item_fractional,"
            "       'O' AS coitem_status, quitem_quhead_id AS coitem_cohead_id,"
            "       '' AS coitem_order_type, -1 AS coitem_order_id,"
            "       quitem_custpn AS coitem_custpn,"
            "       quitem_memo AS coitem_memo, quitem_createorder,"
            "       quitem_order_warehous_id,"
            "       quitem_linenumber AS linenumber,"
            "       quitem_qtyord AS qtyord,"
            "       quitem_qty_uom_id AS qty_uom_id,"
            "       quitem_qty_invuomratio AS qty_invuomratio,"
            "       NULL AS qtyshipped,"
            "       quitem_scheddate AS coitem_scheddate,"
            "       quitem_custprice AS coitem_custprice, quitem_unitcost AS coitem_unitcost,"
            "       quitem_price AS coitem_price, quitem_pricemode AS coitem_pricemode,"
            "       quitem_price_uom_id AS price_uom_id,"
            "       quitem_price_invuomratio AS price_invuomratio,"
            "       quitem_promdate AS promdate,"
            "       -1 AS coitem_substitute_item_id, quitem_prcost AS coitem_prcost,"
            "       0.0 AS qtyatshipping,"
            "       quitem_taxtype_id AS coitem_taxtype_id, quitem_dropship, quitem_itemsrc_id"
            "       locale_qty_scale, quhead_number AS ordnumber "
            "  FROM item, uom, quhead, locale "
            "    LEFT OUTER JOIN usr ON (usr_username = getEffectiveXtUser()), quitem "
            "    LEFT OUTER JOIN (itemsite "
            "               JOIN whsinfo ON (itemsite_warehous_id=warehous_id)) "
            "                             ON (quitem_itemsite_id=itemsite_id) "
            " WHERE ( (quitem_item_id=item_id)"
            "   AND   (item_inv_uom_id=uom_id)"
            "   AND   (quhead_id=quitem_quhead_id)"
            "   AND   (quitem_id=<? value('id') ?>) "
            "   AND   (locale_id = usr_locale_id));"
            "<? endif ?>";

  ParameterList qparams;
  qparams.append("id", _soitemid);
  if (!ISQUOTE(_mode))
    qparams.append("isSalesOrder");
  MetaSQLQuery metaitem(sql);
  item = metaitem.toQuery(qparams);
  if (item.first())
  {
    _soheadid = item.value("coitem_cohead_id").toInt();
    _supplyOrderType = item.value("coitem_order_type").toString();
    _supplyOrderId = item.value("coitem_order_id").toInt();
    _comments->setId(_soitemid);
    _lineNumber->setText(item.value("linenumber").toString());
    _priceRatio = item.value("invpricerat").toDouble();
    _shippedToDate->setDouble(item.value("qtyshipped").toDouble());

    _item->setId(item.value("item_id").toInt());  // should precede _taxtype/code
    _invuomid = item.value("item_inv_uom_id").toInt();
    _qtyUOM->setId(item.value("qty_uom_id").toInt());
    _priceUOM->setId(item.value("price_uom_id").toInt());
    _priceUOMCache = _priceUOM->id();
    _qtyinvuomratio    = item.value("qty_invuomratio").toDouble();
    _priceinvuomratio  = item.value("price_invuomratio").toDouble();
    _unitCost->setLocalValue(item.value("coitem_unitcost").toDouble());
    // do tax stuff before _qtyOrdered so signal cascade has data to work with
    _taxtype->setId(item.value("coitem_taxtype_id").toInt());
    _orderNumber->setText(item.value("ordnumber").toString());
    _qtyOrderedCache = item.value("qtyord").toDouble();
    _qtyOrdered->setDouble(_qtyOrderedCache);
    _supplyOrderQtyOrderedCache = _qtyOrderedCache;
    _supplyOrderQtyOrderedInvCache = _qtyOrderedCache * _qtyinvuomratio;
    _scheduledDateCache = item.value("coitem_scheddate").toDate();
    _supplyOrderScheduledDateCache = _scheduledDateCache;
    _scheduledDate->setDate(_scheduledDateCache);
    _notes->setText(item.value("coitem_memo").toString());
    if (!item.value("quitem_createorder").isNull())
      _createSupplyOrder->setChecked(item.value("quitem_createorder").toBool());
    if (!item.value("promdate").isNull() && _metrics->boolean("UsePromiseDate"))
      _promisedDate->setDate(item.value("promdate").toDate());
    if (item.value("coitem_substitute_item_id").toInt() > 0)
    {
      _sub->setChecked(true);
      _subItem->setId(item.value("coitem_substitute_item_id").toInt());
    }
    _customerPrice->setLocalValue(item.value("coitem_custprice").toDouble());
    _listPrice->setBaseValue(item.value("item_listprice").toDouble() * (_priceinvuomratio / _priceRatio));
    _netUnitPrice->setLocalValue(item.value("coitem_price").toDouble());
    _priceMode = item.value("coitem_pricemode").toString();
    _margin->setLocalValue(((_netUnitPrice->localValue() - _unitCost->localValue()) / _priceinvuomratio) * (_qtyOrdered->toDouble() * _qtyinvuomratio));
    _leadTime        = item.value("itemsite_leadtime").toInt();
    _originalQtyOrd  = _qtyOrdered->toDouble() * _qtyinvuomratio;
    if (!item.value("quitem_order_warehous_id").isNull())
      _supplyWarehouse->setId(item.value("quitem_order_warehous_id").toInt());
    if (item.value("qtyshipped").toDouble() > 0)
    {
      _qtyUOM->setEnabled(false);
      _priceUOM->setEnabled(false);
    }

    _customerPN->setText(item.value("coitem_custpn").toString());

    if (ISQUOTE(_mode))
    {
      if (!item.value("quitem_dropship").isNull())
        _supplyDropShip->setChecked(item.value("quitem_dropship").toBool());
      if (!item.value("quitem_itemsrc_id").isNull())
        _itemsrc = item.value("quitem_itemsrc_id").toInt();
      else
        _itemsrc = -1;
      _supplyOverridePrice->setLocalValue(item.value("coitem_prcost").toDouble());
    }
    else
    {
      _warranty->setChecked(item.value("coitem_warranty").toBool());
      _altCosAccnt->setId(item.value("coitem_cos_accnt_id").toInt());
      _altRevAccnt->setId(item.value("coitem_rev_accnt_id").toInt());
      _qtyreserved = item.value("coitem_qtyreserved").toDouble();
    }

    sCalculateDiscountPrcnt();
    sLookupTax();
    sDetermineAvailability();
  }
  else if (item.lastError().type() != QSqlError::NoError)
  {
    systemError(this, item.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (item.value("coitem_order_type").toString() != "")
    _supplyOrderType = item.value("coitem_order_type").toString();
  if (item.value("coitem_order_id").toInt() != -1)
    _supplyOrderId = item.value("coitem_order_id").toInt();
  if (_supplyOrderId != -1)
    _createSupplyOrder->setChecked(true);

  // _warehouse is populated with active records. append if this one is inactive
  if (ISORDER(_mode))
  {
    _warehouse->append(item.value("warehous_id").toInt(),
                       item.value("warehous_code").toString());
    _warehouse->setId(item.value("warehous_id").toInt());
    _warehouse->setEnabled(FALSE);

    if ( (cView != _mode) && (item.value("coitem_status").toString() == "O") )
      _cancel->setEnabled((item.value("qtyshipped").toDouble()==0.0) && (item.value("qtyatshipping").toDouble()==0.0));
    else
      _cancel->setEnabled(false);
  }
  else  // if (ISQUOTE(_mode))
  {
    if (cEditQuote == _mode && item.value("warehous_id").toInt() == -1)
    {
      _updateItemsite = true;
      _warehouse->append(-1, item.value("warehous_code").toString());
      _warehouse->setId(-1);
    }
    else
    {
      _warehouse->append(item.value("warehous_id").toInt(),
                         item.value("warehous_code").toString());
      _warehouse->setId(item.value("warehous_id").toInt());
      _warehouse->setEnabled(FALSE);
    }
  }
}

void salesOrderItem::sFindSellingWarehouseItemsites( int id )
{
  _warehouse->findItemsites(id);
  _supplyWarehouse->findItemsites(id);
  if (_preferredWarehouseid > 0)
  {
    _warehouse->setId(_preferredWarehouseid);
    _supplyWarehouse->setId(_preferredWarehouseid);
  }
}

void salesOrderItem::sPriceGroup()
{
  if (!omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_netUnitPrice->currAbbr()));
}

void salesOrderItem::sNext()
{
  XSqlQuery salesNext;
  if (_modified)
  {
    switch ( QMessageBox::question( this, tr("Unsaved Changed"),
                                    tr("<p>You have made some changes which have not yet been saved!\n"
                                         "Would you like to save them now?"),
                                    QMessageBox::Yes | QMessageBox::Default,
                                    QMessageBox::No,
                                    QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave(false);
        if (_modified)  // catch an error saving
          return;

      case QMessageBox::No:
        break;

      case QMessageBox::Cancel:
      default:
        return;
    }
  }

  clear();
  prepare();
  _item->setFocus();

  if (cNew == _mode || cNewQuote == _mode)
  {
    _modified = false;
    return;
  }

  if (ISQUOTE(_mode))
    salesNext.prepare("SELECT a.quitem_id AS id, 0 AS sub"
              "  FROM quitem AS a, quitem as b"
              " WHERE ((a.quitem_quhead_id=b.quitem_quhead_id)"
              "   AND  (a.quitem_linenumber > b.quitem_linenumber)"
              "   AND  (b.quitem_id=:id))"
              " ORDER BY a.quitem_linenumber"
              " LIMIT 1;");
  else
    salesNext.prepare("SELECT a.coitem_id AS id, a.coitem_subnumber AS sub"
              "  FROM coitem AS a, coitem AS b"
              " WHERE ((a.coitem_cohead_id=b.coitem_cohead_id)"
	      "   AND  (a.coitem_status <> 'X')"
              "   AND ((a.coitem_linenumber > b.coitem_linenumber)"
              "    OR ((a.coitem_linenumber = b.coitem_linenumber)"
              "   AND  (a.coitem_subnumber > b.coitem_subnumber)))"
              "   AND  (b.coitem_id=:id))"
              " ORDER BY a.coitem_linenumber, a.coitem_subnumber"
              " LIMIT 1;");
  salesNext.bindValue(":id", _soitemid);
  salesNext.exec();
  if (salesNext.first())
  {
    ParameterList params;
    if (_custid != -1)
      params.append("cust_id", _custid);
    params.append("soitem_id", salesNext.value("id").toInt());
    if (ISQUOTE(_mode))
    {
      if (cNewQuote == _mode || cEditQuote == _mode)
        params.append("mode", "editQuote");
      else
        params.append("mode", "viewQuote");
    }
    else
    {
      if ((cNew == _initialMode || cEdit == _initialMode) && (salesNext.value("sub").toInt() == 0))
        params.append("mode", "edit");
      else
        params.append("mode", "view");
    }
    set(params);
  }
  else if (salesNext.lastError().type() != QSqlError::NoError)
  {
    systemError(this, salesNext.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else if (cView != _initialMode && cViewQuote != _initialMode)
  {
    ParameterList params;
    if (_custid != -1)
      params.append("cust_id", _custid);
    if (_scheduledDate->isValid())
      params.append("shipDate", _scheduledDate->date());
    params.append("sohead_id", _soheadid);
    if (ISQUOTE(_mode))
      params.append("mode", "newQuote");
    else
      params.append("mode", "new");
    set(params);
    setItemExtraClause();
  }
}

void salesOrderItem::sPrev()
{
  XSqlQuery salesPrev;
  if (_modified)
  {
    switch ( QMessageBox::question( this, tr("Unsaved Changed"),
                                    tr("<p>You have made some changes which have not yet been saved!\n"
                                         "Would you like to save them now?"),
                                    QMessageBox::Yes | QMessageBox::Default,
                                    QMessageBox::No,
                                    QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave(false);
        if (_modified)  // catch an error saving
          return;

      case QMessageBox::No:
        break;

      case QMessageBox::Cancel:
      default:
        return;
    }
  }

  clear();
  prepare();
  _item->setFocus();

  if (ISQUOTE(_mode))
  {
    if (cNewQuote == _mode)
      salesPrev.prepare("SELECT quitem_id AS id, 0 AS sub"
                "  FROM quitem"
                " WHERE (quitem_quhead_id=:sohead_id)"
                " ORDER BY quitem_linenumber DESC"
                " LIMIT 1;");
    else
      salesPrev.prepare("SELECT a.quitem_id AS id, 0 AS sub"
                "  FROM quitem AS a, quitem as b"
                " WHERE ((a.quitem_quhead_id=b.quitem_quhead_id)"
                "   AND  (a.quitem_linenumber < b.quitem_linenumber)"
                "   AND  (b.quitem_id=:id))"
                " ORDER BY a.quitem_linenumber DESC"
                " LIMIT 1;");
  }
  else
  {
    if (cNew == _mode)
      salesPrev.prepare("SELECT coitem_id AS id, coitem_subnumber AS sub"
                "  FROM coitem"
                " WHERE (coitem_cohead_id=:sohead_id)"
                " ORDER BY coitem_linenumber DESC, coitem_subnumber DESC"
                " LIMIT 1;");
    else
      salesPrev.prepare("SELECT a.coitem_id AS id, a.coitem_subnumber AS sub"
                "  FROM coitem AS a, coitem AS b"
                " WHERE ((a.coitem_cohead_id=b.coitem_cohead_id)"
		"   AND  (a.coitem_status <> 'X')"
                "   AND ((a.coitem_linenumber < b.coitem_linenumber)"
                "    OR ((a.coitem_linenumber = b.coitem_linenumber)"
                "   AND  (a.coitem_subnumber < b.coitem_subnumber)))"
                "   AND  (b.coitem_id=:id))"
                " ORDER BY a.coitem_linenumber DESC, a.coitem_subnumber DESC"
                " LIMIT 1;");
  }
  salesPrev.bindValue(":id", _soitemid);
  salesPrev.bindValue(":sohead_id", _soheadid);
  salesPrev.exec();
  if (salesPrev.first())
  {
    ParameterList params;
    if (_custid != -1)
      params.append("cust_id", _custid);
    params.append("soitem_id", salesPrev.value("id").toInt());
    if (ISQUOTE(_mode))
    {
      if (cNewQuote == _mode || cEditQuote == _mode)
        params.append("mode", "editQuote");
      else
        params.append("mode", "viewQuote");
    }
    else
    {
      if ((cNew == _initialMode || cEdit == _initialMode) && (salesPrev.value("sub").toInt() == 0))
        params.append("mode", "edit");
      else
        params.append("mode", "view");
    }
    set(params);
  }
  else if (salesPrev.lastError().type() != QSqlError::NoError)
  {
    systemError(this, salesPrev.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void salesOrderItem::sChanged()
{
  _modified = true;
  
}

void salesOrderItem::reject()
{
  XSqlQuery salesreject;
  bool saved = false;
  if (_modified)
  {
    switch ( QMessageBox::question( this, tr("Unsaved Changed"),
                                    tr("<p>You have made some changes which have not yet been saved!\n"
                                         "Would you like to save them now?"),
                                    QMessageBox::Yes | QMessageBox::Default,
                                    QMessageBox::No,
                                    QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave(false);
        if (_modified)  // catch an error saving
          return;

        saved = true;
        break;

      case QMessageBox::No:
        break;

      case QMessageBox::Cancel:
      default:
        return;
    }
  }

  if (!saved && (cNew == _mode || cNewQuote == _mode))
  {
    // DELETE ANY COMMENTS
    if (ISQUOTE(_mode))
      salesreject.prepare("SELECT deleteQuoteItem(:coitem_id);"
                          "DELETE FROM comment WHERE comment_source_id=:coitem_id AND comment_source = 'QI';");
    else
      salesreject.prepare("SELECT deleteSoItem(:coitem_id);"
                          "DELETE FROM comment WHERE comment_source_id=:coitem_id AND comment_source = 'SI';");
    salesreject.bindValue(":coitem_id", _soitemid);
    salesreject.exec();
    if (salesreject.lastError().type() != QSqlError::NoError)
    {
      systemError(this, salesreject.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  XDialog::reject();
}

void salesOrderItem::sCancel()
{
  XSqlQuery salesCancel;
  _canceling = true;

  sSave(false);
  if (_error)
    return;

  if ( (_mode == cEdit) || (_mode == cNew) )
  {
    XSqlQuery existpo;
    existpo.prepare("SELECT 1"  // cheaper than EXISTS?
                    " FROM poitem JOIN coitem ON (coitem_order_id = poitem_id)"
                    " WHERE ((coitem_id = :soitem_id)"
                    "   AND  (coitem_order_type='P'));" );
    existpo.bindValue(":soitem_id", _soitemid);
    existpo.exec();
    if (existpo.first())
    {
      QMessageBox::warning(this, tr("Can not delete PO"),
                           tr("Purchase Order linked to this Sales Order "
                              "Item will not be affected. The Purchase Order "
                              "should be closed or deleted manually if necessary."));
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Linked P/O"),
                                  existpo, __FILE__, __LINE__))
    {
      return;
    }
  }

  salesCancel.prepare("UPDATE coitem SET coitem_status='X' WHERE (coitem_id=:coitem_id);");
  salesCancel.bindValue(":coitem_id", _soitemid);
  salesCancel.exec();
  if (salesCancel.lastError().type() != QSqlError::NoError)
  {
      systemError(this, salesCancel.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  clear();
  prepare();
  _prev->setEnabled(true);
  _item->setFocus();

  _modified  = false;
  _canceling = false;
}

void salesOrderItem::sLookupTax()
{
  XSqlQuery calcq;
  calcq.prepare("SELECT calculateTax(:taxzone_id, :taxtype_id, :date, :curr_id, :ext ) AS val");

  calcq.bindValue(":taxzone_id",  _taxzoneid);
  calcq.bindValue(":taxtype_id",  _taxtype->id());
  calcq.bindValue(":date",  _netUnitPrice->effective());
  calcq.bindValue(":curr_id",  _netUnitPrice->id());
  calcq.bindValue(":ext",     _extendedPrice->localValue());

  calcq.exec();
  if (calcq.first())
  {
    _cachedRate= calcq.value("val").toDouble();
    _tax->setLocalValue(_cachedRate );
  }
  else if (calcq.lastError().type() != QSqlError::NoError)
  {
      systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void salesOrderItem::sTaxDetail()
{
  taxDetail     newdlg(this, "", true);
  ParameterList params;
  params.append("taxzone_id",   _taxzoneid);
  params.append("taxtype_id",  _taxtype->id());
  params.append("date", _netUnitPrice->effective());
  params.append("curr_id",   _netUnitPrice->id());
  params.append("subtotal",    _extendedPrice->localValue());

  if (_mode == cView)
    params.append("readOnly");

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    if (_taxtype->id() != newdlg.taxtype())
      _taxtype->setId(newdlg.taxtype());
  }
}

void salesOrderItem::sQtyUOMChanged()
{
  if (_qtyUOM->id() == _invuomid)
  {
    _qtyinvuomratio = 1.0;
    if (_invIsFractional)
      _qtyOrdered->setValidator(omfgThis->qtyVal());
    else
      _qtyOrdered->setValidator(new QIntValidator(this));
  }
  else
  {
    XSqlQuery invuom;
    invuom.prepare("SELECT itemuomtouomratio(item_id, :uom_id, item_inv_uom_id) AS ratio,"
                   "       itemuomfractionalbyuom(item_id, :uom_id) AS frac "
                   "  FROM item"
                   " WHERE(item_id=:item_id);");
    invuom.bindValue(":item_id", _item->id());
    invuom.bindValue(":uom_id", _qtyUOM->id());
    invuom.exec();
    if (invuom.first())
    {
      _qtyinvuomratio = invuom.value("ratio").toDouble();
      if (invuom.value("frac").toBool())
        _qtyOrdered->setValidator(omfgThis->qtyVal());
      else
        _qtyOrdered->setValidator(new QIntValidator(this));
    }
    else
      systemError(this, invuom.lastError().databaseText(), __FILE__, __LINE__);
  }

  if (_qtyUOM->id() != _invuomid || cView == _mode || cViewQuote == _mode)
    _priceUOM->setEnabled(false);
  else
    _priceUOM->setEnabled(true);
  _priceUOM->setId(_qtyUOM->id());
//  sCalculateExtendedPrice();
  sDeterminePrice(true);

  if (_qtyOrdered->toDouble() != (double)qRound(_qtyOrdered->toDouble()) &&
      _qtyOrdered->validator()->inherits("QIntValidator"))
  {
    QMessageBox::warning(this, tr("Invalid Quantity"),
                         tr("This UOM for this Item does not allow fractional "
                            "quantities. Please fix the quantity."));
    _qtyOrdered->setFocus();
    _qtyOrdered->setText("");
    return;
  }
}

void salesOrderItem::sPriceUOMChanged()
{
  if (_priceUOM->id() == -1 || _qtyUOM->id() == -1)
    return;

  if (_priceUOM->id() == _invuomid)
    _priceinvuomratio = 1.0;
  else
  {
    XSqlQuery invuom;
    invuom.prepare("SELECT itemuomtouomratio(item_id, :uom_id, item_inv_uom_id) AS ratio"
                   "  FROM item"
                   " WHERE(item_id=:item_id);");
    invuom.bindValue(":item_id", _item->id());
    invuom.bindValue(":uom_id", _priceUOM->id());
    invuom.exec();
    if (invuom.first())
      _priceinvuomratio = invuom.value("ratio").toDouble();
    else
      systemError(this, invuom.lastError().databaseText(), __FILE__, __LINE__);
  }

  XSqlQuery item;
  item.prepare("SELECT item_listprice,"
               "       itemCost(:item_id, :cust_id, :shipto_id, :qty, :qtyUOM, :priceUOM,"
               "                :curr_id, :effective, :asof, :warehous_id) AS unitcost "
               "  FROM item LEFT OUTER JOIN itemsite ON (itemsite_item_id=item_id AND itemsite_warehous_id=:warehous_id)"
               " WHERE(item_id=:item_id);");
  item.bindValue(":cust_id", _custid);
  item.bindValue(":shipto_id", _shiptoid);
  item.bindValue(":qty", _qtyOrdered->toDouble());
  item.bindValue(":qtyUOM", _qtyUOM->id());
  item.bindValue(":priceUOM", _priceUOM->id());
  item.bindValue(":curr_id", _customerPrice->id());
  item.bindValue(":effective", _customerPrice->effective());
  if (_metrics->value("soPriceEffective") == "OrderDate")
    item.bindValue(":asof", _netUnitPrice->effective());
  else if (_metrics->value("soPriceEffective") == "ScheduleDate" && _scheduledDate->isValid())
    item.bindValue(":asof", _scheduledDate->date());
  else
    item.bindValue(":asof", omfgThis->dbDate());
  item.bindValue(":item_id", _item->id());
  item.bindValue(":warehous_id", _warehouse->id());
  item.exec();
  item.first();
  _listPrice->setBaseValue(item.value("item_listprice").toDouble() * (_priceinvuomratio / _priceRatio));
  _unitCost->setBaseValue(item.value("unitcost").toDouble() * _priceinvuomratio);
  sDeterminePrice(true);
}

void salesOrderItem::sCalcUnitCost()
{
  XSqlQuery salesCalcUnitCost;
  if (_costmethod == "J" && _supplyOrderId > -1 && _qtyOrdered->toDouble() != 0)
  {
    salesCalcUnitCost.prepare("SELECT COALESCE(SUM(wo_postedvalue),0) AS wo_value "
              "FROM wo "
              "WHERE ((wo_ordtype='S') "
              "AND (wo_ordid=:soitem_id));");
    salesCalcUnitCost.bindValue(":soitem_id", _soitemid);
    salesCalcUnitCost.exec();
    if (salesCalcUnitCost.first())
      _unitCost->setBaseValue(salesCalcUnitCost.value("wo_value").toDouble() / _qtyOrdered->toDouble() * _qtyinvuomratio);
  }
  else
  {
    salesCalcUnitCost.prepare( "SELECT * FROM "
                      "itemCost(:item_id, :cust_id, :shipto_id, :qty, :qtyUOM, :priceUOM,"
                      "         :curr_id, :effective, :asof, :warehous_id) AS unitcost;" );
    salesCalcUnitCost.bindValue(":cust_id", _custid);
    salesCalcUnitCost.bindValue(":shipto_id", _shiptoid);
    salesCalcUnitCost.bindValue(":qty", _qtyOrdered->toDouble());
    salesCalcUnitCost.bindValue(":qtyUOM", _qtyUOM->id());
    salesCalcUnitCost.bindValue(":priceUOM", _priceUOM->id());
    salesCalcUnitCost.bindValue(":item_id", _item->id());
    salesCalcUnitCost.bindValue(":curr_id", _customerPrice->id());
    salesCalcUnitCost.bindValue(":effective", _customerPrice->effective());
    if (_metrics->value("soPriceEffective") == "OrderDate")
      salesCalcUnitCost.bindValue(":asof", _netUnitPrice->effective());
    else if (_metrics->value("soPriceEffective") == "ScheduleDate" && _scheduledDate->isValid())
      salesCalcUnitCost.bindValue(":asof", _scheduledDate->date());
    else
      salesCalcUnitCost.bindValue(":asof", omfgThis->dbDate());
    salesCalcUnitCost.bindValue(":warehous_id", _warehouse->id());
    salesCalcUnitCost.exec();
    if (salesCalcUnitCost.first())
      _unitCost->setBaseValue(salesCalcUnitCost.value("unitcost").toDouble() * _priceinvuomratio);
  }
}

void salesOrderItem::sHandleButton()
{
  if (_inventoryButton->isChecked())
    _availabilityStack->setCurrentWidget(_inventoryPage);
  else if (_itemSourcesButton->isChecked())
    _availabilityStack->setCurrentWidget(_itemSourcesPage);
  else if (_supplyOrderButton->isChecked())
    _availabilityStack->setCurrentWidget(_supplyOrderPage);
  else
    _availabilityStack->setCurrentWidget(_substitutesPage);

  if (_historyCostsButton->isChecked())
    _historyStack->setCurrentWidget(_historyCostsPage);
  else if (_historySalesButton->isChecked())
    _historyStack->setCurrentWidget(_historySalesPage);

  if ((_item->itemType() == "K"))
  {
    int lineNum, headId;
    XSqlQuery firstQry;

    firstQry.prepare("SELECT coitem_cohead_id, coitem_linenumber "
                               "FROM coitem "
                               "JOIN itemsite ON (itemsite_id=coitem_itemsite_id) "
                               "JOIN item ON (itemsite_item_id=item_id) "
                               "WHERE (coitem_id=:coitemid) AND (item_type='K');");
    firstQry.bindValue(":coitemid", _soitemid);
    firstQry.exec();
    if (firstQry.first())
    {
      XSqlQuery secondQry;
      lineNum = firstQry.value("coitem_linenumber").toInt();
      headId = firstQry.value("coitem_cohead_id").toInt();
      secondQry.prepare("SELECT coitem_qtyshipped "
                                  "FROM coitem "
                                  "JOIN cohead ON (cohead_id=coitem_cohead_id) "
                                  "WHERE (coitem_linenumber=:line) "
                                  "AND (coitem_cohead_id=:head) "
                                  "AND (coitem_qtyshipped>0);");
      secondQry.bindValue(":line", lineNum);
      secondQry.bindValue(":head", headId);
      secondQry.exec();
      if(secondQry.first())
      {
          QMessageBox::critical(this, tr("Kit"),
                                tr("At least one child item from this kit has already shipped."));
         _cancel->setEnabled(false);
      }
      else
      {
        XSqlQuery thirdQry;
        thirdQry.prepare("SELECT shipitem_qty, shiphead_shipped "
                         "FROM shipitem "
                         "JOIN shiphead ON (shiphead_id=shipitem_shiphead_id) "
                         "WHERE (shipitem_orderitem_id=:soitemid) "
                         "AND (shiphead_shipped='f') "
                         "AND (shipitem_qty>0)");
        thirdQry.bindValue(":soitemid", _soitemid);
        thirdQry.exec();
        if(thirdQry.first())
        {
            QMessageBox::critical(this, tr("Kit"),
                                  tr("At least one child item from this kit is at shipping."));
            _cancel->setEnabled(false);
        }
      }
    }
  }
}

void salesOrderItem::setItemExtraClause()
{
  if (_mode != cNew)
    return;

  _item->clearExtraClauseList();
  _item->addExtraClause("(itemsite_active)" );
  _item->addExtraClause("(itemsite_sold)");
  _item->addExtraClause( QString("(item_id IN (SELECT custitem FROM custitem(%1, %2, '%3') ) )").arg(_custid).arg(_shiptoid).arg(_scheduledDate->date().toString(Qt::ISODate)) );
}

void salesOrderItem::sHandleScheduleDate()
{
  XSqlQuery salesHandleScheduleDate;
  if ((!_scheduledDate->isValid() ||
      (_scheduledDate->date() == _scheduledDateCache)))
  {
//    sDetermineAvailability();
    return;
  }

  if (_createSupplyOrder->isChecked())
  {
    XSqlQuery checkpo;
    checkpo.prepare( "SELECT pohead_id, poitem_id, poitem_status "
                     "FROM pohead JOIN poitem ON (pohead_id = poitem_pohead_id) "
                     "            JOIN coitem ON (coitem_order_id = poitem_id) "
                     "WHERE ((coitem_id = :soitem_id) "
                     "  AND  (coitem_order_type='P'));" );
    checkpo.bindValue(":soitem_id", _soitemid);
    checkpo.exec();
    if (checkpo.first())
    {
      if ((checkpo.value("poitem_status").toString()) == "C")
      {
        QMessageBox::critical(this, tr("Cannot Update Item"),
                              tr("The Purchase Order Item this Sales Order Item is linked to is closed.  The date may not be updated."));
        _scheduledDate->setDate(_scheduledDateCache);
        _scheduledDate->setFocus();
        return;
      }
    }
  }

  // Check effectivity
  setItemExtraClause();
  if (_item->isValid() && _metrics->value("soPriceEffective") == "ScheduleDate")
  {
    ParameterList params;
    params.append("item_id", _item->id());
    params.append("cust_id", _custid);
    params.append("shipto_id", _shiptoid);
    params.append("shipDate", _scheduledDate->date());

    QString sql("SELECT customerCanPurchase(<? value('item_id') ?>, "
                "<? value('cust_id') ?>, "
                "<? value('shipto_id') ?>, "
                "<? value('shipDate') ?>) AS canPurchase; ");

    MetaSQLQuery mql(sql);
    salesHandleScheduleDate = mql.toQuery(params);
    if (salesHandleScheduleDate.first())
    {
      if (!salesHandleScheduleDate.value("canPurchase").toBool())
      {
        QMessageBox::critical(this, tr("Invalid Item for Date"),
                              tr("This item may not be purchased in this date.  Please select another date or item."));
        _scheduledDate->setDate(_scheduledDateCache);
        _scheduledDate->setFocus();
        return;
      }
    }
  }

  sDeterminePrice();
  sDetermineAvailability();
}
