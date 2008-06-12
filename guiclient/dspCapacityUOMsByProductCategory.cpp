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

#include "dspCapacityUOMsByProductCategory.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include "item.h"
#include "guiclient.h"

/*
 *  Constructs a dspCapacityUOMsByProductCategory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCapacityUOMsByProductCategory::dspCapacityUOMsByProductCategory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _productCategory->setType(ProductCategory);

  _item->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft   );
  _item->addColumn(tr("Description"),    -1,          Qt::AlignLeft   );
  _item->addColumn(tr("Inv. UOM"),       _uomColumn,  Qt::AlignCenter );
  _item->addColumn(tr("Cap. UOM"),       _uomColumn,  Qt::AlignCenter );
  _item->addColumn(tr("Cap./Inv. Rat."), _qtyColumn,  Qt::AlignRight  );
  _item->addColumn(tr("Alt. UOM"),       _uomColumn,  Qt::AlignCenter );
  _item->addColumn(tr("Alt/Inv Ratio"),  _qtyColumn,  Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCapacityUOMsByProductCategory::~dspCapacityUOMsByProductCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCapacityUOMsByProductCategory::languageChange()
{
  retranslateUi(this);
}

void dspCapacityUOMsByProductCategory::sPrint()
{
  ParameterList params;

  _productCategory->appendValue(params);

  orReport report("CapacityUOMsByProductCategory", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCapacityUOMsByProductCategory::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspCapacityUOMsByProductCategory::sEditItem()
{
  item::editItem(_item->id());
}

void dspCapacityUOMsByProductCategory::sFillList()
{
  sFillList(-1, FALSE);
}

void dspCapacityUOMsByProductCategory::sFillList(int pItemid, bool pLocalUpdate)
{
  QString sql( "SELECT item_id, item_number,"
               " (item_descrip1 || ' ' || item_descrip2), uom_name,"
               " itemcapuom(item_id), formatQty(itemcapinvrat(item_id)), itemaltcapuom(item_id), formatQty(itemaltcapinvrat(item_id)) "
               "FROM item JOIN uom ON (item_inv_uom_id=uom_id) "
               "WHERE ( (item_sold)");

  if (_productCategory->isSelected())
    sql += " AND (item_prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += " AND (item_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :prodcat_pattern)))";

  sql += " ) "
         "ORDER BY item_number";

  q.prepare(sql);
  _productCategory->bindValue(q);
  q.exec();

  if ((pItemid != -1) && (pLocalUpdate))
    _item->populate(q, pItemid);
  else
    _item->populate(q);
}

