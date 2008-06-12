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

#include "taxBreakdown.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "taxCache.h"
#include "taxDetail.h"

taxBreakdown::taxBreakdown(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _currency->setLabel(_currencyLit);
  _taxcurrency->setLabel(_taxcurrencyLit);

  connect(_adjTaxLit,    SIGNAL(leftClickedURL(QString)),this, SLOT(sAdjTaxDetail()));
  connect(_freightTaxLit,SIGNAL(leftClickedURL(QString)),this, SLOT(sFreightTaxDetail()));
  connect(_lineTaxLit,   SIGNAL(leftClickedURL(QString)),this, SLOT(sLineTaxDetail()));
  connect(_save,         SIGNAL(clicked()),              this, SLOT(sSave()));
  connect(_totalTaxLit,  SIGNAL(leftClickedURL(QString)),this, SLOT(sTotalTaxDetail()));

  _orderid	= -1;
  _ordertype	= "";
  _taxCache.clear();
}

taxBreakdown::~taxBreakdown()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxBreakdown::languageChange()
{
  retranslateUi(this);
}

SetResponse taxBreakdown::set(const ParameterList& pParams)
{
  QVariant param;
  bool	   valid;

  param = pParams.value("order_id", &valid);
  if (valid)
    _orderid = param.toInt();

  param = pParams.value("order_type", &valid);
  if (valid)
    _ordertype = param.toString();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if(param.toString() == "view")
    {
      _mode = cView;
      _save->hide();
    }
    else
      _mode = cEdit;
  }

  ParameterList params;
  if (_ordertype == "I")
  {
    _currencyLit->setText(tr("Invoice Currency:"));
    _header->setText(tr("Tax Breakdown for Invoice:"));
    _totalLit->setText(tr("Invoice Total:"));

    params.append("invchead_id", _orderid);
  }
  else if (_ordertype == "S")
  {
    _currencyLit->setText(tr("Sales Order Currency:"));
    _header->setText(tr("Tax Breakdown for Sales Order:"));
    _totalLit->setText(tr("Sales Order Total:"));

    _adjTaxLit->setVisible(false);
    _adjTax->setVisible(false);

    params.append("cohead_id", _orderid);
  }
  else if (_ordertype == "Q")
  {
    _currencyLit->setText(tr("Quote Currency:"));
    _header->setText(tr("Tax Breakdown for Quote:"));
    _totalLit->setText(tr("Quote Total:"));

    _adjTaxLit->setVisible(false);
    _adjTax->setVisible(false);

    params.append("quhead_id", _orderid);
  }
  else if (_ordertype == "RA")
  {
    _currencyLit->setText(tr("Return Authorization Currency:"));
    _header->setText(tr("Tax Breakdown for Return:"));
    _totalLit->setText(tr("Return Total:"));

    _adjTaxLit->setVisible(false);
    _adjTax->setVisible(false);

    params.append("rahead_id", _orderid);
  }
  else if (_ordertype == "B")
  {
    _currencyLit->setText(tr("Billing Currency:"));
    _header->setText(tr("Tax Breakdown for Billing Order:"));
    _totalLit->setText(tr("Billing Total:"));

    params.append("cobmisc_id", _orderid);
  }
  else if (_ordertype == "CM")
  {
    _currencyLit->setText(tr("Credit Memo Currency:"));
    _header->setText(tr("Tax Breakdown for Credit Memo:"));
    _totalLit->setText(tr("Credit Memo Total:"));

    params.append("cmhead_id", _orderid);
  }
  else if (_ordertype == "TO")
  {
    _currencyLit->setText(tr("Transfer Order Currency:"));
    _header->setText(tr("Tax Breakdown for Transfer Order:"));
    _totalLit->setText(tr("Transfer Order Total:"));

    params.append("tohead_id", _orderid);
  }

  QString sql =
	"<? if exists(\"invchead_id\") ?>"
	"  SELECT invchead_invcnumber AS number,"
	"         invchead_taxauth_id AS taxauth_id,"
	"         invchead_curr_id AS curr_id,"
	"         invchead_tax_curr_id AS tax_curr_id,"
	"         invchead_invcdate AS date,"
	"         SUM(ROUND(((invcitem_billed * invcitem_qty_invuomratio) * (invcitem_price) / "
	"                  (CASE WHEN(item_id IS NULL) THEN 1 "
	"			ELSE invcitem_price_invuomratio END)), 2)) AS line,"
	"         SUM(invcitem_tax_ratea) AS linea,"
	"         SUM(invcitem_tax_rateb) AS lineb,"
	"         SUM(invcitem_tax_ratec) AS linec,"
	"         invchead_freight AS freight,"
	"         invchead_freighttax_id AS freightTax,"
	"         invchead_freighttax_pcta AS freightPcta,"
	"         invchead_freighttax_pctb AS freightPctb,"
	"         invchead_freighttax_pctc AS freightPctc,"
	"         invchead_freighttax_ratea AS freighta,"
	"         invchead_freighttax_rateb AS freightb,"
	"         invchead_freighttax_ratec AS freightc,"
	"         invchead_adjtax_id AS adjTax,"
	"         invchead_adjtax_pcta AS adjPcta,"
	"         invchead_adjtax_pctb AS adjPctb,"
	"         invchead_adjtax_pctc AS adjPctc,"
	"         invchead_adjtax_ratea AS adja,"
	"         invchead_adjtax_rateb AS adjb,"
	"         invchead_adjtax_ratec AS adjc"
	"  FROM invcitem LEFT OUTER JOIN"
	"       invchead ON (invcitem_invchead_id=invchead_id) LEFT OUTER JOIN"
	"       item ON (invcitem_item_id=item_id)"
	"  WHERE (invcitem_invchead_id=<? value(\"invchead_id\") ?>)"
	"  GROUP BY number, taxauth_id, curr_id, tax_curr_id,"
	"         date, freight, freightTax,"
	"         freightPcta, freightPctb, freightPctc,"
	"         freighta, freightb, freightc,"
	"         adjTax, adjPcta, adjPctb, adjPctc, adja, adjb, adjc;"
	"<? elseif exists(\"cmhead_id\") ?>"
	"  SELECT cmhead_number AS number,"
	"         cmhead_taxauth_id AS taxauth_id,"
	"         cmhead_curr_id AS curr_id,"
	"         cmhead_tax_curr_id AS tax_curr_id,"
	"         cmhead_docdate AS date,"
	"         SUM(ROUND((cmitem_qtycredit * cmitem_qty_invuomratio) * (cmitem_unitprice / "
	"                  (CASE WHEN(item_id IS NULL) THEN 1 "
	"			ELSE cmitem_price_invuomratio END)), 2)) AS line,"
	"         SUM(cmitem_tax_ratea) AS linea,"
	"         SUM(cmitem_tax_rateb) AS lineb,"
	"         SUM(cmitem_tax_ratec) AS linec,"
	"         cmhead_freight AS freight,"
	"         cmhead_freighttax_id AS freightTax,"
	"         cmhead_freighttax_pcta AS freightPcta,"
	"         cmhead_freighttax_pctb AS freightPctb,"
	"         cmhead_freighttax_pctc AS freightPctc,"
	"         cmhead_freighttax_ratea AS freighta,"
	"         cmhead_freighttax_rateb AS freightb,"
	"         cmhead_freighttax_ratec AS freightc,"
	"         cmhead_adjtax_id AS adjTax,"
	"         cmhead_adjtax_pcta AS adjPcta,"
	"         cmhead_adjtax_pctb AS adjPctb,"
	"         cmhead_adjtax_pctc AS adjPctc,"
	"         cmhead_adjtax_ratea AS adja,"
	"         cmhead_adjtax_rateb AS adjb,"
	"         cmhead_adjtax_ratec AS adjc"
	"  FROM cmitem JOIN"
	"       cmhead ON (cmitem_cmhead_id=cmhead_id) LEFT OUTER JOIN"
	"       itemsite ON (cmitem_itemsite_id=itemsite_id) LEFT OUTER JOIN"
	"       item ON (itemsite_item_id=item_id)"
	"  WHERE (cmitem_cmhead_id=<? value(\"cmhead_id\") ?>)"
	"  GROUP BY number, taxauth_id, curr_id, tax_curr_id,"
	"         date, freight, freightTax,"
	"         freightPcta, freightPctb, freightPctc,"
	"         freighta, freightb, freightc,"
	"         adjTax, adjPcta, adjPctb, adjPctc, adja, adjb, adjc;"
	"<? elseif exists(\"cobmisc_id\") ?>"
	"  SELECT cohead_number AS number,"
	"         cobmisc_taxauth_id AS taxauth_id,"
	"         cobmisc_curr_id AS curr_id,"
	"         cobmisc_tax_curr_id AS tax_curr_id,"
	"         cobmisc_invcdate AS date,"
	"         SUM(ROUND((cobill_qty * coitem_price) / "
	"                  (CASE WHEN(item_id IS NULL) THEN 1 "
	"			ELSE coitem_price_invuomratio END), 2)) AS line,"
	"         SUM(cobill_tax_ratea) AS linea,"
	"         SUM(cobill_tax_rateb) AS lineb,"
	"         SUM(cobill_tax_ratec) AS linec,"
	"         cobmisc_freight AS freight,"
	"         cobmisc_freighttax_id AS freightTax,"
	"         cobmisc_freighttax_pcta AS freightPcta,"
	"         cobmisc_freighttax_pctb AS freightPctb,"
	"         cobmisc_freighttax_pctc AS freightPctc,"
	"         cobmisc_freighttax_ratea AS freighta,"
	"         cobmisc_freighttax_rateb AS freightb,"
	"         cobmisc_freighttax_ratec AS freightc,"
	"         cobmisc_adjtax_id AS adjTax,"
	"         cobmisc_adjtax_pcta AS adjPcta,"
	"         cobmisc_adjtax_pctb AS adjPctb,"
	"         cobmisc_adjtax_pctc AS adjPctc,"
	"         cobmisc_adjtax_ratea AS adja,"
	"         cobmisc_adjtax_rateb AS adjb,"
	"         cobmisc_adjtax_ratec AS adjc"
	"  FROM coitem JOIN cobill ON (cobill_coitem_id=coitem_id) JOIN"
	"       cobmisc ON (cobill_cobmisc_id=cobmisc_id) JOIN"
	"       cohead  ON (coitem_cohead_id=cohead_id) LEFT OUTER JOIN"
	"       itemsite ON (coitem_itemsite_id=itemsite_id) LEFT OUTER JOIN"
	"       item ON (itemsite_item_id=item_id)"
	"  WHERE (cobill_cobmisc_id=<? value(\"cobmisc_id\") ?>)"
	"  GROUP BY number, cobmisc_taxauth_id, curr_id, tax_curr_id,"
	"         date, freight, freightTax,"
	"         freightPcta, freightPctb, freightPctc,"
	"         freighta, freightb, freightc,"
	"         adjTax, adjPcta, adjPctb, adjPctc, adja, adjb, adjc;"
	"<? elseif exists(\"cohead_id\") ?>"
	"  SELECT cohead_number AS number,"
	"         cohead_taxauth_id AS taxauth_id,"
	"         cohead_curr_id AS curr_id,"
	"         cohead_curr_id AS tax_curr_id,"
	"         cohead_orderdate AS date,"
	"         SUM(ROUND((coitem_qtyord * coitem_qty_invuomratio) * (coitem_price / coitem_price_invuomratio),"
	"                    2)) AS line,"
	"         SUM(ROUND((calculateTax(coitem_tax_id,"
	"                        (ROUND((coitem_qtyord * coitem_qty_invuomratio) * (coitem_price /"
	"                        coitem_price_invuomratio), 2)), 0, 'A')), 2)) AS linea,"
	"         SUM(ROUND((calculateTax(coitem_tax_id,"
	"                        (ROUND((coitem_qtyord * coitem_qty_invuomratio) * (coitem_price /"
	"                        coitem_price_invuomratio), 2)), 0, 'B')), 2)) AS lineb,"
	"         SUM(ROUND((calculateTax(coitem_tax_id,"
	"                        (ROUND((coitem_qtyord * coitem_qty_invuomratio) * (coitem_price /"
	"                        coitem_price_invuomratio), 2)), 0, 'C')), 2)) AS linec,"
	"         cohead_freight AS freight,"
	"         tax_id AS freightTax,"
	"         tax_ratea AS freightPcta,"
	"         tax_rateb AS freightPctb,"
	"         tax_ratec AS freightPctc,"
	"         ROUND(calculateTax(tax_id, ROUND(cohead_freight, 2),"
	"                         0, 'A'), 2) AS freighta,"
	"         ROUND(calculateTax(tax_id, ROUND(cohead_freight, 2),"
	"                         0, 'B'), 2) AS freightb,"
	"         ROUND(calculateTax(tax_id, ROUND(cohead_freight, 2),"
	"                         0, 'C'), 2) AS freightc,"
	"         NULL AS adjTax,"
	"         NULL AS adjPcta,"
	"         NULL AS adjPctb,"
	"         NULL AS adjPctc,"
	"         NULL AS adja,"
	"         NULL AS adjb,"
	"         NULL AS adjc"
	"  FROM coitem, item, itemsite, cohead LEFT OUTER JOIN"
	"       taxauth ON (cohead_taxauth_id=taxauth_id) LEFT OUTER JOIN"
	"       tax ON (tax_id=getFreightTaxSelection(taxauth_id))"
	"  WHERE ((coitem_cohead_id=<? value(\"cohead_id\") ?>)"
	"    AND  (coitem_cohead_id=cohead_id)"
	"    AND  (coitem_itemsite_id=itemsite_id)"
	"    AND  (itemsite_item_id=item_id))"
	"  GROUP BY number, cohead_taxauth_id, curr_id, tax_curr_id,"
	"         date, freight, freightTax,"
	"         freightPcta, freightPctb, freightPctc;"
	"<? elseif exists(\"quhead_id\") ?>"
	"  SELECT quhead_number AS number,"
	"         quhead_taxauth_id AS taxauth_id,"
	"         quhead_curr_id AS curr_id,"
	"         quhead_curr_id AS tax_curr_id,"
	"         quhead_quotedate AS date,"
	"         SUM(ROUND((quitem_qtyord * quitem_qty_invuomratio) * (quitem_price / quitem_price_invuomratio),"
	"                    2)) AS line,"
	"         SUM(ROUND((calculateTax(quitem_tax_id,"
	"                        (ROUND((quitem_qtyord * quitem_qty_invuomratio) * (quitem_price /"
	"                        quitem_price_invuomratio), 2)), 0, 'A')), 2)) AS linea,"
	"         SUM(ROUND((calculateTax(quitem_tax_id,"
	"                        (ROUND((quitem_qtyord * quitem_qty_invuomratio) * (quitem_price /"
	"                        quitem_price_invuomratio), 2)), 0, 'B')), 2)) AS lineb,"
	"         SUM(ROUND((calculateTax(quitem_tax_id,"
	"                        (ROUND((quitem_qtyord * quitem_qty_invuomratio) * (quitem_price /"
	"                        quitem_price_invuomratio), 2)), 0, 'C')), 2)) AS linec,"
	"         quhead_freight AS freight,"
	"         tax_id AS freightTax,"
	"         tax_ratea AS freightPcta,"
	"         tax_rateb AS freightPctb,"
	"         tax_ratec AS freightPctc,"
	"         ROUND(calculateTax(tax_id, ROUND(quhead_freight, 2),"
	"                         0, 'A'), 2) AS freighta,"
	"         ROUND(calculateTax(tax_id, ROUND(quhead_freight, 2),"
	"                         0, 'B'), 2) AS freightb,"
	"         ROUND(calculateTax(tax_id, ROUND(quhead_freight, 2),"
	"                         0, 'C'), 2) AS freightc,"
	"         NULL AS adjTax,"
	"         NULL AS adjPcta,"
	"         NULL AS adjPctb,"
	"         NULL AS adjPctc,"
	"         NULL AS adja,"
	"         NULL AS adjb,"
	"         NULL AS adjc"
	"  FROM quitem, item, quhead LEFT OUTER JOIN"
	"       taxauth ON (quhead_taxauth_id=taxauth_id) LEFT OUTER JOIN"
	"       tax ON (tax_id=getFreightTaxSelection(taxauth_id))"
	"  WHERE ((quitem_quhead_id=<? value(\"quhead_id\") ?>)"
	"    AND  (quitem_quhead_id=quhead_id)"
	"    AND  (quitem_item_id=item_id))"
	"  GROUP BY number, quhead_taxauth_id, curr_id, tax_curr_id,"
	"         date, freight, freightTax,"
	"         freightPcta, freightPctb, freightPctc;"

	"<? elseif exists(\"rahead_id\") ?>"
	"  SELECT rahead_number AS number,"
	"         rahead_taxauth_id AS taxauth_id,"
	"         rahead_curr_id AS curr_id,"
	"         rahead_curr_id AS tax_curr_id,"
	"         rahead_authdate AS date,"
	"         SUM(ROUND((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio),"
	"                    2)) AS line,"
	"         SUM(ROUND((calculateTax(raitem_tax_id,"
	"                        (ROUND((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice /"
	"                        raitem_price_invuomratio), 2)), 0, 'A')), 2)) AS linea,"
	"         SUM(ROUND((calculateTax(raitem_tax_id,"
	"                        (ROUND((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice /"
	"                        raitem_price_invuomratio), 2)), 0, 'B')), 2)) AS lineb,"
	"         SUM(ROUND((calculateTax(raitem_tax_id,"
	"                        (ROUND((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice /"
	"                        raitem_price_invuomratio), 2)), 0, 'C')), 2)) AS linec,"
	"         rahead_freight AS freight,"
	"         tax_id AS freightTax,"
	"         tax_ratea AS freightPcta,"
	"         tax_rateb AS freightPctb,"
	"         tax_ratec AS freightPctc,"
	"         ROUND(calculateTax(tax_id, ROUND(rahead_freight, 2),"
	"                         0, 'A'), 2) AS freighta,"
	"         ROUND(calculateTax(tax_id, ROUND(rahead_freight, 2),"
	"                         0, 'B'), 2) AS freightb,"
	"         ROUND(calculateTax(tax_id, ROUND(rahead_freight, 2),"
	"                         0, 'C'), 2) AS freightc,"
	"         NULL AS adjTax,"
	"         NULL AS adjPcta,"
	"         NULL AS adjPctb,"
	"         NULL AS adjPctc,"
	"         NULL AS adja,"
	"         NULL AS adjb,"
	"         NULL AS adjc"
	"  FROM raitem, rahead LEFT OUTER JOIN"
	"       taxauth ON (rahead_taxauth_id=taxauth_id) LEFT OUTER JOIN"
	"       tax ON (tax_id=getFreightTaxSelection(taxauth_id))"
	"  WHERE ((raitem_rahead_id=<? value(\"rahead_id\") ?>)"
	"    AND  (raitem_rahead_id=rahead_id))"
	"  GROUP BY number, rahead_taxauth_id, curr_id, tax_curr_id,"
	"         date, freight, freightTax,"
	"         freightPcta, freightPctb, freightPctc;"

	"<? elseif exists(\"tohead_id\") ?>"
	"  SELECT tohead_number AS number,"
	"         tohead_taxauth_id AS taxauth_id,"
	"         tohead_freight_curr_id AS curr_id,"
	"         tohead_freight_curr_id AS tax_curr_id,"
	"         tohead_orderdate AS date,"
	"         SUM(ROUND(toitem_freight, 2)) AS line,"
	"         SUM(ROUND((toitem_freighttax_ratea), 2)) AS linea,"
	"         SUM(ROUND((toitem_freighttax_rateb), 2)) AS lineb,"
	"         SUM(ROUND((toitem_freighttax_ratec), 2)) AS linec,"
	"         tohead_freight AS freight,"
	"         tohead_freighttax_id AS freightTax,"
	"         tohead_freighttax_pcta AS freightPcta,"
	"         tohead_freighttax_pctb AS freightPctb,"
	"         tohead_freighttax_pctc AS freightPctc,"
	"         ROUND(tohead_freighttax_ratea, 2) AS freighta,"
	"         ROUND(tohead_freighttax_rateb, 2) AS freightb,"
	"         ROUND(tohead_freighttax_ratec, 2) AS freightc,"
	"         NULL AS adjTax,"
	"         NULL AS adjPcta,"
	"         NULL AS adjPctb,"
	"         NULL AS adjPctc,"
	"         NULL AS adja,"
	"         NULL AS adjb,"
	"         NULL AS adjc"
	"  FROM toitem, tohead "
	"  WHERE ((toitem_tohead_id=<? value(\"tohead_id\") ?>)"
	"    AND  (toitem_tohead_id=tohead_id)"
	"    AND  (toitem_status!='X')) "
	"  GROUP BY number, taxauth_id, curr_id, tax_curr_id,"
	"         date,		freight,	freightTax,"
	"         freighta,	freightb,	freightc,"
	"         freightPcta,	freightPctb,	freightPctc;"
	"<? endif ?>";

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    // do dates and currencies first because of signal/slot cascades
    _total->setEffective(q.value("date").toDate());
    _currency->setId(q.value("curr_id").toInt());
    _taxcurrency->setId(q.value("tax_curr_id").toInt());

    // now the rest
    _document->setText(q.value("number").toString());
    _taxauth->setId(q.value("taxauth_id").toInt());
    _line->setLocalValue(q.value("line").toDouble());
    _freight->setLocalValue(q.value("freight").toDouble());
    _pretax->setLocalValue(_line->localValue() + _freight->localValue());

    _taxCache.setLine(q.value("linea").toDouble(),
		      q.value("lineb").toDouble(),
		      q.value("linec").toDouble());

    _taxCache.setFreightId(q.value("freightTax").toInt());
    _taxCache.setFreightPct(q.value("freightPcta").toDouble(),
			    q.value("freightPctb").toDouble(),
			    q.value("freightPctc").toDouble());
    _taxCache.setFreight(q.value("freighta").toDouble(),
			 q.value("freightb").toDouble(),
			 q.value("freightc").toDouble());

    _taxCache.setAdjId(q.value("adjTax").toInt());
    _taxCache.setAdjPct(q.value("adjPcta").toDouble(),
			q.value("adjPctb").toDouble(),
			q.value("adjPctc").toDouble());
    _taxCache.setAdj(q.value("adja").toDouble(),
		     q.value("adjb").toDouble(),
		     q.value("adjc").toDouble());

    SetResponse calc = sRecalc();
    if (calc != NoError)
      return calc;
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return UndefinedError;
  }

  return NoError;
}

void taxBreakdown::sSave()
{
  ParameterList params;
  if (_ordertype == "I")
    params.append("invchead_id", _orderid);
  else if (_ordertype == "B")
    params.append("cobmisc_id", _orderid);
  else if (_ordertype == "CM")
    params.append("cmhead_id", _orderid);
  else if (_ordertype == "TO")
    params.append("tohead_id", _orderid);
  else
    accept();

  params.append("adja",	 	_taxCache.adj(0));
  params.append("adjb",	 	_taxCache.adj(1));
  params.append("adjc",	 	_taxCache.adj(2));
  params.append("adjPctA",	_taxCache.adjPct(0));
  params.append("adjPctB",	_taxCache.adjPct(1));
  params.append("adjPctC",	_taxCache.adjPct(2));
  params.append("freighta",	_taxCache.freight(0));
  params.append("freightb",	_taxCache.freight(1));
  params.append("freightc",	_taxCache.freight(2));
  params.append("freightPctA",	_taxCache.freightPct(0));
  params.append("freightPctB",	_taxCache.freightPct(1));
  params.append("freightPctC",	_taxCache.freightPct(2));

  if (_taxCache.adjId() > 0)
    params.append("adjTax",	_taxCache.adjId());
  if (_taxCache.freightId() > 0)
    params.append("freightTax", _taxCache.freightId());

  QString sql =
	"<? if exists(\"invchead_id\") ?>"
	"  UPDATE invchead SET "
	"         invchead_adjtax_id=<? value(\"adjTax\") ?>,"
	"         invchead_adjtax_pcta=<? value(\"adjPctA\") ?>,"
	"         invchead_adjtax_pctb=<? value(\"adjPctB\") ?>,"
	"         invchead_adjtax_pctc=<? value(\"adjPctC\") ?>,"
	"         invchead_adjtax_ratea=<? value(\"adja\") ?>,"
	"         invchead_adjtax_rateb=<? value(\"adjb\") ?>,"
	"         invchead_adjtax_ratec=<? value(\"adjc\") ?>,"
	"         invchead_freighttax_id=<? value(\"freightTax\") ?>,"
	"         invchead_freighttax_pcta=<? value(\"freightPctA\") ?>,"
	"         invchead_freighttax_pctb=<? value(\"freightPctB\") ?>,"
	"         invchead_freighttax_pctc=<? value(\"freightPctC\") ?>,"
	"         invchead_freighttax_ratea=<? value(\"freighta\") ?>,"
	"         invchead_freighttax_rateb=<? value(\"freightb\") ?>,"
	"         invchead_freighttax_ratec=<? value(\"freightc\") ?>"
	"  WHERE (invchead_id=<? value(\"invchead_id\") ?>)"
	"<? elseif exists(\"cmhead_id\") ?>"
	"  UPDATE cmhead SET "
	"         cmhead_adjtax_id=<? value(\"adjTax\") ?>,"
	"         cmhead_adjtax_pcta=<? value(\"adjPctA\") ?>,"
	"         cmhead_adjtax_pctb=<? value(\"adjPctB\") ?>,"
	"         cmhead_adjtax_pctc=<? value(\"adjPctC\") ?>,"
	"         cmhead_adjtax_ratea=<? value(\"adja\") ?>,"
	"         cmhead_adjtax_rateb=<? value(\"adjb\") ?>,"
	"         cmhead_adjtax_ratec=<? value(\"adjc\") ?>,"
	"         cmhead_freighttax_id=<? value(\"freightTax\") ?>,"
	"         cmhead_freighttax_pcta=<? value(\"freightPctA\") ?>,"
	"         cmhead_freighttax_pctb=<? value(\"freightPctB\") ?>,"
	"         cmhead_freighttax_pctc=<? value(\"freightPctC\") ?>,"
	"         cmhead_freighttax_ratea=<? value(\"freighta\") ?>,"
	"         cmhead_freighttax_rateb=<? value(\"freightb\") ?>,"
	"         cmhead_freighttax_ratec=<? value(\"freightc\") ?>"
	"  WHERE (cmhead_id=<? value(\"cmhead_id\") ?>)"
	"<? elseif exists(\"cobmisc_id\") ?>"
	"  UPDATE cobmisc SET "
	"         cobmisc_adjtax_id=<? value(\"adjTax\") ?>,"
	"         cobmisc_adjtax_pcta=<? value(\"adjPctA\") ?>,"
	"         cobmisc_adjtax_pctb=<? value(\"adjPctB\") ?>,"
	"         cobmisc_adjtax_pctc=<? value(\"adjPctC\") ?>,"
	"         cobmisc_adjtax_ratea=<? value(\"adja\") ?>,"
	"         cobmisc_adjtax_rateb=<? value(\"adjb\") ?>,"
	"         cobmisc_adjtax_ratec=<? value(\"adjc\") ?>,"
	"         cobmisc_freighttax_id=<? value(\"freightTax\") ?>,"
	"         cobmisc_freighttax_pcta=<? value(\"freightPctA\") ?>,"
	"         cobmisc_freighttax_pctb=<? value(\"freightPctB\") ?>,"
	"         cobmisc_freighttax_pctc=<? value(\"freightPctC\") ?>,"
	"         cobmisc_freighttax_ratea=<? value(\"freighta\") ?>,"
	"         cobmisc_freighttax_rateb=<? value(\"freightb\") ?>,"
	"         cobmisc_freighttax_ratec=<? value(\"freightc\") ?>"
	"  WHERE (cobmisc_id=<? value(\"cobmisc_id\") ?>)"
	"<? elseif exists(\"tohead_id\") ?>"
	"  UPDATE tohead SET "
	"         tohead_freighttax_id=<? value(\"freightTax\") ?>,"
	"         tohead_freighttax_pcta=<? value(\"freightPctA\") ?>,"
	"         tohead_freighttax_pctb=<? value(\"freightPctB\") ?>,"
	"         tohead_freighttax_pctc=<? value(\"freightPctC\") ?>,"
	"         tohead_freighttax_ratea=<? value(\"freighta\") ?>,"
	"         tohead_freighttax_rateb=<? value(\"freightb\") ?>,"
	"         tohead_freighttax_ratec=<? value(\"freightc\") ?>"
	"  WHERE (tohead_id=<? value(\"tohead_id\") ?>)"
	"<? endif ?>";

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void taxBreakdown::sAdjTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("curr_id", _taxcurrency->id());
  params.append("tax_id",  _taxCache.adjId());
  params.append("valueA",  _taxCache.adj(0));
  params.append("valueB",  _taxCache.adj(1));
  params.append("valueC",  _taxCache.adj(2));
  params.append("pctA",    _taxCache.adjPct(0));
  params.append("pctB",    _taxCache.adjPct(1));
  params.append("pctC",    _taxCache.adjPct(2));
  params.append("date",    _adjTax->effective());

  // s/o and quote don't have a place to store changes
  if (_ordertype == "S" || _ordertype == "Q" || cView == _mode)
    params.append("readOnly");

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    _taxCache.setAdjId(newdlg.tax());
    _taxCache.setAdj(newdlg.amountA(), newdlg.amountB(), newdlg.amountC());
    _taxCache.setAdjPct(newdlg.pctA(), newdlg.pctB(), newdlg.pctC());
    sRecalc();
  }
}

void taxBreakdown::sFreightTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("curr_id", _taxcurrency->id());
  params.append("tax_id",  _taxCache.freightId());
  params.append("valueA",  _taxCache.freight(0));
  params.append("valueB",  _taxCache.freight(1));
  params.append("valueC",  _taxCache.freight(2));
  params.append("pctA",    _taxCache.freightPct(0));
  params.append("pctB",    _taxCache.freightPct(1));
  params.append("pctC",    _taxCache.freightPct(2));
  params.append("date",    _freight->effective());
  params.append("subtotal",CurrDisplay::convert(_freight->id(),
						_taxcurrency->id(),
						_freight->localValue(),
						_freight->effective()));

  // s/o and quote don't have a place to store changes
  if (_ordertype == "S" || _ordertype == "Q" || cView == _mode)
    params.append("readOnly");

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    _taxCache.setFreightId(newdlg.tax());
    _taxCache.setFreight(newdlg.amountA(), newdlg.amountB(), newdlg.amountC());
    _taxCache.setFreightPct(newdlg.pctA(), newdlg.pctB(), newdlg.pctC());
    sRecalc();
  }
}

void taxBreakdown::sLineTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("curr_id", _taxcurrency->id());
  params.append("valueA",  _taxCache.line(0));
  params.append("valueB",  _taxCache.line(1));
  params.append("valueC",  _taxCache.line(2));
  params.append("date",    _line->effective());
  params.append("subtotal",CurrDisplay::convert(_line->id(),
						_taxcurrency->id(),
						_line->localValue(),
						_line->effective()));
  params.append("readOnly");

  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void taxBreakdown::sTotalTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("curr_id", _taxcurrency->id());
  params.append("valueA",  _taxCache.total(0));
  params.append("valueB",  _taxCache.total(1));
  params.append("valueC",  _taxCache.total(2));
  params.append("date",    _total->effective());
  params.append("subtotal",CurrDisplay::convert(_pretax->id(),
						_taxcurrency->id(),
						_pretax->localValue(),
						_pretax->effective()));
  params.append("readOnly");

  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

double taxBreakdown::a()
{
  return _taxCache.total(0);
}

double taxBreakdown::b()
{
  return _taxCache.total(1);
}

double taxBreakdown::c()
{
  return _taxCache.total(2);
}

SetResponse taxBreakdown::sRecalc()
{
  _lineTax->setLocalValue(_taxCache.line());
  _freightTax->setLocalValue(_taxCache.freight());
  _adjTax->setLocalValue(_taxCache.adj());

  _totalTax->setLocalValue(_taxCache.total());

  _total->setLocalValue(_pretax->localValue() +
			CurrDisplay::convert(_totalTax->id(),
					     _total->id(),
					     _totalTax->localValue(),
					     _totalTax->effective())); 
  return NoError;
}
