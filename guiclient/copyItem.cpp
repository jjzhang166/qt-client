/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copyItem.h"

#include <QCloseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "bomItem.h"
#include "itemSite.h"
#include "storedProcErrorLookup.h"
#include "guiErrorCheck.h"

copyItem::copyItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_next, SIGNAL(clicked()), this, SLOT(sNext()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sCancel()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(sCancel()));
  connect(_source, SIGNAL(newId(int)), this, SLOT(sUpdateItem()));
  connect(_rollupPrices, SIGNAL(toggled(bool)), this, SLOT(sFillBomitem()));
  connect(_searchForBOM, SIGNAL(textChanged(const QString&)), this, SLOT(sFillItem()));
  connect(_copyBOM, SIGNAL(toggled(bool)), this, SLOT(sCopyBom()));
  connect(_addBOM, SIGNAL(clicked()), this, SLOT(sAddBomitem()));
  connect(_revokeBOM, SIGNAL(clicked()), this, SLOT(sRevokeBomitem()));
  connect(_availablebomitems, SIGNAL(itemSelected(int)), this, SLOT(sAddBomitem()));
  connect(_addedbomitems, SIGNAL(itemSelected(int)), this, SLOT(sEditBomitem()));
  connect(_copyItemsite, SIGNAL(toggled(bool)), this, SLOT(sCopyItemsite()));
  connect(_addItemsite, SIGNAL(clicked()), this, SLOT(sAddItemsite()));
  connect(_revokeItemsite, SIGNAL(clicked()), this, SLOT(sRevokeItemsite()));
  connect(_addeditemsites, SIGNAL(itemSelected(int)), this, SLOT(sEditItemsite()));

  _availablebomitems->addColumn("Item Number",   _itemColumn, Qt::AlignLeft,  true, "item_number");
  _availablebomitems->addColumn("Alias Number",  _itemColumn, Qt::AlignLeft,  true, "itemalias_number");
  _availablebomitems->addColumn("Description",   -1,          Qt::AlignLeft,  true, "item_descrip1");

  _addedbomitems->addColumn("Item Number", _itemColumn, Qt::AlignLeft,   true, "item_number");
  _addedbomitems->addColumn("Qty Per",     _qtyColumn,  Qt::AlignRight,  true, "bomitem_qtyper");
  _addedbomitems->addColumn("Description",  -1,         Qt::AlignLeft,   true, "item_descrip1");
  
  _addeditemsites->addColumn("Site",        _itemColumn, Qt::AlignLeft,   true, "warehous_code");
  _addeditemsites->addColumn("Description", -1,          Qt::AlignLeft,   true, "warehous_descrip");
  
  _listPrice->setValidator(omfgThis->priceVal());
  _listCost->setValidator(omfgThis->costVal());

  _newitemid = -1;
  _isActive = false;
  _inTransaction = false;
  _captive = false;
  _committed = false;
}

copyItem::~copyItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void copyItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _source->setId(param.toInt());
    _source->setEnabled(false);
  }

  return NoError;
}

void copyItem::sNext()
{
  // Validate that the item number chosen is available.
  if (okToSave())
  {
    // Create the new item and move on to the second step,
    // choosing what else to copy (BOM, Routings, etc.)

    if (saveItem())
    {
      // Copy Itemsites and BOM if those check boxes are already checked
      // due to the restoration of the previous state of the screen from the
      // last time copyItem was used.
 
      sCopyBom();
      sCopyItemsite();

      // Display the next page so the user can make alterations to the staged
      // copy of child relations and decide what else to copy.  With the item
      // in existence, not just in a pending transaction, other users will
      // not be able to select the same item number in a separate, attempted copy.

      _pages->setCurrentIndex(1);

      // Ultimately, the user will either click "Copy" (sCopy), click "Cancel"
      // (sCancel), or close the window (closeEvent).  In the latter two cases,
      // the creation of the item and child objects will be undone.
    }
  }
}

void copyItem::sUpdateItem()
{
  XSqlQuery query;
  query.prepare("SELECT item_descrip1, item_listprice, item_listcost, item_active "
                "FROM item "
                "WHERE (item_id=:sourceitemid);");

  query.bindValue(":sourceitemid", _source->id());
  query.exec();

  if (query.first())
  {
    _targetItemDescrip->setText(query.value("item_descrip1").toString());
    _listPrice->setDouble(query.value("item_listprice").toDouble());
    _listCost->setDouble(query.value("item_listcost").toDouble());
    _isActive = query.value("item_active").toBool();
    _next->setEnabled(true);
  }
  else
  {
    _next->setEnabled(false);
  }
}

bool copyItem::saveItem()
{
  bool saved = false;

  if(_source->id() == -1)
    return saved;

  XSqlQuery itemsave;
  XSqlQuery itemupdate;

  itemsave.prepare("SELECT copyItem(item_id, :targetitemnumber) AS result,"
                   "       item_id "
                   "FROM item "
                   "WHERE (item_id=:sourceitemid);");

  itemsave.bindValue(":sourceitemid", _source->id());
  itemsave.bindValue(":targetitemnumber", _targetItemNumber->text().trimmed().toUpper());
  itemsave.exec();

  if (itemsave.first())
  {
    _newitemid = itemsave.value("result").toInt();

    // Set item inactive until copy is committed.

    itemupdate.prepare("UPDATE item SET item_active=false, "
                       "                item_descrip1=:item_descrip1 "
                       "WHERE (item_id=:item_id);");
    itemupdate.bindValue(":item_id", _newitemid);
    itemupdate.bindValue(":item_descrip1", _targetItemDescrip->text());
    itemupdate.exec();

    if (itemupdate.lastError().type() != QSqlError::NoError)
    {
      // Remove the copied item since the update failed.
      cancelCopy();

      // Then report the error to the user.
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying Item"),
                               itemupdate, __FILE__, __LINE__);
    }
    else
    {
      saved = true;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying Item"),
                                  itemsave, __FILE__, __LINE__))
  {
    saved = false;
  }

  return saved;
}

void copyItem::sCopyBom()
{
  if (_newitemid < 0)
    return;

  if (_copyBOM->isChecked())
  {
    XSqlQuery bomitemq;
    bomitemq.prepare("SELECT copyBom(:sourceitemid, :targetitemid, true) AS result;");
    bomitemq.bindValue(":sourceitemid", _source->id());
    bomitemq.bindValue(":targetitemid", _newitemid);
    bomitemq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Copying Bom"),
                             bomitemq, __FILE__, __LINE__))
      return;
  }
  else
  {
    XSqlQuery bomitemq;
    bomitemq.prepare("SELECT deleteBom(:targetitemid) AS result;");
    bomitemq.bindValue(":targetitemid", _newitemid);
    bomitemq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Bom"),
                             bomitemq, __FILE__, __LINE__))
      return;
  }
  
  sFillBomitem();
}

void copyItem::sAddBomitem()
{
  if (_newitemid == -1)
  {
    QMessageBox::critical(this, tr("Error"), tr("Please enter a Source Item."));
    return;
  }
  
  if (_availablebomitems->id() == -1)
  {
    QMessageBox::critical(this, tr("Error"), tr("Please select an Available BOM Item."));
    return;
  }

  XSqlQuery bomitemq;
  int uomid=-1;
  QString uomname;
  double qtyper = 1.0;
  bool ok;
  
  bomitemq.prepare("SELECT item_inv_uom_id, uom_name "
                   "FROM item JOIN uom ON (uom_id=item_inv_uom_id) "
                   "WHERE (item_id=:item_id);");
  bomitemq.bindValue(":item_id", _availablebomitems->id());
  bomitemq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting UOM"),
                           bomitemq, __FILE__, __LINE__))
    return;

  if (bomitemq.first())
  {
    uomid = bomitemq.value("item_inv_uom_id").toInt();
    uomname = bomitemq.value("uom_name").toString();
  }
  qtyper = QInputDialog::getDouble(this, tr("Quantity Per"),
                                 uomname,
                                 qtyper, 0.00001, 999999.99999, 5, &ok);
  if ( !ok )
    return;
  
  bomitemq.prepare( "INSERT INTO bomitem"
                   " ( bomitem_parent_item_id, bomitem_seqnumber,"
                   "   bomitem_item_id, bomitem_qtyper, bomitem_scrap,"
                   "   bomitem_status, bomitem_effective, bomitem_expires,"
                   "   bomitem_createwo, bomitem_issuemethod, bomitem_schedatwooper,"
                   "   bomitem_ecn, bomitem_moddate, bomitem_subtype,"
                   "   bomitem_uom_id, bomitem_rev_id, bomitem_booitem_seq_id,"
                   "   bomitem_char_id, bomitem_value, bomitem_notes,"
                   "   bomitem_ref, bomitem_qtyfxd, bomitem_issuewo )"
                   " SELECT"
                   "   :targetitemid, (MAX(bomitem_seqnumber) + 10),"
                   "   :bomitemid, :qtyper, 0.0,"
                   "   NULL, startOfTime(), endOfTime(),"
                   "   false, 'M', true,"
                   "   NULL, CURRENT_DATE, 'I',"
                   "   :uomid, -1, -1,"
                   "   NULL, NULL, NULL,"
                   "   NULL, 0.0, false "
                   " FROM bomitem "
                   " WHERE (bomitem_parent_item_id=:targetitemid);" );
  bomitemq.bindValue(":targetitemid", _newitemid);
  bomitemq.bindValue(":bomitemid", _availablebomitems->id());
  bomitemq.bindValue(":qtyper", qtyper);
  bomitemq.bindValue(":uomid", uomid);
  bomitemq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Adding Bomitem"),
                           bomitemq, __FILE__, __LINE__))
    return;
  
  sFillBomitem();
}

void copyItem::sEditBomitem()
{
  if (_addedbomitems->id() == -1)
  {
    QMessageBox::critical(this, tr("Error"), tr("Please select a Component Item."));
    return;
  }
  
  ParameterList params;
  params.append("mode", "edit");
  params.append("bomitem_id", _addedbomitems->id());
  
  bomItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  
  sFillBomitem();
}

void copyItem::sRevokeBomitem()
{
  if (_addedbomitems->id() == -1)
  {
    QMessageBox::critical(this, tr("Error"), tr("Please select a Component Item."));
    return;
  }
  
  XSqlQuery bomitemq;
  bomitemq.prepare("DELETE FROM bomitem WHERE (bomitem_id=:bomitemid);");
  bomitemq.bindValue(":bomitemid", _addedbomitems->id());
  bomitemq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Revoking BOM Item"),
                           bomitemq, __FILE__, __LINE__))
    return;
  
  sFillBomitem();
}

void copyItem::sFillItem()
{
  if (_searchForBOM->text().length() < 3)
    return;
  
  _availablebomitems->clear();
  XSqlQuery bomitemq;
  bomitemq.prepare("SELECT item_id, item_number, itemalias_number, item_descrip1 "
                   "FROM item LEFT OUTER JOIN itemalias ON (itemalias_item_id=item_id) "
                   "WHERE ( (item_number ~* :searchfor) OR"
                   "        (itemalias_number ~* :searchfor) OR"
                   "        (item_descrip1 ~* :searchfor) )"
                   "  AND (item_active OR fetchMetricBool('AllowInactiveBomItems'));");
  bomitemq.bindValue(":searchfor", _searchForBOM->text());
  bomitemq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Filling Bomitem"),
                           bomitemq, __FILE__, __LINE__))
    return;
  _availablebomitems->populate(bomitemq);
}

void copyItem::sFillBomitem()
{
  _addedbomitems->clear();
  XSqlQuery bomitemq;
  bomitemq.prepare("SELECT bomitem_id, item_number, item_descrip1, bomitem_qtyper "
                   "FROM bomitem JOIN item ON (item_id=bomitem_item_id) "
                   "WHERE (bomitem_parent_item_id=:targetitemid) "
                   "ORDER BY bomitem_seqnumber;");
  bomitemq.bindValue(":targetitemid", _newitemid);
  bomitemq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Filling Bomitem"),
                           bomitemq, __FILE__, __LINE__))
    return;
  _addedbomitems->populate(bomitemq);
  
  if (_rollupPrices->isChecked())
  {
    bomitemq.prepare("SELECT roundSale(SUM(item_listprice * bomitem_qtyper * (1.0 + bomitem_scrap))) AS listprice,"
                     "       roundSale(SUM(item_listcost * bomitem_qtyper * (1.0 + bomitem_scrap))) AS listcost "
                     "FROM bomitem JOIN item ON (item_id=bomitem_item_id) "
                     "WHERE (bomitem_parent_item_id=:targetitemid);");
    bomitemq.bindValue(":targetitemid", _newitemid);
    bomitemq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Filling Bomitem"),
                             bomitemq, __FILE__, __LINE__))
      return;
    else if (bomitemq.first())
    {
      _listPrice->setDouble(bomitemq.value("listprice").toDouble());
      _listCost->setDouble(bomitemq.value("listcost").toDouble());
    }
  }
  else
  {
    bomitemq.prepare("SELECT item_listprice, item_listcost "
                     "FROM item "
                     "WHERE (item_id=:sourceitemid);");
    bomitemq.bindValue(":sourceitemid", _source->id());
    bomitemq.exec();
    if(bomitemq.first())
    {
      _listPrice->setDouble(bomitemq.value("item_listprice").toDouble());
      _listCost->setDouble(bomitemq.value("item_listcost").toDouble());
    }
  }
}

void copyItem::sCopyItemsite()
{
  if (_newitemid < 0)
    return;

  if (_copyItemsite->isChecked())
  {
    XSqlQuery itemsiteq;
    itemsiteq.prepare("SELECT copyItemsite(itemsite_id, itemsite_warehous_id, :targetitemid) AS result FROM "
                      "(SELECT itemsite_id, itemsite_warehous_id"
                      "  FROM itemsite JOIN whsinfo ON (warehous_id=itemsite_warehous_id) "
                      " WHERE (itemsite_item_id=:sourceitemid) "
                      " ORDER BY warehous_sequence DESC) AS data;");
    itemsiteq.bindValue(":sourceitemid", _source->id());
    itemsiteq.bindValue(":targetitemid", _newitemid);
    itemsiteq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Copying Itemsite"),
                             itemsiteq, __FILE__, __LINE__))
      return;
  }
  else
  {
    XSqlQuery itemsiteq;
    itemsiteq.prepare("DELETE FROM itemsite "
                      "WHERE (itemsite_item_id=:targetitemid);");
    itemsiteq.bindValue(":targetitemid", _newitemid);
    itemsiteq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Itemsite"),
                             itemsiteq, __FILE__, __LINE__))
      return;
  }
  
  sFillItemsite();
}

void copyItem::sAddItemsite()
{
  if (_addeditemsites->id() == -1)
  {
    QMessageBox::critical(this, tr("Error"), tr("Please select an Item Site."));
    return;
  }
  
  XSqlQuery itemCopy;
  itemCopy.prepare("SELECT copyItemSite(:olditemsiteid, NULL) AS result;");
  itemCopy.bindValue(":olditemsiteid", _addeditemsites->id());
  itemCopy.exec();
  if (itemCopy.first())
  {
    int result = itemCopy.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying Item Site"),
                             storedProcErrorLookup("copyItemSite", result),
                             __FILE__, __LINE__);
      return;
    }
    ParameterList params;
    params.append("mode", "edit");
    params.append("itemsite_id", result);
    
    itemSite newdlg(this, "", true);
    newdlg.set(params);
    if (newdlg.exec() != XDialog::Accepted)
    {
      itemCopy.prepare("SELECT deleteItemSite(:itemsite_id) AS result;");
      itemCopy.bindValue(":itemsite_id", result);
      itemCopy.exec();
      if (itemCopy.first())
      {
        int result = itemCopy.value("result").toInt();
        if (result < 0)
        {
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying Item Site"),
                                 storedProcErrorLookup("deleteItemSite", result),
                                 __FILE__, __LINE__);
          return;
        }
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying Item Site"),
                                    itemCopy, __FILE__, __LINE__))
      {
        return;
      }
    }
    sFillItemsite();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying Item Site"),
                                itemCopy, __FILE__, __LINE__))
  {
    return;
  }
}

void copyItem::sEditItemsite()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", _addeditemsites->id());
  
  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void copyItem::sRevokeItemsite()
{
  if (_addeditemsites->id() == -1)
  {
    QMessageBox::critical(this, tr("Error"), tr("Please select an Item Site."));
    return;
  }
  
  XSqlQuery itemsiteq;
  itemsiteq.prepare("DELETE FROM itemsite WHERE (itemsite_id=:itemsiteid);");
  itemsiteq.bindValue(":itemsiteid", _addeditemsites->id());
  itemsiteq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Revoking Item Site"),
                           itemsiteq, __FILE__, __LINE__))
    return;
  
  sFillItemsite();
}

void copyItem::sFillItemsite()
{
  _addeditemsites->clear();
  XSqlQuery itemsiteq;
  itemsiteq.prepare("SELECT itemsite_id, item_number, warehous_code, warehous_descrip "
                    "FROM itemsite JOIN item ON (item_id=itemsite_item_id)"
                    "              JOIN whsinfo ON (warehous_id=itemsite_warehous_id) "
                    "WHERE (itemsite_item_id=:targetitemid) "
                    "ORDER BY warehous_code;");
  itemsiteq.bindValue(":targetitemid", _newitemid);
  itemsiteq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Filling Item Site"),
                           itemsiteq, __FILE__, __LINE__))
    return;
  _addeditemsites->populate(itemsiteq);
}

bool copyItem::okToSave()
{
  XSqlQuery copyokToSave;
  _targetItemNumber->setText(_targetItemNumber->text().trimmed().toUpper());

  QList<GuiErrorCheck>errors;
  errors<<GuiErrorCheck(_targetItemNumber->text().length() == 0, _targetItemNumber,
                        tr("<p>Please enter a Target Item Number."));

  if(GuiErrorCheck::reportErrors(this,tr("Missing Target Item Number"),errors))
    return false;

  copyokToSave.prepare( "SELECT item_number "
             "FROM item "
             "WHERE item_number=:item_number;" );
  copyokToSave.bindValue(":item_number", _targetItemNumber->text());
  copyokToSave.exec();
  if (copyokToSave.first())
  {
    QMessageBox::critical(this, tr("Item Number Exists"),
                          tr("<p>An Item with the item number '%1' already "
                             "exists. You may not copy over an existing item.")
                            .arg(_targetItemNumber->text()));

    _targetItemNumber->clear();
    _targetItemNumber->setFocus();
    return false;
  }

  return true;
}

void copyItem::sCopy()
{
  XSqlQuery copyCopy;

  copyCopy.prepare("UPDATE item SET item_active=:item_active, "
                   "                item_listprice=:item_listprice, "
                   "                item_listcost=:item_listcost "
                   "WHERE (item_id=:item_id);");
  copyCopy.bindValue(":item_id", _newitemid);
  copyCopy.bindValue(":item_active", _isActive);
  copyCopy.bindValue(":item_listprice", _listPrice->toDouble());
  copyCopy.bindValue(":item_listcost", _listCost->toDouble());
  copyCopy.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying Item"),
                                copyCopy, __FILE__, __LINE__))
  {
    return;
  }

  copyCopy.prepare("SELECT doUpdateCosts(:item_id, true, :lowMaterial, :dirLabor, "
                   "         :lowDirLabor, :overhead, :lowOverhead, :machOverhead, "
                   "         :lowMachOverhead, :lowUser, :rollUp, :updateActual)");
  copyCopy.bindValue(":item_id",         _newitemid);
  copyCopy.bindValue(":lowMaterial",     "t");
  if (_metrics->boolean("Routings"))
  {
    copyCopy.bindValue(":dirLabor",        "t");
    copyCopy.bindValue(":lowDirLabor",     "t");
    copyCopy.bindValue(":overhead",        "t");
    copyCopy.bindValue(":lowOverhead",     "t");
    if (_metrics->value("TrackMachineOverhead") == "M")
    {
      copyCopy.bindValue(":machOverhead",  "t");
      copyCopy.bindValue(":lowMachOverhead", "t");
    }
    else
    {
      copyCopy.bindValue(":machOverhead",  "f");
      copyCopy.bindValue(":lowMachOverhead", "f");
    }
  }
  else
  {
    copyCopy.bindValue(":dirLabor",        "f");
    copyCopy.bindValue(":lowDirLabor",     "f");
    copyCopy.bindValue(":overhead",        "f");
    copyCopy.bindValue(":lowOverhead",     "f");
    copyCopy.bindValue(":machOverhead",    "f");
    copyCopy.bindValue(":lowMachOverhead", "f");
  }
  copyCopy.bindValue(":lowUser",         "t");
  copyCopy.bindValue(":rollUp",          "f");
  copyCopy.bindValue(":updateActual",    "t" );
  copyCopy.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying Item"),
                                copyCopy, __FILE__, __LINE__))
  {
    return;
  }

  copyCopy.prepare( "SELECT doPostCosts(:item_id, true, "
                    "         :material, :lowMaterial, :labor, :lowLabor, "
                    "         :overhead, :lowOverhead, :machOverhead, :lowMachOverhead, "
                    "         :user, :lowUser, :rollUp)" );
  copyCopy.bindValue(":item_id",         _newitemid);
  copyCopy.bindValue(":material",        "t");
  copyCopy.bindValue(":lowMaterial",     "t");
  if (_metrics->boolean("Routings"))
  {
    copyCopy.bindValue(":labor",           "t");
    copyCopy.bindValue(":lowLabor",        "t");
    copyCopy.bindValue(":overhead",        "t");
    copyCopy.bindValue(":lowOverhead",     "t");
    if (_metrics->value("TrackMachineOverhead") == "M")
    {
      copyCopy.bindValue(":machOverhead",  "t");
      copyCopy.bindValue(":lowMachOverhead", "t");
    }
    else
    {
      copyCopy.bindValue(":machOverhead",  "f");
      copyCopy.bindValue(":lowMachOverhead", "f");
    }
  }
  else
  {
    copyCopy.bindValue(":labor",           "f");
    copyCopy.bindValue(":lowLabor",        "f");
    copyCopy.bindValue(":overhead",        "f");
    copyCopy.bindValue(":lowOverhead",     "f");
    copyCopy.bindValue(":machOverhead",    "f");
    copyCopy.bindValue(":lowMachOverhead", "f");
  }
  copyCopy.bindValue(":user",            "t");
  copyCopy.bindValue(":lowUser",         "t");
  copyCopy.bindValue(":rollUp",          "f");
  copyCopy.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying Item"),
                                copyCopy, __FILE__, __LINE__))
  {
    return;
  }

  _committed = true;

  omfgThis->sItemsUpdated(_newitemid, true);

  if (_copyBOM->isChecked())
    omfgThis->sBOMsUpdated(_newitemid, true);

  if (_captive)
    done(_newitemid);
  else
    clear();
}

void copyItem::sCancel()
{
  // The user decided not to copy the item.
  cancelCopy();
  close();
}

void copyItem::cancelCopy()
{
  if (_newitemid > 0)
  { 
    // Delete the item's BOM (if any)

    XSqlQuery bomitemq;
    bomitemq.prepare("SELECT deleteBom(:targetitemid) AS result;");
    bomitemq.bindValue(":targetitemid", _newitemid);
    bomitemq.exec();

    if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Bom"),
                             bomitemq, __FILE__, __LINE__))
    {
      return;
    }

    // Delete the item sites (if any)

    XSqlQuery itemsiteq;
    itemsiteq.prepare("DELETE FROM itemsite "
                      "WHERE (itemsite_item_id=:targetitemid);");
    itemsiteq.bindValue(":targetitemid", _newitemid);
    itemsiteq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Itemsite"),
                             itemsiteq, __FILE__, __LINE__))
      return;
    
    // Delete the item

    XSqlQuery query;
    query.prepare("SELECT deleteItem(:itemid)");
    query.bindValue(":itemid", _newitemid);
    query.exec();
  }
}

void copyItem::clear()
{
  _newitemid = -1;
  _committed = false;
  _source->setId(-1);
  _pages->setCurrentIndex(0);
  _targetItemNumber->clear();
  _targetItemDescrip->clear();
  _listCost->clear();
  _listPrice->clear();
  _availablebomitems->clear();
  _addedbomitems->clear();
  _addeditemsites->clear();
  _source->setFocus();
  _close->setText(tr("&Close"));
}

void copyItem::closeEvent(QCloseEvent *pEvent)
{
  if (!_committed)
    cancelCopy(); 

  XDialog::closeEvent(pEvent);
}

int copyItem::id() const
{
  return _newitemid;
}

