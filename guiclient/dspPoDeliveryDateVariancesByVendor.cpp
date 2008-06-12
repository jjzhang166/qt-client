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

#include "dspPoDeliveryDateVariancesByVendor.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>

/*
 *  Constructs a dspPoDeliveryDateVariancesByVendor as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPoDeliveryDateVariancesByVendor::dspPoDeliveryDateVariancesByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_vendor, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());
  
  _porecv->addColumn(tr("P/O #"),              _orderColumn, Qt::AlignRight  );
  _porecv->addColumn(tr("Vendor"),             _orderColumn, Qt::AlignLeft   );
  _porecv->addColumn(tr("Date"),               _dateColumn,  Qt::AlignCenter );
  _porecv->addColumn(tr("Vend. Item #"),       _itemColumn,  Qt::AlignLeft   );
  _porecv->addColumn(tr("Vendor Description"), -1,           Qt::AlignLeft   );
  _porecv->addColumn(tr("Qty."),               _qtyColumn,   Qt::AlignRight  );
  _porecv->addColumn(tr("Due Date"),           _dateColumn,  Qt::AlignRight  );
  _porecv->addColumn(tr("Recv. Date"),         _dateColumn,  Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPoDeliveryDateVariancesByVendor::~dspPoDeliveryDateVariancesByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPoDeliveryDateVariancesByVendor::languageChange()
{
  retranslateUi(this);
}

void dspPoDeliveryDateVariancesByVendor::sPrint()
{
  ParameterList params;
  params.append("vend_id", _vendor->id());

  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  orReport report("DeliveryDateVariancesByVendor", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoDeliveryDateVariancesByVendor::sFillList()
{
  QString sql( "SELECT porecv_id, porecv_ponumber, vend_name,"
               "       formatDate(porecv_date),"
               "       firstLine(porecv_vend_item_number),"
               "       firstLine(porecv_vend_item_descrip),"
               "       formatQty(porecv_qty),"
               "       formatDate(porecv_duedate),"
               "       formatDate(porecv_date) "
               "FROM porecv, vend "
               "WHERE ( (porecv_vend_id=vend_id)"
               " AND (vend_id=:vend_id)"
               " AND (DATE(porecv_date) BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (porecv_itemsite_id in (SELECT itemsite_id FROM itemsite WHERE (itemsite_warehous_id=:warehous_id)))";

  if (_selectedPurchasingAgent->isChecked())
    sql += " AND (porecv_agent_username=:username)";

  sql += ") "
         "ORDER BY porecv_date DESC;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":vend_id", _vendor->id());
  q.bindValue(":username", _agent->currentText());
  q.exec();
  _porecv->populate(q);
}
