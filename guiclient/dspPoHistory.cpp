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

#include "dspPoHistory.h"

#include <QSqlError>

#include <parameter.h>
#include <openreports.h>

#include "copyPurchaseOrder.h"

dspPoHistory::dspPoHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_copy,	SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_po, SIGNAL(newId(int)), this, SLOT(sFillList()));

  _po->setType((cPOOpen | cPOClosed));

  _poitem->addColumn(tr("#"),            _whsColumn,  Qt::AlignCenter, true, "poitem_linenumber");
  _poitem->addColumn(tr("Item/Doc. #"),  _itemColumn, Qt::AlignLeft,   true, "itemnumber");
  _poitem->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true, "uomname");
  _poitem->addColumn(tr("Due/Recvd."),   _dateColumn, Qt::AlignCenter, true, "poitem_duedate");
  _poitem->addColumn(tr("Vend. Item #"), -1,          Qt::AlignLeft,   true, "poitem_vend_item_number");
  _poitem->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true, "poitem_vend_uom");
  _poitem->addColumn(tr("Ordered"),      _qtyColumn,  Qt::AlignRight,  true, "poitem_qty_ordered");
  _poitem->addColumn(tr("Received"),     _qtyColumn,  Qt::AlignRight,  true, "poitem_qty_received");
  _poitem->addColumn(tr("Returned"),     _qtyColumn,  Qt::AlignRight,  true, "poitem_qty_returned");
}

dspPoHistory::~dspPoHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPoHistory::languageChange()
{
  retranslateUi(this);
}

void dspPoHistory::sFillList()
{
  if (_po->isValid())
  {
    q.prepare( "SELECT poitem_id, poitem.*,"
               "       COALESCE(item_number, :nonInventory) AS itemnumber,"
               "       COALESCE(uom_name, :na) AS uomname,"
               "      'qty' AS poitem_qty_ordered_xtnumericrole,"
               "      'qty' AS poitem_qty_received_xtnumericrole,"
               "      'qty' AS poitem_qty_returned_xtnumericrole "
               "FROM poitem LEFT OUTER JOIN"
               "     ( itemsite JOIN item"
               "       ON (itemsite_item_id=item_id) JOIN uom ON (item_inv_uom_id=uom_id))"
               "     ON (poitem_itemsite_id=itemsite_id) "
               "WHERE (poitem_pohead_id=:pohead_id) "
               "ORDER BY poitem_linenumber;" );
    q.bindValue(":nonInventory", tr("Non-Inventory"));
    q.bindValue(":na", tr("N/A"));
    q.bindValue(":pohead_id", _po->id());
    q.exec();
    _poitem->populate(q);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _poitem->clear();
}

void dspPoHistory::sPrint()
{
  ParameterList params;
  params.append("pohead_id", _po->id());

  orReport report("POHistory", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoHistory::sCopy()
{
  ParameterList params;
  params.append("pohead_id", _po->id());

  copyPurchaseOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}
