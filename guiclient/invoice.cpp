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

#include "invoice.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>
#include <QWorkspace>

#include "invoiceItem.h"
#include "shipToList.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"

#define cViewQuote (0x20 | cView)

invoice::invoice(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, fl)
{
  if(name)
    setObjectName(name);

  setupUi(this);

  (void)statusBar();

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustomerInfo(int)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_shipToList, SIGNAL(clicked()), this, SLOT(sShipToList()));
  connect(_copyToShipto, SIGNAL(clicked()), this, SLOT(sCopyToShipto()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_shipToNumber, SIGNAL(lostFocus()), this, SLOT(sParseShipToNumber()));
  connect(_shipToNumber, SIGNAL(returnPressed()), this, SLOT(sParseShipToNumber()));
  connect(_shipToName, SIGNAL(textChanged(const QString&)), this, SLOT(sShipToModified()));
  connect(_subtotal, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_tax, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_miscAmount, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  connect(_allocatedCM, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_outstandingCM, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_authCC, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_shipToAddr, SIGNAL(changed()), this, SLOT(sShipToModified()));
  connect(_shipToPhone, SIGNAL(textChanged(const QString&)), this, SLOT(sShipToModified()));
  connect(_authCC, SIGNAL(idChanged(int)), this, SLOT(populateCCInfo()));
  connect(_allocatedCM, SIGNAL(idChanged(int)), this, SLOT(populateCMInfo()));
  connect(_outstandingCM, SIGNAL(idChanged(int)), this, SLOT(populateCMInfo()));
  connect(_authCC, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(populateCCInfo()));
  connect(_allocatedCM, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(populateCMInfo()));
  connect(_outstandingCM, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(populateCMInfo()));
  connect(_invcitem, SIGNAL(valid(bool)),       _edit, SLOT(setEnabled(bool)));
  connect(_invcitem, SIGNAL(valid(bool)),     _delete, SLOT(setEnabled(bool)));
  connect(_invcitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_taxauth,  SIGNAL(newID(int)),	 this, SLOT(sTaxAuthChanged()));
  connect(_shipChrgs, SIGNAL(newID(int)), this, SLOT(sHandleShipchrg(int)));

  statusBar()->hide();

  setFreeFormShipto(false);

#ifndef Q_WS_MAC
  _shipToList->setMaximumWidth(25);
#endif

  _taxCache.clear();
  _custtaxauthid  = -1;
  _invcheadid	  = -1;
  _shiptoid	  = -1;
  _taxauthidCache = -1;
  _loading = false;

  _invcitem->addColumn(tr("#"),           _seqColumn,      Qt::AlignCenter );
  _invcitem->addColumn(tr("Item"),        _itemColumn,     Qt::AlignLeft   );
  _invcitem->addColumn(tr("Description"), -1,              Qt::AlignLeft   );
  _invcitem->addColumn(tr("Qty. UOM"),    _uomColumn,      Qt::AlignLeft   );
  _invcitem->addColumn(tr("Ordered"),     _qtyColumn,      Qt::AlignRight  );
  _invcitem->addColumn(tr("Billed"),      _qtyColumn,      Qt::AlignRight  );
  _invcitem->addColumn(tr("Price UOM"),   _uomColumn,      Qt::AlignLeft   );
  _invcitem->addColumn(tr("Price"),       _moneyColumn,    Qt::AlignRight  );
  _invcitem->addColumn(tr("Extended"),    _bigMoneyColumn, Qt::AlignRight  );

  _custCurrency->setLabel(_custCurrencyLit);

  _project->setType(ProjectLineEdit::SalesOrder);
  if(!_metrics->boolean("UseProjects"))
    _project->hide();

  _miscAmount->setAllowNegative(true);
}

invoice::~invoice()
{
    // no need to delete child widgets, Qt does it all for us
}

void invoice::languageChange()
{
    retranslateUi(this);
}

enum SetResponse invoice::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("invchead_id", &valid);
  if (valid)
  {
    _invcheadid = param.toInt();
    populate();
    populateCMInfo();
    populateCCInfo();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      setName("invoice new");
      _mode = cNew;

      q.exec("SELECT NEXTVAL('invchead_invchead_id_seq') AS invchead_id;");
      if (q.first())
        _invcheadid = q.value("invchead_id").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      q.exec("SELECT fetchInvcNumber() AS number;");
      if (q.first())
        _invoiceNumber->setText(q.value("number").toString());
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _orderDate->setDate(omfgThis->dbDate());
      _shipDate->setDate(omfgThis->dbDate());
      _invoiceDate->setDate(omfgThis->dbDate(), true);

      q.prepare("INSERT INTO invchead ("
	        "    invchead_id, invchead_invcnumber, invchead_orderdate,"
		"    invchead_invcdate, invchead_cust_id, invchead_posted,"
		"    invchead_printed, invchead_commission, invchead_freight,"
		"    invchead_tax, invchead_misc_amount, invchead_shipchrg_id "
		") VALUES ("
		"    :invchead_id, :invchead_invcnumber, :invchead_orderdate, "
		"    :invchead_invcdate, -1, false,"
		"    false, 0, 0,"
		"    0, 0, -1"
		");");
      q.bindValue(":invchead_id",	 _invcheadid);
      q.bindValue(":invchead_invcnumber",_invoiceNumber->text());
      q.bindValue(":invchead_orderdate", _orderDate->date());
      q.bindValue(":invchead_invcdate",	 _invoiceDate->date());
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      connect(_cust,	    SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
      connect(_cust,        SIGNAL(valid(bool)), this, SLOT(populateCMInfo()));
      connect(_orderNumber, SIGNAL(lostFocus()), this, SLOT(populateCCInfo()));
    }
    else if (param.toString() == "edit")
    {
      setName(QString("invoice edit %1").arg(_invcheadid));
      _mode = cEdit;

      _new->setEnabled(TRUE);
      _cust->setReadOnly(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      setName(QString("invoice view %1").arg(_invcheadid));
      _mode = cView;

      _invoiceNumber->setEnabled(FALSE);
      _orderNumber->setEnabled(FALSE);
      _invoiceDate->setEnabled(FALSE);
      _shipDate->setEnabled(FALSE);
      _orderDate->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);
      _cust->setReadOnly(TRUE);
      _salesrep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);
      _taxauth->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _fob->setEnabled(FALSE);
      _shipVia->setEnabled(FALSE);
      _billToName->setEnabled(FALSE);
      _billToAddr->setEnabled(FALSE);
      _billToPhone->setEnabled(FALSE);
      _shipToNumber->setEnabled(FALSE);
      _shipToName->setEnabled(FALSE);
      _shipToAddr->setEnabled(FALSE);
      _shipToPhone->setEnabled(FALSE);
      _miscAmount->setEnabled(FALSE);
      _miscChargeDescription->setEnabled(FALSE);
      _miscChargeAccount->setReadOnly(TRUE);
      _freight->setEnabled(FALSE);
      _payment->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _shipToList->hide();
      _edit->hide();
      _save->hide();
      _delete->hide();
      _project->setEnabled(false);
      _shipChrgs->setEnabled(FALSE);

      disconnect(_invcitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      disconnect(_invcitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      disconnect(_invcitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_invcitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _close->setFocus();
    }
  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());

  return NoError;
}

void invoice::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM invcitem "
               "WHERE (invcitem_invchead_id=:invchead_id);"
               "DELETE FROM invchead "
               "WHERE (invchead_id=:invchead_id);" );
    q.bindValue(":invchead_id", _invcheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  close();
}

void invoice::sPopulateCustomerInfo(int pCustid)
{
  if (pCustid != -1)
  {
      XSqlQuery cust;
      cust.prepare( "SELECT cust_salesrep_id, formatScrap(cust_commprcnt) AS commission,"
		    "       cust_creditstatus, cust_terms_id, COALESCE(cust_taxauth_id, -1) AS cust_taxauth_id,"
		    "       cust_ffshipto, cust_ffbillto, "
		    "       COALESCE(shipto_id, -1) AS shiptoid, "
		    "       cust_curr_id "
		    "FROM custinfo LEFT OUTER JOIN shipto ON ( (shipto_cust_id=cust_id) AND (shipto_default) ) "
		    "WHERE (cust_id=:cust_id);" );
      cust.bindValue(":cust_id", pCustid);
      cust.exec();
      if (cust.first())
      {
	_salesrep->setId(cust.value("cust_salesrep_id").toInt());
	_commission->setText(cust.value("commission").toString());
	_terms->setId(cust.value("cust_terms_id").toInt());
	_custtaxauthid = cust.value("cust_taxauth_id").toInt();
	_taxauth->setId(cust.value("cust_taxauth_id").toInt());
	_custCurrency->setId(cust.value("cust_curr_id").toInt());

	bool ffBillTo = cust.value("cust_ffbillto").toBool();
	_billToName->setEnabled(ffBillTo);
	_billToAddr->setEnabled(ffBillTo);
	_billToPhone->setEnabled(ffBillTo);

	setFreeFormShipto(cust.value("cust_ffshipto").toBool());
	if (cust.value("shiptoid").toInt() != -1)
	  populateShipto(cust.value("shiptoid").toInt());
	else
	{
	  _shipToNumber->clear();
	  _shipToName->clear();
	  _shipToAddr->clear();
	  _shipToPhone->clear();
	}
      }
      if (cust.lastError().type() != QSqlError::None)
      {
	systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
  }
  else
  {
    _salesrep->setCurrentItem(-1);
    _commission->clear();
    _terms->setCurrentItem(-1);
    _custtaxauthid  = -1;
    _taxauthidCache = -1;
    _taxauth->setCurrentItem(-1);
  }
}

void invoice::sShipToList()
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

void invoice::sParseShipToNumber()
{
  XSqlQuery shiptoid;
  shiptoid.prepare( "SELECT shipto_id "
                    "FROM shipto "
                    "WHERE ( (shipto_cust_id=:shipto_cust_id)"
                    " AND (UPPER(shipto_num)=UPPER(:shipto_num)) );" );
  shiptoid.bindValue(":shipto_cust_id", _cust->id());
  shiptoid.bindValue(":shipto_num", _shipToNumber->text());
  shiptoid.exec();
  if (shiptoid.first())
    populateShipto(shiptoid.value("shipto_id").toInt());
  else if (_shiptoid != -1)
    populateShipto(-1);
  if (shiptoid.lastError().type() != QSqlError::None)
  {
    systemError(this, shiptoid.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void invoice::populateShipto(int pShiptoid)
{
  if (pShiptoid != -1)
  {
    XSqlQuery shipto;
    shipto.prepare( "SELECT shipto_num, shipto_name, shipto_addr_id, "
                    "       cntct_phone, shipto_shipvia,"
                    "       shipto_salesrep_id, COALESCE(shipto_taxauth_id, -1) AS shipto_taxauth_id,"
		    "       formatScrap(shipto_commission) AS commission "
                    "FROM shiptoinfo LEFT OUTER JOIN "
		    "     cntct ON (shipto_cntct_id=cntct_id)"
                    "WHERE (shipto_id=:shipto_id);" );
    shipto.bindValue(":shipto_id", pShiptoid);
    shipto.exec();
    if (shipto.first())
    {
      _shipToName->setText(shipto.value("shipto_name"));
      _shipToAddr->setId(shipto.value("shipto_addr_id").toInt());
      _shipToPhone->setText(shipto.value("cntct_phone"));
      _shipToNumber->setText(shipto.value("shipto_num"));
      _salesrep->setId(shipto.value("shipto_salesrep_id").toInt());
      _commission->setText(shipto.value("commission"));
      _shipVia->setText(shipto.value("shipto_shipvia"));
      _taxauth->setId(shipto.value("shipto_taxauth_id").toInt());
    }
    else if (shipto.lastError().type() != QSqlError::None)
    {
      systemError(this, shipto.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _shipToNumber->clear();
    _shipToName->clear();
    _shipToAddr->clear();
    _shipToPhone->clear();
  }

  _shiptoid = pShiptoid;
}

void invoice::sCopyToShipto()
{
  _shiptoid = -1;
  _shipToNumber->clear();
  _shipToName->setText(_billToName->text());
  _shipToAddr->setId(_billToAddr->id());
  _shipToPhone->setText(_billToPhone->text());
  _taxauth->setId(_custtaxauthid);
}

void invoice::sSave()
{
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _cust->id() <= 0,
      tr("<p>You must enter a Customer for this Invoice before saving it."),
      _cust
    },
    // TODO: add more error checks here?
    { true, "", NULL }
  };
  
  if (_total->localValue() < 0 )
  {
    QMessageBox::information(this, tr("Total Less than Zero"),
                             tr("<p>The Total must be a positive value.") );
    _cust->setFocus();
    return;
  }
  
  //  We can't post a Misc. Charge without a Sales Account
  if ( (! _miscAmount->isZero()) && (!_miscChargeAccount->isValid()) )
  {
    QMessageBox::warning( this, tr("No Misc. Charge Account Number"),
                          tr("<p>You may not enter a Misc. Charge without "
                             "indicating the G/L Sales Account number for the "
                             "charge.  Please set the Misc. Charge amount to 0 "
                             "or select a Misc. Charge Sales Account." ) );
    _tabWidget->setCurrentPage(1);
    _miscChargeAccount->setFocus();
    return;
  }


  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Invoice"), error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  // save address info in case someone wants to use 'em again later
  // but don't make any global changes to the data and ignore errors
  _billToAddr->save(AddressCluster::CHANGEONE);
  _shipToAddr->save(AddressCluster::CHANGEONE);

  q.prepare( "UPDATE invchead "
	     "SET invchead_cust_id=:invchead_cust_id,"
	     "    invchead_invcdate=:invchead_invcdate,"
	     "    invchead_shipdate=:invchead_shipdate,"
	     "    invchead_invcnumber=:invchead_invcnumber,"
	     "    invchead_ordernumber=:invchead_ordernumber,"
	     "    invchead_orderdate=:invchead_orderdate,"
	     "    invchead_ponumber=:invchead_ponumber,"
	     "    invchead_billto_name=:invchead_billto_name, invchead_billto_address1=:invchead_billto_address1,"
	     "    invchead_billto_address2=:invchead_billto_address2, invchead_billto_address3=:invchead_billto_address3,"
	     "    invchead_billto_city=:invchead_billto_city, invchead_billto_state=:invchead_billto_state,"
	     "    invchead_billto_zipcode=:invchead_billto_zipcode, invchead_billto_phone=:invchead_billto_phone,"
	     "    invchead_billto_country=:invchead_billto_country,"
	     "    invchead_shipto_id=:invchead_shipto_id,"
	     "    invchead_shipto_name=:invchead_shipto_name, invchead_shipto_address1=:invchead_shipto_address1,"
	     "    invchead_shipto_address2=:invchead_shipto_address2, invchead_shipto_address3=:invchead_shipto_address3,"
	     "    invchead_shipto_city=:invchead_shipto_city, invchead_shipto_state=:invchead_shipto_state,"
	     "    invchead_shipto_zipcode=:invchead_shipto_zipcode, invchead_shipto_phone=:invchead_shipto_phone,"
	     "    invchead_shipto_country=:invchead_shipto_country,"
	     "    invchead_salesrep_id=:invchead_salesrep_id,"
	     "    invchead_terms_id=:invchead_terms_id, invchead_commission=:invchead_commission,"
	     "    invchead_misc_amount=:invchead_misc_amount, invchead_misc_descrip=:invchead_misc_descrip,"
	     "    invchead_misc_accnt_id=:invchead_misc_accnt_id,"
	     "    invchead_adjtax_id=:invchead_adjtax_id,"
	     "    invchead_adjtax_ratea=:invchead_adjtax_ratea,"
	     "    invchead_adjtax_rateb=:invchead_adjtax_rateb,"
	     "    invchead_adjtax_ratec=:invchead_adjtax_ratec,"
	     "    invchead_freighttax_id=:invchead_freighttax_id,"
	     "    invchead_freighttax_ratea=:invchead_freighttax_ratea,"
	     "    invchead_freighttax_rateb=:invchead_freighttax_rateb,"
	     "    invchead_freighttax_ratec=:invchead_freighttax_ratec,"
	     "    invchead_freight=:invchead_freight,"
	     "    invchead_payment=:invchead_payment,"
	     "    invchead_curr_id=:invchead_curr_id,"
	     "    invchead_shipvia=:invchead_shipvia, invchead_fob=:invchead_fob, invchead_notes=:invchead_notes,"
             "    invchead_recurring=:invchead_recurring,"
             "    invchead_recurring_interval=:invchead_recurring_interval,"
             "    invchead_recurring_type=:invchead_recurring_type,"
             "    invchead_recurring_until=:invchead_recurring_until,"
	     "    invchead_prj_id=:invchead_prj_id, "
             "    invchead_shipchrg_id=:invchead_shipchrg_id "
	     "WHERE (invchead_id=:invchead_id);" );

  q.bindValue(":invchead_id",			_invcheadid);
  q.bindValue(":invchead_invcnumber",		_invoiceNumber->text().toInt());
  q.bindValue(":invchead_cust_id",		_cust->id());
  q.bindValue(":invchead_invcdate",		_invoiceDate->date());
  q.bindValue(":invchead_shipdate",		_shipDate->date());
  q.bindValue(":invchead_orderdate",		_orderDate->date());
  q.bindValue(":invchead_ponumber",		_poNumber->text());
  q.bindValue(":invchead_billto_name",		_billToName->text());
  q.bindValue(":invchead_billto_address1",	_billToAddr->line1());
  q.bindValue(":invchead_billto_address2",	_billToAddr->line2());
  q.bindValue(":invchead_billto_address3",	_billToAddr->line3());
  q.bindValue(":invchead_billto_city",		_billToAddr->city());
  q.bindValue(":invchead_billto_state",		_billToAddr->state());
  q.bindValue(":invchead_billto_zipcode",	_billToAddr->postalCode());
  q.bindValue(":invchead_billto_country",	_billToAddr->country());
  q.bindValue(":invchead_billto_phone",		_billToPhone->text());
  q.bindValue(":invchead_shipto_id",		_shiptoid);
  q.bindValue(":invchead_shipto_name",		_shipToName->text());
  q.bindValue(":invchead_shipto_address1",	_shipToAddr->line1());
  q.bindValue(":invchead_shipto_address2",	_shipToAddr->line2());
  q.bindValue(":invchead_shipto_address3",	_shipToAddr->line3());
  q.bindValue(":invchead_shipto_city",		_shipToAddr->city());
  q.bindValue(":invchead_shipto_state",		_shipToAddr->state());
  q.bindValue(":invchead_shipto_zipcode",	_shipToAddr->postalCode());
  q.bindValue(":invchead_shipto_country",	_shipToAddr->country());
  q.bindValue(":invchead_shipto_phone",		_shipToPhone->text());
  q.bindValue(":invchead_taxauth_id",		_taxauth->id());
  q.bindValue(":invchead_salesrep_id",		_salesrep->id());
  q.bindValue(":invchead_terms_id",		_terms->id());
  q.bindValue(":invchead_commission",	(_commission->toDouble() / 100.0));
  q.bindValue(":invchead_adjtax_ratea",		_taxCache.adj(0));
  q.bindValue(":invchead_adjtax_rateb",		_taxCache.adj(1));
  q.bindValue(":invchead_adjtax_ratec",		_taxCache.adj(2));
  q.bindValue(":invchead_freighttax_ratea",	_taxCache.freight(0));
  q.bindValue(":invchead_freighttax_rateb",	_taxCache.freight(1));
  q.bindValue(":invchead_freighttax_ratec",	_taxCache.freight(2));
  q.bindValue(":invchead_freight",	_freight->localValue());
  q.bindValue(":invchead_payment",	_payment->localValue());
  q.bindValue(":invchead_curr_id",	_custCurrency->id());
  q.bindValue(":invchead_misc_amount",	_miscAmount->localValue());
  q.bindValue(":invchead_misc_descrip",	_miscChargeDescription->text());
  q.bindValue(":invchead_misc_accnt_id",_miscChargeAccount->id());
  q.bindValue(":invchead_shipvia",	_shipVia->currentText());
  q.bindValue(":invchead_fob",		_fob->text());
  q.bindValue(":invchead_notes",	_notes->text());
  q.bindValue(":invchead_prj_id",	_project->id());
  q.bindValue(":invchead_shipchrg_id",	_shipChrgs->id());
  q.bindValue(":invchead_recurring", QVariant(_recurring->isChecked(), 0));
  if(_recurring->isChecked()) // only set the following if it's recurring otherwise they end upp null
  {
    QString rtype[] = {"D", "W", "M", "Y"};
    q.bindValue(":invchead_recurring_interval", _recurringInterval->value());
    q.bindValue(":invchead_recurring_type", rtype[_recurringType->currentIndex()]);
    if(_recurringUntil->isValid())
      q.bindValue(":invchead_recurring_until", _recurringUntil->date());
  }

  if (_orderNumber->text().length())
    q.bindValue(":invchead_ordernumber", _orderNumber->text().toInt());
  if (_taxCache.adjId() > 0)
    q.bindValue(":invchead_adjtax_id", _taxCache.adjId());
  if (_taxCache.freightId() > 0)
    q.bindValue(":invchead_freighttax_id", _taxCache.freightId());

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sInvoicesUpdated(_invcheadid, TRUE);

  _invcheadid = -1;
  close();
}

void invoice::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("invchead_id", _invcheadid);
  params.append("invoiceNumber", _invoiceNumber->text());
  params.append("cust_id", _cust->id());
  params.append("cust_curr_id", _custCurrency->id());
  params.append("curr_date", _orderDate->date());

  invoiceItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillItemList();
}

void invoice::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("invcitem_id", _invcitem->id());
  params.append("invoiceNumber", _invoiceNumber->text());
  params.append("cust_id", _cust->id());
  params.append("cust_curr_id", _custCurrency->id());
  params.append("curr_date", _orderDate->date());

  invoiceItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillItemList();
}

void invoice::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("invcitem_id", _invcitem->id());
  params.append("invoiceNumber", _invoiceNumber->text());
  params.append("cust_id", _cust->id());
  params.append("cust_curr_id", _custCurrency->id());
  params.append("curr_date", _orderDate->date());

  invoiceItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void invoice::sDelete()
{
  q.prepare( "DELETE FROM invcitem "
             "WHERE (invcitem_id=:invcitem_id);" );
  q.bindValue(":invcitem_id", _invcitem->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillItemList();
}

void invoice::populate()
{
  q.prepare( "SELECT invchead.*, "
             "    COALESCE(invchead_taxauth_id, -1) AS taxauth_id,"
             "    currToCurr(invchead_tax_curr_id, invchead_curr_id, "
             "               invchead_tax, invchead_invcdate) AS invccurrtax, "
             "    COALESCE(cust_taxauth_id, -1) AS cust_taxauth_id,"
	     "    cust_ffbillto, cust_ffshipto, "
	     "    invchead_shipchrg_id, invchead_ordernumber, "
             "    CASE WHEN invchead_shipchrg_id = -1 AND cohead_shipchrg_id = -1 THEN -1  "
             "    WHEN NOT(invchead_shipchrg_id = -1) THEN invchead_shipchrg_id "
             "    ELSE cohead_shipchrg_id "
             "    END AS shipchrg_id "
             "FROM invchead "
             "LEFT OUTER JOIN cohead "
             "ON cohead.cohead_number = invchead.invchead_ordernumber, "
             "custinfo "
             "WHERE ( (invchead_cust_id=cust_id)"
             " AND (invchead_id=:invchead_id) );" );
  q.bindValue(":invchead_id", _invcheadid);
  q.exec();
  if (q.first())
  {
    _loading = true;
    // We are setting the _taxauthidCache here to the value that
    // sPopulateCustomerInfo is going to set the _taxauth to.
    // The reason for doing this is to prevent sPopulateCustomerInfo
    // from triggering the sTaxAuthChanged() function which does some
    // database manipulation that may be unnecessary or even harmful
    // and we intend to set the values a little further down and they
    // may differ.
    _taxauthidCache = q.value("cust_taxauth_id").toInt(); 

    _cust->setId(q.value("invchead_cust_id").toInt());
    _custCurrency->setId(q.value("invchead_curr_id").toInt());

    _invoiceNumber->setText(q.value("invchead_invcnumber").toString());
    _orderNumber->setText(q.value("invchead_ordernumber").toString());
    if (! _orderNumber->text().isEmpty() && _orderNumber->text().toInt() != 0)
	_custCurrency->setEnabled(FALSE);

    _invoiceDate->setDate(q.value("invchead_invcdate").toDate(), true);
    _orderDate->setDate(q.value("invchead_orderdate").toDate());
    _shipDate->setDate(q.value("invchead_shipdate").toDate());
    _poNumber->setText(q.value("invchead_ponumber").toString());
    _shipVia->setText(q.value("invchead_shipvia").toString());
    _fob->setText(q.value("invchead_fob").toString());
    _shipChrgs->setId(q.value("shipchrg_id").toInt());

    if(q.value("invchead_recurring").toBool())
    {
      _recurring->setChecked(true);
      _recurringInterval->setValue(q.value("invchead_recurring_interval").toInt());
      QString rtype = q.value("invchead_recurring_type").toString();
      if("Y" == rtype)
        _recurringType->setCurrentIndex(3);
      else if("M" == rtype)
        _recurringType->setCurrentIndex(2);
      else if("W" == rtype)
        _recurringType->setCurrentIndex(1);
      else if("D" == rtype)
        _recurringType->setCurrentIndex(0);
      _recurringUntil->setDate(q.value("invchead_recurring_until").toDate());
    }

    _salesrep->setId(q.value("invchead_salesrep_id").toInt());
    _commission->setText(formatPercent(q.value("invchead_commission").toDouble()));
    _taxauthidCache = q.value("taxauth_id").toInt(); 
    _taxauth->setId(_taxauthidCache);
    _taxcurrid = q.value("invchead_tax_curr_id").toInt();
    _tax->setLocalValue(q.value("invccurrtax").toDouble());
    _terms->setId(q.value("invchead_terms_id").toInt());
    _project->setId(q.value("invchead_prj_id").toInt());

    bool ffBillTo = q.value("cust_ffbillto").toBool();
    _billToName->setEnabled(ffBillTo);
    _billToAddr->setEnabled(ffBillTo);
    _billToPhone->setEnabled(ffBillTo);

    _billToName->setText(q.value("invchead_billto_name").toString());
    _billToAddr->setLine1(q.value("invchead_billto_address1").toString());
    _billToAddr->setLine2(q.value("invchead_billto_address2").toString());
    _billToAddr->setLine3(q.value("invchead_billto_address3").toString());
    _billToAddr->setCity(q.value("invchead_billto_city").toString());
    _billToAddr->setState(q.value("invchead_billto_state").toString());
    _billToAddr->setPostalCode(q.value("invchead_billto_zipcode").toString());
    _billToAddr->setCountry(q.value("invchead_billto_country").toString());
    _billToPhone->setText(q.value("invchead_billto_phone").toString());

    setFreeFormShipto(q.value("cust_ffshipto").toBool());
    _shipToName->setText(q.value("invchead_shipto_name").toString());
    _shipToAddr->setLine1(q.value("invchead_shipto_address1").toString());
    _shipToAddr->setLine2(q.value("invchead_shipto_address2").toString());
    _shipToAddr->setLine3(q.value("invchead_shipto_address3").toString());
    _shipToAddr->setCity(q.value("invchead_shipto_city").toString());
    _shipToAddr->setState(q.value("invchead_shipto_state").toString());
    _shipToAddr->setPostalCode(q.value("invchead_shipto_zipcode").toString());
    _shipToAddr->setCountry(q.value("invchead_shipto_country").toString());
    _shipToPhone->setText(q.value("invchead_shipto_phone").toString());
    _shiptoid = q.value("invchead_shipto_id").toInt();
    _shipToNumber->clear();
    if(_shiptoid != -1)
    {
      XSqlQuery shipto;
      shipto.prepare("SELECT shipto_num FROM shipto WHERE shipto_id=:shipto_id");
      shipto.bindValue(":shipto_id", _shiptoid);
      shipto.exec();
      if(shipto.first())
        _shipToNumber->setText(shipto.value("shipto_num"));
      else
        _shiptoid = -1;
      if (shipto.lastError().type() != QSqlError::None)
	systemError(this, shipto.lastError().databaseText(), __FILE__, __LINE__);
    }

    _freight->setLocalValue(q.value("invchead_freight").toDouble());
    _payment->setLocalValue(q.value("invchead_payment").toDouble());
    _miscChargeDescription->setText(q.value("invchead_misc_descrip"));
    _miscChargeAccount->setId(q.value("invchead_misc_accnt_id").toInt());
    _miscAmount->setLocalValue(q.value("invchead_misc_amount").toDouble());

    if (q.value("invchead_adjtax_id").isNull())
      _taxCache.setAdjId(-1);
    else
      _taxCache.setAdjId(q.value("invchead_adjtax_id").toInt());
    _taxCache.setAdj(q.value("invchead_adjtax_ratea").toDouble(),
		     q.value("invchead_adjtax_rateb").toDouble(),
		     q.value("invchead_adjtax_ratec").toDouble());
    _taxCache.setAdjPct(q.value("invchead_adjtax_pcta").toDouble(),
		        q.value("invchead_adjtax_pctb").toDouble(),
		        q.value("invchead_adjtax_pctc").toDouble());

    if (q.value("invchead_freighttax_id").isNull())
      _taxCache.setFreightId(-1);
    else
      _taxCache.setFreightId(q.value("invchead_freighttax_id").toInt());
    _taxCache.setFreight(q.value("invchead_freighttax_ratea").toDouble(),
			 q.value("invchead_freighttax_rateb").toDouble(),
			 q.value("invchead_freighttax_ratec").toDouble());
    _taxCache.setFreightPct(q.value("invchead_freighttax_pcta").toDouble(),
			    q.value("invchead_freighttax_pctb").toDouble(),
			    q.value("invchead_freighttax_pctc").toDouble());

    _notes->setText(q.value("invchead_notes").toString());

    _loading = false;

    sFillItemList();
  }
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void invoice::sFillItemList()
{
  q.prepare( "SELECT invcitem_id, invcitem_linenumber,"
             "       CASE WHEN (item_id IS NULL) THEN invcitem_number"
             "            ELSE item_number"
             "       END AS itemnumber,"
             "       CASE WHEN (item_id IS NULL) THEN invcitem_descrip"
             "            ELSE (item_descrip1 || ' ' || item_descrip2)"
             "       END AS itemdescription,"
             "       quom.uom_name,"
             "       formatQty(invcitem_ordered) AS f_ordered,"
             "       formatQty(invcitem_billed) AS f_billed,"
             "       puom.uom_name,"
             "       formatSalesPrice(invcitem_price) AS f_price,"
             "       formatMoney(round((invcitem_billed * invcitem_qty_invuomratio) * (invcitem_price / "
	     "                  (CASE WHEN(item_id IS NULL) THEN 1 "
	     "			      ELSE invcitem_price_invuomratio END)), 2)) AS f_extend "
             "FROM invcitem LEFT OUTER JOIN item on (invcitem_item_id=item_id) "
             "  LEFT OUTER JOIN uom AS quom ON (invcitem_qty_uom_id=quom.uom_id)"
             "  LEFT OUTER JOIN uom AS puom ON (invcitem_price_uom_id=puom.uom_id)"
             "WHERE (invcitem_invchead_id=:invchead_id) "
             "ORDER BY invcitem_linenumber;" );
  q.bindValue(":invchead_id", _invcheadid);
  q.bindValue(":curr_id", _custCurrency->id());
  q.bindValue(":date", _orderDate->date());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _invcitem->clear();
  _invcitem->populate(q);

  //  Determine the subtotal
  q.prepare( "SELECT SUM( round(((invcitem_billed * invcitem_qty_invuomratio) * (invcitem_price /"
             "            CASE WHEN (item_id IS NULL) THEN 1"
             "                 ELSE invcitem_price_invuomratio"
             "            END)),2) ) AS subtotal "
             "FROM invcitem LEFT OUTER JOIN item ON (invcitem_item_id=item_id) "
             "WHERE (invcitem_invchead_id=:invchead_id);" );
  q.bindValue(":invchead_id", _invcheadid);
  q.exec();
  if (q.first())
    _subtotal->setLocalValue(q.value("subtotal").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _custCurrency->setEnabled(_invcitem->topLevelItemCount() == 0);

  // TODO: Calculate the Freight weight here.
  recalculateTax();
}

void invoice::sFreightChanged()
{
  XSqlQuery freightq;
  freightq.prepare("SELECT currToCurr(:dispcurr, :taxcurr, calculateTax(:tax_id, :freight, 0, 'A'), :invcdate) AS freighta,"
		   "     currtoCurr(:dspcurr, :taxcurr, calculateTax(:tax_id, :freight, 0, 'B'), :invcdate) AS freightb,"
		   "     currtoCurr(:dspcurr, :taxcurr, calculateTax(:tax_id, :freight, 0, 'C'), :invcdate) AS freightc;");
  freightq.bindValue(":tax_id", _taxCache.freightId());
  freightq.bindValue(":freight", _freight->localValue());
  freightq.bindValue(":dispcurr", _freight->id());
  freightq.bindValue(":taxcurr",  _taxcurrid);
  freightq.bindValue(":invcdate", _invoiceDate->date());
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

void invoice::sCalculateTotal()
{
  _total->setLocalValue(_subtotal->localValue() + _miscAmount->localValue() +
			_freight->localValue() + _tax->localValue());
  _balance->setLocalValue(_total->localValue() -
			  _allocatedCM->localValue() - _authCC->localValue() -
			  _outstandingCM->localValue());
  if (_balance->localValue() < 0)
    _balance->setLocalValue(0);
}

void invoice::closeEvent(QCloseEvent *pEvent)
{
  if ( (_mode == cNew) && (_invcheadid != -1) )
  {
    q.prepare( "DELETE FROM invcitem "
               "WHERE (invcitem_invchead_id=:invchead_id);"
               "SELECT releaseInvcNumber(:invoiceNumber);" );
    q.bindValue(":invchead_id", _invcheadid);
    q.bindValue(":invoiceNumber", _invoiceNumber->text().toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  XMainWindow::closeEvent(pEvent);
}

void invoice::sTaxDetail()
{
  XSqlQuery taxq;
  if (_mode != cView)
  {
    taxq.prepare("UPDATE invchead SET invchead_taxauth_id=:taxauth, "
		    "  invchead_freight=:freight,"
		    "  invchead_freighttax_ratea=:freighta,"
		    "  invchead_freighttax_rateb=:freightb,"
		    "  invchead_freighttax_ratec=:freightc,"
		    "  invchead_invcdate=:invchead_invcdate "
		    "WHERE (invchead_id=:invchead_id);");
    if (_taxauth->isValid())
      taxq.bindValue(":taxauth",	_taxauth->id());
    taxq.bindValue(":freight",		_freight->localValue());
    taxq.bindValue(":invchead_id",	_invcheadid);
    taxq.bindValue(":freighta",		_taxCache.freight(0));
    taxq.bindValue(":freightb",		_taxCache.freight(1));
    taxq.bindValue(":freightc",		_taxCache.freight(2));
    taxq.bindValue(":invchead_invcdate",_invoiceDate->date());
    taxq.exec();
    if (taxq.lastError().type() != QSqlError::None)
    {
      systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  ParameterList params;
  params.append("order_id", _invcheadid);
  params.append("order_type", "I");

  if (_mode == cView)
    params.append("mode", "view");
  else if (_mode == cNew || _mode == cEdit)
    params.append("mode", "edit");

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
  {
    populate();
  }
}

void invoice::recalculateTax()
{
  XSqlQuery itemq;

  //  Determine the line item tax
  itemq.prepare( "SELECT SUM(invcitem_tax_ratea) AS itemtaxa,"
		 "       SUM(invcitem_tax_rateb) AS itemtaxb,"
		 "       SUM(invcitem_tax_ratec) AS itemtaxc "
		 "FROM invcitem "
		 "WHERE (invcitem_invchead_id=:invchead_id);" );
  itemq.bindValue(":invchead_id", _invcheadid);
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

  _tax->setLocalValue(CurrDisplay::convert(_taxcurrid, _tax->id(),
	      _taxCache.total(), _tax->effective()));
  // changing _tax fires sCalculateTotal()
}

void invoice::setFreeFormShipto(bool pFreeForm)
{
  _ffShipto = pFreeForm;

  _shipToName->setEnabled(_ffShipto);
  _shipToAddr->setEnabled(_ffShipto);
  _shipToPhone->setEnabled(_ffShipto);

  if (_mode != cView)
    _copyToShipto->setEnabled(_ffShipto);
  else
    _copyToShipto->setEnabled(false);
}

void invoice::sShipToModified()
{
  _shiptoid = -1;
  _shipToNumber->clear();
}

void invoice::keyPressEvent( QKeyEvent * e )
{
#ifdef Q_WS_MAC
  if(e->key() == Qt::Key_N && e->state() == Qt::ControlModifier)
  {
    _new->animateClick();
    e->accept();
  }
  else if(e->key() == Qt::Key_E && e->state() == Qt::ControlModifier)
  {
    _edit->animateClick();
    e->accept();
  }
  if(e->isAccepted())
    return;
#endif
  e->ignore();
}

void invoice::newInvoice(int pCustid)
{
  // Check for an Item window in new mode already.
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->name(), "invoice new")==0)
    {
      w->setFocus();
      if(omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "new");
  if(-1 != pCustid)
    params.append("cust_id", pCustid);

  invoice *newdlg = new invoice();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void invoice::editInvoice( int pId )
{
  // Check for an Item window in edit mode for the specified invoice already.
  QString n = QString("invoice edit %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->name(), n)==0)
    {
      w->setFocus();
      if(omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("invchead_id", pId);
  params.append("mode", "edit");

  invoice *newdlg = new invoice();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void invoice::viewInvoice( int pId )
{
  // Check for an Item window in edit mode for the specified invoice already.
  QString n = QString("invoice view %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->name(), n)==0)
    {
      w->setFocus();
      if(omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("invchead_id", pId);
  params.append("mode", "view");

  invoice *newdlg = new invoice();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void invoice::populateCMInfo()
{
  XSqlQuery cm;

  // Allocated C/M's
  if (_mode == cNew)
  {
    cm.prepare("SELECT COALESCE(SUM(currToCurr(aropen_curr_id, :curr_id,"
	      "                               aropenco_amount, :effective)),0) AS amount"
	      "  FROM aropenco, cohead, aropen"
	      " WHERE ( (aropenco_cohead_id=cohead_id)"
	      "   AND   (aropenco_aropen_id=aropen_id)"
	      "   AND   (cohead_number=:cohead_number) );");
    cm.bindValue(":cohead_number", _orderNumber->text());
  }
  else
  {
    cm.prepare("SELECT COALESCE(SUM(currToCurr(aropen_curr_id, :curr_id,"
	      "                               aropenco_amount, :effective)),0) AS amount"
	      "  FROM aropenco, cohead, invchead, aropen"
	      " WHERE ( (aropenco_cohead_id=cohead_id)"
	      "   AND   (aropenco_aropen_id=aropen_id)"
	      "   AND   (invchead_ordernumber=cohead_number)"
	      "   AND   (invchead_id=:invchead_id) ); ");
    cm.bindValue(":invchead_id", _invcheadid);
  }
  cm.bindValue(":curr_id",     _allocatedCM->id());
  cm.bindValue(":effective",   _allocatedCM->effective());
  cm.exec();
  if(cm.first())
    _allocatedCM->setLocalValue(cm.value("amount").toDouble());
  else if (cm.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cm.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _allocatedCM->setLocalValue(0.00);

  // Unallocated C/M's
  cm.prepare("SELECT SUM(amount) AS amount"
            "  FROM ( SELECT aropen_id, "
	    "                currToCurr(aropen_curr_id, :curr_id,"
	    "                           noNeg(aropen_amount - aropen_paid - SUM(COALESCE(aropenco_amount,0))),"
	    "                           :effective) AS amount"
            "           FROM aropen LEFT OUTER JOIN aropenco ON (aropenco_aropen_id=aropen_id)"
            "          WHERE ( (aropen_cust_id=:cust_id)"
            "            AND   (aropen_doctype IN ('C', 'R'))"
            "            AND   (aropen_open))"
            "          GROUP BY aropen_id, aropen_amount, aropen_paid, aropen_curr_id) AS data; ");
  cm.bindValue(":cust_id",     _cust->id());
  cm.bindValue(":curr_id",     _outstandingCM->id());
  cm.bindValue(":effective",   _outstandingCM->effective());
  cm.exec();
  if(cm.first())
    _outstandingCM->setLocalValue(cm.value("amount").toDouble());
  else if (cm.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cm.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _outstandingCM->setLocalValue(0.00);
}

void invoice::populateCCInfo()
{
  XSqlQuery cc;
  int ccValidDays = _metrics->value("CCValidDays").toInt();
  if(ccValidDays < 1)
    ccValidDays = 7;

  if (_mode == cNew)
  {
    cc.prepare("SELECT COALESCE(SUM(currToCurr(payco_curr_id, :curr_id,"
	      "                               payco_amount, :effective)),0) AS amount"
	      "  FROM ccpay, payco, cohead"
	      " WHERE ( (ccpay_status = 'A')"
	      "   AND   (date_part('day', CURRENT_TIMESTAMP - ccpay_transaction_datetime) < :ccValidDays)"
	      "   AND   (payco_ccpay_id=ccpay_id)"
	      "   AND   (payco_cohead_id=cohead_id)"
	      "   AND   (cohead_number=:cohead_number) );");
    cc.bindValue(":cohead_number", _orderNumber->text());
  }
  else
  {
    cc.prepare("SELECT COALESCE(SUM(currToCurr(payco_curr_id, :curr_id,"
	      "                               payco_amount, :effective)),0) AS amount"
	      "  FROM ccpay, payco, cohead, invchead"
	      " WHERE ( (ccpay_status = 'A')"
	      "   AND   (date_part('day', CURRENT_TIMESTAMP - ccpay_transaction_datetime) < :ccValidDays)"
	      "   AND   (payco_ccpay_id=ccpay_id)"
	      "   AND   (payco_cohead_id=cohead_id)"
	      "   AND   (invchead_ordernumber=cohead_number)"
	      "   AND   (invchead_id=:invchead_id) ); ");
    cc.bindValue(":invchead_id", _invcheadid);
  }
  cc.bindValue(":ccValidDays", ccValidDays);
  cc.bindValue(":curr_id",     _authCC->id());
  cc.bindValue(":effective",   _authCC->effective());
  cc.exec();
  if(cc.first())
    _authCC->setLocalValue(cc.value("amount").toDouble());
  else if (cc.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cc.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _authCC->setLocalValue(0.00);
}

void invoice::sTaxAuthChanged()
{
  if (cView == _mode || _loading || _taxauthidCache == _taxauth->id())
    return;

  XSqlQuery taxauthq;
  taxauthq.prepare("SELECT COALESCE(taxauth_curr_id, :curr_id) AS taxauth_curr_id "
		   "FROM taxauth "
		   "WHERE (taxauth_id=:taxauth_id);");
  taxauthq.bindValue(":taxauth_id", _taxauth->id());
  taxauthq.bindValue(":curr_id",    _custCurrency->id());
  taxauthq.exec();
  if (taxauthq.first())
    _taxcurrid = taxauthq.value("taxauth_curr_id").toInt();
  else if (taxauthq.lastError().type() != QSqlError::None)
  {
    _taxauth->setId(_taxauthidCache);
    systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  taxauthq.prepare("SELECT changeInvoiceTaxAuth(:invchead_id, :taxauth_id) AS result;");
  taxauthq.bindValue(":invchead_id", _invcheadid);
  taxauthq.bindValue(":taxauth_id", _taxauth->id());
  taxauthq.exec();
  if (taxauthq.first())
  {
    int result = taxauthq.value("result").toInt();
    if (result < 0)
    {
      _taxauth->setId(_taxauthidCache);
      systemError(this, storedProcErrorLookup("changeInvoiceTaxAuth", result),
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

  taxauthq.prepare("SELECT invchead_freighttax_id, invchead_adjtax_ratea,"
		   "       invchead_adjtax_rateb, invchead_adjtax_ratec "
		   "FROM invchead "
		   "WHERE (invchead_id=:invchead_id);");
  taxauthq.bindValue(":invchead_id", _invcheadid);
  taxauthq.exec();
  if (taxauthq.first())
  {
    if (taxauthq.value("invchead_freighttax_id").isNull())
      _taxCache.setFreightId(-1);
    else
      _taxCache.setFreightId(taxauthq.value("invchead_freighttax_id").toInt());
    _taxCache.setAdj(taxauthq.value("invchead_adjtax_ratea").toDouble(),
		     taxauthq.value("invchead_adjtax_rateb").toDouble(),
		     taxauthq.value("invchead_adjtax_ratec").toDouble());
  }
  else if (taxauthq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFreightChanged();
}

void invoice::sHandleShipchrg(int pShipchrgid)
{
  if ( (_mode == cView) || (_mode == cViewQuote) )
    _freight->setEnabled(FALSE);
  else
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
  }
}

