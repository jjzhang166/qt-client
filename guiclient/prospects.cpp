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

#include "prospects.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include "prospect.h"
#include "storedProcErrorLookup.h"

prospects::prospects(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    connect(_delete,	SIGNAL(clicked()),	this, SLOT(sDelete()));
    connect(_edit,	SIGNAL(clicked()),	this, SLOT(sEdit()));
    connect(_new,	SIGNAL(clicked()),	this, SLOT(sNew()));
    connect(_prospect,	SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)),
						this, SLOT(sPopulateMenu(QMenu*)));
    connect(_view,	SIGNAL(clicked()),	this, SLOT(sView()));

    statusBar()->hide();
    
    if (_privileges->check("MaintainProspectMasters"))
    {
      connect(_prospect, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_prospect, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_prospect, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    }
    else
    {
      _new->setEnabled(FALSE);
      connect(_prospect, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }

    _prospect->addColumn(tr("Number"),  _orderColumn, Qt::AlignCenter );
    _prospect->addColumn(tr("Name"),    -1,           Qt::AlignLeft   );
    _prospect->addColumn(tr("Address"), 175,          Qt::AlignLeft   );
    _prospect->addColumn(tr("Phone #"), 100,          Qt::AlignLeft   );

    connect(omfgThis, SIGNAL(prospectsUpdated()), SLOT(sFillList()));

    sFillList();
}

prospects::~prospects()
{
    // no need to delete child widgets, Qt does it all for us
}

void prospects::languageChange()
{
    retranslateUi(this);
}

void prospects::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospects::sEdit()
{
  ParameterList params;
  params.append("prospect_id", _prospect->id());
  params.append("mode", "edit");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospects::sView()
{
  ParameterList params;
  params.append("prospect_id", _prospect->id());
  params.append("mode", "view");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospects::sDelete()
{
  q.prepare("SELECT deleteProspect(:prospect_id) AS result;");
  q.bindValue(":prospect_id", _prospect->id());
  q.exec();
  if (q.first())
  {
    int returnVal = q.value("result").toInt();
    if (returnVal < 0)
    {
        systemError(this, storedProcErrorLookup("deleteProspect", returnVal),
		    __FILE__, __LINE__);
        return;
    }
    omfgThis->sProspectsUpdated();
    omfgThis->sCrmAccountsUpdated(-1);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void prospects::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem("View...", this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem("Edit...", this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainProspectMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  if (!_privileges->check("MaintainProspectMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem("Delete", this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainProspectMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void prospects::sFillList()
{
  _prospect->clear();
  q.prepare( "SELECT prospect_id, prospect_number, prospect_name, addr_line1, cntct_phone "
             "FROM prospect LEFT OUTER JOIN "
	     "     cntct ON (prospect_cntct_id=cntct_id) LEFT OUTER JOIN "
	     "     addr  ON (cntct_addr_id=addr_id) "
             "ORDER BY prospect_number;" );
  q.exec();
  _prospect->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
