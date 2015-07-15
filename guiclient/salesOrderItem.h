/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SALESORDERITEM_H
#define SALESORDERITEM_H

#include "guiclient.h"
#include <QStandardItemModel>
#include "xdialog.h"
#include <parameter.h>
#include "ui_salesOrderItem.h"

class salesOrderItem : public XDialog, public Ui::salesOrderItem
{
  Q_OBJECT

  public: salesOrderItem(QWidget *parent = 0, const char * = 0, Qt::WindowFlags fl = 0);
    ~salesOrderItem();

    virtual void  prepare();
    virtual void  clear();
    virtual void  setItemExtraClause();
    Q_INVOKABLE virtual int id() { return _soitemid; }
    Q_INVOKABLE virtual int soheadid() { return _soheadid; }
    Q_INVOKABLE virtual int custid() { return _custid; }
    Q_INVOKABLE virtual int supplyid() { return _supplyOrderId; }
    Q_INVOKABLE virtual int mode()  { return _mode; }
    Q_INVOKABLE virtual int modeType() const;

  public slots:
    virtual SetResponse set( const ParameterList &pParams );
    virtual void        sSaveClicked();
    virtual void        sSave(bool pPartial);
    virtual void        sPopulateItemsiteInfo();
    virtual void        sListPrices();
    virtual void        sDeterminePrice();
    virtual void        sDeterminePrice( bool force );
    virtual void        sRecalcPrice();
    virtual void        sPopulateItemInfo( int pItemid );
    virtual void        sRecalcAvailability();
    virtual void        sDetermineAvailability();
    virtual void        sDetermineAvailability( bool p );
    virtual void        sPopulateItemSources( int pItemid );
    virtual void        sPopulateItemSubs( int pItemid );
    virtual void        sPopulateSubMenu(QMenu *, QTreeWidgetItem *, int);
    virtual void        sSubstitute();
    virtual void        sReserveStock();
    virtual void        sPopulateHistory();
    virtual void        sCalculateDiscountPrcnt();
    virtual void        sCalculateExtendedPrice();
    virtual void        sCheckSupplyOrder();
    virtual void        sHandleSupplyOrder();
    virtual void        sPopulateOrderInfo();
    virtual void        sRollupPrices();
    virtual void        sFillWoIndentedList();
    virtual void        sPopulateWoMenu( QMenu * pMenu, QTreeWidgetItem * selected );
    virtual void        sNewWoMatl();
    virtual void        sEditWoMatl();
    virtual void        sViewWoMatl();
    virtual void        sDeleteWoMatl();
    virtual void        sCalculateFromDiscount();
    virtual void        sCalculateFromMarkup();
    virtual void        populate();
    virtual void        sFindSellingWarehouseItemsites( int id );
    virtual void        sPriceGroup();
    virtual void        sNext();
    virtual void        sPrev();
    virtual void        sChanged();
    virtual void        sCancel();
    virtual void        sLookupTax();
    virtual void        sTaxDetail();
    virtual void        sQtyUOMChanged();
    virtual void        sPriceUOMChanged();
    virtual void        sCalcUnitCost();
    virtual void        sHandleButton();
    virtual void        sHandleScheduleDate();

  protected slots:
    virtual void  languageChange();

    virtual void  reject();

  private:
    QString _custName;
    double  _priceRatio;
    int     _preferredWarehouseid;
    int     _shiptoid;
    QString _shiptoname;
    int     _supplyOrderId;
    int     _leadTime;
    int     _custid;
    int     _soheadid;
    int     _soitemid;
    int     _mode;
    int     _initialMode;
    int     _itemsrc;
    bool    _modified;
    bool    _canceling;
    bool    _partialsaved;
    bool    _error;
    bool    _stocked;
    int     _availabilityLastItemid;
    int     _availabilityLastWarehousid;
    QDate   _availabilityLastSchedDate;
    bool    _availabilityLastShow;
    bool    _availabilityLastShowIndent;
    double  _originalQtyOrd;
    double  _availabilityQtyOrdered;
    bool    _invIsFractional;
    bool    _updateItemsite;
    bool    _updatePrice;
    int     _priceUOMCache;
    double  _qtyOrderedCache;
    double  _supplyOrderQtyCache;
    double  _supplyOrderQtyOrderedCache;
    double  _supplyOrderQtyOrderedInvCache;
    QDate   _supplyOrderDueDateCache;
    QDate   _supplyOrderScheduledDateCache;
    bool    _supplyOrderDropShipCache;
    double  _supplyOverridePriceCache;
    double  _cachedPct;
    double  _cachedRate;
    int     _taxzoneid;
    QStandardItemModel *_itemchar;
    int     _invuomid;
    double  _qtyinvuomratio;
    double  _priceinvuomratio;
    double  _qtyreserved;
    QDate   _scheduledDateCache;
    QString _costmethod;
    QString _priceType;
    QString _priceMode;
    QString _supplyOrderType;

    // For holding variables for characteristic pricing
    QList<QVariant> _charVars;

    enum
    {
      CHAR_ID    = 0,
      CHAR_VALUE = 1,
      CHAR_PRICE = 2
    };

    enum
    {
      ITEM_ID    = 0,
      CUST_ID    = 1,
      SHIPTO_ID  = 2,
      QTY        = 3,
      CURR_ID    = 4,
      EFFECTIVE  = 5
    };
};

#endif  // SALESORDERITEM_H
