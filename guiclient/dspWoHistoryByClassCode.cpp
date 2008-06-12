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

#include "dspWoHistoryByClassCode.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMenu>
#include <openreports.h>
#include "workOrder.h"

/*
 *  Constructs a dspWoHistoryByClassCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoHistoryByClassCode::dspWoHistoryByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_showCost, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ClassCode);

  _wo->addColumn(tr("W/O #"),       _orderColumn,  Qt::AlignLeft   );
  _wo->addColumn(tr("Item #"),      _itemColumn,   Qt::AlignLeft   );
  _wo->addColumn(tr("Description"), -1,            Qt::AlignLeft   );
  _wo->addColumn(tr("Status"),      _statusColumn, Qt::AlignCenter );
  _wo->addColumn(tr("Whs."),        _whsColumn,    Qt::AlignCenter );
  _wo->addColumn(tr("Ordered"),     _qtyColumn,    Qt::AlignRight  );
  _wo->addColumn(tr("Received"),    _qtyColumn,    Qt::AlignRight  );
  _wo->addColumn(tr("Start Date"),  _dateColumn,   Qt::AlignRight  );
  _wo->addColumn(tr("Due Date"),    _dateColumn,   Qt::AlignRight  );
  _wo->addColumn(tr("Cost"),	    _costColumn,   Qt::AlignRight );

  sHandleCosts(_showCost->isChecked());
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoHistoryByClassCode::~dspWoHistoryByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoHistoryByClassCode::languageChange()
{
  retranslateUi(this);
}

void dspWoHistoryByClassCode::sPrint()
{
  ParameterList params;

  _classCode->appendValue(params);
  _warehouse->appendValue(params);

  if(_topLevel->isChecked())
    params.append("showOnlyTopLevel");

  if(_showCost->isChecked())
    params.append("showCosts");

  orReport report("WOHistoryByClassCode", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoHistoryByClassCode::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByClassCode::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByClassCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspWoHistoryByClassCode::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
    _wo->showColumn(9);
  else
    _wo->hideColumn(9);
}

void dspWoHistoryByClassCode::sFillList()
{
  if (!checkParameters())
    return;

  QString sql( "SELECT wo_id, formatWoNumber(wo_id), item_number,"
               "       (item_descrip1 || ' ' || item_descrip2),"
               "       wo_status, warehous_code,"
               "       formatQty(wo_qtyord) AS ordered,"
               "       formatQty(wo_qtyrcv) AS received,"
               "       formatDate(wo_startdate) AS startdate,"
               "       formatDate(wo_duedate) AS duedate,"
               "       formatCost(wo_postedvalue) "
               "FROM wo, itemsite, warehous, item "
               "WHERE ((wo_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  if (_topLevel->isChecked())
    sql += " AND ( (wo_ordtype<>'W') OR (wo_ordtype IS NULL) )";

  sql += ") "
         "ORDER BY item_number;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _classCode->bindValue(q);
  q.exec();
  _wo->populate(q);
}

bool dspWoHistoryByClassCode::checkParameters()
{
  return TRUE;
}
