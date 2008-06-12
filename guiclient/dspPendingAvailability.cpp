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

#include "dspPendingAvailability.h"

#include <QVariant>
#include <QValidator>
#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a dspPendingAvailability as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPendingAvailability::dspPendingAvailability(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_item, SIGNAL(privateIdChanged(int)), _warehouse, SLOT(findItemsites(int)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_warehouse, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured);

  _effective->setNullString(tr("Now"));
  _effective->setNullDate(QDate::currentDate());
  _effective->setAllowNullDate(TRUE);
  _effective->setNull();

  _buildDate->setNullString(tr("Latest"));
  _buildDate->setNullDate(omfgThis->endOfTime());
  _buildDate->setAllowNullDate(TRUE);
  _buildDate->setNull();

  _qtyToBuild->setValidator(omfgThis->qtyVal());
  _qtyToBuild->setText("1.0");

  _items->addColumn(tr("#"),            _seqColumn,  Qt::AlignCenter );
  _items->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft   );
  _items->addColumn(tr("Description"),  -1,          Qt::AlignLeft   );
  _items->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter );
  _items->addColumn(tr("Pend. Alloc."), _qtyColumn,  Qt::AlignRight  );
  _items->addColumn(tr("Total Alloc."), _qtyColumn,  Qt::AlignRight  );
  _items->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight  );
  _items->addColumn(tr("Availability"), _qtyColumn,  Qt::AlignRight  );

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPendingAvailability::~dspPendingAvailability()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPendingAvailability::languageChange()
{
  retranslateUi(this);
}

void dspPendingAvailability::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("warehous_id", _warehouse->id());
  params.append("buildQty", _qtyToBuild->toDouble());
  params.append("effectiveDate", _effective->date());
  params.append("buildDate", _buildDate->date());

  if(_showShortages->isChecked())
    params.append("showShortages");

  orReport report("PendingWOMaterialAvailability", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPendingAvailability::sFillList()
{
  if (!checkParameters())
    return;

  _items->clear();

  QString sql( "SELECT itemsite_id, bomitem_seqnumber, item_number, item_descrip, uom_name,"
               "       pendalloc, formatQty(pendalloc) AS f_pendalloc,"
               "       formatQty(totalalloc + pendalloc) AS f_totalalloc,"
               "       qoh, formatQty(qoh) AS f_qoh,"
               "       (qoh + ordered - (totalalloc + pendalloc)) AS totalavail,"
               "       formatQty(qoh + ordered - (totalalloc + pendalloc)) AS f_totalavail,"
               "       reorderlevel "
               "FROM ( SELECT itemsite_id, bomitem_seqnumber, item_number,"
               "              (item_descrip1 || ' ' || item_descrip2) AS item_descrip, uom_name,"
               "              ((itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap))) * :buildQty) AS pendalloc,"
               "              qtyAllocated(itemsite_id, DATE(:buildDate)) AS totalalloc,"
               "              noNeg(itemsite_qtyonhand) AS qoh,"
               "              qtyOrdered(itemsite_id, DATE(:buildDate)) AS ordered,"
               "              CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel"
			   "       FROM itemsite, item, bomitem(:item_id), uom "
               "       WHERE ( (bomitem_item_id=itemsite_item_id)"
               "        AND (itemsite_item_id=item_id)"
               "        AND (item_inv_uom_id=uom_id)"
               "        AND (itemsite_warehous_id=:warehous_id)" );

  if (_effective->isNull())
    sql += " AND (CURRENT_DATE BETWEEN bomitem_effective AND (bomitem_expires-1))) ) AS data ";
  else
    sql += " AND (:effective BETWEEN bomitem_effective AND (bomitem_expires-1))) ) AS data ";

  sql += "ORDER BY bomitem_seqnumber";

  q.prepare(sql);
  q.bindValue(":buildQty", _qtyToBuild->toDouble());
  q.bindValue(":buildDate", _buildDate->date());
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":effective", _effective->date());
  q.exec();
  XTreeWidgetItem *last = 0;
  while (q.next())
  {
    if ( (!_showShortages->isChecked()) ||
         (q.value("totalavail").toDouble() < 0.0) )
    {
      last = new XTreeWidgetItem( _items, last, q.value("itemsite_id").toInt(),
				 q.value("bomitem_seqnumber"), q.value("item_number"),
				 q.value("item_descrip"), q.value("uom_name"),
				 q.value("f_pendalloc"), q.value("f_totalalloc"),
				 q.value("f_qoh"), q.value("f_totalavail")  );

      if (q.value("qoh").toDouble() < q.value("pendalloc").toDouble())
        last->setTextColor(6, "red");

      if (q.value("totalavail").toDouble() < 0.0)
        last->setTextColor(7, "red");
      else if (q.value("totalavail").toDouble() <= q.value("reorderlevel").toDouble())
        last->setTextColor(7, "orange");
    }
  }
}

bool dspPendingAvailability::checkParameters()
{
  return TRUE;
}
