/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemCost.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "currcluster.h"
#include "mqlutil.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "storedProcErrorLookup.h"

#define cItemCost    0x01
#define cBOMItemCost 0x02

static bool _foundCostElems;

itemCost::itemCost(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _captive = false;
  _itemcostid = -1;

  _postCost->setEnabled(_privileges->check("PostActualCosts"));
  _foundCostElems = true; // assume it's true until sPopulateCostelem
}

itemCost::~itemCost()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemCost::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemCost::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("bomitem_id", &valid);
  if (valid)
  {
    _type = cBOMItemCost;
    _bomitemid = param.toInt();
    itemet.prepare( "SELECT bomitem_item_id "
               "FROM bomitem "
               "WHERE (bomitem_id=:bomitem_id);" );
    itemet.bindValue(":bomitem_id", _bomitemid);
    itemet.exec();
    if (itemet.first())
    {
      _item->setId(itemet.value("bomitem_item_id").toInt());
      _item->setReadOnly(true);
    }
  }

  param = pParams.value("bomitem_item_id", &valid);
  if (valid)
  {
    _type = cBOMItemCost;
    _item->setId(param.toInt());
    _item->setReadOnly(true);
  }

  param = pParams.value("bomitemcost_id", &valid);
  if (valid)
  {
    _type = cBOMItemCost;
    _itemcostid = param.toInt();
    _item->setReadOnly(true);

    itemet.prepare( "SELECT bomitem_item_id, formatBoolYN(bomitemcost_lowlevel) AS lowlevel,"
               "       bomitemcost_actcost, bomitemcost_curr_id, bomitemcost_updated "
               "FROM bomitem, bomitemcost "
               "WHERE ( (bomitemcost_bomitem_id=bomitem_id)"
               " AND    (bomitemcost_id=:bomitemcost_id) );" );
    itemet.bindValue(":bomitemcost_id", _itemcostid);
    itemet.exec();
    if (itemet.first())
    {
      _item->setId(itemet.value("bomitem_item_id").toInt());
      _lowerLevel->setText(itemet.value("lowlevel").toString());
      _actualCost->set(itemet.value("bomitemcost_actcost").toDouble(),
                       itemet.value("bomitemcost_curr_id").toInt(),
                       QDate::currentDate(),
                       false);
    }
    else if (itemet.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemet.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _type = cItemCost;
    _item->setId(param.toInt());
    _item->setReadOnly(true);
  }

  param = pParams.value("itemcost_id", &valid);
  if (valid)
  {
    _type = cItemCost;
    _itemcostid = param.toInt();
    _item->setReadOnly(true);

    itemet.prepare( "SELECT item_id, formatBoolYN(itemcost_lowlevel) AS lowlevel,"
               "       itemcost_actcost, itemcost_curr_id, itemcost_updated "
               "FROM item, itemcost "
               "WHERE ( (itemcost_item_id=item_id)"
               " AND    (itemcost_id=:itemcost_id) );" );
    itemet.bindValue(":itemcost_id", _itemcostid);
    itemet.exec();
    if (itemet.first())
    {
      _item->setId(itemet.value("item_id").toInt());
      _lowerLevel->setText(itemet.value("lowlevel").toString());
      _actualCost->set(itemet.value("itemcost_actcost").toDouble(),
		       itemet.value("itemcost_curr_id").toInt(),
		       QDate::currentDate(),
		       false);
    }
    else if (itemet.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemet.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _lowerLevel->hide();
      _lowerLevelLit->hide();
      if (_type == cItemCost)
        setWindowTitle("Create Item Cost");
      else
        setWindowTitle("Create BOM Item Cost");
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _costelem->setEnabled(false);
      if (_type == cItemCost)
        setWindowTitle("Enter Actual Cost");
      else
        setWindowTitle("Enter BOM Item Actual Cost");
    }
    else if (param.toString() == "view")
    {
      _actualCost->setEnabled(false);
      _costelem->setEnabled(false);
      _postCost->setEnabled(false);
      _save->hide();
      _close->setText("&Close");
      if (_type == cItemCost)
        setWindowTitle("View Actual Cost");
      else
        setWindowTitle("View BOM Item Actual Cost");
    }
  }

  sPopulateCostelem();

  return _foundCostElems ? NoError : UndefinedError;
}

void itemCost::sSave()
{
  XSqlQuery itemSave;
  if (_mode != cNew && _mode != cEdit)
    done(_itemcostid);

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_costelem->id() < 0, _costelem,
                          tr("You must select a Costing Element to save this Item Cost."))
     ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Item Cost"), errors))
    return;

  if (_type == cItemCost)
  {
    if (_mode == cNew)
    {
      itemSave.exec("SELECT NEXTVAL('itemcost_itemcost_id_seq') AS itemcost_id;");
      if (itemSave.first())
      {
        _itemcostid = itemSave.value("itemcost_id").toInt();

        itemSave.prepare( "INSERT INTO itemcost "
                   "( itemcost_id, itemcost_item_id,"
                   "  itemcost_costelem_id, itemcost_lowlevel,"
                   "  itemcost_stdcost, itemcost_posted,"
                   "  itemcost_actcost, itemcost_updated, "
                   "  itemcost_curr_id ) "
                   "VALUES "
                   "( :itemcost_id, :itemcost_item_id,"
                   "  :itemcost_costelem_id, false,"
                   "  0, startOfTime(),"
                   "  :itemcost_actcost, CURRENT_DATE, "
                   "  :itemcost_curr_id );" );
        itemSave.bindValue(":itemcost_item_id", _item->id());
        itemSave.bindValue(":itemcost_costelem_id", _costelem->id());
      }
    }
    else if (_mode == cEdit)
    {
        itemSave.prepare( "UPDATE itemcost SET"
                   " itemcost_actcost=:itemcost_actcost,"
                   " itemcost_curr_id=:itemcost_curr_id, "
                   " itemcost_updated=CURRENT_DATE "
                   "WHERE (itemcost_id=:itemcost_id);");
    }
    itemSave.bindValue(":itemcost_id", _itemcostid);
    itemSave.bindValue(":itemcost_actcost", _actualCost->localValue());
    itemSave.bindValue(":itemcost_curr_id", _actualCost->id());

    if (itemSave.exec() && _postCost->isChecked())
    {
      itemSave.prepare("SELECT postCost(:itemcost_id) AS result;");
      itemSave.bindValue(":itemcost_id", _itemcostid);
      itemSave.exec();
      if (itemSave.first())
      {
        int result = itemSave.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("postCost", result),
                      __FILE__, __LINE__);
          return;
        }
      }
    }

    if (itemSave.lastError().type() != QSqlError::NoError)  // if EITHER itemSave.exec() failed
    {
      systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (_type == cBOMItemCost)
  {
    if (_mode == cNew)
    {
      itemSave.exec("SELECT NEXTVAL('bomitemcost_bomitemcost_id_seq') AS itemcost_id;");
      if (itemSave.first())
      {
        _itemcostid = itemSave.value("itemcost_id").toInt();

        itemSave.prepare( "INSERT INTO bomitemcost "
                   "( bomitemcost_id, bomitemcost_bomitem_id,"
                   "  bomitemcost_costelem_id, bomitemcost_lowlevel,"
                   "  bomitemcost_stdcost, bomitemcost_posted,"
                   "  bomitemcost_actcost, bomitemcost_updated, "
                   "  bomitemcost_curr_id ) "
                   "VALUES "
                   "( :bomitemcost_id, :bomitemcost_bomitem_id,"
                   "  :bomitemcost_costelem_id, false,"
                   "  0, startOfTime(),"
                   "  :bomitemcost_actcost, CURRENT_DATE, "
                   "  :bomitemcost_curr_id );" );
        itemSave.bindValue(":bomitemcost_bomitem_id", _bomitemid);
        itemSave.bindValue(":bomitemcost_costelem_id", _costelem->id());
      }
    }
    else if (_mode == cEdit)
    {
      itemSave.prepare( "UPDATE bomitemcost SET"
                 " bomitemcost_actcost=:bomitemcost_actcost,"
                 " bomitemcost_curr_id=:bomitemcost_curr_id, "
                 " bomitemcost_updated=CURRENT_DATE "
                 "WHERE (bomitemcost_id=:bomitemcost_id);");
    }
    itemSave.bindValue(":bomitemcost_id", _itemcostid);
    itemSave.bindValue(":bomitemcost_actcost", _actualCost->localValue());
    itemSave.bindValue(":bomitemcost_curr_id", _actualCost->id());

    if (itemSave.exec() && _postCost->isChecked())
    {
      itemSave.prepare("UPDATE bomitemcost SET bomitemcost_stdcost=round(currToBase(bomitemcost_curr_id, bomitemcost_actcost, CURRENT_DATE),6), "
                "                       bomitemcost_posted=CURRENT_DATE "
                "WHERE bomitemcost_id=:bomitemcost_id;");
      itemSave.bindValue(":bomitemcost_id", _itemcostid);
      itemSave.exec();
    }

    if (itemSave.lastError().type() != QSqlError::NoError)  // if EITHER itemSave.exec() failed
    {
      systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  done(_itemcostid);
}

void itemCost::sPopulateCostelem()
{
  XSqlQuery itemPopulateCostelem;
  if (_type == cItemCost)
  {
    if (_mode == cNew)
    {
      ParameterList params;
      params.append("item_id", _item->id());
      params.append("itemtype", _item->itemType());

      itemPopulateCostelem = mqlLoad("costelem", "unusedbyitem").toQuery(params);
      _costelem->populate(itemPopulateCostelem);
      if (itemPopulateCostelem.size() <= 0)
      {
        QMessageBox::warning(this, tr("No Costing Elements Remaining"),
                             tr("<p>Item %1 already has all available costing "
                                "elements assigned. No new Item Costs can be "
                                "created for it until more costing elements are "
                                "defined.")
                             .arg(_item->itemNumber()));
        _foundCostElems = false;
      }
    }
    else // _mode == cEdit || cView)
    {
      QString sql( "SELECT costelem_id, costelem_type "
                   "FROM costelem, itemcost "
                   "WHERE ((costelem_id=itemcost_costelem_id)"
                   "  AND  (itemcost_id=:itemcost_id));");
      itemPopulateCostelem.prepare(sql);
      itemPopulateCostelem.bindValue(":itemcost_id", _itemcostid);
      itemPopulateCostelem.exec();
      _costelem->populate(itemPopulateCostelem);
    }
  }
  if (_type == cBOMItemCost)
  {
    if (_mode == cNew)
    {
      ParameterList params;
      params.append("bomitem_id", _bomitemid);
      params.append("itemtype", _item->itemType());

      itemPopulateCostelem = mqlLoad("costelem", "unusedbyitem").toQuery(params);
      _costelem->populate(itemPopulateCostelem);
      if (itemPopulateCostelem.size() <= 0)
      {
        QMessageBox::warning(this, tr("No Costing Elements Remaining"),
                             tr("<p>BOM Item %1 already has all available costing "
                                "elements assigned. No new BOM Item Costs can be "
                                "created for it until more costing elements are "
                                "defined.")
                             .arg(_item->itemNumber()));
        _foundCostElems = false;
      }
    }
    else // _mode == cEdit || cView)
    {
      QString sql( "SELECT costelem_id, costelem_type "
                   "FROM costelem, bomitemcost "
                   "WHERE ((costelem_id=bomitemcost_costelem_id)"
                   "  AND  (bomitemcost_id=:itemcost_id));");
      itemPopulateCostelem.prepare(sql);
      itemPopulateCostelem.bindValue(":itemcost_id", _itemcostid);
      itemPopulateCostelem.exec();
      _costelem->populate(itemPopulateCostelem);
    }
  }
}
