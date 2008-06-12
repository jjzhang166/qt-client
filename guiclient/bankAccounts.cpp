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

#include "bankAccounts.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "bankAccount.h"

bankAccounts::bankAccounts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_adjustment,    SIGNAL(clicked()), this, SLOT(sPostAdjustment()));
  connect(_bankaccnt, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_delete,        SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,          SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,           SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,         SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_serviceCharge, SIGNAL(clicked()), this, SLOT(sPostServiceCharge()));
  connect(_view,          SIGNAL(clicked()), this, SLOT(sView()));
  
  _bankaccnt->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft, true, "bankaccnt_name"  );
  _bankaccnt->addColumn(tr("Description"),          -1, Qt::AlignLeft, true, "bankaccnt_descrip"  );
  _bankaccnt->addColumn(tr("Type"),        _dateColumn, Qt::AlignCenter,true, "type");
  _bankaccnt->addColumn(tr("A/P"),           _ynColumn, Qt::AlignCenter,true, "ap");
  _bankaccnt->addColumn(tr("A/R"),           _ynColumn, Qt::AlignCenter,true, "ar");
  _bankaccnt->addColumn(tr("Currency"),_currencyColumn, Qt::AlignCenter,true, "currabbr");

  if (omfgThis->singleCurrency())
      _bankaccnt->hideColumn("currabbr");

  if (_privileges->check("MaintainBankAccounts"))
  {
    connect(_bankaccnt, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_bankaccnt, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_bankaccnt, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_bankaccnt, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

bankAccounts::~bankAccounts()
{
  // no need to delete child widgets, Qt does it all for us
}

void bankAccounts::languageChange()
{
  retranslateUi(this);
}

void bankAccounts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  bankAccount newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void bankAccounts::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bankaccnt_id", _bankaccnt->id());

  bankAccount newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void bankAccounts::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bankaccnt_id", _bankaccnt->id());

  bankAccount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void bankAccounts::sFillList()
{
  q.prepare( "SELECT bankaccnt_id, bankaccnt_name, bankaccnt_descrip,"
             "       CASE WHEN (bankaccnt_type='K') THEN :checking"
             "            WHEN (bankaccnt_type='C') THEN :cash"
             "            ELSE '?'"
             "       END AS type,"
             "       formatBoolYN(bankaccnt_ap) AS ap,"
	     "       formatBoolYN(bankaccnt_ar) AS ar, "
	     "       currConcat(bankaccnt_curr_id) AS currabbr "
             "FROM bankaccnt "
             "ORDER BY bankaccnt_name;" );
  q.bindValue(":checking", tr("Checking"));
  q.bindValue(":cash", tr("Cash"));
  q.exec();
  _bankaccnt->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void bankAccounts::sDelete()
{
  // TODO: put all of this into a stored proc
  q.prepare( "SELECT cashrcpt_id "
             "FROM cashrcpt "
             "WHERE (cashrcpt_bankaccnt_id=:bankaccnt_id) "
             "LIMIT 1;" );
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Bank Account"),
                           tr("<p>The selected Bank Account cannot be deleted "
			      "as Cash Receipts have been posted against it."));
    return;
  }

  q.prepare( "SELECT checkhead_id "
             "FROM checkhead "
             "WHERE (checkhead_bankaccnt_id=:bankaccnt_id) "
             "LIMIT 1;" );
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Bank Account"),
                           tr("<p>The selected Bank Account cannot be delete "
			      "as Checks have been posted against it.") );
    return;
  }

  q.prepare( "DELETE FROM bankaccnt "
             "WHERE (bankaccnt_id=:bankaccnt_id);" );
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void bankAccounts::sPopulateMenu( QMenu * )
{
}

void bankAccounts::sPrint()
{
  orReport report("BankAccountsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void bankAccounts::sPostServiceCharge()
{
}

void bankAccounts::sPostAdjustment()
{
}
