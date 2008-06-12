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

#include "billingEditList.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>

#include "printInvoices.h"
#include "selectBillingQty.h"
#include "selectOrderForBilling.h"
#include "storedProcErrorLookup.h"

billingEditList::billingEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_cobill, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _cobill->setRootIsDecorated(TRUE);
  _cobill->addColumn(tr("Document #"), (_itemColumn+ _cobill->indentation()),  Qt::AlignLeft, true, "documentnumber");
  _cobill->addColumn(tr("Order #"),    _orderColumn, Qt::AlignLeft,  true, "ordernumber");
  _cobill->addColumn(tr("Cust./Item #"),_itemColumn, Qt::AlignLeft,  true, "shortname");
  _cobill->addColumn(tr("Name/Description"),     -1, Qt::AlignLeft,  true, "longname");
  _cobill->addColumn(tr("UOM"),          _uomColumn, Qt::AlignCenter,true, "iteminvuom");
  _cobill->addColumn(tr("Qty. to Bill"), _qtyColumn, Qt::AlignRight, true, "qtytobill");
  _cobill->addColumn(tr("Price"),       _costColumn, Qt::AlignRight, true, "price");
  _cobill->addColumn(tr("Ext. Price"), _moneyColumn, Qt::AlignRight, true, "extprice");
  _cobill->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currabbr");

  connect(omfgThis, SIGNAL(billingSelectionUpdated(int, int)), this, SLOT(sFillList()));

  sFillList();
}

billingEditList::~billingEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void billingEditList::languageChange()
{
  retranslateUi(this);
}

void billingEditList::sEditBillingOrd()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", _cobill->id());

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void billingEditList::sCancelBillingOrd()
{
  if ( QMessageBox::critical( this, tr("Cancel Billing"),
                              tr("Are you sure that you want to cancel billing for the selected order?"),
                              tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare( "SELECT cancelBillingSelection(cobmisc_id) AS result "
               "FROM cobmisc "
               "WHERE (cobmisc_cohead_id=:sohead_id);" );
    q.bindValue(":sohead_id", _cobill->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("cancelBillingSelection", result), __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void billingEditList::sEditBillingQty()
{ 
  ParameterList params;
  params.append("soitem_id", _cobill->altId());

  selectBillingQty newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void billingEditList::sCancelBillingQty()
{
}

void billingEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  _orderid = _cobill->id();

  if (((XTreeWidgetItem *)pSelected)->altId() == -2)
  {
    _itemtype = 1;
    pMenu->insertItem("Edit Billing...", this, SLOT(sEditBillingOrd()), 0);
    pMenu->insertItem("Cancel Billing...", this, SLOT(sCancelBillingOrd()), 0);
  }
  else if (((XTreeWidgetItem *)pSelected)->altId() != -1)
  {
    _itemtype = 2;
    pMenu->insertItem("Edit Billing...", this, SLOT(sEditBillingQty()), 0);
    pMenu->insertItem("Cancel Billing...", this, SLOT(sCancelBillingQty()), 0);
  }
}

void billingEditList::sFillList()
{
  /* indent: order
                line item
                  credit account
                line item
                  credit account
                debit account for entire order
  */
  q.exec("SELECT orderid, itemid, seq, documentnumber, ordernumber,"
         "       shortname, longname, iteminvuom, qtytobill, price, extprice,"
         "       currConcat(curr_id) AS currabbr,"
         "       'qty'        AS qtytobill_xtnumericrole,"
         "       'salesprice' AS price_xtnumericrole,"
         "       'extprice'   AS extprice_xtnumericrole,"
         "       CASE WHEN seq = 3 THEN 1"
         "            ELSE seq END AS xtindentrole,"
         "       CASE WHEN account = 'Not Assigned' THEN 'error'"
         "       END AS qtforegroundrole "
         "FROM ("
         "  SELECT orderid, itemid, 0 AS seq,"      // order info
         "        documentnumber, ordernumber, cust_number AS shortname,"
         "        billtoname AS longname, '' AS iteminvuom, NULL AS qtytobill,"
         "        NULL AS price, NULL AS extprice, account, NULL AS curr_id,"
         "        ordernumber AS sortord, linenumber "
         "  FROM billingEditList "
         "  WHERE (itemid = -2) "
         "  UNION "       // line item info
         "  SELECT orderid, itemid, 1,"
         "        documentnumber, '', item,"
         "        itemdescrip, iteminvuom, qtytobill, price, extprice, account, curr_id,"
         "        ordernumber, linenumber "
         "  FROM billingEditList "
         "  WHERE (itemid >= 0) "
         "  UNION "       // credits
         "  SELECT orderid, itemid, 2,"
         "         documentnumber, '', sence,"
         "         account, '', NULL, NULL, extprice, account, curr_id,"
         "        ordernumber, linenumber "
         "  FROM billingEditList "
         "  WHERE (itemid >= 0) "
         "  UNION "       // debits
         "  SELECT orderid, itemid, 3,"
         "         '', '', sence,"
         "         account, '', NULL, NULL, extprice, account, curr_id,"
         "        ordernumber, 9999999999 "
         "  FROM billingEditList "
         "  WHERE (itemid = -2) "
         ") AS sub "
         "ORDER BY sortord, linenumber, seq;");
  _cobill->populate(q, true);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void billingEditList::sPrint()
{
  orReport report("BillingEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
