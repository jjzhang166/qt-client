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

#include "woTimeClock.h"

#include <QMessageBox>
#include <QSqlError>
#include <QTimer>
#include <QVariant>
#include <metasql.h>

#include "inputManager.h"
#include "postOperations.h"
#include "postProduction.h"
#include "scrapWoMaterialFromWIP.h"
#include "storedProcErrorLookup.h"

// current _wooperList selection has precedence
#define WOOPERID ((_wooperList->currentItem() == 0) ? _wooper->id() : _wooperList->id())
#define WOID ((_wooperList->currentItem() == 0) ? _wo->id() : _wooperList->altId())

woTimeClock::woTimeClock(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_clockIn, SIGNAL(clicked()), this, SLOT(sClockIn()));
  connect(_clockOut, SIGNAL(clicked()), this, SLOT(sClockOut()));
  connect(_lastEvent, SIGNAL(textChanged(const QString&)), this, SLOT(sSetTimer()));
  connect(_user, SIGNAL(valid(bool)), this, SLOT(sCheckValid()));
  connect(_wo, SIGNAL(valid(bool)), this, SLOT(sCheckValid()));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sPopulateWooper()));
  connect(_wooper, SIGNAL(newID(int)), this, SLOT(sCheckValid()));
  connect(_wooperList, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));

  _captive = false;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));
  omfgThis->inputManager()->notify(cBCWorkOrderOperation, this, this, SLOT(sWooperScanned(int)));
  omfgThis->inputManager()->notify(cBCUser, this, _user, SLOT(setId(int)));

  _wooperLit-> setHidden(! (_metrics->value("WOTCPostStyle") == "Operations"));
  _wooper->    setHidden(! (_metrics->value("WOTCPostStyle") == "Operations"));
  
  _wooperList->addColumn(tr("W/O #"),	  _orderColumn,    Qt::AlignLeft, true, "wonum");
  _wooperList->addColumn(tr("Operation"), -1,              Qt::AlignLeft, true, "woseq");
  _wooperList->addColumn(tr("Clock In"),  _timeDateColumn, Qt::AlignLeft, true, "wotc_timein");

  if (! (_metrics->value("WOTCPostStyle") == "Operations"))
    _wooperList->hideColumn(1);

  _wo->setType(cWoReleased | cWoIssued);
  _wooper->setAllowNull(true);

  sCheckValid();
}

woTimeClock::~woTimeClock()
{
  // no need to delete child widgets, Qt does it all for us
}

void woTimeClock::languageChange()
{
  retranslateUi(this);
}

enum SetResponse woTimeClock::set(ParameterList& pParams)
{
  QString  returnValue;
  QVariant param;
  bool     valid;
  
  // if the app is started with this as the main window then the startup
  // code should call set() with "captive" true after constructing this window
  param = pParams.value("captive", &valid);
  if (valid)
  {
    _captive = true;
    _close->setText(tr("Quit"));
  }
  
  return NoError;
}

bool woTimeClock::close(bool alsoDelete)
{
  if (_captive)
  {
    int answer = QMessageBox::question(this, tr("Quit the application?"),
				       tr("Are you sure you want to quit the application?"),
				       QMessageBox::Yes,
				       QMessageBox::No | QMessageBox::Default);
    if (answer == QMessageBox::No)
      return false;
  }
  
  return XMainWindow::close(alsoDelete);
}

void woTimeClock::sClockIn()
{
  QString statusMsg = tr("User %1 clocked in to %2 at %3")
		      .arg(_user->username())
		      .arg(_wo->woNumber())
		      .arg(QDateTime::currentDateTime().toString());
  
  q.prepare("SELECT woClockIn(:wotc_wo_id, :wotc_usr_id, NOW(), "
	    ":wotc_wooper_id) AS result;");
  q.bindValue(":wotc_wo_id", WOID);
  q.bindValue(":wotc_usr_id", _user->id());
  q.bindValue(":wotc_wooper_id", WOOPERID);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this,
		storedProcErrorLookup("woClockIn", result).arg(_wo->woNumber()),
		__FILE__, __LINE__);
      statusMsg = statusMsg + "<br>" + 
		storedProcErrorLookup("woClockIn", result).arg(_wo->woNumber());
      return;
    }
    omfgThis->sWorkOrdersUpdated(WOID, TRUE);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _lastEvent->setText(statusMsg);
  clear();
}

void woTimeClock::sClockOut()
{
  int wotc_id = -1;
  q.prepare("SELECT woClockOut(:wotc_wo_id, :wotc_usr_id, NOW(),"
	    "                  :wotc_wooper_id) AS result;");
  q.bindValue(":wotc_wo_id", WOID);
  q.bindValue(":wotc_usr_id", _user->id());
  q.bindValue(":wotc_wooper_id", WOOPERID);

  q.exec();
  if (q.first())
  {
    wotc_id = q.value("result").toInt();
    if (wotc_id < 0)
    {
      systemError(this, storedProcErrorLookup("woClockOut", wotc_id), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _lastEvent->setText(tr("User %1 clocked out of %2 at %3\n")
		      .arg(_user->username())
		      .arg(_wo->woNumber())
		      .arg(QDateTime::currentDateTime().toString()));

  int dlgResult = XDialog::Accepted;
  if (_metrics->value("WOTCPostStyle") == "Operations")
    dlgResult = callPostOperations(wotc_id);
  else if (_metrics->value("WOTCPostStyle") == "Production")
    dlgResult = callPostProduction();

  if (dlgResult == XDialog::Accepted)
    sScrap();
  else // cancel the clockout
  {
    q.prepare("SELECT unWoClockOut(:wotc_id) AS result;");
    q.bindValue(":wotc_id", wotc_id);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
	systemError(this, storedProcErrorLookup("unWoClockOut", result), __FILE__, __LINE__);
    }
    else if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

    _lastEvent->setText(tr("User %1 canceled clock out of %2 at %3\n")
			.arg(_user->username())
			.arg(_wo->woNumber())
			.arg(QDateTime::currentDateTime().toString()));
  }

  clear();
}

int woTimeClock::callPostProduction()
{
  int returnCode;
  
  ParameterList params;
  params.append("wo_id", WOID);
  params.append("usr_id", _user->id());
  params.append("backflush", true);
  params.append("fromWOTC", true);
  
  postProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  returnCode = newdlg.exec();
  if (returnCode == XDialog::Accepted)
    _lastEvent->setText(tr("User %1 posted production at %2\n")
			.arg(_user->username())
			.arg(QDateTime::currentDateTime().toString()));
  return returnCode;
}

int woTimeClock::callPostOperations(int wotc_id)
{
  int returnCode;
  
  ParameterList params;
  params.append("usr_id", _user->id());
  if (wotc_id != -1)
    params.append("wotc_id", wotc_id);
  else if (WOOPERID != -1)
    params.append("wooper_id", WOOPERID);
  else
    params.append("wo_id", WOID);
  params.append("issueComponents", true);
  params.append("fromWOTC", true);
  
  postOperations newdlg(this, "", TRUE);
  newdlg.set(params);
  returnCode = newdlg.exec();
  if (returnCode == XDialog::Accepted)
    _lastEvent->setText(tr("User %1 posted operation at %2\n")
			.arg(_user->username())
			.arg(QDateTime::currentDateTime().toString()));
  return returnCode;
}

void woTimeClock::sPostProduction()
{
  callPostProduction();
}

void woTimeClock::sScrap()
{
  ParameterList params;
  params.append("wo_id", WOID);
  params.append("fromWOTC", true);
  
  int line = __LINE__;
  if (_metrics->value("WOTCPostStyle") == "Production")
  {
    q.prepare("SELECT wo_qtyrcv AS qty FROM wo WHERE wo_id=:wo_id;");
    q.bindValue(":wo_id", WOID);
    line = __LINE__;
  }
  else if (_metrics->value("WOTCPostStyle") == "Operations")
  {
    params.append("wooper_id", WOOPERID);

    q.prepare("SELECT wooper_rcvinv AS qty FROM wooper WHERE wooper_id=:wooper_id;");
    q.bindValue(":wooper_id", WOOPERID);
    line = __LINE__;
  }
  q.exec();
  if (q.first())
    params.append("allowTopLevel", q.value("qty").toInt() > 0);
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, line);

  scrapWoMaterialFromWIP newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    _lastEvent->setText(tr("User %1 entered scrap at %2\n")
			.arg(_user->username())
			.arg(QDateTime::currentDateTime().toString()));
}

void woTimeClock::clear()
{
  _user->setId(-1);	// _user->clear();
  _wo->setId(-1);	// _wo->clear();
  _wooperList->clear();
}

void woTimeClock::sCheckValid()
{
  if (_user->isValid())
  {
    QString sql = "SELECT wotc_wooper_id, wotc_wo_id, formatWoNumber(wo_id) AS wonum,"
		  "      (wooper_seqnumber || ' - ' || wooper_descrip1 ||"
		  "                  ' - ' || wooper_descrip2) AS woseq,"
		  "      wotc_timein "
		  "FROM wo, wotc LEFT OUTER JOIN"
		  "     wooper ON (wooper_id=wotc_wooper_id) "
		  "WHERE ((wo_id=wotc_wo_id)"
		  "  AND  (wotc_usr_id= <? value(\"usr_id\") ?> ) "
		  "  AND  (wotc_timeout IS NULL)"
		  "  <? if exists(\"wo_id\") ?> AND (wotc_wo_id= <? value(\"wo_id\" ?> ) <? endif ?>"
		  ");";

    ParameterList params;
    params.append("usr_id", _user->id());
    if (_wo->isValid())
      params.append("wo_id",  _wo->id());	// not WOID

    MetaSQLQuery mql(sql);
    XSqlQuery cnt = mql.toQuery(params);
    _wooperList->populate(cnt, true);
    if (cnt.lastError().type() != QSqlError::NoError)
    {
      systemError(this, cnt.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (_wooperList->topLevelItemCount() > 0)
      _clockOut->setFocus();
    else
      _clockIn->setFocus();
  }
  else
    _wooperList->clear();

  sHandleButtons(); // not signal/slot to ensure correct order
}

void woTimeClock::sHandleButtons()
{
  if (_metrics->value("WOTCPostStyle") == "Operations")
    _wooper->setEnabled(_wo->isValid() && _wooperList->currentItem() == 0);

  if (_metrics->value("WOTCPostStyle") == "Operations")
  {
    _clockIn-> setEnabled(_user->isValid() && _wooper->id() > -1 && // !WOOPERID
			  _wooperList->findItems(_wooper->currentText(),
						  Qt::MatchExactly).empty());
    _clockOut->setEnabled(_user->isValid() && WOOPERID > -1 &&
			   (!_wooperList->findItems(_wooper->currentText(),
						  Qt::MatchExactly).empty() ||
			    _wooper->currentText().isEmpty()));
  }
  else
  {
    _clockIn-> setEnabled(_user->isValid() && _wo->id() > -1 &&	// !WOID
			  _wooperList->findItems(_wo->woNumber(),
						  Qt::MatchExactly).empty());
    _clockOut->setEnabled(_user->isValid() && (_wooperList->altId() > 0 || //!WOID
			   (_wo->isValid() &&
			    _wooperList->currentItem() == 0 &&
			    !_wooperList->findItems(_wo->woNumber(),
						    Qt::MatchExactly).empty())));
  }

}

void woTimeClock::sSetTimer()
{
  if (! _lastEvent->text().isEmpty())
    QTimer::singleShot(10 * 1000, _lastEvent, SLOT(clear()));
}

void woTimeClock::sPopulateWooper()
{
  if (_metrics->value("WOTCPostStyle") == "Operations")
  {
     QString sql("SELECT wooper_id, (wooper_seqnumber || ' - ' || wooper_descrip1 || ' - ' || wooper_descrip2) "
		 "FROM wooper "
		 "WHERE ((wooper_wo_id=%1) "
		 "  AND  (NOT wooper_rncomplete)) "
		 "ORDER BY wooper_seqnumber;");
     XSqlQuery query;
     query.exec(sql.arg(_wo->id()));	// not WOID
     _wooper->populate(query);
     if (query.lastError().type() != QSqlError::NoError)
     {
       systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
       return;
     }

     query.prepare("SELECT wotc_wooper_id "
		   "FROM wotc "
		   "WHERE ((wotc_usr_id=:usr_id)"
		   "  AND  (wotc_wo_id=:wo_id)"
		   "  AND  (wotc_timeout IS NULL));");
     query.bindValue(":usr_id", _user->id());
     query.bindValue(":wo_id", _wo->id());	// not WOID
     query.exec();
     if (query.lastError().type() != QSqlError::NoError)
       systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
     else
       _wooper->setId(-1);
   }
}

void woTimeClock::sWooperScanned(int sWooperid)
{
  _wooper->setId(sWooperid);
  XSqlQuery wo;
  wo.prepare("SELECT wooper_wo_id FROM wooper WHERE (wooper_id=:wooper_id);");
  wo.bindValue(":wooper_id", sWooperid);
  if (wo.exec() && wo.first())
  {
    _wo->setId(wo.value("wooper_wo_id").toInt());
    _wooper->setId(sWooperid);
    if (_wooper->id() == -1)
      _lastEvent->setText(tr("Scanned Work Order Operation is not valid. "
			     "It may have been marked Complete."));
    else
      _lastEvent->setText(tr("Scanned Work Order Operation %1 (%2).")
			    .arg(_wooper->currentText())
			    .arg(sWooperid));
  }
  else if (wo.lastError().type() != QSqlError::NoError)
    systemError(this, wo.lastError().databaseText(), __FILE__, __LINE__);
  else if (wo.size() <= 0 || wo.value("wooper_wo_id").isNull())
    _lastEvent->setText(tr("Scanned Work Order Operation does not exist.\n"
			   "The Work Order may be closed"));
}
