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

#include "dspUnusedPurchasedItems.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <openreports.h>
#include <parameter.h>

/*
 *  Constructs a dspUnusedPurchasedItems as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspUnusedPurchasedItems::dspUnusedPurchasedItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspUnusedPurchasedItems::~dspUnusedPurchasedItems()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspUnusedPurchasedItems::languageChange()
{
    retranslateUi(this);
}


void dspUnusedPurchasedItems::init()
{
  statusBar()->hide();
  
  _classCode->setType(ClassCode);

  _item->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft  );
  _item->addColumn(tr("Description"), -1,          Qt::AlignLeft  );
  _item->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignLeft  );
  _item->addColumn(tr("Total QOH"),   _qtyColumn,  Qt::AlignRight );
  _item->addColumn(tr("Last Cnt'd"),  _dateColumn, Qt::AlignRight );
  _item->addColumn(tr("Last Used"),   _dateColumn, Qt::AlignRight );
}

void dspUnusedPurchasedItems::sPrint()
{
  ParameterList params;

  _classCode->appendValue(params);

  if(_includeUncontrolled->isChecked())
    params.append("includeUncontrolledItems");

  orReport report("UnusedPurchasedItems", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspUnusedPurchasedItems::sFillList()
{
  QString sql( "SELECT DISTINCT item_id, item_number,"
               "                (item_descrip1 || ' ' || item_descrip2), uom_name,"
               "                formatQty(SUM(itemsite_qtyonhand)),"
               "                formatDate(MAX(itemsite_datelastcount), 'Never'),"
               "                formatDate(MAX(itemsite_datelastused), 'Never') "
               "FROM item, itemsite, uom "
               "WHERE ((itemsite_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (item_id NOT IN (SELECT DISTINCT bomitem_item_id FROM bomitem))"
               " AND (NOT item_sold)"
               " AND (item_type IN ('P', 'O'))" );

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  if (!_includeUncontrolled->isChecked())
    sql += " AND (itemsite_controlmethod <> 'N')";

  sql += ") "
         "GROUP BY item_id, item_number, uom_name, item_descrip1, item_descrip2 "
         "ORDER BY item_number;";

  q.prepare(sql);
  _classCode->bindValue(q);
  q.exec();
  _item->populate(q);
}
