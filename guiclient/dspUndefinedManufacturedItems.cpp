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

#include "dspUndefinedManufacturedItems.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <parameter.h>
#include "boo.h"
#include "bom.h"
#include "item.h"

/*
 *  Constructs a dspUndefinedManufacturedItems as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspUndefinedManufacturedItems::dspUndefinedManufacturedItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    init();
  
  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _boo->setChecked(true);
    _bom->setChecked(true);
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspUndefinedManufacturedItems::~dspUndefinedManufacturedItems()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspUndefinedManufacturedItems::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void dspUndefinedManufacturedItems::init()
{
  statusBar()->hide();
  
  if (!_privileges->check("ViewBOMs"))
  {
    _bom->setChecked(FALSE);
    _bom->setEnabled(FALSE);
  }

  if (!_privileges->check("ViewBOOs"))
  {
    _boo->setChecked(FALSE);
    _boo->setEnabled(FALSE);
  }

  _item->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _item->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _item->addColumn(tr("Type"),        _uomColumn,   Qt::AlignCenter );
  _item->addColumn(tr("Active"),      _orderColumn, Qt::AlignCenter );
  _item->addColumn(tr("Exception"),   _itemColumn,  Qt::AlignCenter );

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
}

void dspUndefinedManufacturedItems::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if (((XTreeWidgetItem *)_item->currentItem())->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Create BOO..."), this, SLOT(sCreateBOO()), 0);
    if (!_privileges->check("MaintainBOOs"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (((XTreeWidgetItem *)_item->currentItem())->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Create BOM..."), this, SLOT(sCreateBOM()), 0);
    if (!_privileges->check("MaintainBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspUndefinedManufacturedItems::sCreateBOO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUndefinedManufacturedItems::sCreateBOM()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUndefinedManufacturedItems::sEditItem()
{
  item::editItem(_item->id());
}

void dspUndefinedManufacturedItems::sFillList()
{
  sFillList(-1, TRUE);
}

void dspUndefinedManufacturedItems::sFillList(int pItemid, bool pLocal)
{
  if ((_boo->isChecked()) || (_bom->isChecked()))
  {
    QString sql;

    if (_boo->isChecked())
    {
      sql = "SELECT item_id, 1, item_number, (item_descrip1 || ' ' || item_descrip2), item_type,"
            "       formatBoolYN(item_active), :noBoo AS exception "
            "FROM item "
            "WHERE ( (item_type='M')"
            " AND (item_id NOT IN (SELECT DISTINCT booitem_item_id FROM booitem) )";

      if (!_showInactive->isChecked())
        sql += " AND (item_active) ";

      sql += ") ";
    }

    if (_bom->isChecked())
    {
      if (_boo->isChecked())
        sql += "UNION ";

      sql += "SELECT item_id, 2, item_number, (item_descrip1 || ' ' || item_descrip2), item_type,"
             "       formatBoolYN(item_active), :noBom AS exception "
             "FROM item "
             "WHERE ( (item_type IN ('M', 'F'))"
             " AND (item_id NOT IN (SELECT DISTINCT bomitem_parent_item_id FROM bomitem) )";

      if (!_showInactive->isChecked())
        sql += " AND (item_active) ";

      sql += ") ";
    }

    sql += "ORDER BY item_number;";

    q.prepare(sql);
    q.bindValue(":noBoo", tr("No BOO"));
    q.bindValue(":noBom", tr("No BOM"));
    q.exec();

    if ((pItemid != -1) && (pLocal))
      _item->populate(q, pItemid, TRUE);
    else
      _item->populate(q, TRUE);
  }
  else
    _item->clear();
}

