/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemSources.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

#include "itemSource.h"
#include "buyCard.h"
#include "dspPoItemsByVendor.h"
#include "dspPoItemReceivingsByItem.h"
#include "guiclient.h"
#include "parameterwidget.h"

dspItemSources::dspItemSources(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspItemSources", fl)
{
  setWindowTitle(tr("Item Sources"));
  setReportName("ItemSources");
  setMetaSQLOptions("itemSources", "prices");
  setUseAltId(true);
  setParameterWidgetVisible(true);
//  setNewVisible(true);
//  setQueryOnStartEnabled(true);

  if (_metrics->boolean("MultiWhs"))
  {
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);
    parameterWidget()->append(tr("Drop Ship Only"), "showDropShip", ParameterWidget::Exists);
  }
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->append(tr("Item Number Pattern"), "item_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item Description"), "item_descrip_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Vendor"), "vend_id", ParameterWidget::Vendor);
  parameterWidget()->append(tr("Vendor Name"), "vend_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Vendor Item Number Pattern"), "vend_item_number_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Contract"), "contrct_id", XComboBox::Contracts);
  parameterWidget()->append(tr("Contract Number Pattern"), "contract_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Manufacturer Pattern"), "manuf_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Manufacturer Item Number Pattern"), "manuf_item_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Effective Start"), "effectiveStartDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Effective End"), "effectiveEndDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Expires Start"), "expireStartDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Expires End"), "expireEndDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Show Inactive"), "showInactive", ParameterWidget::Exists);

  if (_metrics->boolean("MultiWhs"))
  {
    list()->addColumn(tr("Site"),              _qtyColumn, Qt::AlignCenter, true, "warehous_code");
    list()->addColumn(tr("Order Type"),                -1, Qt::AlignCenter, true, "itemsrcp_dropship");
  }
  list()->addColumn(tr("Item Number"),        _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),        -1,          Qt::AlignLeft,   false, "item_descrip"   );
  list()->addColumn(tr("UOM"),                _uomColumn,  Qt::AlignCenter, false, "uom_name" );
  list()->addColumn(tr("Vendor #"),           _itemColumn, Qt::AlignLeft,   true,  "vend_number"   );
  list()->addColumn(tr("Vendor Name"),        -1,          Qt::AlignLeft,   true,  "vend_name"   );
  list()->addColumn(tr("Contract #"),         _itemColumn, Qt::AlignLeft,   true,  "contrct_number"   );
  list()->addColumn(tr("Effective"),          _dateColumn, Qt::AlignCenter, false, "itemsrc_effective"   );
  list()->addColumn(tr("Expires"),            _dateColumn, Qt::AlignCenter, false, "itemsrc_expires"   );
  list()->addColumn(tr("Vendor Currency"),    _itemColumn, Qt::AlignCenter, false, "vend_curr"   );
  list()->addColumn(tr("Vendor Item Number"), _itemColumn, Qt::AlignLeft,   true,  "itemsrc_vend_item_number"   );
  list()->addColumn(tr("Manufacturer"),       _itemColumn, Qt::AlignLeft,   false, "itemsrc_manuf_name" );
  list()->addColumn(tr("Manuf. Item#"),       _itemColumn, Qt::AlignLeft,   false, "itemsrc_manuf_item_number" );
  list()->addColumn(tr("Default"),            _uomColumn,  Qt::AlignCenter, true,  "itemsrc_default" );
  list()->addColumn(tr("Vendor UOM"),         _uomColumn,  Qt::AlignCenter, false, "itemsrc_vend_uom"   );
  list()->addColumn(tr("UOM Ratio"),          _qtyColumn,  Qt::AlignRight,  false, "itemsrc_invvendoruomratio"  );
  list()->addColumn(tr("Min. Order"),         _qtyColumn,  Qt::AlignRight,  false, "itemsrc_minordqty" );
  list()->addColumn(tr("Order Mult."),        _qtyColumn,  Qt::AlignRight,  false, "itemsrc_multordqty" );
  list()->addColumn(tr("Vendor Ranking"),     _qtyColumn,  Qt::AlignRight,  false, "itemsrc_ranking" );
  list()->addColumn(tr("Lead Time"),          _qtyColumn,  Qt::AlignRight,  false, "itemsrc_leadtime" );
  list()->addColumn(tr("Qty. Break"),         _qtyColumn,  Qt::AlignRight,  true,  "itemsrcp_qtybreak" );
  list()->addColumn(tr("Base Unit Price"),    _moneyColumn,Qt::AlignRight,  true,  "price_base" );
  list()->addColumn(tr("Item Currency"),      _itemColumn, Qt::AlignRight,  false, "item_curr" );
  list()->addColumn(tr("Unit Price"),         _moneyColumn,Qt::AlignRight,  false, "price_local" );

  if (_privileges->check("MaintainItemSources"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }
}

void dspItemSources::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = menuThis->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuItem = menuThis->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources") || _privileges->check("ViewItemSource"));

  menuItem = menuThis->addAction(tr("Set as Default..."), this, SLOT(sDefault()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

//  menuItem = menuThis->addAction(tr("Copy..."), this, SLOT(sCopy()));
//  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

//  menuItem = menuThis->addAction(tr("Delete..."), this, SLOT(sDelete()));
//  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuThis->addSeparator();

//  menuThis->addAction("View Buy Card...",  this, SLOT(sBuyCard()));

//  menuThis->addAction("View Receipts and Returns...",  this, SLOT(sReceipts()));
}

void dspItemSources::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  itemSource newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSources::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", list()->altId());

  itemSource newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSources::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsrc_id", list()->altId());

  itemSource newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSources::sDefault()
{
  XSqlQuery itemSave;
    itemSave.prepare( "UPDATE itemsrc "
               "SET itemsrc_default=true "
               "WHERE (itemsrc_id=:itemsrc_id);" );

  itemSave.bindValue(":itemsrc_id", list()->altId());
  itemSave.exec();
  if (itemSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspItemSources::sCopy()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("itemsrc_id", list()->altId());

  itemSource newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSources::sDelete()
{
  XSqlQuery itemDelete;
  itemDelete.prepare("SELECT poitem_id, itemsrc_active "
            "FROM poitem, itemsrc "
            "WHERE ((poitem_itemsrc_id=:itemsrc_id) "
            "AND (itemsrc_id=:itemsrc_id)); ");
  itemDelete.bindValue(":itemsrc_id", list()->altId());
  itemDelete.exec();
  if (itemDelete.first())
  {
    if (itemDelete.value("itemsrc_active").toBool())
    {
      if (QMessageBox::question(this, tr("Delete Item Source"),
                                tr("<p>This item source is used by existing "
                                   "purchase order records and may not be "
                                   "deleted. Would you like to deactivate it "
                                   "instead?"),
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
      {
        itemDelete.prepare( "UPDATE itemsrc SET "
                   "  itemsrc_active=false "
                   "WHERE (itemsrc_id=:itemsrc_id);" );
        itemDelete.bindValue(":itemsrc_id", list()->altId());
        itemDelete.exec();
      }
    }
    else
      QMessageBox::critical(this, tr("Delete Item Source"),
                            tr("<p>This item source is used by existing "
                               "purchase order records and may not be deleted."));
    return;
  }

  itemDelete.prepare( "SELECT item_number "
             "FROM itemsrc, item "
             "WHERE ( (itemsrc_item_id=item_id)"
             " AND (itemsrc_id=:itemsrc_id) );" );
  itemDelete.bindValue(":itemsrc_id", list()->altId());
  itemDelete.exec();
  if (itemDelete.first())
  {
    if (QMessageBox::question(this, tr("Delete Item Source"),
                              tr( "Are you sure that you want to delete the "
                                 "Item Source for %1?")
                                  .arg(itemDelete.value("item_number").toString()),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) == QMessageBox::Yes)
    {
      itemDelete.prepare( "DELETE FROM itemsrc "
                 "WHERE (itemsrc_id=:itemsrc_id);"
                 "DELETE FROM itemsrcp "
                 "WHERE (itemsrcp_itemsrc_id=:itemsrc_id);" );
      itemDelete.bindValue(":itemsrc_id", list()->altId());
      itemDelete.exec();
    }
  }
}

void dspItemSources::sBuyCard()
{
  ParameterList params;
  params.append("itemsrc_id", list()->altId());

  buyCard *newdlg = new buyCard();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSources::sReceipts()
{
  ParameterList params;
  params.append("itemsrc_id", list()->id());

  dspPoItemReceivingsByItem *newdlg = new dspPoItemReceivingsByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspItemSources::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  params.append("always", tr("'Always'"));
  params.append("never", tr("'Never'"));
  params.append("all", tr("All"));
  params.append("stock", tr("Into Stock"));
  params.append("dropship", tr("Drop Ship"));

  return true;
}
