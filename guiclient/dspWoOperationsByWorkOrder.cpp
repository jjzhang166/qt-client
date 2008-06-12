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

#include "dspWoOperationsByWorkOrder.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <QMenu>
#include <openreports.h>
#include "inputManager.h"
#include "woOperation.h"

/*
 *  Constructs a dspWoOperationsByWorkOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoOperationsByWorkOrder::dspWoOperationsByWorkOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wooper, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_wo, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wooper->addColumn( tr("Seq #"),         _seqColumn,  Qt::AlignCenter );
  _wooper->addColumn( tr("Scheduled"),     _dateColumn, Qt::AlignCenter );
  _wooper->addColumn( tr("Work Center"),   _itemColumn, Qt::AlignLeft   );
  _wooper->addColumn( tr("Std. Oper."),    _itemColumn, Qt::AlignLeft   );
  _wooper->addColumn( tr("Description"),   -1,          Qt::AlignLeft   );
  _wooper->addColumn( tr("Setup Remain."), _itemColumn, Qt::AlignRight  );
  _wooper->addColumn( tr("Run Remain."),   _itemColumn, Qt::AlignRight  );
  _wooper->addColumn( tr("Qty. Complete"), _qtyColumn,  Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoOperationsByWorkOrder::~dspWoOperationsByWorkOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoOperationsByWorkOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspWoOperationsByWorkOrder::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspWoOperationsByWorkOrder::sPrint()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  orReport report("WOOperationsByWorkOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoOperationsByWorkOrder::sPopulateMenu(QMenu *pMenu)
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

void dspWoOperationsByWorkOrder::sViewOperation()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoOperationsByWorkOrder::sEditOperation()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspWoOperationsByWorkOrder::sDeleteOperation()
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

void dspWoOperationsByWorkOrder::sFillList()
{
  q.prepare( "SELECT wooper_id, wooper_seqnumber, formatDate(wooper_scheduled),"
             "       CASE WHEN (wooper_wrkcnt_id <> -1) THEN (SELECT wrkcnt_code FROM wrkcnt WHERE (wrkcnt_id=wooper_wrkcnt_id))"
             "            ELSE 'Any'"
             "       END,"
             "       CASE WHEN (wooper_stdopn_id <> -1) THEN (SELECT stdopn_number FROM stdopn WHERE (stdopn_id=wooper_stdopn_id))"
             "            ELSE ''"
             "       END,"
             "       (wooper_descrip1 || ' ' || wooper_descrip2),"
             "       CASE WHEN (wooper_sucomplete) THEN :complete"
             "            ELSE formatTime(noNeg(wooper_sutime - wooper_suconsumed))"
             "       END,"
             "       CASE WHEN (wooper_rncomplete) THEN :complete"
             "            ELSE formatTime(noNeg(wooper_rntime - wooper_rnconsumed))"
             "       END,"
             "       formatQty(wooper_qtyrcv) "
             "FROM wooper "
             "WHERE (wooper_wo_id=:wo_id) "
             "ORDER BY wooper_seqnumber;" );
  q.bindValue(":complete", tr("Complete"));
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  _wooper->populate(q);
}
