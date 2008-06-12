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

#include "dspItemsWithoutItemSources.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include "item.h"
#include "itemSource.h"

/*
 *  Constructs a dspItemsWithoutItemSources as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemsWithoutItemSources::dspItemsWithoutItemSources(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemsWithoutItemSources::~dspItemsWithoutItemSources()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemsWithoutItemSources::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void dspItemsWithoutItemSources::init()
{
  statusBar()->hide();
  
  _item->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _item->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _item->addColumn(tr("Type"),        _itemColumn,  Qt::AlignCenter );

}

void dspItemsWithoutItemSources::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Create Item Source..."), this, SLOT(sCreateItemSource()), 0);
  if (!_privileges->check("MaintainItemSources"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspItemsWithoutItemSources::sCreateItemSource()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspItemsWithoutItemSources::sEditItem()
{
  item::editItem(_item->id());
}

void dspItemsWithoutItemSources::sFillList()
{
  q.prepare( "SELECT item_id, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2) AS f_description,"
             "       CASE WHEN (item_type = 'P') THEN :purchased"
             "            WHEN (item_type = 'O') THEN :outside"
             "       END AS f_type "
             "FROM item "
             "WHERE ( (item_type IN ('P', 'O'))"
             " AND (item_active)"
             " AND (item_id NOT IN (SELECT DISTINCT itemsrc_item_id FROM itemsrc WHERE (itemsrc_active))) ) "
             "ORDER BY item_number;" );
  q.bindValue(":purchased", tr("Purchased"));
  q.bindValue(":outside", tr("Outside"));
  q.exec();
  _item->populate(q);
}

