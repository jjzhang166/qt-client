/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "contract.h"

#include <QAction>
#include <QCloseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "crmacctcluster.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include <metasql.h>
#include <parameter.h>
#include "mqlutil.h"
#include "itemSource.h"
#include "purchaseOrder.h"
#include "enterPoReceipt.h"
#include "enterPoReturn.h"
#include <openreports.h>

contract::contract(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close,              SIGNAL(clicked()), this, SLOT(sRejected()));
  connect(_save,               SIGNAL(clicked()), this, SLOT(sSaveClicked()));
  connect(_newItemSrc,         SIGNAL(clicked()), this, SLOT(sNewItemSrc()));
  connect(_newPo,              SIGNAL(clicked()), this, SLOT(sNewPo()));
  connect(_editPo,             SIGNAL(clicked()), this, SLOT(sEditPo()));
  connect(_viewPo,             SIGNAL(clicked()), this, SLOT(sViewPo()));
  connect(_deletePo,           SIGNAL(clicked()), this, SLOT(sDeletePo()));
  connect(_releasePo,          SIGNAL(clicked()), this, SLOT(sReleasePo()));
  connect(_newRcpt,            SIGNAL(clicked()), this, SLOT(sNewRcpt()));
  connect(_newRtrn,            SIGNAL(clicked()), this, SLOT(sNewRtrn()));
  connect(_print,              SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemSource,         SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_itemSource,         SIGNAL(itemClicked(XTreeWidgetItem *, int)), this, SLOT(sHandleButtons(XTreeWidgetItem *, int)));
  connect(_itemSource,         SIGNAL(itemSelected(int)), this, SLOT(sEditItemSrc()));

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Expires"));

  if (_metrics->value("Application") == "Standard")
  {
    _itemSource->addColumn(tr("Item Number"),         _itemColumn, Qt::AlignLeft,   true, "item_number");
    _itemSource->addColumn(tr("Description"),                  -1, Qt::AlignLeft,   true, "item_descrip1");
    _itemSource->addColumn(tr("Vendor Item Number"),  _itemColumn, Qt::AlignLeft,   true, "itemsrc_vend_item_number");
    _itemSource->addColumn(tr("Vendor UOM"),           _uomColumn, Qt::AlignLeft,   true, "itemsrc_vend_uom");
    _itemSource->addColumn(tr("PO Number/Oper"),     _orderColumn, Qt::AlignLeft,   true, "poitem_ordnumber");
    _itemSource->addColumn(tr("Contracted"),           _qtyColumn, Qt::AlignRight,  true, "itemsrc_contrct_min");
    _itemSource->addColumn(tr("Unreleased"),           _qtyColumn, Qt::AlignRight,  true, "poitem_qty_unreleased" );
    _itemSource->addColumn(tr("Released"),             _qtyColumn, Qt::AlignRight,  true, "poitem_qty_ordered" );
    _itemSource->addColumn(tr("Received"),             _qtyColumn, Qt::AlignRight,  true, "poitem_qty_received");
    _itemSource->addColumn(tr("Returned"),             _qtyColumn, Qt::AlignRight,  true, "poitem_qty_returned" );
    _itemSource->addColumn(tr("Due Date"),            _dateColumn, Qt::AlignCenter, true, "poitem_duedate" );
    _itemSource->addColumn(tr("Status/User"),         _userColumn, Qt::AlignCenter, true, "poitem_status" );
  } 
  else
	_tab->removeTab(_tab->indexOf(_itemSrcTab));

  _captive = false;
  _new = false;
}

contract::~contract()
{
    // no need to delete child widgets, Qt does it all for us
}

void contract::languageChange()
{
    retranslateUi(this);
}

enum SetResponse contract::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("contrct_id", &valid);
  if (valid)
  {
    _contrctid = param.toInt();
    _documents->setId(_contrctid);
    populate();
  }

  param = pParams.value("vend_id", &valid);
  if (valid)
  {
    _vendor->setId(param.toInt());
    _vendor->setEnabled(false);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _new = true;

      _newItemSrc->setEnabled(true);
      _newPo->setEnabled(false);
      _editPo->setEnabled(false);
      _viewPo->setEnabled(false);
      _deletePo->setEnabled(false);
      _releasePo->setEnabled(false);
      _newRcpt->setEnabled(false);
      _newRtrn->setEnabled(false);

      itemet.exec("SELECT NEXTVAL('contrct_contrct_id_seq') AS contrct_id;");
      if (itemet.first())
      {
        _contrctid = itemet.value("contrct_id").toInt();
        _documents->setId(_contrctid);
      }
      else if (itemet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, itemet.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      _captive = true;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _vendor->setEnabled(false);
      _newItemSrc->setEnabled(true);
      _newPo->setEnabled(false);
      _editPo->setEnabled(false);
      _viewPo->setEnabled(false);
      _deletePo->setEnabled(false);
      _releasePo->setEnabled(false);
      _newRcpt->setEnabled(false);
      _newRtrn->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _vendor->setEnabled(false);
      _dates->setEnabled(false);
      _number->setEnabled(false);
      _descrip->setEnabled(false);
//      _documents->setReadOnly(true);
      _newItemSrc->setEnabled(true);
      _newPo->setEnabled(false);
      _editPo->setEnabled(false);
      _viewPo->setEnabled(false);
      _deletePo->setEnabled(false);
      _releasePo->setEnabled(false);
      _newRcpt->setEnabled(false);
      _newRtrn->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
    if (param.toString() == "copy")
    {
      _mode = cCopy;
      _new = true;
      _captive = true;
//      int contrctidold = _contrctid;

      itemet.exec("SELECT NEXTVAL('contrct_contrct_id_seq') AS contrct_id;");
      if (itemet.first())
        _contrctid = itemet.value("contrct_id").toInt();
      else if (itemet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, itemet.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      
      _dates->setStartDate(omfgThis->dbDate());

      sSave();
    }
  }

  return NoError;
}

void contract::sSaveClicked()
{
  _captive = false;
  if (sSave())
  {
    omfgThis->sContractsUpdated(_contrctid, true);
    close();
  }
}

bool contract::sSave()
{
  XSqlQuery itemSave;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_vendor->isValid(), _vendor,
                          tr( "You must select a Vendor before you may save this Contract." ) )
         << GuiErrorCheck(_dates->endDate() < _dates->startDate(), _dates,
                          tr("The expiration date cannot be earlier than the effective date.") )
         << GuiErrorCheck(_number->isNull(), _number,
                          tr( "You must enter a Contract Number before you may save this Contract." ) )
         << GuiErrorCheck(_descrip->isNull(), _descrip,
                          tr( "You must enter a Description before you may save this Contract." ) )
     ;

  /* TODO - need this?
  itemSave.prepare( "SELECT count(*) AS numberOfOverlaps "
                    "FROM contrct "
                    "WHERE (contrct_vend_id = :contrct_vend_id)"
                    "  AND (contrct_id != :contrct_id)"
                    "  AND ( (contrct_effective BETWEEN :contrct_effective AND :contrct_expires OR"
                    "         contrct_expires BETWEEN :contrct_effective AND :contrct_expires)"
                    "   OR   (contrct_effective <= :contrct_effective AND"
                    "         contrct_expires   >= :contrct_expires) );" );
  itemSave.bindValue(":contrct_id", _contrctid);
  itemSave.bindValue(":contrct_vend_id", _vendor->id());
  itemSave.bindValue(":contrct_effective", _dates->startDate());
  itemSave.bindValue(":contrct_expires", _dates->endDate());
  itemSave.exec();
  if (itemSave.first())
  {
    if (itemSave.value("numberOfOverlaps").toInt() > 0)
    {
      errors << GuiErrorCheck(true, _dates,
                              tr("The date range overlaps with another date range.\n"
                                 "Please check the values of these dates."));
    }
  }
  else if (itemSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  */

  if(_mode == cNew || _mode == cCopy)
  {
    itemSave.prepare( "SELECT contrct_id "
                      "  FROM contrct "
                      " WHERE ((contrct_vend_id=:vend_id) "
                      "   AND (contrct_number=:contrct_number));" );
    itemSave.bindValue(":vend_id", _vendor->id());
    itemSave.bindValue(":contrct_number", _number->text());
    itemSave.exec();
    if(itemSave.first())
    {
      errors << GuiErrorCheck(true, _vendor,
                              tr("A Contract already exists for the Vendor,\n"
                                 "Contract Number you have specified."));
    }
    else if (itemSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }
  
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Contract"), errors))
    return false;

  if (_mode == cNew || _mode == cCopy)
    itemSave.prepare( "INSERT INTO contrct "
               "( contrct_id, contrct_vend_id,"
               "  contrct_number, contrct_descrip,"
               "  contrct_effective, contrct_expires,"
               "  contrct_note ) "
               "VALUES "
               "( :contrct_id, :contrct_vend_id,"
               "  :contrct_number, :contrct_descrip,"
               "  :contrct_effective, :contrct_expires,"
               "  :contrct_note );" );
  if (_mode == cEdit)
    itemSave.prepare( "UPDATE contrct "
               "SET contrct_number=:contrct_number,"
               "    contrct_descrip=:contrct_descrip,"
               "    contrct_effective=:contrct_effective,"
               "    contrct_expires=:contrct_expires,"
               "    contrct_note=:contrct_note "
               "WHERE (contrct_id=:contrct_id);" );

  itemSave.bindValue(":contrct_id", _contrctid);
  itemSave.bindValue(":contrct_vend_id", _vendor->id());
  itemSave.bindValue(":contrct_effective", _dates->startDate());
  itemSave.bindValue(":contrct_expires", _dates->endDate());
  itemSave.bindValue(":contrct_number", _number->text());
  itemSave.bindValue(":contrct_descrip", _descrip->text());
  itemSave.bindValue(":contrct_note", _notes->toPlainText());
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
    _captive = false;
  }
    
  return true;
}

void contract::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem * pSelected )
{
  QAction *menuItem;
  QString oper;
  QString stat;

  oper = pSelected->text(_itemSource->column("poitem_ordnumber"));
  stat = pSelected->text(_itemSource->column("poitem_status"));

  if (!((oper == "Receipt") || (oper == "Return")) && (oper > " "))
  {
     if (_mode == cNew || _mode == cEdit)
     {
       menuItem = pMenu->addAction(tr("Edit Purchase Order..."), this, SLOT(sEditPo()));
       menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
     }

     menuItem = pMenu->addAction(tr("View Purchase Order..."), this, SLOT(sViewPo()));
     menuItem->setEnabled(_privileges->check("ViewPurchaseOrders") || _privileges->check("MaintainPurchaseOrders"));

     if (stat == "Unreleased" && _mode == cEdit)
     {
       menuItem = pMenu->addAction(tr("Delete Purchase Order..."), this, SLOT(sDeletePo()));
       menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

       menuItem = pMenu->addAction(tr("Release Purchase Order..."), this, SLOT(sReleasePo()));
       menuItem->setEnabled(_privileges->check("ReleasePurchaseOrders"));
     }

     pMenu->addSeparator();
  }

  if (_itemSource->altId() > 0)
  {
    if (_mode == cNew || _mode == cEdit)
    {
       menuItem = pMenu->addAction(tr("New Receipt..."), this, SLOT(sNewRcpt()));
       menuItem->setEnabled(_privileges->check("EnterReceipts"));

       menuItem = pMenu->addAction(tr("New Return..."), this, SLOT(sNewRtrn()));
       menuItem->setEnabled(_privileges->check("EnterReturns"));
    }
  }
}

void contract::sFillList()
{
  XSqlQuery itemsrcFillList;
  MetaSQLQuery mql = mqlLoad("contract", "itemsources");

  ParameterList params;
  params.append("contrct_id", _contrctid);

  itemsrcFillList = mql.toQuery(params);
  _itemSource->populate(itemsrcFillList, true);
  if (itemsrcFillList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemsrcFillList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void contract::sNewItemSrc()
{
  _captive = true;
  if (!sSave())
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("contrct_id", _contrctid);
  params.append("vend_id", _vendor->id());

  itemSource newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
  _vendor->setReadOnly(true);
}

void contract::sEditItemSrc()
{
  _captive = true;
  if (!sSave())
    return;
  
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", _itemSource->id());
  
  itemSource newdlg(this, "", true);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contract::sNewPo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("vend_id", _vendor->id());
  params.append("itemsrc_id", _itemSource->id());

//  XSqlQuery itemsite;

//  itemsite.prepare( "SELECT itemsite_id "
//                      "  FROM itemsite JOIN item    ON (item_id         = itemsite_item_id) "
//                      "                JOIN itemsrc ON (itemsrc_item_id = item_id) "
//                      "WHERE (itemsrc_id =:itemsite_id) LIMIT 1;" );
//  itemsite.bindValue(":itemsite_id", _itemSource->id());
//  itemsite.exec();
//  if (itemsite.first())
//  {
//    params.append("itemsite_id", itemsite.value("itemsite_id").toInt());
//  }
//  else
//  {
//    QMessageBox::warning(omfgThis, tr("Cannot Create P/O"),
//                         tr("<p>A Purchase Order cannot be automatically "
//                            "created for this Item as there are no Item "
//                            "Sites for it.  You must create one or "
//                            "more Item Sites for this Item before "
//                            "the application can automatically create "
//                            "Purchase Orders for it." ) );
//    return;
//  }

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

  sFillList();
}

void contract::sEditPo()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", _itemSource->altId());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

  sFillList();
}

void contract::sViewPo()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", _itemSource->altId());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

  sFillList();
}

void contract::sDeletePo()
{
  if (_mode == cView)
  {
    QMessageBox::warning(omfgThis, tr("View mode"),
                         tr("Not allowed under View mode.") );
    return;
  }

  if (QMessageBox::question(this, tr("Delete Purchase Order"),
                                  tr("<p>Are you sure you want to delete the "
                                     "selected Purchase Order?"),
         QMessageBox::Yes,
         QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;
 
  XSqlQuery unpostedDelete;

  unpostedDelete.prepare( "SELECT deletePo(:pohead_id) AS result;");
  unpostedDelete.bindValue(":pohead_id", _itemSource->altId());
  unpostedDelete.exec();
  if (unpostedDelete.first() && ! unpostedDelete.value("result").toBool())
       systemError(this, tr("<p>Only Unposted Purchase Orders may be "
                            "deleted. Check the status of Purchase Order "
                            "%1. If it is 'U' then contact your system "
                            "Administrator.").arg(_itemSource->currentItem()->rawValue("poitem_ordnumber").toString()),
                   __FILE__, __LINE__);
  else if (unpostedDelete.lastError().type() != QSqlError::NoError)
    systemError(this, unpostedDelete.lastError().databaseText(), __FILE__, __LINE__);

  sFillList();
}

void contract::sReleasePo()
{
  if (_mode == cView)
  {
    QMessageBox::warning(omfgThis, tr("View mode"),
                         tr("Not allowed under View mode.") );
    return;
  }
  
  if (QMessageBox::question(this, tr("Release Purchase Order"),
                            tr("<p>Are you sure you want to release the "
                               "selected Purchase Order?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;
  
  XSqlQuery unpostedRelease;
  
  unpostedRelease.prepare( "SELECT releasePurchaseOrder(:pohead_id) AS result;");
  unpostedRelease.bindValue(":pohead_id", _itemSource->altId());
  unpostedRelease.exec();
  if (unpostedRelease.first() && (unpostedRelease.value("result").toInt() < 0))
    systemError(this, tr("<p>Only Unrelease Purchase Orders may be "
                         "released. Check the status of Purchase Order "
                         "%1. If it is 'U' then contact your system "
                         "Administrator.").arg(_itemSource->currentItem()->rawValue("poitem_ordnumber").toString()),
                __FILE__, __LINE__);
  else if (unpostedRelease.lastError().type() != QSqlError::NoError)
    systemError(this, unpostedRelease.lastError().databaseText(), __FILE__, __LINE__);
  
  sFillList();
}

void contract::sNewRcpt()
{
  ParameterList params;
  params.append("pohead_id", _itemSource->altId());

  enterPoReceipt *newdlg = new enterPoReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

  sFillList();
}

void contract::sNewRtrn()
{
  ParameterList params;
  params.append("pohead_id", _itemSource->altId());

  enterPoReturn *newdlg = new enterPoReturn();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

  sFillList();
}

void contract::sPrint()
{
  ParameterList params;
  params.append("contrct_id", _contrctid);
  orReport report("ContractActivity", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void contract::sHandleButtons(XTreeWidgetItem *pItem, int pCol)
{
  QString oper;
  QString stat;

  if (_mode != cNew)
  {
    oper = pItem->text(_itemSource->column("poitem_ordnumber"));
    stat = pItem->text(_itemSource->column("poitem_status"));

    if (!(oper > " "))
      _newPo->setEnabled(true);
    else
      _newPo->setEnabled(false);

      if (!((oper == "Receipt") || (oper == "Return")) && (oper > " "))
      {
        if (_mode == cNew || _mode == cEdit)
          _editPo->setEnabled(_privileges->check("MaintainPurchaseOrders"));
        else
          _editPo->setEnabled(false);

        _viewPo->setEnabled(_privileges->check("ViewPurchaseOrders") || _privileges->check("MaintainPurchaseOrders"));

        if (stat == "Unreleased" && _mode == cEdit)
        {
          _deletePo->setEnabled(_privileges->check("MaintainPurchaseOrders"));
          _releasePo->setEnabled(_privileges->check("ReleasePurchaseOrders"));
        }
        else
        {
          _deletePo->setEnabled(false);
          _releasePo->setEnabled(false);
        }
      }
      else
      {
        _editPo->setEnabled(false);
        _viewPo->setEnabled(false);
        _deletePo->setEnabled(false);
        _releasePo->setEnabled(false);
      }

      if (pItem->altId() > 0)
      {
        if (_mode == cNew || _mode == cEdit)
        {
          _newRcpt->setEnabled(_privileges->check("EnterReceipts"));
          _newRtrn->setEnabled(_privileges->check("EnterReturns"));
        }
        else
        {
          _newRcpt->setEnabled(false);
          _newRtrn->setEnabled(false);
        }
      }
      else
      {
        _newRcpt->setEnabled(false);
        _newRtrn->setEnabled(false);
      }
    }

}

void contract::populate()
{
  XSqlQuery contrctQ;
  contrctQ.prepare( "SELECT * "
                    "FROM contrct "
                    "WHERE (contrct_id=:contrct_id);" );
  contrctQ.bindValue(":contrct_id", _contrctid);
  contrctQ.exec();
  if (contrctQ.first())
  {
    _vendor->setId(contrctQ.value("contrct_vend_id").toInt());
    _dates->setStartDate(contrctQ.value("contrct_effective").toDate());
    _dates->setEndDate(contrctQ.value("contrct_expires").toDate());
    _number->setText(contrctQ.value("contrct_number").toString());
    _descrip->setText(contrctQ.value("contrct_descrip").toString());
    _notes->setText(contrctQ.value("contrct_note").toString());
	sFillList();
  }
  else if (contrctQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, contrctQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void contract::sRejected()
{
  XSqlQuery itemRejected;
  if (_new && (_itemSource->topLevelItemCount() == 0))
  {
    itemRejected.prepare( "DELETE FROM contrct "
               "WHERE (contrct_id=:contrct_id);" );
    itemRejected.bindValue(":contrct_id", _contrctid);
    itemRejected.exec();
    if (itemRejected.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemRejected.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
