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

#include "dspLaborVarianceByBOOItem.h"

#include <QVariant>
#include <QStatusBar>
#include <QMenu>
#include <parameter.h>
#include <openreports.h>
#include "guiclient.h"

/*
 *  Constructs a dspLaborVarianceByBOOItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspLaborVarianceByBOOItem::dspLaborVarianceByBOOItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_woopervar, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_booitem, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateComponentItems(int)));
  connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
  connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemSites(int)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cGeneralManufactured);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _woopervar->addColumn(tr("Post Date"),  _dateColumn, Qt::AlignCenter );
  _woopervar->addColumn(tr("Ordered"),    _qtyColumn,  Qt::AlignRight  );
  _woopervar->addColumn(tr("Produced"),   _qtyColumn,  Qt::AlignRight  );
  _woopervar->addColumn(tr("Proj Setup"), _timeColumn, Qt::AlignRight  );
  _woopervar->addColumn(tr("Proj. Run"),  _timeColumn, Qt::AlignRight  );
  _woopervar->addColumn(tr("Act. Setup"), _timeColumn, Qt::AlignRight  );
  _woopervar->addColumn(tr("Act. Run"),   _timeColumn, Qt::AlignRight  );
  _woopervar->addColumn(tr("Setup Var."), _timeColumn, Qt::AlignRight  );
  _woopervar->addColumn(tr("Run Var."),   _timeColumn, Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspLaborVarianceByBOOItem::~dspLaborVarianceByBOOItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspLaborVarianceByBOOItem::languageChange()
{
  retranslateUi(this);
}

void dspLaborVarianceByBOOItem::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _item->id());
  params.append("booitem_id", _booitem->id());

  orReport report("LaborVarianceByBOOItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspLaborVarianceByBOOItem::sPopulateMenu(QMenu *)
{
}

void dspLaborVarianceByBOOItem::sPopulateComponentItems(int pItemid)
{
  if (pItemid != -1)
  {
    q.prepare( "SELECT booitem_id,"
               "       (booitem_seqnumber || '-' || ' ' || booitem_descrip1 || ' ' || booitem_descrip2) "
               "FROM booitem "
               "WHERE (booitem_item_id=:item_id)"
               "ORDER BY booitem_seqnumber;" );
    q.bindValue(":item_id", pItemid);
    q.exec();
    _booitem->populate(q);
  }
  else
    _booitem->clear();
}

void dspLaborVarianceByBOOItem::sFillList()
{
  if (!checkParameters())
    return;

  QString sql( "SELECT woopervar_id, formatDate(woopervar_posted),"
               "       formatQty(woopervar_qtyord),"
               "       formatQty(woopervar_qtyrcv),"
               "       formatTime(woopervar_stdsutime),"
               "       formatTime(woopervar_stdrntime),"
               "       formatTime(woopervar_sutime),"
               "       formatTime(woopervar_rntime),"
               "       formattime(woopervar_sutime - woopervar_stdsutime),"
               "       formattime(woopervar_rntime - woopervar_stdrntime) "
               "FROM woopervar, itemsite "
               "WHERE ( (woopervar_parent_itemsite_id=itemsite_id)"
               " AND (woopervar_booitem_id=:booitem_id)"
               " AND (woopervar_posted BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY woopervar_posted DESC;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":booitem_id", _booitem->id());
  q.exec();
  _woopervar->populate(q);
}

bool dspLaborVarianceByBOOItem::checkParameters()
{
  if (_booitem->id() == -1)
  {
    return FALSE;
  }

  if (!_dates->startDate().isValid())
  {
    return FALSE;
  }

  if (!_dates->endDate().isValid())
  {
    return FALSE;
  }

  return TRUE;
}


