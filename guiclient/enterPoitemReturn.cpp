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

#include "enterPoitemReturn.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include "storedProcErrorLookup.h"

enterPoitemReturn::enterPoitemReturn(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_return, SIGNAL(clicked()), this, SLOT(sReturn()));

  _toReturn->setValidator(omfgThis->qtyVal());
  _item->setReadOnly(TRUE);

  _rejectCode->setAllowNull(TRUE);
  _rejectCode->populate( "SELECT rjctcode_id, rjctcode_code "
                         "FROM rjctcode "
                         "ORDER BY rjctcode_code;" );
}

enterPoitemReturn::~enterPoitemReturn()
{
  // no need to delete child widgets, Qt does it all for us
}

void enterPoitemReturn::languageChange()
{
  retranslateUi(this);
}

enum SetResponse enterPoitemReturn::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    _poitemid = param.toInt();

    q.prepare( "SELECT pohead_number, poitem_linenumber,"
               "       COALESCE(itemsite_id, -1) AS itemsiteid,"
               "       noNeg(poitem_qty_received - poitem_qty_returned) AS returnable,"
               "       poitem_vend_item_number, poitem_vend_uom, poitem_vend_item_descrip,"
               "       formatQty(poitem_qty_ordered) AS f_qtyordered,"
               "       formatQty(noNeg(poitem_qty_received - poitem_qty_returned)) AS f_returnableqty,"
               "       formatRatio(poitem_invvenduomratio) AS invvenduomratio "
               "FROM pohead, poitem LEFT OUTER JOIN "
               "             ( itemsite JOIN item "
               "               ON (itemsite_item_id=item_id)"
               "             ) ON (poitem_itemsite_id=itemsite_id) "
               "WHERE ( (poitem_pohead_id=pohead_id)"
               " AND (poitem_id=:poitem_id) );" );
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    if (q.first())
    {
      _poNumber->setText(q.value("pohead_number").toString());
      _lineNumber->setText(q.value("poitem_linenumber").toString());
      _vendorItemNumber->setText(q.value("poitem_vend_item_number").toString());
      _vendorDescrip->setText(q.value("poitem_vend_item_descrip").toString());
      _vendorUOM->setText(q.value("poitem_vend_uom").toString());
      _ordered->setText(q.value("f_qtyordered").toString());
      _received->setText(q.value("f_returnableqty").toString());
      _invVendorUOMRatio->setText(q.value("invvenduomratio").toString());

      if (q.value("itemsiteid").toInt() != -1)
        _item->setItemsiteid(q.value("itemsiteid").toInt());

      _cachedReceived = q.value("returnable").toDouble();

      q.prepare( "SELECT COALESCE(SUM(poreject_qty), 0) AS qtytoreturn "
                 "FROM poreject "
                 "WHERE ( (poreject_poitem_id=:poitem_id)"
                 " AND (NOT poreject_posted) );" );
      q.bindValue(":poitem_id", _poitemid);
      q.exec();
      if (q.first())
        _toReturn->setText(q.value("qtytoreturn").toString());
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  return NoError;
}

void enterPoitemReturn::sReturn()
{
  if (_rejectCode->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Enter Return"),
                           tr("You may not enter this return until you select a Reject Code.\n") );
    _rejectCode->setFocus();
    return;
  }

  if (_cachedReceived < _toReturn->toDouble())
  {
    QMessageBox::critical( this, tr("Cannot Enter Return"),
                           tr("You may not enter a return whose returned quantity is greater than the returnable quantity.") );
    _toReturn->setFocus();
    return;
  }

  if (_toReturn->toDouble() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Enter Return"),
                           tr("You must enter a quantity to return.") );
    _toReturn->setFocus();
    return;
  }

  q.prepare("SELECT enterPoReturn(:poitem_id, :qty, :rjctcode_id) AS result;");
  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":qty", _toReturn->toDouble());
  q.bindValue(":rjctcode_id", _rejectCode->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      QMessageBox::critical(this, tr("Cannot Enter Return"),
			    storedProcErrorLookup("enterPoReturn", result));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}
