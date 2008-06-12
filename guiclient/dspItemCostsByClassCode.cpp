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

#include "dspItemCostsByClassCode.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include "dspItemCostSummary.h"
#include "maintainItemCosts.h"
#include "updateActualCostsByItem.h"
#include "postCostsByItem.h"

/*
 *  Constructs a dspItemCostsByClassCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemCostsByClassCode::dspItemCostsByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemcost, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ClassCode);

  _itemcost->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _itemcost->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _itemcost->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter );
  _itemcost->addColumn(tr("Std. Cost"),   _costColumn,  Qt::AlignRight  );
  _itemcost->addColumn(tr("Act. Cost"),   _costColumn,  Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemCostsByClassCode::~dspItemCostsByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemCostsByClassCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspItemCostsByClassCode::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("classcode_id", &valid);
  if (valid)
    _classCode->setId(param.toInt());
  
  param = pParams.value("classcode_pattern", &valid);
  if (valid)
    _classCode->setPattern(param.toString());
  
  if (pParams.inList("run"))
    sFillList();

  return NoError;
}

void dspItemCostsByClassCode::sPrint()
{
  ParameterList params;

  _classCode->appendValue(params);

  if(_onlyShowZero->isChecked())
    params.append("onlyShowZeroCosts");

  orReport report("ItemCostsByClassCode", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemCostsByClassCode::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  pMenu->insertItem(tr("Maintain Item Costs..."), this, SLOT(sMaintainItemCosts()), 0);
  pMenu->insertItem(tr("View Item Costing Summary..."), this, SLOT(sViewItemCostingSummary()), 0);
  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Update Actual Costs..."), this, SLOT(sUpdateCosts()), 0);
  if (!_privileges->check("UpdateActualCosts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Post Actual Costs..."), this, SLOT(sPostCosts()), 0);
  if (!_privileges->check("PostActualCosts"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspItemCostsByClassCode::sMaintainItemCosts()
{
  ParameterList params;
  params.append("item_id", _itemcost->id());
  params.append("run");

  maintainItemCosts *newdlg = new maintainItemCosts();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemCostsByClassCode::sViewItemCostingSummary()
{
  ParameterList params;
  params.append("item_id", _itemcost->id());
  params.append("run");

  dspItemCostSummary *newdlg = new dspItemCostSummary();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemCostsByClassCode::sUpdateCosts()
{
  ParameterList params;
  params.append("item_id", _itemcost->id());

  updateActualCostsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemCostsByClassCode::sPostCosts()
{
  ParameterList params;
  params.append("item_id", _itemcost->id());

  postCostsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemCostsByClassCode::sFillList()
{
  QString sql( "SELECT item_id, item_number, description,"
               "       uom_name, formatCost(scost), formatCost(acost) "
               "FROM ( SELECT item_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
               "              uom_name, stdcost(item_id) AS scost, actcost(item_id) AS acost"
               "       FROM item, classcode, uom"
               "       WHERE ((item_classcode_id=classcode_id)"
               "         AND  (item_inv_uom_id=uom_id)" );

  if (_classCode->isSelected())
    sql += " AND (classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (classcode_code ~ :classcode_pattern)";

  sql += ") ) AS data ";

  if (_onlyShowZero->isChecked())
    sql += "WHERE ( (scost=0) OR (acost=0) ) ";

  sql += "ORDER BY item_number";

  q.prepare(sql);
  _classCode->bindValue(q);
  q.exec();
  _itemcost->populate(q);
}
