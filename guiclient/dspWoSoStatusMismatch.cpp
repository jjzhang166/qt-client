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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "dspWoSoStatusMismatch.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <parameter.h>
#include "closeWo.h"
#include "rptWoSoStatusMismatch.h"
#include "dspWoMaterialsByWorkOrder.h"

/*
 *  Constructs a dspWoSoStatusMismatch as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoSoStatusMismatch::dspWoSoStatusMismatch(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoSoStatusMismatch::~dspWoSoStatusMismatch()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoSoStatusMismatch::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void dspWoSoStatusMismatch::init()
{
  statusBar()->hide();

  _wo->addColumn(tr("W/O #"),       _orderColumn,  Qt::AlignLeft   );
  _wo->addColumn(tr("Status"),      _statusColumn, Qt::AlignCenter );
  _wo->addColumn(tr("Item Number"), -1,            Qt::AlignLeft   );
  _wo->addColumn(tr("UOM"),         _uomColumn,    Qt::AlignLeft   );
  _wo->addColumn(tr("Whs."),        _whsColumn,    Qt::AlignCenter );
  _wo->addColumn(tr("S/O #"),       _orderColumn,  Qt::AlignLeft   );
  _wo->addColumn(tr("Ordered"),     _qtyColumn,    Qt::AlignRight  );
  _wo->addColumn(tr("Received"),    _qtyColumn,    Qt::AlignRight  );
  _wo->addColumn(tr("Start Date"),  _dateColumn,   Qt::AlignCenter );
  _wo->addColumn(tr("Due Date"),    _dateColumn,   Qt::AlignCenter );

  sFillList();
}

void dspWoSoStatusMismatch::sPrint()
{
  ParameterList params;
  params.append("print");
  _warehouse->appendValue(params);

  rptWoSoStatusMismatch newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspWoSoStatusMismatch::sCloseWo()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  closeWo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void dspWoSoStatusMismatch::sPopulateMenu(Q3PopupMenu *pMenu, Q3ListViewItem *pSelected)
{
  if ((pSelected->text(1) == "E") || (pSelected->text(1) == "I"))
    pMenu->insertItem(tr("View W/O Material Requirements..."), this, SLOT(sViewWomatlreq()), 0);

  pMenu->insertItem(tr("Close W/O..."), this, SLOT(sCloseWo()), 0);
}

void dspWoSoStatusMismatch::sViewWomatlreq()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoSoStatusMismatch::sFillList()
{
  QString sql( "SELECT wo_id,"
               "       formatWONumber(wo_id) AS wonumber,"
               "       wo_status, item_number, item_invuom,"
               "       warehous_code,"
               "       cohead_number,"
               "       formatQty(wo_qtyord) AS qtyord,"
               "       formatQty(wo_qtyrcv) AS qtyrcv,"
               "       formatDate(wo_startdate) AS startdate,"
               "       formatDate(wo_duedate) AS duedate "
               "FROM coitem, cohead, wo, itemsite, warehous, item "
               "WHERE ( (coitem_cohead_id=cohead_id)"
               " AND (coitem_order_id=wo_id)"
               " AND (coitem_status='C')"
               " AND (wo_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (wo_status IN ('O','E','S','R','I') )" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY wo_duedate";

  q.prepare(sql);
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  _wo->populate(q);
}

