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

#include "bomList.h"

#include <QMessageBox>

#include <openreports.h>
#include <parameter.h>

#include "bom.h"
#include "copyBOM.h"

bomList::bomList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _bom->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft );
  _bom->addColumn(tr("Description"), -1,          Qt::AlignLeft );
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
  
  if (_privileges->check("MaintainBOMs"))
  {
    connect(_bom, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_bom, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_bom, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));

    connect(_bom, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_bom, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList(-1, FALSE);

  _searchFor->setFocus();
}

bomList::~bomList()
{
  // no need to delete child widgets, Qt does it all for us
}

void bomList::languageChange()
{
  retranslateUi(this);
}

void bomList::sCopy()
{
  ParameterList params;
  params.append("item_id", _bom->id());

  copyBOM newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void bomList::sDelete()
{
  if (QMessageBox::critical( this, tr("Delete Bill of Materials"),
                             tr( "Are you sure that you want to delete the selected Bill of Materials?"),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare( "SELECT deletebom(:item_id);" );
    q.bindValue(":item_id", _bom->id());
    q.exec();

    omfgThis->sBOMsUpdated(-1, TRUE);
  }
}

void bomList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _bom->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bomList::sFillList( int pItemid, bool pLocal )
{
  QString sql;

  sql = "SELECT DISTINCT item_id, "
        " CASE WHEN "
        "  bomitem_rev_id=-1 THEN "
        "   0 "
        " ELSE 1 "
        " END AS revcontrol, "
        " item_number, (item_descrip1 || ' ' || item_descrip2) "
        "FROM item "
        "  LEFT OUTER JOIN bomitem ON (item_id=bomitem_parent_item_id) "
        "  LEFT OUTER JOIN bomhead ON (item_id=bomhead_item_id) "
        "WHERE (((bomitem_id IS NOT NULL) "
        "OR (bomhead_id IS NOT NULL)) ";

  if (!_showInactive->isChecked())
    sql += " AND (item_active)";

  sql += ") "
         "ORDER BY item_number;";

  if ((pItemid != -1) && (pLocal))
    _bom->populate(sql, TRUE, pItemid);
  else
    _bom->populate(sql, TRUE);
}

void bomList::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bomList::sPrint()
{
  ParameterList params;
  params.append( "item_id", _bom->id() );

  orReport report("SingleLevelBOM", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void bomList::sSearch( const QString &pTarget )
{
  _bom->clearSelection();
  int i;
  for (i = 0; i < _bom->topLevelItemCount(); i++)
  {
   if (_bom->topLevelItem(i)->text(0).contains(pTarget, Qt::CaseInsensitive))
    break;
  }
    
  if (i < _bom->topLevelItemCount())
  {
    _bom->setCurrentItem(_bom->topLevelItem(i));
    _bom->scrollToItem(_bom->topLevelItem(i));
  }
}

void bomList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _bom->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bomList::sFillList()
{
  sFillList(-1, TRUE);
}

void bomList::sHandleButtons()
{
  if (_bom->altId() == 0)
    _delete->setEnabled(TRUE);
  else
    _delete->setEnabled(FALSE);
}

