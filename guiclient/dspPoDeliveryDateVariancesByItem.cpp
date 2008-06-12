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

#include "dspPoDeliveryDateVariancesByItem.h"

#include <QMessageBox>

#include <openreports.h>
#include <metasql.h>
#include <parameter.h>

#include "guiclient.h"

dspPoDeliveryDateVariancesByItem::dspPoDeliveryDateVariancesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cGeneralPurchased);
  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   TRUE);
  
  _recv->addColumn(tr("P/O #"),      _orderColumn, Qt::AlignRight, true, "recv_order_number");
  _recv->addColumn(tr("Vendor"),     _orderColumn, Qt::AlignLeft,  true, "vend_name");
  _recv->addColumn(tr("Vend. Item #"),_itemColumn, Qt::AlignLeft,  true, "itemnumber");
  _recv->addColumn(tr("Vendor Description"),   -1, Qt::AlignLeft,  true, "itemdescrip");
  _recv->addColumn(tr("Qty."),         _qtyColumn, Qt::AlignRight, true, "recv_qty");
  _recv->addColumn(tr("Due Date"),    _dateColumn, Qt::AlignCenter,true, "recv_duedate");
  _recv->addColumn(tr("Recv. Date"),  _dateColumn, Qt::AlignCenter,true, "recv_date");
}

dspPoDeliveryDateVariancesByItem::~dspPoDeliveryDateVariancesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPoDeliveryDateVariancesByItem::languageChange()
{
  retranslateUi(this);
}

bool dspPoDeliveryDateVariancesByItem::setParams(ParameterList &pParams)
{
  pParams.append("item_id", _item->id());

  _warehouse->appendValue(pParams);
  _dates->appendValue(pParams);

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  return true;
}

void dspPoDeliveryDateVariancesByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("DeliveryDateVariancesByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoDeliveryDateVariancesByItem::sFillList()
{
  QString sql( "SELECT recv.*, vend_name,"
               "       firstLine(recv_vend_item_number) AS itemnumber,"
               "       firstLine(recv_vend_item_descrip) AS itemdescrip,"
               "       'qty' AS recv_qty_xtnumericrole "
               "FROM recv, vend, itemsite "
               "WHERE ((recv_vend_id=vend_id)"
               "  AND  (recv_itemsite_id=itemsite_id)"
               "  AND  (itemsite_item_id=<? value(\"item_id\") ?>)"
               "  AND  (recv_order_type='PO')"
               "  AND  (DATE(recv_date) BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?>)"
               "<? if exists(\"warehous_id\") ?>"
               " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
               "<? endif ?>"
               " <? if exists(\"agentUsername\") ?>"
               " AND (recv_agent_username=<? value(\"agentUsername\") ?>)"
               "<? endif ?>"
               ") "
               "ORDER BY recv_date DESC;" );
  MetaSQLQuery mql(sql);
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _recv->populate(q);
}
