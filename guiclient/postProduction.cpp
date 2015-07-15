/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
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
/*
  XSqlQuery type;
  type.prepare( "SELECT itemsite_costmethod "
                "FROM itemsite,wo "
                "WHERE ((wo_id=:wo_id) "
                "AND (wo_itemsite_id=itemsite_id)); ");
  type.bindValue(":wo_id", _wo->id());
  type.exec();
  if (type.first() && type.value("itemsite_costmethod").toString() == "J")
  {
    QMessageBox::critical( this, tr("Invalid Work Order"),
                          tr("<p>Work Orders Item Sites with the Job cost method "
                             "are posted when shipping the Sales Order they are "
                             "associated with.") );
    return false;
  }
  else if (type.lastError().type() != QSqlError::NoError)
  {
    systemError(this, type.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
*/
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

QString postProduction::handleSeriesAdjustAfterPost(int itemlocSeries)
{
  QString result = QString::null;
  if (distributeInventory::SeriesAdjust(itemlocSeries, this) == XDialog::Rejected)
    result = tr("Transaction Canceled");

  if (DEBUG)
    qDebug("postProduction::handleSeriesAdjustAfterPost() returning %s",
           qPrintable(result));
  return result;
}

QString postProduction::handleTransferAfterPost()
{
  QString result = QString::null;
  if (_immediateTransfer->isChecked())
  {
    if (_wo->currentWarehouse() == _transferWarehouse->id())
      result = tr("<p>Cannot post an immediate transfer for the newly posted "
                  "production as the transfer Site is the same as the "
                  "production Site. You must manually transfer the "
                  "production to the intended Site.");
    else
    {
      XSqlQuery xferq;
      xferq.prepare("SELECT interWarehouseTransfer(itemsite_item_id, "
                    "    itemsite_warehous_id, :to_warehous_id, :qty,"
                    "    'W', formatWoNumber(wo_id),"
                    "   'Immediate Transfer from Production Posting') AS result"
                    "  FROM wo, itemsite "
                    " WHERE ( (wo_itemsite_id=itemsite_id)"
                    "     AND (wo_id=:wo_id) );" );
      xferq.bindValue(":wo_id", _wo->id());
      xferq.bindValue(":to_warehous_id", _transferWarehouse->id());
      xferq.bindValue(":qty", _qty->toDouble());
      xferq.exec();
      if (xferq.first() &&
          distributeInventory::SeriesAdjust(xferq.value("result").toInt(),
                                            this) == XDialog::Rejected)
          result = tr("Transaction Canceled");
      else if (xferq.lastError().type() != QSqlError::NoError)
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

  // If this is a child W/O and the originating womatl
  // is auto issue then issue this receipt to the parent W/O
  issueq.prepare("SELECT issueWoMaterial(womatl_id, :qty,"
                 "       :itemlocseries, NOW(), invhist_id ) AS result "
                 "FROM wo, womatl, invhist "
                 "WHERE (wo_id=:wo_id)"
                 "  AND (womatl_id=wo_womatl_id)"
                 "  AND (womatl_issuewo)"
                 "  AND (invhist_series=:itemlocseries)"
                 "  AND (invhist_transtype='RM');");
  issueq.bindValue(":itemlocseries", itemlocSeries);
  issueq.bindValue(":wo_id", _wo->id());
  issueq.bindValue(":qty", _qty->toDouble());
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
  issueq.prepare("SELECT issueToShipping('SO', coitem_id, :qty,"
                 "       :itemlocseries, NOW(), invhist_id) AS result "
                 "FROM wo, itemsite, coitem, invhist "
                 "WHERE (wo_id=:wo_id)"
                 "  AND (wo_ordtype='S')"
                 "  AND (itemsite_id=wo_itemsite_id)"
                 "  AND (itemsite_costmethod='J')"
                 "  AND (coitem_id=wo_ordid)"
                 "  AND (invhist_series=:itemlocseries)"
                 "  AND (invhist_transtype='RM');");
  issueq.bindValue(":itemlocseries", itemlocSeries);
  issueq.bindValue(":wo_id", _wo->id());
  issueq.bindValue(":qty", _qty->toDouble());
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

  int itemlocSeries = 0;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  postPost.exec("BEGIN;");	// handle cancel of lot, serial, or loc distributions
  postPost.prepare("SELECT postProduction(:wo_id, :qty, :backflushMaterials, 0, :date) AS result;");
  postPost.bindValue(":wo_id", _wo->id());
  if (_wo->method() == "A")
    postPost.bindValue(":qty", _qty->toDouble());
  else
    postPost.bindValue(":qty", _qty->toDouble() * -1);
  postPost.bindValue(":backflushMaterials", QVariant(_backflush->isChecked()));
  postPost.bindValue(":date",  _transDate->date());
  postPost.exec();
  if (postPost.first())
  {
    itemlocSeries = postPost.value("result").toInt();

    if (itemlocSeries < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("postProduction", itemlocSeries),
                  __FILE__, __LINE__);
      return;
    }

    QString errmsg = updateWoAfterPost();
    if (! errmsg.isEmpty())
    {
      rollback.exec();
      systemError(this, errmsg, __FILE__, __LINE__);
      return;
    }

    errmsg = handleSeriesAdjustAfterPost(itemlocSeries);
    if (! errmsg.isEmpty())
    {
      rollback.exec();
      QMessageBox::information(this, tr("Post Production"), errmsg);
      return;
    }

    errmsg = handleIssueToParentAfterPost(itemlocSeries);
    if (! errmsg.isEmpty())
    {
      rollback.exec();
      systemError(this, errmsg, __FILE__, __LINE__);
      return;
    }

    errmsg = handleTransferAfterPost();
    if (! errmsg.isEmpty())
    {
      rollback.exec();
      systemError(this, errmsg, __FILE__, __LINE__);
      return;
    }

    postPost.exec("COMMIT;");

    omfgThis->sWorkOrdersUpdated(_wo->id(), true);

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
    systemError(this, postPost.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_scrap->isChecked())
    sScrap();

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

