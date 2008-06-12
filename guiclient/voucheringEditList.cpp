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

#include "voucheringEditList.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "voucher.h"
#include "miscVoucher.h"

voucheringEditList::voucheringEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_vo, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _vo->setRootIsDecorated(TRUE);
  _vo->addColumn(tr("Vchr. #"), _orderColumn + _vo->indentation(), Qt::AlignRight, true, "vouchernumber"  );
  _vo->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight, true, "ponumber");
  _vo->addColumn(tr("Invc./Item #"), _itemColumn, Qt::AlignRight, true, "itemnumber");
  _vo->addColumn(tr("Vendor #"),     _itemColumn, Qt::AlignRight, true, "vendnumber");
  _vo->addColumn(tr("Name/Description"),      -1, Qt::AlignLeft,  true, "description");
  _vo->addColumn(tr("Vend. Type"),   _itemColumn, Qt::AlignLeft,  true, "itemtype");
  _vo->addColumn(tr("UOM"),           _uomColumn, Qt::AlignCenter,true, "iteminvuom");
  _vo->addColumn(tr("Qty. Vchrd."),   _qtyColumn, Qt::AlignRight, true, "f_qty");
  _vo->addColumn(tr("Cost"),        _moneyColumn, Qt::AlignRight, true, "cost");

  connect(omfgThis, SIGNAL(vouchersUpdated()), this, SLOT(sFillList()));

  sFillList();
}

voucheringEditList::~voucheringEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void voucheringEditList::languageChange()
{
  retranslateUi(this);
}

void voucheringEditList::sPrint()
{
  orReport report("VoucheringEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void voucheringEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  if (_vo->altId() == -1)
  {
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainVouchers"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  }
}

void voucheringEditList::sEdit()
{
  q.prepare( "SELECT vohead_pohead_id "
             "FROM vohead "
             "WHERE (vohead_id=:vohead_id);" );
  q.bindValue(":vohead_id", _vo->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("vohead_id", _vo->id());

    if (q.value("vohead_pohead_id").toInt() == -1)
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
}

void voucheringEditList::sView()
{
  q.prepare( "SELECT vohead_pohead_id "
             "FROM vohead "
             "WHERE (vohead_id=:vohead_id);" );
  q.bindValue(":vohead_id", _vo->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("vohead_id", _vo->id());

    if (q.value("vohead_pohead_id").toInt() == -1)
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
}

void voucheringEditList::sFillList()
{
  /* indent: order
                line item
                  credit account
                line item
                  credit account
                debit account for entire order
  */
  q.prepare("SELECT orderid, seq,"
            "       CASE WHEN seq = 0 THEN vouchernumber"
            "            ELSE ''"
            "       END AS vouchernumber, ponumber, itemnumber,"
            "       vendnumber, description, itemtype, iteminvuom, f_qty, cost,"
            "       'curr' AS cost_xtnumericrole,"
            "       seq AS cost_xttotalrole,"
            "       CASE WHEN seq = 3 THEN 1"
            "            ELSE seq END AS xtindentrole,"
            "       CASE WHEN findAPAccount(vendid) < 0 THEN 'error'"
            "       END AS qtforegroundrole "
            "FROM (SELECT orderid,"
            "       CASE WHEN length(ponumber) > 0 THEN 0 ELSE 1 END AS seq,"
            "       vouchernumber, ponumber,"
            "       CASE WHEN (itemid = 1) THEN invoicenumber"
            "            ELSE itemnumber END AS itemnumber,"
            "       CASE WHEN (itemid = 1) THEN itemnumber"
            "            ELSE ''         END AS vendnumber,"
            "       vendid, description,"
            "       itemtype, iteminvuom, f_qty, cost "
            "FROM voucheringEditList "
            "UNION "    // pull out the credits
            "SELECT DISTINCT orderid, 2 AS seq, vouchernumber, '' AS ponumber,"
            "       :credit AS itemnumber, '' AS vendnumber, vendid,"
            "       account AS description,"
            "       '' AS itemtype, '' AS iteminvuom, NULL as f_qty, cost "
            "FROM voucheringEditList "
            "WHERE itemid = 2 "
            "UNION "    // calculate the debits
            "SELECT orderid, 3 AS seq, vouchernumber, '' AS ponumber,"
            "       :debit AS itemnumber, '' AS vendnumber, vendid,"
            "       CASE WHEN findAPAccount(vendid) < 0 THEN :notassigned"
            "            ELSE formatGLAccountLong(findAPAccount(vendid))"
            "       END AS description,"
            "       '' AS itemtype, '' AS iteminvuom, NULL as f_qty,"
            "       SUM(cost) AS cost "
            "FROM voucheringEditList "
            "WHERE itemid = 2 "
            "GROUP BY orderid, vouchernumber, vendid "
            "ORDER BY vouchernumber, ponumber desc, seq) AS sub;");
  q.bindValue(":credit",      tr("Credit"));
  q.bindValue(":debit",       tr("Debit"));
  q.bindValue(":notassigned", tr("Not Assigned"));
  q.exec();
  _vo->populate(q);
  // TODO: implement indentation

  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
