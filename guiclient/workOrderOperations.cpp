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

#include "workOrderOperations.h"

#include <QMessageBox>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "storedProcErrorLookup.h"
#include "woOperation.h"

workOrderOperations::workOrderOperations(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_wooper, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNewOperation()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEditOperation()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sViewOperation()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDeleteOperation()));
  connect(_moveUp, SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_moveDown, SIGNAL(clicked()), this, SLOT(sMoveDown()));

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wooper->addColumn(tr("Seq #"),         _seqColumn,  Qt::AlignCenter );
  _wooper->addColumn(tr("Work Center"),   _itemColumn, Qt::AlignLeft   );
  _wooper->addColumn(tr("Std. Oper."),    _itemColumn, Qt::AlignLeft   );
  _wooper->addColumn(tr("Description"),   -1,          Qt::AlignLeft   );
  _wooper->addColumn(tr("Setup Remain."), _itemColumn, Qt::AlignRight  );
  _wooper->addColumn(tr("Run Remain."),   _itemColumn, Qt::AlignRight  );
  
  if (_privileges->check("MaintainWoOperations"))
  {
    connect(_wooper, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_wooper, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_wooper, SIGNAL(valid(bool)), _moveUp, SLOT(setEnabled(bool)));
    connect(_wooper, SIGNAL(valid(bool)), _moveDown, SLOT(setEnabled(bool)));
    connect(_wooper, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_wooper, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_wooper, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(workOrderOperationsUpdated(int, int, bool)), this, SLOT(sCatchOperationsUpdated(int, int, bool)));

  _wo->setFocus();
}

workOrderOperations::~workOrderOperations()
{
    // no need to delete child widgets, Qt does it all for us
}

void workOrderOperations::languageChange()
{
  retranslateUi(this);
}

enum SetResponse workOrderOperations::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wooper->setFocus();
  }

  return NoError;
}

void workOrderOperations::sPopulateMenu(QMenu *menu)
{
  int menuItem;

  menuItem = menu->insertItem(tr("View Operation..."), this, SLOT(sViewOperation()), 0);
  if ((!_privileges->check("ViewWoOperations")) && (!_privileges->check("MaintainWoOperations")))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Edit Operation..."), this, SLOT(sEditOperation()), 0);
  if (!_privileges->check("MaintainWoOperations"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Delete Operation..."), this, SLOT(sDeleteOperation()), 0);
  if (!_privileges->check("MaintainWoOperations"))
    menu->setItemEnabled(menuItem, FALSE);
}

void workOrderOperations::sNewOperation()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("wo_id", _wo->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void workOrderOperations::sViewOperation()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void workOrderOperations::sEditOperation()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void workOrderOperations::sDeleteOperation()
{
  if (QMessageBox::question( this, tr("Delete W/O Operation"),
                             tr("<p>If you Delete the selected W/O Operation "
				"you will not be able to post Labor to this "
				"Operation to this W/O. Are you sure that you "
				"want to delete the selected W/O Operation?" ),
				QMessageBox::Yes,
				QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM wooper "
               "WHERE (wooper_id=:wooper_id);" );
    q.bindValue(":wooper_id", _wooper->id());
    q.exec();
    if (q.first())
      omfgThis->sWorkOrderOperationsUpdated(_wo->id(), _wooper->id(), TRUE);
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
}

void workOrderOperations::sMoveUp()
{
  q.prepare("SELECT moveWooperUp(:wooper_id) AS result;");
  q.bindValue(":wooper_id", _wooper->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("moveWooperUp", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sWorkOrderOperationsUpdated(_wo->id(), _wooper->id(), TRUE);
}

void workOrderOperations::sMoveDown()
{
  q.prepare("SELECT moveWooperDown(:wooper_id) AS result;");
  q.bindValue(":wooper_id", _wooper->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("moveWooperDown", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sWorkOrderOperationsUpdated(_wo->id(), _wooper->id(), TRUE);
}

void workOrderOperations::sCatchOperationsUpdated(int pWoid, int, bool)
{
  if (pWoid == _wo->id())
    sFillList();
}

void workOrderOperations::sFillList()
{
  q.prepare( "SELECT wooper_id, wooper_seqnumber, wrkcnt_code,"
             "       COALESCE(stdopn_number, :none),"
             "       (wooper_descrip1 || ' ' || wooper_descrip2),"
             "       CASE WHEN (wooper_sucomplete) THEN (formatTime(noNeg(wooper_sutime - wooper_suconsumed)) || '/' || :complete)"
             "            ELSE formatTime(noNeg(wooper_sutime - wooper_suconsumed))"
             "       END,"
             "       CASE WHEN (wooper_rncomplete) THEN (formatTime(noNeg(wooper_rntime - wooper_rnconsumed)) || '/' || :complete)"
             "            ELSE formatTime(noNeg(wooper_rntime - wooper_rnconsumed))"
             "       END "
             "FROM wrkcnt,"
             "     wooper LEFT OUTER JOIN stdopn ON (wooper_stdopn_id=stdopn_id) "
             "WHERE ( (wooper_wrkcnt_id=wrkcnt_id)"
             " AND (wooper_wo_id=:wo_id) ) "
             "ORDER BY wooper_seqnumber;" );
  q.bindValue(":none", tr("None"));
  q.bindValue(":complete", tr("Complete"));
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  _wooper->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
