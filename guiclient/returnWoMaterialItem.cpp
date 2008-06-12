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

#include "returnWoMaterialItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"

returnWoMaterialItem::returnWoMaterialItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_return, SIGNAL(clicked()), this, SLOT(sReturn()));
  connect(_womatl, SIGNAL(newId(int)), this, SLOT(sSetQOH(int)));
  connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQty()));

  _captive = FALSE;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoExploded | cWoReleased | cWoIssued);
  _qty->setValidator(omfgThis->qtyVal());
}

returnWoMaterialItem::~returnWoMaterialItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void returnWoMaterialItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse returnWoMaterialItem::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    q.prepare("SELECT womatl_wo_id FROM womatl WHERE (womatl_id=:womatl_id); ");
    q.bindValue(":womatl_id", param.toInt());
    q.exec();
    if(q.first())
      _wo->setId(q.value("womatl_wo_id").toInt());

    _womatl->setId(param.toInt());
    _qty->setFocus();
  }

  return NoError;
}

void returnWoMaterialItem::sReturn()
{
  if (!_wo->isValid())
  {
    QMessageBox::critical( this, tr("Select Work Order"),
                           tr("You must select the Work Order from which you with to return Materal") );
    _wo->setFocus();
    return;
  }

  if (_qty->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Quantity to Return"),
                           tr( "You must enter a positive quantity, less than the amount issued,\n"
                               "to return from the selected W/O to Inventory." ) );
    _qty->setFocus();
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery returnItem;
  returnItem.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  returnItem.prepare("SELECT returnWoMaterial(:womatl_id, :qty) AS result;");
  returnItem.bindValue(":womatl_id", _womatl->id());
  returnItem.bindValue(":qty", _qty->toDouble());
  returnItem.exec();
  if (returnItem.first())
  {
    int result = returnItem.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("returnWoMaterial", result),
                  __FILE__, __LINE__);
      return;
    }
    else if (distributeInventory::SeriesAdjust(returnItem.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Material Return"), tr("Transaction Canceled") );
      return;
    }

    returnItem.exec("COMMIT;");
  }
  else if (returnItem.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, returnItem.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
  {
    _qty->clear();
    _close->setText(tr("&Close"));
    _womatl->setId(-1);
    _womatl->setFocus();
  }
}

void returnWoMaterialItem::sSetQOH(int pWomatlid)
{
  if (pWomatlid == -1)
  {
    _cachedQOH = 0.0;
    _beforeQty->clear();
    _afterQty->clear();
  }
  else
  {
    XSqlQuery qoh;
    qoh.prepare( "SELECT itemuomtouom(itemsite_item_id, NULL, womatl_uom_id, itemsite_qtyonhand) AS qtyonhand,"
                 "       uom_name "
                 "  FROM womatl, itemsite, uom"
                 " WHERE((womatl_itemsite_id=itemsite_id)"
                 "   AND (womatl_uom_id=uom_id)"
                 "   AND (womatl_id=:womatl_id) );" );
    qoh.bindValue(":womatl_id", pWomatlid);
    qoh.exec();
    if (qoh.first())
    {
      _uom->setText(qoh.value("uom_name").toString());
      _cachedQOH = qoh.value("qtyonhand").toDouble();
      _beforeQty->setText(formatQty(_cachedQOH));
    }
    else
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
  }
}

void returnWoMaterialItem::sUpdateQty()
{
  if (_womatl->isValid())
    _afterQty->setText(formatQty(_cachedQOH + _qty->toDouble()));
}

