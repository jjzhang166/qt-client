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

#include "purchaseOrder.h"

#include <QCloseEvent>
#include <QList>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QValidator>
#include <QVariant>

#include "comment.h"
#include "deliverPurchaseOrder.h"
#include "itemSourceList.h"
#include "poitemTableModel.h"
#include "printPurchaseOrder.h"
#include "purchaseOrderItem.h"
#include "vendorAddressList.h"

#define cDelete 0x01
#define cClose  0x02

purchaseOrder::purchaseOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_freight, SIGNAL(valueChanged()), this, SLOT(sCalculateTotals()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_orderDate,	SIGNAL(newDate(QDate)), this, SLOT(sHandleOrderDate()));
  connect(_orderNumber, SIGNAL(lostFocus()), this, SLOT(sHandleOrderNumber()));
  connect(_orderNumber, SIGNAL(textChanged(const QString&)), this, SLOT(sSetUserOrderNumber()));
  connect(_poCurrency,	SIGNAL(newID(int)),	this, SLOT(sCurrencyChanged()));
  connect(_poitem, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(sHandleDeleteButton()));
  connect(_purchaseOrderInformation, SIGNAL(currentChanged(int)), this, SLOT(sTabChanged(int)));
  connect(_qecurrency,	SIGNAL(newID(int)),	this, SLOT(sCurrencyChanged()));
  connect(_qedelete,	SIGNAL(clicked()),	this, SLOT(sQEDelete()));
  connect(_qesave,	SIGNAL(clicked()),	this, SLOT(sQESave()));
  connect(_save,        SIGNAL(clicked()),      this, SLOT(sSave()));
  connect(_tax,         SIGNAL(valueChanged()), this, SLOT(sCalculateTotals()));
  connect(_vendaddrList, SIGNAL(clicked()),     this, SLOT(sVendaddrList()));
  connect(_vendor,       SIGNAL(newId(int)),    this, SLOT(sHandleVendor(int)));

#ifndef Q_WS_MAC
  _vendaddrList->setMaximumWidth(25);
#endif

  _poCurrency->setLabel(_poCurrencyLit);

  _poitem->addColumn(tr("#"),           _whsColumn,    Qt::AlignCenter,true, "poitem_linenumber");
  _poitem->addColumn(tr("Status"),      _statusColumn, Qt::AlignCenter,true, "poitemstatus");
  _poitem->addColumn(tr("Item"),        _itemColumn,   Qt::AlignLeft,  true, "item_number");
  _poitem->addColumn(tr("Description"), -1,            Qt::AlignLeft,  true, "item_descrip");
  _poitem->addColumn(tr("Due Date"),    _dateColumn,   Qt::AlignRight, true,"poitem_duedate");
  _poitem->addColumn(tr("Ordered"),     _qtyColumn,    Qt::AlignRight, true, "poitem_qty_ordered");
  _poitem->addColumn(tr("Unit Price"),  _priceColumn,  Qt::AlignRight, true, "poitem_unitprice");
  _poitem->addColumn(tr("Ext. Price"),  _moneyColumn,  Qt::AlignRight, true, "extprice");

  _qeitem = new PoitemTableModel(this);
  _qeitemView->setModel(_qeitem);
  _qeitem->select();

  q.exec("SELECT usr_id "
         "FROM usr "
         "WHERE (usr_username=CURRENT_USER);");
  if(q.first())
    _agent->setId(q.value("usr_id").toInt());

  _userOrderNumber = FALSE;
  _printed	   = false;

  setPoheadid(-1);

  _cachedTabIndex = 0;

  _mode = cView;	// initialize _mode to something safe - bug 3768
 
  _printPO->setChecked(_metrics->boolean("DefaultPrintPOOnSave"));

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _receivingWhseLit->hide();
    _warehouse->hide();
  }
}

void purchaseOrder::setPoheadid(const int pId)
{
  _poheadid = pId;
  _qeitem->setHeadId(pId);
}

purchaseOrder::~purchaseOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void purchaseOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse purchaseOrder::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  int      itemsiteid;
  int      parentwo = -1;
  int      parentso = -1;
  int      prjid = -1;
  double   qty;
  QDate    dueDate;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    setPoheadid(param.toInt());
    populate();
  }
  else
    setPoheadid(-1);

  int _prid = -1;
  param = pParams.value("pr_id", &valid);
  if (valid)
    _prid = param.toInt();

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    itemsiteid = param.toInt();
  else
    itemsiteid = -1;

  param = pParams.value("qty", &valid);
  if (valid)
    qty = param.toDouble();
  else
    qty = 0.0;

  param = pParams.value("dueDate", &valid);
  if (valid)
    dueDate = param.toDate();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if ( (param.toString() == "new") || (param.toString() == "releasePr") )
    {
      _mode = cNew;

      if (param.toString() == "releasePr")
      {
        q.prepare( "SELECT pr_itemsite_id, pr_qtyreq, pr_duedate,"
                   "       CASE WHEN(pr_order_type='W') THEN"
                   "              COALESCE((SELECT womatl_wo_id FROM womatl WHERE womatl_id=pr_order_id),-1)"
                   "            ELSE -1"
                   "       END AS parentwo,"
                   "       CASE WHEN(pr_order_type='S') THEN pr_order_id"
                   "            ELSE -1"
                   "       END AS parentso,"
                   "       pr_prj_id "
                   "FROM pr "
                   "WHERE (pr_id=:pr_id);" );
        q.bindValue(":pr_id", _prid);
        q.exec();
        if (q.first())
        {
          itemsiteid = q.value("pr_itemsite_id").toInt();
          qty = q.value("pr_qtyreq").toDouble();
          dueDate = q.value("pr_duedate").toDate();
          parentwo = q.value("parentwo").toInt();
          parentso = q.value("parentso").toInt();
          prjid = q.value("pr_prj_id").toInt();
        }
        else
        {
          systemError(this, tr("A System Error occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__) );
          return UndefinedError;
        }
      }

      connect(_poitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_poitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_poitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_vendor, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
      //_new->setEnabled(TRUE);

      if (itemsiteid != -1)
      {
	      q.prepare( "SELECT itemsite_item_id, itemsrc_id "
                   "FROM itemsite, itemsrc "
                   "WHERE ( (itemsrc_item_id=itemsite_item_id)"
                   " AND (itemsite_id=:itemsite_id) ) "
                    "LIMIT 1;" );
        q.bindValue(":itemsite_id", itemsiteid);
        q.exec();
        if (!q.first())
        {
          QMessageBox::warning(omfgThis, tr("Cannot Create P/O"),
                               tr("<p>A Purchase Order cannot be automatically "
                                  "created for this Item as there are no Item "
                                  "Sources for it.  You must create one or "
                                  "more Item Sources for this Item before "
                                  "the application can automatically create "
                                  "Purchase Orders for it." ) );
          return UndefinedError;
        }

        ParameterList itemSourceParams;
        itemSourceParams.append("item_id", q.value("itemsite_item_id").toInt());
        itemSourceParams.append("qty", qty);
  
        itemSourceList newdlg(omfgThis, "", TRUE);
        newdlg.set(itemSourceParams);
        int itemsrcid = newdlg.exec();
        if (itemsrcid == XDialog::Rejected)
      	{
          deleteLater();
          return UndefinedError;
        }

        q.prepare( "SELECT itemsrc_vend_id "
                   "FROM itemsrc "
                   "WHERE (itemsrc_id=:itemsrc_id);" );
        q.bindValue(":itemsrc_id", itemsrcid);
        q.exec();
        if (!q.first())
        {
          systemError(this, tr("A System Error occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__) );
          return UndefinedError;
        }
        else
        {
          int vendid = q.value("itemsrc_vend_id").toInt();;

          q.prepare( "SELECT pohead_id "
                     "FROM pohead "
                     "WHERE ( (pohead_status='U')"
                     " AND (pohead_vend_id=:vend_id) ) "
                     "ORDER BY pohead_number "
                     "LIMIT 1;" );
          q.bindValue(":vend_id", vendid);
          q.exec();
          if (q.first())
          {
            if(QMessageBox::question( this, tr("An Open P/O Exists"),
                tr("An Open Purchase Order already exists for this Vendor.\n"
                   "Would you like to use this Purchase Order?\n"
                   "Click Yes to use the existing P/O otherwise a new one will be created."),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
            {
//  Use an existing pohead
              _mode = cEdit;

              setPoheadid(q.value("pohead_id").toInt());
              _orderNumber->setEnabled(FALSE);
              _orderDate->setEnabled(FALSE);
              _vendor->setReadOnly(TRUE);
              populate();
            }
            else
            {
              _vendor->setId(vendid);
              createHeader();
            }
          }
          else
          {
//  Create a new pohead for the chosen vend
            _vendor->setId(vendid);
            createHeader();
          }

//  Start to create the new Poitem
          ParameterList newItemParams;
          newItemParams.append("mode", "new");
          newItemParams.append("pohead_id", _poheadid);
          newItemParams.append("itemsite_id", itemsiteid);

          if (qty > 0.0)
            newItemParams.append("qty", qty);

          if (!dueDate.isNull())
            newItemParams.append("dueDate", dueDate);

          if (parentwo != -1)
            newItemParams.append("parentWo", parentwo);

          if (parentso != -1)
            newItemParams.append("parentSo", parentso);

          if (prjid != -1)
            newItemParams.append("prj_id", prjid);

          purchaseOrderItem poItem(this, "", TRUE);
          poItem.set(newItemParams);
          if (poItem.exec() != XDialog::Rejected)
          {
            if(_mode == cEdit)
            {
              // check for another open window
              QWidgetList list = omfgThis->windowList();
              for(int i = 0; i < list.size(); i++)
              {
                QWidget * w = list.at(i);
                if(w->isA("purchaseOrder") && w != this)
                {
                  purchaseOrder *other = (purchaseOrder*)w;
                  if(_poheadid == other->_poheadid)
                  {
                    if(_prid != -1 && cEdit == other->_mode)
                    {
                      q.prepare("SELECT deletePr(:pr_id) AS _result;");
                      q.bindValue(":pr_id", _prid);
                      q.exec();
                      omfgThis->sPurchaseRequestsUpdated();
                    }
                    other->sFillList();
                    other->setFocus();
                    return UndefinedError;
                  }
                }
              }
            }
            sFillList();
          }
          else
            _prid = -1;
        }
      }
      else
//  This is a new P/O without a chosen Itemsite Line Item
        createHeader();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _orderNumber->setEnabled(FALSE);
      _orderDate->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);
      _vendor->setReadOnly(TRUE);

      connect(_poitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_poitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _new->setEnabled(TRUE);

    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _orderNumber->setEnabled(FALSE);
      _orderDate->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);
      _agent->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _vendor->setReadOnly(TRUE);
      _shipVia->setEnabled(FALSE);
      _fob->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _freight->setEnabled(FALSE);
      _tax->setEnabled(FALSE);
      _vendaddrList->hide();
      _poCurrency->setEnabled(FALSE);
      _qeitemView->setEnabled(FALSE);
      _qesave->setEnabled(FALSE);
      _qedelete->setEnabled(FALSE);
      _qecurrency->setEnabled(FALSE);

      _delete->hide();
      _edit->setText(tr("&View"));
      _close->setText(tr("&Close"));
      _save->hide();

      connect(_poitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_poitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));

      _close->setFocus();
    }
  }

  if (_prid != -1 && cNew != _mode)
  {
    q.prepare("SELECT deletePr(:pr_id) AS _result;");
    q.bindValue(":pr_id", _prid);
    q.exec();
    _prid = -1;
    omfgThis->sPurchaseRequestsUpdated();
  }

  if(_prid != -1)
    _pridList.append(_prid);

  return NoError;
}

void purchaseOrder::createHeader()
{
//  Determine the new PO Number
  if ( (_metrics->value("PONumberGeneration") == "A") ||
       (_metrics->value("PONumberGeneration") == "O") )
  {
    populateOrderNumber();
    _vendor->setFocus();
  }
  else
    _orderNumber->setFocus();

  q.exec("SELECT NEXTVAL('pohead_pohead_id_seq') AS pohead_id;");
  if (q.first())
    setPoheadid(q.value("pohead_id").toInt());
  else
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
    return;
  }

  // need to set at least the _order date before the INSERT
  _comments->setId(_poheadid);
  _orderDate->setDate(omfgThis->dbDate(), true);
  _status->setText("U");
  _vendor->setType(__activeVendors);

  q.prepare( "INSERT INTO pohead "
             "( pohead_id, pohead_number, pohead_status,"
             "  pohead_agent_username, pohead_vend_id, "
	     "  pohead_orderdate, pohead_curr_id, pohead_saved) "
             "VALUES "
             "( :pohead_id, :pohead_number, 'U',"
             "  :pohead_agent_username, :pohead_vend_id, "
	     "  :pohead_orderdate, :pohead_curr_id, false );" );
  q.bindValue(":pohead_id", _poheadid);
  q.bindValue(":pohead_agent_username", _agent->currentText());
  q.bindValue(":pohead_number", _orderNumber->text().toInt());
  q.bindValue(":pohead_vend_id", _vendor->id());
  q.bindValue(":pohead_orderdate", _orderDate->date());
  q.bindValue(":pohead_curr_id", _poCurrency->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}

void purchaseOrder::populate()
{
  XSqlQuery po;
  po.prepare( "SELECT pohead_number, pohead_warehous_id, pohead_orderdate, "
	      "       pohead_status, pohead_printed, "
              "       pohead_shipvia, pohead_comments,"
              "       pohead_fob, pohead_terms_id, pohead_vend_id, pohead_prj_id,"
              "       pohead_tax,"
              "       pohead_freight,"
              "       pohead_agent_username,"
              "       vend_name, vend_address1, vend_address2, vend_address3,"
              "       vend_city, vend_state, vend_zip, vend_country,"
              "       COALESCE(vendaddr_id, -1) AS vendaddrid,"
              "       vendaddr_code, vendaddr_name, vendaddr_address1, vendaddr_address2,"
              "       vendaddr_address3, vendaddr_city, vendaddr_state, vendaddr_zipcode,"
              "       vendaddr_country, pohead_curr_id, vend_emailpodelivery "
              "FROM vend, pohead LEFT OUTER JOIN vendaddr ON (pohead_vendaddr_id=vendaddr_id) "
              "WHERE ( (pohead_vend_id=vend_id)"
              " AND (pohead_id=:pohead_id) );" );
  po.bindValue(":pohead_id", _poheadid);
  po.exec();
  if (po.first())
  {
    _orderNumber->setText(po.value("pohead_number"));
    _warehouse->setId(po.value("pohead_warehous_id").toInt());
    _orderDate->setDate(po.value("pohead_orderdate").toDate(), true);
    _agent->setText(po.value("pohead_agent_username").toString());
    _status->setText(po.value("pohead_status").toString());
    _printed = po.value("pohead_printed").toBool();
    _terms->setId(po.value("pohead_terms_id").toInt());
    _shipVia->setText(po.value("pohead_shipvia"));
    _fob->setText(po.value("pohead_fob"));
    _notes->setText(po.value("pohead_comments").toString());

    _vendaddrid = po.value("vendaddrid").toInt();
    if (_vendaddrid == -1)
    {
      _vendaddrCode->setText(tr("Main"));
      _vendaddrName->setText(po.value("vend_name"));
      _vendaddrAddr1->setText(po.value("vend_address1"));
      _vendaddrAddr2->setText(po.value("vend_address2"));
      _vendaddrAddr3->setText(po.value("vend_address3"));
      _vendaddrCity->setText(po.value("vend_city"));
      _vendaddrState->setText(po.value("vend_state"));
      _vendaddrZipCode->setText(po.value("vend_zip"));
      _vendaddrCountry->setText(po.value("vend_country"));
    }
    else
    {
      _vendaddrCode->setText(po.value("vendaddr_code"));
      _vendaddrName->setText(po.value("vendaddr_name"));
      _vendaddrAddr1->setText(po.value("vendaddr_address1"));
      _vendaddrAddr2->setText(po.value("vendaddr_address2"));
      _vendaddrAddr3->setText(po.value("vendaddr_address3"));
      _vendaddrCity->setText(po.value("vendaddr_city"));
      _vendaddrState->setText(po.value("vendaddr_state"));
      _vendaddrZipCode->setText(po.value("vendaddr_zipcode"));
      _vendaddrCountry->setText(po.value("vendaddr_country"));
    }

    _comments->setId(_poheadid);
    _vendor->setId(po.value("pohead_vend_id").toInt());
    _vendEmail = po.value("vend_emailpodelivery").toBool();
    _poCurrency->setId(po.value("pohead_curr_id").toInt());
    _tax->setLocalValue(po.value("pohead_tax").toDouble());
    _freight->setLocalValue(po.value("pohead_freight").toDouble());
  }

  sFillList();
}

void purchaseOrder::sSave()
{
  _save->setFocus();

  if(_orderNumber->hasFocus())
    sHandleOrderNumber();

  if (_orderNumber->text().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Save Purchase Order"),
      tr("You may not save this Purchase Order until you have entered a valid Purchase Order Number.") );
    _orderNumber->setFocus();
    return;
  }

  if (!_vendor->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Purchase Order"),
                           tr("You may not save this Purchase Order until you have selected a Vendor." ) );

    _vendor->setFocus();
    return;
  }

  if (_qeitem->isDirty() && ! sQESave())
    return;

  q.prepare( "SELECT poitem_id " 
             "FROM poitem "
             "WHERE (poitem_pohead_id=:pohead_id) "
             "LIMIT 1;" );
  q.bindValue(":pohead_id", _poheadid);
  q.exec();
  if (!q.first())
  {
    QMessageBox::critical( this, tr("Cannot Save Purchase Order"),
                           tr( "You may not save this Purchase Order until you have created at least one Line Item for it." ) );

    _new->setFocus();
    return;
  }

  q.prepare( "UPDATE pohead "
             "SET pohead_warehous_id=:pohead_warehous_id, pohead_orderdate=:pohead_orderdate,"
             "    pohead_shipvia=:pohead_shipvia,"
             "    pohead_tax=:pohead_tax, pohead_freight=:pohead_freight,"
             "    pohead_fob=:pohead_fob, pohead_agent_username=:pohead_agent_username,"
             "    pohead_terms_id=:pohead_terms_id,"
             "    pohead_prj_id=:pohead_prj_id, pohead_vendaddr_id=:pohead_vendaddr_id,"
             "    pohead_comments=:pohead_comments, "
             "    pohead_curr_id=:pohead_curr_id,"
             "    pohead_saved=true "
             "WHERE (pohead_id=:pohead_id);" );
  q.bindValue(":pohead_id", _poheadid);
  q.bindValue(":pohead_warehous_id", _warehouse->id());
  q.bindValue(":pohead_orderdate", _orderDate->date());
  q.bindValue(":pohead_shipvia", _shipVia->text());
  q.bindValue(":pohead_fob", _fob->text());
  q.bindValue(":pohead_agent_username", _agent->currentText());
  q.bindValue(":pohead_terms_id", _terms->id());
  q.bindValue(":pohead_prj_id", -1);
  q.bindValue(":pohead_vendaddr_id", _vendaddrid);
  q.bindValue(":pohead_comments", _notes->text());
  q.bindValue(":pohead_tax", _tax->localValue());
  q.bindValue(":pohead_freight", _freight->localValue());
  q.bindValue(":pohead_curr_id", _poCurrency->id());
  q.exec();
 
  omfgThis->sPurchaseOrdersUpdated(_poheadid, TRUE);

  if (!_pridList.isEmpty())
  {
    q.prepare("SELECT deletePr(:pr_id) AS _result;");
    for(int i = 0; i < _pridList.size(); ++i)
    {
      q.bindValue(":pr_id", _pridList.at(i));
      q.exec();
    }
    omfgThis->sPurchaseRequestsUpdated();
  }

  if (_printPO->isChecked())
  {
    ParameterList params;
    params.append("pohead_id", _poheadid);

    printPurchaseOrder newdlgP(this, "", true);
    newdlgP.set(params);
    newdlgP.exec();

    if (_vendEmail && _metrics->boolean("EnableBatchManager"))
    {
      deliverPurchaseOrder newdlgD(this, "", true);
      newdlgD.set(params);
      newdlgD.exec();
    }
  }

  if (_mode == cNew)
  {
    _purchaseOrderInformation->setCurrentPage(0);

    _agent->setText(omfgThis->username());
    _terms->setId(-1);
    _vendor->setId(-1);

    _orderNumber->clear();
    _orderDate->clear();
    _shipVia->clear();
    _status->clear();
    _fob->clear();
    _notes->clear();
    _tax->clear();
    _subtotal->clear();
    _freight->clear();
    _total->clear();
    _poitem->clear();
    _poCurrency->setEnabled(true);
    _qecurrency->setEnabled(true);
    _qeitem->removeRows(0, _qeitem->rowCount());

    _vendaddrCode->clear();
    _vendaddrName->clear();
    _vendaddrAddr1->clear();
    _vendaddrAddr2->clear();
    _vendaddrAddr3->clear();
    _vendaddrCity->clear();
    _vendaddrState->clear();
    _vendaddrZipCode->clear();
    _vendaddrCountry->clear();

    _close->setText(tr("&Close"));

    createHeader();
  }
  else
  {
    close();
  }
}

void purchaseOrder::sNew()
{
  if (_mode == cEdit || _mode == cNew)
  {
    q.prepare( "UPDATE pohead "
               "SET pohead_warehous_id=:pohead_warehous_id, pohead_vend_id=:pohead_vend_id,"
               "    pohead_number=:pohead_number, "
	       "    pohead_curr_id=:pohead_curr_id, "
	       "    pohead_orderdate=:pohead_orderdate "
               "WHERE (pohead_id=:pohead_id);" );
    q.bindValue(":pohead_warehous_id", _warehouse->id());
    q.bindValue(":pohead_vend_id", _vendor->id());
    q.bindValue(":pohead_number", _orderNumber->text().toInt());
    q.bindValue(":pohead_id", _poheadid);
    q.bindValue(":pohead_curr_id", _poCurrency->id());
    q.bindValue(":pohead_orderdate", _orderDate->date());
    q.exec();
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("pohead_id", _poheadid);

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void purchaseOrder::sEdit()
{
  ParameterList params;
  params.append("poitem_id", _poitem->id());

  if (_mode == cEdit || _mode == cNew)
    params.append("mode", "edit");
  else if (_mode == cView)
    params.append("mode", "view");

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void purchaseOrder::sDelete()
{
  q.prepare( "DELETE FROM poitem "
             "WHERE (poitem_id=:poitem_id);" );
  q.bindValue(":poitem_id", _poitem->id());
  q.exec();

  sFillList();
}

void purchaseOrder::sVendaddrList()
{
  ParameterList params;
  params.append("vend_id", _vendor->id());

  vendorAddressList newdlg(this, "", TRUE);
  newdlg.set(params);

  int vendaddrid;
  if ((vendaddrid = newdlg.exec()) != XDialog::Rejected)
  {
    if (vendaddrid != -1)
    {
      q.prepare( "SELECT vendaddr_code, vendaddr_name,"
                 "       vendaddr_address1, vendaddr_address2, vendaddr_address3, "
                 "       vendaddr_city, vendaddr_state, vendaddr_zipcode, vendaddr_country "
                 "FROM vendaddr "
                 "WHERE (vendaddr_id=:vendaddr_id);" );
      q.bindValue(":vendaddr_id", vendaddrid);
      q.exec();
      if (q.first())
      {
        _vendaddrid = vendaddrid;
        _vendaddrCode->setText(q.value("vendaddr_code"));
        _vendaddrName->setText(q.value("vendaddr_name"));
        _vendaddrAddr1->setText(q.value("vendaddr_address1"));
        _vendaddrAddr2->setText(q.value("vendaddr_address2"));
        _vendaddrAddr3->setText(q.value("vendaddr_address3"));
        _vendaddrCity->setText(q.value("vendaddr_city"));
        _vendaddrState->setText(q.value("vendaddr_state"));
        _vendaddrZipCode->setText(q.value("vendaddr_zipcode"));
        _vendaddrCountry->setText(q.value("vendaddr_country"));
      }
    }
    else
    {
      q.prepare( "SELECT vend_name, "
                 "       vend_address1, vend_address2, vend_address3, "
                 "       vend_city, vend_state, vend_zip, vend_country "
                 "FROM vend "
                 "WHERE (vend_id=:vend_id);" );
      q.bindValue(":vend_id", _vendor->id());
      q.exec();
      if (q.first())
      {
        _vendaddrid = -1;
        _vendaddrCode->setText(tr("Main"));
        _vendaddrName->setText(q.value("vend_name"));
        _vendaddrAddr1->setText(q.value("vend_address1"));
        _vendaddrAddr2->setText(q.value("vend_address2"));
        _vendaddrAddr3->setText(q.value("vend_address3"));
        _vendaddrCity->setText(q.value("vend_city"));
        _vendaddrState->setText(q.value("vend_state"));
        _vendaddrZipCode->setText(q.value("vend_zip"));
        _vendaddrCountry->setText(q.value("vend_country"));
      }
    }
  }
}

void purchaseOrder::sHandleDeleteButton()
{
  if ( (_mode == cNew) || (_mode == cEdit))
  {
    QTreeWidgetItem *selected = _poitem->currentItem();

    // _poitem->currentItem()->text(1) == values have to match sFillList()
    if (selected == 0)
      _delete->setEnabled(FALSE);
    else if (_poitem->currentItem()->text(1) == tr("Unposted"))
    {
      _deleteMode = cDelete;
      _delete->setEnabled(TRUE);
      _delete->setText(tr("&Delete"));
    }
    else if (_poitem->currentItem()->text(1) == tr("Open"))
    {
      _deleteMode = cClose;
      _delete->setEnabled(TRUE);
      _delete->setText(tr("C&lose"));
    }
  }
}
 

void purchaseOrder::sHandleVendor(int pVendid)
{
  if ( (pVendid != -1) && (_mode == cNew) )
  {
    q.prepare( "UPDATE pohead "
               "SET pohead_warehous_id=:pohead_warehous_id, "
               " pohead_vend_id=:pohead_vend_id, pohead_curr_id=:pohead_curr_id "
               "WHERE (pohead_id=:pohead_id);" );
    q.bindValue(":pohead_warehous_id", _warehouse->id());
    q.bindValue(":pohead_vend_id", pVendid);
    q.bindValue(":pohead_id", _poheadid);
    q.bindValue(":pohead_curr_id", _poCurrency->id());
    q.exec();

    q.prepare( "SELECT vend_terms_id, vend_curr_id, "
               "       vend_fobsource, vend_fob, vend_shipvia,"
               "       vend_name, vend_address1, vend_address2, vend_address3,"
               "       vend_city, vend_state, vend_zip, vend_country,"
               "       COALESCE(vendaddr_id, -1) AS vendaddrid "
               "FROM vend LEFT OUTER JOIN vendaddr ON (vendaddr_vend_id=vend_id) "
               "WHERE (vend_id=:vend_id) "
               "LIMIT 1;" );
    q.bindValue(":vend_id", pVendid);
    q.exec();
    if (q.first())
    {
      _poCurrency->setId(q.value("vend_curr_id").toInt());
      
      if (_terms->id() == -1)
        _terms->setId(q.value("vend_terms_id").toInt());

      if (_shipVia->text().length() == 0)
        _shipVia->setText(q.value("vend_shipvia"));

      if (q.value("vend_fobsource").toString() == "V")
      {
        _useWarehouseFOB = FALSE;
        _fob->setText(q.value("vend_fob"));
      }
      else
      {
        _useWarehouseFOB = TRUE;
        _fob->setText(tr("Destination"));
      }

      if (q.value("vendaddrid").toInt())
      {
        _vendaddrid = -1;
        _vendaddrCode->setText(tr("Main"));
        _vendaddrName->setText(q.value("vend_name"));
        _vendaddrAddr1->setText(q.value("vend_address1"));
        _vendaddrAddr2->setText(q.value("vend_address2"));
        _vendaddrAddr3->setText(q.value("vend_address3"));
        _vendaddrCity->setText(q.value("vend_city"));
        _vendaddrState->setText(q.value("vend_state"));
        _vendaddrZipCode->setText(q.value("vend_zip"));
        _vendaddrCountry->setText(q.value("vend_country"));
      }
    }
  }
}

void purchaseOrder::sFillList()
{
  q.prepare( "SELECT poitem.*,"
             "       CASE WHEN(poitem_status='C') THEN :closed"
             "            WHEN(poitem_status='U') THEN :unposted"
             "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered>(poitem_qty_received-poitem_qty_returned))) THEN :partial"
             "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered=(poitem_qty_received-poitem_qty_returned))) THEN :received"
             "            WHEN(poitem_status='O') THEN :open"
             "            ELSE poitem_status"
             "       END AS poitemstatus,"
             "       CASE WHEN (itemsite_id IS NULL) THEN poitem_vend_item_number"
             "            ELSE item_number"
             "       END AS item_number,"
             "       CASE WHEN (itemsite_id IS NULL) THEN firstLine(poitem_vend_item_descrip)"
             "            ELSE (item_descrip1 || ' ' || item_descrip2)"
             "       END AS item_descrip,"
             "       (poitem_unitprice * poitem_qty_ordered) AS extprice, "
             "       'qty' AS poitem_qty_ordered_xtnumericrole,"
             "       'purchprice' AS poitem_unitprice_xtnumericrole,"
             "       'curr' AS extprice_xtnumericrole "
             "FROM poitem LEFT OUTER JOIN"
             "     ( itemsite JOIN item"
             "       ON (itemsite_item_id=item_id) )"
             "     ON (poitem_itemsite_id=itemsite_id) "
             "WHERE (poitem_pohead_id=:pohead_id) "
             "ORDER BY poitem_linenumber;" );
  q.bindValue(":pohead_id", _poheadid);
  q.bindValue(":closed", tr("Closed"));
  q.bindValue(":unposted", tr("Unposted"));
  q.bindValue(":partial", tr("Partial"));
  q.bindValue(":received", tr("Received"));
  q.bindValue(":open", tr("Open"));

  q.exec();
  _poitem->populate(q);

  sCalculateTotals();

  _poCurrency->setEnabled(_poitem->topLevelItemCount() == 0);
  _qecurrency->setEnabled(_poitem->topLevelItemCount() == 0);

  _qeitem->select();
}

void purchaseOrder::sCalculateTotals()
{
  q.prepare( "SELECT SUM(poitem_qty_ordered * poitem_unitprice) AS total,"
             "       SUM(poitem_qty_ordered * poitem_unitprice) AS f_total,"
             "       SUM(poitem_freight) AS freightsub "
             "FROM poitem "
             "WHERE (poitem_pohead_id=:pohead_id);" );
  q.bindValue(":pohead_id", _poheadid);
  q.exec();
  if (q.first())
  {
    _subtotal->setLocalValue(q.value("f_total").toDouble());
    _totalFreight->setLocalValue(q.value("freightsub").toDouble() + _freight->localValue());
    _total->setLocalValue(q.value("total").toDouble() + _tax->localValue() + q.value("freightsub").toDouble() + _freight->localValue());
  }
}

void purchaseOrder::sSetUserOrderNumber()
{
  _userOrderNumber = TRUE;
}
    
void purchaseOrder::sHandleOrderNumber()
{
  if (_orderNumber->text().length() == 0)
  {
    if ( (_metrics->value("PONumberGeneration") == "A") ||
         (_metrics->value("PONumberGeneration") == "O") )
      populateOrderNumber();
    else
      return;
  }
  else if (_userOrderNumber)
  {
    q.prepare( "SELECT pohead_id "
               "FROM pohead "
               "WHERE (pohead_number=:pohead_number);" );
    q.bindValue(":pohead_number", _orderNumber->text().toInt());
    q.exec();
    if (q.first())
    {
      int poheadid = q.value("pohead_id").toInt();
      if(poheadid == _poheadid)
        return;

      if(cNew != _mode || !_pridList.isEmpty())
      {
        QMessageBox::warning( this, tr("Enter Purchase Order #"),
          tr("The Purchase Order number you entered is already in use. Please enter a different number.") );
        _orderNumber->clear();
        _orderNumber->setFocus();
        return;        
      }

      q.prepare( "DELETE FROM pohead "
                 "WHERE (pohead_id=:pohead_id);"
                 "DELETE FROM poitem "
                 "WHERE (poitem_pohead_id=:pohead_id);"
                 "SELECT releasePoNumber(:orderNumber);" );
      q.bindValue(":pohead_id", _poheadid);
      q.bindValue(":orderNumber", _orderNumber->text().toInt());
      q.exec();
      if (q.lastError().type() != QSqlError::None)
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

      _mode = cEdit;
      setPoheadid(poheadid);
      populate();
      _orderNumber->setEnabled(FALSE);
      _orderDate->setEnabled(FALSE);
      _vendor->setReadOnly(TRUE);
    }
  }
  if(_poheadid != -1)
  {
    q.prepare("UPDATE pohead"
              "   SET pohead_number=:pohead_number"
              " WHERE (pohead_id=:pohead_id);");
    q.bindValue(":pohead_id", _poheadid);
    q.bindValue(":pohead_number", _orderNumber->text().toInt());
    if(!q.exec())
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
  }
}

void purchaseOrder::populateOrderNumber()
{
  XSqlQuery on;
  on.exec("SELECT fetchPoNumber() AS ponumber;");
  if (on.first())
    _orderNumber->setText(on.value("ponumber"));

  if (_metrics->value("PONumberGeneration") == "A")
    _orderNumber->setEnabled(FALSE);
}

void purchaseOrder::closeEvent(QCloseEvent *pEvent)
{
  if ((_mode == cNew) && (_poheadid != -1))
  {
    if (_poitem->topLevelItemCount() > 0 &&
        QMessageBox::question(this, tr("Delete Purchase Order?"),
			      tr("<p>Are you sure you want to delete this "
				 "Purchase Order and all of its associated "
				 "Line Items?"),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      pEvent->ignore();
      return;
    }

    q.prepare( "DELETE FROM pohead "
               "WHERE (pohead_id=:pohead_id);"
               "DELETE FROM poitem "
               "WHERE (poitem_pohead_id=:pohead_id);"
               "SELECT releasePoNumber(:orderNumber);" );
    q.bindValue(":pohead_id", _poheadid);
    q.bindValue(":orderNumber", _orderNumber->text().toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  // TODO: if sQeSave == false then find a way to return control to the user
  if (_qeitem->isDirty())
  {
    if (QMessageBox::question(this, tr("Save Quick Entry Data?"),
		    tr("Do you want to save your Quick Entry changes?"),
		    QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
      sQESave();
  }

  XMainWindow::closeEvent(pEvent);
}

bool purchaseOrder::sQESave()
{
  _qesave->setFocus();
  if (! _qeitem->submitAll())
  {
    if (! _qeitem->lastError().databaseText().isEmpty())
      systemError(this, _qeitem->lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  sFillList();
  return true;
}

void purchaseOrder::sQEDelete()
{
  if (! _qeitem->removeRow(_qeitemView->currentIndex().row()))
  {
    systemError(this, tr("Removing row from view failed"), __FILE__, __LINE__);
    return;
  }
}

void purchaseOrder::sCurrencyChanged()
{
  // The user can only set the QE currency from the QE tab
  if (_purchaseOrderInformation->currentWidget()->isAncestorOf(_qecurrency) &&
      _poCurrency->id() != _qecurrency->id())
    _poCurrency->setId(_qecurrency->id());
  else
    _qecurrency->setId(_poCurrency->id());

  static_cast<PoitemTableModel*>(_qeitemView->model())->setCurrId(_poCurrency->id());
}

void purchaseOrder::sTabChanged(int pIndex)
{
  if (pIndex != _cachedTabIndex &&
      _cachedTabIndex == _purchaseOrderInformation->indexOf(_quickEntryTab) &&
      _qeitem->isDirty())
  {
    if (QMessageBox::question(this, tr("Save Quick Entry Data?"),
		    tr("Do you want to save your Quick Entry changes?"),
		    QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
      if (! sQESave())
      {
	_purchaseOrderInformation->setCurrentIndex(_cachedTabIndex);
	return;
      }
  }
  _cachedTabIndex = pIndex;
}

void purchaseOrder::sHandleOrderDate()
{
  static_cast<PoitemTableModel*>(_qeitemView->model())->setTransDate(_orderDate->date());
}
