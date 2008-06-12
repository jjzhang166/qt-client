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

#include "dspARApplications.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>

#include <datecluster.h>
#include <metasql.h>
#include <parameter.h>
#include <openreports.h>

#include "arOpenItem.h"
#include "creditMemo.h"
#include "dspInvoiceInformation.h"

dspARApplications::dspARApplications(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_arapply,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),
                  this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print,	SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query,	SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
    
  _arapply->addColumn(tr("Cust. #"),   _orderColumn, Qt::AlignCenter );
  _arapply->addColumn(tr("Customer"),            -1, Qt::AlignLeft   );
  _arapply->addColumn(tr("Date"),       _dateColumn, Qt::AlignCenter );
  _arapply->addColumn("hidden source type",      10, Qt::AlignCenter );
  _arapply->addColumn(tr("Source"),	  _itemColumn, Qt::AlignCenter );
  _arapply->addColumn(tr("Doc #"),     _orderColumn, Qt::AlignCenter );
  _arapply->addColumn("hidden target type",      10, Qt::AlignCenter );
  _arapply->addColumn(tr("Apply-To"),   _itemColumn, Qt::AlignCenter );
  _arapply->addColumn(tr("Doc #"),     _orderColumn, Qt::AlignCenter );
  _arapply->addColumn(tr("Amount"), _bigMoneyColumn, Qt::AlignRight  );

  _arapply->hideColumn(3);
  _arapply->hideColumn(6);

  _allCustomers->setFocus();
}

dspARApplications::~dspARApplications()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspARApplications::languageChange()
{
  retranslateUi(this);
}

void dspARApplications::sPrint()
{
  if ( (_selectedCustomer->isChecked()) && (!_cust->isValid()) )
  {
    QMessageBox::warning( this, tr("Select Customer"),
                          tr("You must select a Customer whose A/R Applications you wish to view.") );
    _cust->setFocus();
    return;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  if ( (!_cashReceipts->isChecked()) && (!_creditMemos->isChecked()) )
  {
    QMessageBox::critical( this, tr("Select Document Type"),
                           tr("You must indicate which Document Type(s) you wish to view.") );
    _cashReceipts->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);

  if (_selectedCustomer->isChecked())
    params.append("cust_id", _cust->id());
  else if (_selectedCustomerType->isChecked())
    params.append("custtype_id", _customerTypes->id());
  else if (_customerTypePattern->isChecked())
    params.append("custtype_pattern", _customerType->text());

  if (_cashReceipts->isChecked())
    params.append("showCashReceipts");

  if (_creditMemos->isChecked())
    params.append("showCreditMemos");

  orReport report("ARApplications", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspARApplications::sViewCreditMemo()
{
  ParameterList params;
  params.append("mode", "view");

  q.prepare("SELECT 1 AS type, cmhead_id AS id "
	    "FROM cmhead "
	    "WHERE (cmhead_number=:docnum) "
	    "UNION "
	    "SELECT 2 AS type, aropen_id AS id "
	    "FROM aropen "
	    "WHERE ((aropen_docnumber=:docnum)"
	    "  AND (aropen_doctype IN ('C', 'R')) "
	    ") ORDER BY type LIMIT 1;");
  q.bindValue(":docnum", _arapply->currentItem()->text(5));
  q.exec();
  if (q.first())
  {
    if (q.value("type").toInt() == 1)
    {
      params.append("cmhead_id", q.value("id"));
      creditMemo* newdlg = new creditMemo();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (q.value("type").toInt() == 2)
    {
      params.append("aropen_id", q.value("id"));
      params.append("docType", "creditMemo");
      arOpenItem newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    QMessageBox::information(this, tr("Credit Memo Not Found"),
			     tr("<p>The Credit Memo #%1 could not be found.")
			     .arg(_arapply->currentItem()->text(5)));
    return;
  }
}

void dspARApplications::sViewDebitMemo()
{
  ParameterList params;

  params.append("mode", "view");
  q.prepare("SELECT aropen_id "
	    "FROM aropen "
	    "WHERE ((aropen_docnumber=:docnum) AND (aropen_doctype='D'));");
  q.bindValue(":docnum", _arapply->currentItem()->text(8));
  q.exec();
  if (q.first())
  {
    params.append("aropen_id", q.value("aropen_id"));
    params.append("docType", "debitMemo");
    arOpenItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspARApplications::sViewInvoice()
{
  ParameterList params;

  params.append("mode", "view");
  params.append("invoiceNumber", _arapply->currentItem()->text(8));
  dspInvoiceInformation* newdlg = new dspInvoiceInformation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspARApplications::sPopulateMenu(QMenu* pMenu)
{
  int menuItem;

  if (_arapply->currentItem()->text(3) == "C/M")
  {
    menuItem = pMenu->insertItem(tr("View Source Credit Memo..."), this, SLOT(sViewCreditMemo()), 0);
    if (! _privileges->check("MaintainARMemos") &&
	! _privileges->check("ViewARMemos"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if (_arapply->currentItem()->text(6) == "D")
  {
    menuItem = pMenu->insertItem(tr("View Apply-To Debit Memo..."), this, SLOT(sViewDebitMemo()), 0);
    if (! _privileges->check("MaintainARMemos") &&
	! _privileges->check("ViewARMemos"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (_arapply->currentItem()->text(6) == "I")
  {
    menuItem = pMenu->insertItem(tr("View Apply-To Invoice..."), this, SLOT(sViewInvoice()), 0);
    if (! _privileges->check("MaintainMiscInvoices") &&
	! _privileges->check("ViewMiscInvoices"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspARApplications::sFillList()
{
  if ( (_selectedCustomer->isChecked()) && (!_cust->isValid()) )
  {
    QMessageBox::warning( this, tr("Select Customer"),
                          tr("You must select a Customer whose A/R Applications you wish to view.") );
    _cust->setFocus();
    return;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  if ( (!_cashReceipts->isChecked()) && (!_creditMemos->isChecked()) )
  {
    QMessageBox::critical( this, tr("Select Document Type"),
                           tr("You must indicate which Document Type(s) you wish to view.") );
    _cashReceipts->setFocus();
    return;
  }

  QString sql( "SELECT arapply_id, cust_number, cust_name,"
               "       formatDate(arapply_postdate) AS f_postdate,"
	       "       arapply_source_doctype, arapply_fundstype, "
               "       CASE WHEN (arapply_source_doctype IN ('C','R')) THEN TEXT(arapply_source_docnumber)"
               "            ELSE arapply_refnumber"
               "         END AS source,"
               "       arapply_target_doctype,"
               "       TEXT(arapply_target_docnumber) AS target,"
               "       formatMoney(arapply_applied) AS f_applied, arapply_applied "
               "FROM arapply, custinfo "
               "WHERE ( (arapply_cust_id=cust_id)"
               " AND (arapply_postdate BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?>)"
               " AND (arapply_source_doctype IN ("
	       "<? if exists(\"creditMemos\") ?>"
	       "  <? if exists(\"cashReceipts\") ?>"
	       "	'K', 'C', 'R' "
	       "  <? else ?>"
	       "	'C', 'R' "
	       "  <? endif ?>"
	       "<? else ?>"
	       "	'K' "
	       "<? endif ?>"
	       "))"
	       "<? if exists(\"cust_id\") ?>"
	       "  AND (cust_id=<? value(\"cust_id\") ?>)"
	       "<? elseif exists(\"custtype_id\") ?>"
	       "  AND (cust_custtype_id=<? value(\"custtype_id\") ?>)"
	       "<? elseif exists(\"custtype_pattern\") ?>"
	       "  AND (cust_custtype_id IN (SELECT custtype_id FROM custtype"
	       "                            WHERE (custtype_code ~ <? value(\"custtype_id\") ?>)))"
	       "<? endif ?>"
	       ") "
	       "ORDER BY arapply_postdate, source;"
	       );

  ParameterList params;

  if (_cashReceipts->isChecked())
    params.append("cashReceipts");

  if (_creditMemos->isChecked())
    params.append("creditMemos");

  params.append("startDate", _dates->startDate());
  params.append("endDate", _dates->endDate());

  if (_selectedCustomer->isChecked())
    params.append("cust_id", _cust->id());
  else if (_selectedCustomerType->isChecked())
    params.append("custtype_id", _customerTypes->id());
  else if (_customerTypePattern->isChecked())
    params.append("custtype_pattern", _customerType->text());

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    _arapply->clear();

    double total = 0;

    XTreeWidgetItem* last = 0;
    do
    {
      QString fundstype = q.value("arapply_fundstype").toString();
      QString doctype;
      if (q.value("arapply_source_doctype") == "C")
	    doctype = tr("Credit Memo");
      else if (q.value("arapply_source_doctype") == "R")
        doctype = tr("Cash Deposit");
      else if (fundstype == "A")
	    doctype = tr("AmEx");
      else if (fundstype == "C")
	    doctype = tr("Check");
      else if (fundstype == "D")
	    doctype = tr("Discover");
      else if (fundstype == "K")
	    doctype = tr("Cash");
      else if (fundstype == "M")
	    doctype = tr("M/C");
      else if (fundstype == "R")
	    doctype = tr("Other C/C");
      else if (fundstype == "T")
	    doctype = tr("Cert. Check");
      else if (fundstype == "V")
	    doctype = tr("Visa");
      else if (fundstype == "W")
	    doctype = tr("Wire Trans.");
      else if (fundstype == "O")
	    doctype = tr("Other");

      QString targetdoctype = q.value("arapply_target_doctype").toString();
      if (targetdoctype == "D")
	targetdoctype = tr("Debit Memo");
      else if (targetdoctype == "I")
	targetdoctype = tr("Invoice");
      else if (targetdoctype == "K")
	targetdoctype = tr("A/P Check");
      else
	targetdoctype = tr("Other");

      last = new XTreeWidgetItem( _arapply, last,
				 q.value("arapply_id").toInt(),
				 q.value("cust_number"),
				 q.value("cust_name"),
				 q.value("f_postdate"),
				 (q.value("arapply_source_doctype") == "C") ?
					"C/M" : ((q.value("arapply_source_doctype") == "R") ? "Cash Deposit" : fundstype),
				 doctype,
				 q.value("source"),
				 q.value("arapply_target_doctype").toString(),
				 targetdoctype,
				 q.value("target"), q.value("f_applied") );

      total += q.value("arapply_applied").toDouble();
    }
    while (q.next());

    last = new XTreeWidgetItem(_arapply, last, -1, "", tr("Total Applications:"));
    last->setText(9, formatMoney(total));
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
