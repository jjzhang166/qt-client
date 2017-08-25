/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scrapWoMaterialFromWIP.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include "inputManager.h"
#include "distributeInventory.h"
#include "postProduction.h"
#include "returnWoMaterialItem.h"
#include "errorReporter.h"

scrapWoMaterialFromWIP::scrapWoMaterialFromWIP(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleButtons()));
  connect(_scrap,              SIGNAL(clicked()), this, SLOT(sScrap()));
  connect(_scrapComponent, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_scrapTopLevel,  SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_topLevelQty, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleButtons()));
  connect(_topLevelQty, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleButtons()));
  connect(_wo,     SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_womatl, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));

  _captive = false;
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);
  _fromWOTC = false;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoIssued);
  _qty->setValidator(omfgThis->qtyVal());
  _topLevelQty->setValidator(omfgThis->qtyVal());
  _qtyScrappedFromWIP->setPrecision(omfgThis->qtyVal());

  adjustSize();
  _wo->setFocus();
}

scrapWoMaterialFromWIP::~scrapWoMaterialFromWIP()
{
  // no need to delete child widgets, Qt does it all for us
}

void scrapWoMaterialFromWIP::languageChange()
{
  retranslateUi(this);
}

enum SetResponse scrapWoMaterialFromWIP::set(const ParameterList &pParams)
{
  XSqlQuery scrapet;
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    _captive = true;

    scrapet.prepare("SELECT womatl_wo_id FROM womatl WHERE (womatl_id=:womatl_id); ");
    scrapet.bindValue(":womatl_id", param.toInt());
    scrapet.exec();
    if(scrapet.first())
    {
      _wo->setId(scrapet.value("womatl_wo_id").toInt());
      _wo->setEnabled(false);
    }
    _womatl->setId(param.toInt());
    _womatl->setEnabled(false);
  }

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setEnabled(false);
  }

  param = pParams.value("allowTopLevel", &valid);
  if (valid)
    _scrapTopLevel->setEnabled(param.toBool());

  param = pParams.value("fromWOTC", &valid);
  if (valid)
    _fromWOTC = true;

  param = pParams.value("wooper_id", &valid);
  if (valid)
    _womatl->setWooperid(param.toInt());
    
  param = pParams.value("transDate", &valid);
  if (valid)
    _transDate->setDate(param.toDate());

  param = pParams.value("canPostProd", &valid);
  if (valid)
  {
    connect(_scrapTopLevel, SIGNAL(toggled(bool)), _prodPosted, SLOT(setEnabled(bool)));
    _prodPosted->setForgetful(false);
  }

  return NoError;
}

void scrapWoMaterialFromWIP::sScrap()
{
  XSqlQuery scrapScrap;
  if (!_transDate->isValid())
  {
    QMessageBox::critical(this, tr("Invalid date"),
                          tr("You must enter a valid transaction date.") );
    _transDate->setFocus();
    return;
  }
  
  if (_scrapComponent->isChecked() && _qty->toDouble() <= 0)
  {
    QMessageBox::critical( this, tr("Cannot Scrap from WIP"),
                           tr("You must enter a quantity of the selected W/O Material to Scrap." ) );
    _qty->setFocus();
    return;
  }
  else if (_scrapComponent->isChecked() && _qty->toDouble() > _womatl->qtyIssued())
  {
    QMessageBox::critical( this, tr("Cannot Scrap from WIP"),
                           tr("The component quantity to scrap must be less than or equal to quantity issued." ) );
    _qty->setFocus();
    return;
  }
  else if (_scrapTopLevel->isChecked() && _topLevelQty->toDouble() <= 0)
  {
    QMessageBox::critical( this, tr("Cannot Scrap from WIP"),
                           tr("You must enter a quantity of the W/O Top Level Item to Scrap." ) );
    _topLevelQty->setFocus();
    return;
  }

  int itemlocSeries = 0;
  int invhistid = 0;

  // Stage cleanup query
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");

  if (_scrapComponent->isChecked())
  {
    scrapScrap.prepare("SELECT scrapWoMaterial(:womatl_id, :qty, :date) AS result;");
    scrapScrap.bindValue(":womatl_id", _womatl->id());
    scrapScrap.bindValue(":qty", _qty->toDouble());
    scrapScrap.bindValue(":date",  _transDate->date());
  }
  else if (_scrapTopLevel->isChecked())
  {
    XSqlQuery itemsite;
    itemsite.prepare("SELECT wo_itemsite_id, wo_prj_id, itemsite_costmethod "
                     "FROM wo "
                     "WHERE wo_id=:woId;");
    itemsite.bindValue(":woId", _wo->id());
    itemsite.exec();
    if (!itemsite.first())
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error checking balance"),
        itemsite.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (itemsite.value("itemsite_costmethod").toString() == tr("J"))
    {
      QMessageBox::information( this, tr("Scrap Work Order Material"), tr("This item is Job costed and can not be scrapped.") );
      return;
    }

    // If Post Production, invhist_id will be present, which means all distribution will be handled in the database.
    if (!_prodPosted->isChecked())
    {
      // Post production first
      postProduction newdlg(this);
      ParameterList params;
      newdlg.set(params);
      newdlg._transDate->setDate(_transDate->date());
      newdlg._transDate->setEnabled(false);
      newdlg._wo->setId(_wo->id());
      newdlg._wo->setEnabled(false);
      newdlg._scrap->setForgetful(true);
      newdlg._scrap->setChecked(false);
      newdlg._scrap->hide();
      newdlg._qty->setText(_topLevelQty->text());
      newdlg._qty->setEnabled(false);
      newdlg._immediateTransfer->setForgetful(true);
      newdlg._immediateTransfer->setChecked(false);
      newdlg._immediateTransfer->setEnabled(false);
      itemlocSeries = newdlg.exec();
      if (itemlocSeries)
      {
        XSqlQuery invhist;
        invhist.prepare("SELECT invhist_id FROM invhist "
                        "WHERE ((invhist_series=:itemlocSeries) "
                        " AND (invhist_transtype='RM')); ");
        invhist.bindValue(":itemlocSeries", itemlocSeries);
        invhist.exec();
        if (invhist.first())
          invhistid = invhist.value("invhist_id").toInt();
        else
        {
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Inventory History Information"),
                               invhist, __FILE__, __LINE__);
          return;
        }
      }
    }
    else
    {
      // Generate the series and itemlocdist record (if controlled)
      itemlocSeries = distributeInventory::SeriesCreate(itemsite.value("wo_itemsite_id").toInt(),
        _topLevelQty->toDouble() * -1, "SI", "SI", _wo->id());
      if (itemlocSeries <= 0)
        return;
      cleanup.bindValue(":itemlocSeries", itemlocSeries);

      // Distribute inventory
      if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(),
        QDate(), true) == XDialog::Rejected)
      {
        cleanup.exec();
        QMessageBox::information( this, tr("Scrap Work Order Material"), tr("Transaction Canceled") );
        return;
      }
    }

    scrapScrap.prepare("SELECT invScrap(:itemsite_id, :qty, formatWoNumber(:wo_id), "
                       " :descrip, :date, :invhist_id, :wo_prj_id, :itemlocSeries, TRUE) AS result;");
    scrapScrap.bindValue(":itemsite_id", itemsite.value("wo_itemsite_id").toInt());
    scrapScrap.bindValue(":wo_id", _wo->id());
    scrapScrap.bindValue(":qty",   _topLevelQty->toDouble());
    scrapScrap.bindValue(":descrip", tr("Top Level Item"));
    scrapScrap.bindValue(":date",  _transDate->date());
    scrapScrap.bindValue(":wo_prj_id", itemsite.value("wo_prj_id").toInt());
    scrapScrap.bindValue(":itemlocSeries", itemlocSeries);
    if (invhistid)
      scrapScrap.bindValue(":invhist_id", invhistid);
  }

  // Execute either invScrap or scrapWoMaterial depending on business logic above
  scrapScrap.exec();
  if (scrapScrap.first())
  {
    if (invhistid)
    {
      XSqlQuery post;
      post.prepare("SELECT postitemlocseries(:itemlocSeries) AS result;");
      post.bindValue(":itemlocSeries", itemlocSeries);
      post.exec();
    }

    if (_captive)
      accept();
    else
    {
      _qty->clear();
      _wo->setId(_wo->id());
      _womatl->setFocus();
    }
  }
  else
  {
    cleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Scrapping Material"),
                         scrapScrap, __FILE__, __LINE__);
    return;
  }
}

void scrapWoMaterialFromWIP::sHandleButtons()
{
  if (_wo->id() != -1 && _wo->method() == "D")
  {
    QMessageBox::critical( this, windowTitle(),
                      tr("Posting of scrap against disassembly work orders is not supported.") );
    _wo->setId(-1);
    _wo->setFocus();
    return;
  }
  
  _scrap->setEnabled(_wo->isValid() && (
                     (_scrapTopLevel->isChecked() && _topLevelQty->toDouble()) ||
         (_scrapComponent->isChecked() && _qty->toDouble() &&
          _womatl->isValid()) ));
}
