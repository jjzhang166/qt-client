/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scrapTrans.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

scrapTrans::scrapTrans(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);
  
  connect(_post,      SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_qty,       SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQty()));
  connect(_item,      SIGNAL(newId(int)), this, SLOT(sPopulateQOH()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateQOH()));

  _captive = false;

  _item->setType(ItemLineEdit::cGeneralInventory | ItemLineEdit::cActive);
  _warehouse->setType(WComboBox::AllActiveInventory);
  _qty->setValidator(omfgThis->transQtyVal());
  _beforeQty->setPrecision(omfgThis->qtyVal());
  _afterQty->setPrecision(omfgThis->qtyVal());

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  _item->setFocus();
}

scrapTrans::~scrapTrans()
{
  // no need to delete child widgets, Qt does it all for us
}

void scrapTrans::languageChange()
{
  retranslateUi(this);
}


enum SetResponse scrapTrans::set(const ParameterList &pParams)
{
  XSqlQuery scrapet;
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  int      invhistid = -1;

  param = pParams.value("invhist_id", &valid);
  if (valid)
    invhistid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _usernameLit->clear();
      _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
      _transDate->setDate(omfgThis->dbDate());
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _transDate->setEnabled(false);
      _item->setReadOnly(true);
      _warehouse->setEnabled(false);
      _qty->setEnabled(false);
      _documentNum->setEnabled(false);
      _notes->setEnabled(false);
      _post->hide();
      _close->setText(tr("&Close"));

      scrapet.prepare( "SELECT * "
                 "FROM invhist "
                 "WHERE (invhist_id=:invhist_id);" );
      scrapet.bindValue(":invhist_id", invhistid);
      scrapet.exec();
      if (scrapet.first())
      {
        _transDate->setDate(scrapet.value("invhist_transdate").toDate());
        _username->setText(scrapet.value("invhist_user").toString());
        _qty->setDouble(scrapet.value("invhist_invqty").toDouble());
        _beforeQty->setDouble(scrapet.value("invhist_qoh_before").toDouble());
        _afterQty->setDouble(scrapet.value("invhist_qoh_after").toDouble());
        _documentNum->setText(scrapet.value("invhist_ordnumber"));
        _notes->setText(scrapet.value("invhist_comments").toString());
        _item->setItemsiteid(scrapet.value("invhist_itemsite_id").toInt());
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Inventory History Information"),
                                    scrapet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }

    }
  }

  return NoError;
}

void scrapTrans::sPost()
{
  int itemlocSeries;
  struct {
    bool        condition;
    QString     msg;
    QWidget     *widget;
  } error[] = {
    { ! _item->isValid(),
      tr("You must select an Item before posting this transaction."), _item },
    { _qty->text().length() == 0,
      tr("<p>You must enter a Quantity before posting this Transaction."),
      _qty },
    { true, "", NULL }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Post Transaction"),
                          error[errIndex].msg);
    error[errIndex].widget->setFocus();
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
    parentItemlocdist.prepare("SELECT createitemlocdistparent(:itemsite_id, :qty, 'SI'::TEXT, NULL, :itemlocSeries) AS result;");
    parentItemlocdist.bindValue(":itemsite_id", _itemsiteId);
    parentItemlocdist.bindValue(":qty", _qty->toDouble() * -1);
    parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
    parentItemlocdist.exec();
    if (parentItemlocdist.first())
    {
      if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(),
        QDate(), true) == XDialog::Rejected)
      {
        cleanup.exec();
        QMessageBox::information(this, tr("Scrap Transaction"), tr("Transaction Canceled") );
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
                              parentItemlocdist, __FILE__, __LINE__))
    {
      return;
    }
  }

  // Post inventory transaction
  XSqlQuery scrapPost;
  scrapPost.prepare("SELECT invScrap(:itemsite_id, :qty, :docNumber, "
                    " :comments, :date, NULL, NULL, :itemlocSeries, TRUE) AS result;");
  scrapPost.bindValue(":itemsite_id", _itemsiteId);
  scrapPost.bindValue(":qty", _qty->toDouble());
  scrapPost.bindValue(":docNumber", _documentNum->text());
  scrapPost.bindValue(":comments", _notes->toPlainText());
  scrapPost.bindValue(":item_id", _item->id());
  scrapPost.bindValue(":warehous_id", _warehouse->id());
  scrapPost.bindValue(":date",        _transDate->date());
  scrapPost.bindValue(":itemlocSeries", itemlocSeries);
  scrapPost.exec();
  if (scrapPost.first())
  {
    int result = scrapPost.value("result").toInt();
    if (result < 0 || result != itemlocSeries)
    {
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Scrap Transaction"),
        scrapPost, __FILE__, __LINE__);
      return;
    }

    if (_captive)
      close();
    else
    {
      _close->setText(tr("&Close"));
      _item->setId(-1);
      _qty->clear();
      _documentNum->clear();
      _notes->clear();
      _beforeQty->clear();
      _afterQty->clear();
      _transDate->setDate(omfgThis->dbDate());

      _item->setFocus();
    }
  }
  else if (scrapPost.lastError().type() != QSqlError::NoError)
  {
    cleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Scrap Transaction"),
                         scrapPost, __FILE__, __LINE__);
    return;
  }
  else
  {
    cleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: Transaction Not Completed Due to Item:[%2] "
                            "not found at Site:[%3].")
                         .arg(windowTitle())
                         .arg(_item->itemNumber())
                         .arg(_warehouse->currentText()),__FILE__,__LINE__);
  }
}

void scrapTrans::sPopulateQOH()
{
  XSqlQuery scrapPopulateQOH;
  if (_mode != cView && _item->isValid() && _warehouse->isValid())
  {
    scrapPopulateQOH.prepare( "SELECT itemsite_qtyonhand, itemsite_id, "
               "  isControlledItemsite(itemsite_id) AS controlled "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id) "
               " AND (itemsite_warehous_id=:warehous_id) );" );
    scrapPopulateQOH.bindValue(":item_id", _item->id());
    scrapPopulateQOH.bindValue(":warehous_id", _warehouse->id());
    scrapPopulateQOH.exec();
    if (scrapPopulateQOH.first())
    {
      _itemsiteId = scrapPopulateQOH.value("itemsite_id").toInt();
      _controlledItem = scrapPopulateQOH.value("controlled").toBool();
      _cachedQOH = scrapPopulateQOH.value("itemsite_qtyonhand").toDouble();
      _beforeQty->setDouble(scrapPopulateQOH.value("itemsite_qtyonhand").toDouble());
      if (_item->isFractional())
        _qty->setValidator(omfgThis->transQtyVal());
      else
        _qty->setValidator(new QIntValidator(this));

      sPopulateQty();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                  scrapPopulateQOH, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void scrapTrans::sPopulateQty()
{
  _afterQty->setDouble(_cachedQOH - _qty->toDouble());
}

