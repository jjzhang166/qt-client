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

#include "dspCountSlipsByWarehouse.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

dspCountSlipsByWarehouse::dspCountSlipsByWarehouse(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_cntslip, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_showUnposted, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_numericSlips, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_dates, SIGNAL(updated()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _cntslip->addColumn(tr("Slip #"),       _itemColumn,  Qt::AlignLeft   );
  _cntslip->addColumn(tr("Tag #"),        _orderColumn, Qt::AlignLeft   );
  _cntslip->addColumn(tr("Whs."),         _whsColumn,   Qt::AlignCenter );
  _cntslip->addColumn(tr("Item"),         _itemColumn,  Qt::AlignLeft   );
  _cntslip->addColumn(tr("Description"),  -1,           Qt::AlignLeft   );
  _cntslip->addColumn(tr("Entered (By)"), _userColumn,  Qt::AlignCenter );
  _cntslip->addColumn(tr("Qty. Counted"), _qtyColumn,   Qt::AlignRight  );
  _cntslip->addColumn(tr("Posted"),       _dateColumn,  Qt::AlignCenter );

  if (_preferences->boolean("XCheckBox/forgetful"))
    _showUnposted->setChecked(true);
  
  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCountSlipsByWarehouse::~dspCountSlipsByWarehouse()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCountSlipsByWarehouse::languageChange()
{
  retranslateUi(this);
}

void dspCountSlipsByWarehouse::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if(_showUnposted->isChecked())
    params.append("showUnposted");

  if(_numericSlips->isChecked())
    params.append("asNumeric");

  orReport report("CountSlipsByWarehouse", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCountSlipsByWarehouse::sPopulateMenu(QMenu *, QTreeWidgetItem *)
{
}

void dspCountSlipsByWarehouse::sFillList()
{
  _cntslip->clear();

  QString sql("SELECT cntslip_id, ");

  if (_numericSlips->isChecked())
    sql += "CASE WHEN (isNumeric(cntslip_number)) THEN TEXT(toNumeric(cntslip_number, 0))"
           "     ELSE cntslip_number "
           "END AS slipnumber, ";
  else
    sql += "cntslip_number AS slipnumber, ";

  sql += " invcnt_tagnumber, warehous_code,"
         " item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
         " (formatDate(cntslip_entered) || ' (' || getUsername(cntslip_user_id) || ')') AS f_entered, "
         " formatQty(cntslip_qty) AS f_qty, formatBoolYN(cntslip_posted) AS posted "
         "FROM cntslip, invcnt, itemsite, item, warehous "
         "WHERE ((cntslip_cnttag_id=invcnt_id)"
         " AND (invcnt_itemsite_id=itemsite_id)"
         " AND (itemsite_item_id=item_id)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (cntslip_entered BETWEEN :startDate AND :endDate)";

  if (!_showUnposted->isChecked())
    sql += " AND (cntslip_posted)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY slipnumber";

  q.prepare(sql);
  _dates->bindValue(q);
  _warehouse->bindValue(q);
  q.exec();
  if (q.first())
  {
    int           slipNumber = -1;
    XTreeWidgetItem *last      = NULL;

    do
    {
      if (_numericSlips->isChecked())
      {
        if ( (slipNumber != -1) && (slipNumber != (q.value("slipnumber").toInt() - 1)) )
        {
          if (slipNumber == q.value("slipnumber").toInt() - 2)
            last = new XTreeWidgetItem( _cntslip, last, -1,
                                      QVariant("----"), "----", "----", "----",
                                      tr("Missing Slip #%1").arg(slipNumber + 1),
                                      "----", "----", "----" );
          else
            last = new XTreeWidgetItem( _cntslip, last, -1,
                                      QVariant("----"), "----", "----", "----",
                                      tr("Missing Slips #%1 to #%2").arg(slipNumber + 1).arg(q.value("slipnumber").toInt() - 1),
                                      "----", "----", "----" );

          last->setTextColor("red");
        }

        slipNumber = q.value("slipnumber").toInt();
      }

      last = new XTreeWidgetItem( _cntslip, last, q.value("cntslip_id").toInt(),
                                q.value("slipnumber"), q.value("invcnt_tagnumber").toString(),
                                q.value("warehous_code").toString(), q.value("item_number").toString(),
                                q.value("description").toString(), q.value("f_entered").toString(),
                                q.value("f_qty").toString(), q.value("posted").toString() );
    }
    while (q.next());
  }
}
