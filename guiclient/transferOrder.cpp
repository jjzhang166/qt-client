/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "transferOrder.h"

#include <stdlib.h>

#include <QAction>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "distributeInventory.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "issueLineToShipping.h"
#include "transferOrderItem.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"

const char *_statusTypes[] = { "U", "O", "C" };

#define cClosed       0x01
#define cActiveOpen   0x02
#define cInactiveOpen 0x04
#define cCanceled     0x08
#define cUnreleased   0x16

transferOrder::transferOrder(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl),
      _cachedTabIndex(0),
      _captive(false),
      _ignoreSignals(false),
      _lineMode(0),
      _mode(cView),
      _orderNumberGen(0),
      _qeitem(0),
      _saved(false),
      _taxzoneidCache(-1),
      _toheadid(-1),
      _userEnteredOrderNumber(false),
      _whstaxzoneid(-1)
{
  setupUi(this);

  _qeitem = new ToitemTableModel(this);
  _qeitemView->setModel(_qeitem);

  connect(_action,	    SIGNAL(clicked()), this, SLOT(sAction()));
  connect(_clear,	    SIGNAL(pressed()), this, SLOT(sClear()));
  connect(_delete,	    SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_dstWhs,         SIGNAL(newID(int)), this, SLOT(sHandleDstWhs(int)));
  connect(_edit,	    SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_freightCurrency, SIGNAL(newID(int)), this, SLOT(sCurrencyChanged()));
  connect(_issueLineBalance, SIGNAL(clicked()), this, SLOT(sIssueLineBalance()));
  connect(_issueStock,	     SIGNAL(clicked()), this, SLOT(sIssueStock()));
  connect(_new,		     SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_orderDate,	SIGNAL(newDate(QDate)), _qeitem, SLOT(setTransDate(QDate)));
  connect(_orderNumber,    SIGNAL(editingFinished()), this, SLOT(sHandleOrderNumber()));
  connect(_orderNumber,	SIGNAL(textChanged(const QString&)), this, SLOT(sSetUserEnteredOrderNumber()));
  connect(_packDate,	SIGNAL(newDate(QDate)), _qeitem, SLOT(setShipDate(QDate)));
  connect(_qecurrency,	   SIGNAL(newID(int)), this, SLOT(sCurrencyChanged()));
  connect(_qedelete,	    SIGNAL(clicked()), this, SLOT(sQEDelete()));
  connect(_qesave,	    SIGNAL(clicked()), this, SLOT(sQESave()));
  connect(_save,	    SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_saveAndAdd,	    SIGNAL(clicked()), this, SLOT(sSaveAndAdd()));
  connect(_shipDate,   SIGNAL(newDate(QDate)), _qeitem, SLOT(setShipDate(QDate)));
  connect(_showCanceled,SIGNAL(toggled(bool)), this, SLOT(sFillItemList()));
  connect(_srcWhs,         SIGNAL(newID(int)), this, SLOT(sHandleSrcWhs(int)));
  connect(_tabs,  SIGNAL(currentChanged(int)), this, SLOT(sTabChanged(int)));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_taxzone,        SIGNAL(newID(int)), this, SLOT(sCalculateTax()));
  connect(_toitem, SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtons()));
  connect(_toitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_trnsWhs,  SIGNAL(newID(int)), this, SLOT(sHandleTrnsWhs(int)));
  connect(omfgThis, SIGNAL(transferOrdersUpdated(int)), this, SLOT(sHandleTransferOrderEvent(int)));

  _toitem->addColumn(tr("#"),           _seqColumn,     Qt::AlignCenter, true,  "toitem_linenumber" );
  _toitem->addColumn(tr("Item"),        _itemColumn,    Qt::AlignLeft,   true,  "item_number"   );
  _toitem->addColumn(tr("Description"), -1,             Qt::AlignLeft,   true,  "description"   );
  _toitem->addColumn(tr("Status"),      _statusColumn,  Qt::AlignCenter, true,  "toitem_status" );
  _toitem->addColumn(tr("Sched. Date"), _dateColumn,    Qt::AlignCenter, true,  "toitem_schedshipdate" );
  _toitem->addColumn(tr("Ordered"),     _qtyColumn,     Qt::AlignRight,  true,  "toitem_qty_ordered"  );
  _toitem->addColumn(tr("At Shipping"), _qtyColumn,     Qt::AlignRight,  true,  "atshipping"  );
  _toitem->addColumn(tr("Shipped"),     _qtyColumn,     Qt::AlignRight,  true,  "toitem_qty_shipped"  );
  _toitem->addColumn(tr("Balance"),     _qtyColumn,     Qt::AlignRight,  true,  "balance"  );
  _toitem->addColumn(tr("Received"),    _qtyColumn,     Qt::AlignRight,  true,  "toitem_qty_received" );

  _orderNumber->setValidator(omfgThis->orderVal());

  if(!_metrics->boolean("UseProjects"))
  {
    _projectLit->hide();
    _project->hide();
  }

  _mode = cView;

  setToheadid(-1);

  _srcWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  _trnsWhs->setId(_metrics->value("DefaultTransitWarehouse").toInt());
  _dstWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  getWhsInfo(_trnsWhs->id(), _trnsWhs);

  _weight->setValidator(omfgThis->qtyVal());
  
  // TODO: remove when 5695 is fixed
  _freight->hide();
  _freightLit->hide();
  // end 5695
}

void transferOrder::setToheadid(const int pId)
{
  _toheadid = pId;
  _qeitem->setHeadId(pId);
  _comments->setId(_toheadid);
  _documents->setId(_toheadid);
       
  if (_mode == cEdit
      && ! _lock.acquire("tohead", _toheadid, AppLock::Interactive))
  {
    setViewMode();
  }
}

transferOrder::~transferOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void transferOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse transferOrder::set(const ParameterList &pParams)
{
  XSqlQuery transferet;
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  int      itemid;
  int      srcwarehousid = -1;
  int      destwarehousid = -1;
  double   qty;
  QDate    dueDate;
  
  setToheadid(-1);
  int      _planordid = -1;

  param = pParams.value("planord_id", &valid);
  if (valid)
    _planordid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      setObjectName("transferOrder new");
      _mode = cNew;
      if ( (_metrics->value("TONumberGeneration") == "A") ||
           (_metrics->value("TONumberGeneration") == "O")   )
      {
        if (! insertPlaceholder())
          return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      if(_metrics->boolean("AlwaysShowSaveAndAdd"))
        _saveAndAdd->setEnabled(true);
      else
        _saveAndAdd->hide();
    }
    else if (param.toString() == "view")
      setViewMode();
    else if (param.toString() == "releaseTO")
    {
      _mode = cNew;
      transferet.prepare( "SELECT planord.*,"
                 "       srcsite.itemsite_item_id AS itemid,"
                 "       srcsite.itemsite_warehous_id AS srcwarehousid,"
                 "       destsite.itemsite_warehous_id AS destwarehousid "
                 "FROM planord JOIN itemsite srcsite  ON (srcsite.itemsite_id=planord_supply_itemsite_id) "
                 "             JOIN itemsite destsite ON (destsite.itemsite_id=planord_itemsite_id) "
                 "WHERE (planord_id=:planord_id);" );
      transferet.bindValue(":planord_id", _planordid);
      transferet.exec();
      if (transferet.first())
      {
        itemid = transferet.value("itemid").toInt();
        qty = transferet.value("planord_qty").toDouble();
        dueDate = transferet.value("planord_duedate").toDate();
        srcwarehousid = transferet.value("srcwarehousid").toInt();
        destwarehousid = transferet.value("destwarehousid").toInt();
      }
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }

      connect(_toitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_toitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_toitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      transferet.prepare( "SELECT tohead_id "
                 "FROM tohead "
                 "WHERE ( (tohead_status='U')"
                 "  AND   (tohead_src_warehous_id=:tohead_src_warehous_id) "
                 "  AND   (tohead_dest_warehous_id=:tohead_dest_warehous_id) ) "
                 "ORDER BY tohead_number "
                 "LIMIT 1;" );
      transferet.bindValue(":tohead_src_warehous_id", srcwarehousid);
      transferet.bindValue(":tohead_dest_warehous_id", destwarehousid);
      transferet.exec();
      if (transferet.first())
      {
//  Transfer order found
        if(QMessageBox::question( this, tr("Unreleased Transfer Order Exists"),
                                        tr("An Unreleased Transfer Order\n"
                                           "already exists for this\n"
                                           "Source/Destination Warehouse.\n"
                                           "Would you like to use this Transfer Order?\n"
                                           "Click Yes to use the existing Transfer Order\n"
                                           "otherwise a new one will be created."),
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
        {
//  Use an existing order
          _mode = cEdit;

          setToheadid(transferet.value("tohead_id").toInt());
          _orderNumber->setEnabled(false);
          _orderDate->setEnabled(false);
          populate();
        }
        else
        {
//  Do not use an existing order, create new
          _srcWhs->setId(srcwarehousid);
          _dstWhs->setId(destwarehousid);
          insertPlaceholder();
        }
      }
      else
      {
//  Transfer order not found, create new
        _srcWhs->setId(srcwarehousid);
        _dstWhs->setId(destwarehousid);
        insertPlaceholder();
      }

//  Start to create the new toitem
      ParameterList newItemParams;
      newItemParams.append("mode", "new");
      newItemParams.append("tohead_id", _toheadid);
      newItemParams.append("srcwarehouse_id", _srcWhs->id());
      newItemParams.append("orderNumber",	_orderNumber->text());
      newItemParams.append("orderDate",	_orderDate->date());
      newItemParams.append("taxzone_id",	_taxzone->id());
      newItemParams.append("curr_id",	_freightCurrency->id());
      newItemParams.append("item_id", itemid);
      newItemParams.append("captive", true);

      if (qty > 0.0)
        newItemParams.append("qty", qty);

      if (!dueDate.isNull())
        newItemParams.append("dueDate", dueDate);

      transferOrderItem toItem(this, "", true);
      toItem.set(newItemParams);
// TODO
//      if (toItem.exec() != XDialog::Rejected)
      if (toItem.exec() == XDialog::Rejected)
      {
        transferet.prepare("SELECT deletePlannedOrder(:planord_id, false) AS _result;");
        transferet.bindValue(":planord_id", _planordid);
        transferet.exec();
// TODO
//        omfgThis->sPlannedOrdersUpdated();
        if(_mode == cEdit)
        {
          // check for another open window
          QWidgetList list = omfgThis->windowList();
          for(int i = 0; i < list.size(); i++)
          {
            QWidget * w = list.at(i);
            if(QString::compare(w->metaObject()->className(), "transferOrder") &&
               w != this)
            {
              transferOrder *other = (transferOrder*)w;
              if(_toheadid == other->_toheadid)
              {
                other->sFillItemList();
                return UndefinedError;
              }
            }
          }
        }
        sFillItemList();
      }
      else
        _planordid = -1;
    }
  }

  param = pParams.value("src_warehous_id", &valid);
  if (valid)
    _srcWhs->setId(param.toInt());

  param = pParams.value("dst_warehous_id", &valid);
  if (valid)
    _dstWhs->setId(param.toInt());

  if (cNew == _mode)
  {
    _status->setCurrentIndex(0);

    // TODO: Why don't the constructor or setId earlier in set() handle this?
    getWhsInfo(_srcWhs->id(), _srcWhs);
    getWhsInfo(_dstWhs->id(), _dstWhs);

    _captive = false;
    _edit->setEnabled(false);
    _action->setEnabled(false);
    _delete->setEnabled(false);
    _close->setText("&Cancel");
  }
  else if (cEdit == _mode)
  {
    _captive = true;
    _orderNumber->setEnabled(false);
  }

  if( !_metrics->boolean("EnableTOShipping"))
  {
    _requireInventory->hide();
    _issueStock->hide();
    _issueLineBalance->hide();
    _toitem->hideColumn(7);
    _toitem->hideColumn(8);
  }
  else
    _toitem->setSelectionMode(QAbstractItemView::ExtendedSelection);

  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    setToheadid(param.toInt());
    if(cEdit == _mode)
      setObjectName(QString("transferOrder edit %1").arg(_toheadid));
    else if(cView == _mode)
      setObjectName(QString("transferOrder view %1").arg(_toheadid));
    populate();
  }

  if ( pParams.inList("enableSaveAndAdd") &&
       ((_mode == cNew) || (_mode == cEdit)) )
  {
    _saveAndAdd->show();
    _saveAndAdd->setEnabled(true);
  }

  if (cNew == _mode || cEdit == _mode)
  {
    _orderDate->setEnabled(_privileges->check("OverrideTODate"));
    _packDate->setEnabled(_privileges->check("AlterPackDate"));
  }
  else
  {
    _orderDate->setEnabled(false);
    _packDate->setEnabled(false);
  }

  param = pParams.value("captive", &valid);
  if (valid)
    _captive = true;

  return NoError;
}

int transferOrder::id() const
{
  return _toheadid;
}

int transferOrder::mode() const
{
  return _mode;
}

bool transferOrder::insertPlaceholder()
{
  XSqlQuery transferinsertPlaceholder;
  if(_trnsWhs->id() == -1)
  {
    QMessageBox::critical(this, tr("No Transit Site"),
      tr("There are no transit sites defined in the system."
         " You must define at least one transit site to use Transfer Orders.") );
    return false;
  }

  _ignoreSignals = true;
  populateOrderNumber();
  _ignoreSignals = false;

  transferinsertPlaceholder.prepare("INSERT INTO tohead ("
        "          tohead_number, tohead_src_warehous_id,"
	    "          tohead_trns_warehous_id, tohead_dest_warehous_id,"
	    "          tohead_status, tohead_shipform_id"
	    ") VALUES ("
        "          :tohead_number, :tohead_src_warehous_id,"
	    "          :tohead_trns_warehous_id, :tohead_dest_warehous_id,"
	    "          :tohead_status, :tohead_shipform_id)"
            " RETURNING tohead_id;");

  if (_orderNumber->text().isEmpty())
    transferinsertPlaceholder.bindValue(":tohead_number", QString::number(_toheadid * -1));
  else
    transferinsertPlaceholder.bindValue(":tohead_number",	  _orderNumber->text());
  transferinsertPlaceholder.bindValue(":tohead_src_warehous_id",  _srcWhs->id());
  transferinsertPlaceholder.bindValue(":tohead_trns_warehous_id", _trnsWhs->id());
  transferinsertPlaceholder.bindValue(":tohead_dest_warehous_id", _dstWhs->id());

  transferinsertPlaceholder.bindValue(":tohead_status", "U");
  
  if (_shippingForm->isValid())
    transferinsertPlaceholder.bindValue(":tohead_shipform_id",	  _shippingForm->id());

  transferinsertPlaceholder.exec();
  if (transferinsertPlaceholder.first())
  {
    setToheadid(transferinsertPlaceholder.value("tohead_id").toInt());
    _orderDate->setDate(omfgThis->dbDate(), true);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Header"),
                                transferinsertPlaceholder, __FILE__, __LINE__))
  {
    return false;
  }

  return true;
}

void transferOrder::sSaveAndAdd()
{
  XSqlQuery transferSaveAndAdd;
  if (save(false))
  {
    transferSaveAndAdd.prepare("SELECT addToPackingListBatch('TO', :tohead_id) AS result;");
    transferSaveAndAdd.bindValue(":tohead_id", _toheadid);
    transferSaveAndAdd.exec();
    if (transferSaveAndAdd.first())
      ; // nothing to do
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Error Adding to Packing List Batch"),
                                  transferSaveAndAdd, __FILE__, __LINE__))
      return;

    sReleaseTohead();
    if (_captive)
      close();
    else
      clear();
  }
}

void transferOrder::sSave()
{
  if (save(false))
  {
    sReleaseTohead();
    if (_captive)
      close();
    else
      clear();
  }
}

bool transferOrder::save(bool partial)
{
  XSqlQuery transferave;
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_srcWhs->id() == -1, _srcWhs,
                          tr("You must select a Source Site for this "
                             "Transfer Order before you may save it."))
         << GuiErrorCheck(_trnsWhs->id() == -1,	// but see fr 5581
                          _dstWhs,
                          tr("You must select a Transit Site for "
                             "this Transfer Order before you may save it."))
         << GuiErrorCheck(_dstWhs->id() == -1, _dstWhs,
                          tr("You must select a Destination Site for "
                             "this Transfer Order before you may save it."))
         << GuiErrorCheck(_srcWhs->id() == _dstWhs->id(), _dstWhs,
                          tr("The Source and Destination Sites "
                             "must be different."))
         << GuiErrorCheck((!partial && _toitem->topLevelItemCount() == 0), _new,
                          tr("You must create at least one Line Item for "
                             "this Transfer Order before you may save it."))
         << GuiErrorCheck(_orderNumber->text().toInt() == 0, _orderNumber,
                          tr("You must enter a valid T/O # for this Transfer"
                             "Order before you may save it."))
  ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Transfer Order"), errors))
    return false;

  if (_qeitem->isDirty() && ! sQESave())
    return false;

  // place holder for automatic project creation
  // if(! partial && _metrics->boolean("AutoCreateProjectsForOrders") &&
  //    (_project->id() == -1) && (cNew == _mode))

  // save contact and address info in case someone wants to use 'em again later
  // but don't make any global changes to the data and ignore errors
  // TODO: put in real checking - perhaps the address of the warehouse is wrong
  _ignoreSignals = true;
  _srcAddr->save(AddressCluster::CHANGEONE);
  _dstAddr->save(AddressCluster::CHANGEONE);
  _ignoreSignals = false;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  transferave.exec("BEGIN;");
  transferave.prepare("UPDATE tohead "
	    "SET tohead_number=:number,"
	    "    tohead_status=:status,"
	    "    tohead_orderdate=:orderdate,"
	    "    tohead_src_warehous_id=:src_warehous_id,"
	    "    tohead_srcname=:srcname,"
	    "    tohead_srcaddress1=:srcaddress1,"
	    "    tohead_srcaddress2=:srcaddress2,"
	    "    tohead_srcaddress3=:srcaddress3,"
	    "    tohead_srccity=:srccity,"
	    "    tohead_srcstate=:srcstate,"
	    "    tohead_srcpostalcode=:srcpostalcode,"
	    "    tohead_srccountry=:srccountry,"
	    "    tohead_srccntct_id=:srccntct_id,"
	    "    tohead_srccntct_name=:srccntct_name,"
	    "    tohead_srcphone=:srcphone,"
	    "    tohead_trns_warehous_id=:trns_warehous_id,"
	    "    tohead_trnsname=:trnsname,"
	    "    tohead_dest_warehous_id=:dest_warehous_id,"
	    "    tohead_destname=:destname,"
	    "    tohead_destaddress1=:destaddress1,"
	    "    tohead_destaddress2=:destaddress2,"
	    "    tohead_destaddress3=:destaddress3,"
	    "    tohead_destcity=:destcity,"
	    "    tohead_deststate=:deststate,"
	    "    tohead_destpostalcode=:destpostalcode,"
	    "    tohead_destcountry=:destcountry,"
	    "    tohead_destcntct_id=:destcntct_id,"
	    "    tohead_destcntct_name=:destcntct_name,"
	    "    tohead_destphone=:destphone,"
	    "    tohead_agent_username=:agent_username,"
	    "    tohead_shipvia=:shipvia,"
	    "    tohead_shipform_id=:shipform_id,"
	    "    tohead_taxzone_id=:taxzone_id,"
	    "    tohead_freight=:freight,"
	    "    tohead_freight_curr_id=:freight_curr_id,"
	    "    tohead_shipcomplete=:shipcomplete,"
	    "    tohead_ordercomments=:ordercomments,"
	    "    tohead_shipcomments=:shipcomments,"
	    "    tohead_packdate=:packdate,"
	    "    tohead_prj_id=:prj_id,"
	    "    tohead_lastupdated=CURRENT_TIMESTAMP "
	   "WHERE (tohead_id=:id);" );

  transferave.bindValue(":id", _toheadid );

  transferave.bindValue(":number",		_orderNumber->text().toInt());
  transferave.bindValue(":status",    _statusTypes[_status->currentIndex()]);
  transferave.bindValue(":orderdate",		_orderDate->date());
  transferave.bindValue(":src_warehous_id",	_srcWhs->id());
  transferave.bindValue(":srcname",		_srcWhs->currentText());
  transferave.bindValue(":srcaddress1",		_srcAddr->line1());
  transferave.bindValue(":srcaddress2",		_srcAddr->line2());
  transferave.bindValue(":srcaddress3",		_srcAddr->line3());
  transferave.bindValue(":srccity",		_srcAddr->city());
  transferave.bindValue(":srcstate",		_srcAddr->state());
  transferave.bindValue(":srcpostalcode",		_srcAddr->postalCode());
  transferave.bindValue(":srccountry",		_srcAddr->country());
  if (_srcContact->id() > 0)
    transferave.bindValue(":srccntct_id",		_srcContact->id());
  transferave.bindValue(":srccntct_name",		_srcContact->name());
  transferave.bindValue(":srcphone",		_srcContact->phone());

  transferave.bindValue(":trns_warehous_id",	_trnsWhs->id());
  transferave.bindValue(":trnsname",		_trnsWhs->currentText());

  transferave.bindValue(":dest_warehous_id",	_dstWhs->id());
  transferave.bindValue(":destname",		_dstWhs->currentText());
  transferave.bindValue(":destaddress1",		_dstAddr->line1());
  transferave.bindValue(":destaddress2",		_dstAddr->line2());
  transferave.bindValue(":destaddress3",		_dstAddr->line3());
  transferave.bindValue(":destcity",		_dstAddr->city());
  transferave.bindValue(":deststate",		_dstAddr->state());
  transferave.bindValue(":destpostalcode",	_dstAddr->postalCode());
  transferave.bindValue(":destcountry",		_dstAddr->country());
  if (_dstContact->id() > 0)
    transferave.bindValue(":destcntct_id",	_dstContact->id());
  transferave.bindValue(":destcntct_name",	_dstContact->name());
  transferave.bindValue(":destphone",		_dstContact->phone());

  transferave.bindValue(":agent_username",	_agent->currentText());
  transferave.bindValue(":shipvia",		_shipVia->currentText());

  if (_taxzone->isValid())
    transferave.bindValue(":taxzone_id",		_taxzone->id());

  transferave.bindValue(":freight",		_freight->localValue());
  transferave.bindValue(":freight_curr_id",	_freight->id());

  transferave.bindValue(":shipcomplete",	QVariant(_shipComplete->isChecked()));
  transferave.bindValue(":ordercomments",		_orderComments->toPlainText());
  transferave.bindValue(":shipcomments",		_shippingComments->toPlainText());

  if (_packDate->isValid())
    transferave.bindValue(":packdate", _packDate->date());
  else
    transferave.bindValue(":packdate", _orderDate->date());

  if (_project->isValid())
    transferave.bindValue(":prj_id",		_project->id());

  if (_shippingForm->isValid())
    transferave.bindValue(":shipform_id",		_shippingForm->id());

  transferave.exec();
  if (transferave.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Transfer Order"),
                         transferave, __FILE__, __LINE__);
    return false;
  }

  // TODO: should this be done before saving the t/o?
  if ((cNew == _mode) && (!_saved)
      && ! _lock.acquire("tohead", _toheadid, AppLock::Silent))
  {
    rollback.exec();
    (void)ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                               _lock.lastError(), __FILE__, __LINE__);
    return false;
  }

  // keep the status of toitems in synch with tohead
  if (_statusTypes[_status->currentIndex()] == QString("U"))
  {
    transferave.prepare("SELECT unreleaseTransferOrder(:head_id) AS result;");
    transferave.bindValue(":head_id", _toheadid);
    transferave.exec();
    if (transferave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Transfer Order"),
                           transferave, __FILE__, __LINE__);
      return false;
    }
  }
  else if (_statusTypes[_status->currentIndex()] == QString("O"))
  {
    transferave.prepare("SELECT releaseTransferOrder(:head_id) AS result;");
    transferave.bindValue(":head_id", _toheadid);
    transferave.exec();
    if (transferave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Transfer Order"),
                           transferave, __FILE__, __LINE__);
      return false;
    }
  }
  else if (_statusTypes[_status->currentIndex()] == QString("C"))
  {
    transferave.prepare("SELECT closeTransferOrder(:head_id) AS result;");
    transferave.bindValue(":head_id", _toheadid);
    transferave.exec();
    if (transferave.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Transfer Order"),
                           transferave, __FILE__, __LINE__);
      return false;
    }
  }

  transferave.exec("COMMIT;");
  _saved = true;
  omfgThis->sTransferOrdersUpdated(_toheadid);

  emit saved(_toheadid);

  return true;
}

void transferOrder::sPopulateMenu(QMenu *pMenu)
{
  if ((_mode == cNew) || (_mode == cEdit))
  {
    int _numSelected = _toitem->selectedItems().size();
    if(_numSelected == 1)
    {
      if (_lineMode == cClosed)
        pMenu->addAction(tr("Open Line..."), this, SLOT(sAction()));
      else if (_lineMode == cActiveOpen)
      {
        pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
        pMenu->addAction(tr("Close Line..."), this, SLOT(sAction()));
      }
      else if (_lineMode == cInactiveOpen)
      {
        pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
        pMenu->addAction(tr("Close Line..."), this, SLOT(sAction()));
        pMenu->addAction(tr("Delete Line..."), this, SLOT(sDelete()));
      }
      else if (_lineMode == cUnreleased)
      {
        pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
        pMenu->addAction(tr("Delete Line..."), this, SLOT(sDelete()));
      }
    }

    if(_metrics->boolean("EnableTOShipping"))
    {
      if(_numSelected == 1)
        pMenu->addSeparator();

      pMenu->addAction(tr("Return Stock"), this, SLOT(sReturnStock()));
      pMenu->addAction(tr("Issue Stock..."), this, SLOT(sIssueStock()));
      pMenu->addAction(tr("Issue Line Balance"), this, SLOT(sIssueLineBalance()));
    }
  }
}
 
void transferOrder::populateOrderNumber()
{
  XSqlQuery transferpopulateOrderNumber;
  if (_mode == cNew)
  {
    if ( (_metrics->value("TONumberGeneration") == "A") ||
         (_metrics->value("TONumberGeneration") == "O")   )
    {
      transferpopulateOrderNumber.exec("SELECT fetchToNumber() AS tonumber;");
      if (transferpopulateOrderNumber.first())
      {
        _orderNumber->setText(transferpopulateOrderNumber.value("tonumber"));
        _orderNumberGen = transferpopulateOrderNumber.value("tonumber").toInt();

        if (_metrics->value("TONumberGeneration") == "A")
          _orderNumber->setEnabled(false);
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Number"),
                                    transferpopulateOrderNumber, __FILE__, __LINE__))
        return;
    }
    _new->setEnabled(! _orderNumber->text().isEmpty());
  }

  _userEnteredOrderNumber = false;
}

void transferOrder::sSetUserEnteredOrderNumber()
{
  _userEnteredOrderNumber = true;
}
    
void transferOrder::sHandleOrderNumber()
{
  if (_ignoreSignals || !isActiveWindow())
    return;

  if (_orderNumber->text().length() == 0)
  {

    if (_mode == cNew)
    {
      if ( (_metrics->value("TONumberGeneration") == "A") ||
           (_metrics->value("TONumberGeneration") == "O") )
        populateOrderNumber();
      else
      {
        if(!_close->hasFocus())
        {
        QMessageBox::warning( this, tr("Enter T/O #"),
                              tr("<p>You must enter a T/O # for this Transfer "
				 "Order before you may continue." ) );
        _orderNumber->setFocus();
        }
        return;
      }
    }

  }
  else
  {
    XSqlQuery query;
    if ( (_mode == cNew) && (_userEnteredOrderNumber) )
    {
      query.prepare( "SELECT tohead_id "
                     "FROM tohead "
                     "WHERE (tohead_number=:tohead_number);" );
      query.bindValue(":tohead_number", _orderNumber->text());
      query.exec();
      if (query.first())
      {
        _mode = cEdit;
        setToheadid(query.value("tohead_id").toInt());
        populate();
        _orderNumber->setEnabled(false);
      }
      else
      {
        if ((_metrics->value("TONumberGeneration") != "A") && 
            (_metrics->value("TONumberGeneration") != "O") &&
            ! insertPlaceholder())
        {
          _orderNumber->clear();
          return;
        }
        QString orderNumber = _orderNumber->text();
        query.prepare( "SELECT releaseToNumber(:orderNumber) AS result;" );
        query.bindValue(":orderNumber", _orderNumberGen);
        query.exec();
	if (query.first())
          ; // nothing to do
	else if (ErrorReporter::error(QtCriticalMsg, this, tr("Releasing Error"),
                                      query, __FILE__, __LINE__))
	  return;

        _orderNumber->setText(orderNumber);
        _userEnteredOrderNumber = false;
        _orderNumber->setEnabled(false);
      }
    }

  }

  if (cView != _mode)
    _new->setEnabled(! _orderNumber->text().isEmpty());
  
  sHandleTrnsWhs(_trnsWhs->id());
  sHandleSrcWhs(_srcWhs->id());
}

void transferOrder::sNew()
{
  if( !_saved && ((_mode == cNew) ) )
    if(!save(true))
      return;

  ParameterList params;
  params.append("tohead_id",	_toheadid);
  params.append("srcwarehouse_id", _srcWhs->id());
  params.append("orderNumber",	_orderNumber->text());
  params.append("orderDate",	_orderDate->date());
  params.append("taxzone_id",	_taxzone->id());
  params.append("curr_id",	_freightCurrency->id());

  if ((_mode == cNew) || (_mode == cEdit))
    params.append("mode", "new");

  transferOrderItem newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillItemList();
}

void transferOrder::sEdit()
{
  ParameterList params;
  params.append("toitem_id", _toitem->id());

  if (_mode == cView)
    params.append("mode", "view");
  else if ((_mode == cNew) || (_mode == cEdit))
    params.append("mode", "edit");

  transferOrderItem newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted &&
      ((_mode == cNew) || (_mode == cEdit)) )
  {
    sFillItemList();
    sCalculateTax();
  }
    
}

void transferOrder::sHandleButtons()
{
  XTreeWidgetItem *selected = 0;
  QList<XTreeWidgetItem*> selectedlist = _toitem->selectedItems();
  int _numSelected = selectedlist.size();
  if (_numSelected > 0)
    selected = (XTreeWidgetItem*)(selectedlist[0]);
  
  if (selected)
  {
    _issueStock->setEnabled(_privileges->check("IssueStockToShipping"));
    _issueLineBalance->setEnabled(_privileges->check("IssueStockToShipping"));

    if(_numSelected == 1)
    {
      _edit->setEnabled(true);
      int lineMode = selected->altId();

      if (lineMode == 1)
      {
        _lineMode = cClosed;

        _action->setText(tr("Open"));
        _action->setEnabled(true);
        _delete->setEnabled(false);
      }
      else if (lineMode == 2)
      {
        _lineMode = cActiveOpen;

        _action->setText(tr("Close"));
        _action->setEnabled(true);
        _delete->setEnabled(false);
      }
      else if (lineMode == 3)
      {
        _lineMode = cInactiveOpen;

        _action->setText(tr("Close"));
        _action->setEnabled(true);
        _delete->setEnabled(true);
      }
      else if (lineMode == 4)
      {
        _lineMode = cCanceled;

        _action->setEnabled(false);
        _delete->setEnabled(false);
      }
      else if (lineMode == 5)
      {
        _lineMode = cUnreleased;

        _action->setText(tr("Close"));
        _action->setEnabled(false);
        _delete->setEnabled(true);
      }
      else
      {
        _action->setEnabled(false);
        _delete->setEnabled(false);
      }

      if(1 == lineMode || 4 == lineMode || 5 == lineMode)
      {
        _issueStock->setEnabled(false);
        _issueLineBalance->setEnabled(false);
      }
    }
    else
    {
      _edit->setEnabled(false);
      _action->setEnabled(false);
      _delete->setEnabled(false);
    }
  }
  else
  {
    _edit->setEnabled(false);
    _action->setEnabled(false);
    _delete->setEnabled(false);
    _issueStock->setEnabled(false);
    _issueLineBalance->setEnabled(false);
  }
}

void transferOrder::sAction()
{
  XSqlQuery transferAction;
  if (_lineMode == cCanceled)
    return;

  if ( (_mode == cNew) || (_mode == cEdit) )
  {
    if (_lineMode == cClosed)
    {
      transferAction.prepare( "UPDATE toitem "
                 "SET toitem_status=:toitem_status "
                 "WHERE (toitem_id=:toitem_id);" );
      transferAction.bindValue(":toitem_status", _statusTypes[_status->currentIndex()]);
      transferAction.bindValue(":toitem_id", _toitem->id());
      transferAction.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Opening Line"),
                               transferAction, __FILE__, __LINE__))
        return;
    }
    else
    {
      transferAction.prepare( "SELECT closeToItem(:toitem_id) AS result;" );
      transferAction.bindValue(":toitem_id", _toitem->id());
      transferAction.exec();
      if (transferAction.first())
      {
        int result = transferAction.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("closeToItem", result),
                      __FILE__, __LINE__);
          return;
        }
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Closing Line"),
                                    transferAction, __FILE__, __LINE__))
        return;
    }
    sFillItemList();

    transferAction.prepare("SELECT tohead_status FROM tohead WHERE (tohead_id=:tohead_id);");
    transferAction.bindValue(":tohead_id", _toheadid);
    transferAction.exec();
    if (transferAction.first())
    {
      if (transferAction.value("tohead_status").toString() == "U")
        _status->setCurrentIndex(0);
      else if (transferAction.value("tohead_status").toString() == "O")
        _status->setCurrentIndex(1);
      else if (transferAction.value("tohead_status").toString() == "C")
        _status->setCurrentIndex(2);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Status"),
                                  transferAction, __FILE__, __LINE__))
      return;
  }
}

void transferOrder::sDelete()
{ 
  XSqlQuery transferDelete;
  if ( (_mode == cEdit) || (_mode == cNew) )
  {
    if (QMessageBox::question(this, tr("Delete Selected Line Item?"),
                              tr("<p>Are you sure that you want to delete the "
                                 "selected Line Item?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    {
      transferDelete.prepare( "DELETE FROM toitem "
                 "WHERE (toitem_id=:toitem_id);" );
      transferDelete.bindValue(":toitem_id", _toitem->id());
      transferDelete.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Line"),
                               transferDelete, __FILE__, __LINE__))
        return;
      sFillItemList();

      if (_toitem->topLevelItemCount() == 0)
      {
        if (QMessageBox::question(this, tr("Cancel Transfer Order?"),
                                  tr("<p>You have deleted all of the Line "
                                     "Items for this Transfer Order. Would you "
                                     "like to cancel this Transfer Order?"),
                                  QMessageBox::Yes,
                                  QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
        {
          transferDelete.prepare( "SELECT deleteTO(:tohead_id) AS result;");
          transferDelete.bindValue(":tohead_id", _toheadid);
          transferDelete.exec();
          if (transferDelete.first())
          {
            int result = transferDelete.value("result").toInt();
            if (result < 0)
              systemError(this, storedProcErrorLookup("deleteTO", result),
			  __FILE__, __LINE__);            
          }
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Order"),
                               transferDelete, __FILE__, __LINE__);

          clear();
          omfgThis->sTransferOrdersUpdated(_toheadid);
        }
      }
    }
  }
}

void transferOrder::populate()
{
  if ( (_mode == cEdit) || (_mode == cView) )
  {
    XSqlQuery to;
    to.prepare( "SELECT * "
                "FROM tohead "
                "WHERE (tohead_id=:tohead_id);" );
    to.bindValue(":tohead_id", _toheadid);
    to.exec();
    if (to.first())
    {
      _orderNumber->setText(to.value("tohead_number"));
      _orderNumber->setEnabled(false);
      _orderDate->setDate(to.value("tohead_orderdate").toDate(), true);

      if (to.value("tohead_status").toString() == "U")
        _status->setCurrentIndex(0);
      else if (to.value("tohead_status").toString() == "O")
        _status->setCurrentIndex(1);
      else if (to.value("tohead_status").toString() == "C")
      {
        _status->setCurrentIndex(2);
        _status->setEnabled(false);
      }

      _srcWhs->setId(to.value("tohead_src_warehous_id").toInt());
      if (! _srcWhs->isValid())
        _srcWhs->setCode(to.value("tohead_srcname").toString());
      if (_srcAddr->line1() !=to.value("tohead_srcaddress1").toString() ||
          _srcAddr->line2() !=to.value("tohead_srcaddress2").toString() ||
          _srcAddr->line3() !=to.value("tohead_srcaddress3").toString() ||
          _srcAddr->city()  !=to.value("tohead_srccity").toString() ||
          _srcAddr->state() !=to.value("tohead_srcstate").toString() ||
          _srcAddr->postalCode()!=to.value("tohead_srcpostalcode").toString() ||
          _srcAddr->country()!=to.value("tohead_srccountry").toString() )
      {
        _srcAddr->setId(-1);
        _srcAddr->setLine1(to.value("tohead_srcaddress1").toString());
        _srcAddr->setLine2(to.value("tohead_srcaddress2").toString());
        _srcAddr->setLine3(to.value("tohead_srcaddress3").toString());
        _srcAddr->setCity(to.value("tohead_srccity").toString());
        _srcAddr->setState(to.value("tohead_srcstate").toString());
        _srcAddr->setPostalCode(to.value("tohead_srcpostalcode").toString());
        _srcAddr->setCountry(to.value("tohead_srccountry").toString());
      }
      _srcContact->setId(to.value("tohead_srccntct_id").toInt());
      if (_srcContact->name() != to.value("tohead_srccntct_name").toString() ||
	  _srcContact->phone() != to.value("tohead_srcphone").toString())
      {
	_srcContact->setId(-1);
	_srcContact->setObjectName(to.value("tohead_srccntct_name").toString());
	_srcContact->setPhone(to.value("tohead_srcphone").toString());
      }

      _trnsWhs->setId(to.value("tohead_trns_warehous_id").toInt());
      if (! _trnsWhs->isValid())
	_trnsWhs->setCode(to.value("tohead_trnsname").toString());

      _dstWhs->setId(to.value("tohead_dest_warehous_id").toInt());
      if (! _dstWhs->isValid())
	_dstWhs->setCode(to.value("tohead_destname").toString());
      if (_dstAddr->line1() !=to.value("tohead_destaddress1").toString() ||
          _dstAddr->line2() !=to.value("tohead_destaddress2").toString() ||
          _dstAddr->line3() !=to.value("tohead_destaddress3").toString() ||
          _dstAddr->city()  !=to.value("tohead_destcity").toString() ||
          _dstAddr->state() !=to.value("tohead_deststate").toString() ||
          _dstAddr->postalCode()!=to.value("tohead_destpostalcode").toString() ||
          _dstAddr->country()!=to.value("tohead_destcountry").toString() )
      {
        _dstAddr->setId(-1);
        _dstAddr->setLine1(to.value("tohead_destaddress1").toString());
        _dstAddr->setLine2(to.value("tohead_destaddress2").toString());
        _dstAddr->setLine3(to.value("tohead_destaddress3").toString());
        _dstAddr->setCity(to.value("tohead_destcity").toString());
        _dstAddr->setState(to.value("tohead_deststate").toString());
        _dstAddr->setPostalCode(to.value("tohead_destpostalcode").toString());
        _dstAddr->setCountry(to.value("tohead_destcountry").toString());
      }
      _dstContact->setId(to.value("tohead_destcntct_id").toInt());
      if (_dstContact->name() != to.value("tohead_destcntct_name").toString() ||
        _dstContact->phone() != to.value("tohead_destphone").toString())
      {
        _dstContact->setId(-1);
        _dstContact->setObjectName(to.value("tohead_destcntct_name").toString());
        _dstContact->setPhone(to.value("tohead_destphone").toString());
      }

      _agent->setText(to.value("tohead_agent_username").toString());
      _shipVia->setText(to.value("tohead_shipvia").toString());
      _shippingForm->setId(to.value("tohead_shipform_id").toInt());
      _taxzoneidCache = to.value("tohead_taxzone_id").toInt();
      _taxzone->setId(to.value("tohead_taxzone_id").toInt());

      _freight->setId(to.value("tohead_freight_curr_id").toInt());
      _freight->setLocalValue(to.value("tohead_freight").toDouble());

      _shipComplete->setChecked(to.value("tohead_shipcomplete").toBool());
      _orderComments->setText(to.value("tohead_ordercomments").toString());
      _shippingComments->setText(to.value("tohead_shipcomments").toString());
      _packDate->setDate(to.value("tohead_packdate").toDate());
      _project->setId(to.value("tohead_prj_id").toInt());

      _comments->setId(_toheadid);
      _documents->setId(_toheadid);
      sFillItemList();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Order"),
                                  to, __FILE__, __LINE__))
      return;
  }
}

void transferOrder::sFillItemList()
{
  XSqlQuery transferFillItemList;
  transferFillItemList.prepare( "SELECT MIN(toitem_schedshipdate) AS shipdate "
	     "FROM toitem "
	     "WHERE ((toitem_status <> 'X')"
	     "  AND  (toitem_tohead_id=:head_id));" );

  transferFillItemList.bindValue(":head_id", _toheadid);
  transferFillItemList.exec();
  if (transferFillItemList.first())
  {
    _shipDate->setDate(transferFillItemList.value("shipdate").toDate());

    if (cNew == _mode)
      _packDate->setDate(transferFillItemList.value("shipdate").toDate());
  }
  else
  {
    _shipDate->clear();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Number"),
                             transferFillItemList, __FILE__, __LINE__))
      return;
  }

  _toitem->clear();
  if (_mode == cEdit || _mode == cNew)
  {
    _srcWhs->setEnabled(true);
    _trnsWhs->setEnabled(true);
    _dstWhs->setEnabled(true);
  }

  QString sql("SELECT toitem_id, closestatus,"
	      "       toitem_status,"
	      "       toitem_linenumber, item_number, description,"
	      "       toitem_schedshipdate,"
	      "       toitem_qty_ordered, toitem_qty_shipped, toitem_qty_received,"
        "       balance, atshipping, tagged, in_future, enabletoshipping, backorder,"
        "       CASE WHEN (backorder AND toitem_status NOT IN ('C', 'X')) THEN 'red' "
        "            WHEN (enabletoshipping AND tagged AND in_future) THEN 'darkgreen' "
        "            WHEN (enabletoshipping AND tagged) THEN 'red' "
        "       END AS qtforegroundrole,"
        "       'qty' AS toitem_qty_ordered_xtnumericrole,"
        "       'qty' AS toitem_qty_shipped_xtnumericrole,"
        "       'qty' AS toitem_qty_received_xtnumericrole,"
        "       'qty' AS atshipping_xtnumericrole,"
        "       'qty' AS balance_xtnumericrole "
        "      FROM ( "
        "SELECT toitem_id,"
	      "       toitem_status,"
	      "       CASE WHEN (toitem_status='C') THEN 1"
	      "            WHEN (toitem_status='X') THEN 4"
	      "            WHEN (toitem_status='U') THEN 5"
	      "            WHEN ((toitem_status='O')"
	      "                  AND ((qtyAtShipping('TO', toitem_id) > 0) OR"
	      "                          (toitem_qty_shipped > 0) ) ) THEN 2"
	      "            ELSE 3"
	      "       END AS closestatus,"
	      "       toitem_linenumber, item_number,"
	      "       (item_descrip1 || ' ' || item_descrip2) AS description,"
	      "       toitem_schedshipdate,"
	      "       toitem_qty_ordered, toitem_qty_shipped, toitem_qty_received, "
	      "       noNeg(toitem_qty_ordered - toitem_qty_shipped) AS balance,"
	      "       qtyAtShipping('TO', toitem_id) AS atshipping,"
	      "       (noNeg(toitem_qty_ordered - toitem_qty_shipped) <> qtyAtShipping('TO', toitem_id)) AS tagged,"
	      "       CASE WHEN toitem_schedshipdate > CURRENT_DATE THEN true"
	      "            ELSE false"
	      "       END AS in_future,"
        "       fetchMetricBool('EnableTOShipping') AS enabletoshipping,"
        "       ((SELECT COALESCE(toitem_id, -1) FROM toitem WHERE ((toitem_status='C') AND (toitem_tohead_id=tohead_id)) LIMIT 1) <> -1) AS backorder " 
	      "FROM item, toitem, tohead "
	      "WHERE ((toitem_item_id=item_id)"
        "  AND  (tohead_id=toitem_tohead_id)"
	      "<? if exists(\"showCanceled\") ?>"
	      "<? else ?>"
	      "  AND  (toitem_status != 'X')"
	      "<? endif ?>"
	      "  AND  (toitem_tohead_id=<? value(\"tohead_id\") ?>) ) ) AS data "
	      "ORDER BY toitem_linenumber;" );

  ParameterList params;
  params.append("tohead_id", _toheadid);
  if (_showCanceled->isChecked())
    params.append("showCanceled");
	     
  MetaSQLQuery mql(sql);
  transferFillItemList = mql.toQuery(params);
  _toitem->populate(transferFillItemList, true);
  if (transferFillItemList.first())
  {
    do
    {
      if (transferFillItemList.value("toitem_qty_received").toDouble() > 0)
      {
	_dstWhs->setEnabled(false);
	_trnsWhs->setEnabled(false);
	_srcWhs->setEnabled(false);
      }
      else if (transferFillItemList.value("toitem_qty_shipped").toDouble() > 0)
      {
	_trnsWhs->setEnabled(false);
	_srcWhs->setEnabled(false);
      }
      else if (transferFillItemList.value("atshipping").toDouble() > 0)
      {
	_srcWhs->setEnabled(false);
      }
    }
    while (transferFillItemList.next());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Lines"),
                                transferFillItemList, __FILE__, __LINE__))
    return;

  transferFillItemList.prepare("SELECT formatQty(SUM(COALESCE(toitem_qty_ordered, 0.00) *"
	    "                 (COALESCE(item_prodweight, 0.00) +"
	    "                  COALESCE(item_packweight, 0.00)))) AS grossweight,"
	    "       SUM(toitem_freight) AS linefreight "
	    "FROM toitem, item "
	    "WHERE ((toitem_item_id=item_id)"
	    " AND (toitem_status<>'X')"
	    " AND (toitem_tohead_id=:head_id));");

  transferFillItemList.bindValue(":head_id", _toheadid);
  transferFillItemList.exec();
  if (transferFillItemList.first())
  {
    _weight->setText(transferFillItemList.value("grossweight").toDouble());
    _itemFreight->setLocalValue(transferFillItemList.value("linefreight").toDouble());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Summary"),
                                transferFillItemList, __FILE__, __LINE__))
    return;

  sCalculateTax(); // triggers sCalculateTotal();

  _srcWhs->setEnabled(_toitem->topLevelItemCount() == 0);
  _freightCurrency->setEnabled(_toitem->topLevelItemCount() == 0);
  _qecurrency->setEnabled(_toitem->topLevelItemCount() == 0);

  _qeitem->select();
}

void transferOrder::sCalculateTotal()
{
  _total->setLocalValue(_tax->localValue() + _freight->localValue() +
			_itemFreight->localValue());
}

bool transferOrder::deleteForCancel()
{
  XSqlQuery query;
  bool deleteTo = false;

  if (cNew == _mode && _toitem->topLevelItemCount()== 0)
  {
    deleteTo = true;
  }

  if (cNew == _mode && _toitem->topLevelItemCount() > 0)
  {
    if (QMessageBox::question(this, tr("Delete Transfer Order?"),
                          tr("<p>Are you sure you want to delete this "
                             "Transfer Order and its associated Line Items?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return false;
    else
      deleteTo = true;
  }

  if (cNew != _mode && _toitem->topLevelItemCount() == 0)
  {
    if (QMessageBox::question(this, tr("Delete Transfer Order?"),
                              tr("<p>This Transfer Order does not have any line items.  "
                                 "Are you sure you want to delete this Transfer Order?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return false;
    else
      deleteTo = true;
  }

  if (deleteTo)
  {
    query.prepare("SELECT deleteTO(:tohead_id) AS result;");
    query.bindValue(":tohead_id", _toheadid);
    query.exec();
    if (query.first())
    {
      int result = query.value("result").toInt();
      if (result < 0)
        systemError(this, storedProcErrorLookup("deleteTO", result),
                    __FILE__, __LINE__);

      sReleaseNumber();
    }
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Order"),
                         query, __FILE__, __LINE__);
  }

  if(cNew == _mode)
    sReleaseTohead();

  return true;
}

void transferOrder::sClear()
{
  if (! deleteForCancel())
    return;
  clear();
}

void transferOrder::clear()
{
  if (cView != _mode)
    sReleaseTohead();
  _toheadid = -1;

  _tabs->setCurrentIndex(0);

  _orderNumber->setEnabled(true);
  _orderNumberGen = 0;
  _orderNumber->clear();

  _shipDate->clear();
  _packDate->clear();
  _srcWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  _trnsWhs->setId(_metrics->value("DefaultTransitWarehouse").toInt());
  _dstWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  _status->setCurrentIndex(0);
  _agent->setCurrentIndex(-1);
  _taxzoneidCache = -1;
  _taxzone->setId(-1);
  _whstaxzoneid        = -1;
  _shipVia->setId(-1);
//  cannot clear _shippingForm
//  _shippingForm->setId(-1);
  _freight->clear();
  _orderComments->clear();
  _shippingComments->clear();
  _tax->clear();
  _freight->clear();
  _total->clear();
  _weight->clear();
  _project->setId(-1);
  _srcWhs->setEnabled(true);
  _freightCurrency->setEnabled(true);
  _qecurrency->setEnabled(true);
  _qeitem->removeRows(0, _qeitem->rowCount());

  _shipComplete->setChecked(false);

  if ( (_mode == cEdit) || (_mode == cNew) )
  {
    _mode = cNew;
    setObjectName("transferOrder new");
    _orderDate->setDate(omfgThis->dbDate(), true);
  }

  if ( (_metrics->value("TONumberGeneration") == "A") ||
       (_metrics->value("TONumberGeneration") == "O")   )
  {
    if (! insertPlaceholder())
      return;
  }
  
  if (_orderNumber->text().isEmpty())
    _orderNumber->setFocus();
  else
    _srcWhs->setFocus();

  _toitem->clear();

  _saved = false;
}

void transferOrder::closeEvent(QCloseEvent *pEvent)
{
  // TODO: if sQeSave == false then find a way to return control to the user
  if (_qeitem->isDirty())
  {
    if (QMessageBox::question(this, tr("Save Quick Entry Data?"),
		    tr("Do you want to save your Quick Entry changes?"),
		    QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
      sQESave();
  }

  if (! deleteForCancel())
  {
    pEvent->ignore();
    return;
  }

  disconnect(_orderNumber,    SIGNAL(editingFinished()), this, SLOT(sHandleOrderNumber()));

  sReleaseTohead();
  
  if (cView != _mode)
    omfgThis->sTransferOrdersUpdated(-1);

  XWidget::closeEvent(pEvent);
}

void transferOrder::sHandleShipchrg(int pShipchrgid)
{
  if (_mode == cView)
    _freight->setEnabled(false);
  else
  {
    XSqlQuery query;
    query.prepare( "SELECT shipchrg_custfreight "
                   "FROM shipchrg "
                   "WHERE (shipchrg_id=:shipchrg_id);" );
    query.bindValue(":shipchrg_id", pShipchrgid);
    query.exec();
    if (query.first())
    {
      if (query.value("shipchrg_custfreight").toBool())
        _freight->setEnabled(true);
      else
      {
        _freight->setEnabled(false);
        _freight->clear();
      }
    }
  }
}

void transferOrder::sHandleTransferOrderEvent(int pToheadid)
{
  if (pToheadid == _toheadid)
    sFillItemList();
}

void transferOrder::sTaxDetail()
{
  XSqlQuery taxq;
  if (cView != _mode)
  {
    taxq.prepare("UPDATE tohead SET tohead_taxzone_id=:taxzone, "
		  "  tohead_freight=:freight,"
		  "  tohead_orderdate=:date "
		  "WHERE (tohead_id=:head_id);");
    if (_taxzone->isValid())
      taxq.bindValue(":taxzone", _taxzone->id());
    taxq.bindValue(":freight", _freight->localValue());
    taxq.bindValue(":date",    _orderDate->date());
    taxq.bindValue(":head_id", _toheadid);
    taxq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Details"),
                             taxq, __FILE__, __LINE__))
      return;
  }

  ParameterList params;
  params.append("order_id", _toheadid);
  params.append("order_type", "TO");
  params.append("mode", "view"); // because tohead has no fields to hold changes

  taxBreakdown newdlg(this, "", true);
  if (newdlg.set(params) == NoError)
  {
    newdlg.exec();
    populate();
  }
}

void transferOrder::setViewMode()
{
  if(cEdit == _mode)
  {
    // Undo some changes set for the edit mode
    _captive = false;
  }
  _new->setEnabled(false);

  disconnect(omfgThis, SIGNAL(transferOrdersUpdated(int)), this, SLOT(sHandleTransferOrderEvent(int)));

  _mode = cView;
  setObjectName(QString("transferOrder view %1").arg(_toheadid));

  _orderNumber->setEnabled(false);
  _status->setEnabled(false);
  _packDate->setEnabled(false);
  _srcWhs->setEnabled(false);
  _dstWhs->setEnabled(false);
  _trnsWhs->setEnabled(false);
  _agent->setEnabled(false);
  _taxzone->setEnabled(false);
  _shipVia->setEnabled(false);
  _shippingForm->setEnabled(false);
  _freight->setEnabled(false);
  _orderComments->setEnabled(false);
  _shippingComments->setEnabled(false);
  _qeitemView->setEnabled(false);
  _qesave->setEnabled(false);
  _qedelete->setEnabled(false);
  _qecurrency->setEnabled(false);
  _freightCurrency->setEnabled(false);
  _srcContact->setEnabled(false);
  _dstContact->setEnabled(false);

  _edit->setText(tr("View"));
  _comments->setReadOnly(true);
  _project->setReadOnly(true);
//  _documents->setReadOnly(true);
  _shipComplete->setEnabled(false);
  _save->hide();
  _clear->hide();
  _issueStock->hide();
  _issueLineBalance->hide();
  _project->setReadOnly(true);
  if(_metrics->boolean("AlwaysShowSaveAndAdd"))
    _saveAndAdd->setEnabled(false);
  else
    _saveAndAdd->hide();
  _action->hide();
  _delete->hide();
}

void transferOrder::keyPressEvent( QKeyEvent * e )
{
#ifdef Q_OS_MAC
  if(e->key() == Qt::Key_N && (e->modifiers() & Qt::ControlModifier))
  {
    _new->animateClick();
    e->accept();
  }
  else if(e->key() == Qt::Key_E && (e->modifiers() & Qt::ControlModifier))
  {
    _edit->animateClick();
    e->accept();
  }
  if(e->isAccepted())
    return;
#endif
  e->ignore();
}

void transferOrder::newTransferOrder(int pSrcWhsid, int pDstWhsid)
{
  // Check for an Item window in new mode already.
  if(pSrcWhsid == -1 && pDstWhsid == -1)
  {
    QWidgetList list = omfgThis->windowList();
    for(int i = 0; i < list.size(); i++)
    {
      QWidget * w = list.at(i);
      if(QString::compare(w->objectName(), "transferOrder new")==0)
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
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "new");
  if(pSrcWhsid != -1)
    params.append("src_warehous_id", pSrcWhsid);
  if (pDstWhsid != -1)
    params.append("dst_warehous_id", pDstWhsid);

  transferOrder *newdlg = new transferOrder();
  if(newdlg->set(params) != UndefinedError)
    omfgThis->handleNewWindow(newdlg);
  else
    delete newdlg;
}

void transferOrder::editTransferOrder( int pId , bool enableSaveAndAdd )
{
  // Check for an Item window in edit mode for the specified transferOrder already.
  QString n = QString("transferOrder edit %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
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
  params.append("tohead_id", pId);
  if(enableSaveAndAdd)
    params.append("enableSaveAndAdd");

  transferOrder *newdlg = new transferOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void transferOrder::viewTransferOrder( int pId )
{
  // Check for an Item window in view mode for the specified transferOrder already.
  QString n = QString("transferOrder view %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
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
  params.append("tohead_id", pId);

  transferOrder *newdlg = new transferOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void transferOrder::sReturnStock()
{
  XSqlQuery transferReturnStock;
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  transferReturnStock.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  transferReturnStock.prepare("SELECT returnItemShipments('TO', :toitem_id, 0, CURRENT_TIMESTAMP) AS result;");
  QList<XTreeWidgetItem*> selected = _toitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    transferReturnStock.bindValue(":toitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
    transferReturnStock.exec();
    if (transferReturnStock.first())
    {
      int result = transferReturnStock.value("result").toInt();
      if (result < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("returnItemShipments", result) +
                          tr("<br>Line Item %1").arg(selected[i]->text(0)),
                           __FILE__, __LINE__);
        return;
      }
      if (distributeInventory::SeriesAdjust(transferReturnStock.value("result").toInt(), this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Return Stock"), tr("Transaction Canceled") );
        return;
      }

    }
    else if (transferReturnStock.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                        transferReturnStock.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  transferReturnStock.exec("COMMIT;");

  sFillItemList();
}

void transferOrder::sIssueStock()
{
  bool update  = false;
  QList<XTreeWidgetItem*> selected = _toitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem* toitem = (XTreeWidgetItem*)(selected[i]);
    // skip if status = C or X or U
    if (toitem->altId() != 1 && toitem->altId() != 4 && toitem->altId() != 5)
    {
      ParameterList params;
      params.append("toitem_id", toitem->id());

      if(_requireInventory->isChecked())
        params.append("requireInventory");

      issueLineToShipping newdlg(this, "", true);
      newdlg.set(params);
      if (newdlg.exec() != XDialog::Rejected)
        update = true;
    }
  }

  if (update)
    sFillItemList();
}

void transferOrder::sIssueLineBalance()
{
  XSqlQuery transferIssueLineBalance;
  QList<XTreeWidgetItem*> selected = _toitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem* toitem = (XTreeWidgetItem*)(selected[i]);
    // skip if status = C or X or U
    if (toitem->altId() != 1 && toitem->altId() != 4 && toitem->altId() != 5)
    {

      if(_requireInventory->isChecked())
      {
        transferIssueLineBalance.prepare("SELECT sufficientInventoryToShipItem('TO', :toitem_id) AS result;");
        transferIssueLineBalance.bindValue(":toitem_id", toitem->id());
        transferIssueLineBalance.exec();
        if (transferIssueLineBalance.first())
        {
          int result = transferIssueLineBalance.value("result").toInt();
          if (result < 0)
          {
            transferIssueLineBalance.prepare("SELECT item_number, tohead_srcname "
                "  FROM toitem, tohead, item "
                " WHERE ((toitem_item_id=item_id)"
                "   AND  (toitem_tohead_id=tohead_id)"
                "   AND  (toitem_id=:toitem_id)); ");
            transferIssueLineBalance.bindValue(":toitem_id", toitem->id());
            transferIssueLineBalance.exec();
            if (! transferIssueLineBalance.first() && transferIssueLineBalance.lastError().type() != QSqlError::NoError)
            {
              ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Item"),
                                   transferIssueLineBalance, __FILE__, __LINE__);
              systemError(this,
                storedProcErrorLookup("sufficientInventoryToShipItem",
                          result)
                .arg(transferIssueLineBalance.value("item_number").toString())
                .arg(transferIssueLineBalance.value("tohead_srcname").toString()), __FILE__, __LINE__);
              return;
            }
          }
        }
        else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Checking Inventory"),
                               transferIssueLineBalance, __FILE__, __LINE__))
          return;
      }

      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

      transferIssueLineBalance.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
      transferIssueLineBalance.prepare("SELECT issueLineBalanceToShipping('TO', :toitem_id, CURRENT_TIMESTAMP) AS result;");
      transferIssueLineBalance.bindValue(":toitem_id", toitem->id());
      transferIssueLineBalance.exec();
      if (transferIssueLineBalance.first())
      {
        int result = transferIssueLineBalance.value("result").toInt();
        if (result < 0)
        {
          rollback.exec();
          systemError(this, storedProcErrorLookup("issueLineBalance", result) +
                            tr("<br>Line Item %1").arg(selected[i]->text(0)),
                      __FILE__, __LINE__);
          return;
        }
        if (distributeInventory::SeriesAdjust(transferIssueLineBalance.value("result").toInt(), this) == XDialog::Rejected)
        {
          rollback.exec();
          QMessageBox::information( this, tr("Issue to Shipping"), tr("Transaction Canceled") );
          return;
        }

        transferIssueLineBalance.exec("COMMIT;");
      }
      else
      {
        rollback.exec();
        systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                          transferIssueLineBalance.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }

  sFillItemList();
}

void transferOrder::sCalculateTax()
{
  XSqlQuery taxq;

  taxq.prepare( "SELECT SUM(tax) AS tax "
                "FROM ("
                "SELECT ROUND(calculateTax(:taxzone_id,getFreightTaxTypeId(),:date,:curr_id,toitem_freight),2) AS tax "
                "FROM toitem "
                "WHERE (toitem_tohead_id=:tohead_id) "
                "UNION ALL "
                "SELECT ROUND(calculateTax(:taxzone_id,getFreightTaxTypeId(),:date,:curr_id,:freight),2) AS tax "
                ") AS data;" );
  taxq.bindValue(":tohead_id", _toheadid);
  taxq.bindValue(":taxzone_id", _taxzone->id());
  taxq.bindValue(":date", _orderDate->date());
  taxq.bindValue(":freight", _freight->localValue());
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Calculating Tax"),
                                taxq, __FILE__, __LINE__))
    return;
  
  sCalculateTotal();
}

bool transferOrder::sQESave()
{
  _qesave->setFocus();
  if (! _qeitem->submitAll())
  {
    if (! _qeitem->lastError().databaseText().isEmpty())
      systemError(this, _qeitem->lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  sFillItemList();
  return true;
}

void transferOrder::sQEDelete()
{
  if (! _qeitem->removeRow(_qeitemView->currentIndex().row()))
  {
    systemError(this, tr("Removing row from view failed"), __FILE__, __LINE__);
    return;
  }
}

void transferOrder::sCurrencyChanged()
{
  // The user can only set the QE currency from the QE tab
  if (_tabs->currentWidget()->isAncestorOf(_qecurrency) &&
      _freightCurrency->id() != _qecurrency->id())
    _freightCurrency->setId(_qecurrency->id());
  else
    _qecurrency->setId(_freightCurrency->id());

  static_cast<ToitemTableModel*>(_qeitemView->model())->setCurrId(_freightCurrency->id());
}

void transferOrder::sTabChanged(int pIndex)
{
  if (pIndex != _cachedTabIndex)
  {
    if (pIndex == _tabs->indexOf(_qeTab) && ! _shipDate->isValid() && ! _packDate->isValid())
    {
      QMessageBox::warning(this, tr("Quick Entry Requires a Date"),
			   tr("<p>You must enter either a Scheduled Date or a "
			      "Pack Date before using the Quick Entry tab."));
      _cachedTabIndex = 0;
      _tabs->setCurrentIndex(0);
      if (_shipDate->isEnabled())
	_shipDate->setFocus();
      else if (_packDate->isEnabled())
	_packDate->setFocus();
    }
    else if (_cachedTabIndex == _tabs->indexOf(_qeTab) && _qeitem->isDirty())
    {
      if (QMessageBox::question(this, tr("Save Quick Entry Data?"),
		      tr("Do you want to save your Quick Entry changes?"),
		      QMessageBox::Yes | QMessageBox::Default,
		      QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
	if (! sQESave())
	{
	  _tabs->setCurrentIndex(_cachedTabIndex);
	  return;
	}
      _cachedTabIndex = pIndex;
    }
    else
      _cachedTabIndex = pIndex;
  }
}

void transferOrder::getWhsInfo(const int pid, const QWidget* pwidget)
{
  XSqlQuery whsq;
  whsq.prepare("SELECT * FROM whsinfo WHERE warehous_id=:id;");
  whsq.bindValue(":id", pid);
  whsq.exec();
  if (whsq.first())
  {
    if (pwidget == _srcWhs)
    {
      _srcContact->setId(whsq.value("warehous_cntct_id").toInt());
      _srcAddr->setId(whsq.value("warehous_addr_id").toInt());
      _qeitem->setSrcWhsId(pid);
    }
    else if (pwidget == _dstWhs)
    {
      _dstContact->setId(whsq.value("warehous_cntct_id").toInt());
      _dstAddr->setId(whsq.value("warehous_addr_id").toInt());
      _taxzone->setId(whsq.value("warehous_taxzone_id").toInt());
    }
    else if (pwidget == _trnsWhs)
    {
      _shippingForm->setId(whsq.value("warehous_shipform_id").toInt());
      _shipVia->setId(whsq.value("warehous_shipvia_id").toInt());
      _shippingComments->setText(whsq.value("warehous_shipcomments").toString());
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Site"),
                                whsq, __FILE__, __LINE__))
    return;
}

void transferOrder::sHandleDstWhs(const int pid)
{
  getWhsInfo(pid, _dstWhs);
}

void transferOrder::sHandleSrcWhs(const int pid)
{
  getWhsInfo(pid, _srcWhs);
}

void transferOrder::sHandleTrnsWhs(const int pid)
{
  getWhsInfo(pid, _trnsWhs);
}

void transferOrder::sReleaseTohead()
{
  if (! _lock.release())
    (void)ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                               _lock.lastError(), __FILE__, __LINE__);
}

void transferOrder::sReleaseNumber()
{
  XSqlQuery transferReleaseNumber;
  if(-1 != _orderNumberGen)
  {
    transferReleaseNumber.prepare("SELECT releaseToNumber(:number);" );
    transferReleaseNumber.bindValue(":number", _orderNumberGen);
    transferReleaseNumber.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Releasing Number"),
                         transferReleaseNumber, __FILE__, __LINE__);
    _orderNumberGen = -1;
  }
}
