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

#include "openSalesOrders.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "copySalesOrder.h"
#include "creditcardprocessor.h"
#include "deliverSalesOrder.h"
#include "printPackingList.h"
#include "printSoForm.h"
#include "rescheduleSoLineItems.h"
#include "salesOrder.h"
#include "storedProcErrorLookup.h"

openSalesOrders::openSalesOrders(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_so, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));

  _so->addColumn(tr("S/O #"),            _orderColumn, Qt::AlignLeft   );
  _so->addColumn(tr("Cust. #"),          _orderColumn, Qt::AlignLeft   );
  _so->addColumn(tr("Customer"),         -1,           Qt::AlignLeft   );
  _so->addColumn(tr("Cust. P/O Number"), _itemColumn,  Qt::AlignLeft   );
  _so->addColumn(tr("Ordered"),          _dateColumn,  Qt::AlignCenter );
  _so->addColumn(tr("Scheduled"),        _dateColumn,  Qt::AlignCenter );
  
  if (_privleges->check("MaintainSalesOrders"))
  {
    connect(_so, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_so, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_so, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_so, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_so, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  sFillList();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

openSalesOrders::~openSalesOrders()
{
  // no need to delete child widgets, Qt does it all for us
}

void openSalesOrders::languageChange()
{
  retranslateUi(this);
}

void openSalesOrders::setParams(ParameterList &params)
{
  params.append("error", tr("Error"));
  _warehouse->appendValue(params);
}

void openSalesOrders::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("ListOpenSalesOrders", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void openSalesOrders::sNew()
{
  salesOrder::newSalesOrder(-1);
}

void openSalesOrders::sEdit()
{
  salesOrder::editSalesOrder(_so->id(), false);
}

void openSalesOrders::sView()
{
  salesOrder::viewSalesOrder(_so->id());
}

void openSalesOrders::sCopy()
{
  ParameterList params;
  params.append("sohead_id", _so->id());
      
  copySalesOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void openSalesOrders::sReschedule()
{
  ParameterList params;
  params.append("sohead_id", _so->id());
      
  rescheduleSoLineItems newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void openSalesOrders::sDelete()
{
  if ( QMessageBox::warning(this, tr("Delete Sales Order?"),
                            tr("<p>Are you sure that you want to completely "
			       "delete the selected Sales Order?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteSo(:sohead_id) AS result;");
    q.bindValue(":sohead_id", _so->id());
    q.exec();
    if (q.first())
    {
      bool closeInstead = false;
      int result = q.value("result").toInt();
      if (result == -4 && _privleges->check("ProcessCreditCards"))
      {
	if ( QMessageBox::question(this, tr("Cannot Delete Sales Order"),
				   storedProcErrorLookup("deleteSo", result) + 
				   "<br>Would you like to refund the amount "
				   "charged and close the Sales Order instead?",
				   QMessageBox::Yes | QMessageBox::Default,
				   QMessageBox::No) == QMessageBox::Yes)
	{
	  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
	  if (! cardproc)
	    QMessageBox::critical(this, tr("Credit Card Processing Error"),
				  CreditCardProcessor::errorMsg());
	  else
	  {
	    XSqlQuery ccq;
	    ccq.prepare("SELECT ccpay_ccard_id, ccpay_curr_id,"
			"       SUM(ccpay_amount     * sense) AS amount,"
			"       SUM(ccpay_r_tax      * sense) AS tax,"
			"       SUM(ccpay_r_shipping * sense) AS freight "
			"FROM (SELECT ccpay_ccard_id, ccpay_curr_id, "
			"             CASE WHEN ccpay_status = 'C' THEN  1"
			"                  WHEN ccpay_status = 'R' THEN -1"
			"             END AS sense,"
			"             ccpay_amount,"
			"             COALESCE(ccpay_r_tax::NUMERIC, 0) AS ccpay_r_tax,"
			"             COALESCE(ccpay_r_shipping::NUMERIC, 0) AS ccpay_r_shipping "
			"      FROM ccpay, payco "
			"      WHERE ((ccpay_id=payco_ccpay_id)"
			"        AND  (ccpay_status IN ('C', 'R'))"
			"        AND  (payco_cohead_id=:coheadid)) "
			"      ) AS dummy "
			"GROUP BY ccpay_ccard_id, ccpay_curr_id;");
	    ccq.bindValue(":coheadid", _so->id());
	    ccq.exec();
	    if (ccq.first())
	    do
	    {
	      QString docnum = _so->currentItem()->text(0);
	      QString refnum = docnum;
	      int ccpayid = -1;
	      int coheadid = _so->id();
	      int returnVal = cardproc->credit(ccq.value("ccpay_ccard_id").toInt(),
					       -2,
					       ccq.value("amount").toDouble(),
					       ccq.value("tax").toDouble(),
					       true,
					       ccq.value("freight").toDouble(),
					       0,
					       ccq.value("ccpay_curr_id").toInt(),
					       docnum, refnum, ccpayid,
					       "cohead", coheadid);
	      if (returnVal < 0)
	      {
		QMessageBox::critical(this, tr("Credit Card Processing Error"),
				      cardproc->errorMsg());
		return;
	      }
	      else if (returnVal > 0)
	      {
		QMessageBox::warning(this, tr("Credit Card Processing Warning"),
				     cardproc->errorMsg());
		closeInstead = true;
	      }
	      else if (! cardproc->errorMsg().isEmpty())
	      {
		QMessageBox::information(this, tr("Credit Card Processing Note"),
				     cardproc->errorMsg());
		closeInstead = true;
	      }
	    } while (ccq.next());
	    else if (ccq.lastError().type() != QSqlError::None)
	    {
	      systemError(this, ccq.lastError().databaseText(),
			  __FILE__, __LINE__);
	      return;
	    }
	    else
	    {
	      systemError(this, tr("Could not find the ccpay records!"),
			  __FILE__, __LINE__);
	      return;
	    }

	  }
	}
      }
      else if (result == -1 || result == -5)
      {
	if ( QMessageBox::question(this, tr("Cannot Delete Sales Order"),
				   storedProcErrorLookup("deleteSo", result) + 
				   "<br>Would you like to Close the selected "
				   "Sales Order instead?",
				   QMessageBox::Yes | QMessageBox::Default,
				   QMessageBox::No) == QMessageBox::Yes)
	  closeInstead = true;
      }
      else if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteSo", result),
		    __FILE__, __LINE__);
	return;
      }

      if (closeInstead)
      {
	q.prepare( "UPDATE coitem "
		   "SET coitem_status='C' "
		   "WHERE ((coitem_status <> 'X')"
		   "  AND  (coitem_cohead_id=:sohead_id));" );
	q.bindValue(":sohead_id", _so->id());
	q.exec();
	if (q.lastError().type() != QSqlError::None)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }

      omfgThis->sSalesOrdersUpdated(-1);
      omfgThis->sProjectsUpdated(-1);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void openSalesOrders::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

void openSalesOrders::sPrintPackingList()
{
  ParameterList params;
  params.append("sohead_id", _so->id());

  printPackingList newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void openSalesOrders::sAddToPackingListBatch()
{
  q.prepare("SELECT addToPackingListBatch(:sohead_id) AS result;");
  q.bindValue(":sohead_id", _so->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("addToPackingListBatch", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void openSalesOrders::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privleges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Copy..."), this, SLOT(sCopy()), 0);
  if (!_privleges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Reschedule..."), this, SLOT(sReschedule()), 0);
  if (!_privleges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  if (!_privleges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Print Packing List..."), this, SLOT(sPrintPackingList()), 0);
  if (!_privleges->check("PrintPackingLists"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Add to Packing List Batch..."), this, SLOT(sAddToPackingListBatch()), 0);
  if (!_privleges->check("MaintainPackingListBatch"))
    pMenu->setItemEnabled(menuItem, FALSE);

  if (_metrics->boolean("EnableBatchManager"))
  {
    menuItem = pMenu->insertItem(tr("Email Order Acknowledgment..."), this, SLOT(sDeliver()), 0);
  }

  menuItem = pMenu->insertItem(tr("Print Sales Order Form..."), this, SLOT(sPrintForms()), 0); 

}


void openSalesOrders::sFillList()
{
  ParameterList params;
  setParams(params);

  QString sql( "SELECT DISTINCT cohead_id, cohead_number,"
               "       COALESCE(cust_number, :error),"
               "       cohead_billtoname, cohead_custponumber,"
               "       formatDate(cohead_orderdate) AS f_ordered,"
               "       formatDate(MIN(coitem_scheddate)) AS f_scheduled "
               "FROM cohead LEFT OUTER JOIN cust ON (cohead_cust_id=cust_id) "
               "     LEFT OUTER JOIN coitem JOIN itemsite ON (coitem_itemsite_id=itemsite_id) "
               "     ON (coitem_cohead_id=cohead_id) "
               "WHERE (((coitem_status = 'O') OR (coitem_status IS NULL)) "
	       "<? if  exists(\"warehous_id\") ?>"
	       " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	       "<? endif ?>"
	       " ) "
	       "GROUP BY cohead_id, cohead_number, cust_number, cohead_billtoname,"
	       "         cohead_custponumber, cohead_orderdate "
	       "ORDER BY cohead_number " );
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);

  _so->populate(q);
  _so->setDragString("soheadid=");
}

void openSalesOrders::sDeliver()
{
  ParameterList params;
  params.append("sohead_id", _so->id());

  deliverSalesOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void openSalesOrders::sPrintForms()
{
  ParameterList params;
  params.append("sohead_id", _so->id());

  printSoForm newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}
