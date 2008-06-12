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

#include "bankAdjustmentEditList.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "guiclient.h"
#include "bankAdjustment.h"
#include "storedProcErrorLookup.h"

bankAdjustmentEditList::bankAdjustmentEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_adjustments, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

 _bankaccnt->populate("SELECT bankaccnt_id,"
		     "       (bankaccnt_name || '-' || bankaccnt_descrip),"
		     "       bankaccnt_name "
		     "FROM bankaccnt "
		     "ORDER BY bankaccnt_name;");
  
  _adjustments->addColumn(tr("Bank Account"),_itemColumn, Qt::AlignLeft,  true, "f_bank");
  _adjustments->addColumn(tr("Adj. Type"),  _orderColumn, Qt::AlignLeft,  true, "bankadjtype_name");
  _adjustments->addColumn(tr("Dist. Date"),  _dateColumn, Qt::AlignCenter,true, "bankadj_date");
  _adjustments->addColumn(tr("Doc. Number"),          -1, Qt::AlignRight, true, "bankadj_docnumber");
  _adjustments->addColumn(tr("Amount"),  _bigMoneyColumn, Qt::AlignRight, true, "bankadj_amount");
  _adjustments->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currabbr");

  if (omfgThis->singleCurrency())
      _adjustments->hideColumn("currabbr");
  
  if (_privileges->check("MaintainBankAdjustments"))
  {
    connect(_adjustments, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_adjustments, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_adjustments, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_adjustments, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
  
  connect(omfgThis, SIGNAL(bankAdjustmentsUpdated(int, bool)), this, SLOT(sFillList()));
  
  sFillList();
}

bankAdjustmentEditList::~bankAdjustmentEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void bankAdjustmentEditList::languageChange()
{
  retranslateUi(this);
}

void bankAdjustmentEditList::sPrint()
{
  ParameterList params;

  params.append("bankaccnt_id", _bankaccnt->id());

  orReport report("BankAdjustmentEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void bankAdjustmentEditList::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  params.append("bankaccnt_id", _bankaccnt->id());
  
  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bankAdjustmentEditList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bankadj_id", _adjustments->id());
  
  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bankAdjustmentEditList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bankadj_id", _adjustments->id());
  
  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bankAdjustmentEditList::sDelete()
{
  q.prepare( "DELETE FROM bankadj "
             "WHERE ( (bankadj_id=:bankadj_id)"
             " AND (NOT bankadj_posted) ); ");
  q.bindValue(":bankadj_id", _adjustments->id());
  q.exec();
  sFillList();
}

void bankAdjustmentEditList::sFillList()
{
  q.prepare( "SELECT *,"
	     "       (bankaccnt_name || '-' || bankaccnt_descrip) AS f_bank,"
             "       'curr' AS bankadj_amount_xtnumericrole, "
	     "       currConcat(bankadj_curr_id) AS currabbr "
             "FROM bankadj LEFT OUTER JOIN bankaccnt ON (bankadj_bankaccnt_id=bankaccnt_id)"
             "                       LEFT OUTER JOIN bankadjtype ON (bankadj_bankadjtype_id=bankadjtype_id) "
             "WHERE ( (NOT bankadj_posted)"
             " AND (bankadj_bankaccnt_id=:bankaccnt_id) ); ");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  _adjustments->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void bankAdjustmentEditList::sPopulateMenu( QMenu * pMenu )
{
  int menuItem;
  
  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainBankAdjustments"))
    pMenu->setItemEnabled(menuItem, FALSE);
  
  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  
  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainBankAdjustments"))
    pMenu->setItemEnabled(menuItem, FALSE);
  
  pMenu->insertSeparator();
  
  menuItem = pMenu->insertItem(tr("Post..."), this, SLOT(sPost()), 0);
  if (!_privileges->check("PostBankAdjustments"))
    pMenu->setItemEnabled(menuItem, FALSE);
}


void bankAdjustmentEditList::sPost()
{
  q.prepare("SELECT postBankAdjustment(:bankadjid) AS result;");
  q.bindValue(":bankadjid", _adjustments->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("postBankAdjustment", result),
		  __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
