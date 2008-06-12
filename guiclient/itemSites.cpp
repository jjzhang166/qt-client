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

#include "itemSites.h"

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "itemSite.h"
#include "storedProcErrorLookup.h"

itemSites::itemSites(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_itemSite, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _itemSite->addColumn(tr("Item Number"),   _itemColumn, Qt::AlignLeft   );
  _itemSite->addColumn(tr("Active"),        _dateColumn, Qt::AlignCenter );
  _itemSite->addColumn(tr("Description"),   -1,          Qt::AlignLeft   );
  _itemSite->addColumn(tr("Whs."),          _whsColumn,  Qt::AlignCenter );
  _itemSite->addColumn(tr("Cntrl. Method"), _itemColumn, Qt::AlignCenter );
  _itemSite->setDragString("itemsiteid=");

  _searchFor->setAcceptDrops(FALSE);
  
  connect(omfgThis, SIGNAL(itemsitesUpdated()), SLOT(sFillList()));

  if (_privileges->check("MaintainItemSites"))
  {
    connect(_itemSite, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_itemSite, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_itemSite, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_itemSite, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  if (_privileges->check("DeleteItemSites"))
    connect(_itemSite, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

  _copy->setVisible(_metrics->boolean("MultiWhs"));

  sFillList();

  _searchFor->setFocus();
}

itemSites::~itemSites()
{
    // no need to delete child widgets, Qt does it all for us
}

void itemSites::languageChange()
{
    retranslateUi(this);
}

void itemSites::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemSites::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", _itemSite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemSites::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", _itemSite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemSites::sCopy()
{
  q.prepare("SELECT copyItemSite(:olditemsiteid, NULL) AS result;");
  q.bindValue(":olditemsiteid", _itemSite->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("copyItemSite", result), __FILE__, __LINE__);
      return;
    }
    ParameterList params;
    params.append("mode", "edit");
    params.append("itemsite_id", result);

    itemSite newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() != XDialog::Accepted)
    {
      q.prepare("SELECT deleteItemSite(:itemsite_id) AS result;");
      q.bindValue(":itemsite_id", result);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("deleteItemSite", result), __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSites::sDelete()
{
  q.prepare("SELECT deleteItemSite(:itemsite_id) AS result;");
  q.bindValue(":itemsite_id", _itemSite->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteItemSite", result), __FILE__, __LINE__);
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

void itemSites::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Item Site"), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Edit Item Site"), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemSites"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void itemSites::sFillList()
{
  QString sql( "SELECT itemsite_id, item_number, formatBoolYN(itemsite_active),"
               "       (item_descrip1 || ' ' || item_descrip2), warehous_code,"
               "       CASE WHEN itemsite_controlmethod='R' THEN :regular"
               "            WHEN itemsite_controlmethod='N' THEN :none"
               "            WHEN itemsite_controlmethod='L' THEN :lotNumber"
               "            WHEN itemsite_controlmethod='S' THEN :serialNumber"
               "       END "
               "FROM itemsite, item, warehous "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)" );

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  if (!_showInactive->isChecked())
    sql += " AND (itemsite_active)";

  sql += ") "
         "ORDER BY item_number, warehous_code;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.bindValue(":regular", tr("Regular"));
  q.bindValue(":none", tr("None"));
  q.bindValue(":lotNumber", tr("Lot #"));
  q.bindValue(":serialNumber", tr("Serial #"));
  q.exec();
  _itemSite->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSites::sSearch(const QString &pTarget)
{
  QList<QTreeWidgetItem*> items = _itemSite->findItems(pTarget, Qt::MatchStartsWith, 0);

  if (items.count() > 0)
  {
    _itemSite->clearSelection();
    _itemSite->setItemSelected(items.at(0), true);
    _itemSite->scrollToItem(items.at(0));
  }
}

void itemSites::keyPressEvent( QKeyEvent * e )
{
#ifdef Q_WS_MAC
  if(e->key() == Qt::Key_N && e->state() == Qt::ControlModifier)
  {
    _new->animateClick();
    e->accept();
  }
  else if(e->key() == Qt::Key_E && e->state() == Qt::ControlModifier)
  {
    _edit->animateClick();
    e->accept();
  }
  if(e->isAccepted())
    return;
#endif
  e->ignore();
}
