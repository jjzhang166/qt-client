/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "expenseTrans.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"

expenseTrans::expenseTrans(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_post,                 SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_qty,SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQty()));
  connect(_warehouse,           SIGNAL(newID(int)), this, SLOT(sPopulateQOH(int)));

  _captive = FALSE;
  _prjid = -1;

  _item->setType(ItemLineEdit::cGeneralInventory | ItemLineEdit::cActive);
  _warehouse->setType(WComboBox::AllActiveInventory);

  _qty->setValidator(omfgThis->transQtyVal());
  _afterQty->setPrecision(omfgThis->qtyVal());
  _beforeQty->setPrecision(omfgThis->qtyVal());

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  _item->setFocus();
}

expenseTrans::~expenseTrans()
{
  // no need to delete child widgets, Qt does it all for us
}

void expenseTrans::languageChange()
{
  retranslateUi(this);
}

enum SetResponse expenseTrans::set(const ParameterList &pParams)
{
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

      _transDate->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _documentNum->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _post->hide();
      _expcat->setEnabled(FALSE);
      _close->setText(tr("&Close"));

      XSqlQuery histq;
      histq.prepare("SELECT invhist.*, invhistexpcat_expcat_id"
                "  FROM invhist"
                "  LEFT OUTER JOIN invhistexpcat ON (invhist_id=invhistexpcat_invhist_id)"
                " WHERE (invhist_id=:invhist_id);" );
      histq.bindValue(":invhist_id", invhistid);
      histq.exec();
      if (histq.first())
      {
        _transDate->setDate(histq.value("invhist_transdate").toDate());
        _username->setText(histq.value("invhist_user").toString());
        _qty->setDouble(histq.value("invhist_invqty").toDouble());
        _beforeQty->setDouble(histq.value("invhist_qoh_before").toDouble());
        _afterQty->setDouble(histq.value("invhist_qoh_after").toDouble());
        _documentNum->setText(histq.value("invhist_ordnumber"));
        _notes->setText(histq.value("invhist_comments").toString());
        _item->setItemsiteid(histq.value("invhist_itemsite_id").toInt());
        _expcat->setId(histq.value("invhistexpcat_expcat_id").toInt());
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting History"),
                                    histq, __FILE__, __LINE__))
	return UndefinedError;
    }
  }

  return NoError;
}

void expenseTrans::sPost()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(! _item->isValid(), _item,
                          tr("You must select an Item before posting this "
                             "transaction."))
         << GuiErrorCheck(_qty->text().isEmpty(), _qty,
                          tr("<p>You must enter a Quantity before posting this "
                             "transaction."))
         << GuiErrorCheck(!_expcat->isValid(), _expcat,
                          tr("You must select an Expense Category before "
                             "posting this transaction."))
         ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Post Transaction"), errors))
    return;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery begin("BEGIN;");	// because of possible distribution cancelations
  XSqlQuery expq;
  expq.prepare("SELECT invExpense(itemsite_id, :qty, :expcatid, :docNumber,"
               "                  :comments, :date, :prj_id) AS result"
               "  FROM itemsite"
               " WHERE ((itemsite_item_id=:item_id)"
               "    AND (itemsite_warehous_id=:warehous_id));" );
  expq.bindValue(":qty",         _qty->toDouble());
  expq.bindValue(":expcatid",    _expcat->id());
  expq.bindValue(":docNumber",   _documentNum->text());
  expq.bindValue(":comments",    _notes->toPlainText());
  expq.bindValue(":item_id",     _item->id());
  expq.bindValue(":warehous_id", _warehouse->id());
  expq.bindValue(":date",        _transDate->date());
  if (_prjid != -1)
    expq.bindValue(":prj_id", _prjid);
  expq.exec();

  if (expq.first())
  {
    if (distributeInventory::SeriesAdjust(expq.value("result").toInt(),
                                          this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information(this, tr("Expense Transaction"),
                               tr("Transaction Canceled") );
      return;
    }

    XSqlQuery commit("COMMIT;");

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
  else if (expq.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Could Not Post"),
                         expq, __FILE__, __LINE__);
    return;
  }
  else
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Item not found"),
                         tr("<p>No transaction was done because Item %1 "
                            "was not found at Site %2.")
                         .arg(_item->itemNumber(), _warehouse->currentText()));
  }
}

void expenseTrans::sPopulateQOH(int pWarehousid)
{
  if (_mode != cView)
  {
    XSqlQuery qohq;
    qohq.prepare( "SELECT itemsite_qtyonhand "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    qohq.bindValue(":item_id", _item->id());
    qohq.bindValue(":warehous_id", pWarehousid);
    qohq.exec();
    if (qohq.first())
    {
      _cachedQOH = qohq.value("itemsite_qtyonhand").toDouble();
      _beforeQty->setDouble(qohq.value("itemsite_qtyonhand").toDouble());

      if (_item->isFractional())
        _qty->setValidator(omfgThis->transQtyVal());
      else
        _qty->setValidator(new QIntValidator(this));

      sPopulateQty();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting QOH"),
                                  qohq, __FILE__, __LINE__))
      return;
  }
}

void expenseTrans::sPopulateQty()
{
  _afterQty->setDouble(_cachedQOH - _qty->toDouble());
}

