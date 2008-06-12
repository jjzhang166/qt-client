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

#include "issueWoMaterialBatch.h"

#include <QVariant>
#include <QMessageBox>
#include "inputManager.h"
#include "distributeInventory.h"

/*
 *  Constructs a issueWoMaterialBatch as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
issueWoMaterialBatch::issueWoMaterialBatch(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_issue, SIGNAL(clicked()), this, SLOT(sIssue()));
    connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
issueWoMaterialBatch::~issueWoMaterialBatch()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void issueWoMaterialBatch::languageChange()
{
    retranslateUi(this);
}


void issueWoMaterialBatch::init()
{
  _hasPush = FALSE;

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _womatl->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft   );
  _womatl->addColumn(tr("Description"),  -1,          Qt::AlignLeft   );
  _womatl->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter );
  _womatl->addColumn(tr("Issue Method"), _itemColumn, Qt::AlignCenter );
  _womatl->addColumn(tr("Required"),     _qtyColumn,  Qt::AlignRight  );
  _womatl->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight  );
  _womatl->addColumn(tr("Short"),        _qtyColumn,  Qt::AlignRight  );
}

enum SetResponse issueWoMaterialBatch::set(ParameterList &pParams)
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

void issueWoMaterialBatch::sIssue()
{
  XSqlQuery issue;
  issue.prepare("SELECT itemsite_id, "
                "       item_number, "
                "       warehous_code, "
                "       (COALESCE((SELECT SUM(itemloc_qty) "
                "                    FROM itemloc "
                "                   WHERE (itemloc_itemsite_id=itemsite_id)), 0.0) "
                "        >= roundQty(item_fractional, noNeg(itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, womatl_qtyreq - womatl_qtyiss)))) AS isqtyavail "
                "  FROM womatl, itemsite, item, warehous "
                " WHERE ( (womatl_itemsite_id=itemsite_id) "
                "   AND (itemsite_item_id=item_id) "
                "   AND (itemsite_warehous_id=warehous_id) "
                "   AND (NOT ((item_type = 'R') OR (itemsite_controlmethod = 'N'))) "
                "   AND ((itemsite_controlmethod IN ('L', 'S')) OR (itemsite_loccntrl)) "
                "   AND (womatl_issuemethod IN ('S', 'M')) "
                "   AND (womatl_wo_id=:wo_id)); ");
  issue.bindValue(":wo_id", _wo->id());
  issue.exec();
  while(issue.next())
  {
    if(!(issue.value("isqtyavail").toBool()))
    {
      QMessageBox::critical(this, tr("Insufficient Inventory"),
        tr("Item Number %1 in Warehouse %2 is a Multiple Location or\n"
           "Lot/Serial controlled Item which is short on Inventory.\n"
           "This transaction cannot be completed as is. Please make\n"
           "sure there is sufficient Quantity on Hand before proceeding.")
          .arg(issue.value("item_number").toString())
          .arg(issue.value("warehous_code").toString()));
      return;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  issue.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  issue.prepare("SELECT issueWoMaterialBatch(:wo_id) AS result;");
  issue.bindValue(":wo_id", _wo->id());
  issue.exec();
  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);

  if (issue.first())
  {
    if (issue.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at issueWoMaterialBatch::%1, Work Order ID #%2, Error #%3.")
                         .arg(__LINE__)
                         .arg(_wo->id())
                         .arg(issue.value("result").toInt()) );
      return;
    }
    else
    {
      if (distributeInventory::SeriesAdjust(issue.value("result").toInt(), this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Material Issue"), tr("Transaction Canceled") );
        return;
      }

      issue.exec("COMMIT;");

      if (_captive)
        accept();
      else
      {
        _wo->setId(-1);
        _wo->setFocus();
      }
    }
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at issueWoMaterialBatch::%1, Work Order ID #%2.")
                       .arg(__LINE__)
                       .arg(_wo->id()) );
    return;
  }
}

void issueWoMaterialBatch::sFillList()
{
  _womatl->clear();
  _hasPush = FALSE;

  if (_wo->isValid())
  {
    XSqlQuery womatl;
    womatl.prepare( "SELECT womatl_id, item_number,"
                    " (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, uom_name,"
                    " CASE WHEN (womatl_issuemethod = 'S') THEN :push"
                    "      WHEN (womatl_issuemethod = 'L') THEN :pull"
                    "      WHEN (womatl_issuemethod = 'M') THEN :mixed"
                    "      ELSE :error"
                    " END AS issuemethod,"
                    " formatQty(noNeg(womatl_qtyreq - womatl_qtyiss)) AS required,"
                    " formatQty(itemsite_qtyonhand) AS qoh,"
                    " formatQty(abs(noneg((itemsite_qtyonhand - womatl_qtyreq) * -1))) AS short "
                    "FROM womatl, itemsite, item, uom "
                    "WHERE ((womatl_itemsite_id=itemsite_id)"
                    " AND (itemsite_item_id=item_id)"
                    " AND (womatl_uom_id=uom_id)"
                    " AND (womatl_wo_id=:wo_id)) "
                    "ORDER BY item_number;" );
    womatl.bindValue(":push", tr("Push"));
    womatl.bindValue(":pull", tr("Pull"));
    womatl.bindValue(":mixed", tr("Mixed"));
    womatl.bindValue(":error", tr("Error"));
    womatl.bindValue(":wo_id", _wo->id());
    womatl.exec();

    XTreeWidgetItem *last = 0;
    while (womatl.next())
    {
      last = new XTreeWidgetItem(_womatl, last,
				 womatl.value("womatl_id").toInt(),
				 womatl.value("item_number"),
				 womatl.value("itemdescrip"),
				 womatl.value("uom_name"),
				 womatl.value("issuemethod"),
				 womatl.value("required"),
				 womatl.value("qoh"),
				 womatl.value("short") );

      if (womatl.value("issuemethod") == "Pull")
      {
	// TODO: there's no equivalent to setSelectable with QTreeView
	// does it matter here?
        // last->setSelectable(FALSE);
        last->setTextColor("blue");
      }
      else
      {
        _hasPush = TRUE;

        if (womatl.value("short").toDouble() > 0.0)
          last->setTextColor(6, "red");
      }
    }
  }

  _issue->setEnabled(_hasPush);
}
