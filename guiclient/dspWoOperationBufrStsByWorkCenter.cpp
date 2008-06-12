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

#include "dspWoOperationBufrStsByWorkCenter.h"

#include <QMenu>
#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <openreports.h>
#include "woOperation.h"
#include "submitReport.h"

/*
 *  Constructs a dspWoOperationBufrStsByWorkCenter as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoOperationBufrStsByWorkCenter::dspWoOperationBufrStsByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wooper, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_wrkcnt, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));

  _wrkcnt->populate( "SELECT wrkcnt_id, wrkcnt_code "
                     "FROM wrkcnt "
                     "ORDER BY wrkcnt_code;" );

  _wooper->addColumn(tr("W/O #"),         _orderColumn,  Qt::AlignLeft   );
  _wooper->addColumn(tr("Status"),        _statusColumn, Qt::AlignCenter );
  _wooper->addColumn(tr("Type"),          _uomColumn,    Qt::AlignLeft);
  _wooper->addColumn(tr("Item Number"),   _itemColumn,   Qt::AlignLeft   );
  _wooper->addColumn(tr("Seq #"),         _seqColumn,    Qt::AlignCenter );
  _wooper->addColumn(tr("Std. Oper."),    _itemColumn,   Qt::AlignLeft   );
  _wooper->addColumn(tr("Description"),   -1,            Qt::AlignLeft   );
  _wooper->addColumn(tr("Setup Remain."), _itemColumn,   Qt::AlignRight  );
  _wooper->addColumn(tr("Run Remain."),   _itemColumn,   Qt::AlignRight  );
  _wooper->addColumn(tr("Qty. Remain."),  _qtyColumn,    Qt::AlignRight  );
  _wooper->addColumn(tr("UOM"),           _uomColumn,    Qt::AlignCenter );
  
  if (_preferences->boolean("XCheckBox/forgetful"))
    _QtyAvailOnly->setChecked(true);

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoOperationBufrStsByWorkCenter::~dspWoOperationBufrStsByWorkCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoOperationBufrStsByWorkCenter::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspWoOperationBufrStsByWorkCenter::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wrkcnt_id", &valid);
  if (valid)
    _wrkcnt->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspWoOperationBufrStsByWorkCenter::sPrint()
{
  ParameterList params(buildParameters());

  orReport report("WOOperationBufrStsByWorkCenter", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoOperationBufrStsByWorkCenter::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Operation..."), this, SLOT(sViewOperation()), 0);
  if ((!_privileges->check("ViewWoOperations")) && (!_privileges->check("MaintainWoOperations")))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Operation..."), this, SLOT(sEditOperation()), 0);
  if (!_privileges->check("MaintainWoOperations"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete Operation..."), this, SLOT(sDeleteOperation()), 0);
  if (!_privileges->check("MaintainWoOperations"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspWoOperationBufrStsByWorkCenter::sViewOperation()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoOperationBufrStsByWorkCenter::sEditOperation()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspWoOperationBufrStsByWorkCenter::sDeleteOperation()
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

    sFillList();
  }
}

void dspWoOperationBufrStsByWorkCenter::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

void dspWoOperationBufrStsByWorkCenter::sFillList()
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

  QString sql( "SELECT wooper_id, formatWoNumber(wo_id) AS f_wonumber, bufrsts_status,"
               "       CASE WHEN (bufrsts_type='T') THEN 'Time'"
               "           ELSE 'Stock'"
               "       END AS f_type,"
               "       item_number, wooper_seqnumber,"
               "       CASE WHEN (wooper_stdopn_id <> -1) THEN ( SELECT stdopn_number"
               "                                                   FROM stdopn"
               "                                                  WHERE (stdopn_id=wooper_stdopn_id) )"
               "            ELSE ''"
               "       END AS f_stdoper,"
               "       (wooper_descrip1 || ' ' || wooper_descrip2) AS f_descrip,"
               "       CASE WHEN (wooper_sucomplete) THEN :complete"
               "            ELSE formatTime(noNeg(wooper_sutime - wooper_suconsumed))"
               "       END AS f_sucomplete,"
               "       CASE WHEN (wooper_rncomplete) THEN :complete"
               "            ELSE formatTime(noNeg(wooper_rntime - wooper_rnconsumed))"
               "       END AS f_rncomplete,"
               "       formatQty(noNeg(wo_qtyord - wooper_qtyrcv)) AS f_qtyremain, uom_name,"
               "       (bufrsts_status > 65) AS emergency "
               "  FROM wooper, wo, itemsite, item, uom, bufrsts "
               " WHERE ( (wooper_wo_id=wo_id)"
               "   AND   (wo_itemsite_id=itemsite_id)"
               "   AND   (wo_status <> 'C')"
               "   AND   (itemsite_item_id=item_id)"
               "   AND   (item_inv_uom_id=uom_id)"
               "   AND   (bufrsts_target_type='W')"
               "   AND   (bufrsts_target_id=wo_id)"
               "   AND   (bufrsts_date=current_date)"
               "   AND   (wooper_rncomplete=False)"
               "   AND   (wooper_wrkcnt_id=:wrkcnt_id)" );

  if (_QtyAvailOnly->isChecked())
    sql += " AND ((wooperqtyavail(wooper_id) > 0))";

  sql += ") "
         "ORDER BY bufrsts_status DESC, wo_number, wo_subnumber, wooper_seqnumber;";

  q.prepare(sql);
  q.bindValue(":complete", tr("Complete"));
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  XTreeWidgetItem *last = 0;
  while(q.next())
  {
    last = new XTreeWidgetItem(_wooper, last,
			      q.value("wooper_id").toInt(),
			      q.value("f_wonumber"), q.value("bufrsts_status"),
			      q.value("f_type"), q.value("item_number"),
			      q.value("wooper_seqnumber"), q.value("f_stdoper"),
			      q.value("f_descrip"), q.value("f_sucomplete"),
			      q.value("f_rncomplete"), q.value("f_qtyremain"),
			      q.value("uom_name") );
    if(q.value("emergency").toBool())
      last->setTextColor(1, "red");
  }
}

void dspWoOperationBufrStsByWorkCenter::sSubmit()
{
  ParameterList params(buildParameters());
  params.append("report_name","WOOperationBufrStsByWorkCenter");

  submitReport newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.check() == cNoReportDefinition)
    QMessageBox::critical( this, tr("Report Definition Not Found"),
                           tr( "The report defintions for this report, \"WOOperationBufrStsByWorkCenter\" cannot be found.\n"
                               "Please contact your Systems Administrator and report this issue." ) );
  else
    newdlg.exec();
}

ParameterList dspWoOperationBufrStsByWorkCenter::buildParameters()
{
  ParameterList params;
  params.append("wrkcnt_id", _wrkcnt->id());

  if(_QtyAvailOnly->isChecked())
    params.append("QtyAvailOnly");

  return params;

}

