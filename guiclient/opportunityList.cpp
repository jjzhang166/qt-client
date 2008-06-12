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

#include "opportunityList.h"

#include "xdialog.h"
#include <QMenu>
#include <QSqlError>
#include <QVariant>
#include <metasql.h>
#include <openreports.h>

#include "opportunity.h"
#include "storedProcErrorLookup.h"
#include "mqlutil.h"

opportunityList::opportunityList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  _targetDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _targetDates->setEndNull(tr("Latest"),	  omfgThis->endOfTime(),   TRUE);
  _targetDates->setStartCaption(tr("First Target Date:"));
  _targetDates->setEndCaption(tr("Last Target Date:"));

  _usr->setEnabled(_privileges->check("MaintainOtherTodoLists"));
  _usr->setType(User);
  q.prepare("SELECT usr_id "
	    "FROM usr "
	    "WHERE (usr_username=CURRENT_USER);");
  q.exec();
  if (q.first())
  {
    _myUsrId = q.value("usr_id").toInt();
    _usr->setId(_myUsrId);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    close();
  }

  connect(_close,	SIGNAL(clicked()),	this,	SLOT(sClose()));
  connect(_delete,	SIGNAL(clicked()),	this,	SLOT(sDelete()));
  connect(_targetDates,	SIGNAL(updated()),	this,   SLOT(sFillList()));
  connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
  connect(_new,		SIGNAL(clicked()),	this,	SLOT(sNew()));
  connect(_print,	SIGNAL(clicked()),	this,	SLOT(sPrint()));
  connect(_list,	SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_list,	SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this,	SLOT(sPopulateMenu(QMenu*)));
  connect(_usr,		SIGNAL(updated()),	this,	SLOT(sFillList()));
  connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));

  if(_privileges->check("MaintainOpportunities"))
  {
    connect(_list, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_list, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_list, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    connect(_list, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  statusBar()->hide();

  _list->addColumn(tr("Name"),        _itemColumn,     Qt::AlignLeft );
  _list->addColumn(tr("CRM Acct."),   _userColumn,     Qt::AlignLeft );
  _list->addColumn(tr("Owner"),       _userColumn,     Qt::AlignLeft );
  _list->addColumn(tr("Stage"),       _orderColumn,    Qt::AlignLeft );
  _list->addColumn(tr("Source"),      _orderColumn,    Qt::AlignLeft,   false );
  _list->addColumn(tr("Type"),        _orderColumn,    Qt::AlignLeft,   false );
  _list->addColumn(tr("Prob.%"),      _prcntColumn,    Qt::AlignCenter, false );
  _list->addColumn(tr("Amount"),      _moneyColumn,    Qt::AlignRight,  false );
  _list->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   false );
  _list->addColumn(tr("Target Date"), _dateColumn,     Qt::AlignLeft );
  _list->addColumn(tr("Actual Date"), _dateColumn,     Qt::AlignLeft,   false );

  sFillList();
}

void opportunityList::languageChange()
{
  retranslateUi(this);
}

void opportunityList::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  bool editPriv = _privileges->check("MaintainOpportunities");
  bool viewPriv = _privileges->check("VeiwOpportunities") || editPriv;

  menuItem = pMenu->insertItem(tr("New..."), this, SLOT(sNew()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  pMenu->setItemEnabled(menuItem, viewPriv);

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);
}

enum SetResponse opportunityList::set(const ParameterList& pParams)
{
  QVariant param;
  bool	   valid;

  param = pParams.value("usr_id", &valid);
  if (valid)
  {
    _usr->setId(param.toInt());
    sFillList();
  }

  return NoError;
}

void opportunityList::sClose()
{
  close();
}

void opportunityList::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  if (_usr->isSelected())
    params.append("usr_id", _usr->id());

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunityList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ophead_id", _list->id());

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunityList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("ophead_id", _list->id());

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void opportunityList::sDelete()
{
  q.prepare("SELECT deleteOpportunity(:ophead_id) AS result;");
  q.bindValue(":ophead_id", _list->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteOpportunity", result));
      return;
    }
    else
      sFillList();
    }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

}

void opportunityList::setParams(ParameterList &params)
{
  _usr->appendValue(params);
  _targetDates->appendValue(params);
}

void opportunityList::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("OpportunityList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void opportunityList::sFillList()
{

  MetaSQLQuery mql = mqlLoad(":/crm/opportunityList/FillListDetail.mql");

  ParameterList params;
  setParams(params);

  XSqlQuery itemQ = mql.toQuery(params);

  if (itemQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _list->populate(itemQ);
}

