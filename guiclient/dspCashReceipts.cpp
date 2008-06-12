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

#include "dspCashReceipts.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>
#include <datecluster.h>
#include <openreports.h>

/*
 *  Constructs a dspCashReceipts as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCashReceipts::dspCashReceipts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_selectedCustomerType, SIGNAL(toggled(bool)), _customerTypes, SLOT(setEnabled(bool)));
  connect(_customerTypePattern, SIGNAL(toggled(bool)), _customerType, SLOT(setEnabled(bool)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_selectedCustomer, SIGNAL(toggled(bool)), _cust, SLOT(setEnabled(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _customerTypes->setType(XComboBox::CustomerTypes);
  
  _arapply->addColumn(tr("Cust. #"),     _orderColumn,    Qt::AlignCenter );
  _arapply->addColumn(tr("Customer"),    -1,              Qt::AlignLeft   );
  _arapply->addColumn(tr("Date"),        _dateColumn,     Qt::AlignCenter );
  _arapply->addColumn(tr("Source"),      _itemColumn,     Qt::AlignCenter );
  _arapply->addColumn(tr("Apply-To"),    _itemColumn,     Qt::AlignCenter );
  _arapply->addColumn(tr("Amount"),      _bigMoneyColumn, Qt::AlignRight  );

  _allCustomers->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCashReceipts::~dspCashReceipts()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCashReceipts::languageChange()
{
  retranslateUi(this);
}

void dspCashReceipts::sPrint()
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

  ParameterList params;
  _dates->appendValue(params);

  if (_selectedCustomer->isChecked())
    params.append("cust_id", _cust->id());
  else if (_selectedCustomerType->isChecked())
    params.append("custtype_id", _customerTypes->id());
  else if (_customerTypePattern->isChecked())
    params.append("custtype_pattern", _customerType->text());

  orReport report("CashReceipts", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCashReceipts::sFillList()
{
  _arapply->clear();

  if ( (_selectedCustomer->isChecked()) && (!_cust->isValid()) )
  {
    QMessageBox::warning( this, tr("Select Customer"),
                          tr("You must select a Customer whose Cash Receipts you wish to view.") );
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

  QString sql( "SELECT arapply_id, 1 AS type, cust_number, cust_name,"
               "       formatDate(arapply_postdate) AS f_postdate,"
               "       ( CASE WHEN (arapply_source_doctype='C') THEN :creditMemo"
               "              WHEN (arapply_source_doctype='R') THEN :cashdeposit"
               "              WHEN (arapply_fundstype='C') THEN :check"
               "              WHEN (arapply_fundstype='T') THEN :certifiedCheck"
               "              WHEN (arapply_fundstype='M') THEN :masterCard"
               "              WHEN (arapply_fundstype='V') THEN :visa"
               "              WHEN (arapply_fundstype='A') THEN :americanExpress"
               "              WHEN (arapply_fundstype='D') THEN :discoverCard"
               "              WHEN (arapply_fundstype='R') THEN :otherCreditCard"
               "              WHEN (arapply_fundstype='K') THEN :cash"
               "              WHEN (arapply_fundstype='W') THEN :wireTransfer"
               "              WHEN (arapply_fundstype='O') THEN :other"
               "         END || ' ' ||"
               "         CASE WHEN (arapply_source_doctype IN ('C','R')) THEN TEXT(arapply_source_docnumber)"
               "              ELSE arapply_refnumber"
               "         END ) AS source,"
               "       ( CASE WHEN (arapply_target_doctype='D') THEN :debitMemo"
               "              WHEN (arapply_target_doctype='I') THEN :invoice"
               "              ELSE :other"
               "         END || ' ' || TEXT(arapply_target_docnumber) ) AS target,"
               "       formatMoney(arapply_applied) AS f_applied, arapply_applied,"
               "       arapply_postdate AS sortdate "
               "FROM arapply, cust "
               "WHERE ( (arapply_cust_id=cust_id)"
               " AND (arapply_postdate BETWEEN :startDate AND :endDate)"
               " AND (arapply_source_doctype ='K') " );

  if (_selectedCustomer->isChecked())
    sql += " AND (cust_id=:cust_id)";
  else if (_selectedCustomerType->isChecked())
    sql += " AND (cust_custtype_id=:custtype_id)";
  else if (_customerTypePattern->isChecked())
    sql += " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ :custtype_pattern)))";

  sql += " ) UNION "
         "SELECT cashrcpt_id, 2 AS type, cust_number, cust_name,"
         "       formatDate(cashrcpt_distdate) AS f_postdate,"
         "       ( CASE WHEN (cashrcpt_fundstype='C') THEN :check"
         "              WHEN (cashrcpt_fundstype='T') THEN :certifiedCheck"
         "              WHEN (cashrcpt_fundstype='M') THEN :masterCard"
         "              WHEN (cashrcpt_fundstype='V') THEN :visa"
         "              WHEN (cashrcpt_fundstype='A') THEN :americanExpress"
         "              WHEN (cashrcpt_fundstype='D') THEN :discoverCard"
         "              WHEN (cashrcpt_fundstype='R') THEN :otherCreditCard"
         "              WHEN (cashrcpt_fundstype='K') THEN :cash"
         "              WHEN (cashrcpt_fundstype='W') THEN :wireTransfer"
         "              WHEN (cashrcpt_fundstype='O') THEN :other"
         "         END || ' ' || cashrcpt_docnumber ) AS source,"
         "       '' AS target,"
         "       formatMoney(cashrcpt_amount) AS f_applied, cashrcpt_amount,"
         "       cashrcpt_distdate AS sortdate "
         "  FROM cashrcpt, cust "
         " WHERE ((cashrcpt_cust_id=cust_id)"
         "   AND  (cashrcpt_distdate BETWEEN :startDate AND :endDate) ";

  if (_selectedCustomer->isChecked())
    sql += " AND (cust_id=:cust_id)";
  else if (_selectedCustomerType->isChecked())
    sql += " AND (cust_custtype_id=:custtype_id)";
  else if (_customerTypePattern->isChecked())
    sql += " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ :custtype_pattern)))";

  sql += " ) UNION "
         "SELECT aropen_id, 3 AS type, cust_number, cust_name,"
         "       formatDate(aropen_docdate) As f_postdate,"
         "       ( CASE WHEN (substr(aropen_notes, 16, 1)='C') THEN :check"
         "              WHEN (substr(aropen_notes, 16, 1)='T') THEN :certifiedCheck"
         "              WHEN (substr(aropen_notes, 16, 1)='M') THEN :masterCard"
         "              WHEN (substr(aropen_notes, 16, 1)='V') THEN :visa"
         "              WHEN (substr(aropen_notes, 16, 1)='A') THEN :americanExpress"
         "              WHEN (substr(aropen_notes, 16, 1)='D') THEN :discoverCard"
         "              WHEN (substr(aropen_notes, 16, 1)='R') THEN :otherCreditCard"
         "              WHEN (substr(aropen_notes, 16, 1)='K') THEN :cash"
         "              WHEN (substr(aropen_notes, 16, 1)='W') THEN :wireTransfer"
         "              WHEN (substr(aropen_notes, 16, 1)='O') THEN :other"
         "         END || ' ' ||"
         "         substr(aropen_notes, 18) ) AS source,"
         "       :unapplied AS target,"
         "       formatMoney(aropen_amount) AS f_applied, aropen_amount,"
         "       aropen_duedate AS sortdate "
         "  FROM aropen, cust"
         " WHERE ((aropen_cust_id=cust_id)"
         "   AND  (aropen_doctype='R')"
         "   AND  (aropen_docdate BETWEEN :startDate AND :endDate) ";

  if (_selectedCustomer->isChecked())
    sql += " AND (cust_id=:cust_id)";
  else if (_selectedCustomerType->isChecked())
    sql += " AND (cust_custtype_id=:custtype_id)";
  else if (_customerTypePattern->isChecked())
    sql += " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ :custtype_pattern)))";

  sql += " ) "
         "ORDER BY sortdate, source;";

  q.prepare(sql);
  _dates->bindValue(q);
  q.bindValue(":creditMemo", tr("C/M"));
  q.bindValue(":debitMemo", tr("D/M"));
  q.bindValue(":cashdeposit", tr("Cash Deposit"));
  q.bindValue(":invoice", tr("Invoice"));
  q.bindValue(":cash", tr("C/R"));
  q.bindValue(":check", tr("Check"));
  q.bindValue(":certifiedCheck", tr("Cert. Check"));
  q.bindValue(":masterCard", tr("M/C"));
  q.bindValue(":visa", tr("Visa"));
  q.bindValue(":americanExpress", tr("AmEx"));
  q.bindValue(":discoverCard", tr("Discover"));
  q.bindValue(":otherCreditCard", tr("Other C/C"));
  q.bindValue(":cash", tr("Cash"));
  q.bindValue(":wireTransfer", tr("Wire Trans."));
  q.bindValue(":other", tr("Other"));
  q.bindValue(":unapplied", tr("Cash Deposit"));

  if (_selectedCustomer->isChecked())
    q.bindValue(":cust_id", _cust->id());
  else if (_selectedCustomerType->isChecked())
    q.bindValue(":custtype_id", _customerTypes->id());
  else if (_customerTypePattern->isChecked())
    q.bindValue(":custtype_pattern", _customerType->text());

  q.exec();
  if (q.first())
  {
    double total = 0;

    XTreeWidgetItem *last = 0;
    do
    {
      last = new XTreeWidgetItem( _arapply, last, q.value("arapply_id").toInt(),
				 q.value("cust_number"), q.value("cust_name"),
				 q.value("f_postdate"), q.value("source"),
				 q.value("target"), q.value("f_applied") );

      total += q.value("arapply_applied").toDouble();
    }
    while (q.next());

    //XTreeWidgetItem *totals = new XTreeWidgetItem(_arapply, _arapply->lastItem(), -1, "", tr("Total Applications:"));
    //totals->setText(5, formatMoney(total));
  }
}

