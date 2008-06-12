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

#include "materialReceiptTrans.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "inputManager.h"
#include "distributeInventory.h"
#include "storedProcErrorLookup.h"

materialReceiptTrans::materialReceiptTrans(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_item,      SIGNAL(newId(int)), this, SLOT(sPopulateQty()));
  connect(_post,       SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateQty()));

  _captive = FALSE;

  _item->setType(ItemLineEdit::cActive);
  _qty->setValidator(omfgThis->qtyVal());
  _wo->setType(cWoOpen | cWoExploded | cWoReleased | cWoIssued);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));
  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  _item->setFocus();
}

materialReceiptTrans::~materialReceiptTrans()
{
  // no need to delete child widgets, Qt does it all for us
}

void materialReceiptTrans::languageChange()
{
  retranslateUi(this);
}

enum SetResponse materialReceiptTrans::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  int      _invhistid = -1;

  param = pParams.value("invhist_id", &valid);
  if (valid)
    _invhistid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      setCaption(tr("Enter Material Receipt"));
      _usernameLit->clear();
      _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
      _transDate->setDate(omfgThis->dbDate());

      connect(_qty, SIGNAL(textChanged(const QString &)), this, SLOT(sUpdateQty(const QString &)));
      connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateQty()));
      connect(_issueToWo, SIGNAL(toggled(bool)), this, SLOT(sPopulateQty()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      setCaption(tr("Material Receipt"));
      _transDate->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _issueToWo->setEnabled(FALSE);
      _wo->setReadOnly(TRUE);
      _documentNum->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _post->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();

      XSqlQuery popq;
      popq.prepare( "SELECT * "
                 "FROM invhist "
                 "WHERE (invhist_id=:invhist_id);" );
      popq.bindValue(":invhist_id", _invhistid);
      popq.exec();
      if (popq.first())
      {
        // _item first so it doesn't trigger sPopulateQty
        _item->setItemsiteid(popq.value("invhist_itemsite_id").toInt());
        _transDate->setDate(popq.value("invhist_transdate").toDate());
        _username->setText(popq.value("invhist_user").toString());
        _qty->setText(formatQty(popq.value("invhist_invqty").toDouble()));
        _beforeQty->setText(formatQty(popq.value("invhist_qoh_before").toDouble()));
        _afterQty->setText(formatQty(popq.value("invhist_qoh_after").toDouble()));
        _documentNum->setText(popq.value("invhist_ordnumber"));
        _notes->setText(popq.value("invhist_comments").toString());
      }
      else if (popq.lastError().type() != QSqlError::None)
      {
	systemError(this, popq.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
  }

  return NoError;
}

void materialReceiptTrans::sPost()
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

  if (_issueToWo->isChecked())
  {
    q.prepare( "SELECT womatl_id, womatl_issuemethod "
               "FROM womatl, wo, itemsite "
               "WHERE ( ( womatl_itemsite_id=itemsite_id)"
               " AND (womatl_wo_id=wo_id)"
               " AND (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id)"
               " AND (wo_id=:wo_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (q.first())
    {
      if ( (q.value("womatl_issuemethod").toString() == "S") ||
           (q.value("womatl_issuemethod").toString() == "M") )
      {
        int womatlid = q.value("womatl_id").toInt();

	q.prepare( "SELECT invReceiptIssueToWomatl(itemsite_id, :qty, :docNumber, :womatl_id, :comments) AS result "
                   "FROM itemsite "
                   "WHERE ( (itemsite_item_id=:item_id)"
                   " AND (itemsite_warehous_id=:warehous_id) );" );
        q.bindValue(":qty", _qty->toDouble());
        q.bindValue(":docNumber", _documentNum->text());
        q.bindValue(":womatl_id", womatlid);
        q.bindValue(":comments", _notes->text());
        q.bindValue(":item_id", _item->id());
        q.bindValue(":warehous_id", _warehouse->id());
        q.exec();
        if (q.first())
        {
          int result = q.value("result").toInt();
          if (result < 0)
          {
            systemError(this,
                        storedProcErrorLookup("invReceiptIssueToWomatl", result),
                        __FILE__, __LINE__);
            return;
          }
        }
        else if (q.lastError().type() != QSqlError::None)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }

        if (_captive)
          close();
        else
        {
          _close->setText(tr("&Close"));
          _item->setId(-1);
          _qty->clear();
          _beforeQty->clear();
          _afterQty->clear();
          _documentNum->clear();
          _issueToWo->setChecked(FALSE);
          _wo->setId(-1);
          _notes->clear();

          _item->setFocus();
        }
      }
      else
        QMessageBox::critical(this, tr("Cannot Receive and Issue Material"),
                              tr("<p>The select Item may not be issued againt "
                                 "the selected W/O as the W/O Material "
                                 "Requirement Issue Method is Pull. Material "
                                 "is issued to this W/O Material Requirement "
                                 "via a Backflush." ) );
    }
    else
      QMessageBox::critical(this, tr("Cannot Receive and Issue Material"),
                            tr("The select Item may not be issued againt the "
                               "selected W/O as there there isn't a W/O "
                               "Material Requirement for the selected W/O/Item "
                               "combination." ) );
  }
  else
  {
    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    q.exec("BEGIN;");	// because of possible distribution cancelations
    q.prepare( "SELECT invReceipt(itemsite_id, :qty, '', :docNumber,"
               "                  :comments, :date) AS result "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":qty", _qty->toDouble());
    q.bindValue(":docNumber", _documentNum->text());
    q.bindValue(":comments", _notes->text());
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.bindValue(":date",        _transDate->date());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("invReceipt", result),
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
        QMessageBox::information(this, tr("Enter Receipt"),
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
        _beforeQty->clear();
        _afterQty->clear();
        _documentNum->clear();
        _issueToWo->setChecked(FALSE);
        _wo->setId(-1);
        _notes->clear();

        _item->setFocus();
      }
    } 
    else if (q.lastError().type() != QSqlError::None)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      rollback.exec();
      systemError( this,
                  tr("<p>No transaction was done because Item %1 "
                     "was not found at Warehouse %2.")
                  .arg(_item->itemNumber()).arg(_warehouse->currentText()));
    }
  }
}

void materialReceiptTrans::sPopulateQty()
{
  q.prepare( "SELECT itemsite_qtyonhand "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (q.first())
  {
    _cachedQOH = q.value("itemsite_qtyonhand").toDouble();
    _beforeQty->setText(formatQty(q.value("itemsite_qtyonhand").toDouble()));

    if (_issueToWo->isChecked())
      _afterQty->setText(formatQty(q.value("itemsite_qtyonhand").toDouble()));
    else if (_qty->text().length())
      _afterQty->setText(formatQty(_cachedQOH + _qty->toDouble()));
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _wo->setWarehouse(_warehouse->id());
}

void materialReceiptTrans::sUpdateQty(const QString &pQty)
{
  if (_issueToWo->isChecked())
    _afterQty->setText(_beforeQty->text());
  else
    _afterQty->setText(formatQty(_cachedQOH + pQty.toDouble()));
}
