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

#include "dspFrozenItemSites.h"

#include <QMenu>

#include <openreports.h>

dspFrozenItemSites::dspFrozenItemSites(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemsite, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _itemsite->addColumn(tr("Whs."),        _whsColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   );
  _itemsite->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _itemsite->addColumn(tr("Count Tag #"), _qtyColumn,  Qt::AlignRight  );

  sFillList();
}

dspFrozenItemSites::~dspFrozenItemSites()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspFrozenItemSites::languageChange()
{
  retranslateUi(this);
}

void dspFrozenItemSites::sPrint()
{
  ParameterList params;

  _warehouse->appendValue(params);

  orReport report("FrozenItemSites", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspFrozenItemSites::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Thaw"), this, SLOT(sThaw()), 0);
  if (!_privileges->check("ThawInventory"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspFrozenItemSites::sThaw()
{
  q.prepare("SELECT thawItemsite(:itemsite_id) AS result;");
  q.bindValue(":itemsite_id", _itemsite->id());
  q.exec();

  sFillList();
}

void dspFrozenItemSites::sFillList()
{
  QString sql( "SELECT itemsite_id, warehous_code, item_number,"
               " (item_descrip1 || ' ' || item_descrip2),"
               " COALESCE((SELECT invcnt_tagnumber FROM invcnt WHERE ((NOT invcnt_posted) AND (invcnt_itemsite_id=itemsite_id)) LIMIT 1), '') "
               "FROM itemsite, item, warehous "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (itemsite_freeze)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += " ) "
         "ORDER BY warehous_code, item_number";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.exec();
  _itemsite->populate(q);
}
