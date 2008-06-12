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

#include "dspItemCostSummary.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include "dspItemCostDetail.h"
#include "itemCost.h"

/*
 *  Constructs a dspItemCostSummary as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemCostSummary::dspItemCostSummary(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_itemcost, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _itemcost->addColumn(tr("Element"),   -1,           Qt::AlignLeft   );
  _itemcost->addColumn(tr("Lower"),     _costColumn,  Qt::AlignCenter );
  _itemcost->addColumn(tr("Std. Cost"), _costColumn,  Qt::AlignRight  );
  _itemcost->addColumn(tr("Posted"),    _dateColumn,  Qt::AlignCenter );
  _itemcost->addColumn(tr("Act. Cost"), _costColumn,  Qt::AlignRight  );
  _itemcost->addColumn(tr("Updated"),   _dateColumn,  Qt::AlignCenter );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemCostSummary::~dspItemCostSummary()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemCostSummary::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspItemCostSummary::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspItemCostSummary::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

  orReport report("ItemCostSummary", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemCostSummary::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if (pSelected->text(1) == "Yes")
  {
    pMenu->insertItem(tr("View Costing Detail..."), this, SLOT(sViewDetail()), 0);
    pMenu->insertSeparator();
  }
}

void dspItemCostSummary::sViewDetail()
{
  ParameterList params;
  params.append("itemcost_id", _itemcost->id());
  params.append("run");

  dspItemCostDetail *newdlg = new dspItemCostDetail();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemCostSummary::sFillList()
{
  if (_item->isValid())
  {
    double standardCost = 0.0;
    double actualCost = 0.0;

    q.prepare( "SELECT itemcost_id,"
               "       CASE WHEN (costelem_sys) THEN 1"
               "            ELSE 0"
               "       END,"
               "       costelem_type, formatBoolYN(itemcost_lowlevel),"
               "       formatCost(itemcost_stdcost), formatDate(itemcost_posted, 'Never'),"
               "       formatcost(itemcost_actcost), formatDate(itemcost_updated, 'Never'),"
               "       itemcost_stdcost AS stdcost, itemcost_actcost AS actcost "
               "FROM itemcost, costelem "
               "WHERE ( (itemcost_costelem_id=costelem_id)"
               " AND (itemcost_item_id=:item_id) ) "
               "ORDER BY itemcost_lowlevel, costelem_type;" );
    q.bindValue(":item_id", _item->id());
    q.exec();
    _itemcost->populate(q, TRUE);

    if (q.first())
    {
      do
      {
        standardCost += q.value("stdcost").toDouble();
        actualCost += q.value("actcost").toDouble();
      }
      while (q.next());

      new XTreeWidgetItem( _itemcost,
			   _itemcost->topLevelItem(_itemcost->topLevelItemCount() - 1),
			   -1,
			   QVariant(tr("Totals")), "", formatCost(standardCost),
			   "", formatCost(actualCost) );
    }
  }
  else
    _itemcost->clear();
}
