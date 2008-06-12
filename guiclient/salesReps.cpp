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

#include "salesReps.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>

#include <parameter.h>
#include <openreports.h>

#include "guiclient.h"
#include "salesRep.h"
#include "storedProcErrorLookup.h"

salesReps::salesReps(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_salesrep, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainSalesReps"))
  {
    connect(_salesrep, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_salesrep, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_salesrep, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_salesrep, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _salesrep->addColumn(tr("Number"), 70,  Qt::AlignLeft,  true, "salesrep_number");
  _salesrep->addColumn(tr("Name"),   -1,  Qt::AlignLeft,  true, "salesrep_name");
  _salesrep->addColumn(tr("Active"), 50,  Qt::AlignCenter,true, "active");

  sFillList();
}

salesReps::~salesReps()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesReps::languageChange()
{
  retranslateUi(this);
}

void salesReps::sDelete()
{
  q.prepare("SELECT deleteSalesRep(:salesrep_id) AS result;" );
  q.bindValue(":salesrep_id", _salesrep->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteSalesRep", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void salesReps::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  salesRep newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesReps::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("salesrep_id", _salesrep->id());

  salesRep newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesReps::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("salesrep_id", _salesrep->id());

  salesRep newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void salesReps::sFillList()
{
  q.exec("SELECT *, formatBoolYN(salesrep_active) AS active "
         "FROM salesrep "
         "ORDER BY salesrep_number;" );
  _salesrep->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void salesReps::sPopulateMenu( QMenu * menu )
{
  int menuItem;

  menuItem = menu->insertItem(tr("Edit Sales Rep..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainSalesReps"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("View Sales Rep..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainSalesReps")) && (!_privileges->check("ViewSalesReps")))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Delete Sales Rep..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainSalesReps"))
    menu->setItemEnabled(menuItem, FALSE);
}

void salesReps::sPrint()
{
  orReport report("SalesRepsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
