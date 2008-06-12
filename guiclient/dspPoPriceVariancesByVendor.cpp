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

#include "dspPoPriceVariancesByVendor.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a dspPoPriceVariancesByVendor as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPoPriceVariancesByVendor::dspPoPriceVariancesByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_vendor, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _agent->populate( "SELECT usesysid, usename "
                    "FROM pg_user, usr "
                    "WHERE ( (usename=usr_username)"
                    " AND (usr_active)"
                    " AND (usr_agent) ) "
                    "ORDER BY usename;" );
  
  _porecv->addColumn(tr("P/O #"),              _orderColumn, Qt::AlignRight  );
  _porecv->addColumn(tr("Date"),               _dateColumn,  Qt::AlignCenter );
  _porecv->addColumn(tr("Item Number"),        _itemColumn,  Qt::AlignLeft   );
  _porecv->addColumn(tr("Description"),        -1,           Qt::AlignLeft   );
  _porecv->addColumn(tr("Qty."),               _qtyColumn,   Qt::AlignRight  );
  _porecv->addColumn(tr("Purch. Cost"),        _priceColumn, Qt::AlignRight  );
  _porecv->addColumn(tr("Vouchered Cost"),     _priceColumn, Qt::AlignRight  );
  _porecv->addColumn(tr("Std. Cost at Rcpt."), _priceColumn, Qt::AlignRight  );
  _porecv->addColumn(tr("Currency"),           _currencyColumn, Qt::AlignRight  );

  if (omfgThis->singleCurrency())
      _porecv->hideColumn(8);
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPoPriceVariancesByVendor::~dspPoPriceVariancesByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPoPriceVariancesByVendor::languageChange()
{
  retranslateUi(this);
}

void dspPoPriceVariancesByVendor::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                      tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  params.append("vend_id", _vendor->id());

  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  orReport report("PurchasePriceVariancesByVendor", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoPriceVariancesByVendor::sFillList()
{
  QString sql( "SELECT porecv_id, porecv_ponumber,"
               "       formatDate(porecv_date),"
               "       COALESCE(item_number, (:nonInv || porecv_vend_item_number)) AS itemnumber,"
               "       COALESCE(item_descrip1, porecv_vend_item_descrip) AS itemdescrip,"
               "       formatQty(porecv_qty),"
               "       formatPurchPrice(porecv_purchcost),"
               "       formatPurchPrice(currToCurr(vohead_curr_id, porecv_curr_id, SUM(vodist_amount) / vodist_qty, vohead_docdate)),"
               "       formatPurchPrice(porecv_recvcost), "
	       "       currConcat(porecv_curr_id) "
               "FROM vend,"
               "     porecv LEFT OUTER JOIN"
               "     ( itemsite JOIN item"
               "       ON (itemsite_item_id=item_id)"
               "     ) ON (porecv_itemsite_id=itemsite_id) "
               "     LEFT OUTER JOIN"
               "     ( vodist JOIN vohead"
               "       ON (vodist_vohead_id=vohead_id and vohead_posted)"
               "     ) ON ( (vodist_poitem_id=porecv_poitem_id) AND (vodist_vohead_id=porecv_vohead_id) )"
               "WHERE ( (porecv_vend_id=vend_id)"
               " AND (vend_id=:vend_id)"
               " AND (DATE(porecv_date) BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (porecv_itemsite_id IN (SELECT itemsite_id FROM itemsite WHERE (itemsite_warehous_id=:warehous_id)))";

  if (_selectedPurchasingAgent->isChecked())
    sql += " AND (porecv_agent_username=:username)";

  sql += ") "
         "GROUP BY porecv_id, porecv_ponumber, porecv_date, item_number, porecv_vend_item_number,"
         "         item_descrip1, porecv_vend_item_descrip, porecv_qty, porecv_purchcost, porecv_recvcost,"
         "         vodist_qty, vohead_curr_id, porecv_curr_id, vohead_docdate "
         "ORDER BY porecv_date DESC;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":vend_id", _vendor->id());
  q.bindValue(":nonInv", tr("NonInv - "));

  if (_selectedPurchasingAgent->isChecked())
    q.bindValue(":username", _agent->currentText());

  q.exec();
  _porecv->populate(q);
}

