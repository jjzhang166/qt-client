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

// TODO: add checkboxes to distinguish between sales and transfer orders
#include "dspShipmentsByDate.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "inputManager.h"
#include "mqlutil.h"
#include "salesOrderList.h"
#include "printShippingForm.h"

dspShipmentsByDate::dspShipmentsByDate(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_ship, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _ship->setRootIsDecorated(TRUE);
  _ship->addColumn(tr("Shipment #"),         _orderColumn, Qt::AlignLeft  );
  _ship->addColumn(tr("Order Type"),	                 80, Qt::AlignLeft  );
  _ship->addColumn(tr("Ship Date"),           _itemColumn, Qt::AlignCenter);
  _ship->addColumn(tr("#"),                    _seqColumn, Qt::AlignCenter);
  _ship->addColumn(tr("S/O #/Item"),          _itemColumn, Qt::AlignLeft  );
  _ship->addColumn(tr("Customer/Description"),         -1, Qt::AlignLeft  );
  _ship->addColumn(tr("Whs."),                 _whsColumn, Qt::AlignCenter);
  _ship->addColumn(tr("Ordered"),              _qtyColumn, Qt::AlignRight );
  _ship->addColumn(tr("Shipped"),              _qtyColumn, Qt::AlignRight );
  _ship->addColumn(tr("Tracking #"),           _qtyColumn, Qt::AlignRight );
  _ship->addColumn(tr("Freight at Shipping"),  _qtyColumn, Qt::AlignRight );
  _ship->addColumn(tr("Currency"),        _currencyColumn, Qt::AlignRight );
}

dspShipmentsByDate::~dspShipmentsByDate()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspShipmentsByDate::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspShipmentsByDate::set(const ParameterList &pParams)
{ 
  QVariant param;
  bool     valid;

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());
  
  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());
  
  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspShipmentsByDate::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Print Shipping Form..."), this, SLOT(sPrintShippingForm()), 0);
  if (!_privileges->check("PrintBillsOfLading"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspShipmentsByDate::setParams(ParameterList & params)
{
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  _dates->appendValue(params);
}

void dspShipmentsByDate::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  setParams(params);

  orReport report("ShipmentsByDate", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspShipmentsByDate::sPrintShippingForm()
{
  ParameterList params;
  params.append("cosmisc_id", _ship->id());
  params.append("shiphead_id", _ship->id());

  printShippingForm newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspShipmentsByDate::sFillList()
{
  _ship->clear();

  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr("<p>You must enter a Start Date before running this query.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr("<p>You must enter a End Date before running this query.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  setParams(params);
  MetaSQLQuery fillm = mqlLoad(":/sr/displays/ShipmentsByDate/FillListDetail.mql");
  q = fillm.toQuery(params);
  if (q.first())
  {
    XTreeWidgetItem *soshead = 0;
    int shipheadid = -1;
    do
    {
      if (q.value("shiphead_id").toInt() != shipheadid)
      {
        shipheadid = q.value("shiphead_id").toInt();

        soshead = new XTreeWidgetItem( _ship, soshead,
				     q.value("shiphead_id").toInt(),
				     q.value("lineitem_id").toInt(),
                                     q.value("shiphead_number"),
				     q.value("shiphead_order_type"),
				     q.value("f_shipdate"), "",
                                     q.value("order_number"),
				     q.value("customer"),
                                     "", "", "",
                                     q.value("shiphead_tracknum"),
				     q.value("f_freight"));
	soshead->setText(11, q.value("freight_curr_abbr"));
      }

      new XTreeWidgetItem( soshead, q.value("shiphead_id").toInt(),
			 q.value("lineitem_id").toInt(),
                         "", "", "", q.value("linenumber"),
                         q.value("item_number"), q.value("itemdescription"),
                         q.value("warehous_code"), q.value("f_qtyord"),
                         q.value("f_qtyshipped") );
    }
    while (q.next());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
