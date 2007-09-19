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

#include "transferOrder.h"

#include <stdlib.h>

#include <Q3DragObject>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QWorkspace>

#include <metasql.h>

#include "distributeInventory.h"
#include "issueLineToShipping.h"
#include "transferOrderItem.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"

#define cClosed       0x01
#define cActiveOpen   0x02
#define cInactiveOpen 0x04
#define cCanceled     0x08

transferOrder::transferOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  _qeitem = new ToitemTableModel(this);
  _qeitemView->setModel(_qeitem);

  connect(_action,	    SIGNAL(clicked()), this, SLOT(sAction()));
  connect(_clear,	    SIGNAL(pressed()), this, SLOT(sClear()));
  connect(_delete,	    SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_dstWhs,         SIGNAL(newID(int)), this, SLOT(sHandleDstWhs(int)));
  connect(_edit,	    SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_freight,    SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  connect(_freightCurrency, SIGNAL(newID(int)), this, SLOT(sCurrencyChanged()));
  connect(_issueLineBalance, SIGNAL(clicked()), this, SLOT(sIssueLineBalance()));
  connect(_issueStock,	     SIGNAL(clicked()), this, SLOT(sIssueStock()));
  connect(_new,		     SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_orderDate,	SIGNAL(newDate(QDate)), _qeitem, SLOT(setTransDate(QDate)));
  connect(_orderNumber,    SIGNAL(lostFocus()), this, SLOT(sHandleOrderNumber()));
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
  connect(_taxauth,        SIGNAL(newID(int)), this, SLOT(sTaxAuthChanged()));
  connect(_toitem, SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtons()));
  connect(_toitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_trnsWhs,  SIGNAL(newID(int)), this, SLOT(sHandleTrnsWhs(int)));
  connect(omfgThis, SIGNAL(transferOrdersUpdated(int)), this, SLOT(sHandleTransferOrderEvent(int)));

  _toitem->addColumn(tr("#"),           _seqColumn,     Qt::AlignCenter );
  _toitem->addColumn(tr("Item"),        _itemColumn,    Qt::AlignLeft   );
  _toitem->addColumn(tr("Description"), -1,             Qt::AlignLeft   );
  _toitem->addColumn(tr("Status"),      _statusColumn,  Qt::AlignCenter );
  _toitem->addColumn(tr("Sched. Date"), _dateColumn,    Qt::AlignCenter );
  _toitem->addColumn(tr("Ordered"),     _qtyColumn,     Qt::AlignRight  );
  _toitem->addColumn(tr("At Shipping"), _qtyColumn,     Qt::AlignRight  );
  _toitem->addColumn(tr("Shipped"),     _qtyColumn,     Qt::AlignRight  );
  _toitem->addColumn(tr("Balance"),     _qtyColumn,     Qt::AlignRight  );
  _toitem->addColumn(tr("Received"),	_qtyColumn,	Qt::AlignRight  );

  _orderNumber->setValidator(omfgThis->orderVal());

  if(!_metrics->boolean("UseProjects"))
    _project->hide();

  _mode = cView;

  setToheadid(-1);

  _captive		= false;
  _freighttaxid		= -1;
  _orderNumberGen	= 0;
  _saved		= false;
  _taxauthidCache	= -1;
  _whstaxauthid		= -1;
  _srcWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  _trnsWhs->setId(_metrics->value("DefaultTransitWarehouse").toInt());
  _dstWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  getWhsInfo(_trnsWhs->id(), _trnsWhs);

  _taxCache.clear();

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
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      setName("transferOrder new");
      _mode = cNew;
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
  }

  param = pParams.value("src_warehous_id", &valid);
  if (valid)
    _srcWhs->setId(param.toInt());

  param = pParams.value("dst_warehous_id", &valid);
  if (valid)
    _dstWhs->setId(param.toInt());

  if (cNew == _mode)
  {
    _ignoreSignals = TRUE;

    populateOrderNumber();
    if (_orderNumber->text().isEmpty())
      _orderNumber->setFocus();
    else
      _packDate->setFocus();

    _ignoreSignals = FALSE;

    q.exec("SELECT NEXTVAL('tohead_tohead_id_seq') AS head_id;");
    if (q.first())
    {
      setToheadid(q.value("head_id").toInt());
      _orderDate->setDate(omfgThis->dbDate(), true);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    _status->setText("O");

    if (! insertPlaceholder())
      return UndefinedError;

    setAcceptDrops(true);
    _captive = FALSE;
    _edit->setEnabled(FALSE);
    _action->setEnabled(FALSE);
    _delete->setEnabled(FALSE);
    _close->setText("&Cancel");
  }
  else if (cEdit == _mode)
  {
    _captive = TRUE;
    setAcceptDrops(TRUE);
    _orderNumber->setEnabled(FALSE);

    _new->setFocus();
  }
  else if (cView == _mode)
  {
    _project->setReadOnly(true);

    _close->setFocus();
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
      setName(QString("transferOrder edit %1").arg(_toheadid));
    else if(cView == _mode)
      setName(QString("transferOrder view %1").arg(_toheadid));
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
    _orderDate->setEnabled(_privleges->check("OverrideTODate"));
    _packDate->setEnabled(_privleges->check("AlterPackDate"));
  }
  else
  {
    _orderDate->setEnabled(FALSE);
    _packDate->setEnabled(FALSE);
  }

  // TODO: Why don't the constructor or setId earlier in set() handle this?
  getWhsInfo(_srcWhs->id(), _srcWhs);
  getWhsInfo(_dstWhs->id(), _dstWhs);

  return NoError;
}

bool transferOrder::insertPlaceholder()
{
  if(_trnsWhs->id() == -1)
  {
    QMessageBox::critical(this, tr("No Transit Warehouse"),
      tr("There are no transit warehouses defined in the system."
         " You must define at least one transit warehouse to use Transfer Orders.") );
    return false;
  }

  q.prepare("INSERT INTO tohead ("
	    "          tohead_id, tohead_number, tohead_src_warehous_id,"
	    "          tohead_trns_warehous_id, tohead_dest_warehous_id,"
	    "          tohead_status, tohead_shipform_id"
	    ") VALUES ("
	    "          :tohead_id, :tohead_number, :tohead_src_warehous_id,"
	    "          :tohead_trns_warehous_id, :tohead_dest_warehous_id,"
	    "          :tohead_status, :tohead_shipform_id);");

  q.bindValue(":tohead_id", _toheadid);
  if (_orderNumber->text().isEmpty())
    q.bindValue(":tohead_number", QString::number(_toheadid * -1));
  else
    q.bindValue(":tohead_number",	  _orderNumber->text());
  q.bindValue(":tohead_src_warehous_id",  _srcWhs->id());
  q.bindValue(":tohead_trns_warehous_id", _trnsWhs->id());
  q.bindValue(":tohead_dest_warehous_id", _dstWhs->id());
  q.bindValue(":tohead_status",		  _status->text());
  if (_shippingForm->isValid())
    q.bindValue(":tohead_shipform_id",	  _shippingForm->id());

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  return true;
}

void transferOrder::sSaveAndAdd()
{
  if (save(false))
  {
    q.prepare("SELECT addToPackingListBatch('TO', :tohead_id) AS result;");
    q.bindValue(":tohead_id", _toheadid);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this,
		    storedProcErrorLookup("addToPackingListBatch", result),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

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
    if (_captive)
      close();
    else
      clear();
  }
}

bool transferOrder::save(bool partial)
{
  struct {
    bool	condition;
    QString	msg;
    QWidget	*widget;
  } error[] = {
    { _srcWhs->id() == -1,
      tr("You must select a Source Warehouse for this "
	 "Transfer Order before you may save it."),
    _srcWhs
    },
    { _trnsWhs->id() == -1,	// but see fr 5581
      tr("You must select a Transit Warehouse for "
	 "this Transfer Order before you may save it."),
      _dstWhs
    },
    { _dstWhs->id() == -1,
      tr("You must select a Destination Warehouse for "
	 "this Transfer Order before you may save it."),
      _dstWhs
    },
    { _srcWhs->id() == _dstWhs->id(),
      tr("The Source and Destination Warehouses must be different."),
      _dstWhs
    },
    { (!partial && _toitem->topLevelItemCount() == 0),
      tr("You must create at least one Line Item for "
      "this Transfer Order before you may save it."),
      _new
    },
    { _orderNumber->text().toInt() == 0,
      tr("You must enter a valid T/O # for this Transfer"
	 "Order before you may save it."),
      _orderNumber
    },
    { true, "", 0 }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Transfer Order"),
			  QString("<p>") + error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return false;
  }

  if (_qeitem->isDirty() && ! sQESave())
    return false;

  // place holder for automatic project creation
  // if(! partial && _metrics->boolean("AutoCreateProjectsForOrders") &&
  //    (_project->id() == -1) && (cNew == _mode))

  // save contact and address info in case someone wants to use 'em again later
  // but don't make any global changes to the data and ignore errors
  // TODO: put in real checking - perhaps the address of the warehouse is wrong
  _ignoreSignals = TRUE;
  _srcAddr->save(AddressCluster::CHANGEONE);
  _dstAddr->save(AddressCluster::CHANGEONE);
  _ignoreSignals = FALSE;

  q.prepare("UPDATE tohead "
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
	    "    tohead_taxauth_id=:taxauth_id,"
	    "    tohead_freight=:freight,"
	    "    tohead_freight_curr_id=:freight_curr_id,"
	    "    tohead_freighttax_id=:freighttax_id,"
	    "    tohead_freighttax_pcta=:freighttax_pcta,"
	    "    tohead_freighttax_pctb=:freighttax_pctb,"
	    "    tohead_freighttax_pctc=:freighttax_pctc,"
	    "    tohead_freighttax_ratea=:freighttax_ratea,"
	    "    tohead_freighttax_rateb=:freighttax_rateb,"
	    "    tohead_freighttax_ratec=:freighttax_ratec,"
	    "    tohead_shipcomplete=:shipcomplete,"
	    "    tohead_ordercomments=:ordercomments,"
	    "    tohead_shipcomments=:shipcomments,"
	    "    tohead_packdate=:packdate,"
	    "    tohead_prj_id=:prj_id,"
	    "    tohead_lastupdated=CURRENT_TIMESTAMP "
	   "WHERE (tohead_id=:id);" );

  q.bindValue(":id", _toheadid );

  q.bindValue(":number",		_orderNumber->text().toInt());
  q.bindValue(":status",		_status->text());
  q.bindValue(":orderdate",		_orderDate->date());

  q.bindValue(":src_warehous_id",	_srcWhs->id());
  q.bindValue(":srcname",		_srcWhs->currentText());
  q.bindValue(":srcaddress1",		_srcAddr->line1());
  q.bindValue(":srcaddress2",		_srcAddr->line2());
  q.bindValue(":srcaddress3",		_srcAddr->line3());
  q.bindValue(":srccity",		_srcAddr->city());
  q.bindValue(":srcstate",		_srcAddr->state());
  q.bindValue(":srcpostalcode",		_srcAddr->postalCode());
  q.bindValue(":srccountry",		_srcAddr->country());
  if (_srcContact->id() > 0)
    q.bindValue(":srccntct_id",		_srcContact->id());
  q.bindValue(":srccntct_name",		_srcContact->name());
  q.bindValue(":srcphone",		_srcContact->phone());

  q.bindValue(":trns_warehous_id",	_trnsWhs->id());
  q.bindValue(":trnsname",		_trnsWhs->currentText());

  q.bindValue(":dest_warehous_id",	_dstWhs->id());
  q.bindValue(":destname",		_dstWhs->currentText());
  q.bindValue(":destaddress1",		_dstAddr->line1());
  q.bindValue(":destaddress2",		_dstAddr->line2());
  q.bindValue(":destaddress3",		_dstAddr->line3());
  q.bindValue(":destcity",		_dstAddr->city());
  q.bindValue(":deststate",		_dstAddr->state());
  q.bindValue(":destpostalcode",	_dstAddr->postalCode());
  q.bindValue(":destcountry",		_dstAddr->country());
  if (_dstContact->id() > 0)
    q.bindValue(":destcntct_id",	_dstContact->id());
  q.bindValue(":destcntct_name",	_dstContact->name());
  q.bindValue(":destphone",		_dstContact->phone());

  q.bindValue(":agent_username",	_agent->currentText());
  q.bindValue(":shipvia",		_shipVia->currentText());

  if (_taxauth->isValid())
    q.bindValue(":taxauth_id",		_taxauth->id());

  q.bindValue(":freight",		_freight->localValue());
  q.bindValue(":freight_curr_id",	_freight->id());

  if (_taxCache.freightId() > 0)
    q.bindValue(":freighttax_id",	_taxCache.freightId());
  q.bindValue(":freighttax_pcta",	_taxCache.freightPct(0));
  q.bindValue(":freighttax_pctb",	_taxCache.freightPct(1));
  q.bindValue(":freighttax_pctc",	_taxCache.freightPct(2));
  q.bindValue(":freighttax_ratea",	_taxCache.freight(0));
  q.bindValue(":freighttax_rateb",	_taxCache.freight(1));
  q.bindValue(":freighttax_ratec",	_taxCache.freight(2));

  q.bindValue(":shipcomplete",	QVariant(_shipComplete->isChecked(), 0));
  q.bindValue(":ordercomments",		_orderComments->text());
  q.bindValue(":shipcomments",		_shippingComments->text());

  if (_packDate->isValid())
    q.bindValue(":packdate", _packDate->date());
  else
    q.bindValue(":packdate", _orderDate->date());

  if (_project->isValid())
    q.bindValue(":prj_id",		_project->id());

  if (_shippingForm->isValid())
    q.bindValue(":shipform_id",		_shippingForm->id());

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  // if this is a new to record and we haven't saved already
  // then we need to lock this record.
  if((cNew == _mode) && (!_saved))
  {
    // should I bother to check because no one should have this but us?
    q.prepare("SELECT lockTohead(:head_id) AS result;");
    q.bindValue(":head_id", _toheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  _saved = true;
  omfgThis->sTransferOrdersUpdated(_toheadid);

  return TRUE;
}

void transferOrder::sPopulateMenu(QMenu *pMenu)
{
  if ((_mode == cNew) || (_mode == cEdit))
  {
    int _numSelected = _toitem->selectedItems().size();
    if(_numSelected == 1)
    {
      if (_lineMode == cClosed)
        pMenu->insertItem(tr("Open Line..."), this, SLOT(sAction()), 0);
      else if (_lineMode == cActiveOpen)
      {
        pMenu->insertItem(tr("Edit Line..."), this, SLOT(sEdit()), 0);
        pMenu->insertItem(tr("Close Line..."), this, SLOT(sAction()), 0);
      }
      else if (_lineMode == cInactiveOpen)
      {
        pMenu->insertItem(tr("Edit Line..."), this, SLOT(sEdit()), 0);
        pMenu->insertItem(tr("Close Line..."), this, SLOT(sAction()), 0);
        pMenu->insertItem(tr("Delete Line..."), this, SLOT(sDelete()), 0);
      }
    }

    if(_metrics->boolean("EnableTOShipping"))
    {
      if(_numSelected == 1)
        pMenu->insertSeparator();

      pMenu->insertItem(tr("Return Stock"), this, SLOT(sReturnStock()), 0);
      pMenu->insertItem(tr("Issue Stock..."), this, SLOT(sIssueStock()), 0);
      pMenu->insertItem(tr("Issue Line Balance"), this, SLOT(sIssueLineBalance()), 0);
    }
  }
}
 
void transferOrder::populateOrderNumber()
{
  if (_mode == cNew)
  {
    if ( (_metrics->value("TONumberGeneration") == "A") ||
         (_metrics->value("TONumberGeneration") == "O")   )
    {
      q.exec("SELECT fetchToNumber() AS tonumber;");
      if (q.first())
      {
        _orderNumber->setText(q.value("tonumber"));
        _orderNumberGen = q.value("tonumber").toInt();

        if (_metrics->value("TONumberGeneration") == "A")
          _orderNumber->setEnabled(FALSE);
      }
      else if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    _new->setEnabled(! _orderNumber->text().isEmpty());
  }

  _userEnteredOrderNumber = FALSE;
}

void transferOrder::sSetUserEnteredOrderNumber()
{
  _userEnteredOrderNumber = TRUE;
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
        QMessageBox::warning( this, tr("Enter T/O #"),
                              tr("<p>You must enter a T/O # for this Transfer "
				 "Order before you may continue." ) );
        _orderNumber->setFocus();
        return;
      }
    }

  }
  else
  {
    XSqlQuery query;
    if ( (_mode == cNew) && (_userEnteredOrderNumber) )
    {
      query.prepare("SELECT deleteTO(:tohead_id) AS result;");
      query.bindValue(":tohead_id", _toheadid);
      query.exec();
      if (query.first())
      {
        int result = query.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("deleteTO", result), __FILE__, __LINE__);
          return;
        }
      }
      else if (query.lastError().type() != QSqlError::NoError)
      {
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }

      query.prepare( "SELECT tohead_id "
                     "FROM tohead "
                     "WHERE (tohead_number=:tohead_number);" );
      query.bindValue(":tohead_number", _orderNumber->text().toInt());
      query.exec();
      if (query.first())
      {
        _mode = cEdit;
        setToheadid(query.value("tohead_id").toInt());
        populate();
        _orderNumber->setEnabled(FALSE);
      }
      else
      {
        QString orderNumber = _orderNumber->text();
        clear();
        query.prepare( "SELECT releaseToNumber(:orderNumber) AS result;" );
        query.bindValue(":orderNumber", _orderNumberGen);
        query.exec();
	if (query.first())
	{
	  int result = query.value("result").toInt();
	  if (result < 0)
	  {
	    systemError(this, storedProcErrorLookup("releaseToNumber", result),
			__FILE__, __LINE__);
	    return;
	  }
	}
	else if (query.lastError().type() != QSqlError::None)
	{
	  systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
        _orderNumber->setText(orderNumber);
        _userEnteredOrderNumber = FALSE;
        _orderNumber->setEnabled(FALSE);
      }
    }

  }

  if (cView != _mode)
    _new->setEnabled(! _orderNumber->text().isEmpty());
}

void transferOrder::sNew()
{
  if( !_saved && ((_mode == cNew) ) )
    if(!save(true))
      return;

  ParameterList params;
  params.append("tohead_id",	_toheadid);
  params.append("srcwarehouse_id", _srcWhs->id());
  params.append("orderNumber",	_orderNumber->text().toInt());
  params.append("orderDate",	_orderDate->date());
  params.append("taxauth_id",	_taxauth->id());
  params.append("curr_id",	_freightCurrency->id());

  if ((_mode == cNew) || (_mode == cEdit))
    params.append("mode", "new");

  transferOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Accepted)
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

  transferOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Accepted &&
      ((_mode == cNew) || (_mode == cEdit)) )
    sFillItemList();
}

void transferOrder::sHandleButtons()
{
  XTreeWidgetItem *selected = 0;
  QList<QTreeWidgetItem*> selectedlist = _toitem->selectedItems();
  int _numSelected = selectedlist.size();
  if (_numSelected > 0)
    selected = (XTreeWidgetItem*)(selectedlist[0]);
  
  if (selected)
  {
    _issueStock->setEnabled(_privleges->check("IssueStockToShipping"));
    _issueLineBalance->setEnabled(_privleges->check("IssueStockToShipping"));

    if(_numSelected == 1)
    {
      _edit->setEnabled(TRUE);
      int lineMode = selected->altId();

      if (lineMode == 1)
      {
	_lineMode = cClosed;

	_action->setText(tr("Open"));
	_action->setEnabled(TRUE);
	_delete->setEnabled(FALSE);
      }
      else if (lineMode == 2)
      {
	_lineMode = cActiveOpen;

	_action->setText(tr("Close"));
	_action->setEnabled(TRUE);
	_delete->setEnabled(FALSE);
      }
      else if (lineMode == 3)
      {
	_lineMode = cInactiveOpen;

	_action->setText(tr("Close"));
	_action->setEnabled(TRUE);
	_delete->setEnabled(TRUE);
      }
      else if (lineMode == 4)
      {
	_lineMode = cCanceled;

	_action->setEnabled(FALSE);
	_delete->setEnabled(FALSE);
      }
      else
      {
	_action->setEnabled(FALSE);
	_delete->setEnabled(FALSE);
      }

      if(1 == lineMode || 4 == lineMode)
      {
	_issueStock->setEnabled(FALSE);
	_issueLineBalance->setEnabled(FALSE);
      }
    }
    else
    {
      _edit->setEnabled(FALSE);
      _action->setEnabled(FALSE);
      _delete->setEnabled(FALSE);
    }
  }
  else
  {
    _edit->setEnabled(FALSE);
    _action->setEnabled(FALSE);
    _delete->setEnabled(FALSE);
    _issueStock->setEnabled(FALSE);
    _issueLineBalance->setEnabled(FALSE);
  }
}

void transferOrder::sAction()
{
  if (_lineMode == cCanceled)
    return;

  if ( (_mode == cNew) || (_mode == cEdit) )
  {
    if (_lineMode == cClosed)
    {
      q.prepare( "UPDATE toitem "
                 "SET toitem_status='O' "
                 "WHERE (toitem_id=:toitem_id);" );
      q.bindValue(":toitem_id", _toitem->id());
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    else
    {
      q.prepare( "SELECT closeToItem(:toitem_id) AS result;" );
      q.bindValue(":toitem_id", _toitem->id());
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("closeToItem", result),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    sFillItemList();
  }
}

void transferOrder::sDelete()
{ 
  if ( (_mode == cEdit) || (_mode == cNew) )
  {
    if (QMessageBox::question(this, tr("Delete Selected Line Item?"),
                              tr("<p>Are you sure that you want to delete the "
                                 "selected Line Item?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    {
      q.prepare( "DELETE FROM toitem "
                 "WHERE (toitem_id=:toitem_id);" );
      q.bindValue(":toitem_id", _toitem->id());
      q.exec();
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
          q.prepare( "SELECT deleteTO(:tohead_id) AS result;");
          q.bindValue(":tohead_id", _toheadid);
          q.exec();
          if (q.first())
          {
            int result = q.value("result").toInt();
            if (result < 0)
              systemError(this, storedProcErrorLookup("deleteTO", result),
			  __FILE__, __LINE__);
          }
          else if (q.lastError().type() != QSqlError::NoError)
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

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
    if(_mode == cEdit)
    {
      to.prepare("SELECT lockTohead(:tohead_id) AS result;");
      to.bindValue(":tohead_id", _toheadid);
      to.exec();
      if(to.first())
      {
        if(to.value("result").toBool() != true)
        {
          QMessageBox::critical( this, tr("Record Currently Being Edited"),
            tr("<p>The record you are trying to edit is currently being edited "
               "by another user. Continue in View Mode.") );
          setViewMode();
        }
      }
      else
      {
        QMessageBox::critical( this, tr("Cannot Lock Record for Editing"),
          tr("<p>There was an unexpected error while trying to lock the record "
             "for editing. Please report this to your administator.") );
        setViewMode();
      }
    }
    to.prepare( "SELECT * "
                "FROM tohead "
                "WHERE (tohead_id=:tohead_id);" );
    to.bindValue(":tohead_id", _toheadid);
    to.exec();
    if (to.first())
    {
      _orderNumber->setText(to.value("tohead_number"));
      _orderNumber->setEnabled(FALSE);
      _orderDate->setDate(to.value("tohead_orderdate").toDate(), true);
      _status->setText(to.value("tohead_status").toString());

      _srcWhs->setId(to.value("tohead_src_warehous_id").toInt());
      if (! _srcWhs->isValid())
	_srcWhs->setCurrentText(to.value("tohead_srcname").toString());
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
	_srcContact->setName(to.value("tohead_srccntct_name").toString());
	_srcContact->setPhone(to.value("tohead_srcphone").toString());
      }

      _trnsWhs->setId(to.value("tohead_trns_warehous_id").toInt());
      if (! _trnsWhs->isValid())
	_trnsWhs->setCurrentText(to.value("tohead_trnsname").toString());

      _dstWhs->setId(to.value("tohead_dest_warehous_id").toInt());
      if (! _dstWhs->isValid())
	_dstWhs->setCurrentText(to.value("tohead_destname").toString());
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
	_dstContact->setName(to.value("tohead_destcntct_name").toString());
	_dstContact->setPhone(to.value("tohead_destphone").toString());
      }

      _agent->setText(to.value("tohead_agent_username").toString());
      _shipVia->setText(to.value("tohead_shipvia").toString());
      _shippingForm->setId(to.value("tohead_shipform_id").toInt());
      _taxauthidCache = to.value("tohead_taxauth_id").toInt();
      _taxauth->setId(to.value("tohead_taxauth_id").toInt());

      _freight->setId(to.value("tohead_freight_curr_id").toInt());
      _freight->setLocalValue(to.value("tohead_freight").toDouble());

      if (to.value("tohead_freighttax_id").isNull())
	_taxCache.setFreightId(-1);
      _taxCache.setFreightPct(to.value("tohead_freighttax_pcta").toDouble(),
			      to.value("tohead_freighttax_pctb").toDouble(),
			      to.value("tohead_freighttax_pctc").toDouble());
      _taxCache.setFreight(to.value("tohead_freighttax_ratea").toDouble(),
			   to.value("tohead_freighttax_rateb").toDouble(),
			   to.value("tohead_freighttax_ratec").toDouble());

      _shipComplete->setChecked(to.value("tohead_shipcomplete").toBool());
      _orderComments->setText(to.value("tohead_ordercomments").toString());
      _shippingComments->setText(to.value("tohead_shipcomments").toString());
      _packDate->setDate(to.value("tohead_packdate").toDate());
      _project->setId(to.value("tohead_prj_id").toInt());

      _comments->setId(_toheadid);
      sFillItemList();
    }
    else if (to.lastError().type() != QSqlError::None)
    {
      systemError(this, to.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void transferOrder::sFillItemList()
{
  q.prepare( "SELECT MIN(toitem_schedshipdate) AS shipdate "
	     "FROM toitem "
	     "WHERE ((toitem_status <> 'X')"
	     "  AND  (toitem_tohead_id=:head_id));" );

  q.bindValue(":head_id", _toheadid);
  q.exec();
  if (q.first())
  {
    _shipDate->setDate(q.value("shipdate").toDate());

    if (cNew == _mode)
      _packDate->setDate(q.value("shipdate").toDate());
  }
  else
  {
    _shipDate->clear();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  _toitem->clear();
  _srcWhs->setEnabled(true);
  _trnsWhs->setEnabled(true);
  _dstWhs->setEnabled(true);

  QString sql("SELECT toitem_id,"
	      "       toitem_status,"
	      "       CASE WHEN (toitem_status='C') THEN 1"
	      "            WHEN (toitem_status='X') THEN 4"
	      "            WHEN ((toitem_status='O')"
	      "                  AND ((qtyAtShipping('TO', toitem_id) > 0) OR"
	      "                          (toitem_qty_shipped > 0) ) ) THEN 2"
	      "            ELSE 3"
	      "       END AS closestatus,"
	      "       toitem_linenumber, item_number,"
	      "       (item_descrip1 || ' ' || item_descrip2) AS description,"
	      "       formatDate(toitem_schedshipdate) AS f_scheddate,"
	      "       formatQty(toitem_qty_ordered) AS f_ordered,"
	      "       formatQty(toitem_qty_shipped) AS f_shipped,"
	      "       formatQty(noNeg(toitem_qty_ordered - toitem_qty_shipped)) AS f_balance,"
	      "       qtyAtShipping('TO', toitem_id) AS atshipping,"
	      "       formatQty(qtyAtShipping('TO', toitem_id)) AS f_atshipping,"
	      "       formatQty(toitem_qty_received) AS f_received,"
	      "       (noNeg(toitem_qty_ordered - toitem_qty_shipped) <> qtyAtShipping('TO', toitem_id)) AS tagged,"
	      "       toitem_qty_shipped, toitem_qty_received, "
	      "       CASE WHEN toitem_schedshipdate > CURRENT_DATE THEN 1"
	      "            ELSE 0"
	      "       END AS in_future "
	      "FROM item, toitem "
	      "WHERE ((toitem_item_id=item_id)"
	      "<? if exists(\"showCanceled\") ?>"
	      "<? else ?>"
	      "  AND  (toitem_status != 'X')"
	      "<? endif ?>"
	      "  AND  (toitem_tohead_id=<? value(\"tohead_id\") ?>) ) "
	      "ORDER BY toitem_linenumber;" );

  ParameterList params;
  params.append("tohead_id", _toheadid);
  if (_showCanceled->isChecked())
    params.append("showCanceled");
	     
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    XSqlQuery partial;
    partial.prepare( "SELECT toitem_id "
		     "FROM toitem "
		     "WHERE ( (toitem_status='C')"
		     " AND (toitem_tohead_id=:tohead_id) ) "
		     "LIMIT 1;" );
    partial.bindValue(":tohead_id", _toheadid);
    partial.exec();
    bool backOrderFlag = partial.first();

    XTreeWidgetItem *last = 0;
    do
    {
     last = new XTreeWidgetItem(_toitem, last,
			 q.value("toitem_id").toInt(),
			 q.value("closestatus").toInt(),
			 q.value("toitem_linenumber"), q.value("item_number"),
			 q.value("description"),
			 q.value("toitem_status"), q.value("f_scheddate"),
			 q.value("f_ordered"), q.value("f_atshipping"),
			 q.value("f_shipped"),
			 q.value("f_balance"),
			 q.value("f_received")
			 );

      if ( (backOrderFlag) && (q.value("toitem_status").toString() != "C") && (q.value("toitem_status").toString() != "X") )
	last->setTextColor("red");

      if (q.value("toitem_qty_received").toDouble() > 0)
      {
	_dstWhs->setEnabled(false);
	_trnsWhs->setEnabled(false);
	_srcWhs->setEnabled(false);
      }
      else if (q.value("toitem_qty_shipped").toDouble() > 0)
      {
	_trnsWhs->setEnabled(false);
	_srcWhs->setEnabled(false);
      }
      else if (q.value("atshipping").toDouble() > 0)
      {
	_srcWhs->setEnabled(false);
      }

      if(_metrics->boolean("EnableTOShipping"))
      {
	if (q.value("tagged").toBool())
	  last->setTextColor("red");
	if ((q.value("in_future").toBool()) && (q.value("tagged").toBool()))
	  last->setTextColor("darkgreen");
      }
    }
    while (q.next());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT formatQty(SUM(COALESCE(toitem_qty_ordered, 0.00) *"
	    "                 (COALESCE(item_prodweight, 0.00) +"
	    "                  COALESCE(item_packweight, 0.00)))) AS grossweight,"
	    "       SUM(toitem_freight) AS linefreight "
	    "FROM toitem, item "
	    "WHERE ((toitem_item_id=item_id)"
	    " AND (toitem_status<>'X')"
	    " AND (toitem_tohead_id=:head_id));");

  q.bindValue(":head_id", _toheadid);
  q.exec();
  if (q.first())
  {
    _weight->setText(q.value("grossweight").toString());
    _itemFreight->setLocalValue(q.value("linefreight").toDouble());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  recalculateTax(); // triggers sCalculateTotal();

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

  if (cNew == _mode && _toitem->topLevelItemCount() > 0)
  {
    if (QMessageBox::question(this, tr("Delete Transfer Order?"),
                          tr("<p>Are you sure you want to delete this "
                             "Transfer Order and its associated Line Items?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return false;
  }

  if (_mode == cNew)
  {
    query.prepare("SELECT deleteTO(:tohead_id) AS result;");
    query.bindValue(":tohead_id", _toheadid);
    query.exec();
    if (query.first())
    {
      int result = query.value("result").toInt();
      if (result < 0)
        systemError(this, storedProcErrorLookup("deleteSO", result),
                    __FILE__, __LINE__);
    }
    else if (query.lastError().type() != QSqlError::NoError)
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
  }

  if(cView != _mode)
  {
    query.prepare("SELECT releaseTohead(:tohead_id) AS result;");
    query.bindValue(":tohead_id", _toheadid);
    query.exec();
    if (query.first() && ! query.value("result").toBool())
      systemError(this, tr("Could not release this Transfer Order record."),
                  __FILE__, __LINE__);
    else if (query.lastError().type() != QSqlError::NoError)
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
  }

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
  {
    q.prepare("SELECT releaseTohead(:tohead_id) AS result;");
    q.bindValue(":tohead_id", _toheadid);
    q.exec();
    if (q.first() && ! q.value("result").toBool())
      systemError(this, tr("Could not release this Transfer Order record."),
                  __FILE__, __LINE__);
    else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }
  _toheadid = -1;

  _tabs->setCurrentPage(0);

  _orderNumber->setEnabled(TRUE);
  _orderNumberGen = 0;
  _orderNumber->clear();

  _shipDate->clear();
  _packDate->clear();
  _srcWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  _trnsWhs->setId(_metrics->value("DefaultTransitWarehouse").toInt());
  _dstWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  _status->setText("O");
  _agent->setCurrentItem(-1);
  _srcAddr->setId(-1);
  _dstAddr->setId(-1);
  _srcContact->clear();
  _dstContact->clear();
  _taxauthidCache = -1;
  _taxauth->setId(-1);
  _whstaxauthid        = -1;
  _shipVia->setId(-1);
  _shippingForm->setId(-1);
  _freight->clear();
  _orderComments->clear();
  _shippingComments->clear();
  _taxCache.clear();
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
    setName("transferOrder new");
    _orderDate->setDate(omfgThis->dbDate(), true);
  }

  populateOrderNumber();
  if (_orderNumber->text().isEmpty())
    _orderNumber->setFocus();
  else
    _srcWhs->setFocus();

  XSqlQuery headid;
  headid.exec("SELECT NEXTVAL('tohead_tohead_id_seq') AS toheadid");

  if (headid.first())
    setToheadid(headid.value("toheadid").toInt());
  else if (headid.lastError().type() != QSqlError::None)
    systemError(this, headid.lastError().databaseText(), __FILE__, __LINE__);

  if (! insertPlaceholder())
    return;

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

  if (cNew == _mode && _saved)
    omfgThis->sTransferOrdersUpdated(-1);

  QMainWindow::closeEvent(pEvent);
}

void transferOrder::dragEnterEvent(QDragEnterEvent *pEvent)
{
  if (!_srcWhs->isValid())
  {
    message(tr("<p>You must select a Source Warehouse for this Transfer Order before you "
               "may add Line Items to it."), 5000);
    pEvent->accept(FALSE);
  }
  else
  {
    QString dragData;

    if (Q3TextDrag::decode(pEvent, dragData))
    {
      if (dragData.contains("itemid="))
        pEvent->accept(TRUE);
    }
    else
      pEvent->accept(FALSE);
  }
}

void transferOrder::dropEvent(QDropEvent *pEvent)
{
  QString dropData;

  if (Q3TextDrag::decode(pEvent, dropData))
  {
    if (dropData.contains("itemid="))
    {
      QString target = dropData.mid((dropData.find("itemid=") + 7), (dropData.length() - 7));

      if (target.contains(","))
        target = target.left(target.find(","));

      ParameterList params;
      params.append("tohead_id",	_toheadid);
      params.append("orderNumber", _orderNumber->text().toInt());
      params.append("item_id", target.toInt());
      params.append("curr_id", _freight->id());
      params.append("orderDate", _orderDate->date());

      if (_mode == cNew)
        params.append("mode", "new");

      transferOrderItem newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();

      sFillItemList();
    }
  }
}

void transferOrder::sHandleShipchrg(int pShipchrgid)
{
  if ( (_mode == cView) )
    _freight->setEnabled(FALSE);
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
        _freight->setEnabled(TRUE);
      else
      {
        _freight->setEnabled(FALSE);
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
    taxq.prepare("UPDATE tohead SET tohead_taxauth_id=:taxauth, "
		  "  tohead_freight=:freight,"
		  "  tohead_orderdate=:date "
		  "WHERE (tohead_id=:head_id);");
    if (_taxauth->isValid())
      taxq.bindValue(":taxauth", _taxauth->id());
    taxq.bindValue(":freight", _freight->localValue());
    taxq.bindValue(":date",    _orderDate->date());
    taxq.bindValue(":head_id", _toheadid);
    taxq.exec();
    if (taxq.lastError().type() != QSqlError::None)
    {
      systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  ParameterList params;
  params.append("order_id", _toheadid);
  params.append("order_type", "TO");
  params.append("mode", "view"); // because tohead has no fields to hold changes

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() == QDialog::Accepted)
  {
    populate();
  }
}

void transferOrder::setViewMode()
{
  if(cEdit == _mode)
  {
    // Undo some changes set for the edit mode
    _captive = false;
    setAcceptDrops(false);
  }
  _new->setEnabled(false);

  disconnect(omfgThis, SIGNAL(transferOrdersUpdated(int)), this, SLOT(sHandleTransferOrderEvent(int)));

  _mode = cView;
  setName(QString("transferOrder view %1").arg(_toheadid));

  _orderNumber->setEnabled(FALSE);
  _packDate->setEnabled(FALSE);
  _srcWhs->setEnabled(FALSE);
  _dstWhs->setEnabled(FALSE);
  _trnsWhs->setEnabled(FALSE);
  _agent->setEnabled(FALSE);
  _taxauth->setEnabled(FALSE);
  _shipVia->setEnabled(FALSE);
  _shippingForm->setEnabled(FALSE);
  _freight->setEnabled(FALSE);
  _orderComments->setEnabled(FALSE);
  _shippingComments->setEnabled(FALSE);
  _qeitemView->setEnabled(FALSE);
  _qesave->setEnabled(FALSE);
  _qedelete->setEnabled(FALSE);
  _qecurrency->setEnabled(FALSE);
  _freightCurrency->setEnabled(FALSE);

  _edit->setText(tr("View"));
  _comments->setReadOnly(true);
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
#ifdef Q_WS_MAC
  if(e->key() == Qt::Key_N && e->state() == Qt::ControlModifier)
  {
    _new->animateClick();
    e->accept();
  }
  else if(e->key() == Qt::Key_E && e->state() == Qt::ControlModifier)
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
    QWidgetList list = omfgThis->workspace()->windowList(QWorkspace::CreationOrder);
    for(int i = 0; i < list.size(); i++)
    {
      QWidget * w = list.at(i);
      if(QString::compare(w->name(), "transferOrder new")==0)
      {
        w->setFocus();
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
  QWidgetList list = omfgThis->workspace()->windowList(QWorkspace::CreationOrder);
  for(int i = 0; i < list.size(); i++)
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
  QWidgetList list = omfgThis->workspace()->windowList(QWorkspace::CreationOrder);
  for(int i = 0; i < list.size(); i++)
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
  params.append("tohead_id", pId);

  transferOrder *newdlg = new transferOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void transferOrder::sReturnStock()
{
  q.prepare("SELECT returnItemShipments('TO', :toitem_id, 0, CURRENT_TIMESTAMP) AS result;");
  QList<QTreeWidgetItem*> selected = _toitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.bindValue(":toitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("returnItemShipments", result) +
                          tr("<br>Line Item %1").arg(selected[i]->text(0)),
                           __FILE__, __LINE__);
        return;
      }
      distributeInventory::SeriesAdjust(result, this);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                        q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillItemList();
}

void transferOrder::sIssueStock()
{
  bool update  = FALSE;
  QList<QTreeWidgetItem*> selected = _toitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem* toitem = (XTreeWidgetItem*)(selected[i]);
    if (toitem->altId() != 1 && toitem->altId() != 4)
    {
      ParameterList params;
      params.append("toitem_id", toitem->id());

      if(_requireInventory->isChecked())
        params.append("requireInventory");

      issueLineToShipping newdlg(this, "", TRUE);
      newdlg.set(params);
      if (newdlg.exec() != QDialog::Rejected)
        update = TRUE;
    }
  }

  if (update)
    sFillItemList();
}

void transferOrder::sIssueLineBalance()
{
  QList<QTreeWidgetItem*> selected = _toitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem* toitem = (XTreeWidgetItem*)(selected[i]);
    if (toitem->altId() != 1 && toitem->altId() != 4)
    {

      if(_requireInventory->isChecked())
      {
	q.prepare("SELECT sufficientInventoryToShipItem('TO', :toitem_id) AS result;");
	q.bindValue(":toitem_id", toitem->id());
	q.exec();
	if (q.first())
	{
	  int result = q.value("result").toInt();
	  if (result < 0)
	  {
	    q.prepare("SELECT item_number, tohead_srcname "
		      "  FROM toitem, tohead, item "
		      " WHERE ((toitem_item_id=item_id)"
		      "   AND  (toitem_tohead_id=tohead_id)"
		      "   AND  (toitem_id=:toitem_id)); ");
	    q.bindValue(":toitem_id", toitem->id());
	    q.exec();
	    if (! q.first() && q.lastError().type() != QSqlError::None)
		systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	    systemError(this,
			storedProcErrorLookup("sufficientInventoryToShipItem",
					      result)
			.arg(q.value("item_number").toString())
			.arg(q.value("tohead_srcname").toString()), __FILE__, __LINE__);
	    return;
	  }
	}
	else if (q.lastError().type() != QSqlError::None)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }

      q.prepare("SELECT issueLineBalanceToShipping('TO', :toitem_id, CURRENT_TIMESTAMP) AS result;");
      q.bindValue(":toitem_id", toitem->id());
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("issueLineBalance", result) +
                            tr("<br>Line Item %1").arg(selected[i]->text(0)),
                      __FILE__, __LINE__);
          return;
        }
        distributeInventory::SeriesAdjust(q.value("result").toInt(), this);
      }
      else
      {
        systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                          q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }

  sFillItemList();
}

void transferOrder::sFreightChanged()
{
  XSqlQuery freightq;
  freightq.prepare("SELECT calculateTax(:tax_id, :freight, 0, 'A') AS freighta,"
                   "     calculateTax(:tax_id, :freight, 0, 'B') AS freightb,"
                   "     calculateTax(:tax_id, :freight, 0, 'C') AS freightc;");
  freightq.bindValue(":tax_id", _taxCache.freightId());
  freightq.bindValue(":freight", _freight->localValue());
  freightq.exec();
  if (freightq.first())
  {
    _taxCache.setFreight(freightq.value("freighta").toDouble(),
			 freightq.value("freightb").toDouble(),
			 freightq.value("freightc").toDouble());
  }
  else if (freightq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, freightq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  recalculateTax();
}

void transferOrder::recalculateTax()
{
  XSqlQuery itemq;

  itemq.prepare( "SELECT SUM(toitem_freighttax_ratea) AS itemtaxa,"
		 "       SUM(toitem_freighttax_rateb) AS itemtaxb,"
		 "       SUM(toitem_freighttax_ratec) AS itemtaxc "
		 "FROM toitem "
		 "WHERE ((toitem_tohead_id=:head_id)"
		 "  AND  (toitem_status != 'X'));" );

  itemq.bindValue(":head_id", _toheadid);
  itemq.exec();
  if (itemq.first())
  {
    _taxCache.setLine(itemq.value("itemtaxa").toDouble(),
		      itemq.value("itemtaxb").toDouble(),
		      itemq.value("itemtaxc").toDouble());
  }
  else if (itemq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _tax->setLocalValue(_taxCache.total());
  sCalculateTotal();
}

void transferOrder::sTaxAuthChanged()
{
  XSqlQuery taxauthq;
  if (_taxauth->id() != _taxauthidCache && _toheadid > 0)
  {
    taxauthq.prepare("SELECT changeTOTaxAuth(:head_id, :taxauth_id) AS result;");
    taxauthq.bindValue(":head_id", _toheadid);
    taxauthq.bindValue(":taxauth_id", _taxauth->id());
    taxauthq.exec();
    if (taxauthq.first())
    {
      int result = taxauthq.value("result").toInt();
      if (result < 0)
      {
        _taxauth->setId(_taxauthidCache);
        systemError(this,
                    storedProcErrorLookup("changeTOTaxAuth", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (taxauthq.lastError().type() != QSqlError::None)
    {
      _taxauth->setId(_taxauthidCache);
      systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _taxauthidCache = _taxauth->id();
  }

  taxauthq.prepare("SELECT COALESCE(getFreightTaxSelection(:taxauth), -1) AS result;");
  taxauthq.bindValue(":taxauth", _taxauth->id());
  taxauthq.exec();
  if (taxauthq.first())
  {
    // -1 => there is none, which isn't an error, hence no error lookup
    _taxCache.setFreightId(taxauthq.value("result").toInt());
  }
  else if (taxauthq.lastError().type() != QSqlError::None)
  {
    systemError(this, taxauthq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  recalculateTax();
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
      _taxauth->setId(whsq.value("warehous_taxauth_id").toInt());
    }
    else if (pwidget == _trnsWhs)
    {
      _shippingForm->setId(whsq.value("warehous_shipform_id").toInt());
      _shipVia->setId(whsq.value("warehous_shipvia_id").toInt());
      _shippingComments->setText(whsq.value("warehous_shipcomments").toString());
    }
  }
  else if (whsq.lastError().type() != QSqlError::None)
  {
    systemError(this, whsq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
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
