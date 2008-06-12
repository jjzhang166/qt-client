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

#include "warehouses.h"

#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>
#include <QVariant>

#include <metasql.h>

#include <openreports.h>
#include "dspItemSitesByParameterList.h"
#include "warehouse.h"

warehouses::warehouses(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_warehouse, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  
  _warehouse->addColumn(tr("Whs."),        _whsColumn, Qt::AlignCenter );
  _warehouse->addColumn(tr("Active"),      _ynColumn,  Qt::AlignCenter );
  _warehouse->addColumn(tr("Description"), 130,        Qt::AlignLeft   );
  _warehouse->addColumn(tr("Address"),     -1,         Qt::AlignLeft   );

  if (_privileges->check("MaintainWarehouses"))
  {
    connect(_warehouse, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_warehouse, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_warehouse, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(warehousesUpdated()),  SLOT(sFillList())  );

  sFillList();
}

warehouses::~warehouses()
{
  // no need to delete child widgets, Qt does it all for us
}

void warehouses::languageChange()
{
  retranslateUi(this);
}

void warehouses::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  warehouse newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void warehouses::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("warehous_id", _warehouse->id());

  warehouse newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void warehouses::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("warehous_id", _warehouse->id());

  warehouse newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void warehouses::setParams(ParameterList & params)
{
  if (_showInactive->isChecked())
    params.append("showInactive");
}

void warehouses::sFillList()
{
  QString whss = "SELECT warehous_id, warehous_code,"
                 "       formatBoolYN(warehous_active),"
                 "       warehous_descrip, warehous_addr1 "
                 "FROM warehous "
		 "<? if not exists(\"showInactive\") ?>"
		 "WHERE (warehous_active) "
		 "<? endif ?>"
		 "ORDER BY warehous_code;" ;
  ParameterList whsp;
  setParams(whsp);
  MetaSQLQuery whsm(whss);
  XSqlQuery whsq = whsm.toQuery(whsp);

  _warehouse->populate(whsq);
}

void warehouses::sListItemSites()
{
  ParameterList params;
  params.append("run");
  params.append("classcode");
  params.append("warehous_id", _warehouse->id());

  dspItemSitesByParameterList *newdlg = new dspItemSitesByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void warehouses::sPopulateMenu( QMenu * pMenu )
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainWarehouses"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("List Item Sites..."), this, SLOT(sListItemSites()), 0);
  if (!_privileges->check("ViewItemSites"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void warehouses::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("WarehouseMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
