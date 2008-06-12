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

#include "transferOrders.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "copyTransferOrder.h"
#include "issueToShipping.h"
#include "storedProcErrorLookup.h"
#include "transferOrder.h"
#include "printPackingList.h"

transferOrders::transferOrders(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_copy,	  SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_delete,	  SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_destWarehouse, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_edit,	  SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,		  SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,	  SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_issue,	  SIGNAL(clicked()), this, SLOT(sIssue()));
  connect(_srcWarehouse,  SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_to, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view,	  SIGNAL(clicked()), this, SLOT(sView()));
  connect(omfgThis, SIGNAL(transferOrdersUpdated(int)), this, SLOT(sFillList()));

  _to->addColumn(tr("T/O #"),            _orderColumn, Qt::AlignLeft   );
  _to->addColumn(tr("Source Whs."),	 _whsColumn,   Qt::AlignLeft   );
  _to->addColumn(tr("Dest. Whs."),	 _whsColumn,   Qt::AlignLeft   );
  _to->addColumn(tr("Ordered"),          _dateColumn,  Qt::AlignCenter );
  _to->addColumn(tr("Scheduled"),        _dateColumn,  Qt::AlignCenter );
  
  if (_privileges->check("MaintainTransferOrders"))
  {
    connect(_to,       SIGNAL(valid(bool)), _edit,    SLOT(setEnabled(bool)));
    connect(_to,       SIGNAL(valid(bool)), _copy,    SLOT(setEnabled(bool)));
    connect(_to,       SIGNAL(valid(bool)), _delete,  SLOT(setEnabled(bool)));
    connect(_to,       SIGNAL(valid(bool)), _issue,   SLOT(setEnabled(bool)));
    connect(_to, SIGNAL(itemSelected(int)), _edit,    SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_to, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _destWarehouse->setAll();

  sFillList();
}

transferOrders::~transferOrders()
{
    // no need to delete child widgets, Qt does it all for us
}

void transferOrders::languageChange()
{
    retranslateUi(this);
}

void transferOrders::setParams(ParameterList& params)
{
  if (_srcWarehouse->isSelected())
    params.append("src_warehous_id", _srcWarehouse->id());
  if (_destWarehouse->isSelected())
    params.append("dest_warehous_id", _destWarehouse->id());
  params.append("tohead_status", "O");
}

void transferOrders::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("ListTransferOrders", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void transferOrders::sNew()
{
  transferOrder::newTransferOrder(
		  (_srcWarehouse->isSelected()  ? _srcWarehouse->id()  : -1),
		  (_destWarehouse->isSelected() ? _destWarehouse->id() : -1));
}

void transferOrders::sEdit()
{
  transferOrder::editTransferOrder(_to->id(), false);
}

void transferOrders::sView()
{
  transferOrder::viewTransferOrder(_to->id());
}

void transferOrders::sCopy()
{
  ParameterList params;
  params.append("tohead_id", _to->id());
      
  copyTransferOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void transferOrders::sIssue()
{
  ParameterList params;
  params.append("tohead_id", _to->id());

  issueToShipping *newdlg = new issueToShipping();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void transferOrders::sDelete()
{
  if ( QMessageBox::question(this, tr("Delete Transfer Order?"),
                             tr("<p>Are you sure that you want to completely "
				"delete the selected Transfer Order?"),
			     QMessageBox::Yes,
			     QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteTo(:tohead_id) AS result;");
    q.bindValue(":tohead_id", _to->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result == -1)
      {
	if (QMessageBox::question(this, tr("Cannot Delete Transfer Order"),
				  tr("<p>The selected Transfer Order cannot be "
				     "deleted as there have been shipments "
				     "posted against it. Would you like to "
				     "Close it instead?"),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
	  return;

	q.prepare( "SELECT closeTransferOrder(:tohead_id) AS result;");
	q.bindValue(":tohead_id", _to->id());
	q.exec();
	if (q.first())
	{
	  int result = q.value("result").toInt();
	  if (result < 0)
	  {
	    systemError(this, storedProcErrorLookup("closeTransferOrder", result), __FILE__, __LINE__);
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
      else if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteTo", result),
		    __FILE__, __LINE__);
	return;
      }
      omfgThis->sTransferOrdersUpdated(-1);
      omfgThis->sProjectsUpdated(-1);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void transferOrders::sPrintPackingList()
{
  ParameterList params;
  params.append("tohead_id", _to->id());
  params.append("print");

  printPackingList newdlg(this, "", TRUE);
  if (newdlg.set(params) != NoError_Print)
    newdlg.exec();
}

void transferOrders::sAddToPackingListBatch()
{
  q.prepare("SELECT addToPackingListBatch('TO', :tohead_id) AS result;");
  q.bindValue(":tohead_id", _to->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("addToPackingListBatch", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void transferOrders::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainTransferOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainTransferOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Issue To Shipping..."), this, SLOT(sIssue()), 0);
  if (!_privileges->check("IssueStockToShipping"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Copy..."), this, SLOT(sCopy()), 0);
  if (!_privileges->check("MaintainTransferOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Print Packing List..."), this, SLOT(sPrintPackingList()), 0);
  if (!_privileges->check("PrintPackingLists"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Add to Packing List Batch..."), this, SLOT(sAddToPackingListBatch()), 0);
  if (!_privileges->check("MaintainPackingListBatch"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void transferOrders::sFillList()
{
  QString sql( "SELECT DISTINCT tohead_id, tohead_number,"
               "       (SELECT warehous_code FROM whsinfo"
	       "        WHERE (warehous_id=tohead_src_warehous_id)) AS srcWhs,"
	       "       (SELECT warehous_code FROM whsinfo"
	       "        WHERE (warehous_id=tohead_dest_warehous_id)) AS destWhs,"
               "       formatDate(tohead_orderdate) AS f_ordered,"
               "       formatDate(MIN(toitem_schedshipdate)) AS f_scheduled "
               "FROM tohead LEFT OUTER JOIN toitem ON (tohead_id=toitem_tohead_id)"
               "WHERE ("
	       "<? if exists(\"tohead_status\") ?>"
	       " (tohead_status = <? value(\"tohead_status\") ?>)"
	       "<? else ?>"
	       "   true"
	       "<? endif ?>"
	       "<? if exists(\"src_warehous_id\") ?>"
	       "  AND  (tohead_src_warehous_id=<? value(\"src_warehous_id\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"dest_warehous_id\") ?>"
	       "  AND  (tohead_dest_warehous_id=<? value(\"dest_warehous_id\") ?>)"
	       "<? endif ?>"
	       " ) "
	       "GROUP BY tohead_id, tohead_number,"
	       "         tohead_src_warehous_id, tohead_dest_warehous_id,"
	       "         tohead_orderdate "
	       "ORDER BY tohead_number;" );

  ParameterList params;
  setParams(params);
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _to->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _to->setDragString("toheadid=");
}
