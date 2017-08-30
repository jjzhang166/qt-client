/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
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

expenseTrans::expenseTrans(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_post,          SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_qty,           SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQty()));
  connect(_warehouse,     SIGNAL(newID(int)), this, SLOT(sPopulateQOH()));
  connect(_item,          SIGNAL(newId(int)), this, SLOT(sPopulateQOH()));

  _captive = false;
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

      _transDate->setEnabled(false);
      _item->setReadOnly(true);
      _warehouse->setEnabled(false);
      _qty->setEnabled(false);
      _documentNum->setEnabled(false);
      _notes->setEnabled(false);
      _post->hide();
      _expcat->setEnabled(false);
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
  int itemlocSeries = 0;
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
    parentItemlocdist.prepare("SELECT createitemlocdistparent(:itemsite_id, :qty, 'EX'::TEXT, NULL, :itemlocSeries) AS result;");
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
        QMessageBox::information( this, tr("Expense Transaction"), tr("Posting Distribution Detail Failed") );
        return;
      }
    }
    else
    {
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Record"),
                           parentItemlocdist, __FILE__, __LINE__);
      return;
    }
  }

  // Proceed to post inventory transaction
  XSqlQuery expq;
  expq.prepare("SELECT invExpense(:itemsite_id, :qty, :expcatid, :docNumber, "
               "  :comments, :date, :prj_id, :itemlocSeries, TRUE) AS result;");
  expq.bindValue(":itemsite_id",         _itemsiteId);
  expq.bindValue(":qty",         _qty->toDouble());
  expq.bindValue(":expcatid",    _expcat->id());
  expq.bindValue(":docNumber",   _documentNum->text());
  expq.bindValue(":comments",    _notes->toPlainText());
  expq.bindValue(":date",        _transDate->date());
  expq.bindValue(":itemlocSeries", itemlocSeries);
  if (_prjid != -1)
    expq.bindValue(":prj_id", _prjid);
  expq.exec();
  if (expq.first())
  {
    int result = expq.value("result").toInt();
    if (result < 0 || result != itemlocSeries)
    {
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Inventory Expense transaction."),
        expq, __FILE__, __LINE__);
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
  else if (expq.lastError().type() != QSqlError::NoError)
  {
    cleanup.exec(); 
    ErrorReporter::error(QtCriticalMsg, this, tr("Could Not Post"),
                         expq, __FILE__, __LINE__);
    return;
  }
  else
  {
    cleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Item not found"),
                         tr("<p>No transaction was done because Item %1 "
                            "was not found at Site %2.")
                         .arg(_item->itemNumber(), _warehouse->currentText()));
  }
}

void expenseTrans::sPopulateQOH()
{
  if (_mode != cView && _item->isValid() && _warehouse->isValid())
  {
    XSqlQuery qohq;
    qohq.prepare( "SELECT itemsite_qtyonhand, "
               "  itemsite_id, isControlledItemsite(itemsite_id) AS controlled "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    qohq.bindValue(":item_id", _item->id());
    qohq.bindValue(":warehous_id", _warehouse->id());
    qohq.exec();
    if (qohq.first())
    {
      _itemsiteId = qohq.value("itemsite_id").toInt();
      _controlledItem = qohq.value("controlled").toBool();
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

