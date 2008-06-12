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

#include "issueWoMaterialItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

#include "inputManager.h"
#include "distributeInventory.h"

/*
 *  Constructs a issueWoMaterialItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
issueWoMaterialItem::issueWoMaterialItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_wo, SIGNAL(valid(bool)), _issue, SLOT(setEnabled(bool)));
    connect(_wo, SIGNAL(newId(int)), this, SLOT(sPopulateComp(int)));
    connect(_compItemNumber, SIGNAL(newID(int)), this, SLOT(sPopulateCompInfo(int)));
    connect(_qtyToIssue, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQOH()));
    connect(_issue, SIGNAL(clicked()), this, SLOT(sIssue()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

    _captive = FALSE;

    omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));
    omfgThis->inputManager()->notify(cBCItem, this, this, SLOT(sCatchItemid(int)));
    omfgThis->inputManager()->notify(cBCItemSite, this, this, SLOT(sCatchItemsiteid(int)));

    _wo->setType(cWoExploded | cWoIssued | cWoReleased);
    _compItemNumber->setAllowNull(TRUE);
    _qtyToIssue->setValidator(omfgThis->qtyVal());
}

/*
 *  Destroys the object and frees any allocated resources
 */
issueWoMaterialItem::~issueWoMaterialItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void issueWoMaterialItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse issueWoMaterialItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);
    _issue->setFocus();
  }

  return NoError;
}

void issueWoMaterialItem::sCatchItemid(int pItemid)
{
  if (_wo->isValid())
  {
    q.prepare( "SELECT womatl_id "
               "FROM womatl, itemsite "
               "WHERE ( (womatl_itemsite_id=itemsite_id)"
               " AND (womatl_wo_id=:wo_id)"
               " AND (itemsite_item_id=:item_id) );" );
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":item_id", pItemid);
    q.exec();
    if (q.first())
      _compItemNumber->setId(q.value("womatl_id").toInt());
    else
      audioReject();
  }
  else
    audioReject();
}

void issueWoMaterialItem::sCatchItemsiteid(int pItemsiteid)
{
  if (_wo->isValid())
  {
    q.prepare( "SELECT womatl_id "
               "FROM womatl "
               "WHERE ((womatl_itemsite_id=:itemsite_id)"
	       "  AND  (womatl_wo_id=:wo_id));" );
    q.bindValue(":itemsite_id", pItemsiteid);
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (q.first())
      _compItemNumber->setId(q.value("womatl_id").toInt());
    else
      audioReject();
  }
  else
    audioReject();
}

void issueWoMaterialItem::sIssue()
{
  q.prepare("SELECT itemsite_id, item_number, warehous_code, "
            "       (COALESCE((SELECT SUM(itemloc_qty) "
            "                    FROM itemloc "
            "                   WHERE (itemloc_itemsite_id=itemsite_id)), 0.0) >= roundQty(item_fractional, itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, :qty))) AS isqtyavail "
            "  FROM womatl, itemsite, item, warehous "
            " WHERE ((womatl_itemsite_id=itemsite_id) "
            "   AND (itemsite_item_id=item_id) "
            "   AND (itemsite_warehous_id=warehous_id) "
            "   AND (NOT ((item_type = 'R') OR (itemsite_controlmethod = 'N'))) "
            "   AND ((itemsite_controlmethod IN ('L', 'S')) OR (itemsite_loccntrl)) "
            "   AND (womatl_id=:womatl_id)); ");
  q.bindValue(":womatl_id", _compItemNumber->id());
  q.bindValue(":qty", _qtyToIssue->toDouble());
  q.exec();
  while(q.next())
  {
    if(!(q.value("isqtyavail").toBool()))
    {
      QMessageBox::critical(this, tr("Insufficient Inventory"),
        tr("Item Number %1 in Warehouse %2 is a Multiple Location or\n"
           "Lot/Serial controlled Item which is short on Inventory.\n"
           "This transaction cannot be completed as is. Please make\n"
           "sure there is sufficient Quantity on Hand before proceeding.")
          .arg(q.value("item_number").toString())
          .arg(q.value("warehous_code").toString()));
      return;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT issueWoMaterial(:womatl_id, :qty, TRUE) AS result;");
  q.bindValue(":womatl_id", _compItemNumber->id());
  q.bindValue(":qty", _qtyToIssue->toDouble());
  q.exec();
  if (q.first())
  {
    if (q.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at issueWoMaterialItem::%1, Work Order ID #%2, Error #%3.")
                         .arg(__LINE__)
                         .arg(_wo->id())
                         .arg(q.value("result").toInt()) );
      return;
    }
    else if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Material Issue"), tr("Transaction Canceled") );
      return;
    }

    q.exec("COMMIT;");
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at issueWoMaterialItem::%1, Work Order ID #%2.")
                       .arg(__LINE__)
                       .arg(_wo->id()) );
    return;
  }

  if (_captive)
    close();
  else
  {
    _close->setText(tr("Close"));
    _qtyToIssue->clear();
    _compItemNumber->setNull();
    _compItemNumber->setFocus();
  }
}

void issueWoMaterialItem::sPopulateComp(int pWoid)
{
  q.prepare( "SELECT womatl_id, item_number "
             "FROM womatl, itemsite, item "
             "WHERE ( (womatl_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (womatl_issuemethod IN ('S', 'M'))"
             " AND (womatl_wo_id=:wo_id) );" );
  q.bindValue(":wo_id", pWoid);
  q.exec();
  _compItemNumber->populate(q);
  _compItemNumber->setFocus();
}

void issueWoMaterialItem::sPopulateCompInfo(int pWomatlid)
{
  if (pWomatlid != -1)
  {
    q.prepare( "SELECT item_descrip1, item_descrip2, uom_name, itemuomtouom(itemsite_item_id, NULL, womatl_uom_id, itemsite_qtyonhand) AS qtyonhand,"
               "       formatQty(womatl_qtyreq) AS qtyreq,"
               "       formatQty(womatl_qtyiss) AS qtyiss,"
               "       formatQty(noNeg(womatl_qtyreq - womatl_qtyiss)) AS qtybalance "
               "FROM womatl, itemsite, item, uom "
               "WHERE ( (womatl_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (womatl_uom_id=uom_id)"
               " AND (womatl_id=:womatl_id) );" );
    q.bindValue(":womatl_id", pWomatlid);
    q.exec();
    if (q.first())
    {
      _compDescription1->setText(q.value("item_descrip1").toString());
      _compDescription2->setText(q.value("item_descrip2").toString());
      _compUOM->setText(q.value("uom_name").toString());
      _uomQty->setText(q.value("uom_name").toString());
      _qtyRequired->setText(q.value("qtyreq").toString());
      _qtyIssued->setText(q.value("qtyiss").toString());
      _qtyToIssue->setText(q.value("qtybalance").toString());
      
      _cachedQOH = q.value("qtyonhand").toDouble();
      _beforeQty->setText(formatQty(_cachedQOH));

      sPopulateQOH();

      _qtyToIssue->setFocus();

      return;
    }
  }
  else
  {
    _compDescription1->clear();
    _compDescription2->clear();
    _compUOM->clear();
    _uomQty->clear();
    _qtyRequired->clear();
    _qtyIssued->clear();
    _qtyToIssue->clear();

    _cachedQOH = 0.0;
    _beforeQty->clear();
    _afterQty->clear();
  }
}

void issueWoMaterialItem::sPopulateQOH()
{
  _afterQty->setText(formatQty(_cachedQOH - _qtyToIssue->toDouble()));
}

