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

#include "dspStandardOperationsByWorkCenter.h"

#include <QMenu>

#include <openreports.h>
#include <parameter.h>

#include "standardOperation.h"

dspStandardOperationsByWorkCenter::dspStandardOperationsByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wrkcnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_stdopn, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _stdopn->addColumn(tr("Std. Oper. #"), _itemColumn, Qt::AlignLeft  );
  _stdopn->addColumn(tr("Description"),  -1,          Qt::AlignLeft  );

  sFillList();
}

dspStandardOperationsByWorkCenter::~dspStandardOperationsByWorkCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspStandardOperationsByWorkCenter::languageChange()
{
  retranslateUi(this);
}

void dspStandardOperationsByWorkCenter::sPrint()
{
  ParameterList params;
  params.append("wrkcnt_id", _wrkcnt->id());

  orReport report("StandardOperationsByWorkCenter", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspStandardOperationsByWorkCenter::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  pMenu->insertItem(tr("View Standard Operation..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Edit Standard Operation..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainStandardOperations"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspStandardOperationsByWorkCenter::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("stdopn_id", _stdopn->id());

  standardOperation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspStandardOperationsByWorkCenter::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("stdopn_id", _stdopn->id());

  standardOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspStandardOperationsByWorkCenter::sFillList()
{
  q.prepare( "SELECT wrkcnt_descrip, warehous_code "
             "FROM wrkcnt, warehous "
             "WHERE ( (wrkcnt_warehous_id=warehous_id)"
             " AND (wrkcnt_id=:wrkcnt_id) );" );
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  if (q.first())
  {
    _description->setText(q.value("wrkcnt_descrip").toString());
    _warehouse->setText(q.value("warehous_code").toString());
  }
//  ToDo

  q.prepare( "SELECT stdopn_id, stdopn_number,"
             "       (stdopn_descrip1 || ' ' || stdopn_descrip2) "
             "FROM stdopn "
             "WHERE (stdopn_wrkcnt_id=:wrkcnt_id) "
             "ORDER BY stdopn_number;" );
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  _stdopn->populate(q);
}
