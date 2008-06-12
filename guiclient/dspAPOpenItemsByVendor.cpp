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

#include "dspAPOpenItemsByVendor.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include "apOpenItem.h"

/*
 *  Constructs a dspAPOpenItemsByVendor as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspAPOpenItemsByVendor::dspAPOpenItemsByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_apopen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_vend, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _apopen->addColumn(tr("Doc. Type"), -1,              Qt::AlignCenter );
  _apopen->addColumn(tr("Doc. #"), _orderColumn,    Qt::AlignRight  );
  _apopen->addColumn(tr("P/O #"),     _orderColumn,    Qt::AlignRight  );
  _apopen->addColumn(tr("Invoice #"),     _orderColumn,    Qt::AlignRight  );
  _apopen->addColumn(tr("Doc. Date"), _dateColumn,     Qt::AlignCenter );
  _apopen->addColumn(tr("Due Date"),  _dateColumn,     Qt::AlignCenter );
  _apopen->addColumn(tr("Amount"),    _bigMoneyColumn, Qt::AlignRight  );
  _apopen->addColumn(tr("Paid"),      _bigMoneyColumn, Qt::AlignRight  );
  _apopen->addColumn(tr("Balance"),   _bigMoneyColumn, Qt::AlignRight  );
  _apopen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft   );
  _apopen->addColumn(tr("Balance"),   _bigMoneyColumn, Qt::AlignRight  );

  if (omfgThis->singleCurrency())
  {
    _apopen->hideColumn(8);
    _apopen->hideColumn(9);
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
    _apopen->headerItem()->setText(10, tr("Balance\n(in %1)").arg(currConcat));
  }

  _asOf->setDate(omfgThis->dbDate(), true);
  _vend->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspAPOpenItemsByVendor::~dspAPOpenItemsByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspAPOpenItemsByVendor::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspAPOpenItemsByVendor::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vend->setId(param.toInt());

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

void dspAPOpenItemsByVendor::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("EditAPOpenItem"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspAPOpenItemsByVendor::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("apopen_id", _apopen->id());
  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAPOpenItemsByVendor::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", _apopen->id());
  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAPOpenItemsByVendor::sPrint()
{
  ParameterList params;
  params.append("vend_id", _vend->id());
  params.append("asofDate", _asOf->date());
  _dates->appendValue(params);

  orReport report("APOpenItemsByVendor", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspAPOpenItemsByVendor::sFillList()
{
  _apopen->clear();

  q.prepare( "SELECT apopen_id, apopen_ponumber, apopen_docnumber,"
             "       CASE WHEN (apopen_doctype='C') THEN :creditMemo"
             "            WHEN (apopen_doctype='D') THEN :debitMemo"
             "            WHEN (apopen_doctype='V') THEN :voucher"
             "            ELSE :other"
             "       END AS f_doctype,"
             "       apopen_invcnumber AS invoicenumber,"
             "       formatDate(apopen_docdate) AS f_docdate,"
             "       formatDate(apopen_duedate) AS f_duedate,"
             "       formatMoney(apopen_amount) AS f_amount,"
             "       formatMoney(apapplied(apopen_id,:asofdate)) AS f_paid,"
             "       CASE WHEN (apopen_doctype='C') THEN ((apopen_amount - apapplied(apopen_id,:asofdate)) * -1)"
             "            WHEN (apopen_doctype IN ('V', 'D')) THEN (apopen_amount - apapplied(apopen_id,:asofdate))"
             "            ELSE (apopen_amount - apapplied(apopen_id,:asofdate))"
             "       END AS balance, currConcat(apopen_curr_id) AS currAbbr,"
             "       currToBase(apopen_curr_id,"
	     "           CASE WHEN (apopen_doctype='C') THEN ((apopen_amount - apapplied(apopen_id,:asofdate)) * -1)"
             "            WHEN (apopen_doctype IN ('V', 'D')) THEN (apopen_amount - apapplied(apopen_id,:asofdate))"
             "            ELSE (apopen_amount - apopen_paid)"
             "            END, apopen_docdate) AS base_balance "
             "FROM apopen "
             " WHERE ( (COALESCE(apopen_closedate,date :asofdate + integer '1')>:asofdate) "
             "   AND   (apopen_docdate<=:asofdate)"
             "   AND   (apopen_vend_id=:vend_id) "
             "   AND   (apopen_duedate BETWEEN :startDate AND :endDate) "
             "   AND   ((currtobase(apopen_curr_id,apopen_amount,:asofdate) - apapplied(apopen_id,:asofdate)) > 0)) "
             " ORDER BY apopen_docdate;" );
  _dates->bindValue(q);
  q.bindValue(":vend_id", _vend->id());
  q.bindValue(":creditMemo", tr("C/M"));
  q.bindValue(":debitMemo", tr("D/M"));
  q.bindValue(":voucher", tr("Voucher"));
  q.bindValue(":asofdate", _asOf->date());
  q.exec();
  if (q.first())
  {
    double total= 0.0;

    XTreeWidgetItem *last = 0;
    do
    {
      last = new XTreeWidgetItem( _apopen, last, q.value("apopen_id").toInt(),
				 q.value("f_doctype"), q.value("apopen_docnumber"),
				 q.value("apopen_ponumber"), q.value("invoicenumber"), q.value("f_docdate"),
				 q.value("f_duedate"), q.value("f_amount"),
				 q.value("f_paid"), formatMoney(q.value("balance").toDouble()),
				 q.value("currAbbr"),
				 formatMoney(q.value("base_balance").toDouble()));
 
      total += q.value("base_balance").toDouble();
    }
    while (q.next());

    last = new XTreeWidgetItem( _apopen, last, -1,
                       QVariant(tr("Total")), "", "", "", "", "", "", "", "", "",
                       formatMoney(total) );
  }
}

