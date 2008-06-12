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

#include "dspItemsByCharacteristic.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include "boo.h"
#include "bom.h"
#include "item.h"
#include "guiclient.h"

/*
 *  Constructs a dspItemsByCharacteristic as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemsByCharacteristic::dspItemsByCharacteristic(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _char->populate( "SELECT char_id, char_name "
                   "FROM char "
                   "WHERE (char_items) "
                   "ORDER BY char_name;" );

  _item->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft   );
  _item->addColumn(tr("Description"),    -1,          Qt::AlignLeft   );
  _item->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignCenter );
  _item->addColumn(tr("Value"),          _itemColumn, Qt::AlignLeft   );
  _item->addColumn(tr("Type"),           _itemColumn, Qt::AlignCenter );
  _item->addColumn(tr("UOM"),            _uomColumn,  Qt::AlignCenter );

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemsByCharacteristic::~dspItemsByCharacteristic()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemsByCharacteristic::languageChange()
{
  retranslateUi(this);
}

void dspItemsByCharacteristic::sPrint()
{
  ParameterList params;

  params.append("char_id", _char->id());
  params.append("value", _value->text());

  if(_showInactive->isChecked())
    params.append("showInactive");

  orReport report("ItemsByCharacteristic", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemsByCharacteristic::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Item Master..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Item Master..."), this, SLOT(sView()), 0);

  if (selected->text(2) == "M")
  {
    menuItem = pMenu->insertItem(tr("Edit Bill of Material..."), this, SLOT(sEditBOM()), 0);
    if (!_privileges->check("MaintainBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Bill of Material..."), this, SLOT(sViewBOM()), 0);
    if ( (!_privileges->check("MaintainBOMs")) && (!_privileges->check("ViewBOMs")) )
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Edit Item Bill of Operations..."), this, SLOT(sEditBOO()), 0);
    if (!_privileges->check("MaintainBOOs"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Item Bill of Operations..."), this, SLOT(sViewBOO()), 0);
    if ( (!_privileges->check("MaintainBOOs")) && (!_privileges->check("ViewBOOs")) )
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspItemsByCharacteristic::sEdit()
{
  item::editItem(_item->id());
}

void dspItemsByCharacteristic::sView()
{
  item::viewItem(_item->id());
}

void dspItemsByCharacteristic::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _item->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByCharacteristic::sViewBOM()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _item->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByCharacteristic::sEditBOO()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _item->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByCharacteristic::sViewBOO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _item->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByCharacteristic::sFillList()
{
  sFillList(-1, TRUE);
}

void dspItemsByCharacteristic::sFillList(int pItemid, bool pLocal)
{
  QString sql( "SELECT item_id, item_number, (item_descrip1 || ' ' || item_descrip2),"
               "       char_name, charass_value,"
               "       CASE WHEN (item_type='P') THEN :purchased"
               "            WHEN (item_type='M') THEN :manufactured"
               "            WHEN (item_type='J') THEN :job"
               "            WHEN (item_type='F') THEN :phantom"
               "            WHEN (item_type='B') THEN :breeder"
               "            WHEN (item_type='C') THEN :coProduct"
               "            WHEN (item_type='Y') THEN :byProduct"
               "            WHEN (item_type='R') THEN :reference"
               "            WHEN (item_type='S') THEN :costing"
               "            WHEN (item_type='T') THEN :tooling"
               "            WHEN (item_type='O') THEN :outsideProcess"
               "            WHEN (item_type='L') THEN :planning"
               "            ELSE :error"
               "       END,"
               "       uom_name "
               "FROM item, charass, char, uom "
               "WHERE ( (charass_target_type='I')"
               " AND (charass_target_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (charass_char_id=char_id)"
               " AND (char_id=:char_id)"
               " AND (charass_value ~* :charass_value) " );
    
  if (!_showInactive->isChecked())
    sql += " AND (item_active)";

  sql += ") "
         "ORDER BY item_number;";

  q.prepare(sql);
  q.bindValue(":purchased", tr("Purchased"));
  q.bindValue(":manufactured", tr("Manufactured"));
  q.bindValue(":job", tr("Job"));
  q.bindValue(":phantom", tr("Phantom"));
  q.bindValue(":breeder", tr("Breeder"));
  q.bindValue(":coProduct", tr("Co-Product"));
  q.bindValue(":byProduct", tr("By-Product"));
  q.bindValue(":reference", tr("Reference"));
  q.bindValue(":costing", tr("Costing"));
  q.bindValue(":tooling", tr("Tooling"));
  q.bindValue(":outsideProcess", tr("Outside Process"));
  q.bindValue(":planning", tr("Planning"));
  q.bindValue(":error", tr("Error"));
  q.bindValue(":char_id", _char->id());
  q.bindValue(":charass_value", _value->text().stripWhiteSpace());
  q.exec();

  if ((pItemid != -1) && (pLocal))
    _item->populate(q, pItemid);
  else
    _item->populate(q);

  _item->setDragString("itemid=");
}

