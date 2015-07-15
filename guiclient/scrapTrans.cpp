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

scrapTrans::scrapTrans(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_post,                 SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_qty,SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQty()));
  connect(_warehouse,           SIGNAL(newID(int)), this, SLOT(sPopulateQOH(int)));

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
      else if (scrapet.lastError().type() != QSqlError::NoError)
      {
	systemError(this, scrapet.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

    }
  }

  return NoError;
}

void scrapTrans::sPost()
{
  XSqlQuery scrapPost;
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

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  scrapPost.exec("BEGIN;");	// because of possible distribution cancelations
  scrapPost.prepare( "SELECT invScrap(itemsite_id, :qty, :docNumber,"
             "                :comments, :date) AS result "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  scrapPost.bindValue(":qty", _qty->toDouble());
  scrapPost.bindValue(":docNumber", _documentNum->text());
  scrapPost.bindValue(":comments", _notes->toPlainText());
  scrapPost.bindValue(":item_id", _item->id());
  scrapPost.bindValue(":warehous_id", _warehouse->id());
  scrapPost.bindValue(":date",        _transDate->date());
  scrapPost.exec();
  if (scrapPost.first())
  {
    int result = scrapPost.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("invScrap", result),
                  __FILE__, __LINE__);
      return;
    }
    else if (scrapPost.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, scrapPost.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (distributeInventory::SeriesAdjust(scrapPost.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information(this, tr("Scrap Transaction"),
                               tr("Transaction Canceled") );
      return;
    }

    scrapPost.exec("COMMIT;");

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
    rollback.exec();
    systemError(this, scrapPost.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    rollback.exec();
    systemError( this,
                tr("<p>No transaction was done because Item %1 "
                   "was not found at Site %2.")
                .arg(_item->itemNumber()).arg(_warehouse->currentText()));
  }
}

void scrapTrans::sPopulateQOH(int pWarehousid)
{
  XSqlQuery scrapPopulateQOH;
  if (_mode != cView)
  {
    scrapPopulateQOH.prepare( "SELECT itemsite_qtyonhand "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    scrapPopulateQOH.bindValue(":item_id", _item->id());
    scrapPopulateQOH.bindValue(":warehous_id", pWarehousid);
    scrapPopulateQOH.exec();
    if (scrapPopulateQOH.first())
    {
      _cachedQOH = scrapPopulateQOH.value("itemsite_qtyonhand").toDouble();
      _beforeQty->setDouble(scrapPopulateQOH.value("itemsite_qtyonhand").toDouble());

      if (_item->isFractional())
        _qty->setValidator(omfgThis->transQtyVal());
      else
        _qty->setValidator(new QIntValidator(this));

      sPopulateQty();
    }
    else if (scrapPopulateQOH.lastError().type() != QSqlError::NoError)
    {
      systemError(this, scrapPopulateQOH.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void scrapTrans::sPopulateQty()
{
  _afterQty->setDouble(_cachedQOH - _qty->toDouble());
}

