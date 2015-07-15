/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSource.h"

#include <QAction>
#include <QCloseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "crmacctcluster.h"
#include "itemSourcePrice.h"
#include "xcombobox.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include <metasql.h>
#include <parameter.h>
#include "mqlutil.h"

itemSource::itemSource(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  XSqlQuery itemitemSource;
  setupUi(this);

  connect(_add,                SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_close,              SIGNAL(clicked()), this, SLOT(reject()));
  connect(_delete,             SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,               SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_itemsrcp,SIGNAL(populateMenu(QMenu*, XTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_save,               SIGNAL(clicked()), this, SLOT(sSaveClicked()));
  connect(_vendor,             SIGNAL(newId(int)), this, SLOT(sVendorChanged(int)));
  connect(_vendorCurrency,     SIGNAL(newID(int)), this, SLOT(sFillPriceList()));
  connect(_contract,           SIGNAL(newID(int)), this, SLOT(sContractChanged(int)));
  connect(this,                SIGNAL(rejected()), this, SLOT(sRejected()));

//  TODO method doesn't exist?
//  connect(_vendorUOM, SIGNAL(textChanged()), this, SLOT(sClearVendorUOM()));
//  connect(_invVendorUOMRatio, SIGNAL(textChanged(QString)), this, SLOT(sClearVendorUOM()));

  _vendorUOM->setType(XComboBox::UOMs);

  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured | ItemLineEdit::cTooling);
  _item->setDefaultType(ItemLineEdit::cGeneralPurchased);

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Expires"));

  _captive = false;
  _new = false;
  
  QString base;
  itemitemSource.exec("SELECT currConcat(baseCurrID()) AS base;");
  if (itemitemSource.first())
    base = itemitemSource.value("base").toString();
  else
    base = tr("Base");

  if (_metrics->boolean("MultiWhs"))
  {
    _itemsrcp->addColumn(tr("Site"),                      _qtyColumn, Qt::AlignCenter,true, "warehous_code");
    _itemsrcp->addColumn(tr("Order Type"),                        -1, Qt::AlignCenter,true, "itemsrcp_dropship");
  }
  _itemsrcp->addColumn(tr("Qty Break"),                   _qtyColumn, Qt::AlignRight, true, "itemsrcp_qtybreak");
  _itemsrcp->addColumn(tr("Unit Price"),                          -1, Qt::AlignRight, true, "itemsrcp_price");
  _itemsrcp->addColumn(tr("Currency"),               _currencyColumn, Qt::AlignLeft,  true, "item_curr");
  _itemsrcp->addColumn(tr("Discount Percent"),                    -1, Qt::AlignRight, true, "itemsrcp_discntprcnt" );
  _itemsrcp->addColumn(tr("Discount Fixed Amt."),                 -1, Qt::AlignRight, true, "itemsrcp_fixedamtdiscount" );
  _itemsrcp->addColumn(tr("Unit Price\n(%1)").arg(base),_moneyColumn, Qt::AlignRight, true, "price_base");
  _itemsrcp->addColumn(tr("Type"),                      _orderColumn, Qt::AlignLeft,  true, "type" );
  _itemsrcp->addColumn(tr("Method"),                    _orderColumn, Qt::AlignLeft,  true, "method" );

  if (omfgThis->singleCurrency())
  {
    _itemsrcp->hideColumn(1);
    _itemsrcp->hideColumn(2);
    _itemsrcp->headerItem()->setText(3, tr("Unit Price"));
  }

  _invVendorUOMRatio->setValidator(omfgThis->ratioVal());
  _minOrderQty->setValidator(omfgThis->qtyVal());
  _multOrderQty->setValidator(omfgThis->qtyVal());
  _contractedQty->setValidator(omfgThis->qtyVal());
  _contractedQty->setAlignment(Qt::AlignRight);

  _vendorCurrency->setType(XComboBox::Currencies);
  _vendorCurrency->setLabel(_vendorCurrencyLit);
  
  itemitemSource.exec("SELECT MAX(itemsrc_id),itemsrc_manuf_name, itemsrc_manuf_name "
                      "FROM itemsrc "
                      "WHERE (itemsrc_manuf_name != '') "
                      "GROUP BY itemsrc_manuf_name "
                      "ORDER BY itemsrc_manuf_name;");
  _manufName->populate(itemitemSource);
  _manufName->setCurrentIndex(0);
}

itemSource::~itemSource()
{
    // no need to delete child widgets, Qt does it all for us
}

void itemSource::languageChange()
{
    retranslateUi(this);
}

enum SetResponse itemSource::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsrc_id", &valid);
  if (valid)
  {
    _itemsrcid = param.toInt();
    _documents->setId(_itemsrcid);
    populate();
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setEnabled(false);
  }

  param = pParams.value("vend_id", &valid);
  if (valid)
  {
    _vendor->setId(param.toInt());
    _vendor->setEnabled(false);
  }
  
  param = pParams.value("contrct_id", &valid);
  if (valid)
  {
    _contract->setId(param.toInt());
  }
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _new = true;

      itemet.exec("SELECT NEXTVAL('itemsrc_itemsrc_id_seq') AS _itemsrc_id;");
      if (itemet.first())
      {
        _itemsrcid = itemet.value("_itemsrc_id").toInt();
        _documents->setId(_itemsrcid);
      }
      else if (itemet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, itemet.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      _captive = true;
      
      connect(_itemsrcp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_itemsrcp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _item->setReadOnly(true);
      _vendor->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(true);
      _active->setEnabled(false);
      _default->setEnabled(false);
      _vendor->setEnabled(false);
      _dates->setEnabled(false);
      _vendorItemNumber->setEnabled(false);
      _vendorItemDescrip->setEnabled(false);
      _vendorUOM->setEnabled(false);
      _invVendorUOMRatio->setEnabled(false);
      _vendorRanking->setEnabled(false);
      _minOrderQty->setEnabled(false);
      _multOrderQty->setEnabled(false);
      _leadTime->setEnabled(false);
      _notes->setEnabled(false);
      _upcCode->setEnabled(false);
      _documents->setReadOnly(true);
      _add->setEnabled(false);
      _delete->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
    if (param.toString() == "copy")
    {
      _mode = cCopy;
      _new = true;
      _captive = true;
      int itemsrcidold = _itemsrcid;

      itemet.exec("SELECT NEXTVAL('itemsrc_itemsrc_id_seq') AS _itemsrc_id;");
      if (itemet.first())
        _itemsrcid = itemet.value("_itemsrc_id").toInt();
      else if (itemet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, itemet.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      
      connect(_itemsrcp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      
      _item->setReadOnly(true);
      _vendorItemNumber->setText(_vendorItemNumber->text().prepend("Copy Of "));
      _dates->setStartDate(omfgThis->dbDate());

      if (sSave())
      {
        itemet.prepare("INSERT INTO itemsrcp ( "
                       "itemsrcp_itemsrc_id, itemsrcp_qtybreak, itemsrcp_price, "
                       "itemsrcp_updated, itemsrcp_curr_id, itemsrcp_dropship, "
                       "itemsrcp_warehous_id, itemsrcp_type, itemsrcp_discntprcnt, "
                       "itemsrcp_fixedamtdiscount) "
                       "SELECT :itemsrcid, itemsrcp_qtybreak, itemsrcp_price, "
                       "current_date, itemsrcp_curr_id, itemsrcp_dropship, "
                       "itemsrcp_warehous_id, itemsrcp_type, itemsrcp_discntprcnt, "
                       "itemsrcp_fixedamtdiscount "
                       "FROM itemsrcp "
                       "WHERE (itemsrcp_itemsrc_id=:itemsrcidold); ");
        itemet.bindValue(":itemsrcid", _itemsrcid);
        itemet.bindValue(":itemsrcidold", itemsrcidold);
        itemet.exec();
        sFillPriceList();
      }
    }
  }

  if (_metrics->value("Application") != "Standard")
  {
    _contractedQtyLit->hide();
    _contractedQty->hide();
  }

  return NoError;
}

int itemSource::id() const
{
  return _itemsrcid;
}

int itemSource::mode() const
{
  return _mode;
}

void itemSource::sSaveClicked()
{
  _captive = false;
  sSave();
}

bool itemSource::sSave()
{
  XSqlQuery itemSave;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_item->isValid(), _item,
                          tr( "You must select an Item that this Item Source represents\n"
                              "before you may save this Item Source." ) )
         << GuiErrorCheck(!_vendor->isValid(), _vendor,
                          tr( "You must select this Vendor that this Item Source is sold by\n"
                              "before you may save this Item Source." ) )
         << GuiErrorCheck(_dates->endDate() < _dates->startDate(), _dates,
                          tr("The expiration date cannot be earlier than the effective date."))
         << GuiErrorCheck(_vendorUOM->currentText().length() == 0, _vendorUOM,
                          tr( "You must indicate the Unit of Measure that this Item Source is sold in\n"
                               "before you may save this Item Source." ) )
         << GuiErrorCheck(_invVendorUOMRatio->toDouble() == 0.0, _invVendorUOMRatio,
                          tr( "You must indicate the Ratio of Inventory to Vendor Unit of Measures\n"
                               "before you may save this Item Source." ) )
     ;

  itemSave.prepare( "SELECT itemsrc_id "
                   "  FROM itemsrc "
                   " WHERE ((itemsrc_item_id=:itemsrc_item_id) "
                   "   AND (itemsrc_vend_id=:itemsrc_vend_id) "
                   "   AND ((itemsrc_contrct_id=:itemsrc_contrct_id) "
                   "    OR  (itemsrc_contrct_id IS NULL AND :itemsrc_contrct_id IS NULL)) "
                   "   AND (itemsrc_effective=:itemsrc_effective) "
                   "   AND (itemsrc_expires=:itemsrc_expires) "
                   "   AND (itemsrc_vend_item_number=:itemsrc_vend_item_number) "
                   "   AND (UPPER(itemsrc_manuf_name)=UPPER(:itemsrc_manuf_name)) "
                   "   AND (UPPER(itemsrc_manuf_item_number)=UPPER(:itemsrc_manuf_item_number)) "
                   "   AND (itemsrc_id != :itemsrc_id) );");
  itemSave.bindValue(":itemsrc_id", _itemsrcid);
  itemSave.bindValue(":itemsrc_item_id", _item->id());
  itemSave.bindValue(":itemsrc_vend_id", _vendor->id());
  if (_contract->isValid())
    itemSave.bindValue(":itemsrc_contrct_id", _contract->id());
  itemSave.bindValue(":itemsrc_effective", _dates->startDate());
  itemSave.bindValue(":itemsrc_expires", _dates->endDate());
  itemSave.bindValue(":itemsrc_vend_item_number", _vendorItemNumber->text());
  itemSave.bindValue(":itemsrc_manuf_name", _manufName->currentText());
  itemSave.bindValue(":itemsrc_manuf_item_number", _manufItemNumber->text());
  itemSave.exec();
  if(itemSave.first())
  {
    errors << GuiErrorCheck(true, _item,
                            tr("An Item Source already exists for the Item Number, Vendor,\n"
                               "Contract, Effective Date, Expires Date,\n"
                               "Vendor Item, Manfacturer Name and Manufacturer Item Number you have specified."));
  }
  else if (itemSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  
  if(_active->isChecked())
  {
    itemSave.prepare("SELECT item_id "
                     "FROM item "
                     "WHERE ((item_id=:item_id)"
                     "  AND  (item_active)) "
                     "LIMIT 1; ");
    itemSave.bindValue(":item_id", _item->id());
    itemSave.exec();
    if (!itemSave.first())         
    { 
      errors << GuiErrorCheck(true, _active,
                              tr("This Item Source refers to an inactive Item and must be marked as inactive.") );
    }
  }
    
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Item Source"), errors))
    return false;

  if (_mode == cNew || _mode == cCopy)
  {
    itemSave.prepare( "INSERT INTO itemsrc "
               "( itemsrc_id, itemsrc_item_id, itemsrc_active, itemsrc_default, itemsrc_vend_id,"
               "  itemsrc_contrct_id, itemsrc_effective, itemsrc_expires, itemsrc_contrct_min,"
               "  itemsrc_vend_item_number, itemsrc_vend_item_descrip,"
               "  itemsrc_vend_uom, itemsrc_invvendoruomratio,"
               "  itemsrc_minordqty, itemsrc_multordqty, itemsrc_upccode,"
               "  itemsrc_leadtime, itemsrc_ranking,"
               "  itemsrc_comments, itemsrc_manuf_name, "
               "  itemsrc_manuf_item_number, itemsrc_manuf_item_descrip ) "
               "VALUES "
               "( :itemsrc_id, :itemsrc_item_id, :itemsrc_active, :itemsrc_default, :itemsrc_vend_id,"
               "  :itemsrc_contrct_id, :itemsrc_effective, :itemsrc_expires, :itemsrc_contrct_min,"
               "  :itemsrc_vend_item_number, :itemsrc_vend_item_descrip,"
               "  :itemsrc_vend_uom, :itemsrc_invvendoruomratio,"
               "  :itemsrc_minordqty, :itemsrc_multordqty, :itemsrc_upccode,"
               "  :itemsrc_leadtime, :itemsrc_ranking,"
               "  :itemsrc_comments, :itemsrc_manuf_name, "
               "  :itemsrc_manuf_item_number, :itemsrc_manuf_item_descrip );" );
  }
  if (_mode == cEdit)
  {
    itemSave.prepare( "UPDATE itemsrc "
               "SET itemsrc_active=:itemsrc_active,"
               "    itemsrc_default=:itemsrc_default,"
               "    itemsrc_vend_id=:itemsrc_vend_id,"
               "    itemsrc_contrct_id=:itemsrc_contrct_id,"
               "    itemsrc_effective=:itemsrc_effective,"
               "    itemsrc_expires=:itemsrc_expires,"
               "    itemsrc_contrct_min=:itemsrc_contrct_min,"
               "    itemsrc_vend_item_number=:itemsrc_vend_item_number,"
               "    itemsrc_vend_item_descrip=:itemsrc_vend_item_descrip,"
               "    itemsrc_vend_uom=:itemsrc_vend_uom,"
               "    itemsrc_invvendoruomratio=:itemsrc_invvendoruomratio,"
               "    itemsrc_upccode=:itemsrc_upccode,"
               "    itemsrc_minordqty=:itemsrc_minordqty, itemsrc_multordqty=:itemsrc_multordqty,"
               "    itemsrc_leadtime=:itemsrc_leadtime, itemsrc_ranking=:itemsrc_ranking,"
               "    itemsrc_comments=:itemsrc_comments, itemsrc_manuf_name=:itemsrc_manuf_name, "
               "    itemsrc_manuf_item_number=:itemsrc_manuf_item_number, "
               "    itemsrc_manuf_item_descrip=:itemsrc_manuf_item_descrip "
               "WHERE (itemsrc_id=:itemsrc_id);" );
  }

  itemSave.bindValue(":itemsrc_id", _itemsrcid);
  itemSave.bindValue(":itemsrc_item_id", _item->id());
  itemSave.bindValue(":itemsrc_active", QVariant(_active->isChecked()));
  itemSave.bindValue(":itemsrc_default", QVariant(_default->isChecked()));
  itemSave.bindValue(":itemsrc_vend_id", _vendor->id());
  if (_contract->isValid())
    itemSave.bindValue(":itemsrc_contrct_id", _contract->id());
  itemSave.bindValue(":itemsrc_effective", _dates->startDate());
  itemSave.bindValue(":itemsrc_expires", _dates->endDate());
  itemSave.bindValue(":itemsrc_contrct_min", _contractedQty->toDouble());
  itemSave.bindValue(":itemsrc_vend_item_number", _vendorItemNumber->text());
  itemSave.bindValue(":itemsrc_vend_item_descrip", _vendorItemDescrip->toPlainText());
  itemSave.bindValue(":itemsrc_vend_uom", _vendorUOM->currentText());
  itemSave.bindValue(":itemsrc_invvendoruomratio", _invVendorUOMRatio->toDouble());
  itemSave.bindValue(":itemsrc_upccode", _upcCode->text());
  itemSave.bindValue(":itemsrc_minordqty", _minOrderQty->toDouble());
  itemSave.bindValue(":itemsrc_multordqty", _multOrderQty->toDouble());
  itemSave.bindValue(":itemsrc_leadtime", _leadTime->text().toInt());
  itemSave.bindValue(":itemsrc_ranking", _vendorRanking->value());
  itemSave.bindValue(":itemsrc_comments", _notes->toPlainText().trimmed());
  itemSave.bindValue(":itemsrc_manuf_name", _manufName->currentText());
  itemSave.bindValue(":itemsrc_manuf_item_number", _manufItemNumber->text());
  itemSave.bindValue(":itemsrc_manuf_item_descrip", _manufItemDescrip->toPlainText());
  itemSave.exec();
  if (itemSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  if (_captive)
  {
    if (_mode != cCopy)
    {
      _vendor->setEnabled(false);
    }
    _mode = cEdit;
    _item->setReadOnly(true);
    _captive = false;
  }
  else
    done(_itemsrcid);
    
  return true;
}

void itemSource::sAdd()
{
  if (_mode == cNew || _mode == cCopy)
  {
    if (!sSave())
      return;
  }
  
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsrc_id", _itemsrcid);
  params.append("curr_id", _vendorCurrency->id());

  itemSourcePrice newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillPriceList();
}

void itemSource::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrcp_id", _itemsrcp->id());

  itemSourcePrice newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillPriceList();
}

void itemSource::sDelete()
{
  XSqlQuery itemDelete;
//  Make sure the user is sure
  if (  QMessageBox::warning( this, tr("Delete Item Source Price"),
                              tr("Are you sure you want to delete this Item Source price?"),
                              tr("&Delete"), tr("&Cancel"), 0, 0, 1)  == 0  )
  {
    itemDelete.prepare( "DELETE FROM itemsrcp "
               "WHERE (itemsrcp_id=:itemsrcp_id);" );
    itemDelete.bindValue(":itemsrcp_id", _itemsrcp->id());
    itemDelete.exec();
    if (itemDelete.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemDelete.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillPriceList();
  }
}

void itemSource::sPopulateMenu(QMenu *pMenu)
{
  if (_mode != cView)
  {
    QAction *menuItem;
    
    menuItem = pMenu->addAction("Edit Item Source Price...", this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainItemSources"));
    
    menuItem = pMenu->addAction("Delete Item Source Price...", this, SLOT(sDelete()));
    menuItem->setEnabled(_privileges->check("MaintainItemSources"));
  }
}

void itemSource::sFillPriceList()
{
  XSqlQuery priceq;
  MetaSQLQuery mql = mqlLoad("itemSources", "prices");
  ParameterList params;
  params.append("itemsrc_id", _itemsrcid);
  params.append("nominal",tr("Nominal"));
  params.append("discount",tr("Discount"));
  params.append("price", tr("Price"));
  params.append("fixed", tr("Fixed"));
  params.append("percent", tr("Percent"));
  params.append("mixed", tr("Mixed"));
  params.append("all", tr("All"));
  params.append("stock", tr("Into Stock"));
  params.append("dropship", tr("Drop Ship"));

  priceq = mql.toQuery(params);
  _itemsrcp->populate(priceq);
}

void itemSource::populate()
{
  XSqlQuery itemsrcQ;
  itemsrcQ.prepare( "SELECT * FROM itemsrc WHERE (itemsrc_id=:itemsrc_id);" );
  itemsrcQ.bindValue(":itemsrc_id", _itemsrcid);
  itemsrcQ.exec();
  if (itemsrcQ.first())
  {
    _item->setId(itemsrcQ.value("itemsrc_item_id").toInt());
    _active->setChecked(itemsrcQ.value("itemsrc_active").toBool());
    _default->setChecked(itemsrcQ.value("itemsrc_default").toBool());
    _vendor->setId(itemsrcQ.value("itemsrc_vend_id").toInt());
    _contract->setId(itemsrcQ.value("itemsrc_contrct_id").toInt());
    _dates->setStartDate(itemsrcQ.value("itemsrc_effective").toDate());
    _dates->setEndDate(itemsrcQ.value("itemsrc_expires").toDate());
    _vendorItemNumber->setText(itemsrcQ.value("itemsrc_vend_item_number").toString());
    _vendorItemDescrip->setText(itemsrcQ.value("itemsrc_vend_item_descrip").toString());
    _vendorUOM->setCode(itemsrcQ.value("itemsrc_vend_uom").toString());
    _invVendorUOMRatio->setDouble(itemsrcQ.value("itemsrc_invvendoruomratio").toDouble());
    _upcCode->setText(itemsrcQ.value("itemsrc_upccode"));
    _minOrderQty->setDouble(itemsrcQ.value("itemsrc_minordqty").toDouble());
    _multOrderQty->setDouble(itemsrcQ.value("itemsrc_multordqty").toDouble());
    _vendorRanking->setValue(itemsrcQ.value("itemsrc_ranking").toInt());
    _leadTime->setValue(itemsrcQ.value("itemsrc_leadtime").toInt());
    _notes->setText(itemsrcQ.value("itemsrc_comments").toString());
    _manufName->setCode(itemsrcQ.value("itemsrc_manuf_name").toString());
    _manufItemNumber->setText(itemsrcQ.value("itemsrc_manuf_item_number").toString());
    _manufItemDescrip->setText(itemsrcQ.value("itemsrc_manuf_item_descrip").toString());

    if (_metrics->value("Application") == "Standard")
    {
      if (_contract->id() > 0)
      {
        _contractedQty->setDouble(itemsrcQ.value("itemsrc_contrct_min").toDouble());
        _contractedQty->setDisabled(false);
        _contractedQtyLit->setDisabled(false);
      }
      else
      {
        _contractedQty->setDouble(0.00);
        _contractedQty->setDisabled(true);
        _contractedQtyLit->setDisabled(true);
      }
    }

    sFillPriceList();
  }
  else if (itemsrcQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemsrcQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSource::sRejected()
{
  XSqlQuery itemRejected;
  if (_new)
  {
    itemRejected.prepare( "DELETE FROM itemsrc "
               "WHERE (itemsrc_id=:itemsrc_id);" );
    itemRejected.bindValue(":itemsrc_id", _itemsrcid);
    itemRejected.exec();
    if (itemRejected.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemRejected.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void itemSource::sVendorChanged( int pId )
{
  XSqlQuery vendorChanged;
  vendorChanged.prepare("SELECT vend_curr_id "
                        "FROM vendinfo "
                        "WHERE (vend_id = :vend_id)");
  vendorChanged.bindValue(":vend_id", pId);
  vendorChanged.exec();
  if (vendorChanged.first())
  {
    _vendorCurrency->setId(vendorChanged.value("vend_curr_id").toInt());
  }
  else if (vendorChanged.lastError().type() != QSqlError::NoError)
  {
    systemError(this, vendorChanged.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  vendorChanged.prepare("SELECT contrct_id, contrct_number "
                        "FROM contrct "
                        "WHERE (contrct_vend_id = :vend_id)");
  vendorChanged.bindValue(":vend_id", pId);
  vendorChanged.exec();
  _contract->populate(vendorChanged);
  if (vendorChanged.lastError().type() != QSqlError::NoError)
  {
    systemError(this, vendorChanged.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSource::sContractChanged( int pId )
{
  XSqlQuery contractChanged;
  contractChanged.prepare("SELECT contrct_effective, contrct_expires "
                          "FROM contrct "
                          "WHERE (contrct_id = :contrct_id);");
  contractChanged.bindValue(":contrct_id", pId);
  contractChanged.exec();
  if (contractChanged.first())
  {
    _dates->setStartDate(contractChanged.value("contrct_effective").toDate());
    _dates->setEndDate(contractChanged.value("contrct_expires").toDate());
    _dates->setEnabled(false);
    _contractedQty->setDisabled(false);
    _contractedQtyLit->setDisabled(false);
  }
  else if (contractChanged.lastError().type() != QSqlError::NoError)
  {
    systemError(this, contractChanged.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {   
    _dates->setEnabled(true);
    _contractedQty->setDisabled(true);
    _contractedQtyLit->setDisabled(true);
  }
}
