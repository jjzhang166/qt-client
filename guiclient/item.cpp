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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "item.h"

#include <QVariant>
#include <QMessageBox>
#include <QWorkspace>
#include <QValidator>
#include <QStatusBar>
#include <QKeyEvent>
#include <QSqlError>

#include <openreports.h>
#include "itemSite.h"
#include "characteristicAssignment.h"
#include "itemImage.h"
#include "comment.h"
#include "itemAlias.h"
#include "itemList.h"
#include "itemSubstitute.h"
#include "boo.h"
#include "bom.h"
#include "itemAvailabilityWorkbench.h"
#include "itemFile.h"
#include "itemtax.h"
#include "image.h"

const char *_itemTypes[] = { "P", "M", "F", "R", "S", "T", "O", "L", "B", "C", "Y" };
//const int _itemTypeCount = 8;
const char *_planningTypes[] = { "M", "S", "N" };

/*
 *  Constructs a item as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
item::item(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemNumber, SIGNAL(lostFocus()), this, SLOT(sFormatItemNumber()));
  connect(_inventoryUOM, SIGNAL(newID(int)), this, SLOT(sPopulateUOMs()));
  connect(_sold, SIGNAL(toggled(bool)), _soldGroup, SLOT(setEnabled(bool)));
  connect(_charass, SIGNAL(itemSelected(int)), _editCharacteristic, SLOT(animateClick()));
  connect(_newCharacteristic, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_newImage, SIGNAL(clicked()), this, SLOT(sNewImage()));
  connect(_editCharacteristic, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_editImage, SIGNAL(clicked()), this, SLOT(sEditImage()));
  connect(_viewImage, SIGNAL(clicked()), this, SLOT(sViewImage()));
  connect(_printImage, SIGNAL(clicked()), this, SLOT(sPrintImage()));
  connect(_deleteCharacteristic, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_deleteImage, SIGNAL(clicked()), this, SLOT(sDeleteImage()));
  connect(_priceUOMRatio, SIGNAL(lostFocus()), this, SLOT(sCalculateExtPrice()));
  connect(_listprice, SIGNAL(lostFocus()), this, SLOT(sCalculateExtPrice()));
  connect(_itemtype, SIGNAL(activated(int)), this, SLOT(sHandleItemtype()));
  connect(_newAlias, SIGNAL(clicked()), this, SLOT(sNewAlias()));
  connect(_editAlias, SIGNAL(clicked()), this, SLOT(sEditAlias()));
  connect(_deleteAlias, SIGNAL(clicked()), this, SLOT(sDeleteAlias()));
  connect(_itemalias, SIGNAL(itemSelected(int)), _editAlias, SLOT(animateClick()));
  connect(_newSubstitute, SIGNAL(clicked()), this, SLOT(sNewSubstitute()));
  connect(_editSubstitute, SIGNAL(clicked()), this, SLOT(sEditSubstitute()));
  connect(_deleteSubstitute, SIGNAL(clicked()), this, SLOT(sDeleteSubstitute()));
  connect(_itemsub, SIGNAL(itemSelected(int)), _editSubstitute, SLOT(animateClick()));
  connect(_newTransform, SIGNAL(clicked()), this, SLOT(sNewTransformation()));
  connect(_deleteTransform, SIGNAL(clicked()), this, SLOT(sDeleteTransformation()));
  connect(_sold, SIGNAL(toggled(bool)), _soldGroup, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_boo, SIGNAL(clicked()), this, SLOT(sEditBOO()));
  connect(_bom, SIGNAL(clicked()), this, SLOT(sEditBOM()));
  connect(_site, SIGNAL(clicked()), this, SLOT(sEditItemSite()));
  connect(_workbench, SIGNAL(clicked()), this, SLOT(sWorkbench()));
  connect(_itemSite, SIGNAL(valid(bool)), _viewItemSite, SLOT(setEnabled(bool)));
  connect(_deleteItemSite, SIGNAL(clicked()), this, SLOT(sDeleteItemSite()));
  connect(_viewItemSite, SIGNAL(clicked()), this, SLOT(sViewItemSite()));
  connect(_editItemSite, SIGNAL(clicked()), this, SLOT(sEditItemSite()));
  connect(_newItemSite, SIGNAL(clicked()), this, SLOT(sNewItemSite()));
  connect(_file, SIGNAL(valid(bool)), _viewFile, SLOT(setEnabled(bool)));
  connect(_file, SIGNAL(valid(bool)), _openFile, SLOT(setEnabled(bool)));
  connect(_deleteFile, SIGNAL(clicked()), this, SLOT(sDeleteFile()));
  connect(_editFile, SIGNAL(clicked()), this, SLOT(sEditFile()));
  connect(_newFile, SIGNAL(clicked()), this, SLOT(sNewFile()));
  connect(_viewFile, SIGNAL(clicked()), this, SLOT(sViewFile()));
  connect(_openFile, SIGNAL(clicked()), this, SLOT(sOpenFile()));
  connect(_itemtaxNew, SIGNAL(clicked()), this, SLOT(sNewItemtax()));
  connect(_itemtaxEdit, SIGNAL(clicked()), this, SLOT(sEditItemtax()));
  connect(_itemtaxDelete, SIGNAL(clicked()), this, SLOT(sDeleteItemtax()));

  statusBar()->hide();
  _disallowPlanningType = false;

  _capacityUOMRatio->setValidator(omfgThis->ratioVal());
  _altCapacityUOMRatio->setValidator(omfgThis->ratioVal());
  _priceUOMRatio->setValidator(omfgThis->ratioVal());
  _listprice->setValidator(omfgThis->moneyVal());
  _prodWeight->setValidator(omfgThis->weightVal());
  _packWeight->setValidator(omfgThis->weightVal());

  _classcode->setAllowNull(TRUE);
  _classcode->setType(XComboBox::ClassCodes);

  _inventoryUOM->setType(XComboBox::UOMs);
  _capacityUOM->setType(XComboBox::UOMs);
  _altCapacityUOM->setType(XComboBox::UOMs);
  _shippingUOM->setType(XComboBox::UOMs);
  _priceUOM->setType(XComboBox::UOMs);

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft );
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft );
  _charass->addColumn(tr("Default"),        _ynColumn,   Qt::AlignCenter );

  _itemimage->addColumn(tr("Image Name"),  _itemColumn, Qt::AlignLeft );
  _itemimage->addColumn(tr("Description"), -1,          Qt::AlignLeft );
  _itemimage->addColumn(tr("Purpose"),     _itemColumn, Qt::AlignLeft );

  _itemalias->addColumn(tr("Alias Number"), _itemColumn, Qt::AlignLeft );
  _itemalias->addColumn(tr("Comments"),     -1,          Qt::AlignLeft );

  _itemsub->addColumn(tr("Rank"),        _whsColumn,  Qt::AlignCenter );
  _itemsub->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   );
  _itemsub->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _itemsub->addColumn(tr("Ratio"),       _qtyColumn,  Qt::AlignRight  );

  _itemtrans->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   );
  _itemtrans->addColumn(tr("Description"), -1,          Qt::AlignLeft   );

  _itemSite->addColumn(tr("Active"),        _dateColumn, Qt::AlignCenter );
  _itemSite->addColumn(tr("Whs."),          _whsColumn*2,  Qt::AlignCenter );
  _itemSite->addColumn(tr("Cntrl. Method"), _itemColumn, Qt::AlignCenter );
  _itemSite->setDragString("itemsiteid=");

  connect(omfgThis, SIGNAL(itemsitesUpdated()), SLOT(sFillListItemSites()));

  _file->addColumn(tr("Title"), _itemColumn, Qt::AlignLeft );
  _file->addColumn(tr("URL"), -1, Qt::AlignLeft );

  _itemtax->addColumn(tr("Tax Type"),      _itemColumn, Qt::AlignLeft );
  _itemtax->addColumn(tr("Tax Authority"),          -1, Qt::AlignLeft );

  if(!_privleges->check("MaintainBOOs") || !_metrics->boolean("Routings"))
    _boo->hide();
    
  if(!_privleges->check("MaintainBOMs"))
    _bom->hide();
    
  if((!_privleges->check("MaintainItemSites")) || (_metrics->boolean("MultiWhs")))
    _site->hide();

  if(!_privleges->check("ViewItemAvailabilityWorkbench"))
    _workbench->hide();
    
  if (!_metrics->boolean("Transforms"))
    _tab->removePage(_tab->page(8));
    
  if (!_metrics->boolean("MultiWhs"))
    _tab->removePage(_tab->page(3));
 
  if (!_metrics->boolean("BBOM"))
  {
    _itemtype->removeItem(10);
    _itemtype->removeItem(9);
    _itemtype->removeItem(8);
  }
    
  if (_metrics->value("Application") != "OpenMFG")
  {
    _planningType->setCurrentItem(2);
    _planningType->hide();
    _planningTypeLit->hide();
  }

#ifdef Q_WS_MAC
  _tab->setUsesScrollButtons(true);
  _tab->setElideMode(Qt::ElideNone);
#endif
}

/*
 *  Destroys the object and frees any allocated resources
 */
item::~item()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void item::languageChange()
{
    retranslateUi(this);
}

enum SetResponse item::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      setName("item new");
      _mode = cNew;

      _line1->hide();
      _bom->hide();
      _boo->hide();
      _workbench->hide();
      _site->hide();

      q.exec("SELECT NEXTVAL('item_item_id_seq') AS item_id");
      if (q.first())
        _itemid = q.value("item_id").toInt();
//  ToDo

      _comments->setId(_itemid);
      _exclusive->setChecked(_metrics->boolean("DefaultSoldItemsExclusive"));

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      connect(_itemalias, SIGNAL(valid(bool)), _editAlias, SLOT(setEnabled(bool)));
      connect(_itemalias, SIGNAL(valid(bool)), _deleteAlias, SLOT(setEnabled(bool)));
      connect(_itemsub, SIGNAL(valid(bool)), _editSubstitute, SLOT(setEnabled(bool)));
      connect(_itemsub, SIGNAL(valid(bool)), _deleteSubstitute, SLOT(setEnabled(bool)));
      connect(_itemtrans, SIGNAL(valid(bool)), _deleteTransform, SLOT(setEnabled(bool)));
      connect(_itemtax, SIGNAL(valid(bool)), _itemtaxEdit, SLOT(setEnabled(bool)));
      connect(_itemtax, SIGNAL(valid(bool)), _itemtaxDelete, SLOT(setEnabled(bool)));

      _itemNumber->setFocus();
    }
    else if (param.toString() == "edit")
    {
      setName(QString("item edit %1").arg(_itemid));
      _mode = cEdit;

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      connect(_itemalias, SIGNAL(valid(bool)), _editAlias, SLOT(setEnabled(bool)));
      connect(_itemalias, SIGNAL(valid(bool)), _deleteAlias, SLOT(setEnabled(bool)));
      connect(_itemsub, SIGNAL(valid(bool)), _editSubstitute, SLOT(setEnabled(bool)));
      connect(_itemsub, SIGNAL(valid(bool)), _deleteSubstitute, SLOT(setEnabled(bool)));
      connect(_itemtrans, SIGNAL(valid(bool)), _deleteTransform, SLOT(setEnabled(bool)));
      connect(_file, SIGNAL(valid(bool)), _editFile, SLOT(setEnabled(bool)));
      connect(_file, SIGNAL(itemSelected(int)), _editFile, SLOT(animateClick()));
      connect(_file, SIGNAL(valid(bool)), _deleteFile, SLOT(setEnabled(bool)));
      connect(_itemtax, SIGNAL(valid(bool)), _itemtaxEdit, SLOT(setEnabled(bool)));
      connect(_itemtax, SIGNAL(valid(bool)), _itemtaxDelete, SLOT(setEnabled(bool)));

      if (_privleges->check("MaintainItemSites"))
      {
        connect(_itemSite, SIGNAL(valid(bool)), _editItemSite, SLOT(setEnabled(bool)));
        connect(_itemSite, SIGNAL(itemSelected(int)), _editItemSite, SLOT(animateClick()));
      }
      else
      {
        connect(_itemSite, SIGNAL(itemSelected(int)), _viewItemSite, SLOT(animateClick()));
      }

      if (_privleges->check("DeleteItemSites"))
        connect(_itemSite, SIGNAL(valid(bool)), _deleteItemSite, SLOT(setEnabled(bool)));

      _itemNumber->setEnabled(FALSE);
      _newItemSite->setEnabled(TRUE);
      _newFile->setEnabled(TRUE);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      setName(QString("item view %1").arg(_itemid));
      _mode = cView;

      _itemNumber->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _description1->setEnabled(FALSE);
      _description2->setEnabled(FALSE);
      _maximumDesiredCost->setEnabled(FALSE);
      _itemtype->setEnabled(FALSE);
      _sold->setEnabled(FALSE);
      _soldGroup->setEnabled(false);
      _pickListItem->setEnabled(FALSE);
      _fractional->setEnabled(FALSE);
      _classcode->setEnabled(FALSE);
      _inventoryUOM->setEnabled(FALSE);
      _capacityUOM->setEnabled(FALSE);
      _capacityUOMRatio->setEnabled(FALSE);
      _altCapacityUOM->setEnabled(FALSE);
      _altCapacityUOMRatio->setEnabled(FALSE);
      _shippingUOM->setEnabled(FALSE);
      _shippingUOMRatio->setEnabled(FALSE);
      _prodWeight->setEnabled(FALSE);
      _packWeight->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _extDescription->setReadOnly(TRUE);
      _newCharacteristic->setEnabled(FALSE);
      _newImage->setEnabled(FALSE);
      _newAlias->setEnabled(FALSE);
      _newSubstitute->setEnabled(FALSE);
      _newTransform->setEnabled(FALSE);
      _itemtaxNew->setEnabled(FALSE);
      _planningType->setEnabled(false);
      _close->setText(tr("&Close"));

      connect(_itemSite, SIGNAL(itemSelected(int)), _viewItemSite, SLOT(animateClick()));
      connect(_file, SIGNAL(itemSelected(int)), _viewFile, SLOT(animateClick()));

      _save->hide();

      _close->setFocus();
    }
  }

  if(cView == _mode)
    connect(_itemimage, SIGNAL(itemSelected(int)), _viewImage, SLOT(animateClick()));
  else
  {
    connect(_itemimage, SIGNAL(itemSelected(int)), _editImage, SLOT(animateClick()));
    connect(_itemimage, SIGNAL(valid(bool)), _editImage, SLOT(setEnabled(bool)));
    connect(_itemimage, SIGNAL(valid(bool)), _deleteImage, SLOT(setEnabled(bool)));
  }

  return NoError;
}

void item::sSave()
{
  QString itemNumber = _itemNumber->text().stripWhiteSpace().upper();

  if(_disallowPlanningType && QString(_itemTypes[_itemtype->currentItem()]) == "L")
  {
    QMessageBox::warning( this, tr("Planning Type Disallowed"),
      tr("This item is part of one or more Bills of Materials and cannot be a Planning Item.") );
    return;
  }

  if (_mode == cEdit)
  {
    q.prepare( "SELECT item_id "
               "FROM item "
               "WHERE ( (item_number=:item_number)"
               " AND (item_id <> :item_id) );" );
    q.bindValue(":item_number", itemNumber);
    q.bindValue(":item_id", _itemid);
    q.exec();
    if (q.first())
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

  if (_itemtype->currentItem() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must select an Item Type for this Item Type before continuing.")  );
    _itemtype->setFocus();
    return;
  }

  if (_classcode->currentItem() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must select a Class Code for this Item before continuing.") );
    _classcode->setFocus();
    return;
  }

  if (_inventoryUOM->currentText().stripWhiteSpace().length() == 0)
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

  if (_classcode->id() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Item"),
                              tr("You must select a Class Code for this Item before continuing.")  );
    _classcode->setFocus();
    return;
  }

  if (cEdit == _mode && _itemtype->currentText() != _originalItemType)
  {
    q.prepare( "SELECT itemcost_id "
               "  FROM itemcost "
               " WHERE (itemcost_item_id=:item_id);" );
    q.bindValue(":item_id", _itemid);
    q.exec();
    if (q.first())
    {
      if(QMessageBox::question( this, tr("Item Costs Exist"),
                                tr("You have changed the Item Type of this Item.\n"
                                   "This Item has Item Costs associated with it that will be\n"
                                   "deleted before this change may occurr.\n"
                                   "Do you wish to continue saving and delete the Item Costs?"),
                                QMessageBox::Ok,
                                QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default ) == QMessageBox::Cancel)
      {
        return;
      }

      q.prepare("SELECT updateCost(itemcost_id, 0) FROM itemcost WHERE (itemcost_item_id=:item_id);");
      q.bindValue(":item_id", _itemid);
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }

  int sourceItemid = _itemid;

  if (_mode == cCopy)
  {
    q.exec("SELECT NEXTVAL('item_item_id_seq') AS _item_id");
    if (q.first())
       _itemid = q.value("_item_id").toInt();
// ToDo
  }

  if ( (_mode == cNew) || (_mode == cCopy) )
    q.prepare( "INSERT INTO item "
               "( item_id, item_number, item_active,"
               "  item_descrip1, item_descrip2,"
               "  item_type, item_invuom, item_classcode_id,"
               "  item_planning_type,"
               "  item_picklist, item_sold, item_fractional,"
               "  item_capuom, item_capinvrat,"
               "  item_altcapuom, item_altcapinvrat,"
               "  item_shipuom, item_shipinvrat,"
               "  item_maxcost, item_prodweight, item_packweight,"
               "  item_prodcat_id, item_priceuom, item_invpricerat,"
               "  item_exclusive,"
               "  item_listprice, item_upccode, item_config,"
               "  item_comments, item_extdescrip ) "
               "VALUES "
               "( :item_id, :item_number, :item_active,"
               "  :item_descrip1, :item_descrip2,"
               "  :item_type, :item_invuom, :item_classcode_id,"
               "  :item_planning_type,"
               "  :item_picklist, :item_sold, :item_fractional,"
               "  :item_capuom, :item_capinvrat,"
               "  :item_altcapuom, :item_altcapinvrat,"
               "  :item_shipuom, :item_shipinvrat,"
               "  :item_maxcost, :item_prodweight, :item_packweight,"
               "  :item_prodcat_id, :item_priceuom, :item_invpricerat,"
               "  :item_exclusive,"
               "  :item_listprice, :item_upccode, :item_config,"
               "  :item_comments, :item_extdescrip );" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE item "
               "SET item_number=:item_number, item_descrip1=:item_descrip1, item_descrip2=:item_descrip2,"
               "    item_type=:item_type, item_invuom=:item_invuom, item_classcode_id=:item_classcode_id,"
               "    item_planning_type=:item_planning_type,"
               "    item_picklist=:item_picklist, item_sold=:item_sold, item_fractional=:item_fractional,"
               "    item_active=:item_active,"
               "    item_capuom=:item_capuom, item_capinvrat=:item_capinvrat,"
               "    item_altcapuom=:item_altcapuom, item_altcapinvrat=:item_altcapinvrat,"
               "    item_shipuom=:item_shipuom, item_shipinvrat=:item_shipinvrat,"
               "    item_maxcost=:item_maxcost, item_prodweight=:item_prodweight, item_packweight=:item_packweight,"
               "    item_prodcat_id=:item_prodcat_id,"
               "    item_priceuom=:item_priceuom, item_invpricerat=:item_invpricerat,"
               "    item_exclusive=:item_exclusive,"
               "    item_listprice=:item_listprice, item_upccode=:item_upccode, item_config=:item_config,"
               "    item_comments=:item_comments, item_extdescrip=:item_extdescrip "
               "WHERE (item_id=:item_id);" );

  q.bindValue(":item_id", _itemid);
  q.bindValue(":item_number", itemNumber);
  q.bindValue(":item_descrip1", _description1->text());
  q.bindValue(":item_descrip2", _description2->text());
  q.bindValue(":item_type", _itemTypes[_itemtype->currentItem()]);
  q.bindValue(":item_planning_type", _planningTypes[_planningType->currentItem()]);
  q.bindValue(":item_classcode_id", _classcode->id());
  q.bindValue(":item_sold", QVariant(_sold->isChecked(), 0));
  q.bindValue(":item_prodcat_id", _prodcat->id());
  q.bindValue(":item_exclusive", QVariant(_exclusive->isChecked(), 0));
  q.bindValue(":item_priceuom", _priceUOM->currentText());
  q.bindValue(":item_invpricerat", (_priceUOMRatio->toDouble()==0.0?1.0:_priceUOMRatio->toDouble()));
  q.bindValue(":item_listprice", _listprice->toDouble());
  q.bindValue(":item_upccode", _upcCode->text());
  q.bindValue(":item_active", QVariant(_active->isChecked(), 0));
  q.bindValue(":item_picklist", QVariant(_pickListItem->isChecked(), 0));
  q.bindValue(":item_fractional", QVariant(_fractional->isChecked(), 0));
  q.bindValue(":item_config", QVariant(FALSE, 0));
  q.bindValue(":item_invuom", _inventoryUOM->currentText());
  q.bindValue(":item_capuom", _capacityUOM->currentText());
  q.bindValue(":item_capinvrat", _capacityUOMRatio->toDouble());
  q.bindValue(":item_altcapuom", _altCapacityUOM->currentText());
  q.bindValue(":item_altcapinvrat", _altCapacityUOMRatio->toDouble());
  q.bindValue(":item_shipuom", _shippingUOM->currentText());
  q.bindValue(":item_shipinvrat", _shippingUOMRatio->toDouble());
  q.bindValue(":item_maxcost", _maximumDesiredCost->localValue());
  q.bindValue(":item_prodweight", _prodWeight->toDouble());
  q.bindValue(":item_packweight", _packWeight->toDouble());
  q.bindValue(":item_comments", _notes->text());
  q.bindValue(":item_extdescrip", _extDescription->text());
  q.exec();

  if (_mode == cCopy)
  {
//  Copy all of the costs for this item
    q.prepare( "INSERT INTO itemcost "
               "(itemcost_item_id, itemcost_costelem_id, itemcost_lowlevel,"
               " itemcost_stdcost, itemcost_posted, itemcost_actcost, "
	       " itemcost_updated, itemcost_curr_id ) "
               "SELECT :item_id, itemcost_costelem_id, itemcost_lowlevel,"
               "       itemcost_stdcost, itemcost_posted, itemcost_actcost, "
	       "       itemcost_updated, itemcost_curr_id "
               "FROM itemcost "
               "WHERE (itemcost_item_id=:sourceItem_id);" );
    q.bindValue(":item_id", _itemid);
    q.bindValue(":sourceItem_id", sourceItemid);
    q.exec();
  }

  omfgThis->sItemsUpdated(_itemid, TRUE);

  if ( ( (_mode == cNew) || (_mode == cCopy) ) && ( (_itemTypes[_itemtype->currentItem()] != "B") ||
                                                    (_itemTypes[_itemtype->currentItem()] != "F") ||
                                                    (_itemTypes[_itemtype->currentItem()] != "R") ||
                                                    (_itemTypes[_itemtype->currentItem()] != "S") ) )
  {
    if (QMessageBox::information( this, tr("Create New Item Sites"),
                                  tr("Would you like to create new Item Sites for the newly created Item?"),
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

  close();
}

void item::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void item::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void item::sDelete()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();

  sFillList();
}

void item::sFillList()
{
  q.prepare( "SELECT charass_id, char_name, charass_value, formatBoolYN(charass_default) "
             "FROM charass, char "
             "WHERE ( (charass_target_type='I')"
             " AND (charass_char_id=char_id)"
             " AND (charass_target_id=:item_id) ) "
             "ORDER BY char_name;" );
  q.bindValue(":item_id", _itemid);
  q.exec();
  _charass->populate(q);
}

void item::sNewImage()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);

  itemImage newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillImageList();
}

void item::sEditImage()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemimage_id", _itemimage->id());

  itemImage newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillImageList();
}

void item::sViewImage()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("image_id", _itemimage->altId());

  image newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void item::sPrintImage()
{
  ParameterList params;
  params.append("itemimage_id", _itemimage->id());

  orReport report("ItemImage", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


void item::sDeleteImage()
{
  q.prepare( "DELETE FROM itemimage "
             "WHERE (itemimage_id=:itemimage_id);" );
  q.bindValue(":itemimage_id", _itemimage->id());
  q.exec();

  sFillImageList();
}

void item::sFillImageList()
{
  q.prepare( "SELECT itemimage_id, itemimage_image_id, image_name, firstLine(image_descrip),"
             "       CASE WHEN (itemimage_purpose='I') THEN :inventory"
             "            WHEN (itemimage_purpose='P') THEN :product"
             "            WHEN (itemimage_purpose='E') THEN :engineering"
             "            WHEN (itemimage_purpose='M') THEN :misc"
             "            ELSE :other"
             "       END "
             "FROM itemimage, image "
             "WHERE ( (itemimage_image_id=image_id)"
             " AND (itemimage_item_id=:item_id) ) "
             "ORDER BY image_name;" );
  q.bindValue(":inventory", tr("Inventory Description"));
  q.bindValue(":product", tr("Product Description"));
  q.bindValue(":engineering", tr("Engineering Reference"));
  q.bindValue(":misc", tr("Miscellaneous"));
  q.bindValue(":other", tr("Other"));
  q.bindValue(":item_id", _itemid);
  q.exec();
  _itemimage->populate(q, true);
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
  if ((_mode == cNew) && (_itemNumber->text().length()))
  {
    _itemNumber->setText(_itemNumber->text().stripWhiteSpace().upper());

  //  Check to see if this item exists
    q.prepare( "SELECT item_id "
               "FROM item "
               "WHERE (item_number=:item_number);" );
    q.bindValue(":item_number", _itemNumber->text());
    q.exec();
    if (q.first())
    {
      _itemNumber->setEnabled(FALSE);
      _itemid = q.value("item_id").toInt();
      _mode = cEdit;
      populate();
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
  item.prepare( "SELECT *, formatCost(item_maxcost) AS f_maxcost,"
                "       ( (item_type IN ('P', 'M', 'C', 'Y', 'R', 'A')) AND (item_sold) ) AS sold "
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
        _itemtype->setCurrentItem(counter);
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
    _inventoryUOM->setText(item.value("item_invuom"));
    _pickListItem->setChecked(item.value("item_picklist").toBool());
    _fractional->setChecked(item.value("item_fractional").toBool());
    _capacityUOM->setText(item.value("item_capuom").toString());
    _capacityUOMRatio->setText(formatUOMRatio(item.value("item_capinvrat").toDouble()));
    _altCapacityUOM->setText(item.value("item_altcapuom"));
    _altCapacityUOMRatio->setText(formatUOMRatio(item.value("item_altcapinvrat").toDouble()));
    _shippingUOM->setText(item.value("item_shipuom"));
    _shippingUOMRatio->setText(formatUOMRatio(item.value("item_shipinvrat").toDouble()));
    _prodWeight->setText(formatWeight(item.value("item_prodweight").toDouble()));
    _packWeight->setText(formatWeight(item.value("item_packweight").toDouble()));
    _maximumDesiredCost->setLocalValue(item.value("f_maxcost").toDouble());
    _notes->setText(item.value("item_comments").toString());
    _extDescription->setText(item.value("item_extdescrip").toString());
    _sold->setChecked(item.value("item_sold").toBool());
    _prodcat->setId(item.value("item_prodcat_id").toInt());
    _upcCode->setText(item.value("item_upccode"));
    _exclusive->setChecked(item.value("item_exclusive").toBool());
    _listprice->setText(formatSalesPrice(item.value("item_listprice").toDouble()));
    _priceUOM->setText(item.value("item_priceuom"));
    _priceUOMRatio->setText(formatUOMRatio(item.value("item_invpricerat").toDouble()));

    for (int pcounter = 0; pcounter < _planningType->count(); pcounter++)
      if (QString(item.value("item_planning_type").toString()[0]) == _planningTypes[pcounter])
        _planningType->setCurrentItem(pcounter);

    sFillList();
    sFillImageList();
    sFillAliasList();
    sFillSubstituteList();
    sFillTransformationList();
    _comments->setId(_itemid);

    sCalculateExtPrice();
    sFillListItemSites();
    sFillListFiles();
    sFillListItemtax();
  }
//  ToDo
}

void item::clear()
{
  _disallowPlanningType = false;
  q.exec("SELECT NEXTVAL('item_item_id_seq') AS item_id");
  if (q.first())
    _itemid = q.value("item_id").toInt();
//  ToDo

  _itemNumber->clear();
  _description1->clear();
  _description2->clear();
  _inventoryUOM->clear();
  _capacityUOM->setCurrentItem(0);
  _capacityUOMRatio->clear();
  _altCapacityUOM->setCurrentItem(0);
  _altCapacityUOMRatio->clear();
  _shippingUOM->setCurrentItem(0);
  _shippingUOMRatio->clear();
  _priceUOM->clear();
  _priceUOMRatio->clear();
  _listprice->clear();

  _active->setChecked(TRUE);
  _pickListItem->setChecked(TRUE);
  _sold->setChecked(FALSE);
  _exclusive->setChecked(_metrics->boolean("DefaultSoldItemsExclusive"));
  _fractional->setChecked(FALSE);

  _itemtype->setCurrentItem(0);
  _classcode->setNull();
  _prodcat->setNull();

  _notes->clear();
  _extDescription->clear();
  _charass->clear();
  _itemimage->clear();
  _comments->setId(_itemid);
  _itemalias->clear();
  _itemsub->clear();
  _itemtax->clear();
}

void item::sPopulateUOMs()
{
  if (_inventoryUOM->currentText().length())
  {
    _inventoryUOM->setText(_inventoryUOM->currentText().stripWhiteSpace());

    if (!_capacityUOM->currentText().length())
    {
      _capacityUOM->setText(_inventoryUOM->currentText());
      _capacityUOMRatio->setText("1.0");
    }

    if (!_altCapacityUOM->currentText().length())
    {
      _altCapacityUOM->setText(_inventoryUOM->currentText());
      _altCapacityUOMRatio->setText("1.0");
    }

    if (!_shippingUOM->currentText().length())
    {
      _shippingUOM->setText(_inventoryUOM->currentText());
      _shippingUOMRatio->setText("1.0");
    }

    if (!_priceUOM->currentText().length())
    {
      _priceUOM->setText(_inventoryUOM->currentText());
      _priceUOMRatio->setText("1.0");
    }
  }
}

void item::sHandleItemtype()
{
  QString itemType = QString(*(_itemTypes + _itemtype->currentItem()));
  bool pickList = FALSE;
  bool sold     = FALSE;
  bool weight   = FALSE;
  bool config   = FALSE;
  bool shipUOM  = FALSE;
  bool capUOM   = FALSE;

  if (itemType == "P")
  {
    pickList = TRUE;
    sold     = TRUE;
    weight   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
  }

  if (itemType == "M")
  {
    pickList = TRUE;
    sold     = TRUE;
    weight   = TRUE;
    config   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
  }

  if (itemType == "B")
    capUOM   = TRUE;

  if (itemType == "C")
  {
    pickList = TRUE;
    sold     = TRUE;
    weight   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
  }

  if (itemType == "Y")
  {
    pickList = TRUE;
    sold     = TRUE;
    weight   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
  }

  if (itemType == "R")
  {
    sold     = TRUE;
    weight   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
  }

  if (itemType == "T")
  {
    pickList = TRUE;
    weight   = TRUE;
    capUOM   = TRUE;
    shipUOM  = TRUE;
  }

  if (itemType == "O")
    capUOM   = TRUE;

  if (itemType == "A")
    sold     = TRUE;

  if (itemType == "L")
  {
    _planningType->setEnabled(false);
    _planningType->setCurrentItem(1);
  }
  else
    _planningType->setEnabled(true);

  _pickListItem->setChecked(pickList);
  _pickListItem->setEnabled(pickList);

  _sold->setChecked(sold);
  _sold->setEnabled(sold);

  _prodWeight->setEnabled(weight);
  _packWeight->setEnabled(weight);

  if (!capUOM)
  {
    _capacityUOM->setCurrentText(tr("N/A"));
    _capacityUOMRatio->setText("1.0");
    _altCapacityUOM->setCurrentText(tr("N/A"));
    _altCapacityUOMRatio->setText("1.0");
  }
  _capacityUOM->setEnabled(capUOM);
  _capacityUOMRatio->setEnabled(capUOM);
  _altCapacityUOM->setEnabled(capUOM);
  _altCapacityUOMRatio->setEnabled(capUOM);

  if (!shipUOM)
  {
    _shippingUOM->setCurrentText(tr("N/A"));
    _shippingUOMRatio->setText("1.0");
  }
  _shippingUOM->setEnabled(capUOM);
  _shippingUOMRatio->setEnabled(capUOM);
}


void item::sCalculateExtPrice()
{
  if (_priceUOMRatio->toDouble() == 0.0)
    _priceUOMRatio->setText("1.0");

  if (_listprice->toDouble() == 0.0)
    _listprice->setText("0.0");

  _extPrice->setText(formatSalesPrice(_listprice->toDouble() / _priceUOMRatio->toDouble()));
}

void item::sNewAlias()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);
  params.append("item_number", _itemNumber->text());

  itemAlias newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
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

  if (newdlg.exec() != QDialog::Rejected)
    sFillAliasList();
}

void item::sDeleteAlias()
{
  q.prepare( "DELETE FROM itemalias "
             "WHERE (itemalias_id=:itemalias_id);" );
  q.bindValue(":itemalias_id", _itemalias->id());
  q.exec();

  sFillAliasList();
}

void item::sFillAliasList()
{
  q.prepare( "SELECT itemalias_id, itemalias_number, itemalias_comments "
             "FROM itemalias "
             "WHERE (itemalias_item_id=:item_id) "
             "ORDER BY itemalias_number;" );
  q.bindValue(":item_id", _itemid);
  q.exec();
  _itemalias->populate(q);
}

void item::sNewSubstitute()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);

  itemSubstitute newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillSubstituteList();
}

void item::sEditSubstitute()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsub_id", _itemsub->id());

  itemSubstitute newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillSubstituteList();
}

void item::sDeleteSubstitute()
{
  q.prepare( "DELETE FROM itemsub "
             "WHERE (itemsub_id=:itemsub_id);" );
  q.bindValue(":itemsub_id", _itemsub->id());
  q.exec();

  sFillSubstituteList();
}

void item::sFillSubstituteList()
{
  q.prepare( "SELECT itemsub_id, itemsub_rank, item_number, item_descrip1,"
             "       formatRatio(itemsub_uomratio) "
             "FROM itemsub, item "
             "WHERE ( (itemsub_sub_item_id=item_id)"
             " AND (itemsub_parent_item_id=:item_id) ) "
             "ORDER BY itemsub_rank, item_number" );
  q.bindValue(":item_id", _itemid);
  q.exec();
  _itemsub->populate(q);
}


void item::sNewTransformation()
{
  ParameterList params;
  itemList newdlg(this, "", TRUE);
  newdlg.set(params);

  int itemid = newdlg.exec();
  if (itemid != -1)
  {
    q.prepare( "SELECT itemtrans_id "
               "FROM itemtrans "
               "WHERE ( (itemtrans_source_item_id=:source_item_id)"
               " AND (itemtrans_target_item_id=:target_item_id) );" );
    q.bindValue(":source_item_id", _itemid);
    q.bindValue(":target_item_id", itemid);
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Dupliate Transformation"),
                            tr("The selected Item is already a Transformation target for this Item.") );
      return;
    }

    q.prepare( "INSERT INTO itemtrans "
               "( itemtrans_source_item_id, itemtrans_target_item_id )"
               "VALUES "
               "( :source_item_id, :target_item_id );" );
    q.bindValue(":source_item_id", _itemid);
    q.bindValue(":target_item_id", itemid);
    q.exec();
    sFillTransformationList();
  }
}


void item::sDeleteTransformation()
{
  q.prepare( "DELETE FROM itemtrans "
             "WHERE (itemtrans_id=:itemtrans_id);" );
  q.bindValue(":itemtrans_id", _itemtrans->id());
  q.exec();
  sFillTransformationList();
}

void item::sFillTransformationList()
{
  q.prepare( "SELECT itemtrans_id,"
             "       item_number, (item_descrip1 || ' ' || item_descrip2) "
             "FROM itemtrans, item "
             "WHERE ( (itemtrans_target_item_id=item_id)"
             " AND (itemtrans_source_item_id=:item_id) ) "
             "ORDER BY item_number;" );
  q.bindValue(":item_id", _itemid);
  q.exec();
  _itemtrans->populate(q);
}

void item::keyPressEvent( QKeyEvent * e )
{
#ifdef Q_WS_MAC
  if(e->key() == Qt::Key_S && e->state() == Qt::ControlModifier)
  {
    _save->animateClick();
    e->accept();
  }
  if(e->isAccepted())
    return;
#endif
  e->ignore();
}

void item::newItem()
{
  // Check for an Item window in new mode already.
  QWidgetList list = omfgThis->workspace()->windowList(QWorkspace::CreationOrder);
  for(int i = 0; i < list.size(); ++i)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->name(), "item new")==0)
    {
      w->setFocus();
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
  QWidgetList list = omfgThis->workspace()->windowList(QWorkspace::CreationOrder);
  for(int i = 0; i < list.size(); ++i)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->name(), n)==0)
    {
      w->setFocus();
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
  QWidgetList list = omfgThis->workspace()->windowList(QWorkspace::CreationOrder);
  for(int i = 0; i < list.size(); ++i)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->name(), n)==0)
    {
      w->setFocus();
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

void item::sEditBOO()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _itemid);

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void item::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _itemid);

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void item::sWorkbench()
{
  ParameterList params;
  params.append("item_id", _itemid);

  itemAvailabilityWorkbench *newdlg = new itemAvailabilityWorkbench();
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
  ParameterList params;
  if (_mode == cEdit)
    params.append("mode", "edit");
  else
    params.append("mode", "view");
    
  if (!_metrics->boolean("MultiWhs"))
  {
    q.prepare("SELECT itemsite_id "
              "FROM itemsite "
              "WHERE (itemsite_item_id=:item_id);");
    q.bindValue(":item_id",_itemid);
    q.exec();
    if (q.first())
      params.append("itemsite_id", q.value("itemsite_id").toInt());
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
  else
    params.append("itemsite_id", _itemSite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void item::sViewItemSite()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", _itemSite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void item::sDeleteItemSite()
{
  q.prepare("SELECT deleteItemSite(:itemsite_id) AS result;");
  q.bindValue(":itemsite_id", _itemSite->id());
  q.exec();
  if (q.first())
  {
    switch (q.value("result").toInt())
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
  QString sql( "SELECT itemsite_id, formatBoolYN(itemsite_active),"
               "       warehous_code,"
               "       CASE WHEN itemsite_controlmethod='R' THEN :regular"
               "            WHEN itemsite_controlmethod='N' THEN :none"
               "            WHEN itemsite_controlmethod='L' THEN :lotNumber"
               "            WHEN itemsite_controlmethod='S' THEN :serialNumber"
               "       END "
               "FROM itemsite, item, warehous "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id) "
               " AND (item_id=:item_id) ) "
               "ORDER BY item_number, warehous_code;" );

  q.prepare(sql);
  q.bindValue(":item_id", _itemid);
  q.bindValue(":regular", tr("Regular"));
  q.bindValue(":none", tr("None"));
  q.bindValue(":lotNumber", tr("Lot #"));
  q.bindValue(":serialNumber", tr("Serial #"));
  q.exec();
  _itemSite->populate(q);
}

void item::sNewFile()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _itemid);

  itemFile newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillListFiles();
}

void item::sEditFile()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemfile_id", _file->id());

  itemFile newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillListFiles();
}

void item::sViewFile()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemfile_id", _file->id());

  itemFile newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void item::sDeleteFile()
{
  q.prepare("DELETE FROM itemfile"
            " WHERE (itemfile_id=:itemfile_id);" );
  q.bindValue(":itemfile_id", _file->id());
  q.exec();
  sFillListFiles();
}

void item::sFillListFiles()
{
  QString sql( "SELECT itemfile_id,"
               "       itemfile_title,"
               "       itemfile_url "
               "  FROM itemfile "
               " WHERE (itemfile_item_id=:item_id);" );
  q.prepare(sql);
  q.bindValue(":item_id", _itemid);
  q.exec();
  _file->populate(q);
}

void item::sOpenFile()
{
  q.prepare("SELECT itemfile_url"
            "  FROM itemfile"
            " WHERE (itemfile_id=:itemfile_id); ");
  q.bindValue(":itemfile_id", _file->id());
  q.exec();
  if(q.first())
    omfgThis->launchBrowser(this, q.value("itemfile_url").toString());
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
  q.prepare("DELETE FROM itemtax"
            " WHERE (itemtax_id=:itemtax_id);");
  q.bindValue(":itemtax_id", _itemtax->id());
  q.exec();
  sFillListItemtax();
}

void item::sFillListItemtax()
{
  q.prepare("SELECT itemtax_id, taxtype_name, COALESCE(taxauth_code,:any)"
            "  FROM itemtax JOIN taxtype ON (itemtax_taxtype_id=taxtype_id)"
            "       LEFT OUTER JOIN taxauth ON (itemtax_taxauth_id=taxauth_id)"
            " WHERE (itemtax_item_id=:item_id)"
            " ORDER BY taxtype_name;");
  q.bindValue(":item_id", _itemid);
  q.bindValue(":any", tr("Any"));
  q.exec();
  _itemtax->populate(q, _itemtax->id());
}

