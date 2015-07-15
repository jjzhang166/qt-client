/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shipOrder.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "enterPoReceipt.h"
#include "itemSite.h"
#include "mqlutil.h"
#include "printInvoice.h"
#include "printPackingList.h"
#include "storedProcErrorLookup.h"

shipOrder::shipOrder(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_create,   SIGNAL(toggled(bool)), this, SLOT(sCreateToggled(bool)));
  connect(_select,   SIGNAL(toggled(bool)), this, SLOT(sSelectToggled(bool)));
  connect(_ship,     SIGNAL(clicked()),     this, SLOT(sShip()));
  connect(_shipment, SIGNAL(newId(int)),    this, SLOT(sFillList()));
  connect(_shipment, SIGNAL(newId(int)),    this, SLOT(sFillTracknum()));
  connect(_order,    SIGNAL(newId(int,QString)),    this, SLOT(sHandleOrder()));
  connect(_warehouse,SIGNAL(newID(int)),            this, SLOT(sHandleOrder()));
  connect(_tracknum, SIGNAL(activated(const QString&)), this, SLOT(sFillFreight()));
  connect(_tracknum, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(sFillFreight()));
  connect(_tracknum, SIGNAL(highlighted(const QString&)), this, SLOT(sFillFreight()));

  _captive = false;

  _coitem->addColumn( tr("#"),           _whsColumn,  Qt::AlignCenter , true, "linenumber");
  _coitem->addColumn( tr("Item Number"), _itemColumn, Qt::AlignLeft   , true, "item_number");
  _coitem->addColumn( tr("Description"), -1,          Qt::AlignLeft   , true, "itemdescrip");
  _coitem->addColumn( tr("UOM"),         _uomColumn,  Qt::AlignCenter , true, "uom_name");
  _coitem->addColumn( tr("Qty."),        _qtyColumn,  Qt::AlignRight  , true, "shipitem_qty");

  sCreateToggled(_create->isChecked());
  sHandleButtons();
  _reject = false;

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  
  _order->setAllowedStatuses(OrderLineEdit::Open);
  _order->setAllowedTypes(OrderLineEdit::Sales |
                          OrderLineEdit::Transfer);
  _order->setFromSitePrivsEnforced(true);
  _order->setFocus();

  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);

  _shipValue->setPrecision(omfgThis->moneyVal());
  _shipValue->setDouble(0);
}

shipOrder::~shipOrder()
{
    // no need to delete child widgets, Qt does it all for us
}

void shipOrder::languageChange()
{
    retranslateUi(this);
}

enum SetResponse shipOrder::set(const ParameterList &pParams)
{
  XSqlQuery shipet;
  XSqlQuery siteq;
  XDialog::set(pParams);
  QString  returnValue;
  QVariant param;
  bool     valid;

  param = pParams.value("shiphead_id", &valid);
  if (valid)
  {
    shipet.prepare( "SELECT shiphead_order_id, shiphead_order_type "
               "FROM shiphead "
               "WHERE (shiphead_id=:shiphead_id);" );
    shipet.bindValue(":shiphead_id", param.toInt());
    shipet.exec();
    if (shipet.first())
    {
      _captive = true;	// so order handling can reject if necessary
      if (shipet.value("shiphead_order_type").toString() == "SO")
      {
        siteq.prepare( "SELECT DISTINCT itemsite_warehous_id "
                       "FROM shiphead JOIN shipitem ON (shipitem_shiphead_id=shiphead_id)"
                       "              JOIN coitem ON (coitem_id=shipitem_orderitem_id)"
                       "              JOIN itemsite ON (itemsite_id=coitem_itemsite_id)"
                       "WHERE (shiphead_id=:shiphead_id);" );
        siteq.bindValue(":shiphead_id", param.toInt());
        siteq.exec();
        if (siteq.first())
        {
          _warehouse->setId(siteq.value("itemsite_warehous_id").toInt());
        }
        _order->setId(param.toInt(), "SO");
      }
      else if (shipet.value("shiphead_order_type").toString() == "TO")
      {
        siteq.prepare( "SELECT DISTINCT tohead_src_warehous_id "
                      "FROM shiphead JOIN shipitem ON (shipitem_shiphead_id=shiphead_id)"
                      "              JOIN toitem ON (toitem_id=shipitem_orderitem_id)"
                      "              JOIN tohead ON (tohead_id=toitem_tohead_id)"
                      "WHERE (shiphead_id=:shiphead_id);" );
        siteq.bindValue(":shiphead_id", param.toInt());
        siteq.exec();
        if (siteq.first())
        {
          _warehouse->setId(siteq.value("tohead_src_warehous_id").toInt());
        }
        _order->setId(param.toInt(), "TO");
      }
    }
    else if (shipet.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipet.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    _shipment->setId(param.toInt());	// last because of signal cascade
  }

  if (_reject)
    return UndefinedError;

  return NoError;
}

void shipOrder::sShip()
{
  XSqlQuery shipq;
  shipq.prepare( "UPDATE shiphead "
	     "   SET shiphead_shipvia=:shiphead_shipvia,"
	     "       shiphead_freight=:shiphead_freight,"
	     "       shiphead_freight_curr_id=:shiphead_freight_curr_id,"
	     "       shiphead_tracknum=:shiphead_tracknum "
	     " WHERE (shiphead_id=:shiphead_id);");
  shipq.bindValue(":shiphead_shipvia",	_shipVia->currentText());
  shipq.bindValue(":shiphead_freight",	_freight->localValue());
  shipq.bindValue(":shiphead_freight_curr_id", _freight->id());
  shipq.bindValue(":shiphead_tracknum",	_tracknum->currentText());
  shipq.bindValue(":shiphead_id",		_shipment->id());
  shipq.exec();
  if (shipq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, shipq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");
  // failed insertGLTransaction RETURNs -5 rather than RAISE EXCEPTION
  shipq.exec("BEGIN;");

  shipq.prepare( "SELECT shipShipment(:shiphead_id, :ts) AS result;");
  shipq.bindValue(":shiphead_id", _shipment->id());
  shipq.bindValue(":ts",          _transDate->date());
  shipq.exec();
  if (shipq.first())
  {
    int result = shipq.value("result").toInt();
    if (result == -6)
    {
      rollback.exec();
      shipq.prepare("SELECT itemsite_id, tohead_trns_warehous_id,"
	        "       tohead_dest_warehous_id "
	        "FROM shiphead, shipitem, tohead, toitem, itemsite "
	        "WHERE ((itemsite_item_id=toitem_item_id)"
	        "  AND  (itemsite_warehous_id=tohead_src_warehous_id)"
	        "  AND  (toitem_tohead_id=tohead_id)"
	        "  AND  (shipitem_orderitem_id=toitem_id)"
	        "  AND  (shiphead_id=shipitem_shiphead_id)"
	        "  AND  (shiphead_order_type='TO')"
	        "  AND  (NOT shiphead_shipped)"
	        "  AND  (shiphead_id=:shiphead_id));");
      shipq.bindValue(":shiphead_id", _shipment->id());
      shipq.exec();
      while (shipq.next())
      {
	// missing transit itemsite is fatal here but not missing dest
	int transis = itemSite::createItemSite(this,
					       shipq.value("itemsite_id").toInt(),
				     shipq.value("tohead_trns_warehous_id").toInt(),
				     false);
	int destis  = itemSite::createItemSite(this,
					       shipq.value("itemsite_id").toInt(),
				     shipq.value("tohead_dest_warehous_id").toInt(),
				     true);
	if (transis <= 0 || destis < 0)
	  return;
      }
      if (shipq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, shipq.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      sShip();	// beware of endless loop if you change createItemSite err check
      return;	// return because the recursive call cleans up for us
    }
    else if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("shipShipment", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (shipq.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    QString errorStr = shipq.lastError().databaseText();
    if(errorStr.startsWith("ERROR:  null value in column \"gltrans_accnt_id\" violates not-null constraint"))
      errorStr = tr("One or more required accounts are not set or set incorrectly."
                    " Please make sure that all your Cost Category and Sales Account Assignments"
                    " are complete and correct.");
    systemError(this, errorStr, __FILE__, __LINE__);
    return;
  }

  shipq.exec("COMMIT;");
  if (shipq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, shipq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  // END the transaction

  if (_print->isChecked())
  {
    ParameterList params;
    params.append("shiphead_id", _shipment->id());
    params.append("print");

    printPackingList newdlg(this, "", true);
    newdlg.set(params);
  }

  if (_order->isSO() && _select->isChecked())
  {
    shipq.prepare("SELECT selectUninvoicedShipment(:shiphead_id) AS result;");
    shipq.bindValue(":shiphead_id", _shipment->id());
    shipq.exec();
    if (shipq.first())
    {
      int cobmiscid = shipq.value("result").toInt();
      if (cobmiscid < 0)
      {
	systemError(this,
	      storedProcErrorLookup("selectUninvoicedShipment", cobmiscid),
	      __FILE__, __LINE__);
	return;
      }
      else if (0 == cobmiscid)
      {
	QMessageBox::information(this, tr("Already Invoiced"),
				 tr("<p>This shipment appears to have been "
				   "invoiced already. It will not be selected "
				   "for billing again."));
      }
      else if (_create->isChecked())
      {
        shipq.prepare("SELECT createInvoice(:cobmisc_id) AS result;");
	shipq.bindValue(":cobmisc_id", cobmiscid);
	shipq.exec();
	if (shipq.first())
	{
	  int result = shipq.value("result").toInt();
	  if (result < 0)
	  {
	    systemError(this,
		      storedProcErrorLookup("postBillingSelection", result),
		      __FILE__, __LINE__);
	    return;
	  }
	  ParameterList params;
	  params.append("invchead_id", result);

	  printInvoice newdlg(this, "", true);
	  newdlg.set(params);
	  newdlg.exec();

	  omfgThis->sInvoicesUpdated(result, true);
	  omfgThis->sSalesOrdersUpdated(_order->id());
	}
	else if (shipq.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, shipq.lastError().databaseText() +
		      tr("<p>Although Sales Order %1 was successfully shipped "
			 "and selected for billing, the Invoice was not "
			 "created properly. You may need to create an Invoice "
			 "manually from the Billing Selection.")
			.arg(_order->id()),
		      __FILE__, __LINE__);
	  return;
	}

	omfgThis->sBillingSelectionUpdated(_order->id(), true);
      }
    }
    else if (shipq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipq.lastError().databaseText() +
		  tr("<p>Although Sales Order %1 was successfully shipped, "
		     "it was not selected for billing. You must manually "
		     "select this Sales Order for Billing.")
		    .arg(_order->id()),
		  __FILE__, __LINE__);
      return;
    }
  }

  if (_order->isTO() && _receive->isChecked())
  {
    QString recverr = tr("<p>Receiving inventory for this Transfer Order "
			"failed although Shipping the Order succeeded. "
			"Manually receive this order using the Enter Order "
			"Receipt window.");
    shipq.exec("BEGIN;");
    ParameterList params;

    if (_metrics->boolean("MultiWhs"))
      params.append("MultiWhs");
    params.append("tohead_id",   _order->id());
    params.append("orderid",     _order->id());
    params.append("ordertype",   "TO");
    params.append("shiphead_id", _shipment->id());

    MetaSQLQuery recvm = mqlLoad("receipt", "receiveAll");
    shipq = recvm.toQuery(params);

    while (shipq.next())
    {
      int result = shipq.value("result").toInt();
      if (result < 0)
      {
	rollback.exec();
	systemError(this,
		    recverr + storedProcErrorLookup("enterReceipt", result),
		    __FILE__, __LINE__);
      }
      omfgThis->sPurchaseOrderReceiptsUpdated();
    }
    if (shipq.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, recverr + "<br>" + shipq.lastError().databaseText(),
		  __FILE__, __LINE__);
    }

    shipq.exec("COMMIT;");
    if (shipq.lastError().type() != QSqlError::NoError)
    {
      systemError(this,
		  recverr + "<br>" + shipq.lastError().databaseText(),
		  __FILE__, __LINE__);
    }

    ParameterList recvParams;
    recvParams.append("tohead_id", _order->id());
    enterPoReceipt *newdlg = new enterPoReceipt();
    newdlg->set(recvParams);

    // to address bug 5680, replace
    // omfgThis->handleNewWindow(newdlg, Qt::ApplicationModal);
    // with
    newdlg->sPost();
    // end of replacement
  }

  if (_captive)
    accept();
  else
  {
    _order->setId(-1);
    _order->setEnabled(true);
    sHandleButtons();
    _billToName->clear();
    _shipToName->clear();
    _shipToAddr1->clear();
    _freight->setEnabled(true);
    _freight->reset();
    _shipVia->clear();
    _shipment->clear();
    _shipment->setEnabled(false);
    _close->setText(tr("&Close"));

    _order->setFocus();
  }

  // Update the shipdatasum record to reflect shipped
  shipq.prepare("UPDATE shipdatasum "
		"   SET shipdatasum_shipped=true "
                " WHERE ((shipdatasum_cosmisc_tracknum = :tracknum)"
		"   AND  (   (shipdatasum_shiphead_number=:shiphead_number)"
		"         OR (:shiphead_number IS NULL)));");
  shipq.bindValue(":tracknum", _tracknum->currentText());
  if (! _shipment->number().isEmpty())
    shipq.bindValue(":shiphead_number", _shipment->number());
  shipq.exec();
  if (shipq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, shipq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void shipOrder::sHandleOrder()
{
  if (_order->id() < 0 || _warehouse->id() < 0)
  {
    _shipment->clear();
    _billToName->setText("");
    _shipToName->setText("");
    _shipToAddr1->setText("");
    _shipValue->setDouble(0);
    _coitem->clear();
    _order->setFocus();
    return;
  }
  else if (_order->isSO())
    sHandleSo();
  else if (_order->isTO())
    sHandleTo();
}

void shipOrder::sHandleSo()
{
  XSqlQuery shipHandleSo;
  _coitem->clear();
  _shipment->setEnabled(false);
  _shipment->removeOrderLimit();

  sHandleButtons();


  shipHandleSo.prepare( "SELECT DISTINCT"
                        "       cohead_holdtype, cust_name, cohead_shiptoname, "
                        "       cohead_shiptoaddress1, cohead_curr_id, cohead_freight "
                        "FROM cohead JOIN custinfo ON (cust_id=cohead_cust_id)"
                        "            JOIN coitem ON (coitem_cohead_id=cohead_id)"
                        "            JOIN itemsite ON (itemsite_id=coitem_itemsite_id) "
                        "WHERE ((cohead_id=:sohead_id)"
                        "  AND  (itemsite_warehous_id=:warehous_id));" );
  shipHandleSo.bindValue(":sohead_id", _order->id());
  shipHandleSo.bindValue(":warehous_id", _warehouse->id());
  shipHandleSo.exec();
  if (shipHandleSo.first())
  {
    QString msg;
    if ( (shipHandleSo.value("cohead_holdtype").toString() == "C"))
      msg = storedProcErrorLookup("shipShipment", -12);
    else if (shipHandleSo.value("cohead_holdtype").toString() == "P")
      msg = storedProcErrorLookup("shipShipment", -13);
    else if (shipHandleSo.value("cohead_holdtype").toString() == "R")
      msg = storedProcErrorLookup("shipShipment", -14);
    else if (shipHandleSo.value("cohead_holdtype").toString() == "S")
      msg = storedProcErrorLookup("shipShipment", -15);

    if (! msg.isEmpty())
    {
      QMessageBox::warning(this, tr("Cannot Ship Order"), msg);
      if (_captive)
      {
        _reject = true;	// so set() can return an error
        reject();	// this only works if shipOrder has been exec()'ed
      }
      else
      {
        _order->setId(-1);
        return;
      }
    }

    _freight->setId(shipHandleSo.value("cohead_curr_id").toInt());
    _freight->setLocalValue(shipHandleSo.value("cohead_freight").toDouble());
    _billToName->setText(shipHandleSo.value("cust_name").toString());
    _shipToName->setText(shipHandleSo.value("cohead_shiptoname").toString());
    _shipToAddr1->setText(shipHandleSo.value("cohead_shiptoaddress1").toString());

    QString sql("SELECT shiphead_id "
                "FROM shiphead "
                "<? if exists(\"shiphead_id\") ?>"
                "WHERE (shiphead_id=<? value(\"shiphead_id\") ?>)"
                "<? else ?>"
                "WHERE (shiphead_number=getOpenShipment('SO', <? value(\"sohead_id\") ?>, <? value(\"warehous_id\") ?>))"
                "<? endif ?>"
                ";" );
    ParameterList params;
    params.append("sohead_id", _order->id());
    params.append("warehous_id", _warehouse->id());
    if (_shipment->isValid())
      params.append("shiphead_id", _shipment->id());
    MetaSQLQuery mql(sql);
    shipHandleSo = mql.toQuery(params);
    if (shipHandleSo.first())
    {
      if (_shipment->id() != shipHandleSo.value("shiphead_id").toInt())
        _shipment->setId(shipHandleSo.value("shiphead_id").toInt());

      if (shipHandleSo.next())
      {
        _shipment->setType("SO");
        _shipment->limitToOrder(_order->id());
        _shipment->setEnabled(true);
      }
    }
    else if (shipHandleSo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipHandleSo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else if (_shipment->isValid())
    {
      params.clear();
      params.append("sohead_id", _order->id());
      params.append("warehous_id", _warehouse->id());
      MetaSQLQuery mql(sql);
      shipHandleSo = mql.toQuery(params);
      if (shipHandleSo.first())
      {
        _shipment->setId(shipHandleSo.value("shiphead_id").toInt());
        if (shipHandleSo.next())
        {
          _shipment->setType("SO");
          _shipment->limitToOrder(_order->id());
          _shipment->setEnabled(true);
        }
      }
      else if (shipHandleSo.lastError().type() != QSqlError::NoError)
      {
        systemError(this, shipHandleSo.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      else
        _shipment->clear();
    }
    else
    {
      QMessageBox::warning(this, tr("Nothing to ship"),
                        tr("<p>You may not ship this Sales Order because "
                           "no stock has been issued to shipping for it."));
      _order->setFocus();
      return;
    }
  }
  else if (shipHandleSo.lastError().type() != QSqlError::NoError)
  {
    systemError(this, shipHandleSo.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void shipOrder::sHandleTo()
{
  XSqlQuery shipHandleTo;
  _coitem->clear();
  _shipment->setEnabled(false);
  _shipment->removeOrderLimit();

  sHandleButtons();

  shipHandleTo.prepare("SELECT tohead_freight_curr_id, tohead_destname,"
                       "       tohead_destaddress1,"
                       "       SUM(toitem_freight) + tohead_freight AS freight "
                       "FROM tohead JOIN toitem ON (toitem_tohead_id=tohead_id) "
                       "WHERE ((toitem_status<>'X')"
                       "  AND  (tohead_src_warehous_id=:warehous_id)"
                       "  AND  (tohead_id=:tohead_id)) "
                       "GROUP BY tohead_freight_curr_id, tohead_destname,"
                       "         tohead_destaddress1, tohead_freight;");
  shipHandleTo.bindValue(":tohead_id", _order->id());
  shipHandleTo.bindValue(":warehous_id", _warehouse->id());
  shipHandleTo.exec();
  if (shipHandleTo.first())
  {
    _freight->setId(shipHandleTo.value("tohead_freight_curr_id").toInt());
    _freight->setLocalValue(shipHandleTo.value("freight").toDouble());
    _billToName->setText(tr("Transfer Order"));
    _shipToName->setText(shipHandleTo.value("tohead_destname").toString());
    _shipToAddr1->setText(shipHandleTo.value("tohead_destaddress1").toString());
  }
  else if (shipHandleTo.lastError().type() != QSqlError::NoError)
  {
    systemError(this, shipHandleTo.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  QString sql("SELECT shiphead_id "
              "FROM shiphead "
              "<? if exists(\"shiphead_id\") ?>"
              "WHERE (shiphead_id=<? value(\"shiphead_id\") ?>)"
              "<? else ?>"
              "WHERE (shiphead_number=getOpenShipment('TO', <? value(\"tohead_id\") ?>, <? value(\"warehous_id\") ?>))"
              "<? endif ?>"
              ";" );
  ParameterList params;
  params.append("tohead_id", _order->id());
  params.append("warehous_id", _warehouse->id());
  if (_shipment->isValid())
    params.append("shiphead_id", _shipment->id());
  MetaSQLQuery mql(sql);
  shipHandleTo = mql.toQuery(params);
  if (shipHandleTo.first())
  {
    if (_shipment->id() != shipHandleTo.value("shiphead_id").toInt())
      _shipment->setId(shipHandleTo.value("shiphead_id").toInt());

    if (shipHandleTo.next())
    {
      _shipment->setType("TO");
      _shipment->limitToOrder(_order->id());
      _shipment->setEnabled(true);
    }
  }
  else if (shipHandleTo.lastError().type() != QSqlError::NoError)
  {
    systemError(this, shipHandleTo.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else if (_shipment->isValid())
  {
    params.clear();
    params.append("tohead_id", _order->id());
    params.append("warehous_id", _warehouse->id());
    MetaSQLQuery mql(sql);
    shipHandleTo = mql.toQuery(params);
    if (shipHandleTo.first())
    {
      _shipment->setId(shipHandleTo.value("shiphead_id").toInt());
      if (shipHandleTo.next())
      {
        _shipment->setType("TO");
        _shipment->limitToOrder(_order->id());
        _shipment->setEnabled(true);
      }
    }
    else if (shipHandleTo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipHandleTo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
      _shipment->clear();
  }
  else
  {
    QMessageBox::warning(this, tr("Nothing to ship"),
                      tr("<p>You may not ship this Transfer Order because "
                         "no stock has been issued to shipping for it."));
    _order->setFocus();
    return;
  }
}

void shipOrder::sHandleButtons()
{
  _select->setChecked(_order->isSO() &&
                      _privileges->check("SelectBilling") &&
		     _metrics->boolean("AutoSelectForBilling"));
  _select->setEnabled(_order->isSO() &&
		      _privileges->check("SelectBilling"));
  _create->setEnabled(_order->isSO() &&
		      _privileges->check("SelectBilling"));
  _print->setEnabled(_order->isSO() || _order->isTO());
  _receive->setEnabled(_order->isTO() &&
		       _privileges->check("EnterReceipts"));

  _select->setVisible(! _order->isTO());
  _create->setVisible(! _order->isTO());
  _receive->setVisible(! _order->isSO());
}

void shipOrder::sFillList()
{
  if (_shipment->isValid())
  {
    calcFreight();

    QString ordertype;
    XSqlQuery shipq;
    shipq.prepare("SELECT shiphead_order_id, shiphead_shipvia, shiphead_order_type,"
                  "       shiphead_tracknum, shiphead_freight, shiphead_freight_curr_id,"
                  "       COALESCE(shipchrg_custfreight, true) AS custfreight,"
                  "       COALESCE(shiphead_shipdate,CURRENT_DATE) AS effective "
                  "FROM shiphead LEFT OUTER JOIN "
                  "     shipchrg ON (shiphead_shipchrg_id=shipchrg_id) "
                  "WHERE ( (NOT shiphead_shipped)"
                  " AND (shiphead_id=:shiphead_id));" );
    shipq.bindValue(":shiphead_id", _shipment->id());
    shipq.exec();
    if (shipq.first())
    {
      _order->setId(shipq.value("shiphead_order_id").toInt(),shipq.value("shiphead_order_type").toString());
      _shipVia->setText(shipq.value("shiphead_shipvia").toString());
      ordertype = shipq.value("shiphead_order_type").toString();
      _tracknum->addItem(shipq.value("shiphead_tracknum").toString(), shipq.value("shiphead_tracknum").toString());

      if (shipq.value("custfreight").toBool())
      {
	_freight->setEnabled(true);
	_freight->set(shipq.value("shiphead_freight").toDouble(),
		      shipq.value("shiphead_freight_curr_id").toInt(),
		      shipq.value("effective").toDate(), false);
      }
      else
      {
	_freight->setEnabled(false);
	_freight->set(0,
		      shipq.value("shiphead_freight_curr_id").toInt(),
		      shipq.value("effective").toDate());
      }
    }
    else if (shipq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      QMessageBox::warning(this, tr("Shipment/Order mismatch"),
			   tr("<p>Shipment #%1 either is not part of "
			      "Order #%2 or has already shipped. Please change "
			      "either the Order # or Shipment #.")
			     .arg(_shipment->number())
			     .arg(_order->number()));
      _shipment->clear();
      _shipment->setEnabled(false);
      return;
    }

    ParameterList itemp;
    itemp.append("shiphead_id", _shipment->id());
    itemp.append("ordertype", ordertype);
    if (ordertype == "SO")
      itemp.append("sohead_id", _order->id());
    else if (ordertype == "TO")
      itemp.append("tohead_id", _order->id());

    QString items = "<? if exists(\"sohead_id\") ?>"
		 "SELECT coitem_id,"
		 "       formatSOlinenumber(coitem_id) AS linenumber, item_number,"
		 "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
		 "       uom_name,"
		 "       SUM(shipitem_qty) AS shipitem_qty, "
                 "       'qty' AS shipitem_qty_xtnumericrole "
                 "FROM shiphead JOIN shipitem ON (shipitem_shiphead_id=shiphead_id) "
                 "              JOIN coitem ON (coitem_id=shipitem_orderitem_id) "
                 "              JOIN itemsite ON (itemsite_id=coitem_itemsite_id) "
                 "              JOIN item ON (item_id=itemsite_item_id) "
                 "              JOIN uom ON (uom_id=coitem_qty_uom_id) "
                 "WHERE ( (shiphead_id=<? value(\"shiphead_id\") ?>)"
                 "  AND   (NOT shiphead_shipped) ) "
		 "GROUP BY coitem_id, coitem_linenumber, item_number,"
		 "         uom_name, itemdescrip, coitem_subnumber "
                 "ORDER BY coitem_linenumber, coitem_subnumber;"
		 "<? elseif exists(\"tohead_id\") ?>"
		 "SELECT toitem_id,"
		 "       toitem_linenumber AS linenumber, item_number,"
		 "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
		 "       uom_name,"
		 "       SUM(shipitem_qty) AS shipitem_qty, "
                 "       'qty' AS shipitem_qty_xtnumericrole "
                 "FROM shiphead JOIN shipitem ON (shipitem_shiphead_id=shiphead_id) "
                 "              JOIN toitem ON (toitem_id=shipitem_orderitem_id) "
                 "              JOIN item ON (item_id=toitem_item_id) "
                 "              JOIN uom ON (uom_id=item_inv_uom_id) "
                 "WHERE ( (shiphead_id=<? value(\"shiphead_id\") ?>)"
                 "  AND   (NOT shiphead_shipped) ) "
		 "GROUP BY toitem_id, toitem_linenumber, item_number,"
		 "         uom_name, itemdescrip "
                 "ORDER BY toitem_linenumber;"
		 "<? endif ?>" ;
    MetaSQLQuery itemm(items);
    shipq = itemm.toQuery(itemp);
    _coitem->populate(shipq);
    if (shipq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    QString vals = "<? if exists(\"sohead_id\") ?>"
	    "SELECT SUM(round((shipitem_qty * coitem_qty_invuomratio) * (coitem_price / coitem_price_invuomratio),2)) AS value "
            "FROM shiphead JOIN shipitem ON (shipitem_shiphead_id=shiphead_id) "
            "              JOIN coitem ON (coitem_id=shipitem_orderitem_id) "
            "WHERE ( (shiphead_id=<? value(\"shiphead_id\") ?>)"
            "  AND   (NOT shiphead_shipped) );"
	    "<? elseif exists(\"tohead_id\") ?>"
	    "SELECT SUM(toitem_stdcost * shipitem_qty) AS value "
            "FROM shiphead JOIN shipitem ON (shipitem_shiphead_id=shiphead_id) "
            "              JOIN toitem ON (toitem_id=shipitem_orderitem_id) "
            "WHERE ( (shiphead_id=<? value(\"shiphead_id\") ?>)"
            "  AND   (NOT shiphead_shipped) );"
	    "<? endif ?>" ;
    MetaSQLQuery valm(vals);
    shipq = valm.toQuery(itemp);	// shared parameters
    if(shipq.first())
      _shipValue->setDouble(shipq.value("value").toDouble());
    else if (shipq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _billToName->clear();
    _shipToName->clear();
    _shipToAddr1->clear();
    _freight->setEnabled(true);
    _freight->reset();
    _shipVia->clear();
    _tracknum->clear();
    _shipment->clear();
    _shipment->setEnabled(false);
  }
}

void shipOrder::sSelectToggled( bool yes )
{
  if(!yes && _create->isChecked())
    _create->setChecked(false);
}

void shipOrder::sCreateToggled( bool yes )
{
  if(yes && !_select->isChecked() && _privileges->check("SelectBilling"))
    _select->setChecked(true);
}

// TODO: add cosmisc_id to shipdata and shipdatasum tables
// TODO: integrate transfer orders with shipdatasum table
void shipOrder::sFillFreight()
{
  if (_order->isSO())
  {
    XSqlQuery shipdataQ;
    shipdataQ.prepare( "SELECT shipdatasum_base_freight,"
	       "     shipdatasum_shipper || ' - ' || shipdatasum_billing_option AS shipper_data "
	       "FROM shipdatasum, cohead "
	       "WHERE ( (shipdatasum_cohead_number=cohead_number)"
	       " AND (cohead_id=:sohead_id)  "
	       " AND (shipdatasum_cosmisc_tracknum=:tracknum)"
	       " AND  (   (shipdatasum_shiphead_number=:shiphead_number)"
	       "       OR (:shiphead_number IS NULL))"
	       ") ;");
    shipdataQ.bindValue(":sohead_id", _order->id());
    shipdataQ.bindValue(":tracknum",  _tracknum->currentText());
    if (! _shipment->number().isEmpty())
      shipdataQ.bindValue(":shiphead_number", _shipment->number());
    shipdataQ.exec();
    if (shipdataQ.first())
    {
      _freight->setEnabled(true);
      _freight->setLocalValue(shipdataQ.value("shipdatasum_base_freight").toDouble());
      _shipVia->setText(shipdataQ.value("shipper_data").toString());
    }
    else if (shipdataQ.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipdataQ.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void shipOrder::sFillTracknum()
{
/* Let's not bother everybody all the time about this one possible problem
  if (_order->isTO())
  {
    QMessageBox::warning(this, tr("Not Supported in Transfer Order"),
			 tr("<p>Automatic insertion of Tracking Numbers is "
			    "not yet supported for Transfer Orders."));
    return;
  }
*/
  if (_order->isSO())
  {
    XSqlQuery shipdataQ;
//    _tracknum->clear();
    shipdataQ.prepare( "SELECT -2, shipdatasum_cosmisc_tracknum "
		       "FROM shipdatasum, cohead  "
		       "WHERE ( (shipdatasum_cohead_number=cohead_number)"
		       " AND (cohead_id=:sohead_id) "
		       " AND  (  (shipdatasum_shiphead_number=:shiphead_number)"
		       "      OR (:shiphead_number IS NULL))"
		       " AND (NOT shipdatasum_shipped) ) "
		       "ORDER BY shipdatasum_lastupdated;" );
    shipdataQ.bindValue(":sohead_id", _order->id());
    if (! _shipment->number().isEmpty())
      shipdataQ.bindValue(":shiphead_number", _shipment->number());
    shipdataQ.exec();
    if (shipdataQ.first())
      _tracknum->populate(shipdataQ);
    else if (shipdataQ.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipdataQ.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void shipOrder::calcFreight()
{
  if (_shipment->isValid())
  {
    XSqlQuery data;
    data.prepare("SELECT calcShipFreight(:shiphead_id) AS freight;");
    data.bindValue(":shiphead_id",_shipment->id());
    data.exec();
    if(data.first())
    {
      _freight->setLocalValue(data.value("freight").toDouble());
    }
    else if (data.lastError().type() != QSqlError::NoError)
    {
      systemError(this, data.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}



