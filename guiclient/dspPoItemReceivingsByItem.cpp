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

#include "dspPoItemReceivingsByItem.h"

#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "mqlutil.h"

dspPoItemReceivingsByItem::dspPoItemReceivingsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showVariances, SIGNAL(toggled(bool)), this, SLOT(sHandleVariance(bool)));

  _item->setType(ItemLineEdit::cGeneralPurchased);

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());
  
  _showVariances->setEnabled(_privileges->check("ViewCosts"));

  _porecv->addColumn(tr("P/O #"),        _orderColumn, Qt::AlignRight, true, "ponumber");
  _porecv->addColumn(tr("Vendor"),       120,          Qt::AlignLeft,  true, "vend_name");
  _porecv->addColumn(tr("Due Date"),     _dateColumn,  Qt::AlignCenter,true, "duedate");
  _porecv->addColumn(tr("Recv. Date"),   _dateColumn,  Qt::AlignCenter,true, "recvdate");
  _porecv->addColumn(tr("Vend. Item #"), _itemColumn,  Qt::AlignLeft,  true, "venditemnumber");
  _porecv->addColumn(tr("Description"),  -1,           Qt::AlignLeft,  true, "venditemdescrip");
  _porecv->addColumn(tr("Rcvd/Rtnd"),    _qtyColumn,   Qt::AlignRight, true, "sense");
  _porecv->addColumn(tr("Qty."),         _qtyColumn,   Qt::AlignRight, true, "qty");
  if (_privileges->check("ViewCosts"))
  {
    _porecv->addColumn(tr("Purch. Cost"), _priceColumn,  Qt::AlignRight,true,"purchcost");
    _porecv->addColumn(tr("Recv. Cost"),  _priceColumn,  Qt::AlignRight,true,"recvcost");
  }

  sHandleVariance(_showVariances->isChecked());
}

dspPoItemReceivingsByItem::~dspPoItemReceivingsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPoItemReceivingsByItem::languageChange()
{
  retranslateUi(this);
}

bool dspPoItemReceivingsByItem::setParams(ParameterList &pParams)
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter Item Number"),
                          tr( "Please enter a valid Item Number." ) );
    _item->setFocus();
    return false;
  }

  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return false;
  }

  _warehouse->appendValue(pParams);
  _dates->appendValue(pParams);

  pParams.append("item_id",  _item->id() );
  pParams.append("received", tr("Received"));
  pParams.append("returned", tr("Returned"));
  pParams.append("nonInv",   tr("NonInv - "));
  pParams.append("na",       tr("N/A"));

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  if (_showVariances->isChecked())
    pParams.append("showVariances");

  return true;
}

void dspPoItemReceivingsByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("ReceiptsReturnsByItem", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);
}

void dspPoItemReceivingsByItem::sHandleVariance(bool pShowVariances)
{
  if (pShowVariances)
  {
    _porecv->showColumn(8);
    _porecv->showColumn(9);
  }
  else
  {
    _porecv->hideColumn(8);
    _porecv->hideColumn(9);
  }
}

void dspPoItemReceivingsByItem::sFillList()
{
  ParameterList params;
  if (! setParams(params))
  {
    _porecv->clear();
    return;
  }
  MetaSQLQuery mql = mqlLoad(":/sr/itemReceivings/FillList.mql");
  q = mql.toQuery(params);

  _porecv->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
