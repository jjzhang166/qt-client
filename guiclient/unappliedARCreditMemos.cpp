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

#include "unappliedARCreditMemos.h"

#include <QVariant>
#include <QStatusBar>
#include <QMenu>
#include <parameter.h>
#include <openreports.h>
#include "applyARCreditMemo.h"
#include "arOpenItem.h"
/*
 *  Constructs a unappliedARCreditMemos as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
unappliedARCreditMemos::unappliedARCreditMemos(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_aropen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_aropen, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));

  _new->setEnabled(_privileges->check("MaintainARMemos"));

  connect(_aropen, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  _aropen->addColumn( tr("Doc. #"),   _itemColumn,  Qt::AlignCenter );
  _aropen->addColumn( tr("Customer"), -1,           Qt::AlignLeft   );
  _aropen->addColumn( tr("Amount"),   _moneyColumn, Qt::AlignRight  );
  _aropen->addColumn( tr("Applied"),  _moneyColumn, Qt::AlignRight  );
  _aropen->addColumn( tr("Balance"),  _moneyColumn, Qt::AlignRight  );
  _aropen->addColumn( tr("Currency"), _currencyColumn, Qt::AlignLeft );

  if (omfgThis->singleCurrency())
    _aropen->hideColumn(5);

  if (_privileges->check("ApplyARMemos"))
    connect(_aropen, SIGNAL(valid(bool)), _apply, SLOT(setEnabled(bool)));

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
unappliedARCreditMemos::~unappliedARCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void unappliedARCreditMemos::languageChange()
{
  retranslateUi(this);
}

void unappliedARCreditMemos::sPrint()
{
  ParameterList params;

  orReport report("UnappliedARCreditMemos", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


void unappliedARCreditMemos::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void unappliedARCreditMemos::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropen->id());

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void unappliedARCreditMemos::sPopulateMenu( QMenu * )
{
}

void unappliedARCreditMemos::sFillList()
{
  q.prepare( "SELECT aropen_id, aropen_docnumber,"
             "       (cust_number || '-' || cust_name),"
             "       formatMoney(aropen_amount),"
             "       formatMoney(aropen_paid + COALESCE(prepared,0.0)),"
             "       formatMoney(aropen_amount - aropen_paid - COALESCE(prepared,0.0)), "
	         "       currConcat(aropen_curr_id) "
             "FROM aropen "
	         "       LEFT OUTER JOIN (SELECT aropen_id AS prepared_aropen_id,"
             "                               SUM(currToCurr(checkitem_curr_id, aropen_curr_id, checkitem_amount + checkitem_discount, checkitem_docdate)) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN aropen ON (checkitem_aropen_id=aropen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void))"
             "                         GROUP BY aropen_id) AS sub1"
             "         ON (prepared_aropen_id=aropen_id)"
			 ", cust "
             "WHERE ( (aropen_doctype IN ('C', 'R'))"
             " AND (aropen_open)"
             " AND ((aropen_amount - aropen_paid - COALESCE(prepared,0.0)) > 0.0)"
             " AND (aropen_cust_id=cust_id) ) "
             "ORDER BY aropen_docnumber;" );
  q.exec();
  _aropen->populate(q);
}

void unappliedARCreditMemos::sApply()
{
  ParameterList params;
  params.append("aropen_id", _aropen->id());

  applyARCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

