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

#include "returnAuthorization.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QValidator>
#include <QVariant>

#include <metasql.h>

#include "creditcardprocessor.h"
#include "enterPoitemReceipt.h"
#include "enterPoReceipt.h"
#include "mqlutil.h"
#include "returnAuthorizationItem.h"
#include "shipToList.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"
#include "printRaForm.h"

#include "salesOrder.h"
#include "salesOrderItem.h"
#include "dspShipmentsBySalesOrder.h"
#include "dspSalesOrderStatus.h"

#define TO_RECEIVE_COL 11

returnAuthorization::returnAuthorization(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_copyToShipto, SIGNAL(clicked()), this, SLOT(sCopyToShipto()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_shipToNumber, SIGNAL(lostFocus()), this, SLOT(sParseShipToNumber()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSaveClick()));
  connect(_shipToList, SIGNAL(clicked()), this, SLOT(sShipToList()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_subtotal, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_tax, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_miscCharge, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_freight, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  connect(_taxauth, SIGNAL(newID(int)), this, SLOT(sTaxAuthChanged()));
  connect(_origso, SIGNAL(newId(int)), this, SLOT(sOrigSoChanged()));
  connect(_shipToAddr, SIGNAL(changed()), this, SLOT(sClearShiptoNumber()));
  connect(_disposition, SIGNAL(currentIndexChanged(int)), this, SLOT(sDispositionChanged()));
  connect(_creditBy, SIGNAL(currentIndexChanged(int)), this, SLOT(sCreditByChanged()));
  connect(_authorizeLine, SIGNAL(clicked()), this, SLOT(sAuthorizeLine()));
  connect(_clearAuthorization, SIGNAL(clicked()), this, SLOT(sClearAuthorization()));
  connect(_authorizeAll, SIGNAL(clicked()), this, SLOT(sAuthorizeAll()));
  connect(_enterReceipt, SIGNAL(clicked()), this, SLOT(sEnterReceipt()));
  connect(_receiveAll,   SIGNAL(clicked()), this, SLOT(sReceiveAll()));
  connect(_raitem,     SIGNAL(valid(bool)), this, SLOT(sHandleEnterReceipt(bool)));
  connect(_raitem,     SIGNAL(valid(bool)), this, SLOT(sHandleAction()));
  connect(_action, SIGNAL(clicked()), this, SLOT(sAction()));
  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
  connect(_refund, SIGNAL(clicked()), this, SLOT(sRefund()));
  connect(_postReceipts, SIGNAL(clicked()), this, SLOT(sPostReceipts()));
  connect(_raitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_authNumber, SIGNAL(textEdited(QString)), this, SLOT(sCheckNumber()));

  _commission->setValidator(omfgThis->percentVal());
  _newso->setReadOnly(true);

#ifndef Q_WS_MAC
  _shipToList->setMaximumWidth(25);
#endif

  _taxcurrid           = -1;
  _custtaxauthid       = -1;
  _taxauthidCache      = -1;
  _taxCache.clear();
  _shiptoid            = -1;
  _ignoreShiptoSignals = false;
  _ignoreSoSignals = false;
  _ffBillto = TRUE;
  _ffShipto = TRUE;
  _custEmail = FALSE;

  _origso->setType((cSoReleased));
  _authNumber->setValidator(omfgThis->orderVal());
  _comments->setType(Comments::ReturnAuth);

  _currency->setLabel(_currencyLit);

  _raitem->addColumn(tr("#"),           _seqColumn,   Qt::AlignCenter );
  _raitem->addColumn(tr("Item"),        _itemColumn,  Qt::AlignLeft   );
  _raitem->addColumn(tr("UOM"),         _statusColumn,Qt::AlignLeft   );
  _raitem->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _raitem->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter );
  _raitem->addColumn(tr("Status"),      _statusColumn,Qt::AlignCenter );
  _raitem->addColumn(tr("Disposition"), _itemColumn,  Qt::AlignLeft   );
  _raitem->addColumn(tr("Warranty"),    _qtyColumn,Qt::AlignCenter );
  _raitem->addColumn(tr("Sold"),        _qtyColumn,   Qt::AlignRight  );
  _raitem->addColumn(tr("Authorized"),  _qtyColumn,   Qt::AlignRight  );
  _raitem->addColumn(tr("Received"),    _qtyColumn,   Qt::AlignRight  );
  _raitem->addColumn(tr("To Receive"),  _qtyColumn,   Qt::AlignRight  );
  _raitem->addColumn(tr("Shipped"),     _qtyColumn,   Qt::AlignRight  );
  _raitem->addColumn(tr("Price"),       _priceColumn, Qt::AlignRight  );
  _raitem->addColumn(tr("Extended"),    _moneyColumn, Qt::AlignRight  );
  _raitem->addColumn(tr("Credited"),    _moneyColumn, Qt::AlignRight  );
  _raitem->addColumn(tr("Orig. Order"), _itemColumn,  Qt::AlignLeft   );
  _raitem->addColumn(tr("New Order"),   _itemColumn,  Qt::AlignLeft   );
  _raitem->addColumn(tr("Sched. Date"), _dateColumn,  Qt::AlignLeft   );

  _authorizeLine->hide();
  _clearAuthorization->hide();
  _authorizeAll->hide();

  if (! _metrics->boolean("CCAccept") || !_privileges->check("ProcessCreditCards"))
  {
    _refund->hide();
    _CCCVVLit->hide();
    _CCCVV->hide();
  }
  
  if (! _metrics->boolean("CCAccept"))
    _creditBy->removeItem(3);
  
  _printRA->setChecked(_metrics->boolean("DefaultPrintRAOnSave"));

  _receiveAll->setEnabled(_privileges->check("EnterReceipts"));
  _postReceipts->setEnabled(_privileges->check("EnterReceipts"));
}

returnAuthorization::~returnAuthorization()
{
    // no need to delete child widgets, Qt does it all for us
}

void returnAuthorization::languageChange()
{
    retranslateUi(this);
}

enum SetResponse returnAuthorization::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  QString metric;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      q.prepare("SELECT NEXTVAL('rahead_rahead_id_seq') AS rahead_id;");
      q.exec();
      if (q.first())
      {
        _raheadid = q.value("rahead_id").toInt();
        _comments->setId(_raheadid);
      }
      else if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      _cust->setFocus();      
      setNumber();
      _authDate->setDate(omfgThis->dbDate(), true);

      q.prepare("INSERT INTO rahead ("
                "    rahead_id, rahead_number, rahead_authdate"
                ") VALUES ("
                "    :rahead_id, COALESCE(:rahead_number,0), :rahead_authdate"
                ");");
      q.bindValue(":rahead_id", _raheadid);
      if (!_authNumber->text().isEmpty())
        q.bindValue(":rahead_number", _authNumber->text());
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      metric = _metrics->value("DefaultRaDisposition");
      if (metric == "C")
        _disposition->setCurrentItem(0);
      else if (metric == "R")
        _disposition->setCurrentItem(1);
      else if (metric == "P")
        _disposition->setCurrentItem(2);
      else if (metric == "V")
        _disposition->setCurrentItem(3);
      else if (metric == "M")
        _disposition->setCurrentItem(4);

      if (_metrics->value("DefaultRaTiming") == "R")
        _timing->setCurrentItem(1);

      metric = _metrics->value("DefaultRaCreditMethod");
      if (metric == "N")
        _creditBy->setCurrentItem(0);
      else if (metric == "M")
        _creditBy->setCurrentItem(1);
      else if (metric == "K")
        _creditBy->setCurrentItem(2);
      else if (metric == "C")
        _creditBy->setCurrentItem(3);

      connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustomerInfo()));
      connect(_cust, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _authNumber->setEnabled(FALSE);
      _save->setFocus();
      _cancel->setText("&Close");
      
      connect(_authNumber, SIGNAL(lostFocus()), this, SLOT(sCheckAuthorizationNumber()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _authNumber->setEnabled(FALSE);
      _authDate->setEnabled(FALSE);
      _expireDate->setEnabled(FALSE);
      _salesRep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);
      _taxauth->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);
      _disposition->setEnabled(FALSE);
      _timing->setEnabled(FALSE);
      _creditBy->setEnabled(FALSE);

      _origso->setEnabled(FALSE);
      _newso->setEnabled(FALSE);
      _incident->setEnabled(FALSE);
      _project->setEnabled(FALSE);

      _cust->setReadOnly(TRUE);
      _billToName->setEnabled(FALSE);
      _billToAddr->setEnabled(FALSE);

      _customerPO->setEnabled(FALSE);

      _miscCharge->setEnabled(FALSE);
      _miscChargeDescription->setEnabled(FALSE);
      _miscChargeAccount->setReadOnly(TRUE);
      _freight->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _copyToShipto->setEnabled(FALSE);
      _shipToNumber->setEnabled(FALSE);
      _shipToName->setEnabled(FALSE);
      _shipToAddr->setEnabled(FALSE);
      _currency->setEnabled(FALSE);
      _shipToList->hide();
      _save->hide();
      _new->hide();
      _delete->hide();
      _edit->hide();
      _action->hide();
      _authorizeLine->hide();
      _clearAuthorization->hide();
      _authorizeAll->hide();
      _enterReceipt->hide();
      _receiveAll->hide();
      _postReceipts->hide();

      _cancel->setText("&Close");
      _cancel->setFocus();
    }
  }

  param = pParams.value("incdt_id", &valid);
  if (valid)
    _incident->setId(param.toInt());

  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _raheadid = param.toInt();
    _comments->setId(_raheadid);
    populate();
  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());

  param = pParams.value("sohead_id", &valid);
  if (cNew == _mode && valid)
    _origso->setId(param.toInt());

  return NoError;
}

void returnAuthorization::setNumber()
{
  if ( (_metrics->value("RANumberGeneration") == "A") || 
       (_metrics->value("RANumberGeneration") == "O")   )
  {
    q.prepare("SELECT fetchRaNumber() AS ranumber;");
    q.exec();
    if (q.first())
    {
      _authNumber->setText(q.value("ranumber").toString());

      if (_metrics->value("RANumberGeneration") == "A")
        _authNumber->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _authNumber->setFocus();
}

bool returnAuthorization::sSave(bool partial)
{
  const char *dispositionTypes[] = { "C", "R", "P", "V", "M" };
  const char *creditMethods[] = { "N", "M", "K", "C" };

  if (_authNumber->text().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Save Return Authorization"),
      tr("You may not save this Return Authorization until you have entered a valid Authorization Number.") );
    _authNumber->setFocus();
    return false;
  }

  if ( ! _miscCharge->isZero() && (!_miscChargeAccount->isValid()) )
  {
    QMessageBox::warning( this, tr("No Misc. Charge Account Number"),
                         tr("<p>You may not enter a Misc. Charge without "
                            "indicating the G/L Sales Account number for the "
                            "charge. Please set the Misc. Charge amount to 0 "
                            "or select a Misc. Charge Sales Account." ) );
    _returnAuthInformation->setCurrentPage(1);
    _miscChargeAccount->setFocus();
    return false;
  }
  
  if (!partial && _raitem->topLevelItemCount() == 0)
  {
    QMessageBox::warning( this, tr("Create Line Items for this Order"),
                          tr("<p>You must create at least one Line Item for "
                          "this Return Authorization before you may save it."));
    _new->setFocus();
    return FALSE;
  }

  if (_total->localValue() < 0 )
  {
    QMessageBox::information(this, tr("Total Less than Zero"),
                             tr("<p>The Total must be a positive value.") );
    _cust->setFocus();
    return false;
  }

  // save address info in case someone wants to use 'em again later
  // but don't make any global changes to the data and ignore errors
  _billToAddr->save(AddressCluster::CHANGEONE);
  _shipToAddr->save(AddressCluster::CHANGEONE);

  q.prepare( "UPDATE rahead "
             "   SET rahead_cust_id=:rahead_cust_id,rahead_number=:rahead_number,"
             "       rahead_authdate=:rahead_authdate,rahead_expiredate=:rahead_expiredate,"
             "       rahead_salesrep_id=:rahead_salesrep_id, rahead_commission=:rahead_commission,"
             "       rahead_taxauth_id=:rahead_taxauth_id,rahead_rsncode_id=:rahead_rsncode_id,"
             "       rahead_disposition=:rahead_disposition,rahead_timing=:rahead_timing,"
             "       rahead_creditmethod=:rahead_creditmethod,rahead_orig_cohead_id=:rahead_orig_cohead_id,"
             "       rahead_new_cohead_id=:rahead_new_cohead_id, "
             "       rahead_incdt_id=:rahead_incdt_id,rahead_prj_id=:rahead_prj_id,"
             "       rahead_billtoname=:rahead_billtoname, rahead_billtoaddress1=:rahead_billtoaddress1,"
             "       rahead_billtoaddress2=:rahead_billtoaddress2, rahead_billtoaddress3=:rahead_billtoaddress3,"
             "       rahead_billtocity=:rahead_billtocity, rahead_billtostate=:rahead_billtostate,"
             "       rahead_billtozip=:rahead_billtozip,rahead_billtocountry=:rahead_billtocountry,"
             "       rahead_shipto_id=:rahead_shipto_id,"
             "       rahead_shipto_name=:rahead_shipto_name, rahead_shipto_address1=:rahead_shipto_address1,"
             "       rahead_shipto_address2=:rahead_shipto_address2, rahead_shipto_address3=:rahead_shipto_address3,"
             "       rahead_shipto_city=:rahead_shipto_city, rahead_shipto_state=:rahead_shipto_state,"
             "       rahead_shipto_zipcode=:rahead_shipto_zipcode,rahead_shipto_country=:rahead_shipto_country,"
             "       rahead_custponumber=:rahead_custponumber,rahead_notes=:rahead_notes, "
             "       rahead_misc_accnt_id=:rahead_misc_accnt_id,rahead_misc=:rahead_misc, "
             "       rahead_misc_descrip=:rahead_misc_descrip,"
             "       rahead_curr_id=:rahead_curr_id, "
             "       rahead_freight=:rahead_freight,"
             "       rahead_printed=:rahead_printed "
             " WHERE(rahead_id=:rahead_id);" );

  q.bindValue(":rahead_id", _raheadid);
  q.bindValue(":rahead_number", _authNumber->text().toInt());
  q.bindValue(":rahead_authdate", _authDate->date());
  q.bindValue(":rahead_expiredate", _expireDate->date());
  if (_salesRep->isValid() && _cust->isValid())
    q.bindValue(":rahead_salesrep_id", _salesRep->id());
  q.bindValue(":rahead_commission", (_commission->toDouble() / 100));
  if (_taxauth->isValid())
    q.bindValue(":rahead_taxauth_id", _taxauth->id());
  if (_rsnCode->id() > 0)
    q.bindValue(":rahead_rsncode_id", _rsnCode->id());
  q.bindValue(":rahead_disposition", QString(dispositionTypes[_disposition->currentItem()]));
  if (_timing->currentItem() == 0)
    q.bindValue(":rahead_timing", "I");
  else
    q.bindValue(":rahead_timing", "R");
  q.bindValue(":rahead_creditmethod", QString(creditMethods[_creditBy->currentItem()]));
  if (_origso->isValid())
    q.bindValue(":rahead_orig_cohead_id", _origso->id());
  if (_newso->isValid())
    q.bindValue(":rahead_new_cohead_id", _newso->id());
  if (_incident->isValid())
    q.bindValue(":rahead_incdt_id", _incident->id());
  if (_project->isValid())
    q.bindValue(":rahead_prj_id", _project->id());
  if (_cust->isValid())
    q.bindValue(":rahead_cust_id", _cust->id());
  q.bindValue(":rahead_billtoname", _billToName->text().stripWhiteSpace());
  q.bindValue(":rahead_billtoaddress1", _billToAddr->line1());
  q.bindValue(":rahead_billtoaddress2", _billToAddr->line2());
  q.bindValue(":rahead_billtoaddress3", _billToAddr->line3());
  q.bindValue(":rahead_billtocity",     _billToAddr->city());
  q.bindValue(":rahead_billtostate",    _billToAddr->state());
  q.bindValue(":rahead_billtozip",      _billToAddr->postalCode());
  q.bindValue(":rahead_billtocountry",  _billToAddr->country());
  if (_shiptoid > 0)
    q.bindValue(":rahead_shipto_id",    _shiptoid);
  q.bindValue(":rahead_shipto_name", _shipToName->text().stripWhiteSpace());
  q.bindValue(":rahead_shipto_address1", _shipToAddr->line1());
  q.bindValue(":rahead_shipto_address2", _shipToAddr->line2());
  q.bindValue(":rahead_shipto_address3", _shipToAddr->line3());
  q.bindValue(":rahead_shipto_city",     _shipToAddr->city());
  q.bindValue(":rahead_shipto_state",    _shipToAddr->state());
  q.bindValue(":rahead_shipto_zipcode",  _shipToAddr->postalCode());
  q.bindValue(":rahead_shipto_country",  _shipToAddr->country());
  q.bindValue(":rahead_custponumber", _customerPO->text().stripWhiteSpace());
  q.bindValue(":rahead_notes", _notes->text());
  q.bindValue(":rahead_misc", _miscCharge->localValue());
  if (_miscChargeAccount->id() != -1)
    q.bindValue(":rahead_misc_accnt_id", _miscChargeAccount->id());
  q.bindValue(":rahead_misc_descrip", _miscChargeDescription->text());
  q.bindValue(":rahead_curr_id", _currency->id());
  q.bindValue(":rahead_freight", _freight->localValue());

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  omfgThis->sReturnAuthorizationsUpdated();
  omfgThis->sProjectsUpdated(_project->id());

  _comments->setId(_raheadid);
  
  connect(_authNumber, SIGNAL(lostFocus()), this, SLOT(sCheckAuthorizationNumber()));
  
  return true;
}

void returnAuthorization::sPostReceipts()
{
  for (int i = 0; i < _raitem->topLevelItemCount(); i++)
  {
    if (_raitem->topLevelItem(i)->text(TO_RECEIVE_COL).toFloat() > 0)
    {
      enterPoReceipt::post("RA", _raheadid);
      sFillList();
      _mode = cEdit;
      _cancel->setText("&Close");
      break;
    }
  }
}

void returnAuthorization::sSaveClick()
{
  if (sSave(false))
  {
    if (_printRA->isChecked())
    {
      ParameterList params;
      params.append("rahead_id", _raheadid);

      printRaForm newdlgS(this, "", true);
      newdlgS.set(params);
      newdlgS.exec();

/*   TO DO: This isn't going to work right now because the EDI profile is specific to S/O.
     We really need to rearchitect the EDI profiles for customers to one to many
     With profile types for Invoice, S/O, R/A etc...
     
      if (_custEmail && _metrics->boolean("EnableBatchManager"))
      {
        deliverReturnAuthorization newdlgD(this, "", true);
        newdlgD.set(params);
        newdlgD.exec();
      }
  */
    }
    _raheadid=-1;
    close();
  }
}

void returnAuthorization::sShipToList()
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

void returnAuthorization::sOrigSoChanged()
{
  if (_origso->isValid())
  {
    _authorizeLine->show();
    _clearAuthorization->show();
    _authorizeAll->show();
  }
  else
  {
    _authorizeLine->hide();
    _clearAuthorization->hide();
    _authorizeAll->hide();
  }
  if (_origso->isValid())
  {
    q.prepare("SELECT rahead_number FROM rahead,raitem "
              " WHERE((rahead_orig_cohead_id=:cohead_id) "
              "   AND (rahead_id != :rahead_id) "
              "   AND (raitem_rahead_id=rahead_id) "
              "   AND (raitem_status = 'O')); ");
    q.bindValue(":cohead_id",_origso->id());
    q.bindValue(":rahead_id",_raheadid);
    q.exec();
    if (q.first())
    {
      QMessageBox::critical(this, tr("Invalid Sales Order"),
          tr("This sales order is already linked to open return authorization %1.").arg(q.value("rahead_number").toString())  );
      _origso->setId(-1);
      if (_cust->isValid())
      {
        _origso->setCustId(_cust->id());
        _origso->setType((cSoReleased | cSoCustomer));
      }
      else
        _origso->setType(cSoReleased);
      return;
    }
  }
  if (!_ignoreSoSignals) 
  {
    sSave(true);
    sFillList();

    if (_origso->isValid())
    {
      XSqlQuery sohead;
      sohead.prepare( "SELECT cohead.*,custinfo.*, custtype_code, "
                      "       cohead_commission, "
                      "       shipto_num "
                      "FROM cohead "
                      "  LEFT OUTER JOIN shiptoinfo ON (cohead_shipto_id=shipto_id), "
                      "  custinfo, custtype "
                      "WHERE ((cohead_id=:cohead_id) "
                      "AND (cohead_cust_id=cust_id) "
                      "AND (cust_custtype_id=custtype_id) ) "
                      "LIMIT 1;" );
      sohead.bindValue(":cohead_id", _origso->id());
      sohead.exec();
      if (sohead.first())
      {
        _salesRep->setId(sohead.value("cohead_salesrep_id").toInt());
        _commission->setDouble(sohead.value("cohead_commission").toDouble() * 100);

        _taxauth->setId(sohead.value("cohead_taxauth_id").toInt());
        _customerPO->setText(sohead.value("cohead_custponumber"));

        _cust->setEnabled(FALSE);

        _cust->setId(sohead.value("cohead_cust_id").toInt());
        _custType->setText(sohead.value("custtype_code").toString());
        _billToName->setText(sohead.value("cohead_billtoname"));
        _billToAddr->setLine1(sohead.value("cohead_billtoaddress1").toString());
        _billToAddr->setLine2(sohead.value("cohead_billtoaddress2").toString());
        _billToAddr->setLine3(sohead.value("cohead_billtoaddress3").toString());
        _billToAddr->setCity(sohead.value("cohead_billtocity").toString());
        _billToAddr->setState(sohead.value("cohead_billtostate").toString());
        _billToAddr->setPostalCode(sohead.value("cohead_billtozipcode").toString());
        _billToAddr->setCountry(sohead.value("cohead_billtocountry").toString());
        if ( (_mode == cNew) || (_mode == cEdit) )
          _ffBillto = sohead.value("cust_ffbillto").toBool();
        else
          _ffBillto = FALSE;
        _billToName->setEnabled(_ffBillto);
        _billToAddr->setEnabled(_ffBillto);

        _ignoreShiptoSignals = TRUE;
        _shiptoid = sohead.value("cohead_shipto_id").toInt();
        _shipToNumber->setText(sohead.value("shipto_num").toString());
        _shipToName->setText(sohead.value("cohead_shiptoname"));
        _shipToAddr->setLine1(sohead.value("cohead_shiptoaddress1").toString());
        _shipToAddr->setLine2(sohead.value("cohead_shiptoaddress2").toString());
        _shipToAddr->setLine3(sohead.value("cohead_shiptoaddress3").toString());
        _shipToAddr->setCity(sohead.value("cohead_shiptocity").toString());
        _shipToAddr->setState(sohead.value("cohead_shiptostate").toString());
        _shipToAddr->setPostalCode(sohead.value("cohead_shiptozipcode").toString());
        _shipToAddr->setCountry(sohead.value("cohead_shiptocountry").toString());
        if ( (_mode == cNew) || (_mode == cEdit) )
          _ffShipto = sohead.value("cust_ffshipto").toBool();
        else
          _ffShipto = FALSE;
        _copyToShipto->setEnabled(_ffShipto);
        _shipToName->setEnabled(_ffShipto);
        _shipToNumber->setEnabled(_ffShipto);
        _shipToAddr->setEnabled(_ffShipto);
        _ignoreShiptoSignals = FALSE;
        sSave(true);
      }
      else if (sohead.lastError().type() != QSqlError::None)
      {
        systemError(this, sohead.lastError().databaseText(), __FILE__, __LINE__);
        _origso->setId(-1);
        return;
      }
    }
  }
}

void returnAuthorization::sParseShipToNumber()
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

void returnAuthorization::populateShipto(int pShiptoid)
{
  if (pShiptoid != -1)
  {
    XSqlQuery query;
    query.prepare( "SELECT shipto_num, shipto_name,"
                   " shipto_addr_id, shipto_taxauth_id, "
                   " shipto_salesrep_id, shipto_commission "
                   "FROM shiptoinfo "
                   "WHERE (shipto_id=:shipto_id);" );
    query.bindValue(":shipto_id", pShiptoid);
    query.exec();
    if (query.first())
    {
      _shiptoid = pShiptoid;
      _ignoreShiptoSignals = TRUE;
      _shipToNumber->setText(query.value("shipto_num"));
      _shipToName->setText(query.value("shipto_name"));
      _shipToAddr->setId(query.value("shipto_addr_id").toInt());
      _taxauth->setId(query.value("shipto_taxauth_id").toInt());
      _salesRep->setId(query.value("shipto_salesrep_id").toInt());
      _commission->setDouble(query.value("shipto_commission").toDouble() * 100);
      _ignoreShiptoSignals = FALSE;
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

  _shiptoid = pShiptoid;
}

void returnAuthorization::sPopulateCustomerInfo()
{
    if (_cust->isValid())
    {
      _origso->setCustId(_cust->id());
      _origso->setType((cSoReleased | cSoCustomer));
      
      XSqlQuery query;
      query.prepare( "SELECT custtype_code, cust_salesrep_id,"
                     "       cust_commprcnt,"
                     "       cust_taxauth_id, cust_curr_id, "
                     "       cust_name, cntct_addr_id, "
                     "       cust_ffshipto, cust_ffbillto, "
                     "       COALESCE(shipto_id, -1) AS shiptoid, "
                     "       cust_soemaildelivery "
                     "FROM custinfo "
                     "  LEFT OUTER JOIN shiptoinfo ON ((cust_id=shipto_cust_id) "
                     "                             AND (shipto_default)) "
                     "  LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id), "
                     "  custtype "
                     "WHERE ( (cust_id=:cust_id) "
                     "AND (custtype_id=cust_custtype_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
      {
        _custType->setText(query.value("custtype_code").toString());
        _salesRep->setId(query.value("cust_salesrep_id").toInt());
        _commission->setDouble(query.value("cust_commprcnt").toDouble() * 100);

        _custtaxauthid = query.value("cust_taxauth_id").toInt();
        _taxauth->setId(query.value("cust_taxauth_id").toInt());
        _currency->setId(query.value("cust_curr_id").toInt());

        _billToName->setText(query.value("cust_name"));
        _billToAddr->setId(query.value("cntct_addr_id").toInt());

        if ( (_mode == cNew) || (_mode == cEdit) )
          _ffBillto = query.value("cust_ffbillto").toBool();
        else
          _ffBillto = FALSE;
        _billToName->setEnabled(_ffBillto);
        _billToAddr->setEnabled(_ffBillto);

        if ( (_mode == cNew) || (_mode == cEdit) )
          _ffShipto = query.value("cust_ffshipto").toBool();
        else
          _ffShipto = FALSE;
        _copyToShipto->setEnabled(_ffShipto);
        _shipToName->setEnabled(_ffShipto);
        _shipToNumber->setEnabled(_ffShipto);
        _shipToAddr->setEnabled(_ffShipto);
        _custEmail = query.value("cust_soemaildelivery").toBool();
        populateShipto(query.value("shiptoid").toInt());
      }
      else if (query.lastError().type() != QSqlError::None)
      {
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    else
    {
      _origso->setCustId(-1);
      _origso->setType((cSoReleased));
      _salesRep->setCurrentItem(-1);
      _taxauth->setId(-1);
      _custtaxauthid = -1;
      _billToName->setEnabled(TRUE);
      _billToAddr->setEnabled(TRUE);
      _billToName->clear();
      _billToAddr->clear();
      _shipToNumber->setEnabled(TRUE);
      _shipToName->setEnabled(TRUE);
      _shipToAddr->setEnabled(TRUE);
      _shipToNumber->clear();
      _shipToName->clear();
      _shipToAddr->clear();
    }
}

void returnAuthorization::sCheckAuthorizationNumber()
{
  if ( (_authNumber->text().length()) &&
       ( (_metrics->value("RANumberGeneration") == "O") || 
         (_metrics->value("RANumberGeneration") == "M")   ) )
  {
    _authNumber->setEnabled(FALSE);

    XSqlQuery query;
    query.prepare( "SELECT rahead_id "
                   "FROM rahead "
                   "WHERE (rahead_number=:rahead_number)" );
    query.bindValue(":rahead_number", _authNumber->text().toInt());
    query.exec();
    if (query.first())
    {
      _raheadid = query.value("rahead_id").toInt();

      if (_cust->id() > 0)
        _cust->setReadOnly(TRUE);

      populate();

      _mode = cEdit;
    }
    else if (query.lastError().type() != QSqlError::None)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void returnAuthorization::sClearShiptoNumber()
{
  if (!_ignoreShiptoSignals)
  {
//  Convert the captive shipto to a free-form shipto
    _shipToNumber->clear();

    _shiptoid = -1;
  }
}

void returnAuthorization::sCopyToShipto()
{
  _shiptoid = -1;
  _shipToNumber->clear();
  _shipToName->setText(_billToName->text());
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

void returnAuthorization::sNew()
{
  if (sSave(true))
  {
    ParameterList params;
    params.append("mode", "new");
    params.append("rahead_id", _raheadid);

    returnAuthorizationItem newdlg(this, "", TRUE);
    newdlg.set(params);
  
    if (newdlg.exec() != XDialog::Rejected)
      populate();
  }
}

void returnAuthorization::sEdit()
{
  if (sSave(true))
  {
    bool fill;
    fill = FALSE;
    QList<QTreeWidgetItem*> selected = _raitem->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      ParameterList params;
      params.append("raitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
      params.append("rahead_id", _raheadid);

      if (_mode==cView || ((XTreeWidgetItem*)(selected[i]))->altId() == -1)
        params.append("mode", "view");
      else
        params.append("mode", "edit");

      returnAuthorizationItem newdlg(this, "", TRUE);
      newdlg.set(params);
  
      if (newdlg.exec() != XDialog::Rejected)
        fill = TRUE;
    }
    if (fill)
      populate();
  }
}

void returnAuthorization::sView()
{
  QList<QTreeWidgetItem*> selected = _raitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("raitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
    params.append("rahead_id", _raheadid);
    params.append("mode", "view");

    returnAuthorizationItem newdlg(this, "", TRUE);
    newdlg.set(params);
  
    newdlg.exec();
  }
}

void returnAuthorization::sDelete()
{
  if (QMessageBox::question(this, "Delete current Line Item?",
                            tr("<p>Are you sure that you want to delete the "
                               "the selected Line Item(s)?"),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
  {
    QList<QTreeWidgetItem*> selected = _raitem->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      q.prepare( "DELETE FROM raitem "
                 "WHERE (raitem_id=:raitem_id);" );
      q.bindValue(":raitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    populate();
  }
}

void returnAuthorization::sFillList()
{
  q.prepare( "SELECT raitem_id, "
             "       CASE WHEN (raitem_status='C')     THEN -1"
             "            WHEN (raitem_disposition='C') THEN 0"
             "            WHEN (raitem_disposition='R') THEN 1"
             "            WHEN (raitem_disposition='P') THEN 2"
             "            WHEN (raitem_disposition='V') THEN 3"
             "            WHEN (raitem_disposition='S') THEN 4"
             "            ELSE -1 END AS disposition_code,"
             "       raitem_linenumber, item_number,uom_name,"
             "       (item_descrip1 || ' ' || item_descrip2), warehous_code,"
             "       raitem_status, "
             "       CASE WHEN (raitem_disposition='C') THEN :credit "
             "            WHEN (raitem_disposition='R') THEN :return "
             "            WHEN (raitem_disposition='P') THEN :replace "
             "            WHEN (raitem_disposition='V') THEN :service "
             "            WHEN (raitem_disposition='S') THEN :ship "
             "            ELSE 'Error' END AS disposition, "
             "       formatboolyn(raitem_warranty),"
             "       formatQty(COALESCE(oc.coitem_qtyshipped,0)),"
             "       formatQty(raitem_qtyauthorized),"
             "       formatQty(raitem_qtyreceived),"
             "       formatQty(qtyToReceive('RA', raitem_id)),"
             "       formatQty(COALESCE(nc.coitem_qtyshipped,0)),"
             "       formatSalesPrice(raitem_unitprice),"
             "       formatMoney(round((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio),2)),"
             "       formatMoney(raitem_amtcredited),"
             "       och.cohead_number::text || '-' || oc.coitem_linenumber::text, "
             "       nch.cohead_number::text || '-' || nc.coitem_linenumber::text, "
             "       CASE WHEN raitem_disposition IN ('P','V','S') THEN "
             "         formatdate(raitem_scheddate) "
             "       ELSE "
             "         'N/A' "
             "       END "
             "FROM raitem "
             " LEFT OUTER JOIN coitem oc ON (raitem_orig_coitem_id=oc.coitem_id) "
             " LEFT OUTER JOIN cohead och ON (och.cohead_id=oc.coitem_cohead_id) "
             " LEFT OUTER JOIN coitem nc ON (raitem_new_coitem_id=nc.coitem_id) "
             " LEFT OUTER JOIN cohead nch ON (nch.cohead_id=nc.coitem_cohead_id),"
             " itemsite, item, whsinfo, uom "
             "WHERE ( (raitem_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (itemsite_warehous_id=warehous_id)"
             " AND (raitem_rahead_id=:rahead_id) "
             " AND (raitem_qty_uom_id=uom_id) ) "
             "ORDER BY raitem_linenumber;" );
  q.bindValue(":rahead_id", _raheadid);
  q.bindValue(":credit", tr("Credit"));
  q.bindValue(":return", tr("Return"));
  q.bindValue(":replace", tr("Replace"));
  q.bindValue(":service", tr("Service"));
  q.bindValue(":ship", tr("Ship"));
  q.exec();
  if ((q.first()) && (_mode == cEdit))
  {
    _cust->setDisabled(_cust->isValid());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  _raitem->populate(q, true);

  sCalculateSubtotal();
  recalculateTax();

  _currency->setEnabled((_raitem->topLevelItemCount() == 0) && (_mode == cEdit));

  //Disable order changes if any order qty authorized
  q.prepare("SELECT raitem_id "
            "FROM raitem "
            "WHERE ( (raitem_rahead_id=:rahead_id) "
            "AND (raitem_orig_coitem_id IS NOT NULL) "
            "AND (raitem_qtyauthorized > 0) );");
  q.bindValue(":rahead_id", _raheadid);
  q.exec();
  _origso->setEnabled((!q.first()) && (_mode == cEdit || _mode == cNew));

  //Disable changes if any transactions
  q.prepare("SELECT raitem_id "
            "FROM raitem "
            "  LEFT OUTER JOIN coitem ON (raitem_new_coitem_id=coitem_id) "
            "WHERE ( (raitem_rahead_id=:rahead_id) "
            "AND (((raitem_qtyreceived + COALESCE(coitem_qtyshipped,0) + "
            " raitem_amtcredited) > 0 OR raitem_status = 'C')) );");
  q.bindValue(":rahead_id", _raheadid);
  q.exec();
  if (q.first())
  {
    _salesRep->setEnabled(false);
    _commission->setEnabled(false);
    _taxauth->setEnabled(false);
    _disposition->setEnabled(false);
    _timing->setEnabled(false);
    _cust->setEnabled(false);
    _billToName->setEnabled(false);
    _billToAddr->setEnabled(false);
  }
  else
  {
    _salesRep->setEnabled(true);
    _commission->setEnabled(true);
    _taxauth->setEnabled(true);
    _disposition->setEnabled(true);
    _timing->setEnabled(true);
    _cust->setEnabled(!_origso->isValid());
    _billToName->setEnabled(true);
    _billToAddr->setEnabled(true);
  }
  _comments->refresh();
  sCreditByChanged();
}

void returnAuthorization::sCalculateSubtotal()
{
//  Determine the subtotal and line item tax
  XSqlQuery query;
  query.prepare( "SELECT SUM(round((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio),2)) AS subtotal,"
               "       SUM(round((raitem_qtyauthorized * raitem_qty_invuomratio) * stdCost(item_id),2)) AS totalcost "
               "FROM raitem, itemsite, item "
               "WHERE ( (raitem_rahead_id=:rahead_id)"
               " AND (raitem_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id) );" );
  query.bindValue(":rahead_id", _raheadid);
  query.exec();
  if (query.first())
    _subtotal->setLocalValue(query.value("subtotal").toDouble());
}

void returnAuthorization::sCalculateTotal()
{
  _total->setLocalValue(_subtotal->localValue() +
                        _tax->localValue() +
                        _freight->localValue() +
                        _miscCharge->localValue() );
}

void returnAuthorization::populate()
{
  XSqlQuery rahead;
  rahead.prepare( "SELECT rahead.*, custtype_code, "
                  "       cust_name, cust_ffbillto, cust_ffshipto, shipto_num, "
                  "       rahead_commission "
                  "FROM rahead "
                  "  LEFT OUTER JOIN shiptoinfo ON (rahead_shipto_id=shipto_id) "
                  "  LEFT OUTER JOIN custinfo ON (rahead_cust_id=cust_id) "
                  "  LEFT OUTER JOIN custtype ON (cust_custtype_id=custtype_id) "
                  "WHERE (rahead_id=:rahead_id);" );
  rahead.bindValue(":rahead_id", _raheadid);
  rahead.exec();
  if (rahead.first())
  {
    _authNumber->setText(rahead.value("rahead_number"));
    _authDate->setDate(rahead.value("rahead_authdate").toDate(), true);
    _expireDate->setDate(rahead.value("rahead_expiredate").toDate());

    _salesRep->setId(rahead.value("rahead_salesrep_id").toInt());
    _commission->setDouble(rahead.value("rahead_commission").toDouble() * 100);
    // do taxauth first so we can overwrite the result of the signal cascade
    _taxauth->setId(rahead.value("rahead_taxauth_id").toInt());
    _taxcurrid = rahead.value("rahead_curr_id").toInt();
    if (!rahead.value("rahead_rsncode_id").isNull() && rahead.value("rahead_rsncode_id").toInt() != -1)
      _rsnCode->setId(rahead.value("rahead_rsncode_id").toInt());

    if (rahead.value("rahead_timing").toString() == "I")
      _timing->setCurrentItem(0);
    else
      _timing->setCurrentItem(1);

    if (rahead.value("rahead_creditmethod").toString() == "N")
      _creditBy->setCurrentItem(0);
    else if (rahead.value("rahead_creditmethod").toString() == "M")
      _creditBy->setCurrentItem(1);
    else if (rahead.value("rahead_creditmethod").toString() == "K")
      _creditBy->setCurrentItem(2);
    else if (rahead.value("rahead_creditmethod").toString() == "C")
      _creditBy->setCurrentItem(3);

    _cust->setId(rahead.value("rahead_cust_id").toInt());
    _custType->setText(rahead.value("custtype_code").toString());
    _ignoreSoSignals = TRUE;
    _origso->setId(rahead.value("rahead_orig_cohead_id").toInt());
    _newso->setId(rahead.value("rahead_new_cohead_id").toInt());
    _ignoreSoSignals = FALSE;
    _incident->setId(rahead.value("rahead_incdt_id").toInt());
    _project->setId(rahead.value("rahead_prj_id").toInt());

    _ffBillto = rahead.value("cust_ffbillto").toBool();
    _billToName->setEnabled(_ffBillto);
    _billToAddr->setEnabled(_ffBillto);

    _billToName->setText(rahead.value("rahead_billtoname"));
    _billToAddr->setLine1(rahead.value("rahead_billtoaddress1").toString());
    _billToAddr->setLine2(rahead.value("rahead_billtoaddress2").toString());
    _billToAddr->setLine3(rahead.value("rahead_billtoaddress3").toString());
    _billToAddr->setCity(rahead.value("rahead_billtocity").toString());
    _billToAddr->setState(rahead.value("rahead_billtostate").toString());
    _billToAddr->setPostalCode(rahead.value("rahead_billtozip").toString());
    _billToAddr->setCountry(rahead.value("rahead_billtocountry").toString());

    _ffShipto = rahead.value("cust_ffshipto").toBool();
    _shipToName->setEnabled(_ffShipto);
    _shipToAddr->setEnabled(_ffShipto);
    _copyToShipto->setEnabled(_ffShipto);

    _ignoreShiptoSignals = TRUE;
    _shiptoid = rahead.value("rahead_shipto_id").toInt();
    _shipToNumber->setText(rahead.value("shipto_num").toString());
    _shipToName->setText(rahead.value("rahead_shipto_name"));
    _shipToAddr->setLine1(rahead.value("rahead_shipto_address1").toString());
    _shipToAddr->setLine2(rahead.value("rahead_shipto_address2").toString());
    _shipToAddr->setLine3(rahead.value("rahead_shipto_address3").toString());
    _shipToAddr->setCity(rahead.value("rahead_shipto_city").toString());
    _shipToAddr->setState(rahead.value("rahead_shipto_state").toString());
    _shipToAddr->setPostalCode(rahead.value("rahead_shipto_zipcode").toString());
    _shipToAddr->setCountry(rahead.value("rahead_shipto_country").toString());
    _ignoreShiptoSignals = FALSE;
    _customerPO->setText(rahead.value("rahead_custponumber"));

    _currency->setId(rahead.value("rahead_curr_id").toInt());
    _freight->setLocalValue(rahead.value("rahead_freight").toDouble());

    _miscCharge->setLocalValue(rahead.value("rahead_misc").toDouble());
    _miscChargeDescription->setText(rahead.value("rahead_misc_descrip"));
    _miscChargeAccount->setId(rahead.value("rahead_misc_accnt_id").toInt());

    _notes->setText(rahead.value("rahead_notes").toString());

    if (rahead.value("rahead_headcredited").toBool())
    {
      _freight->setEnabled(FALSE);
      _miscCharge->setEnabled(FALSE);
      _miscChargeDescription->setEnabled(FALSE);
      _miscChargeAccount->setEnabled(FALSE);
    }

    if (rahead.value("rahead_disposition").toString() == "C")
      sDispositionChanged();
    else if (rahead.value("rahead_disposition").toString() == "R")
      _disposition->setCurrentItem(1);
    else if (rahead.value("rahead_disposition").toString() == "P")
      _disposition->setCurrentItem(2);
    else if (rahead.value("rahead_disposition").toString() == "V")
      _disposition->setCurrentItem(3);
    else if (rahead.value("rahead_disposition").toString() == "M")
      _disposition->setCurrentItem(4);

    recalculateTax();

    sFillList();
  }
  else if (rahead.lastError().type() != QSqlError::None)
  {
    systemError(this, rahead.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void returnAuthorization::closeEvent(QCloseEvent *pEvent)
{
  if ( (_mode == cNew) && (_raheadid != -1) )
  {
    q.prepare( "UPDATE raitem SET raitem_qtyauthorized=0 "
               "WHERE (raitem_rahead_id=:rahead_id); "
               "SELECT importcoitemstora(:rahead_id,NULL);"
               "DELETE FROM rahead "
               "WHERE (rahead_id=:rahead_id);" );
    q.bindValue(":rahead_id", _raheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      omfgThis->sReturnAuthorizationsUpdated();
      omfgThis->sProjectsUpdated(_project->id());
    }

    if ( (_metrics->value("RANumberGeneration") == "A") || 
         (_metrics->value("RANumberGeneration") == "O")   )
    {
      q.prepare("SELECT releaseRaNumber(:number) AS result;");
      q.bindValue(":number", _authNumber->text());
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }

  XMainWindow::closeEvent(pEvent);
}

void returnAuthorization::sTaxDetail()
{
  XSqlQuery taxq;
  taxq.prepare("UPDATE rahead SET rahead_taxauth_id=:taxauth,"
               "  rahead_freight=:freight,"
//               "  rahead_freighttax_ratea=:freighta,"
//               "  rahead_freighttax_rateb=:freightb,"
//               "  rahead_freighttax_ratec=:freightc,"
               "  rahead_authdate=:docdate "
               "WHERE (rahead_id=:rahead_id);");
  if (_taxauth->isValid())
    taxq.bindValue(":taxauth", _taxauth->id());
  taxq.bindValue(":freight",   _freight->localValue());
  taxq.bindValue(":freighta",  _taxCache.freight(0));
  taxq.bindValue(":freightb",  _taxCache.freight(1));
  taxq.bindValue(":freightc",  _taxCache.freight(2));
  taxq.bindValue(":docdate",   _authDate->date());
  taxq.bindValue(":rahead_id", _raheadid);
  taxq.exec();
  if (taxq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("order_id",   _raheadid);
  params.append("order_type", "RA");

  if(cView == _mode)
    params.append("mode", "view");
  else
    params.append("mode", "edit");

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    populate();
}

void returnAuthorization::sFreightChanged()
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

void returnAuthorization::recalculateTax()
{
  XSqlQuery itemq;

  //  Determine the line item tax
  itemq.prepare( "SELECT SUM(ROUND(calculateTax(raitem_tax_id, ROUND((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio), 2), 0, 'A'), 2)) AS itemtaxa,"
                 "       SUM(ROUND(calculateTax(raitem_tax_id, ROUND((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio), 2), 0, 'B'), 2)) AS itemtaxb,"
                 "       SUM(ROUND(calculateTax(raitem_tax_id, ROUND((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio), 2), 0, 'C'), 2)) AS itemtaxc "
                 "FROM raitem, itemsite, item "
                 "WHERE ((raitem_rahead_id=:rahead_id)"
                 "  AND  (raitem_itemsite_id=itemsite_id)"
                 "  AND  (itemsite_item_id=item_id));" );

  itemq.bindValue(":rahead_id", _raheadid);
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
}

void returnAuthorization::sTaxAuthChanged()
{
  if (_raheadid == -1 || _taxauthidCache == _taxauth->id())
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

  taxauthq.prepare("SELECT changeraTaxAuth(:rahead_id, :taxauth_id) AS result;");
  taxauthq.bindValue(":rahead_id", _raheadid);
  taxauthq.bindValue(":taxauth_id", _taxauth->id());
  taxauthq.exec();
  if (taxauthq.first())
  {
    int result = taxauthq.value("result").toInt();
    if (result < 0)
    {
      _taxauth->setId(_taxauthidCache);
      systemError(this, storedProcErrorLookup("changeraTaxAuth", result),
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

  taxauthq.prepare("SELECT COALESCE(getFreightTaxSelection(:taxauth), -1) AS result;");
  taxauthq.bindValue(":taxauth", _taxauth->id());
  taxauthq.exec();
  if (taxauthq.first())
  {
    _taxCache.setFreightId(taxauthq.value("result").toInt());
  }
  else if (taxauthq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFreightChanged();
}

void returnAuthorization::sDispositionChanged()
{
  _new->setEnabled(_cust->isValid() || 
              (_disposition->currentIndex() == 1 && _creditBy->currentIndex() == 0));
  
  bool enableReceipt = _privileges->check("EnterReceipts") &&
                      (_disposition->currentItem() != 0);

  _receiveAll->setEnabled(enableReceipt);
  _postReceipts->setEnabled(enableReceipt);

  if (_disposition->currentItem() == 0)
  {
    _timing->setCurrentItem(0);
    _timing->setEnabled(FALSE);
    if (_creditBy->currentItem() == 0)
      _creditBy->setCurrentItem(1);
  }
  else
    _timing->setEnabled(TRUE);

  _refund->setEnabled(_creditBy->currentItem() == 3);
}

void returnAuthorization::sCreditByChanged()
{
    if (_creditBy->currentItem() == 0 && _total->localValue() > 0)
    {
      QMessageBox::information(this, tr("Credit By 'None' not allowed"),
                            tr("This Return Authorization has authorized credit amounts. "
                "You may not set the Credit By to 'None' unless all credit amounts are zero."));
      _creditBy->setCurrentItem(1);
    }
    else if (_creditBy->currentItem() == 0  || _total->localValue() == 0)
    {
      _currency->setEnabled(false);
      _miscChargeDescription->setEnabled(false);
      _miscChargeAccount->setEnabled(false);
      _miscCharge->setEnabled(false);
      _freight->setEnabled(false);
    }
    else
    {
      _currency->setEnabled(true);
      _miscChargeDescription->setEnabled(true);
      _miscChargeAccount->setEnabled(true);
      _miscCharge->setEnabled(true);
      _freight->setEnabled(true);
    }
}

void returnAuthorization::sAuthorizeLine()
{
  QList<QTreeWidgetItem*> selected = _raitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    QString sql ( "UPDATE raitem SET raitem_qtyauthorized=coitem_qtyshipped "
                  "FROM coitem "
          "WHERE ((coitem_id=raitem_orig_coitem_id) "
          "AND (raitem_id=:raitem_id));" );
    q.prepare(sql);
    q.bindValue(":raitem_id",  ((XTreeWidgetItem*)(selected[i]))->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_newso->isValid())
    omfgThis->sSalesOrdersUpdated(_newso->id());
    
  populate();
}
void returnAuthorization::sClearAuthorization()
{
  QList<QTreeWidgetItem*> selected = _raitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
        QString sql ( "UPDATE raitem SET raitem_qtyauthorized=0 "
                      "FROM coitem "
                      "WHERE ((coitem_id=raitem_orig_coitem_id) "
                      "AND (raitem_id=:raitem_id));" );
        q.prepare(sql);
        q.bindValue(":raitem_id",  ((XTreeWidgetItem*)(selected[i]))->id());
        q.exec();
        if (q.lastError().type() != QSqlError::NoError)
        {
           systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
           return;
        }
  }

  if (_newso->isValid())
    omfgThis->sSalesOrdersUpdated(_newso->id());

  populate();
}

void returnAuthorization::sAuthorizeAll()
{
  QString sql ( "UPDATE raitem SET raitem_qtyauthorized=coitem_qtyshipped "
                "FROM coitem "
                "WHERE ((coitem_id=raitem_orig_coitem_id) "
                "AND (raitem_status = 'O') "
                "AND (raitem_qtyauthorized < coitem_qtyshipped) "
                "AND (coitem_qtyshipped >= raitem_qtyreceived) "
                "AND (raitem_rahead_id=:raitem_rahead_id));" );
  q.prepare(sql);
  q.bindValue(":raitem_rahead_id",  _raheadid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_newso->isValid())
    omfgThis->sSalesOrdersUpdated(_newso->id());

  populate();
}

void returnAuthorization::sEnterReceipt()
{
  ParameterList params;
  q.prepare("SELECT * "
            "FROM recv "
            "WHERE ((recv_orderitem_id=:id)"
            "  AND  (recv_order_type = 'RA')"
            "  AND  (NOT recv_posted));");
  q.bindValue(":id", _raitem->id());
  q.exec();
  if (q.first())
  {
    params.append("recv_id", q.value("recv_id"));
    params.append("mode", "edit");
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    params.append("lineitem_id", _raitem->id());
    params.append("order_type",  "RA");
    params.append("mode", "new");
  }

  enterPoitemReceipt newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void returnAuthorization::sReceiveAll()
{
  ParameterList params;
  params.append("orderid",   _raheadid);
  params.append("ordertype", "RA");
  params.append("nonInventory", tr("Non-Inventory"));
  params.append("na",           tr("N/A"));
  params.append("EnableReturnAuth", TRUE);
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  MetaSQLQuery recvm = mqlLoad(":/sr/enterReceipt/ReceiveAll.mql");
  q = recvm.toQuery(params);

  while (q.next())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("enterReceipt", result),
                  __FILE__, __LINE__);
      return;
    }
    omfgThis->sPurchaseOrderReceiptsUpdated();
  }
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void returnAuthorization::sHandleEnterReceipt(bool p)
{
  if (p)
    _enterReceipt->setEnabled(_raitem->altId() == 1 ||
                              _raitem->altId() == 2 ||
                              _raitem->altId() == 3);
  else
    _enterReceipt->setEnabled(false);
}

void returnAuthorization::sHandleAction()
{
  QList<QTreeWidgetItem*> selected = _raitem->selectedItems();
  if (selected.size() > 1)
  {
    _action->setText("Close");
    _action->setEnabled(FALSE);
  }
  else if (_raitem->altId() == -1)
  {
    _action->setText("Open");
    _edit->setEnabled(FALSE);
  }
  else
  {
    _action->setText("Close");
    _edit->setEnabled(TRUE);
  }
}

void returnAuthorization::sAction()
{
  q.prepare("UPDATE raitem SET "
            "raitem_status = :status "
            "WHERE (raitem_id=:raitem_id); ");
  q.bindValue(":raitem_id", _raitem->id());
  if (_raitem->altId() == -1)
    q.bindValue(":status",QString("O"));
  else
    q.bindValue(":status",QString("C"));
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    sFillList();
}

void returnAuthorization::sHandleSalesOrderEvent(int pSoheadid, bool)
{
  if ((pSoheadid == _origso->id()) || (pSoheadid == _newso->id()))
    sFillList();
}

void returnAuthorization::sRefund()
{
  if (! sSave(true))
    return;

  bool _post = _disposition->currentItem() == 0 && _timing->currentItem() == 0 &&
                _creditBy->currentItem() == 3;

  q.prepare("SELECT createRaCreditMemo(:rahead_id,:post) AS result;");
  q.bindValue(":rahead_id", _raheadid);
  q.bindValue(":post",      QVariant(_post));
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("createRaCreditMemo", result), __FILE__, __LINE__);
      return;
    }
    ParameterList ccp;
    ccp.append("cmhead_id", result);
    MetaSQLQuery ccm = mqlLoad(":/so/creditMemoCreditCard.mql");
    XSqlQuery ccq = ccm.toQuery(ccp);
    if (ccq.first())
    {
      int ccpayid = ccq.value("ccpay_id").toInt();
      QMessageBox::information( this, tr("New Credit Memo Created"),
                                tr("<p>A new CreditMemo has been created and "
                                   "assigned #%1")
                                   .arg(ccq.value("cmhead_number").toString()));
      CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
      if (! cardproc)
        QMessageBox::critical(this, tr("Credit Card Processing Error"),
                              CreditCardProcessor::errorMsg());
      else
      {
        QString docnum = ccq.value("cmhead_number").toString();
        QString refnum = ccq.value("cohead_number").toString();
        int refid;
        int returnValue = cardproc->credit(ccq.value("ccard_id").toInt(),
                                       (_CCCVV->text().isEmpty()) ? -1 :
                                                        _CCCVV->text().toInt(),
                                       ccq.value("total").toDouble(),
                                       ccq.value("tax_in_cmcurr").toDouble(),
                                       ccq.value("cmhead_tax_id").isNull(),
                                       ccq.value("cmhead_freight").toDouble(),
                                       0,
                                       ccq.value("cmhead_curr_id").toInt(),
                                       docnum, refnum, ccpayid, QString(), refid);
        if (returnValue < 0)
          QMessageBox::critical(this, tr("Credit Card Processing Error"),
                                cardproc->errorMsg());
        else if (returnValue > 0)
          QMessageBox::warning(this, tr("Credit Card Processing Warning"),
                               cardproc->errorMsg());
        else if (! cardproc->errorMsg().isEmpty())
          QMessageBox::information(this, tr("Credit Card Processing Note"),
                               cardproc->errorMsg());
      }
    }
    else if (ccq.lastError().type() != QSqlError::None)
    {
      systemError(this, ccq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      QMessageBox::critical(this, tr("Credit Card Processing Error"),
                            tr("Could not find a Credit Card to use for "
                               "this Credit transaction."));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void returnAuthorization::sPopulateMenu( QMenu * pMenu,  QTreeWidgetItem *selected)
{
  int menuItem;
  menuItem = pMenu->insertItem(tr("Edit Line..."), this, SLOT(sEdit()), 0);
  if (selected->text(5) == "O")
    menuItem = pMenu->insertItem(tr("Close Line..."), this, SLOT(sAction()), 0);
  if (selected->text(5) == "C")
    menuItem = pMenu->insertItem(tr("Open Line..."), this, SLOT(sAction()), 0);
  menuItem = pMenu->insertItem(tr("Delete Line..."), this, SLOT(sDelete()), 0);
  pMenu->insertSeparator();

  if (selected->text(16).length() != 0)
  {
    pMenu->insertItem(tr("View Original Order..."), this, SLOT(sViewOrigOrder()), 0);
    if(!_privileges->check("ViewSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  
    pMenu->insertSeparator();

  if (selected->text(17).length() != 0)
  {
    menuItem = pMenu->insertItem(tr("Edit New Order..."), this, SLOT(sEditNewOrder()), 0);
    if(!_privileges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
    pMenu->insertItem(tr("View New Order..."), this, SLOT(sViewNewOrder()), 0);
    if(!_privileges->check("ViewSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Edit New Order Line..."), this, SLOT(sEditNewOrderLine()), 0);
    if(!_privileges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
    pMenu->insertItem(tr("View New Order Line..."), this, SLOT(sViewNewOrderLine()), 0);
    if(!_privileges->check("ViewSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  
    pMenu->insertSeparator();

    pMenu->insertItem(tr("New Order Shipment Status..."), this, SLOT(sShipmentStatus()), 0);
    pMenu->insertItem(tr("New Order Shipments..."), this, SLOT(sShipment()), 0);
  
  }
}

void returnAuthorization::sViewOrigOrder()
{
  salesOrder::viewSalesOrder(_origso->id());
}

void returnAuthorization::sEditNewOrder()
{
  salesOrder::editSalesOrder(_newso->id(), false);
}

void returnAuthorization::sViewNewOrder()
{
  salesOrder::viewSalesOrder(_newso->id());
}

void returnAuthorization::sEditNewOrderLine()
{
  q.prepare("SELECT coitem_id, cohead_number, "
            "  cohead_curr_id, cohead_orderdate "
            "FROM raitem, cohead, coitem "
            "WHERE ((raitem_id=:raitem_id) "
            "AND (raitem_new_coitem_id=coitem_id) "
            "AND (coitem_cohead_id=cohead_id));");
  q.bindValue(":raitem_id", _raitem->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("soitem_id", q.value("coitem_id").toInt());
    params.append("cust_id", _cust->id());
    params.append("orderNumber", q.value("cohead_number").toString());
    params.append("curr_id", q.value("cohead_curr_id").toInt());
    params.append("orderDate", q.value("cohead_orderdate").toDate());
    params.append("mode", "edit");

    salesOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void returnAuthorization::sViewNewOrderLine()
{
  q.prepare("SELECT coitem_id, cohead_number, "
            "  cohead_curr_id, cohead_orderdate "
            "FROM raitem, cohead, coitem "
            "WHERE ((raitem_id=:raitem_id) "
            "AND (raitem_new_coitem_id=coitem_id) "
            "AND (coitem_cohead_id=cohead_id));");
  q.bindValue(":raitem_id", _raitem->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("soitem_id", q.value("coitem_id").toInt());
    params.append("cust_id", _cust->id());
    params.append("orderNumber", q.value("cohead_number").toString());
    params.append("curr_id", q.value("cohead_curr_id").toInt());
    params.append("orderDate", q.value("cohead_orderdate").toDate());
    params.append("mode", "view");

    salesOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void returnAuthorization::sShipmentStatus()
{
  ParameterList params;
  params.append("sohead_id", _newso->id());
  params.append("run");

  dspSalesOrderStatus *newdlg = new dspSalesOrderStatus();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorization::sShipment()
{
  ParameterList params;
  params.append("sohead_id", _newso->id() );

  dspShipmentsBySalesOrder* newdlg = new dspShipmentsBySalesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorization::sCheckNumber()
{
  q.prepare( "SELECT rahead_id "
                 "FROM rahead "
                 "WHERE (rahead_number=:rahead_number);" );
  q.bindValue(":rahead_number", _authNumber->text());
  q.exec();
  if (q.first())
  {
    _mode = cEdit;
    _raheadid = q.value("rahead_id").toInt();
    populate();
    _authNumber->setEnabled(FALSE);
    _cust->setReadOnly(TRUE);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

