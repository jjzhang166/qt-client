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

#include "accountingYearPeriod.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

accountingYearPeriod::accountingYearPeriod(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
}

accountingYearPeriod::~accountingYearPeriod()
{
    // no need to delete child widgets, Qt does it all for us
}

void accountingYearPeriod::languageChange()
{
    retranslateUi(this);
}

enum SetResponse accountingYearPeriod::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("yearperiod_id", &valid);
  if (valid)
  {
    _periodid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _startDate->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _startDate->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _startDate->setEnabled(FALSE);
      _endDate->setEnabled(FALSE);
      _closed->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      _close->setFocus();
    }
  }

  return NoError;
}

void accountingYearPeriod::sSave()
{
  if (_mode == cNew)
  {
    q.prepare("SELECT createAccountingYearPeriod(:startDate, :endDate) AS _period_id;");
    q.bindValue(":startDate", _startDate->date());
    q.bindValue(":endDate", _endDate->date());
    q.exec();
    if (q.first())
    {
      _periodid = q.value("_period_id").toInt();
      if (_periodid < 0)
      {
	systemError(this, storedProcErrorLookup("createAccountingYearPeriod", _periodid), __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (_mode == cEdit)
  {

    if ( (_cachedClosed) && (!_closed->isChecked()) )
    {
      q.prepare("SELECT openAccountingYearPeriod(:period_id) AS result;");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("openAccountingYearPeriod", result), __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    if ( (_cachedStartDate != _startDate->date()) ||
         (_cachedEndDate != _endDate->date()) )
    {
      q.prepare("SELECT changeAccountingYearPeriodDates(:period_id, :startDate, :endDate) AS result;");
      q.bindValue(":period_id", _periodid);
      q.bindValue(":startDate", _startDate->date());
      q.bindValue(":endDate", _endDate->date());
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("changeAccountingYearPeriodDates", result), __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    if ( (!_cachedClosed) && (_closed->isChecked()) )
    {
      q.prepare("SELECT closeAccountingYearPeriod(:period_id) AS result;");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("closeAccountingYearPeriod", result), __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }

  done(_periodid);
}

void accountingYearPeriod::populate()
{
  q.prepare( "SELECT yearperiod_start, yearperiod_end, yearperiod_closed "
             "FROM yearperiod "
             "WHERE (yearperiod_id=:period_id);" );
  q.bindValue(":period_id", _periodid);
  q.exec();
  if (q.first())
  {
    _cachedStartDate = q.value("yearperiod_start").toDate();
    _cachedEndDate = q.value("yearperiod_end").toDate();
    _cachedClosed = q.value("yearperiod_closed").toBool();

    _startDate->setDate(q.value("yearperiod_start").toDate());
    _endDate->setDate(q.value("yearperiod_end").toDate());
    _closed->setChecked(q.value("yearperiod_closed").toBool());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
