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

#include "dspWoEffortByUser.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>
#include <QMenu>
#include <QSqlError>
#include <openreports.h>
#include "implodeWo.h"
#include "explodeWo.h"
#include "closeWo.h"
#include "printWoTraveler.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "changeWoQty.h"
#include "workOrder.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "dspWoOperationsByWorkOrder.h"
#include "dspWoEffortByUser.h"
#include "wotc.h"

/*
 *  Constructs a dspWoEffortByUser as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoEffortByUser::dspWoEffortByUser(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wotc, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_user, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));

  _dates->setStartCaption(tr("Start W/O Start Date:"));
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("End W/O Start Date:"));

  _wotc->addColumn(tr("W/O #"),      _orderColumn,      Qt::AlignLeft   );
  _wotc->addColumn(tr("Status"),     _statusColumn,     Qt::AlignCenter );
  _wotc->addColumn(tr("Pri."),       _statusColumn,     Qt::AlignCenter );
  _wotc->addColumn(tr("Whs."),       _whsColumn,        Qt::AlignCenter );
  _wotc->addColumn(tr("Operation"),  -1,                Qt::AlignLeft   );
  _wotc->addColumn(tr("Clock In"),   _timeDateColumn,   Qt::AlignLeft   );
  _wotc->addColumn(tr("Clock Out"),  _timeDateColumn,   Qt::AlignLeft   );
  _wotc->addColumn(tr("Effort"),     _timeColumn,       Qt::AlignRight  );
  
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _user->setFocus();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoEffortByUser::~dspWoEffortByUser()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoEffortByUser::languageChange()
{
  retranslateUi(this);
}

void dspWoEffortByUser::sPrint()
{
  ParameterList params;
  params.append("usr_id", _user->id());
  _dates->appendValue(params);

  orReport report("WOEffortByUser", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

int dspWoEffortByUser::getWoId()
{
  q.prepare("SELECT wotc_wo_id "
	    "FROM wotc "
	    "WHERE (wotc_id=:wotc_id);");
  q.bindValue(":wotc_id", _wotc->id());
  q.exec();
  if (q.first())
    return q.value("wotc_wo_id").isNull() ? -1 : q.value("wotc_wo_id").toInt();
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  return -1;
}

void dspWoEffortByUser::sViewWO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", getWoId());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoEffortByUser::sCloseWO()
{
  ParameterList params;
  params.append("wo_id", getWoId());

  closeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoEffortByUser::sViewWomatl()
{
  ParameterList params;
  params.append("wo_id", getWoId());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoEffortByUser::sViewWooper()
{
  ParameterList params;
  params.append("wo_id", getWoId());
  params.append("run");

  dspWoOperationsByWorkOrder *newdlg = new dspWoOperationsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoEffortByUser::sNew()
{
  ParameterList params;
  params.append("wo_id", _wotc->altId());
  params.append("usr_id", _user->id());
  params.append("mode",  "new");

  wotc *newdlg = new wotc(this, "", TRUE);
  newdlg->set(params);
  if (newdlg->exec() == QDialog::Accepted)
    sFillList();
}

void dspWoEffortByUser::sEdit()
{
  ParameterList params;
  params.append("wotc_id", _wotc->id());
  params.append("mode",    "edit");

  wotc *newdlg = new wotc(this, "", TRUE);
  newdlg->set(params);
  if (newdlg->exec() == QDialog::Accepted)
    sFillList();
}

void dspWoEffortByUser::sView()
{
  ParameterList params;
  params.append("wotc_id", _wotc->id());
  params.append("mode",    "view");
  
  wotc *newdlg = new wotc(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void dspWoEffortByUser::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Entry?"),
			    tr("Are you sure that you want to delete the "
			       "selected time clock entry?"),
			    tr("&Yes"), tr("&No"), 0, 0, 1) == 0)
  {
    q.prepare("DELETE FROM wotc "
	      "WHERE (wotc_id=:wotc_id);");
    q.bindValue(":wotc_id", _wotc->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    else
      sFillList();
  }
}

void dspWoEffortByUser::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  QString status(selected->text(1));
  int     menuItem;

  if (_wotc->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("New"), this, SLOT(sNew()), 0);
    pMenu->setItemEnabled(menuItem, _privleges->check("MaintainWoTimeClock"));
    
    menuItem = pMenu->insertItem(tr("Edit"), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, _privleges->check("MaintainWoTimeClock"));

    menuItem = pMenu->insertItem(tr("View"), this, SLOT(sView()), 0);

    menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
    pMenu->setItemEnabled(menuItem, _privleges->check("MaintainWoTimeClock"));

    pMenu->insertSeparator();
  }

  menuItem = pMenu->insertItem(tr("View W/O"), this, SLOT(sViewWO()), 0);

  if ((status == "R") || (status == "I"))
  {
    menuItem = pMenu->insertItem(tr("Close W/O..."), this, SLOT(sCloseWO()), 0);
    if (!_privleges->check("CloseWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }


  if ((status == "E") || (status == "R") || (status == "I"))
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("View W/O Material Requirements..."), this, SLOT(sViewWomatl()), 0);
    if (!_privleges->check("ViewWoMaterials"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View W/O Operations..."), this, SLOT(sViewWooper()), 0);
    if (!_privleges->check("ViewWoOperations"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspWoEffortByUser::sFillList()
{
  _wotc->clear();

  if ((_user->isValid()) && (_dates->allValid()))
  {
    QString sql( "SELECT wotc_id, wo_id, formatWONumber(wo_id) AS wonumber,"
		 " wo_status, wo_priority, warehous_code,"
                 " formatDateTime(wotc_timein) AS timein,"
                 " formatDateTime(wotc_timeout) AS timeout,"
		 " wooper_seqnumber || ' - ' || wooper_descrip1 || ' - ' || wooper_descrip2 AS wooper,"
		 " formatInterval(wotcTime(wotc_id)) AS wotcTime "
                 "FROM wo, itemsite, warehous, wotc, wooper "
                 "WHERE ((wo_itemsite_id=itemsite_id)"
                 " AND (itemsite_warehous_id=warehous_id)"
		 " AND (wotc_wooper_id=wooper_id)"
		 " AND (wotc_wo_id=wo_id)"
		 " AND (wotc_usr_id=:usr_id)"
                 " AND (wo_startdate BETWEEN :startDate AND :endDate)) "
		 "UNION "
		 "SELECT wotc_id, wo_id, formatWONumber(wo_id) AS wonumber,"
		 " wo_status, wo_priority, warehous_code,"
                 " formatDateTime(wotc_timein) AS timein,"
                 " formatDateTime(wotc_timeout) AS timeout,"
		 " CAST(wooperpost_seqnumber AS TEXT) AS wooper,"
		 " formatInterval(wotcTime(wotc_id)) AS wotcTime "
                 "FROM wo, itemsite, warehous, wotc LEFT OUTER JOIN"
		 "     wooperpost ON (wooperpost_wotc_id=wotc_id) "
                 "WHERE ((wo_itemsite_id=itemsite_id)"
                 " AND (itemsite_warehous_id=warehous_id)"
		 " AND (wotc_wooper_id IS NULL)"
		 " AND (wotc_wo_id=wo_id)"
		 " AND (wotc_usr_id=:usr_id)"
                 " AND (wo_startdate BETWEEN :startDate AND :endDate)) "
		 "ORDER BY wonumber, timein, timeout;");

    q.prepare(sql);
    _dates->bindValue(q);
    q.bindValue(":usr_id", _user->id());
    q.exec();

    XTreeWidgetItem *last = 0;
    while (q.next() && (q.lastError().type() == QSqlError::NoError))
    {
      last = new XTreeWidgetItem(_wotc, last,
			       q.value("wotc_id").toInt(), q.value("wo_id").toInt(),
			       q.value("wonumber"),    q.value("wo_status"),
			       q.value("wo_priority"), q.value("warehous_code"),
			       q.value("wooper"),      q.value("timein"),
			       q.value("timeout"),     q.value("wotcTime"));
    }
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    XSqlQuery total;
    total.prepare("SELECT formatInterval(woTime(:wotc_wo_id, :wotc_usr_id)) AS total;");
    for (int i = 0; i < _wotc->topLevelItemCount(); i++)
    {
      last = _wotc->topLevelItem(i);
      if ((i == _wotc->topLevelItemCount() - 1) ||
	  last->altId() != ((XTreeWidgetItem*)(_wotc->topLevelItem(i + 1)))->altId())
      {
	total.bindValue(":wotc_wo_id",  last->altId());
	total.bindValue(":wotc_usr_id", _user->id());
	total.exec();
	if (total.first())
	{
	  last = new XTreeWidgetItem(_wotc, last, -1, last->altId(),
				   last->text(0), last->text(1), last->text(2),
				   last->text(3), tr("Total"), "", "",
				   total.value("total"));
	  i++;
	}
	else if (total.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, total.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
    } 
  }
}

void dspWoEffortByUser::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}
