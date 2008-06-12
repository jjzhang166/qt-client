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

#include "dspShipmentsByShipment.h"

#include <QMenu>
#include <Q3Process>
#include <QMessageBox>
#include <QRegExp>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <openreports.h>

#include "inputManager.h"
#include "printShippingForm.h"

dspShipmentsByShipment::dspShipmentsByShipment(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_shipment, SIGNAL(newId(int)), this, SLOT(sFillList(int)));
  connect(_soship, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _shipment->setType("SO");

  _soship->setRootIsDecorated(TRUE);
  _soship->addColumn(tr("Shipment #"),         _orderColumn, Qt::AlignLeft   );
  _soship->addColumn(tr("Ship Date"),           _itemColumn, Qt::AlignCenter );
  _soship->addColumn(tr("#"),                   _seqColumn,  Qt::AlignCenter );
  _soship->addColumn(tr("Item"),                _itemColumn, Qt::AlignLeft   );
  _soship->addColumn(tr("Description"),         -1,          Qt::AlignLeft   );
  _soship->addColumn(tr("Whs."),                _whsColumn,  Qt::AlignCenter );
  _soship->addColumn(tr("Ordered"),             _qtyColumn,  Qt::AlignRight  );
  _soship->addColumn(tr("Shipped"),             _qtyColumn,  Qt::AlignRight  );
  _soship->addColumn(tr("Tracking Number"),     _qtyColumn,  Qt::AlignRight  );
  _soship->addColumn(tr("Freight at Shipping"), _qtyColumn,  Qt::AlignRight  );
}

dspShipmentsByShipment::~dspShipmentsByShipment()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspShipmentsByShipment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspShipmentsByShipment::set(const ParameterList &pParams)
{ 
  QVariant param;
  bool     valid;

  param = pParams.value("shiphead_id", &valid);
  if (valid)
  {
    _shipment->setId(param.toInt());
    _shipment->setEnabled(FALSE);
  }

  return NoError;
}

void dspShipmentsByShipment::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Print Shipping Form..."), this, SLOT(sPrintShippingForm()), 0);
  if (!_privileges->check("PrintBillsOfLading"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Query Shipment Status..."), this, SLOT(sFillURL()), 0);
}

void dspShipmentsByShipment::sPrint()
{
  ParameterList params;
  params.append("sohead_id", _shipment->id());

  orReport report("ShipmentsByShipment", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspShipmentsByShipment::sPrintShippingForm()
{
  ParameterList params;
  params.append("cosmisc_id", _soship->id());

  printShippingForm newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspShipmentsByShipment::sFillList(int pShipheadid)
{
  _soship->clear();

  if (pShipheadid != -1)
  {
    q.prepare( "SELECT formatDate(orderdate) AS orderdate,"
               "       custponumber,"
               "       cust_name,"
               "       cust_phone"
               "  FROM cust, (SELECT cohead_cust_id AS order_cust_id,"
               "                       cohead_orderdate AS orderdate,"
               "                       cohead_custponumber AS custponumber"
               "                  FROM cohead JOIN shiphead ON (shiphead_order_id=cohead_id AND shiphead_order_type='SO')"
               "                 WHERE(shiphead_id=:shiphead_id)"
               "                UNION"
               "                SELECT NULL AS order_cust_id,"
               "                       tohead_orderdate AS orderdate,"
               "                       NULL AS custponumber"
               "                  FROM tohead JOIN shiphead ON (shiphead_order_id=tohead_id AND shiphead_order_type='TO')"
               "                 WHERE(shiphead_id=:shiphead_id)"
               "               ) AS taborder "
               " WHERE(order_cust_id=cust_id);");
    q.bindValue(":shiphead_id", pShipheadid);
    q.exec();
    if (q.first())
    {
      //_orderDate->setText(q.value("orderdate").toString());
      _poNumber->setText(q.value("custponumber").toString());
      _custName->setText(q.value("cust_name").toString());
      _custPhone->setText(q.value("cust_phone").toString());
    }

    q.prepare( "SELECT cosmisc_id, coitem_id,"
	       "       formatShipmentNumber(cosmisc_id) AS cosmisc_number,"
               "       formatDate(cosmisc_shipdate) AS f_shipdate,"
               "       coitem_linenumber, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2) AS itemdescription,"
               "       warehous_code,"
               "       cosmisc_tracknum,"
               "       formatQty(coitem_qtyord) AS f_qtyord,"
               "       formatQty(SUM(coship_qty)) AS f_qtyshipped,"
               "       formatMoney(cosmisc_freight) AS f_freight "
               "FROM coship, cosmisc, coitem, itemsite, item, warehous "
               "WHERE ( (coship_cosmisc_id=cosmisc_id)"
               " AND (coship_coitem_id=coitem_id)"
               " AND (cosmisc_shipped)"
               " AND (coitem_itemsite_id=itemsite_id)"
               " AND (coitem_status <> 'X')"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (cosmisc_id=:shiphead_id) ) "
               "GROUP BY cosmisc_id, coitem_id,"
               "         cosmisc_shipdate, coitem_linenumber,"
               "         item_number, item_descrip1, item_descrip2, "
	       "         warehous_code, cosmisc_tracknum, cosmisc_freight,"
               "         coitem_qtyord "
               "ORDER BY cosmisc_id DESC, coitem_linenumber DESC;" );
    q.bindValue(":shiphead_id", pShipheadid);
    q.exec();
    if (q.first())
    {
      XTreeWidgetItem *soshead = 0;
      int cosmiscid = -1;
      do
      {
        if (q.value("cosmisc_id").toInt() != cosmiscid)
        {
          cosmiscid = q.value("cosmisc_id").toInt();

          soshead = new XTreeWidgetItem( _soship, q.value("cosmisc_id").toInt(),
				       q.value("coitem_id").toInt(), q.value("cosmisc_number"),
                                       q.value("f_shipdate").toString(), "", "", "", "", "", "",
                                       q.value("cosmisc_tracknum"), q.value("f_freight") );
        }

        new XTreeWidgetItem( soshead, q.value("cosmisc_id").toInt(), q.value("coitem_id").toInt(),
                           "", "", q.value("coitem_linenumber"),
                           q.value("item_number"), q.value("itemdescription"),
                           q.value("warehous_code"), q.value("f_qtyord"),
                           q.value("f_qtyshipped") );
      }
      while (q.next());
      _soship->expandAll();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
  }
}

void dspShipmentsByShipment::sFillURL()
{
    QString url;
    
     q.prepare( "SELECT upper(cosmisc_shipvia) AS cosmisc_shipvia, cosmisc_tracknum, cohead_shiptozipcode "
               "FROM cosmisc, cohead "
               "WHERE ((cosmisc_id=:cosmisc_id) "
               "AND (cosmisc_cohead_id=cohead_id));");
     
    q.bindValue(":cosmisc_id", _soship->id());
    q.exec();
    if (q.first()) {
     bool findShipper;
     findShipper = false;
     
// Code for UPS	
     if (q.value("cosmisc_shipvia").toString ().left(3) == "UPS") {
       QString url("http://wwwapps.ups.com/WebTracking/processInputRequest?HTMLVersion=5.0&loc=en_US&Requester=UPSHome&tracknum=");
       url +=  q.value("cosmisc_tracknum").toString ();
       url +=  "&AgreeToTermsAndConditions=yes&track.x=40&track.y=9";
       findShipper = true;
        omfgThis->launchBrowser(this, url);
      }
     
 // Code for SAIA	
     if (q.value("cosmisc_shipvia").toString ().left(4) == "SAIA") {
       QString url("http://www.SaiaSecure.com/tracing/manifest.asp?UID=&PWD=&PRONum1=");
       QString _partial;
       QString _tracknum;
       _tracknum = q.value("cosmisc_tracknum").toString ();
       int _length_tracknum;
       int _how_many;
       _length_tracknum = _tracknum.length();
       _how_many = _length_tracknum - 5;
       _partial = _tracknum.left(3);
       _partial += _tracknum.mid( 4, _how_many);
       url +=  _partial;
       url +=  "&Type=Pro&x=55&y=8";
       findShipper = true;
        omfgThis->launchBrowser(this, url);
      }

 // Code for A&B
     if ((q.value("cosmisc_shipvia").toString ().left(5) == "A & B") || (q.value("cosmisc_shipvia").toString ().left(3) == "A&B")) {
       QString url("http://www.aandbfreight.com/");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("A & B"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

 // Code for AVERITT
     if (q.value("cosmisc_shipvia").toString ().left(7) == "AVERITT") {
       QString url("http://www.averittexpress.com/");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("AVERITT"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

 // Code for DHL	
     if (q.value("cosmisc_shipvia").toString ().left(3) == "DHL") {
       QString url("http://www.dhl-usa.com/home/Home.asp");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("DHL"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

 // Code for FEDEX	
     if (q.value("cosmisc_shipvia").toString ().left(5) == "FEDEX") {
       QString url("http://www.fedex.com/Tracking?ascend_header=1&clienttype=dotcom&cntry_code=us&language=english&tracknumbers=");
       QString _partial;
       QString _tracknum;
       _tracknum = q.value("cosmisc_tracknum").toString ();
       int _length_tracknum;
       int _how_many;
       _length_tracknum = _tracknum.length();
       _how_many = _length_tracknum - 5;
       QString _check_tail;
       _check_tail = _tracknum.right(2);
       _check_tail = _check_tail.left(1);
       if (_check_tail == "-") {
       // Ok we need to strip the hyphen out
         _partial = _tracknum.left( _length_tracknum -2);
         _partial += _tracknum.right(1);
       }
       else
       {
         _partial = _tracknum;
       }
       url +=  _partial;
       findShipper = true;
        omfgThis->launchBrowser(this, url);
      }

 // Code for R&L	
     if ((q.value("cosmisc_shipvia").toString ().left(5) == "R & L") || (q.value("cosmisc_shipvia").toString ().left(3) == "R&L"))  {       QString url("http://www.rlcarriers.com/new/index.asp");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("R&L"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

 // Code for ROADWAY	
     if (q.value("cosmisc_shipvia").toString ().left(7) == "ROADWAY") {
       QString url("http://www.quiktrak.roadway.com/cgi-bin/quiktrak?type=0&pro0=");
       url += q.value("cosmisc_tracknum").toString ();
       url += "&zip0=";
       url += q.value("cohead_shiptozipcode").toString ();
       findShipper = true;
       omfgThis->launchBrowser(this, url);
      }

 // Code for USF	
     if (q.value("cosmisc_shipvia").toString ().left(3) == "USF") {
       QString url("http://www.usfc.com/shipmentStatus/shipmentStatustWS.jsp?TrackType=H&TrackNumber=");
       url +=  q.value("cosmisc_tracknum").toString ();
       findShipper = true;
       omfgThis->launchBrowser(this, url);
      }

 // Code for WATKINS	
     if (q.value("cosmisc_shipvia").toString ().left(7) == "WATKINS") {
       QString url("http://www.watkins.com/");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("WATKINS"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

 // Code for YELLOW	
     if (q.value("cosmisc_shipvia").toString ().left(7) == "YELLOW") {
       QString url("http://www.myyellow.com/dynamic/services/content/index.jsp");
       findShipper = true;
       omfgThis->launchBrowser(this, url);
       QMessageBox::information(this, tr("YELLOW"), tr("We cannot direct link to shipper at this time - tracking number is ") + q.value("cosmisc_tracknum").toString (), QMessageBox::Ok);
      }

     if (!findShipper){
      QMessageBox::information(this, tr("Shipper"), tr("We do not currently process this shipper ") + q.value("cosmisc_shipvia").toString (), QMessageBox::Ok);
     }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
}
