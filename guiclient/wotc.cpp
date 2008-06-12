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

#include "wotc.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

wotc::wotc(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save,   SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_user, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_wo,    SIGNAL(newId(int)), this, SLOT(sPopulateWooper()));
  connect(_wo,   SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));

  _wooper->setAllowNull(true);
  _wooper->setHidden(! (_metrics->value("WOTCPostStyle") == "Operations"));
}

wotc::~wotc()
{
  // no need to delete child widgets, Qt does it all for us
}

void wotc::languageChange()
{
    retranslateUi(this);
}

enum SetResponse wotc::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _wo->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _close->setText(tr("&Close"));
      _save->hide();
      _wo->setEnabled(false);
      _user->setEnabled(false);
      _close->setFocus();
    }
  }
  
  param = pParams.value("wotc_id", &valid);
  if (valid)
    _wotcid = param.toInt();
  
  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  param = pParams.value("usr_id", &valid);
  if (valid)
    _user->setId(param.toInt());

  populate();

  return NoError;
}

void wotc::sHandleButtons()
{
  bool enable = _wo->isValid() && _user->isValid() &&
		(_mode == cNew || _mode == cEdit);
  
  _dateIn->setEnabled(enable);
  _dateOut->setEnabled(enable);
  _timeIn->setEnabled(enable);
  _timeOut->setEnabled(enable);
  _save->setEnabled(enable);
}

void wotc::populate()
{
  if (_wotcid != -1)
  {
    XSqlQuery pop;
    pop.prepare("SELECT wotc_wo_id, wotc_usr_id,"
	        "       wotc_timein, wotc_timeout,"
	        "       wotc_wooper_id "
	        "FROM wotc "
	        "WHERE (wotc_id=:wotc_id);");
    pop.bindValue(":wotc_id", _wotcid);
    pop.exec();
    if (pop.first())
    {
      _wo->setId(pop.value("wotc_wo_id").toInt());
      _user->setId(pop.value("wotc_usr_id").toInt());
      _dateIn->setDate(pop.value("wotc_timein").toDate());
      _dateOut->setDate(pop.value("wotc_timeout").toDate());
      _timeIn->setTime(pop.value("wotc_timein").toTime());
      _timeOut->setTime(pop.value("wotc_timeout").toTime());
      if (! pop.value("wotc_wooper_id").isNull())
	_wooper->setId(pop.value("wotc_wooper_id").toInt());
    }
    else if (pop.lastError().type() != QSqlError::NoError)
      systemError(this, pop.lastError().databaseText(), __FILE__, __LINE__);
  }
}

void wotc::sSave()
{
  QWidget* invalidWidget = 0;
  QString msg;
  
  QDateTime dtIn(_dateIn->date(), _timeIn->time());
  QDateTime dtOut(_dateOut->date(), _timeOut->time());
  
  if (! _wo->isValid())
  {
    msg = tr("Please specify a work order before saving this time clock entry.");
    invalidWidget = _wo;
  }
  else if (! _user->isValid())
  {
    msg = tr("Please specify a valid user before saving this time clock entry.");
    invalidWidget = _user;
  }
  else if (! _dateIn->isValid())			// see mantis bug 4341
  {
    msg = tr("Please specify a valid clock in date/time.");
    invalidWidget = _dateIn;
  }
  else if (_dateOut->isValid() && dtIn > dtOut)		// see mantis bug 4341
  {
    msg = tr("Please specify a clock in date/time earlier than the clock out date/time.");
    invalidWidget = _dateIn;
  }
  else if (! _dateOut->isValid())
  {
    QDate tmp;
    dtOut.setDate(tmp);					// see mantis bug 4341
  }
  
  if (invalidWidget)
  {
    QMessageBox::critical(this, tr("Invalid Work Order Time Clock Entry"), msg);
    invalidWidget->setFocus();
    return;
  }
  
  if (_mode == cNew)
    q.prepare("INSERT INTO wotc (wotc_wo_id, wotc_usr_id,"
	      "                  wotc_timein, wotc_timeout, wotc_wooper_id)"
	      "    VALUES (:wotc_wo_id, :wotc_usr_id, "
	      "            :wotc_timein, :wotc_timeout, :wotc_wooper_id);");
  else if (_mode == cEdit)
  {
    q.prepare("UPDATE wotc SET"
	      " wotc_wo_id=:wotc_wo_id, wotc_usr_id=:wotc_usr_id,"
	      " wotc_timein=:wotc_timein, wotc_timeout=:wotc_timeout,"
	      " wotc_wooper_id=:wotc_wooper_id "
	      "WHERE (wotc_id=:wotc_id);");
    q.bindValue(":wotc_id", _wotcid);
  }
  q.bindValue(":wotc_wo_id", _wo->id());
  q.bindValue(":wotc_usr_id", _user->id());
  q.bindValue(":wotc_timein", dtIn);
  q.bindValue(":wotc_timeout", (dtOut.isNull()) ? "" : dtOut.toString()); // bug 6655

  if (_wooper->id() != -1)
    q.bindValue(":wotc_wooper_id", _wooper->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  accept();
}

void wotc::sPopulateWooper()
{
  // apparent timing problem using _wo->status()
  QString woStatus;
  q.prepare("SELECT wo_status FROM wo WHERE (wo_id=:wo_id);");
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first())
    woStatus = q.value("wo_status").toString();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (woStatus == "C")
  {
    _wooper->setEnabled(false);
    _wooper->clear();
  }
  else if (_metrics->value("WOTCPostStyle") == "Operations")
  {
    _wooper->setEnabled(_mode == cEdit || _mode == cNew);
     QString sql("SELECT wooper_id, (wooper_seqnumber || ' - ' || wooper_descrip1 || ' - ' || wooper_descrip2) "
		 "FROM wooper "
		 "WHERE ((wooper_wo_id=%1) "
		 // "  AND  (NOT wooper_rncomplete)) "
		 ") "
		 "ORDER BY wooper_seqnumber;");
     XSqlQuery query;
     query.exec(sql.arg(_wo->id()));
     if (query.lastError().type() != QSqlError::NoError)
     {
       systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
       return;
     }
     _wooper->populate(query);

     if (_wotcid != -1)
     {
       query.prepare("SELECT wotc_wooper_id "
		     "FROM wotc "
		     "WHERE ((wotc_usr_id=:usr_id)"
		     "  AND  (wotc_wo_id=:wo_id)"
		     "  AND  (wotc_timeout IS NULL));");
       query.bindValue(":usr_id", _user->id());
       query.bindValue(":wo_id", _wo->id());
       query.exec();
       if (query.first())
	 _wooper->setId(query.value("wotc_wooper_id").toInt());
       else if (query.lastError().type() != QSqlError::NoError)
	 systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
       else
	 _wooper->setId(_wooper->id(1));
     }
   }
}
