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

#include "batchManager.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include "metasql.h"
#include "xsqlquery.h"

#include "batchItem.h"

batchManager::batchManager(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  QButtonGroup * buttonGroup = new QButtonGroup(this);
  buttonGroup->addButton(_currentUser);
  buttonGroup->addButton(_allUsers);
  buttonGroup->addButton(_selectedUser);

  connect(_autoUpdate,   SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_batch, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_cancel,       SIGNAL(clicked()),          this, SLOT(sCancel()));
  connect(_reschedule,   SIGNAL(clicked()),          this, SLOT(sReschedule()));
  connect(_showCompleted,SIGNAL(toggled(bool)),      this, SLOT(sFillList()));
  connect(_usr,          SIGNAL(newID(int)),         this, SLOT(sFillList()));
  connect(buttonGroup,   SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));

  _db = QSqlDatabase();
  
  _timer = new QTimer(this);

  _batch->addColumn( tr("User"),          _dateColumn, Qt::AlignLeft, true, "batch_user");
  _batch->addColumn( tr("Action"),        _itemColumn, Qt::AlignLeft, true, "batch_action");
  _batch->addColumn( tr("Scheduled"), _timeDateColumn, Qt::AlignLeft, true, "batch_scheduled");
  _batch->addColumn( tr("Run Status"),_timeDateColumn, Qt::AlignLeft, true, "runstatus");
  _batch->addColumn( tr("Completed"), _timeDateColumn, Qt::AlignLeft, true, "batch_completed");
  _batch->addColumn( tr("Exit Status"),            -1, Qt::AlignLeft, true, "exitstatus" );

  setDatabase(_db);
}

batchManager::~batchManager()
{
  // no need to delete child widgets, Qt does it all for us
}

void batchManager::languageChange()
{
  retranslateUi(this);
}

void batchManager::sPopulateMenu(QMenu *)
{
}

void batchManager::sFillList()
{
  QString sql( "SELECT batch_id, *,"
               "       CASE WHEN (batch_started IS NULL)   THEN <? value(\"scheduled\") ?>"
               "            WHEN (batch_completed IS NULL) THEN <? value(\"inProcess\") ?>"
               "            ELSE <? value(\"completed\") ?>"
               "       END AS runstatus,"
               "       firstLine(batch_exitstatus) AS exitstatus "
               "FROM batch, usr "
               "WHERE ( (batch_user=usr_username)"
               "<? if not exists(\"showCompleted\") ?>"
               "    AND (batch_completed IS NULL)"
               "<? endif ?>"
               "<? if exists(\"userid\") ?>"
               "    AND (usr_id=<? value(\"userid\") ?>)"
               "<? elseif exists(\"current_user\") ?>"
               "    AND (usr_username=CURRENT_USER)"
               "<? endif ?>"
               ") "
               "ORDER BY batch_scheduled, batch_completed;" );

  ParameterList params;
  params.append("scheduled", tr("Scheduled"));
  params.append("inProcess", tr("In-Process"));
  params.append("completed", tr("Done"));
  if (_showCompleted->isChecked())
    params.append("showCompleted");
  if (_selectedUser->isChecked())
    params.append("userid", _usr->id());
  else if(_currentUser->isChecked())
    params.append("current_user");

  XSqlQuery batch(_db);
  MetaSQLQuery mql(sql);
  batch = mql.toQuery(params);
  _batch->populate(batch);
  if (batch.lastError().type() != QSqlError::None)
  {
    QMessageBox::critical(this,
                          tr("System Error in %1::%2").arg(__FILE__, __LINE__),
                          batch.lastError().databaseText());
    return;
  }

  _timer->stop();
  if (_autoUpdate->isChecked())
    _timer->singleShot(60000, this, SLOT(sFillList()));
}


void batchManager::sReschedule()
{
  if (batchItem::Reschedule(_batch->id(), this, _db) != QDialog::Rejected)
    sFillList();
}


void batchManager::sCancel()
{
  XSqlQuery cancel(_db);
  cancel.prepare("SELECT cancelBatchItem(:batch_id) AS result;");
  cancel.bindValue(":batch_id", _batch->id());
  cancel.exec();

  sFillList();
}


void batchManager::setDatabase( QSqlDatabase db )
{
  //if(_db.isValid())
  //  disconnect(&_db, SIGNAL(destroyed()), this, SLOT(close()));

  _db = db;
  if(_db.isValid())
    setCaption(tr("Batch Manager -- on %1/%2 AS %3")
               .arg(_db.hostName())
               .arg(_db.databaseName())
               .arg(_db.userName()));
  else
    setCaption(tr("Batch Manager"));
  
  //if(_db.isValid())
  //  connect(&_db, SIGNAL(destroyed()), this, SLOT(close()));

  XSqlQuery users( "SELECT usr_id, usr_username, usr_username"
                   "  FROM usr "
                   " ORDER BY usr_username", _db );
  _usr->populate(users);
  
  sFillList();
}

void batchManager::setViewOtherEvents( bool viewOtherEvents )
{
  _allUsers->setEnabled(viewOtherEvents);
  _selectedUser->setEnabled(viewOtherEvents);
  if (!viewOtherEvents)
    _currentUser->setChecked(TRUE);
}
