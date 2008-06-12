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

#include "woMaterialItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "inputManager.h"

/*
 *  Constructs a woMaterialItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
woMaterialItem::woMaterialItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  _bomitemid=-1;


  // signals and slots connections
  connect(_qtyPer, SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQtyRequired()));
  connect(_scrap, SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQtyRequired()));
  connect(_item, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sItemIdChanged()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _captive = FALSE;

  _wo->setType(cWoOpen | cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _qtyPer->setValidator(omfgThis->qtyPerVal());
  _scrap->setValidator(omfgThis->scrapVal());

  QString issueMethod = _metrics->value("DefaultWomatlIssueMethod");
  if (issueMethod == "S")
    _issueMethod->setCurrentItem(0);
  else if (issueMethod == "L")
    _issueMethod->setCurrentItem(1);
  else if (issueMethod == "M")
    _issueMethod->setCurrentItem(2);
}

/*
 *  Destroys the object and frees any allocated resources
 */
woMaterialItem::~woMaterialItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void woMaterialItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse woMaterialItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);

    _item->setFocus();
  }
  
  param = pParams.value("bomitem_id", &valid);
  if (valid)
    _bomitemid=param.toInt();

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("qtyPer", &valid);
  if (valid)
    _qtyPer->setText(formatQtyPer(param.toDouble()));

  param = pParams.value("uom_id", &valid);
  if (valid)
    _uom->setId(param.toInt());

  param = pParams.value("scrap", &valid);
  if (valid)
    _scrap->setText(formatPercent(param.toDouble()));

  param = pParams.value("issueMethod", &valid);
  if (valid)
  {
    if (param.toString() == "push")
      _issueMethod->setCurrentItem(0);
    else if (param.toString() == "pull")
      _issueMethod->setCurrentItem(1);
    else if (param.toString() == "mixed")
      _issueMethod->setCurrentItem(2);
  }

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _wo->setEnabled(FALSE);
    _item->setEnabled(FALSE);

    _womatlid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _wo->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _wo->setEnabled(FALSE);
      _item->setEnabled(FALSE);
      _qtyPer->setEnabled(FALSE);
      _uom->setEnabled(FALSE);
      _scrap->setEnabled(FALSE);
      _issueMethod->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void woMaterialItem::sSave()
{
  QString issueMethod;

  switch (_issueMethod->currentItem())
  {
    case 0:
      issueMethod = 'S';
      break;

    case 1:
      issueMethod = 'L';
      break;

    case 2:
      issueMethod = 'M';
      break;
  }

  if (_mode == cNew)
  {
    q.prepare( "SELECT component.itemsite_id AS itemsiteid "
               "FROM wo, itemsite AS parent, itemsite AS component "
               "WHERE ( (parent.itemsite_warehous_id=component.itemsite_warehous_id)"
               " AND (parent.itemsite_id=wo_itemsite_id)"
               " AND (component.itemsite_item_id=:item_id)"
               " AND (wo_id=:wo_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (!q.first())
    {
      QMessageBox::warning( this, tr("Cannot Create W/O Material Requirement"),
                            tr( "A W/O Material Requirement cannot be created for the selected\n"
                                "Work Order/Item as the selected Item does not exist in the warehouse\n"
                                "that the selected Work Order does." ));
      _item->setId(-1);
      _item->setFocus();
      return;
    }

    int itemsiteid = q.value("itemsiteid").toInt();

    q.prepare("SELECT createWoMaterial(:wo_id, :itemsite_id, :issueMethod, :uom_id, :qtyPer, :scrap, :bomitem_id) AS womatlid;");
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":itemsite_id", itemsiteid);
    q.bindValue(":issueMethod", issueMethod);
    q.bindValue(":qtyPer", _qtyPer->toDouble());
    q.bindValue(":uom_id", _uom->id());
    q.bindValue(":scrap", (_scrap->toDouble() / 100));
    q.bindValue(":bomitem_id", _bomitemid);
    q.exec();
    if (q.first())
      _womatlid = q.value("womatlid").toInt();
//  ToDo
  }
  else if (_mode == cEdit)
  {
    q.prepare( "UPDATE womatl "
               "SET womatl_qtyper=:qtyPer, womatl_scrap=:scrap, womatl_issuemethod=:issueMethod,"
               "    womatl_uom_id=:uom_id,"
               "    womatl_qtyreq=(:qtyPer * (1 + :scrap) * wo_qtyord) "
               "FROM wo "
               "WHERE ( (womatl_wo_id=wo_id)"
               " AND (womatl_id=:womatl_id) );" );
    q.bindValue(":womatl_id", _womatlid);
    q.bindValue(":issueMethod", issueMethod);
    q.bindValue(":qtyPer", _qtyPer->toDouble());
    q.bindValue(":uom_id", _uom->id());
    q.bindValue(":scrap", (_scrap->toDouble() / 100));
    q.exec();
  }

  omfgThis->sWorkOrderMaterialsUpdated(_wo->id(), _womatlid, TRUE);

  if (_captive)
    done(_womatlid);
  else
  {
    _item->setId(-1);
    _qtyPer->clear();
    _qtyRequired->clear();
    _scrap->clear();
    _item->setFocus();
  }
}

void woMaterialItem::sUpdateQtyRequired()
{
  _qtyRequired->setText(formatQty(_wo->qtyOrdered() * (_qtyPer->toDouble() * (1 + (_scrap->toDouble() / 100)))));
}

void woMaterialItem::populate()
{
  q.prepare( "SELECT womatl_wo_id, itemsite_item_id,"
             "       formatQtyPer(womatl_qtyper) AS qtyper,"
             "       womatl_uom_id,"
             "       formatScrap(womatl_scrap) AS scrap,"
             "       womatl_issuemethod "
             "FROM womatl, itemsite "
             "WHERE ( (womatl_itemsite_id=itemsite_id)"
             " AND (womatl_id=:womatl_id) );" );
  q.bindValue(":womatl_id", _womatlid);
  q.exec();
  if (q.first())
  {
    _wo->setId(q.value("womatl_wo_id").toInt());
    _item->setId(q.value("itemsite_item_id").toInt());
    _qtyPer->setText(q.value("qtyper").toString());
    _uom->setId(q.value("womatl_uom_id").toInt());
    _scrap->setText(q.value("scrap").toString());

    if (q.value("womatl_issuemethod").toString() == "S")
      _issueMethod->setCurrentItem(0);
    else if (q.value("womatl_issuemethod").toString() == "L")
      _issueMethod->setCurrentItem(1);
    else if (q.value("womatl_issuemethod").toString() == "M")
      _issueMethod->setCurrentItem(2);
  }
}

void woMaterialItem::sItemIdChanged()
{
  XSqlQuery uom;
  uom.prepare("SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN uom ON (item_inv_uom_id=uom_id)"
              " WHERE(item_id=:item_id)"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_to_uom_id=uom_id)"
              " WHERE((itemuomconv_from_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_from_uom_id=uom_id)"
              " WHERE((itemuomconv_to_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " ORDER BY uom_name;");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  _uom->populate(uom);
  uom.prepare("SELECT item_inv_uom_id"
              "  FROM item"
              " WHERE(item_id=:item_id);");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  if(uom.first())
    _uom->setId(uom.value("item_inv_uom_id").toInt());
}

