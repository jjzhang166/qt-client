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

#include "enterPoReturn.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "distributeInventory.h"
#include "enterPoitemReturn.h"
#include "storedProcErrorLookup.h"
#include "postPoReturnCreditMemo.h"

enterPoReturn::enterPoReturn(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_enter,	SIGNAL(clicked()),	this,	SLOT(sEnter()));
  connect(_po,		SIGNAL(newId(int)),	this,	SLOT(sFillList()));
  connect(_post,	SIGNAL(clicked()),	this,	SLOT(sPost()));
  connect(_showClosed,SIGNAL(toggled(bool)),	this,	SLOT(sShowClosed()));

  _po->setType(cPOOpen);

  _poitem->addColumn(tr("#"),            _whsColumn,  Qt::AlignCenter );
  _poitem->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft   );
  _poitem->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter );
  _poitem->addColumn(tr("Vend. Item #"), -1,          Qt::AlignLeft   );
  _poitem->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter );
  _poitem->addColumn(tr("Ordered"),      _qtyColumn,  Qt::AlignRight  );
  _poitem->addColumn(tr("Received"),     _qtyColumn,  Qt::AlignRight  );
  _poitem->addColumn(tr("Returned"),     _qtyColumn,  Qt::AlignRight  );
  _poitem->addColumn(tr("To Return"),    _qtyColumn,  Qt::AlignRight  );

  _returnAddr->setEnabled(_printReport->isChecked());
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
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _po->setId(param.toInt());
    _poitem->setFocus();
  }

  return NoError;
}

void enterPoReturn::sPost()
{
  bool createMemo = false;
  
  q.prepare("SELECT SUM(poreject_qty) AS qtyToReturn, MIN(poitem_status) AS poitem_status "
            "FROM poreject, poitem "
            "WHERE ((poreject_poitem_id=poitem_id)"
            "  AND  (NOT poreject_posted)"
	    "  AND  (poitem_pohead_id=:pohead_id));");
  q.bindValue(":pohead_id", _po->id());
  q.exec();
  if (q.first())
  {
    if (q.value("qtyToReturn").toDouble() == 0)
    {
      QMessageBox::critical(this, tr("Nothing selected for Return"),
			    tr("<p>No Purchase Order Items have been selected "
			       "to be returned. Select at least one Line Item "
			       "and enter a Return before trying to Post."));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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
    systemError(this, tr("<p>There was an error saving this address (%1). "
			 "Check the database server log for errors.")
		      .arg(saveResult),
		__FILE__, __LINE__);
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
  
  //Offer to create credit memo
  if ( (_privileges->check("MaintainAPMemos")) && (q.value("poitem_status").toString() == "C") )
    if ( QMessageBox::question( this, tr("Create Credit Memo"),
                             tr("One or more line items on this P/O are closed. \n"
								"Would you like to automatically create a credit memo against this return?"),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0 )
      createMemo = true;
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

  q.exec("BEGIN;");	// because of possible insertgltransaction failures

  // post the returns
  q.prepare("SELECT postPoReturns(:pohead_id,false) AS result;");
  q.bindValue(":pohead_id", _po->id());
  q.exec();
  int result = 0;
  if (q.first())
  {
    result = q.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("postPoReturns", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // if we are creating the credit memo go ahead and loop the returns that
  // we have just posted and create the credit memos.
  if(createMemo)
  {
    while(reject.next())
    {
      ParameterList params;
      params.append("poreject_id", reject.value("poreject_id").toInt());

      postPoReturnCreditMemo newdlg(this, "", TRUE);
      newdlg.set(params);

      if(newdlg.exec() != XDialog::Accepted)
      {
        rollback.exec();
        return;
      }
    }
  }

  if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
  {
    rollback.exec();
    QMessageBox::information( this, tr("Enter PO Return"), tr("Transaction Canceled") );
    return;
  }
  
  q.exec("COMMIT;");

  //Offer option to post credit memo for closed line items.	

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

  enterPoitemReturn newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec())
    sFillList();
}

void enterPoReturn::sFillList()
{
  _poitem->clear();

  if (_po->id() != -1)
  {
    q.prepare( "SELECT 1, vendaddr_addr_id AS addr_id "
	       "FROM vendaddrinfo, pohead "
	       "WHERE ((vendaddr_id=pohead_vendaddr_id)"
	       "  AND  (pohead_id=:pohead_id))"
	       "UNION "
	       "SELECT 2, vend_addr_id AS addr_id "
	       "FROM vendinfo, pohead "
	       "WHERE ((vend_id=pohead_vend_id)"
	       "  AND  (pohead_id=:pohead_id)) "
	       "ORDER BY 1 "
	       "LIMIT 1;");
    q.bindValue(":pohead_id", _po->id());
    q.exec();
    if (q.first())
      _returnAddr->setId(q.value("addr_id").toInt());
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT poitem_id, poitem_linenumber,"
               "       COALESCE(item_number, :nonInventory),"
               "       COALESCE(uom_name, :na),"
               "       poitem_vend_item_number, poitem_vend_uom,"
               "       formatQty(poitem_qty_ordered),"
               "       formatQty(poitem_qty_received),"
               "       formatQty(poitem_qty_returned),"
               "       formatQty( COALESCE( ( SELECT SUM(poreject_qty)"
               "                              FROM poreject"
               "                              WHERE ( (poreject_poitem_id=poitem_id)"
               "                               AND (NOT poreject_posted) ) ), 0 ) ) "
               "FROM poitem LEFT OUTER JOIN "
               "     ( itemsite JOIN item "
               "       ON (itemsite_item_id=item_id) JOIN uom ON (item_inv_uom_id=uom_id)"
               "     ) ON (poitem_itemsite_id=itemsite_id) "
               "WHERE (poitem_pohead_id=:pohead_id) "
               "ORDER BY poitem_linenumber;" );
    q.bindValue(":nonInventory", tr("Non-Inventory"));
    q.bindValue(":na", tr("N/A"));
    q.bindValue(":pohead_id", _po->id());
    q.exec();
    _poitem->populate(q);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void enterPoReturn::closeEvent(QCloseEvent *pEvent)
{
  if (_po->isValid())
  {
//  Cancel all received qty's for this po
    q.prepare( "DELETE FROM poreject "
               "WHERE ( poreject_id IN ( SELECT poreject_id"
               "                         FROM poreject, poitem"
               "                         WHERE ( (poreject_poitem_id=poitem_id)"
               "                          AND (NOT poreject_posted)"
               "                          AND (poitem_pohead_id=:pohead_id) ) ) );" );
    q.bindValue(":pohead_id", _po->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    }
  }

  XMainWindow::closeEvent(pEvent);
}

void enterPoReturn::sShowClosed()
{
  if (_showClosed->isChecked())
    _po->setType(cPOClosed & cPOOpen);
  else
    _po->setType(cPOOpen);
}
