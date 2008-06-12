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

#include "creditMemoEditList.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <openreports.h>

#include "selectOrderForBilling.h"
#include "selectBillingQty.h"
#include "printInvoices.h"
#include "creditMemo.h"
#include "creditMemoItem.h"

creditMemoEditList::creditMemoEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_cmhead, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _cmhead->setRootIsDecorated(TRUE);
  _cmhead->addColumn(tr("Document #"),       (_itemColumn+ _cmhead->indentation()),  Qt::AlignLeft   );
  _cmhead->addColumn(tr("Order #"),          _orderColumn,                            Qt::AlignLeft   );
  _cmhead->addColumn(tr("Cust./Item #"),     _itemColumn,                             Qt::AlignLeft   );
  _cmhead->addColumn(tr("Name/Description"), -1,                                      Qt::AlignLeft   );
  _cmhead->addColumn(tr("UOM"),              _uomColumn,                              Qt::AlignCenter );
  _cmhead->addColumn(tr("Qty. to Bill"),     _qtyColumn,                              Qt::AlignRight  );
  _cmhead->addColumn(tr("Price"),            _costColumn,                             Qt::AlignRight  );
  _cmhead->addColumn(tr("Ext. Price"),       _moneyColumn,                            Qt::AlignRight  );

  connect(omfgThis, SIGNAL(creditMemosUpdated()), this, SLOT(sFillList()));

  sFillList();
}

creditMemoEditList::~creditMemoEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void creditMemoEditList::languageChange()
{
  retranslateUi(this);
}

void creditMemoEditList::sEditCreditMemo()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmhead_id", _cmhead->id());

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void creditMemoEditList::sEditCreditMemoItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmitem_id", _cmhead->altId());

  creditMemoItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void creditMemoEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  _orderid = _cmhead->id();

  pMenu->insertItem("Edit Credit Memo...", this, SLOT(sEditCreditMemo()), 0);

  if (((XTreeWidgetItem *)pSelected)->altId() != -1)
    pMenu->insertItem("Edit Credit Memo Item...", this, SLOT(sEditCreditMemoItem()), 0);
}

void creditMemoEditList::sFillList()
{
  _cmhead->clear();

  q.exec("SELECT * FROM creditMemoEditList;");
  if (q.first())
  {
    int           thisOrderid;
    int           orderid    = -9999;
    XTreeWidgetItem *orderLine = NULL;
    XTreeWidgetItem *lastLine  = NULL;
    XTreeWidgetItem *selected  = NULL;

    //  Fill the list with the query contents
    do
    {
      thisOrderid = q.value("orderid").toInt();

//  Check to see if this a new order number
      if (thisOrderid != orderid)
      {
//  New order number, make a new list item header
        orderid  = thisOrderid;
        lastLine = NULL;

        XTreeWidgetItem *thisLine = new XTreeWidgetItem( _cmhead, orderLine, q.value("orderid").toInt(), -1,
                                                     q.value("documentnumber"), q.value("ordernumber"),
                                                     q.value("cust_number"), q.value("billtoname") );
        orderLine = thisLine;

//  Add the distribution line
        thisLine = new XTreeWidgetItem( orderLine, q.value("orderid").toInt(), -1,
                                      "", "",
                                      q.value("sence"), q.value("account"),
                                      "", "", "", q.value("extprice") );
        if (q.value("account") == "Not Assigned")
        {
          thisLine->setTextColor("red");
          orderLine->setTextColor("red");
        }

//  If we are looking for a selected order and this is it, cache it
        if (thisOrderid == _orderid)
          selected = orderLine;
      }
      else
      {
        XTreeWidgetItem *itemLine = new XTreeWidgetItem( orderLine, lastLine, q.value("orderid").toInt(), q.value("itemid").toInt(),
                                                     "", "",
                                                     q.value("item"), q.value("itemdescrip"),
                                                     q.value("iteminvuom"), q.value("qtytobill"),
                                                     q.value("price"), q.value("extprice") );

//  Add the distribution line
        XTreeWidgetItem *thisLine = new XTreeWidgetItem( itemLine, q.value("orderid").toInt(), -1,
                                                     "", "",
                                                     q.value("sence"), q.value("account"),
                                                     "", "", "", q.value("extprice") );
        if (q.value("account") == "Not Assigned")
        {
          thisLine->setTextColor("red");
          itemLine->setTextColor("red");
          orderLine->setTextColor("red");
        }

        lastLine = itemLine;
      }
    }
    while (q.next());

//  Select and show the select item, if any
    if (selected != NULL)
    {
      _cmhead->setCurrentItem(selected);
      _cmhead->scrollToItem(selected);
    }
  }
}

void creditMemoEditList::sPrint()
{
  orReport report("CreditMemoEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
