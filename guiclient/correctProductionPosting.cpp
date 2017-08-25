/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "correctProductionPosting.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "closeWo.h"
#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

correctProductionPosting::correctProductionPosting(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_correct, SIGNAL(clicked()), this, SLOT(sCorrect()));
  connect(_wo,     SIGNAL(newId(int)), this, SLOT(populate()));

  _captive = false;
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);
  _qtyReceivedCache = 0.0;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoIssued);
  _qty->setValidator(omfgThis->qtyVal());
  _qtyOrdered->setPrecision(omfgThis->transQtyVal());
  _qtyReceived->setPrecision(omfgThis->transQtyVal());
  _qtyBalance->setPrecision(omfgThis->transQtyVal());

  if (_preferences->boolean("XCheckBox/forgetful"))
    _backFlush->setChecked(true);
  _nonPickItems->setEnabled(_backFlush->isChecked() &&
			    _privileges->check("ChangeNonPickItems"));

  // TODO: unhide as part of implementation of 5847
  _nonPickItems->hide();

  adjustSize();
}

correctProductionPosting::~correctProductionPosting()
{
  // no need to delete child widgets, Qt does it all for us
}

void correctProductionPosting::languageChange()
{
  retranslateUi(this);
}

enum SetResponse correctProductionPosting::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _captive = true;

    _wo->setId(param.toInt());
    _wo->setReadOnly(true);
  }

  return NoError;
}

bool correctProductionPosting::okToPost()
{

  QList<GuiErrorCheck>errors;
  errors<<GuiErrorCheck(!_transDate->isValid(), _transDate,
                        tr("You must enter a valid transaction date."))
        <<GuiErrorCheck(_qty->toDouble() > _qtyReceivedCache, _qty,
                        tr("The Quantity to correct must be less than or equal to the Quantity already posted."));

  if(GuiErrorCheck::reportErrors(this,tr("Cannot Post Transaction"),errors))
      return false;


  XSqlQuery itemtypeq;
  itemtypeq.prepare( "SELECT itemsite_costmethod, itemsite_qtyonhand "
             "FROM wo, itemsite "
             "WHERE ( (wo_itemsite_id=itemsite_id)"
             " AND (wo_id=:wo_id) );" );
  itemtypeq.bindValue(":wo_id", _wo->id());
  itemtypeq.exec();
  if (itemtypeq.first())
  {
    if (itemtypeq.value("itemsite_costmethod").toString() == "J")
    {
      QMessageBox::warning(this, tr("Cannot Post Correction"),
                           tr("You may not post a correction to a Work Order for a "
                              "Item Site with the Job cost method. You must, "
                              "instead, adjust shipped quantities."));
      return false;
    }

    if (itemtypeq.value("itemsite_qtyonhand").toDouble() < _qty->toDouble())
    {
      QMessageBox::warning(this, tr("Cannot Post Correction"),
                           tr("You may not post a correction to a Work Order for a "
                              "Item Site with a quantity on hand less than the "
                              "correction quantity."));
      return false;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production Correction"),
                                itemtypeq, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

void correctProductionPosting::clear()
{
  _wo->setId(-1);
  _qty->clear();
  _close->setText(tr("&Close"));
  _wo->setFocus();
}

void correctProductionPosting::sCorrect()
{
  if (! okToPost())
    return;

  // xtmfg calls correctProductionPosting::okToPost() but not ::sPost() && has a situation
  // where qty 0 is OK, so don't move the _qty == 0 check to okToPost()
  if (_qty->toDouble() == 0.0)
  {
    QMessageBox::critical(this, tr("Enter Quantity to Post"),
                          tr("You must enter a quantity of production to Post.") );
    _qty->setFocus();
    return;
  }

  int itemlocSeries = handleSeriesAdjustBeforePost();

  // Stage distribution cleanup function to be called on error
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");
  cleanup.bindValue(":itemlocSeries", itemlocSeries);

  // If the series aren't set properly, cleanup and exit. handleSeriesAdjustBeforePost already displayed the error messages.
  if (itemlocSeries <= 0)
  {
    cleanup.exec();
    return;
  }

  // correctProduction
  XSqlQuery correctCorrect;
  correctCorrect.prepare("SELECT correctProduction(:wo_id, :qty, :backflushMaterials, "
                         ":itemlocSeries, :date, NULL, TRUE) AS result;");
  correctCorrect.bindValue(":wo_id", _wo->id());
  if (_wo->method() == "A")
    correctCorrect.bindValue(":qty", _qty->toDouble());
  else
    correctCorrect.bindValue(":qty", _qty->toDouble() * -1);
  correctCorrect.bindValue(":backflushMaterials",  QVariant(_backFlush->isChecked()));
  correctCorrect.bindValue(":itemlocSeries", itemlocSeries);
  correctCorrect.bindValue(":date",  _transDate->date());
  correctCorrect.exec();
  if (correctCorrect.first())
  {
    omfgThis->sWorkOrdersUpdated(_wo->id(), true);
  }
  else if (correctCorrect.lastError().type() != QSqlError::NoError)
  {
    cleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Production Correction"),
                                    correctCorrect, __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
    clear();
}

void correctProductionPosting::populate()
{
  if (_wo->id() != -1)
  {
    if (_wo->method() == "D")
    {
      _backFlush->setEnabled(false);
      _backFlush->setChecked(false);
      _qtyOrderedLit->setText(tr("Qty. to Disassemble:"));
      _qtyReceivedLit->setText(tr("Qty. Disassembled:"));
    }
    else
    {
      _backFlush->setEnabled(true);
      _qtyOrderedLit->setText(tr("Qty. Ordered:"));
      _qtyReceivedLit->setText(tr("Qty. Received:"));
    }
    
    _qtyOrdered->setText(_wo->qtyOrdered());
    _qtyReceived->setText(_wo->qtyReceived());
    _qtyBalance->setText(_wo->qtyBalance());

    _qtyReceivedCache = _wo->qtyReceived();
  }
  else
  {
    _qtyOrdered->clear();
    _qtyReceived->clear();
    _qtyBalance->clear();

    _qtyReceivedCache = 0;
  }
}

int correctProductionPosting::handleSeriesAdjustBeforePost()
{
  bool hasControlledMaterialItems = false;
  // Get the parent series id
  XSqlQuery postInfo;
  postInfo.prepare("SELECT item_type, roundQty(item_fractional, :qty) AS parent_qty, wo_status, wo_itemsite_id, "
                       "  isControlledItemsite(wo_itemsite_id) AS controlled "
                       "FROM wo "
                       "  JOIN itemsite ON itemsite_id=wo_itemsite_id "                       
                       "  JOIN item ON item_id=itemsite_item_id "
                       "WHERE wo_id=:wo_id;");
  postInfo.bindValue(":wo_id", _wo->id());
  postInfo.bindValue(":qty", _qty->toDouble());
  postInfo.exec();
  if (!postInfo.first())
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Failed to Retrieve WO and Itemsite Info"),
                            postInfo, __FILE__, __LINE__);
    return -1;
  }

  int itemlocSeries = distributeInventory::SeriesCreate(postInfo.value("wo_itemsite_id").toInt(),
    postInfo.value("parent_qty").toDouble() * -1, "WO", "RM", _wo->id());
  if (itemlocSeries <= 0)
    return -1;

  // If backflush materials, createItemlocdistParent for each (for returnWoMaterial() call)
  if (_backFlush->isChecked())
  {
    XSqlQuery backflushMaterials;
    backflushMaterials.prepare("SELECT womatl_id, itemsite_id, qty, womatl_qtyreq, womatl_qtyiss, "
                               "  COALESCE(itemuomtouom(item_id, womatl_uom_id, NULL, qty), qty) AS itemlocdist_qty " // from returnWoMaterial.sql
                               "FROM ( "
                               "  SELECT wo_qtyrcv, womatl_id, womatl_uom_id, itemsite_id, womatl_qtyreq, womatl_qtyiss, item_id, "
                               "    roundQty(item_fractional, "
                               "      (CASE WHEN wo_qtyrcv - :qty > 0 THEN 0 ELSE womatl_qtyfxd END "
                               "        + :qty * womatl_qtyper) "
                               "      * (1 + womatl_scrap)) AS qty " // from correctProduction.sql
                               "FROM wo "
                               "  JOIN womatl ON (womatl_wo_id=wo_id AND womatl_issuemethod='L') "
                               "  JOIN itemsite ON (itemsite_id=womatl_itemsite_id) "
                               "  JOIN item ON (item_id=itemsite_item_id) "
                               "WHERE wo_id=:wo_id "
                               "  AND isControlledItemsite(womatl_itemsite_id) "
                               ") data "
                               "ORDER BY womatl_id;");
    backflushMaterials.bindValue(":wo_id", _wo->id());
    backflushMaterials.bindValue(":qty", postInfo.value("parent_qty").toDouble());
    backflushMaterials.exec();
    while (backflushMaterials.next())
    {
      if (backflushMaterials.value("qty").toDouble() > 0)
      {
        // Check another returnWoMaterial.sql condition
        if (backflushMaterials.value("womatl_qtyreq").toDouble() >= 0 ? 
            backflushMaterials.value("womatl_qtyiss").toDouble() >= backflushMaterials.value("qty").toDouble() : 
            backflushMaterials.value("womatl_qtyiss").toDouble() <= backflushMaterials.value("qty").toDouble())
        {
          int result = distributeInventory::SeriesCreate(backflushMaterials.value("itemsite_id").toInt(),
            backflushMaterials.value("itemlocdist_qty").toDouble(), "WO", "IM", _wo->id(), itemlocSeries);
          if (result != itemlocSeries)
            return -1;

          hasControlledMaterialItems = true;
        }
      }
    }
  }

  // Call distribute inventory if needed
  if ((postInfo.value("controlled").toBool() || hasControlledMaterialItems) && 
    (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(), QDate(), true) == XDialog::Rejected))
  {
    QMessageBox::information( this, tr("Correct Production Posting"), tr("Transaction Canceled") );
    return -1;
  }

  return itemlocSeries;
}
