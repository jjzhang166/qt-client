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

#include "dspWoOperationsByWorkCenter.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <openreports.h>
#include "woOperation.h"
#include "postOperations.h"
#include "postProduction.h"
#include "dspRunningAvailability.h"
#include "dspMPSDetail.h"

/*
 *  Constructs a dspWoOperationsByWorkCenter as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoOperationsByWorkCenter::dspWoOperationsByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wooper, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _wrkcnt->populate( "SELECT wrkcnt_id, wrkcnt_code "
                     "FROM wrkcnt "
                     "ORDER BY wrkcnt_code;" );

  _wooper->addColumn(tr("Source"),        _orderColumn, Qt::AlignLeft   );
  _wooper->addColumn(tr("W/O #"),         _orderColumn, Qt::AlignLeft   );
  _wooper->addColumn(tr("Due Date"),      _dateColumn,  Qt::AlignCenter );
  _wooper->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignLeft   );
  _wooper->addColumn(tr("Seq #"),         _seqColumn,   Qt::AlignCenter );
  _wooper->addColumn(tr("Std. Oper."),    _itemColumn,  Qt::AlignLeft   );
  _wooper->addColumn(tr("Description"),   -1,           Qt::AlignLeft   );
  _wooper->addColumn(tr("Setup Remain."), _itemColumn,  Qt::AlignRight  );
  _wooper->addColumn(tr("Run Remain."),   _itemColumn,  Qt::AlignRight  );
  _wooper->addColumn(tr("Qty. Remain."),  _qtyColumn,   Qt::AlignRight  );
  _wooper->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter );
  
  if (_preferences->boolean("XCheckBox/forgetful"))
    _loadOnly->setChecked(true);

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoOperationsByWorkCenter::~dspWoOperationsByWorkCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoOperationsByWorkCenter::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspWoOperationsByWorkCenter::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wrkcnt_id", &valid);
  if (valid)
    _wrkcnt->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  _loadOnly->setChecked(pParams.inList("loadOnly"));

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspWoOperationsByWorkCenter::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("wrkcnt_id", _wrkcnt->id());

  if(_loadOnly->isChecked())
    params.append("loadOnly");

  orReport report("WOOperationsByWorkCenter", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoOperationsByWorkCenter::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;
  bool multi = false;

  int cnt = _wooper->selectedItems().size();
  multi = (cnt > 1);

  menuItem = pMenu->insertItem(tr("View Operation..."), this, SLOT(sViewOperation()), 0);
  if (multi || (!_privileges->check("ViewWoOperations")) && (!_privileges->check("MaintainWoOperations")))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Operation..."), this, SLOT(sEditOperation()), 0);
  if (multi || !_privileges->check("MaintainWoOperations"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete Operation..."), this, SLOT(sDeleteOperation()), 0);
  if (multi || !_privileges->check("MaintainWoOperations"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Post Production..."), this, SLOT(sPostProduction()), 0);
  if (multi || !_privileges->check("PostProduction"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Post Operations..."), this, SLOT(sPostOperations()), 0);
  if (multi || !_privileges->check("PostWoOperations"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sRunningAvailability()), 0);
  if (!_privileges->check("ViewInventoryAvailability"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("MPS Detail..."), this, SLOT(sMPSDetail()), 0);
  if (!_privileges->check("ViewMPS"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Print Pick List(s)..."), this, SLOT(sPrintPickLists()), 0);
  if (!_privileges->check("PrintWorkOrderPaperWork"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspWoOperationsByWorkCenter::sViewOperation()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoOperationsByWorkCenter::sEditOperation()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspWoOperationsByWorkCenter::sDeleteOperation()
{
  if (QMessageBox::critical( this, tr("Delete W/O Operation"),
                             tr( "If you Delete the selected W/O Operation\n"
                                 "you will not be able to post Labor to this Operation\n"
                                 "to this W/O.  Are you sure that you want to delete the\n"
                                 "selected W/O Operation?"),
                                 tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare( "DELETE FROM wooper "
               "WHERE (wooper_id=:wooper_id);" );
    q.bindValue(":wooper_id", _wooper->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void dspWoOperationsByWorkCenter::sRunningAvailability()
{
  q.prepare("SELECT wo_itemsite_id"
            "  FROM wo"
            " WHERE (wo_id=:wo_id);");
  q.bindValue(":wo_id", _wooper->altId());
  q.exec();
  if(q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("wo_itemsite_id").toInt());

    dspRunningAvailability * newdlg = new dspRunningAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspWoOperationsByWorkCenter::sMPSDetail()
{
  q.prepare("SELECT wo_itemsite_id"
            "  FROM wo"
            " WHERE (wo_id=:wo_id);");
  q.bindValue(":wo_id", _wooper->altId());
  q.exec();
  if(q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("wo_itemsite_id").toInt());

    dspMPSDetail * newdlg = new dspMPSDetail();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}


void dspWoOperationsByWorkCenter::sFillList()
{
  _wooper->clear();

  q.prepare( "SELECT wrkcnt_descrip, warehous_code "
             "FROM wrkcnt, warehous "
             "WHERE ( (wrkcnt_warehous_id=warehous_id)"
             " AND (wrkcnt_id=:wrkcnt_id) );" );
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  if (q.first())
  {
    _description->setText(q.value("wrkcnt_descrip").toString());
    _warehouse->setText(q.value("warehous_code").toString());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  QString sql( "SELECT wooper_id, wooper_wo_id, formatWoNumber(wo_id) AS wonumber, formatDate(wooper_scheduled) AS f_scheduled, item_number, wooper_seqnumber,"
               "       CASE WHEN (wooper_stdopn_id <> -1) THEN ( SELECT stdopn_number FROM stdopn WHERE (stdopn_id=wooper_stdopn_id) )"
               "            ELSE ''"
               "       END AS stdoper,"
               "       (wooper_descrip1 || ' ' || wooper_descrip2) AS descrip,"
               "       CASE WHEN (wooper_sucomplete) THEN :complete"
               "            ELSE formatTime(noNeg(wooper_sutime - wooper_suconsumed))"
               "       END AS setupremain,"
               "       CASE WHEN (wooper_rncomplete) THEN :complete"
               "            ELSE formatTime(noNeg(wooper_rntime - wooper_rnconsumed))"
               "       END AS runremain,"
               "       formatQty(noNeg(wo_qtyord - wooper_qtyrcv)) AS qtyremain, uom_name,"
               "       CASE WHEN(wo_ordtype='M') THEN :mrp"
               "            WHEN(wo_ordtype='P') THEN :mps"
               "            WHEN(wo_ordtype='S') THEN (:so||'-'||formatSoNumber(wo_ordid))"
               "            WHEN(wo_ordtype='W') THEN (:wo||'-'||formatWoNumber(wo_ordid))"
               "            WHEN(wo_ordtype IS NULL OR wo_ordtype='') THEN :manual"
               "            ELSE wo_ordtype"
               "       END AS source,"
               "       (date(wooper_scheduled) < CURRENT_DATE) AS overdue "
               "FROM wooper, wo, itemsite, item, uom "
               "WHERE ( (wooper_wo_id=wo_id)"
               " AND (wo_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (DATE(wooper_scheduled) BETWEEN :startDate AND :endDate)"
               " AND (wooper_wrkcnt_id=:wrkcnt_id)" );

  if (_loadOnly->isChecked())
    sql += " AND ( ((wooper_sutime - wooper_suconsumed) > 0) OR ((wooper_rntime - wooper_rnconsumed) > 0) )";

  sql += ") "
         "ORDER BY wooper_scheduled, wo_number, wo_subnumber, wooper_seqnumber;";

  q.prepare(sql);
  _dates->bindValue(q);
  q.bindValue(":complete", tr("Complete"));
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.bindValue(":mrp", tr("MRP"));
  q.bindValue(":mps", tr("MPS"));
  q.bindValue(":so", tr("SO"));
  q.bindValue(":wo", tr("WO"));
  q.bindValue(":manual", tr("Manual"));
  q.exec();

  XTreeWidgetItem * last = 0;
  while(q.next())
  {
    last = new XTreeWidgetItem(_wooper, last, q.value("wooper_id").toInt(),
                             q.value("wooper_wo_id").toInt(),
                             q.value("source"),
                             q.value("wonumber"), q.value("f_scheduled"),
                             q.value("item_number"), q.value("wooper_seqnumber"),
                             q.value("stdoper"), q.value("descrip"),
                             q.value("setupremain"), q.value("runremain"),
                             q.value("qtyremain"), q.value("uom_name") );
    if(q.value("overdue").toBool())
      last->setTextColor("red");
  }
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspWoOperationsByWorkCenter::sPostProduction()
{
  ParameterList params;
  params.append("wo_id", _wooper->altId());

  postProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoOperationsByWorkCenter::sPostOperations()
{
  ParameterList params;
  params.append("wooper_id", _wooper->id());

  postOperations newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoOperationsByWorkCenter::sPrintPickLists()
{
  QPrinter printer(QPrinter::HighResolution);
  int counter = 0;
  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("Could not initialize printing system for multiple reports."));
    return;
  }
  QList<QTreeWidgetItem*> selected = _wooper->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("wo_id", ((XTreeWidgetItem*)selected[i])->altId());

    orReport report("PickList", params);
    if (! (report.isValid() && report.print(&printer, (counter == 0))) )
    {
      report.reportError(this);
      orReport::endMultiPrint(&printer);
      return;
    }

    counter++;
  }
  orReport::endMultiPrint(&printer);
}
