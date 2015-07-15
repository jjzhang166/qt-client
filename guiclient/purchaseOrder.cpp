/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "purchaseOrder.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "comment.h"
#include "itemSourceList.h"
#include "mqlutil.h"
#include "poitemTableModel.h"
#include "printPurchaseOrder.h"
#include "purchaseOrderItem.h"
#include "vendorAddressList.h"
#include "taxBreakdown.h"
#include "salesOrder.h"
#include "workOrder.h"
#include "openPurchaseOrder.h" 

#define cDelete 0x01
#define cClose  0x02

purchaseOrder::purchaseOrder(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  XSqlQuery purchasepurchaseOrder;
  setupUi(this);

  _so->setNameVisible(false);
  _so->setDescriptionVisible(false);

  _orderNumber->setValidator(new QIntValidator(this));

  connect(_poitem,                   SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this,          SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_delete,                   SIGNAL(clicked()),                                 this,          SLOT(sDelete()));
  connect(_edit,                     SIGNAL(clicked()),                                 this,          SLOT(sEdit()));
  connect(_freight,                  SIGNAL(valueChanged()),                            this,          SLOT(sCalculateTotals()));
  connect(_new,                      SIGNAL(clicked()),                                 this,          SLOT(sNew()));
  connect(_orderDate,                SIGNAL(newDate(QDate)),                            this,          SLOT(sHandleOrderDate()));
  connect(_orderNumber,              SIGNAL(editingFinished()),                         this,          SLOT(sHandleOrderNumber()));
  connect(_orderNumber,              SIGNAL(textChanged(const QString&)),               this,          SLOT(sSetUserOrderNumber()));
  connect(_poCurrency,               SIGNAL(newID(int)),                                this,          SLOT(sCurrencyChanged()));
  connect(_poitem,                   SIGNAL(itemSelectionChanged()),                    this,          SLOT(sHandleDeleteButton()));
  connect(_purchaseOrderInformation, SIGNAL(currentChanged(int)),                       this,          SLOT(sTabChanged(int)));
  connect(_qecurrency,               SIGNAL(newID(int)),                                this,          SLOT(sCurrencyChanged()));
  connect(_qedelete,                 SIGNAL(clicked()),                                 this,          SLOT(sQEDelete()));
  connect(_qesave,                   SIGNAL(clicked()),                                 this,          SLOT(sQESave()));
  connect(_save,                     SIGNAL(clicked()),                                 this,          SLOT(sSave()));
  connect(_taxLit,                   SIGNAL(leftClickedURL(const QString&)),            this,          SLOT(sTaxDetail()));
  connect(_tax,                      SIGNAL(valueChanged()),                            this,          SLOT(sCalculateTotals()));
  connect(_taxZone,                  SIGNAL(newID(int)),                                this,          SLOT(sTaxZoneChanged()));
  connect(_vendaddrList,             SIGNAL(clicked()),                                 this,          SLOT(sVendaddrList()));
  connect(_vendor,                   SIGNAL(newId(int)),                                this,          SLOT(sHandleVendor(int)));
  connect(_vendAddr,                 SIGNAL(changed()),                                 _vendaddrCode, SLOT(clear()));
  connect(_warehouse,                SIGNAL(newID(int)),                                this,          SLOT(sHandleShipTo()));
  connect(_shiptoAddr,               SIGNAL(newId(int)),                                this,          SLOT(sHandleShipToName()));

  connect(_vendAddr, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _vendCntct, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  connect(_shiptoAddr, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _shiptoCntct, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

#ifndef Q_OS_MAC
  _vendaddrList->setMaximumWidth(25);
#endif

  _poCurrency->setLabel(_poCurrencyLit);

  _poitem->addColumn(tr("#"),              _whsColumn,    Qt::AlignCenter,true,  "poitem_linenumber");
  _poitem->addColumn(tr("Status"),         _statusColumn, Qt::AlignCenter,true,  "poitem_status");
  _poitem->addColumn(tr("Item"),           _itemColumn,   Qt::AlignLeft,  true,  "item_number");
  _poitem->addColumn(tr("Description"),    -1,            Qt::AlignLeft,  true,  "item_descrip");
  _poitem->addColumn(tr("Orgl. Due Date"), _dateColumn,   Qt::AlignRight, false, "orgl_duedate");
  _poitem->addColumn(tr("Due Date Now"),   _dateColumn,   Qt::AlignRight, true,  "poitem_duedate");
  _poitem->addColumn(tr("Ordered"),        _qtyColumn,    Qt::AlignRight, true,  "poitem_qty_ordered");
  _poitem->addColumn(tr("Received"),       _qtyColumn,    Qt::AlignRight, false, "poitem_qty_received");
  _poitem->addColumn(tr("Returned"),       _qtyColumn,    Qt::AlignRight, false, "poitem_qty_returned");
  _poitem->addColumn(tr("Vouchered"),      _qtyColumn,    Qt::AlignRight, false, "poitem_qty_vouchered");
  _poitem->addColumn(tr("UOM"),            _uomColumn,    Qt::AlignCenter,true,  "poitem_vend_uom");
  _poitem->addColumn(tr("Unit Price"),     _priceColumn,  Qt::AlignRight, true,  "poitem_unitprice");
  _poitem->addColumn(tr("Ext. Price"),     _moneyColumn,  Qt::AlignRight, true,  "extprice");
  _poitem->addColumn(tr("Freight"),        _moneyColumn,  Qt::AlignRight, false, "poitem_freight");
  _poitem->addColumn(tr("Freight Recv."),  _moneyColumn,  Qt::AlignRight, false, "poitem_freight_received");
  _poitem->addColumn(tr("Freight Vchr."),  _moneyColumn,  Qt::AlignRight, false, "poitem_freight_vouchered");
  _poitem->addColumn(tr("Std. Cost"),      _moneyColumn,  Qt::AlignRight, false, "poitem_stdcost");
  _poitem->addColumn(tr("Vend. Item#"),    _itemColumn,   Qt::AlignCenter,false, "poitem_vend_item_number");
  _poitem->addColumn(tr("Manufacturer"),   _itemColumn,   Qt::AlignRight, false, "poitem_manuf_name");
  _poitem->addColumn(tr("Manuf. Item#"),   _itemColumn,   Qt::AlignRight, false, "poitem_manuf_item_number");
  _poitem->addColumn(tr("Demand Type"),    _itemColumn,   Qt::AlignCenter,false, "demand_type");
  _poitem->addColumn(tr("Order"),          _itemColumn,   Qt::AlignRight, false, "order_number");

  if (_metrics->value("Application") == "Standard")
	_poitem->addColumn(tr("Contract"),                -1,   Qt::AlignLeft,   true, "contrct_number");

  _charass->setType("PO");
  
  _qeitem = new PoitemTableModel(this);
  _qeitemView->setModel(_qeitem);
  _qeitem->select();

  purchasepurchaseOrder.exec("SELECT usr_id "
         "FROM usr "
         "WHERE (usr_username=getEffectiveXtUser());");
  if(purchasepurchaseOrder.first())
    _agent->setId(purchasepurchaseOrder.value("usr_id").toInt());

  _captive         = false;
  _userOrderNumber = false;
  _printed         = false;
  _NumberGen       = -1;

  setPoheadid(-1);
  _vendaddrid = -1;

  _cachedTabIndex = 0;

  _mode = cView;        // initialize _mode to something safe - bug 3768
 
  _printPO->setChecked(_metrics->boolean("DefaultPrintPOOnSave"));

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _receivingWhseLit->hide();
    _warehouse->hide();
  }

  if (_metrics->boolean("EnableDropShipments"))
    _dropShip->setEnabled(false);
  else
    _dropShip->hide();
  _so->setReadOnly(true);

  _projectId = -1;

XSqlQuery getWeightUOM;
getWeightUOM.prepare("SELECT uom_name FROM uom WHERE (uom_item_weight);");
getWeightUOM.exec();
if (getWeightUOM.first())
  {
    QString newLabel (tr("Total Weight (%1):"));
    _totalWeightLit->setText(newLabel.arg(getWeightUOM.value("uom_name").toString()));
  }

}

void purchaseOrder::setPoheadid(const int pId)
{
  _poheadid = pId;
  _qeitem->setHeadId(pId);
  emit newId(_poheadid);
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
  XSqlQuery purchaseet;
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  int      itemsiteid = -1;
  int      itemsrcid = -1;
  int      parentwo = -1;
  int      parentso = -1;
  int      prjid = -1;
  double   qty = 0.0;
  QDate    dueDate;
  QString  prnotes;

  setPoheadid(-1);
  int _prid = -1;
  param = pParams.value("pr_id", &valid);
  if (valid)
    _prid = param.toInt();

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    itemsiteid = param.toInt();

  param = pParams.value("itemsrc_id", &valid);
  if (valid)
    itemsrcid = param.toInt();
  
  param = pParams.value("qty", &valid);
  if (valid)
    qty = param.toDouble();

  param = pParams.value("dueDate", &valid);
  if (valid)
    dueDate = param.toDate();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if ( (param.toString() == "new") || (param.toString() == "releasePr") )
    {
      _mode = cNew;
      emit newMode(_mode);

      if (param.toString() == "releasePr")
      {
        purchaseet.prepare( "SELECT pr_itemsite_id, pr_qtyreq, pr_duedate,"
                   "       CASE WHEN(pr_order_type='W') THEN pr_order_id"
                   "            ELSE -1"
                   "       END AS parentwo,"
                   "       CASE WHEN(pr_order_type='S') THEN pr_order_id"
                   "            ELSE -1"
                   "       END AS parentso,"
                   "       pr_prj_id, pr_releasenote "
                   "FROM pr "
                   "WHERE (pr_id=:pr_id);" );
        purchaseet.bindValue(":pr_id", _prid);
        purchaseet.exec();
        if (purchaseet.first())
        {
          itemsiteid = purchaseet.value("pr_itemsite_id").toInt();
          qty = purchaseet.value("pr_qtyreq").toDouble();
          dueDate = purchaseet.value("pr_duedate").toDate();
          parentwo = purchaseet.value("parentwo").toInt();
          parentso = purchaseet.value("parentso").toInt();
          prjid = purchaseet.value("pr_prj_id").toInt();
          prnotes = purchaseet.value("pr_releasenote").toString();
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
      //_new->setEnabled(true);
      int openpoid =-1;
      if (itemsiteid != -1 || itemsrcid != -1)
      {
        if (itemsiteid != -1)
        {
          purchaseet.prepare( "SELECT itemsite_item_id, itemsrc_id, itemsrc_default "
                             "FROM itemsite, itemsrc "
                             "WHERE ( (itemsrc_item_id=itemsite_item_id)"
                             " AND (itemsite_id=:itemsite_id) ) "
                             "LIMIT 1;" );
          purchaseet.bindValue(":itemsite_id", itemsiteid);
          purchaseet.exec();
          if (!purchaseet.first())
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
          if (purchaseet.first())
          {
            XSqlQuery itemsrcdefault;
            MetaSQLQuery mql = mqlLoad("itemSources", "detail");
            
            ParameterList paramsdft;
            paramsdft.append("item_id", purchaseet.value("itemsite_item_id").toInt());
            paramsdft.append("defaultOnly", true);
            itemsrcdefault = mql.toQuery(paramsdft);
            itemsrcdefault.exec();
            if (itemsrcdefault.first())
            {
              itemsrcid=(itemsrcdefault.value("itemsrc_id").toInt());
            }
            else
            {
              ParameterList itemSourceParams;
              itemSourceParams.append("item_id", purchaseet.value("itemsite_item_id").toInt());
              itemSourceParams.append("qty", qty);
              
              itemSourceList newdlg(omfgThis, "", true);
              newdlg.set(itemSourceParams);
              itemsrcid = newdlg.exec();
              if (itemsrcid == XDialog::Rejected)
              {
                deleteLater();
                return UndefinedError;
              }
            }
          }
        }
        
        if (itemsrcid != -1)
        {
          purchaseet.prepare( "SELECT itemsrc_vend_id, vend_name  "
                             "FROM itemsrc LEFT OUTER JOIN vendinfo ON (vend_id = itemsrc_vend_id) "
                             "WHERE (itemsrc_id=:itemsrc_id);" );
          purchaseet.bindValue(":itemsrc_id", itemsrcid);
          purchaseet.exec();
          if (!purchaseet.first())
          {
            systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
            return UndefinedError;
          }
        }

        int vendid = purchaseet.value("itemsrc_vend_id").toInt();
        QString vendname = purchaseet.value("vend_name").toString();

        purchaseet.prepare( "SELECT pohead_id "
                            "FROM pohead "
                            "WHERE ( (pohead_status='U')"
                            " AND (pohead_vend_id=:vend_id) ) "
                            "ORDER BY pohead_number "
                            "LIMIT 1;" );
        purchaseet.bindValue(":vend_id", vendid);
        purchaseet.exec();
        if (purchaseet.first())
        {
          if(QMessageBox::question( this, tr("Purchase Order Exists"),
                                    tr("An Unreleased Purchase Order already exists for this Vendor.\n"
                                       "Would you like to use this Purchase Order?\n"
                                       "Click Yes to use the existing Purchase Order otherwise a new one will be created."),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
          {
            ParameterList openPurchaseOrderParams;
            openPurchaseOrderParams.append("vend_id", vendid);
            openPurchaseOrderParams.append("vend_name", vendname);
            openPurchaseOrder newdlg(omfgThis, "", true);
            newdlg.set(openPurchaseOrderParams);
            openpoid = newdlg.exec();

            if (openpoid == XDialog::Rejected)
            {
              deleteLater();
              return UndefinedError;
            }
//  Use an existing pohead
            _mode = cEdit;
            emit newMode(_mode);

            setPoheadid(openpoid);
            _orderNumber->setEnabled(false);
            _orderDate->setEnabled(false);
            _vendor->setReadOnly(true);
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
        newItemParams.append("captive", true);
        newItemParams.append("pohead_id", _poheadid);
        if (itemsiteid > 0)
          newItemParams.append("itemsite_id", itemsiteid);
        if (itemsrcid > 0)
          newItemParams.append("itemsrc_id", itemsrcid);

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

        newItemParams.append("pr_releasenote", prnotes);

        purchaseOrderItem poItem(this, "", true);
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
              if (strcmp(w->metaObject()->className(), "purchaseOrder") == 0 && w != this)
              {
                purchaseOrder *other = (purchaseOrder*)w;
                if(_poheadid == other->_poheadid)
                {
                  if(_prid != -1 && cEdit == other->_mode)
                  {
                    purchaseet.prepare("SELECT deletePr(:pr_id) AS _result;");
                    purchaseet.bindValue(":pr_id", _prid);
                    purchaseet.exec();
                    omfgThis->sPurchaseRequestsUpdated();
                  }
                  other->sFillList();
                  return UndefinedError;
                }
              }
            }
          }
          sFillList();
        }
        else
        {
          _prid = -1;
        }
      }
      else
      {
      //  This is a new P/O without a chosen Itemsite Line Item
        createHeader();
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      emit newMode(_mode);

      _orderNumber->setEnabled(false);
      _orderDate->setEnabled(false);
      _warehouse->setEnabled(false);
      _vendor->setReadOnly(true);

      connect(_poitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_poitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _new->setEnabled(true);

    }
    else if (param.toString() == "view")
    {
      setViewMode();
    }
  }

  if (_prid != -1 && cNew != _mode)
  {
    purchaseet.prepare("SELECT deletePr(:pr_id) AS _result;");
    purchaseet.bindValue(":pr_id", _prid);
    purchaseet.exec();
    _prid = -1;
    omfgThis->sPurchaseRequestsUpdated();
  }

  if(_prid != -1)
    _pridList.append(_prid);

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    setPoheadid(param.toInt());
    populate();
  }

 if (cNew == _mode)
  {
    param = pParams.value("prj_id", &valid);
    if (valid)
      _projectId = param.toInt();
  }

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendor->setId(param.toInt());
  param = pParams.value("captive", &valid);
  if (valid)
    _captive = true;

  return NoError;
}

int purchaseOrder::id() const
{
  return _poheadid;
}

/** \return one of cNew, cEdit, cView, ...
 \todo   change possible modes to an enum in guiclient.h (and add cUnknown?)
 */
int purchaseOrder::mode() const
{
  return _mode;
}

void purchaseOrder::setViewMode()
{
  if (cEdit == _mode)
  {
    // Undo some changes set for the edit mode
    disconnect(_poitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    disconnect(_poitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

    _new->setEnabled(false);
  }

  _mode = cView;
  emit newMode(_mode);
  
  _orderNumber->setEnabled(false);
  _orderDate->setEnabled(false);
  _warehouse->setEnabled(false);
  _taxZone->setEnabled(false);
  _agent->setEnabled(false);
  _terms->setEnabled(false);
  _terms->setType(XComboBox::Terms);
  _vendor->setReadOnly(true);
  _vendCntct->setEnabled(false);
  _vendAddr->setEnabled(false);
  _shiptoCntct->setEnabled(false);
  _shiptoName->setEnabled(false);
  _shiptoAddr->setEnabled(false);
  _shipVia->setEnabled(false);
  _fob->setEnabled(false);
  _status->setEnabled(false);
  _notes->setEnabled(false);
  _new->setEnabled(false);
  _freight->setEnabled(false);
  _tax->setEnabled(false);
  _vendaddrList->hide();
  _purchaseOrderInformation->removeTab(_purchaseOrderInformation->indexOf(_quickEntryTab));
  _poCurrency->setEnabled(false);
  _qeitemView->setEnabled(false);
  _qesave->setEnabled(false);
  _qedelete->setEnabled(false);
  _qecurrency->setEnabled(false);
  _comments->setReadOnly(true);
  //      _documents->setReadOnly(true);
  _charass->setReadOnly(true);
  
  _delete->hide();
  _edit->setText(tr("&View"));
  _close->setText(tr("&Close"));
  _save->hide();
  
  connect(_poitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_poitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
}

void purchaseOrder::createHeader()
{
  XSqlQuery purchasecreateHeader;
//  Determine the new PO Number
  if ( (_metrics->value("PONumberGeneration") == "A") ||
       (_metrics->value("PONumberGeneration") == "O") )
  {
    populateOrderNumber();
    // Tabbed mode has problems with this behavior
    if(omfgThis->showTopLevel())
      _vendor->setFocus();
  }
  else if (omfgThis->showTopLevel())
    _orderNumber->setFocus();

  purchasecreateHeader.exec("SELECT NEXTVAL('pohead_pohead_id_seq') AS pohead_id;");
  if (purchasecreateHeader.first())
    setPoheadid(purchasecreateHeader.value("pohead_id").toInt());
  else
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
    return;
  }

  // need to set at least the _order date before the INSERT
  _comments->setId(_poheadid);
  _documents->setId(_poheadid);
  _orderDate->setDate(omfgThis->dbDate(), true);
  _status->setCurrentIndex(0);
  _vendor->setShowInactive(false);

  purchasecreateHeader.prepare( "INSERT INTO pohead "
             "( pohead_id, pohead_number, pohead_status,"
             "  pohead_agent_username, pohead_vend_id, pohead_taxzone_id,"
             "  pohead_orderdate, pohead_curr_id, pohead_saved) "
             "VALUES "
             "( :pohead_id, :pohead_number, 'U',"
             "  :pohead_agent_username, :pohead_vend_id, :pohead_taxzone_id, "
             "  :pohead_orderdate, :pohead_curr_id, false );" );
  purchasecreateHeader.bindValue(":pohead_id", _poheadid);
  purchasecreateHeader.bindValue(":pohead_agent_username", _agent->currentText());
  purchasecreateHeader.bindValue(":pohead_number", _orderNumber->text().isEmpty() ? "TEMP" + QString(0 - _poheadid)
                                                                                  : _orderNumber->text());
  if (_vendor->isValid())
    purchasecreateHeader.bindValue(":pohead_vend_id", _vendor->id());
  if (_taxZone->isValid())
    purchasecreateHeader.bindValue(":pohead_taxzone_id", _taxZone->id());
  purchasecreateHeader.bindValue(":pohead_orderdate", _orderDate->date());
  purchasecreateHeader.bindValue(":pohead_curr_id", _poCurrency->id());
  purchasecreateHeader.exec();
  if (purchasecreateHeader.lastError().type() != QSqlError::NoError)
    systemError(this, purchasecreateHeader.lastError().databaseText(), __FILE__, __LINE__);

  // Populate Ship To contact and addresses for the Receiving Site
  sHandleShipTo();

  (void)_lock.acquire("pohead", _poheadid, AppLock::Interactive);
}

void purchaseOrder::populate()
{
  XSqlQuery po;

  if (_mode == cEdit && ! _lock.acquire("pohead", _poheadid, AppLock::Interactive))
  {
    setViewMode();
  }
  
  po.prepare( "SELECT pohead.*, COALESCE(pohead_warehous_id, -1) AS warehous_id,"
              "       COALESCE(pohead_cohead_id, -1) AS cohead_id,"
              "       CASE WHEN (pohead_status='U') THEN 0"
              "            WHEN (pohead_status='O') THEN 1"
              "            WHEN (pohead_status='C') THEN 2"
              "       END AS status,"
              "       COALESCE(pohead_terms_id, -1) AS terms_id,"
              "       COALESCE(pohead_vend_id, -1) AS vend_id,"
              "       COALESCE(vendaddr_id, -1) AS vendaddrid,"
              "       vendaddr_code "
              "FROM pohead JOIN vendinfo ON (pohead_vend_id=vend_id)"
              "     LEFT OUTER JOIN vendaddrinfo ON (pohead_vendaddr_id=vendaddr_id)"
              "     LEFT OUTER JOIN cohead ON (pohead_cohead_id=cohead_id) "
              "WHERE (pohead_id=:pohead_id);" );
  po.bindValue(":pohead_id", _poheadid);
  po.exec();
  if (po.first())
  {
    if (po.value("pohead_status").toString() == "C")
      setViewMode();
    
    _orderNumber->setText(po.value("pohead_number"));
    _warehouse->setId(po.value("warehous_id").toInt());
    _orderDate->setDate(po.value("pohead_orderdate").toDate(), true);
    if(po.value("pohead_released").isValid())                                                             
      _releaseDate->setDate(po.value("pohead_released").toDate(), true);
    _agent->setText(po.value("pohead_agent_username").toString());
    _status->setCurrentIndex(po.value("status").toInt());
    _printed = po.value("pohead_printed").toBool();
    _terms->setId(po.value("terms_id").toInt());
    _shipVia->setText(po.value("pohead_shipvia"));
    _fob->setText(po.value("pohead_fob"));
    _notes->setText(po.value("pohead_comments").toString());
        _so->setId(po.value("cohead_id").toInt());

        if ((po.value("cohead_id").toInt())!=-1)
        {
          _dropShip->setEnabled(true);
          _dropShip->setChecked(po.value("pohead_dropship").toBool());
        }
        else
    {
          _dropShip->setChecked(false);
      _dropShip->setEnabled(false);
        }

    _vendaddrid = po.value("vendaddrid").toInt();
    
    _vendCntct->setId(po.value("pohead_vend_cntct_id").toInt());
    _vendCntct->setHonorific(po.value("pohead_vend_cntct_honorific").toString());
    _vendCntct->setFirst(po.value("pohead_vend_cntct_first_name").toString());
    _vendCntct->setMiddle(po.value("pohead_vend_cntct_middle").toString());
    _vendCntct->setLast(po.value("pohead_vend_cntct_last_name").toString());
    _vendCntct->setSuffix(po.value("pohead_vend_cntct_suffix").toString());
    _vendCntct->setPhone(po.value("pohead_vend_cntct_phone").toString());
    _vendCntct->setTitle(po.value("pohead_vend_cntct_title").toString());
    _vendCntct->setFax(po.value("pohead_vend_cntct_fax").toString());
    _vendCntct->setEmailAddress(po.value("pohead_vend_cntct_email").toString());

        _shiptoCntct->setId(po.value("pohead_shipto_cntct_id").toInt());
    _shiptoCntct->setHonorific(po.value("pohead_shipto_cntct_honorific").toString());
    _shiptoCntct->setFirst(po.value("pohead_shipto_cntct_first_name").toString());
    _shiptoCntct->setMiddle(po.value("pohead_shipto_cntct_middle").toString());
    _shiptoCntct->setLast(po.value("pohead_shipto_cntct_last_name").toString());
    _shiptoCntct->setSuffix(po.value("pohead_shipto_cntct_suffix").toString());
    _shiptoCntct->setPhone(po.value("pohead_shipto_cntct_phone").toString());
    _shiptoCntct->setTitle(po.value("pohead_shipto_cntct_title").toString());
    _shiptoCntct->setFax(po.value("pohead_shipto_cntct_fax").toString());
    _shiptoCntct->setEmailAddress(po.value("pohead_shipto_cntct_email").toString());

    disconnect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
        if (_vendaddrid == -1)
      _vendaddrCode->setText(tr("Main"));
        else
          _vendaddrCode->setText(po.value("vendaddr_code"));
        _vendAddr->setId(_vendaddrid);
    _vendAddr->setLine1(po.value("pohead_vendaddress1").toString());
    _vendAddr->setLine2(po.value("pohead_vendaddress2").toString());
    _vendAddr->setLine3(po.value("pohead_vendaddress3").toString());
    _vendAddr->setCity(po.value("pohead_vendcity").toString());
    _vendAddr->setState(po.value("pohead_vendstate").toString());
    _vendAddr->setPostalCode(po.value("pohead_vendzipcode").toString());
    _vendAddr->setCountry(po.value("pohead_vendcountry").toString());
        connect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));

    _shiptoName->setText(po.value("pohead_shiptoname").toString());

    _shiptoAddr->setId(po.value("pohead_shiptoaddress_id").toInt());
    _shiptoAddr->setLine1(po.value("pohead_shiptoaddress1").toString());
    _shiptoAddr->setLine2(po.value("pohead_shiptoaddress2").toString());
    _shiptoAddr->setLine3(po.value("pohead_shiptoaddress3").toString());
    _shiptoAddr->setCity(po.value("pohead_shiptocity").toString());
    _shiptoAddr->setState(po.value("pohead_shiptostate").toString());
    _shiptoAddr->setPostalCode(po.value("pohead_shiptozipcode").toString());
    _shiptoAddr->setCountry(po.value("pohead_shiptocountry").toString());

    _comments->setId(_poheadid);
    _documents->setId(_poheadid);
    _charass->setId(_poheadid);
    _vendor->setId(po.value("vend_id").toInt());
    _taxZone->setId(po.value("pohead_taxzone_id").toInt());
    _poCurrency->setId(po.value("pohead_curr_id").toInt());
    _freight->setLocalValue(po.value("pohead_freight").toDouble());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting P/O"),
                                po, __FILE__, __LINE__))
    return;

  sFillList();
  emit populated();
}

void purchaseOrder::sSave()
{
  XSqlQuery purchaseSave;
  _save->setFocus();

  if(_orderNumber->hasFocus())
    sHandleOrderNumber();

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_orderNumber->text().isEmpty(), _orderNumber,
                          tr("You may not save this Purchase Order until you have entered a valid Purchase Order Number."))
         << GuiErrorCheck(!_vendor->isValid(), _vendor,
                          tr("You may not save this Purchase Order until you have selected a Vendor." ))
         << GuiErrorCheck(_metrics->boolean("RequirePOTax") && !_taxZone->isValid(), _taxZone,
                          tr("You may not save this Purchase Order until you have selected a Tax Zone." ))
         << GuiErrorCheck(!_orderDate->isValid(), _orderDate,
                          tr("You may not save this Purchase Order until you have entered a valid Purchase Order Date."))
     ;

  if (_qeitem->isDirty() && ! sQESave())
    return;

  purchaseSave.prepare( "SELECT poitem_id " 
                        "FROM poitem "
                        "WHERE (poitem_pohead_id=:pohead_id) "
                        "LIMIT 1;" );
  purchaseSave.bindValue(":pohead_id", _poheadid);
  purchaseSave.exec();
  if (!purchaseSave.first())
  {
    errors << GuiErrorCheck(true, _new,
                            tr( "You may not save this Purchase Order until you have created at least one Line Item for it." ));
  }

  purchaseSave.prepare( "SELECT pohead_status "
                        "FROM pohead "
                        "WHERE (pohead_id=:pohead_id);" );
  purchaseSave.bindValue(":pohead_id", _poheadid);
  purchaseSave.exec();
  if (purchaseSave.first())
  {
    if ((purchaseSave.value("pohead_status") == "O") && (_status->currentIndex() == 0))
    {
      errors << GuiErrorCheck(true, _status,
                              tr( "This Purchase Order has been released. You may not set its Status back to 'Unreleased'." ));
    }
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Purchase Order"), errors))
    return;

  purchaseSave.prepare( "UPDATE pohead "
             "SET pohead_warehous_id=:pohead_warehous_id, pohead_orderdate=:pohead_orderdate,"
             "    pohead_shipvia=:pohead_shipvia, pohead_taxzone_id=:pohead_taxzone_id,"
             "    pohead_freight=:pohead_freight,"
             "    pohead_fob=:pohead_fob, pohead_agent_username=:pohead_agent_username,"
             "    pohead_terms_id=:pohead_terms_id,"
             "    pohead_vendaddr_id=:pohead_vendaddr_id,"
             "    pohead_comments=:pohead_comments, "
             "    pohead_curr_id=:pohead_curr_id,"
             "    pohead_status=:pohead_status,"
             "    pohead_saved=true,"
             "    pohead_vend_cntct_id=:pohead_vend_cntct_id,"
             "    pohead_vend_cntct_honorific=:pohead_vend_cntct_honorific,"
             "    pohead_vend_cntct_first_name=:pohead_vend_cntct_first_name,"
             "    pohead_vend_cntct_middle=:pohead_vend_cntct_middle,"
             "    pohead_vend_cntct_last_name=:pohead_vend_cntct_last_name,"
             "    pohead_vend_cntct_suffix=:pohead_vend_cntct_suffix,"
             "    pohead_vend_cntct_phone=:pohead_vend_cntct_phone,"
             "    pohead_vend_cntct_title=:pohead_vend_cntct_title,"
             "    pohead_vend_cntct_fax=:pohead_vend_cntct_fax,"
             "    pohead_vend_cntct_email=:pohead_vend_cntct_email,"
             "    pohead_vendaddress1=:pohead_vendaddress1,"
             "    pohead_vendaddress2=:pohead_vendaddress2,"
             "    pohead_vendaddress3=:pohead_vendaddress3,"
             "    pohead_vendcity=:pohead_vendcity,"
             "    pohead_vendstate=:pohead_vendstate,"
             "    pohead_vendzipcode=:pohead_vendzipcode,"
             "    pohead_vendcountry=:pohead_vendcountry,"
             "    pohead_shipto_cntct_id=:pohead_shipto_cntct_id,"
             "    pohead_shipto_cntct_honorific=:pohead_shipto_cntct_honorific,"
             "    pohead_shipto_cntct_first_name=:pohead_shipto_cntct_first_name,"
             "    pohead_shipto_cntct_middle=:pohead_shipto_cntct_middle,"
             "    pohead_shipto_cntct_last_name=:pohead_shipto_cntct_last_name,"
             "    pohead_shipto_cntct_suffix=:pohead_shipto_cntct_suffix,"
             "    pohead_shipto_cntct_phone=:pohead_shipto_cntct_phone,"
             "    pohead_shipto_cntct_title=:pohead_shipto_cntct_title,"
             "    pohead_shipto_cntct_fax=:pohead_shipto_cntct_fax,"
             "    pohead_shipto_cntct_email=:pohead_shipto_cntct_email,"
             "    pohead_shiptoname=:pohead_shiptoname,"
             "    pohead_shiptoaddress_id=:pohead_shiptoaddress_id,"
             "    pohead_shiptoaddress1=:pohead_shiptoaddress1,"
             "    pohead_shiptoaddress2=:pohead_shiptoaddress2,"
             "    pohead_shiptoaddress3=:pohead_shiptoaddress3,"
             "    pohead_shiptocity=:pohead_shiptocity,"
             "    pohead_shiptostate=:pohead_shiptostate,"
             "    pohead_shiptozipcode=:pohead_shiptozipcode,"
             "    pohead_shiptocountry=:pohead_shiptocountry,"
             "    pohead_dropship=:pohead_dropship "
             "WHERE (pohead_id=:pohead_id);" );
  purchaseSave.bindValue(":pohead_id", _poheadid);
  if (_warehouse->isValid())
    purchaseSave.bindValue(":pohead_warehous_id", _warehouse->id());
  if (_taxZone->id() != -1) 
    purchaseSave.bindValue(":pohead_taxzone_id", _taxZone->id());
  purchaseSave.bindValue(":pohead_orderdate", _orderDate->date());
  purchaseSave.bindValue(":pohead_shipvia", _shipVia->currentText());
  purchaseSave.bindValue(":pohead_fob", _fob->text());
  purchaseSave.bindValue(":pohead_agent_username", _agent->currentText());
  if (_terms->isValid())
    purchaseSave.bindValue(":pohead_terms_id", _terms->id());
  if (_vendaddrid != -1)
    purchaseSave.bindValue(":pohead_vendaddr_id", _vendaddrid);
  purchaseSave.bindValue(":pohead_comments", _notes->toPlainText());
  if (_vendCntct->isValid())
    purchaseSave.bindValue(":pohead_vend_cntct_id", _vendCntct->id());
  purchaseSave.bindValue(":pohead_vend_cntct_honorific", _vendCntct->honorific());
  purchaseSave.bindValue(":pohead_vend_cntct_first_name", _vendCntct->first());
  purchaseSave.bindValue(":pohead_vend_cntct_middle", _vendCntct->middle());
  purchaseSave.bindValue(":pohead_vend_cntct_last_name", _vendCntct->last());
  purchaseSave.bindValue(":pohead_vend_cntct_suffix", _vendCntct->suffix());
  purchaseSave.bindValue(":pohead_vend_cntct_phone", _vendCntct->phone());
  purchaseSave.bindValue(":pohead_vend_cntct_title", _vendCntct->title());
  purchaseSave.bindValue(":pohead_vend_cntct_fax", _vendCntct->fax());
  purchaseSave.bindValue(":pohead_vend_cntct_email", _vendCntct->emailAddress());
  purchaseSave.bindValue(":pohead_vendaddress1", _vendAddr->line1());
  purchaseSave.bindValue(":pohead_vendaddress2", _vendAddr->line2());
  purchaseSave.bindValue(":pohead_vendaddress3", _vendAddr->line3());
  purchaseSave.bindValue(":pohead_vendcity", _vendAddr->city());
  purchaseSave.bindValue(":pohead_vendstate", _vendAddr->state());
  purchaseSave.bindValue(":pohead_vendzipcode", _vendAddr->postalCode());
  purchaseSave.bindValue(":pohead_vendcountry", _vendAddr->country());
  if (_shiptoCntct->isValid())
    purchaseSave.bindValue(":pohead_shipto_cntct_id", _shiptoCntct->id());
  purchaseSave.bindValue(":pohead_shipto_cntct_honorific", _shiptoCntct->honorific());
  purchaseSave.bindValue(":pohead_shipto_cntct_first_name", _shiptoCntct->first());
  purchaseSave.bindValue(":pohead_shipto_cntct_middle", _shiptoCntct->middle());
  purchaseSave.bindValue(":pohead_shipto_cntct_last_name", _shiptoCntct->last());
  purchaseSave.bindValue(":pohead_shipto_cntct_suffix", _shiptoCntct->suffix());
  purchaseSave.bindValue(":pohead_shipto_cntct_phone", _shiptoCntct->phone());
  purchaseSave.bindValue(":pohead_shipto_cntct_title", _shiptoCntct->title());
  purchaseSave.bindValue(":pohead_shipto_cntct_fax", _shiptoCntct->fax());
  purchaseSave.bindValue(":pohead_shipto_cntct_email", _shiptoCntct->emailAddress());
  purchaseSave.bindValue(":pohead_shiptoname", _shiptoName->text());
  if (_shiptoAddr->isValid())
    purchaseSave.bindValue(":pohead_shiptoaddress_id", _shiptoAddr->id());
  purchaseSave.bindValue(":pohead_shiptoaddress1", _shiptoAddr->line1());
  purchaseSave.bindValue(":pohead_shiptoaddress2", _shiptoAddr->line2());
  purchaseSave.bindValue(":pohead_shiptoaddress3", _shiptoAddr->line3());
  purchaseSave.bindValue(":pohead_shiptocity", _shiptoAddr->city());
  purchaseSave.bindValue(":pohead_shiptostate", _shiptoAddr->state());
  purchaseSave.bindValue(":pohead_shiptozipcode", _shiptoAddr->postalCode());
  purchaseSave.bindValue(":pohead_shiptocountry", _shiptoAddr->country());
  purchaseSave.bindValue(":pohead_freight", _freight->localValue());
  purchaseSave.bindValue(":pohead_curr_id", _poCurrency->id());
  if (_status->currentIndex() == 0)
    purchaseSave.bindValue(":pohead_status", "U");
  else if (_status->currentIndex() == 1)
    purchaseSave.bindValue(":pohead_status", "O");
  else if (_status->currentIndex() == 2)
    purchaseSave.bindValue(":pohead_status", "C");
  purchaseSave.bindValue(":pohead_dropship", QVariant(_dropShip->isChecked()));

  purchaseSave.exec();
 
  omfgThis->sPurchaseOrdersUpdated(_poheadid, true);

  if (!_pridList.isEmpty())
  {
    purchaseSave.prepare("SELECT deletePr(:pr_id) AS _result;");
    for(int i = 0; i < _pridList.size(); ++i)
    {
      purchaseSave.bindValue(":pr_id", _pridList.at(i));
      purchaseSave.exec();
    }
    omfgThis->sPurchaseRequestsUpdated();
  }

  if (_printPO->isChecked() && (_status->currentIndex() != 2)) // don't print closed
  {
    ParameterList params;
    params.append("pohead_id", _poheadid);

    printPurchaseOrder newdlgP(this, "", true);
    newdlgP.set(params);
    newdlgP.exec();
  }

  emit saved(_poheadid);

  XSqlQuery clearq;
  if (! _lock.release())
    ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                         _lock.lastError(), __FILE__, __LINE__);
  
  if (_mode == cNew && !_captive)
  {
    _purchaseOrderInformation->setCurrentIndex(0);

    _agent->setText(omfgThis->username());
    _terms->setId(-1);
    _vendor->setReadOnly(false);
    _vendor->setId(-1);
    _taxZone->setId(-1);

    _orderNumber->clear();
    _orderDate->clear();
    _shipVia->clear();
    _status->setCurrentIndex(0);
    _fob->clear();
    _notes->clear();
    _tax->clear();
    _freight->clear();
    _total->clear();
    _totalWeight->clear();
    _totalQtyOrd->clear();
    _poitem->clear();
    _poCurrency->setEnabled(true);
    _qecurrency->setEnabled(true);
    _qeitem->removeRows(0, _qeitem->rowCount());
    _vendaddrCode->clear();
    _vendCntct->clear();
    _vendAddr->clear();
    _shiptoCntct->clear();
    _shiptoName->clear();
    _shiptoAddr->clear();

    createHeader();
    _subtotal->clear();
  }
  else
  {
    close();
  }
}

void purchaseOrder::sNew()
{
  if (!saveDetail())
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("pohead_id", _poheadid);
  params.append("vend_id", _vendor->id());
  params.append("warehous_id", _warehouse->id());
  params.append("dropship", QVariant(_dropShip->isChecked()));
  if (_projectId != -1)
    params.append("prj_id", _projectId);

  purchaseOrderItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
  _vendor->setReadOnly(true);
}

void purchaseOrder::sEdit()
{
  if (!saveDetail())
    return;

  ParameterList params;
  params.append("pohead_id", _poheadid);
  params.append("vend_id", _vendor->id());
  params.append("warehous_id", _warehouse->id());
  params.append("dropship", QVariant(_dropShip->isChecked()));
  params.append("poitem_id", _poitem->id());

  if (_mode == cEdit || _mode == cNew)
    params.append("mode", "edit");
  else if (_mode == cView)
    params.append("mode", "view");

  purchaseOrderItem newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void purchaseOrder::sDelete()
{
  XSqlQuery purchaseDelete;
  if (_deleteMode == cDelete)
  {
    if (QMessageBox::question(this, tr("Delete Purchase Order Item?"),
                                    tr("<p>Are you sure you want to delete this "
                                       "Purchase Order Line Item?"),
           QMessageBox::Yes,
            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    purchaseDelete.prepare( "DELETE FROM poitem "
               "WHERE (poitem_id=:poitem_id);" );
    purchaseDelete.bindValue(":poitem_id", _poitem->id());
    purchaseDelete.exec();
    if (purchaseDelete.lastError().type() != QSqlError::NoError)
    {
      systemError(this, purchaseDelete.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    if (QMessageBox::question(this, tr("Close Purchase Order Item?"),
                                    tr("<p>Are you sure you want to close this "
                                       "Purchase Order Line Item?"),
           QMessageBox::Yes,
            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    purchaseDelete.prepare( "UPDATE poitem SET poitem_status='C' "
               "WHERE (poitem_id=:poitem_id);" );
    purchaseDelete.bindValue(":poitem_id", _poitem->id());
    purchaseDelete.exec();
    if (purchaseDelete.lastError().type() != QSqlError::NoError)
    {
      systemError(this, purchaseDelete.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillList();
}

void purchaseOrder::sVendaddrList()
{
  XSqlQuery purchaseVendaddrList;
  ParameterList params;
  params.append("vend_id", _vendor->id());

  vendorAddressList newdlg(this, "", true);
  newdlg.set(params);

  int vendaddrid;
  if ((vendaddrid = newdlg.exec()) != XDialog::Rejected)
  {
    if (vendaddrid != -1)
    {
      purchaseVendaddrList.prepare( "SELECT vendaddr_code AS code, "
                 "       vendaddr_name AS name, "
                 "       addr_line1, addr_line2, addr_line3, "
                 "       addr_city, addr_state, addr_postalcode, addr_country "
                 "FROM vendaddrinfo "
                 "LEFT OUTER JOIN addr ON (vendaddr_addr_id=addr_id) "
                 "WHERE (vendaddr_id=:vendaddr_id);" );
      purchaseVendaddrList.bindValue(":vendaddr_id", vendaddrid);

    }
    else
    {
      purchaseVendaddrList.prepare( "SELECT 'Main' AS code, "
                 "       vend_name AS name, "
                 "       addr_line1, addr_line2, addr_line3, "
                 "       addr_city, addr_state, addr_postalcode, addr_country "
                 "FROM vendinfo "
                 "LEFT OUTER JOIN addr ON (vend_addr_id=addr_id) "
                 "WHERE (vend_id=:vend_id);" );
      purchaseVendaddrList.bindValue(":vend_id", _vendor->id());
    }
      purchaseVendaddrList.exec();
      if (purchaseVendaddrList.first())
      {
        _vendaddrid = vendaddrid;
        disconnect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
        _vendaddrCode->setText(purchaseVendaddrList.value("code").toString());
        _vendAddr->setLine1(purchaseVendaddrList.value("addr_line1").toString());
        _vendAddr->setLine2(purchaseVendaddrList.value("addr_line2").toString());
        _vendAddr->setLine3(purchaseVendaddrList.value("addr_line3").toString());
        _vendAddr->setCity(purchaseVendaddrList.value("addr_city").toString());
        _vendAddr->setState(purchaseVendaddrList.value("addr_state").toString());
        _vendAddr->setPostalCode(purchaseVendaddrList.value("addr_postalcode").toString());
        _vendAddr->setCountry(purchaseVendaddrList.value("addr_country").toString());
        connect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
      }

  }
}

void purchaseOrder::sHandleDeleteButton()
{
  if ( (_mode == cNew) || (_mode == cEdit))
  {
    QTreeWidgetItem *selected = _poitem->currentItem();

    if (selected == 0)
      _delete->setEnabled(false);
    else if (_poitem->currentItem()->rawValue("poitem_status") == "U")
    {
      _deleteMode = cDelete;
      _delete->setEnabled(true);
      _delete->setText(tr("&Delete"));
    }
    else
    {
      _deleteMode = cClose;
      _delete->setEnabled(true);
      _delete->setText(tr("C&lose"));
    }
  }
}
 
void purchaseOrder::sHandleVendor(int pVendid)
{
  XSqlQuery purchaseHandleVendor;
  if ( (pVendid != -1) && (_mode == cNew) )
  {
    purchaseHandleVendor.prepare( "UPDATE pohead "
               "SET pohead_warehous_id=:pohead_warehous_id, "
               " pohead_vend_id=:pohead_vend_id, pohead_curr_id=:pohead_curr_id "
               "WHERE (pohead_id=:pohead_id);" );
    if (_warehouse->isValid())
      purchaseHandleVendor.bindValue(":pohead_warehous_id", _warehouse->id());
    purchaseHandleVendor.bindValue(":pohead_vend_id", pVendid);
    purchaseHandleVendor.bindValue(":pohead_id", _poheadid);
    purchaseHandleVendor.bindValue(":pohead_curr_id", _poCurrency->id());
    purchaseHandleVendor.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Updating Vendor Info"),
                             purchaseHandleVendor, __FILE__, __LINE__))
      return;

    XSqlQuery vq;
    vq.prepare("SELECT addr_id, addr_line1, addr_line2, addr_line3,"
               "       addr_city, addr_state, addr_postalcode, addr_country,"
               "       cntct_id, cntct_honorific, cntct_first_name,"
               "       cntct_middle, cntct_last_name, cntct_suffix,"
               "       cntct_phone, cntct_title, cntct_fax, cntct_email,"
               "       vend_terms_id, vend_curr_id,"
               "       vend_fobsource, vend_fob, vend_shipvia,"
               "       vend_name,"
               "       COALESCE(vend_addr_id, -1) AS vendaddrid,"
               "       COALESCE(vend_taxzone_id, -1) AS vendtaxzoneid,"
               "       crmacct_id"
               "  FROM vendinfo"
               "  LEFT OUTER JOIN addr ON (vend_addr_id=addr_id)"
               "  LEFT OUTER JOIN crmacct ON (vend_id=crmacct_vend_id)"
               "  LEFT OUTER JOIN cntct ON (vend_cntct1_id=cntct_id) "
               "WHERE (vend_id=:vend_id) "
               "LIMIT 1;" );
    vq.bindValue(":vend_id", pVendid);
    vq.exec();
    if (vq.first())
    {
      _taxZone->setId(vq.value("vendtaxzoneid").toInt());
      _poCurrency->setId(vq.value("vend_curr_id").toInt());
      _terms->setId(vq.value("vend_terms_id").toInt());
      _shipVia->setText(vq.value("vend_shipvia"));

      if (vq.value("vend_fobsource").toString() == "V")
      {
        _useWarehouseFOB = false;
        _fob->setText(vq.value("vend_fob"));
      }
      else
      {
        _useWarehouseFOB = true;
        _fob->setText(tr("Destination"));
      }

      if (vq.value("cntct_id").toInt())
      {
        _vendCntct->setId(vq.value("cntct_id").toInt());
        _vendCntct->setHonorific(vq.value("cntct_honorific").toString());
        _vendCntct->setFirst(vq.value("cntct_first_name").toString());
        _vendCntct->setMiddle(vq.value("cntct_middle").toString());
        _vendCntct->setLast(vq.value("cntct_last_name").toString());
        _vendCntct->setSuffix(vq.value("cntct_suffix").toString());
        _vendCntct->setPhone(vq.value("cntct_phone").toString());
        _vendCntct->setTitle(vq.value("cntct_title").toString());
        _vendCntct->setFax(vq.value("cntct_fax").toString());
        _vendCntct->setEmailAddress(vq.value("cntct_email").toString());
      }

      if (vq.value("addr_id").toInt())
      {
        _vendaddrid = -1;
        disconnect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
        _vendaddrCode->setText(tr("Main"));
        _vendAddr->setId(vq.value("addr_id").toInt());
        _vendAddr->setLine1(vq.value("addr_line1").toString());
        _vendAddr->setLine2(vq.value("addr_line2").toString());
        _vendAddr->setLine3(vq.value("addr_line3").toString());
        _vendAddr->setCity(vq.value("addr_city").toString());
        _vendAddr->setState(vq.value("addr_state").toString());
        _vendAddr->setPostalCode(vq.value("addr_postalcode").toString());
        _vendAddr->setCountry(vq.value("addr_country").toString());
        connect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
      }

      if (vq.value("crmacct_id").toInt())
        _vendCntct->setSearchAcct(vq.value("crmacct_id").toInt());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor Info"),
                                  vq, __FILE__, __LINE__))
    {
      return;
    }

    _qeitem->setHeadId(_poheadid);
  }
}

void purchaseOrder::sFillList()
{
  XSqlQuery purchaseFillList;
  MetaSQLQuery mql = mqlLoad("poItems", "list");

  ParameterList params;
  params.append("pohead_id", _poheadid);
  params.append("closed", tr("Closed"));
  params.append("unposted", tr("Unreleased"));
  params.append("partial", tr("Partial"));
  params.append("received", tr("Received"));
  params.append("open", tr("Open"));
  params.append("so", tr("SO"));
  params.append("wo", tr("WO"));

  purchaseFillList = mql.toQuery(params);
  _poitem->populate(purchaseFillList);
  if (purchaseFillList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, purchaseFillList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sCalculateTax();
  sCalculateTotals();

  _poCurrency->setEnabled(_poitem->topLevelItemCount() == 0);
  _qecurrency->setEnabled(_poitem->topLevelItemCount() == 0);

  _qeitem->select();
}

void purchaseOrder::sCalculateTotals()
{
  XSqlQuery purchaseCalculateTotals;
  purchaseCalculateTotals.prepare( "SELECT SUM(poitem_qty_ordered * poitem_unitprice) AS total,"
             "       SUM(poitem_qty_ordered * poitem_unitprice) AS f_total,"
             "       SUM(poitem_freight) AS freightsub, "
             "       SUM(poitem_qty_ordered) AS qtyord_total, "
             "       SUM(poitem_qty_ordered * (item_prodweight + item_packweight)) AS wt_total "
             "FROM poitem "
             "  LEFT OUTER JOIN itemsite ON poitem_itemsite_id = itemsite_id "
             "  LEFT OUTER JOIN item ON itemsite_item_id = item_id "
             "WHERE (poitem_pohead_id=:pohead_id);" );
  purchaseCalculateTotals.bindValue(":pohead_id", _poheadid);
  purchaseCalculateTotals.exec();
  if (purchaseCalculateTotals.first())
  {
    _totalQtyOrd->setLocalValue(purchaseCalculateTotals.value("qtyord_total").toDouble());
    _totalWeight->setLocalValue(purchaseCalculateTotals.value("wt_total").toDouble());
    _subtotal->setLocalValue(purchaseCalculateTotals.value("f_total").toDouble());
    _totalFreight->setLocalValue(purchaseCalculateTotals.value("freightsub").toDouble() + _freight->localValue());
    _total->setLocalValue(purchaseCalculateTotals.value("total").toDouble() + _tax->localValue() + purchaseCalculateTotals.value("freightsub").toDouble() + _freight->localValue());
  }
}

void purchaseOrder::sSetUserOrderNumber()
{
  _userOrderNumber = true;
}
    
void purchaseOrder::sHandleOrderNumber()
{
  XSqlQuery purchaseHandleOrderNumber;
  if(-1 != _NumberGen && _orderNumber->text().toInt() != _NumberGen)
    sReleaseNumber();

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
    purchaseHandleOrderNumber.prepare( "SELECT pohead_id "
               "FROM pohead "
               "WHERE (pohead_number=:pohead_number);" );
    purchaseHandleOrderNumber.bindValue(":pohead_number", _orderNumber->text());
    purchaseHandleOrderNumber.exec();
    if (purchaseHandleOrderNumber.first())
    {
      int poheadid = purchaseHandleOrderNumber.value("pohead_id").toInt();
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

      purchaseHandleOrderNumber.prepare( "DELETE FROM pohead "
                 "WHERE (pohead_id=:pohead_id);"
                 "DELETE FROM poitem "
                 "WHERE (poitem_pohead_id=:pohead_id);"
                 "SELECT releasePoNumber(:orderNumber);" );
      purchaseHandleOrderNumber.bindValue(":pohead_id", _poheadid);
      purchaseHandleOrderNumber.bindValue(":orderNumber", _orderNumber->text().toInt());
      purchaseHandleOrderNumber.exec();
      if (purchaseHandleOrderNumber.lastError().type() != QSqlError::NoError)
        systemError(this, purchaseHandleOrderNumber.lastError().databaseText(), __FILE__, __LINE__);

      if (! _lock.release())
        ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                             _lock.lastError(), __FILE__, __LINE__);
      
      _mode = cEdit;
      emit newMode(_mode);
      setPoheadid(poheadid);
      populate();
      _orderNumber->setEnabled(false);
      _orderDate->setEnabled(false);
      _vendor->setReadOnly(true);
    }
  }
  if(_poheadid != -1)
  {
    purchaseHandleOrderNumber.prepare("UPDATE pohead"
              "   SET pohead_number=:pohead_number"
              " WHERE (pohead_id=:pohead_id);");
    purchaseHandleOrderNumber.bindValue(":pohead_id", _poheadid);
    purchaseHandleOrderNumber.bindValue(":pohead_number", _orderNumber->text());
    purchaseHandleOrderNumber.exec();
    if (purchaseHandleOrderNumber.lastError().type() != QSqlError::NoError)
      systemError(this, purchaseHandleOrderNumber.lastError().databaseText(), __FILE__, __LINE__);
  }
}

void purchaseOrder::populateOrderNumber()
{
  XSqlQuery on;
  on.exec("SELECT fetchPoNumber() AS ponumber;");
  if (on.first())
  {
    _orderNumber->setText(on.value("ponumber"));
    _NumberGen = on.value("ponumber").toInt();
  }
  else if (on.lastError().type() != QSqlError::NoError)
  {
    systemError(this, on.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_metrics->value("PONumberGeneration") == "A")
    _orderNumber->setEnabled(false);
}

void purchaseOrder::closeEvent(QCloseEvent *pEvent)
{
  XSqlQuery purchasecloseEvent;
  if ((_mode == cNew) &&
      (_poheadid != -1) &&
      !_captive)
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

    purchasecloseEvent.prepare( "DELETE FROM pohead "
               "WHERE (pohead_id=:pohead_id);"
               "DELETE FROM poitem "
               "WHERE (poitem_pohead_id=:pohead_id);" );
    purchasecloseEvent.bindValue(":pohead_id", _poheadid);
    purchasecloseEvent.exec();
    if (purchasecloseEvent.lastError().type() != QSqlError::NoError)
      systemError(this, purchasecloseEvent.lastError().databaseText(), __FILE__, __LINE__);

    sReleaseNumber();
  }
  else if (_mode == cEdit && _poitem->topLevelItemCount() == 0)
  {
    if (QMessageBox::question(this, tr("Delete Purchase Order?"),
                              tr("<p>This Purchase Order does not have any line items.  "
                                  "Are you sure you want to delete this Purchase Order?"),
                                  QMessageBox::Yes,
                                  QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      pEvent->ignore();
      return;
    }

    purchasecloseEvent.prepare( "DELETE FROM pohead "
               "WHERE (pohead_id=:pohead_id);" );
    purchasecloseEvent.bindValue(":pohead_id", _poheadid);
    purchasecloseEvent.exec();
    if (purchasecloseEvent.lastError().type() != QSqlError::NoError)
      systemError(this, purchasecloseEvent.lastError().databaseText(), __FILE__, __LINE__);
  }

  // TODO: if sQeSave == false then find a way to return control to the user
  if (_qeitem->isDirty())
  {
    // it shouldn't even ask to save these if the screen is canceled for new PO
    if(_mode == cEdit) {
      if (QMessageBox::question(this, tr("Save Quick Entry Data?"),
                      tr("Do you want to save your Quick Entry changes?"),
                      QMessageBox::Yes | QMessageBox::Default,
                      QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
        sQESave();
    }
  }

  XSqlQuery clearq;
  if (! _lock.release())
    ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                         _lock.lastError(), __FILE__, __LINE__);
  
  XWidget::closeEvent(pEvent);
}

void purchaseOrder::sReleaseNumber()
{
  XSqlQuery purchaseReleaseNumber;
  if(-1 != _NumberGen)
  {
    purchaseReleaseNumber.prepare("SELECT releasePoNumber(:number);" );
    purchaseReleaseNumber.bindValue(":number", _NumberGen);
    purchaseReleaseNumber.exec();
    if (purchaseReleaseNumber.lastError().type() != QSqlError::NoError)
      systemError(this, purchaseReleaseNumber.lastError().databaseText(), __FILE__, __LINE__);
    _NumberGen = -1;
  }
}

bool purchaseOrder::sQESave()
{
  _qesave->setFocus();
  if (! _qeitem->submitAll())
  {
    // no need to report an error here as it will have already been reported
    // by the poitemTableModel (where it originated)
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
  if (_orderNumber->text() == "" && _purchaseOrderInformation->currentIndex() > 0)
  {
    QMessageBox::information(this, tr("Missing Order Number"), tr("Please enter a Purchase Order number before proceeding"));
    _orderNumber->setFocus();
    _purchaseOrderInformation->setCurrentIndex(0);
    return;
  }

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

void purchaseOrder::sTaxZoneChanged()
{
 if (_poheadid == -1 )
  {
    XSqlQuery taxq;
    taxq.prepare("UPDATE pohead SET "
      "  pohead_taxzone_id=:taxzone_id "
      "WHERE (pohead_id=:pohead_id) ");
    if (_taxZone->isValid())
      taxq.bindValue(":taxzone_id", _taxZone->id());
    taxq.bindValue(":pohead_id", _poheadid);
    taxq.bindValue(":freight", _freight->localValue());
    taxq.exec();
    if (taxq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sCalculateTax();
  }
}

void purchaseOrder::sCalculateTax()
{  
  XSqlQuery taxq;
  taxq.prepare( "SELECT SUM(tax) AS tax "
                "FROM ("
                "SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
                "FROM tax "
                " JOIN calculateTaxDetailSummary('PO', :pohead_id, 'T') ON (taxdetail_tax_id=tax_id)"
                "GROUP BY tax_id) AS data;" );
  taxq.bindValue(":pohead_id", _poheadid);
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (taxq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }              
}

void purchaseOrder::sTaxDetail()
{
  if (!saveDetail())
    return;

  ParameterList params;
  params.append("order_id", _poheadid);
  params.append("order_type", "PO");
  // mode => view since there are no fields to hold modified tax data
  if (_mode == cView)
    params.append("mode", "view");

  taxBreakdown newdlg(this, "", true);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    populate();
}

bool purchaseOrder::saveDetail()
{
  if (_mode == cView)
    return true;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_metrics->boolean("RequirePOTax") && !_taxZone->isValid(), _taxZone,
                          tr("You may not save this Purchase Order until you have selected a Tax Zone." ))
         << GuiErrorCheck(!_orderDate->isValid(), _orderDate,
                          tr("You may not save this Purchase Order until you have entered a valid Purchase Order Date."))
  ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Purchase Order"), errors))
    return false;

  XSqlQuery taxq;
  taxq.prepare( "UPDATE pohead "
                "SET pohead_warehous_id=:pohead_warehous_id,"
                "    pohead_vend_id=:pohead_vend_id,"
                "    pohead_number=:pohead_number,"
                "    pohead_taxzone_id=:pohead_taxzone_id, "
                "    pohead_curr_id=:pohead_curr_id, "
                "    pohead_orderdate=:pohead_orderdate, "
                "    pohead_freight = :pohead_freight "
                "WHERE (pohead_id=:pohead_id);" );
  if (_warehouse->isValid())
    taxq.bindValue(":pohead_warehous_id", _warehouse->id());
  taxq.bindValue(":pohead_vend_id", _vendor->id());
  taxq.bindValue(":pohead_number", _orderNumber->text());
  taxq.bindValue(":pohead_id", _poheadid);
  if (_taxZone->isValid())
    taxq.bindValue(":pohead_taxzone_id", _taxZone->id());
  taxq.bindValue(":pohead_curr_id", _poCurrency->id());
  taxq.bindValue(":pohead_orderdate", _orderDate->date());
  taxq.bindValue(":pohead_freight", _freight->localValue());
  taxq.exec();
  if (taxq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  return true;
}

void purchaseOrder::sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected )
{
  QAction *menuItem;

  if (pSelected->text(_poitem->column("demand_type")) == tr("SO"))
  {
    menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sViewSo()));
    menuItem->setEnabled(_privileges->check("ViewSalesOrders"));

    menuItem = pMenu->addAction(tr("Edit Sales Order..."), this, SLOT(sEditSo()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));
  }

  else if (pSelected->text(_poitem->column("demand_type")) == "WO")
  {
    menuItem = pMenu->addAction(tr("View Work Order..."), this, SLOT(sViewWo()));
    menuItem->setEnabled(_privileges->check("ViewWorkOrders")); 

    menuItem = pMenu->addAction(tr("Edit Work Order..."), this, SLOT(sEditWo()));
    menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));
  }
}

void purchaseOrder::sViewSo()
{
  XSqlQuery fetchso;
  fetchso.prepare( "SELECT coitem_cohead_id "
                   "FROM poitem JOIN coitem ON (coitem_id=poitem_order_id) "
                   "WHERE poitem_id = :poitem_id " );
  fetchso.bindValue(":poitem_id", _poitem->id());
  fetchso.exec();
  if (fetchso.first())
    salesOrder::viewSalesOrder(fetchso.value("coitem_cohead_id").toInt());
}

void purchaseOrder::sEditSo()
{
  XSqlQuery fetchso;
  fetchso.prepare( "SELECT coitem_cohead_id "
                  "FROM poitem JOIN coitem ON (coitem_id=poitem_order_id) "
                  "WHERE poitem_id = :poitem_id " );
  fetchso.bindValue(":poitem_id", _poitem->id());
  fetchso.exec();
  if (fetchso.first())
    salesOrder::editSalesOrder(fetchso.value("coitem_cohead_id").toInt(), true);
}

void purchaseOrder::sViewWo()
{
  XSqlQuery fetchwo;
  fetchwo.prepare( "SELECT womatl_wo_id "
                   "FROM pohead JOIN poitem ON (poitem_pohead_id=pohead_id AND poitem_order_type='W')"
                   "            JOIN womatl ON (womatl_id=poitem_order_id) "
                   "WHERE (pohead_id=:pohead_id);" );
  fetchwo.bindValue(":pohead_id", _poheadid);
  fetchwo.exec();
  if (fetchwo.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("wo_id", (fetchwo.value("womatl_wo_id").toInt()));

    workOrder *newdlg = new workOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void purchaseOrder::sEditWo()
{
  XSqlQuery fetchwo;
  fetchwo.prepare( "SELECT womatl_wo_id "
                   "FROM pohead JOIN poitem ON (poitem_pohead_id=pohead_id AND poitem_order_type='W')"
                   "            JOIN womatl ON (womatl_id=poitem_order_id) "
                   "WHERE (pohead_id=:pohead_id);" );
  fetchwo.bindValue(":pohead_id", _poheadid);
  fetchwo.exec();
  if (fetchwo.first())
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("wo_id", (fetchwo.value("womatl_wo_id").toInt()));

    workOrder *newdlg = new workOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void purchaseOrder::sHandleShipTo()
{
  XSqlQuery purchaseHandleShipTo;
  purchaseHandleShipTo.prepare("SELECT"
                   "       cntct_id, cntct_honorific, cntct_first_name,"
                   "       cntct_middle, cntct_last_name, cntct_suffix,"
                   "       cntct_phone, cntct_title, cntct_fax, cntct_email,"
                   "       addr_id, addr_line1, addr_line2, addr_line3,"
                   "       addr_city, addr_state, addr_postalcode, addr_country"
                   "  FROM whsinfo"
                   "  LEFT OUTER JOIN cntct ON (warehous_cntct_id=cntct_id)"
                   "  LEFT OUTER JOIN addr ON (warehous_addr_id=addr_id)"
                   " WHERE (warehous_id=:warehous_id);");
  purchaseHandleShipTo.bindValue(":warehous_id", _warehouse->id());
  purchaseHandleShipTo.exec();
  if (purchaseHandleShipTo.first())
  {
    _shiptoCntct->setId(purchaseHandleShipTo.value("cntct_id").toInt());
    _shiptoCntct->setHonorific(purchaseHandleShipTo.value("cntct_honorific").toString());
    _shiptoCntct->setFirst(purchaseHandleShipTo.value("cntct_first_name").toString());
    _shiptoCntct->setMiddle(purchaseHandleShipTo.value("cntct_middle").toString());
    _shiptoCntct->setLast(purchaseHandleShipTo.value("cntct_last_name").toString());
    _shiptoCntct->setSuffix(purchaseHandleShipTo.value("cntct_suffix").toString());
    _shiptoCntct->setPhone(purchaseHandleShipTo.value("cntct_phone").toString());
    _shiptoCntct->setTitle(purchaseHandleShipTo.value("cntct_title").toString());
    _shiptoCntct->setFax(purchaseHandleShipTo.value("cntct_fax").toString());
    _shiptoCntct->setEmailAddress(purchaseHandleShipTo.value("cntct_email").toString());

        _shiptoAddr->setId(purchaseHandleShipTo.value("addr_id").toInt());
    _shiptoAddr->setLine1(purchaseHandleShipTo.value("addr_line1").toString());
    _shiptoAddr->setLine2(purchaseHandleShipTo.value("addr_line2").toString());
    _shiptoAddr->setLine3(purchaseHandleShipTo.value("addr_line3").toString());
    _shiptoAddr->setCity(purchaseHandleShipTo.value("addr_city").toString());
    _shiptoAddr->setState(purchaseHandleShipTo.value("addr_state").toString());
    _shiptoAddr->setPostalCode(purchaseHandleShipTo.value("addr_postalcode").toString());
    _shiptoAddr->setCountry(purchaseHandleShipTo.value("addr_country").toString());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Site Info"),
                                purchaseHandleShipTo, __FILE__, __LINE__))
    return;
}

void purchaseOrder::sHandleShipToName()
{
  XSqlQuery purchaseHandleShipTo;
  purchaseHandleShipTo.prepare( "SELECT COALESCE(shipto_name,crmacct_name) AS shiptoname "
	  "FROM address left outer join shiptoinfo on (shipto_addr_id=addr_id) "
                               "WHERE (addr_id=:addr_id);" );
  purchaseHandleShipTo.bindValue(":addr_id", _shiptoAddr->id());
  purchaseHandleShipTo.exec();
  if (purchaseHandleShipTo.first())
  {
    _shiptoName->setText(purchaseHandleShipTo.value("shiptoname").toString());
  }
}
