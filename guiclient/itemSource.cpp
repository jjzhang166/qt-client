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

#include "itemSource.h"

#include <QCloseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "crmacctcluster.h"
#include "itemSourcePrice.h"
#include "xcombobox.h"

itemSource::itemSource(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_vendorList, SIGNAL(clicked()), this, SLOT(sVendorList()));
  connect(_vendor, SIGNAL(nameChanged(const QString&)), _vendorName, SLOT(setText(const QString&)));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_vendorCurrency, SIGNAL(newID(int)), this, SLOT(sFillPriceList()));
  connect(_vendor, SIGNAL(newId(int)), this, SLOT(sVendorChanged(int)));

  _item->setType(ItemLineEdit::cGeneralPurchased);

#ifndef Q_WS_MAC
  _vendorList->setMaximumWidth(25);
#endif

  QString base;
  q.exec("SELECT currConcat(baseCurrID()) AS base;");
  if (q.first())
    base = q.value("base").toString();
  else
    base = tr("Base");

  _itemsrcp->addColumn(tr("Qty Break"),                   _qtyColumn, Qt::AlignRight,true, "itemsrcp_qtybreak");
  _itemsrcp->addColumn(tr("Unit Price"),                          -1, Qt::AlignRight,true, "itemsrcp_price");
  _itemsrcp->addColumn(tr("Currency"),               _currencyColumn, Qt::AlignLeft, true, "currabbr");
  _itemsrcp->addColumn(tr("Unit Price\n(%1)").arg(base),_moneyColumn, Qt::AlignRight,true, "itemsrcp_price_base");
  if (omfgThis->singleCurrency())
  {
    _itemsrcp->hideColumn(1);
    _itemsrcp->hideColumn(2);
    _itemsrcp->headerItem()->setText(3, tr("Unit Price"));
  }

  _invVendorUOMRatio->setValidator(omfgThis->ratioVal());
  _minOrderQty->setValidator(omfgThis->qtyVal());
  _multOrderQty->setValidator(omfgThis->qtyVal());

  _vendorCurrency->setType(XComboBox::Currencies);
  _vendorCurrency->setLabel(_vendorCurrencyLit);
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
  QVariant param;
  bool     valid;

  param = pParams.value("itemsrc_id", &valid);
  if (valid)
  {
    _itemsrcid = param.toInt();
    populate();
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setEnabled(FALSE);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('itemsrc_itemsrc_id_seq') AS _itemsrc_id;");
      if (q.first())
        _itemsrcid = q.value("_itemsrc_id").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      connect(_itemsrcp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_itemsrcp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _item->setReadOnly(TRUE);
      _vendor->setEnabled(FALSE);
      _vendorList->hide();

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _active->setEnabled(FALSE);
      _vendor->setEnabled(FALSE);
      _vendorList->hide();
      _vendorItemNumber->setEnabled(FALSE);
      _vendorItemDescrip->setEnabled(FALSE);
      _vendorUOM->setEnabled(FALSE);
      _invVendorUOMRatio->setEnabled(FALSE);
      _vendorRanking->setEnabled(FALSE);
      _minOrderQty->setEnabled(FALSE);
      _multOrderQty->setEnabled(FALSE);
      _leadTime->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _add->setEnabled(FALSE);
      _delete->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void itemSource::sSave()
{
  if (!_item->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Item Source"),
                           tr( "You must select an Item that this Item Source represents\n"
                               "before you may save this Item Source." ) );
    _item->setFocus();
    return;
  }

  if (!_vendor->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Item Source"),
                           tr( "You must select this Vendor that this Item Source is sold by\n"
                               "before you may save this Item Source." ) );
    _item->setFocus();
    return;
  }

  if(_mode == cNew)
  {
    q.prepare( "SELECT itemsrc_id "
               "  FROM itemsrc "
               " WHERE ((itemsrc_item_id=:item_id) "
               "   AND (itemsrc_vend_id=:vend_id) ) ");
    q.bindValue(":item_id", _item->id());
    q.bindValue(":vend_id", _vendor->id());
    q.exec();
    if(q.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Item Source"),
                            tr("An Item Source already exists for the Item Number and Vendor you have specified.\n"));
      return;
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_vendorUOM->text().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Item Source"),
                          tr( "You must indicate the Unit of Measure that this Item Source is sold in\n"
                               "before you may save this Item Source." ) );
    _vendorUOM->setFocus();
    return;
  }

  if (_invVendorUOMRatio->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Cannot Save Item Source"),
                          tr( "You must indicate the Ratio of Inventory to Vendor Unit of Measures\n"
                               "before you may save this Item Source." ) );
    _invVendorUOMRatio->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO itemsrc "
               "( itemsrc_id, itemsrc_item_id, itemsrc_active, itemsrc_vend_id,"
               "  itemsrc_vend_item_number, itemsrc_vend_item_descrip,"
               "  itemsrc_vend_uom, itemsrc_invvendoruomratio,"
               "  itemsrc_minordqty, itemsrc_multordqty,"
               "  itemsrc_leadtime, itemsrc_ranking,"
               "  itemsrc_comments ) "
               "VALUES "
               "( :itemsrc_id, :itemsrc_item_id, :itemsrc_active, :itemsrc_vend_id,"
               "  :itemsrc_vend_item_number, :itemsrc_vend_item_descrip,"
               "  :itemsrc_vend_uom, :itemsrc_invvendoruomratio,"
               "  :itemsrc_minordqty, :itemsrc_multordqty,"
               "  :itemsrc_leadtime, :itemsrc_ranking,"
               "  :itemsrc_comments );" );
  if (_mode == cEdit)
    q.prepare( "UPDATE itemsrc "
               "SET itemsrc_active=:itemsrc_active,"
               "    itemsrc_vend_item_number=:itemsrc_vend_item_number,"
               "    itemsrc_vend_item_descrip=:itemsrc_vend_item_descrip,"
               "    itemsrc_vend_uom=:itemsrc_vend_uom,"
               "    itemsrc_invvendoruomratio=:itemsrc_invvendoruomratio,"
               "    itemsrc_minordqty=:itemsrc_minordqty, itemsrc_multordqty=:itemsrc_multordqty,"
               "    itemsrc_leadtime=:itemsrc_leadtime, itemsrc_ranking=:itemsrc_ranking,"
               "    itemsrc_comments=:itemsrc_comments "
               "WHERE (itemsrc_id=:itemsrc_id);" );

  q.bindValue(":itemsrc_id", _itemsrcid);
  q.bindValue(":itemsrc_item_id", _item->id());
  q.bindValue(":itemsrc_active", QVariant(_active->isChecked(), 0));
  q.bindValue(":itemsrc_vend_id", _vendor->id());
  q.bindValue(":itemsrc_vend_item_number", _vendorItemNumber->text());
  q.bindValue(":itemsrc_vend_item_descrip", _vendorItemDescrip->text());
  q.bindValue(":itemsrc_vend_uom", _vendorUOM->text().stripWhiteSpace());
  q.bindValue(":itemsrc_invvendoruomratio", _invVendorUOMRatio->toDouble());
  q.bindValue(":itemsrc_minordqty", _minOrderQty->toDouble());
  q.bindValue(":itemsrc_multordqty", _multOrderQty->toDouble());
  q.bindValue(":itemsrc_leadtime", _leadTime->text().toInt());
  q.bindValue(":itemsrc_ranking", _vendorRanking->value());
  q.bindValue(":itemsrc_comments", _notes->text().stripWhiteSpace());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_itemsrcid);
}

void itemSource::sClose()
{
  closeEvent(NULL);
  reject();
}

void itemSource::sAdd()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsrc_id", _itemsrcid);
  params.append("curr_id", _vendorCurrency->id());

  itemSourcePrice newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillPriceList();
}

void itemSource::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrcp_id", _itemsrcp->id());

  itemSourcePrice newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillPriceList();
}

void itemSource::sDelete()
{
//  Make sure the user is sure
  if (  QMessageBox::warning( this, tr("Delete Item Source Price"),
                              tr("Are you sure you want to delete this Item Source price?"),
                              tr("&Delete"), tr("&Cancel"), 0, 0, 1)  == 0  )
  {
    q.prepare( "DELETE FROM itemsrcp "
               "WHERE (itemsrcp_id=:itemsrcp_id);" );
    q.bindValue(":itemsrcp_id", _itemsrcp->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillPriceList();
  }
}

void itemSource::sPopulateMenu(QMenu *pMenu)
{
  int intMenuItem;

  intMenuItem = pMenu->insertItem("Edit Item Source Price...", this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemSources"))
    pMenu->setItemEnabled(intMenuItem, FALSE);

  intMenuItem = pMenu->insertItem("Delete Item Source Price...", this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainItemSources"))
    pMenu->setItemEnabled(intMenuItem, FALSE);
}


void itemSource::sVendorList()
{
  CRMAcctList * newdlg = new CRMAcctList(this);
  newdlg->setSubtype(CRMAcctLineEdit::Vend);
  newdlg->setId(_vendor->id());
  _vendor->setId(newdlg->exec());
}

void itemSource::sFillPriceList()
{
  XSqlQuery priceq;
  priceq.prepare("SELECT *,"
                 " currConcat(itemsrcp_curr_id) AS currabbr, "
                 " currToBase(itemsrcp_curr_id, itemsrcp_price,"
                 "            itemsrcp_updated) AS itemsrcp_price_base,"
                 " 'qty' AS itemsrcp_qtybreak_xtnumericrole,"
                 " 'purchprice' AS itemsrcp_price_xtnumericrole,"
                 " 'purchprice' AS itemsrcp_price_base_xtnumericrole "
                 "FROM itemsrcp "
                 "WHERE (itemsrcp_itemsrc_id=:itemsrc_id) "
                 "ORDER BY itemsrcp_qtybreak;" );
  priceq.bindValue(":itemsrc_id", _itemsrcid);
  priceq.exec();
  _itemsrcp->populate(priceq);
  if (priceq.lastError().type() != QSqlError::None)
  {
    systemError(this, priceq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSource::populate()
{
  XSqlQuery itemsrcQ;
  itemsrcQ.prepare( "SELECT itemsrc_item_id, itemsrc_active, itemsrc_vend_id,"
             "       itemsrc_vend_item_number, itemsrc_vend_item_descrip,"
             "       itemsrc_vend_uom, itemsrc_invvendoruomratio,"
             "       itemsrc_minordqty, itemsrc_multordqty,"
             "       itemsrc_ranking, itemsrc_leadtime, itemsrc_comments "
             "FROM itemsrc "
             "WHERE (itemsrc_id=:itemsrc_id);" );
  itemsrcQ.bindValue(":itemsrc_id", _itemsrcid);
  itemsrcQ.exec();
  if (itemsrcQ.first())
  {
    _item->setId(itemsrcQ.value("itemsrc_item_id").toInt());
    _active->setChecked(itemsrcQ.value("itemsrc_active").toBool());
    _vendor->setId(itemsrcQ.value("itemsrc_vend_id").toInt());
    _vendorItemNumber->setText(itemsrcQ.value("itemsrc_vend_item_number").toString());
    _vendorItemDescrip->setText(itemsrcQ.value("itemsrc_vend_item_descrip").toString());
    _vendorUOM->setText(itemsrcQ.value("itemsrc_vend_uom").toString());
    _invVendorUOMRatio->setDouble(itemsrcQ.value("itemsrc_invvendoruomratio").toDouble());
    _minOrderQty->setDouble(itemsrcQ.value("itemsrc_minordqty").toDouble());
    _multOrderQty->setDouble(itemsrcQ.value("itemsrc_multordqty").toDouble());
    _vendorRanking->setValue(itemsrcQ.value("itemsrc_ranking").toInt());
    _leadTime->setValue(itemsrcQ.value("itemsrc_leadtime").toInt());
    _notes->setText(itemsrcQ.value("itemsrc_comments").toString());
    sFillPriceList();
  }
  else if (itemsrcQ.lastError().type() != QSqlError::None)
  {
    systemError(this, itemsrcQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSource::closeEvent(QCloseEvent *pEvent)
{
  if ( (_mode == cNew) && (pEvent == NULL) )
  {
    q.prepare( "DELETE FROM itemsrcp "
               "WHERE (itemsrcp_itemsrc_id=:itemsrc_id);" );
    q.bindValue(":itemsrc_id", _itemsrcid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (pEvent != NULL)
    XDialog::closeEvent(pEvent);
}

void itemSource::sVendorChanged( int pId )
{
    q.prepare("SELECT vend_curr_id FROM vend WHERE vend_id = :vend_id;");
    q.bindValue(":vend_id", pId);
    q.exec();
    if (q.first())
	_vendorCurrency->setId(q.value("vend_curr_id").toInt());
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
}
