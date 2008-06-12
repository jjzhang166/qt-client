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

#include "creditMemo.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QValidator>
#include <QVariant>

#include "creditMemoItem.h"
#include "invoiceList.h"
#include "shipToList.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"

creditMemo::creditMemo(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_memoNumber, SIGNAL(lostFocus()), this, SLOT(sCheckCreditMemoNumber()));
  connect(_copyToShipto, SIGNAL(clicked()), this, SLOT(sCopyToShipto()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_invoiceList, SIGNAL(clicked()), this, SLOT(sInvoiceList()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_shipToNumber, SIGNAL(lostFocus()), this, SLOT(sParseShipToNumber()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_shipToList, SIGNAL(clicked()), this, SLOT(sShipToList()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_subtotal, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_tax, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_miscCharge, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_freight,	SIGNAL(valueChanged()),	this, SLOT(sCalculateTotal()));
  connect(_freight,	SIGNAL(valueChanged()),	this, SLOT(sFreightChanged()));
  connect(_taxauth,	SIGNAL(newID(int)),	this, SLOT(sTaxAuthChanged()));

#ifndef Q_WS_MAC
  _invoiceList->setMaximumWidth(25);
  _shipToList->setMaximumWidth(25);
#endif

  _taxcurrid		= -1;
  _custtaxauthid	= -1;
  _taxauthidCache	= -1;
  _taxCache.clear();
  _shiptoid		= -1;

  _memoNumber->setValidator(omfgThis->orderVal());

  _currency->setLabel(_currencyLit);

  _cmitem->addColumn(tr("#"),           _seqColumn,   Qt::AlignCenter );
  _cmitem->addColumn(tr("Item"),        _itemColumn,  Qt::AlignLeft   );
  _cmitem->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _cmitem->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter );
  _cmitem->addColumn(tr("Qty UOM"),     _uomColumn,   Qt::AlignLeft   );
  _cmitem->addColumn(tr("Returned"),    _qtyColumn,   Qt::AlignRight  );
  _cmitem->addColumn(tr("Credited"),    _qtyColumn,   Qt::AlignRight  );
  _cmitem->addColumn(tr("Price UOM"),     _uomColumn,   Qt::AlignLeft   );
  _cmitem->addColumn(tr("Price"),       _priceColumn, Qt::AlignRight  );
  _cmitem->addColumn(tr("Extended"),    _moneyColumn, Qt::AlignRight  );

}

creditMemo::~creditMemo()
{
  // no need to delete child widgets, Qt does it all for us
}

void creditMemo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse creditMemo::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cmhead_id", &valid);
  if (valid)
  {
    _cmheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      q.prepare("SELECT NEXTVAL('cmhead_cmhead_id_seq') AS cmhead_id;");
      q.exec();
      if (q.first())
        _cmheadid = q.value("cmhead_id").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      setNumber();
      _memoDate->setDate(omfgThis->dbDate(), true);

      q.prepare("INSERT INTO cmhead ("
		"    cmhead_id, cmhead_number, cmhead_docdate, cmhead_posted"
		") VALUES ("
		"    :cmhead_id, :cmhead_number, :cmhead_docdate, false"
		");");
      q.bindValue(":cmhead_id",		_cmheadid);
      q.bindValue(":cmhead_number",	_memoNumber->text().toInt());
      q.bindValue(":cmhead_docdate",	_memoDate->date());
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustomerInfo()));
      connect(_cust, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _memoNumber->setEnabled(FALSE);
      _cust->setReadOnly(TRUE);
      _invoiceNumber->setEnabled(FALSE);
      _invoiceList->hide();

      _new->setEnabled(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _memoNumber->setEnabled(FALSE);
      _memoDate->setEnabled(FALSE);
      _cust->setReadOnly(TRUE);
      _invoiceNumber->setEnabled(FALSE);
      _salesRep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);

      _billtoName->setEnabled(FALSE);
      _billToAddr->setEnabled(FALSE);

      _taxauth->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);
      _customerPO->setEnabled(FALSE);
      _hold->setEnabled(FALSE);
      _miscCharge->setEnabled(FALSE);
      _miscChargeDescription->setEnabled(FALSE);
      _miscChargeAccount->setReadOnly(TRUE);
      _freight->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _invoiceList->hide();
      _shipToNumber->setEnabled(FALSE);
      _shipToName->setEnabled(FALSE);
      _shipToAddr->setEnabled(FALSE);
      _currency->setEnabled(FALSE);
      _shipToList->hide();
      _save->hide();
      _new->hide();
      _delete->hide();
      _edit->setText(tr("&View"));

      _close->setFocus();
    }
  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());

  return NoError;
}

void creditMemo::setNumber()
{
  if ( (_metrics->value("CMNumberGeneration") == "A") || 
       (_metrics->value("CMNumberGeneration") == "O")   )
  {
    q.prepare("SELECT fetchCmNumber() AS cmnumber;");
    q.exec();
    if (q.first())
    {
      _memoNumber->setText(q.value("cmnumber").toString());

      if (_metrics->value("CMNumberGeneration") == "A")
        _memoNumber->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (_metrics->value("CMNumberGeneration") == "S")
  {
    q.prepare("SELECT fetchSoNumber() AS cmnumber;");
    q.exec();
    if (q.first())
    {
      _memoNumber->setText(q.value("cmnumber").toString());
      _memoNumber->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _memoNumber->setFocus();
}

void creditMemo::sSave()
{
  //  Make sure that all of the required field have been populated
  if (!_cust->isValid())
  {
    QMessageBox::information(this, tr("Select a Customer"),
                             tr("Please select a Customer before continuing.") );
    _cust->setFocus();
    return;
  }

  if ( ! _miscCharge->isZero() && (!_miscChargeAccount->isValid()) )
  {
    QMessageBox::warning( this, tr("No Misc. Charge Account Number"),
                         tr("<p>You may not enter a Misc. Charge without "
			    "indicating the G/L Sales Account number for the "
			    "charge. Please set the Misc. Charge amount to 0 "
			    "or select a Misc. Charge Sales Account." ) );
    _creditMemoInformation->setCurrentPage(1);
    _miscChargeAccount->setFocus();
    return;
  }
  
  if (_total->localValue() < 0 )
  {
    QMessageBox::information(this, tr("Total Less than Zero"),
                             tr("<p>The Total must be a positive value.") );
    _cust->setFocus();
    return;
  }

  // save address info in case someone wants to use 'em again later
  // but don't make any global changes to the data and ignore errors
  _billToAddr->save(AddressCluster::CHANGEONE);
  _shipToAddr->save(AddressCluster::CHANGEONE);

  q.prepare( "UPDATE cmhead "
	     "SET cmhead_invcnumber=:cmhead_invcnumber, cmhead_cust_id=:cmhead_cust_id,"
	     "    cmhead_custponumber=:cmhead_custponumber, cmhead_hold=:cmhead_hold,"
	     "    cmhead_billtoname=:cmhead_billtoname, cmhead_billtoaddress1=:cmhead_billtoaddress1,"
	     "    cmhead_billtoaddress2=:cmhead_billtoaddress2, cmhead_billtoaddress3=:cmhead_billtoaddress3,"
	     "    cmhead_billtocity=:cmhead_billtocity, cmhead_billtostate=:cmhead_billtostate,"
	     "    cmhead_billtozip=:cmhead_billtozip,"
	     "    cmhead_billtocountry=:cmhead_billtocountry,"
	     "    cmhead_shipto_id=:cmhead_shipto_id,"
	     "    cmhead_shipto_name=:cmhead_shipto_name, cmhead_shipto_address1=:cmhead_shipto_address1,"
	     "    cmhead_shipto_address2=:cmhead_shipto_address2, cmhead_shipto_address3=:cmhead_shipto_address3,"
	     "    cmhead_shipto_city=:cmhead_shipto_city, cmhead_shipto_state=:cmhead_shipto_state,"
	     "    cmhead_shipto_zipcode=:cmhead_shipto_zipcode,"
	     "    cmhead_shipto_country=:cmhead_shipto_country,"
	     "    cmhead_docdate=:cmhead_docdate,"
	     "    cmhead_salesrep_id=:cmhead_salesrep_id, cmhead_commission=:cmhead_commission,"
	     "    cmhead_misc=:cmhead_misc, cmhead_misc_accnt_id=:cmhead_misc_accnt_id,"
	     "    cmhead_misc_descrip=:cmhead_misc_descrip,"
	     "    cmhead_freight=:cmhead_freight,"
	     "    cmhead_taxauth_id=:cmhead_taxauth_id,"
	     "    cmhead_tax_curr_id=:cmhead_tax_curr_id,"
	     "    cmhead_adjtax_id=:cmhead_adjtax_id,"
	     "    cmhead_adjtaxtype_id=:cmhead_adjtaxtype_id,"
	     "    cmhead_adjtax_pcta=:cmhead_adjtax_pcta,"
	     "    cmhead_adjtax_pctb=:cmhead_adjtax_pctb,"
	     "    cmhead_adjtax_pctc=:cmhead_adjtax_pctc,"
	     "    cmhead_adjtax_ratea=:cmhead_adjtax_ratea,"
	     "    cmhead_adjtax_rateb=:cmhead_adjtax_rateb,"
	     "    cmhead_adjtax_ratec=:cmhead_adjtax_ratec,"
	     "    cmhead_freighttax_id=:cmhead_freighttax_id,"
	     "    cmhead_freighttaxtype_id=:cmhead_freighttaxtype_id,"
	     "    cmhead_freighttax_pcta=:cmhead_freighttax_pcta,"
	     "    cmhead_freighttax_pctb=:cmhead_freighttax_pctb,"
	     "    cmhead_freighttax_pctc=:cmhead_freighttax_pctc,"
	     "    cmhead_freighttax_ratea=:cmhead_freighttax_ratea,"
	     "    cmhead_freighttax_rateb=:cmhead_freighttax_rateb,"
	     "    cmhead_freighttax_ratec=:cmhead_freighttax_ratec,"
	     "    cmhead_comments=:cmhead_comments, "
	     "    cmhead_rsncode_id=:cmhead_rsncode_id, "
	     "    cmhead_curr_id=:cmhead_curr_id "
	     "WHERE (cmhead_id=:cmhead_id);" );

  q.bindValue(":cmhead_id", _cmheadid);
  q.bindValue(":cmhead_cust_id", _cust->id());
  q.bindValue(":cmhead_number", _memoNumber->text().toInt());
  q.bindValue(":cmhead_invcnumber", _invoiceNumber->invoiceNumber());
  q.bindValue(":cmhead_custponumber", _customerPO->text().stripWhiteSpace());
  q.bindValue(":cmhead_billtoname", _billtoName->text().stripWhiteSpace());
  q.bindValue(":cmhead_billtoaddress1",	_billToAddr->line1());
  q.bindValue(":cmhead_billtoaddress2",	_billToAddr->line2());
  q.bindValue(":cmhead_billtoaddress3",	_billToAddr->line3());
  q.bindValue(":cmhead_billtocity",	_billToAddr->city());
  q.bindValue(":cmhead_billtostate",	_billToAddr->state());
  q.bindValue(":cmhead_billtozip",	_billToAddr->postalCode());
  q.bindValue(":cmhead_billtocountry",	_billToAddr->country());
  if (_shiptoid > 0)
    q.bindValue(":cmhead_shipto_id",	_shiptoid);
  q.bindValue(":cmhead_shipto_name", _shipToName->text().stripWhiteSpace());
  q.bindValue(":cmhead_shipto_address1", _shipToAddr->line1());
  q.bindValue(":cmhead_shipto_address2", _shipToAddr->line2());
  q.bindValue(":cmhead_shipto_address3", _shipToAddr->line3());
  q.bindValue(":cmhead_shipto_city",	 _shipToAddr->city());
  q.bindValue(":cmhead_shipto_state",	 _shipToAddr->state());
  q.bindValue(":cmhead_shipto_zipcode",	 _shipToAddr->postalCode());
  q.bindValue(":cmhead_shipto_country",	 _shipToAddr->country());
  q.bindValue(":cmhead_docdate", _memoDate->date());
  q.bindValue(":cmhead_comments", _comments->text());
  q.bindValue(":cmhead_salesrep_id", _salesRep->id());
  q.bindValue(":cmhead_rsncode_id", _rsnCode->id());
  q.bindValue(":cmhead_hold", QVariant(_hold->isChecked(), 0));
  q.bindValue(":cmhead_commission", (_commission->toDouble() / 100));
  q.bindValue(":cmhead_misc", _miscCharge->localValue());
  q.bindValue(":cmhead_misc_accnt_id", _miscChargeAccount->id());
  q.bindValue(":cmhead_misc_descrip", _miscChargeDescription->text());
  q.bindValue(":cmhead_freight", _freight->localValue());
  if (_taxauth->isValid())
    q.bindValue(":cmhead_taxauth_id",	_taxauth->id());
  if (_taxcurrid > 0)
    q.bindValue(":cmhead_tax_curr_id",	_taxcurrid);
  if (_taxCache.adjId() > 0)
    q.bindValue(":cmhead_adjtax_id",	_taxCache.adjId());
  if (_taxCache.adjType() > 0)
    q.bindValue(":cmhead_adjtaxtype_id",_taxCache.adjType());
  q.bindValue(":cmhead_adjtax_pcta",	_taxCache.adjPct(0));
  q.bindValue(":cmhead_adjtax_pctb",	_taxCache.adjPct(1));
  q.bindValue(":cmhead_adjtax_pctc",	_taxCache.adjPct(2));
  q.bindValue(":cmhead_adjtax_ratea",	_taxCache.adj(0));
  q.bindValue(":cmhead_adjtax_rateb",	_taxCache.adj(1));
  q.bindValue(":cmhead_adjtax_ratec",	_taxCache.adj(2));
  if (_taxCache.freightId() > 0)
    q.bindValue(":cmhead_freighttax_id",	_taxCache.freightId());
  if (_taxCache.freightType() > 0)
    q.bindValue(":cmhead_freighttaxtype_id",	_taxCache.freightType());
  q.bindValue(":cmhead_freighttax_pcta",	_taxCache.freightPct(0));
  q.bindValue(":cmhead_freighttax_pctb",	_taxCache.freightPct(1));
  q.bindValue(":cmhead_freighttax_pctc",	_taxCache.freightPct(2));
  q.bindValue(":cmhead_freighttax_ratea",	_taxCache.freight(0));
  q.bindValue(":cmhead_freighttax_rateb",	_taxCache.freight(1));
  q.bindValue(":cmhead_freighttax_ratec",	_taxCache.freight(2));
  q.bindValue(":cmhead_curr_id", _currency->id());

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sCreditMemosUpdated();

  _cmheadid = -1;
  close();
}

void creditMemo::sShipToList()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("shipto_id", _shiptoid);

  shipToList newdlg(this, "", TRUE);
  newdlg.set(params);

  int shiptoid = newdlg.exec();

  if (shiptoid != -1)
    populateShipto(shiptoid);
}

void creditMemo::sInvoiceList()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("invoiceNumber", _invoiceNumber->invoiceNumber());

  invoiceList newdlg(this, "", TRUE);
  newdlg.set(params);
  int invoiceid = newdlg.exec();

  if (invoiceid != 0)
  {

    XSqlQuery sohead;
    sohead.prepare( "SELECT invchead.*, "
                    "       formatScrap(invchead_commission) AS f_commission "
                    "FROM invchead "
                    "WHERE (invchead_id=:invcid) "
                    "LIMIT 1;" );
    sohead.bindValue(":invcid", invoiceid);
    sohead.exec();
    if (sohead.first())
    {
      _invoiceNumber->setInvoiceNumber(sohead.value("invchead_invcnumber").toString());
      _salesRep->setId(sohead.value("invchead_salesrep_id").toInt());
      _commission->setText(sohead.value("f_commission"));

      _taxauth->setId(sohead.value("invchead_taxauth_id").toInt());
      _customerPO->setText(sohead.value("invchead_ponumber"));

      _cust->setEnabled(FALSE);
      _billtoName->setEnabled(FALSE);
      _billToAddr->setEnabled(FALSE);

      _cust->setId(sohead.value("invchead_cust_id").toInt());
      _billtoName->setText(sohead.value("invchead_billto_name"));
      _billToAddr->setLine1(sohead.value("invchead_billto_address1").toString());
      _billToAddr->setLine2(sohead.value("invchead_billto_address2").toString());
      _billToAddr->setLine3(sohead.value("invchead_billto_address3").toString());
      _billToAddr->setCity(sohead.value("invchead_billto_city").toString());
      _billToAddr->setState(sohead.value("invchead_billto_state").toString());
      _billToAddr->setPostalCode(sohead.value("invchead_billto_zipcode").toString());
      _billToAddr->setCountry(sohead.value("invchead_billto_country").toString());

      _shipToNumber->setEnabled(FALSE);
      _shipToName->setEnabled(FALSE);
      _shipToAddr->setEnabled(FALSE);
      _ignoreShiptoSignals = TRUE;
      _shipToName->setText(sohead.value("invchead_shipto_name"));
      _shipToAddr->setLine1(sohead.value("invchead_shipto_address1").toString());
      _shipToAddr->setLine2(sohead.value("invchead_shipto_address2").toString());
      _shipToAddr->setLine3(sohead.value("invchead_shipto_address3").toString());
      _shipToAddr->setCity(sohead.value("invchead_shipto_city").toString());
      _shipToAddr->setState(sohead.value("invchead_shipto_state").toString());
      _shipToAddr->setPostalCode(sohead.value("invchead_shipto_zipcode").toString());
      _shipToAddr->setCountry(sohead.value("invchead_shipto_country").toString());
      _ignoreShiptoSignals = FALSE;
    }
    else if (sohead.lastError().type() != QSqlError::None)
    {
      systemError(this, sohead.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void creditMemo::sParseShipToNumber()
{
  q.prepare( "SELECT shipto_id "
             "FROM shiptoinfo "
             "WHERE ( (shipto_cust_id=:cust_id)"
             " AND (UPPER(shipto_num)=UPPER(:shipto_num)));" );
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":shipto_num", _shipToNumber->text());
  q.exec();
  if (q.first())
    populateShipto(q.value("shipto_id").toInt());
  else
  {
    if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    populateShipto(-1);
  }
}

void creditMemo::populateShipto(int pShiptoid)
{
  if (pShiptoid != -1)
  {
    XSqlQuery query;
    query.prepare( "SELECT shipto_num, shipto_name,"
		   "       shipto_addr_id, shipto_taxauth_id "
                   "FROM shiptoinfo "
                   "WHERE (shipto_id=:shipto_id);" );
    query.bindValue(":shipto_id", pShiptoid);
    query.exec();
    if (query.first())
    {
      _shipToNumber->setText(query.value("shipto_num"));
      _shipToName->setText(query.value("shipto_name"));
      _shipToAddr->setId(query.value("shipto_addr_id").toInt());
      _taxauth->setId(query.value("shipto_taxauth_id").toInt());
    }
    else if (query.lastError().type() != QSqlError::None)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _shipToNumber->clear();
    _shipToName->clear();
    _shipToAddr->clear();
  }

  _shipToAddr->setEnabled(FALSE);

  _shiptoid = pShiptoid;
}

void creditMemo::sPopulateCustomerInfo()
{
  if (!_invoiceNumber->isValid())
  {
    if (_cust->isValid())
    {
      XSqlQuery query;
      query.prepare( "SELECT cust_salesrep_id,"
                     "       formatScrap(cust_commprcnt) AS f_commission,"
                     "       cust_taxauth_id, cust_curr_id, "
                     "       cust_name, cntct_addr_id, "
                     "       cust_ffshipto, cust_ffbillto "
                     "FROM custinfo LEFT OUTER JOIN"
		     "     cntct ON (cust_cntct_id=cntct_id) "
                     "WHERE (cust_id=:cust_id);" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
      {
        _ffShipto = query.value("cust_ffshipto").toBool();
        _copyToShipto->setEnabled(_ffShipto);

        _salesRep->setId(query.value("cust_salesrep_id").toInt());
        _commission->setText(query.value("f_commission"));

        _custtaxauthid = query.value("cust_taxauth_id").toInt();
        _taxauth->setId(query.value("cust_taxauth_id").toInt());
        _currency->setId(query.value("cust_curr_id").toInt());

        _billtoName->setText(query.value("cust_name"));
        _billToAddr->setId(query.value("cntct_addr_id").toInt());

        bool ffBillTo;
        if ( (_mode == cNew) || (_mode == cEdit) )
          ffBillTo = query.value("cust_ffbillto").toBool();
        else
          ffBillTo = FALSE;

        _billtoName->setEnabled(ffBillTo);
        _billToAddr->setEnabled(ffBillTo);
      }
      else if (query.lastError().type() != QSqlError::None)
      {
	systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    else
    {
      _salesRep->setCurrentItem(-1);
      _taxauth->setId(-1);
      _custtaxauthid	= -1;
    }

    _shipToName->setEnabled(_ffShipto);
    _shipToNumber->clear();
    _shipToName->clear();
    _shipToAddr->clear();
  }
}

void creditMemo::sPopulateByInvoiceNumber(int pInvoiceNumber)
{
  if (pInvoiceNumber == -1)
  {
    if (_cust->isValid())
      sPopulateCustomerInfo();
  }
  else
  {
    XSqlQuery query;
    query.prepare( "SELECT invchead_salesrep_id, invchead_shipto_id, "
		   "   invchead_curr_id "
                   "FROM invchead "
                   "WHERE (invchead_invcnumber=:invcnumber);" );
    query.bindValue(":invcnumber", pInvoiceNumber);
    query.exec();
    if (query.first())
    {
      _salesRep->setId(query.value("invchead_salesrep_id").toInt());
      _currency->setId(query.value("invchead_curr_id").toInt());

      int shiptoid;
      if ((shiptoid = query.value("invchead_shipto_id").toInt()) != -1)
      {
	query.prepare( "SELECT shipto_num,"
                       "       shipto_name, shipto_addr_id "
                       "FROM shiptoinfo "
                       "WHERE (shipto_id=:shipto_id)" );
	query.bindValue(":shipto_id", shiptoid);
	query.exec();
        if (query.first())
        {
          _shipToNumber->setText(query.value("shipto_num"));
          _shipToName->setText(query.value("shipto_name"));
          _shipToAddr->setId(query.value("shipto_addr_id").toInt());
        }
	else if (query.lastError().type() != QSqlError::None)
	{
	  systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
    }
    else if (query.lastError().type() != QSqlError::None)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void creditMemo::sCheckCreditMemoNumber()
{
  if ( (_memoNumber->text().length()) && _mode != cNew &&
       ( (_metrics->value("CMNumberGeneration") == "O") || 
         (_metrics->value("CMNumberGeneration") == "M")   ) )
  {
    _memoNumber->setEnabled(FALSE);

    XSqlQuery query;
    query.prepare( "SELECT cmhead_id, cmhead_posted "
                   "FROM cmhead "
                   "WHERE (cmhead_number=:cmhead_number)" );
    query.bindValue(":cmhead_number", _memoNumber->text().toInt());
    query.exec();
    if (query.first())
    {
      _cmheadid = query.value("cmhead_id").toInt();

      _cust->setReadOnly(TRUE);

      populate();
     
      if (query.value("cmhead_posted").toBool())
      {
        _memoDate->setEnabled(FALSE);
        _invoiceNumber->setEnabled(FALSE);
        _salesRep->setEnabled(FALSE);
        _commission->setEnabled(FALSE);
        _taxauth->setEnabled(FALSE);
        _customerPO->setEnabled(FALSE);
        _hold->setEnabled(FALSE);
        _miscCharge->setEnabled(FALSE);
        _miscChargeDescription->setEnabled(FALSE);
        _miscChargeAccount->setReadOnly(TRUE);
        _freight->setEnabled(FALSE);
        _comments->setReadOnly(TRUE);
        _invoiceList->hide();
        _shipToNumber->setEnabled(FALSE);
        _shipToName->setEnabled(FALSE);
        _cmitem->setEnabled(FALSE);
        _shipToList->hide();
        _save->hide();
        _new->hide();
        _delete->hide();
        _edit->hide();

        _mode = cView;
      }
      else
        _mode = cEdit;
    }
    else if (query.lastError().type() != QSqlError::None)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void creditMemo::sConvertShipto()
{
  if ((!_shipToAddr->isEnabled()) && (!_ignoreShiptoSignals))
  {
//  Convert the captive shipto to a free-form shipto
    _shipToNumber->clear();
    _shipToAddr->setEnabled(TRUE);

    _shiptoid = -1;
  }
}

void creditMemo::sCopyToShipto()
{
  _shiptoid = -1;
  _shipToNumber->clear();
  _shipToName->setText(_billtoName->text());
  if (_billToAddr->id() > 0)
    _shipToAddr->setId(_billToAddr->id());
  else
  {
    _shipToAddr->setLine1(_billToAddr->line1());
    _shipToAddr->setLine2(_billToAddr->line2());
    _shipToAddr->setLine3(_billToAddr->line1());
    _shipToAddr->setCity(_billToAddr->city());
    _shipToAddr->setState(_billToAddr->state());
    _shipToAddr->setPostalCode(_billToAddr->postalCode());
    _shipToAddr->setCountry(_billToAddr->country());
  }

  _taxauth->setId(_custtaxauthid);
}

void creditMemo::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cmhead_id", _cmheadid);
  params.append("shipto_id", _shiptoid);
  params.append("cust_id", _cust->id());
  params.append("invoiceNumber", _invoiceNumber->invoiceNumber());
  params.append("creditMemoNumber", _memoNumber->text().toInt());
  params.append("rsncode_id", _rsnCode->id());
  params.append("curr_id", _currency->id());
  params.append("effective", _memoDate->date());

  creditMemoItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void creditMemo::sEdit()
{
  ParameterList params;
  params.append("cmitem_id", _cmitem->id());
  params.append("cust_id", _cust->id());
  params.append("invoiceNumber", _invoiceNumber->invoiceNumber());
  params.append("creditMemoNumber", _memoNumber->text().toInt());
  params.append("curr_id", _currency->id());
  params.append("effective", _memoDate->date());

  if (_mode == cView)
    params.append("mode", "view");
  else
    params.append("mode", "edit");

  creditMemoItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void creditMemo::sDelete()
{
  q.prepare( "SELECT cmhead_posted "
             "FROM cmitem, cmhead "
             "WHERE ( (cmitem_cmhead_id=cmhead_id)"
             " AND (cmitem_id=:cmitem_id) );" );
  q.bindValue(":cmitem_id", _cmitem->id());
  q.exec();
  if (q.first())
  {
    if (q.value("cmhead_posted").toBool())
    {
      QMessageBox::information(this, "Line Item cannot be delete",
                               tr("<p>This Credit Memo has been Posted and "
				"this cannot be modified.") );
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (QMessageBox::question(this, "Delete current Line Item?",
                            tr("<p>Are you sure that you want to delete the "
			       "current Line Item?"),
			    QMessageBox::Yes | QMessageBox::Default,
			    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM cmitem "
               "WHERE (cmitem_id=:cmitem_id);" );
    q.bindValue(":cmitem_id", _cmitem->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void creditMemo::sFillList()
{
  q.prepare( "SELECT cmitem_id, cmitem_linenumber, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2), warehous_code,"
             "       quom.uom_name,"
             "       formatQty(cmitem_qtyreturned),"
             "       formatQty(cmitem_qtycredit),"
             "       puom.uom_name,"
             "       formatSalesPrice(cmitem_unitprice),"
             "       formatMoney((cmitem_qtycredit * cmitem_qty_invuomratio) * (cmitem_unitprice / cmitem_price_invuomratio)) "
             "FROM cmitem, itemsite, item, warehous, uom AS quom, uom AS puom "
             "WHERE ( (cmitem_itemsite_id=itemsite_id)"
             " AND (cmitem_qty_uom_id=quom.uom_id)"
             " AND (cmitem_price_uom_id=puom.uom_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (itemsite_warehous_id=warehous_id)"
             " AND (cmitem_cmhead_id=:cmhead_id) ) "
             "ORDER BY cmitem_linenumber;" );
  q.bindValue(":cmhead_id", _cmheadid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _cmitem->populate(q);

  sCalculateSubtotal();
  recalculateTax();

  _currency->setEnabled(_cmitem->topLevelItemCount() == 0);
}

void creditMemo::sCalculateSubtotal()
{
//  Determine the subtotal and line item tax
  XSqlQuery query;
  query.prepare( "SELECT SUM(round((cmitem_qtycredit * cmitem_qty_invuomratio) * (cmitem_unitprice / cmitem_price_invuomratio), 2)) AS subtotal,"
                 "       SUM(cmitem_tax_ratea) AS taxa,"
                 "       SUM(cmitem_tax_rateb) AS taxb,"
                 "       SUM(cmitem_tax_ratec) AS taxc "
                 "FROM cmitem, itemsite, item "
                 "WHERE ( (cmitem_itemsite_id=itemsite_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (cmitem_cmhead_id=:cmhead_id) );" );
  query.bindValue(":cmhead_id", _cmheadid);
  query.exec();
  if (query.first())
  {
    _subtotal->setLocalValue(query.value("subtotal").toDouble());

    _taxCache.setLine(query.value("taxa").toDouble(),
		      query.value("taxb").toDouble(),
		      query.value("taxc").toDouble());
  }
}

void creditMemo::sCalculateTotal()
{
  _total->setLocalValue(_subtotal->localValue() +
			_tax->localValue() +
			_freight->localValue() +
			_miscCharge->localValue() );
}

void creditMemo::populate()
{
  XSqlQuery cmhead;
  cmhead.prepare( "SELECT cmhead.*,"
                  "       cust_name, cust_ffbillto, cust_ffshipto,"
                  "       formatScrap(cmhead_commission) AS f_commission "
                  "FROM cust, cmhead "
                  "WHERE ( (cmhead_cust_id=cust_id)"
                  " AND (cmhead_id=:cmhead_id) );" );
  cmhead.bindValue(":cmhead_id", _cmheadid);
  cmhead.exec();
  if (cmhead.first())
  {
    if (cmhead.value("cmhead_posted").toBool())
      _status->setText(tr("Posted"));
    else
      _status->setText(tr("Unposted"));

    _salesRep->setId(cmhead.value("cmhead_salesrep_id").toInt());
    _commission->setText(cmhead.value("f_commission"));
    // do taxauth first so we can overwrite the result of the signal cascade
    _taxauth->setId(cmhead.value("cmhead_taxauth_id").toInt());
    if (cmhead.value("cmhead_tax_curr_id").isNull())
      _taxcurrid = cmhead.value("cmhead_curr_id").toInt();
    else
      _taxcurrid = cmhead.value("cmhead_tax_curr_id").toInt();

    _memoNumber->setText(cmhead.value("cmhead_number"));
    _cust->setId(cmhead.value("cmhead_cust_id").toInt());
    _memoDate->setDate(cmhead.value("cmhead_docdate").toDate(), true);
    _customerPO->setText(cmhead.value("cmhead_custponumber"));
    _hold->setChecked(cmhead.value("cmhead_hold").toBool());

    _currency->setId(cmhead.value("cmhead_curr_id").toInt());
    _taxCache.setAdjId(cmhead.value("cmhead_adjtax_id").toInt());
    _taxCache.setAdjType(cmhead.value("cmhead_adjtaxtype_id").toInt());
    _taxCache.setAdj(cmhead.value("cmhead_adjtax_ratea").toDouble(),
		      cmhead.value("cmhead_adjtax_rateb").toDouble(),
		      cmhead.value("cmhead_adjtax_ratec").toDouble());
    _taxCache.setAdjPct(cmhead.value("cmhead_adjtax_pcta").toDouble(),
			 cmhead.value("cmhead_adjtax_pctb").toDouble(),
			 cmhead.value("cmhead_adjtax_pctc").toDouble());
    _freight->setLocalValue(cmhead.value("cmhead_freight").toDouble());
    _taxCache.setFreightId(cmhead.value("cmhead_freighttax_id").toInt());
    _taxCache.setFreightType(cmhead.value("cmhead_freighttaxtype_id").toInt());
    _taxCache.setFreight(cmhead.value("cmhead_freighttax_ratea").toDouble(),
		      cmhead.value("cmhead_freighttax_rateb").toDouble(),
		      cmhead.value("cmhead_freighttax_ratec").toDouble());
    _taxCache.setFreightPct(cmhead.value("cmhead_freighttax_pcta").toDouble(),
			 cmhead.value("cmhead_freighttax_pctb").toDouble(),
			 cmhead.value("cmhead_freighttax_pctc").toDouble());

    _miscCharge->setLocalValue(cmhead.value("cmhead_misc").toDouble());
    _miscChargeDescription->setText(cmhead.value("cmhead_misc_descrip"));
    _miscChargeAccount->setId(cmhead.value("cmhead_misc_accnt_id").toInt());

    _comments->setText(cmhead.value("cmhead_comments").toString());

    bool ffBillTo = cmhead.value("cust_ffbillto").toBool();
    _billtoName->setEnabled(ffBillTo);
    _billToAddr->setEnabled(ffBillTo);

    _billtoName->setText(cmhead.value("cmhead_billtoname"));
    _billToAddr->setLine1(cmhead.value("cmhead_billtoaddress1").toString());
    _billToAddr->setLine2(cmhead.value("cmhead_billtoaddress2").toString());
    _billToAddr->setLine3(cmhead.value("cmhead_billtoaddress3").toString());
    _billToAddr->setCity(cmhead.value("cmhead_billtocity").toString());
    _billToAddr->setState(cmhead.value("cmhead_billtostate").toString());
    _billToAddr->setPostalCode(cmhead.value("cmhead_billtozip").toString());
    _billToAddr->setCountry(cmhead.value("cmhead_billtocountry").toString());

    if (_mode == cEdit)
      _ffShipto = cmhead.value("cust_ffshipto").toBool();
    else
      _ffShipto = FALSE;

    _shipToName->setEnabled(_ffShipto);

    _shiptoid = cmhead.value("cmhead_shipto_id").toInt();
    _shipToName->setText(cmhead.value("cmhead_shipto_name"));
    _shipToAddr->setLine1(cmhead.value("cmhead_shipto_address1").toString());
    _shipToAddr->setLine2(cmhead.value("cmhead_shipto_address2").toString());
    _shipToAddr->setLine3(cmhead.value("cmhead_shipto_address3").toString());
    _shipToAddr->setCity(cmhead.value("cmhead_shipto_city").toString());
    _shipToAddr->setState(cmhead.value("cmhead_shipto_state").toString());
    _shipToAddr->setPostalCode(cmhead.value("cmhead_shipto_zipcode").toString());
    _shipToAddr->setCountry(cmhead.value("cmhead_shipto_country").toString());

    if (!cmhead.value("cmhead_rsncode_id").isNull() && cmhead.value("cmhead_rsncode_id").toInt() != -1)
      _rsnCode->setId(cmhead.value("cmhead_rsncode_id").toInt());

    if (! cmhead.value("cmhead_invcnumber").toString().isEmpty())
      _invoiceNumber->setInvoiceNumber(cmhead.value("cmhead_invcnumber").toString());

    sFreightChanged();
  }
  else if (cmhead.lastError().type() != QSqlError::None)
  {
    systemError(this, cmhead.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void creditMemo::closeEvent(QCloseEvent *pEvent)
{
  if ( (_mode == cNew) && (_cmheadid != -1) )
  {
    if ( (_metrics->value("CMNumberGeneration") == "A") || 
         (_metrics->value("CMNumberGeneration") == "O")   )
      q.prepare("SELECT releaseCmNumber(:number) AS result;");
    else if (_metrics->value("CMNumberGeneration") == "S")
      q.prepare("SELECT releaseSoNumber(:number) AS result;");

    q.bindValue(":number", _memoNumber->text().toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "DELETE FROM cmitem "
               "WHERE (cmitem_cmhead_id=:cmhead_id);"

               "DELETE FROM cmhead "
               "WHERE (cmhead_id=:cmhead_id);" );
    q.bindValue(":cmhead_id", _cmheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  XMainWindow::closeEvent(pEvent);
}

void creditMemo::sTaxDetail()
{
  XSqlQuery taxq;
  taxq.prepare("UPDATE cmhead SET cmhead_taxauth_id=:taxauth,"
	       "  cmhead_freight=:freight,"
	       "  cmhead_freighttax_ratea=:freighta,"
	       "  cmhead_freighttax_rateb=:freightb,"
	       "  cmhead_freighttax_ratec=:freightc,"
	       "  cmhead_docdate=:docdate "
	       "WHERE (cmhead_id=:cmhead_id);");
  if (_taxauth->isValid())
    taxq.bindValue(":taxauth",	_taxauth->id());
  taxq.bindValue(":freight",	_freight->localValue());
  taxq.bindValue(":freighta",	_taxCache.freight(0));
  taxq.bindValue(":freightb",	_taxCache.freight(1));
  taxq.bindValue(":freightc",	_taxCache.freight(2));
  taxq.bindValue(":docdate",	_memoDate->date());
  taxq.bindValue(":cmhead_id",	_cmheadid);
  taxq.exec();
  if (taxq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("order_id",	_cmheadid);
  params.append("order_type",	"CM");

  if(cView == _mode)
    params.append("mode", "view");
  else
    params.append("mode", "edit");

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    populate();
}

void creditMemo::recalculateTax()
{
  XSqlQuery itemq;

  //  Determine the line item tax
  itemq.prepare( "SELECT SUM(cmitem_tax_ratea) AS itemtaxa,"
		 "       SUM(cmitem_tax_rateb) AS itemtaxb,"
		 "       SUM(cmitem_tax_ratec) AS itemtaxc "
		 "FROM cmitem "
		 "WHERE (cmitem_cmhead_id=:cmhead_id);" );
  itemq.bindValue(":cmhead_id", _cmheadid);
  itemq.exec();
  if (itemq.first())
  {
    _taxCache.setLine(itemq.value("itemtaxa").toDouble(),
		      itemq.value("itemtaxb").toDouble(),
		      itemq.value("itemtaxc").toDouble());
  }
  else if (itemq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_taxcurrid > 0)
    _tax->setLocalValue(CurrDisplay::convert(_taxcurrid, _tax->id(),
			_taxCache.total(), _tax->effective()));
  // changing _tax fires sCalculateTotal()
}

void creditMemo::sTaxAuthChanged()
{
  if (_cmheadid == -1 || _taxauthidCache == _taxauth->id())
    return;

  XSqlQuery taxauthq;
  taxauthq.prepare("SELECT COALESCE(taxauth_curr_id, :curr_id) AS taxauth_curr_id "
		   "FROM taxauth "
		   "WHERE (taxauth_id=:taxauth_id);");
  if (_taxauth->isValid())
    taxauthq.bindValue(":taxauth_id", _taxauth->id());
  taxauthq.bindValue(":curr_id", _currency->id());
  taxauthq.exec();
  if (taxauthq.first())
    _taxcurrid = taxauthq.value("taxauth_curr_id").toInt();
  else if (taxauthq.lastError().type() != QSqlError::None)
  {
    _taxauth->setId(_taxauthidCache);
    systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  taxauthq.prepare("SELECT changeCmheadTaxAuth(:cmhead_id, :taxauth_id) AS result;");
  taxauthq.bindValue(":cmhead_id", _cmheadid);
  taxauthq.bindValue(":taxauth_id", _taxauth->id());
  taxauthq.exec();
  if (taxauthq.first())
  {
    int result = taxauthq.value("result").toInt();
    if (result < 0)
    {
      _taxauth->setId(_taxauthidCache);
      systemError(this, storedProcErrorLookup("changeCmheadTaxAuth", result),
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

  taxauthq.prepare("SELECT cmhead_freighttax_id, cmhead_adjtax_ratea,"
		   "       cmhead_adjtax_rateb, cmhead_adjtax_ratec "
		   "FROM cmhead "
		   "WHERE (cmhead_id=:cmhead_id);");
  taxauthq.bindValue(":cmhead_id", _cmheadid);
  taxauthq.exec();
  if (taxauthq.first())
  {
    if (taxauthq.value("cmhead_freighttax_id").isNull())
      _taxCache.setFreightId(-1);
    else
      _taxCache.setFreightId(taxauthq.value("cmhead_freighttax_id").toInt());
    _taxCache.setAdj(taxauthq.value("cmhead_adjtax_ratea").toDouble(),
		     taxauthq.value("cmhead_adjtax_rateb").toDouble(),
		     taxauthq.value("cmhead_adjtax_ratec").toDouble());
  }
  else if (taxauthq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFreightChanged();
}

void creditMemo::sFreightChanged()
{
  XSqlQuery freightq;
  freightq.prepare("SELECT calculateTax(:tax_id, :freight, 0, 'A') AS freighta,"
		   "     calculateTax(:tax_id, :freight, 0, 'B') AS freightb,"
		   "     calculateTax(:tax_id, :freight, 0, 'C') AS freightc;");
  freightq.bindValue(":tax_id", _taxCache.freightId());
  freightq.bindValue(":freight", _freight->localValue());
  freightq.exec();
  if (freightq.first())
  {
    _taxCache.setFreight(freightq.value("freighta").toDouble(),
			 freightq.value("freightb").toDouble(),
			 freightq.value("freightc").toDouble());
  }
  else if (freightq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, freightq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  recalculateTax();
}
