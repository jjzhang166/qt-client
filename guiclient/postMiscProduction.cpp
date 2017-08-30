/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postMiscProduction.h"

#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

#include "distributeInventory.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

postMiscProduction::postMiscProduction(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  _captive = false;
  _controlled = false;
  _itemsiteid = -1;
  _qty = 0;

  _item->setType(ItemLineEdit::cManufactured);
  _qtyToPost->setValidator(omfgThis->qtyVal());
  
  _immediateTransfer->setEnabled(_privileges->check("CreateInterWarehouseTrans"));
    
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs")) 
  {
    _warehouseLit->hide();
    _warehouse->hide();
    _immediateTransfer->hide();
    _transferWarehouse->hide();
  }
  else
    _transferWarehouse->setEnabled(_immediateTransfer->isChecked());

  if (_preferences->boolean("XCheckBox/forgetful"))
    _backflush->setChecked(true);

  _nonPickItems->setEnabled(_backflush->isChecked() &&
			    _privileges->check("ChangeNonPickItems"));

  // TODO: unhide as part of implementation of 5847
  _nonPickItems->hide();
}

postMiscProduction::~postMiscProduction()
{
  // no need to delete child widgets, Qt does it all for us
}

void postMiscProduction::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postMiscProduction::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _warehouse->setEnabled(false);
    _item->setReadOnly(true);
  }

  return NoError;
}

void postMiscProduction::sPost()
{
  XSqlQuery postPost;
  if (!okToPost())
    return;

  if (!createwo())
    return;

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
    deletewo();
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");
  postPost.exec("BEGIN;");
  
  if (!post(itemlocSeries))
  {
    rollback.exec();
    cleanup.exec();
    deletewo();
    return;
  }
  if (!returntool())
  {
    rollback.exec();
    cleanup.exec();
    return;
  }
  if (!closewo())
  {
    rollback.exec();
    cleanup.exec();
    deletewo();
    return;
  }
  if (_immediateTransfer->isChecked())
  {
    if (!transfer(twItemlocSeries))
    {
      rollback.exec();
      cleanup.exec();
      deletewo();
      return;
    }
  }

  postPost.exec("COMMIT;");
  if (_captive)
    accept();
  else
    clear();
}

bool postMiscProduction::okToPost()
{
  _qty = _qtyToPost->toDouble();
  if (_disassembly->isChecked())
    _qty = _qty * -1;

  if (_qty == 0)
  {
    QMessageBox::warning( this, tr("Invalid Quantity"),
                        tr( "The quantity may not be zero." ) );
    return false;
  }

  if (_immediateTransfer->isChecked())
  {
    if (_warehouse->id() == _transferWarehouse->id())
    {
      QMessageBox::warning( this, tr("Cannot Post Immediate Transfer"),
                            tr( "Transaction canceled. Cannot post an immediate transfer for the newly posted production as the\n"
                                "transfer Site is the same as the production Site.  You must manually\n"
                                "transfer the production to the intended Site." ) );
      return false;
    }
  }

  _itemsiteid = 0;
  XSqlQuery itemsite;
  itemsite.prepare( "SELECT itemsite_id, isControlledItemsite(itemsite_id) AS controlled "
                    "FROM itemsite "
                    "WHERE ( (itemsite_item_id=:item_id)"
                    " AND (itemsite_warehous_id=:warehous_id) );" );
  itemsite.bindValue(":item_id", _item->id());
  if (_qty > 0)
    itemsite.bindValue(":warehous_id", _warehouse->id());
  else
    itemsite.bindValue(":warehous_id", _transferWarehouse->id());
  itemsite.exec();
  if (itemsite.first())
  {
    _itemsiteid = itemsite.value("itemsite_id").toInt();
    _controlled = itemsite.value("controlled").toBool();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Work Order"),
                                itemsite, __FILE__, __LINE__))
  {
    return false;
  }
  else
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: Itemsite not found")
                         .arg(windowTitle()),__FILE__,__LINE__);
    return false;
  }

  return true;
}

bool postMiscProduction::createwo()
{
  _woid = 0;
  XSqlQuery wo;
  wo.prepare( "SELECT createWo(fetchWoNumber(), :itemsite_id, :qty, CURRENT_DATE, CURRENT_DATE, :comments) AS result;" );
  wo.bindValue(":itemsite_id", _itemsiteid);
  wo.bindValue(":qty", _qty);
  wo.bindValue(":comments", (tr("Post Misc Production, Document Number ") +
                             _documentNum->text().trimmed() + ", " + _comments->toPlainText()));
  wo.exec();
  if (wo.first())
  {
    _woid = wo.value("result").toInt();
    if (_woid < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Work Order"),
                             storedProcErrorLookup("createWo", _woid),
                             __FILE__, __LINE__);
      return false;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Work Order"),
                                wo, __FILE__, __LINE__))
  {
    return false;
  }

  // Delete any Child W/O's created
  XSqlQuery child;
  child.prepare( "SELECT MAX(deleteWo(wo_id, true)) AS result "
                 "FROM wo "
                 "WHERE ((wo_ordtype='W') AND (wo_ordid=:ordid));");
  child.bindValue(":ordid", _woid);
  child.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Child Work Order"),
                                child, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

bool postMiscProduction::post(int itemlocSeries)
{
  XSqlQuery post;
  post.prepare("SELECT postProduction(:wo_id, :qty, :backflushMaterials, :itemlocSeries, CURRENT_DATE, TRUE) AS result;");
  post.bindValue(":wo_id", _woid);
  post.bindValue(":qty", _qty);
  post.bindValue(":backflushMaterials", QVariant(_backflush->isChecked()));
  post.bindValue(":itemlocSeries", itemlocSeries);
  post.exec();
  if (post.first())
  {
    int result = post.value("result").toInt();
    if (result < 0 || result != itemlocSeries)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production"),
                             storedProcErrorLookup("postProduction", result),
                             __FILE__, __LINE__);
      return false;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production"),
                                post, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

bool postMiscProduction::returntool()
{
  int itemlocseries = 0;
  XSqlQuery post;
  post.prepare("SELECT returnWoMaterial(womatl_id, womatl_qtyiss, CURRENT_DATE) AS result "
               "FROM womatl JOIN itemsite ON (itemsite_id=womatl_itemsite_id) "
               "            JOIN item ON (item_id=itemsite_item_id) "
               "WHERE (womatl_wo_id=:wo_id) "
               "  AND (item_type='T')"
               "  AND (womatl_qtyiss > 0);");
  post.bindValue(":wo_id", _woid);
  post.exec();
  if (post.first())
  {
    itemlocseries = post.value("result").toInt();
    if (itemlocseries < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Returning Work Order Material"),
                             storedProcErrorLookup("returnWoMaterial", itemlocseries),
                             __FILE__, __LINE__);
      return false;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Returning Work Order Material"),
                                post, __FILE__, __LINE__))
  {
    return false;
  }

  // Distribute Inventory
  if (distributeInventory::SeriesAdjust(itemlocseries, this) == XDialog::Rejected)
  {
    QMessageBox::information( this, tr("Post Misc. Production"), tr("Transaction Canceled") );
    return false;
  }
  
  return true;
}

bool postMiscProduction::closewo()
{
  XSqlQuery close;
  close.prepare("SELECT closeWo(:wo_id, true, CURRENT_DATE) AS result;");
  close.bindValue(":wo_id", _woid);
  close.exec();
  if (close.first())
  {
    if (close.value("result").toInt() < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Closing Work Order"),
                             storedProcErrorLookup("closeWo", close.value("result").toInt()),
                             __FILE__, __LINE__);
      return false;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Closing Work Order"),  
                                close, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

bool postMiscProduction::deletewo()
{
  XSqlQuery close;
  close.prepare("SELECT deleteWo(:wo_id, true, true) AS result;");
  close.bindValue(":wo_id", _woid);
  close.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Work Order"),  
                                close, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

bool postMiscProduction::transfer(int itemlocSeries)
{
  XSqlQuery posttransfer;
  posttransfer.prepare("SELECT interWarehouseTransfer( :item_id, :from_warehous_id, :to_warehous_id,"
                       "  :qty, 'W', COALESCE(:documentNumber, formatWoNumber(:wo_id)), 'Transfer from Misc. Production Posting', "
                       "  :itemlocSeries, "
                       "  now(), "
                       "  TRUE, "
                       "  true ) AS result;");
  posttransfer.bindValue(":item_id", _item->id());
  posttransfer.bindValue(":from_warehous_id", _warehouse->id());
  posttransfer.bindValue(":to_warehous_id", _transferWarehouse->id());
  if (_qty < 0)
    posttransfer.bindValue(":qty", _qty * -1);
  else 
    posttransfer.bindValue(":qty", _qty);
  if (_documentNum->text().length() > 0)
    posttransfer.bindValue(":documentNumber", _documentNum->text().trimmed());
  else 
    posttransfer.bindValue(":wo_id", _woid);
  posttransfer.bindValue(":itemlocSeries", itemlocSeries);
  posttransfer.exec();
  if (!posttransfer.first())
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: A System Error occurred at interWarehousTransfer::%2, Item Site ID #%3, Site ID #%4 to Site ID #%5.")
                            .arg(windowTitle())
                            .arg(__LINE__)
                            .arg(_item->id())
                            .arg(_warehouse->id())
                            .arg(_transferWarehouse->id()),__FILE__,__LINE__);
    return false;
  }

  return true;
}

void postMiscProduction::clear()
{
  _item->setId(-1);
  _qtyToPost->clear();
  _documentNum->clear();
  _comments->clear();
  _close->setText(tr("&Close"));

  _item->setFocus();
}

int postMiscProduction::handleSeriesAdjustBeforePost()
{
  XSqlQuery parentItemlocdist;
  bool hasControlledBackflushItems = false;

  // Get series
  int itemlocSeries = distributeInventory::SeriesCreate(0, 0, QString(), QString());
  if (itemlocSeries <= 0)
    return -1;

  if (_backflush->isChecked())
  {
    // Handle creation of itemlocdist records for each eligible backflush item (sql below from postProduction backflush handling)
    XSqlQuery backflushItems;
    backflushItems.prepare("SELECT womatl_id, item_number, womatl_itemsite_id, womatl_qtyreq, womatl_qtyiss, "
                           "  COALESCE(itemuomtouom(item_id, womatl_uom_id, NULL, qty), qty) * -1 AS itemlocdist_qty, qty "
                           "FROM ( "
                           "  SELECT matlitem.item_number, matlitem.item_id, womatl_itemsite_id, womatl_uom_id, womatl_id, womatl_qtyreq, womatl_qtyiss, "
                           "    CASE WHEN :qty > 0 THEN "
                           "      noNeg(((womatl_qtyfxd + ((roundQty(woitem.item_fractional, :qty) + wo_qtyrcv) * womatl_qtyper)) * (1 + womatl_scrap)) - "
                           "        (womatl_qtyiss + "
                           "          CASE WHEN (womatl_qtywipscrap >  ((womatl_qtyfxd + (roundQty(woitem.item_fractional, :qty) + wo_qtyrcv) * womatl_qtyper) * womatl_scrap)) "
                           "          THEN (womatl_qtyfxd + (roundQty(woitem.item_fractional, :qty) + wo_qtyrcv) * womatl_qtyper) * womatl_scrap "
                           "          ELSE womatl_qtywipscrap "
                           "          END)) "
                           "    ELSE (roundQty(woitem.item_fractional, :qty) * womatl_qtyper) "
                           "    END AS qty "
                           "FROM wo "
                           "  JOIN womatl ON wo_id=womatl_wo_id "
                           "  JOIN itemsite AS woitemsite ON wo_itemsite_id=woitemsite.itemsite_id " 
                           "  JOIN item AS woitem ON woitemsite.itemsite_item_id=woitem.item_id " 
                           "  JOIN itemsite AS matlitemsite ON womatl_itemsite_id=matlitemsite.itemsite_id "
                           "  JOIN item AS matlitem ON matlitemsite.itemsite_item_id=matlitem.item_id "
                           "WHERE wo_id = :woId " 
                           "  AND womatl_issuemethod IN ('L', 'M') " 
                           "  AND isControlledItemsite(matlitemsite.itemsite_id) "
                           ") AS data "
                           "ORDER BY womatl_id;");
    backflushItems.bindValue(":woId", _woid);
    backflushItems.bindValue(":qty", _qty);
    backflushItems.exec();
    while (backflushItems.next())
    {
      // If disassembly, check additonal returnWoMaterial.sql condition
      if ((_qty < 0) && (backflushItems.value("womatl_qtyreq").toDouble() >= 0 ? 
            backflushItems.value("womatl_qtyiss").toDouble() < backflushItems.value("itemlocdist_qty").toDouble() : 
            backflushItems.value("womatl_qtyiss").toDouble() > backflushItems.value("itemlocdist_qty").toDouble()))
        continue;

      int result = distributeInventory::SeriesCreate(
        backflushItems.value("womatl_itemsite_id").toInt(),
        backflushItems.value("itemlocdist_qty").toDouble(), "WO", "IM", 0, itemlocSeries);
      if (result != itemlocSeries)
        return -1; // cleanup will occur in ::sPost

      hasControlledBackflushItems = true;
    }
  }

  // Distribute detail
  if (_controlled || hasControlledBackflushItems)
  {
    if (_controlled)
    {
      XSqlQuery parentItemlocdist;
      parentItemlocdist.prepare("SELECT createitemlocdistparent(wo_itemsite_id, "
                                " roundQty(item_fractional, :qty), "
                                " 'WO', wo_id, :itemlocSeries, NULL, NULL, 'RM') AS result "
                                "FROM wo "
                                " JOIN itemsite ON wo_itemsite_id=itemsite_id "
                                " JOIN item ON itemsite_item_id=item_id "
                                "WHERE wo_id=:woId;");
      parentItemlocdist.bindValue(":woId", _woid);
      parentItemlocdist.bindValue(":qty", _qty);
      parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
      parentItemlocdist.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
                                parentItemlocdist, __FILE__, __LINE__))
        return -1;
    }

    if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(), QDate(), true) ==
      XDialog::Rejected)
    {
      QMessageBox::information( this, tr("Post Production"), tr("Detail distribution was cancelled.") );
      return -1;
    }
  }

  return itemlocSeries;
}

int postMiscProduction::handleTransferSeriesAdjustBeforePost()
{
  int toWhItemsiteId = 0;
  bool toWhControlled = false;

  if (!_immediateTransfer->isChecked())
    return -1;

  // Get TO warehouse itemsite and control values
  XSqlQuery toWh;
  toWh.prepare("SELECT itemsite_id, isControlledItemsite(itemsite_id) AS controlled "
               "FROM itemsite "
               "WHERE itemsite_warehous_id = :warehouseId "
               " AND itemsite_item_id = "
               " (SELECT itemsite_item_id FROM itemsite WHERE itemsite_id = :itemsiteId);");
  toWh.bindValue(":itemsiteId", _itemsiteid);
  if (_disassembly->isChecked())
    toWh.bindValue(":warehouseId", _warehouse->id());
  else
    toWh.bindValue(":warehouseId", _transferWarehouse->id());
  toWh.exec();
  if (toWh.first())
  {
    toWhControlled = toWh.value("controlled").toBool();
    toWhItemsiteId = toWh.value("itemsite_id").toInt();
  }
  else
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error finding TO warehouse itemsite"),
      toWh, __FILE__, __LINE__);
    return -1;
  }

  // Generate itemlocSeries, and FROM wh itemlocdist record if controlled
  int twItemlocSeries = distributeInventory::SeriesCreate(_itemsiteid,
      // if assembly, _qty is positive, make it negative because interWarehouseTransfer function will create invhist record with negative qty for From wh
      _assembly->isChecked() ? _qty * -1 : _qty, 
      "W", "TW", _woid);
  if (twItemlocSeries <= 0)
    return -1;

  // Exit now if neither is controlled return twItemlocSeries to use for interWarehouseTransfer
  if (!_controlled && !toWhControlled)
    return twItemlocSeries;

  if (_controlled && toWhControlled)
  {
    int itemlocdistId;
    XSqlQuery itemlocdist;
    itemlocdist.prepare("SELECT itemlocdist_id FROM itemlocdist WHERE itemlocdist_series=:itemlocSeries;");
    itemlocdist.bindValue(":itemlocSeries", twItemlocSeries);
    itemlocdist.exec();
    if (itemlocdist.size() != 1)
    {
      QMessageBox::information(this, tr("Site Transfer"),
                               tr("Error looking up itemlocdist info. Expected 1 record for itemlocdist_series %1").arg(twItemlocSeries) );
      return -1;
    }
    else if (itemlocdist.first())
    {
      itemlocdistId = itemlocdist.value("itemlocdist_id").toInt();
      if (!(itemlocdistId > 0))
      {
        QMessageBox::information(this, tr("Site Transfer"),
                               tr("Error looking up itemlocdist info. Expected itemlocdist_id to be > 0, not %1 for itemlocdist_series %2")
                               .arg(itemlocdistId).arg(twItemlocSeries) );
        return -1;   
      }

      // Create the TO wh itemlocdist record
      int result = distributeInventory::SeriesCreate(toWhItemsiteId,
        // if disassembly, _qty is positive, make it negative because interWarehouseTransfer function will create invhist record with negative qty for TO wh
        _disassembly->isChecked() ? _qty * -1 : _qty, 
        "W", "TW", _woid, twItemlocSeries, itemlocdistId);
      if (result != twItemlocSeries)
        return -1;
    }
    else 
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Finding Itemlocdist Info"),
        itemlocdist, __FILE__, __LINE__);
      return -1;
    }
  }

  // Distribute detail
  if (distributeInventory::SeriesAdjust(twItemlocSeries, this, QString(), QDate(), QDate(), true) ==
      XDialog::Rejected)
  {
    QMessageBox::information( this, tr("Post Production"), tr("Detail distribution was cancelled.") );
    return -1;
  }

  return twItemlocSeries;
}

