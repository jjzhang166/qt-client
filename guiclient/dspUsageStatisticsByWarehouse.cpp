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

#include "dspUsageStatisticsByWarehouse.h"

#include <QMenu>
#include <QMessageBox>

#include <openreports.h>
#include <parameter.h>

#include "dspInventoryHistoryByItem.h"

dspUsageStatisticsByWarehouse::dspUsageStatisticsByWarehouse(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_usage, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _usage->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft  );
  _usage->addColumn(tr("Description"), -1,          Qt::AlignLeft  );
  _usage->addColumn(tr("Received"),    _qtyColumn,  Qt::AlignRight );
  _usage->addColumn(tr("Issued"),      _qtyColumn,  Qt::AlignRight );
  _usage->addColumn(tr("Sold"),        _qtyColumn,  Qt::AlignRight );
  _usage->addColumn(tr("Scrap"),       _qtyColumn,  Qt::AlignRight );
  _usage->addColumn(tr("Adjustments"), _qtyColumn,  Qt::AlignRight );

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

dspUsageStatisticsByWarehouse::~dspUsageStatisticsByWarehouse()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspUsageStatisticsByWarehouse::languageChange()
{
  retranslateUi(this);
}

void dspUsageStatisticsByWarehouse::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);
  params.append("warehous_id", _warehouse->id());

  orReport report("UsageStatisticsByWarehouse", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspUsageStatisticsByWarehouse::sViewTransactions()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("itemsite_id", _usage->altId());
  params.append("run");

  switch (_column)
  {
    case 2:
      params.append("transtype", "R");
      break;

    case 3:
      params.append("transtype", "I");
      break;

    case 4:
      params.append("transtype", "S");
      break;

    case 5:
      params.append("transtype", "SC");
      break;

    case 6:
      params.append("transtype", "A");
      break;
  }

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUsageStatisticsByWarehouse::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  int intMenuItem;

  _column = pColumn;

  intMenuItem = menuThis->insertItem(tr("View Transactions..."), this, SLOT(sViewTransactions()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    menuThis->setItemEnabled(intMenuItem, FALSE);
}

void dspUsageStatisticsByWarehouse::sFillList()
{
  _usage->clear();

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  q.prepare( "SELECT item_id, itemsite_id, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2),"
             "       formatQty(summTransR(itemsite_id, :startDate, :endDate)),"
             "       formatQty(summTransI(itemsite_id, :startDate, :endDate)),"
             "       formatQty(summTransS(itemsite_id, :startDate, :endDate)),"
             "       formatQty(summTransC(itemsite_id, :startDate, :endDate)),"
             "       formatQty(summTransA(itemsite_id, :startDate, :endDate)) "
             "FROM itemsite, item "
             "WHERE ( (itemsite_item_id=item_id)"
             " AND (itemsite_warehous_id=:warehous_id) ) "
             "ORDER BY item_number;" );
  _dates->bindValue(q);
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  _usage->populate(q, TRUE);
}

