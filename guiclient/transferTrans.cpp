/*
 * Common Public Attribution License Version 1.0.
 *
 * The contents of this file are subject to the Common Public Attribution
 * License Version 1.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla
 * Public License Version 1.1 but Sections 14 and 15 have been added to
 * cover use of software over a computer network and provide for limited
 * attribution for the Original Developer. In addition, Exhibit A has
 * been modified to be consistent with Exhibit B.
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is xTuple ERP: PostBooks Edition
 *
 * The Original Developer is not the Initial Developer and is __________.
 * If left blank, the Original Developer is the Initial Developer.
 * The Initial Developer of the Original Code is OpenMFG, LLC,
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved.
 *
 * Contributor(s): ______________________.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the xTuple End-User License Agreeement (the xTuple License), in which
 * case the provisions of the xTuple License are applicable instead of
 * those above.  If you wish to allow use of your version of this file only
 * under the terms of the xTuple License and not to allow others to use
 * your version of this file under the CPAL, indicate your decision by
 * deleting the provisions above and replace them with the notice and other
 * provisions required by the xTuple License. If you do not delete the
 * provisions above, a recipient may use your version of this file under
 * either the CPAL or the xTuple License.
 *
 * EXHIBIT B.  Attribution Information
 *
 * Attribution Copyright Notice:
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 *
 * Attribution Phrase:
 * Powered by xTuple ERP: PostBooks Edition
 *
 * Attribution URL: www.xtuple.org
 * (to be included in the "Community" menu of the application if possible)
 *
 * Graphic Image as provided in the Covered Code, if any.
 * (online at www.xtuple.com/poweredby)
 *
 * Display of Attribution Information is required in Larger Works which
 * are defined in the CPAL as a work which combines Covered Code or
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "transferTrans.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"

transferTrans::transferTrans(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_fromWarehouse, SIGNAL(newID(int)), this, SLOT(sPopulateFromQty(int)));
  connect(_post, SIGNAL(clicked()),                   this, SLOT(sPost()));
  connect(_qty,  SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQty(const QString&)));
  connect(_toWarehouse, SIGNAL(newID(int)), this, SLOT(sPopulateToQty(int)));

  _captive = FALSE;

  _item->setType(ItemLineEdit::cActive);
  _qty->setValidator(omfgThis->qtyVal());

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _item->setFocus();
}

transferTrans::~transferTrans()
{
  // no need to delete child widgets, Qt does it all for us
}

void transferTrans::languageChange()
{
  retranslateUi(this);
}

enum SetResponse transferTrans::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  int      invhistid = -1;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _fromWarehouse->setEnabled(FALSE);
  }

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _captive = TRUE;

    _qty->setText(formatQty(param.toDouble()));
    _qty->setEnabled(FALSE);
  }

  param = pParams.value("invhist_id", &valid);
  if (valid)
    invhistid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      setCaption(tr("Enter Inter-Warehouse Transfer"));
      _usernameLit->clear();
      _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
      _transDate->setDate(omfgThis->dbDate());

      if (!_item->isValid())
        _item->setFocus();
      else if (_qty->text().length() == 0)
	_qty->setFocus();
      else
        _documentNum->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      setCaption(tr("Inter-Warehouse Transaction Information"));
      _transDate->setEnabled(FALSE);
      _item->setEnabled(FALSE);
      _toWarehouse->setEnabled(FALSE);
      _fromWarehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _documentNum->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _close->setText(tr("&Close"));
      _close->setFocus();
      _post->hide();

      q.prepare( "SELECT invhist.*, "
                 "       ABS(invhist_invqty) AS transqty,"
                 "       CASE WHEN (invhist_invqty > 0) THEN invhist_qoh_before"
                 "            ELSE NULL"
                 "       END AS qohbefore,"
                 "       CASE WHEN (invhist_invqty > 0) THEN invhist_qoh_after"
                 "            ELSE NULL"
                 "       END AS qohafter,"
                 "       CASE WHEN (invhist_invqty > 0) THEN NULL"
                 "            ELSE invhist_qoh_before"
                 "       END AS fromqohbefore,"
                 "       CASE WHEN (invhist_invqty > 0) THEN NULL"
                 "            ELSE invhist_qoh_after"
                 "       END AS fromqohafter,"
                 "       CASE WHEN (invhist_invqty > 0) THEN itemsite_warehous_id"
                 "            ELSE invhist_xfer_warehous_id"
                 "       END AS toWarehouse,"
                 "       CASE WHEN (invhist_invqty > 0) THEN invhist_xfer_warehous_id"
                 "            ELSE itemsite_warehous_id"
                 "       END AS fromWarehouse "
                 "  FROM invhist, itemsite"
                 " WHERE ((invhist_itemsite_id=itemsite_id)"
                 "   AND  (invhist_id=:invhist_id)); " );
      q.bindValue(":invhist_id", invhistid);
      q.exec();
      if (q.first())
      {
        _transDate->setDate(q.value("invhist_transdate").toDate());
        _username->setText(q.value("invhist_user").toString());
        _qty->setText(formatQty(q.value("transqty").toDouble()));
        _fromBeforeQty->setText(formatQty(q.value("fromqohbefore").toDouble()));
        _fromAfterQty->setText(formatQty(q.value("fromqohafter").toDouble()));
        _toBeforeQty->setText(formatQty(q.value("qohbefore").toDouble()));
        _toAfterQty->setText(formatQty(q.value("qohafter").toDouble()));
        _documentNum->setText(q.value("invhist_docnumber"));
        _notes->setText(q.value("invhist_comments").toString());
        _item->setItemsiteid(q.value("invhist_itemsite_id").toInt());
        _toWarehouse->setId(q.value("toWarehouse").toInt());
        _fromWarehouse->setId(q.value("fromWarehouse").toInt());
      }

      _close->setFocus();
    }
  }

  return NoError;
}

void transferTrans::sPost()
{
  struct {
    bool        condition;
    QString     msg;
    QWidget     *widget;
  } error[] = {
    { ! _item->isValid(),
      tr("You must select an Item before posting this transaction."), _item },
    { _qty->text().length() == 0 || _qty->toDouble() <= 0,
      tr("<p>You must enter a positive Quantity before posting this Transaction."),
      _qty },
    { _fromWarehouse->id() == _toWarehouse->id(),
      tr("<p>The Target Warehouse is the same as the Source Warehouse. "
         "You must select a different Warehouse for each before "
         "posting this Transaction"), _fromWarehouse },
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

  q.exec("BEGIN;");	// because of possible distribution cancelations
  q.prepare( "SELECT interWarehouseTransfer(:item_id, :from_warehous_id,"
             "                              :to_warehous_id, :qty, 'Misc', "
             "                              :docNumber, :comments, 0, :date ) AS result;");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":from_warehous_id", _fromWarehouse->id());
  q.bindValue(":to_warehous_id",   _toWarehouse->id());
  q.bindValue(":qty",              _qty->toDouble());
  q.bindValue(":docNumber", _documentNum->text());
  q.bindValue(":comments", _notes->text());
  q.bindValue(":date",        _transDate->date());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("interWarehouseTransfer", result),
                  __FILE__, __LINE__);
      return;
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information(this, tr("Transfer Transaction"),
                               tr("Transaction Canceled") );
      return;
    }

    q.exec("COMMIT;");

    if (_captive)
      close();
    else
    {
      _close->setText(tr("&Close"));
      _item->setId(-1);
      _qty->clear();
      _documentNum->clear();
      _notes->clear();
      _fromBeforeQty->clear();
      _fromAfterQty->clear();
      _toBeforeQty->clear();
      _toAfterQty->clear();

      _item->setFocus();
    }
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at transferTrans::%1, Item Site ID #%2, To Warehouse ID #%3, From Warehouse #%4.")
                       .arg(__LINE__)
                       .arg(_item->id())
                       .arg(_toWarehouse->id())
                       .arg(_fromWarehouse->id()) );
    return;
  }
}

void transferTrans::sPopulateFromQty(int pWarehousid)
{
  if (_mode != cView)
  {
    q.prepare( "SELECT itemsite_qtyonhand "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", pWarehousid);
    q.exec();
    if (q.first())
    {
      _cachedFromBeforeQty = q.value("itemsite_qtyonhand").toDouble();
      _fromBeforeQty->setText(formatQty(q.value("itemsite_qtyonhand").toDouble()));

      if (_qty->text().length())
        _fromAfterQty->setText(formatQty(q.value("itemsite_qtyonhand").toDouble() - _qty->toDouble()));
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void transferTrans::sPopulateToQty(int pWarehousid)
{
  if (_mode != cView)
  {
    q.prepare( "SELECT itemsite_qtyonhand "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", pWarehousid);
    q.exec();
    if (q.first())
    {
      _cachedToBeforeQty = q.value("itemsite_qtyonhand").toDouble();
      _toBeforeQty->setText(formatQty(q.value("itemsite_qtyonhand").toDouble()));

      if (_qty->text().length())
        _toAfterQty->setText(formatQty(q.value("itemsite_qtyonhand").toDouble() + _qty->toDouble()));
    }
  }
}

void transferTrans::sUpdateQty(const QString &pQty)
{
  if (_mode != cView)
  {
    _fromAfterQty->setText(formatQty(_cachedFromBeforeQty - pQty.toDouble()));
    _toAfterQty->setText(formatQty(_cachedToBeforeQty + pQty.toDouble()));
  }
}
