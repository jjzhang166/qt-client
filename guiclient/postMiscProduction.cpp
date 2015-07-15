/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
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

postMiscProduction::postMiscProduction(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  _captive = false;
  _itemsiteid = -1;
  _sense = 1;
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

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  postPost.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations

  if (_qty > 0)
  {
    if (!createwo())
    {
      rollback.exec();
      return;
    }
    if (!post())
    {
      rollback.exec();
      return;
    }
    if (!returntool())
    {
      rollback.exec();
      return;
    }
    if (!closewo())
    {
      rollback.exec();
      return;
    }
    if (_immediateTransfer->isChecked())
    {
      if (!transfer())
      {
        rollback.exec();
        return;
      }
    }
  }
  else
  {
    _sense = -1;
    if (_immediateTransfer->isChecked())
    {
      if (!transfer())
      {
        rollback.exec();
        return;
      }
    }
    if (!createwo())
    {
      rollback.exec();
      return;
    }
    if (!post())
    {
      rollback.exec();
      return;
    }
    if (!returntool())
    {
      rollback.exec();
      return;
    }
    if (!closewo())
    {
      rollback.exec();
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

  return true;
}

bool postMiscProduction::createwo()
{
  _itemsiteid = 0;
  XSqlQuery itemsite;
  itemsite.prepare( "SELECT itemsite_id "
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
  }
  else if (itemsite.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemsite.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  else
  {
    systemError(this, "Itemsite not found", __FILE__, __LINE__);
    return false;
  }

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
      systemError(this, storedProcErrorLookup("createWo", _woid),
                  __FILE__, __LINE__);
      return false;
    }
  }
  else if (wo.lastError().type() != QSqlError::NoError)
  {
    systemError(this, wo.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  // Delete any Child W/O's created
  XSqlQuery child;
  child.prepare( "SELECT MAX(deleteWo(wo_id, true)) AS result "
                 "FROM wo "
                 "WHERE ((wo_ordtype='W') AND (wo_ordid=:ordid));");
  child.bindValue(":ordid", _woid);
  child.exec();
  if (child.first())
  {
    if (child.value("result").toInt() < 0)
    {
      systemError(this, storedProcErrorLookup("deleteWo", child.value("result").toInt()),
                  __FILE__, __LINE__);
      return false;
    }
  }
  else if (child.lastError().type() != QSqlError::NoError)
  {
    systemError(this, child.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  return true;
}

bool postMiscProduction::post()
{
  int _itemlocseries = 0;
  XSqlQuery post;
  post.prepare("SELECT postProduction(:wo_id, :qty, :backflushMaterials, 0, CURRENT_DATE) AS result;");
  post.bindValue(":wo_id", _woid);
  post.bindValue(":qty", _qty);
  post.bindValue(":backflushMaterials", QVariant(_backflush->isChecked()));
  post.exec();
  if (post.first())
  {
    _itemlocseries = post.value("result").toInt();
    if (_itemlocseries < 0)
    {
      systemError(this, storedProcErrorLookup("postProduction", _itemlocseries),
                  __FILE__, __LINE__);
      return false;
    }
  }
  else if (post.lastError().type() != QSqlError::NoError)
  {
    systemError(this, post.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  // Distribute Inventory
  if (distributeInventory::SeriesAdjust(_itemlocseries, this) == XDialog::Rejected)
  {
    QMessageBox::information( this, tr("Post Misc. Production"), tr("Transaction Canceled") );
    return false;
  }

  return true;
}

bool postMiscProduction::returntool()
{
  int _itemlocseries = 0;
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
    _itemlocseries = post.value("result").toInt();
    if (_itemlocseries < 0)
    {
      systemError(this, storedProcErrorLookup("returnWoMaterial", _itemlocseries),
                  __FILE__, __LINE__);
      return false;
    }
  }
  else if (post.lastError().type() != QSqlError::NoError)
  {
    systemError(this, post.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  
  // Distribute Inventory
  if (distributeInventory::SeriesAdjust(_itemlocseries, this) == XDialog::Rejected)
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
      systemError(this, storedProcErrorLookup("closeWo", close.value("result").toInt()),
                  __FILE__, __LINE__);
      return false;
    }
  }
  else if (close.lastError().type() != QSqlError::NoError)
  {
    systemError(this, close.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  return true;
}

bool postMiscProduction::transfer()
{
  XSqlQuery posttransfer;
  posttransfer.prepare( "SELECT interWarehouseTransfer( :item_id, :from_warehous_id, :to_warehous_id,"
             "                               :qty, 'W', :documentNumber, 'Transfer from Misc. Production Posting' ) AS result;" );
  posttransfer.bindValue(":item_id", _item->id());
  posttransfer.bindValue(":from_warehous_id", _warehouse->id());
  posttransfer.bindValue(":to_warehous_id", _transferWarehouse->id());
  posttransfer.bindValue(":qty", _qty * _sense);
  posttransfer.bindValue(":documentNumber", _documentNum->text().trimmed());
  posttransfer.exec();
  if (posttransfer.first())
  {
    if (distributeInventory::SeriesAdjust(posttransfer.value("result").toInt(), this) == XDialog::Rejected)
    {
      QMessageBox::information( this, tr("Post Misc. Production"), tr("Transaction Canceled") );
      return false;
    }
  }
  else
  {
    systemError( this, tr("A System Error occurred at interWarehousTransfer::%1, Item Site ID #%2, Site ID #%3 to Site ID #%4.")
                       .arg(__LINE__)
                       .arg(_item->id())
                       .arg(_warehouse->id())
                       .arg(_transferWarehouse->id()));
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

