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

#include "transformTrans.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"

transformTrans::transformTrans(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_item,       SIGNAL(newId(int)), this, SLOT(sPopulateQOH()));
  connect(_post,        SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_qty, SIGNAL(editingFinished()), this, SLOT(sRecalculateAfter()));
  connect(_source,     SIGNAL(newId(int)), this, SLOT(sPopulateQOH()));
  connect(_source,    SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_target,     SIGNAL(newID(int)), this, SLOT(sPopulateTarget(int)));
  connect(_target,     SIGNAL(newID(int)), this, SLOT(sHandleButtons()));
  connect(_warehouse,  SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_warehouse,  SIGNAL(newID(int)), this, SLOT(sPopulateQOH()));

  _captive = FALSE;

  _item->setType(ItemLineEdit::cActive);
  _qty->setValidator(omfgThis->qtyVal());

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _source->addColumn( tr("Location"),_itemColumn, Qt::AlignLeft, true, "locationname");
  _source->addColumn( tr("Lot/Serial #"),     -1, Qt::AlignLeft, true, "lotserial");
  _source->addColumn( tr("Qty."),     _qtyColumn, Qt::AlignRight,true, "qty");

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  _controlled = true;   // safest assumption
  _item->setFocus();
}

transformTrans::~transformTrans()
{
  // no need to delete child widgets, Qt does it all for us
}

void transformTrans::languageChange()
{
  retranslateUi(this);
}

enum SetResponse transformTrans::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;
  int      invhistid = -1;

  param = pParams.value("invhist_id", &valid);
  if (valid)
    invhistid = param.toInt();

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      setCaption(tr("Enter Transform Transaction"));
      _usernameLit->clear();
      _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
      _transDate->setDate(omfgThis->dbDate());
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      setCaption(tr("Transform Transaction"));
      _transDate->setEnabled(FALSE);
      _item->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);
      _target->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _documentNum->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _close->setText(tr("&Close"));
      _close->setFocus();
      _post->hide();

      q.prepare( "SELECT invhist.*, "
                 "       formatQty(abs(invhist_invqty)) AS transqty,"
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
                 "       CASE WHEN (invhist_invqty > 0) THEN invhist_xfer_warehous_id"
                 "            ELSE itemsite_warehous_id"
                 "       END AS warehous_id "
                 "  FROM invhist, itemsite"
                 " WHERE ((invhist_itemsite_id=itemsite_id)"
                 "   AND  (invhist_id=:invhist_id)); " );
      q.bindValue(":invhist_id", invhistid);
      q.exec();
      if (q.first())
      {
        _transDate->setDate(q.value("invhist_transdate").toDate());
        _username->setText(q.value("invhist_user").toString());
        _qty->setText(q.value("transqty"));
        _fromBeforeQty->setText(q.value("fromqohbefore").toString());
        _fromAfterQty->setText(q.value("fromqohafter").toString());
        _toBeforeQty->setText(q.value("qohbefore").toString());
        _toAfterQty->setText(q.value("qohafter").toString());
        _documentNum->setText(q.value("invhist_docnumber").toString());
        _notes->setText(q.value("invhist_comments").toString());
        _item->setItemsiteid(q.value("invhist_itemsite_id").toInt());
        _warehouse->setId(q.value("warehous_id").toInt());
      }

      _close->setFocus();
    }
  }

  return NoError;
}

void transformTrans::sPost()
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
    { _qty->toDouble() > _fromBeforeQty->text().toDouble(),
      tr("<p>You may not transform a quantity that is greater than the "
         "quantity of the Transform Source."), _qty },
    { _target->id() < 0,
      tr("<p>>You must select a Target Item before posting this transaction."),
      _target },
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
  q.prepare( "SELECT postTransformTrans(s.itemsite_id, t.itemsite_id,"
             "                          :itemloc_id, :qty, :docnumber,"
              "                          :comments, :date) AS result "
             "FROM itemsite AS s, itemsite AS t "
             "WHERE ( (s.itemsite_warehous_id=t.itemsite_warehous_id)"
             " AND (s.itemsite_warehous_id=:warehous_id)"
             " AND (s.itemsite_item_id=:sourceItemid)"
             " AND (t.itemsite_item_id=:targetItemid) );" );
  q.bindValue(":warehous_id",  _warehouse->id());
  q.bindValue(":sourceItemid", _item->id());
  q.bindValue(":targetItemid", _target->id());
  q.bindValue(":itemloc_id",   _source->altId());
  q.bindValue(":qty",          _qty->toDouble());
  q.bindValue(":comments",     _notes->text());
  q.bindValue(":docnumber",    _documentNum->text());
  q.bindValue(":date",         _transDate->date());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("postTransformTrans", result),
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
      QMessageBox::information(this, tr("Transform Transaction"),
                               tr("Transaction Canceled") );
      return;
    }

    q.exec("COMMIT;");
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
                tr("<p>No transaction was done because either Item %1 or Item "
                   "%2 was not found at Warehouse %3.")
                .arg(_item->itemNumber(), _target->currentText(),
                     _warehouse->currentText()));
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
    _fromBeforeQty->clear();
    _fromAfterQty->clear();
    _toBeforeQty->clear();
    _toAfterQty->clear();
    _controlled = true;

    _item->setFocus();
  }
}

void transformTrans::sPopulateTarget(int /*pItemid*/)
{
  q.prepare( "SELECT item_descrip1, item_descrip2, itemsite_qtyonhand "
             "FROM item, itemsite "
             "WHERE ((item_id=itemsite_item_id)"
             "  AND  (itemsite_item_id=:item_id)"
             "  AND  (itemsite_warehous_id=:warehous_id));" );
  q.bindValue(":item_id",     _target->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (q.first())
  {
    _descrip1->setText(q.value("item_descrip1").toString());
    _descrip2->setText(q.value("item_descrip2").toString());
    _toBeforeQty->setText(formatQty(q.value("itemsite_qtyonhand").toDouble()));
    sRecalculateAfter();

  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    QMessageBox::warning(this, tr("No Transform Targets"),
                         tr("This Source Item cannot be Transformed because "
                            "it has no Transformations. Either select another "
                            "Source Item or use the Transformations tab on the "
                            "Item window to define a target Item."));
    _item->setFocus();
    return;
  }
}

void transformTrans::sFillList()
{
  _source->clear();

  q.prepare( "SELECT item_id, item_number "
             "FROM item, itemsite, itemtrans "
             "WHERE ( (itemtrans_target_item_id=item_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (itemsite_warehous_id=:warehous_id)"
             " AND (itemtrans_source_item_id=:item_id) ) "
             "ORDER BY item_number;" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  _target->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "SELECT itemsite_id,"
             "       ( (itemsite_loccntrl) OR (itemsite_controlmethod IN ('L', 'S')) ) AS detail "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (q.first())
  {
    int itemsiteid = q.value("itemsite_id").toInt();
    _controlled = q.value("detail").toBool();

    if (_controlled)
      q.prepare( "SELECT itemloc_itemsite_id AS itemsiteid, itemloc_id AS itemlocid,"
                 "       CASE WHEN (location_id IS NULL) THEN :na"
                 "            ELSE formatLocationName(location_id)"
                 "       END AS locationname,"
                 "       ls_number AS lotserial,"
                 "       itemloc_qty AS qty, 'qty' AS qty_xtnumericrole "
                 "FROM itemloc "
                 "  LEFT OUTER JOIN location ON (itemloc_location_id=location_id) "
                 "  LEFT OUTER JOIN ls ON (itemloc_ls_id=ls_id)"
                 "WHERE ( (itemloc_qty > 0)"
                 " AND (itemloc_itemsite_id=:itemsite_id) );" );
    else
      q.prepare( "SELECT itemsite_id AS itemsiteid, -1 AS itemlocid,"
                 "       TEXT(:na) AS locationname,"
                 "       TEXT(:na) AS lotserial,"
                 "       itemsite_qtyonhand AS qty, 'qty' AS qty_xtnumericrole "
                 "FROM itemsite "
                 "WHERE ( (itemsite_qtyonhand > 0)"
                 " AND (itemsite_id=:itemsite_id) );" );

    q.bindValue(":na", tr("N/A"));
    q.bindValue(":itemsite_id", itemsiteid);
    q.exec();
    _source->populate(q, true);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void  transformTrans::sPopulateQOH()
{
  if (_source->id() > -1)
  {
    _fromBeforeQty->setText(formatQty(_source->currentItem()->data(2, Qt::UserRole).toMap().value("raw").toDouble()));
    sRecalculateAfter();
  }
  else
  {
    q.prepare( "SELECT itemsite_qtyonhand "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id));" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.exec();
    if (q.first())
    {
      _fromBeforeQty->setText(formatQty(q.value("itemsite_qtyonhand").toDouble()));
      sRecalculateAfter();

    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void transformTrans::sRecalculateAfter()
{
  _fromAfterQty->setText(formatQty(_fromBeforeQty->text().toDouble() -
                                   _qty->text().toDouble()));
  _toAfterQty->setText(formatQty(_toBeforeQty->text().toDouble() +
                                   _qty->text().toDouble()));
}

void transformTrans::sHandleButtons()
{
  _post->setEnabled((! _controlled || _source->id() > -1) && _target->id() > -1);
}
