/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "adjustmentTrans.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"

adjustmentTrans::adjustmentTrans(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _adjustmentTypeGroupInt = new QButtonGroup(this);

  connect(_absolute,                   SIGNAL(toggled(bool)), this, SLOT(sPopulateQty()));
  connect(_adjustmentTypeGroupInt,SIGNAL(buttonClicked(int)), this, SLOT(sPopulateQty()));
  connect(_post,                           SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_qty,          SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQty()));
  connect(_qty,          SIGNAL(textChanged(const QString&)), this, SLOT(sCostUpdated()));
  connect(_item,                          SIGNAL(newId(int)), this, SLOT(sPopulateQOH()));
  connect(_warehouse,                     SIGNAL(newID(int)), this, SLOT(sPopulateQOH()));
  connect(_cost, SIGNAL(textChanged(const QString&)), this, SLOT(sCostUpdated()));
  connect(_costManual, SIGNAL(toggled(bool)), this, SLOT(sPopulateQty()));

  _captive = false;

  _item->setType((ItemLineEdit::cGeneralInventory ^ ItemLineEdit::cBreeder) | ItemLineEdit::cActive);
  _item->addExtraClause( QString("(item_type NOT IN ('R', 'F'))") );
  _item->addExtraClause( QString("(itemsite_costmethod != 'J')") );
  _warehouse->setType(WComboBox::AllActiveInventory);
  _afterQty->setPrecision(omfgThis->qtyVal());
  _beforeQty->setPrecision(omfgThis->qtyVal());
  _cost->setValidator(omfgThis->costVal());
  _qty->setValidator(omfgThis->transQtyVal());
  _unitCost->setPrecision(omfgThis->costVal());

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  if (!_metrics->boolean("AllowAvgCostMethod"))
    _tab->removeTab(0);

  _item->setFocus();
}

adjustmentTrans::~adjustmentTrans()
{
  // no need to delete child widgets, Qt does it all for us
}

void adjustmentTrans::languageChange()
{
  retranslateUi(this);
}

enum SetResponse adjustmentTrans::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  int      invhistid = -1;
  bool     noQty     = true;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = true;

    _item->setItemsiteid(param.toInt());
    _item->setEnabled(false);
    _warehouse->setEnabled(false);
  }

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _captive = true;

    _qty->setDouble(param.toDouble());
    _qty->setEnabled(false);
    _afterQty->setDouble(param.toDouble());
    _absolute->setChecked(true);
    _adjustmentTypeGroup->setEnabled(false);

    noQty = false;
  }

  param = pParams.value("invhist_id", &valid);
  if (valid)
    invhistid = param.toInt();

  param = pParams.value("mode", &valid);
  XSqlQuery setAdjustment;
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
      _adjustmentTypeGroup->setEnabled(false);
      _qty->setEnabled(false);
      _documentNum->setEnabled(false);
      _notes->setEnabled(false);
      _post->hide();
      _close->setText(tr("&Close"));

      setAdjustment.prepare( "SELECT * "
                 "FROM invhist "
                 "WHERE (invhist_id=:invhist_id);" );
      setAdjustment.bindValue(":invhist_id", invhistid);
      setAdjustment.exec();
      if (setAdjustment.first())
      {
        _transDate->setDate(setAdjustment.value("invhist_transdate").toDate());
        _username->setText(setAdjustment.value("invhist_user").toString());
        _qty->setDouble(setAdjustment.value("invhist_invqty").toDouble());
        _beforeQty->setDouble(setAdjustment.value("invhist_qoh_before").toDouble());
        _afterQty->setDouble(setAdjustment.value("invhist_qoh_after").toDouble());
        _documentNum->setText(setAdjustment.value("invhist_ordnumber"));
        _notes->setText(setAdjustment.value("invhist_comments").toString());
        _item->setItemsiteid(setAdjustment.value("invhist_itemsite_id").toInt());
      }
      else if (setAdjustment.lastError().type() != QSqlError::NoError)
      {
	systemError(this, setAdjustment.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

    }
  }

  return NoError;
}

void adjustmentTrans::sPost()
{
  XSqlQuery adjustmentPost;
  double qty = _qty->toDouble();
  double cost = _cost->toDouble();

  if (_absolute->isChecked())
  {
    qty = (_qty->toDouble() - _cachedQOH);
    cost = (_cost->toDouble() - _cachedValue);
  }

  struct {
    bool        condition;
    QString     msg;
    QWidget     *widget;
  } error[] = {
    { ! _item->isValid(),
      tr("You must select an Item before posting this transaction."), _item },
    { _qty->text().length() == 0 || qty == 0,
      tr("<p>You must enter a Quantity before posting this Transaction."),
      _qty },
    { _costAdjust->isEnabled() && _costAdjust->isChecked() && _costManual->isChecked() && (_cost->text().length() == 0 || cost == 0),
      tr("<p>You must enter a total cost value for the inventory to be transacted."),
      _cost },
    { _costMethod == "A" && _afterQty->toDouble() < 0,
      tr("<p>Average cost adjustments may not result in a negative quantity on hand."),
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

  adjustmentPost.exec("BEGIN;");	// because of possible distribution cancelations
  adjustmentPost.prepare( "SELECT invAdjustment(itemsite_id, :qty, :docNumber,"
             "                     :comments, :date, :cost) AS result "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  adjustmentPost.bindValue(":qty", qty);
  adjustmentPost.bindValue(":docNumber", _documentNum->text());
  adjustmentPost.bindValue(":comments",    _notes->toPlainText());
  adjustmentPost.bindValue(":item_id", _item->id());
  adjustmentPost.bindValue(":warehous_id", _warehouse->id());
  adjustmentPost.bindValue(":date",        _transDate->date());
  if(!_costAdjust->isChecked())
    adjustmentPost.bindValue(":cost", 0.0);
  else if(_costManual->isChecked())
    adjustmentPost.bindValue(":cost", cost);
  adjustmentPost.exec();
  if (adjustmentPost.first())
  {
    int result = adjustmentPost.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("invAdjustment", result),
                  __FILE__, __LINE__);
    }
    else if (adjustmentPost.lastError().type() != QSqlError::NoError)
    {
      systemError(this, adjustmentPost.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (distributeInventory::SeriesAdjust(adjustmentPost.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information(this, tr("Inventory Adjustment"),
                               tr("Transaction Canceled") );
      return;
    }

    adjustmentPost.exec("COMMIT;");
    if (_captive)
      close();
    else
    {
      _item->setId(-1);
      _qty->clear();
      _documentNum->clear();
      _notes->clear();
      _beforeQty->clear();
      _afterQty->clear();
      _cost->clear();
      _transDate->setDate(omfgThis->dbDate());

      _item->setFocus();
    }
  }
  else if (adjustmentPost.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, adjustmentPost.lastError().databaseText(), __FILE__, __LINE__);
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

void adjustmentTrans::sPopulateQOH()
{
  XSqlQuery populateAdjustment;
  if (_mode != cView)
  {
    populateAdjustment.prepare( "SELECT itemsite_freeze, itemsite_qtyonhand, itemsite_costmethod, itemsite_value "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id));" );
    populateAdjustment.bindValue(":item_id", _item->id());
    populateAdjustment.bindValue(":warehous_id", _warehouse->id());
    populateAdjustment.exec();

    _absolute->setStyleSheet("");
    if (populateAdjustment.first())
    {
      _cachedValue = populateAdjustment.value("itemsite_value").toDouble();
      _cachedQOH = populateAdjustment.value("itemsite_qtyonhand").toDouble();
      if(_cachedQOH == 0.0)
        _costManual->setChecked(true);
      _beforeQty->setDouble(populateAdjustment.value("itemsite_qtyonhand").toDouble());
      _costAdjust->setChecked(true);
      _costAdjust->setEnabled(populateAdjustment.value("itemsite_costmethod").toString() == "A");
      _costMethod = populateAdjustment.value("itemsite_costmethod").toString();

      if (populateAdjustment.value("itemsite_freeze").toBool())
        _absolute->setStyleSheet(QString("* { color: %1; }")
                                 .arg(namedColor("error").name()));

      if (_item->isFractional())
        _qty->setValidator(omfgThis->transQtyVal());
      else
        _qty->setValidator(new QIntValidator(this));
    }
    else if (populateAdjustment.lastError().type() != QSqlError::NoError)
    {
      systemError(this, populateAdjustment.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sPopulateQty();
  }
}

void adjustmentTrans::sPopulateQty()
{
  if (_mode == cNew)
  {
    
    if (_qty->text().trimmed().length())
    {
      if (_absolute->isChecked())
        _afterQty->setDouble(_qty->toDouble());

      else if (_relative->isChecked())
        _afterQty->setDouble(_cachedQOH + _qty->toDouble());
    }
    else
      _afterQty->clear();

    bool neg = ((_afterQty->toDouble() < _cachedQOH) || (_afterQty->toDouble() == 0));
    if(neg)
      _costCalculated->setChecked(true);
    _costManual->setEnabled(!neg);
    _cost->setEnabled(!neg && _costManual->isChecked());
    _lblCost->setEnabled(!neg);
    _unitCost->setEnabled(!neg);
    _unitCostLit->setEnabled(!neg);
    
    if (_afterQty->toDouble() == 0)
    {
      _costAdjust->setChecked(true);
      _costAdjust->setEnabled(false);
    }
    else
      _costAdjust->setEnabled(_costMethod == "A");
  }
}

void adjustmentTrans::sCostUpdated()
{
  if(_cost->toDouble() == 0.0 || _qty->toDouble() == 0.0)
    _unitCost->setText(tr("N/A"));
  else
    _unitCost->setDouble(_cost->toDouble() / _qty->toDouble());
}

