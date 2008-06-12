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

#include "selectBillingQty.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

selectBillingQty::selectBillingQty(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_item,	SIGNAL(newId(int)),	this, SLOT(sHandleItem()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxType,	SIGNAL(newID(int)),	this, SLOT(sLookupTaxCode()));
  connect(_toBill, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleBillingQty()));

  _taxType->setEnabled(_privileges->check("OverrideTax"));
  _taxCode->setEnabled(_privileges->check("OverrideTax"));

  _taxauthid = -1;
}

selectBillingQty::~selectBillingQty()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectBillingQty::languageChange()
{
  retranslateUi(this);
}

enum SetResponse selectBillingQty::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _soitemid = param.toInt();

    _item->setReadOnly(TRUE);

    // get taxauth_id before item->setId()
    XSqlQuery taxauth;
    taxauth.prepare("SELECT cobmisc_taxauth_id "
		    "FROM coitem, cobmisc "
		    "WHERE ((coitem_cohead_id=cobmisc_cohead_id)"
		    "  AND  (coitem_id=:soitem_id));");
    taxauth.bindValue(":soitem_id", _soitemid);
    taxauth.exec();
    if (taxauth.first())
      _taxauthid=taxauth.value("cobmisc_taxauth_id").toInt();
    else if (taxauth.lastError().type() != QSqlError::None)
    {
      systemError(this, taxauth.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    XSqlQuery soitem;
    soitem.prepare( "SELECT itemsite_item_id, cust_partialship,"
                    "       cohead_number, coitem_linenumber,"
                    "       uom_name,"
                    "       formatQty(coitem_qtyord) AS f_qtyordered,"
                    "       formatQty(coitem_qtyshipped) AS f_qtyshipped,"
                    "       formatQty(coitem_qtyord - coitem_qtyshipped) AS f_qtybalance,"
                    "       (coitem_qtyord - coitem_qtyshipped) AS qtybalance,"
                    "       COALESCE(coitem_tax_id, -1) AS tax_id "
                    "FROM coitem, itemsite, cohead, cust, uom "
                    "WHERE ( (coitem_itemsite_id=itemsite_id)"
                    " AND (coitem_cohead_id=cohead_id)"
                    " AND (coitem_status <> 'X')"
                    " AND (coitem_qty_uom_id=uom_id)"
                    " AND (cohead_cust_id=cust_id)"
                    " AND (coitem_id=:soitem_id) )" );
    soitem.bindValue(":soitem_id", _soitemid);
    soitem.exec();
    if (soitem.first())
    {
      _cachedBalanceDue = soitem.value("qtybalance").toDouble();

      _item->setId(soitem.value("itemsite_item_id").toInt());
      _salesOrderNumber->setText(soitem.value("cohead_number").toString());
      _lineNumber->setText(soitem.value("coitem_linenumber").toString());
      _qtyUOM->setText(soitem.value("uom_name").toString());
      _ordered->setText(soitem.value("f_qtyordered").toString());
      _shipped->setText(soitem.value("f_qtyshipped").toString());
      _balance->setText(soitem.value("f_qtybalance").toString());
      _taxCode->setId(soitem.value("tax_id").toInt());

      _cachedPartialShip = soitem.value("cust_partialship").toBool();
      _closeLine->setChecked(!_cachedPartialShip);
      _closeLine->setEnabled(_cachedPartialShip);

      double uninvoiced;
      XSqlQuery coship;
      coship.prepare( "SELECT SUM(coship_qty) AS uninvoiced "
                      "FROM cosmisc, coship "
                      "WHERE ( (coship_cosmisc_id=cosmisc_id)"
                      " AND (NOT coship_invoiced)"
                      " AND (cosmisc_shipped)"
                      " AND (coship_coitem_id=:soitem_id) );" );
      coship.bindValue(":soitem_id", _soitemid);
      coship.exec();
      if (coship.first())
      {
        uninvoiced = coship.value("uninvoiced").toDouble();
        _uninvoiced->setText(formatQty(coship.value("uninvoiced").toDouble()));
      }
      else
      {
        uninvoiced = 0.0;
        _uninvoiced->setText(formatQty(0.0));
      }

      // take uninvoiced into account
      _cachedBalanceDue += uninvoiced;

      XSqlQuery cobill;
      cobill.prepare( "SELECT cobill_qty, cobill_toclose,"
		      "       cobill_taxtype_id, cobill_tax_id "
                      "FROM cobill, cobmisc "
                      "WHERE ( (cobill_cobmisc_id=cobmisc_id) "
		      "  AND   (NOT cobmisc_posted)"
                      "  AND   (cobill_coitem_id=:soitem_id) );" );
      cobill.bindValue(":soitem_id", _soitemid);
      cobill.exec();
      if (cobill.first())
      {
	_toBill->setText(formatQty(cobill.value("cobill_qty").toDouble()));

        if (soitem.value("cust_partialship").toBool())
	  _closeLine->setChecked(cobill.value("cobill_toclose").toBool());

	// overwrite automatically-found values if user previously set them
	_taxType->setId(cobill.value("cobill_taxtype_id").toInt());
	_taxCode->setId(cobill.value("cobill_tax_id").toInt());
      }
      else if (cobill.lastError().type() != QSqlError::None)
      {
	systemError(this, cobill.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
      else
      {
	_toBill->setText(formatQty(uninvoiced));

        if (soitem.value("cust_partialship").toBool())
	  _closeLine->setChecked((uninvoiced == _cachedBalanceDue));
      }

      _toBill->setSelection(0, _toBill->text().length());
    }
    else if (soitem.lastError().type() != QSqlError::None)
    {
      systemError(this, soitem.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  return NoError;
}

void selectBillingQty::sHandleBillingQty()
{
  if (_cachedPartialShip)
    _closeLine->setChecked((_toBill->toDouble() == _cachedBalanceDue));
}

void selectBillingQty::sSave()
{
  XSqlQuery select;
  select.prepare("SELECT selectForBilling(:soitem_id, :qty, :close, "
		 "                        :taxtypeid, :taxid) AS result;");
  select.bindValue(":soitem_id", _soitemid);
  select.bindValue(":qty",	 _toBill->toDouble());
  select.bindValue(":close",	 QVariant(_closeLine->isChecked(), 0));
  if(_taxType->isValid())
    select.bindValue(":taxtypeid", _taxType->id());
  if(_taxCode->isValid())
    select.bindValue(":taxid",   _taxCode->id());
  select.exec();
  if(select.first())
  {
    int result = select.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("selectForBilling", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (select.lastError().type() != QSqlError::None)
  {
    systemError(this, select.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sBillingSelectionUpdated(-1, _soitemid);

  accept();
}

void selectBillingQty::sLookupTaxCode()
{
  XSqlQuery taxq;
  taxq.prepare("SELECT getTaxSelection(:auth, :type) AS result;");
  taxq.bindValue(":auth",    _taxauthid);
  taxq.bindValue(":type",    _taxType->id());
  taxq.exec();
  if (taxq.first())
  {
    int result = taxq.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("getTaxSelection", result),
		  __FILE__, __LINE__);
      return;
    }
    _taxCode->setId(result);
  }
  else if (taxq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _taxCode->setId(-1);
}

void selectBillingQty::sHandleItem()
{
  XSqlQuery itemq;
  itemq.prepare("SELECT getItemTaxType(:item_id, :taxauth) AS result;");
  itemq.bindValue(":item_id", _item->id());
  itemq.bindValue(":taxauth", _taxauthid);
  itemq.exec();
  if (itemq.first())
  {
    int result = itemq.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("getItemTaxType", result),
		  __FILE__, __LINE__);
      return;
    }
    _taxType->setId(result);
  }
  else if (itemq.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _taxType->id(-1);
}
