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

#include "selectOrderForBilling.h"

#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <metasql.h>

#include "salesOrder.h"
#include "salesOrderList.h"
#include "selectBillingQty.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"

selectOrderForBilling::selectOrderForBilling(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_cancel, SIGNAL(clicked()), this, SLOT(sCancelSelection()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEditOrder()));
  connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  connect(_miscCharge, SIGNAL(valueChanged()), this, SLOT(sUpdateTotal()));
  connect(_salesTaxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sEditSelection()));
  connect(_selectBalance, SIGNAL(clicked()), this, SLOT(sSelectBalance()));
  connect(_showClosed, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_so, SIGNAL(newId(int)), this, SLOT(sPopulate(int)));
  connect(_so, SIGNAL(requestList()), this, SLOT(sSoList()));
  connect(_soList, SIGNAL(clicked()), this, SLOT(sSoList()));
  connect(_salesTax,	SIGNAL(valueChanged()),	this, SLOT(sUpdateTotal()));
  connect(_subtotal,	SIGNAL(valueChanged()),	this, SLOT(sUpdateTotal()));
  connect(_taxauth,	SIGNAL(newID(int)),	this, SLOT(sTaxAuthChanged()));

  statusBar()->hide();

#ifndef Q_WS_MAC
  _soList->setMaximumWidth(25);
#endif
  
  _cobmiscid = -1;
  _captive = FALSE;
  _updated = FALSE;

  _taxauthidCache	= -1;
  _taxCache.clear();

  _custCurrency->setLabel(_custCurrencyLit);

  _freight->clear();
  _payment->clear();

  _soitem->addColumn(tr("#"),          _seqColumn,   Qt::AlignCenter );
  _soitem->addColumn(tr("Item"),       -1,           Qt::AlignLeft   );
  _soitem->addColumn(tr("Whs."),       _whsColumn,   Qt::AlignCenter );
  _soitem->addColumn(tr("UOM"),        _uomColumn,   Qt::AlignLeft   );
  _soitem->addColumn(tr("Ordered"),    _qtyColumn,   Qt::AlignRight  );
  _soitem->addColumn(tr("Shipped"),    _qtyColumn,   Qt::AlignRight  );
  _soitem->addColumn(tr("Returned"),   _qtyColumn,   Qt::AlignRight  );
  _soitem->addColumn(tr("Uninvoiced"), _qtyColumn,   Qt::AlignRight  );
  _soitem->addColumn(tr("Selected"),   _qtyColumn,   Qt::AlignRight  );
  _soitem->addColumn(tr("Extended"),   _moneyColumn, Qt::AlignRight  );
  _soitem->addColumn(tr("Close"),      _ynColumn,    Qt::AlignCenter );

  if (_privileges->check("MaintainSalesOrders"))
    connect(_so, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));

  if(!_privileges->check("AllowSelectOrderEditing"))
  {
    _miscCharge->setEnabled(false);
    _miscChargeDescription->setEnabled(false);
    _miscChargeAccount->setEnabled(false);
  }
  else
  {
    connect(_soitem, SIGNAL(valid(bool)), _select, SLOT(setEnabled(bool)));
    connect(_soitem, SIGNAL(valid(bool)), _cancel, SLOT(setEnabled(bool)));
  }
}

selectOrderForBilling::~selectOrderForBilling()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectOrderForBilling::languageChange()
{
  retranslateUi(this);
}

void selectOrderForBilling::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _so->setId(param.toInt());
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _so->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _close->setText(tr("&Cancel"));

      _save->setFocus();
    }
  }
}

void selectOrderForBilling::clear()
{
  _so->setId(-1);
  _cobmiscid = -1;
  _orderDate->clear();
  _shipDate->clear();
  _invoiceDate->clear();
  _custName->clear();
  _poNumber->clear();
  _shipToName->clear();
  _custCurrency->setId(-1);
  _taxauth->setId(-1);
  _taxCache.clear();
  _taxauthidCache = -1;
  _soitem->clear();
  _shipvia->clear();
  _miscChargeAccount->setId(-1);
  _miscChargeDescription->clear();
  _comments->clear();
  _subtotal->clear();
  _miscCharge->clear();
  _freight->clear();
  _salesTax->clear();
  _total->clear();
  _payment->clear();
}

void selectOrderForBilling::sSave()
{
  if (!_shipDate->isValid())
  {
    QMessageBox::information(this, tr("No Ship Date Entered"),
                             tr("<p>You must enter a Ship Date before "
				"selecting this order for billing."  ) );

    _shipDate->setFocus();
    return;
  }

  if ( (! _miscCharge->isZero()) && (!_miscChargeAccount->isValid()) )
  {
    QMessageBox::warning( this, tr("No Misc. Charge Account Number"),
                          tr("<p>You may not enter a Misc. Charge without "
			     "indicating the G/L Sales Account number for the "
			     "charge. Please set the Misc. Charge amount to 0 "
			     "or select a Misc. Charge Sales Account." ) );
    _miscChargeAccount->setFocus();
    return;
  }
  
  if (_cobmiscid != -1)
  {
    q.prepare( "UPDATE cobmisc "
               "SET cobmisc_freight=:cobmisc_freight,"
               "    cobmisc_payment=:cobmisc_payment,"
	       "    cobmisc_taxauth_id=:cobmisc_taxauth_id,"
	       "    cobmisc_notes=:cobmisc_notes,"
               "    cobmisc_shipdate=:cobmisc_shipdate, cobmisc_invcdate=:cobmisc_invcdate,"
               "    cobmisc_shipvia=:cobmisc_shipvia, cobmisc_closeorder=:cobmisc_closeorder,"
               "    cobmisc_misc=:cobmisc_misc, cobmisc_misc_accnt_id=:cobmisc_misc_accnt_id,"
               "    cobmisc_misc_descrip=:cobmisc_misc_descrip, "
	       "    cobmisc_curr_id=:cobmisc_curr_id,"
               "    cobmisc_tax_curr_id=:tax_curr_id,"
               "    cobmisc_adjtax_id=:adjtax_id,"
               "    cobmisc_adjtaxtype_id=:adjtaxtype_id,"
               "    cobmisc_adjtax_pcta=:adjtax_pcta,"
               "    cobmisc_adjtax_pctb=:adjtax_pctb,"
               "    cobmisc_adjtax_pctc=:adjtax_pctc,"
               "    cobmisc_adjtax_ratea=:adjtax_ratea,"
               "    cobmisc_adjtax_rateb=:adjtax_rateb,"
               "    cobmisc_adjtax_ratec=:adjtax_ratec,"
               "    cobmisc_freighttax_id=:freighttax_id,"
               "    cobmisc_freighttaxtype_id=:freighttaxtype_id,"
               "    cobmisc_freighttax_pcta=:freighttax_pcta,"
               "    cobmisc_freighttax_pctb=:freighttax_pctb,"
               "    cobmisc_freighttax_pctc=:freighttax_pctc,"
               "    cobmisc_freighttax_ratea=:freighttax_ratea,"
               "    cobmisc_freighttax_rateb=:freighttax_rateb,"
               "    cobmisc_freighttax_ratec=:freighttax_ratec "
               "WHERE (cobmisc_id=:cobmisc_id);" );
    q.bindValue(":cobmisc_id", _cobmiscid);
    q.bindValue(":cobmisc_freight", _freight->localValue());
    q.bindValue(":cobmisc_payment", _payment->localValue());

    if (_taxauth->isValid())
      q.bindValue(":cobmisc_taxauth_id", _taxauth->id());

    q.bindValue(":cobmisc_notes", _comments->text());
    q.bindValue(":cobmisc_shipdate", _shipDate->date());
    q.bindValue(":cobmisc_invcdate", _invoiceDate->date());
    q.bindValue(":cobmisc_shipvia", _shipvia->text());
    q.bindValue(":cobmisc_closeorder", QVariant(_closeOpenItems->isChecked(), 0));
    q.bindValue(":cobmisc_misc", _miscCharge->localValue());
    q.bindValue(":cobmisc_misc_accnt_id", _miscChargeAccount->id());
    q.bindValue(":cobmisc_misc_descrip", _miscChargeDescription->text().stripWhiteSpace());
    q.bindValue(":cobmisc_curr_id",	_custCurrency->id());

    q.bindValue(":tax_curr_id",	_custCurrency->id());

    if (_taxCache.adjId() > 0)
      q.bindValue(":adjtax_id",		_taxCache.adjId());
    if (_taxCache.adjType() > 0)
      q.bindValue(":adjtaxtype_id",	_taxCache.adjType());
    q.bindValue(":adjtax_pcta",		_taxCache.adjPct(0));
    q.bindValue(":adjtax_pctb",		_taxCache.adjPct(1));
    q.bindValue(":adjtax_pctc",		_taxCache.adjPct(2));
    q.bindValue(":adjtax_ratea",	_taxCache.adj(0));
    q.bindValue(":adjtax_rateb",	_taxCache.adj(1));
    q.bindValue(":adjtax_ratec",	_taxCache.adj(2));

    if (_taxCache.freightId() > 0)
      q.bindValue(":freighttax_id",	_taxCache.freightId());
    if (_taxCache.freightType() > 0)
      q.bindValue(":freighttaxtype_id",	_taxCache.freightType());
    q.bindValue(":freighttax_pcta",	_taxCache.freightPct(0));
    q.bindValue(":freighttax_pctb",	_taxCache.freightPct(1));
    q.bindValue(":freighttax_pctc",	_taxCache.freightPct(2));
    q.bindValue(":freighttax_ratea",	_taxCache.freight(0));
    q.bindValue(":freighttax_rateb",	_taxCache.freight(1));
    q.bindValue(":freighttax_ratec",	_taxCache.freight(2));

    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
    }
  }

  omfgThis->sBillingSelectionUpdated(_so->id(), TRUE);

  if (_captive)
    close();
  else
  {
    clear();
    _so->setFocus();
  }
}

void selectOrderForBilling::sSoList()
{
  ParameterList params;
  params.append("sohead_id", _so->id());

  if (_showClosed->isChecked())
    params.append("soType", (cSoOpen || cSoClosed));
  else
    params.append("soType", cSoOpen);

  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  _so->setId(newdlg.exec());
}

void selectOrderForBilling::sPopulate(int pSoheadid)
{
  if (_so->isValid())
  {
    q.prepare("SELECT createBillingHeader(:sohead_id) AS cobmisc_id;");
    q.bindValue(":sohead_id", pSoheadid);
    q.exec();
    if (q.first())
    {
      _cobmiscid = q.value("cobmisc_id").toInt();
      if (_cobmiscid < 0)
      {
	systemError(this, storedProcErrorLookup("createBillingHeader", _cobmiscid),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    XSqlQuery cobmisc;
    cobmisc.prepare( "SELECT cobmisc_id, cobmisc_notes, cobmisc_shipvia,"
                     "       cohead_orderdate, cobmisc_shipdate,"
		     "       cobmisc_invcdate, cobmisc_taxauth_id,"
                     "       cobmisc_freight AS freight,"
                     "       cobmisc_tax AS f_tax,"
                     "       cobmisc_adjtax_id, cobmisc_adjtax_ratea,"
		     "       cobmisc_adjtax_rateb, cobmisc_adjtax_ratec,"
		     "       cobmisc_adjtaxtype_id,"
                     "       cobmisc_freighttax_id, cobmisc_freighttax_ratea,"
		     "       cobmisc_freighttax_rateb, cobmisc_freighttax_ratec,"
		     "       cobmisc_freighttaxtype_id,"
                     "       cobmisc_misc, cobmisc_misc_accnt_id, cobmisc_misc_descrip,"
                     "       cobmisc_payment AS payment,"
                     "       cobmisc_closeorder,"
                     "       cobmisc_curr_id,"
                     "       cohead_number, cohead_shipto_id,"
                     "       cohead_custponumber,"
                     "       cohead_billtoname, cohead_shiptoname,"
                     "       CASE WHEN (shipchrg_custfreight IS NULL) THEN TRUE"
                     "            ELSE shipchrg_custfreight"
                     "       END AS custfreight "
                     "FROM cobmisc, cohead LEFT OUTER JOIN shipchrg ON (cohead_shipchrg_id=shipchrg_id) "
                     "WHERE ( (cobmisc_id=:cobmisc_id)"
                     " AND (cohead_id=:cohead_id) );" );
    cobmisc.bindValue(":cobmisc_id", _cobmiscid);
    cobmisc.bindValue(":cohead_id", pSoheadid);
    cobmisc.exec();
    if (cobmisc.first())
    {
      _cobmiscid = cobmisc.value("cobmisc_id").toInt();
      // do taxauth first so we can overwrite the result of the signal cascade
      _taxauth->setId(cobmisc.value("cobmisc_taxauth_id").toInt());

      _orderDate->setDate(cobmisc.value("cohead_orderdate").toDate(), true);
      _shipDate->setDate(cobmisc.value("cobmisc_shipdate").toDate());
      _invoiceDate->setDate(cobmisc.value("cobmisc_invcdate").toDate());

      _poNumber->setText(cobmisc.value("cohead_custponumber").toString());
      _custName->setText(cobmisc.value("cohead_billtoname").toString());
      _shipToName->setText(cobmisc.value("cohead_shiptoname").toString());
      _shipvia->setText(cobmisc.value("cobmisc_shipvia").toString());
      _salesTax->setLocalValue(cobmisc.value("f_tax").toDouble());

      _taxCache.setAdjId(cobmisc.value("cobmisc_adjtax_id").toInt());
      _taxCache.setAdjType(cobmisc.value("cobmisc_adjtaxtype_id").toInt());
      _taxCache.setAdj(cobmisc.value("cobmisc_adjtax_ratea").toDouble(),
			cobmisc.value("cobmisc_adjtax_rateb").toDouble(),
			cobmisc.value("cobmisc_adjtax_ratec").toDouble());
 
      _taxCache.setFreightId(cobmisc.value("cobmisc_freighttax_id").toInt());
      _taxCache.setFreightType(cobmisc.value("cobmisc_freighttaxtype_id").toInt());
      _taxCache.setFreight(cobmisc.value("cobmisc_freighttax_ratea").toDouble(),
			  cobmisc.value("cobmisc_freighttax_rateb").toDouble(),
			  cobmisc.value("cobmisc_freighttax_ratec").toDouble());

      _miscCharge->setLocalValue(cobmisc.value("cobmisc_misc").toDouble());
      _miscChargeAccount->setId(cobmisc.value("cobmisc_misc_accnt_id").toInt());
      _miscChargeDescription->setText(cobmisc.value("cobmisc_misc_descrip"));
      _payment->set(cobmisc.value("payment").toDouble(),
		    cobmisc.value("cobmisc_curr_id").toInt(),
		    cobmisc.value("cohead_orderdate").toDate(), false);
      _custCurrency->setId(cobmisc.value("cobmisc_curr_id").toInt());
      _comments->setText(cobmisc.value("cobmisc_notes").toString());
      _closeOpenItems->setChecked(cobmisc.value("cobmisc_closeorder").toBool());

      if (cobmisc.value("custfreight").toBool())
      {
        _freight->setEnabled(TRUE);
        _freight->setLocalValue(cobmisc.value("freight").toDouble());
      }
      else
      {
        _freight->setEnabled(FALSE);
        _freight->clear();
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    clear();
  }

  sFillList();
}

void selectOrderForBilling::sEditOrder()
{
  salesOrder::editSalesOrder(_so->id(), false);
}

void selectOrderForBilling::sEditSelection()
{
  ParameterList params;
  params.append("soitem_id", _soitem->id());

  selectBillingQty newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillList();
    _updated = TRUE;
  }
}

void selectOrderForBilling::sCancelSelection()
{
  q.prepare( "DELETE FROM cobill "
             "WHERE ((cobill_coitem_id=:coitem_id)"
             " AND (SELECT NOT cobmisc_posted"
             "      FROM cobmisc"
             "      WHERE (cobill_cobmisc_id=cobmisc_id) ) ) ");
  q.bindValue(":coitem_id", _soitem->id());
  q.exec();

  sFillList();
}

void selectOrderForBilling::sSelectBalance()
{
  q.prepare("SELECT selectBalanceForBilling(:sohead_id) AS result;");
  q.bindValue(":sohead_id", _so->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("selectBalanceForBilling", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void selectOrderForBilling::sFreightChanged()
{
  XSqlQuery tax;
  tax.prepare( "SELECT calculateTax( :taxid, :freight, 0.0, 'A') AS ratea,"
               "       calculateTax( :taxid, :freight, 0.0, 'B') AS rateb,"
               "       calculateTax( :taxid, :freight, 0.0, 'C') AS ratec;");
  tax.bindValue(":taxid",	_taxCache.freightId());
  tax.bindValue(":freight",	_freight->localValue());
  tax.exec();
  if(tax.first())
  {
    _taxCache.setFreight(tax.value("ratea").toDouble(),
			 tax.value("rateb").toDouble(),
			 tax.value("ratec").toDouble());
  }
  else
  {
    if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

    _taxCache.setFreight(0, 0, 0);
  }
  recalculateTax();
}

void selectOrderForBilling::recalculateTax()
{
  XSqlQuery tax;
  tax.prepare( "SELECT SUM(cobill_tax_ratea) AS ratea,"
	       "       SUM(cobill_tax_rateb) AS rateb,"
	       "       SUM(cobill_tax_ratec) AS ratec "
               "  FROM cobill "
               " WHERE (cobill_cobmisc_id=:cobmisc_id);");
  tax.bindValue(":cobmisc_id", _cobmiscid);
  tax.exec();
  if (tax.first())
  {
    _taxCache.setLine(tax.value("ratea").toDouble(),
		      tax.value("rateb").toDouble(),
		      tax.value("ratec").toDouble());

  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _salesTax->setLocalValue(_taxCache.total());
  // changing _salesTax fires sUpdateTotal()
}

void selectOrderForBilling::sUpdateTotal()
{
  _total->setLocalValue( _subtotal->localValue() +
                                _salesTax->localValue() +
                                _miscCharge->localValue() +
                                _freight->localValue() );
}

void selectOrderForBilling::sFillList()
{
  _soitem->clear();

  if (_so->isValid())
  {
    QString sql( "SELECT coitem_id, coitem_linenumber,"
                 "       item_number, iteminvpricerat(item_id) AS invpricerat,"
                 "       warehous_code, coitem_price,"
                 "       uom_name,"
                 "       formatQty(coitem_qtyord) AS f_qtyord,"
                 "       formatQty(coitem_qtyshipped) AS f_qtyshipped,"
                 "       formatQty(coitem_qtyreturned) AS f_qtyreturned,"
                 "       formatQty( ( SELECT COALESCE(SUM(coship_qty), 0)"
                 "                    FROM coship "
                 "                    WHERE ( (coship_coitem_id=coitem_id)"
                 "                     AND (NOT coship_invoiced) ) )"
                 "                 ) AS f_qtyatship,"
                 "       ( SELECT COALESCE(SUM(cobill_qty), 0)"
                 "         FROM cobill, cobmisc "
                 "         WHERE ( (cobill_cobmisc_id=cobmisc_id)"
                 "          AND (cobill_coitem_id=coitem_id)"
                 "          AND (NOT cobmisc_posted) ) ) AS qtytobill,"
                 "       round(( "
                 "         ( SELECT COALESCE(SUM(cobill_qty), 0)"
                 "           FROM cobill, cobmisc "
                 "           WHERE ( (cobill_cobmisc_id=cobmisc_id)"
                 "            AND (cobill_coitem_id=coitem_id)"
                 "            AND (NOT cobmisc_posted) ) ) * coitem_qty_invuomratio "
                 "           * ( coitem_price / coitem_price_invuomratio)), 2) AS extended, "
                 "       formatMoney(round(( "
                 "         ( SELECT COALESCE(SUM(cobill_qty), 0)"
                 "           FROM cobill, cobmisc "
                 "           WHERE ( (cobill_cobmisc_id=cobmisc_id)"
                 "            AND (cobill_coitem_id=coitem_id)"
                 "            AND (NOT cobmisc_posted) ) ) * coitem_qty_invuomratio "
                 "          * (coitem_price / coitem_price_invuomratio)), 2)) AS f_extended, "
                 "       formatBoolYN( ( SELECT COALESCE(cobill_toclose, FALSE)"
                 "                       FROM cobill, cobmisc "
                 "                       WHERE ((cobill_cobmisc_id=cobmisc_id)"
                 "                        AND (cobill_coitem_id=coitem_id)"
                 "                        AND (NOT cobmisc_posted))"
                 "                       ORDER BY cobill_toclose DESC"
                 "                       LIMIT 1) ) AS toclose "
                 "FROM coitem, itemsite, item, warehous, uom "
                 "WHERE ( (coitem_itemsite_id=itemsite_id)"
                 " AND (coitem_status <> 'X')"
                 " AND (coitem_qty_uom_id=uom_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (itemsite_warehous_id=warehous_id)"
		 " <? if exists(\"showOpenOnly\") ?>"
		 " AND (coitem_status <> 'C')"
		 " <? endif ?>"
		 " AND (coitem_cohead_id=<? value(\"sohead_id\") ?>) ) "
		 "ORDER BY coitem_linenumber;" );

    ParameterList params;
    if (!_showClosed->isChecked())
      params.append("showOpenOnly");
    params.append("sohead_id", _so->id());
    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);
    if (q.first())
    {
      double subtotal = 0.0;

      XTreeWidgetItem* last = 0;
      do
      {
        subtotal += q.value("extended").toDouble();

        last = new XTreeWidgetItem(_soitem, last, q.value("coitem_id").toInt(),
                            q.value("coitem_linenumber"), q.value("item_number"),
                            q.value("warehous_code"), q.value("uom_name"), q.value("f_qtyord"),
                            q.value("f_qtyshipped"), q.value("f_qtyreturned"),
                            q.value("f_qtyatship"), formatQty(q.value("qtytobill").toDouble()),
                            q.value("f_extended"), q.value("toclose") );
      }
      while (q.next());

      _subtotal->setLocalValue(subtotal);
    }
    else
    {
      if (q.lastError().type() != QSqlError::None)
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      _subtotal->clear();
    }

    recalculateTax();
  }
}

void selectOrderForBilling::sHandleShipchrg(int pShipchrgid)
{
  XSqlQuery query;
  query.prepare( "SELECT shipchrg_custfreight "
                 "FROM shipchrg "
                 "WHERE (shipchrg_id=:shipchrg_id);" );
  query.bindValue(":shipchrg_id", pShipchrgid);
  query.exec();
  if (query.first())
  {
    if (query.value("shipchrg_custfreight").toBool())
      _freight->setEnabled(TRUE);
    else
    {
      _freight->setEnabled(FALSE);
      _freight->clear();
    }
  }
  else if (query.lastError().type() != QSqlError::None)
  {
    systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void selectOrderForBilling::sTaxDetail()
{
  XSqlQuery taxq;
  taxq.prepare("UPDATE cobmisc SET cobmisc_taxauth_id=:taxauth,"
	       "  cobmisc_freight=:freight,"
	       "  cobmisc_freighttax_ratea=:freighta,"
	       "  cobmisc_freighttax_rateb=:freightb,"
	       "  cobmisc_freighttax_ratec=:freightc,"
	       "  cobmisc_invcdate=:invcdate "
	       "WHERE (cobmisc_id=:cobmisc_id);");
  if(_taxauth->isValid())
    taxq.bindValue(":taxauth",	_taxauth->id());
  taxq.bindValue(":freight",	_freight->localValue());
  taxq.bindValue(":freighta",	_taxCache.freight(0));
  taxq.bindValue(":freightb",	_taxCache.freight(1));
  taxq.bindValue(":freightc",	_taxCache.freight(2));
  taxq.bindValue(":cobmisc_id",	_cobmiscid);
  taxq.bindValue(":invcdate",   _invoiceDate->date());
  taxq.exec();
  if (taxq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("order_id",	_cobmiscid);
  params.append("order_type",	"B");
  params.append("mode",		"edit");

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    sPopulate(_so->id());
}

void selectOrderForBilling::sTaxAuthChanged()
{
  if (_cobmiscid == -1)
    return;

  XSqlQuery taxauthq;
  taxauthq.prepare("SELECT changeCobTaxAuth(:cobmisc_id, :taxauth_id) AS result;");
  taxauthq.bindValue(":cobmisc_id", _cobmiscid);
  taxauthq.bindValue(":taxauth_id", _taxauth->id());
  taxauthq.exec();
  if (taxauthq.first())
  {
    int result = taxauthq.value("result").toInt();
    if (result < 0)
    {
      _taxauth->setId(_taxauthidCache);
      systemError(this, storedProcErrorLookup("changeCobTaxAuth", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (taxauthq.lastError().type() != QSqlError::None)
  {
    _taxauth->setId(_taxauthidCache);
    systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _taxauthidCache = _taxauth->id();

  taxauthq.prepare("SELECT cobmisc_freighttax_id, cobmisc_adjtax_ratea,"
		   "       cobmisc_adjtax_rateb, cobmisc_adjtax_ratec "
		   "FROM cobmisc "
		   "WHERE (cobmisc_id=:cobmisc_id);");
  taxauthq.bindValue(":cobmisc_id", _cobmiscid);
  taxauthq.exec();
  if (taxauthq.first())
  {
    if (taxauthq.value("cobmisc_freighttax_id").isNull())
      _taxCache.setFreightId(-1);
    else
      _taxCache.setFreightId(taxauthq.value("cobmisc_freighttax_id").toInt());
    _taxCache.setAdj(taxauthq.value("cobmisc_adjtax_ratea").toDouble(),
		     taxauthq.value("cobmisc_adjtax_rateb").toDouble(),
		     taxauthq.value("cobmisc_adjtax_ratec").toDouble());
  }
  else if (taxauthq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFreightChanged();
}

void selectOrderForBilling::closeEvent(QCloseEvent * pEvent)
{
  q.prepare("SELECT releaseUnusedBillingHeader(:cobmisc_id) AS result;");
  q.bindValue(":cobmisc_id", _cobmiscid);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < -2) // don't bother the user with -1:posted or -2:has-lines
      systemError(this, storedProcErrorLookup("releaseUnusedBillingHeader", result),
		  __FILE__, __LINE__);
  }
  else if (q.lastError().type() != QSqlError::None)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  XMainWindow::closeEvent(pEvent);
}
