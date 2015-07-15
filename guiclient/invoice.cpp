/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "invoice.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <QDebug>

#include "distributeInventory.h"
#include "invoiceItem.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"
#include "allocateARCreditMemo.h"

#define cViewQuote (0x20 | cView)

invoice::invoice(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, fl)
{
  if(name)
    setObjectName(name);

  setupUi(this);

  connect(_close,               SIGNAL(clicked()),                       this,         SLOT(sClose()));
  connect(_save,                SIGNAL(clicked()),                       this,         SLOT(sSave()));
  connect(_cust,                SIGNAL(newId(int)),                      _shipTo,      SLOT(setCustid(int)));
  connect(_cust,                SIGNAL(newId(int)),                      this,         SLOT(sPopulateCustomerInfo(int)));
  connect(_new,                 SIGNAL(clicked()),                       this,         SLOT(sNew()));
  connect(_edit,                SIGNAL(clicked()),                       this,         SLOT(sEdit()));
  connect(_view,                SIGNAL(clicked()),                       this,         SLOT(sView()));
  connect(_delete,              SIGNAL(clicked()),                       this,         SLOT(sDelete()));
  connect(_copyToShipto,        SIGNAL(clicked()),                       this,         SLOT(sCopyToShipto()));
  connect(_taxLit,              SIGNAL(leftClickedURL(const QString&)),  this,         SLOT(sTaxDetail()));
  connect(_shipTo,              SIGNAL(newId(int)),                      this,         SLOT(populateShipto(int)));
  connect(_shipToName,          SIGNAL(textChanged(const QString&)),     this,         SLOT(sShipToModified()));
  connect(_subtotal,            SIGNAL(valueChanged()),                  this,         SLOT(sCalculateTotal()));
  connect(_tax,                 SIGNAL(valueChanged()),                  this,         SLOT(sCalculateTotal()));
  connect(_miscAmount,          SIGNAL(valueChanged()),                  this,         SLOT(sCalculateTotal()));
  connect(_freight,             SIGNAL(valueChanged()),                  this,         SLOT(sFreightChanged()));
  connect(_allocatedCM,         SIGNAL(valueChanged()),                  this,         SLOT(sCalculateTotal()));
  connect(_outstandingCM,       SIGNAL(valueChanged()),                  this,         SLOT(sCalculateTotal()));
  connect(_authCC,              SIGNAL(valueChanged()),                  this,         SLOT(sCalculateTotal()));
  connect(_shipToAddr,          SIGNAL(changed()),                       this,         SLOT(sShipToModified()));
  connect(_shipToPhone,         SIGNAL(textChanged(const QString&)),     this,         SLOT(sShipToModified()));
  connect(_authCC,              SIGNAL(idChanged(int)),                  this,         SLOT(populateCCInfo()));
  connect(_allocatedCM,         SIGNAL(idChanged(int)),                  this,         SLOT(populateCMInfo()));
  connect(_outstandingCM,       SIGNAL(idChanged(int)),                  this,         SLOT(populateCMInfo()));
  connect(_authCC,              SIGNAL(effectiveChanged(const QDate&)),  this,         SLOT(populateCCInfo()));
  if (_privileges->check("ApplyARMemos"))
    connect(_allocatedCMLit,    SIGNAL(leftClickedURL(const QString &)), this,         SLOT(sCreditAllocate()));
  connect(_allocatedCM,         SIGNAL(effectiveChanged(const QDate&)),  this,         SLOT(populateCMInfo()));
  connect(_outstandingCM,       SIGNAL(effectiveChanged(const QDate&)),  this,         SLOT(populateCMInfo()));
  connect(_invcitem,            SIGNAL(valid(bool)),                     _edit,        SLOT(setEnabled(bool)));
  connect(_invcitem,            SIGNAL(valid(bool)),                     _delete,      SLOT(setEnabled(bool)));
  connect(_invcitem,            SIGNAL(itemSelected(int)),               _edit,        SLOT(animateClick()));
  connect(_taxzone,             SIGNAL(newID(int)),	                     this,         SLOT(sTaxZoneChanged()));
  connect(_shipChrgs,           SIGNAL(newID(int)),                      this,         SLOT(sHandleShipchrg(int)));
  connect(_cust,                SIGNAL(newCrmacctId(int)),               _billToAddr,  SLOT(setSearchAcct(int)));
  connect(_cust,                SIGNAL(newCrmacctId(int)),               _shipToAddr,  SLOT(setSearchAcct(int)));
  connect(_invoiceNumber,       SIGNAL(editingFinished()),               this,         SLOT(sCheckInvoiceNumber()));

  setFreeFormShipto(false);

  _custtaxzoneid  = -1;
  _invcheadid	  = -1;
  _taxzoneidCache = -1;
  _loading = false;
  _freightCache = 0;
  _posted = false;
  _NumberGen = -1;

  _cust->setType(CLineEdit::ActiveCustomers);

  _shipTo->setNameVisible(false);
  _shipTo->setDescriptionVisible(false);

  _invcitem->addColumn(tr("#"),             _seqColumn,      Qt::AlignCenter, true,  "invcitem_linenumber" );
  _invcitem->addColumn(tr("Order #"),       _itemColumn,     Qt::AlignLeft,   true,  "soitemnumber"   );
  _invcitem->addColumn(tr("Item"),          _itemColumn,     Qt::AlignLeft,   true,  "itemnumber"   );
  _invcitem->addColumn(tr("Description"),   -1,              Qt::AlignLeft,   true,  "itemdescription"   );
  _invcitem->addColumn(tr("Qty. UOM"),      _uomColumn,      Qt::AlignLeft,   true,  "qtyuom"   );
  _invcitem->addColumn(tr("Ordered"),       _qtyColumn,      Qt::AlignRight,  true,  "invcitem_ordered"  );
  _invcitem->addColumn(tr("Billed"),        _qtyColumn,      Qt::AlignRight,  true,  "invcitem_billed"  );
  _invcitem->addColumn(tr("Price UOM"),     _uomColumn,      Qt::AlignLeft,   true,  "priceuom"   );
  _invcitem->addColumn(tr("Price"),         _moneyColumn,    Qt::AlignRight,  true,  "invcitem_price"  );
  _invcitem->addColumn(tr("Extended"),      _bigMoneyColumn, Qt::AlignRight,  true,  "extprice"  );
  _invcitem->addColumn(tr("Unit Cost"),     _costColumn,     Qt::AlignRight,  false, "unitcost");
  _invcitem->addColumn(tr("Margin"),        _priceColumn,    Qt::AlignRight,  false, "margin");
  _invcitem->addColumn(tr("Margin %"),      _prcntColumn,    Qt::AlignRight,  false, "marginpercent");

  _charass->setType("INV");
  
  _custCurrency->setLabel(_custCurrencyLit);

  _project->setType(ProjectLineEdit::SalesOrder);
  if(!_metrics->boolean("UseProjects"))
    _project->hide();

  _miscAmount->setAllowNegative(true);

  _commission->setValidator(omfgThis->percentVal());

  // some customers are creating scripts to show these widgets, probably shouldn't remove
  _paymentLit->hide();
  _payment->hide(); // Issue 9895:  if no objections over time, we should ultimately remove this.

  _recurring->setParent(-1, "I");

  _miscChargeAccount->setType(GLCluster::cRevenue | GLCluster::cExpense);

  _postInvoice->setEnabled(_privileges->check("PostMiscInvoices"));
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
  XSqlQuery invoiceet;
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      setObjectName("invoice new");
      _mode = cNew;

      invoiceet.exec("SELECT NEXTVAL('invchead_invchead_id_seq') AS invchead_id;");
      if (invoiceet.first())
      {
        _invcheadid = invoiceet.value("invchead_id").toInt();
        _recurring->setParent(_invcheadid, "I");
        _documents->setId(_invcheadid);
        _charass->setId(_invcheadid);
      }
      else if (invoiceet.lastError().type() != QSqlError::NoError)
      {
	    systemError(this, invoiceet.lastError().databaseText(), __FILE__, __LINE__);
	    return UndefinedError;
      }

      if ((_metrics->value("InvcNumberGeneration") == "A") ||
          (_metrics->value("InvcNumberGeneration") == "O"))
      {
        invoiceet.exec("SELECT fetchInvcNumber() AS number;");
        if (invoiceet.first())
        {
          _invoiceNumber->setText(invoiceet.value("number").toString());
          _NumberGen = invoiceet.value("number").toInt();
          if (_metrics->value("InvcNumberGeneration") == "A")
            _invoiceNumber->setEnabled(false);
        }
        else if (invoiceet.lastError().type() != QSqlError::NoError)
        {
          systemError(this, invoiceet.lastError().databaseText(), __FILE__, __LINE__);
          return UndefinedError;
        }
      }
      else
        sCheckInvoiceNumber();

      _orderDate->setDate(omfgThis->dbDate());
      _shipDate->setDate(omfgThis->dbDate());
      _invoiceDate->setDate(omfgThis->dbDate(), true);

      invoiceet.prepare("INSERT INTO invchead ("
				"    invchead_id, invchead_invcnumber, invchead_orderdate,"
                "    invchead_invcdate, invchead_cust_id, invchead_posted,"
				"    invchead_printed, invchead_commission, invchead_freight,"
				"    invchead_misc_amount, invchead_shipchrg_id "
				") VALUES ("
				"    :invchead_id, :invchead_invcnumber, :invchead_orderdate, "
				"    :invchead_invcdate, -1, false,"
				"    false, 0, 0,"
				"    0, -1"
				");");
      invoiceet.bindValue(":invchead_id",	 _invcheadid);
      invoiceet.bindValue(":invchead_invcnumber",_invoiceNumber->text().isEmpty() ?
                                               "TEMP" + QString(0 - _invcheadid)
                                             : _invoiceNumber->text());
      invoiceet.bindValue(":invchead_orderdate", _orderDate->date());
      invoiceet.bindValue(":invchead_invcdate",	 _invoiceDate->date());
      invoiceet.exec();
      if (invoiceet.lastError().type() != QSqlError::NoError)
      {
	    systemError(this, invoiceet.lastError().databaseText(), __FILE__, __LINE__);
	    return UndefinedError;
      }

      connect(_cust,	    SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
      connect(_cust,        SIGNAL(valid(bool)), this, SLOT(populateCMInfo()));
      connect(_orderNumber, SIGNAL(editingFinished()), this, SLOT(populateCCInfo()));
    }
    else if (param.toString() == "edit")
    {

      param = pParams.value("invchead_id", &valid);
      if(valid)
      {
        _invcheadid = param.toInt();
        _charass->setId(_invcheadid);
      }

      setObjectName(QString("invoice edit %1").arg(_invcheadid));
      _mode = cEdit;

      _new->setEnabled(true);
      _cust->setReadOnly(true);
    }
    else if (param.toString() == "view")
    {
      setObjectName(QString("invoice view %1").arg(_invcheadid));
      _mode = cView;

      _invoiceNumber->setEnabled(false);
      _orderNumber->setEnabled(false);
      _invoiceDate->setEnabled(false);
      _shipDate->setEnabled(false);
      _orderDate->setEnabled(false);
      _poNumber->setEnabled(false);
      _cust->setReadOnly(true);
      _salesrep->setEnabled(false);
      _commission->setEnabled(false);
      _taxzone->setEnabled(false);
      _terms->setEnabled(false);
      _terms->setType(XComboBox::Terms);
      _fob->setEnabled(false);
      _shipVia->setEnabled(false);
      _billToName->setEnabled(false);
      _billToAddr->setEnabled(false);
      _billToPhone->setEnabled(false);
      _shipTo->setEnabled(false);
      _shipToName->setEnabled(false);
      _shipToAddr->setEnabled(false);
      _shipToPhone->setEnabled(false);
      _miscAmount->setEnabled(false);
      _miscChargeDescription->setEnabled(false);
      _miscChargeAccount->setReadOnly(true);
      _freight->setEnabled(false);
      _payment->setEnabled(false);
      _notes->setReadOnly(true);
      _edit->hide();
      _save->hide();
      _delete->hide();
      _project->setEnabled(false);
      _shipChrgs->setEnabled(false);
      _shippingZone->setEnabled(false);
      _saleType->setEnabled(false);
//      _documents->setReadOnly(true);
      _charass->setReadOnly(true);
      _postInvoice->setVisible(false);

      disconnect(_invcitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      disconnect(_invcitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      disconnect(_invcitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_invcitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }
  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());

  param = pParams.value("invchead_id", &valid);
  if (valid)
  {
    _invcheadid = param.toInt();
    _documents->setId(_invcheadid);
    _charass->setId(_invcheadid);
    populate();
    populateCMInfo();
    populateCCInfo();
  }

  return NoError;
}

int invoice::id() const
{
  return _invcheadid;
}

int invoice::mode() const
{
  return _mode;
}

void invoice::sClose()
{
  XSqlQuery invoiceClose;
  if (_mode == cNew)
  {
    int answer = QMessageBox::question(this,
                   tr("Delete Invoice?"),
                   tr("<p>This Invoice has not been saved.  "
                      "Do you wish to delete this Invoice?"),
                 QMessageBox::No | QMessageBox::Default, QMessageBox::Yes);
    if (QMessageBox::No == answer)
      return;
    else
    {
      // make sure invoice not posted
      invoiceClose.prepare( "SELECT invchead_posted FROM invchead WHERE (invchead_id=:invchead_id);" );
      invoiceClose.bindValue(":invchead_id", _invcheadid);
      invoiceClose.exec();
      if (invoiceClose.lastError().type() != QSqlError::NoError)
        systemError(this, invoiceClose.lastError().databaseText(), __FILE__, __LINE__);
      if (invoiceClose.first() && invoiceClose.value("invchead_posted").toBool())
      {
        QMessageBox::warning( this, tr("Cannot delete Invoice"),
                              tr("<p>The Invoice has been posted and must be saved.") );
        return;
      }

      invoiceClose.prepare( "SELECT deleteInvoice(:invchead_id) AS result;" );
      invoiceClose.bindValue(":invchead_id", _invcheadid);
      invoiceClose.exec();
      if (invoiceClose.first())
      {
        int result = invoiceClose.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("deleteInvoice", result),
                      __FILE__, __LINE__);
        }
      }
      else if (invoiceClose.lastError().type() != QSqlError::NoError)
        systemError(this,
                    tr("Error deleting Invoice %1\n").arg(_invcheadid) +
                       invoiceClose.lastError().databaseText(), __FILE__, __LINE__);
    }
  }
  else if (_mode == cEdit)
  {
    if (_invcitem->topLevelItemCount() <= 0 )
    {
      int answer = QMessageBox::question(this,
                     tr("Delete Invoice with no Line Items?"),
                     tr("<p>This Invoice does not contain any Line Items "
                        "associated with it. Do you wish to delete "
                        "this Invoice?"),
                   QMessageBox::No | QMessageBox::Default, QMessageBox::Yes);
      if (QMessageBox::No == answer)
        return;
      else
      {
        invoiceClose.prepare( "SELECT deleteInvoice(:invchead_id) AS result;" );
        invoiceClose.bindValue(":invchead_id", _invcheadid);
        invoiceClose.exec();
        if (invoiceClose.first())
        {
          int result = invoiceClose.value("result").toInt();
          if (result < 0)
          {
            systemError(this, storedProcErrorLookup("deleteInvoice", result),
                        __FILE__, __LINE__);
          }
        }
        else if (invoiceClose.lastError().type() != QSqlError::NoError)
          systemError(this,
                      tr("Error deleting Invoice %1\n").arg(_invcheadid) +
                         invoiceClose.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
  }

  close();
}

void invoice::sPopulateCustomerInfo(int pCustid)
{
  if (pCustid != -1)
  {
    XSqlQuery cust;
    cust.prepare( "SELECT cust_name, COALESCE(cntct_addr_id,-1) AS addr_id, "
                  "       cust_salesrep_id, cust_commprcnt * 100 AS commission,"
                  "       cust_creditstatus, cust_terms_id, "
                  "       COALESCE(cust_taxzone_id, -1) AS cust_taxzone_id,"
                  "       COALESCE(cust_shipchrg_id, -1) AS cust_shipchrg_id,"
                  "       cust_ffshipto, cust_ffbillto, "
                  "       COALESCE(shipto_id, -1) AS shiptoid, "
                  "       cust_curr_id "
                  "FROM custinfo "
                  "  LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id) "
                  "  LEFT OUTER JOIN shiptoinfo ON ( (shipto_cust_id=cust_id) "
                  "                         AND (shipto_default) ) "
                  "WHERE (cust_id=:cust_id);" );
    cust.bindValue(":cust_id", pCustid);
    cust.exec();
    if (cust.first())
    {
        _billToName->setText(cust.value("cust_name").toString());
        _billToAddr->setId(cust.value("addr_id").toInt());
	_salesrep->setId(cust.value("cust_salesrep_id").toInt());
	_commission->setDouble(cust.value("commission").toDouble());
	_terms->setId(cust.value("cust_terms_id").toInt());
	_custtaxzoneid = cust.value("cust_taxzone_id").toInt();
	_taxzone->setId(cust.value("cust_taxzone_id").toInt());
	_custCurrency->setId(cust.value("cust_curr_id").toInt());
        _shipChrgs->setId(cust.value("cust_shipchrg_id").toInt());

	bool ffBillTo = cust.value("cust_ffbillto").toBool();
        if (_mode != cView)
        {
          _billToName->setEnabled(ffBillTo);
          _billToAddr->setEnabled(ffBillTo);
          _billToPhone->setEnabled(ffBillTo);
        }

	setFreeFormShipto(cust.value("cust_ffshipto").toBool());
	if (cust.value("shiptoid").toInt() != -1)
	  populateShipto(cust.value("shiptoid").toInt());
	else
	{
          _shipTo->setId(-1);
	  _shipToName->clear();
	  _shipToAddr->clear();
	  _shipToPhone->clear();
	}
      }
      if (cust.lastError().type() != QSqlError::NoError)
      {
	systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
  }
  else
  {
    _salesrep->setCurrentIndex(-1);
    _commission->clear();
    _terms->setCurrentIndex(-1);
    _custtaxzoneid  = -1;
    _taxzoneidCache = -1;
    _taxzone->setCurrentIndex(-1);
  }
}

void invoice::populateShipto(int pShiptoid)
{
  if (pShiptoid != -1)
  {
    XSqlQuery shipto;
    shipto.prepare( "SELECT shipto_id, shipto_num, shipto_name, shipto_addr_id, "
                    "       cntct_phone, shipto_shipvia, shipto_salesrep_id, "
                    "       COALESCE(shipto_taxzone_id, -1) AS shipto_taxzone_id,"
                    "       COALESCE(shipto_shipchrg_id, -1) AS shipto_shipchrg_id,"
                    "       COALESCE(shipto_shipzone_id, -1) AS shipto_shipzone_id,"
                    "       shipto_commission * 100 AS commission "
                    "FROM shiptoinfo LEFT OUTER JOIN "
		    "     cntct ON (shipto_cntct_id=cntct_id)"
                    "WHERE (shipto_id=:shipto_id);" );
    shipto.bindValue(":shipto_id", pShiptoid);
    shipto.exec();
    if (shipto.first())
    {
      _shipToAddr->blockSignals(true);
      _shipTo->blockSignals(true);
      _shipToName->blockSignals(true);
      _shipToPhone->blockSignals(true);

      _shipToName->setText(shipto.value("shipto_name"));
      _shipToAddr->setId(shipto.value("shipto_addr_id").toInt());
      _shipToPhone->setText(shipto.value("cntct_phone"));
      _shipTo->setId(shipto.value("shipto_id").toInt());

      _shipToAddr->blockSignals(false);
      _shipTo->blockSignals(false);
      _shipToName->blockSignals(false);
      _shipToPhone->blockSignals(false);

      _salesrep->setId(shipto.value("shipto_salesrep_id").toInt());
      _commission->setDouble(shipto.value("commission").toDouble());
      _shipVia->setText(shipto.value("shipto_shipvia"));
      _taxzone->setId(shipto.value("shipto_taxzone_id").toInt());
      _shipChrgs->setId(shipto.value("shipto_shipchrg_id").toInt());
      _shippingZone->setId(shipto.value("shipto_shipzone_id").toInt());
    }
    else if (shipto.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipto.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _shipTo->blockSignals(true);
    _shipTo->setId(-1);
    _shipTo->blockSignals(false);
    _shipToName->clear();
    _shipToAddr->clear();
    _shipToPhone->clear();
  }
}

void invoice::sCopyToShipto()
{
  _shipTo->blockSignals(true);
  _shipToAddr->blockSignals(true);
  _shipTo->setId(-1);
  _shipToName->setText(_billToName->text());
  _shipToAddr->setId(_billToAddr->id());
  _shipToPhone->setText(_billToPhone->text());
  _taxzone->setId(_custtaxzoneid);
  _shipTo->blockSignals(false);
  _shipToAddr->blockSignals(false);
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

  //Invoices must have atleast one line item.
  if (_invcitem->topLevelItemCount() <= 0 )
  {
    QMessageBox::information(this, tr("No Line Items"),
                             tr("<p>There must be at least one line item for an invoice.") );
    _new->setFocus();
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
    _tabWidget->setCurrentIndex(_tabWidget->indexOf(lineItemsTab));
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
  _shipToAddr->blockSignals(true);
  _billToAddr->save(AddressCluster::CHANGEONE);
  _shipToAddr->save(AddressCluster::CHANGEONE);
  _shipToAddr->blockSignals(false);
  // finally save the invchead
  if (!save())
    return;

  // post the Invoice if user desires
  if (_postInvoice->isChecked())
    postInvoice();

  omfgThis->sInvoicesUpdated(_invcheadid, true);

  _invcheadid = -1;
  close();
}

bool invoice::save()
{
  XSqlQuery invoiceave;
  RecurrenceWidget::RecurrenceChangePolicy cp = _recurring->getChangePolicy();
  if (cp == RecurrenceWidget::NoPolicy)
    return false;

  XSqlQuery rollbackq;
  rollbackq.prepare("ROLLBACK;");

  XSqlQuery begin("BEGIN;");

  invoiceave.prepare( "UPDATE invchead "
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
	     "    invchead_freight=:invchead_freight,"
	     "    invchead_payment=:invchead_payment,"
	     "    invchead_curr_id=:invchead_curr_id,"
	     "    invchead_shipvia=:invchead_shipvia, invchead_fob=:invchead_fob, invchead_notes=:invchead_notes,"
             "    invchead_recurring_invchead_id=:invchead_recurring_invchead_id,"
             "    invchead_prj_id=:invchead_prj_id,"
             "    invchead_shipchrg_id=:invchead_shipchrg_id, "
             "    invchead_taxzone_id=:invchead_taxzone_id,"
             "    invchead_shipzone_id=:invchead_shipzone_id,"
             "    invchead_saletype_id=:invchead_saletype_id "
	     "WHERE (invchead_id=:invchead_id);" );

  invoiceave.bindValue(":invchead_id",			_invcheadid);
  invoiceave.bindValue(":invchead_invcnumber",		_invoiceNumber->text());
  invoiceave.bindValue(":invchead_cust_id",		_cust->id());
  invoiceave.bindValue(":invchead_invcdate",		_invoiceDate->date());
  invoiceave.bindValue(":invchead_shipdate",		_shipDate->date());
  invoiceave.bindValue(":invchead_orderdate",		_orderDate->date());
  invoiceave.bindValue(":invchead_ponumber",		_poNumber->text());
  invoiceave.bindValue(":invchead_billto_name",		_billToName->text());
  invoiceave.bindValue(":invchead_billto_address1",	_billToAddr->line1());
  invoiceave.bindValue(":invchead_billto_address2",	_billToAddr->line2());
  invoiceave.bindValue(":invchead_billto_address3",	_billToAddr->line3());
  invoiceave.bindValue(":invchead_billto_city",		_billToAddr->city());
  invoiceave.bindValue(":invchead_billto_state",		_billToAddr->state());
  invoiceave.bindValue(":invchead_billto_zipcode",	_billToAddr->postalCode());
  invoiceave.bindValue(":invchead_billto_country",	_billToAddr->country());
  invoiceave.bindValue(":invchead_billto_phone",		_billToPhone->text());
  invoiceave.bindValue(":invchead_shipto_id",		_shipTo->id());
  invoiceave.bindValue(":invchead_shipto_name",		_shipToName->text());
  invoiceave.bindValue(":invchead_shipto_address1",	_shipToAddr->line1());
  invoiceave.bindValue(":invchead_shipto_address2",	_shipToAddr->line2());
  invoiceave.bindValue(":invchead_shipto_address3",	_shipToAddr->line3());
  invoiceave.bindValue(":invchead_shipto_city",		_shipToAddr->city());
  invoiceave.bindValue(":invchead_shipto_state",		_shipToAddr->state());
  invoiceave.bindValue(":invchead_shipto_zipcode",	_shipToAddr->postalCode());
  invoiceave.bindValue(":invchead_shipto_country",	_shipToAddr->country());
  invoiceave.bindValue(":invchead_shipto_phone",		_shipToPhone->text());
  if(_taxzone->isValid())
    invoiceave.bindValue(":invchead_taxzone_id",		_taxzone->id());
  invoiceave.bindValue(":invchead_salesrep_id",		_salesrep->id());
  invoiceave.bindValue(":invchead_terms_id",		_terms->id());
  invoiceave.bindValue(":invchead_commission",	(_commission->toDouble() / 100.0));
  invoiceave.bindValue(":invchead_freight",	_freight->localValue());
  invoiceave.bindValue(":invchead_payment",	_payment->localValue());
  invoiceave.bindValue(":invchead_curr_id",	_custCurrency->id());
  invoiceave.bindValue(":invchead_misc_amount",	_miscAmount->localValue());
  invoiceave.bindValue(":invchead_misc_descrip",	_miscChargeDescription->text());
  invoiceave.bindValue(":invchead_misc_accnt_id",_miscChargeAccount->id());
  invoiceave.bindValue(":invchead_shipvia",	_shipVia->currentText());
  invoiceave.bindValue(":invchead_fob",		_fob->text());
  invoiceave.bindValue(":invchead_notes",	_notes->toPlainText());
  invoiceave.bindValue(":invchead_prj_id",	_project->id());
  invoiceave.bindValue(":invchead_shipchrg_id",	_shipChrgs->id());
  if(_shippingZone->isValid())
    invoiceave.bindValue(":invchead_shipzone_id",	_shippingZone->id());
  if(_saleType->isValid())
    invoiceave.bindValue(":invchead_saletype_id",	_saleType->id());
  if(_recurring->isRecurring())
  {
    if(_recurring->parentId() != 0)
      invoiceave.bindValue(":invchead_recurring_invchead_id", _recurring->parentId());
    else
      invoiceave.bindValue(":invchead_recurring_invchead_id", _invcheadid);
  }

  if (_orderNumber->text().length())
    invoiceave.bindValue(":invchead_ordernumber", _orderNumber->text().toInt());

  invoiceave.exec();
  if (invoiceave.lastError().type() != QSqlError::NoError)
  {
    rollbackq.exec();
    systemError(this, invoiceave.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  QString errmsg;
  if (! _recurring->save(true, cp, &errmsg))
  {
    rollbackq.exec();
    systemError(this, errmsg, __FILE__, __LINE__);
    return false;
  }

  XSqlQuery commitq("COMMIT;");

  return true;
}

void invoice::postInvoice()
{
  XSqlQuery unpostedPost;
  int journal = -1;
  unpostedPost.exec("SELECT fetchJournalNumber('AR-IN') AS result;");
  if (unpostedPost.first())
  {
    journal = unpostedPost.value("result").toInt();
    if (journal < 0)
    {
      systemError(this, storedProcErrorLookup("fetchJournalNumber", journal), __FILE__, __LINE__);
      return;
    }
  }
  else if (unpostedPost.lastError().type() != QSqlError::NoError)
  {
    systemError(this, unpostedPost.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery xrate;
  xrate.prepare("SELECT curr_rate "
		"FROM curr_rate, invchead "
		"WHERE ((curr_id=invchead_curr_id)"
		"  AND  (invchead_id=:invchead_id)"
		"  AND  (invchead_invcdate BETWEEN curr_effective AND curr_expires));");
  // if SUM becomes dependent on curr_id then move XRATE before it in the loop
  XSqlQuery sum;
  sum.prepare("SELECT invoicetotal(:invchead_id) AS subtotal;");

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery post;
  post.prepare("SELECT postInvoice(:invchead_id, :journal) AS result;");

  sum.bindValue(":invchead_id", _invcheadid);
  if (sum.exec() && sum.first() && sum.value("subtotal").toDouble() == 0)
  {
     if (QMessageBox::question(this, tr("Invoice Has Value 0"),
	      		  tr("Invoice #%1 has a total value of 0.\n"
		     	     "Would you like to post it anyway?")
			    .arg(_invoiceNumber->text()),
			  QMessageBox::Yes,
			  QMessageBox::No | QMessageBox::Default)
	     == QMessageBox::No)
	       return;
  }
  else if (sum.lastError().type() != QSqlError::NoError)
  {
    systemError(this, sum.lastError().databaseText(), __FILE__, __LINE__);
  }
  else if (sum.value("subtotal").toDouble() != 0)
  {
     xrate.bindValue(":invchead_id", _invcheadid);
     xrate.exec();
     if (xrate.lastError().type() != QSqlError::NoError)
     {
       systemError(this, tr("System Error posting Invoice #%1\n%2")
	            .arg(_invoiceNumber->text())
	            .arg(xrate.lastError().databaseText()),
                __FILE__, __LINE__);
     }
     else if (!xrate.first() || xrate.value("curr_rate").isNull())
     {
       systemError(this, tr("Could not post Invoice #%1 because of a missing exchange rate.")
				.arg(_invoiceNumber->text()));
     }
  }

  unpostedPost.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  post.bindValue(":invchead_id", _invcheadid);
  post.bindValue(":journal",     journal);
  post.exec();
  if (post.first())
  {
     int result = post.value("result").toInt();
     if (result < 0)
     {
       rollback.exec();
       systemError(this, storedProcErrorLookup("postInvoice", result),
	           __FILE__, __LINE__);
     }
     else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
     {
       rollback.exec();
       QMessageBox::information( this, tr("Post Invoices"), tr("Transaction Canceled") );
       return;
     }

     unpostedPost.exec("COMMIT;");
   }
   // contains() string is hard-coded in stored procedure
   else if (post.lastError().databaseText().contains("post to closed period"))
     rollback.exec();

}

void invoice::sNew()
{
  if (!save())
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("invchead_id", _invcheadid);

  invoiceItem newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillItemList();
}

void invoice::sEdit()
{
  if (!save())
    return;

  ParameterList params;
  params.append("mode", "edit");
  params.append("invchead_id", _invcheadid);
  params.append("invcitem_id", _invcitem->id());

  invoiceItem newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillItemList();
}

void invoice::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("invchead_id", _invcheadid);
  params.append("invcitem_id", _invcitem->id());

  invoiceItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void invoice::sDelete()
{
  XSqlQuery invoiceDelete;
  invoiceDelete.prepare( "DELETE FROM invcitem "
             "WHERE (invcitem_id=:invcitem_id);" );
  invoiceDelete.bindValue(":invcitem_id", _invcitem->id());
  invoiceDelete.exec();
  if (invoiceDelete.lastError().type() != QSqlError::NoError)
  {
    systemError(this, invoiceDelete.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillItemList();
}

void invoice::populate()
{
  XSqlQuery invoicepopulate;
  invoicepopulate.prepare( "SELECT invchead.*,"
             "    COALESCE(invchead_taxzone_id, -1) AS taxzone_id,"
             "    COALESCE(cust_taxzone_id, -1) AS cust_taxzone_id,"
	         "    cust_ffbillto, cust_ffshipto, "
	         "    invchead_shipchrg_id, invchead_ordernumber, "
             "    COALESCE(invchead_shipchrg_id,-1) AS shipchrg_id "
             "FROM invchead LEFT OUTER JOIN cohead "
             "ON (cohead.cohead_number = invchead.invchead_ordernumber) "
             "JOIN custinfo ON (invchead_cust_id = cust_id) "
			 "WHERE (invchead_id=:invchead_id);" );
  invoicepopulate.bindValue(":invchead_id", _invcheadid);
  invoicepopulate.exec();
  if (invoicepopulate.first())
  {
    _loading = true;
    // We are setting the _taxzoneidCache here to the value that
    // sPopulateCustomerInfo is going to set the _taxzone to.
    // The reason for doing this is to prevent sPopulateCustomerInfo
    // from triggering the sTaxZoneChanged() function which does some
    // database manipulation that may be unnecessary or even harmful
    // and we intend to set the values a little further down and they
    // may differ.
    _taxzoneidCache = invoicepopulate.value("cust_taxzone_id").toInt();

    _cust->setId(invoicepopulate.value("invchead_cust_id").toInt());
    _custCurrency->setId(invoicepopulate.value("invchead_curr_id").toInt());

    _invoiceNumber->setText(invoicepopulate.value("invchead_invcnumber").toString());
    _invoiceNumber->setEnabled(false);
    _orderNumber->setText(invoicepopulate.value("invchead_ordernumber").toString());
    if (! _orderNumber->text().isEmpty() && _orderNumber->text().toInt() != 0)
	_custCurrency->setEnabled(false);

    _invoiceDate->setDate(invoicepopulate.value("invchead_invcdate").toDate(), true);
    _orderDate->setDate(invoicepopulate.value("invchead_orderdate").toDate());
    _shipDate->setDate(invoicepopulate.value("invchead_shipdate").toDate());
    _poNumber->setText(invoicepopulate.value("invchead_ponumber").toString());
    _shipVia->setText(invoicepopulate.value("invchead_shipvia").toString());
    _fob->setText(invoicepopulate.value("invchead_fob").toString());
    _shipChrgs->setId(invoicepopulate.value("shipchrg_id").toInt());
    _freightCache=invoicepopulate.value("invchead_freight").toDouble();
    _freight->setLocalValue(invoicepopulate.value("invchead_freight").toDouble());

    if(invoicepopulate.value("invchead_recurring_invchead_id").toInt() != 0)
      _recurring->setParent(invoicepopulate.value("invchead_recurring_invchead_id").toInt(),
                          "I");
    else
      _recurring->setParent(_invcheadid, "I");

    _salesrep->setId(invoicepopulate.value("invchead_salesrep_id").toInt());
    _commission->setDouble(invoicepopulate.value("invchead_commission").toDouble() * 100);
    _taxzoneidCache = invoicepopulate.value("taxzone_id").toInt();
    _taxzone->setId(invoicepopulate.value("taxzone_id").toInt());
    _terms->setId(invoicepopulate.value("invchead_terms_id").toInt());
    _project->setId(invoicepopulate.value("invchead_prj_id").toInt());
    _shippingZone->setId(invoicepopulate.value("invchead_shipzone_id").toInt());
    _saleType->setId(invoicepopulate.value("invchead_saletype_id").toInt());

    bool ffBillTo = invoicepopulate.value("cust_ffbillto").toBool();
    if (_mode != cView)
    {
      _billToName->setEnabled(ffBillTo);
      _billToAddr->setEnabled(ffBillTo);
      _billToPhone->setEnabled(ffBillTo);
    }

    _billToName->setText(invoicepopulate.value("invchead_billto_name").toString());
    _billToAddr->setLine1(invoicepopulate.value("invchead_billto_address1").toString());
    _billToAddr->setLine2(invoicepopulate.value("invchead_billto_address2").toString());
    _billToAddr->setLine3(invoicepopulate.value("invchead_billto_address3").toString());
    _billToAddr->setCity(invoicepopulate.value("invchead_billto_city").toString());
    _billToAddr->setState(invoicepopulate.value("invchead_billto_state").toString());
    _billToAddr->setPostalCode(invoicepopulate.value("invchead_billto_zipcode").toString());
    _billToAddr->setCountry(invoicepopulate.value("invchead_billto_country").toString());
    _billToPhone->setText(invoicepopulate.value("invchead_billto_phone").toString());

    setFreeFormShipto(invoicepopulate.value("cust_ffshipto").toBool());
    _shipToName->setText(invoicepopulate.value("invchead_shipto_name").toString());
    _shipToAddr->setLine1(invoicepopulate.value("invchead_shipto_address1").toString());
    _shipToAddr->setLine2(invoicepopulate.value("invchead_shipto_address2").toString());
    _shipToAddr->setLine3(invoicepopulate.value("invchead_shipto_address3").toString());
    _shipToAddr->setCity(invoicepopulate.value("invchead_shipto_city").toString());
    _shipToAddr->setState(invoicepopulate.value("invchead_shipto_state").toString());
    _shipToAddr->setPostalCode(invoicepopulate.value("invchead_shipto_zipcode").toString());
    _shipToAddr->setCountry(invoicepopulate.value("invchead_shipto_country").toString());
    _shipToPhone->setText(invoicepopulate.value("invchead_shipto_phone").toString());
    _shipTo->blockSignals(true);
    _shipTo->setId(invoicepopulate.value("invchead_shipto_id").toInt());
    _shipTo->blockSignals(false);

    _payment->setLocalValue(invoicepopulate.value("invchead_payment").toDouble());
    _miscChargeDescription->setText(invoicepopulate.value("invchead_misc_descrip"));
    _miscChargeAccount->setId(invoicepopulate.value("invchead_misc_accnt_id").toInt());
    _miscAmount->setLocalValue(invoicepopulate.value("invchead_misc_amount").toDouble());

    _notes->setText(invoicepopulate.value("invchead_notes").toString());

    _posted = invoicepopulate.value("invchead_posted").toBool();
    if (_posted)
    {
      _new->setEnabled(false);
      disconnect(_invcitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      disconnect(_invcitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      _invoiceNumber->setEnabled(false);
      _invoiceDate->setEnabled(false);
      _terms->setEnabled(false);
      _salesrep->setEnabled(false);
      _commission->setEnabled(false);
      _taxzone->setEnabled(false);
      _shipChrgs->setEnabled(false);
      _project->setEnabled(false);
      _miscChargeAccount->setEnabled(false);
      _miscAmount->setEnabled(false);
      _freight->setEnabled(false);
      _shippingZone->setEnabled(false);
      _saleType->setEnabled(false);
      _postInvoice->setVisible(false);
    }

    _loading = false;

    sFillItemList();
  }
  if (invoicepopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, invoicepopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void invoice::sFillItemList()
{
  XSqlQuery invoiceFillItemList;
  invoiceFillItemList.prepare( "SELECT invcitem_id, invcitem_linenumber,"
             "       formatSoItemNumber(invcitem_coitem_id) AS soitemnumber, "
             "       CASE WHEN (item_id IS NULL) THEN invcitem_number"
             "            ELSE item_number"
             "       END AS itemnumber,"
             "       CASE WHEN (item_id IS NULL) THEN invcitem_descrip"
             "            ELSE (item_descrip1 || ' ' || item_descrip2)"
             "       END AS itemdescription,"
             "       quom.uom_name AS qtyuom,"
             "       invcitem_ordered, invcitem_billed,"
             "       puom.uom_name AS priceuom,"
             "       invcitem_price,"
             "       round((invcitem_billed * invcitem_qty_invuomratio) * (invcitem_price / "
	           "            (CASE WHEN(item_id IS NULL) THEN 1 "
	           "			            ELSE invcitem_price_invuomratio END)), 2) AS extprice,"
             "       COALESCE(coitem_unitcost, itemCost(itemsite_id), 0.0) AS unitcost,"
             "       ROUND((invcitem_billed * invcitem_qty_invuomratio) *"
             "             ((invcitem_price / COALESCE(invcitem_price_invuomratio,1.0)) - "
             "              currtolocal(:curr_id, COALESCE(coitem_unitcost, itemCost(itemsite_id), 0.0), :date)),2) AS margin,"
             "       CASE WHEN (invcitem_price = 0.0) THEN 100.0"
             "            ELSE (((invcitem_price - currtolocal(:curr_id, COALESCE(coitem_unitcost, itemCost(itemsite_id), 0.0), :date)) / invcitem_price) * 100.0)"
             "       END AS marginpercent,"
             "       'qty' AS invcitem_ordered_xtnumericrole,"
             "       'qty' AS invcitem_billed_xtnumericrole,"
             "       'salesprice' AS invcitem_price_xtnumericrole,"
             "       'curr' AS extprice_xtnumericrole,"
             "       'cost' AS unitcost_xtnumericrole "
             "FROM invcitem LEFT OUTER JOIN item on (invcitem_item_id=item_id) "
             "  LEFT OUTER JOIN uom AS quom ON (invcitem_qty_uom_id=quom.uom_id)"
             "  LEFT OUTER JOIN uom AS puom ON (invcitem_price_uom_id=puom.uom_id)"
             "  LEFT OUTER JOIN coitem ON (coitem_id=invcitem_coitem_id)"
             "  LEFT OUTER JOIN itemsite ON (itemsite_item_id=invcitem_item_id AND itemsite_warehous_id=invcitem_warehous_id)"
             "WHERE (invcitem_invchead_id=:invchead_id) "
             "ORDER BY invcitem_linenumber;" );
  invoiceFillItemList.bindValue(":invchead_id", _invcheadid);
  invoiceFillItemList.bindValue(":curr_id", _custCurrency->id());
  invoiceFillItemList.bindValue(":date", _orderDate->date());
  invoiceFillItemList.exec();
  if (invoiceFillItemList.lastError().type() != QSqlError::NoError)
      systemError(this, invoiceFillItemList.lastError().databaseText(), __FILE__, __LINE__);

  _invcitem->clear();
  _invcitem->populate(invoiceFillItemList);

  //  Determine the subtotal
  invoiceFillItemList.prepare( "SELECT SUM( round(((invcitem_billed * invcitem_qty_invuomratio) * (invcitem_price /"
             "            CASE WHEN (item_id IS NULL) THEN 1"
             "                 ELSE invcitem_price_invuomratio"
             "            END)),2) ) AS subtotal "
             "FROM invcitem LEFT OUTER JOIN item ON (invcitem_item_id=item_id) "
             "WHERE (invcitem_invchead_id=:invchead_id);" );
  invoiceFillItemList.bindValue(":invchead_id", _invcheadid);
  invoiceFillItemList.exec();
  if (invoiceFillItemList.first())
    _subtotal->setLocalValue(invoiceFillItemList.value("subtotal").toDouble());
  else if (invoiceFillItemList.lastError().type() != QSqlError::NoError)
    systemError(this, invoiceFillItemList.lastError().databaseText(), __FILE__, __LINE__);

  _custCurrency->setEnabled(_invcitem->topLevelItemCount() == 0);

  // TODO: Calculate the Freight weight here.
  sCalculateTax();
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
  XSqlQuery invoicecloseEvent;
  if ( (_mode == cNew) && (_invcheadid != -1) )
  {
    // make sure invoice not posted
    invoicecloseEvent.prepare( "SELECT invchead_posted FROM invchead WHERE (invchead_id=:invchead_id);" );
    invoicecloseEvent.bindValue(":invchead_id", _invcheadid);
    invoicecloseEvent.exec();
    if (invoicecloseEvent.lastError().type() != QSqlError::NoError)
      systemError(this, invoicecloseEvent.lastError().databaseText(), __FILE__, __LINE__);
    if (invoicecloseEvent.first() && invoicecloseEvent.value("invchead_posted").toBool())
    {
      QMessageBox::warning( this, tr("Cannot delete Invoice"),
                            tr("<p>The Invoice has been posted and must be saved.") );
      return;
    }

    invoicecloseEvent.prepare( "DELETE FROM invcitem WHERE (invcitem_invchead_id=:invchead_id);" );
    invoicecloseEvent.bindValue(":invchead_id", _invcheadid);
    invoicecloseEvent.bindValue(":invoiceNumber", _invoiceNumber->text().toInt());
    invoicecloseEvent.exec();
    if (invoicecloseEvent.lastError().type() != QSqlError::NoError)
      systemError(this, invoicecloseEvent.lastError().databaseText(), __FILE__, __LINE__);
    sReleaseNumber();
  }

  XWidget::closeEvent(pEvent);
}

void invoice::sReleaseNumber()
{
  XSqlQuery invoiceReleaseNumber;
  if(-1 != _NumberGen)
  {
    invoiceReleaseNumber.prepare("SELECT releaseInvcNumber(:number);" );
    invoiceReleaseNumber.bindValue(":number", _NumberGen);
    invoiceReleaseNumber.exec();
    if (invoiceReleaseNumber.lastError().type() != QSqlError::NoError)
      systemError(this, invoiceReleaseNumber.lastError().databaseText(), __FILE__, __LINE__);
    _NumberGen = -1;
  }
}


void invoice::sTaxDetail()
{
  if (_mode != cView)
  {
    if (!save())
	  return;
  }

  ParameterList params;
  params.append("order_id", _invcheadid);
  params.append("order_type", "I");

  if (_mode == cView || _posted)
    params.append("mode", "view");
  else if (_mode == cNew || _mode == cEdit)
    params.append("mode", "edit");

  taxBreakdown newdlg(this, "", true);
  if (newdlg.set(params) == NoError)
  {
    newdlg.exec();
    sCalculateTax();
  }
}

void invoice::sCalculateTax()
{
  XSqlQuery taxq;
  taxq.prepare( "SELECT SUM(tax) AS tax "
                "FROM ("
                "SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
                "FROM tax "
                " JOIN calculateTaxDetailSummary('I', :invchead_id, 'T') ON (taxdetail_tax_id=tax_id)"
	        "GROUP BY tax_id) AS data;" );
  taxq.bindValue(":invchead_id", _invcheadid);
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (taxq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  // changing _tax fires sCalculateTotal()
}

void invoice::setFreeFormShipto(bool pFreeForm)
{
  _ffShipto = pFreeForm;

  if (_mode != cView)
  {
    _copyToShipto->setEnabled(_ffShipto);
    _shipToName->setEnabled(_ffShipto);
    _shipToAddr->setEnabled(_ffShipto);
    _shipToPhone->setEnabled(_ffShipto);
  }
  else
    _copyToShipto->setEnabled(false);
}

void invoice::sShipToModified()
{
  _shipTo->blockSignals(true);
  _shipTo->setId(-1);
  _shipTo->setCustid(_cust->id());
  _shipTo->blockSignals(false);
}

void invoice::keyPressEvent( QKeyEvent * e )
{
#ifdef Q_OS_MAC
  if(e->key() == Qt::Key_N && (e->modifiers() & Qt::ControlModifier))
  {
    _new->animateClick();
    e->accept();
  }
  else if(e->key() == Qt::Key_E && (e->modifiers() & Qt::ControlModifier))
  {
    _edit->animateClick();
    e->accept();
  }
  if(e->isAccepted())
    return;
#endif
  e->ignore();
}

void invoice::newInvoice(int pCustid, QWidget *parent )
{
  // Check for an Item window in new mode already.
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->objectName(), "invoice new")==0)
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

  invoice *newdlg = new invoice(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void invoice::editInvoice( int pId, QWidget *parent  )
{
  // Check for an Item window in edit mode for the specified invoice already.
  QString n = QString("invoice edit %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->objectName(), n)==0)
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

  invoice *newdlg = new invoice(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void invoice::viewInvoice( int pId, QWidget *parent  )
{
  // Check for an Item window in edit mode for the specified invoice already.
  QString n = QString("invoice view %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->objectName(), n)==0)
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

  invoice *newdlg = new invoice(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void invoice::sCreditAllocate()
{
  ParameterList params;
  params.append("doctype", "I");
  params.append("invchead_id", _invcheadid);
  params.append("cust_id", _cust->id());
  params.append("total",  _total->localValue());
  params.append("balance",  (_total->localValue() - _allocatedCM->localValue()));
  params.append("curr_id",   _total->id());
  params.append("effective", _total->effective());

  allocateARCreditMemo newdlg(this, "", true);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
  {
    populateCMInfo();
  }
}

void invoice::populateCMInfo()
{
  XSqlQuery cm;

  // Allocated C/M's
  cm.prepare("SELECT COALESCE(SUM(currToCurr(aropenalloc_curr_id, :curr_id,"
             "                               aropenalloc_amount, :effective)),0) AS amount"
             "  FROM ( "
             "  SELECT aropenalloc_curr_id, aropenalloc_amount"
             "    FROM cohead JOIN aropenalloc ON (aropenalloc_doctype='S' AND aropenalloc_doc_id=cohead_id)"
             "   WHERE (cohead_number=:cohead_number) "
             "  UNION ALL"
             "  SELECT aropenalloc_curr_id, aropenalloc_amount"
             "    FROM aropenalloc"
             "   WHERE (aropenalloc_doctype='I' AND aropenalloc_doc_id=:invchead_id)"
             "       ) AS data;");
  cm.bindValue(":invchead_id", _invcheadid);
  cm.bindValue(":cohead_number", _orderNumber->text());
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
             "                noNeg(currToCurr(aropen_curr_id, :curr_id, (aropen_amount - aropen_paid), :effective) - "
             "                      SUM(currToCurr(aropenalloc_curr_id, :curr_id, COALESCE(aropenalloc_amount,0), :effective))) AS amount "
             "           FROM aropen LEFT OUTER JOIN aropenalloc ON (aropenalloc_aropen_id=aropen_id)"
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

void invoice::sHandleShipchrg(int pShipchrgid)
{
  if ( (_mode == cView) || (_mode == cViewQuote) )
    _freight->setEnabled(false);
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
        _freight->setEnabled(true);
      else
      {
        _freight->setEnabled(false);
        _freight->clear();
      }
    }
  }
}

void invoice::sTaxZoneChanged()
{
  if (_loading == false && _invcheadid != -1 && _taxzoneidCache != _taxzone->id())
  {
    if (!save())
	  return;
    _taxzoneidCache = _taxzone->id();
    sCalculateTax();
  }
}

void invoice::sFreightChanged()
{
  if (_loading == false && _invcheadid != -1 && _freightCache != _freight->localValue())
  {
    if (!save())
	  return;
    _freightCache = _freight->localValue();
    sCalculateTax();
  }
}

bool invoice::sCheckInvoiceNumber()
{
  XSqlQuery invoiceCheckInvoiceNumber;
  if (cNew == _mode)
  {
    if(-1 != _NumberGen && _invoiceNumber->text().toInt() != _NumberGen)
      sReleaseNumber();

    if (_invoiceNumber->text().isEmpty())
    {
      QMessageBox::warning(this, tr("Enter Invoice Number"),
                           tr("<p>You must enter an Invoice Number before "
                              "you may continue."));
      _invoiceNumber->setFocus();
    }
    else
    {
      XSqlQuery checkq;
      checkq.prepare("SELECT invchead_invcnumber"
                     "  FROM invchead"
                     " WHERE ((invchead_invcnumber=:number)"
                     "   AND  (invchead_id != :id));");
      checkq.bindValue(":number", _invoiceNumber->text());
      checkq.bindValue(":id",     _invcheadid);
      checkq.exec();
      if (checkq.first())
      {
        QMessageBox::warning(this, tr("Duplicate Invoice Number"),
                             tr("<p>%1 is already used by another Invoice. "
                                "Please enter a different Invoice Number.")
                             .arg(_invoiceNumber->text()));
        _invoiceNumber->setFocus();
      }
      else if (checkq.lastError().type() != QSqlError::NoError)
        systemError(this, invoiceCheckInvoiceNumber.lastError().text(), __FILE__, __LINE__);
      else
      {
        _invoiceNumber->setEnabled(false);
        return true;
      }
    }
  }
  else
    qWarning("invoice::sHandleInvoiceNumber() called but mode isn't cNew");

  return false;
}
