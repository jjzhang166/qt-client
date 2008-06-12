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

#include "salesAccounts.h"

#include <QMessageBox>
#include <QSqlError>

#include <parameter.h>
#include <openreports.h>

#include "salesAccount.h"
#include "guiclient.h"

salesAccounts::salesAccounts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _salesaccnt->addColumn(tr("Whs."),            -1,          Qt::AlignCenter );
  _salesaccnt->addColumn(tr("Cust. Type"),      _itemColumn, Qt::AlignCenter );
  _salesaccnt->addColumn(tr("Prod. Cat."),      _itemColumn, Qt::AlignCenter );
  _salesaccnt->addColumn(tr("Sales Accnt. #"),  _itemColumn, Qt::AlignCenter );
  _salesaccnt->addColumn(tr("Credit Accnt. #"), _itemColumn, Qt::AlignCenter );
  _salesaccnt->addColumn(tr("COS Accnt. #"),    _itemColumn, Qt::AlignCenter );
  _salesaccnt->addColumn(tr("Returns Accnt. #"), _itemColumn, Qt::AlignCenter );
  _salesaccnt->addColumn(tr("Cost of Returns Accnt. #"),  _itemColumn, Qt::AlignCenter );
  _salesaccnt->addColumn(tr("Cost of Warranty Accnt. #"), _itemColumn, Qt::AlignCenter );

  if (! _metrics->boolean("EnableReturnAuth"))
  {
    _salesaccnt->hideColumn(6);
    _salesaccnt->hideColumn(7);
    _salesaccnt->hideColumn(8);
  }

  if (_privileges->check("MaintainSalesAccount"))
  {
    connect(_salesaccnt, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_salesaccnt, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_salesaccnt, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_salesaccnt, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

salesAccounts::~salesAccounts()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesAccounts::languageChange()
{
  retranslateUi(this);
}

void salesAccounts::sPrint()
{
  orReport report("SalesAccountAssignmentsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void salesAccounts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  salesAccount newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesAccounts::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("salesaccnt_id", _salesaccnt->id());

  salesAccount newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesAccounts::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("salesaccnt_id", _salesaccnt->id());

  salesAccount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void salesAccounts::sDelete()
{
  q.prepare( "DELETE FROM salesaccnt "
             "WHERE (salesaccnt_id=:salesaccnt_id);" );
  q.bindValue(":salesaccnt_id", _salesaccnt->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void salesAccounts::sFillList()
{
  q.exec("SELECT salesaccnt_id,"
	 "       CASE WHEN (salesaccnt_warehous_id=-1) THEN TEXT('Any')"
	 "            ELSE (SELECT warehous_code FROM warehous WHERE (warehous_id=salesaccnt_warehous_id))"
	 "       END AS warehouscode,"
	 "       CASE WHEN ((salesaccnt_custtype_id=-1) AND (salesaccnt_custtype='.*')) THEN 'All'"
	 "            WHEN (salesaccnt_custtype_id=-1) THEN salesaccnt_custtype"
	 "            ELSE (SELECT custtype_code FROM custtype WHERE (custtype_id=salesaccnt_custtype_id))"
	 "       END AS custtypecode,"
	 "       CASE WHEN ((salesaccnt_prodcat_id=-1) AND (salesaccnt_prodcat='.*')) THEN 'All'"
	 "            WHEN (salesaccnt_prodcat_id=-1) THEN salesaccnt_prodcat"
	 "            ELSE (SELECT prodcat_code FROM prodcat WHERE (prodcat_id=salesaccnt_prodcat_id))"
	 "       END AS prodcatcode,"
	 "       formatGLAccount(salesaccnt_sales_accnt_id),"
	 "       formatGLAccount(salesaccnt_credit_accnt_id),"
	 "       formatGLAccount(salesaccnt_cos_accnt_id),"
	 "       formatGLAccount(salesaccnt_returns_accnt_id),"
	 "       formatGLAccount(salesaccnt_cor_accnt_id),"
	 "       formatGLAccount(salesaccnt_cow_accnt_id) "
	 "FROM salesaccnt "
	 "ORDER BY warehouscode, custtypecode, prodcatcode;" );
  _salesaccnt->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
