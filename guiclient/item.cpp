/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "item.h"

#include <QCloseEvent>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "characteristicAssignment.h"
#include "comment.h"
#include "image.h"
#include "itemAlias.h"
#include "itemAvailabilityWorkbench.h"
#include "itemcluster.h"
#include "itemSite.h"
#include "itemSubstitute.h"
#include "itemUOM.h"
#include "itemtax.h"
#include "itemSource.h"
#include "storedProcErrorLookup.h"

const char *_itemTypes[] = { "P", "M", "F", "R", "S", "T", "O", "L", "K", "B", "C", "Y" };

item::item(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  XSqlQuery itemitem;
  setupUi(this);

  _notes->setSpellEnable(true); 

  _mode=0;
  _itemid = -1;
  
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemNumber, SIGNAL(editingFinished()), this, SLOT(sFormatItemNumber()));
  connect(_inventoryUOM, SIGNAL(newID(int)), this, SLOT(sPopulateUOMs()));
  connect(_classcode, SIGNAL(newID(int)), this, SLOT(sPopulateUOMs()));
  connect(_newCharacteristic, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_editCharacteristic, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_deleteCharacteristic, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_itemtype, SIGNAL(activated(int)), this, SLOT(sHandleItemtype()));
  connect(_newAlias, SIGNAL(clicked()), this, SLOT(sNewAlias()));
  connect(_editAlias, SIGNAL(clicked()), this, SLOT(sEditAlias()));
  connect(_deleteAlias, SIGNAL(clicked()), this, SLOT(sDeleteAlias()));
  connect(_newSubstitute, SIGNAL(clicked()), this, SLOT(sNewSubstitute()));
  connect(_editSubstitute, SIGNAL(clicked()), this, SLOT(sEditSubstitute()));
  connect(_deleteSubstitute, SIGNAL(clicked()), this, SLOT(sDeleteSubstitute()));
  connect(_newTransform, SIGNAL(clicked()), this, SLOT(sNewTransformation()));
  connect(_deleteTransform, SIGNAL(clicked()), this, SLOT(sDeleteTransformation()));
  connect(_site, SIGNAL(clicked()), this, SLOT(sEditItemSite()));
  connect(_workbench, SIGNAL(clicked()), this, SLOT(sWorkbench()));
  connect(_deleteItemSite, SIGNAL(clicked()), this, SLOT(sDeleteItemSite()));
  connect(_viewItemSite, SIGNAL(clicked()), this, SLOT(sViewItemSite()));
  connect(_editItemSite, SIGNAL(clicked()), this, SLOT(sEditItemSite()));
  connect(_newItemSite, SIGNAL(clicked()), this, SLOT(sNewItemSite()));
  connect(_itemtaxNew, SIGNAL(clicked()), this, SLOT(sNewItemtax()));
  connect(_itemtaxEdit, SIGNAL(clicked()), this, SLOT(sEditItemtax()));
  connect(_itemtaxDelete, SIGNAL(clicked()), this, SLOT(sDeleteItemtax()));
  connect(_newUOM, SIGNAL(clicked()), this, SLOT(sNewUOM()));
  connect(_editUOM, SIGNAL(clicked()), this, SLOT(sEditUOM()));
  connect(_deleteUOM, SIGNAL(clicked()), this, SLOT(sDeleteUOM()));
  connect(_configured, SIGNAL(toggled(bool)), this, SLOT(sConfiguredToggled(bool)));
  connect(_notesButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_extDescripButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_commentsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_aliasesButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_substitutesButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_transformationsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_editSrc, SIGNAL(clicked()), this, SLOT(sEditSource()));
  connect(_newSrc, SIGNAL(clicked()), this, SLOT(sNewSource()));
  connect(_viewSrc, SIGNAL(clicked()), this, SLOT(sViewSource()));
  connect(_copySrc, SIGNAL(clicked()), this, SLOT(sCopySource()));
  connect(_deleteSrc, SIGNAL(clicked()), this, SLOT(sDeleteSource()));
  connect(_elementsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_bomButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  
  _disallowPlanningType = false;
  _inTransaction = false;

  _listprice->setValidator(omfgThis->priceVal());
  _listcost->setValidator(omfgThis->costVal());
  _prodWeight->setValidator(omfgThis->weightVal());
  _packWeight->setValidator(omfgThis->weightVal());

  _classcode->setAllowNull(TRUE);
  _classcode->setType(XComboBox::ClassCodes);

  _freightClass->setAllowNull(TRUE);
  _freightClass->setType(XComboBox::FreightClasses);

  _prodcat->setAllowNull(TRUE);
  _prodcat->setType(XComboBox::ProductCategories);

  _inventoryUOM->setType(XComboBox::UOMs);

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true, "char_name" );
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true, "charass_value" );
  _charass->addColumn(tr("Default"),        _ynColumn*2,   Qt::AlignCenter, true, "charass_default" );
  _charass->addColumn(tr("List Price"),     _priceColumn,Qt::AlignRight, true, "charass_price" );
  _charass->hideColumn(3);

  _elements = new maintainItemCosts(this, "maintainItemCosts", Qt::Widget);
  _elementsPage->layout()->addWidget(_elements);
  _elements->findChild<QWidget*>("_close")->hide();
  _elements->findChild<QWidget*>("_item")->hide();
  _elements->findChild<QWidget*>("_costingElementsLit")->hide();

  _bom = new BOM(this, "BOM", Qt::Widget);
  _bomPage->layout()->addWidget(_bom);
  _bom->findChild<QWidget*>("_item")->hide();
  _bom->findChild<QWidget*>("_itemLit")->hide();
  _bom->findChild<QWidget*>("_save")->hide();
  _bom->findChild<QWidget*>("_close")->hide();
  _bom->findChild<QWidget*>("_print")->hide();
  _bom->findChild<QWidget*>("_totalsGroup")->hide();
  _bom->findChild<QWidget*>("_costsGroup")->hide();
  _bom->findChild<QWidget*>("_showExpired")->hide();
  _bom->findChild<QWidget*>("_showFuture")->hide();
  _bom->findChild<QWidget*>("_documentNum")->hide();
  _bom->findChild<QWidget*>("_documentNumLit")->hide();
  _bom->findChild<QWidget*>("_batchSize")->hide();
  _bom->findChild<QWidget*>("_batchSizeLit")->hide();
  _bom->findChild<QWidget*>("_moveUp")->hide();
  _bom->findChild<QWidget*>("_moveDown")->hide();

  QPushButton *moreButton = new QPushButton(this);
  moreButton->setText(tr("More"));
  connect(moreButton, SIGNAL(clicked()), this, SLOT(sEditBOM()));
  _bom->findChild<QVBoxLayout*>("_formButtonsLayout")->addWidget(moreButton);

  ParameterList plist;
  if(_privileges->check("MaintainBOMs"))
    plist.append("mode", "edit");
  else if (_privileges->check("ViewBOMs"))
    plist.append("mode", "view");
  _bom->set(plist);

  _uomconv->addColumn(tr("Conversions/Where Used"), _itemColumn*2, Qt::AlignLeft, true, "uomname");
  _uomconv->addColumn(tr("Ratio"),      -1, Qt::AlignRight, true, "uomvalue"  );
  _uomconv->addColumn(tr("Global"),     _ynColumn*2,    Qt::AlignCenter, true, "global" );
  _uomconv->addColumn(tr("Fractional"), _ynColumn*2,   Qt::AlignCenter, true, "fractional" );
  
  _itemsrc->addColumn(tr("Active"),      _dateColumn,   Qt::AlignCenter, true, "itemsrc_active");
  _itemsrc->addColumn(tr("Vendor"),      _itemColumn, Qt::AlignLeft, true, "vend_number" );
  _itemsrc->addColumn(tr("Name"), 	 -1,          Qt::AlignLeft, true, "vend_name" );
  _itemsrc->addColumn(tr("Vendor Item"), _itemColumn, Qt::AlignLeft, true, "itemsrc_vend_item_number" );
  _itemsrc->addColumn(tr("Manufacturer"), _itemColumn, Qt::AlignLeft, true, "itemsrc_manuf_name" );
  _itemsrc->addColumn(tr("Manuf. Item#"), _itemColumn, Qt::AlignLeft, true, "itemsrc_manuf_item_number" );
  _itemsrc->addColumn(tr("Default"),      _dateColumn,   Qt::AlignCenter, true, "default");

  _itemalias->addColumn(tr("Alias Number"), _itemColumn, Qt::AlignLeft, true, "itemalias_number"  );
  _itemalias->addColumn(tr("Comments"),     -1,          Qt::AlignLeft, true, "itemalias_comments" );

  _itemsub->addColumn(tr("Rank"),        _whsColumn,  Qt::AlignCenter, true, "itemsub_rank" );
  _itemsub->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft, true, "item_number"   );
  _itemsub->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "item_descrip1" );
  _itemsub->addColumn(tr("Ratio"),       _qtyColumn,  Qt::AlignRight, true, "itemsub_uomratio" );

  _itemtrans->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft, true, "item_number"   );
  _itemtrans->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "item_descrip"   );

  _itemSite->addColumn(tr("Active"),        _dateColumn, Qt::AlignCenter, true, "itemsite_active" );
  _itemSite->addColumn(tr("Site"),          _whsColumn,  Qt::AlignCenter, true, "warehous_code" );
  _itemSite->addColumn(tr("Description"),   -1,          Qt::AlignLeft, true, "warehous_descrip"   );
  _itemSite->addColumn(tr("Cntrl. Method"), _itemColumn, Qt::AlignCenter, true, "itemsite_controlmethod" );
  _itemSite->setDragString("itemsiteid=");

  connect(omfgThis, SIGNAL(itemsitesUpdated()), SLOT(sFillListItemSites()));

  _itemtax->addColumn(tr("Tax Type"),_itemColumn, Qt::AlignLeft,true,"taxtype_name");
  _itemtax->addColumn(tr("Tax Zone"),    -1, Qt::AlignLeft,true,"taxzone");

  if (_privileges->check("MaintainItemSources"))
  {
    connect(_itemsrc, SIGNAL(valid(bool)), _editSrc, SLOT(setEnabled(bool)));
    connect(_itemsrc, SIGNAL(valid(bool)), _viewSrc, SLOT(setEnabled(bool)));
    connect(_itemsrc, SIGNAL(valid(bool)), _copySrc, SLOT(setEnabled(bool)));
    connect(_itemsrc, SIGNAL(valid(bool)), _deleteSrc, SLOT(setEnabled(bool)));
    connect(_itemsrc, SIGNAL(itemSelected(int)), _editSrc, SLOT(animateClick()));
    _newSrc->setEnabled(_privileges->check("MaintainItemSources"));
  }
  else if (_privileges->check("ViewItemSources"))
  {
    connect(_itemsrc, SIGNAL(valid(bool)), _viewSrc, SLOT(setEnabled(bool)));
    connect(_itemsrc, SIGNAL(itemSelected(int)), _viewSrc, SLOT(animateClick()));
  }
  else
    _tab->setTabEnabled(_tab->indexOf(_sourcesTab), FALSE);

  if (!_metrics->boolean("Transforms"))
    _transformationsButton->hide();
    
  if (!_metrics->boolean("MultiWhs"))
    _tab->removeTab(_tab->indexOf(_itemsitesTab));
  else if (_privileges->check("MaintainItemSites"))
  {
    connect(_itemSite, SIGNAL(valid(bool)), _editItemSite, SLOT(setEnabled(bool)));
    connect(_itemSite, SIGNAL(valid(bool)), _viewItemSite, SLOT(setEnabled(bool)));
    connect(_itemSite, SIGNAL(itemSelected(int)), _editItemSite, SLOT(animateClick()));
    _newItemSite->setEnabled(_privileges->check("MaintainItemSites"));
  }
  else if (_privileges->check("ViewItemSites"))
  {
    connect(_itemSite, SIGNAL(valid(bool)), _viewItemSite, SLOT(setEnabled(bool)));
    connect(_itemSite, SIGNAL(itemSelected(int)), _viewItemSite, SLOT(animateClick()));
  }
  else
    _tab->setTabEnabled(_tab->indexOf(_itemsitesTab), FALSE);

  itemitem.exec("SELECT uom_name FROM uom WHERE (uom_item_weight);");
  if (itemitem.first())
  {
    QString title (tr("Weight in "));
    title += itemitem.value("uom_name").toString();
    _weightGroup->setTitle(title);
  }

#ifdef Q_WS_MAC
  _tab->setUsesScrollButtons(true);
  _tab->setElideMode(Qt::ElideNone);
#endif

  // TO DO: Implement later
  _taxRecoverable->hide();
}

item::~item()
{
  // no need to delete child widgets, Qt does it all for us
}

void item::languageChange()
{
    retranslateUi(this);
}

enum SetResponse item::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    setId(param.toInt());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _tab->setEnabled(false);

      
      setObjectName("item new");
      _mode = cNew;

      _newUOM->setEnabled(false);
	  _print->hide();

      itemet.exec("SELECT NEXTVAL('item_item_id_seq') AS item_id");
      if (itemet.first())
        _itemid = itemet.value("item_id").toInt();
//  ToDo

      _comments->setId(_itemid);
      _documents->setId(_itemid);
      _exclusive->setChecked(_metrics->boolean("DefaultSoldItemsExclusive"));

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      connect(_uomconv, SIGNAL(valid(bool)), _editUOM, SLOT(setEnabled(bool)));
      connect(_uomconv, SIGNAL(valid(bool)), _deleteUOM, SLOT(setEnabled(bool)));
      connect(_itemalias, SIGNAL(valid(bool)), _editAlias, SLOT(setEnabled(bool)));
      connect(_itemalias, SIGNAL(valid(bool)), _deleteAlias, SLOT(setEnabled(bool)));
      connect(_itemsub, SIGNAL(valid(bool)), _editSubstitute, SLOT(setEnabled(bool)));
      connect(_itemsub, SIGNAL(valid(bool)), _deleteSubstitute, SLOT(setEnabled(bool)));
      connect(_itemtrans, SIGNAL(valid(bool)), _deleteTransform, SLOT(setEnabled(bool)));
      connect(_itemtax, SIGNAL(valid(bool)), _itemtaxEdit, SLOT(setEnabled(bool)));
      connect(_itemtax, SIGNAL(valid(bool)), _itemtaxDelete, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _tab->setEnabled(true);

      setObjectName(QString("item edit %1").arg(_itemid));
      _mode = cEdit;

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      connect(_uomconv, SIGNAL(valid(bool)), _editUOM, SLOT(setEnabled(bool)));
      connect(_uomconv, SIGNAL(valid(bool)), _deleteUOM, SLOT(setEnabled(bool)));
      connect(_itemalias, SIGNAL(valid(bool)), _editAlias, SLOT(setEnabled(bool)));
      connect(_itemalias, SIGNAL(valid(bool)), _deleteAlias, SLOT(setEnabled(bool)));
      connect(_itemsub, SIGNAL(valid(bool)), _editSubstitute, SLOT(setEnabled(bool)));
      connect(_itemsub, SIGNAL(valid(bool)), _deleteSubstitute, SLOT(setEnabled(bool)));
      connect(_itemtrans, SIGNAL(valid(bool)), _deleteTransform, SLOT(setEnabled(bool)));
      connect(_itemtax, SIGNAL(valid(bool)), _itemtaxEdit, SLOT(setEnabled(bool)));
      connect(_itemtax, SIGNAL(valid(bool)), _itemtaxDelete, SLOT(setEnabled(bool)));

      if (_privileges->check("MaintainItemSites"))
      {
        connect(_itemSite, SIGNAL(valid(bool)), _editItemSite, SLOT(setEnabled(bool)));
        connect(_itemSite, SIGNAL(itemSelected(int)), _editItemSite, SLOT(animateClick()));
      }
      else
      {
        connect(_itemSite, SIGNAL(itemSelected(int)), _viewItemSite, SLOT(animateClick()));
      }

      if (_privileges->check("DeleteItemSites"))
        connect(_itemSite, SIGNAL(valid(bool)), _deleteItemSite, SLOT(setEnabled(bool)));

      _itemNumber->setEnabled(FALSE);
    }
    else if (param.toString() == "view")
    {
      _tab->setEnabled(true);

      setObjectName(QString("item view %1").arg(_itemid));
      _mode = cView;

      _itemNumber->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _description1->setEnabled(FALSE);
      _description2->setEnabled(FALSE);
      _maximumDesiredCost->setEnabled(FALSE);
      _itemtype->setEnabled(FALSE);
      _sold->setEnabled(FALSE);
      _pickListItem->setEnabled(FALSE);
      _fractional->setEnabled(FALSE);
      _classcode->setEnabled(FALSE);
      _freightClass->setEnabled(FALSE);
      _inventoryUOM->setEnabled(FALSE);
      _prodWeight->setEnabled(FALSE);
      _packWeight->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _documents->setReadOnly(TRUE);
      _extDescription->setReadOnly(TRUE);
      _newCharacteristic->setEnabled(FALSE);
      _newAlias->setEnabled(FALSE);
      _newSubstitute->setEnabled(FALSE);
      _newTransform->setEnabled(FALSE);
      _taxRecoverable->setEnabled(FALSE);
      _itemtaxNew->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _newSrc->setEnabled(false);
      _newUOM->setEnabled(false);
      _upcCode->setEnabled(false);
      _newItemSite->setEnabled(FALSE);
      _bom->findChild<QWidget*>("_new")->hide();
      _elements->findChild<QWidget*>("_new")->hide();

      disconnect(_itemalias, SIGNAL(valid(bool)), _editAlias, SLOT(setEnabled(bool)));
      disconnect(_itemalias, SIGNAL(valid(bool)), _deleteAlias, SLOT(setEnabled(bool)));
      disconnect(_itemsub, SIGNAL(valid(bool)), _editSubstitute, SLOT(setEnabled(bool)));
      disconnect(_itemsub, SIGNAL(valid(bool)), _deleteSubstitute, SLOT(setEnabled(bool)));
      disconnect(_itemtrans, SIGNAL(valid(bool)), _deleteTransform, SLOT(setEnabled(bool)));

      _save->hide();
    }
  }

  sHandleRightButtons();

  return NoError;
}

void item::saveCore()
{
  XSqlQuery itemaveCore;
  if(cNew != _mode || _inTransaction)
    return;

  if (_inventoryUOM->id() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must select an Inventory Unit of Measure for this Item before continuing.")  );
    _inventoryUOM->setFocus();
    return;
  }
  
  else if (_classcode->id() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must select a Class Code before continuing.")  );
    _classcode->setFocus();
    return;
  }
  
  else
  {
    itemaveCore.exec("BEGIN;");
    _inTransaction = true;

    itemaveCore.prepare("INSERT INTO item"
            "      (item_id, item_number, item_Descrip1, item_descrip2,"
            "       item_classcode_id,"
            "       item_picklist, item_sold, item_fractional, item_active,"
            "       item_type,"
            "       item_prodweight, item_packweight, item_prodcat_id,"
            "       item_exclusive, item_listprice, item_listcost, item_maxcost,"
            "       item_inv_uom_id, item_price_uom_id)"
            "VALUES(:item_id, :item_number, '', '',"
            "       :item_classcode_id,"
            "       false, false, false, :item_active,"
            "       :item_type,"
            "       0.0, 0.0, -1,"
            "       true, 0.0, 0.0, 0.0,"
            "       :item_inv_uom_id, :item_inv_uom_id);");
    itemaveCore.bindValue(":item_id", _itemid);
    itemaveCore.bindValue(":item_number", _itemNumber->text().trimmed().toUpper());
    itemaveCore.bindValue(":item_type", _itemTypes[_itemtype->currentIndex()]);
    itemaveCore.bindValue(":item_classcode_id", _classcode->id());
    itemaveCore.bindValue(":item_inv_uom_id", _inventoryUOM->id());
    itemaveCore.bindValue(":item_active", QVariant(_active->isChecked()));
    if(!itemaveCore.exec() || itemaveCore.lastError().type() != QSqlError::NoError)
    {
      itemaveCore.exec("ROLLBACK;");
      _inTransaction = false;
      return;
    }

    sPopulateUOMs();
    // TODO: We can enable certain functionality here that needs a saved record
    _newUOM->setEnabled(true);
    _tab->setEnabled(true);

    sHandleRightButtons();
  }
} 

void item::sSave()
{
  XSqlQuery itemSave;
  QString sql;
  QString itemNumber = _itemNumber->text().trimmed().toUpper();

  //Check To see if the item has active sites associated with it
  int fActive = false;
  itemSave.prepare("SELECT itemsite_id "
            "FROM itemsite "
            "WHERE ((itemsite_item_id=:item_id)"
            "  AND  (itemsite_active)) "
            "LIMIT 1; ");
  itemSave.bindValue(":item_id", _itemid);
  itemSave.exec();
  if (itemSave.first()) fActive = true;


  if(!_active->isChecked())
  {
    itemSave.prepare("SELECT bomitem_id "
              "FROM bomitem, item "
              "WHERE ((bomitem_parent_item_id=item_id) "
              "AND (item_active) "
              "AND (bomitem_expires > current_date) "
              "AND (getActiveRevId('BOM',bomitem_parent_item_id)=bomitem_rev_id) "
              "AND (bomitem_item_id=:item_id)) "
              "LIMIT 1; ");
    itemSave.bindValue(":item_id", _itemid);
    itemSave.exec();
    if (itemSave.first())         
    { 
      QMessageBox::warning( this, tr("Cannot Save Item"),
        tr("This Item is used in an active Bill of Materials and must be marked as active. "
        "Expire the Bill of Material items to allow this Item to not be active.") );
      return;
    }


    if (fActive)
    { 
      QMessageBox::warning( this, tr("Cannot Save Item"),
        tr("This Item is used in an active Item Site and must be marked as active. "
        "Deactivate the Item Sites to allow this Item to not be active.") );
      return;
    }

    itemSave.prepare("SELECT itemsrc_id "
              "FROM itemsrc "
              "WHERE ((itemsrc_item_id=:item_id)"
              "  AND  (itemsrc_active)) "
              "LIMIT 1; ");
    itemSave.bindValue(":item_id", _itemid);
    itemSave.exec();
    if (itemSave.first())         
    { 
      QMessageBox::warning( this, tr("Cannot Save Item"),
        tr("This Item is used in an active Item Source and must be marked as active. "
        "Deactivate the Item Sources to allow this Item to not be active.") );
      return;
    }
  }

  if(_disallowPlanningType && QString(_itemTypes[_itemtype->currentIndex()]) == "L")
  {
    QMessageBox::warning( this, tr("Planning Type Disallowed"),
      tr("This item is part of one or more Bills of Materials and cannot be a Planning Item.") );
    return;
  }

  if(QString(_itemTypes[_itemtype->currentIndex()]) == "K" && !_sold->isChecked())
  {
    QMessageBox::warning( this, tr("Must be Sold"),
      tr("Kit item types must be Sold. Please mark this item as sold and set the appropriate options and save again.") );
    return;
  }
  
  if(!_sold->isChecked())
  {
    itemSave.prepare("SELECT bomitem_id "
              "FROM bomitem, item "
              "WHERE ((bomitem_parent_item_id=item_id) "
              "AND (item_active) "
              "AND (item_type='K') "
              "AND (bomitem_expires > current_date) "
              "AND (getActiveRevId('BOM',bomitem_parent_item_id)=bomitem_rev_id) "
              "AND (bomitem_item_id=:item_id)) "
              "LIMIT 1; ");
    itemSave.bindValue(":item_id", _itemid);
    itemSave.exec();
    if (itemSave.first())         
    { 
      QMessageBox::warning( this, tr("Cannot Save Item"),
        tr("This item is used in an active bill of materials for a kit and must be marked as sold. "
        "Expire the bill of material items or deactivate the kit items to allow this item to not be sold.") );
      return;
    }
  }

  if (cEdit == _mode && _itemtype->currentText() != _originalItemType && QString(_itemTypes[_itemtype->currentIndex()]) == "K")
  {
    itemSave.prepare("SELECT bomitem_id "
              "FROM bomitem, item "
              "WHERE ((bomitem_item_id=item_id) "
              "AND (item_active) "
              "AND (NOT item_sold) "
              "AND (bomitem_expires > current_date) "
              "AND (getActiveRevId('BOM',bomitem_parent_item_id)=bomitem_rev_id) "
              "AND (bomitem_parent_item_id=:item_id)) "
              "LIMIT 1; ");
    itemSave.bindValue(":item_id", _itemid);
    itemSave.exec();
    if (itemSave.first())         
    { 
      if(QMessageBox::question( this, tr("BOM Items should be marked as Sold"),
                                tr("<p>You have changed the Item Type of this "
                                   "Item to Kit. This Item has BOM Items associated "
                                   "with it that are not marked as Sold. "
                                   "Do you wish to continue saving?"),
                                QMessageBox::Ok,
                                QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default ) == QMessageBox::Cancel)
        return;
    }
  }

  if (_mode == cEdit)
  {
    itemSave.prepare( "SELECT item_id "
               "FROM item "
               "WHERE ( (item_number=:item_number)"
               " AND (item_id <> :item_id) );" );
    itemSave.bindValue(":item_number", itemNumber);
    itemSave.bindValue(":item_id", _itemid);
    itemSave.exec();
    if (itemSave.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Item"),
                            tr("You may not rename this Item to the entered Item Number as it is in use by another Item.") );
      _itemNumber->setFocus();
      return;
    }
  }

  if (_itemNumber->text().length() == 0)
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must enter a Item Number for this Item before continuing.") );
    _itemNumber->setFocus();
    return;
  }

  if (_itemtype->currentIndex() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must select an Item Type for this Item Type before continuing.")  );
    _itemtype->setFocus();
    return;
  }

  if (_classcode->currentIndex() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must select a Class Code for this Item before continuing.") );
    _classcode->setFocus();
    return;
  }

  if (_inventoryUOM->id() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must select an Inventory Unit of Measure for this Item before continuing.")  );
    _inventoryUOM->setFocus();
    return;
  }

  if ((_sold->isChecked()) && (_prodcat->id() == -1))
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must select a Product Category for this Sold Item before continuing.")  );
    _prodcat->setFocus();
    return;
  }

  if ((_sold->isChecked()) && (_priceUOM->id() == -1))
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must select a Selling UOM for this Sold Item before continuing.") );
    return;
  }

  if (_classcode->id() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must select a Class Code for this Item before continuing.")  );
    _classcode->setFocus();
    return;
  }

  if (cEdit == _mode && _itemtype->currentText() != _originalItemType)
  {
    if ((QString(_itemTypes[_itemtype->currentIndex()]) == "R") ||
        (QString(_itemTypes[_itemtype->currentIndex()]) == "S") ||
        (QString(_itemTypes[_itemtype->currentIndex()]) == "T") ||
        (QString(_itemTypes[_itemtype->currentIndex()]) == "F"))
        {
      itemSave.prepare( "SELECT itemsite_id "
                 "  FROM itemsite "
                 " WHERE ((itemsite_item_id=:item_id) "
				 " AND (itemsite_qtyonhand + qtyallocated(itemsite_id,startoftime(),endoftime()) +"
		         "      qtyordered(itemsite_id,startoftime(),endoftime()) > 0 ));" );
      itemSave.bindValue(":item_id", _itemid);
      itemSave.exec();
      if (itemSave.first())
      {
        QMessageBox::information(this, tr("Cannot Save Item"),
                                 tr("<p>This Item has Item Sites with either "
                                    "on hand quantities or pending inventory "
                                    "activity. This item type does not allow "
                                    "on hand balances or inventory activity."));
        _itemtype->setFocus();
        return;
      }
        }

    itemSave.prepare( "SELECT itemcost_id "
               "  FROM itemcost "
               " WHERE (itemcost_item_id=:item_id);" );
    itemSave.bindValue(":item_id", _itemid);
    itemSave.exec();
    if (itemSave.first())
    {
      if(QMessageBox::question( this, tr("Item Costs Exist"),
                                tr("<p>You have changed the Item Type of this "
                                   "Item. This Item has Item Costs associated "
                                   "with it that will be deleted before this "
                                   "change may occur. Do you wish to continue "
                                   "saving and delete the Item Costs?"),
                                QMessageBox::Ok,
                                QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default ) == QMessageBox::Cancel)
        return;
	}

    itemSave.prepare( "SELECT itemsite_id "
               "  FROM itemsite "
               " WHERE (itemsite_item_id=:item_id);" );
    itemSave.bindValue(":item_id", _itemid);
    itemSave.exec();
    if (itemSave.first())
    {
      if(QMessageBox::question( this, tr("Item Sites Exist"),
                                tr("<p>You have changed the Item Type of this "
                                   "Item. To ensure Item Sites do not have "
                                   "invalid settings, all Item Sites for it "
                                   "will be inactivated before this change "
                                   "may occur.  You may afterward edit "
                                   "the Item Sites, enter valid information, "
                                   "and reactivate them.  Do you wish to "
                                   "continue saving and inactivate the Item "
                                   "Sites?"),
                                QMessageBox::Ok,
                                QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default ) == QMessageBox::Cancel)
        return;
    }
  }

  // look for all of the uom ids we have associated with this item
  QStringList knownunits;
  itemSave.prepare("SELECT :uom_id AS uom_id "
            "UNION "
            "SELECT itemuomconv_from_uom_id "
            "FROM itemuomconv "
            "WHERE (itemuomconv_item_id=:item_id) "
            "UNION "
            "SELECT itemuomconv_to_uom_id "
            "FROM itemuomconv "
            "WHERE (itemuomconv_item_id=:item_id);"
           );
  itemSave.bindValue(":item_id", _itemid);
  itemSave.bindValue(":uom_id", _inventoryUOM->id());
  itemSave.exec();
  while (itemSave.next())
    knownunits.append(itemSave.value("uom_id").toString());
  if (itemSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sql = "SELECT DISTINCT uom_name "
        "FROM uomUsedForItem(<? value(\"item_id\") ?>) "
        "WHERE (uom_id NOT IN (<? literal(\"knownunits\") ?>));" ;

  MetaSQLQuery mql(sql);
  ParameterList params;
  params.append("item_id", _itemid);
  params.append("knownunits", knownunits.join(", "));
  itemSave = mql.toQuery(params);
  itemSave.exec();
  QStringList missingunitnames;
  while (itemSave.next())
    missingunitnames.append(itemSave.value("uom_name").toString());
  if (itemSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if (missingunitnames.size() > 0)
  {
    int answer = QMessageBox::question(this, tr("Add conversions?"),
                            tr("<p>There are records referring to this Item "
                               "that do not have UOM Conversions to %1. Would "
                               "you like to create UOM Conversions now?<p>If "
                               "you  answer No then the Item will be saved and "
                               "you may encounter errors later. The units "
                               "without conversions are:<br>%2")
                                .arg(_inventoryUOM->currentText())
                                .arg(missingunitnames.join(", ")),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No);
    if (answer == QMessageBox::Yes)
    {
      _tab->setCurrentIndex(_tab->indexOf(_tabUOM));
      return;
    }
  }

  int sourceItemid = _itemid;

  if (_mode == cCopy)
  {
    itemSave.exec("SELECT NEXTVAL('item_item_id_seq') AS _item_id");
    if (itemSave.first())
       _itemid = itemSave.value("_item_id").toInt();
// ToDo
  }

  if ( (_mode == cNew && !_inTransaction) || (_mode == cCopy) )
         sql = "INSERT INTO item "
               "( item_id, item_number, item_active,"
               "  item_descrip1, item_descrip2,"
               "  item_type, item_inv_uom_id, item_classcode_id,"
               "  item_picklist, item_sold, item_fractional,"
               "  item_maxcost, item_prodweight, item_packweight,"
               "  item_prodcat_id, item_price_uom_id,"
               "  item_exclusive,"
               "  item_listprice, item_listcost,"
               "  item_upccode, item_config,"
               "  item_comments, item_extdescrip, item_warrdays, item_freightclass_id,"
               "  item_tax_recoverable ) "
               "VALUES "
               "( :item_id, :item_number, :item_active,"
               "  :item_descrip1, :item_descrip2,"
               "  :item_type, :item_inv_uom_id, :item_classcode_id,"
               "  :item_picklist, :item_sold, :item_fractional,"
               "  :item_maxcost, :item_prodweight, :item_packweight,"
               "  :item_prodcat_id, :item_price_uom_id,"
               "  :item_exclusive,"
               "  :item_listprice, :item_listcost,"
               "  :item_upccode, :item_config,"
               "  :item_comments, :item_extdescrip, :item_wardays, :item_freightclass_id,"
               "  :item_tax_recoverable );" ;
  else if ((_mode == cEdit) || (cNew == _mode && _inTransaction))
         sql = "UPDATE item "
               "SET item_number=:item_number, item_descrip1=:item_descrip1, item_descrip2=:item_descrip2,"
               "    item_type=:item_type, item_inv_uom_id=:item_inv_uom_id, item_classcode_id=:item_classcode_id,"
               "    item_picklist=:item_picklist, item_sold=:item_sold, item_fractional=:item_fractional,"
               "    item_active=:item_active,"
               "    item_maxcost=:item_maxcost, item_prodweight=:item_prodweight, item_packweight=:item_packweight,"
               "    item_prodcat_id=:item_prodcat_id,"
               "    item_price_uom_id=:item_price_uom_id,"
               "    item_exclusive=:item_exclusive,"
               "    item_listprice=:item_listprice, item_listcost=:item_listcost,"
               "    item_upccode=:item_upccode, item_config=:item_config,"
               "    item_comments=:item_comments, item_extdescrip=:item_extdescrip, item_warrdays=:item_warrdays,"
               "    item_freightclass_id=:item_freightclass_id,"
               "    item_tax_recoverable=:item_tax_recoverable "
               "WHERE (item_id=:item_id);";
  itemSave.prepare(sql);
  itemSave.bindValue(":item_id", _itemid);
  itemSave.bindValue(":item_number", itemNumber);
  itemSave.bindValue(":item_descrip1", _description1->text());
  itemSave.bindValue(":item_descrip2", _description2->text());
  itemSave.bindValue(":item_type", _itemTypes[_itemtype->currentIndex()]);
  itemSave.bindValue(":item_classcode_id", _classcode->id());
  itemSave.bindValue(":item_sold", QVariant(_sold->isChecked()));
  itemSave.bindValue(":item_prodcat_id", _prodcat->id());
  itemSave.bindValue(":item_exclusive", QVariant(_exclusive->isChecked()));
  itemSave.bindValue(":item_price_uom_id", _priceUOM->id());
  itemSave.bindValue(":item_listprice", _listprice->toDouble());
  itemSave.bindValue(":item_listcost", _listcost->toDouble());
  itemSave.bindValue(":item_upccode", _upcCode->text());
  itemSave.bindValue(":item_active", QVariant(_active->isChecked()));
  itemSave.bindValue(":item_picklist", QVariant(_pickListItem->isChecked()));
  itemSave.bindValue(":item_fractional", QVariant(_fractional->isChecked()));
  itemSave.bindValue(":item_config", QVariant(_configured->isChecked()));  
  itemSave.bindValue(":item_inv_uom_id", _inventoryUOM->id());
  itemSave.bindValue(":item_maxcost", _maximumDesiredCost->localValue());
  itemSave.bindValue(":item_prodweight", _prodWeight->toDouble());
  itemSave.bindValue(":item_packweight", _packWeight->toDouble());
  itemSave.bindValue(":item_comments", _notes->toPlainText());
  itemSave.bindValue(":item_extdescrip", _extDescription->toPlainText());
  itemSave.bindValue(":item_warrdays", _warranty->value());
  if (_freightClass->isValid())
    itemSave.bindValue(":item_freightclass_id", _freightClass->id());
  itemSave.bindValue(":item_tax_recoverable", QVariant(_taxRecoverable->isChecked()));
  itemSave.exec();
  if (itemSave.lastError().type() != QSqlError::NoError)
  {
    itemSave.exec("ROLLBACK;");
    _inTransaction = false;
    systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cCopy)
  {
//  Copy all of the costs for this item
    itemSave.prepare( "INSERT INTO itemcost "
               "(itemcost_item_id, itemcost_costelem_id, itemcost_lowlevel,"
               " itemcost_stdcost, itemcost_posted, itemcost_actcost, "
	       " itemcost_updated, itemcost_curr_id ) "
               "SELECT :item_id, itemcost_costelem_id, itemcost_lowlevel,"
               "       itemcost_stdcost, itemcost_posted, itemcost_actcost, "
	       "       itemcost_updated, itemcost_curr_id "
               "FROM itemcost "
               "WHERE (itemcost_item_id=:sourceItem_id);" );
    itemSave.bindValue(":item_id", _itemid);
    itemSave.bindValue(":sourceItem_id", sourceItemid);
    itemSave.exec();
  }

  if(_inTransaction)
  {
    itemSave.exec("COMMIT;");
    _inTransaction = false;
  }

  omfgThis->sItemsUpdated(_itemid, TRUE);

  if ((!fActive) &&
     ( (_mode == cNew) || (_mode == cCopy) ) &&
     (_privileges->check("MaintainItemSites")) &&
     ( (*_itemTypes[_itemtype->currentIndex()] != 'B') ||
     (*_itemTypes[_itemtype->currentIndex()] != 'F') ||
     (*_itemTypes[_itemtype->currentIndex()] != 'R') ||
     (*_itemTypes[_itemtype->currentIndex()] != 'S') ) )
  {
    if (QMessageBox::information( this, tr("Create New Item Sites"),
                                  tr("Would you like to create Item site inventory settings for the newly created Item now?"),
                                  tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
    {
      ParameterList params;
      params.append("mode", "new");
      params.append("item_id", _itemid);

      itemSite newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();
    }
  }

  if(_bomButton->isEnabled() && _bomButton->isChecked())
    _bom->sSave();

  emit saved(_itemid);

  close();
}

void item::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);
  if (_configured->isChecked())
    params.append("showPrices", TRUE);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void item::sEdit()
{
  QString itemType = QString(*(_itemTypes + _itemtype->currentIndex()));
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());
  if (_configured->isChecked())
    params.append("showPrices", TRUE);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void item::sDelete()
{
  XSqlQuery itemDelete;
  itemDelete.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  itemDelete.bindValue(":charass_id", _charass->id());
  itemDelete.exec();

  sFillList();
}

void item::sFillList()
{
  XSqlQuery itemFillList;
  itemFillList.prepare( "SELECT charass_id, char_name, "
             " CASE WHEN char_type < 2 THEN "
             "   charass_value "
             " ELSE "
             "   formatDate(charass_value::date) "
             "END AS charass_value, "
             " charass_default, "
             " charass_price, 'salesprice' AS charass_price_xtnumericrole "
             "FROM charass, char "
             "WHERE ( (charass_target_type='I')"
             " AND (charass_char_id=char_id)"
             " AND (charass_target_id=:item_id) ) "
             "ORDER BY char_order, char_name;" );
  itemFillList.bindValue(":item_id", _itemid);
  itemFillList.exec();
  _charass->populate(itemFillList);
}

void item::sPrint()
{
  if (_itemid != -1)
  {
    ParameterList params;
    params.append("item_id", _itemid);
    params.append("print");

    orReport report("ItemMaster", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
//  ToDo
}

void item::sFormatItemNumber()
{
  XSqlQuery itemFormatItemNumber;
  if ((_mode == cNew) && (_itemNumber->text().length()))
  {
    _itemNumber->setText(_itemNumber->text().trimmed().toUpper());

  //  Check to see if this item exists
    itemFormatItemNumber.prepare( "SELECT item_id "
               "FROM item "
               "WHERE (item_number=:item_number);" );
    itemFormatItemNumber.bindValue(":item_number", _itemNumber->text());
    itemFormatItemNumber.exec();
    if (itemFormatItemNumber.first())
    {
      _mode = cEdit;
      ParameterList params;
      params.append("item_id", itemFormatItemNumber.value("item_id").toInt());
      params.append("mode", "edit");
      set(params);
    }
    else
    {
      _itemNumber->setEnabled(FALSE);
      _mode = cNew;
    }
  }
}

void item::populate()
{
  XSqlQuery item;
  item.prepare( "SELECT *,"
                "       ( (item_type IN ('P', 'M',  'C', 'Y', 'R', 'A')) AND (item_sold) ) AS sold "
                "FROM item "
                "WHERE (item_id=:item_id);" );
  item.bindValue(":item_id", _itemid);
  item.exec();
  if (item.first())
  {
    if (_mode != cCopy)
      _itemNumber->setText(item.value("item_number"));

    for (int counter = 0; counter < _itemtype->count(); counter++)
    {
      if (QString(item.value("item_type").toString()[0]) == _itemTypes[counter])
      {
        _itemtype->setCurrentIndex(counter);
        sHandleItemtype();
      }
    }
    _originalItemType = _itemtype->currentText();

    XSqlQuery checkBOM;
    if(cCopy != _mode && QString(item.value("item_type").toString()[0]) == "L")
    {
      checkBOM.prepare("SELECT bomitem_id FROM bomitem WHERE (bomitem_parent_item_id=:item_id); ");
      checkBOM.bindValue(":item_id", _itemid);
      checkBOM.exec();
      if(checkBOM.first())
        _itemtype->setEnabled(false);
    }
    else
    {
      checkBOM.prepare("SELECT bomitem_id FROM bomitem WHERE (bomitem_item_id=:item_id); ");
      checkBOM.bindValue(":item_id", _itemid);
      checkBOM.exec();
      if(checkBOM.first())
        _disallowPlanningType = true;
    }

    _active->setChecked(item.value("item_active").toBool());
    _description1->setText(item.value("item_descrip1"));
    _description2->setText(item.value("item_descrip2"));
    _classcode->setId(item.value("item_classcode_id").toInt());
    _freightClass->setId(item.value("item_freightclass_id").toInt());
    _inventoryUOM->setId(item.value("item_inv_uom_id").toInt());
    _pickListItem->setChecked(item.value("item_picklist").toBool());
    _fractional->setChecked(item.value("item_fractional").toBool());
    _configured->setChecked(item.value("item_config").toBool());
    _prodWeight->setDouble(item.value("item_prodweight").toDouble());
    _packWeight->setDouble(item.value("item_packweight").toDouble());
    _maximumDesiredCost->setLocalValue(item.value("item_maxcost").toDouble());
    _notes->setText(item.value("item_comments").toString());
    _extDescription->setText(item.value("item_extdescrip").toString());
    _sold->setChecked(item.value("item_sold").toBool());
    _prodcat->setId(item.value("item_prodcat_id").toInt());
    _upcCode->setText(item.value("item_upccode"));
    _exclusive->setChecked(item.value("item_exclusive").toBool());
    _listprice->setDouble(item.value("item_listprice").toDouble());
    _listcost->setDouble(item.value("item_listcost").toDouble());
    _priceUOM->setId(item.value("item_price_uom_id").toInt());
    _warranty->setValue(item.value("item_warrdays").toInt());
    _taxRecoverable->setChecked(item.value("item_tax_recoverable").toBool());

    sFillList();
    _bom->findChild<ItemCluster*>("_item")->setId(_itemid);
    _elements->findChild<ItemCluster*>("_item")->setId(_itemid);
    sFillUOMList();
    sFillSourceList();
    sFillAliasList();
    sFillSubstituteList();
    sFillTransformationList();
    sFillListItemSites();
    sFillListItemtax();
    _comments->setId(_itemid);
    _documents->setId(_itemid);
  }
//  ToDo
}

void item::clear()
{
  XSqlQuery itemclear;
  _disallowPlanningType = false;
  itemclear.exec("SELECT NEXTVAL('item_item_id_seq') AS item_id");
  if (itemclear.first())
    _itemid = itemclear.value("item_id").toInt();
//  ToDo

  _itemNumber->clear();
  _description1->clear();
  _description2->clear();
  _inventoryUOM->clear();
  _priceUOM->clear();
  _listprice->clear();
  _listcost->clear();

  _active->setChecked(TRUE);
  _pickListItem->setChecked(TRUE);
  _sold->setChecked(FALSE);
  _exclusive->setChecked(_metrics->boolean("DefaultSoldItemsExclusive"));
  _fractional->setChecked(FALSE);

  _itemtype->setCurrentIndex(0);
  _classcode->setNull();
  _freightClass->setNull();
  _prodcat->setNull();
  _configured->setChecked(false);

  _notes->clear();
  _extDescription->clear();
  _charass->clear();
  _uomconv->clear();
  _comments->setId(_itemid);
  _documents->setId(_itemid);
  _itemalias->clear();
  _itemsub->clear();
  _itemtax->clear();
}

void item::sPopulateUOMs()
{
  if ((_inventoryUOM->id() != -1) && (_classcode->id()!=-1))
  {
    saveCore();
    sPopulatePriceUOMs();
    if (_priceUOM->id()==-1)
      _priceUOM->setId(_inventoryUOM->id());
  }
}

void item::sHandleItemtype()
{
  QString itemType = QString(*(_itemTypes + _itemtype->currentIndex()));
  bool pickList  = FALSE;
  bool sold      = FALSE;
  bool weight    = FALSE;
  bool config    = FALSE;
  bool shipUOM   = FALSE;
  bool capUOM    = FALSE;
  bool planType  = FALSE;
  bool purchased = FALSE;
  bool freight   = FALSE;
  
  _configured->setEnabled(FALSE);

  if (itemType == "P")
  {
    pickList = TRUE;
    sold     = TRUE;
    weight   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
    planType = TRUE;
    purchased = TRUE;
    freight  = TRUE;
  }

  if (itemType == "M")
  {
    pickList = TRUE;
    sold     = TRUE;
    weight   = TRUE;
    config   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
    planType = TRUE;
    purchased = TRUE;
    freight  = TRUE;
  }

  if (itemType == "F")
    planType = TRUE;

  if (itemType == "B")
  {
    capUOM   = TRUE;
    planType = TRUE;
    purchased = TRUE;
    freight  = TRUE;
  }

  if (itemType == "C")
  {
    pickList = TRUE;
    sold     = TRUE;
    weight   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
    planType = TRUE;
    freight  = TRUE;
  }

  if (itemType == "Y")
  {
    pickList = TRUE;
    sold     = TRUE;
    weight   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
    planType = TRUE;
    freight  = TRUE;
  }

  if (itemType == "R")
  {
    sold     = TRUE;
    weight   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
    freight  = TRUE;
    config   = TRUE;
  }

  if (itemType == "T")
  {
    pickList = TRUE;
    weight   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
    freight  = TRUE;
    purchased = TRUE;
    sold = TRUE;
  }

  if (itemType == "O")
  {
    capUOM   = TRUE;
    planType = TRUE;
    purchased = TRUE;
    freight  = TRUE;
  }

  if (itemType == "A")
  {
    sold     = TRUE;
    planType = TRUE;
    freight  = TRUE;
  }

  if (itemType == "K")
  {
    sold     = true;
    weight   = true;
    _fractional->setChecked(false);
  }
  _fractional->setEnabled(itemType!="K");
  _tab->setTabEnabled(_tab->indexOf(_tabUOM),(itemType!="K"));
  _transformationsButton->setEnabled(itemType!="K");

  _configured->setEnabled(config);
  if (!config)
    _configured->setChecked(false);

  _pickListItem->setChecked(pickList);
  _pickListItem->setEnabled(pickList);

  _sold->setChecked(sold);
  _sold->setEnabled(sold);

  _prodWeight->setEnabled(weight);
  _packWeight->setEnabled(weight);

  _freightClass->setEnabled(freight);

  _tab->setTabEnabled(_tab->indexOf(_sourcesTab), 
        (_privileges->check("ViewItemSources") || 
	 _privileges->check("MaintainItemSources")) && 
	 purchased);

  sHandleRightButtons();
}


void item::sNewAlias()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);
  params.append("item_number", _itemNumber->text());

  itemAlias newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillAliasList();
}


void item::sEditAlias()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemalias_id", _itemalias->id());
  params.append("item_number", _itemNumber->text());

  itemAlias newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillAliasList();
}

void item::sDeleteAlias()
{
  XSqlQuery itemDeleteAlias;
  itemDeleteAlias.prepare( "DELETE FROM itemalias "
             "WHERE (itemalias_id=:itemalias_id);" );
  itemDeleteAlias.bindValue(":itemalias_id", _itemalias->id());
  itemDeleteAlias.exec();

  sFillAliasList();
}

void item::sFillAliasList()
{
  XSqlQuery itemFillAliasList;
  itemFillAliasList.prepare( "SELECT itemalias_id, itemalias_number, firstLine(itemalias_comments) AS itemalias_comments "
             "FROM itemalias "
             "WHERE (itemalias_item_id=:item_id) "
             "ORDER BY itemalias_number;" );
  itemFillAliasList.bindValue(":item_id", _itemid);
  itemFillAliasList.exec();
  _itemalias->populate(itemFillAliasList);
}

void item::sNewSubstitute()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);

  itemSubstitute newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillSubstituteList();
}

void item::sEditSubstitute()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsub_id", _itemsub->id());

  itemSubstitute newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillSubstituteList();
}

void item::sDeleteSubstitute()
{
  XSqlQuery itemDeleteSubstitute;
  itemDeleteSubstitute.prepare( "DELETE FROM itemsub "
             "WHERE (itemsub_id=:itemsub_id);" );
  itemDeleteSubstitute.bindValue(":itemsub_id", _itemsub->id());
  itemDeleteSubstitute.exec();

  sFillSubstituteList();
}

void item::sFillSubstituteList()
{
  XSqlQuery itemFillSubstituteList;
  itemFillSubstituteList.prepare( "SELECT itemsub_id, itemsub_rank, item_number, item_descrip1,"
             "       itemsub_uomratio, 'uomratio' AS itemsub_uomratio_xtnumericrole "
             "FROM itemsub, item "
             "WHERE ( (itemsub_sub_item_id=item_id)"
             " AND (itemsub_parent_item_id=:item_id) ) "
             "ORDER BY itemsub_rank, item_number" );
  itemFillSubstituteList.bindValue(":item_id", _itemid);
  itemFillSubstituteList.exec();
  _itemsub->populate(itemFillSubstituteList);
}


void item::sNewTransformation()
{
  XSqlQuery itemNewTransformation;
  ParameterList params;
  params.append("itemType", ItemLineEdit::cAllItemTypes_Mask ^ ItemLineEdit::cPhantom);
  itemList* newdlg = new itemList(this);
  newdlg->set(params);

  int itemid = newdlg->exec();
  if (itemid != -1)
  {
    itemNewTransformation.prepare( "SELECT itemtrans_id "
               "FROM itemtrans "
               "WHERE ( (itemtrans_source_item_id=:source_item_id)"
               " AND (itemtrans_target_item_id=:target_item_id) );" );
    itemNewTransformation.bindValue(":source_item_id", _itemid);
    itemNewTransformation.bindValue(":target_item_id", itemid);
    itemNewTransformation.exec();
    if (itemNewTransformation.first())
    {
      QMessageBox::warning( this, tr("Cannot Duplicate Transformation"),
                            tr("The selected Item is already a Transformation target for this Item.") );
      return;
    }

    itemNewTransformation.prepare( "INSERT INTO itemtrans "
               "( itemtrans_source_item_id, itemtrans_target_item_id )"
               "VALUES "
               "( :source_item_id, :target_item_id );" );
    itemNewTransformation.bindValue(":source_item_id", _itemid);
    itemNewTransformation.bindValue(":target_item_id", itemid);
    itemNewTransformation.exec();
    sFillTransformationList();
  }
}


void item::sDeleteTransformation()
{
  XSqlQuery itemDeleteTransformation;
  itemDeleteTransformation.prepare( "DELETE FROM itemtrans "
             "WHERE (itemtrans_id=:itemtrans_id);" );
  itemDeleteTransformation.bindValue(":itemtrans_id", _itemtrans->id());
  itemDeleteTransformation.exec();
  sFillTransformationList();
}

void item::sFillTransformationList()
{
  XSqlQuery itemFillTransformationList;
  itemFillTransformationList.prepare( "SELECT itemtrans_id,"
             "       item_number, (item_descrip1 || ' ' || item_descrip2) as item_descrip "
             "FROM itemtrans, item "
             "WHERE ( (itemtrans_target_item_id=item_id)"
             " AND (itemtrans_source_item_id=:item_id) ) "
             "ORDER BY item_number;" );
  itemFillTransformationList.bindValue(":item_id", _itemid);
  itemFillTransformationList.exec();
  _itemtrans->populate(itemFillTransformationList);
}

void item::newItem()
{
  // Check for an Item window in new mode already.
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); ++i)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->objectName(), "item new")==0)
    {
      w->setFocus();
      if(omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "new");

  item *newdlg = new item();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void item::editItem( int pId )
{
  // Check for an Item window in edit mode for the specified item already.
  QString n = QString("item edit %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); ++i)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->objectName(), n)==0)
    {
      w->setFocus();
      if(omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", pId);

  item *newdlg = new item();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void item::viewItem( int pId )
{
  // Check for an Item window in edit mode for the specified item already.
  QString n = QString("item view %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); ++i)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->objectName(), n)==0)
    {
      w->setFocus();
      if(omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", pId);

  item *newdlg = new item();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void item::sEditBOM()
{
  ParameterList params;

  if(_privileges->check("MaintainBOMs"))
    params.append("mode", "edit");
  else
    params.append("mode", "view");
  params.append("item_id", _itemid);

  XDialog *newdlg = new XDialog(this);
  _bomwin = new BOM(this);
  QGridLayout *layout = new QGridLayout;
  layout->setMargin(0);

  _bomwin->set(params);
  layout->addWidget(_bomwin);
  newdlg->setLayout(layout);
  newdlg->setWindowTitle(_bomwin->windowTitle());
  connect(_bomwin->_close, SIGNAL(clicked()), newdlg, SLOT(close()));
  disconnect(_bomwin->_save, SIGNAL(clicked()), _bomwin, SLOT(sSave()));
  connect(_bomwin->_save, SIGNAL(clicked()), this, SLOT(sSaveBom()));

  newdlg->exec();
  _bom->sFillList();

}

void item::sSaveBom()
{
  if (_bomwin->sSave())
    _bomwin->parentWidget()->close();
}

void item::sWorkbench()
{
  ParameterList params;
  params.append("item_id", _itemid);

  itemAvailabilityWorkbench *newdlg = new itemAvailabilityWorkbench(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void item::sNewItemSite()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void item::sEditItemSite()
{
  XSqlQuery itemEditItemSite;
  ParameterList params;
  if (_mode == cEdit || _mode == cNew)
    params.append("mode", "edit");
  else
    params.append("mode", "view");
    
  if (!_metrics->boolean("MultiWhs"))
  {
    itemEditItemSite.prepare("SELECT itemsite_id "
              "FROM itemsite "
              "WHERE (itemsite_item_id=:item_id AND itemsite_active);");
    itemEditItemSite.bindValue(":item_id",_itemid);
    itemEditItemSite.exec();
    if (itemEditItemSite.first())
      params.append("itemsite_id", itemEditItemSite.value("itemsite_id").toInt());
    else
    {
      itemEditItemSite.prepare("SELECT itemsite_id "
                "FROM itemsite "
                "WHERE (itemsite_item_id=:item_id);");
      itemEditItemSite.bindValue(":item_id",_itemid);
      itemEditItemSite.exec();
      if (itemEditItemSite.first())
        params.append("itemsite_id", itemEditItemSite.value("itemsite_id").toInt());
      else
      {
        if((_mode == cEdit) &&
           (QMessageBox::question(this, tr("No Item Site Found"),
              tr("There is no Item Site for this item. Would you like to create one now?"),
              QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes))
          sNewItemSite();
        else if(_mode != cEdit)
          QMessageBox::information(this, tr("No Item Site Found"),
            tr("There is no Item Site for this item."));
        return;
      }
    }
  }
  else
  {
    if (!checkSitePrivs(_itemSite->id()))
      return;
    else
      params.append("itemsite_id", _itemSite->id());
  }

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void item::sViewItemSite()
{
  if (!checkSitePrivs(_itemSite->id()))
      return;

  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", _itemSite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void item::sDeleteItemSite()
{
  XSqlQuery itemDeleteItemSite;
  if (!checkSitePrivs(_itemSite->id()))
      return;
          
  itemDeleteItemSite.prepare("SELECT deleteItemSite(:itemsite_id) AS result;");
  itemDeleteItemSite.bindValue(":itemsite_id", _itemSite->id());
  itemDeleteItemSite.exec();
  if (itemDeleteItemSite.first())
  {
    switch (itemDeleteItemSite.value("result").toInt())
    {
      case -1:
        QMessageBox::warning( this, tr("Cannot Delete Item Site"),
                              tr( "The selected Item Site cannot be deleted as there is Inventory History posted against it.\n"
                                  "You may edit the Item Site and deactivate it." ) );
        return;

      case -2:
        QMessageBox::warning( this, tr("Cannot Delete Item Site"),
                              tr( "The selected Item Site cannot be deleted as there is Work Order History posted against it.\n"
                                  "You may edit the Item Site and deactivate it." ) );
        return;

      case -3:
        QMessageBox::warning( this, tr("Cannot Delete Item Site"),
                              tr( "The selected Item Site cannot be deleted as there is Sales History posted against it.\n"
                                  "You may edit the Item Site and deactivate it." ) );
        return;

      case -4:
        QMessageBox::warning( this, tr("Cannot Delete Item Site"),
                              tr( "The selected Item Site cannot be deleted as there is Purchasing History posted against it.\n"
                                  "You may edit the Item Site and deactivate it." ) );
        return;

      case -5:
        QMessageBox::warning( this, tr("Cannot Delete Item Site"),
                              tr( "The selected Item Site cannot be deleted as there is Planning History posted against it.\n"
                                  "You may edit the Item Site and deactivate it." ) );
        return;

      case -9:
        QMessageBox::warning( this, tr("Cannot Delete Item Site"),
                              tr("The selected Item Site cannot be deleted as there is a non-zero Inventory Quantity posted againt it.") );
        return;


      default:
//  ToDo

      case 0:
        sFillListItemSites();
        break;
    }
  }
//  ToDo
}

void item::sFillListItemSites()
{
  XSqlQuery itemFillListItemSites;
  QString sql( "SELECT itemsite_id, itemsite_active,"
               "       warehous_code, warehous_descrip, "
               "       CASE WHEN itemsite_controlmethod='R' THEN :regular"
               "            WHEN itemsite_controlmethod='N' THEN :none"
               "            WHEN itemsite_controlmethod='L' THEN :lotNumber"
               "            WHEN itemsite_controlmethod='S' THEN :serialNumber"
               "       END AS itemsite_controlmethod "
               "FROM itemsite, item, whsinfo "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id) "
               " AND (item_id=:item_id) ) "
               "ORDER BY item_number, warehous_code;" );

  itemFillListItemSites.prepare(sql);
  itemFillListItemSites.bindValue(":item_id", _itemid);
  itemFillListItemSites.bindValue(":regular", tr("Regular"));
  itemFillListItemSites.bindValue(":none", tr("None"));
  itemFillListItemSites.bindValue(":lotNumber", tr("Lot #"));
  itemFillListItemSites.bindValue(":serialNumber", tr("Serial #"));
  itemFillListItemSites.exec();
  _itemSite->populate(itemFillListItemSites);
}

void item::sNewItemtax()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);

  itemtax newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillListItemtax();
}

void item::sEditItemtax()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemtax_id", _itemtax->id());

  itemtax newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillListItemtax();
}
void item::sDeleteItemtax()
{
  XSqlQuery itemDeleteItemtax;
  itemDeleteItemtax.prepare("DELETE FROM itemtax"
            " WHERE (itemtax_id=:itemtax_id);");
  itemDeleteItemtax.bindValue(":itemtax_id", _itemtax->id());
  itemDeleteItemtax.exec();
  sFillListItemtax();
}

void item::sFillListItemtax()
{
  XSqlQuery itemFillListItemtax;
  itemFillListItemtax.prepare("SELECT itemtax_id, taxtype_name,"
            "       COALESCE(taxzone_code,:any) AS taxzone"
            "  FROM itemtax JOIN taxtype ON (itemtax_taxtype_id=taxtype_id)"
            "       LEFT OUTER JOIN taxzone ON (itemtax_taxzone_id=taxzone_id)"
            " WHERE (itemtax_item_id=:item_id)"
            " ORDER BY taxtype_name;");
  itemFillListItemtax.bindValue(":item_id", _itemid);
  itemFillListItemtax.bindValue(":any", tr("Any"));
  itemFillListItemtax.exec();
  _itemtax->populate(itemFillListItemtax, _itemtax->id());
  if (itemFillListItemtax.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemFillListItemtax.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void item::sNewUOM()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);
  params.append("inventoryUOM", _inventoryUOM->id());

  itemUOM newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillUOMList();
}

void item::sEditUOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemuomconv_id", _uomconv->id());

  itemUOM newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillUOMList();
}

void item::sDeleteUOM()
{
  XSqlQuery itemDeleteUOM;
  itemDeleteUOM.prepare("SELECT deleteItemUOMConv(:itemuomconv_id) AS result;");
  itemDeleteUOM.bindValue(":itemuomconv_id", _uomconv->id());
  itemDeleteUOM.exec();
  if (itemDeleteUOM.first())
  {
    int result = itemDeleteUOM.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteItemUOMConv", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (itemDeleteUOM.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemDeleteUOM.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillUOMList();
}

void item::sFillUOMList()
{
  XSqlQuery itemFillUOMList;
  itemFillUOMList.prepare("SELECT * FROM ( "
            "SELECT itemuomconv_id, -1 AS itemuom_id, 0 AS xtindentrole, "
            "       (nuom.uom_name||'/'||duom.uom_name) AS uomname,"
            "       (formatUOMRatio(itemuomconv_from_value)||'/'||formatUOMRatio(itemuomconv_to_value)) AS uomvalue,"
            "       (uomconv_id IS NOT NULL) AS global,"
            "       itemuomconv_fractional AS fractional"
            "  FROM item"
            "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
            "  JOIN uom AS nuom ON (itemuomconv_from_uom_id=nuom.uom_id)"
            "  JOIN uom AS duom ON (itemuomconv_to_uom_id=duom.uom_id)"
            "  JOIN itemuom ON (itemuom_itemuomconv_id=itemuomconv_id)"
            "  LEFT OUTER JOIN uomconv ON ((uomconv_from_uom_id=duom.uom_id AND uomconv_to_uom_id=nuom.uom_id)"
            "                           OR (uomconv_to_uom_id=duom.uom_id AND uomconv_from_uom_id=nuom.uom_id))"
            " WHERE(item_id=:item_id)"
            " UNION "
            " SELECT itemuomconv_id, itemuom_id, 1 AS xtindentrole,"
            "        uomtype_name AS uomname,"
            "       '' AS uomvalue,"
            "       NULL AS global,"
            "       NULL AS fractional"
            "  FROM item"
            "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
            "  JOIN uom AS nuom ON (itemuomconv_from_uom_id=nuom.uom_id)"
            "  JOIN uom AS duom ON (itemuomconv_to_uom_id=duom.uom_id)"
            "  JOIN itemuom ON (itemuom_itemuomconv_id=itemuomconv_id)"
            "  JOIN uomtype ON (itemuom_uomtype_id=uomtype_id)"
            " WHERE (item_id=:item_id)"
            " ) AS data "
            " ORDER BY itemuomconv_id, xtindentrole, uomname;");
  itemFillUOMList.bindValue(":item_id", _itemid);
  itemFillUOMList.exec();
  _uomconv->populate(itemFillUOMList,TRUE);
  _uomconv->expandAll();

  itemFillUOMList.prepare("SELECT itemInventoryUOMInUse(:item_id) AS result;");
  itemFillUOMList.bindValue(":item_id", _itemid);
  itemFillUOMList.exec();
  if(itemFillUOMList.first())
    _inventoryUOM->setEnabled(!itemFillUOMList.value("result").toBool());
  sPopulatePriceUOMs();
}

void item::sPopulatePriceUOMs()
{
  XSqlQuery itemPopulatePriceUOMs;
  int pid = _priceUOM->id();
  itemPopulatePriceUOMs.prepare("SELECT uom_id, uom_name, uom_name"
            "  FROM uom"
            " WHERE(uom_id=:uom_id)"
            " UNION "
            "SELECT uom_id, uom_name, uom_name"
            "  FROM uom"
            "  JOIN itemuomconv ON (itemuomconv_to_uom_id=uom_id)"
            "  JOIN item ON (itemuomconv_from_uom_id=item_inv_uom_id)"
            "  JOIN itemuom ON (itemuom_itemuomconv_id=itemuomconv_id)"
            "  JOIN uomtype ON (itemuom_uomtype_id=uomtype_id)"
            " WHERE((itemuomconv_item_id=:item_id)"
            "   AND (uomtype_name='Selling'))"
            " UNION "
            "SELECT uom_id, uom_name, uom_name"
            "  FROM uom"
            "  JOIN itemuomconv ON (itemuomconv_from_uom_id=uom_id)"
            "  JOIN item ON (itemuomconv_to_uom_id=item_inv_uom_id)"
            "  JOIN itemuom ON (itemuom_itemuomconv_id=itemuomconv_id)"
            "  JOIN uomtype ON (itemuom_uomtype_id=uomtype_id)"
            " WHERE((itemuomconv_item_id=:item_id)"
            "   AND (uomtype_name='Selling'))"
            " ORDER BY 2;");
  itemPopulatePriceUOMs.bindValue(":item_id", _itemid);
  itemPopulatePriceUOMs.bindValue(":uom_id", _inventoryUOM->id());
  itemPopulatePriceUOMs.exec();
  _priceUOM->populate(itemPopulatePriceUOMs, pid);
  if (itemPopulatePriceUOMs.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemPopulatePriceUOMs.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void item::closeEvent(QCloseEvent *pEvent)
{
  XSqlQuery itemcloseEvent;
  if(_inTransaction)
  {
    itemcloseEvent.exec("ROLLBACK;");
    _inTransaction = false;
  }

  QDirIterator dirIterator(QDir::tempPath() + "/xtTempDoc/",
                           QDir::AllDirs|QDir::Files|QDir::NoSymLinks,
                           QDirIterator::Subdirectories);

  while(dirIterator.hasNext())
  {
    dirIterator.next();
    QFile rfile(dirIterator.fileInfo().absoluteFilePath());
    rfile.remove();
  }

  XWidget::closeEvent(pEvent);
}

void item::sConfiguredToggled(bool p)
{
  if (p)
    _charass->showColumn(3);
  else
    _charass->hideColumn(3);
}

bool item::checkSitePrivs(int itemsiteid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT warehous_id "
                  "FROM itemsite, site() "
                  "WHERE ( (itemsite_id=:itemsiteid) "
                  "  AND   (warehous_id=itemsite_warehous_id) );");
    check.bindValue(":itemsiteid", itemsiteid);
    check.exec();
    if (!check.first())
    {
      QMessageBox::critical(this, tr("Access Denied"),
                            tr("You may not view or edit this Item Site as it references "
                               "a Site for which you have not been granted privileges.")) ;
      return false;
    }
  }
  return true;
}

void item::sHandleButtons()
{
  if (_notesButton->isChecked())
    _remarksStack->setCurrentIndex(0);
  else if (_extDescripButton->isChecked())
    _remarksStack->setCurrentIndex(1);
  else if (_commentsButton->isChecked())
    _remarksStack->setCurrentIndex(2);
    
  if (_elementsButton->isChecked())
    _costStack->setCurrentIndex(0);
  else if (_bomButton->isChecked())
    _costStack->setCurrentIndex(1);

  if (_aliasesButton->isChecked())
    _relationshipsStack->setCurrentIndex(0);
  else if (_transformationsButton->isChecked())
    _relationshipsStack->setCurrentIndex(1);
  else if (_substitutesButton->isChecked())
    _relationshipsStack->setCurrentIndex(2);
}

void item::sHandleRightButtons()
{
  if ((_inventoryUOM->isValid()) && (_classcode->isValid()))
  {
    QString itemtype = _itemTypes[_itemtype->currentIndex()];
    if(!_privileges->check("ViewItemAvailabilityWorkbench"))
      _workbench->hide();
    else
      if (itemtype == "K" || _itemtype->currentIndex()==-1)
        _workbench->hide();
      else
        _workbench->show();

    if((!_privileges->check("CreateCosts")) &&
       (!_privileges->check("EnterActualCosts")) &&
       (!_privileges->check("UpdateActualCosts")) &&
       (!_privileges->check("ViewCosts")))
    {
      _elementsButton->setEnabled(false);
      _bomButton->setChecked(true);
      sHandleButtons();
    }

    _bomButton->setEnabled((_privileges->check("MaintainBOMs") ||
                             _privileges->check("ViewBOMs")) &&
                             itemtype != "R" &&
                             itemtype != "S");

    // If can't do anything in costs tab, then disable
    _tab->setTabEnabled(_tab->indexOf(_costTab),
                        _elementsButton->isEnabled() ||
                        _bomButton->isEnabled());

    if((_privileges->check("MaintainItemSites")) &&
       (!_metrics->boolean("MultiWhs")))
      _site->show();
    else
      _site->hide();
  }
  else
  {
    _workbench->hide();
    _site->hide();
  }
}

void item::sFillSourceList()
{
  XSqlQuery itemFillSourceList;
  QString sql( "SELECT itemsrc_id, vend_number,"
               "       vend_name, itemsrc_vend_item_number, "
	       "       itemsrc_active, itemsrc_manuf_name, "
               "       itemsrc_manuf_item_number, "
			   "       CASE WHEN itemsrc_default = 'TRUE' THEN 'Yes' "
			   "       ELSE 'No' "
			   "       END AS default "
               "FROM item, vendinfo, itemsrc "
               "WHERE ( (itemsrc_item_id=item_id)"
               " AND (itemsrc_vend_id=vend_id)"
	       " AND (itemsrc_item_id=<? value(\"item_id\") ?>) "
               ") ORDER BY vend_number, vend_name;" );
               
  ParameterList params;
  MetaSQLQuery mql(sql);
  params.append("item_id", _itemid);
  itemFillSourceList = mql.toQuery(params);
  _itemsrc->populate(itemFillSourceList);
}

void item::sNewSource()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillSourceList();
}

void item::sEditSource()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", _itemsrc->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillSourceList();
}

void item::sViewSource()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsrc_id", _itemsrc->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void item::sCopySource()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("itemsrc_id", _itemsrc->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillSourceList();
}

void item::sDeleteSource()
{
  XSqlQuery itemDeleteSource;
  itemDeleteSource.prepare("SELECT poitem_id, itemsrc_active "
            "FROM poitem, itemsrc "
            "WHERE ((poitem_itemsrc_id=:itemsrc_id) "
            "AND (itemsrc_id=:itemsrc_id)); ");
  itemDeleteSource.bindValue(":itemsrc_id", _itemsrc->id());
  itemDeleteSource.exec();
  if (itemDeleteSource.first())
  {
    if (itemDeleteSource.value("itemsrc_active").toBool())
    {
      if (QMessageBox::question( this, tr("Delete Item Source"),
                                    tr( "This item source is used by existing purchase order records"
                                    " and may not be deleted.  Would you like to deactivate it instead?"),
                                    tr("&Ok"), tr("&Cancel"), 0, 0, 1 ) == 0  )
      {
        itemDeleteSource.prepare( "UPDATE itemsrc SET "
                   "  itemsrc_active=false "
                   "WHERE (itemsrc_id=:itemsrc_id);" );
        itemDeleteSource.bindValue(":itemsrc_id", _itemsrc->id());
        itemDeleteSource.exec();

        sFillSourceList();
      }
    }
    else
      QMessageBox::critical( this, tr("Delete Item Source"), tr("This item source is used by existing "
                          "purchase order records and may not be deleted."));
    return;
  }
  
  if (QMessageBox::information( this, tr("Delete Item Source"),
                                 tr( "Are you sure that you want to delete the Item Source?"),
                                tr("&Delete"), tr("&Cancel"), 0, 0, 1 ) == 0  )
  {
    itemDeleteSource.prepare( "DELETE FROM itemsrc "
               "WHERE (itemsrc_id=:itemsrc_id);"
               "DELETE FROM itemsrcp "
               "WHERE (itemsrcp_itemsrc_id=:itemsrc_id);" );
    itemDeleteSource.bindValue(":itemsrc_id", _itemsrc->id());
    itemDeleteSource.exec();

    sFillSourceList();
  }
}

void item::setId(int p)
{
  if (_itemid==p)
    return;

  _itemid=p;
  populate();
  emit newId(_itemid);
}

