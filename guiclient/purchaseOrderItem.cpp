/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "purchaseOrderItem.h"

#include <QMessageBox>
#include <QValidator>
#include <QVariant>
#include <QSqlError>
#include <metasql.h>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "mqlutil.h"
#include "taxDetail.h"
#include "itemCharacteristicDelegate.h"
#include "itemSourceSearch.h"
#include "itemSourceList.h"
#include "vendorPriceList.h"

purchaseOrderItem::purchaseOrderItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  XSqlQuery purchasepurchaseOrderItem;
  setupUi(this);

#ifndef Q_OS_MAC
  _vendorItemNumberList->setMaximumWidth(25);
#else
  _listPrices->setMinimumWidth(60);
#endif

  _vendid = -1;
  _preferredWarehouseid = -1;
  _invVendUOMRatio = 1;
  _minimumOrder = 0;
  _orderMultiple = 0;
  _maxCost = 0.0;
  _dropship = false;
  _costmethod = "";
  _captive = false;

  connect(_ordered, SIGNAL(editingFinished()), this, SLOT(sDeterminePrice()));
  connect(_inventoryItem, SIGNAL(toggled(bool)), this, SLOT(sInventoryItemToggled(bool)));
  connect(_item, SIGNAL(privateIdChanged(int)), this, SLOT(sFindWarehouseItemsites(int)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateItemInfo(int)));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateItemsiteInfo()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_vendorItemNumberList, SIGNAL(clicked()), this, SLOT(sVendorItemNumberSearch()));
  connect(_notesButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_listPrices, SIGNAL(clicked()), this, SLOT(sVendorListPrices()));
  connect(_taxLit, SIGNAL(leftClickedURL(QString)), this, SLOT(sTaxDetail()));  // new slot added for tax url //
  connect(_extendedPrice, SIGNAL(valueChanged()), this, SLOT(sCalculateTax()));  // new slot added for price //
  connect(_taxtype, SIGNAL(newID(int)), this, SLOT(sCalculateTax()));            // new slot added for taxtype //

  _bomRevision->setMode(RevisionLineEdit::Use);
  _bomRevision->setType("BOM");
  _booRevision->setMode(RevisionLineEdit::Use);
  _booRevision->setType("BOO");

  _parentwo = -1;
  _parentso = -1;
  _itemsrcid = -1;
  _taxzoneid = -1;   //  _taxzoneid  added //
  _orderQtyCache = -1;

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

  purchasepurchaseOrderItem.exec("SELECT DISTINCT itemsrc_manuf_name FROM itemsrc ORDER BY 1;");
  for (int i = 0; purchasepurchaseOrderItem.next(); i++)
    _manufName->append(i, purchasepurchaseOrderItem.value("itemsrc_manuf_name").toString());
  _manufName->setId(-1);

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  //If not Revision Control, hide controls
  if (!_metrics->boolean("RevControl"))
   _tab->removeTab(_tab->indexOf(_revision));
   
  adjustSize();
  
  //TO DO: Implement later
  _taxRecoverable->hide();
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
  XSqlQuery purchaseet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  bool     haveQty  = false;
  bool     haveDate = false;



  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendid = param.toInt();

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _preferredWarehouseid = param.toInt();

  param = pParams.value("dropship", &valid);
  if (valid)
    _dropship = param.toBool();

  param = pParams.value("parentWo", &valid);
  if (valid)
    _parentwo = param.toInt();

  param = pParams.value("parentSo", &valid);
  if (valid)
    _parentso = param.toInt();

  if (_metrics->boolean("CopyPRtoPOItem"))
  {
    param = pParams.value("pr_releasenote", &valid);
    if(valid)
      _notes->setText(param.toString());
  }

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _poheadid = param.toInt();

    purchaseet.prepare( "SELECT pohead_taxzone_id, pohead_number, pohead_orderdate, pohead_status, " // pohead_taxzone_id added
               "       vend_id, vend_restrictpurch, pohead_curr_id "
               "FROM pohead, vendinfo "
               "WHERE ( (pohead_vend_id=vend_id)"
               " AND (pohead_id=:pohead_id) );" );
    purchaseet.bindValue(":pohead_id", param.toInt());
    purchaseet.exec();
    if (purchaseet.first())
    {
      _poNumber->setText(purchaseet.value("pohead_number").toString());
      _poStatus = purchaseet.value("pohead_status").toString();
      _unitPrice->setEffective(purchaseet.value("pohead_orderdate").toDate());
      _unitPrice->setId(purchaseet.value("pohead_curr_id").toInt());
	  _taxzoneid=purchaseet.value("pohead_taxzone_id").toInt();   // added  to pick up tax zone id.
	  _tax->setEffective(purchaseet.value("pohead_orderdate").toDate());
      _tax->setId(purchaseet.value("pohead_curr_id").toInt());

      if (purchaseet.value("vend_restrictpurch").toBool())
      {
        _item->setQuery( QString( "SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                                  "                (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
                                  "                uom_name, item_type, item_config, item_active, item_upccode "
                                  "FROM item, itemsite, itemsrc, uom  "
                                  "WHERE ( (itemsite_item_id=item_id)"
                                  " AND (itemsrc_item_id=item_id)"
                                  " AND (item_inv_uom_id=uom_id)"
                                  " AND (itemsite_active)"
                                  " AND (item_active)"
                                  " AND (itemsrc_active)"
                                  " AND (itemsrc_vend_id=%1) ) "
                                  "ORDER BY item_number" )
                         .arg(purchaseet.value("vend_id").toInt()) );
        _item->setValidationQuery( QString( "SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                                            "                (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
                                            "                uom_name, item_type, item_config, item_active, item_upccode "
                                            "FROM item, itemsite, itemsrc, uom  "
                                            "WHERE ( (itemsite_item_id=item_id)"
                                            " AND (itemsrc_item_id=item_id)"
                                            " AND (item_inv_uom_id=uom_id)"
                                            " AND (itemsite_active)"
                                            " AND (item_active)"
                                            " AND (itemsrc_active)"
                                            " AND (itemsrc_vend_id=%1) "
                                            " AND (itemsite_item_id=:item_id) ) ")
                                   .arg(purchaseet.value("vend_id").toInt()) );
      }
      else
      {
        _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured |
		               ItemLineEdit::cTooling | ItemLineEdit::cActive);
        _item->setDefaultType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cActive);
      }
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

    purchaseet.prepare( "SELECT pohead_number, pohead_id "
               "FROM pohead, poitem "
               "WHERE ( (pohead_id=poitem_pohead_id) "
               " AND (poitem_id=:poitem_id) );" );
    purchaseet.bindValue(":poitem_id", param.toInt());
    purchaseet.exec();
    if (purchaseet.first())
    {
      _poNumber->setText(purchaseet.value("pohead_number").toString());
	  _poheadid = purchaseet.value("pohead_id").toInt();
    }

    populate();
	sCalculateTax();
  }
  // connect here and not in the .ui to avoid timing issues at initialization
  connect(_unitPrice, SIGNAL(valueChanged()), this, SLOT(sPopulateExtPrice()));

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _save->setEnabled(false);

      purchaseet.exec("SELECT NEXTVAL('poitem_poitem_id_seq') AS poitem_id;");
      if (purchaseet.first())
        _poitemid = purchaseet.value("poitem_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }

      if(_parentwo != -1)
      {
        purchaseet.prepare("SELECT wo_number"
                  "  FROM womatl JOIN wo ON (wo_id=womatl_wo_id)"
                  " WHERE (womatl_id=:parentwo); ");
        purchaseet.bindValue(":parentwo", _parentwo);
        purchaseet.exec();
        if(purchaseet.first())
        {
          _so->setText(purchaseet.value("wo_number").toString());
          _soLine->setText("");
        }
      }

      if(_parentso != -1)
      {
        purchaseet.prepare( "INSERT INTO charass"
                   "      (charass_target_type, charass_target_id,"
                   "       charass_char_id, charass_value) "
                   "SELECT 'PI', :orderid, charass_char_id, charass_value"
                   "  FROM charass"
                   " WHERE ((charass_target_type='SI')"
                   "   AND  (charass_target_id=:soitem_id));");
        purchaseet.bindValue(":orderid", _poitemid);
        purchaseet.bindValue(":soitem_id", _parentso);
        purchaseet.exec();
      }

      purchaseet.prepare( "SELECT (COALESCE(MAX(poitem_linenumber), 0) + 1) AS _linenumber "
                 "FROM poitem "
                 "WHERE (poitem_pohead_id=:pohead_id);" );
      purchaseet.bindValue(":pohead_id", _poheadid);
      purchaseet.exec();
      if (purchaseet.first())
        _lineNumber->setText(purchaseet.value("_linenumber").toString());
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );

        return UndefinedError;
      }

      _bomRevision->setEnabled(_privileges->boolean("UseInactiveRevisions"));
      _booRevision->setEnabled(_privileges->boolean("UseInactiveRevisions"));
      _comments->setId(_poitemid);
      _tab->setTabEnabled(_tab->indexOf(_demandTab), false);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _typeGroup->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _typeGroup->setEnabled(false);
      _vendorItemNumber->setEnabled(false);
      _vendorItemNumberList->setEnabled(false);
      _vendorDescrip->setEnabled(false);
      _warehouse->setEnabled(false);
      _dueDate->setEnabled(false);
      _ordered->setEnabled(false);
      _unitPrice->setEnabled(false);
      _freight->setEnabled(false);
      _notes->setReadOnly(true);
      _comments->setReadOnly(true);
      _project->setEnabled(false);
      _taxtype->setEnabled(false);
      _taxRecoverable->setEnabled(false);
      _bomRevision->setEnabled(false);
      _booRevision->setEnabled(false);

      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(false);
    _warehouse->setEnabled(false);
  }
  
  param = pParams.value("itemsrc_id", &valid);
  if (valid)
    sPopulateItemSourceInfo(param.toInt());

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _ordered->setDouble((param.toDouble()/_invVendUOMRatio));

    if (_item->isValid())
      sDeterminePrice();

    haveQty = true;
  }

  param = pParams.value("dueDate", &valid);
  if (valid)
  {
    _dueDate->setDate(param.toDate());
    haveDate = true;
  }

  param = pParams.value("prj_id", &valid);
  if (valid)
    _project->setId(param.toInt());

  if(_parentso != -1)
  {
    purchaseet.prepare("SELECT cohead_number, coitem_linenumber, coitem_prcost"
              "  FROM coitem JOIN cohead ON (cohead_id=coitem_cohead_id)"
              " WHERE (coitem_id=:parentso); ");
    purchaseet.bindValue(":parentso", _parentso);
    purchaseet.exec();
    if(purchaseet.first())
    {
      _so->setText(purchaseet.value("cohead_number").toString());
      _soLine->setText(purchaseet.value("coitem_linenumber").toString());
      if(purchaseet.value("coitem_prcost").toDouble() > 0)
      {
        _overriddenUnitPrice = true;
        _unitPrice->setLocalValue(purchaseet.value("coitem_prcost").toDouble());
        sPopulateExtPrice();
      }
    }
  }

  param = pParams.value("captive", &valid);
  if (valid)
    _captive = true;
  
  return NoError;
}

void purchaseOrderItem::populate()
{
  XSqlQuery purchasepopulate;
  MetaSQLQuery mql = mqlLoad("purchaseOrderItems", "detail");

  ParameterList params;
  params.append("poitem_id", _poitemid);
  params.append("sonum",     tr("Sales Order #")),
  params.append("wonum",     tr("Work Order #")),
  purchasepopulate = mql.toQuery(params);
  if (purchasepopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, purchasepopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if (purchasepopulate.first())
  {
    _poNumber->setText(purchasepopulate.value("pohead_number").toString());
    _lineNumber->setText(purchasepopulate.value("poitem_linenumber").toString());
    _taxzoneid=purchasepopulate.value("pohead_taxzone_id").toInt();   // added  to pick up tax zone id.
    _dueDate->setDate(purchasepopulate.value("poitem_duedate").toDate());
    _ordered->setDouble(purchasepopulate.value("poitem_qty_ordered").toDouble());
    _orderQtyCache = _ordered->toDouble();
    _received->setDouble(purchasepopulate.value("poitem_qty_received").toDouble());
    _unitPrice->set(purchasepopulate.value("poitem_unitprice").toDouble(),
		    purchasepopulate.value("pohead_curr_id").toInt(),
		    purchasepopulate.value("pohead_orderdate").toDate(), false);
    _freight->setLocalValue(purchasepopulate.value("poitem_freight").toDouble());
    _extendedPrice->setLocalValue(purchasepopulate.value("extended_price").toDouble());
    _taxtype->setId(purchasepopulate.value("poitem_taxtype_id").toInt());
    _taxRecoverable->setChecked(purchasepopulate.value("poitem_tax_recoverable").toBool());
    _notes->setText(purchasepopulate.value("poitem_comments").toString());
    _project->setId(purchasepopulate.value("poitem_prj_id").toInt());
    if(purchasepopulate.value("override_cost").toDouble() > 0)
      _overriddenUnitPrice = true;

    if(purchasepopulate.value("poitem_order_id") != -1)
    {
      _ordered->setEnabled(false);
      _dueDate->setEnabled(false);
      _soLit->setText(purchasepopulate.value("demand_type").toString());
      _so->setText(purchasepopulate.value("order_number").toString());
//      _soLine->setText(purchasepopulate.value("orderline_number").toString());
    }
    else
      _tab->setTabEnabled(_tab->indexOf(_demandTab), false);

    if (purchasepopulate.value("poitem_itemsite_id").toInt() == -1)
    {
      _nonInventoryItem->setChecked(true);
      _expcat->setId(purchasepopulate.value("poitem_expcat_id").toInt());
      sPopulateItemSourceInfo(-1);

      _vendorItemNumber->setText(purchasepopulate.value("poitem_vend_item_number").toString());
      _vendorDescrip->setText(purchasepopulate.value("poitem_vend_item_descrip").toString());
      _vendorUOM->setText(purchasepopulate.value("poitem_vend_uom").toString());
      _uom->setText(purchasepopulate.value("poitem_vend_uom").toString());
    }
    else
    {
      _inventoryItem->setChecked(true);
      _item->setItemsiteid(purchasepopulate.value("poitem_itemsite_id").toInt());
      sPopulateItemSourceInfo(purchasepopulate.value("poitem_itemsrc_id").toInt());
      if (_metrics->boolean("RevControl"))
      {
        _bomRevision->setId(purchasepopulate.value("poitem_bom_rev_id").toInt());
        _booRevision->setId(purchasepopulate.value("poitem_boo_rev_id").toInt());
        _bomRevision->setEnabled(purchasepopulate.value("poitem_status").toString() == "U" && _privileges->boolean("UseInactiveRevisions"));
        _booRevision->setEnabled(purchasepopulate.value("poitem_status").toString() == "U" && _privileges->boolean("UseInactiveRevisions"));
      }
    }

    _itemsrcid = purchasepopulate.value("poitem_itemsrc_id").toInt();
    _contrctNumber->setText(purchasepopulate.value("contrct_number").toString());
    _vendorItemNumber->setText(purchasepopulate.value("poitem_vend_item_number").toString());
    _vendorDescrip->setText(purchasepopulate.value("poitem_vend_item_descrip").toString());
    _vendorUOM->setText(purchasepopulate.value("poitem_vend_uom").toString());
    _uom->setText(purchasepopulate.value("poitem_vend_uom").toString());
    _invVendorUOMRatio->setDouble(purchasepopulate.value("poitem_invvenduomratio").toDouble());
    _invVendUOMRatio = purchasepopulate.value("poitem_invvenduomratio").toDouble();
    _manufName->setText(purchasepopulate.value("poitem_manuf_name").toString());
    if (_manufName->id() < 0)
    {
      _manufName->append(_manufName->count(),
                         purchasepopulate.value("poitem_manuf_name").toString());
      _manufName->setText(purchasepopulate.value("poitem_manuf_name").toString());
    }
    _manufItemNumber->setText(purchasepopulate.value("poitem_manuf_item_number").toString());
    _manufItemDescrip->setText(purchasepopulate.value("poitem_manuf_item_descrip").toString());

    if (_itemsrcid != -1)
    {
      _vendorUOM->setEnabled(false);
      _manufName->setEnabled(false);
      _manufItemNumber->setEnabled(false);
      _manufItemDescrip->setEnabled(false);

      if(_vendorItemNumber->text().isEmpty())
        _vendorItemNumber->setText(purchasepopulate.value("itemsrc_vend_item_number").toString());
      if(_vendorDescrip->toPlainText().isEmpty())
        _vendorDescrip->setText(purchasepopulate.value("itemsrc_vend_item_descrip").toString());
      _minOrderQty->setDouble(purchasepopulate.value("itemsrc_minordqty").toDouble());
      _orderQtyMult->setDouble(purchasepopulate.value("itemsrc_multordqty").toDouble());

      _minimumOrder = purchasepopulate.value("itemsrc_minordqty").toDouble();
      _orderMultiple = purchasepopulate.value("itemsrc_multordqty").toDouble();

      if(_manufName->currentText().isEmpty())
        _manufName->setText(purchasepopulate.value("itemsrc_manuf_name").toString());
      if(_manufItemNumber->text().isEmpty())
        _manufItemNumber->setText(purchasepopulate.value("itemsrc_manuf_item_number").toString());
      if(_manufItemDescrip->toPlainText().isEmpty())
        _manufItemDescrip->setText(purchasepopulate.value("itemsrc_manuf_item_descrip").toString());
    }

    purchasepopulate.prepare( "SELECT DISTINCT char_id, char_name,"
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
    purchasepopulate.bindValue(":item_id", _item->id());
    purchasepopulate.bindValue(":poitem_id", _poitemid);
    purchasepopulate.exec();
    int row = 0;
    QModelIndex idx;
    while(purchasepopulate.next())
    {
      _itemchar->insertRow(_itemchar->rowCount());
      idx = _itemchar->index(row, 0);
      _itemchar->setData(idx, purchasepopulate.value("char_name"), Qt::DisplayRole);
      _itemchar->setData(idx, purchasepopulate.value("char_id"), Qt::UserRole);
      idx = _itemchar->index(row, 1);
      _itemchar->setData(idx, purchasepopulate.value("charass_value"), Qt::DisplayRole);
      _itemchar->setData(idx, _item->id(), Xt::IdRole);
      _itemchar->setData(idx, _item->id(), Qt::UserRole);
      row++;
    }

    _comments->setId(_poitemid);
  }
}

void purchaseOrderItem::prepare()
{
  XSqlQuery prepareq;
  //  Grab the next poitem_id
  prepareq.exec("SELECT NEXTVAL('poitem_poitem_id_seq') AS _poitem_id");
  if (prepareq.first())
  {
    _poitemid = prepareq.value("_poitem_id").toInt();
    _comments->setId(_poitemid);
  }
  else if (prepareq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, prepareq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
        
  prepareq.prepare( "SELECT (COALESCE(MAX(poitem_linenumber), 0) + 1) AS _linenumber "
                    "FROM poitem "
                    "WHERE (poitem_pohead_id=:pohead_id)" );
  prepareq.bindValue(":pohead_id", _poheadid);
  prepareq.exec();
  if (prepareq.first())
    _lineNumber->setText(prepareq.value("_linenumber").toString());
  else if (prepareq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, prepareq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void purchaseOrderItem::clear()
{
  _poitemid = -1;
  _item->setId(-1);
//  _warehouse->setId(-1);
  _expcat->setId(-1);
  _taxtype->setId(-1);
  _taxRecoverable->setChecked(false);
  _lineNumber->clear();
  _ordered->clear();
  _unitPrice->clear();
  _freight->clear();
//  _dueDate->clear();
  _notes->clear();
  _project->setId(-1);
  _bomRevision->setId(-1);
  _booRevision->setId(-1);
  _maxCost = 0.0;
  _dropship = false;
  _parentwo = -1;
  _parentso = -1;
  _costmethod = "";
  _orderQtyCache = -1;
  _overriddenUnitPrice = false;
  _itemchar->removeRows(0, _itemchar->rowCount());
  _itemcharView->setEnabled(true);
  _itemsrcid = -1;
  _vendorItemNumber->clear();
  _vendorDescrip->clear();
  _vendorUOM->setText(_item->uom());
  _uom->setText(_item->uom());
  _minOrderQty->clear();
  _orderQtyMult->clear();
  _invVendorUOMRatio->setDouble(1.0);
  _earliestDate->setDate(omfgThis->dbDate());
  _manufName->setId(-1);
  _manufItemNumber->clear();
  _manufItemDescrip->clear();
  _invVendUOMRatio = 1;
  _minimumOrder = 0;
  _orderMultiple = 0;
  _save->setEnabled(false);
}

void purchaseOrderItem::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_inventoryItem->isChecked() && _expcat->id() == -1, _expcat,
                          tr("<p>You must specify an Expense Category for this non-Inventory Item before you may save."))
         << GuiErrorCheck(_inventoryItem->isChecked() && !_item->isValid(), _item,
                          tr("<p>You must select an Item Number before you may save."))
         << GuiErrorCheck(_inventoryItem->isChecked() && _warehouse->id() == -1, _warehouse,
                          tr("<p>You must select a Supplying Site before you may save."))
         << GuiErrorCheck(!_dueDate->isValid(), _dueDate,
                          tr("<p>You must enter a due date before you may save this Purchase Order Item."))
         << GuiErrorCheck(_metrics->boolean("RequirePOTax") && !_taxtype->isValid(), _taxtype,
                          tr("<p>You must select a Tax Type before you may save." ))
         << GuiErrorCheck(_so->text().length() == 0 && _costmethod == "J", _item,
                          tr("<p>You may not purchase a Job Item without an associated demand."))
         << GuiErrorCheck(!_project->isValid() && _metrics->boolean("RequireProjectAssignment"), _project,
                          tr("<p>You must enter a Project for this order item before you may save it."))
     ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Purchase Order Item"), errors))
    return;

  XSqlQuery purchaseSave;
  if (_ordered->toDouble() == 0.0)
  {
    if (QMessageBox::critical( this, tr("Zero Order Quantity"),
                               tr( "<p>The quantity that you are ordering is zero. "
                                   "<p>Do you wish to Continue or Change the Order Qty?" ),
                               QString("&Continue"), QString("Change Order &Qty."), QString::null, 1, 1 ) == 1)
    {
      _ordered->setFocus();
      return;
    }
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

  if ((_unitPrice->baseValue() / _invVendUOMRatio) > _maxCost && _maxCost > 0.0)
  {
    if (QMessageBox::critical( this, tr("Invalid Unit Price"),
                               tr( "<p>The Unit Price is above the Maximum Desired Cost for this Item."
                                   "<p>Do you wish to Continue or Change the Unit Price?" ),
                               QString("&Continue"), QString("Change Unit &Price."), QString::null, 1, 1 ) == 1)
    {
      _unitPrice->setFocus();
      return;
    }
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
    purchaseSave.prepare( "INSERT INTO poitem "
               "( poitem_id, poitem_pohead_id, poitem_status, poitem_linenumber,"
               "  poitem_taxtype_id, poitem_tax_recoverable,"
               "  poitem_itemsite_id, poitem_expcat_id,"
               "  poitem_itemsrc_id, poitem_vend_item_number, poitem_vend_item_descrip,"
               "  poitem_vend_uom, poitem_invvenduomratio,"
               "  poitem_qty_ordered,"
               "  poitem_unitprice, poitem_freight, poitem_duedate, "
               "  poitem_bom_rev_id, poitem_boo_rev_id, "
               "  poitem_comments, poitem_prj_id, poitem_stdcost, poitem_manuf_name, "
               "  poitem_manuf_item_number, poitem_manuf_item_descrip, poitem_rlsd_duedate ) "
               "VALUES "
               "( :poitem_id, :poitem_pohead_id, :status, :poitem_linenumber,"
               "  :poitem_taxtype_id, :poitem_tax_recoverable,"
               "  :poitem_itemsite_id, :poitem_expcat_id,"
               "  :poitem_itemsrc_id, :poitem_vend_item_number, :poitem_vend_item_descrip,"
               "  :poitem_vend_uom, :poitem_invvenduomratio,"
               "  :poitem_qty_ordered,"
               "  :poitem_unitprice, :poitem_freight, :poitem_duedate, "
               "  :poitem_bom_rev_id, :poitem_boo_rev_id, "
               "  :poitem_comments, :poitem_prj_id, stdcost(:item_id), :poitem_manuf_name, "
               "  :poitem_manuf_item_number, :poitem_manuf_item_descrip, :poitem_duedate) ;" );

    purchaseSave.bindValue(":status", _poStatus);
    purchaseSave.bindValue(":item_id", _item->id());

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
        purchaseSave.bindValue(":poitem_itemsite_id", itemsiteid.value("itemsite_id").toInt());
      else
      {
        QMessageBox::critical( this, tr("Invalid Item/Site"),
                               tr("<p>The Item and Site you have selected does not appear to be a valid combination. "
                                  "Make sure you have a Site selected and that there is a valid itemsite for "
                                  "this Item and Site combination.") );
        return;
      }
    }
    else
    {
      purchaseSave.bindValue(":poitem_expcat_id", _expcat->id());
    }
  }
  else if (_mode == cEdit)
    purchaseSave.prepare( "UPDATE poitem "
               "SET poitem_itemsrc_id=:poitem_itemsrc_id,"   
               "    poitem_taxtype_id=:poitem_taxtype_id,"
               "    poitem_tax_recoverable=:poitem_tax_recoverable,"
               "    poitem_vend_item_number=:poitem_vend_item_number,"
               "    poitem_vend_item_descrip=:poitem_vend_item_descrip,"
               "    poitem_vend_uom=:poitem_vend_uom, poitem_invvenduomratio=:poitem_invvenduomratio,"
               "    poitem_qty_ordered=:poitem_qty_ordered, poitem_unitprice=:poitem_unitprice,"
               "    poitem_freight=:poitem_freight,"
               "    poitem_duedate=:poitem_duedate, poitem_comments=:poitem_comments,"
               "    poitem_prj_id=:poitem_prj_id, "
               "    poitem_bom_rev_id=:poitem_bom_rev_id, "
               "    poitem_boo_rev_id=:poitem_boo_rev_id, "
               "    poitem_manuf_name=:poitem_manuf_name, "
               "    poitem_manuf_item_number=:poitem_manuf_item_number, "
               "    poitem_manuf_item_descrip=:poitem_manuf_item_descrip "
               "WHERE (poitem_id=:poitem_id);" );

  purchaseSave.bindValue(":poitem_id", _poitemid);
  if (_taxtype->id() != -1)
    purchaseSave.bindValue(":poitem_taxtype_id", _taxtype->id());
  purchaseSave.bindValue(":poitem_tax_recoverable", QVariant(_taxRecoverable->isChecked()));
  purchaseSave.bindValue(":poitem_pohead_id", _poheadid);
  purchaseSave.bindValue(":poitem_linenumber", _lineNumber->text().toInt());
  if (_itemsrcid != -1)
    purchaseSave.bindValue(":poitem_itemsrc_id", _itemsrcid);
  purchaseSave.bindValue(":poitem_vend_item_number", _vendorItemNumber->text());
  purchaseSave.bindValue(":poitem_vend_item_descrip", _vendorDescrip->toPlainText());
  purchaseSave.bindValue(":poitem_vend_uom", _vendorUOM->text());
  purchaseSave.bindValue(":poitem_invvenduomratio", _invVendorUOMRatio->toDouble());
  purchaseSave.bindValue(":poitem_qty_ordered", _ordered->toDouble());
  purchaseSave.bindValue(":poitem_unitprice", _unitPrice->localValue());
  purchaseSave.bindValue(":poitem_freight", _freight->localValue());
  purchaseSave.bindValue(":poitem_duedate", _dueDate->date());
  purchaseSave.bindValue(":poitem_manuf_name", _manufName->currentText());
  purchaseSave.bindValue(":poitem_manuf_item_number", _manufItemNumber->text());
  purchaseSave.bindValue(":poitem_manuf_item_descrip", _manufItemDescrip->toPlainText());
  purchaseSave.bindValue(":poitem_comments", _notes->toPlainText());
  if (_project->isValid())
    purchaseSave.bindValue(":poitem_prj_id", _project->id());
  if (_metrics->boolean("RevControl"))
  {
    purchaseSave.bindValue(":poitem_bom_rev_id", _bomRevision->id());
    purchaseSave.bindValue(":poitem_boo_rev_id", _booRevision->id());
  }
  purchaseSave.exec();
  if (purchaseSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, purchaseSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_parentwo != -1)
  {
    purchaseSave.prepare("UPDATE poitem SET poitem_order_id=:parentwo, poitem_order_type='W' WHERE (poitem_id=:poitem_id);");
    purchaseSave.bindValue(":parentwo", _parentwo);
    purchaseSave.bindValue(":poitem_id", _poitemid);
    purchaseSave.exec();
  }

  if (_parentso != -1)
  {
    purchaseSave.prepare("UPDATE poitem SET poitem_order_id=:parentso, poitem_order_type='S' WHERE (poitem_id=:poitem_id);");
    purchaseSave.bindValue(":parentso", _parentso);
    purchaseSave.bindValue(":poitem_id", _poitemid);
    purchaseSave.exec();
    purchaseSave.prepare("UPDATE coitem SET coitem_order_id=:poitem_id, coitem_order_type='P' WHERE (coitem_id=:parentso);");
    purchaseSave.bindValue(":parentso", _parentso);
    purchaseSave.bindValue(":poitem_id", _poitemid);
    purchaseSave.exec();
  }

  if ( _mode != cView )
  {
    purchaseSave.prepare("SELECT updateCharAssignment('PI', :target_id, :char_id, :char_value);");

    QModelIndex idx1, idx2;
    for(int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx1 = _itemchar->index(i, 0);
      idx2 = _itemchar->index(i, 1);
      purchaseSave.bindValue(":target_id", _poitemid);
      purchaseSave.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
      purchaseSave.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
      purchaseSave.exec();
    }
  }

  
  if (cNew == _mode && !_captive)
  {
    clear();
    prepare();
    _item->setFocus();
    if (_orderRefresh->isChecked())
      omfgThis->sPurchaseOrdersUpdated(_poheadid, true);
  }
  else
    done(_poitemid);
}

void purchaseOrderItem::sPopulateExtPrice()
{
  _extendedPrice->setLocalValue(_ordered->toDouble() * _unitPrice->localValue());
}

void purchaseOrderItem::sFindWarehouseItemsites( int id )
{
  _warehouse->findItemsites(id);
  if(_preferredWarehouseid > 0)
    _warehouse->setId(_preferredWarehouseid);
}

void purchaseOrderItem::sPopulateItemInfo(int pItemid)
{
  XSqlQuery item;
  if (pItemid != -1 && _mode == cNew)
  {
      item.prepare("SELECT stdCost(item_id) AS stdcost, "
                   "       getItemTaxType(item_id, pohead_taxzone_id) AS taxtype_id, "
                   "       item_tax_recoverable, COALESCE(item_maxcost, 0.0) AS maxcost "
                   "FROM item, pohead "
                   "WHERE ( (item_id=:item_id) "
                   "  AND   (pohead_id=:pohead_id) );");
      item.bindValue(":item_id", pItemid);
      item.bindValue(":pohead_id", _poheadid);
      item.exec();

      if (item.first())
      {
          // Reset order qty cache
          _orderQtyCache = -1;

          if(_metrics->boolean("RequireStdCostForPOItem") && item.value("stdcost").toDouble() == 0.0)
          {
            QMessageBox::critical( this, tr("Selected Item Missing Cost"),
                    tr("<p>The selected item has no Std. Costing information. "
                       "Please see your controller to correct this situation "
                       "before continuing."));
            _item->setId(-1);
            return;
          }

        _taxtype->setId(item.value("taxtype_id").toInt());
        _taxRecoverable->setChecked(item.value("item_tax_recoverable").toBool());
        _maxCost = item.value("maxcost").toDouble();

        sPopulateItemsiteInfo();

        item.prepare( "SELECT DISTINCT char_id, char_name,"
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
        item.bindValue(":item_id", pItemid);
        item.bindValue(":poitem_id", _poitemid);
        item.exec();
        int row = 0;
        _itemchar->removeRows(0, _itemchar->rowCount());
        QModelIndex idx;
        while(item.next())
        {
          _itemchar->insertRow(_itemchar->rowCount());
          idx = _itemchar->index(row, 0);
          _itemchar->setData(idx, item.value("char_name"), Qt::DisplayRole);
          _itemchar->setData(idx, item.value("char_id"), Qt::UserRole);
          idx = _itemchar->index(row, 1);
          _itemchar->setData(idx, item.value("charass_value"), Qt::DisplayRole);
          _itemchar->setData(idx, pItemid, Xt::IdRole);
          _itemchar->setData(idx, pItemid, Qt::UserRole);
          row++;
        }

        item.prepare("SELECT itemsrc_id "
                     "FROM itemsrc, pohead "
                     "WHERE ( (itemsrc_vend_id=pohead_vend_id)"
                     " AND (itemsrc_item_id=:item_id)"
                     " AND (:effective BETWEEN itemsrc_effective AND (itemsrc_expires - 1))"
                     " AND (itemsrc_active)"
                     " AND (pohead_id=:pohead_id) );" );
        item.bindValue(":item_id", pItemid);
        item.bindValue(":pohead_id", _poheadid);
        item.bindValue(":effective", _unitPrice->effective());
        item.exec();
        if (item.size() == 1)
        {
          item.first();

          if (item.value("itemsrc_id").toInt() != _itemsrcid)
            sPopulateItemSourceInfo(item.value("itemsrc_id").toInt());
        }
        else if (item.size() > 1)
        {
          bool isCurrent = false;
          while (item.next())
          {
            if (item.value("itemsrc_id").toInt() == _itemsrcid)
              isCurrent = true;
          }
          if (!isCurrent)
          {
            _vendorItemNumber->clear();
            sVendorItemNumberList();
          }
        }
        else
        {
          _itemsrcid = -1;

          _vendorItemNumber->clear();
          _vendorDescrip->clear();
          _vendorUOM->setText(_item->uom());
          _uom->setText(_item->uom());
          _minOrderQty->clear();
          _orderQtyMult->clear();
          _invVendorUOMRatio->setDouble(1.0);
          _earliestDate->setDate(omfgThis->dbDate());
          _manufName->setId(-1);
          _manufItemNumber->clear();
          _manufItemDescrip->clear();

          _invVendUOMRatio = 1;
          _minimumOrder = 0;
          _orderMultiple = 0;
        }
      }
  }
}

void purchaseOrderItem::sPopulateItemsiteInfo()
{
  XSqlQuery itemsite;
  if (_item->isValid() && _warehouse->isValid())
  {
    itemsite.prepare("SELECT * "
                     "FROM itemsite "
                     "WHERE ( (itemsite_item_id=:item_id) "
                     "  AND   (itemsite_warehous_id=:warehous_id) );");
    itemsite.bindValue(":item_id", _item->id());
    itemsite.bindValue(":warehous_id", _warehouse->id());
    itemsite.exec();
    if (itemsite.first())
    {
       _costmethod = itemsite.value("itemsite_costmethod").toString();
       if (_costmethod == "J")
         _tab->setTabEnabled(_tab->indexOf(_demandTab), true);
    }
  }
}

void purchaseOrderItem::sPopulateItemSourceInfo(int pItemsrcid)
{
  XSqlQuery src;
  bool skipClear = false;
  if (!_item->isValid())
    skipClear = true;
  if (_mode == cNew)
  {
    if (pItemsrcid != -1)
    {
      src.prepare( "SELECT itemsrc_id, itemsrc_item_id, itemsrc_vend_item_number,"
                 "       itemsrc_vend_item_descrip, itemsrc_vend_uom,"
                 "       itemsrc_minordqty,"
                 "       itemsrc_multordqty,"
                 "       itemsrc_invvendoruomratio,"
                 "       (CURRENT_DATE + itemsrc_leadtime) AS earliestdate, "
                 "       itemsrc_manuf_name, "
                 "       itemsrc_manuf_item_number, "
                 "       itemsrc_manuf_item_descrip, "
                 "       contrct_number "
				 "FROM itemsrc LEFT OUTER JOIN contrct ON (itemsrc_contrct_id = contrct_id) "
                 "WHERE (itemsrc_id=:itemsrc_id);" );
      src.bindValue(":itemsrc_id", pItemsrcid);
      src.exec();
      if (src.first())
      {
        _itemsrcid = src.value("itemsrc_id").toInt();
        _item->setId(src.value("itemsrc_item_id").toInt());
  
        _vendorItemNumber->setText(src.value("itemsrc_vend_item_number").toString());
        _vendorDescrip->setText(src.value("itemsrc_vend_item_descrip").toString());
        _vendorUOM->setText(src.value("itemsrc_vend_uom").toString());
        _uom->setText(src.value("itemsrc_vend_uom").toString());
        _minOrderQty->setDouble(src.value("itemsrc_minordqty").toDouble());
        _orderQtyMult->setDouble(src.value("itemsrc_multordqty").toDouble());
        _invVendorUOMRatio->setDouble(src.value("itemsrc_invvendoruomratio").toDouble());
        _earliestDate->setDate(src.value("earliestdate").toDate());

        _invVendUOMRatio = src.value("itemsrc_invvendoruomratio").toDouble();
        _minimumOrder = src.value("itemsrc_minordqty").toDouble();
        _orderMultiple = src.value("itemsrc_multordqty").toDouble();
        _contrctNumber->setText(src.value("contrct_number").toString());
        
        _manufName->setCode(src.value("itemsrc_manuf_name").toString());
        _manufItemNumber->setText(src.value("itemsrc_manuf_item_number").toString());
        _manufItemDescrip->setText(src.value("itemsrc_manuf_item_descrip").toString());

        if (_ordered->toDouble() != 0)
          sDeterminePrice();

        _ordered->setFocus();

        if(_metrics->boolean("UseEarliestAvailDateOnPOItem") && _dueDate->date() < _earliestDate->date())
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
      _uom->setText(_item->uom());
      _minOrderQty->clear();
      _orderQtyMult->clear();
      _invVendorUOMRatio->setDouble(1.0);
      _earliestDate->setDate(omfgThis->dbDate());
      _manufName->setId(-1);
      _manufItemNumber->clear();
      _manufItemDescrip->clear();
  
      _invVendUOMRatio = 1;
      _minimumOrder = 0;
      _orderMultiple = 0;
    }
  }
}

void purchaseOrderItem::sDeterminePrice()
{
  XSqlQuery purchaseDeterminePrice;
  if ((_orderQtyCache != -1) &&
      (_orderQtyCache != _ordered->toDouble()))
  {
    if (QMessageBox::question(this, tr("Update Price?"),
                tr("<p>The Item qty. has changed. Do you want to update the Price?"),
                QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape) == QMessageBox::No)
    {
      _orderQtyCache = _ordered->toDouble();
	  sPopulateExtPrice();
      return;
    }
  }
   
  if ( (!_overriddenUnitPrice) && 
      (_itemsrcid != -1) && 
      (_ordered->toDouble() != 0.0) &&
      (_orderQtyCache != _ordered->toDouble()))
  {
    purchaseDeterminePrice.prepare( "SELECT itemsrcPrice(:itemsrc_id, :warehous_id, :dropship,"
                                    "                    :qty, :curr_id, :effective) AS new_itemsrc_price;");
    purchaseDeterminePrice.bindValue(":itemsrc_id", _itemsrcid);
    purchaseDeterminePrice.bindValue(":warehous_id", _warehouse->id());
    purchaseDeterminePrice.bindValue(":dropship", _dropship);
    purchaseDeterminePrice.bindValue(":qty", _ordered->toDouble());
    purchaseDeterminePrice.bindValue(":curr_id", _unitPrice->id());
    purchaseDeterminePrice.bindValue(":effective", _unitPrice->effective());
    purchaseDeterminePrice.exec();
    if (purchaseDeterminePrice.first())
      _unitPrice->setLocalValue(purchaseDeterminePrice.value("new_itemsrc_price").toDouble());
    else
      _unitPrice->clear();
  }
  if (_ordered->toDouble() != 0.0)
    _orderQtyCache = _ordered->toDouble();
  sPopulateExtPrice();
  _save->setEnabled(true);
}

void purchaseOrderItem::sInventoryItemToggled( bool yes )
{
  if(yes)
    sPopulateItemSourceInfo(_itemsrcid);
  else
    sPopulateItemSourceInfo(-1);
}

void purchaseOrderItem::sVendorItemNumberSearch()
{
  ParameterList params;
  params.append("vend_id", _vendid);
  if (!_vendorItemNumber->text().isEmpty())
    params.append("search", _vendorItemNumber->text());
  else if (_item->id() != -1)
    params.append("search", _item->itemNumber());
  itemSourceSearch newdlg(this, "", true);
  newdlg.set(params);

  if(newdlg.exec() == XDialog::Accepted)
  {
    int itemsrcid = newdlg.itemsrcId();
    if(itemsrcid != -1)
    {
      _inventoryItem->setChecked(true);
      sPopulateItemSourceInfo(itemsrcid);
    }
    else
    {
      _nonInventoryItem->setChecked(true);
      _expcat->setId(newdlg.expcatId());
      _vendorItemNumber->setText(newdlg.vendItemNumber());
      _vendorDescrip->setText(newdlg.vendItemDescrip());
      _manufName->setText(newdlg.manufName());
      _manufItemNumber->setText(newdlg.manufItemNumber());
      _manufItemDescrip->setText(newdlg.manufItemDescrip());
    }
  }
}

void purchaseOrderItem::sVendorItemNumberList()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("vend_id", _vendid);

  itemSourceList newdlg(this, "", true);
  newdlg.set(params);
  int itemsrcid = newdlg.exec();
  if(itemsrcid > 0)
  {
    _inventoryItem->setChecked(true);
    sPopulateItemSourceInfo(itemsrcid);
  }
}

void purchaseOrderItem::sHandleButtons()
{
  if (_notesButton->isChecked())
    _remarksStack->setCurrentIndex(0);
  else
    _remarksStack->setCurrentIndex(1);
}

void purchaseOrderItem::sVendorListPrices()
{
  ParameterList params;
  params.append("itemsrc_id", _itemsrcid);
  params.append("qty", _ordered->toDouble());
  vendorPriceList newdlg(this, "", true);
  newdlg.set(params);
  if ( (newdlg.exec() == XDialog::Accepted))
  {
    if (_ordered->text().toDouble() < newdlg._selectedQty)
    {
      if (QMessageBox::question(this, tr("Update Quantity?"),
                  tr("<p>You must order at least %1 to qualify for this price. Do you want to update the Quantity?").arg(QString().setNum(newdlg._selectedQty)),
                  QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape) == QMessageBox::No)
        return;
  
      _ordered->setDouble(newdlg._selectedQty);
      _orderQtyCache = -1;
      sDeterminePrice();
    }
  }
}


void purchaseOrderItem::sCalculateTax()
{
  XSqlQuery calcq;

  calcq.prepare("SELECT calculateTax(pohead_taxzone_id,:taxtype_id,pohead_orderdate,pohead_curr_id,ROUND(:ext,2)) AS tax "
                "FROM pohead "
                "WHERE (pohead_id=:pohead_id); " );

  calcq.bindValue(":pohead_id", _poheadid);
  calcq.bindValue(":taxtype_id", _taxtype->id());
  calcq.bindValue(":ext", _extendedPrice->localValue());

  calcq.exec();
  if (calcq.first())
    _tax->setLocalValue(calcq.value("tax").toDouble());

  else if (calcq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  } 
}

void purchaseOrderItem::sTaxDetail()    // new function added from raitem
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("taxzone_id",   _taxzoneid);
  params.append("taxtype_id",  _taxtype->id());
  params.append("date", _tax->effective());
  params.append("curr_id", _tax->id());
  params.append("subtotal", _extendedPrice->localValue());
  //params.append("readOnly");

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    if (_taxtype->id() != newdlg.taxtype())
      _taxtype->setId(newdlg.taxtype());
  }
}
