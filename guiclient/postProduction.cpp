/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postProduction.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "closeWo.h"
#include "distributeInventory.h"
#include "inputManager.h"
#include "scrapWoMaterialFromWIP.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

#define DEBUG false

postProduction::postProduction(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  if (DEBUG)
    qDebug("postProduction()'s _qty is at %p, _post is at %p", _qty, _post);

  connect(_post,  SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_wo,   SIGNAL(newId(int)), this, SLOT(sHandleWoid(int)));

  _captive = false;
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);
  _wo->setType(cWoExploded | cWoReleased | cWoIssued);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, this, SLOT(sCatchWoid(int)));

  _closeWo->setEnabled(_privileges->check("CloseWorkOrders"));

  _qty->setValidator(omfgThis->qtyVal());
  _qtyOrdered->setPrecision(decimalPlaces("qty"));
  _qtyReceived->setPrecision(decimalPlaces("qty"));
  _qtyBalance->setPrecision(decimalPlaces("qty"));

  if (!_metrics->boolean("MultiWhs"))
  {
    _immediateTransfer->hide();
    _transferWarehouse->hide();
  }

  if (_preferences->boolean("XCheckBox/forgetful"))
    _backflush->setChecked(true);

  _nonPickItems->setEnabled(_backflush->isChecked() &&
                            _privileges->check("ChangeNonPickItems"));
  // TODO: unhide as part of implementation of 5847
  _nonPickItems->hide();

  _transferWarehouse->setEnabled(_immediateTransfer->isChecked());

  _controlled = false;
  _itemsiteId = 0;
}

postProduction::~postProduction()
{
  // no need to delete child widgets, Qt does it all for us
}

void postProduction::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postProduction::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setReadOnly(true);
    _qty->setFocus();
  }

  param = pParams.value("backflush", &valid);
  if (valid)
    _backflush->setChecked(param.toBool());

  return NoError;
}

void postProduction::sHandleWoid(int pWoid)
{
  if(!_privileges->check("CloseWorkOrders"))
    _closeWo->setChecked(false);
    
  XSqlQuery itemfrac;
  itemfrac.prepare( "SELECT item_fractional "
                "FROM wo,itemsite,item "
                "WHERE ((wo_id=:wo_id) "
                "AND (wo_itemsite_id=itemsite_id) "
                "AND (itemsite_item_id = item_id)); ");
  itemfrac.bindValue(":wo_id", pWoid);
  itemfrac.exec();
  if (itemfrac.first() && itemfrac.value("item_fractional").toBool() == false)
      _qty->setValidator(new QIntValidator(this));  

  XSqlQuery postHandleWoid;
  if (DEBUG)
    qDebug("postProduction::sHandleWoid(%d) entered with method %s",
           pWoid, qPrintable(_wo->method()));

  if (_wo->method() == "D")
  {
    _qtyOrderedLit->setText(tr("Qty. to Disassemble:"));
    _qtyReceivedLit->setText(tr("Qty. Disassembled:"));
    _backflush->setEnabled(false);
    _backflush->setChecked(false);
  }
  else
  {
    _qtyOrderedLit->setText(tr("Qty. Ordered:"));
    _qtyReceivedLit->setText(tr("Qty. Received:"));

    postHandleWoid.prepare( "SELECT womatl_issuemethod "
              "FROM womatl "
              "WHERE (womatl_wo_id=:womatl_wo_id);" );
    postHandleWoid.bindValue(":womatl_wo_id", pWoid);
    postHandleWoid.exec();
    if (postHandleWoid.first())
    {
      if (postHandleWoid.findFirst("womatl_issuemethod", "L") != -1)
      {
        _backflush->setEnabled(false);
        _backflush->setChecked(true);
      }
      else if (postHandleWoid.findFirst("womatl_issuemethod", "M") != -1)
      {
        _backflush->setEnabled(true);
        _backflush->setChecked(true);
      }
      else
      {
        _backflush->setEnabled(false);
        _backflush->setChecked(false);
      }
    }
  }
}

void postProduction::sReadWorkOrder(int pWoid)
{
  _wo->setId(pWoid);
}

void postProduction::sScrap()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("transDate", _transDate->date());

  scrapWoMaterialFromWIP newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

bool postProduction::okToPost()
{
  if (!_transDate->isValid())
  {
    QMessageBox::critical(this, tr("Invalid date"),
                          tr("You must enter a valid transaction date.") );
    _transDate->setFocus();
    return false;
  }

  XSqlQuery itemsite;
  itemsite.prepare("SELECT wo_itemsite_id, "
                   "  itemsite_costmethod, "
                   "  isControlledItemsite(wo_itemsite_id) AS controlled "
                   "FROM wo "
                   "  JOIN itemsite ON wo_itemsite_id = itemsite_id "
                   "WHERE wo_id = :woId;");
  itemsite.bindValue(":woId", _wo->id());
  itemsite.exec();
  if (itemsite.first())
  {
    _itemsiteId = itemsite.value("wo_itemsite_id").toInt();
    _controlled = itemsite.value("controlled").toBool();
    if (_immediateTransfer->isChecked())
    {
      if (itemsite.value("itemsite_costmethod").toString() == "J")
      {
        QMessageBox::critical( this, tr("Cannot Transfer"),
                          tr("<p>Work Orders Item Sites with Job cost method "
                             "can not be transferred to another warehouse. "
                             "Uncheck the Immediate Transfer to Site box to continue.") );
        return false;
      }
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production"),
    itemsite, __FILE__, __LINE__))
  {
    return false;
  }

  if (_immediateTransfer->isChecked() &&
      _wo->currentWarehouse() == _transferWarehouse->id())
  {
    QMessageBox::critical(this, tr("Same Transfer and Production Sites"),
                          tr("<p>You have selected an Interwarehouse Transfer "
                            "but the Transfer and Production Sites are the "
                            "same. Either choose a different Transfer Site or "
                            "uncheck the Immediate Transfer box."));
    _transferWarehouse->setFocus();
    return false;
  }

  return true;
}

QString postProduction::updateWoAfterPost()
{
  QString result = QString::null;
  if (_productionNotes->toPlainText().trimmed().length())
  {
    XSqlQuery woq;
    woq.prepare( "UPDATE wo "
       "SET wo_prodnotes=(wo_prodnotes || :productionNotes || '\n') "
       "WHERE (wo_id=:wo_id);" );
    woq.bindValue(":productionNotes", _productionNotes->toPlainText().trimmed());
    woq.bindValue(":wo_id", _wo->id());
    woq.exec();
    if (woq.lastError().type() != QSqlError::NoError)
      result = woq.lastError().databaseText();
  }

  if (DEBUG)
    qDebug("postProduction::updateWoAfterPost() returning %s",
           qPrintable(result));
  return result;
}

int postProduction::handleSeriesAdjustBeforePost()
{
  XSqlQuery parentItemlocdist;
  int itemlocSeries;
  bool hasControlledBackflushItems = false;

  // Stage cleanup function to be called on error
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");

  // Series for issueToShipping
  XSqlQuery parentSeries;
  parentSeries.prepare("SELECT NEXTVAL('itemloc_series_seq') AS result;");
  parentSeries.exec();
  if (parentSeries.first() && parentSeries.value("result").toInt() > 0)
  {
    itemlocSeries = parentSeries.value("result").toInt();
    cleanup.bindValue(":itemlocSeries", itemlocSeries);
  }
  else
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Failed to Retrieve the Next itemloc_series_seq"),
      parentSeries, __FILE__, __LINE__);
    return -1;
  }

  // If backflush, createItemlocdistParent for each controlled material item
  if (_backflush->isChecked())
  {
    // Handle creation of itemlocdist records for each eligible backflush item (sql below from postProduction backflush handling)
    XSqlQuery backflushItems;
    backflushItems.prepare(
      "SELECT item.item_number, item.item_fractional, itemsite.itemsite_id, itemsite.itemsite_item_id, "
      " CASE WHEN :qty > 0 THEN " //issueWoMaterial qty = noNeg(expected - consumed)
      "   noNeg(((womatl_qtyfxd + ((roundQty(woitem.item_fractional, :qty) + wo_qtyrcv) * womatl_qtyper)) * (1 + womatl_scrap)) - "
      "   (womatl_qtyiss + "
      "   CASE WHEN (womatl_qtywipscrap >  ((womatl_qtyfxd + (roundQty(woitem.item_fractional, :qty) + wo_qtyrcv) * womatl_qtyper) * womatl_scrap)) "
      "        THEN (womatl_qtyfxd + (roundQty(woitem.item_fractional, :qty) + wo_qtyrcv) * womatl_qtyper) * womatl_scrap "
      "        ELSE womatl_qtywipscrap END)) "
      " ELSE " // returnWoMaterial qty = expected * -1
      "   (roundQty(woitem.item_fractional, :qty) * womatl_qtyper) * -1 " 
      " END AS qty, "
      "	womatl_id, womatl_wo_id, womatl_uom_id, womatl_qtyreq, womatl_qtyiss "
      "FROM womatl, wo "
			"	JOIN itemsite AS woitemsite ON wo_itemsite_id=woitemsite.itemsite_id "
			"	JOIN item AS woitem ON woitemsite.itemsite_item_id=woitem.item_id, "
			"	itemsite, item "
      "WHERE womatl_issuemethod IN ('L', 'M') "
      " AND womatl_wo_id=wo_id "
      " AND womatl_itemsite_id=itemsite.itemsite_id "
      " AND wo_id = :wo_id "
      " AND itemsite.itemsite_item_id=item.item_id "
      " AND isControlledItemsite(itemsite.itemsite_id) "
      "ORDER BY womatl_id;");
    backflushItems.bindValue(":wo_id", _wo->id());
    if (_wo->method() == "A")
      backflushItems.bindValue(":qty", _qty->toDouble());
    else
      backflushItems.bindValue(":qty", _qty->toDouble() * -1);
    backflushItems.exec();
    while (backflushItems.next())
    {
      XSqlQuery womatlItemlocdist;
      // create the itemlocdist record for this controlled issueWoMaterial item
      if (_wo->method() == "A" && _qty->toDouble() > 0)
      {
        // make sure the backflush item has relevant qty for issueWoMaterial
        if (backflushItems.value("qty").toDouble() > 0)
        {
          hasControlledBackflushItems = true;
          womatlItemlocdist.prepare("SELECT createItemlocdistParent(:itemsite_id, COALESCE(itemuomtouom(:item_id, :womatl_uom_id, NULL, :qty), :qty) * -1, 'WO', :wo_id, "
                                    " :itemlocSeries, NULL, NULL, 'IM') AS result;");
          womatlItemlocdist.bindValue(":itemsite_id", backflushItems.value("itemsite_id").toInt());
          womatlItemlocdist.bindValue(":item_id", backflushItems.value("itemsite_item_id").toInt());
          womatlItemlocdist.bindValue(":womatl_uom_id", backflushItems.value("womatl_uom_id").toInt());
          womatlItemlocdist.bindValue(":item_fractional", backflushItems.value("item_fractional").toBool());
          womatlItemlocdist.bindValue(":wo_id", _wo->id());
          womatlItemlocdist.bindValue(":qty", backflushItems.value("qty").toDouble());
          womatlItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
          womatlItemlocdist.exec();
          if (!womatlItemlocdist.first())
          {
            cleanup.exec();
            QMessageBox::information( this, tr("Issue Line to Shipping"), 
              tr("Failed to Create an itemlocdist record for work order backflushed material item %1.")
              .arg(backflushItems.value("item_number").toString()) );
            return -1;
          }
          else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
            womatlItemlocdist, __FILE__, __LINE__))
          {
            cleanup.exec();
            return -1;
          }
        }
        
      }
      // create the itemlocdist record for this controlled returnWoMaterial item
      else if (_wo->method() == "D" || _qty->toDouble() < 0)
      {
        // make sure the backflush item has relevant qty for returnWoMaterial
        if (backflushItems.value("womatl_qtyreq").toDouble() >= 0 ? 
            backflushItems.value("womatl_qtyiss").toDouble() >= backflushItems.value("qty").toDouble() : 
            backflushItems.value("womatl_qtyiss").toDouble() <= backflushItems.value("qty").toDouble())
        {
          hasControlledBackflushItems = true;
          womatlItemlocdist.prepare("SELECT createItemlocdistParent(:itemsite_id, COALESCE(itemuomtouom(:item_id, :womatl_uom_id, NULL, :qty), :qty), "
          													"  'WO', :wo_id, :itemlocSeries, NULL, NULL, 'IM') AS result;");
          womatlItemlocdist.bindValue(":itemsite_id", backflushItems.value("itemsite_id").toInt());
          womatlItemlocdist.bindValue(":item_id", backflushItems.value("itemsite_item_id").toInt());
          womatlItemlocdist.bindValue(":womatl_uom_id", backflushItems.value("womatl_uom_id").toInt());
          womatlItemlocdist.bindValue(":qty", backflushItems.value("qty").toDouble());
          womatlItemlocdist.bindValue(":wo_id", _wo->id());
          womatlItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
          womatlItemlocdist.exec();
       		if (!womatlItemlocdist.first())
          {
            cleanup.exec();
            QMessageBox::information( this, tr("Issue Line to Shipping"), 
              tr("Failed to Create an itemlocdist record for work order backflushed material item %1.")
              .arg(backflushItems.value("item_number").toString()) );
            return -1;
          }
          else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
            womatlItemlocdist, __FILE__, __LINE__))
          {
            cleanup.exec();
            return -1;
          }
        }
      }
    }
  } // backflush handling

  // If controlled item, createItemlocdistParent
  if (_controlled)
  {
    // create the RM itemlocdist record
    parentItemlocdist.prepare("SELECT createItemlocdistParent(wo_itemsite_id, roundQty(item_fractional, :qty), 'WO', :orderitemId, "
      												"	:itemlocSeries, NULL, NULL, 'RM') AS result "
      												"FROM wo "
      												"	JOIN itemsite ON wo_itemsite_id=itemsite_id "
      												"	JOIN item ON itemsite_item_id=item_id "
      												"WHERE wo_id=:wo_id;");
    parentItemlocdist.bindValue(":wo_id", _wo->id());
    if (_wo->method() == "A")
      parentItemlocdist.bindValue(":qty", _qty->toDouble());
    else
      parentItemlocdist.bindValue(":qty", _qty->toDouble() * -1);
    parentItemlocdist.bindValue(":orderitemId", _wo->id());
    parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
    parentItemlocdist.exec();
    if (parentItemlocdist.lastError().type() != QSqlError::NoError)
    {
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist record for post "
        "production controlled item"), parentItemlocdist, __FILE__, __LINE__);
      return -1;
    }
  }

  // Distribute detail
  if ((_controlled || hasControlledBackflushItems) && 
      (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(), QDate(), true) ==
      XDialog::Rejected))
  {
    cleanup.exec();
    QMessageBox::information( this, tr("Post Production"), tr("Detail distribution was cancelled.") );
    return -1;
  }

  return itemlocSeries;
}

int postProduction::handleTransferSeriesAdjustBeforePost()
{
  int twItemlocSeries = 0;
  int toWhItemsiteId = 0;
  int fromWhItemlocdistId = 0;
  bool toWhControlled = false;

  if (!_immediateTransfer->isChecked())
    return -1;

  // Stage cleanup function to be called on error
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");

  XSqlQuery parentSeries;
  parentSeries.prepare("SELECT NEXTVAL('itemloc_series_seq') AS result;");
  parentSeries.exec();
  if (parentSeries.first() && parentSeries.value("result").toInt() > 0)
  {
    twItemlocSeries = parentSeries.value("result").toInt();
    cleanup.bindValue(":itemlocSeries", twItemlocSeries);
  }
  else 
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Failed to Retrieve the Next itemloc_series_seq"),
      parentSeries, __FILE__, __LINE__);
    return -1;
  }

  // Get TO warehouse itemsite and control values
  XSqlQuery toWh;
  toWh.prepare("SELECT itemsite_id, isControlledItemsite(itemsite_id) AS controlled "
  						 "FROM itemsite "
               "WHERE itemsite_warehous_id = :warehouseId "
               " AND itemsite_item_id = "
               " (SELECT itemsite_item_id FROM itemsite WHERE itemsite_id = :itemsiteId);");
  toWh.bindValue(":warehouseId",  _transferWarehouse->id());
  toWh.bindValue(":itemsiteId", _itemsiteId);
  toWh.exec();
  if (toWh.first())
  {
  	toWhControlled = toWh.value("controlled").toBool();
  	toWhItemsiteId = toWh.value("itemsite_id").toInt();
  }
  else
  {
  	cleanup.exec();
  	ErrorReporter::error(QtCriticalMsg, this, tr("Error finding TO warehouse itemsite"),
      toWh, __FILE__, __LINE__);
    return -1;
  }

  if (_controlled)
  {
    XSqlQuery fromWhItemlocdist;
    fromWhItemlocdist.prepare("SELECT createItemlocdistParent(:itemsiteId, :qty * -1, 'W', :orderitemId, "
                              "   :itemlocSeries, NULL, NULL, 'TW') AS result;");
    fromWhItemlocdist.bindValue(":itemsiteId", _itemsiteId);
    fromWhItemlocdist.bindValue(":qty", _qty->toDouble());
    fromWhItemlocdist.bindValue(":orderitemId", _wo->id());
    fromWhItemlocdist.bindValue(":itemlocSeries", twItemlocSeries);
    fromWhItemlocdist.exec();
    if (fromWhItemlocdist.first())
      fromWhItemlocdistId = fromWhItemlocdist.value("result").toInt();
    else
    {
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist record for FROM Warehouse for interwarehousetransfer "),
        fromWhItemlocdist, __FILE__, __LINE__);
      return -1;
    }
  }
  
  if (toWhControlled)
  {
    XSqlQuery toWhItemlocdist;
    toWhItemlocdist.prepare("SELECT createItemlocdistParent(:itemsite_id, :qty, 'W', :orderitemId, "
                            "   :itemlocSeries, NULL, :itemlocdistId, 'TW') AS result;");
    toWhItemlocdist.bindValue(":qty", _qty->toDouble());
    toWhItemlocdist.bindValue(":orderitemId", _wo->id());
    toWhItemlocdist.bindValue(":itemlocSeries", twItemlocSeries);
    toWhItemlocdist.bindValue(":itemsiteId", toWhItemsiteId);
    if (fromWhItemlocdistId > 0)
      toWhItemlocdist.bindValue(":itemlocdistId", fromWhItemlocdistId);
    toWhItemlocdist.exec();
    if (!toWhItemlocdist.first() || toWhItemlocdist.lastError().type() != QSqlError::NoError)
    {
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist record For TO Warehouse for interwarehousetransfer"),
          toWhItemlocdist, __FILE__, __LINE__);
      return -1;
    }
  }

  // Distribute detail
  if ((_controlled || toWhControlled) && distributeInventory::SeriesAdjust(twItemlocSeries, this, QString(), QDate(), QDate(), true) ==
      XDialog::Rejected)
  {
    cleanup.exec();
    QMessageBox::information( this, tr("Post Production"), tr("Detail distribution was cancelled.") );
    return -1;
  }

  return twItemlocSeries;
}

QString postProduction::handleTransferAfterPost(int itemlocSeries)
{
  QString result = QString::null;
  if (_immediateTransfer->isChecked())
  {
    if (_wo->currentWarehouse() == _transferWarehouse->id())
      result = tr("<p>Cannot post an immediate transfer for the newly posted "
                  "production as the transfer Site is the same as the "
                  "production Site. You must manually transfer the "
                  "production to the intended Site.");
    else if (itemlocSeries <= 0)
      result = tr("Failed to post an immediate transfer. Itemloc");
    else
    {
      XSqlQuery xferq;
      xferq.prepare("SELECT interWarehouseTransfer(itemsite_item_id, "
                    "    itemsite_warehous_id, :to_warehous_id, :qty,"
                    "    'W', formatWoNumber(wo_id),"
                    "    'Immediate Transfer from Production Posting', "
                    "    :itemlocSeries, "
                    "    now(), "
                    "    TRUE, "
                    "    TRUE) AS result "
                    "  FROM wo, itemsite "
                    " WHERE ( (wo_itemsite_id=itemsite_id)"
                    "     AND (wo_id=:wo_id) );" );
      xferq.bindValue(":wo_id", _wo->id());
      xferq.bindValue(":to_warehous_id", _transferWarehouse->id());
      xferq.bindValue(":qty", _qty->toDouble());
      xferq.bindValue(":itemlocSeries", itemlocSeries);
      xferq.exec();
      if (xferq.lastError().type() != QSqlError::NoError)
        result = xferq.lastError().databaseText();
    }
  }

  if (DEBUG)
    qDebug("postProduction::handleTransferAfterPost() returning %s",
           qPrintable(result));
  return result;
}

QString postProduction::handleIssueToParentAfterPost(int itemlocSeries)
{
  QString result = QString::null;
  XSqlQuery issueq;
  
  // Find invhist_id.  May not be found if control method is 'None'
  int invhistid = -1;
  issueq.prepare("SELECT invhist_id "
                 "FROM invhist "
                 "WHERE (invhist_series=:itemlocseries)"
                 "  AND (invhist_transtype='RM');");
  issueq.bindValue(":itemlocseries", itemlocSeries);
  issueq.exec();
  if (issueq.first())
  {
    invhistid = issueq.value("invhist_id").toInt();
  }
  else if (issueq.lastError().type() != QSqlError::NoError)
    result = issueq.lastError().databaseText();

  // If this is a child W/O and the originating womatl
  // is auto issue then issue this receipt to the parent W/O
  issueq.prepare("SELECT issueWoMaterial(womatl_id,"
                 "       roundQty(item_fractional, itemuomtouom(itemsite_item_id, NULL, womatl_uom_id, :qty)),"
                 "       :itemlocseries, :date, :invhist_id::INTEGER, NULL, TRUE, FALSE) AS result "
                 "FROM wo, womatl, itemsite, item "
                 "WHERE (wo_id=:wo_id)"
                 "  AND (womatl_id=wo_womatl_id)"
                 "  AND (womatl_issuewo)"
                 "  AND (itemsite_id=womatl_itemsite_id)"
                 "  AND (item_id=itemsite_item_id);");
  issueq.bindValue(":itemlocseries", itemlocSeries);
  issueq.bindValue(":wo_id", _wo->id());
  issueq.bindValue(":qty", _qty->toDouble());
  issueq.bindValue(":date",  _transDate->date());
  if (invhistid > 0)
    issueq.bindValue(":invhist_id", invhistid);
  issueq.exec();
  if (issueq.first())
  {
    if (issueq.value("result").toInt() < 0)
      result = "issueWoMaterial failed";
    else
    {
      issueq.prepare("SELECT postItemLocSeries(:itemlocseries);");
      issueq.bindValue(":itemlocseries", itemlocSeries);
      issueq.exec();
      if (issueq.lastError().type() != QSqlError::NoError)
        result = issueq.lastError().databaseText();
    }
  }
  else if (issueq.lastError().type() != QSqlError::NoError)
    result = issueq.lastError().databaseText();

  // If this is a W/O for a Job Cost item and the parent is a S/O
  // then issue this receipt to the S/O
  issueq.prepare("SELECT issueToShipping('SO', coitem_id,"
                 "       roundQty(item_fractional, itemuomtouom(itemsite_item_id, NULL, coitem_qty_uom_id, :qty)),"
                 "       :itemlocseries, :date, :invhist_id, false, true) AS result "
                 "FROM wo, itemsite, item, coitem "
                 "WHERE (wo_id=:wo_id)"
                 "  AND (wo_ordtype='S')"
                 "  AND (itemsite_id=wo_itemsite_id)"
                 "  AND (itemsite_costmethod='J')"
                 "  AND (item_id=itemsite_item_id)"
                 "  AND (coitem_id=wo_ordid);");
  issueq.bindValue(":itemlocseries", itemlocSeries);
  issueq.bindValue(":wo_id", _wo->id());
  issueq.bindValue(":qty", _qty->toDouble());
  issueq.bindValue(":date",  _transDate->date());
  if (invhistid > 0)
    issueq.bindValue(":invhist_id", invhistid);
  issueq.exec();
  if (issueq.first())
  {
    if (issueq.value("result").toInt() < 0)
      result = "issueToShipping failed";
    else
    {
      issueq.prepare("SELECT postItemLocSeries(:itemlocseries);");
      issueq.bindValue(":itemlocseries", itemlocSeries);
      issueq.exec();
      if (issueq.lastError().type() != QSqlError::NoError)
        result = issueq.lastError().databaseText();
    }
  }
  else if (issueq.lastError().type() != QSqlError::NoError)
    result = issueq.lastError().databaseText();

  if (DEBUG)
    qDebug("postProduction::handleIssueToParentAfterPost() returning %s",
           qPrintable(result));
  return result;
}

void postProduction::sPost()
{
  XSqlQuery postPost;
  if (! okToPost())
    return;

  // xtmfg calls postProduction::okToPost() but not ::sPost() && has a situation
  // where qty 0 is OK, so don't move the _qty == 0 check to okToPost()
  if (_qty->toDouble() == 0.0)
  {
    QMessageBox::critical(this, tr("Enter Quantity to Post"),
                          tr("You must enter a quantity of production to Post.") );
    _qty->setFocus();
    return;
  }


  int itemlocSeries = handleSeriesAdjustBeforePost();
  int twItemlocSeries = _immediateTransfer->isChecked() ? handleTransferSeriesAdjustBeforePost() : 0;

  // Stage cleanup function to be called on error
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE), "
                  " CASE WHEN :twItemlocSeries IS NOT NULL THEN deleteitemlocseries(:twItemlocSeries, true) END;");
  cleanup.bindValue(":itemlocSeries", itemlocSeries);
  if (twItemlocSeries > 0)
    cleanup.bindValue(":twItemlocSeries", twItemlocSeries);

  // If the series aren't set properly, cleanup and exit. The methods that set them already displayed the error messages.
  if (itemlocSeries <= 0 || (_immediateTransfer->isChecked() && twItemlocSeries <= 0))
  {
    cleanup.exec();
    return;
  }

  // postProduction 
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");
  postPost.exec("BEGIN;");
  postPost.prepare("SELECT postProduction(:wo_id, :qty, :backflushMaterials, :itemlocSeries, :date, TRUE) AS result;");
  postPost.bindValue(":wo_id", _wo->id());
  if (_wo->method() == "A")
    postPost.bindValue(":qty", _qty->toDouble());
  else
    postPost.bindValue(":qty", _qty->toDouble() * -1);
  postPost.bindValue(":backflushMaterials", QVariant(_backflush->isChecked()));
  postPost.bindValue(":itemlocSeries", itemlocSeries);
  postPost.bindValue(":date",  _transDate->date() == QDate::currentDate()
                               ? QDateTime::currentDateTime() : QDateTime(_transDate->date()));
  postPost.exec();
  if (postPost.first())
  {
    int result = postPost.value("result").toInt();

    if (result < 0 || result != itemlocSeries)
    {
      rollback.exec();
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production"),
                             storedProcErrorLookup("postProduction", result),
                             __FILE__, __LINE__);
      return;
    }

    QString errmsg = updateWoAfterPost();
    if (! errmsg.isEmpty())
    {
      rollback.exec();
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                           tr("%1: %2")
                           .arg(windowTitle())
                           .arg(errmsg),__FILE__,__LINE__);
      return;
    }

    errmsg = handleIssueToParentAfterPost(itemlocSeries);
    if (! errmsg.isEmpty())
    {
      rollback.exec();
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                           tr("%1: %2")
                           .arg(windowTitle())
                           .arg(errmsg),__FILE__,__LINE__);
      return;
    }

    errmsg = handleTransferAfterPost(twItemlocSeries);
    if (! errmsg.isEmpty())
    {
      rollback.exec();
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                           tr("%1: %2")
                           .arg(windowTitle())
                           .arg(errmsg),__FILE__,__LINE__);
      return;
    }

    postPost.exec("COMMIT;");

    omfgThis->sWorkOrdersUpdated(_wo->id(), true);

    if (_scrap->isChecked())
      sScrap();
    
    if (_closeWo->isChecked())
    {
      ParameterList params;
      params.append("wo_id", _wo->id());
      params.append("transDate", _transDate->date());

      closeWo newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
    }
  }
  else if (postPost.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    cleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production"),
                         postPost, __FILE__, __LINE__);
    return;
  }

  if (_captive)
    done(itemlocSeries);
  else
    clear();
}

void postProduction::sCatchWoid(int pWoid)
{
  _wo->setId(pWoid);
  _qty->setFocus();
}

void postProduction::clear()
{
  _wo->setId(-1);
  _qty->clear();
  _productionNotes->clear();
  _immediateTransfer->setChecked(false);
  _closeWo->setChecked(false);
  _close->setText(tr("&Close"));

  _wo->setFocus();
}

