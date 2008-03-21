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

#include "dspAROpenItemsByCustomer.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include "arOpenItem.h"
#include "dspInvoiceInformation.h"

/*
 *  Constructs a dspAROpenItemsByCustomer as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspAROpenItemsByCustomer::dspAROpenItemsByCustomer(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_aropen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_cust, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _aropen->addColumn(tr("Doc. Type"), -1,              Qt::AlignCenter );
  _aropen->addColumn(tr("Doc. #"),    _orderColumn,    Qt::AlignRight  );
  _aropen->addColumn(tr("Order #"),   _orderColumn,    Qt::AlignRight  );
  _aropen->addColumn(tr("Doc. Date"), _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Due Date"),  _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Amount"),    _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Paid"),      _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Balance"),   _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft   );
  _aropen->addColumn(tr("Balance"),   _bigMoneyColumn, Qt::AlignRight  );

  if (omfgThis->singleCurrency())
  {
    _aropen->hideColumn(7);
    _aropen->hideColumn(8);
  }
  else
  {
    q.prepare("SELECT currConcat(baseCurrId()) AS currConcat;");
    q.exec();
    QString currConcat;
    if (q.first())
      currConcat = q.value("currConcat").toString();
    else
      currConcat = tr("?????");
    _aropen->headerItem()->setText(9, tr("Balance\n(in %1)").arg(currConcat));
  }

  _asOf->setDate(omfgThis->dbDate(), true);
  _cust->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspAROpenItemsByCustomer::~dspAROpenItemsByCustomer()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspAROpenItemsByCustomer::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspAROpenItemsByCustomer::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());
    
  param = pParams.value("asofDate", &valid);
  if (valid)
    _asOf->setDate(param.toDate());
    _asOf->setEnabled(FALSE);

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}
 
void dspAROpenItemsByCustomer::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privleges->check("EditAROpenItem"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  
  XTreeWidgetItem* item = (XTreeWidgetItem*)pItem;
  {
    if (item->text(0) == "Invoice")
      pMenu->insertItem(tr("View Invoice..."), this, SLOT(sViewInvoice()), 0);
  }
}

void dspAROpenItemsByCustomer::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void dspAROpenItemsByCustomer::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void dspAROpenItemsByCustomer::sViewInvoice()
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


void dspAROpenItemsByCustomer::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("asofDate", _asOf->date());
  params.append("cust_id", _cust->id());

  orReport report("AROpenItemsByCustomer", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspAROpenItemsByCustomer::sFillList()
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
             "       formatMoney(aropen_paid + SUM(currtobase(arapply_curr_id,arapply_applied,arapply_postdate))) AS f_paid,"
             "       CASE WHEN (aropen_doctype IN ('C', 'R')) THEN ((aropen_amount - aropen_paid + SUM(currtobase(arapply_curr_id,arapply_applied,arapply_postdate))) * -1)"
             "            WHEN (aropen_doctype IN ('I', 'D')) THEN (aropen_amount - aropen_paid + SUM(currtobase(arapply_curr_id,arapply_applied,arapply_postdate)))"
             "            ELSE (aropen_amount - aropen_paid + SUM(currtobase(arapply_curr_id,arapply_applied,arapply_postdate)))"
             "       END AS balance,"
             "       currConcat(aropen_curr_id) AS currAbbr,"
             "       currToBase(aropen_curr_id,"
             "       CASE WHEN (aropen_doctype IN ('C', 'R')) THEN ((aropen_amount - aropen_paid + SUM(currtobase(arapply_curr_id,arapply_applied,arapply_postdate))) * -1)"
             "            WHEN (aropen_doctype IN ('I', 'D')) THEN (aropen_amount - aropen_paid + SUM(currtobase(arapply_curr_id,arapply_applied,arapply_postdate)))"
             "            ELSE (aropen_amount - aropen_paid + SUM(currtobase(arapply_curr_id,arapply_applied,arapply_postdate)))"
             "       END, aropen_docdate) AS base_balance "
             "  FROM aropen "
             "  LEFT OUTER JOIN arapply ON (((aropen_id=arapply_source_aropen_id) "
             "                              OR (aropen_id=arapply_target_aropen_id)) "
             "                             AND (arapply_distdate>:asofdate)) "
             " WHERE ( (COALESCE(aropen_closedate,date :asofdate + integer '1')>:asofdate) "
             "   AND   (aropen_docdate<=:asofdate)"
             "   AND   (aropen_cust_id=:cust_id) "
             "   AND   (aropen_duedate BETWEEN :startDate AND :endDate)) "
             " GROUP BY aropen_id,aropen_docnumber,aropen_ordernumber,aropen_doctype,aropen_docdate,aropen_duedate,aropen_amount,aropen_curr_id,aropen_paid "
             " ORDER BY aropen_docdate;" );
  _dates->bindValue(q);
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":creditMemo", tr("Credit Memo"));
  q.bindValue(":debitMemo", tr("Debit Memo"));
  q.bindValue(":invoice", tr("Invoice"));
  q.bindValue(":cashdeposit", tr("Cash Deposit"));
  q.bindValue(":asofdate", _asOf->date());
  q.exec();
  if (q.first())
  {
    double total= 0.0;
    XTreeWidgetItem * last = 0;
    do
    {
      last = new XTreeWidgetItem( _aropen, last, q.value("aropen_id").toInt(),
                                  q.value("f_doctype"), q.value("aropen_docnumber"),
                                  q.value("aropen_ordernumber"), q.value("f_docdate"),
                                  q.value("f_duedate"), q.value("f_amount"),
                                  q.value("f_paid"), formatMoney(q.value("balance").toDouble()),
			          q.value("currAbbr"),
			          formatMoney(q.value("base_balance").toDouble()));
 
      total += q.value("base_balance").toDouble();
    }
    while (q.next());

    new XTreeWidgetItem( _aropen, last, -1,
                         QVariant(tr("Total")), "", "", "", "", "", "", "", "",
                         formatMoney(total) );
  }
}

