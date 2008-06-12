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

#include "dspAROpenItems.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include <currcluster.h>

#include "arOpenItem.h"
#include "dspInvoiceInformation.h"

dspAROpenItems::dspAROpenItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_aropen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _aropen->addColumn(tr("Doc. Type"), -1,              Qt::AlignCenter );
  _aropen->addColumn(tr("Doc. #"),    _orderColumn,    Qt::AlignRight  );
  _aropen->addColumn(tr("Cust. #"),   _itemColumn,     Qt::AlignLeft   );
  _aropen->addColumn(tr("Customer"),  _itemColumn,     Qt::AlignLeft   );
  _aropen->addColumn(tr("Order #"),   _orderColumn,    Qt::AlignRight  );
  _aropen->addColumn(tr("Doc. Date"), _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Due Date"),  _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Amount"),    _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Paid"),      _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Balance"),   _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft   );
  if(omfgThis->singleCurrency())
    _aropen->hideColumn(8);
}

dspAROpenItems::~dspAROpenItems()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspAROpenItems::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspAROpenItems::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspAROpenItems::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("EditAROpenItem"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
    
  XTreeWidgetItem* item = (XTreeWidgetItem*)pItem;
  {
    if (item->text(0) == "Invoice")
      pMenu->insertItem(tr("View Invoice..."), this, SLOT(sViewInvoice()), 0);
  }
}

void dspAROpenItems::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItems::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItems::sViewInvoice()
{
  q.prepare("SELECT aropen_docnumber FROM aropen WHERE (aropen_id=:aropen_id);");
  q.bindValue(":aropen_id", _aropen->id());
  q.exec();
  if (q.first());
  {
    ParameterList params;
    params.append("invoiceNumber", q.value("aropen_docnumber"));
    dspInvoiceInformation* newdlg = new dspInvoiceInformation();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAROpenItems::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);

  orReport report("AROpenItems", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspAROpenItems::sFillList()
{
  _aropen->clear();

  q.prepare( "SELECT aropen_id, aropen_docnumber, aropen_ordernumber,"
             "       CASE WHEN (aropen_doctype='C') THEN :creditMemo"
             "            WHEN (aropen_doctype='D') THEN :debitMemo"
             "            WHEN (aropen_doctype='I') THEN :invoice"
             "            WHEN (aropen_doctype='R') THEN :cashdeposit"
             "            ELSE :other"
             "       END AS f_doctype,"
             "       formatDate(aropen_docdate) AS f_docdate,"
             "       formatDate(aropen_duedate) AS f_duedate,"
             "       formatMoney(aropen_amount) AS f_amount,"
             "       formatMoney(aropen_paid) AS f_paid,"
             "       CASE WHEN (aropen_doctype IN ('C', 'R')) THEN ((aropen_amount - aropen_paid) * -1)"
             "            WHEN (aropen_doctype IN ('I', 'D')) THEN (aropen_amount - aropen_paid)"
             "            ELSE (aropen_amount - aropen_paid)"
             "       END AS f_balance,"
             "       currConcat(aropen_curr_id) AS currAbbr,"
             "       currToBase(aropen_curr_id,"
             "            CASE WHEN (aropen_doctype IN ('C', 'R')) THEN ((aropen_amount - aropen_paid) * -1)"
             "                 WHEN (aropen_doctype IN ('I', 'D')) THEN (aropen_amount - aropen_paid)"
             "                 ELSE (aropen_amount - aropen_paid)"
             "            END, CURRENT_DATE) AS base_balance,"
	     "        cust_id, cust_number, cust_name "
             "FROM aropen LEFT OUTER JOIN custinfo ON (aropen_cust_id=cust_id) "
             "WHERE ( (aropen_open)"
             " AND (aropen_duedate BETWEEN :startDate AND :endDate) ) "
             "ORDER BY aropen_docnumber;" );
  _dates->bindValue(q);
  q.bindValue(":creditMemo", tr("C/M"));
  q.bindValue(":debitMemo", tr("D/M"));
  q.bindValue(":invoice", tr("Invoice"));
  q.bindValue(":cashdeposit", tr("C/D"));
  q.exec();

  XTreeWidgetItem * last = 0;
  double total= 0.0;
  while (q.next())
  {
    last = new XTreeWidgetItem( _aropen, last, q.value("aropen_id").toInt(),
				q.value("cust_id").toInt(),
				q.value("f_doctype"),
				q.value("aropen_docnumber"),
				q.value("cust_number"), q.value("cust_name"),
				q.value("aropen_ordernumber"),
				q.value("f_docdate"),
				q.value("f_duedate"), q.value("f_amount"),
				q.value("f_paid"),
				q.value("f_balance"), q.value("currAbbr") );

    total += q.value("base_balance").toDouble();
  }
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  new XTreeWidgetItem( _aropen, last, -1,
		       QVariant(tr("Total")), "", "", "", "", "", "", "", "",
		       formatMoney(total), CurrDisplay::baseCurrAbbr() );
}

