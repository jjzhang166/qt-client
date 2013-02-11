/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
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
#include "itemCharacteristicDelegate.h"
#include "priceList.h"
#include "storedProcErrorLookup.h"
#include "taxDetail.h"
#include "xdoublevalidator.h"
#include "itemSourceList.h"
#include "maintainItemCosts.h"

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

  connect(_item,              SIGNAL(privateIdChanged(int)),        this, SLOT(sFindSellingWarehouseItemsites(int)));
  connect(_item,              SIGNAL(newId(int)),                   this, SLOT(sPopulateItemInfo(int)));
  connect(_item,              SIGNAL(newId(int)),                   this, SLOT(sPopulateItemSources(int)));
  connect(_item,              SIGNAL(newId(int)),                   this, SLOT(sPopulateHistory()));
  connect(_listPrices,        SIGNAL(clicked()),                    this, SLOT(sListPrices()));
  connect(_netUnitPrice,      SIGNAL(idChanged(int)),               this, SLOT(sPriceGroup()));
  connect(_netUnitPrice,      SIGNAL(valueChanged()),               this, SLOT(sCalculateExtendedPrice()));
  connect(_qtyOrdered,        SIGNAL(editingFinished()),                  this, SLOT(sPopulateOrderInfo()));
  connect(_qtyOrdered,        SIGNAL(editingFinished()),                  this, SLOT(sDetermineAvailability()));
  connect(_qtyOrdered,        SIGNAL(editingFinished()),                  this, SLOT(sDeterminePrice()));
  connect(_qtyOrdered,        SIGNAL(textChanged(const QString &)), this, SLOT(sCalcWoUnitCost()));
  connect(_save,              SIGNAL(clicked()),                    this, SLOT(sSave()));
  connect(_scheduledDate,     SIGNAL(newDate(const QDate &)),       this, SLOT(sHandleScheduleDate()));
  connect(_showAvailability,  SIGNAL(toggled(bool)),                this, SLOT(sDetermineAvailability()));
  connect(_showIndented,      SIGNAL(toggled(bool)),                this, SLOT(sDetermineAvailability()));
  connect(_item,              SIGNAL(privateIdChanged(int)),        this, SLOT(sPopulateItemsiteInfo()));
  connect(_warehouse,         SIGNAL(newID(int)),                   this, SLOT(sPopulateItemsiteInfo()));
  connect(_warehouse,         SIGNAL(newID(int)),                   this, SLOT(sDetermineAvailability()));
  connect(_next,              SIGNAL(clicked()),                    this, SLOT(sNext()));
  connect(_prev,              SIGNAL(clicked()),                    this, SLOT(sPrev()));
  connect(_notes,             SIGNAL(textChanged()),                this, SLOT(sChanged()));
  connect(_createSupplyOrder, SIGNAL(toggled(bool)),                this, SLOT(sChanged()));
  connect(_supplyOrderDueDate,SIGNAL(newDate(const QDate &)),       this, SLOT(sChanged()));
  connect(_supplyWarehouse,   SIGNAL(newID(int)),                   this, SLOT(sChanged()));
  connect(_promisedDate,      SIGNAL(newDate(const QDate &)),       this, SLOT(sChanged()));
  connect(_scheduledDate,     SIGNAL(newDate(const QDate &)),       this, SLOT(sChanged()));
  connect(_netUnitPrice,      SIGNAL(valueChanged()),               this, SLOT(sChanged()));
  connect(_qtyOrdered,        SIGNAL(textChanged(const QString &)), this, SLOT(sChanged()));
  connect(_warehouse,         SIGNAL(newID(int)),                   this, SLOT(sChanged()));
  connect(_item,              SIGNAL(newId(int)),                   this, SLOT(sChanged()));
  connect(_cancel,            SIGNAL(clicked()),                    this, SLOT(sCancel()));
  connect(_extendedPrice,     SIGNAL(valueChanged()),               this, SLOT(sLookupTax()));
  connect(_taxLit,            SIGNAL(leftClickedURL(QString)),      this, SLOT(sTaxDetail()));
  connect(_taxtype,           SIGNAL(newID(int)),                   this, SLOT(sLookupTax()));
  connect(_qtyUOM,            SIGNAL(newID(int)),                   this, SLOT(sQtyUOMChanged()));
  connect(_priceUOM,          SIGNAL(newID(int)),                   this, SLOT(sPriceUOMChanged()));
  connect(_inventoryButton,   SIGNAL(toggled(bool)),                this, SLOT(sHandleButton()));
  connect(_itemSourcesButton, SIGNAL(toggled(bool)),                this, SLOT(sHandleButton()));
  connect(_dependenciesButton,SIGNAL(toggled(bool)),                this, SLOT(sHandleButton()));
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
  _error                 = false;
  _originalQtyOrd        = 0.0;
  _updateItemsite        = false;
  _updatePrice           = true;
  _qtyinvuomratio        = 1.0;
  _priceinvuomratio      = 1.0;
  _priceRatio            = 1.0;
  _invuomid              = -1;
  _invIsFractional       = false;
  _qtyreserved           = 0.0;
  _createPO              = false;
  _createPR              = false;
  _priceType             = "N";  // default to nominal
  _priceMode             = "D";  // default to default

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

  _availability->addColumn(tr("#"),           _seqColumn, Qt::AlignCenter,true, "seqnumber");
  _availability->addColumn(tr("Item Number"),_itemColumn, Qt::AlignLeft,  true, "item_number");
  _availability->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "item_descrip");
  _availability->addColumn(tr("UOM"),         _uomColumn, Qt::AlignCenter,true, "uom_name");
  _availability->addColumn(tr("Pend. Alloc."),_qtyColumn, Qt::AlignRight, true, "pendalloc");
  _availability->addColumn(tr("Total Alloc."),_qtyColumn, Qt::AlignRight, true, "totalalloc");
  _availability->addColumn(tr("On Order"),    _qtyColumn, Qt::AlignRight, true, "ordered");
  _availability->addColumn(tr("QOH"),         _qtyColumn, Qt::AlignRight, true, "qoh");
  _availability->addColumn(tr("Availability"),_qtyColumn, Qt::AlignRight, true, "totalavail");

  _itemsrcp->addColumn(tr("Vendor #"),    _itemColumn, Qt::AlignLeft, true, "vend_number");
  _itemsrcp->addColumn(tr("Vendor Name"),          -1, Qt::AlignLeft, true, "vend_name");
  _itemsrcp->addColumn(tr("Description"),          -1, Qt::AlignLeft, true, "itemsrc_descrip");
  _itemsrcp->addColumn(tr("Qty Break"),    _qtyColumn, Qt::AlignRight,true, "itemsrcp_qtybreak");
  _itemsrcp->addColumn(tr("Base Price"), _moneyColumn, Qt::AlignRight,true, "price_base");

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

  _itemchar = new QStandardItemModel(0, 3, this);
  _itemchar->setHeaderData( CHAR_ID, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
  _itemchar->setHeaderData( CHAR_VALUE, Qt::Horizontal, tr("Value"), Qt::DisplayRole);
  _itemchar->setHeaderData( CHAR_PRICE, Qt::Horizontal, tr("Price"), Qt::DisplayRole);

  _itemcharView->hideColumn(CHAR_PRICE);
  _baseUnitPriceLit->hide();
  _baseUnitPrice->hide();
  _supplyDropShip->hide();

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
  _supplyOrderQty->setPrecision(omfgThis->qtyVal());
  _discountFromCust->setValidator(omfgThis->negPercentVal());

  _shippedToDate->setPrecision(omfgThis->qtyVal());
  _discountFromListPrice->setPrecision(omfgThis->percentVal());
  _markupFromListCost->setPrecision(omfgThis->percentVal());
  _profit->setPrecision(omfgThis->percentVal());
  _onHand->setPrecision(omfgThis->qtyVal());
  _allocated->setPrecision(omfgThis->qtyVal());
  _unallocated->setPrecision(omfgThis->qtyVal());
  _onOrder->setPrecision(omfgThis->qtyVal());
  _available->setPrecision(omfgThis->qtyVal());

  //  Disable the Discount Percent stuff if we don't allow them
  if ((!_metrics->boolean("AllowDiscounts")) && (!_privileges->check("OverridePrice")))
  {
    _netUnitPrice->setEnabled(FALSE);
    _discountFromCust->setEnabled(FALSE);
  }

  if ((_metrics->boolean("DisableSalesOrderPriceOverride")) || (!_privileges->check("OverridePrice")))
  {
    _netUnitPrice->setEnabled(false);
  }

  _supplyOverridePrice->hide();
  _supplyOverridePriceLit->hide();

  _supplyOrderId = -1;
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
  _dependenciesButton->setEnabled(_showAvailability->isChecked());
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
      _supplyOrderStatusLit->hide();
      _supplyOrderStatus->hide();
      _item->setReadOnly(false);
      _warehouse->setEnabled(true);
      _cancel->setEnabled(false);
      _supplyOrderId = -1;
      _itemsrc = -1;

      _item->addExtraClause( QString("(item_id IN (SELECT custitem FROM custitem(%1, %2, '%3') ) )").arg(_custid).arg(_shiptoid).arg(asOf.toString(Qt::ISODate)) );

      prepare();

      connect(_netUnitPrice,      SIGNAL(editingFinished()),    this,         SLOT(sCalculateDiscountPrcnt()));
      connect(_discountFromCust,  SIGNAL(editingFinished()),    this,         SLOT(sCalculateFromDiscount()));
      connect(_item,              SIGNAL(valid(bool)),          _listPrices,  SLOT(setEnabled(bool)));
      connect(_createSupplyOrder, SIGNAL(toggled(bool)),        this,         SLOT(sHandleWo(bool)));

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
      _supplyOrderStatusLit->hide();
      _supplyOrderStatus->hide();
      // _createSupplyOrder->hide();
      _supplyOrderQtyLit->hide();
      _supplyOrderQty->hide();
      _supplyOrderDueDateLit->hide();
      _supplyOrderDueDate->hide();
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

      connect(_netUnitPrice,      SIGNAL(editingFinished()),  this,         SLOT(sCalculateDiscountPrcnt()));
      connect(_discountFromCust,  SIGNAL(editingFinished()),  this,         SLOT(sCalculateFromDiscount()));
      connect(_item,              SIGNAL(valid(bool)),  _listPrices,  SLOT(setEnabled(bool)));

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
      connect(_createSupplyOrder, SIGNAL(toggled(bool)),        this, SLOT(sHandleWo(bool)));
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
      // _createSupplyOrder->hide();
      _supplyOrderQtyLit->hide();
      _supplyOrderQty->hide();
      _supplyOrderDueDateLit->hide();
      _supplyOrderDueDate->hide();
      _supplyOrderStatusLit->hide();
      _supplyOrderStatus->hide();
      _cancel->hide();
      _sub->hide();
      _subItem->hide();
      _subItemList->hide();
      _warranty->hide();
      _tabs->removeTab(_tabs->indexOf(_costofsalesTab));

      connect(_qtyOrdered,        SIGNAL(editingFinished()),  this, SLOT(sCalculateExtendedPrice()));
      connect(_netUnitPrice,      SIGNAL(editingFinished()),  this, SLOT(sCalculateDiscountPrcnt()));
      connect(_discountFromCust,  SIGNAL(editingFinished()),  this, SLOT(sCalculateFromDiscount()));
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

      // _createSupplyOrder->hide();
      _supplyOrderQtyLit->hide();
      _supplyOrderQty->hide();
      _supplyOrderDueDateLit->hide();
      _supplyOrderDueDate->hide();
      _supplyOrderStatusLit->hide();
      _supplyOrderStatus->hide();
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
    _qtyOrdered->setEnabled(!viewMode);
    _netUnitPrice->setEnabled(!viewMode);
    _discountFromCust->setEnabled(!viewMode);
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

  return NoError;
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
}

void salesOrderItem::clear()
{
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
  _avgCost->clear();
  _listCost->clear();
  _listPrice->clear();
  _customerPrice->clear();
  _discountFromListPrice->clear();
  _markupFromListCost->clear();
  _discountFromCust->clear();
  _profit->clear();
  _shippedToDate->clear();
  _createSupplyOrder->setChecked(FALSE);
  _supplyOrderQty->clear();
  _supplyOrderDueDate->clear();
  _supplyOrderStatus->clear();
  _onHand->clear();
  _allocated->clear();
  _unallocated->clear();
  _onOrder->clear();
  _available->clear();
  _itemsrcp->clear();
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
  _supplyOverridePrice->clear();
  _originalQtyOrd  = 0.0;
  _qtyOrderedCache   = 0.0;
  _supplyOrderQtyCache   = 0.0;
  _priceUOMCache   = -1;
  _modified        = false;
  _updateItemsite  = false;
  _baseUnitPrice->clear();
  _itemcharView->setEnabled(TRUE);
  _itemsrc = -1;
  _altRevAccnt->clear();
}

void salesOrderItem::sSave()
{
  XSqlQuery salesSave;
  _save->setFocus();

  int   itemsrcid  = _itemsrc;
  if (_createPO && _createSupplyOrder->isChecked() && (_item->itemType() == "P") &&
    ((_mode == cNew) || ((_mode == cEdit) && (_supplyOrderId == -1))))
  {
    if ( _supplyDropShip->isChecked() && _shiptoid < 1)
    {
      QMessageBox::critical(this, tr("Cannot Save Sales Order Item"),
                            tr("<p>You must enter a valid Ship-To # before saving this Sales Order Item."));
      return;
    }

    if (itemsrcid==-1)
    {
      XSqlQuery itemsrcdefault;
      itemsrcdefault.prepare("SELECT itemsrc_id FROM itemsrc "
                             "WHERE ((itemsrc_item_id=:item_id) AND (itemsrc_default) AND (itemsrc_active)) ");
      itemsrcdefault.bindValue(":item_id", _item->id());
      itemsrcdefault.exec();
      if (itemsrcdefault.first())
      {
        itemsrcid=(itemsrcdefault.value("itemsrc_id").toInt());
      }
      else if (itemsrcdefault.lastError().type() != QSqlError::NoError)
      {
        systemError(this, itemsrcdefault.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      else
      {
        ParameterList itemSourceParams;
        itemSourceParams.append("item_id", _item->id());
        itemSourceParams.append("qty", _qtyOrdered->toDouble());
        itemSourceList newdlg(omfgThis, "", TRUE);
        newdlg.set(itemSourceParams);
        itemsrcid = newdlg.exec();
      }
    }
  }

  _error = true;
  if (!_warehouse->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order Item"),
                          tr("<p>You must select a valid Site before saving this Sales Order Item.")  );
    _warehouse->setFocus();
    return;
  }

  if (!(_qtyOrdered->toDouble() > 0))
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order Item"),
                          tr("<p>You must enter a valid Quantity Ordered before saving this Sales Order Item.")  );
    _qtyOrdered->setFocus();
    return;
  }
  else if (_qtyOrdered->toDouble() != (double)qRound(_qtyOrdered->toDouble()) &&
           _qtyOrdered->validator()->inherits("QIntValidator"))
  {
    QMessageBox::warning(this, tr("Invalid Quantity"),
                          tr("This UOM for this Item does not allow fractional "
                            "quantities. Please fix the quantity."));
    _qtyOrdered->setFocus();
    return;
  }

  // Make sure Qty Ordered/Qty UOM does not result in an invalid fractional Inv Qty
  if (!_invIsFractional)
  {
    if (qAbs((_qtyOrdered->toDouble() * _qtyinvuomratio) - (double)qRound(_qtyOrdered->toDouble() * _qtyinvuomratio)) > 0.01)
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
        _qtyOrdered->setFocus();
        return;
      }
    }
  }

  if (_netUnitPrice->isEmpty())
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order Item"),
                          tr("<p>You must enter a Price before saving this Sales Order Item.") );
    _netUnitPrice->setFocus();
    return;
  }

  if ( (_sub->isChecked()) && (!_subItem->isValid()) )
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order Item"),
                          tr("<p>You must enter a Substitute Item before saving this Sales Order Item.") );
    _subItem->setFocus();
    return;
  }

  if (_metrics->boolean("AllowASAPShipSchedules") && !_scheduledDate->isValid())
    _scheduledDate->setDate(QDate::currentDate());
  if (!(_scheduledDate->isValid()))
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order Item"),
                          tr("<p>You must enter a valid Schedule Date before saving this Sales Order Item.") );
    _scheduledDate->setFocus();
    return;
  }

  if (_createSupplyOrder->isChecked() && (_item->itemType() == "M") && _supplyWarehouse->id() == -1)
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order Item"),
                          tr("<p>Before an Order may be created, a valid Supplied at Site must be selected.") );
    return;
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

  if (_mode == cNew)
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
               "       stdCost(:item_id), :soitem_custprice, :soitem_pricemode,"
               "       :soitem_price, :price_uom_id, :price_invuomratio,"
               "       '', -1,"
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
    salesSave.exec();
    if (salesSave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (_mode == cEdit)
  {
    salesSave.prepare( "UPDATE coitem "
               "SET coitem_scheddate=:soitem_scheddate,"
               "    coitem_promdate=:soitem_promdate,"
               "    coitem_qtyord=:soitem_qtyord,"
               "    coitem_qty_uom_id=:qty_uom_id,"
               "    coitem_qty_invuomratio=:qty_invuomratio,"
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
               "    coitem_warranty=:soitem_warranty "
               "WHERE (coitem_id=:soitem_id);" );
    salesSave.bindValue(":soitem_scheddate", _scheduledDate->date());
    salesSave.bindValue(":soitem_promdate", promiseDate);
    salesSave.bindValue(":soitem_qtyord", _qtyOrdered->toDouble());
    salesSave.bindValue(":qty_uom_id", _qtyUOM->id());
    salesSave.bindValue(":qty_invuomratio", _qtyinvuomratio);
    salesSave.bindValue(":soitem_custprice", _customerPrice->localValue());
    salesSave.bindValue(":soitem_price", _netUnitPrice->localValue());
    salesSave.bindValue(":soitem_pricemode", _priceMode);
    salesSave.bindValue(":price_uom_id", _priceUOM->id());
    salesSave.bindValue(":price_invuomratio", _priceinvuomratio);
    salesSave.bindValue(":soitem_prcost", _supplyOverridePrice->localValue());
    salesSave.bindValue(":soitem_memo", _notes->toPlainText());
    if (_supplyOrderId != -1)
    {
      if (_item->itemType() == "M")
        salesSave.bindValue(":soitem_order_type", "W");
      else if ((_item->itemType() == "P") && _createPR)
        salesSave.bindValue(":soitem_order_type", "R");
      else if ((_item->itemType() == "P") && _createPO)
        salesSave.bindValue(":soitem_order_type", "P");
    }
    salesSave.bindValue(":soitem_order_id", _supplyOrderId);
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

    salesSave.exec();
    if (salesSave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    //  Check to see if a Reservations need changes
    if (_qtyOrdered->toDouble() < _qtyOrderedCache)
    {
      if (_qtyreserved > 0.0)
      {
        QMessageBox::warning( this, tr("Unreserve Sales Order Item"),
                              tr("<p>The quantity ordered for this Sales "
                                   "Order Line Item has been changed. "
                                   "Reservations have been removed.") );
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
        }
        else if (salesSave.lastError().type() != QSqlError::NoError)
        {
          systemError(this, tr("Line Item %1\n").arg("") +
                      salesSave.lastError().databaseText(), __FILE__, __LINE__);
        }
      }
    }

    //  Check to see if a W/O needs changes
    if (_supplyOrderId != -1)
    {
      XSqlQuery checkPO;
      checkPO.prepare("SELECT * FROM poitem JOIN coitem ON (coitem_order_id = poitem_id) "
                      "WHERE ((coitem_id = :soitem_id) "
                      "  AND  (coitem_order_type='P'));" );
      checkPO.bindValue(":soitem_id", _soitemid);
      checkPO.exec();
      if (checkPO.first())
      {
        if (checkPO.value("poitem_status").toString()== "C")
        {
          rollback.exec();
          QMessageBox::critical(this, tr("Quantity Can Not be Updated"),
                                tr(" The Purchase Order Item this Sales "
                                   " Order Item is linked to is closed.  "
                                   " The quantity may not be updated. "));
          return;
        }
      }
      else if (checkPO.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, checkPO.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      else
      {
        if (_scheduledDate->date() != _cScheduledDate)
        {
          if (_item->itemType() == "M")
          {
            if (QMessageBox::question(this, tr("Reschedule Work Order?"),
                                      tr("<p>The Scheduled Date for this Line "
                                           "Item has been changed.  Should the "
                                           "W/O for this Line Item be Re-"
                                           "Scheduled to reflect this change?"),
                                      QMessageBox::Yes | QMessageBox::Default,
                                      QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
            {
              salesSave.prepare("SELECT changeWoDates(:wo_id, wo_startdate + (:schedDate-wo_duedate), :schedDate, TRUE) AS result "
                        "FROM wo "
                        "WHERE (wo_id=:wo_id);");
              salesSave.bindValue(":wo_id", _supplyOrderId);
              salesSave.bindValue(":schedDate", _scheduledDate->date());
              salesSave.exec();
              if (salesSave.first())
              {
                int result = salesSave.value("result").toInt();
                if (result < 0)
                {
                  rollback.exec();
                  systemError(this, storedProcErrorLookup("changeWoDates", result),
                              __FILE__, __LINE__);
                  return;
                }
                _cScheduledDate=_scheduledDate->date();
              }
              else if (salesSave.lastError().type() != QSqlError::NoError)
              {
                rollback.exec();
                  systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
                return;
              }
            }
          }
          else
          {
            if (QMessageBox::question(this, tr("Reschedule P/R?"),
                                      tr("<p>The Scheduled Date for this Sales "
                                           "Order Line Item has been changed. "
                                           "Should the associated Purchase Request be changed "
                                           "to reflect this?"),
                                      QMessageBox::Yes | QMessageBox::Default,
                                      QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
            {
              salesSave.prepare("SELECT changePrDate(:pr_id, :schedDate) AS result;");
              salesSave.bindValue(":pr_id", _supplyOrderId);
              salesSave.bindValue(":schedDate", _scheduledDate->date());
              salesSave.exec();
              if (salesSave.first())
              {
                int result = salesSave.value("result").toInt();
                if (result < 0)
                {
                  rollback.exec();
                  systemError(this, storedProcErrorLookup("changePrDate", result),
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
          }
        }

        if (_supplyOrderQty->toDouble() != _supplyOrderQtyCache)
        {
          if (_item->itemType() == "M")
          {
            if (QMessageBox::question(this, tr("Change Work Order Quantity?"),
                                      tr("<p>The quantity ordered for this Sales "
                                           "Order Line Item has been changed. "
                                           "Should the quantity required for the "
                                           "associated Work Order be changed to "
                                           "reflect this?"),
                                      QMessageBox::No | QMessageBox::Escape,
                                      QMessageBox::Yes  | QMessageBox::Default) == QMessageBox::Yes)
            {
              salesSave.prepare("SELECT changeWoQty(:wo_id, :qty, TRUE) AS result;");
              salesSave.bindValue(":wo_id", _supplyOrderId);
              salesSave.bindValue(":qty", _supplyOrderQty->toDouble());
              salesSave.exec();
              if (salesSave.first())
              {
                int result = salesSave.value("result").toInt();
                if (result < 0)
                {
                  rollback.exec();
                  systemError(this, storedProcErrorLookup("changeWoQty", result),
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
          }
          else if (_createPR)
          {
            if (QMessageBox::question(this, tr("Change P/R Quantity?"),
                                      tr("<p>The quantity ordered for this Sales "
                                           "Order Line Item has been changed. "
                                           "Should the quantity required for the "
                                           "associated Purchase Request be changed "
                                           "to reflect this?"),
                                      QMessageBox::Yes | QMessageBox::Default,
                                      QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
            {
              salesSave.prepare("SELECT changePrQty(:pr_id, :qty) AS result;");
              salesSave.bindValue(":pr_id", _supplyOrderId);
              salesSave.bindValue(":qty", _qtyOrdered->toDouble() * _qtyinvuomratio);
              salesSave.exec();
              if (salesSave.first())
              {
                bool result = salesSave.value("result").toBool();
                if (!result)
                {
                  rollback.exec();
                  systemError(this, tr("changePrQty failed"), __FILE__, __LINE__);
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
          }
        }
      }
      // Update Supply Order Characteristics
      if (_itemchar->rowCount() > 0)
      {
        bool changed = false;
        XSqlQuery chgq;
        chgq.prepare("SELECT (charass_value=:value) AS same"
                     "  FROM charass"
                     " WHERE ((charass_char_id=:id)"
                     "    AND (charass_target_id=:target)"
                     "    AND (charass_target_type='SI'));");
        for (int i = 0; i < _itemchar->rowCount(); i++)
        {
          chgq.bindValue(":id",     _itemchar->data(_itemchar->index(i, CHAR_ID),
                                                    Qt::UserRole));
          chgq.bindValue(":value",  _itemchar->data(_itemchar->index(i, CHAR_VALUE)));
          chgq.bindValue(":target", _soitemid);
          if (chgq.exec() && chgq.first() && ! chgq.value("same").toBool())
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

        if (changed &&
            QMessageBox::question(this, tr("Change Characteristics?"),
                                  tr("<p>Should the characteristics for the "
                                       "associated supply order be updated?"),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
        {
          QModelIndex idx1, idx2;

          salesSave.prepare("SELECT updateCharAssignment(:target_type, :target_id, :char_id, :char_value) AS result;");

          for (int i = 0; i < _itemchar->rowCount(); i++)
          {
            idx1 = _itemchar->index(i, CHAR_ID);
            idx2 = _itemchar->index(i, CHAR_VALUE);
            if (_createPO)
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
  }
  else if (_mode == cNewQuote)
  {
    //  Grab the next quitem_id
    salesSave.exec("SELECT NEXTVAL('quitem_quitem_id_seq') AS _quitem_id");
    if (salesSave.first())
      _soitemid = salesSave.value("_quitem_id").toInt();
    else if (salesSave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
                  systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      rollback.exec();
      reject();
      return;
    }

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
               "       stdCost(:item_id), :quitem_custprice, :quitem_price, :quitem_pricemode,"
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
    if (itemsrcid > 0)
      salesSave.bindValue(":quitem_itemsrc_id", itemsrcid);
    salesSave.exec();
    if (salesSave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
          systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (_mode == cEditQuote)
  {
    salesSave.prepare( "UPDATE quitem "
               "SET quitem_scheddate=:quitem_scheddate,"
               "    quitem_promdate=:quitem_promdate,"
               "    quitem_qtyord=:quitem_qtyord,"
               "    quitem_qty_uom_id=:qty_uom_id,"
               "    quitem_qty_invuomratio=:qty_invuomratio,"
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
               "    quitem_itemsrc_id=:quitem_itemsrc_id "
               "WHERE (quitem_id=:quitem_id);" );
    salesSave.bindValue(":quitem_scheddate", _scheduledDate->date());
    salesSave.bindValue(":quitem_promdate", promiseDate);
    salesSave.bindValue(":quitem_qtyord", _qtyOrdered->toDouble());
    salesSave.bindValue(":qty_uom_id", _qtyUOM->id());
    salesSave.bindValue(":qty_invuomratio", _qtyinvuomratio);
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
    if (itemsrcid > 0)
      salesSave.bindValue(":quitem_itemsrc_id", itemsrcid);
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

  if ( (_mode != cView) && (_mode != cViewQuote) )
  {
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
  }

  //  If requested, create a new W/O or P/R for this coitem
  if ( ( (_mode == cNew) || (_mode == cEdit) ) &&
       (_createSupplyOrder->isChecked())               &&
       (_supplyOrderId == -1) )
  {
    QString chartype;
    if (_item->itemType() == "M")
    {
      salesSave.prepare( "SELECT createWo(:orderNumber, itemsite_id, :qty, itemsite_leadtime, :dueDate, :comments, :parent_type, :parent_id ) AS result, itemsite_id "
                 "FROM itemsite "
                 "WHERE ( (itemsite_item_id=:item_id)"
                 " AND (itemsite_warehous_id=:warehous_id) );" );
      salesSave.bindValue(":orderNumber", _orderNumber->text().toInt());
      salesSave.bindValue(":qty", _supplyOrderQty->toDouble());
      salesSave.bindValue(":dueDate", _scheduledDate->date());
      salesSave.bindValue(":comments", _custName + "\n" + _notes->toPlainText());
      salesSave.bindValue(":item_id", _item->id());
      salesSave.bindValue(":warehous_id", _supplyWarehouse->id());
      salesSave.bindValue(":parent_type", QString("S"));
      salesSave.bindValue(":parent_id", _soitemid);
      salesSave.exec();
    }
    else if ((_item->itemType() == "P") && _createPO)
    {
      if (_supplyOverridePrice->localValue() == 0.00)
        salesSave.prepare("SELECT createPurchaseToSale(:soitem_id, :itemsrc_id, :drop_ship) AS result;");
      else
      {
        salesSave.prepare("SELECT createPurchaseToSale(:soitem_id, :itemsrc_id, :drop_ship, :ovrridepoprc) AS result;");
        salesSave.bindValue(":ovrridepoprc", _supplyOverridePrice->localValue());
      }
      salesSave.bindValue(":soitem_id", _soitemid);
      salesSave.bindValue(":itemsrc_id", itemsrcid);
      salesSave.bindValue(":drop_ship", _supplyDropShip->isChecked());
      salesSave.exec();
    }
    else if (_item->itemType() == "P")
    {
      salesSave.prepare("SELECT createPr(:orderNumber, 'S', :soitem_id) AS result;");
      salesSave.bindValue(":orderNumber", _orderNumber->text().toInt());
      salesSave.bindValue(":soitem_id", _soitemid);
      salesSave.exec();
    }

    if (salesSave.first())
    {
      _supplyOrderId = salesSave.value("result").toInt();
      if (_supplyOrderId < 0)
      {
        rollback.exec();
        QString procname;
        if (_item->itemType() == "M")
          procname = "createWo";
        else if ((_item->itemType() == "P") && _createPR)
          procname = "createPr";
        else if ((_item->itemType() == "P") && _createPO)
          procname = "createPurchaseToSale";
        else
          procname = "unnamed stored procedure";
          systemError(this, storedProcErrorLookup(procname, _supplyOrderId),
                    __FILE__, __LINE__);
        return;
      }

      if (_item->itemType() == "M")
      {
        omfgThis->sWorkOrdersUpdated(_supplyOrderId, TRUE);

        //  Update the newly created coitem with the newly create wo_id
        salesSave.prepare( "UPDATE coitem "
                   "SET coitem_order_type='W', coitem_order_id=:orderid "
                   "WHERE (coitem_id=:soitem_id);" );
        salesSave.bindValue(":orderid", _supplyOrderId);
        salesSave.bindValue(":soitem_id", _soitemid);
        salesSave.exec();
        if (salesSave.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
        salesSave.prepare("INSERT INTO charass"
                  "      (charass_target_type, charass_target_id,"
                  "       charass_char_id, charass_value) "
                  "SELECT 'W', :orderid, charass_char_id, charass_value"
                  "  FROM charass"
                  " WHERE ((charass_target_type='SI')"
                  "   AND  (charass_target_id=:soitem_id));");
        salesSave.bindValue(":orderid", _supplyOrderId);
        salesSave.bindValue(":soitem_id", _soitemid);
        salesSave.exec();
        if (salesSave.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
      }
      else if ((_item->itemType() == "P") && !_createPO)
      {
        //  Update the newly created coitem with the newly pr_id
        salesSave.prepare( "UPDATE coitem "
                   "SET coitem_order_type='R', coitem_order_id=:orderid "
                   "WHERE (coitem_id=:soitem_id);" );
        salesSave.bindValue(":orderid", _supplyOrderId);
        salesSave.bindValue(":soitem_id", _soitemid);
        salesSave.exec();
        if (salesSave.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
      }
    }
    else if (salesSave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
          systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  salesSave.exec("COMMIT;");

  if (_mode == cNew)
    omfgThis->sSalesOrdersUpdated(_soheadid);
  else if (_mode == cNewQuote)
    omfgThis->sQuotesUpdated(_soheadid);

  if ( (!_canceling) && (cNew == _mode || cNewQuote == _mode) )
  {
    clear();
    prepare();
    _prev->setEnabled(true);
    _item->setFocus();
  }

  _modified = false;
}

void salesOrderItem::sPopulateItemsiteInfo()
{
  if (_item->isValid())
  {
    XSqlQuery itemsite;
    itemsite.prepare( "SELECT itemsite_leadtime, itemsite_costmethod, itemsite_createsopo, "
                      "       itemsite_createwo, itemsite_createsopr "
                      "FROM item, itemsite "
                      "WHERE ( (itemsite_item_id=item_id)"
                      " AND (itemsite_warehous_id=:warehous_id)"
                      " AND (item_id=:item_id) );" );
    itemsite.bindValue(":warehous_id", _warehouse->id());
    itemsite.bindValue(":item_id", _item->id());
    itemsite.exec();
    if (itemsite.first())
    {
      _leadTime    = itemsite.value("itemsite_leadtime").toInt();
      _costmethod  = itemsite.value("itemsite_costmethod").toString();

      if (cNew == _mode || cNewQuote == _mode)
      {
        if (_costmethod == "J")
        {
          _createSupplyOrder->setChecked(TRUE);
          _createSupplyOrder->setEnabled(FALSE);
        }
        else if (_item->itemType() == "M")
          _createSupplyOrder->setChecked(itemsite.value("itemsite_createwo").toBool());
        else if (_item->itemType() == "P")
        {
          _createPR = itemsite.value("itemsite_createsopr").toBool();
          _createSupplyOrder->setChecked(itemsite.value("itemsite_createsopr").toBool() || itemsite.value("itemsite_createsopo").toBool() );
        }
        else
        {
          _createSupplyOrder->setChecked(FALSE);
          _createSupplyOrder->setEnabled(FALSE);
        }
      }
    }
    else if (itemsite.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemsite.lastError().databaseText(), __FILE__, __LINE__);
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
             _scheduledDate->date() == _dateCache) ) ) )
    return;

  double  charTotal  =0;
  bool    dateChanged =(_dateCache != _scheduledDate->date());
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
                     "             :curr_id, :effective, :asof, NULL);" );
  itemprice.bindValue(":cust_id", _custid);
  itemprice.bindValue(":shipto_id", _shiptoid);
  itemprice.bindValue(":qty", _qtyOrdered->toDouble());
  itemprice.bindValue(":qtyUOM", _qtyUOM->id());
  itemprice.bindValue(":priceUOM", _priceUOM->id());
  itemprice.bindValue(":item_id", _item->id());
  itemprice.bindValue(":curr_id", _customerPrice->id());
  itemprice.bindValue(":effective", _customerPrice->effective());
  itemprice.bindValue(":asof", asOf);
  itemprice.exec();
  if (itemprice.first())
  {
    if (itemprice.value("itemprice_price").toDouble() == -9999.0)
    {
      if (!_updatePrice)
        return;

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
      if (_unitCost->baseValue() > 0.0)
        _profit->setDouble((_customerPrice->baseValue() - _unitCost->baseValue()) / _unitCost->baseValue() * 100.0);
      else
        _profit->setDouble(100.0);
      if (_updatePrice) // Configuration or user said they also want net unit price updated
        _netUnitPrice->setLocalValue(price + charTotal);

      sCalculateDiscountPrcnt();
      _qtyOrderedCache = _qtyOrdered->toDouble();
      _priceUOMCache = _priceUOM->id();
      _dateCache     = _scheduledDate->date();
    }
  }
  else if (itemprice.lastError().type() != QSqlError::NoError)
            systemError(this, itemprice.lastError().databaseText(), __FILE__, __LINE__);
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
               "       item_listcost, item_listprice, item_fractional,"
               "       stdcost(item_id) AS f_unitcost,"
               "       avgcost(item_id) AS f_avgcost,"
               "       itemsite_createsopo, itemsite_dropship,"
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
        _createPO = true;
        _createSupplyOrder->setTitle(tr("Create Purchase Order"));
        _supplyOrderLit->setText(tr("PO #:"));
        _supplyOrderLineLit->setText(tr("PO Line #:"));
        _supplyOrderStatusLit->show();
        _supplyOverridePrice->show();
        _supplyOverridePriceLit->show();
        if (_metrics->boolean("EnableDropShipments"))
        {
          _supplyDropShip->show();
          _supplyDropShip->setChecked(salesPopulateItemInfo.value("itemsite_dropship").toBool());
        }

        if (_mode == cNew || (ISQUOTE(_mode)))
        {
          _supplyOrderLit->hide();
          _supplyOrder->hide();
          _supplyOrderLineLit->hide();
          _supplyOrderLine->hide();
          _supplyOrderStatus->hide();
          _supplyOrderStatusLit->hide();
        }
        else
        {
          XSqlQuery povalues;
          povalues.prepare("SELECT pohead_number, poitem_linenumber, poitem_status, "
                           "ROUND(poitem_qty_ordered, 2) AS poitem_qty_ordered, "
                           "poitem_duedate, ROUND(poitem_unitprice, 2) AS "
                           "poitem_unitprice, pohead_dropship "
                           "FROM pohead JOIN poitem ON (pohead_id = poitem_pohead_id) "
                           "            JOIN coitem ON (coitem_order_id = poitem_id) "
                           "WHERE ((coitem_id = :soitem_id) "
                           "  AND  (coitem_order_type='P'));" );
          povalues.bindValue(":soitem_id", _soitemid);
          povalues.exec();
          if (povalues.first())
          {
            _supplyOrder->setText(povalues.value("pohead_number").toString());
            _supplyOrderLine->setText(povalues.value("poitem_linenumber").toString());
            _supplyOrderStatus->setText(povalues.value("poitem_status").toString());
            _supplyOrderQty->setDouble(povalues.value("poitem_qty_ordered").toDouble());
            _supplyOrderDueDate->setDate(povalues.value("poitem_duedate").toDate());
            _supplyDropShip->setChecked(povalues.value("pohead_dropship").toBool());
            _supplyOverridePrice->setLocalValue(povalues.value("poitem_unitprice").toDouble());

            _createSupplyOrder->setChecked(true);
            _createSupplyOrder->setEnabled(false);
          }
          else if (povalues.lastError().type() != QSqlError::NoError)
          {
            systemError(this, povalues.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
        }

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
          _createSupplyOrder->setChecked(FALSE);
          _createSupplyOrder->setEnabled(FALSE);
        }
        if (_metrics->boolean("EnableDropShipments") && (_mode == cEdit))
        {
          XSqlQuery dropship;
          dropship.prepare("SELECT pohead_dropship "
                           "FROM pohead JOIN poitem ON (pohead_id = poitem_pohead_id) "
                           "  RIGHT OUTER JOIN coitem ON (poitem_id = coitem_order_id) "
                           "WHERE ((coitem_id = :soitem_id) "
                           "  AND  (coitem_order_type='P'));" );
          dropship.bindValue(":soitem_id", _soitemid);
          dropship.exec();
          if (dropship.first())
            _supplyDropShip->setChecked(dropship.value("pohead_dropship").toBool());
          else if (dropship.lastError().type() != QSqlError::NoError)
          {
            systemError(this, dropship.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
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
      _listCost->setBaseValue(salesPopulateItemInfo.value("item_listcost").toDouble());
      _unitCost->setBaseValue(salesPopulateItemInfo.value("f_unitcost").toDouble());
      _avgCost->setBaseValue(salesPopulateItemInfo.value("f_avgcost").toDouble());
      _taxtype->setId(salesPopulateItemInfo.value("taxtype_id").toInt());

      sCalculateDiscountPrcnt();
      if (!(salesPopulateItemInfo.value("itemsite_createsopo").toBool()))
      {
        if (_item->itemType() == "M")
        {
          if ( (_mode == cNew) || (_mode == cEdit) )
            _createSupplyOrder->setEnabled((_item->itemType() == "M"));

          _createSupplyOrder->setTitle(tr("C&reate Work Order"));
          _supplyOrderQtyLit->setText(tr("W/O Q&ty.:"));
          _supplyOrderDueDateLit->setText(tr("W/O Due Date:"));
          _supplyOrderStatusLit->setText(tr("W/O Status:"));
          if (_metrics->boolean("MultiWhs"))
          {
            _supplyWarehouseLit->show();
            _supplyWarehouse->show();
          }
          _supplyOverridePrice->hide();
          _supplyOverridePriceLit->hide();
          _supplyOrderLit->hide();
          _supplyOrder->hide();
          _supplyOrderLineLit->hide();
          _supplyOrderLine->hide();
        }
        else if (_item->itemType() == "P")
        {
          if ( (_mode == cNew) || (_mode == cEdit) )
            _createSupplyOrder->setEnabled(TRUE);

          _createSupplyOrder->setTitle(tr("C&reate Purchase Request"));
          _supplyOrderQtyLit->setText(tr("P/R Q&ty.:"));
          _supplyOrderDueDateLit->setText(tr("P/R Due Date:"));
          _supplyOrderStatusLit->setText(tr("P/R Status:"));
          _supplyWarehouseLit->hide();
          _supplyWarehouse->hide();
          _supplyOverridePrice->show();
          _supplyOverridePriceLit->show();
          _supplyOrderLit->hide();
          _supplyOrder->hide();
          _supplyOrderLineLit->hide();
          _supplyOrderLine->hide();
        }
        else
        {
          if ( (_mode == cNew) || (_mode == cEdit) )
            _createSupplyOrder->setEnabled(_item->itemType() != "K");

          _createSupplyOrder->setTitle(tr("C&reate Order"));
          _supplyOrderQtyLit->setText(tr("Order Q&ty.:"));
          _supplyOrderDueDateLit->setText(tr("Order Due Date:"));
          _supplyOrderStatusLit->setText(tr("Order Status:"));
          if (_metrics->boolean("MultiWhs"))
          {
            _supplyWarehouseLit->show();
            _supplyWarehouse->show();
          }
          _supplyOverridePrice->hide();
          _supplyOverridePriceLit->hide();
          _supplyOrderLit->hide();
          _supplyOrder->hide();
          _supplyOrderLineLit->hide();
          _supplyOrderLine->hide();
        }
      }
    }
    else if (salesPopulateItemInfo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, salesPopulateItemInfo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    _charVars.replace(ITEM_ID, _item->id());
    disconnect( _itemchar,  SIGNAL(itemChanged(QStandardItem *)), this, SLOT(sRecalcPrice()));
    disconnect( _itemchar,  SIGNAL(itemChanged(QStandardItem *)), this, SLOT(sRecalcAvailability()));

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

  if ((_item->isValid()) && (_scheduledDate->isValid()) && (_showAvailability->isChecked()) )
  {
    XSqlQuery availability;
    availability.prepare( "SELECT itemsite_id,"
                          "       qoh,"
                          "       allocated,"
                          "       (noNeg(qoh - allocated)) AS unallocated,"
                          "       ordered,"
                          "       (qoh - allocated + ordered) AS available,"
                          "       itemsite_leadtime "
                          "FROM ( SELECT itemsite_id, itemsite_qtyonhand AS qoh,"
                          "              qtyAllocated(itemsite_id, DATE(:date)) AS allocated,"
                          "              qtyOrdered(itemsite_id, DATE(:date)) AS ordered, "
                          "              itemsite_leadtime "
                          "       FROM itemsite, item "
                          "       WHERE ((itemsite_item_id=item_id)"
                          "        AND (item_id=:item_id)"
                          "        AND (itemsite_warehous_id=:warehous_id)) ) AS data;" );
    availability.bindValue(":date", _scheduledDate->date());
    availability.bindValue(":item_id", _item->id());
    availability.bindValue(":warehous_id", _warehouse->id());
    availability.exec();
    if (availability.first())
    {
      _onHand->setDouble(availability.value("qoh").toDouble());
      _allocated->setDouble(availability.value("allocated").toDouble());
      _unallocated->setDouble(availability.value("unallocated").toDouble());
      _onOrder->setDouble(availability.value("ordered").toDouble());
      _available->setDouble(availability.value("available").toDouble());
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
            "       qoh, "
            "       (totalalloc + pendalloc) AS totalalloc,"
            "       (qoh + ordered - (totalalloc + pendalloc)) AS totalavail,"
            "       'qty' AS pendalloc_xtnumericrole,"
            "       'qty' AS ordered_xtnumericrole,"
            "       'qty' AS qoh_xtnumericrole,"
            "       'qty' AS totalalloc_xtnumericrole,"
            "       'qty' AS totalavail_xtnumericrole,"
            "       CASE WHEN qoh < pendalloc THEN 'error'"
            "            WHEN (qoh + ordered - (totalalloc + pendalloc)) < 0  THEN 'error'"
            "            WHEN (qoh + ordered - (totalalloc + pendalloc)) < reorderlevel THEN 'warning'"
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
            "                itemsite_qtyonhand AS qoh,"
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
                      "       qoh, "
                      "       (totalalloc + pendalloc) AS totalalloc,"
                      "       (qoh + ordered - (totalalloc + pendalloc)) AS totalavail,"
                      "       'qty' AS pendalloc_xtnumericrole,"
                      "       'qty' AS ordered_xtnumericrole,"
                      "       'qty' AS qoh_xtnumericrole,"
                      "       'qty' AS totalalloc_xtnumericrole,"
                      "       'qty' AS totalavail_xtnumericrole,"
                      "       CASE WHEN qoh < pendalloc THEN 'error'"
                      "            WHEN (qoh + ordered - (totalalloc + pendalloc)) < 0  THEN 'error'"
                      "            WHEN (qoh + ordered - (totalalloc + pendalloc)) < reorderlevel THEN 'warning'"
                      "       END AS qtforegroundrole "
                      "FROM ( SELECT cs.itemsite_id AS itemsiteid,"
                      "              CASE WHEN(cs.itemsite_useparams) THEN cs.itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
                      "              bomitem_seqnumber, item_number,"
                      "              (item_descrip1 || ' ' || item_descrip2) AS item_descrip, uom_name,"
                      "              itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, (bomitem_qtyfxd + bomitem_qtyper * :qty) * (1 + bomitem_scrap)) AS pendalloc,"
                      "              (qtyAllocated(cs.itemsite_id, DATE(:schedDate)) - itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, (bomitem_qtyfxd + bomitem_qtyper * :origQtyOrd) * (1 + bomitem_scrap))) AS totalalloc,"
                      "              cs.itemsite_qtyonhand AS qoh,"
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

  if (_priceMode == "M")  // markup
  {
    _discountFromCustLit->setText(tr("Cust. Markup %:"));
    if (netUnitPrice == 0.0)
    {
      _discountFromListPrice->setDouble(100.0);
      _markupFromListCost->setDouble(100.0);
      _discountFromCust->setDouble(100.0);
    }
    else
    {
      if (_listPrice->isZero())
        _discountFromListPrice->setText(tr("N/A"));
      else
        _discountFromListPrice->setDouble((1.0 - (netUnitPrice / _listPrice->baseValue())) * 100.0);

      if (_listCost->isZero())
        _markupFromListCost->setText(tr("N/A"));
      else
        _markupFromListCost->setDouble(((netUnitPrice / _listCost->baseValue()) - 1.0) * 100.0);

      if (_customerPrice->isZero())
        _discountFromCust->setText(tr("N/A"));
      else
        _discountFromCust->setDouble(((netUnitPrice / _customerPrice->baseValue()) - 1.0) * 100.0);
    }
  }
  else  // discount
  {
    _discountFromCustLit->setText(tr("Cust. Discount %:"));
    if (netUnitPrice == 0.0)
    {
      _discountFromListPrice->setDouble(100.0);
      _markupFromListCost->setDouble(100.0);
      _discountFromCust->setDouble(100.0);
    }
    else
    {
      if (_listPrice->isZero())
        _discountFromListPrice->setText(tr("N/A"));
      else
        _discountFromListPrice->setDouble((1.0 - (netUnitPrice / _listPrice->baseValue())) * 100.0);

      if (_listCost->isZero())
        _markupFromListCost->setText(tr("N/A"));
      else
        _markupFromListCost->setDouble(((netUnitPrice / _listCost->baseValue()) - 1.0) * 100.0);

      if (_customerPrice->isZero())
        _discountFromCust->setText(tr("N/A"));
      else
        _discountFromCust->setDouble((1.0 - (netUnitPrice / _customerPrice->baseValue())) * 100.0);
    }
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

  sCalculateExtendedPrice();
}

void salesOrderItem::sCalculateExtendedPrice()
{
  _extendedPrice->setLocalValue(((_qtyOrdered->toDouble() * _qtyinvuomratio) / _priceinvuomratio) * _netUnitPrice->localValue());
}

void salesOrderItem::sHandleWo(bool pCreate)
{
  XSqlQuery salesHandleWo;
  if (pCreate)
  {
    if (_supplyOrderId == -1)
      sPopulateOrderInfo();
  }
  else
  {
    if (_supplyOrderId != -1)
    {
      XSqlQuery query;

      if (_item->itemType() == "M")
      {
        if (QMessageBox::question(this, tr("Delete Work Order"),
                                  tr("<p>You are requesting to delete the Work "
                                       "Order created for this Sales Order Item."
                                       "Are you sure you want to do this?"),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
        {
          query.prepare("SELECT deleteWo(:wo_id, TRUE) AS result;");
          query.bindValue(":wo_id", _supplyOrderId);
          query.exec();
          if (query.first())
          {
            int result = query.value("result").toInt();
            if (result < 0)
            {
              systemError(this, storedProcErrorLookup("deleteWo", result),
                          __FILE__, __LINE__);
              _createSupplyOrder->setChecked(true); // if (pCreate) => won't recurse
              return;
            }
          }
          else if (query.lastError().type() != QSqlError::NoError)
          {
              systemError(this, query.lastError().databaseText(),
                        __FILE__, __LINE__);
            _createSupplyOrder->setChecked(true); // if (pCreate) => won't recurse
            return;
          }

          omfgThis->sWorkOrdersUpdated(-1, TRUE);

          _supplyWarehouse->clear();
          _supplyWarehouse->findItemsites(_item->id());
          _supplyWarehouse->setId(_warehouse->id());
        }
      }
      else if (_item->itemType() == "P")
      {
        salesHandleWo.prepare("SELECT poitem_id "
                  "FROM poitem JOIN coitem ON (poitem_id = coitem_order_id) "
                  "WHERE ( (coitem_order_type = 'P') "
                  "  AND (coitem_id = :coitemid) );");
        salesHandleWo.bindValue(":coitemid", _soitemid);
        salesHandleWo.exec();
        if (salesHandleWo.first())
        {
          if (QMessageBox::question(this, tr("Delete Purchase Order Item"),
                                    tr("<p>You are requesting to delete the "
                                         "Purchase Order Item created for this Sales "
                                         "Order Item. The associated Purchase Order "
                                         "will also be deleted if no other Purchase "
                                         "Order Item exists for that Purchase Order. "
                                         "Are you sure you want to do this?"),
                                    QMessageBox::Yes | QMessageBox::Default,
                                    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
          {
            query.prepare("SELECT deletepoitem(:coitemid) AS result;");
            query.bindValue(":coitemid", _soitemid);
            query.exec();
            if (query.first())
            {
              bool result = query.value("result").toBool();
              if (!result)
              {
                systemError(this, tr("deletepoitem failed"), __FILE__, __LINE__);
                return;
              }
            }
            else if (query.lastError().type() != QSqlError::NoError)
            {
                systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
              return;
            }
          }
          else
          {
            _createSupplyOrder->setChecked(TRUE);
            return;
          }
        }
        else
        {
          if (QMessageBox::question(this, tr("Delete Purchase Request"),
                                    tr("<p>You are requesting to delete the "
                                         "Purchase Request created for this Sales "
                                         "Order Item. Are you sure you want to do "
                                         "this?"),
                                    QMessageBox::Yes | QMessageBox::Default,
                                    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
          {
            query.prepare("SELECT deletePr(:pr_id) AS result;");
            query.bindValue(":pr_id", _supplyOrderId);
            query.exec();
            if (query.first())
            {
              bool result = query.value("result").toBool();
              if (!result)
              {
                systemError(this, tr("deletePr failed"), __FILE__, __LINE__);
                return;
              }
            }
            else if (query.lastError().type() != QSqlError::NoError)
            {
                systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
              return;
            }
          }
          else
          {
            _createSupplyOrder->setChecked(TRUE);
            return;
          }
        }
      }
    }

    _supplyOrderId = -1;
    _supplyOrderQty->clear();
    _supplyOrderDueDate->clear();
    _supplyOrderStatus->clear();

    _createSupplyOrder->setChecked(FALSE);
  }
}

void salesOrderItem::sPopulateOrderInfo()
{
  if (_createSupplyOrder->isChecked() && ((_mode == cNew) || (_mode == cEdit) || (_mode == cView)))
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
      if (((checkpo.value("poitem_status").toString()) == "C") && (_qtyOrdered->toDouble() != _qtyOrderedCache))
      {
        QMessageBox::critical(this, tr("Cannot Update Item"),
                              tr("The Purchase Order Item this Sales Order Item is linked to is closed.  The quantity may not be updated."));
        _qtyOrdered->setDouble(_qtyOrderedCache);
        return;
      }
      else
      {
        _supplyOrderDueDate->setDate(_scheduledDate->date());

        if (_createSupplyOrder->isChecked())
        {
          XSqlQuery qty;
          qty.prepare( "SELECT validateOrderQty(itemsite_id, :qty, TRUE) AS qty "
                       "FROM itemsite "
                       "WHERE ((itemsite_item_id=:item_id)"
                       " AND (itemsite_warehous_id=:warehous_id));" );
          qty.bindValue(":qty", _qtyOrdered->toDouble() * _qtyinvuomratio);
          qty.bindValue(":item_id", _item->id());
          qty.bindValue(":warehous_id", (_item->itemType() == "M") ? _supplyWarehouse->id() : _warehouse->id());
          qty.exec();
          if (qty.first())
            _supplyOrderQty->setDouble(qty.value("qty").toDouble());

          else if (qty.lastError().type() != QSqlError::NoError)
          {
            systemError(this, qty.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
          return;
        }
      }
    }
    else if (checkpo.lastError().type() != QSqlError::NoError)
    {
            systemError(this, checkpo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    _supplyOrderDueDate->setDate(_scheduledDate->date());

    if (_createSupplyOrder->isChecked())
    {
      XSqlQuery qty;
      qty.prepare( "SELECT validateOrderQty(itemsite_id, :qty, TRUE) AS qty "
                   "FROM itemsite "
                   "WHERE ((itemsite_item_id=:item_id)"
                   " AND (itemsite_warehous_id=:warehous_id));" );
      qty.bindValue(":qty", _qtyOrdered->toDouble() * _qtyinvuomratio);
      qty.bindValue(":item_id", _item->id());
      qty.bindValue(":warehous_id", (_item->itemType() == "M") ? _supplyWarehouse->id() : _warehouse->id());
      qty.exec();
      if (qty.first())
        _supplyOrderQty->setDouble(qty.value("qty").toDouble());

      else if (qty.lastError().type() != QSqlError::NoError)
      {
            systemError(this, qty.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
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
      if (_priceMode == "M")  // markup
        _netUnitPrice->setLocalValue(_customerPrice->localValue() +
                                    (_customerPrice->localValue() * _discountFromCust->toDouble() / 100.0));
      else  // discount
        _netUnitPrice->setLocalValue(_customerPrice->localValue() -
                                    (_customerPrice->localValue() * _discountFromCust->toDouble() / 100.0));
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
          "       stdCost(item_id) AS stdcost,"
          "       coitem_status, coitem_cohead_id,"
          "       coitem_order_type, coitem_order_id, coitem_custpn,"
          "       coitem_memo, NULL AS quitem_createorder,"
          "       NULL AS quitem_order_warehous_id,"
          "       formatSoLineNumber(coitem_id) AS linenumber,"
          "       coitem_qtyord AS qtyord,"
          "       coitem_qty_uom_id AS qty_uom_id,"
          "       coitem_qty_invuomratio AS qty_invuomratio,"
          "       coitem_qtyshipped AS qtyshipped,"
          "       coitem_scheddate,"
          "       coitem_custprice,"
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
            "       stdcost(item_id) AS stdcost,"
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
            "       quitem_custprice AS coitem_custprice, "
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
    _unitCost->setBaseValue(item.value("stdcost").toDouble());
    // do tax stuff before _qtyOrdered so signal cascade has data to work with
    _taxtype->setId(item.value("coitem_taxtype_id").toInt());
    _supplyOrderId       = item.value("coitem_order_id").toInt();
    _orderNumber->setText(item.value("ordnumber").toString());
    _qtyOrderedCache = item.value("qtyord").toDouble();
    if(item.value("item_fractional") == true)
    {
        _qtyOrdered->setValidator(omfgThis->qtyVal());
        _qtyOrdered->setText(_qtyOrderedCache);
    }
    else
    {
        _qtyOrdered->setValidator(new QIntValidator());
        _qtyOrdered->setText(qRound(_qtyOrderedCache));
    }
    _dateCache     = item.value("coitem_scheddate").toDate();
    _scheduledDate->setDate(_dateCache);
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
    if (_unitCost->baseValue() > 0.0)
      _profit->setDouble((_customerPrice->baseValue() - _unitCost->baseValue()) / _unitCost->baseValue() * 100.0);
    else
      _profit->setDouble(100.0);
    _listPrice->setBaseValue(item.value("item_listprice").toDouble() * (_priceinvuomratio / _priceRatio));
    _netUnitPrice->setLocalValue(item.value("coitem_price").toDouble());
    _priceMode = item.value("coitem_pricemode").toString();
    _leadTime        = item.value("itemsite_leadtime").toInt();
    _qtyOrderedCache = _qtyOrdered->toDouble();
    _originalQtyOrd  = _qtyOrdered->toDouble() * _qtyinvuomratio;
    _cScheduledDate  = _scheduledDate->date();
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
    }

    _warranty->setChecked(item.value("coitem_warranty").toBool());
    _altCosAccnt->setId(item.value("coitem_cos_accnt_id").toInt());
    _altRevAccnt->setId(item.value("coitem_rev_accnt_id").toInt());
    _qtyreserved = item.value("coitem_qtyreserved").toDouble();

    sCalculateDiscountPrcnt();
    sLookupTax();
    sDetermineAvailability();
  }
  else if (item.lastError().type() != QSqlError::NoError)
  {
    systemError(this, item.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_supplyOrderId != -1)
  {
    XSqlQuery query;

    if (item.value("coitem_order_type").toString() == "W")
    {
      if (_metrics->boolean("MultiWhs"))
      {
        _supplyWarehouseLit->show();
        _supplyWarehouse->show();
      }
      _supplyOverridePrice->hide();
      _supplyOverridePriceLit->hide();
      query.prepare( "SELECT wo_status,"
                     "       wo_qtyord AS qty,"
                     "       wo_duedate, warehous_id, warehous_code "
                     "FROM wo, itemsite, whsinfo "
                     "WHERE ((wo_itemsite_id=itemsite_id)"
                     " AND (itemsite_warehous_id=warehous_id)"
                     " AND (wo_id=:wo_id));" );
      query.bindValue(":wo_id", _supplyOrderId);
      query.exec();
      if (query.first())
      {
        _createSupplyOrder->setChecked(TRUE);

        _supplyOrderQtyCache = query.value("qty").toDouble();
        _supplyOrderQty->setDouble(query.value("qty").toDouble());
        _supplyOrderDueDate->setDate(query.value("wo_duedate").toDate());
        _supplyOrderStatus->setText(query.value("wo_status").toString());

        if ((query.value("wo_status").toString() == "R") || (query.value("wo_status").toString() == "C"))
          _createSupplyOrder->setEnabled(FALSE);

        if (_item->isConfigured() && (query.value("wo_status").toString() != "O"))
          _itemcharView->setEnabled(FALSE);

        _supplyWarehouse->clear();
        _supplyWarehouse->append(query.value("warehous_id").toInt(), query.value("warehous_code").toString());
        _supplyWarehouse->setEnabled(FALSE);
      }
      else
      {
        _supplyOrderId = -1;
        _createSupplyOrder->setChecked(FALSE);
      }
    }
    else if (item.value("coitem_order_type").toString() == "P")
    {
      _createPO = true;
      _supplyWarehouseLit->hide();
      _supplyWarehouse->hide();
      _supplyOverridePrice->show();
      _supplyOverridePriceLit->show();

      XSqlQuery qry;
      qry.prepare("SELECT pohead_number, poitem_linenumber, poitem_status, "
                  "ROUND(poitem_qty_ordered, 2) AS poitem_qty_ordered, "
                  "poitem_duedate, ROUND(poitem_unitprice, 2) AS "
                  "poitem_unitprice, poitem_itemsrc_id, pohead_dropship "
                  "FROM pohead JOIN poitem ON (pohead_id = poitem_pohead_id) "
                  "WHERE (poitem_id = :poitem_id);");
      qry.bindValue(":poitem_id", _supplyOrderId);
      qry.exec();
      if (qry.first())
      {
        _createSupplyOrder->setTitle(tr("Create Purchase Order"));
        _supplyOrderLit->setText(tr("PO #:"));
        _supplyOrderLineLit->setText(tr("PO Line #:"));
        _supplyOrderQtyLit->setText(tr("PO Q&ty.:"));
        _supplyOrderDueDateLit->setText(tr("PO Due Date:"));
        _supplyOrderStatusLit->setText(tr("PO Status:"));
        _supplyOrder->setText(qry.value("pohead_number").toString());
        _supplyOrderLine->setText(qry.value("poitem_linenumber").toString());
        _supplyOrderStatus->setText(qry.value("poitem_status").toString());
        _supplyOrderQtyCache = query.value("qty").toDouble();
        _supplyOrderQty->setDouble(qry.value("poitem_qty_ordered").toDouble());
        _supplyOrderDueDate->setDate(qry.value("poitem_duedate").toDate());
        _supplyDropShip->setChecked(qry.value("pohead_dropship").toBool());
        _supplyOverridePrice->setLocalValue(qry.value("poitem_unitprice").toDouble());
        _itemsrc = qry.value("poitem_itemsrc_id").toInt();

        _supplyOrderLit->show();
        _supplyOrderLineLit->show();
        _supplyOrderQtyLit->show();
        _supplyOrderDueDateLit->show();
        _supplyOrderStatusLit->show();
        _supplyOrder->show();
        _supplyOrderLine->show();
        _supplyOrderStatus->show();
        _supplyOrderQty->show();
        _supplyOrderDueDate->show();
        _supplyOverridePrice->show();
        _supplyDropShip->setVisible(_metrics->boolean("EnableDropShipments"));

        _createSupplyOrder->setChecked(TRUE);
        _createSupplyOrder->setEnabled(FALSE);
      }
    }
    else if (item.value("coitem_order_type").toString() == "R")
    {
      _createPR = true;
      _supplyWarehouseLit->hide();
      _supplyWarehouse->hide();
      _supplyOverridePrice->show();
      _supplyOverridePriceLit->show();

      query.prepare( "SELECT pr_status,"
                     "       pr_qtyreq AS qty,"
                     "       pr_duedate "
                     "FROM pr "
                     "WHERE (pr_id=:pr_id);" );
      query.bindValue(":pr_id", _supplyOrderId);
      query.exec();
      if (query.first())
      {
        _createSupplyOrder->setChecked(TRUE);

        _supplyOrderQtyCache = query.value("qty").toDouble();
        _supplyOrderQty->setDouble(query.value("qty").toDouble());
        _supplyOrderDueDate->setDate(query.value("pr_duedate").toDate());
        _supplyOrderStatus->setText(query.value("pr_status").toString());

        if ((query.value("pr_status").toString() == "R") || (query.value("pr_status").toString() == "C"))
          _createSupplyOrder->setEnabled(FALSE);
      }
      else
      {
        _supplyOrderId = -1;
        _createSupplyOrder->setChecked(FALSE);
      }
    }
  }
  else if (ISORDER(_mode))
    _createSupplyOrder->setChecked(FALSE);

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
        sSave();
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
        sSave();
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
        sSave();
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
      salesreject.prepare("DELETE FROM comment WHERE comment_source_id=:coitem_id AND comment_source = 'QI';");
    else
      salesreject.prepare("DELETE FROM comment WHERE comment_source_id=:coitem_id AND comment_source = 'SI';");
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

  sSave();
  if (_error)
    return;

  if ( (_mode == cEdit) || (_mode == cNew) )
  {
    XSqlQuery existpo;
    existpo.prepare("SELECT * FROM poitem JOIN coitem ON (coitem_order_id = poitem_id) "
                    "WHERE ((coitem_id = :soitem_id) "
                    "  AND  (coitem_order_type='P'));" );
    existpo.bindValue(":soitem_id", _soitemid);
    existpo.exec();
    if (existpo.first())
    {
      QMessageBox::warning(this, tr("Can not delete PO"),
                           tr("Purchase Order linked to this Sales Order "
                              "Item will not be affected. The Purchase Order "
                              "should be closed or deleted manually if necessary."));
    }
    else if (existpo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, existpo.lastError().databaseText(), __FILE__, __LINE__);
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
      _qtyOrdered->setValidator(new XDoubleValidator(this));
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
        _qtyOrdered->setValidator(new XDoubleValidator(this));
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
  sCalculateExtendedPrice();
  sPopulateOrderInfo();

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
  item.prepare("SELECT item_listprice"
               "  FROM item"
               " WHERE(item_id=:item_id);");
  item.bindValue(":item_id", _item->id());
  item.exec();
  item.first();
  _listPrice->setBaseValue(item.value("item_listprice").toDouble() * (_priceinvuomratio / _priceRatio));
  sDeterminePrice(true);
}

void salesOrderItem::sCalcWoUnitCost()
{
  XSqlQuery salesCalcWoUnitCost;
  if (_costmethod == "J" && _supplyOrderId > -1 && _qtyOrdered->toDouble() != 0)
  {
    salesCalcWoUnitCost.prepare("SELECT COALESCE(SUM(wo_postedvalue),0) AS wo_value "
              "FROM wo "
              "WHERE ((wo_ordtype='S') "
              "AND (wo_ordid=:soitem_id));");
    salesCalcWoUnitCost.bindValue(":soitem_id", _soitemid);
    salesCalcWoUnitCost.exec();
    if (salesCalcWoUnitCost.first())
      _unitCost->setBaseValue(salesCalcWoUnitCost.value("wo_value").toDouble() / _qtyOrdered->toDouble() * _qtyinvuomratio);
  }
}

void salesOrderItem::sHandleButton()
{
  if (_inventoryButton->isChecked())
    _availabilityStack->setCurrentWidget(_inventoryPage);
  else if (_itemSourcesButton->isChecked())
    _availabilityStack->setCurrentWidget(_itemSourcesPage);
  else
    _availabilityStack->setCurrentWidget(_dependenciesPage);

  if (_historyCostsButton->isChecked())
    _historyStack->setCurrentWidget(_historyCostsPage);
  else if (_historySalesButton->isChecked())
    _historyStack->setCurrentWidget(_historySalesPage);
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
  if ((_metrics->value("soPriceEffective") != "ScheduleDate") ||
      (!_scheduledDate->isValid() ||
      (_scheduledDate->date() == _dateCache)))
  {
    sDetermineAvailability();
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
        _scheduledDate->setDate(_dateCache);
        return;
      }
    }
  }

  // Check effectivity
  setItemExtraClause();
  if (_item->isValid())
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
        _scheduledDate->setDate(_dateCache);
        _scheduledDate->setFocus();
      }
    }
  }

  sDeterminePrice();
  sDetermineAvailability();
  sPopulateOrderInfo();
}
