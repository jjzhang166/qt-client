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

#include "items.h"

#include <QVariant>
#include <metasql.h>
#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>
#include <QSqlError>
#include "copyItem.h"
#include "item.h"
#include "storedProcErrorLookup.h"

/*
 *  Constructs a items as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
items::items(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    QButtonGroup* _statusGroupInt = new QButtonGroup(this);
    _statusGroupInt->addButton(_showAll);
    _statusGroupInt->addButton(_showPurchased);
    _statusGroupInt->addButton(_showManufactured);
    _statusGroupInt->addButton(_showSold);

    // signals and slots connections
    connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
    connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_statusGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
    connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_item, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
items::~items()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void items::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void items::init()
{
  statusBar()->hide();
  
  _item->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   );
  _item->addColumn(tr("Active"),      _ynColumn,   Qt::AlignCenter );
  _item->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _item->addColumn(tr("Class Code"),  _dateColumn, Qt::AlignCenter );
  _item->addColumn(tr("Type"),        _itemColumn, Qt::AlignCenter );
  _item->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignCenter );
  _item->setDragString("itemid=");
  
  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  
  if (_privileges->check("MaintainItemMasters"))
  {
    connect(_item, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_item, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  if (_privileges->check("DeleteItemMasters"))
    connect(_item, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

  sFillList();

  _searchFor->setFocus();
}

void items::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Copy..."), this, SLOT(sCopy()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void items::sNew()
{
  item::newItem();
}

void items::sEdit()
{
  item::editItem(_item->id());
}

void items::sView()
{
  item::viewItem(_item->id());
}

void items::sDelete()
{
  q.prepare("SELECT deleteItem(:item_id) AS result;");
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteItem", result), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void items::sFillList( int pItemid, bool pLocal )
{
  QString sql( "SELECT item_id, item_number, formatBoolYN(item_active),"
               "       (item_descrip1 || ' ' || item_descrip2), classcode_code,"
               "       CASE WHEN (item_type='P') THEN text(<? value(\"purchased\") ?>)"
               "            WHEN (item_type='M') THEN text(<? value(\"manufactured\") ?>)"
			   "            WHEN (item_type='J') THEN text(<? value(\"job\") ?>)"
               "            WHEN (item_type='F') THEN text(<? value(\"phantom\") ?>)"
               "            WHEN (item_type='B') THEN text(<? value(\"breeder\") ?>)"
               "            WHEN (item_type='C') THEN text(<? value(\"coProduct\") ?>)"
               "            WHEN (item_type='Y') THEN text(<? value(\"byProduct\") ?>)"
               "            WHEN (item_type='R') THEN text(<? value(\"reference\") ?>)"
               "            WHEN (item_type='S') THEN text(<? value(\"costing\") ?>)"
               "            WHEN (item_type='T') THEN text(<? value(\"tooling\") ?>)"
               "            WHEN (item_type='A') THEN text(<? value(\"assortment\") ?>)"
               "            WHEN (item_type='O') THEN text(<? value(\"outside\") ?>)"
               "            WHEN (item_type='L') THEN text(<? value(\"planning\") ?>)"
               "            ELSE text(<? value(\"error\") ?>)"
               "       END,"
               "       uom_name "
               "FROM item, classcode, uom "
               "WHERE ( (item_classcode_id=classcode_id)"
               " AND (item_inv_uom_id=uom_id)"
               "<? if exists(\"showPurchased\") ?>"
               " AND (item_type IN ('P', 'O'))"
               "<? elseif exists(\"showManufactured\") ?>"
               " AND (item_type IN ('M', 'F', 'B', 'J'))"
               "<? elseif exists(\"showSold\") ?>"
               " AND (item_sold)"
               "<? endif ?>"
               "<? if exists(\"onlyShowActive\") ?>"
               " AND (item_active)"
               "<? endif ?>"
               ") ORDER BY"
               "<? if exists(\"ListNumericItemNumbersFirst\") ?>"
               " toNumeric(item_number, 999999999999999),"
               "<? endif ?>"
               " item_number;" );

  ParameterList params;

  if(_showPurchased->isChecked())
    params.append("showPurchased");
  else if(_showManufactured->isChecked())
    params.append("showManufactured");
  else if(_showSold->isChecked())
    params.append("showSold");

  if (!_showInactive->isChecked())
    params.append("onlyShowActive");

  if (_preferences->boolean("ListNumericItemNumbersFirst"))
    params.append("ListNumericItemNumbersFirst");
  
  params.append("purchased", tr("Purchased"));
  params.append("manufactured", tr("Manufactured"));
  params.append("job", tr("Job"));
  params.append("phantom", tr("Phantom"));
  params.append("breeder", tr("Breeder"));
  params.append("coProduct", tr("Co-Product"));
  params.append("byProduct", tr("By-Product"));
  params.append("reference", tr("Reference"));
  params.append("costing", tr("Costing"));
  params.append("tooling", tr("Tooling"));
  params.append("outside", tr("Outside Process"));
  params.append("planning", tr("Planning"));
  params.append("assortment", tr("Assortment"));
  params.append("error", tr("Error"));

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);

  if ((pItemid != -1) && (pLocal))
    _item->populate(q, pItemid);
  else
    _item->populate(q);
}

void items::sFillList()
{
  sFillList(-1, FALSE);
}

void items::sSearch( const QString &pTarget )
{
  _item->clearSelection();
  int i;
  for (i = 0; i < _item->topLevelItemCount(); i++)
  {
    if (_item->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }

  if (i < _item->topLevelItemCount())
  {
    _item->setCurrentItem(_item->topLevelItem(i));
    _item->scrollToItem(_item->topLevelItem(i));
  }
}

void items::sCopy()
{
  ParameterList params;
  params.append("item_id", _item->id());

  copyItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}
