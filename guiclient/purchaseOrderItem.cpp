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

#include "purchaseOrderItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

#include "itemCharacteristicDelegate.h"
#include "itemSourceSearch.h"

purchaseOrderItem::purchaseOrderItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

#ifndef Q_WS_MAC
  _vendorItemNumberList->setMaximumWidth(25);
#endif

  connect(_ordered, SIGNAL(lostFocus()), this, SLOT(sDeterminePrice()));
  connect(_inventoryItem, SIGNAL(toggled(bool)), this, SLOT(sInventoryItemToggled(bool)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateItemSourceInfo(int)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_ordered, SIGNAL(lostFocus()), this, SLOT(sUpdateVendorQty()));
  connect(_vendorItemNumberList, SIGNAL(clicked()), this, SLOT(sVendorItemNumberList()));

  _parentwo = -1;
  _parentso = -1;

  _overriddenUnitPrice = false;

  _ordered->setValidator(omfgThis->qtyVal());

  _project->setType(ProjectLineEdit::PurchaseOrder);
  if(!_metrics->boolean("UseProjects"))
    _project->hide();

  _itemchar = new QStandardItemModel(0, 2, this);
  _itemchar->setHeaderData( 0, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
  _itemchar->setHeaderData( 1, Qt::Horizontal, tr("Value"), Qt::DisplayRole);

  _itemcharView->setModel(_itemchar);
  ItemCharacteristicDelegate * delegate = new ItemCharacteristicDelegate(this);
  _itemcharView->setItemDelegate(delegate);

  _minOrderQty->setValidator(omfgThis->qtyVal());
  _orderQtyMult->setValidator(omfgThis->qtyVal());
  _received->setValidator(omfgThis->qtyVal());
  _invVendorUOMRatio->setValidator(omfgThis->ratioVal());

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  //If not Revision Control, hide controls
  if (!_metrics->boolean("RevControl"))
   _tab->removePage(_tab->page(4));
   
  resize(minimumSize());
}

/*
 *  Destroys the object and frees any allocated resources
 */
purchaseOrderItem::~purchaseOrderItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void purchaseOrderItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse purchaseOrderItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  bool     haveQty  = FALSE;
  bool     haveDate = FALSE;

  param = pParams.value("parentWo", &valid);
  if (valid)
    _parentwo = param.toInt();

  param = pParams.value("parentSo", &valid);
  if (valid)
    _parentso = param.toInt();

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _poheadid = param.toInt();

    q.prepare( "SELECT pohead_number, pohead_orderdate, pohead_status, "
               "       vend_id, vend_restrictpurch, pohead_curr_id "
               "FROM pohead, vend "
               "WHERE ( (pohead_vend_id=vend_id)"
               " AND (pohead_id=:pohead_id) );" );
    q.bindValue(":pohead_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _poNumber->setText(q.value("pohead_number").toString());
      _poStatus = q.value("pohead_status").toString();
      _unitPrice->setEffective(q.value("pohead_orderdate").toDate());
      _unitPrice->setId(q.value("pohead_curr_id").toInt());

      if (q.value("vend_restrictpurch").toBool())
      {
        _item->setQuery( QString( "SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                                  "                uom_name, item_type, item_config "
                                  "FROM item, itemsite, itemsrc, uom  "
                                  "WHERE ( (itemsite_item_id=item_id)"
                                  " AND (itemsrc_item_id=item_id)"
                                  " AND (item_inv_uom_id=uom_id)"
                                  " AND (itemsite_active)"
                                  " AND (item_active)"
                                  " AND (itemsrc_active)"
                                  " AND (itemsrc_vend_id=%1) ) "
                                  "ORDER BY item_number;" )
                         .arg(q.value("vend_id").toInt()) );
        _item->setValidationQuery( QString( "SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                                            "                uom_name, item_type, item_config "
                                            "FROM item, itemsite, itemsrc, uom  "
                                            "WHERE ( (itemsite_item_id=item_id)"
                                            " AND (itemsrc_item_id=item_id)"
                                            " AND (item_inv_uom_id=uom_id)"
                                            " AND (itemsite_active)"
                                            " AND (item_active)"
                                            " AND (itemsrc_active)"
                                            " AND (itemsrc_vend_id=%1) "
                                            " AND (itemsite_item_id=:item_id) ) "
                                            "ORDER BY item_number;" )
                                   .arg(q.value("vend_id").toInt()) );
      }
      else
        _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cActive);
    }
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return UndefinedError;
    }
  }

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    _poitemid = param.toInt();

    q.prepare( "SELECT pohead_number "
               "FROM pohead, poitem "
               "WHERE ( (pohead_id=poitem_pohead_id) "
               " AND (poitem_id=:poitem_id) );" );
    q.bindValue(":poitem_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _poNumber->setText(q.value("pohead_number").toString());
    }

    populate();
  }
  // connect here and not in the .ui to avoid timing issues at initialization
  connect(_unitPrice, SIGNAL(valueChanged()), this, SLOT(sPopulateExtPrice()));

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('poitem_poitem_id_seq') AS poitem_id;");
      if (q.first())
        _poitemid = q.value("poitem_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }

      if(_parentso != -1)
      {
        q.prepare( "INSERT INTO charass"
                   "      (charass_target_type, charass_target_id,"
                   "       charass_char_id, charass_value) "
                   "SELECT 'PI', :orderid, charass_char_id, charass_value"
                   "  FROM charass"
                   " WHERE ((charass_target_type='SI')"
                   "   AND  (charass_target_id=:soitem_id));");
        q.bindValue(":orderid", _poitemid);
        q.bindValue(":soitem_id", _parentso);
        q.exec();
      }

      q.prepare( "SELECT (COALESCE(MAX(poitem_linenumber), 0) + 1) AS _linenumber "
                 "FROM poitem "
                 "WHERE (poitem_pohead_id=:pohead_id);" );
      q.bindValue(":pohead_id", _poheadid);
      q.exec();
      if (q.first())
        _lineNumber->setText(q.value("_linenumber").toString());
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );

        return UndefinedError;
      }

      if (!_item->isValid())
        _item->setFocus();
      else if (!haveQty)
        _ordered->setFocus();
      else if (!haveDate)
        _dueDate->setFocus();

      _comments->setEnabled(FALSE);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _typeGroup->setEnabled(FALSE);

      _ordered->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _typeGroup->setEnabled(FALSE);
      _vendorItemNumber->setEnabled(FALSE);
      _vendorItemNumberList->setEnabled(FALSE);
      _vendorDescrip->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _ordered->setEnabled(FALSE);
      _unitPrice->setEnabled(FALSE);
      _freight->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _comments->setReadOnly(TRUE);
      _project->setEnabled(FALSE);

      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);

    q.prepare( "SELECT DISTINCT char_id, char_name,"
               "       COALESCE(b.charass_value, (SELECT c.charass_value FROM charass c WHERE ((c.charass_target_type='I') AND (c.charass_target_id=:item_id) AND (c.charass_default) AND (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "  FROM charass a, char "
               "    LEFT OUTER JOIN charass b"
               "      ON (b.charass_target_type='PI'"
               "      AND b.charass_target_id=:poitem_id"
               "      AND b.charass_char_id=char_id) "
               " WHERE ( (a.charass_char_id=char_id)"
               "   AND   (a.charass_target_type='I')"
               "   AND   (a.charass_target_id=:item_id) ) "
               " ORDER BY char_name;" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    int row = 0;
    QModelIndex idx;
    while(q.next())
    {
      _itemchar->insertRow(_itemchar->rowCount());
      idx = _itemchar->index(row, 0);
      _itemchar->setData(idx, q.value("char_name"), Qt::DisplayRole);
      _itemchar->setData(idx, q.value("char_id"), Qt::UserRole);
      idx = _itemchar->index(row, 1);
      _itemchar->setData(idx, q.value("charass_value"), Qt::DisplayRole);
      _itemchar->setData(idx, _item->id(), Qt::UserRole);
      row++;
    }
  }

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _ordered->setDouble((param.toDouble()/_invVendUOMRatio));

    if (_item->isValid())
      sDeterminePrice();

    haveQty = TRUE;
  }

  param = pParams.value("dueDate", &valid);
  if (valid)
  {
    _dueDate->setDate(param.toDate());
    haveDate = TRUE;
  }

  param = pParams.value("prj_id", &valid);
  if (valid)
    _project->setId(param.toInt());

  if(_parentso != -1)
  {
    q.prepare("SELECT coitem_prcost"
              "  FROM coitem"
              " WHERE (coitem_id=:parentso); ");
    q.bindValue(":parentso", _parentso);
    q.exec();
    if(q.first())
    {
      if(q.value("coitem_prcost").toDouble() > 0)
      {
        _overriddenUnitPrice = true;
        _unitPrice->setLocalValue(q.value("coitem_prcost").toDouble());
        sPopulateExtPrice();
      }
    }
  }

  return NoError;
}

void purchaseOrderItem::populate()
{
  q.prepare( "SELECT pohead_number, poitem_linenumber, poitem_itemsite_id,"
             "       poitem_itemsrc_id, poitem_vend_item_number, poitem_vend_item_descrip,"
             "       poitem_vend_uom,"
             "       poitem_invvenduomratio,"
             "       poitem_expcat_id, poitem_duedate,"
             "       poitem_qty_ordered,"
             "       poitem_qty_received,"
	     "       pohead_curr_id, pohead_orderdate, "
             "       poitem_unitprice,"
             "       poitem_freight,"
             "       poitem_unitprice * poitem_qty_ordered AS f_extended,"
             "       poitem_comments, poitem_prj_id,"
			 "       poitem_bom_rev_id,poitem_boo_rev_id, "
             "       COALESCE(coitem_prcost, 0.0) AS overrideCost "
             "FROM pohead, poitem LEFT OUTER JOIN coitem ON (poitem_soitem_id=coitem_id) "
             "WHERE ( (poitem_pohead_id=pohead_id) "
             " AND (poitem_id=:poitem_id) );" );
  q.bindValue(":poitem_id", _poitemid);
  q.exec();
  if (q.first())
  {
    _poNumber->setText(q.value("pohead_number").toString());
    _lineNumber->setText(q.value("poitem_linenumber").toString());
    _dueDate->setDate(q.value("poitem_duedate").toDate());
    _ordered->setDouble(q.value("poitem_qty_ordered").toDouble());
    _received->setDouble(q.value("poitem_qty_received").toDouble());
    _unitPrice->set(q.value("poitem_unitprice").toDouble(),
		    q.value("pohead_curr_id").toInt(),
		    q.value("pohead_orderdate").toDate(), false);
    _freight->setLocalValue(q.value("poitem_freight").toDouble());
    _extendedPrice->setLocalValue(q.value("f_extended").toDouble());
    _notes->setText(q.value("poitem_comments").toString());
    _project->setId(q.value("poitem_prj_id").toInt());

    if(q.value("overrideCost").toDouble() > 0)
      _overriddenUnitPrice = true;

    if (q.value("poitem_itemsite_id").toInt() == -1)
    {
      _nonInventoryItem->setChecked(TRUE);
      _expcat->setId(q.value("poitem_expcat_id").toInt());
      sPopulateItemSourceInfo(-1);

      _vendorItemNumber->setText(q.value("poitem_vend_item_number").toString());
      _vendorDescrip->setText(q.value("poitem_vend_item_descrip").toString());
      _vendorUOM->setText(q.value("poitem_vend_uom").toString());
    }
    else
    {
      _inventoryItem->setChecked(TRUE);
      _item->setItemsiteid(q.value("poitem_itemsite_id").toInt());
      sPopulateItemSourceInfo(_item->id());
      if (_metrics->boolean("RevControl"))
      {
        _bomRevision->setId(q.value("poitem_bom_rev_id").toInt());
        _booRevision->setId(q.value("poitem_boo_rev_id").toInt());
      }
    }

    _itemsrcid = q.value("poitem_itemsrc_id").toInt();
    _vendorItemNumber->setText(q.value("poitem_vend_item_number").toString());
    _vendorDescrip->setText(q.value("poitem_vend_item_descrip").toString());
    if (_itemsrcid == -1)
    {
      _vendorUOM->setText(q.value("poitem_vend_uom").toString());
      _invVendorUOMRatio->setDouble(q.value("poitem_invvenduomratio").toDouble());
      _invVendUOMRatio = q.value("poitem_invvenduomratio").toDouble();
    }
    else
    {
      q.prepare( "SELECT itemsrc_id, itemsrc_vend_item_number,"
                 "       itemsrc_vend_item_descrip, itemsrc_vend_uom,"
                 "       itemsrc_minordqty,"
                 "       itemsrc_multordqty,"
                 "       itemsrc_invvendoruomratio "
                 "FROM itemsrc "
                 "WHERE (itemsrc_id=:itemsrc_id);" );
      q.bindValue(":itemsrc_id", _itemsrcid);
      q.exec();
      if (q.first())
      {
        _vendorItemNumber->setEnabled(FALSE);
        _vendorItemNumberList->setEnabled(FALSE);
        _vendorDescrip->setEnabled(FALSE);
        _vendorUOM->setEnabled(FALSE);

        if(_vendorItemNumber->text().isEmpty())
          _vendorItemNumber->setText(q.value("itemsrc_vend_item_number").toString());
        if(_vendorDescrip->text().isEmpty())
          _vendorDescrip->setText(q.value("itemsrc_vend_item_descrip").toString());
        _vendorUOM->setText(q.value("itemsrc_vend_uom").toString());
        _minOrderQty->setDouble(q.value("itemsrc_minordqty").toDouble());
        _orderQtyMult->setDouble(q.value("itemsrc_multordqty").toDouble());
        _invVendorUOMRatio->setDouble(q.value("itemsrc_invvendoruomratio").toDouble());

        _invVendUOMRatio = q.value("itemsrc_invvendoruomratio").toDouble();
        _minimumOrder = q.value("itemsrc_minordqty").toDouble();
        _orderMultiple = q.value("itemsrc_multordqty").toDouble();
      }
//  ToDo
    }

    q.prepare( "SELECT DISTINCT char_id, char_name,"
               "       COALESCE(b.charass_value, (SELECT c.charass_value FROM charass c WHERE ((c.charass_target_type='I') AND (c.charass_target_id=:item_id) AND (c.charass_default) AND (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "  FROM charass a, char "
               "    LEFT OUTER JOIN charass b"
               "      ON (b.charass_target_type='PI'"
               "      AND b.charass_target_id=:poitem_id"
               "      AND b.charass_char_id=char_id) "
               " WHERE ( (a.charass_char_id=char_id)"
               "   AND   (a.charass_target_type='I')"
               "   AND   (a.charass_target_id=:item_id) ) "
               " ORDER BY char_name;" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    int row = 0;
    QModelIndex idx;
    while(q.next())
    {
      _itemchar->insertRow(_itemchar->rowCount());
      idx = _itemchar->index(row, 0);
      _itemchar->setData(idx, q.value("char_name"), Qt::DisplayRole);
      _itemchar->setData(idx, q.value("char_id"), Qt::UserRole);
      idx = _itemchar->index(row, 1);
      _itemchar->setData(idx, q.value("charass_value"), Qt::DisplayRole);
      _itemchar->setData(idx, _item->id(), Qt::UserRole);
      row++;
    }

    _comments->setId(_poitemid);
  }
}

void purchaseOrderItem::sSave()
{
  if (!_inventoryItem->isChecked() && _expcat->id() == -1)
  {
    QMessageBox::critical( this, tr("Expense Category Required"),
                           tr("<p>You must specify an Expense Category for this non-Inventory Item before you may save it.") );
    return;
  }

  if (_inventoryItem->isChecked() && !_item->isValid())
  {
    QMessageBox::critical( this, tr("No Item Selected"),
                           tr("<p>You must select an Item Number before you may save.") );
    return;
  }

  if (_inventoryItem->isChecked() && _warehouse->id() == -1)
  {
    QMessageBox::critical( this, tr("No Warehouse Selected"),
                           tr("<p>You must select a Supplying Warehouse before you may save.") );
    return;
  }

  if (_ordered->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Cannot Save Purchase Order Item"),
                           tr("<p>You must enter a quantity before you may save this Purchase Order Item.") );
    _ordered->setFocus();
    return;
  }

  if (_ordered->toDouble() < _minimumOrder)
  {
    if (QMessageBox::critical( this, tr("Invalid Order Quantity"),
                               tr( "<p>The quantity that you are ordering is below the Minimum Order Quantity for this "
                                   "Item Source.  You may continue but this Vendor may not honor pricing or delivery quotations. "
                                   "<p>Do you wish to Continue or Change the Order Qty?" ),
                               QString("&Continue"), QString("Change Order &Qty."), QString::null, 1, 1 ) == 1)
    {
      _ordered->setFocus();
      return;
    }
  }

  if ((int)_orderMultiple)
  {
    if (qRound(_ordered->toDouble()) % (int)_orderMultiple)
    {
      if (QMessageBox::critical( this, tr("Invalid Order Quantity"),
                                 tr( "<p>The quantity that you are ordering does not fall within the Order Multiple for this "
                                     "Item Source.  You may continue but this Vendor may not honor pricing or delivery quotations. "
                                     "<p>Do you wish to Continue or Change the Order Qty?" ),
                                 QString("&Continue"), QString("Change Order &Qty."), QString::null, 1, 1 ) == 1)
      {
        _ordered->setFocus();
        return;
      }
    }
  }

  if (!_dueDate->isValid())
  {
    QMessageBox::critical( this, tr("Invalid Due Date"),
                           tr("<p>You must enter a due date before you may save this Purchase Order Item.") );
    _dueDate->setFocus();
    return;
  }

  if (_dueDate->date() < _earliestDate->date())
  {
    if (QMessageBox::critical( this, tr("Invalid Due Date "),
                               tr( "<p>The Due Date that you are requesting does not fall within the Lead Time Days for this "
                                   "Item Source.  You may continue but this Vendor may not honor pricing or delivery quotations "
                                   "or may not be able to deliver by the requested Due Date. "
                                   "<p>Do you wish to Continue or Change the Due Date?" ),
                               QString("&Continue"), QString("Change Order &Due Date"), QString::null, 1, 1 ) == 1)
    {
      _dueDate->setFocus();
      return;
    }
  }

  if (_mode == cNew)
  {
    q.prepare( "INSERT INTO poitem "
               "( poitem_id, poitem_pohead_id, poitem_status, poitem_linenumber,"
               "  poitem_itemsite_id, poitem_expcat_id,"
               "  poitem_itemsrc_id, poitem_vend_item_number, poitem_vend_item_descrip,"
               "  poitem_vend_uom, poitem_invvenduomratio,"
               "  poitem_qty_ordered,"
               "  poitem_unitprice, poitem_freight, poitem_duedate, "
			   "  poitem_bom_rev_id, poitem_boo_rev_id, "
	       "  poitem_comments, poitem_prj_id, poitem_stdcost ) "
               "VALUES "
               "( :poitem_id, :poitem_pohead_id, :status, :poitem_linenumber,"
               "  :poitem_itemsite_id, :poitem_expcat_id,"
               "  :poitem_itemsrc_id, :poitem_vend_item_number, :poitem_vend_item_descrip,"
               "  :poitem_vend_uom, :poitem_invvenduomratio,"
               "  :poitem_qty_ordered,"
               "  :poitem_unitprice, :poitem_freight, :poitem_duedate, "
			   "  :poitem_bom_rev_id, :poitem_boo_rev_id, "
	       "  :poitem_comments, :poitem_prj_id, stdcost(:item_id) );" );

    q.bindValue(":status", _poStatus);
    q.bindValue(":item_id", _item->id());

    if (_inventoryItem->isChecked())
    {
      XSqlQuery itemsiteid;
      itemsiteid.prepare( "SELECT itemsite_id "
                          "FROM itemsite "
                          "WHERE ( (itemsite_item_id=:item_id)"
                          " AND (itemsite_warehous_id=:warehous_id) );" );
      itemsiteid.bindValue(":item_id", _item->id());
      itemsiteid.bindValue(":warehous_id", _warehouse->id());
      itemsiteid.exec();
      if (itemsiteid.first())
        q.bindValue(":poitem_itemsite_id", itemsiteid.value("itemsite_id").toInt());
      else
      {
        QMessageBox::critical( this, tr("Invalid Item/Warehouse"),
                               tr("<p>The Item and Warehouse you have selected does not appear to be a valid combination. "
                                  "Make sure you have a warehouse selected and that there is a valid itemsite for "
                                  "this Item and Warehouse combination.") );
        return;
      }

      q.bindValue(":poitem_expcat_id", -1);
    }
    else
    {
      q.bindValue(":poitem_itemsite_id", -1);
      q.bindValue(":poitem_expcat_id", _expcat->id());
    }
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE poitem "
               "SET poitem_itemsrc_id=:poitem_itemsrc_id,"
               "    poitem_vend_item_number=:poitem_vend_item_number,"
               "    poitem_vend_item_descrip=:poitem_vend_item_descrip,"
               "    poitem_vend_uom=:poitem_vend_uom, poitem_invvenduomratio=:poitem_invvenduomratio,"
               "    poitem_qty_ordered=:poitem_qty_ordered, poitem_unitprice=:poitem_unitprice,"
               "    poitem_freight=:poitem_freight,"
               "    poitem_duedate=:poitem_duedate, poitem_comments=:poitem_comments,"
               "    poitem_prj_id=:poitem_prj_id, "
			   "    poitem_bom_rev_id=:poitem_bom_rev_id, "
			   "    poitem_boo_rev_id=:poitem_boo_rev_id "
               "WHERE (poitem_id=:poitem_id);" );

  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":poitem_pohead_id", _poheadid);
  q.bindValue(":poitem_linenumber", _lineNumber->text().toInt());
  q.bindValue(":poitem_itemsrc_id", _itemsrcid);
  q.bindValue(":poitem_vend_item_number", _vendorItemNumber->text());
  q.bindValue(":poitem_vend_item_descrip", _vendorDescrip->text());
  q.bindValue(":poitem_vend_uom", _vendorUOM->text());
  q.bindValue(":poitem_invvenduomratio", _invVendorUOMRatio->toDouble());
  q.bindValue(":poitem_qty_ordered", _ordered->toDouble());
  q.bindValue(":poitem_unitprice", _unitPrice->localValue());
  q.bindValue(":poitem_freight", _freight->localValue());
  q.bindValue(":poitem_duedate", _dueDate->date());
  q.bindValue(":poitem_comments", _notes->text());
  q.bindValue(":poitem_prj_id", _project->id());
  if (_metrics->boolean("RevControl"))
  {
    q.bindValue(":poitem_bom_rev_id", _bomRevision->id());
	q.bindValue(":poitem_boo_rev_id", _booRevision->id());
  }
  q.exec();

  if (_parentwo != -1)
  {
    q.prepare("UPDATE poitem SET poitem_wohead_id=:parentwo WHERE (poitem_id=:poitem_id);");
    q.bindValue(":parentwo", _parentwo);
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
  }

  if (_parentso != -1)
  {
    q.prepare("UPDATE poitem SET poitem_soitem_id=:parentso WHERE (poitem_id=:poitem_id);");
    q.bindValue(":parentso", _parentso);
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
  }

  if ( _mode != cView )
  {
    q.prepare("SELECT updateCharAssignment('PI', :target_id, :char_id, :char_value);");

    QModelIndex idx1, idx2;
    for(int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx1 = _itemchar->index(i, 0);
      idx2 = _itemchar->index(i, 1);
      q.bindValue(":target_id", _poitemid);
      q.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
      q.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
      q.exec();
    }
  }

  done(_poitemid);
}

void purchaseOrderItem::sPopulateExtPrice()
{
  _extendedPrice->setLocalValue(_ordered->toDouble() * _unitPrice->localValue());
}

void purchaseOrderItem::sPopulateItemSourceInfo(int pItemid)
{
  bool skipClear = false;
  _itemchar->removeRows(0, _itemchar->rowCount());
  if (_mode == cNew)
  {
    if (pItemid != -1)
    {
      if(_metrics->boolean("RequireStdCostForPOItem"))
      {
        q.prepare("SELECT stdCost(:item_id) AS result");
        q.bindValue(":item_id", pItemid);
        q.exec();
        if(q.first() && q.value("result").toDouble() == 0.0)
        {
          QMessageBox::critical( this, tr("Selected Item Missing Cost"),
		  tr("<p>The selected item has no Std. Costing information. "
		     "Please see your controller to correct this situation "
		     "before continuing."));
          _item->setId(-1);
          return;
        }
      }

      q.prepare( "SELECT itemsrc_id, itemsrc_vend_item_number,"
                 "       itemsrc_vend_item_descrip, itemsrc_vend_uom,"
                 "       itemsrc_minordqty,"
                 "       itemsrc_multordqty,"
                 "       itemsrc_invvendoruomratio,"
                 "       (CURRENT_DATE + itemsrc_leadtime) AS earliestdate "
                 "FROM pohead, itemsrc "
                 "WHERE ( (itemsrc_vend_id=pohead_vend_id)"
                 " AND (itemsrc_item_id=:item_id)"
                 " AND (pohead_id=:pohead_id) );" );
      q.bindValue(":item_id", pItemid);
      q.bindValue(":pohead_id", _poheadid);
      q.exec();
      if (q.first())
      {
        _itemsrcid = q.value("itemsrc_id").toInt();
  
        _vendorItemNumber->setText(q.value("itemsrc_vend_item_number").toString());
        _vendorDescrip->setText(q.value("itemsrc_vend_item_descrip").toString());
        _vendorUOM->setText(q.value("itemsrc_vend_uom").toString());
        _minOrderQty->setDouble(q.value("itemsrc_minordqty").toDouble());
        _orderQtyMult->setDouble(q.value("itemsrc_multordqty").toDouble());
        _invVendorUOMRatio->setDouble(q.value("itemsrc_invvendoruomratio").toDouble());
        _earliestDate->setDate(q.value("earliestdate").toDate());

        _invVendUOMRatio = q.value("itemsrc_invvendoruomratio").toDouble();
        _minimumOrder = q.value("itemsrc_minordqty").toDouble();
        _orderMultiple = q.value("itemsrc_multordqty").toDouble();

        if (_ordered->toDouble() != 0)
          sDeterminePrice();

        _ordered->setFocus();

        if(_metrics->boolean("UseEarliestAvailDateOnPOItem"))
          _dueDate->setDate(_earliestDate->date());

        skipClear = true;
      }
    }

    if(!skipClear)
    {
      _itemsrcid = -1;
  
      _vendorItemNumber->clear();
      _vendorDescrip->clear();
      _vendorUOM->setText(_item->uom());
      _minOrderQty->clear();
      _orderQtyMult->clear();
      _invVendorUOMRatio->setDouble(1.0);
      _earliestDate->setDate(omfgThis->dbDate());
  
      _invVendUOMRatio = 1;
      _minimumOrder = 0;
      _orderMultiple = 0;
    }

    q.prepare( "SELECT DISTINCT char_id, char_name,"
               "       COALESCE(b.charass_value, (SELECT c.charass_value FROM charass c WHERE ((c.charass_target_type='I') AND (c.charass_target_id=:item_id) AND (c.charass_default) AND (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "  FROM charass a, char "
               "    LEFT OUTER JOIN charass b"
               "      ON (b.charass_target_type='PI'"
               "      AND b.charass_target_id=:poitem_id"
               "      AND b.charass_char_id=char_id) "
               " WHERE ( (a.charass_char_id=char_id)"
               "   AND   (a.charass_target_type='I')"
               "   AND   (a.charass_target_id=:item_id) ) "
               " ORDER BY char_name;" );
    q.bindValue(":item_id", pItemid);
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    int row = 0;
    QModelIndex idx;
    while(q.next())
    {
      _itemchar->insertRow(_itemchar->rowCount());
      idx = _itemchar->index(row, 0);
      _itemchar->setData(idx, q.value("char_name"), Qt::DisplayRole);
      _itemchar->setData(idx, q.value("char_id"), Qt::UserRole);
      idx = _itemchar->index(row, 1);
      _itemchar->setData(idx, q.value("charass_value"), Qt::DisplayRole);
      _itemchar->setData(idx, pItemid, Qt::UserRole);
      row++;
    }
  }
}

void purchaseOrderItem::sDeterminePrice()
{
  if ( (!_overriddenUnitPrice) && (_itemsrcid != -1) && (_ordered->toDouble() != 0.0) )
  {
    q.prepare( "SELECT currToCurr(itemsrcp_curr_id, :curr_id, itemsrcp_price, :effective) "
		"AS new_itemsrcp_price "
               "FROM itemsrcp "
               "WHERE ( (itemsrcp_itemsrc_id=:itemsrc_id)"
               " AND (itemsrcp_qtybreak <= :qty) ) "
               "ORDER BY itemsrcp_qtybreak DESC "
               "LIMIT 1;" );
    q.bindValue(":itemsrc_id", _itemsrcid);
    q.bindValue(":qty", _ordered->toDouble());
    q.bindValue(":curr_id", _unitPrice->id());
    q.bindValue(":effective", _unitPrice->effective());
    q.exec();
    if (q.first())
      _unitPrice->setLocalValue(q.value("new_itemsrcp_price").toDouble());
    else
	_unitPrice->clear();
  }

  sPopulateExtPrice();
}

void purchaseOrderItem::sUpdateVendorQty()
{
}

void purchaseOrderItem::sInventoryItemToggled( bool yes )
{
  if(yes)
    sPopulateItemSourceInfo(_item->id());
  else
    sPopulateItemSourceInfo(-1);
}

void purchaseOrderItem::sVendorItemNumberList()
{
  ParameterList params;

  q.prepare( "SELECT vend_id"
             "  FROM pohead, vend "
             " WHERE((pohead_vend_id=vend_id)"
             "   AND (pohead_id=:pohead_id));" );
  q.bindValue(":pohead_id", _poheadid);
  q.exec();
  if (q.first())
    params.append("vend_id", q.value("vend_id").toInt());
  params.append("search", _vendorItemNumber->text());
  itemSourceSearch newdlg(this, "", true);
  newdlg.set(params);

  if(newdlg.exec() == XDialog::Accepted)
  {
    int itemid = newdlg.itemId();
    if(itemid != -1)
    {
      _inventoryItem->setChecked(TRUE);
      _item->setId(itemid);
    }
    else
    {
      _nonInventoryItem->setChecked(TRUE);
      _expcat->setId(newdlg.expcatId());
    }
    _vendorItemNumber->setText(newdlg.vendItemNumber());
    _vendorDescrip->setText(newdlg.vendItemDescrip());
  }
}
