/*
 * Common Public Attribution License Version q1.0. 
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
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "salesOrderItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QValidator>
#include <QVariant>

#include <metasql.h>

#include "itemCharacteristicDelegate.h"
#include "priceList.h"
#include "storedProcErrorLookup.h"
#include "taxDetail.h"

#define cNewQuote  (0x20 | cNew)
#define cEditQuote (0x20 | cEdit)
#define cViewQuote (0x20 | cView)

#define ISQUOTE(mode) (((mode) & 0x20) == 0x20)
#define ISORDER(mode) (! ISQUOTE(mode))

#define iDontUpdate 1
#define iAskToUpdate 2
#define iJustUpdate 3

salesOrderItem::salesOrderItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(privateIdChanged(int)), this, SLOT(sFindSellingWarehouseItemsites(int)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateItemInfo(int)));
  connect(_listPrices, SIGNAL(clicked()), this, SLOT(sListPrices()));
  connect(_netUnitPrice, SIGNAL(idChanged(int)), this, SLOT(sPriceGroup()));
  connect(_netUnitPrice, SIGNAL(valueChanged()), this, SLOT(sCalculateExtendedPrice()));
  connect(_qtyOrdered, SIGNAL(lostFocus()), this, SLOT(sPopulateOrderInfo()));
  connect(_qtyOrdered, SIGNAL(lostFocus()), this, SLOT(sDetermineAvailability()));
  connect(_qtyOrdered, SIGNAL(lostFocus()), this, SLOT(sDeterminePrice()));
  connect(_qtyOrdered, SIGNAL(textChanged(const QString&)), this, SLOT(sCalcWoUnitCost()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_scheduledDate, SIGNAL(newDate(const QDate&)), this, SLOT(sDetermineAvailability()));
  connect(_scheduledDate, SIGNAL(newDate(const QDate&)), this, SLOT(sPopulateOrderInfo()));
  connect(_showAvailability, SIGNAL(toggled(bool)), this, SLOT(sDetermineAvailability()));
  connect(_showIndented, SIGNAL(toggled(bool)), this, SLOT(sDetermineAvailability()));
  connect(_item, SIGNAL(privateIdChanged(int)), this, SLOT(sPopulateItemsiteInfo()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateItemsiteInfo()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sDetermineAvailability()));
  connect(_next, SIGNAL(clicked()), this, SLOT(sNext()));
  connect(_prev, SIGNAL(clicked()), this, SLOT(sPrev()));
  connect(_notes, SIGNAL(textChanged()), this, SLOT(sChanged()));
  connect(_createOrder, SIGNAL(toggled(bool)), this, SLOT(sChanged()));
  connect(_orderQty, SIGNAL(textChanged(const QString&)), this, SLOT(sChanged()));
  connect(_orderQty, SIGNAL(textChanged(const QString&)), this, SLOT(sCalcWoUnitCost()));
  connect(_orderDueDate, SIGNAL(newDate(const QDate&)), this, SLOT(sChanged()));
  connect(_supplyWarehouse, SIGNAL(newID(int)), this, SLOT(sChanged()));
  connect(_discountFromCust, SIGNAL(textChanged(const QString&)), this, SLOT(sChanged()));
  connect(_promisedDate, SIGNAL(newDate(const QDate&)), this, SLOT(sChanged()));
  connect(_scheduledDate, SIGNAL(newDate(const QDate&)), this, SLOT(sChanged()));
  connect(_netUnitPrice, SIGNAL(valueChanged()), this, SLOT(sChanged()));
  connect(_qtyOrdered, SIGNAL(textChanged(const QString&)), this, SLOT(sChanged()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sChanged()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sChanged()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(sCancel()));
  connect(_extendedPrice,   SIGNAL(valueChanged()), this, SLOT(sLookupTax()));
  connect(_taxLit, SIGNAL(leftClickedURL(QString)), this, SLOT(sTaxDetail()));
  connect(_taxcode,		SIGNAL(newID(int)), this, SLOT(sLookupTax()));
  connect(_taxtype,		SIGNAL(newID(int)), this, SLOT(sLookupTaxCode()));
  connect(_qtyUOM, SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
  connect(_priceUOM, SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));

#ifndef Q_WS_MAC
  _listPrices->setMaximumWidth(25);
  _subItemList->setMaximumWidth(25);
#endif

  _leadTime = 999;
  _shiptoid = -1;
  _preferredWarehouseid = -1;
  _modified = false;
  _canceling = false;
  _error = false;
  _originalQtyOrd = 0.0;
  _updateItemsite = false;
  _qtyinvuomratio = 1.0;
  _priceinvuomratio = 1.0;
  _priceRatio = 1.0;
  _invuomid = -1;

  _authNumber->hide();
  _authNumberLit->hide();
  _authLineNumber->hide();
  _authLineNumberLit->hide();

  _availabilityLastItemid = -1;
  _availabilityLastWarehousid = -1;
  _availabilityLastSchedDate = QDate();
  _availabilityLastShow = false;
  _availabilityQtyOrdered = 0.0;

//  Configure some Widgets
  _item->setType(ItemLineEdit::cSold | ItemLineEdit::cActive);
  _item->addExtraClause( QString("(itemsite_active)") ); // ItemLineEdit::cActive doesn't compare against the itemsite record
  _item->addExtraClause( QString("(itemsite_sold)") );   // ItemLineEdit::cSold doesn't compare against the itemsite record
  _discountFromCust->setValidator(new QDoubleValidator(-9999, 100, 2, this));

  _taxtype->setEnabled(_privleges->check("OverrideTax"));
  _taxcode->setEnabled(_privleges->check("OverrideTax"));

  _availability->addColumn(tr("#"),            _seqColumn,  Qt::AlignCenter );
  _availability->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft   );
  _availability->addColumn(tr("Description"),  -1,          Qt::AlignLeft   );
  _availability->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter );
  _availability->addColumn(tr("Pend. Alloc."), _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Total Alloc."), _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("On Order"),      _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Availability"), _qtyColumn,  Qt::AlignRight  );
  
  _itemchar = new QStandardItemModel(0, 2, this);
  _itemchar->setHeaderData( 0, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
  _itemchar->setHeaderData( 1, Qt::Horizontal, tr("Value"), Qt::DisplayRole);

  _itemcharView->setModel(_itemchar);
  ItemCharacteristicDelegate * delegate = new ItemCharacteristicDelegate(this);
  _itemcharView->setItemDelegate(delegate);

  if (!_metrics->boolean("UsePromiseDate"))
  {
    _promisedDateLit->hide();
    _promisedDate->hide();
  }

  _showAvailability->setChecked(_preferences->boolean("ShowSOItemAvailability"));

  _qtyOrdered->setValidator(omfgThis->qtyVal());
  _orderQty->setValidator(omfgThis->qtyVal());

//  Disable the Discount Percent stuff if we don't allow them
  if ((!_metrics->boolean("AllowDiscounts")) && (!_privleges->check("OverridePrice")))
  {
    _netUnitPrice->setEnabled(FALSE);
    _discountFromCust->setEnabled(FALSE);
  }

  if (_metrics->boolean("DisableSalesOrderPriceOverride"))
    _netUnitPrice->setEnabled(false);

  _overridePoPrice->hide();
  _overridePoPriceLit->hide();

  _taxauthid = -1;

  sPriceGroup();

  if (!_metrics->boolean("EnableReturnAuth"))
    _warranty->hide();

  resize(minimumSize());
  
  //TO DO **** Fix tab order issues and offer alternate means for "Express Tab Order"  ****
}

salesOrderItem::~salesOrderItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesOrderItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesOrderItem::set(const ParameterList &pParams)
{
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

  param = pParams.value("taxauth_id", &valid);
  if (valid)
    _taxauthid = param.toInt();

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _custid = param.toInt();
    q.prepare("SELECT cust_preferred_warehous_id, "
	      "(cust_number || '-' || cust_name) as f_name "
              "  FROM custinfo"
              " WHERE (cust_id=:cust_id); ");
    q.bindValue(":cust_id", _custid);
    q.exec();
    if(q.first())
    {
      _preferredWarehouseid = q.value("cust_preferred_warehous_id").toInt();
      _custName = q.value("f_name").toString();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("shipto_id", &valid);
  if (valid)
    _shiptoid = param.toInt();

  param = pParams.value("orderNumber", &valid);
  if (valid)
    _orderNumber->setText(param.toString());

  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _netUnitPrice->setId(param.toInt());
	_tax->setId(param.toInt());
  }

  param = pParams.value("orderDate", &valid);
  if (valid)
    _netUnitPrice->setEffective(param.toDate());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _save->setEnabled(FALSE);
      _next->setText(tr("New"));
      _comments->setType(Comments::SalesOrderItem);
      _comments->setEnabled(FALSE);
      _orderStatusLit->hide();
      _orderStatus->hide();
      _item->setReadOnly(false);
      _warehouse->setEnabled(true);
      _item->setFocus();
      _orderId= -1;

      _item->addExtraClause( QString("(NOT item_exclusive OR customerCanPurchase(item_id, %1, %2))").arg(_custid).arg(_shiptoid) );

      prepare();

      connect(_netUnitPrice, SIGNAL(lostFocus()), this, SLOT(sCalculateDiscountPrcnt()));
      connect(_discountFromCust, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
      connect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));
      connect(_createOrder, SIGNAL(toggled(bool)), this, SLOT(sHandleWo(bool)));

      q.prepare("SELECT count(*) AS cnt"
                "  FROM coitem"
                " WHERE (coitem_cohead_id=:sohead_id);");
      q.bindValue(":sohead_id", _soheadid);
      q.exec();
      if(!q.first() || q.value("cnt").toInt() == 0)
        _prev->setEnabled(false);
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    else if (param.toString() == "newQuote")
    {
      _mode = cNewQuote;
      _item->setType(ItemLineEdit::cSold | ItemLineEdit::cItemActive);
      _item->clearExtraClauseList();

      setCaption(tr("Quote Item"));

      _save->setEnabled(FALSE);
      _next->setText(tr("New"));
      _comments->setType(Comments::QuoteItem);
      _comments->setEnabled(FALSE);
      _orderStatusLit->hide();
      _orderStatus->hide();
      //_createOrder->hide();
      _orderQtyLit->hide();
      _orderQty->hide();
      _orderDueDateLit->hide();
      _orderDueDate->hide();
      _cancel->hide();
      _sub->hide();
      _subItem->hide();
      _subItemList->hide();
      _item->setReadOnly(false);
      _warehouse->setEnabled(true);
      _item->setFocus();
      _orderId = -1;
	  _warranty->hide();
      _tabs->removePage(_tabs->page(6));

      _item->addExtraClause( QString("(NOT item_exclusive OR customerCanPurchase(item_id, %1, %2))").arg(_custid).arg(_shiptoid) );

      prepare();

      connect(_netUnitPrice, SIGNAL(lostFocus()), this, SLOT(sCalculateDiscountPrcnt()));
      connect(_discountFromCust, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
      connect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));

      q.prepare("SELECT count(*) AS cnt"
                "  FROM quitem"
                " WHERE (quitem_quhead_id=:sohead_id);");
      q.bindValue(":sohead_id", _soheadid);
      q.exec();
      if(!q.first() || q.value("cnt").toInt() == 0)
        _prev->setEnabled(false);
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _item->setReadOnly(TRUE);
      _listPrices->setEnabled(TRUE);
      _comments->setType(Comments::SalesOrderItem);
      _qtyOrdered->setFocus();

      connect(_qtyOrdered, SIGNAL(lostFocus()), this, SLOT(sCalculateExtendedPrice()));
      connect(_netUnitPrice, SIGNAL(lostFocus()), this, SLOT(sCalculateDiscountPrcnt()));
      connect(_discountFromCust, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
      connect(_createOrder, SIGNAL(toggled(bool)), this, SLOT(sHandleWo(bool)));

      _save->setFocus();
    }
    else if (param.toString() == "editQuote")
    {
      _mode = cEditQuote;
      _item->setType(ItemLineEdit::cSold | ItemLineEdit::cItemActive);
      _item->clearExtraClauseList();

      setCaption(tr("Quote Item"));

      _item->setReadOnly(TRUE);
      _listPrices->setEnabled(TRUE);
      _comments->setType(Comments::QuoteItem);
      //_createOrder->hide();
      _orderQtyLit->hide();
      _orderQty->hide();
      _orderDueDateLit->hide();
      _orderDueDate->hide();
      _orderStatusLit->hide();
      _orderStatus->hide();
      _cancel->hide();
      _sub->hide();
      _subItem->hide();
      _subItemList->hide();
      _qtyOrdered->setFocus();
	  _warranty->hide();
      _tabs->removePage(_tabs->page(6));

      connect(_qtyOrdered, SIGNAL(lostFocus()), this, SLOT(sCalculateExtendedPrice()));
      connect(_netUnitPrice, SIGNAL(lostFocus()), this, SLOT(sCalculateDiscountPrcnt()));
      connect(_discountFromCust, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _comments->setType(Comments::SalesOrderItem);
      _sub->setEnabled(false);
      _subItem->setEnabled(false);
      _supplyWarehouse->setEnabled(FALSE);
      _overridePoPrice->setEnabled(FALSE);
    }
    else if (param.toString() == "viewQuote")
    {
      _mode = cViewQuote;
      _item->setType(ItemLineEdit::cSold | ItemLineEdit::cItemActive);
      _item->clearExtraClauseList();

      setCaption(tr("Quote Item"));

      //_createOrder->hide();
      _orderQtyLit->hide();
      _orderQty->hide();
      _orderDueDateLit->hide();
      _orderDueDate->hide();
      _orderStatusLit->hide();
      _orderStatus->hide();
      _cancel->hide();
      _sub->hide();
      _subItem->hide();
      _comments->setType(Comments::QuoteItem);
	  _warranty->hide();
      _tabs->removePage(_tabs->page(6));
    }
  }

  if(cView == _mode || cViewQuote == _mode)
  {
    _item->setReadOnly(true);
    _qtyOrdered->setEnabled(false);
    _netUnitPrice->setEnabled(false);
    _discountFromCust->setEnabled(false);
    _scheduledDate->setEnabled(false);
    _createOrder->setEnabled(false);
    _notes->setEnabled(false);
    _comments->setReadOnly(true);
    _taxtype->setEnabled(false);
    _taxcode->setEnabled(false);
    _itemcharView->setEnabled(false);
    _promisedDate->setEnabled(false);

    _subItemList->hide();
    _save->hide();

    _close->setFocus();
  }

  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _soitemid = param.toInt();
    populate();

    if (ISQUOTE(_mode))
      q.prepare("SELECT a.quitem_id AS id"
                "  FROM quitem AS a, quitem as b"
                " WHERE ((a.quitem_quhead_id=b.quitem_quhead_id)"
                "   AND  (b.quitem_id=:id))"
                " ORDER BY a.quitem_linenumber "
                " LIMIT 1;");
    else
      q.prepare("SELECT a.coitem_id AS id"
                "  FROM coitem AS a, coitem AS b"
                " WHERE ((a.coitem_cohead_id=b.coitem_cohead_id)"
                "   AND  (b.coitem_id=:id))"
                " ORDER BY a.coitem_linenumber "
                " LIMIT 1;");
    q.bindValue(":id", _soitemid);
    q.exec();
    if(!q.first() || q.value("id").toInt() == _soitemid)
      _prev->setEnabled(false);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    if (ISQUOTE(_mode))
      q.prepare("SELECT a.quitem_id AS id"
                "  FROM quitem AS a, quitem as b"
                " WHERE ((a.quitem_quhead_id=b.quitem_quhead_id)"
                "   AND  (b.quitem_id=:id))"
                " ORDER BY a.quitem_linenumber DESC"
                " LIMIT 1;");
    else
      q.prepare("SELECT a.coitem_id AS id"
                "  FROM coitem AS a, coitem AS b"
                " WHERE ((a.coitem_cohead_id=b.coitem_cohead_id)"
                "   AND  (b.coitem_id=:id))"
                " ORDER BY a.coitem_linenumber DESC"
                " LIMIT 1;");
    q.bindValue(":id", _soitemid);
    q.exec();
    if(q.first() && q.value("id").toInt() == _soitemid)
    {
      if(cView == _mode || cViewQuote == _mode)
        _next->setEnabled(false);
      else
        _next->setText(tr("New"));
    }
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
	  
	//See if this is linked to a Return Authorization and handle
    if ((ISORDER(_mode)) && (_metrics->boolean("EnableReturnAuth")))
	{
      q.prepare("SELECT rahead_number,raitem_linenumber " 
			    "FROM raitem,rahead "
				"WHERE ((raitem_new_coitem_id=:coitem_id) "
				"AND (rahead_id=raitem_rahead_id));");
	  q.bindValue(":coitem_id",_soitemid);
      q.exec();
	  if (q.first())
	  {
        _authNumber->show();
        _authNumberLit->show();
        _authLineNumber->show();
        _authLineNumberLit->show();
		_authNumber->setText(q.value("rahead_number").toString());
		_authLineNumber->setText(q.value("raitem_linenumber").toString());
		_netUnitPrice->setEnabled(FALSE);
		_qtyOrdered->setEnabled(FALSE);
		_discountFromCust->setEnabled(FALSE);
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

  //If not multi-warehouse and a sales order hide whs control
  //Leave the warehouse controls available on Quotes as it is
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
  if (_mode == cNew)
  {
    q.prepare( "SELECT (COALESCE(MAX(coitem_linenumber), 0) + 1) AS _linenumber "
               "FROM coitem "
               "WHERE (coitem_cohead_id=:coitem_id)" );
    q.bindValue(":coitem_id", _soheadid);
    q.exec();
    if (q.first())
      _lineNumber->setText(q.value("_linenumber").toString());
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT MIN(coitem_scheddate) AS scheddate "
               "FROM coitem "
               "WHERE (coitem_cohead_id=:cohead_id);" );
    q.bindValue(":cohead_id", _soheadid);
    q.exec();
    if (q.first())
      _scheduledDate->setDate(q.value("scheddate").toDate());
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (_mode == cNewQuote)
  {
    q.prepare( "SELECT (COALESCE(MAX(quitem_linenumber), 0) + 1) AS n_linenumber "
               "FROM quitem "
               "WHERE (quitem_quhead_id=:quhead_id)" );
    q.bindValue(":quhead_id", _soheadid);
    q.exec();
    if (q.first())
      _lineNumber->setText(q.value("n_linenumber").toString());
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT MIN(quitem_scheddate) AS scheddate "
               "FROM quitem "
               "WHERE (quitem_quhead_id=:quhead_id);" );
    q.bindValue(":quhead_id", _soheadid);
    q.exec();
    if (q.first())
      _scheduledDate->setDate(q.value("scheddate").toDate());
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
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
  _scheduledDate->clear();
  _promisedDate->clear();
  _unitCost->clear();
  _listPrice->clear();
  _customerPrice->clear();
  _discountFromList->clear();
  _discountFromCust->clear();
  _shippedToDate->clear();
  _createOrder->setChecked(FALSE);
  _orderQty->clear();
  _orderDueDate->clear();
  _orderStatus->clear();
  _onHand->clear();
  _allocated->clear();
  _unallocated->clear();
  _onOrder->clear();
  _available->clear();
  _leadtime->clear();
  _itemchar->removeRows(0, _itemchar->rowCount());
  _notes->clear();
  _sub->setChecked(false);
  _subItem->clear();
  _subItem->setEnabled(false);
  _subItemList->setEnabled(false);
  _comments->setId(-1);
  _warehouse->clear();	// are these two _warehouse steps necessary?
  _warehouse->setType(WComboBox::Sold);
  _overridePoPrice->clear();
  _originalQtyOrd = 0.0;
  _modified = false;
  _updateItemsite = false;
}

void salesOrderItem::sSave()
{
  _save->setFocus();

  _error = true;
  if (!(_qtyOrdered->toDouble() > 0))
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order Item"),
                          tr("<p>You must enter a valid Quantity Ordered before saving this Sales Order Item.")  );
    _qtyOrdered->setFocus();
    return;
  }

  if (_netUnitPrice->isEmpty())
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order Item"),
                          tr("<p>You must enter a Price before saving this Sales Order Item.") );
    _netUnitPrice->setFocus();
    return;
  }

  if(_metrics->boolean("AllowASAPShipSchedules") && !_scheduledDate->isValid())
    _scheduledDate->setDate(QDate::currentDate());
  if (!(_scheduledDate->isValid()))
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order Item"),
                          tr("<p>You must enter a valid Schedule Date before saving this Sales Order Item.") );
    _scheduledDate->setFocus();
    return;
  }

  if(_createOrder->isChecked() && ((_item->itemType() == "M") || (_item->itemType() == "J")) && _supplyWarehouse->id() == -1)
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order Item"),
                          tr("<p>Before an Order may be created, a valid Supplied at Warehouse must be selected.") );
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

  if (_mode == cNew)
  {
//  Grab the next coitem_id
    q.exec("SELECT NEXTVAL('coitem_coitem_id_seq') AS _coitem_id");
    if (q.first())
      _soitemid = q.value("_coitem_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      reject();
      return;
    }

    q.prepare( "INSERT INTO coitem "
               "( coitem_id, coitem_cohead_id, coitem_linenumber, coitem_itemsite_id,"
               "  coitem_status, coitem_scheddate, coitem_promdate,"
               "  coitem_qtyord, coitem_qty_uom_id, coitem_qty_invuomratio,"
               "  coitem_qtyshipped, coitem_qtyreturned,"
               "  coitem_unitcost, coitem_custprice,"
               "  coitem_price, coitem_price_uom_id, coitem_price_invuomratio,"
               "  coitem_order_type, coitem_order_id,"
               "  coitem_custpn, coitem_memo, coitem_substitute_item_id,"
	           "  coitem_prcost, coitem_tax_id, coitem_cos_accnt_id, coitem_warranty ) "
               "SELECT :soitem_id, :soitem_sohead_id, :soitem_linenumber, itemsite_id,"
               "       'O', :soitem_scheddate, :soitem_promdate,"
               "       :soitem_qtyord, :qty_uom_id, :qty_invuomratio, 0, 0,"
               "       stdCost(:item_id), :soitem_custprice,"
               "       :soitem_price, :price_uom_id, :price_invuomratio,"
               "       '', -1,"
               "       :soitem_custpn, :soitem_memo, :soitem_substitute_item_id,"
			   "       :soitem_prcost, :soitem_tax_id, :soitem_cost_accnt_id, :soitem_warranty "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":soitem_id", _soitemid);
    q.bindValue(":soitem_sohead_id", _soheadid);
    q.bindValue(":soitem_linenumber", _lineNumber->text().toInt());
    q.bindValue(":soitem_scheddate", _scheduledDate->date());
    q.bindValue(":soitem_promdate", promiseDate);
    q.bindValue(":soitem_qtyord", _qtyOrdered->toDouble());
    q.bindValue(":qty_uom_id", _qtyUOM->id());
    q.bindValue(":qty_invuomratio", _qtyinvuomratio);
    q.bindValue(":soitem_custprice", _customerPrice->localValue());
    q.bindValue(":soitem_price", _netUnitPrice->localValue());
    q.bindValue(":price_uom_id", _priceUOM->id());
    q.bindValue(":price_invuomratio", _priceinvuomratio);
    q.bindValue(":soitem_prcost", _overridePoPrice->localValue());
    q.bindValue(":soitem_custpn", _customerPN->text());
    q.bindValue(":soitem_memo", _notes->text());
    q.bindValue(":item_id", _item->id());
    if(_sub->isChecked())
      q.bindValue(":soitem_substitute_item_id", _subItem->id());
    q.bindValue(":warehous_id", _warehouse->id());
    if (_taxcode->isValid())
      q.bindValue(":soitem_tax_id", _taxcode->id());
	if (_altCosAccnt->isValid())
	  q.bindValue(":soitem_cos_accnt_id", _altCosAccnt->id());
	q.bindValue(":soitem_warranty",QVariant(_warranty->isChecked(), 0));
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (_mode == cEdit)
  {
    q.prepare( "UPDATE coitem "
               "SET coitem_scheddate=:soitem_scheddate, coitem_promdate=:soitem_promdate,"
               "    coitem_qtyord=:soitem_qtyord, coitem_qty_uom_id=:qty_uom_id, coitem_qty_invuomratio=:qty_invuomratio,"
               "    coitem_price=:soitem_price, coitem_price_uom_id=:price_uom_id, coitem_price_invuomratio=:price_invuomratio,"
               "    coitem_memo=:soitem_memo,"
               "    coitem_order_type=:soitem_order_type,"
               "    coitem_order_id=:soitem_order_id, coitem_substitute_item_id=:soitem_substitute_item_id,"
               "    coitem_prcost=:soitem_prcost,"
               "    coitem_tax_id=:soitem_tax_id, "
			   "    coitem_cos_accnt_id=:soitem_cos_accnt_id, "
			   "    coitem_warranty=:soitem_warranty "
               "WHERE (coitem_id=:soitem_id);" );
    q.bindValue(":soitem_scheddate", _scheduledDate->date());
    q.bindValue(":soitem_promdate", promiseDate);
    q.bindValue(":soitem_qtyord", _qtyOrdered->toDouble());
    q.bindValue(":qty_uom_id", _qtyUOM->id());
    q.bindValue(":qty_invuomratio", _qtyinvuomratio);
    q.bindValue(":soitem_price", _netUnitPrice->localValue());
    q.bindValue(":price_uom_id", _priceUOM->id());
    q.bindValue(":price_invuomratio", _priceinvuomratio);
    q.bindValue(":soitem_prcost", _overridePoPrice->localValue());
    q.bindValue(":soitem_memo", _notes->text());
    if(_orderId != -1)
    {
      if ((_item->itemType() == "M") || (_item->itemType() == "J"))
        q.bindValue(":soitem_order_type", "W");
      else if (_item->itemType() == "P")
        q.bindValue(":soitem_order_type", "R");
    }
    q.bindValue(":soitem_order_id", _orderId);
    q.bindValue(":soitem_id", _soitemid);
    if(_sub->isChecked())
      q.bindValue(":soitem_substitute_item_id", _subItem->id());
    if (_taxcode->isValid())
      q.bindValue(":soitem_tax_id", _taxcode->id());
	if (_altCosAccnt->isValid())
	  q.bindValue(":soitem_cos_accnt_id", _altCosAccnt->id());
	q.bindValue(":soitem_warranty",QVariant(_warranty->isChecked(), 0));

    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

//  Check to see if a S/O should be re-scheduled
    if (_orderId != -1)
    {
      if (_scheduledDate->date() != _cScheduledDate)
      {
        if (QMessageBox::question(this, tr("Reschedule W/O?"),
				  tr("<p>The Scheduled Date for this Line "
				      "Item has been changed.  Should the "
				      "W/O for this Line Item be Re-"
				      "Scheduled to reflect this change?"),
				     QMessageBox::Yes | QMessageBox::Default,
				     QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
        {
          q.prepare("SELECT changeWoDates(:wo_id, :schedDate, :schedDate, TRUE) AS result;");
          q.bindValue(":wo_id", _orderId);
          q.bindValue(":schedDate", _scheduledDate->date());
          q.exec();
	  if (q.first())
	  {
	    int result = q.value("result").toInt();
	    if (result < 0)
	    {
	      systemError(this, storedProcErrorLookup("changeWoDates", result),
			  __FILE__, __LINE__);
	      return;
	    }
	  }
	  else if (q.lastError().type() != QSqlError::None)
	  {
	    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	    return;
	  }
        }
      }

      if (_qtyOrdered->toDouble() != _cQtyOrdered)
      {
        if((_item->itemType() == "M") || (_item->itemType() == "J"))
        {
          if (QMessageBox::question(this, tr("Change W/O Quantity?"),
				    tr("<p>The quantity ordered for this Sales "
				       "Order Line Item has been changed. "
				       "Should the quantity required for the "
				       "associated Work Order be changed to "
				       "reflect this?"),
				    QMessageBox::Yes | QMessageBox::Default,
				    QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
          {
            q.prepare("SELECT changeWoQty(:wo_id, :qty, TRUE) AS result;");
            q.bindValue(":wo_id", _orderId);
            q.bindValue(":qty", _qtyOrdered->toDouble() * _qtyinvuomratio);
            q.exec();
	    if (q.first())
	    {
	      int result = q.value("result").toInt();
	      if (result < 0)
	      {
		systemError(this, storedProcErrorLookup("changeWoQty", result),
			    __FILE__, __LINE__);
		return;
	      }
	    }
	    else if (q.lastError().type() != QSqlError::None)
	    {
	      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	      return;
	    }
          }
        }
        else
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
            q.prepare("SELECT changePrQty(:pr_id, :qty) AS result;");
            q.bindValue(":pr_id", _orderId);
            q.bindValue(":qty", _qtyOrdered->toDouble() * _qtyinvuomratio);
            q.exec();
	    if (q.first())
	    {
	      bool result = q.value("result").toBool();
	      if (! result)
	      {
		systemError(this, tr("changePrQty failed"), __FILE__, __LINE__);
		return;
	      }
	    }
	    else if (q.lastError().type() != QSqlError::None)
	    {
	      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	      return;
	    }
          }
        }
      }
      _cQtyOrdered = _qtyOrdered->toDouble();
    }
  }
  else if (_mode == cNewQuote)
  {
//  Grab the next coitem_id
    q.exec("SELECT NEXTVAL('quitem_quitem_id_seq') AS _quitem_id");
    if (q.first())
      _soitemid = q.value("_quitem_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      reject();
      return;
    }

    q.prepare( "INSERT INTO quitem "
               "( quitem_id, quitem_quhead_id, quitem_linenumber, quitem_itemsite_id,"
               "  quitem_item_id, quitem_scheddate, quitem_qtyord,"
               "  quitem_qty_uom_id, quitem_qty_invuomratio,"
               "  quitem_unitcost, quitem_custprice, quitem_price,"
               "  quitem_price_uom_id, quitem_price_invuomratio,"
               "  quitem_custpn, quitem_memo, quitem_createorder, "
	       "  quitem_order_warehous_id, quitem_prcost, quitem_tax_id ) "
               "VALUES(:quitem_id, :quitem_quhead_id, :quitem_linenumber,"
               "       (SELECT itemsite_id FROM itemsite WHERE ((itemsite_item_id=:item_id) AND (itemsite_warehous_id=:warehous_id))),"
               "       :item_id, :quitem_scheddate, :quitem_qtyord,"
               "       :qty_uom_id, :qty_invuomratio,"
               "       stdCost(:item_id), :quitem_custprice, :quitem_price,"
               "       :price_uom_id, :price_invuomratio,"
               "       :quitem_custpn, :quitem_memo, :quitem_createorder, "
	       "       :quitem_order_warehous_id, :quitem_prcost, :quitem_tax_id);" );
    q.bindValue(":quitem_id", _soitemid);
    q.bindValue(":quitem_quhead_id", _soheadid);
    q.bindValue(":quitem_linenumber", _lineNumber->text().toInt());
    q.bindValue(":quitem_scheddate", _scheduledDate->date());
    q.bindValue(":quitem_qtyord", _qtyOrdered->toDouble());
    q.bindValue(":qty_uom_id", _qtyUOM->id());
    q.bindValue(":qty_invuomratio", _qtyinvuomratio);
    q.bindValue(":quitem_custprice", _customerPrice->localValue());
    q.bindValue(":quitem_price", _netUnitPrice->localValue());
    q.bindValue(":price_uom_id", _priceUOM->id());
    q.bindValue(":price_invuomratio", _priceinvuomratio);
    q.bindValue(":quitem_custpn", _customerPN->text());
    q.bindValue(":quitem_memo", _notes->text());
    q.bindValue(":quitem_createorder", QVariant(_createOrder->isChecked(), 0));
    q.bindValue(":quitem_order_warehous_id", _supplyWarehouse->id());
    q.bindValue(":quitem_prcost", _overridePoPrice->localValue());
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    if (_taxcode->isValid())
      q.bindValue(":quitem_tax_id", _taxcode->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (_mode == cEditQuote)
  {
    q.prepare( "UPDATE quitem "
               "SET quitem_scheddate=:quitem_scheddate,"
               "    quitem_qtyord=:quitem_qtyord,"
               "    quitem_qty_uom_id=:qty_uom_id, quitem_qty_invuomratio=:qty_invuomratio,"
               "    quitem_price=:quitem_price,"
               "    quitem_price_uom_id=:price_uom_id, quitem_price_invuomratio=:price_invuomratio,"
               "    quitem_memo=:quitem_memo, quitem_createorder=:quitem_createorder,"
               "    quitem_order_warehous_id=:quitem_order_warehous_id,"
	       "    quitem_prcost=:quitem_prcost, quitem_tax_id=:quitem_tax_id "
               "WHERE (quitem_id=:quitem_id);" );
    q.bindValue(":quitem_scheddate", _scheduledDate->date());
    q.bindValue(":quitem_promdate", promiseDate);
    q.bindValue(":quitem_qtyord", _qtyOrdered->toDouble());
    q.bindValue(":qty_uom_id", _qtyUOM->id());
    q.bindValue(":qty_invuomratio", _qtyinvuomratio);
    q.bindValue(":quitem_price", _netUnitPrice->localValue());
    q.bindValue(":price_uom_id", _priceUOM->id());
    q.bindValue(":price_invuomratio", _priceinvuomratio);
    q.bindValue(":quitem_memo", _notes->text());
    q.bindValue(":quitem_createorder", QVariant(_createOrder->isChecked(), 0));
    q.bindValue(":quitem_order_warehous_id", _supplyWarehouse->id());
    q.bindValue(":quitem_prcost", _overridePoPrice->localValue());
    q.bindValue(":quitem_id", _soitemid);
    if (_taxcode->isValid())
      q.bindValue(":quitem_tax_id", _taxcode->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if(_updateItemsite)
    {
      q.prepare("UPDATE quitem "
                "   SET quitem_itemsite_id=(SELECT itemsite_id FROM itemsite WHERE ((itemsite_item_id=quitem_item_id) AND (itemsite_warehous_id=:warehous_id)))"
                " WHERE (quitem_id=:quitem_id);" );
      q.bindValue(":warehous_id", _warehouse->id());
      q.bindValue(":quitem_id", _soitemid);
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }

  if ( (_mode != cView) && (_mode != cViewQuote) )
  {
    QString type = "SI";
    if(_mode & 0x20)
      type = "QI";

    q.prepare("SELECT updateCharAssignment(:target_type, :target_id, :char_id, :char_value) AS result;");

    QModelIndex idx1, idx2;
    for(int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx1 = _itemchar->index(i, 0);
      idx2 = _itemchar->index(i, 1);
      q.bindValue(":target_type", type);
      q.bindValue(":target_id", _soitemid);
      q.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
      q.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("updateCharAssignment", result),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }

//  If requested, create a new W/O or P/R for this coitem
  if ( ( (_mode == cNew) || (_mode == cEdit) ) &&
     (_createOrder->isChecked())               &&
     (_orderId == -1) )
  {
    QString chartype;
    if ((_item->itemType() == "M") || (_item->itemType() == "J"))
    {
		q.prepare( "SELECT createWo(:orderNumber, itemsite_id, :qty, itemsite_leadtime, :dueDate, :comments, :parent_type, :parent_id ) AS result, itemsite_id "
                 "FROM itemsite "
                 "WHERE ( (itemsite_item_id=:item_id)"
                 " AND (itemsite_warehous_id=:warehous_id) );" );
      q.bindValue(":orderNumber", _orderNumber->text().toInt());
      q.bindValue(":qty", _orderQty->toDouble());
      q.bindValue(":dueDate", _scheduledDate->date());
      q.bindValue(":comments", _custName + "\n" + _notes->text());
      q.bindValue(":item_id", _item->id());
      q.bindValue(":warehous_id", _supplyWarehouse->id());
	  q.bindValue(":parent_type", QString("S"));
	  q.bindValue(":parent_id", _soitemid);
      q.exec();
    }
    else if (_item->itemType() == "P")
    {
      q.prepare("SELECT createPr(:orderNumber, 'S', :soitem_id) AS result;");
      q.bindValue(":orderNumber", _orderNumber->text().toInt());
      q.bindValue(":soitem_id", _soitemid);
      q.exec();
    }

    if (q.first())
    {
      _orderId = q.value("result").toInt();
      if (_orderId < 0)
      {
	QString procname;
	if ((_item->itemType() == "M") || (_item->itemType() == "J"))
	  procname = "createWo";
	else if (_item->itemType() == "P")
	  procname = "createPr";
	else
	  procname = "unnamed stored procedure";
	systemError(this, storedProcErrorLookup(procname, _orderId),
		    __FILE__, __LINE__);
	return;
      }

      if ((_item->itemType() == "M") || (_item->itemType() == "J"))
      {
        omfgThis->sWorkOrdersUpdated(_orderId, TRUE);

//  Update the newly created coitem with the newly create wo_id
        q.prepare( "UPDATE coitem "
                   "SET coitem_order_type='W', coitem_order_id=:orderid "
                   "WHERE (coitem_id=:soitem_id);" );
        q.bindValue(":orderid", _orderId);
        q.bindValue(":soitem_id", _soitemid);
        q.exec();
	if (q.lastError().type() != QSqlError::None)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
        q.prepare("INSERT INTO charass"
                  "      (charass_target_type, charass_target_id,"
                  "       charass_char_id, charass_value) "
                  "SELECT 'W', :orderid, charass_char_id, charass_value"
                  "  FROM charass"
                  " WHERE ((charass_target_type='SI')"
                  "   AND  (charass_target_id=:soitem_id));");
        q.bindValue(":orderid", _orderId);
        q.bindValue(":soitem_id", _soitemid);
        q.exec();
	if (q.lastError().type() != QSqlError::None)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
      else if (_item->itemType() == "P")
      {
//  Update the newly created coitem with the newly pr_id
        q.prepare( "UPDATE coitem "
                   "SET coitem_order_type='R', coitem_order_id=:orderid "
                   "WHERE (coitem_id=:soitem_id);" );
        q.bindValue(":orderid", _orderId);
        q.bindValue(":soitem_id", _soitemid);
        q.exec();
	if (q.lastError().type() != QSqlError::None)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_mode == cNew)
    omfgThis->sSalesOrdersUpdated(_soheadid);
  else if (_mode == cNewQuote)
    omfgThis->sQuotesUpdated(_soheadid);

  if( (!_canceling) && (cNew == _mode || cNewQuote == _mode) )
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
    itemsite.prepare( "SELECT itemsite_leadtime, "
                      "       itemsite_createwo, itemsite_createpr "
                      "FROM item, itemsite "
                      "WHERE ( (itemsite_item_id=item_id)"
                      " AND (itemsite_warehous_id=:warehous_id)"
                      " AND (item_id=:item_id) );" );
    itemsite.bindValue(":warehous_id", _warehouse->id());
    itemsite.bindValue(":item_id", _item->id());
    itemsite.exec();
    if (itemsite.first())
    {
      _leadTime = itemsite.value("itemsite_leadtime").toInt();

      if (cNew == _mode || cNewQuote == _mode)
      {
        if (_item->itemType() == "M")
          _createOrder->setChecked(itemsite.value("itemsite_createwo").toBool());

		else if (_item->itemType() == "J")
		{
          _createOrder->setChecked(TRUE);
		  _createOrder->setEnabled(FALSE);
		}

        else if (_item->itemType() == "P")
          _createOrder->setChecked(itemsite.value("itemsite_createpr").toBool());

        else
        {
          _createOrder->setChecked(FALSE);
          _createOrder->setEnabled(FALSE);
        }
      }
    }
    else if (itemsite.lastError().type() != QSqlError::None)
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

  priceList newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Accepted)
  {
    _netUnitPrice->setLocalValue(newdlg._selectedPrice * (_priceinvuomratio / _priceRatio));
    sCalculateDiscountPrcnt();
  }
}

void salesOrderItem::sDeterminePrice()
{
  if(cView == _mode || cViewQuote == _mode)
    return;

  if ((_item->isValid()) && (_qtyOrdered->text().length()))
  {
    if (_mode == cEditQuote || _mode == cEdit)
    {
      if ((_qtyOrdered->toDouble() == _orderQtyChanged) || (_metrics->value("UpdatePriceLineEdit").toInt() == iDontUpdate))
        return;

      if( _metrics->value("UpdatePriceLineEdit").toInt() != iJustUpdate )
      {
        if (QMessageBox::question(this, tr("Update Price?"),
                tr("<p>The Item qty. has changed. Do you want to update the Price?"),
                QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape) == QMessageBox::No)
        {
          _orderQtyChanged = _qtyOrdered->toDouble();
          return;
        }
      }
    }

    XSqlQuery itemprice;
    itemprice.prepare( "SELECT itemPrice(item_id, :cust_id, :shipto_id, :qty, "
    		       "		 :curr_id, :effective) AS price "
                       "FROM item "
                       "WHERE (item_id=:item_id);" );
    itemprice.bindValue(":cust_id", _custid);
    itemprice.bindValue(":shipto_id", _shiptoid);
    itemprice.bindValue(":qty", _qtyOrdered->toDouble() * _qtyinvuomratio);
    itemprice.bindValue(":item_id", _item->id());
    itemprice.bindValue(":curr_id", _customerPrice->id());
    itemprice.bindValue(":effective", _customerPrice->effective());
    itemprice.exec();
    if (itemprice.first())
    {
    // This trap doesn't make sense.  -9999 is only returned if the item is exclusive and there is no price schedule
      if (itemprice.value("price").toDouble() == -9999.0)
      {
        QMessageBox::critical(this, tr("Customer Cannot Buy at Quantity"),
                              tr("<p>Although the selected Customer may "
			         "purchase the selected Item at some quantity "
				 "levels, the entered Quantity Ordered is too "
				 "low.  You may click on the price list button "
				 "(...) next to the Unit Price to determine "
				 "the minimum quantity the selected Customer "
				 "may purchase." ) );
        _originalQtyOrd = 0.0;
        _qtyOrdered->clear();
        _customerPrice->clear();
        _netUnitPrice->clear();

        _qtyOrdered->setFocus();
	return;
      }
      else
      {
        double price = itemprice.value("price").toDouble();
        price = price * (_priceinvuomratio / _priceRatio);
        _customerPrice->setLocalValue(price);
        _netUnitPrice->setLocalValue(price);

        sCalculateDiscountPrcnt();
        _orderQtyChanged = _qtyOrdered->toDouble();
      }
    }
    else if (itemprice.lastError().type() != QSqlError::None)
    {
      systemError(this, itemprice.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void salesOrderItem::sPopulateItemInfo(int pItemid)
{
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
    q.prepare( "SELECT item_type, item_config, uom_name,"
               "       item_inv_uom_id, item_price_uom_id,"
               "       iteminvpricerat(item_id) AS invpricerat,"
               "       item_listprice, "
               "       stdcost(item_id) AS f_unitcost,"
	       "       getItemTaxType(item_id, :taxauth) AS taxtype_id "
               "FROM item JOIN uom ON (item_inv_uom_id=uom_id)"
               "WHERE (item_id=:item_id);" );
    q.bindValue(":item_id", pItemid);
    q.bindValue(":taxauth", _taxauthid);
    q.exec();
    if (q.first())
    {
      if (_mode == cNew)
        sDeterminePrice();

      _priceRatio = q.value("invpricerat").toDouble(); // Always ratio from default price uom
      _invuomid = q.value("item_inv_uom_id").toInt();
      _priceinvuomratio = _priceRatio; // the ration from the currently selected price uom
      _qtyinvuomratio = 1.0;

      _qtyUOM->setId(q.value("item_inv_uom_id").toInt());
      _priceUOM->setId(q.value("item_price_uom_id").toInt());
      
      _listPrice->setBaseValue(q.value("item_listprice").toDouble());
      _unitCost->setBaseValue(q.value("f_unitcost").toDouble());
      _taxtype->setId(q.value("taxtype_id").toInt());

      sCalculateDiscountPrcnt();

      if ((_item->itemType() == "M") || (_item->itemType() == "J"))
      {
        if ( (_mode == cNew) || (_mode == cEdit) )
          _createOrder->setEnabled((_item->itemType() == "M"));

        _createOrder->setTitle(tr("C&reate Work Order"));
        _orderQtyLit->setText(tr("W/O Q&ty.:"));
        _orderDueDateLit->setText(tr("W/O Due Date:"));
        _orderStatusLit->setText(tr("W/O Status:"));
        if (_metrics->boolean("MultiWhs"))
        {
          _supplyWarehouseLit->show();
          _supplyWarehouse->show();
        }
        _overridePoPrice->hide();
        _overridePoPriceLit->hide();
      }
      else if (_item->itemType() == "P")
      {
        if ( (_mode == cNew) || (_mode == cEdit) )
          _createOrder->setEnabled(TRUE);

        _createOrder->setTitle(tr("C&reate Purchase Request"));
        _orderQtyLit->setText(tr("P/R Q&ty.:"));
        _orderDueDateLit->setText(tr("P/R Due Date:"));
        _orderStatusLit->setText(tr("P/R Status:"));
        _supplyWarehouseLit->hide();
        _supplyWarehouse->hide();
        _overridePoPrice->show();
        _overridePoPriceLit->show();
      }
      else
      {
        if ( (_mode == cNew) || (_mode == cEdit) )
          _createOrder->setEnabled(TRUE);

        _createOrder->setTitle(tr("C&reate Order"));
        _orderQtyLit->setText(tr("Order Q&ty.:"));
        _orderDueDateLit->setText(tr("Order Due Date:"));
        _orderStatusLit->setText(tr("Order Status:"));
        if (_metrics->boolean("MultiWhs"))
        {
          _supplyWarehouseLit->show();
          _supplyWarehouse->show();
        }
        _overridePoPrice->hide();
        _overridePoPriceLit->hide();
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    q.prepare( "SELECT DISTINCT char_id, char_name,"
               "       COALESCE(b.charass_value, (SELECT c.charass_value FROM charass c WHERE ((c.charass_target_type='I') AND (c.charass_target_id=:item_id) AND (c.charass_default) AND (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "  FROM charass a, char "
               "    LEFT OUTER JOIN charass b"
               "      ON (b.charass_target_type=:sotype"
               "      AND b.charass_target_id=:soitem_id"
               "      AND b.charass_char_id=char_id) "
               " WHERE ( (a.charass_char_id=char_id)"
               "   AND   (a.charass_target_type='I')"
               "   AND   (a.charass_target_id=:item_id) ) "
               " ORDER BY char_name;" );
    q.bindValue(":item_id", pItemid);
    if(_mode & 0x20)
      q.bindValue(":sotype", "QI");
    else
      q.bindValue(":sotype", "SI");
    q.bindValue(":soitem_id", _soitemid);
    q.exec();
    int row = 0;
    QModelIndex idx;
    while(q.next())
    {
      _itemchar->insertRow(_itemchar->rowCount());
      idx = _itemchar->index(row, 0);
      _itemchar->setData(idx, q.value("char_name"), Qt::DisplayRole);
      _itemchar->setData(idx, q.value("char_id"), Qt::UserRole);
      idx = _itemchar->index(row, 1);
      _itemchar->setData(idx, q.value("charass_value"), Qt::DisplayRole);
      _itemchar->setData(idx, pItemid, Qt::UserRole);
      row++;
    }
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void salesOrderItem::sDetermineAvailability()
{
  if(  (_item->id()==_availabilityLastItemid)
    && (_warehouse->id()==_availabilityLastWarehousid)
    && (_scheduledDate->date()==_availabilityLastSchedDate)
    && (_showAvailability->isChecked()==_availabilityLastShow)
    && (_showIndented->isChecked()==_availabilityLastShowIndent)
    && ((_qtyOrdered->toDouble() * _qtyinvuomratio)==_availabilityQtyOrdered) )
    return;

  _availabilityLastItemid = _item->id();
  _availabilityLastWarehousid = _warehouse->id();
  _availabilityLastSchedDate = _scheduledDate->date();
  _availabilityLastShow = _showAvailability->isChecked();
  _availabilityLastShowIndent = _showIndented->isChecked();
  _availabilityQtyOrdered = (_qtyOrdered->toDouble() * _qtyinvuomratio);

  _availability->clear();

  if ((_item->isValid()) && (_scheduledDate->isValid()) && (_showAvailability->isChecked()) )
  {
    XSqlQuery availability;
    availability.prepare( "SELECT itemsite_id,"
                          "       formatQty(qoh) AS f_qoh,"
                          "       formatQty(allocated) AS f_allocated,"
                          "       formatQty(noNeg(qoh - allocated)) AS f_unallocated,"
                          "       formatQty(ordered) AS f_ordered,"
                          "       (qoh - allocated + ordered) AS available,"
                          "       formatQty(qoh - allocated + ordered) AS f_available, "
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
      _onHand->setText(availability.value("f_qoh").toString());
      _allocated->setText(availability.value("f_allocated").toString());
      _unallocated->setText(availability.value("f_unallocated").toString());
      _onOrder->setText(availability.value("f_ordered").toString());
      _available->setText(availability.value("f_available").toString());
	  _leadtime->setText(availability.value("itemsite_leadtime").toString());

      if (availability.value("available").toDouble() < _availabilityQtyOrdered)
        _available->setPaletteForegroundColor(QColor("red"));
      else
        _available->setPaletteForegroundColor(QColor("black"));

      if ((_item->itemType() == "M") || (_item->itemType() == "J"))
      {
        if(_showIndented->isChecked())
        {
          availability.prepare("SELECT indentedBOM(:item_id) AS bomwork_set_id;");
          availability.bindValue(":item_id", _item->id());
          availability.exec();
          if(availability.first())
          {
            int _worksetid = availability.value("bomwork_set_id").toInt();
	    if (_worksetid < 0)
	    {
	      systemError(this, storedProcErrorLookup("indentedBOM", _worksetid),
			  __FILE__, __LINE__);
	      return;
	    }
            availability.prepare("SELECT itemsiteid, reorderlevel,"
                                 "       bomwork_level, bomwork_id, bomwork_parent_id,"
                                 "       bomwork_seqnumber AS bomitem_seqnumber,"
                                 "       item_number, item_descrip, uom_name,"
                                 "       pendalloc, formatQty(pendalloc) AS f_pendalloc,"
                                 "       ordered, formatQty(ordered) AS f_ordered,"
                                 "       qoh, formatQty(qoh) AS f_qoh,"
                                 "       formatQty(totalalloc + pendalloc) AS f_totalalloc,"
                                 "       (qoh + ordered - (totalalloc + pendalloc)) as totalavail,"
                                 "       formatQty(qoh + ordered - (totalalloc + pendalloc)) as f_totalavail"
                                 "  FROM ( SELECT itemsite_id AS itemsiteid,"
                                 "                CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
                                 "                bomwork_id, bomwork_parent_id,"
                                 "                bomwork_level, bomwork_seqnumber,"
                                 "                item_number, uom_name,"
                                 "                (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
                                 "                ((bomwork_qtyper * (1 + bomwork_scrap)) * :qty) AS pendalloc,"
                                 "                (qtyAllocated(itemsite_id, DATE(:schedDate)) - ((bomwork_qtyper * (1 + bomwork_scrap)) * :origQtyOrd)) AS totalalloc,"
                                 "                noNeg(itemsite_qtyonhand) AS qoh,"
                                 "                qtyOrdered(itemsite_id, DATE(:schedDate)) AS ordered"
                                 "           FROM bomwork, uom,"
				 "                item LEFT OUTER JOIN"
				 "                itemsite ON ((itemsite_item_id=item_id)"
                                 "                         AND (itemsite_warehous_id=:warehous_id))"
                                 "          WHERE ( (item_inv_uom_id=uom_id)"
                                 "            AND   (bomwork_item_id=item_id)"
                                 "            AND   (bomwork_set_id=:bomwork_set_id)"
                                 "                )"
                                 "       ) AS data "
                                 "ORDER BY bomwork_level, bomwork_seqnumber DESC;");
            availability.bindValue(":bomwork_set_id", _worksetid);
            availability.bindValue(":warehous_id", _warehouse->id());
            availability.bindValue(":qty", _availabilityQtyOrdered);
            availability.bindValue(":schedDate", _scheduledDate->date());
            availability.bindValue(":origQtyOrd", _originalQtyOrd);
            availability.exec();
            while(availability.next())
            {
              XTreeWidgetItem *last = NULL;

              //  If the current bomwork is top level, make it a child of the XTreeWidget
              if (availability.value("bomwork_parent_id").toInt() == -1)
                last = new XTreeWidgetItem( _availability, availability.value("bomwork_id").toInt(),
                                          availability.value("bomitem_seqnumber"), availability.value("item_number"),
                                          availability.value("item_descrip"), availability.value("uom_name"),
                                          availability.value("f_pendalloc"), availability.value("f_totalalloc"),
                                          availability.value("f_ordered"), availability.value("f_qoh"),
                                          availability.value("f_totalavail")  );
  
              else
              {
                //  March though the existing list, looking for the parent for the current bomwork
                for (int i = 0; i < _availability->topLevelItemCount(); i++)
                {
		  XTreeWidgetItem *cursor = _availability->topLevelItem(i);
                  cursor->setExpanded(TRUE);
                  if (cursor->id() == availability.value("bomwork_parent_id").toInt())
                  {
                    //  Found it, add the current bomwork as a child of its parent
                    last = new XTreeWidgetItem( cursor, availability.value("bomwork_id").toInt(),
                                              availability.value("bomitem_seqnumber"), availability.value("item_number"),
                                              availability.value("item_descrip"), availability.value("uom_name"),
                                              availability.value("f_pendalloc"), availability.value("f_totalalloc"),
                                              availability.value("f_ordered"), availability.value("f_qoh"),
                                              availability.value("f_totalavail")  );
                    break;
                  }
                }
              }

              if (last && availability.value("qoh").toDouble() < availability.value("pendalloc").toDouble())
                last->setTextColor(7, "red");
    
              if (last && availability.value("totalavail").toDouble() < 0.0)
                last->setTextColor(8, "red");
              else if (last && availability.value("totalavail").toDouble() <= availability.value("reorderlevel").toDouble())
                last->setTextColor(8, "orange");
            }
      
            //  All done with the bomwork set, delete it
            availability.prepare("SELECT deleteBOMWorkset(:bomwork_set_id) AS result;");
            availability.bindValue(":bomwork_set_id", _worksetid);
            availability.exec();
	    if (availability.first())
	    {
	      int result = availability.value("result").toInt();
	      if (result < 0)
	      {
		systemError(this, storedProcErrorLookup("deleteBOMWorkset", result),
			    __FILE__, __LINE__);
		return;
	      }
	    }
	    else if (availability.lastError().type() != QSqlError::None)
	    {
	      systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
	      return;
	    }
          }
        }
        else
        {
          int itemsiteid = availability.value("itemsite_id").toInt();
          availability.prepare( "SELECT itemsiteid, reorderlevel,"
                                "       bomitem_seqnumber, item_number, item_descrip, uom_name,"
                                "       pendalloc, formatQty(pendalloc) AS f_pendalloc,"
                                "       ordered, formatQty(ordered) AS f_ordered,"
                                "       qoh, formatQty(qoh) AS f_qoh,"
                                "       formatQty(totalalloc + pendalloc) AS f_totalalloc,"
                                "       (qoh + ordered - (totalalloc + pendalloc)) AS totalavail,"
                                "       formatQty(qoh + ordered - (totalalloc + pendalloc)) AS f_totalavail "
                                "FROM ( SELECT cs.itemsite_id AS itemsiteid,"
                                "              CASE WHEN(cs.itemsite_useparams) THEN cs.itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
                                "              bomitem_seqnumber, item_number,"
                                "              (item_descrip1 || ' ' || item_descrip2) AS item_descrip, uom_name,"
                                "              ((itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap))) * :qty) AS pendalloc,"
                                "              (qtyAllocated(cs.itemsite_id, DATE(:schedDate)) - ((itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap))) * :origQtyOrd)) AS totalalloc,"
                                "              noNeg(cs.itemsite_qtyonhand) AS qoh,"
                                "              qtyOrdered(cs.itemsite_id, DATE(:schedDate)) AS ordered "
                                "       FROM item, bomitem, uom,"
				"            itemsite AS ps LEFT OUTER JOIN"
				"            itemsite AS cs ON ((cs.itemsite_warehous_id=ps.itemsite_warehous_id)"
				"                           AND (cs.itemsite_item_id=ps.itemsite_item_id)) "
                                "       WHERE ( (bomitem_item_id=item_id)"
                                "        AND (item_inv_uom_id=uom_id)"
                                "        AND (bomitem_parent_item_id=ps.itemsite_item_id)"
                                "        AND (:schedDate BETWEEN bomitem_effective AND (bomitem_expires-1))"
                                "        AND (ps.itemsite_id=:itemsite_id) ) ) AS data "
                                "ORDER BY bomitem_seqnumber;" );
          availability.bindValue(":itemsite_id", itemsiteid);
          availability.bindValue(":qty", _availabilityQtyOrdered);
          availability.bindValue(":schedDate", _scheduledDate->date());
          availability.bindValue(":origQtyOrd", _originalQtyOrd);
          availability.exec();
	  XTreeWidgetItem *last = 0;
          while (availability.next())
          {
	    last = new XTreeWidgetItem(_availability, last,
				       availability.value("itemsiteid").toInt(),
				       availability.value("bomitem_seqnumber"),
				       availability.value("item_number"),
				       availability.value("item_descrip"),
				       availability.value("uom_name"),
				       availability.value("f_pendalloc"),
				       availability.value("f_totalalloc"),
				       availability.value("f_ordered"),
				       availability.value("f_qoh"),
				       availability.value("f_totalavail")  );
  
            if (availability.value("qoh").toDouble() < availability.value("pendalloc").toDouble())
              last->setTextColor(7, "red");
  
            if (availability.value("totalavail").toDouble() < 0.0)
              last->setTextColor(8, "red");
            else if (availability.value("totalavail").toDouble() <= availability.value("reorderlevel").toDouble())
              last->setTextColor(8, "orange");
          }
        }
      }
      else
        _availability->setEnabled(FALSE);
    }
    else if (availability.lastError().type() != QSqlError::None)
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

void salesOrderItem::sCalculateDiscountPrcnt()
{
  double netUnitPrice = _netUnitPrice->baseValue();

  if (netUnitPrice == 0.0)
  {
    _discountFromList->setText(QString::number(100));
    _discountFromCust->setText(QString::number(100));
  }
  else
  {
    if (_listPrice->isZero())
      _discountFromList->setText(tr("N/A"));
    else
      _discountFromList->setText(formatSalesPrice((1 - (netUnitPrice / _listPrice->baseValue())) * 100));

    if (_customerPrice->isZero())
      _discountFromCust->setText(tr("N/A"));
    else
      _discountFromCust->setText(formatSalesPrice((1 - (netUnitPrice / _customerPrice->baseValue())) * 100));
  }

  sCalculateExtendedPrice();
}

void salesOrderItem::sCalculateExtendedPrice()
{
  _extendedPrice->setLocalValue(((_qtyOrdered->toDouble() * _qtyinvuomratio) / _priceinvuomratio) * _netUnitPrice->localValue());
}

void salesOrderItem::sHandleWo(bool pCreate)
{
  if (pCreate)
  {
    if (_orderId == -1)
      sPopulateOrderInfo();
  }

  else
  {
    if (_orderId != -1)
    {
      XSqlQuery query;

      if ((_item->itemType() == "M") || (_item->itemType() == "J"))
      {
        if (QMessageBox::question(this, tr("Delete Work Order"),
                                  tr("<p>You are requesting to delete the Work "
				     "Order created for this Sales Order Item."
				     "Are you sure you want to do this?"),
				  QMessageBox::Yes | QMessageBox::Default,
				  QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
        {
          query.prepare("SELECT deleteWo(:wo_id, TRUE) AS result;");
          query.bindValue(":wo_id", _orderId);
          query.exec();
	  if (query.first())
	  {
	    int result = query.value("result").toInt();
	    if (result < 0)
	    {
	      systemError(this, storedProcErrorLookup("deleteWo", result),
			  __FILE__, __LINE__);
	      _createOrder->setChecked(true); // if (pCreate) => won't recurse
	      return;
	    }
	  }
	  else if (query.lastError().type() != QSqlError::NoError)
	  {
	    systemError(this, query.lastError().databaseText(),
			__FILE__, __LINE__);
	    _createOrder->setChecked(true); // if (pCreate) => won't recurse
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
        if (QMessageBox::question(this, tr("Delete Purchase Request"),
				  tr("<p>You are requesting to delete the "
				     "Purchase Request created for this Sales "
				     "Order Item. Are you sure you want to do "
				     "this?"),
				  QMessageBox::Yes | QMessageBox::Default,
				  QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
        {
          query.prepare("SELECT deletePr(:pr_id) AS result;");
          query.bindValue(":pr_id", _orderId);
          query.exec();
	  if (q.first())
	  {
	    bool result = q.value("result").toBool();
	    if (! result)
	    {
	      systemError(this, tr("deletePr failed"), __FILE__, __LINE__);
	      return;
	    }
	  }
	  else
	  if (query.lastError().type() != QSqlError::None)
	  {
	    systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
	    return;
	  }
        }
        else
        {
          _createOrder->setChecked(TRUE);
          return;
        }
      }
    }

    _orderId = -1;
    _orderQty->clear();
    _orderDueDate->clear();
    _orderStatus->clear();

    _createOrder->setChecked(FALSE);
  }
}

void salesOrderItem::sPopulateOrderInfo()
{
  if (_createOrder->isChecked())
  {
    _orderDueDate->setDate(_scheduledDate->date());

    if (_createOrder->isChecked())
    {
      XSqlQuery qty;
      qty.prepare( "SELECT validateOrderQty(itemsite_id, :qty, TRUE) AS qty "
                   "FROM itemsite "
                   "WHERE ((itemsite_item_id=:item_id)"
                   " AND (itemsite_warehous_id=:warehous_id));" );
      qty.bindValue(":qty", _qtyOrdered->toDouble() * _qtyinvuomratio);
      qty.bindValue(":item_id", _item->id());
      qty.bindValue(":warehous_id", ((_item->itemType() == "M") || (_item->itemType() == "J") ? _supplyWarehouse->id() : _warehouse->id()));
      qty.exec();
      if (qty.first())
        _orderQty->setText(qty.value("qty").toString());

      else if (qty.lastError().type() != QSqlError::None)
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
    _netUnitPrice->setLocalValue(_customerPrice->localValue() -
	(_customerPrice->localValue() * _discountFromCust->toDouble() / 100.0));
    sCalculateDiscountPrcnt();
  }
}

void salesOrderItem::populate()
{
  if (_mode == cNew || _mode == cNewQuote)
    return;

  ParameterList qparams;

  QString sql("SELECT taxauth_id, "
	      "<? if exists(\"isSalesOrder\") ?>"
	      "       cohead_curr_id AS curr_id "
	      "FROM cohead, coitem, taxauth "
	      "WHERE ((cohead_taxauth_id=taxauth_id)"
	      "  AND  (cohead_id=coitem_cohead_id)"
	      "  AND  (coitem_id=<? value(\"id\") ?>))"
	      "<? else ?>"
	      "       quhead_curr_id AS curr_id "
	      "FROM quhead, quitem, taxauth "
	      "WHERE ((quhead_taxauth_id=taxauth_id)"
	      "  AND  (quhead_id=quitem_quhead_id)"
	      "  AND  (quitem_id=<? value(\"id\") ?>))"
	      "<? endif ?>"
	      ";");
  qparams.append("id", _soitemid);
  if (! ISQUOTE(_mode))
    qparams.append("isSalesOrder");
  MetaSQLQuery mql(sql);
  q = mql.toQuery(qparams);
  if (q.first())
    _taxauthid = q.value("taxauth_id").toInt();
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery item;
  sql = "<? if exists(\"isSalesOrder\") ?>"
	"SELECT itemsite_leadtime, warehous_id, warehous_code,"
	"       item_id, uom_name, iteminvpricerat(item_id) AS invpricerat, item_listprice,"
        "       item_inv_uom_id,"
	"       stdCost(item_id) AS stdcost,"
	"       coitem_status, coitem_cohead_id,"
	"       coitem_order_type, coitem_order_id, coitem_custpn,"
	"       coitem_memo, NULL AS quitem_createorder,"
	"       NULL AS quitem_order_warehous_id,"
	"       coitem_linenumber,"
	"       coitem_qtyord AS qtyord,"
        "       coitem_qty_uom_id AS qty_uom_id,"
        "       coitem_qty_invuomratio AS qty_invuomratio,"
	"       coitem_qtyshipped AS qtyshipped,"
	"       coitem_scheddate,"
	"       coitem_custprice,"
	"       coitem_price,"
        "       coitem_price_uom_id AS price_uom_id,"
        "       coitem_price_invuomratio AS price_invuomratio,"
	"       coitem_promdate AS promdate,"
	"       coitem_substitute_item_id, coitem_prcost,"
	"       (SELECT COALESCE(SUM(coship_qty), 0)"
	"          FROM coship"
	"         WHERE (coship_coitem_id=coitem_id)) AS coship_qty,"
	"       coitem_tax_id,"
		"       getItemTaxType(item_id, cohead_taxauth_id) AS coitem_taxtype_id, "
		"       coitem_cos_accnt_id, coitem_warranty "
	"FROM coitem, warehous, itemsite, item, uom, cohead "
	"WHERE ( (coitem_itemsite_id=itemsite_id)"
	" AND (itemsite_warehous_id=warehous_id)"
	" AND (itemsite_item_id=item_id)"
        " AND (item_inv_uom_id=uom_id)"
	" AND (cohead_id=coitem_cohead_id)"
	" AND (coitem_id=<? value(\"id\") ?>));" 
	"<? else ?>"
	"SELECT itemsite_leadtime, COALESCE(warehous_id, -1) AS warehous_id, "
	"       warehous_code,"
	"       item_id, uom_name, iteminvpricerat(item_id) AS invpricerat, item_listprice,"
        "       item_inv_uom_id,"
	"       stdcost(item_id) AS stdcost,"
	"       'O' AS coitem_status, quitem_quhead_id AS coitem_cohead_id,"
	"	'' AS coitem_order_type, -1 AS coitem_order_id,"
	"       '' AS coitem_custpn,"
	"       quitem_memo AS coitem_memo, quitem_createorder,"
	"       quitem_order_warehous_id,"
	"       quitem_linenumber AS coitem_linenumber,"
	"       quitem_qtyord AS qtyord,"
        "       quitem_qty_uom_id AS qty_uom_id,"
        "       quitem_qty_invuomratio AS qty_invuomratio,"
	"       NULL AS qtyshipped,"
	"       quitem_scheddate AS coitem_scheddate,"
	"       quitem_custprice AS coitem_custprice, "
	"       quitem_price AS coitem_price,"
        "       quitem_price_uom_id AS price_uom_id,"
        "       quitem_price_invuomratio AS price_invuomratio,"
	"       NULL AS promdate,"
	"       -1 AS coitem_substitute_item_id, quitem_prcost AS coitem_prcost,"
	"       0 AS coship_qty,"
	"       quitem_tax_id AS coitem_tax_id,"
	"       getItemTaxType(item_id, quhead_taxauth_id) AS coitem_taxtype_id "
	"  FROM item, uom, quhead, quitem LEFT OUTER JOIN (itemsite JOIN warehous ON (itemsite_warehous_id=warehous_id)) ON (quitem_itemsite_id=itemsite_id) "
	" WHERE ( (quitem_item_id=item_id)"
        "   AND   (item_inv_uom_id=uom_id)"
	"   AND   (quhead_id=quitem_quhead_id)"
	"   AND   (quitem_id=<? value(\"id\") ?>) );"
	"<? endif ?>" ;

  qparams.clear();
  qparams.append("id", _soitemid);
  if (! ISQUOTE(_mode))
    qparams.append("isSalesOrder");
  MetaSQLQuery metaitem(sql);
  item = metaitem.toQuery(qparams);
  if (item.first())
  {
    _soheadid = item.value("coitem_cohead_id").toInt();
    _comments->setId(_soitemid);
    _lineNumber->setText(item.value("coitem_linenumber").toString());
    _priceRatio = item.value("invpricerat").toDouble();
    _shippedToDate->setText(formatQty(item.value("qtyshipped").toDouble()));

    _item->setId(item.value("item_id").toInt()); // should precede _taxtype/code
    _invuomid = item.value("item_inv_uom_id").toInt();
    _qtyUOM->setId(item.value("qty_uom_id").toInt());
    _priceUOM->setId(item.value("price_uom_id").toInt());
    _qtyinvuomratio = item.value("qty_invuomratio").toDouble();
    _priceinvuomratio = item.value("price_invuomratio").toDouble();
    _unitCost->setBaseValue(item.value("stdcost").toDouble());
    // do tax stuff before _qtyOrdered so signal cascade has data to work with
    _taxtype->setId(item.value("coitem_taxtype_id").toInt());
    _taxcode->setId(item.value("coitem_tax_id").toInt());
    _orderId = item.value("coitem_order_id").toInt();
    _orderQtyChanged = item.value("qtyord").toDouble();
    _qtyOrdered->setText(formatQty(_orderQtyChanged));
    _scheduledDate->setDate(item.value("coitem_scheddate").toDate());
    _notes->setText(item.value("coitem_memo").toString());
    if (!item.value("quitem_createorder").isNull())
      _createOrder->setChecked(item.value("quitem_createorder").toBool());
    if (!item.value("promdate").isNull() && _metrics->boolean("UsePromiseDate"))
      _promisedDate->setText(item.value("promdate").toString());
    if(item.value("coitem_substitute_item_id").toInt() > 0)
    {
      _sub->setChecked(true);
      _subItem->setId(item.value("coitem_substitute_item_id").toInt());
    }
    _customerPrice->setLocalValue(item.value("coitem_custprice").toDouble());
    _listPrice->setBaseValue(item.value("item_listprice").toDouble() * (_priceinvuomratio / _priceRatio));
    _netUnitPrice->setLocalValue(item.value("coitem_price").toDouble());
    _leadTime = item.value("itemsite_leadtime").toInt();
    _cQtyOrdered = _qtyOrdered->toDouble();
    _originalQtyOrd = _qtyOrdered->toDouble();
    _cScheduledDate = _scheduledDate->date();
    if (! item.value("quitem_order_warehous_id").isNull())
      _supplyWarehouse->setId(item.value("quitem_order_warehous_id").toInt());
    if(item.value("qtyshipped").toDouble() > 0)
    {
      _qtyUOM->setEnabled(false);
      _priceUOM->setEnabled(false);
    }

    _customerPN->setText(item.value("coitem_custpn").toString());

    _overridePoPrice->setLocalValue(item.value("coitem_prcost").toDouble());
 
    _warranty->setChecked(item.value("coitem_warranty").toBool());
	_altCosAccnt->setId(item.value("coitem_cos_accnt_id").toInt());

    sCalculateDiscountPrcnt();
    sLookupTax();
    sDetermineAvailability();
  }
  else if (item.lastError().type() != QSqlError::None)
  {
    systemError(this, item.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_orderId != -1)
  {
    XSqlQuery query;

    if (item.value("coitem_order_type").toString() == "W")
    {
      if (_metrics->boolean("MultiWhs"))
      {
        _supplyWarehouseLit->show();
        _supplyWarehouse->show();
      }
      _overridePoPrice->hide();
      _overridePoPriceLit->hide();
      query.prepare( "SELECT wo_status,"
		     "       formatQty(wo_qtyord) AS f_qty,"
		     "       wo_duedate, warehous_id, warehous_code "
		     "FROM wo, itemsite, warehous "
		     "WHERE ((wo_itemsite_id=itemsite_id)"
		     " AND (itemsite_warehous_id=warehous_id)"
		     " AND (wo_id=:wo_id));" );
      query.bindValue(":wo_id", _orderId);
      query.exec();
      if (query.first())
      {
	_createOrder->setChecked(TRUE);

	_orderQty->setText(query.value("f_qty").toString());
	_orderDueDate->setDate(query.value("wo_duedate").toDate());
	_orderStatus->setText(query.value("wo_status").toString());

	if ((query.value("wo_status").toString() == "R") || (query.value("wo_status").toString() == "C"))
	  _createOrder->setEnabled(FALSE);

	_supplyWarehouse->clear();
	_supplyWarehouse->append(query.value("warehous_id").toInt(), query.value("warehous_code").toString());
	_supplyWarehouse->setEnabled(FALSE);
      }
      else
      {
	_orderId = -1;
	_createOrder->setChecked(FALSE);
      }
    }
    else if (item.value("coitem_order_type").toString() == "R")
    {
      _supplyWarehouseLit->hide();
      _supplyWarehouse->hide();
      _overridePoPrice->show();
      _overridePoPriceLit->show();
      query.prepare( "SELECT pr_status,"
		     "       formatQty(pr_qtyreq) AS qty,"
		     "       pr_duedate "
		     "FROM pr "
		     "WHERE (pr_id=:pr_id);" );
      query.bindValue(":pr_id", _orderId);
      query.exec();
      if (query.first())
      {
	_createOrder->setChecked(TRUE);

	_orderQty->setText(query.value("qty").toString());
	_orderDueDate->setDate(query.value("pr_duedate").toDate());
	_orderStatus->setText(query.value("pr_status").toString());

	if ((query.value("pr_status").toString() == "R") || (query.value("pr_status").toString() == "C"))
	  _createOrder->setEnabled(FALSE);
      }
      else
      {
	_orderId = -1;
	_createOrder->setChecked(FALSE);
      }
    }
  }
  else if(ISORDER(_mode))
    _createOrder->setChecked(FALSE);

  if (ISORDER(_mode))
  {
    _warehouse->clear();
    _warehouse->append(item.value("warehous_id").toInt(), item.value("warehous_code").toString());
    _warehouse->setEnabled(FALSE);

    if( (cView != _mode) && (item.value("coitem_status").toString() == "O") )
      _cancel->setEnabled((item.value("qtyshipped").toDouble()==0.0) && (item.value("coship_qty").toDouble()==0.0));
    else
      _cancel->setEnabled(false);
  }
  else // if (ISQUOTE(_mode))
  {
    if(cEditQuote == _mode && item.value("warehous_id").toInt() == -1)
    {
      _updateItemsite = true;
      _warehouse->append(-1, item.value("warehous_code").toString());
      _warehouse->setId(-1);
    }
    else
    {
      _warehouse->clear();
      _warehouse->append(item.value("warehous_id").toInt(), item.value("warehous_code").toString());
      _warehouse->setEnabled(FALSE);
    }
  }
}

void salesOrderItem::sFindSellingWarehouseItemsites( int id )
{
  _warehouse->findItemsites(id);
  _supplyWarehouse->findItemsites(id);
  if(_preferredWarehouseid > 0)
  {
    _warehouse->setId(_preferredWarehouseid);
    _supplyWarehouse->setId(_preferredWarehouseid);
  }
}

void salesOrderItem::sPriceGroup()
{
  if (! omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_netUnitPrice->currAbbr()));
}

void salesOrderItem::sNext()
{
  if(_modified)
  {
    switch( QMessageBox::question( this, tr("Unsaved Changed"),
              tr("<p>You have made some changes which have not yet been saved!\n"
                 "Would you like to save them now?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No,
              QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave();
        if(_modified) // catch an error saving
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

  if(cNew == _mode || cNewQuote == _mode)
  {
    _modified = false;
    return;
  }

  if (ISQUOTE(_mode))
    q.prepare("SELECT a.quitem_id AS id"
              "  FROM quitem AS a, quitem as b"
              " WHERE ((a.quitem_quhead_id=b.quitem_quhead_id)"
              "   AND  (a.quitem_linenumber > b.quitem_linenumber)"
              "   AND  (b.quitem_id=:id))"
              " ORDER BY a.quitem_linenumber"
              " LIMIT 1;");
  else
    q.prepare("SELECT a.coitem_id AS id"
              "  FROM coitem AS a, coitem AS b"
              " WHERE ((a.coitem_cohead_id=b.coitem_cohead_id)"
              "   AND  (a.coitem_linenumber > b.coitem_linenumber)"
              "   AND  (b.coitem_id=:id))"
              " ORDER BY a.coitem_linenumber"
              " LIMIT 1;");
  q.bindValue(":id", _soitemid);
  q.exec();
  if(q.first())
  {
    ParameterList params;
    if(_custid != -1)
      params.append("cust_id", _custid);
    params.append("soitem_id", q.value("id").toInt());
    if (ISQUOTE(_mode))
    {
      if(cNewQuote == _mode || cEditQuote == _mode)
        params.append("mode", "editQuote");
      else
        params.append("mode", "viewQuote");
    } 
    else
    {
      if(cNew == _mode || cEdit == _mode)
        params.append("mode", "edit");
      else
        params.append("mode", "view");
    }
    set(params);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else if(cView != _mode && cViewQuote != _mode)
  {
    ParameterList params;
    if(_custid != -1)
      params.append("cust_id", _custid);
    params.append("sohead_id", _soheadid);
    if (ISQUOTE(_mode))
      params.append("mode", "newQuote");
    else
      params.append("mode", "new");
    set(params);
  }
}


void salesOrderItem::sPrev()
{
  if(_modified)
  {
    switch( QMessageBox::question( this, tr("Unsaved Changed"),
              tr("<p>You have made some changes which have not yet been saved!\n"
                 "Would you like to save them now?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No,
              QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave();
        if(_modified) // catch an error saving
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
    if(cNewQuote == _mode)
      q.prepare("SELECT quitem_id AS id"
                "  FROM quitem"
                " WHERE (quitem_quhead_id=:sohead_id)"
                " ORDER BY quitem_linenumber DESC"
                " LIMIT 1;");
    else
      q.prepare("SELECT a.quitem_id AS id"
                "  FROM quitem AS a, quitem as b"
                " WHERE ((a.quitem_quhead_id=b.quitem_quhead_id)"
                "   AND  (a.quitem_linenumber < b.quitem_linenumber)"
                "   AND  (b.quitem_id=:id))"
                " ORDER BY a.quitem_linenumber DESC"
                " LIMIT 1;");
  }
  else
  {
    if(cNew == _mode)
      q.prepare("SELECT coitem_id AS id"
                "  FROM coitem"
                " WHERE (coitem_cohead_id=:sohead_id)"
                " ORDER BY coitem_linenumber DESC"
                " LIMIT 1;");
    else
      q.prepare("SELECT a.coitem_id AS id"
                "  FROM coitem AS a, coitem AS b"
                " WHERE ((a.coitem_cohead_id=b.coitem_cohead_id)"
                "   AND  (a.coitem_linenumber < b.coitem_linenumber)"
                "   AND  (b.coitem_id=:id))"
                " ORDER BY a.coitem_linenumber DESC"
                " LIMIT 1;");
  }
  q.bindValue(":id", _soitemid);
  q.bindValue(":sohead_id", _soheadid);
  q.exec();
  if(q.first())
  {
    ParameterList params;
    if(_custid != -1)
      params.append("cust_id", _custid);
    params.append("soitem_id", q.value("id").toInt());
    if (ISQUOTE(_mode))
    {
      if(cNewQuote == _mode || cEditQuote == _mode)
        params.append("mode", "editQuote");
      else
        params.append("mode", "viewQuote");
    } 
    else
    {
      if(cNew == _mode || cEdit == _mode)
        params.append("mode", "edit");
      else
        params.append("mode", "view");
    }
    set(params);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void salesOrderItem::sChanged()
{
  _modified = true;
}

void salesOrderItem::reject()
{
  if(_modified)
  {
    switch( QMessageBox::question( this, tr("Unsaved Changed"),
              tr("<p>You have made some changes which have not yet been saved!\n"
                 "Would you like to save them now?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No,
              QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave();
        if(_modified) // catch an error saving
          return;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
      default:
        return;
    }
  }
  QDialog::reject();
}

void salesOrderItem::sCancel()
{
  _canceling = true;
  
  sSave();
  if(_error) 
    return;

  q.prepare("UPDATE coitem SET coitem_status='X' WHERE (coitem_id=:coitem_id);");
  q.bindValue(":coitem_id", _soitemid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  clear();
  prepare();
  _prev->setEnabled(true);
  _item->setFocus();

  _modified = false;
  _canceling = false;
}

void salesOrderItem::sLookupTaxCode()
{
  XSqlQuery taxq;
  taxq.prepare("SELECT tax_ratea, tax_rateb, tax_ratec, tax_id "
	       "FROM tax "
	       "WHERE (tax_id=getTaxSelection(:auth, :type));");
  taxq.bindValue(":auth",    _taxauthid);
  taxq.bindValue(":type",    _taxtype->id());
  taxq.exec();
  if (taxq.first())
  {
    _taxcode->setId(taxq.value("tax_id").toInt());
    _cachedPctA	= taxq.value("tax_ratea").toDouble();
    _cachedPctB	= taxq.value("tax_rateb").toDouble();
    _cachedPctC	= taxq.value("tax_ratec").toDouble();
  }
  else if (taxq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _taxcode->setId(-1);
}

void salesOrderItem::sLookupTax()
{
  XSqlQuery calcq;
  calcq.prepare("SELECT calculateTax(:tax_id, :ext, 0, 'A') AS valA,"
		"               calculateTax(:tax_id, :ext, 0, 'B') AS valB,"
		"               calculateTax(:tax_id, :ext, 0, 'C') AS valC;");

  calcq.bindValue(":tax_id",  _taxcode->id());
  calcq.bindValue(":ext",     _extendedPrice->localValue());
  calcq.exec();
  if (calcq.first())
  {
    _cachedRateA= calcq.value("valA").toDouble();
    _cachedRateB= calcq.value("valB").toDouble();
    _cachedRateC= calcq.value("valC").toDouble();
    _tax->setLocalValue(_cachedRateA + _cachedRateB + _cachedRateC);
  }
  else if (calcq.lastError().type() != QSqlError::None)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void salesOrderItem::sTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("tax_id",   _taxcode->id());
  params.append("curr_id",  _tax->id());
  params.append("valueA",   _cachedRateA);
  params.append("valueB",   _cachedRateB);
  params.append("valueC",   _cachedRateC);
  params.append("pctA",	    _cachedPctA);
  params.append("pctB",	    _cachedPctB);
  params.append("pctC",	    _cachedPctC);
  params.append("subtotal", CurrDisplay::convert(_extendedPrice->id(), _tax->id(),
						 _extendedPrice->localValue(),
						 _extendedPrice->effective()));
  if(cView == _mode || cViewQuote == _mode)
    params.append("readOnly");

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    _cachedRateA = newdlg.amountA();
    _cachedRateB = newdlg.amountB();
    _cachedRateC = newdlg.amountC();
    _cachedPctA	 = newdlg.pctA();
    _cachedPctB	 = newdlg.pctB();
    _cachedPctC	 = newdlg.pctC();

    if (_taxcode->id() != newdlg.tax())
      _taxcode->setId(newdlg.tax());

    _tax->setLocalValue(_cachedRateA + _cachedRateB + _cachedRateC);
  }
}

void salesOrderItem::sQtyUOMChanged()
{
  if(_qtyUOM->id() == _invuomid)
    _qtyinvuomratio = 1.0;
  else
  {
    XSqlQuery invuom;
    invuom.prepare("SELECT itemuomtouomratio(item_id, :uom_id, item_inv_uom_id) AS ratio"
                   "  FROM item"
                   " WHERE(item_id=:item_id);");
    invuom.bindValue(":item_id", _item->id());
    invuom.bindValue(":uom_id", _qtyUOM->id());
    invuom.exec();
    if(invuom.first())
      _qtyinvuomratio = invuom.value("ratio").toDouble();
    else
      systemError(this, invuom.lastError().databaseText(), __FILE__, __LINE__);
  }

  if(_qtyUOM->id() != _invuomid)
  {
    _priceUOM->setId(_qtyUOM->id());
    _priceUOM->setEnabled(false);
  }
  else
    _priceUOM->setEnabled(true);
  sCalculateExtendedPrice();
}

void salesOrderItem::sPriceUOMChanged()
{
  if(_priceUOM->id() == -1 || _qtyUOM->id() == -1)
    return;

  if(_priceUOM->id() == _invuomid)
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
    if(invuom.first())
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
  sDeterminePrice();
}

void salesOrderItem::sCalcWoUnitCost()
{
  if (_item->itemType() == "J" && _orderId > -1 && _qtyOrdered->toDouble() != 0)
  {
    q.prepare("SELECT COALESCE(SUM(wo_postedvalue),0) AS wo_value "
		      "FROM wo "
			  "WHERE ((wo_ordtype='S') "
			  "AND (wo_ordid=:soitem_id));");
	q.bindValue(":soitem_id", _soitemid);
	q.exec();
	if (q.first())
      _unitCost->setBaseValue(q.value("wo_value").toDouble() / _qtyOrdered->toDouble() * _qtyinvuomratio);
  }
}
