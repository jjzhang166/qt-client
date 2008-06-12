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

#include "dspOrderActivityByProject.h"

#include <QVariant>
#include <QWorkspace>
#include <QMenu>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include "guiclient.h"
#include "salesOrder.h"
#include "invoice.h"
#include "purchaseOrderItem.h"

/*
 *  Constructs a dspOrderActivityByProject as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspOrderActivityByProject::dspOrderActivityByProject(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_project, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_project, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_showPo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showSo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showWo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_orders, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  _orders->addColumn(tr("Type"),        _orderColumn, Qt::AlignLeft   );
  _orders->addColumn(tr("Order #"),     _itemColumn,  Qt::AlignLeft   );
  _orders->addColumn(tr("Status"),      _orderColumn, Qt::AlignCenter );
  _orders->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignLeft   );
  
  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _showPo->setChecked(true);
    _showSo->setChecked(true);
    _showWo->setChecked(true);
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspOrderActivityByProject::~dspOrderActivityByProject()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspOrderActivityByProject::languageChange()
{
  retranslateUi(this);
}

void dspOrderActivityByProject::sPopulateMenu( QMenu * pMenu )
{
  int menuItem;

  if(_orders->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Edit Sales Order..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Sales Order..."), this, SLOT(sView()), 0);
    if (!_privileges->check("MaintainSalesOrders") && !_privileges->check("ViewSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if(_orders->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Edit Quote..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainQuotes"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Quote..."), this, SLOT(sView()), 0);
    if (!_privileges->check("MaintainQuotes") && !_privileges->check("ViewQuotes"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if(_orders->altId() == 3)
  {
    menuItem = pMenu->insertItem(tr("Edit Invoice..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainMiscInvoices"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Invoice..."), this, SLOT(sView()), 0);
    if (!_privileges->check("MaintainMiscInvoices") && !_privileges->check("ViewMiscInvoices"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if(_orders->altId() == 5)
  {
    menuItem = pMenu->insertItem(tr("Edit P/O Item..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View P/O Item..."), this, SLOT(sView()), 0);
    if (!_privileges->check("MaintainPurchaseOrders") && !_privileges->check("ViewPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

}

void dspOrderActivityByProject::sEdit()
{
  if(_orders->altId() == 1)
  {
    salesOrder::editSalesOrder(_orders->id(), false);
  }
  else if(_orders->altId() == 2)
  {
    ParameterList params;
    params.append("mode", "editQuote");
    params.append("quhead_id", _orders->id());

    salesOrder *newdlg = new salesOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(_orders->altId() == 3)
  {
    invoice::editInvoice(_orders->id());
  }
  else if(_orders->altId() == 5)
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("poitem_id", _orders->id());

    purchaseOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspOrderActivityByProject::sView()
{
  if(_orders->altId() == 1)
  {
    salesOrder::viewSalesOrder(_orders->id());
  }
  else if(_orders->altId() == 2)
  {
    ParameterList params;
    params.append("mode", "viewQuote");
    params.append("quhead_id", _orders->id());

    salesOrder *newdlg = new salesOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(_orders->altId() == 3)
  {
    invoice::viewInvoice(_orders->id());
  }
  else if(_orders->altId() == 5)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("poitem_id", _orders->id());

    purchaseOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspOrderActivityByProject::sFillList()
{
  if(_project->id() == -1)
  {
    _orders->clear();
    return;
  }

  QString sql;

  if(_showSo->isChecked())
  {
    sql += "SELECT cohead_id AS id, 1 AS typeid,"
           "       text(:so) AS type, text(cohead_number) AS ordernumber,"
           "       text('') AS status,"
           "       text('') AS qty "
           "  FROM cohead "
           " WHERE (cohead_prj_id=:prj_id) ";

    sql += " UNION "
           "SELECT quhead_id AS id, 2 AS typeid,"
           "       text(:quote) AS type, text(quhead_number) AS ordernumber,"
           "       text('') AS status,"
           "       text('') AS qty "
           "  FROM quhead "
           " WHERE (quhead_prj_id=:prj_id) ";

    sql += " UNION "
           "SELECT invchead_id AS id, 3 AS typeid,"
           "       text(:invoice) AS type, text(invchead_invcnumber) AS ordernumber,"
           "       text('') AS status,"
           "       text('') AS qty "
           "  FROM invchead "
           " WHERE (invchead_prj_id=:prj_id) ";
  }

  if(_showWo->isChecked())
  {
    if(_showSo->isChecked())
      sql += " UNION ";

    sql += "SELECT wo_id AS id, 4 AS typeid,"
           "       text(:wo) AS type, formatWoNumber(wo_id) AS ordernumber,"
           "       wo_status AS status,"
           "       formatQty(wo_qtyord) AS qty "
           "  FROM wo "
           " WHERE (wo_prj_id=:prj_id) ";
  }

  if(_showPo->isChecked())
  {
    if(_showSo->isChecked() || _showWo->isChecked())
      sql += " UNION ";

    sql += "SELECT poitem_id AS id, 5 AS typeid,"
           "       text(:po) AS type, (text(pohead_number) || '-' || text(poitem_linenumber)) AS ordernumber,"
           "       poitem_status AS status,"
           "       formatQty(poitem_qty_ordered) AS qty "
           "  FROM pohead, poitem "
           " WHERE ((poitem_pohead_id=pohead_id) "
           "   AND  (poitem_prj_id=:prj_id)) ";

    sql += " UNION "
           "SELECT pr_id AS id, 6 AS typeid,"
           "       text(:pr) AS type, text(pr_number) AS ordernumber,"
           "       pr_status AS status,"
           "       formatQty(pr_qtyreq) AS qty "
           "  FROM pr "
           " WHERE (pr_prj_id=:prj_id) ";
  }

  sql += " ORDER BY ordernumber; ";

  q.prepare(sql);
  q.bindValue(":prj_id", _project->id());
  q.bindValue(":so", tr("S/O"));
  q.bindValue(":wo", tr("W/O"));
  q.bindValue(":po", tr("P/O"));
  q.bindValue(":pr", tr("P/R"));
  q.bindValue(":quote", tr("Quote"));
  q.bindValue(":invoice", tr("Invoice"));
  q.exec();

  _orders->populate(q, true);
}

void dspOrderActivityByProject::sPrint()
{
  ParameterList params;

  params.append("prj_id", _project->id());

  if (_showSo->isChecked())
    params.append("showSo");

  if (_showWo->isChecked())
    params.append("showWo");

  if (_showPo->isChecked())
    params.append("showPo");

  orReport report("OrderActivityByProject", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

