/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "issueWoMaterialItem.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>
#include <QValidator>

#include "inputManager.h"
#include "distributeInventory.h"

issueWoMaterialItem::issueWoMaterialItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_womatl, SIGNAL(newId(int)), this, SLOT(sSetQOH(int)));
  connect(_qtyToIssue, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQOH()));
  connect(_issue, SIGNAL(clicked()), this, SLOT(sIssue()));

  _captive = false;
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));
  omfgThis->inputManager()->notify(cBCItem, this, this, SLOT(sCatchItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, this, SLOT(sCatchItemsiteid(int)));

// Issue #22778 - Add hidden metric to allow issuing to Exploded WOs
  if (_metrics->boolean("IssueToExplodedWO"))
    _wo->setType(cWoExploded | cWoIssued | cWoReleased);
  else
    _wo->setType(cWoIssued | cWoReleased);

  _qtyToIssue->setValidator(omfgThis->qtyVal());
  _beforeQty->setPrecision(omfgThis->transQtyVal());
  _afterQty->setPrecision(omfgThis->transQtyVal());

  _wo->setFocus();
}

issueWoMaterialItem::~issueWoMaterialItem()
{
    // no need to delete child widgets, Qt does it all for us
}

void issueWoMaterialItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse issueWoMaterialItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _captive = true;

    _wo->setId(param.toInt());
    _wo->setEnabled(false);
  }

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    _womatl->setId(param.toInt());
    _womatl->setEnabled(false);
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _qtyToIssue->setDouble(param.toDouble());

  return NoError;
}

void issueWoMaterialItem::sCatchItemid(int pItemid)
{
  XSqlQuery issueCatchItemid;
  if (_wo->isValid())
  {
    issueCatchItemid.prepare( "SELECT womatl_id "
               "FROM womatl, itemsite "
               "WHERE ( (womatl_itemsite_id=itemsite_id)"
               " AND (womatl_wo_id=:wo_id)"
               " AND (itemsite_item_id=:item_id) );" );
    issueCatchItemid.bindValue(":wo_id", _wo->id());
    issueCatchItemid.bindValue(":item_id", pItemid);
    issueCatchItemid.exec();
    if (issueCatchItemid.first())
      _womatl->setId(issueCatchItemid.value("womatl_id").toInt());
    else
      audioReject();
  }
  else
    audioReject();
}

void issueWoMaterialItem::sCatchItemsiteid(int pItemsiteid)
{
  XSqlQuery issueCatchItemsiteid;
  if (_wo->isValid())
  {
    issueCatchItemsiteid.prepare( "SELECT womatl_id "
               "FROM womatl "
               "WHERE ((womatl_itemsite_id=:itemsite_id)"
	       "  AND  (womatl_wo_id=:wo_id));" );
    issueCatchItemsiteid.bindValue(":itemsite_id", pItemsiteid);
    issueCatchItemsiteid.bindValue(":wo_id", _wo->id());
    issueCatchItemsiteid.exec();
    if (issueCatchItemsiteid.first())
      _womatl->setId(issueCatchItemsiteid.value("womatl_id").toInt());
    else
      audioReject();
  }
  else
    audioReject();
}

void issueWoMaterialItem::sIssue()
{
  XSqlQuery issueIssue;
  if (!_transDate->isValid())
  {
    QMessageBox::critical(this, tr("Invalid date"),
                          tr("You must enter a valid transaction date.") );
    _transDate->setFocus();
    return;
  }
  
  issueIssue.prepare("SELECT itemsite_id, item_number, warehous_code, "
            "       (COALESCE((SELECT SUM(itemloc_qty) "
            "                    FROM itemloc "
            "                   WHERE (itemloc_itemsite_id=itemsite_id)), 0.0) >= roundQty(item_fractional, itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, :qty))) AS isqtyavail "
            "  FROM womatl, itemsite, item, whsinfo "
            " WHERE ((womatl_itemsite_id=itemsite_id) "
            "   AND (itemsite_item_id=item_id) "
            "   AND (itemsite_warehous_id=warehous_id) "
            "   AND (NOT ((item_type = 'R') OR (itemsite_controlmethod = 'N'))) "
            "   AND ((itemsite_controlmethod IN ('L', 'S')) OR (itemsite_loccntrl)) "
            "   AND (womatl_id=:womatl_id)); ");
  issueIssue.bindValue(":womatl_id", _womatl->id());
  issueIssue.bindValue(":qty", _qtyToIssue->toDouble());
  issueIssue.exec();
  while(issueIssue.next())
  {
    if(!(issueIssue.value("isqtyavail").toBool()))
    {
      QMessageBox::critical(this, tr("Insufficient Inventory"),
        tr("Item Number %1 in Site %2 is a Multiple Location or\n"
           "Lot/Serial controlled Item which is short on Inventory.\n"
           "This transaction cannot be completed as is. Please make\n"
           "sure there is sufficient Quantity on Hand before proceeding.")
          .arg(issueIssue.value("item_number").toString())
          .arg(issueIssue.value("warehous_code").toString()));
      return;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  issueIssue.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  issueIssue.prepare("SELECT issueWoMaterial(:womatl_id, :qty, true, :date) AS result;");
  issueIssue.bindValue(":womatl_id", _womatl->id());
  issueIssue.bindValue(":qty", _qtyToIssue->toDouble());
  issueIssue.bindValue(":date",  _transDate->date());
  issueIssue.exec();
  if (issueIssue.first())
  {
    if (issueIssue.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at issueWoMaterialItem::%1, Work Order ID #%2, Error #%3.")
                         .arg(__LINE__)
                         .arg(_wo->id())
                         .arg(issueIssue.value("result").toInt()) );
      return;
    }
    else if (distributeInventory::SeriesAdjust(issueIssue.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Material Issue"), tr("Transaction Canceled") );
      return;
    }

    if (_metrics->boolean("LotSerialControl"))
    {
      // Insert special pre-assign records for the lot/serial#
      // so they are available when the material is returned
      XSqlQuery lsdetail;
      lsdetail.prepare("INSERT INTO lsdetail "
                       "            (lsdetail_itemsite_id, lsdetail_created, lsdetail_source_type, "
                       "             lsdetail_source_id, lsdetail_source_number, lsdetail_ls_id, lsdetail_qtytoassign) "
                       "SELECT invhist_itemsite_id, NOW(), 'IM', "
                       "       :orderitemid, invhist_ordnumber, invdetail_ls_id, (invdetail_qty * -1.0) "
                       "FROM invhist JOIN invdetail ON (invdetail_invhist_id=invhist_id) "
                       "WHERE (invhist_series=:itemlocseries)"
                       "  AND (COALESCE(invdetail_ls_id, -1) > 0);");
      lsdetail.bindValue(":orderitemid", _womatl->id());
      lsdetail.bindValue(":itemlocseries", issueIssue.value("result").toInt());
      lsdetail.exec();
      if (lsdetail.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, lsdetail.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }

    issueIssue.exec("COMMIT;");
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at issueWoMaterialItem::%1, Work Order ID #%2.")
                       .arg(__LINE__)
                       .arg(_wo->id()) );
    return;
  }

  if (_captive)
    close();
  else
  {
    _close->setText(tr("Close"));
    _qtyToIssue->clear();
    _womatl->setId(-1);
    _womatl->setWoid(_wo->id());
    _womatl->setFocus();
  }
}

void issueWoMaterialItem::sSetQOH(int pWomatlid)
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
      _qtyToIssue->setDouble(_womatl->qtyRequired() - _womatl->qtyIssued());
    else
      _qtyToIssue->setDouble(_womatl->qtyIssued() * -1);
    
    XSqlQuery qoh;
    qoh.prepare( "SELECT itemuomtouom(itemsite_item_id, NULL, womatl_uom_id, qtyAvailable(itemsite_id)) AS availableqoh,"
                 "       uom_name "
                 "  FROM womatl, itemsite, uom"
                 " WHERE((womatl_itemsite_id=itemsite_id)"
                 "   AND (womatl_uom_id=uom_id)"
                 "   AND (womatl_id=:womatl_id) );" );
    qoh.bindValue(":womatl_id", pWomatlid);
    qoh.exec();
    if (qoh.first())
    {
      _uomQty->setText(qoh.value("uom_name").toString());
      _cachedQOH = qoh.value("availableqoh").toDouble();
      _beforeQty->setDouble(_cachedQOH);
    }
    else
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
  }
  sPopulateQOH();
}

void issueWoMaterialItem::sPopulateQOH()
{
  _afterQty->setDouble(_cachedQOH - _qtyToIssue->toDouble());
}

