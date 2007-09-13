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

#include "packingListBatch.h"

#include <Q3DragObject>
#include <QMenu>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"
#include "printPackingList.h"
#include "salesOrder.h"
#include "salesOrderList.h"
#include "socluster.h"
#include "storedProcErrorLookup.h"
#include "transferOrder.h"
#include "transferOrderList.h"

#define	TYPE_COL 1	// must match _pack->addColumn for "Type"

packingListBatch::packingListBatch(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_add,		     SIGNAL(clicked()), this, SLOT(sAddSO()));
  connect(_addTO,	     SIGNAL(clicked()), this, SLOT(sAddTO()));
  connect(_autoUpdate,	 SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_delete,	     SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_deletePrinted,    SIGNAL(clicked()), this, SLOT(sClearPrinted()));
  connect(_pack, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_printBatch,       SIGNAL(clicked()), this, SLOT(sPrintBatch()));
  connect(_printEditList,    SIGNAL(clicked()), this, SLOT(sPrintEditList()));
  connect(_printPackingList, SIGNAL(clicked()), this, SLOT(sPrintPackingList()));

  setAcceptDrops(TRUE);

  _pack->addColumn(tr("Order #"),       80,           Qt::AlignCenter );
  _pack->addColumn(tr("Type"),		40,           Qt::AlignCenter );
  _pack->addColumn(tr("Shipment #"),    80,           Qt::AlignCenter );
  _pack->addColumn(tr("Customer #"),    _itemColumn,  Qt::AlignLeft   );
  _pack->addColumn(tr("Customer Name"), -1,           Qt::AlignLeft   );
  _pack->addColumn(tr("Hold Type"),     _dateColumn,  Qt::AlignCenter );
  _pack->addColumn(tr("Prt'd"),         _ynColumn,    Qt::AlignCenter );

  if (_privleges->check("MaintainPackingListBatch"))
  {
    _add->setEnabled(TRUE);
    _addTO->setEnabled(TRUE);
    _deletePrinted->setEnabled(TRUE);
    connect(_pack, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  }

  if (_privleges->check("PrintPackingLists"))
  {
    _printBatch->setEnabled(TRUE);
    connect(_pack, SIGNAL(valid(bool)), _printPackingList, SLOT(setEnabled(bool)));
  }

  _addTO->setVisible(_metrics->boolean("MultiWhs"));

  sFillList();
}

packingListBatch::~packingListBatch()
{
    // no need to delete child widgets, Qt does it all for us
}

void packingListBatch::languageChange()
{
    retranslateUi(this);
}

void packingListBatch::sPrintBatch()
{
  QPrinter printer;
  bool     setupPrinter = TRUE;

  XSqlQuery updateq;
  updateq.prepare("UPDATE pack "
		  "SET pack_printed=TRUE "
		  "WHERE (pack_id=:packid);" );

  ParameterList params;
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  MetaSQLQuery mql = mqlLoad(":/sr/forms/packingListBatch/PrintBatch.mql");
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("Could not initialize printing system for multiple reports."));
    return;
  }
  while (q.next())
  {
    int osmiscid = q.value("pack_shiphead_id").toInt();

    // set sohead_id and cosmisc_id for compatibility with existing reports
    ParameterList params;
    params.append("sohead_id", q.value("pack_head_id").toInt());
    params.append("pack_head_id", q.value("pack_head_id").toInt());
    if (osmiscid > 0)
    {
      params.append("cosmisc_id", osmiscid);
      params.append("shiphead_id",  osmiscid);
    }
    params.append("pack_head_type", q.value("pack_head_type").toString());

    orReport report(q.value(osmiscid > 0 ? "packform" : "pickform").toString(), params);
    if (report.isValid() && report.print(&printer, setupPrinter))
    {
        setupPrinter = FALSE;
	updateq.bindValue(":packid", q.value("pack_id").toInt());
	updateq.exec();
	if (updateq.lastError().type() != QSqlError::None)
	{
	  systemError(this, updateq.lastError().databaseText(), __FILE__, __LINE__);
	  orReport::endMultiPrint(&printer);
	  return;
	}
    }
    else
    {
      report.reportError(this);
      orReport::endMultiPrint(&printer);
      return;
    }
  }
  orReport::endMultiPrint(&printer);

  if (setupPrinter)
  {
    QMessageBox::warning(this, tr("Nothing to Print"),
			 tr("<p>All of the Packing Lists appear to have been "
			    "printed already."));
  }
  else
  {
    sFillList();
  }
}

void packingListBatch::setParams(ParameterList & params)
{
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  params.append("none",		tr("None"));
  params.append("credit",	tr("Credit"));
  params.append("ship",		tr("Ship"));
  params.append("pack",		tr("Pack"));
  params.append("other",	tr("Other"));
}

void packingListBatch::sPrintEditList()
{
  ParameterList params;
  setParams(params);
  orReport report("PackingListBatchEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void packingListBatch::sClearPrinted()
{
  q.exec( "DELETE FROM pack "
          "WHERE (pack_printed);" );
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void packingListBatch::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Sales Order..."), this, SLOT(sViewSalesOrder()), 0);
  if (_pack->currentItem()->text(TYPE_COL) != "SO" ||
      (! _privleges->check("MaintainSalesOrders") &&
       ! _privleges->check("ViewSalesOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);
    

  menuItem = pMenu->insertItem(tr("View Transfer Order..."), this, SLOT(sViewTransferOrder()), 0);
  if (_pack->currentItem()->text(TYPE_COL) != "TO" ||
      (! _privleges->check("MaintainTransferOrders") &&
       ! _privleges->check("ViewTransferOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void packingListBatch::sViewSalesOrder()
{
  salesOrder::viewSalesOrder(_pack->id());
}

void packingListBatch::sViewTransferOrder()
{
  transferOrder::viewTransferOrder(_pack->id());
}

void packingListBatch::sAddSO()
{
  ParameterList params;
  params.append("soType", cSoOpen);
  
  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  int soid;
  if ((soid = newdlg.exec()) != -1)
  {
    q.prepare("SELECT addToPackingListBatch('SO', :sohead_id) AS result;");
    q.bindValue(":sohead_id", soid);
    q.exec();
    if (q.first())
      sFillList();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void packingListBatch::sAddTO()
{
  ParameterList params;
  params.append("toType", cToOpen);
  
  transferOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  int toid;
  if ((toid = newdlg.exec()) != -1)
  {
    q.prepare("SELECT addToPackingListBatch('TO', :tohead_id) AS result;");
    q.bindValue(":tohead_id", toid);
    q.exec();
    if (q.first())
      sFillList();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void packingListBatch::sDelete()
{
  QString sql( "DELETE FROM pack "
	       "WHERE ((pack_head_id=<? value(\"head_id\") ?>)"
	       "  AND  (pack_head_type=<? value(\"head_type\") ?>)"
	       " <? if exists(\"shiphead_id\") ?>"
	       "  AND  (pack_shiphead_id=<? value(\"shiphead_id\") ?>)"
	       " <? else ?>"
	       "  AND  (pack_shiphead_id IS NULL)"
	       "<? endif ?>"
	       ");" );

  ParameterList params;
  params.append("head_id",   _pack->id());
  params.append("head_type", _pack->currentItem()->text(TYPE_COL));
  if (_pack->altId() > 0)
    params.append("shiphead_id", _pack->altId());

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void packingListBatch::sPrintPackingList()
{
  if (_pack->altId() == -1)
  {
    QMessageBox::critical(this, tr("Shipment Number Required"),
			  tr("<p>Packing Lists may only be printed for "
			     "existing Shipments and there is no Shipment for "
			     "this Order. Issue Stock To Shipping to "
			     "create a Shipment."));
    return;
  }

  ParameterList params;
  if (_pack->currentItem()->text(TYPE_COL) == "TO")
  {
    params.append("head_id",     _pack->id());
    params.append("shiphead_id", _pack->altId());
    params.append("head_type",   _pack->currentItem()->text(TYPE_COL));
    params.append("print");
  }
  else
  {
    params.append("sohead_id",   _pack->id());
    params.append("cosmisc_id",  _pack->altId());
    params.append("shiphead_id", _pack->altId());
  }

  printPackingList newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError_Print ||
      newdlg.exec() != QDialog::Rejected)
  {
    QString sql( "UPDATE pack "
		 "SET pack_printed=TRUE "
		 "WHERE ((pack_head_id=<? value(\"head_id\") ?>)"
	       "  AND  (pack_head_type=<? value(\"head_type\") ?>)"
		 " <? if exists(\"shiphead_id\") ?>"
		 "  AND  (pack_shiphead_id=<? value(\"shiphead_id\") ?>)"
		 " <? else ?>"
		 "  AND  (pack_shiphead_id IS NULL)"
		 "<? endif ?>"
		 ");" );

    ParameterList params;
    params.append("head_id",   _pack->id());
    params.append("head_type", _pack->currentItem()->text(TYPE_COL));
    if (_pack->altId() > 0)
      params.append("shiphead_id", _pack->altId());

    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void packingListBatch::sFillList()
{
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql = mqlLoad(":/sr/forms/packingListBatch/FillListDetail.mql");
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _pack->clear();
  _pack->populate(q, true);
}

void packingListBatch::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

void packingListBatch::dragEnterEvent(QDragEnterEvent *pEvent)
{
  QString dragData;

  if ( (Q3TextDrag::decode(pEvent, dragData)) && (_privleges->check("MaintainPackingListBatch")) )
  {
    if (dragData.contains("soheadid=") || dragData.contains("toheadid="))
      pEvent->accept(TRUE);
  }
  else
    pEvent->accept(FALSE);
}

void packingListBatch::dropEvent(QDropEvent *pEvent)
{
  QString dropData;

  if (Q3TextDrag::decode(pEvent, dropData))
  {
    if (dropData.contains("soheadid=") || dropData.contains("toheadid="))
    {
      QString target;
      QString targettype;
      if (dropData.contains("soheadid="))
      {
	target = dropData.mid((dropData.find("soheadid=") + 9), (dropData.length() - 9));
	targettype="SO";
      }
      else if (dropData.contains("toheadid="))
      {
	target = dropData.mid((dropData.find("toheadid=") + 9), (dropData.length() - 9));
	targettype="TO";
      }

      if (target.contains(","))
        target = target.left(target.find(","));

      if (target.toInt() != -1)
      {
        q.prepare( "SELECT addToPackingListBatch(:head_type, :head_id) AS result;");
	q.bindValue(":head_type", targettype);
        q.bindValue(":head_id",   target.toInt());
        q.exec();
        if (q.first())
        {
	  int result = q.value("result").toInt();
	  if (result < 0)
	  {
	    systemError(this,
			storedProcErrorLookup("addToPackingListBatch", result),
			__FILE__, __LINE__);
	    return;
	  }

          sFillList();
        }
	else if (q.lastError().type() != QSqlError::None)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
    }
  }
}
