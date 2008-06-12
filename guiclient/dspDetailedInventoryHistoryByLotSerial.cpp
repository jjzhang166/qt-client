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

#include "dspDetailedInventoryHistoryByLotSerial.h"

#include <QMenu>
#include <QStack>
#include <QMessageBox>

#include <openreports.h>

#include "adjustmentTrans.h"
#include "transferTrans.h"
#include "scrapTrans.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "countTag.h"

dspDetailedInventoryHistoryByLotSerial::dspDetailedInventoryHistoryByLotSerial(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_invhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  _transType->append(cTransTransfers, tr("Transfers")              );
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentItem(0);

  _invhist->addColumn(tr("Whs"),          _whsColumn,          Qt::AlignCenter    );
  _invhist->addColumn(tr("Date"),         (_dateColumn + 30),  Qt::AlignRight  );
  _invhist->addColumn(tr("Type"),         _transColumn,        Qt::AlignCenter );
  _invhist->addColumn(tr("Order #"),      _orderColumn,        Qt::AlignLeft   );
  _invhist->addColumn(tr("Item Number"),  _itemColumn,         Qt::AlignLeft   );
  _invhist->addColumn(tr("Location"),     _dateColumn,         Qt::AlignLeft   );
  _invhist->addColumn(tr("Lot/Serial #"), -1,                  Qt::AlignLeft   );
  _invhist->addColumn(tr("UOM"),          _uomColumn,          Qt::AlignCenter );
  _invhist->addColumn(tr("Trans-Qty"),    _qtyColumn,          Qt::AlignRight  );
  _invhist->addColumn(tr("Qty. Before"),  _qtyColumn,          Qt::AlignRight  );
  _invhist->addColumn(tr("Qty. After"),   _qtyColumn,          Qt::AlignRight  );
}

dspDetailedInventoryHistoryByLotSerial::~dspDetailedInventoryHistoryByLotSerial()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspDetailedInventoryHistoryByLotSerial::languageChange()
{
  retranslateUi(this);
}

void dspDetailedInventoryHistoryByLotSerial::sPrint()
{
  QString trace;

  if (_dateGroup->isChecked())
  {
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
  }


  if ( ((_selected->isChecked() && _lotSerial->number().trimmed().length() == 0)
     || (_pattern->isChecked() && _lotSerialPattern->text().trimmed().length() == 0))
     && (!_item->isValid()) )
  {
    QMessageBox::warning( this, tr("Enter Lot/Serial #"),
                          tr("<p>You must enter a Lot/Serial or Item criteria to print Inventory "
			     "Detail by Lot/Serial #.</p>") );
    _lotSerial->setFocus();
    return;
  }

  if (_traceGroup->isChecked())
  {
    if (_forward->isChecked())
      trace="F";
    else
      trace="B";
  }
  else
    trace="N";

  ParameterList params;
  _dates->appendValue(params);
  if (_item->isValid())
    params.append("itemid", _item->id());
  if (_warehouse->isSelected())
    params.append("warehouseid", _warehouse->id());
  params.append("transType", _transType->id());
  params.append("trace", trace);
  if (_selected->isChecked())
  {
    if (_lotSerial->number().trimmed().length() > 0)
      params.append("lotSerial", _lotSerial->number().trimmed());
  }
  else
  {
    params.append("pattern");
    params.append("lotSerial", _lotSerialPattern->text().trimmed());
  }

  orReport report("DetailedInventoryHistoryByLotSerial", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspDetailedInventoryHistoryByLotSerial::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(2));

  ParameterList params;
  params.append("mode", "view");
  params.append("invhist_id", _invhist->id());

  if (transType == "AD")
  {
    adjustmentTrans *newdlg = new adjustmentTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "TW")
  {
    transferTrans *newdlg = new transferTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "SI")
  {
    scrapTrans *newdlg = new scrapTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "EX")
  {
    expenseTrans *newdlg = new expenseTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "RX")
  {
    materialReceiptTrans *newdlg = new materialReceiptTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "CC")
  {
    countTag newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspDetailedInventoryHistoryByLotSerial::sPopulateMenu(QMenu *menuThis)
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(2));

  if ( (transType == "AD") ||
       (transType == "TW") ||
       (transType == "SI") ||
       (transType == "EX") ||
       (transType == "RX") ||
       (transType == "CC") )
    menuThis->insertItem(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()), 0);
}

void dspDetailedInventoryHistoryByLotSerial::sFillList()
{
  QString trace;
  _invhist->clear();

  if (_dateGroup->isChecked())
  {
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
  }


  if ( ((_selected->isChecked() && _lotSerial->number().trimmed().length() == 0)
     || (_pattern->isChecked() && _lotSerialPattern->text().trimmed().length() == 0))
     && (!_item->isValid()) )
  {
    QMessageBox::warning( this, tr("Enter Lot/Serial #"),
                          tr("<p>You must enter a Lot/Serial or Item criteria to view Inventory "
			     "Detail by Lot/Serial #.</p>") );
    _lotSerial->setFocus();
    return;
  }


  if (_traceGroup->isChecked())
  {
    if (_forward->isChecked())
      trace="F";
    else
      trace="B";
  }
  else
    trace="N";

  q.prepare( "SELECT * FROM lshist(:itemid,:warehouseid,:lotserial,:pattern,:transType,:startDate,:endDate,:trace,1); ");
  _dates->bindValue(q);
  if (_item->isValid())
    q.bindValue(":itemid", _item->id());
  if (_warehouse->isSelected())
    q.bindValue(":warehouseid", _warehouse->id());
  if (_selected->isChecked())
  {
    if (_lotSerial->number().trimmed().length() > 0)
      q.bindValue(":lotserial", _lotSerial->number().trimmed());
    q.bindValue(":pattern", FALSE);
  }
  else
  {   
    q.bindValue(":lotserial", _lotSerialPattern->text().trimmed());
    q.bindValue(":pattern", TRUE);
  }
  q.bindValue(":transType", _transType->id());
  q.bindValue(":trace", trace);
  q.exec();

  QStack<XTreeWidgetItem*> parent;
  XTreeWidgetItem *last = 0;
  int level = 1;
  while (q.next())
  {
    while(q.value("lshist_level").toInt() < level)
    {
      level--;
      last = parent.pop();
    }
    
    while(q.value("lshist_level").toInt() > level)
    {
      level++;
      parent.push(last);
      last = 0;
    }
    
    if(!parent.isEmpty() && parent.top())
      last = new XTreeWidgetItem(parent.top(), last, q.value("lshist_id").toInt(),
                             q.value("lshist_warehous_code"),
			     q.value("lshist_transdate"), q.value("lshist_transtype"),
			     q.value("lshist_ordernumber"), q.value("lshist_item_number"),
			     q.value("lshist_locationname"), q.value("lshist_lotserial"),
			     q.value("lshist_invuom"), q.value("lshist_transqty") );
    else
      last = new XTreeWidgetItem(_invhist, last, q.value("lshist_id").toInt(),
                             q.value("lshist_warehous_code"),
			     q.value("lshist_transdate"), q.value("lshist_transtype"),
			     q.value("lshist_ordernumber"), q.value("lshist_item_number"),
			     q.value("lshist_locationname"), q.value("lshist_lotserial"),
			     q.value("lshist_invuom"), q.value("lshist_transqty") );
                             
    if (q.value("lshist_posted").toBool())
    {
      last->setText(9, q.value("lshist_qty_before").toString());
      last->setText(10, q.value("lshist_qty_after").toString());
    }
    else
      last->setTextColor("orange");
  }
  _invhist->expandAll();
}

