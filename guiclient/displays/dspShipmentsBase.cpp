/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspShipmentsBase.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QRegExp>
#include <QVariant>

#include <metasql.h>
#include <mqlutil.h>
#include <openreports.h>

#include "errorReporter.h"
#include "inputManager.h"
#include "printShippingForm.h"


dspShipmentsBase::dspShipmentsBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : display(parent, name, fl)
{
  setupUi(optionsWidget());
  setListLabel(tr("Shipments"));
  setMetaSQLOptions("shipments", "detail");
  setUseAltId(true);

  connect(_shipment, SIGNAL(newId(int)), this, SLOT(sPopulateShipment(int)));
  connect(_salesOrder, SIGNAL(newId(int,QString)), this, SLOT(sPopulateSalesOrder(int)));

  _salesOrder->setAllowedTypes(OrderLineEdit::Sales);
  _shipment->setType("SO");

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _salesOrder, SLOT(setId(int)));

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Shipment #"),         _orderColumn, Qt::AlignLeft,   true,  "shiphead_number"   );
  list()->addColumn(tr("Ship Date"),           _itemColumn, Qt::AlignCenter, true,  "shiphead_shipdate" );
  list()->addColumn(tr("#"),                   _seqColumn,  Qt::AlignCenter, true,  "linenumber" );
  list()->addColumn(tr("Item"),                _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),         -1,          Qt::AlignLeft,   true,  "itemdescription"   );
  list()->addColumn(tr("Site"),                _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Ordered"),             _qtyColumn,  Qt::AlignRight,  true,  "qtyord"  );
  list()->addColumn(tr("Shipped"),             _qtyColumn,  Qt::AlignRight,  true,  "qtyshipped"  );
  list()->addColumn(tr("Tracking Number"),     _qtyColumn,  Qt::AlignRight,  true,  "shiphead_tracknum"  );
  list()->addColumn(tr("Freight at Shipping"), _qtyColumn,  Qt::AlignRight,  true,  "shiphead_freight"  );
  list()->addColumn(tr("Notes"),               _itemColumn,  Qt::AlignRight, false,  "notes"  );
}

void dspShipmentsBase::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspShipmentsBase::set(const ParameterList &pParams)
{ 
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _salesOrder->setId(param.toInt());
    _salesOrder->setEnabled(false);
    setQueryOnStartEnabled(true);
  }

  param = pParams.value("shiphead_id", &valid);
  if (valid)
  {
    _shipment->setId(param.toInt());
    _shipment->setEnabled(false);
    setQueryOnStartEnabled(true);
  }

  return NoError;
}

void dspShipmentsBase::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Print Shipping Form..."), this, SLOT(sPrintShippingForm()));
  menuItem->setEnabled(_privileges->check("PrintBillsOfLading"));

  menuItem = pMenu->addAction(tr("Query Shipment Status..."), this, SLOT(sFillURL()));
}

bool dspShipmentsBase::setParams(ParameterList& params)
{
  if(_salesOrder->isVisibleTo(this))
    params.append("sohead_id", _salesOrder->id());

  if(_shipment->isVisibleTo(this))
  {
    params.append("shiphead_id", _shipment->id());
    params.append("MultiWhs", true);
  }

  return true;
}

void dspShipmentsBase::sPrintShippingForm()
{
  ParameterList params;
  params.append("shiphead_id", list()->id());

  printShippingForm newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspShipmentsBase::sPopulateSalesOrder(int pSoheadid)
{
  if (pSoheadid != -1)
  {
    XSqlQuery coq;
    coq.prepare( "SELECT cohead_number,"
                 "       cohead_orderdate,"
                 "       cohead_custponumber,"
                 "       cust_name, cntct_phone "
                 "  FROM cohead"
                 "  JOIN custinfo ON (cohead_cust_id=cust_id)"
                 "  LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id)"
                 " WHERE (cohead_id=:sohead_id);" );
    coq.bindValue(":sohead_id", pSoheadid);
    coq.exec();
    if (coq.first())
    {
      _orderDate->setDate(coq.value("cohead_orderdate").toDate());
      _poNumber->setText(coq.value("cohead_custponumber").toString());
      _custName->setText(coq.value("cust_name").toString());
      _custPhone->setText(coq.value("cntct_phone").toString());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Sales Order"),
                                  coq, __FILE__, __LINE__))
      return;
  }
  else
  {
    _orderDate->clear();
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
  }
}

void dspShipmentsBase::sPopulateShipment(int pShipheadid)
{
  if (pShipheadid != -1)
  {
    XSqlQuery shq;
    shq.prepare("SELECT cust_name, cntct_phone,"
                "       cohead_orderdate AS orderdate,"
                "       cohead_custponumber AS custponumber"
                "  FROM shiphead"
                "  JOIN cohead ON (shiphead_order_id=cohead_id)"
                "  JOIN custinfo ON (cohead_cust_id=cust_id)"
                "  JOIN cntct ON (cust_cntct_id=cntct_id)"
                " WHERE (shiphead_id=:shiphead_id) AND (shiphead_order_type='SO')"
                " UNION "
                "SELECT warehous_code, cntct_phone,"
                "       tohead_orderdate AS orderdate,"
                "       NULL AS custponumber"
                "  FROM shiphead"
                "  JOIN tohead ON (shiphead_order_id=tohead_id)"
                "  JOIN whsinfo ON (tohead_dest_warehous_id=warehous_id)"
                "  LEFT OUTER JOIN cntct ON (warehous_cntct_id=cntct_id)"
                " WHERE (shiphead_id=:shiphead_id) AND (shiphead_order_type='TO');");
    shq.bindValue(":shiphead_id", pShipheadid);
    shq.exec();
    if (shq.first())
    {
      _orderDate->setDate(shq.value("orderdate").toDate());
      _poNumber->setText(shq.value("custponumber").toString());
      _custName->setText(shq.value("cust_name").toString());
      _custPhone->setText(shq.value("cntct_phone").toString());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Shipment"),
                                  shq, __FILE__, __LINE__))
      return;
  }
  else
  {
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
  }
}

void dspShipmentsBase::sFillURL()
{
   XSqlQuery shq;
   shq.prepare("SELECT UPPER(shiphead_shipvia) AS shiphead_shipvia,"
               "       shiphead_tracknum, cohead_shiptozipcode "
               "  FROM shiphead, cohead "
               " WHERE ((shiphead_id=:shiphead_id) "
               "    AND (shiphead_order_id=cohead_id));");
   
  shq.bindValue(":shiphead_id", list()->id());
  shq.exec();
  if (shq.first())
  {
   QString url;
   bool    canOpenTrackingPage = true;
   QString tracknum = shq.value("shiphead_tracknum").toString();
   QString shipvia  = shq.value("shiphead_shipvia").toString();
   
   if (shipvia.startsWith("UPS"))
   {
     url = "http://wwwapps.ups.com/WebTracking/processInputRequest?HTMLVersion=5.0&loc=en_US&Requester=UPSHome&tracknum=";
     url +=  tracknum;
     url +=  "&AgreeToTermsAndConditions=yes&track.x=40&track.y=9";
    }
   
   if (shipvia.startsWith("SAIA"))
   {
     url = "http://www.SaiaSecure.com/tracing/manifest.asp?UID=&PWD=&PRONum1=";
     int _how_many = tracknum.length() - 5;
     QString _partial;
     _partial = tracknum.left(3);
     _partial += tracknum.mid( 4, _how_many);
     url +=  _partial;
     url +=  "&Type=Pro&x=55&y=8";
    }

   if (shipvia.startsWith("A & B") || shipvia.startsWith("A&B"))
   {
     url = "http://www.aandbfreight.com/";
     canOpenTrackingPage = false;
    }

   if (shipvia.startsWith("AVERITT"))
   {
     url = "http://www.averittexpress.com/";
     canOpenTrackingPage = false;
    }

   if (shipvia.startsWith("DHL"))
   {
     // see http://www.dhl-usa.com/en/express/tracking/tracking_tools.html
     url = QString("http://www.dhl-usa.com/home/content/us/en/express/tracking.shtml?brand=DHL&AWB=")
         + tracknum;
   }

   if (shipvia.startsWith("FEDEX"))
   {
     url = "http://www.fedex.com/Tracking?ascend_header=1&clienttype=dotcom&cntry_code=us&language=english&tracknumbers=";
     QString _partial;
     QString _check_tail;
     _check_tail = tracknum.right(2);
     _check_tail = _check_tail.left(1);
     if (_check_tail == "-")
     {
     // Ok we need to strip the hyphen out
       _partial = tracknum.left(tracknum.length() - 2);
       _partial += tracknum.right(1);
     }
     else
       _partial = tracknum;

     url += _partial;
    }

   //add USPS Tracking
   if (shipvia.startsWith("USPS"))
   {
     url = QString("https://tools.usps.com/go/TrackConfirmAction_input?qtc_tLabels1=")
         + tracknum;
   }

   if (shipvia.startsWith("R & L") || shipvia.startsWith("R&L"))
   {
     url = "http://www.rlcarriers.com/index.asp";
     canOpenTrackingPage = false;
   }

   if (shipvia.startsWith("ROADWAY"))
   {
     url = "http://www.quiktrak.roadway.com/cgi-bin/quiktrak?type=0&pro0=";
     url += tracknum;
     url += "&zip0=";
     url += shq.value("cohead_shiptozipcode").toString ();
    }

   if (shipvia.startsWith("USF"))
   {
     url = "http://www.usfc.com/shipmentStatus/shipmentStatustWS.jsp?TrackType=H&TrackNumber=";
     url += tracknum;
    }

   if (shipvia.startsWith("WATKINS"))
   {
     url = "http://www.watkins.com/";
     canOpenTrackingPage = false;
    }

   if (shipvia.startsWith("YELLOW"))
   {
     /* TODO: http://my.yrc.com/dynamic/national/servlet?
                   CONTROLLER=com.rdwy.ec.rextracking
                   .http.controller.ProcessPublicTrackingController
                   &type=1&pro0=nnnnnnnnnn&ozip0=nnnnn&dzip0=nnnnnn 
        type = 1 to track by bill of lading #, 2 by p/o #, 3 by booking #
        ozip0 = origin zip code, dzip = destination zip code
      */
     url = "http://www.myyellow.com/dynamic/services/content/index.jsp";
     canOpenTrackingPage = false;
    }

   if (url.isEmpty())
    QMessageBox::information(this, tr("Shipper"),
                             tr("We do not currently process the shipper %1.")
                             .arg(shipvia));
   else
     omfgThis->launchBrowser(this, url);

   if (! canOpenTrackingPage)
     QMessageBox::information(this, tr("Cannot Track %1").arg(shipvia),
                              tr("We cannot directly show tracking pages for "
                                 "the shipper %1. The tracking number is %2.")
                                  .arg(shipvia, tracknum));
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Shipment Information"),
                                shq, __FILE__, __LINE__))
  {
    return;
  }
}
