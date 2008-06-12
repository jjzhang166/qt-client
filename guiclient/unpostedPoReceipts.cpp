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

// TODO: rename unpostedReceipts
#include "unpostedPoReceipts.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "distributeInventory.h"
#include "enterPoReceipt.h"
#include "enterPoitemReceipt.h"
#include "failedPostList.h"
#include "getGLDistDate.h"
#include "mqlutil.h"
#include "purchaseOrderItem.h"
#include "storedProcErrorLookup.h"
#include "transferOrderItem.h"

#define RECV_ORDER_TYPE_COL	1
#define RECV_QTY_COL		11
#define RECV_DATE_COL		12
#define RECV_GLDISTDATE_COL	13

unpostedPoReceipts::unpostedPoReceipts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_delete,        SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,          SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,	          SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_post,          SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_print,         SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_recv, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_viewOrderItem,    SIGNAL(clicked()), this, SLOT(sViewOrderItem()));
  connect(omfgThis, SIGNAL(purchaseOrderReceiptsUpdated()), this, SLOT(sFillList()));

  _recv->addColumn(tr("Order #"),       _orderColumn, Qt::AlignRight  );
  _recv->addColumn(tr("Type"),          50,           Qt::AlignCenter );
  _recv->addColumn(tr("From"),          -1,           Qt::AlignLeft   );
  _recv->addColumn(tr("Line #"),        50,           Qt::AlignRight  );
  _recv->addColumn(tr("Due Date"),      _dateColumn,  Qt::AlignCenter );
  _recv->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignRight  );
  _recv->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter );
  _recv->addColumn(tr("Vend. Item #"),  _itemColumn,  Qt::AlignLeft   );
  _recv->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter );
  _recv->addColumn(tr("Ordered"),       _qtyColumn,   Qt::AlignRight  );
  _recv->addColumn(tr("Received"),      _qtyColumn,   Qt::AlignRight  );
  _recv->addColumn(tr("To Receive"),    _qtyColumn,   Qt::AlignRight  );
  _recv->addColumn(tr("Receipt Date"),  _dateColumn,  Qt::AlignCenter );
  _recv->addColumn(tr("G/L Post Date"), _dateColumn,  Qt::AlignCenter );

  if (! _privileges->check("ChangePORecvPostDate"))
    _recv->hideColumn(RECV_GLDISTDATE_COL);

  if(!_privileges->check("ViewPurchaseOrders"))
    disconnect(_recv, SIGNAL(valid(bool)), _viewOrderItem, SLOT(setEnabled(bool)));

  sFillList();
}

unpostedPoReceipts::~unpostedPoReceipts()
{
    // no need to delete child widgets, Qt does it all for us
}

void unpostedPoReceipts::languageChange()
{
  retranslateUi(this);
}

void unpostedPoReceipts::setParams(ParameterList & params)
{
  params.append("nonInventory",	tr("Non-Inventory"));
  params.append("na",		tr("N/A"));
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
}

void unpostedPoReceipts::sPrint()
{
  ParameterList params;
  setParams(params);
  orReport report("UnpostedPoReceipts", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void unpostedPoReceipts::sNew()
{
  ParameterList params;

  enterPoReceipt *newdlg = new enterPoReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedPoReceipts::sEdit()
{
  ParameterList params;
  params.append("mode",		"edit");
  params.append("recv_id",	_recv->id());

  enterPoitemReceipt *newdlg = new enterPoitemReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedPoReceipts::sDelete()
{
  if (QMessageBox::question(this, tr("Cancel Receipts?"),
			    tr("<p>Are you sure you want to delete these "
			       "unposted Receipts?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM recv "
	       "WHERE (recv_id IN (:id));" );
    QList<QTreeWidgetItem*>selected = _recv->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      q.bindValue(":id", ((XTreeWidgetItem*)(selected[i]))->id() );
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    omfgThis->sPurchaseOrderReceiptsUpdated();
  }
}

void unpostedPoReceipts::sViewOrderItem()
{
  ParameterList params;
  params.append("mode",		"view");
  if (_recv->currentItem()->text(RECV_ORDER_TYPE_COL) == "PO")
  {
    params.append("poitem_id",	_recv->altId());
    purchaseOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (_recv->currentItem()->text(RECV_ORDER_TYPE_COL) == "TO")
  {
    params.append("toitem_id",	_recv->altId());
    transferOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void unpostedPoReceipts::sPost()
{
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (_privileges->check("ChangePORecvPostDate"))
  {
    getGLDistDate newdlg(this, "", TRUE);
    newdlg.sSetDefaultLit(tr("Receipt Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  XSqlQuery setDate;
  setDate.prepare("UPDATE recv SET recv_gldistdate=:distdate "
		  "WHERE recv_id=:recv_id;");

  QList<QTreeWidgetItem*>selected = _recv->selectedItems();
  QList<QTreeWidgetItem*>triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    int id = ((XTreeWidgetItem*)(selected[i]))->id();

    if (changeDate)
    {
      setDate.bindValue(":distdate",  newDate);
      setDate.bindValue(":recv_id", id);
      setDate.exec();
      if (setDate.lastError().type() != QSqlError::None)
      {
        systemError(this, setDate.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
  }
  
  XSqlQuery postLine;
  postLine.prepare("SELECT postReceipt(:id, NULL::integer) AS result;");
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();

      q.exec("BEGIN;");
      postLine.bindValue(":id", id);
      postLine.exec();
      if (postLine.first())
      {
        int result = postLine.value("result").toInt();
        if (result < 0)
        {
          rollback.exec();
          systemError(this, storedProcErrorLookup("postReceipt", result),
              __FILE__, __LINE__);
          continue;
        }

        if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
        {
          QMessageBox::information( this, tr("Unposted Receipts"), tr("Post Canceled") );
          rollback.exec();
          return;
        }
        q.exec("COMMIT;");
         
      }
      // contains() string is hard-coded in stored procedure
      else if (postLine.lastError().databaseText().contains("posted to closed period"))
      {
        if (changeDate)
        {
          triedToClosed = selected;
          break;
        }
        else
          triedToClosed.append(selected[i]);
      }
      else if (postLine.lastError().type() != QSqlError::None)
      {
        rollback.exec();
        systemError(this, postLine.lastError().databaseText(), __FILE__, __LINE__);
      }
    } // for each selected line

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, _recv->headerItem(), _recv->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
    }
  } while (tryagain);

  omfgThis->sPurchaseOrderReceiptsUpdated();
}

void unpostedPoReceipts::sPopulateMenu(QMenu *pMenu,QTreeWidgetItem *pItem)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Receipt..."),	this, SLOT(sEdit()));
  menuItem = pMenu->insertItem(tr("Delete Receipt..."),	this, SLOT(sDelete()));

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Post Receipt..."),	this, SLOT(sPost()));

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("View Order Item..."),this, SLOT(sViewOrderItem()));
  pMenu->setItemEnabled(menuItem, ((pItem->text(RECV_ORDER_TYPE_COL) == "PO" &&
				    _privileges->check("ViewPurchaseOrders")) ||
				   (pItem->text(RECV_ORDER_TYPE_COL) == "TO" &&
				    _privileges->check("ViewTransferOrders"))) );
}

void unpostedPoReceipts::sFillList()
{
  ParameterList fillp;
  setParams(fillp);
  MetaSQLQuery fillm = mqlLoad(":/sr/unpostedReceipts/FillListDetail.mql");
  XSqlQuery fillq = fillm.toQuery(fillp);

  _recv->clear();

  XTreeWidgetItem *line = 0;
  while (fillq.next())
  {
    line = new XTreeWidgetItem(_recv, line,
			       fillq.value("recv_id").toInt(),
			       fillq.value("recv_orderitem_id").toInt(),
			       fillq.value("recv_order_number"),
			       fillq.value("recv_order_type"),
			       fillq.value("orderhead_from"),
			       fillq.value("orderitem_linenumber"),
			       fillq.value("recv_duedate"),
			       fillq.value("item_number"),
			       fillq.value("uom_name"),
			       fillq.value("recv_vend_item_number"),
			       fillq.value("recv_vend_uom"),
			       fillq.value("qty_ordered"),
			       fillq.value("qty_received") );
    line->setText(RECV_QTY_COL, fillq.value("recv_qty").toString());
    line->setText(RECV_DATE_COL, fillq.value("recv_date").toString());
    line->setText(RECV_GLDISTDATE_COL, fillq.value("recv_gldistdate").toString());
  }
  if (fillq.lastError().type() != QSqlError::None)
  {
    systemError(this, fillq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
