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

#include "unpostedPurchaseOrders.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>
#include <QWorkspace>

#include <metasql.h>
#include <parameter.h>

#include "purchaseOrder.h"
#include "printPurchaseOrder.h"
#include "guiclient.h"
#include "deliverPurchaseOrder.h"
#include "storedProcErrorLookup.h"

unpostedPurchaseOrders::unpostedPurchaseOrders(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    connect(_allOpenOrders,SIGNAL(clicked()),	this,	SLOT(sFillList()));
    connect(_delete,	SIGNAL(clicked()),	this,	SLOT(sDelete()));
    connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
    connect(_new,	SIGNAL(clicked()),	this,	SLOT(sNew()));
    connect(_pohead,	SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)),
						this,	SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem *)));
    connect(_pohead,	SIGNAL(valid(bool)),	this,	SLOT(sHandleButtons()));
    connect(_post,	SIGNAL(clicked()),	this,	SLOT(sPost()));
    connect(_print,	SIGNAL(clicked()),	this,	SLOT(sPrint()));
    connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));

    connect(omfgThis,	SIGNAL(purchaseOrdersUpdated(int, bool)),
						this,	SLOT(sFillList()));

    statusBar()->hide();
    
    _pohead->addColumn(tr("Number"),    _orderColumn, Qt::AlignCenter );
    _pohead->addColumn(tr("Vendor"),    -1,           Qt::AlignLeft   );
    _pohead->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignCenter );
    _pohead->addColumn(tr("Status"),    _ynColumn,    Qt::AlignCenter );

    _pohead->setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (_privileges->check("MaintainPurchaseOrders"))
      _new->setEnabled(TRUE);

    sFillList();
}

unpostedPurchaseOrders::~unpostedPurchaseOrders()
{
  // no need to delete child widgets, Qt does it all for us
}

void unpostedPurchaseOrders::languageChange()
{
    retranslateUi(this);
}

void unpostedPurchaseOrders::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedPurchaseOrders::sEdit()
{
  QList<QTreeWidgetItem*> list = _pohead->selectedItems();
  bool done = false;
  for (int i = 0; i < list.size(); i++)
  {
    if ((list[i]->text(3) == "U" && _privileges->check("MaintainPurchaseOrders")) ||
	(list[i]->text(3) == "O" && _privileges->check("MaintainPostedPurchaseOrders")))
    {
      ParameterList params;
      params.append("mode", "edit");
      params.append("pohead_id", ((XTreeWidgetItem*)(list[i]))->id());
  
      purchaseOrder *newdlg = new purchaseOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      done = true;
      break;
    }
  }
  if (done)
    omfgThis->sPurchaseOrdersUpdated(-1, TRUE);
  else
    QMessageBox::information(this, tr("Nothing To Edit"),
			     tr("<p>There were no selected Purchase Orders "
				"that you could edit."),
			     QMessageBox::Ok|QMessageBox::Default);
}

void unpostedPurchaseOrders::sView()
{
  QList<QTreeWidgetItem*> list = _pohead->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("pohead_id", ((XTreeWidgetItem*)(list[i]))->id());
  
    purchaseOrder *newdlg = new purchaseOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
    break;
  }
}

void unpostedPurchaseOrders::sDelete()
{
  if ( QMessageBox::warning( this, tr("Delete Selected Purchase Orders"),
                             tr("<p>Are you sure that you want to delete the "
			        "selected Purchase Orders?" ),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    q.prepare("SELECT deletePo(:pohead_id) AS result;");

    QList<QTreeWidgetItem*> list = _pohead->selectedItems();
    bool done = false;
    for (int i = 0; i < list.size(); i++)
    {
      if (list[i]->text(3) == "U")
      {
	q.bindValue(":pohead_id", ((XTreeWidgetItem*)(list[i]))->id());
	q.exec();
	if (q.first() && ! q.value("result").toBool())
	    systemError(this, tr("<p>Only Unposted Purchase Orders may be "
				 "deleted. Check the status of Purchase Order "
				 "%1. If it is 'U' then contact your system "
				 "Administrator.").arg(list[i]->text(0)),
			__FILE__, __LINE__);
	else if (q.lastError().type() != QSqlError::None)
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	else
	  done = true;
      }
    }
    if (done)
      omfgThis->sPurchaseOrdersUpdated(-1, TRUE);
    else
      QMessageBox::information(this, tr("Nothing To Delete"),
			       tr("<p>There were no selected Purchase Orders "
				  "that could be deleted."),
			       QMessageBox::Ok|QMessageBox::Default);
  }
}

void unpostedPurchaseOrders::sPrint()
{
  QList<QTreeWidgetItem*> list = _pohead->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
    ParameterList params;
    params.append("pohead_id", ((XTreeWidgetItem*)(list[i]))->id());
  
    printPurchaseOrder newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
    break;
  }
}

void unpostedPurchaseOrders::sDeliver()
{
  QList<QTreeWidgetItem*> list = _pohead->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
    ParameterList params;
    params.append("pohead_id", ((XTreeWidgetItem*)(list[i]))->id());
  
    deliverPurchaseOrder newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
    break;
  }
}

void unpostedPurchaseOrders::sPost()
{
  if ( QMessageBox::warning( this, tr("Post Selected Purchase Orders"),
                             tr("<p>Are you sure that you want to post "
			        "the selected Purchase Orders?" ),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    q.prepare("SELECT postPurchaseOrder(:pohead_id) AS result;");

    QList<QTreeWidgetItem*> list = _pohead->selectedItems();
    bool done = false;
    for (int i = 0; i < list.size(); i++)
    {
      if (list[i]->text(3) == "U" && _privileges->check("PostPurchaseOrders"))
      {
	q.bindValue(":pohead_id", ((XTreeWidgetItem*)(list[i]))->id());
	q.exec();
	if (q.first())
	{
	  int result = q.value("result").toInt();
	  if (result < 0)
	    systemError(this, storedProcErrorLookup("postPurchaseOrder", result),
			__FILE__, __LINE__);
	  else
	    done = true;
	}
	else if (q.lastError().type() != QSqlError::None)
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
    if (done)
      omfgThis->sPurchaseOrdersUpdated(-1, TRUE);
    else
      QMessageBox::information(this, tr("Nothing To Post"),
			       tr("<p>There were no selected Purchase Orders "
				  "to be posted."),
			       QMessageBox::Ok|QMessageBox::Default);
  }
}

void unpostedPurchaseOrders::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem)
{
  int menuItem;

  bool canMaintain = (pItem->text(3) == "U" && _privileges->check("MaintainPurchaseOrders")) ||
		     (pItem->text(3) == "O" && _privileges->check("MaintainPostedPurchaseOrders"));

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  pMenu->setItemEnabled(menuItem, canMaintain);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  pMenu->setItemEnabled(menuItem, canMaintain || _privileges->check("ViewPurchaseOrders"));

  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  pMenu->setItemEnabled(menuItem, (pItem->text(3) == "U" && _privileges->check("MaintainPurchaseOrders")));

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Print..."), this, SLOT(sPrint()), 0);
  pMenu->setItemEnabled(menuItem, canMaintain);

  if (_metrics->boolean("EnableBatchManager"))
  {
    menuItem = pMenu->insertItem(tr("Deliver..."), this, SLOT(sDeliver()), 0);
    pMenu->setItemEnabled(menuItem, canMaintain);
  }

  menuItem = pMenu->insertItem(tr("Post..."), this, SLOT(sPost()), 0);
  pMenu->setItemEnabled(menuItem, _privileges->check("PostPurchaseOrders") &&
				  pItem->text(3) == "U");
}

void unpostedPurchaseOrders::sHandleButtons()
{
  // see if the selected items list contains both unposted and open/posted items
  bool unposted = false;
  bool open = false;
  QList<QTreeWidgetItem*> list = _pohead->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
    if (! unposted && list[i]->text(3) == "U")
      unposted = true;
    if (! open && list[i]->text(3) == "O")
      open = true;
    if (open && unposted)
      break;
  }

  _delete->setEnabled(unposted && _privileges->check("MaintainPurchaseOrders"));
  _edit->setEnabled((unposted && _privileges->check("MaintainPurchaseOrders")) ||
		    (open && _privileges->check("MaintainPostedPurchaseOrders")));
  _post->setEnabled(unposted && _privileges->check("PostPurchaseOrders"));
  _print->setEnabled((unposted && _privileges->check("MaintainPurchaseOrders")) ||
		     (open && _privileges->check("MaintainPostedPurchaseOrders")));

  if ((unposted && _privileges->check("MaintainPurchaseOrders")) ||
      (open && _privileges->check("MaintainPostedPurchaseOrders")))
  {
    disconnect(_pohead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    connect(_pohead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    disconnect(_pohead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_pohead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
}

void unpostedPurchaseOrders::sFillList()
{
  _pohead->clear();

  ParameterList params;
  if (_allOpenOrders->isChecked())
    params.append("showPosted");

  QString sql( "SELECT pohead_id, pohead_number, vend_name,"
               "       formatDate(MIN(poitem_duedate)), "
	       "       pohead_status "
               "FROM vend, pohead LEFT OUTER JOIN "
	       "     poitem ON (poitem_pohead_id=pohead_id) "
               "WHERE ( (pohead_vend_id=vend_id)"
	       "<? if exists(\"showPosted\") ?> "
               "  AND (pohead_status IN ('U', 'O') ) "
	       "  AND (pohead_id NOT IN (SELECT vohead_pohead_id FROM vohead WHERE vohead_pohead_id IS NOT NULL))"
	       "<? else ?> "
               "  AND (pohead_status='U') "
	       "<? endif ?> "
	       ") "
               "GROUP BY pohead_number, pohead_id, vend_name, pohead_status "
	       "ORDER BY pohead_number;" );
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _pohead->populate(q);
}
