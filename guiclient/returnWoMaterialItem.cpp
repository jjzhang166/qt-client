/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "returnWoMaterialItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

returnWoMaterialItem::returnWoMaterialItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_return, SIGNAL(clicked()), this, SLOT(sReturn()));
  connect(_womatl, SIGNAL(newId(int)), this, SLOT(sSetQOH(int)));
  connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQty()));

  _captive = false;
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _qty->setValidator(omfgThis->qtyVal());
  _beforeQty->setPrecision(omfgThis->transQtyVal());
  _afterQty->setPrecision(omfgThis->transQtyVal());

  // Change to Issued only
  //_wo->setType(cWoExploded | cWoReleased | cWoIssued);
  _wo->setType(cWoIssued);
  _wo->setFocus();
}

returnWoMaterialItem::~returnWoMaterialItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void returnWoMaterialItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse returnWoMaterialItem::set(const ParameterList &pParams)
{
  XSqlQuery returnet;
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    returnet.prepare("SELECT womatl_wo_id FROM womatl WHERE (womatl_id=:womatl_id); ");
    returnet.bindValue(":womatl_id", param.toInt());
    returnet.exec();
    if(returnet.first())
    {
      _wo->setId(returnet.value("womatl_wo_id").toInt());
      _wo->setEnabled(false);
    }
   if (valid)
   {
    _womatl->setId(param.toInt());
    _womatl->setEnabled(false);
   }
  }

  return NoError;
}

void returnWoMaterialItem::sReturn()
{
  int itemlocSeries;
  if (!_transDate->isValid())
  {
    QMessageBox::critical(this, tr("Invalid date"),
                          tr("You must enter a valid transaction date.") );
    _transDate->setFocus();
    return;
  }
  else if (!_wo->isValid())
  {
    QMessageBox::critical( this, tr("Select Work Order"),
                           tr("You must select the Work Order from which you with to return Materal") );
    _wo->setFocus();
    return;
  }

  // Stage cleanup function to be called on error
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");
  
  // Get parent series id
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
    return;
  }

  // If controlled item: create the parent itemlocdist record, call distributeInventory::seriesAdjust
  if (_controlledItem)
  {
    XSqlQuery parentItemlocdist;
    parentItemlocdist.prepare("SELECT createitemlocdistparent(:itemsite_id, "
                              " itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, :qty), "
                              " 'WO', womatl_wo_id, :itemlocSeries, NULL, NULL, 'IM') AS result "
                              "FROM womatl "
                              " JOIN itemsite ON womatl_itemsite_id = itemsite_id "
                              "WHERE womatl_id = :womatl_id;");
    parentItemlocdist.bindValue(":itemsite_id", _itemsiteId);
    parentItemlocdist.bindValue(":qty", _qty->toDouble());
    parentItemlocdist.bindValue(":womatl_id", _womatl->id());
    parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
    parentItemlocdist.exec();
    if (parentItemlocdist.first())
    {
      if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(),
        QDate(), true) == XDialog::Rejected)
      {
        cleanup.exec();
        QMessageBox::information(this, tr("Material Return"), 
          tr("Transaction Canceled") );
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
                              parentItemlocdist, __FILE__, __LINE__))
    {
      return;
    }
  }

  // Proceed to posting inventory transaction
  XSqlQuery returnItem;
  returnItem.prepare("SELECT returnWoMaterial(:womatl_id, :qty, :itemlocSeries, :date, false, TRUE) AS result;");
  returnItem.bindValue(":womatl_id", _womatl->id());
  returnItem.bindValue(":qty", _qty->toDouble());
  returnItem.bindValue(":itemlocSeries", itemlocSeries);
  returnItem.bindValue(":date",  _transDate->date());
  returnItem.exec();
  if (returnItem.first())
  {
    int result = returnItem.value("result").toInt();
    if (result < 0 || result != itemlocSeries)
    {
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Information"),
                           storedProcErrorLookup("returnWoMaterial", result),
                           __FILE__, __LINE__);
      return;
    }
  }
  else if (returnItem.lastError().type() != QSqlError::NoError)
  {
    cleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Information"),
                         returnItem, __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
  {
    _qty->clear();
    _close->setText(tr("&Close"));
    _womatl->setWoid(_wo->id());
    _womatl->setFocus();
  }
}

void returnWoMaterialItem::sSetQOH(int pWomatlid)
{
  if (pWomatlid == -1)
  {
    _cachedQOH = 0.0;
    _beforeQty->clear();
    _afterQty->clear();
  }
  else
  {
    if (_wo->method() == "A")
      _qty->setDouble(_womatl->qtyIssued());
    else
    {
      // depending on how the womatl_id is set, sometimes qtyRequired and qtyIssued are ABS
      if (_womatl->qtyRequired() < 0.0)
        _qty->setDouble((_womatl->qtyIssued() - _womatl->qtyRequired()));
      else
        _qty->setDouble((_womatl->qtyRequired() - _womatl->qtyIssued()));
    }
    
    XSqlQuery qoh;
    qoh.prepare( "SELECT itemuomtouom(itemsite_item_id, NULL, womatl_uom_id, "
                 "  qtyAvailable(itemsite_id)) AS availableqoh, uom_name, itemsite_id, "
                 "  isControlledItemsite(itemsite_id) AS controlled "
                 "FROM womatl, itemsite, uom "
                 "WHERE womatl_itemsite_id = itemsite_id "
                 "  AND womatl_uom_id = uom_id "
                 "  AND womatl_id=:womatl_id;" );
    qoh.bindValue(":womatl_id", pWomatlid);
    qoh.exec();
    if (qoh.first())
    {
      _itemsiteId = qoh.value("itemsite_id").toInt();
      _controlledItem = qoh.value("controlled").toBool();
      _uom->setText(qoh.value("uom_name").toString());
      _cachedQOH = qoh.value("availableqoh").toDouble();
      _beforeQty->setDouble(_cachedQOH);
    }
    else
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Work Order Item Information"),
                         qoh, __FILE__, __LINE__);
  }
  sUpdateQty();
}

void returnWoMaterialItem::sUpdateQty()
{
  if (_womatl->isValid())
    _afterQty->setDouble(_cachedQOH + _qty->toDouble());
}

