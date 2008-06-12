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

#include "dspRWTransactions.h"

#include <QVariant>
#include <QStatusBar>
#include <openreports.h>

/*
 *  Constructs a dspRWTransactions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspRWTransactions::dspRWTransactions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    connect(_selectedAccount, SIGNAL(toggled(bool)), _account, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspRWTransactions::~dspRWTransactions()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspRWTransactions::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void dspRWTransactions::init()
{
  statusBar()->hide();
  
  _gltrans->addColumn(tr("Date"),      _dateColumn,     Qt::AlignCenter );
  _gltrans->addColumn(tr("Source"),    _orderColumn,    Qt::AlignCenter );
  _gltrans->addColumn(tr("Doc. Type"), _docTypeColumn,  Qt::AlignCenter );
  _gltrans->addColumn(tr("Doc. #"),    _orderColumn,    Qt::AlignLeft   );
  _gltrans->addColumn(tr("Reference"), -1,              Qt::AlignLeft   );
  _gltrans->addColumn(tr("Account"),   _itemColumn,     Qt::AlignLeft   );
  _gltrans->addColumn(tr("Debit"),     _moneyColumn,    Qt::AlignRight  );
  _gltrans->addColumn(tr("Credit"),    _moneyColumn,    Qt::AlignRight  );
  _gltrans->addColumn(tr("Exported"),  _ynColumn+15,    Qt::AlignCenter );
}

enum SetResponse dspRWTransactions::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
  {
    _selectedAccount->setChecked(TRUE);
    _account->setId(param.toInt());
  }

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("period_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT period_start, period_end "
               "FROM period "
               "WHERE (period_id=:period_id);" );
    q.bindValue(":period_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _dates->setStartDate(q.value("period_start").toDate());
      _dates->setEndDate(q.value("period_end").toDate());
    }
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspRWTransactions::sPopulateMenu(QMenu *)
{
}

void dspRWTransactions::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);

  if (_selectedAccount->isChecked())
    params.append("accnt_id", _account->id());

  if (_showExported->isChecked())
    params.append("showExported");
  else if (_showUnexported->isChecked())
    params.append("showUnexported");
  
  if (_arDist->isChecked())
    params.append("showARDist");
  else if (_apDist->isChecked())
    params.append("showAPDist");
  else
    params.append("showOtherDist");

  orReport report("RWTransactions", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspRWTransactions::sFillList()
{
  _gltrans->clear();
  QString sql( "SELECT gltrans_id, formatDate(gltrans_date) AS f_date,"
               "       gltrans_source, gltrans_doctype, gltrans_docnumber,"
               "       firstLine(gltrans_notes) AS f_notes,"
               "       (formatGLAccount(accnt_id)||' - '|| accnt_descrip) AS f_accnt,"
               "       CASE WHEN (gltrans_amount < 0) THEN formatMoney(abs(gltrans_amount))"
               "            ELSE ''"
               "       END AS f_debit,"
               "       CASE WHEN (gltrans_amount > 0) THEN formatMoney(gltrans_amount)"
               "            ELSE ''"
               "       END AS f_credit,"
               "       formatBoolYN(gltrans_exported) AS f_exported,"
               "       CASE WHEN (gltrans_amount < 0) THEN abs(gltrans_amount)"
               "            ELSE 0"
               "       END AS debit,"
               "       CASE WHEN (gltrans_amount > 0) THEN gltrans_amount"
               "            ELSE 0"
               "       END AS credit "
               "FROM gltrans, accnt "
               "WHERE ((gltrans_accnt_id=accnt_id)"
               " AND (gltrans_date BETWEEN :startDate AND :endDate)" );

  if (_selectedAccount->isChecked())
    sql += " AND (gltrans_accnt_id=:accnt_id)";

  if (_showExported->isChecked())
    sql += " AND (gltrans_exported)";
  else if (_showUnexported->isChecked())
    sql += " AND (NOT gltrans_exported)";
  
  if (_arDist->isChecked())
    sql += " AND ((gltrans_source='A/R') AND (gltrans_doctype IN ('IN', 'CM')))";
  else if (_apDist->isChecked())
    sql += " AND ((gltrans_source='A/P') AND (gltrans_doctype='VO'))";
  else
    sql += " AND (NOT ((gltrans_source='A/R') AND (gltrans_doctype IN ('IN', 'CM'))))"
           " AND (NOT ((gltrans_source='A/P') AND (gltrans_doctype='VO')))";

  sql += ") "
         "ORDER BY accnt_number, accnt_profit, accnt_sub, gltrans_journalnumber, gltrans_date, gltrans_docnumber;";

  q.prepare(sql);
  _dates->bindValue(q);
  q.bindValue(":accnt_id", _account->id());
  q.exec();

  double debit = 0.0;
  double credit = 0.0;
  XTreeWidgetItem *last =  0;
  while(q.next())
  {
    debit += q.value("debit").toDouble();
    credit += q.value("credit").toDouble();
    last = new XTreeWidgetItem( _gltrans, last, q.value("gltrans_id").toInt(),
                       q.value("f_date"), q.value("gltrans_source"), q.value("gltrans_doctype"),
                       q.value("gltrans_docnumber"), q.value("f_notes"),
                       q.value("f_accnt"), q.value("f_debit"), q.value("f_credit"), q.value("f_exported") );
  }
  last = new XTreeWidgetItem( _gltrans, last, -1, "", "", "", "", tr("Total:"), "", formatMoney(debit), formatMoney(credit));
}
