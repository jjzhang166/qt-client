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

#include "searchForItem.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMenu>
#include <parameter.h>
#include "item.h"
#include "bom.h"
#include "boo.h"
#include "bbom.h"

/*
 *  Constructs a searchForItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
searchForItem::searchForItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_showInactive, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_searchNumber, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDescrip1, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDescrip2, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_search, SIGNAL(lostFocus()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu *, QTreeWidgetItem *)));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _item->addColumn(tr("Number"),      _itemColumn, Qt::AlignCenter, true, "item_number" );
  _item->addColumn(tr("Description"), -1,          Qt::AlignLeft  , true, "description" );
  _item->addColumn(tr("Type"),        _itemColumn, Qt::AlignLeft  , true, "type" );

  if (_privileges->check("MaintainItemMasters"))
  {
    connect(_item, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    connect(_item, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), SLOT(sFillList()));

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _searchNumber->setChecked(true);
    _searchDescrip1->setChecked(true);
    _searchDescrip2->setChecked(true);
  }

  _search->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
searchForItem::~searchForItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void searchForItem::languageChange()
{
  retranslateUi(this);
}

void searchForItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  bool hasBOM = false;
  q.prepare("SELECT (count(*) != 0) AS hasBOM"
            "  FROM bomhead"
            " WHERE (bomhead_item_id=:item_id); ");
  q.bindValue(":item_id", _item->id());
  q.exec();
  if(q.first())
    hasBOM = q.value("hasBOM").toBool();

  bool hasBOO = false;
  if (_metrics->value("Application") == "OpenMFG")
  {
    q.prepare("SELECT (count(*) != 0) AS hasBOO"
              "  FROM boohead"
              " WHERE (boohead_item_id=:item_id); ");
    q.bindValue(":item_id", _item->id());
    q.exec();
    if(q.first())
      hasBOO = q.value("hasBOO").toBool();
  }
  
  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  if ((((XTreeWidgetItem *)pSelected)->text(2) == tr("Purchased")) ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Manufactured")) ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Job")) ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Breeder")) ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Kit")))
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("View BOM..."), this, SLOT(sViewBOM()), 0);
    if (!hasBOM || !_privileges->check("ViewBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Edit BOM..."), this, SLOT(sEditBOM()), 0);
    if (!hasBOM || !_privileges->check("MaintainBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if ((((XTreeWidgetItem *)pSelected)->text(2) == tr("Purchased") ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Job")) ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Manufactured"))
      && _metrics->boolean("Routings")))
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("View BOO..."), this, SLOT(sViewBOO()), 0);
    if (!hasBOO || !_privileges->check("ViewBOOs"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Edit BOO..."), this, SLOT(sEditBOO()), 0);
    if (!hasBOO || !_privileges->check("MaintainBOOs"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if (((XTreeWidgetItem *)pSelected)->text(2) == tr("Breeder"))
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("View Breeder BOM..."), this, SLOT(sViewBBOM()), 0);
    if (!_privileges->check("ViewBBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Edit Breeder BOM..."), this, SLOT(sEditBBOM()), 0);
    if (!_privileges->check("MaintainBBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void searchForItem::sView()
{
  item::viewItem(_item->id());
}

void searchForItem::sEdit()
{
  item::editItem(_item->id());
}

void searchForItem::sViewBOM()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _item->id());
  
  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void searchForItem::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _item->id());
  
  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void searchForItem::sViewBOO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _item->id());
  
  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void searchForItem::sEditBOO()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _item->id());
  
  boo *newdlg = new boo();
  newdlg->set(params);
  newdlg->show();
}

void searchForItem::sViewBBOM()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _item->id());
  
  bbom *newdlg = new bbom();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void searchForItem::sEditBBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _item->id());
  
  bbom *newdlg = new bbom();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void searchForItem::sFillList()
{
  _item->clear();
  if (_search->text().trimmed().length() == 0)    
    return;

  QString sql( "SELECT item_id,"
               "       item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
               "       CASE WHEN (item_type='P') THEN :purchased"
               "            WHEN (item_type='M') THEN :manufactured" 
               "            WHEN (item_type='F') THEN :phantom"
               "            WHEN (item_type='B') THEN :breeder"
               "            WHEN (item_type='C') THEN :coProduct"
               "            WHEN (item_type='Y') THEN :byProduct"
               "            WHEN (item_type='R') THEN :reference"
               "            WHEN (item_type='S') THEN :costing"
               "            WHEN (item_type='T') THEN :tooling"
               "            WHEN (item_type='A') THEN :assortment"
               "            WHEN (item_type='O') THEN :outside"
               "            WHEN (item_type='J') THEN :job"
               "            WHEN (item_type='L') THEN :planning"
               "            WHEN (item_type='K') THEN :kit"
               "            ELSE :error"
               "       END AS type "
               "FROM item "
               "WHERE ( ( ((:useNumber) AND (item_number ~* :searchString))"
               "       OR ((:useDescrip1) AND (item_descrip1 ~* :searchString))"
               "       OR ((:useDescrip2) AND (item_descrip2 ~* :searchString)) )" );

  if (!_showInactive->isChecked())
    sql += " AND (item_active)";

  sql += " ) "
         "ORDER BY item_number;";

  q.prepare(sql);
  q.bindValue(":purchased", tr("Purchased"));
  q.bindValue(":manufactured", tr("Manufactured"));
  q.bindValue(":phantom", tr("Phantom"));
  q.bindValue(":breeder", tr("Breeder"));
  q.bindValue(":coProduct", tr("Co-Product"));
  q.bindValue(":byProduct", tr("By-Product"));
  q.bindValue(":reference", tr("Reference"));
  q.bindValue(":costing", tr("Costing"));
  q.bindValue(":tooling", tr("Tooling"));
  q.bindValue(":outside", tr("Outside Process"));
  q.bindValue(":assortment", tr("Assortment"));
  q.bindValue(":job", tr("Job"));
  q.bindValue(":planning", tr("Planning"));
  q.bindValue(":kit", tr("Kit"));
  q.bindValue(":error", tr("Error"));
  q.bindValue(":useNumber", QVariant(_searchNumber->isChecked()));
  q.bindValue(":useDescrip1", QVariant(_searchDescrip1->isChecked()));
  q.bindValue(":useDescrip2", QVariant(_searchDescrip2->isChecked()));
  q.bindValue(":searchString", _search->text().toUpper());
  q.exec();
  _item->populate(q);
}

