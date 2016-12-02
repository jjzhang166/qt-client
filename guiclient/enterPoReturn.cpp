/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "enterPoReturn.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <metasql.h>

#include "distributeInventory.h"
#include "enterPoitemReturn.h"
#include "storedProcErrorLookup.h"
#include "postPoReturnCreditMemo.h"
#include "errorReporter.h"

enterPoReturn::enterPoReturn(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_enter,	SIGNAL(clicked()),	this,	SLOT(sEnter()));
  connect(_po,      SIGNAL(newId(int, const QString&)), this, SLOT(sFillList()));
  connect(_post,	SIGNAL(clicked()),	this,	SLOT(sPost()));
  
  _poitem->addColumn(tr("#"),            _whsColumn,  Qt::AlignCenter , true, "poitem_linenumber");
  _poitem->addColumn(tr("Site"),         _whsColumn,  Qt::AlignLeft   , true, "warehous_code");
  _poitem->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft   , true, "item_number");
  _poitem->addColumn(tr("Description"),  -1,          Qt::AlignLeft   , true, "itemdescription");
  _poitem->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter , true, "inv_uom");
  _poitem->addColumn(tr("Vend. Item #"), -1,          Qt::AlignLeft   , true, "poitem_vend_item_number");
  _poitem->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter , true, "poitem_vend_uom");
  _poitem->addColumn(tr("Ordered"),      _qtyColumn,  Qt::AlignRight  , true, "poitem_qty_ordered");
  _poitem->addColumn(tr("Received"),     _qtyColumn,  Qt::AlignRight  , true, "poitem_qty_received");
  _poitem->addColumn(tr("Returned"),     _qtyColumn,  Qt::AlignRight  , true, "poitem_qty_returned");
  _poitem->addColumn(tr("To Return"),    _qtyColumn,  Qt::AlignRight  , true, "poitem_qty_toreturn");

  _captive = false;
  _dropshipWarn = new XErrorMessage(this);
  _returnAddr->setEnabled(_printReport->isChecked());
  _po->setAllowedStatuses(OrderLineEdit::Open | OrderLineEdit::Closed);
  _po->setAllowedTypes(OrderLineEdit::Purchase);
  _po->setLockSelected(true);

  if (_metrics->boolean("EnableDropShipments"))
    _dropShip->setEnabled(false);
  else
    _dropShip->hide();
}

enterPoReturn::~enterPoReturn()
{
  // no need to delete child widgets, Qt does it all for us
}

void enterPoReturn::languageChange()
{
  retranslateUi(this);
}

enum SetResponse enterPoReturn::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _captive = true;
    _po->setId(param.toInt());
  }

  return NoError;
}

void enterPoReturn::sPost()
{
  XSqlQuery enterPost;
  bool createMemo = false;
  
  enterPost.prepare("SELECT SUM(poreject_qty) AS qtyToReturn, MIN(poitem_status) AS poitem_status "
            "FROM poreject, poitem "
            "WHERE ((poreject_poitem_id=poitem_id)"
            "  AND  (NOT poreject_posted)"
	    "  AND  (poitem_pohead_id=:pohead_id));");
  enterPost.bindValue(":pohead_id", _po->id());
  enterPost.exec();
  if (enterPost.first())
  {
    if (enterPost.value("qtyToReturn").toDouble() == 0)
    {
      QMessageBox::critical(this, tr("Nothing selected for Return"),
			    tr("<p>No Purchase Order Items have been selected "
			       "to be returned. Select at least one Line Item "
			       "and enter a Return before trying to Post."));
      return;
    }
  //Offer to create credit memo
    if ( (_privileges->check("MaintainAPMemos")) && (enterPost.value("poitem_status").toString() == "C") )
      if ( QMessageBox::question( this, tr("Create Credit Memo"),
                                        tr("One or more line items on this P/O are closed. \n"
                                           "Would you like to automatically create a credit memo against this return?"),
                                        tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0 )
        createMemo = true;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting P/O Return"),
                                enterPost, __FILE__, __LINE__))
  {
    return;
  }

  int saveResult = _returnAddr->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    if (QMessageBox::question(this,
		    tr("Saving Shared Address"),
		    tr("<p>There are multiple uses for this Address. "
		       "If you save this Address, the Address for all "
		       "of these uses will be changed. Would you like to "
		       "save this Address?"),
		    QMessageBox::No | QMessageBox::Default, QMessageBox::Yes))
	saveResult = _returnAddr->save(AddressCluster::CHANGEALL);
  }
  if (0 > saveResult)	// NOT else if
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Data"),
                         tr("%1: There was an error saving P/O Return Information. "
                            "Check the database server log for errors.")
                         .arg(windowTitle()),
                         __FILE__,__LINE__);
  }

  // print while we can still differentiate current from previous returns
  if (_printReport->isChecked())
  {
    ParameterList params;
    params.append("pohead_id", _po->id());
    if (_returnAddr->isValid())
      params.append("addr_id", _returnAddr->id());
    orReport report("UnpostedReturnsForPO", params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      return;
    }
  }
  
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  // Get the list of returns that are going to be posted
  XSqlQuery reject;
  reject.prepare("SELECT poreject_id"
                 "  FROM poreject, poitem"
                 " WHERE((NOT poreject_posted)"
                 "   AND (poreject_poitem_id=poitem_id)"
                 "   AND (poitem_pohead_id=:pohead_id));");
  reject.bindValue(":pohead_id", _po->id());
  reject.exec();

  enterPost.exec("BEGIN;");	// because of possible insertgltransaction failures

  // post the returns
  enterPost.prepare("SELECT postPoReturns(:pohead_id,false) AS result;");
  enterPost.bindValue(":pohead_id", _po->id());
  enterPost.exec();
  int result = 0;
  if (enterPost.first())
  {
    result = enterPost.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting P/O Return"),
                             storedProcErrorLookup("postPoReturns", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (enterPost.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting P/O Return"),
                         enterPost, __FILE__, __LINE__);
    return;
  }

  if (distributeInventory::SeriesAdjust(enterPost.value("result").toInt(), this) == XDialog::Rejected)
  {
    rollback.exec();
    QMessageBox::information( this, tr("Enter PO Return"), tr("Transaction Canceled") );
    return;
  }
  
  enterPost.exec("COMMIT;");

  // if we are creating the credit memo go ahead and loop the returns that
  // we have just posted and create the credit memos.
  if(createMemo)
  {
    while(reject.next())
    {
      ParameterList params;
      params.append("poreject_id", reject.value("poreject_id").toInt());

      postPoReturnCreditMemo newdlg(this, "", true);
      newdlg.set(params);

      newdlg.exec();
    }
  }


  if (_captive)
    close();
  else
  {
    _close->setText(tr("&Close"));
    _po->setId(-1);
  }
}

void enterPoReturn::sEnter()
{
  ParameterList params;
  params.append("poitem_id", _poitem->id());
  params.append("poreject_rma", _rma->text());
  if(_dropShip->isChecked())
    _dropshipWarn->showMessage(tr("Returns made against Drop Shipped Purchase Orders "
                                  "will not reverse shipment transactions generated "
                                  "by the original receipt. Shipment transactions should "
                                  "be reversed separately if necessary."));

  enterPoitemReturn newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec())
    sFillList();
}

void enterPoReturn::sFillList()
{
  XSqlQuery enterFillList;
  _poitem->clear();

  if (_po->id() != -1)
  {
    enterFillList.prepare( "SELECT 1, vendaddr_addr_id AS addr_id, pohead_dropship "
	       "FROM vendaddrinfo, pohead "
	       "WHERE ((vendaddr_id=pohead_vendaddr_id)"
	       "  AND  (pohead_id=:pohead_id))"
	       "UNION "
	       "SELECT 2, vend_addr_id AS addr_id, pohead_dropship "
	       "FROM vendinfo, pohead "
	       "WHERE ((vend_id=pohead_vend_id)"
	       "  AND  (pohead_id=:pohead_id)) "
	       "ORDER BY 1 "
	       "LIMIT 1;");
    enterFillList.bindValue(":pohead_id", _po->id());
    enterFillList.exec();
    if (enterFillList.first())
	{
      _returnAddr->setId(enterFillList.value("addr_id").toInt());
	  _dropShip->setChecked(enterFillList.value("pohead_dropship").toBool());
	}
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving P/O Item Information"),
                                  enterFillList, __FILE__, __LINE__))
    {
      return;
    }

    QString sql( "SELECT poitem_id, poitem_linenumber,"
               "       warehous_code, "
               "       COALESCE(item_number, <? value(\"nonInventory\") ?>) AS item_number,"
               "       CASE WHEN (LENGTH(TRIM(BOTH '    ' FROM poitem_vend_item_descrip)) <= 0) THEN "
               "                 (item_descrip1 || ' ' || item_descrip2) "
               "            ELSE poitem_vend_item_descrip "
               "       END AS itemdescription, "
               "       COALESCE(uom_name, <? value(\"na\") ?>) AS inv_uom,"
               "       poitem_vend_item_number, poitem_vend_uom,"
               "       poitem_qty_ordered,"
               "       poitem_qty_received,"
               "       poitem_qty_returned,"
               "       COALESCE( ( SELECT SUM(poreject_qty)"
               "                              FROM poreject"
               "                              WHERE ( (poreject_poitem_id=poitem_id)"
               "                               AND (NOT poreject_posted) ) ), 0 ) AS poitem_qty_toreturn, "
               "      'qty' AS poitem_qty_ordered_xtnumericrole, "
               "      'qty' AS poitem_qty_received_xtnumericrole, "
               "      'qty' AS poitem_qty_returned_xtnumericrole, "
               "      'qty' AS poitem_qty_toreturn_xtnumericrole "
               "FROM poitem LEFT OUTER JOIN "
               "     ( itemsite "
               "        JOIN item ON (itemsite_item_id=item_id) "
               "        JOIN uom ON (item_inv_uom_id=uom_id) "
               "        JOIN site() ON (itemsite_warehous_id=warehous_id) "
               "     ) ON (poitem_itemsite_id=itemsite_id) "
               "WHERE (poitem_pohead_id= <? value(\"pohead_id\") ?>) "
               "ORDER BY poitem_linenumber;" );
          
    ParameterList params;
    params.append("na", tr("N/A"));
    params.append("nonInventory", tr("Non-Inventory"));
    params.append("pohead_id", _po->id());
    MetaSQLQuery mql(sql);
    enterFillList = mql.toQuery(params);
    _poitem->populate(enterFillList);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving P/O Item Information"),
                                  enterFillList, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void enterPoReturn::closeEvent(QCloseEvent *pEvent)
{
  XSqlQuery entercloseEvent;
  if (_po->isValid())
  {
//  Cancel all received qty's for this po
    entercloseEvent.prepare( "DELETE FROM poreject "
               "WHERE ( poreject_id IN ( SELECT poreject_id"
               "                         FROM poreject, poitem"
               "                         WHERE ( (poreject_poitem_id=poitem_id)"
               "                          AND (NOT poreject_posted)"
               "                          AND (poitem_pohead_id=:pohead_id) ) ) );" );
    entercloseEvent.bindValue(":pohead_id", _po->id());
    entercloseEvent.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving P/O Return Information"),
                                  entercloseEvent, __FILE__, __LINE__))
    {
      return;
    }
  }

  XWidget::closeEvent(pEvent);
}
