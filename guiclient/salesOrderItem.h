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

public:
    salesOrderItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~salesOrderItem();

    virtual void prepare();
    virtual void clear();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sPopulateItemsiteInfo();
    virtual void sListPrices();
    virtual void sDeterminePrice();
    virtual void sDeterminePrice( bool p );
    virtual void sRecalcPrice();
    virtual void sPopulateItemInfo( int pItemid );
    virtual void sRecalcAvailability();
    virtual void sDetermineAvailability();
    virtual void sDetermineAvailability( bool p );
    virtual void sCalculateDiscountPrcnt();
    virtual void sCalculateExtendedPrice();
    virtual void sHandleWo( bool pCreate );
    virtual void sPopulateOrderInfo();
    virtual void sCalculateFromDiscount();
    virtual void populate();
    virtual void sFindSellingWarehouseItemsites( int id );
    virtual void sPriceGroup();
    virtual void sNext();
    virtual void sPrev();
    virtual void sChanged();
    virtual void sCancel();
    virtual void sLookupTax();
    virtual void sLookupTaxCode();
    virtual void sTaxDetail();
    virtual void sQtyUOMChanged();
    virtual void sPriceUOMChanged();
    virtual void sCalcWoUnitCost();

protected slots:
    virtual void languageChange();

    virtual void reject();


private:
    QString _custName;
    double _priceRatio;
    double _cQtyOrdered;
    QDate _cScheduledDate;
    int _preferredWarehouseid;
    int _shiptoid;
    int _orderId;
    int _leadTime;
    int _custid;
    int _soheadid;
    int _soitemid;
    int _mode;
    bool _modified;
    bool _canceling;
    bool _error;
    int _availabilityLastItemid;
    int _availabilityLastWarehousid;
    QDate _availabilityLastSchedDate;
    bool _availabilityLastShow;
    bool _availabilityLastShowIndent;
    double _originalQtyOrd;
    double _availabilityQtyOrdered;
    bool _invIsFractional;
    bool _updateItemsite;
    double _orderQtyChanged;
    double	_cachedPctA;
    double	_cachedPctB;
    double	_cachedPctC;
    double	_cachedRateA;
    double	_cachedRateB;
    double	_cachedRateC;
    int		_taxauthid;
    QStandardItemModel * _itemchar;
    int _invuomid;
    double _qtyinvuomratio;
    double _priceinvuomratio;
    
    //For holding variables for characteristic pricing
    QList<QVariant> _charVars;
    
    enum {
      CHAR_ID      = 0,
      CHAR_VALUE   = 1,
      CHAR_PRICE   = 2
    };
    
    enum {
      ITEM_ID   = 0,
      CUST_ID   = 1,
      SHIPTO_ID = 2,
      QTY       = 3,
      CURR_ID   = 4,
      EFFECTIVE = 5
    };

};

#endif // SALESORDERITEM_H
