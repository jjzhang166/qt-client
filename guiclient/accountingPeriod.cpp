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

#include "accountingPeriod.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"
#include "unpostedGLTransactions.h"

accountingPeriod::accountingPeriod(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _year->setType(XComboBox::FiscalYears);
}

accountingPeriod::~accountingPeriod()
{
    // no need to delete child widgets, Qt does it all for us
}

void accountingPeriod::languageChange()
{
    retranslateUi(this);
}

enum SetResponse accountingPeriod::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("period_id", &valid);
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
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _startDate->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _startDate->setEnabled(FALSE);
      _endDate->setEnabled(FALSE);
      _closed->setEnabled(FALSE);
      _frozen->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      _close->setFocus();
    }
  }

  return NoError;
}

void accountingPeriod::sSave()
{
  if (_mode == cNew)
  {
    q.prepare("SELECT createAccountingPeriod(:startDate, :endDate, :yearperiod_id, :quarter) AS _period_id;");
    q.bindValue(":startDate", _startDate->date());
    q.bindValue(":endDate", _endDate->date());
    q.bindValue(":yearperiod_id", _year->id());
    q.bindValue(":quarter", _quarter->value());

    q.exec();
    if (q.first())
    {
      _periodid = q.value("_period_id").toInt();
      if (_periodid < 0)
      {
	systemError(this, storedProcErrorLookup("createAccountingPeriod", _periodid),
		    __FILE__, __LINE__);
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
    if ( (_cachedFrozen) && (!_frozen->isChecked()) )
    {
      q.prepare("SELECT thawAccountingPeriod(:period_id) AS result;");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("thawAccountingPeriod", result),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    if ( (_cachedClosed) && (!_closed->isChecked()) )
    {
      bool reallyOpen = false;

      q.prepare("SELECT COUNT(gltrans_sequence) AS count "
		"FROM gltrans, period "
		"WHERE ( (NOT gltrans_posted) "
		"AND (gltrans_date BETWEEN period_start AND period_end) "
		"AND (period_id=:period_id) );");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	if (q.value("count").toInt() <= 0)
	  reallyOpen = true;
	else
	{
	  ParameterList params;

	  unpostedGLTransactions newdlg(this, "", true);
	  params.append("period_id", _periodid);
	  newdlg.set(params);

	  reallyOpen = (newdlg.exec() == XDialog::Accepted);
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      if (reallyOpen)
      {
	q.prepare("SELECT openAccountingPeriod(:period_id) AS result;");
	q.bindValue(":period_id", _periodid);
	q.exec();
	if (q.first())
	{
	  int result = q.value("result").toInt();
	  if (result < 0)
	  {
	    systemError(this, storedProcErrorLookup("openAccountingPeriod", result),
			__FILE__, __LINE__);
	    return;
	  }
	}
	else if (q.lastError().type() != QSqlError::None)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
      else
	return;
    }

    if ( (_cachedStartDate != _startDate->date()) ||
         (_cachedEndDate != _endDate->date()) )
    {
      q.prepare("SELECT changeAccountingPeriodDates(:period_id, :startDate, :endDate) AS result;");
      q.bindValue(":period_id", _periodid);
      q.bindValue(":startDate", _startDate->date());
      q.bindValue(":endDate", _endDate->date());
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("changeAccountingPeriodDates", result),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    if ( (!_cachedFrozen) && (_frozen->isChecked()) )
    {
      q.prepare("SELECT freezeAccountingPeriod(:period_id) AS result;");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("freezeAccountingPeriod", result),
		      __FILE__, __LINE__);
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
      q.prepare("SELECT closeAccountingPeriod(:period_id) AS result;");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("closeAccountingPeriod", result),
		      __FILE__, __LINE__);
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

  if( (_cachedName != _name->text()) || (_cachedYearPeriodId != _year->id()) || (_cachedQuarter != _quarter->value()) )
  {
    q.prepare("SELECT yearperiod_id "
              "FROM yearperiod "
              "WHERE ((yearperiod_id=:yearperiod_id) "
              "AND (:startDate>=yearperiod_start) "
              " AND (:endDate<=yearperiod_end)); ");
    q.bindValue(":yearperiod_id", _year->id());
    q.bindValue(":startDate", _startDate->date());
    q.bindValue(":endDate", _endDate->date());
    q.exec();
    if (q.first())
    {
      q.prepare("UPDATE period SET period_name=:period_name,"
		"                  period_yearperiod_id=:yearperiod_id,"
		"                  period_quarter=:quarter"
                " WHERE (period_id=:period_id); ");
      q.bindValue(":period_id", _periodid);
      q.bindValue(":period_name", _name->text());
      q.bindValue(":yearperiod_id", _year->id());
      q.bindValue(":quarter",	_quarter->value());
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      QMessageBox::critical( this, tr("Cannot Save Period"),
            tr("Period dates are outside the selected Fiscal Year.") );
      return;
    }
  }

  done(_periodid);
}

void accountingPeriod::populate()
{
  q.prepare( "SELECT period_start, period_end, period_closed, period_freeze,"
             "       period_name, period_yearperiod_id, period_quarter "
             "FROM period "
             "WHERE (period_id=:period_id);" );
  q.bindValue(":period_id", _periodid);
  q.exec();
  if (q.first())
  {
    _cachedStartDate = q.value("period_start").toDate();
    _cachedEndDate = q.value("period_end").toDate();
    _cachedClosed = q.value("period_closed").toBool();
    _cachedFrozen = q.value("period_freeze").toBool();
    _cachedName = q.value("period_name").toString();
    _cachedYearPeriodId = q.value("period_yearperiod_id").toInt();
    _cachedQuarter = q.value("period_quarter").toInt();

    _startDate->setDate(q.value("period_start").toDate());
    _endDate->setDate(q.value("period_end").toDate());
    _closed->setChecked(q.value("period_closed").toBool());
    _frozen->setChecked(q.value("period_freeze").toBool());
    _name->setText(q.value("period_name").toString());
    _year->setId(q.value("period_yearperiod_id").toInt());
    _quarter->setValue(q.value("period_quarter").toInt());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
