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

#include "reserveSalesOrderItem.h"

#include <QApplication>
#include <QSqlError>
#include <QVariant>
#include <QValidator>

#include <metasql.h>

#include "xmessagebox.h"
#include "storedProcErrorLookup.h"

reserveSalesOrderItem::reserveSalesOrderItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_issue, SIGNAL(clicked()), this, SLOT(sIssue()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _item->setReadOnly(TRUE);

  _qtyToIssue->setValidator(omfgThis->qtyVal());
}

reserveSalesOrderItem::~reserveSalesOrderItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void reserveSalesOrderItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse reserveSalesOrderItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    //_orderNumberLit->setText(tr("Sales Order #:"));
    populate();
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _qtyToIssue->setText(param.toString());

  return NoError;
}

void reserveSalesOrderItem::sIssue()
{
  if (_qtyToIssue->toDouble() <= 0)
  {
    QMessageBox::warning( this, tr("Invalid Quantity to Issue to Shipping"),
                          tr(  "<p>Please enter a non-negative, non-zero value to indicate the amount "
                               "of Stock you wish to Reserve for this Order Line." ));
    _qtyToIssue->setFocus();
    return;
  }

  XSqlQuery issue;
  issue.prepare("SELECT reserveSoLineQty(:lineitem_id, :qty) AS result;");
  issue.bindValue(":lineitem_id", _itemid);
  issue.bindValue(":qty", _qtyToIssue->toDouble());
  issue.exec();

  if (issue.first())
  {
    int result = issue.value("result").toInt();
    if (result < 0)
    {
      systemError( this, storedProcErrorLookup("reserveSoLineQty", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (issue.lastError().type() != QSqlError::None)
  {
    systemError(this, issue.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void reserveSalesOrderItem::populate()
{
  ParameterList itemp;
  itemp.append("soitem_id", _itemid);

  QString sql = "SELECT cohead_number AS order_number,"
                "       coitem_linenumber,"
		"       itemsite_item_id AS item_id,"
		"       warehous_code,"
                "       uom_name,"
                "       formatQty(itemsite_qtyonhand) AS qtyonhand,"
                "       formatQty(qtyReserved(itemsite_id)) AS totreserved,"
                "       formatQty(qtyUnreserved(itemsite_id)) AS totunreserved,"
		"       formatQty(coitem_qtyord) AS qtyordered,"
		"       formatQty(coitem_qtyshipped) AS qtyshipped,"
		"       formatQty(coitem_qtyreturned) AS qtyreturned,"
                "       formatQty(coitem_qtyreserved) AS qtyreserved,"
		"       formatQty(noNeg(coitem_qtyord - coitem_qtyshipped +"
		"                       coitem_qtyreturned - coitem_qtyreserved)) AS balance "
		"FROM cohead, coitem, itemsite, item, warehous, uom "
		"WHERE ((coitem_cohead_id=cohead_id)"
		"  AND  (coitem_itemsite_id=itemsite_id)"
		"  AND  (coitem_status <> 'X')"
                "  AND  (coitem_qty_uom_id=uom_id)"
		"  AND  (itemsite_item_id=item_id)"
		"  AND  (itemsite_warehous_id=warehous_id)"
		"  AND  (coitem_id=<? value(\"soitem_id\") ?>) );";

  MetaSQLQuery itemm(sql);
  XSqlQuery itemq = itemm.toQuery(itemp);

  if (itemq.first())
  {
    _salesOrderNumber->setText(itemq.value("order_number").toString());
    _salesOrderLine->setText(itemq.value("coitem_linenumber").toString());
    _item->setId(itemq.value("item_id").toInt());
    _warehouse->setText(itemq.value("warehous_code").toString());
    _qtyUOM->setText(itemq.value("uom_name").toString());
    _qtyOrdered->setText(itemq.value("qtyordered").toString());
    _qtyShipped->setText(itemq.value("qtyshipped").toString());
    _balance->setText(itemq.value("balance").toString());
    _reserved->setText(itemq.value("qtyreserved").toString());
    _onHand->setText(itemq.value("qtyonhand").toString());
    _allocated->setText(itemq.value("totreserved").toString());
    _unreserved->setText(itemq.value("totunreserved").toString());
  }
  else if (itemq.lastError().type() != QSqlError::None)
  {
    systemError(this, itemq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _qtyToIssue->setText(itemq.value("balance").toString());
}
