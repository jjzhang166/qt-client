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

#include "dspWoHistoryByItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <QWorkspace>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include "workOrder.h"

/*
 *  Constructs a dspWoHistoryByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoHistoryByItem::dspWoHistoryByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_showCost, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));
  connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cJob);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _wo->addColumn(tr("W/O #"),      -1,            Qt::AlignLeft   );
  _wo->addColumn(tr("Status"),     _statusColumn, Qt::AlignCenter );
  _wo->addColumn(tr("Whs."),       _whsColumn,    Qt::AlignCenter );
  _wo->addColumn(tr("Ordered"),    _qtyColumn,    Qt::AlignRight  );
  _wo->addColumn(tr("Received"),   _qtyColumn,    Qt::AlignRight  );
  _wo->addColumn(tr("Start Date"), _dateColumn,   Qt::AlignCenter );
  _wo->addColumn(tr("Due Date"),   _dateColumn,   Qt::AlignCenter );
  _wo->addColumn(tr("Cost"),	   _costColumn,   Qt::AlignRight );

  sHandleCosts(_showCost->isChecked());
  
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _item->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoHistoryByItem::~dspWoHistoryByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoHistoryByItem::languageChange()
{
  retranslateUi(this);
}

void dspWoHistoryByItem::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _item->id());

  if(_showOnlyTopLevel->isChecked())
    params.append("showOnlyTopLevel");

  if(_showCost->isChecked())
    params.append("showCosts");

  orReport report("WOHistoryByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoHistoryByItem::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspWoHistoryByItem::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
    _wo->showColumn(7);
  else
    _wo->hideColumn(7);

  sFillList();
}

void dspWoHistoryByItem::sFillList()
{
  if (!checkParameters())
    return;

  _wo->clear();

  QString sql( "SELECT wo_id,"
               "       formatWONumber(wo_id) AS wonumber,"
               "       wo_status, warehous_code,"
               "       formatQty(wo_qtyord) AS qtyord,"
               "       formatQty(wo_qtyrcv) AS qtyrcv,"
               "       formatDate(wo_startdate) AS startdate,"
               "       formatDate(wo_duedate) AS duedate,"
               "       ( (wo_startdate <= CURRENT_DATE) AND (wo_status IN ('O','E','S','R')) ) AS latestart,"
               "       (wo_duedate <= CURRENT_DATE) AS latedue,"
               "       formatCost(wo_postedvalue) AS value "
               "FROM wo, itemsite, warehous "
               "WHERE ((wo_itemsite_id=itemsite_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (itemsite_item_id=:item_id)"
               " AND (wo_duedate BETWEEN :startDate AND :endDate)" );

  if (_showOnlyTopLevel->isChecked())
    sql += " AND ( (wo_ordtype<>'W') OR (wo_ordtype IS NULL) )";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY wo_startdate DESC, wo_number, wo_subnumber;";

  q.prepare(sql);
  _dates->bindValue(q);
  _warehouse->bindValue(q);
  q.bindValue(":item_id", _item->id());
  q.exec();
  XTreeWidgetItem *last = 0;
  while (q.next())
  {
    last = new XTreeWidgetItem( _wo, last, q.value("wo_id").toInt(),
			       q.value("wonumber"), q.value("wo_status"),
			       q.value("warehous_code"),  q.value("qtyord"),
			       q.value("qtyrcv"), q.value("startdate"),
			       q.value("duedate") );

    if (q.value("latestart").toBool())
      last->setTextColor(5, "red");

    if (q.value("latedue").toBool())
      last->setTextColor(6, "red");

    if (_showCost->isChecked())
      last->setText(7, q.value("value").toString());
  }
}

bool dspWoHistoryByItem::checkParameters()
{
  return TRUE;
}

