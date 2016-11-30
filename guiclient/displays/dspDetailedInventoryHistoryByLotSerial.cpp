/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspDetailedInventoryHistoryByLotSerial.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include "adjustmentTrans.h"
#include "transferTrans.h"
#include "scrapTrans.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "countTag.h"
#include "parameterwidget.h"

dspDetailedInventoryHistoryByLotSerial::dspDetailedInventoryHistoryByLotSerial(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspDetailedInventoryHistoryByLotSerial", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Detailed Inventory History by Lot/Serial #"));
  setListLabel(tr("Inventory History"));
  setReportName("DetailedInventoryHistoryByLotSerial");
  setMetaSQLOptions("detailedInventoryHistory", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);

  connect(_selected, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_pattern, SIGNAL(clicked()), this, SLOT(sSelect()));

  QString qryTransType = QString("SELECT %1 AS id, '%8' AS type  UNION "
                                 "SELECT %2 AS id, '%9' AS type  UNION "
                                 "SELECT %3 AS id, '%10' AS type  UNION "
                                 "SELECT %4 AS id, '%11' AS type  UNION "
                                 "SELECT %5 AS id, '%12' AS type  UNION "
                                 "SELECT %6 AS id, '%13' AS type  UNION "
                                 "SELECT %7 AS id, '%14' AS type;")
      .arg(cTransAll)
      .arg(cTransReceipts)
      .arg(cTransIssues)
      .arg(cTransShipments)
      .arg(cTransAdjCounts)
      .arg(cTransTransfers)
      .arg(cTransScraps)
      .arg(tr("All Transactions"))
      .arg(tr("Receipts"))
      .arg(tr("Issues") )
      .arg(tr("Shipments"))
      .arg(tr("Adjustments and Counts"))
      .arg(tr("Transfers"))
      .arg(tr("Scraps") );

  QString qryTrace = QString("SELECT 0 AS id, '%1' AS type  "
                             "UNION "
                             "SELECT 1 AS id, '%2' AS type; ")
                             .arg(tr("Forward"))
                             .arg(tr("Backward"));

  list()->addColumn(tr("Site"),         _whsColumn,          Qt::AlignCenter, true,  "lshist_warehous_code" );
  list()->addColumn(tr("Date"),         (_dateColumn + 30),  Qt::AlignRight,  true,  "lshist_transdate"  );
  list()->addColumn(tr("Type"),         _transColumn,        Qt::AlignCenter, true,  "lshist_transtype" );
  list()->addColumn(tr("Order #"),      _orderColumn,        Qt::AlignLeft,   true,  "lshist_ordernumber"   );
  list()->addColumn(tr("Item Number"),  _itemColumn,         Qt::AlignLeft,   true,  "lshist_item_number"   );
  list()->addColumn(tr("Location"),     _dateColumn,         Qt::AlignLeft,   true,  "lshist_locationname"   );
  list()->addColumn(tr("Lot/Serial #"), -1,                  Qt::AlignLeft,   true,  "lshist_lotserial"   );
  list()->addColumn(tr("Perishable"), -1,                    Qt::AlignLeft,   true,  "lshist_perishable"   );
  list()->addColumn(tr("Expiration"),   _dateColumn,         Qt::AlignLeft,   true,  "lshist_expiration"   );
  list()->addColumn(tr("UOM"),          _uomColumn,          Qt::AlignCenter, true,  "lshist_invuom" );
  list()->addColumn(tr("Trans-Qty"),    _qtyColumn,          Qt::AlignRight,  true,  "lshist_transqty"  );
  list()->addColumn(tr("Qty. Before"),  _qtyColumn,          Qt::AlignRight,  true,  "lshist_qty_before"  );
  list()->addColumn(tr("Qty. After"),   _qtyColumn,          Qt::AlignRight,  true,  "lshist_qty_after"  );

  list()->addColumn(tr("Customer #/Name"),              170, Qt::AlignLeft,   false, "customerinfo");
  list()->addColumn(tr("ShipTo Name"),                  170, Qt::AlignLeft,   false, "shiptoname");
  list()->addColumn(tr("ShipTo Line1"),                 200, Qt::AlignLeft,   false, "shiptol1");
  list()->addColumn(tr("ShipTo Line2"),                 200, Qt::AlignLeft,   false, "shiptol2");
  list()->addColumn(tr("ShipTo Line3"),                 200, Qt::AlignLeft,   false, "shiptol3");
  list()->addColumn(tr("City/State/Zip"),               200, Qt::AlignLeft,   false, "citystatezip");
  list()->addColumn(tr("Country"),                      120, Qt::AlignLeft,   false, "country");
  list()->addColumn(tr("Contact"),                      120, Qt::AlignLeft,   false, "contact");
  list()->addColumn(tr("Contact Phone"),                100, Qt::AlignLeft,   false, "contact_phone");
  list()->addColumn(tr("Contact Fax"),                  100, Qt::AlignLeft,   false, "contact_fax");
  list()->addColumn(tr("Contact Email"),                175, Qt::AlignLeft,   false, "contact_email");
  list()->addColumn(tr("Phone"),                        100, Qt::AlignLeft,   false, "phone");
  
  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date);
  parameterWidget()->append(tr("End Date"),   "endDate",   ParameterWidget::Date);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehouseid", ParameterWidget::Site);
  parameterWidget()->appendComboBox(tr("Transaction Type"), "transType", qryTransType);
  parameterWidget()->appendComboBox(tr("Multi-Level Trace"), "traceId", qryTrace);
}

void dspDetailedInventoryHistoryByLotSerial::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspDetailedInventoryHistoryByLotSerial::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)list()->currentItem())->text(2));

  ParameterList params;
  params.append("mode", "view");
  params.append("invhist_id", list()->id());

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
    countTag newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspDetailedInventoryHistoryByLotSerial::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  QString transType(((XTreeWidgetItem *)list()->currentItem())->text(2));

  if ( (transType == "AD") ||
       (transType == "TW") ||
       (transType == "SI") ||
       (transType == "EX") ||
       (transType == "RX") ||
       (transType == "CC") )
    menuThis->addAction(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()));
}

void dspDetailedInventoryHistoryByLotSerial::sFillList()
{
  display::sFillList();
  list()->expandAll();
}

void dspDetailedInventoryHistoryByLotSerial::sSelect()
{
  if (_selected->isChecked())
  {
    _lotSerial->setEnabled(true);
    _lotSerialPattern->setEnabled(false);
  }
  else if (_pattern->isChecked())
  {
    _lotSerial->setEnabled(false);
    _lotSerialPattern->setEnabled(true);
  }
}

bool dspDetailedInventoryHistoryByLotSerial::setParams(ParameterList &params)
{
  QString trace = "N";
  params = parameterWidget()->parameters();

  if ( ((_selected->isChecked() && _lotSerial->number().trimmed().length() == 0)
     || (_pattern->isChecked() && _lotSerialPattern->text().trimmed().length() == 0))
     && (!_item->isValid()) )
  {
    QMessageBox::warning( this, tr("Enter Lot/Serial #"),
                          tr("<p>You must enter a Lot/Serial or Item criteria to print Inventory "
                 "Detail by Lot/Serial #.</p>") );
    _lotSerial->setFocus();
    return false;
  }

  bool valid;
  QVariant param;

  param = params.value("traceId", &valid);
  if (valid)
  {
    int traceid = param.toInt();

    if (traceid)
      trace = "B";
    else
      trace = "F";
  }

  if (_item->isValid())
    params.append("itemid", _item->id());
  params.append("trace", trace);
  if (_selected->isChecked())
  {
    if (_lotSerial->number().trimmed().length() > 0)
      params.append("lotSerial", _lotSerial->number().trimmed());
  }
  else
  {
    params.append("pattern", "t");
    params.append("lotSerial", _lotSerialPattern->text().trimmed());
  }

  return true;
}

