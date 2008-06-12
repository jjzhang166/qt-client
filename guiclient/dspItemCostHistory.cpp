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

#include "dspItemCostHistory.h"

#include <QMessageBox>

#include <openreports.h>
#include <parameter.h>

dspItemCostHistory::dspItemCostHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _itemcost->addColumn(tr("Element"),              -1, Qt::AlignLeft,  true, "cost_elem_type");
  _itemcost->addColumn(tr("Lower"),       _costColumn, Qt::AlignCenter,true, "lowlevel");
  _itemcost->addColumn(tr("Type"),        _costColumn, Qt::AlignLeft,  true, "type");
  _itemcost->addColumn(tr("Time"),    _timeDateColumn, Qt::AlignCenter,true, "costhist_date");
  _itemcost->addColumn(tr("User"),         _qtyColumn, Qt::AlignCenter,true, "username");
  _itemcost->addColumn(tr("Old"),         _costColumn, Qt::AlignRight, true, "costhist_oldcost");
  _itemcost->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "oldcurr");
  _itemcost->addColumn(tr("New"),         _costColumn, Qt::AlignRight, true, "costhist_newcost");
  _itemcost->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "newcurr");

  if (omfgThis->singleCurrency())
  {
      _itemcost->hideColumn("oldcurr");
      _itemcost->hideColumn("newcurr");
  }
}

dspItemCostHistory::~dspItemCostHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspItemCostHistory::languageChange()
{
  retranslateUi(this);
}

void dspItemCostHistory::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

  orReport report("ItemCostHistory", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemCostHistory::sFillList()
{
  _itemcost->clear();

  if (_item->isValid())
  {
    q.prepare( "SELECT costhist_id, costelem_type,"
              "        formatBoolYN(costhist_lowlevel) AS lowlevel,"
               "       CASE WHEN costhist_type='A' THEN :actual"
               "            WHEN costhist_type='S' THEN :standard"
               "            WHEN costhist_type='D' THEN :delete"
               "            WHEN costhist_type='N' THEN :new"
               "       END AS type,"
               "       costhist_date,"
               "       getUserName(costhist_user_id) AS username,"
               "       costhist_oldcost,"
               "       currConcat(costhist_oldcurr_id) AS oldcurr, "
	       "       costhist_newcost,"
               "       currConcat(costhist_newcurr_id) AS newcurr,"
               "       'cost' AS costhist_oldcost_xtnumericrole,"
               "       'cost' AS costhist_newcost_xtnumericrole "
               "FROM costhist, costelem "
               "WHERE ( (costhist_costelem_id=costelem_id)"
               " AND (costhist_item_id=:item_id) ) "
               "ORDER BY costhist_date, costelem_type;" );
    q.bindValue(":actual", tr("Actual"));
    q.bindValue(":standard", tr("Standard"));
    q.bindValue(":delete", tr("Delete"));
    q.bindValue(":new", tr("New"));
    q.bindValue(":item_id", _item->id());
    q.exec();
    _itemcost->populate(q);
  }
}
