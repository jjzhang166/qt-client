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

#include "selectPayments.h"

#include <QSqlError>

#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "selectBankAccount.h"
#include "selectPayment.h"
#include "storedProcErrorLookup.h"

selectPayments::selectPayments(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);


  QButtonGroup * vendorButtonGroup = new QButtonGroup(this);
  vendorButtonGroup->addButton(_allVendors);
  vendorButtonGroup->addButton(_selectedVendor);
  vendorButtonGroup->addButton(_selectedVendorType);
  vendorButtonGroup->addButton(_vendorTypePattern);

  QButtonGroup * dueButtonGroup = new QButtonGroup(this);
  dueButtonGroup->addButton(_dueAll);
  dueButtonGroup->addButton(_dueOlder);
  dueButtonGroup->addButton(_dueBetween);

  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_clearAll, SIGNAL(clicked()), this, SLOT(sClearAll()));
  connect(_dueBetweenDates, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_dueOlderDate, SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_selectDiscount, SIGNAL(clicked()), this, SLOT(sSelectDiscount()));
  connect(_selectDue, SIGNAL(clicked()), this, SLOT(sSelectDue()));
  connect(_selectLine, SIGNAL(clicked()), this, SLOT(sSelectLine()));
  connect(_vend, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_vendorType, SIGNAL(lostFocus()), this, SLOT(sFillList()));
  connect(_vendorTypes, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(dueButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
  connect(vendorButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));

  _ignoreUpdates = false;
  
  _vendorTypes->setType(XComboBox::VendorTypes);
  _bankaccnt->setType(XComboBox::APBankAccounts);

  QString base;
  q.exec("SELECT currConcat(baseCurrID()) AS base;");
  if (q.first())
    base = q.value("base").toString();

  _apopen->addColumn(tr("Vendor"),    -1,           Qt::AlignLeft   );
  _apopen->addColumn(tr("Doc. Type"), _orderColumn, Qt::AlignCenter );
  _apopen->addColumn(tr("Doc. #"),    _orderColumn, Qt::AlignRight  );
  _apopen->addColumn(tr("Inv. #"),    _orderColumn, Qt::AlignRight  );
  _apopen->addColumn(tr("P/O #"),     _orderColumn, Qt::AlignRight  );
  _apopen->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignCenter );
  _apopen->addColumn(tr("Doc. Date"), _dateColumn,  Qt::AlignCenter );
  _apopen->addColumn(tr("Amount"),    _moneyColumn, Qt::AlignRight  );
  _apopen->addColumn(tr("Selected"),  _moneyColumn, Qt::AlignRight  );
  _apopen->addColumn(tr("Discount"),  _moneyColumn, Qt::AlignRight  );
  _apopen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft );
  _apopen->addColumn(tr("Running (%1)").arg(base), _moneyColumn, Qt::AlignRight  );

  if (omfgThis->singleCurrency())
  {
      _apopen->hideColumn(10);
      _apopen->headerItem()->setText(11, tr("Running"));
  }

  connect(omfgThis, SIGNAL(paymentsUpdated(int, int, bool)), this, SLOT(sFillList()));

  sFillList();
}

selectPayments::~selectPayments()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectPayments::languageChange()
{
  retranslateUi(this);
}

void selectPayments::sPrint()
{
  ParameterList params;

  if (_selectedVendor->isChecked())
    params.append("vend_id", _vend->id());
  else if (_selectedVendorType->isChecked())
    params.append("vendtype_id", _vendorTypes->id());
  else if (_vendorTypePattern->isChecked())
    params.append("vendtype_pattern", _vendorType->text());

  orReport report("SelectPaymentsList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void selectPayments::sSelectDue()
{
  ParameterList params;
  params.append("type", "P");

  int bankaccntid = _bankaccnt->id();
  if(bankaccntid == -1)
  {
    selectBankAccount newdlg(this, "", TRUE);
    newdlg.set(params);
    bankaccntid = newdlg.exec();
  }

  if (bankaccntid != -1)
  {
    if (_allVendors->isChecked())
      q.prepare( "SELECT selectDueItemsForPayment(vend_id, :bankaccnt_id) AS result "
                 "FROM vend;" );
    else if (_selectedVendor->isChecked())
      q.prepare("SELECT selectDueItemsForPayment(:vend_id, :bankaccnt_id) AS result;");
    else if (_selectedVendorType->isChecked())
      q.prepare( "SELECT selectDueItemsForPayment(vend_id, :bankaccnt_id) AS result "
                 "FROM vend "
                 "WHERE (vend_vendtype_id=:vendtype_id);" );
    else if (_vendorTypePattern->isChecked())
      q.prepare( "SELECT selectDueItemsForPayment(vend_id, :bankaccnt_id) AS result "
                 "FROM vend "
                 "WHERE (vend_vendtype_id IN (SELECT vendtype_id FROM vendtype WHERE (vendtype_code ~ :venttype_code)));" );

    q.bindValue(":bankaccnt_id", bankaccntid);
    q.bindValue(":vend_id", _vend->id());
    q.bindValue(":vendtype_id", _vendorTypes->id());
    q.bindValue(":vendtype_code", _vendorType->text());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sPaymentsUpdated(-1, -1, TRUE);
  }
}

void selectPayments::sSelectDiscount()
{
  ParameterList params;
  params.append("type", "P");

  int bankaccntid = _bankaccnt->id();
  if(bankaccntid == -1)
  {
    selectBankAccount newdlg(this, "", TRUE);
    newdlg.set(params);
    bankaccntid = newdlg.exec();
  }

  if (bankaccntid != -1)
  {
    if (_allVendors->isChecked())
      q.prepare( "SELECT selectDiscountItemsForPayment(vend_id, :bankaccnt_id) AS result "
                 "FROM vend;" );
    else if (_selectedVendor->isChecked())
      q.prepare("SELECT selectDiscountItemsForPayment(:vend_id, :bankaccnt_id) AS result;");
    else if (_selectedVendorType->isChecked())
      q.prepare( "SELECT selectDiscountItemsForPayment(vend_id, :bankaccnt_id) AS result "
                 "FROM vend "
                 "WHERE (vend_vendtype_id=:vendtype_id);" );
    else if (_vendorTypePattern->isChecked())
      q.prepare( "SELECT selectDiscountItemsForPayment(vend_id, :bankaccnt_id) AS result "
                 "FROM vend "
                 "WHERE (vend_vendtype_id IN (SELECT vendtype_id FROM vendtype WHERE (vendtype_code ~ :venttype_code)));" );

    q.bindValue(":bankaccnt_id", bankaccntid);
    q.bindValue(":vend_id", _vend->id());
    q.bindValue(":vendtype_id", _vendorTypes->id());
    q.bindValue(":vendtype_code", _vendorType->text());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sPaymentsUpdated(-1, -1, TRUE);
  }
}

void selectPayments::sClearAll()
{
  if (_allVendors->isChecked())
    q.prepare( "SELECT clearPayment(apselect_id) AS result "
               "FROM apselect;" );
  else if (_selectedVendor->isChecked())
    q.prepare( "SELECT clearPayment(apselect_id) AS result "
               "FROM apopen, apselect "
               "WHERE ( (apselect_apopen_id=apopen_id)"
               " AND (apopen_vend_id=:vend_id) );" );
  else if (_selectedVendorType->isChecked())
    q.prepare( "SELECT clearPayment(apselect_id) AS result "
               "FROM vend, apopen, apselect "
               "WHERE ( (apselect_apopen_id=apopen_id)"
               " AND (apopen_vend_id=vend_id)"
               " AND (vend_vendtype_id=:vendtype_id) );" );
  else if (_vendorTypePattern->isChecked())
    q.prepare( "SELECT clearPayment(apselect_id) AS result "
               "FROM vend, apopen, apselect "
               "WHERE ( (apselect_apopen_id=apopen_id)"
               " AND (apopen_vend_id=vend_id)"
               " AND (vend_vendtype_id IN (SELECT vendtype_id FROM vendtype WHERE (vendtype_code ~ :vendtype_code)));" );

  q.bindValue(":vend_id", _vend->id());
  q.bindValue(":vendtype_id", _vendorTypes->id());
  q.bindValue(":vendtype_code", _vendorType->text());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sPaymentsUpdated(-1, -1, TRUE);
}

void selectPayments::sSelect()
{
  _ignoreUpdates = true;
  bool update = false;
  QList<QTreeWidgetItem*> list = _apopen->selectedItems();
  XTreeWidgetItem * cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    ParameterList params;
    params.append("apopen_id", cursor->id());

    if(_bankaccnt->id() != -1)
      params.append("bankaccnt_id", _bankaccnt->id());

    selectPayment newdlg(this, "", TRUE);
    newdlg.set(params);
    if(newdlg.exec() != XDialog::Rejected)
      update = true;
  }
  _ignoreUpdates = false;
  if(update)
    sFillList();
}

void selectPayments::sSelectLine()
{
  ParameterList params;
  params.append("type", "P");

  int bankaccntid = _bankaccnt->id();
  if(bankaccntid == -1)
  {
    selectBankAccount newdlg(this, "", TRUE);
    newdlg.set(params);
    bankaccntid = newdlg.exec();
  }

  if (bankaccntid != -1)
  {
    bool update = FALSE;
    QList<QTreeWidgetItem*> list = _apopen->selectedItems();
    XTreeWidgetItem * cursor = 0;
    q.prepare("SELECT selectPayment(:apopen_id, :bankaccnt_id) AS result;");
    for(int i = 0; i < list.size(); i++)
    {
      cursor = (XTreeWidgetItem*)list.at(i);
      q.bindValue(":apopen_id", cursor->id());
      q.bindValue(":bankaccnt_id", bankaccntid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, cursor->text(0) + " " + cursor->text(2) + "\n" +
			    storedProcErrorLookup("selectPayment", result),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      update = TRUE;
    }
  
    if(update)
      omfgThis->sPaymentsUpdated(-1, -1, TRUE);
  }
}

void selectPayments::sClear()
{
  bool update = FALSE;
  QList<QTreeWidgetItem*> list = _apopen->selectedItems();
  XTreeWidgetItem * cursor = 0;
  q.prepare("SELECT clearPayment(:apopen_id) AS result;");
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    q.bindValue(":apopen_id", cursor->altId());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, cursor->text(0) + " " + cursor->text(2) + "\n" +
			  storedProcErrorLookup("clearPayment", result),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    update = TRUE;
  }

  if(update)
    omfgThis->sPaymentsUpdated(-1, -1, TRUE);
}

void selectPayments::sFillList()
{
  if(_ignoreUpdates)
    return;

  if((_dueOlder->isChecked() && !_dueOlderDate->isValid())
    ||(_dueBetween->isChecked() && !_dueBetweenDates->allValid()))
    return;

  _apopen->clear();

  QString sql( "SELECT apopen_id, COALESCE(apselect_id, -1) AS apselectid,"
               "       (vend_number || '-' || vend_name) AS vendor,"
               "       CASE WHEN (apopen_doctype='V') THEN :voucher"
               "            When (apopen_doctype='D') THEN :debitMemo"
               "       END AS doctype,"
               "       apopen_docnumber, apopen_ponumber,"
               "       formatDate(apopen_duedate) AS f_duedate,"
               "       formatDate(apopen_docdate) AS f_docdate,"
               "       formatMoney(apopen_amount - apopen_paid - "
               "                   COALESCE((SELECT SUM(currToCurr(checkitem_curr_id, apopen_curr_id, checkitem_amount + checkitem_discount, CURRENT_DATE)) "
               "                             FROM checkitem, checkhead "
               "                             WHERE ((checkitem_checkhead_id=checkhead_id) "
               "                              AND (checkitem_apopen_id=apopen_id) "
               "                              AND (NOT checkhead_void) "
               "                              AND (NOT checkhead_posted)) "
               "                           ), 0)) AS f_amount,"
               "       (apopen_amount - apopen_paid - "
               "                   COALESCE((SELECT SUM(currToCurr(checkitem_curr_id, apopen_curr_id, checkitem_amount + checkitem_discount, CURRENT_DATE)) "
               "                             FROM checkitem, checkhead "
               "                             WHERE ((checkitem_checkhead_id=checkhead_id) "
               "                              AND (checkitem_apopen_id=apopen_id) "
               "                              AND (NOT checkhead_void) "
               "                              AND (NOT checkhead_posted)) "
               "                           ), 0)) AS amount,"
               "       COALESCE(currToBase(apselect_curr_id, SUM(apselect_amount), CURRENT_DATE), 0) AS selected_base,"
               "       COALESCE(SUM(apselect_amount), 0) AS selected,"
               "       formatMoney(COALESCE(SUM(apselect_discount),0)) AS f_discount,"
               "       (apopen_duedate <= CURRENT_DATE) AS late, apopen_invcnumber,"
               "       currConcat(apopen_curr_id) AS curr_concat "
               "FROM vend, apopen LEFT OUTER JOIN apselect ON (apselect_apopen_id=apopen_id) "
               "WHERE ( (apopen_open)"
               " AND (apopen_doctype IN ('V', 'D'))"
               " AND (apopen_vend_id=vend_id)" );

  if (_selectedVendor->isChecked())
    sql += " AND (vend_id=:vend_id)";
  else if (_selectedVendorType->isChecked())
    sql += " AND (vend_vendtype_id=:vendtype_id)";
  else if (_vendorTypePattern->isChecked())
    sql += " AND (vend_vendtype_id IN (SELECT vendtype_id FROM vendtype WHERE (vendtype_code ~ :vendtype_code)))";

  if (_dueOlder->isChecked())
    sql += " AND (apopen_duedate < :olderDate)";
  else if(_dueBetween->isChecked())
    sql += " AND (apopen_duedate BETWEEN :startDate AND :endDate)";

  sql += ") "
         "GROUP BY apopen_id, apselect_id, vend_number, vend_name,"
         "         apopen_doctype, apopen_docnumber, apopen_ponumber,"
         "         apopen_duedate, apopen_docdate, apopen_amount, apopen_paid, "
         "         curr_concat, apopen_curr_id, apselect_curr_id, apopen_invcnumber "
         "ORDER BY apopen_duedate, (apopen_amount - apopen_paid) DESC;";

  q.prepare(sql);
  q.bindValue(":vend_id", _vend->id());
  q.bindValue(":vendtype_id", _vendorTypes->id());
  q.bindValue(":vendtype_code", _vendorType->text());
  q.bindValue(":voucher", tr("Voucher"));
  q.bindValue(":debitMemo", tr("D/M"));
  if(_dueOlder->isChecked())
    q.bindValue(":olderDate", _dueOlderDate->date());
  else if(_dueBetween->isChecked())
    _dueBetweenDates->bindValue(q);
  q.exec();
  double running = 0;
  XTreeWidgetItem * last = 0;

  while (q.next())
  {
    // skip records that have a zero amount
    if(q.value("amount").toDouble() == 0.0)
      continue;

    running += q.value("selected_base").toDouble();

    last = new XTreeWidgetItem( _apopen, last,
				q.value("apopen_id").toInt(), q.value("apselectid").toInt(),
				q.value("vendor"), q.value("doctype"),
				q.value("apopen_docnumber"), q.value("apopen_invcnumber"), q.value("apopen_ponumber"),
				q.value("f_duedate"), q.value("f_docdate"),
				q.value("f_amount"),
				formatMoney(q.value("selected").toDouble()),
				q.value("f_discount"),
				q.value("curr_concat"));
    last->setText(11, formatMoney(running));
    if (q.value("late").toBool())
      last->setTextColor("red");
  }
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
