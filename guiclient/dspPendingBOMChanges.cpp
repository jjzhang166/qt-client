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

#include "dspPendingBOMChanges.h"

#include <QMenu>

#include <openreports.h>
#include <parameter.h>

#include "bomItem.h"

dspPendingBOMChanges::dspPendingBOMChanges(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_revision, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_cutoff, SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _item->setType(ItemLineEdit::cGeneralManufactured);

  _cutoff->setNullString(tr("Latest"));
  _cutoff->setNullDate(omfgThis->endOfTime().addDays(-1));
  _cutoff->setAllowNullDate(TRUE);
  _cutoff->setNull();

  _bomitem->addColumn(tr("Date"),        _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Action"),      _itemColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Seq #"),       40,           Qt::AlignCenter );
  _bomitem->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _bomitem->addColumn(tr("Description"), -1,           Qt::AlignCenter );
  _bomitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Qty. Per"),    _qtyColumn,   Qt::AlignRight  );
  _bomitem->addColumn(tr("Scrap %"),     _prcntColumn, Qt::AlignRight  );
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  //If not Revision Control, hide control
  _revision->setVisible(_metrics->boolean("RevControl"));
}

dspPendingBOMChanges::~dspPendingBOMChanges()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPendingBOMChanges::languageChange()
{
  retranslateUi(this);
}

void dspPendingBOMChanges::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());
  params.append("cutOffDate", _cutoff->date());

  orReport report("PendingBOMChanges", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPendingBOMChanges::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit BOM Item..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainBOMs"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View BOM Item..."), this, SLOT(sView()), 0);
  if ( (!_privileges->check("MaintainBOMs")) && (!_privileges->check("ViewBOMs")) )
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspPendingBOMChanges::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bomitem_id", _bomitem->id());

  bomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPendingBOMChanges::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bomitem_id", _bomitem->id());

  bomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspPendingBOMChanges::sFillList()
{
  sFillList(-1, FALSE);
}

void dspPendingBOMChanges::sFillList(int, bool)
{
  if ((_item->isValid()) && (_cutoff->isValid()))
  {
    q.prepare( "SELECT bomitem_id, formatDate(bomitem_effective), :effective AS action,"
               "       bomitem_seqnumber, item_number, (item_descrip1 || ' ' || item_descrip2),"
               "       uom_name, formatQtyper(itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper)),"
               "       formatScrap(bomitem_scrap), bomitem_effective AS actiondate "
			   "FROM bomitem(:item_id,:revision_id), item, uom "
               "WHERE ( (bomitem_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (bomitem_effective BETWEEN CURRENT_DATE AND :cutOffDate) ) "
               "UNION SELECT bomitem_id, formatDate(bomitem_expires), :expires AS action, "
               "             bomitem_seqnumber, item_number, (item_descrip1 || ' ' || item_descrip2),"
               "             uom_name, formatQtyper(itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper)),"
               "             formatScrap(bomitem_scrap), bomitem_expires AS actiondate "
			   "FROM bomitem(:item_id,:revision_id), item, uom "
               "WHERE ( (bomitem_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (bomitem_expires BETWEEN CURRENT_DATE AND :cutOffDate) ) "
               "ORDER BY action, actiondate, bomitem_seqnumber;" );
    q.bindValue(":effective", tr("Effective"));
    q.bindValue(":expires", tr("Expires"));
    q.bindValue(":item_id", _item->id());
	q.bindValue(":revision_id", _revision->id());
    q.bindValue(":cutOffDate", _cutoff->date());
    q.exec();
    _bomitem->populate(q);
  }
  else
    _bomitem->clear();
}
