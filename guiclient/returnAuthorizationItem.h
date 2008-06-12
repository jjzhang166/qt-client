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

#ifndef RETURNAUTHORIZATIONITEM_H
#define RETURNAUTHORIZATIONITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "taxCache.h"
#include "ui_returnAuthorizationItem.h"

class returnAuthorizationItem : public XDialog, public Ui::returnAuthorizationItem
{
    Q_OBJECT

public:
    returnAuthorizationItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~returnAuthorizationItem();

public slots:
	
    SetResponse set(const ParameterList & pParams );
    void sPopulateItemInfo();
    void sPopulateItemsiteInfo();
    void populate();
    void sCalculateDiscountPrcnt();
    void sCalculateFromDiscount();
    void sListPrices();
    void sLookupTax();
    void sLookupTaxCode();
    void sPriceGroup();
    void sTaxDetail();
    void sQtyUOMChanged();
    void sPriceUOMChanged();
    void sDispositionChanged();
    void sDetermineAvailability();
    void sHandleWo( bool pCreate );
    void sPopulateOrderInfo();
    void sCalcWoUnitCost();
    
    //Lot/Serial page
    void sNew();
    void sEdit();
    void sDelete();
    void sFillList();
   
protected slots:
    void languageChange();

    bool sSave();
    void sSaveClicked();
    void sCalculateExtendedPrice();
    void updatePriceInfo();
    void rejectEvent();

private:
	
    int      _mode;
    int      _raitemid;
    int	     _raheadid;
    int      _custid;
    int      _crmacctid;
    int	     _coitemid;
    int	     _shiptoid;
    QString  _creditmethod;
    double   _priceRatio;
    double   _listPriceCache;
    double   _salePriceCache;
    double   _qtyAuthCache;
    int	     _taxauthid;
    taxCache _taxCache;
    double	 _qtySoldCache;
    double   _qtycredited;
	int	     _dispositionCache;
    int      _invuomid;
    double   _qtyinvuomratio;
    double   _priceinvuomratio;

    int      _orderId;

    int      _availabilityLastItemid;
    int      _availabilityLastWarehousid;
    QDate    _availabilityLastSchedDate;
    bool     _availabilityLastShow;
    double   _availabilityQtyOrdered;
    int      _leadTime;
    QDate    _cScheduledDate;
    double   _cQtyOrdered;
    int      _preferredWarehouseid;
    QString  _status;
    double   _soldQty;

};

#endif // RETURNAUTHORIZATIONITEM_H
