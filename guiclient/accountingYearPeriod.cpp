/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "accountingYearPeriod.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"
#include "errorReporter.h"

accountingYearPeriod::accountingYearPeriod(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
    connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
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
  XDialog::set(pParams);
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
    XSqlQuery check;
    check.exec("SELECT yearperiod_id "
               "FROM yearperiod "
               "WHERE (yearperiod_closed) "
               "LIMIT 1; ");
    if (check.first())
    {
      _startDate->setEnabled(false);
    }

    if (param.toString() == "new")
    {
      _mode = cNew;

      check.exec("SELECT yearperiod_end + 1 AS start_date "
                 "FROM yearperiod "
                 "ORDER BY yearperiod_end DESC "
                 "LIMIT 1; ");
      if (check.first())
      {
        _startDate->setDate(check.value("start_date").toDate());
        _endDate->setDate(check.value("start_date").toDate().addYears(1).addDays(-1));
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _startDate->setEnabled(false);
      _endDate->setEnabled(false);
      _closed->setEnabled(false);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void accountingYearPeriod::sSave()
{
  XSqlQuery saveAccountingYear;
  if (_mode == cNew)
  {
    saveAccountingYear.prepare("SELECT createAccountingYearPeriod(:startDate, :endDate) AS _period_id;");
    saveAccountingYear.bindValue(":startDate", _startDate->date());
    saveAccountingYear.bindValue(":endDate", _endDate->date());
    saveAccountingYear.exec();
    if (saveAccountingYear.first())
    {
      _periodid = saveAccountingYear.value("_period_id").toInt();
       if (_periodid < 0)
      {
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Fiscal Year"),
                               storedProcErrorLookup("createAccountingYearPeriod", _periodid),
                               __FILE__, __LINE__);
          return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Fiscal Year"),
                                  saveAccountingYear, __FILE__, __LINE__))
    {
        return;
    }
  }
  else if (_mode == cEdit)
  {

    if ( (_cachedClosed) && (!_closed->isChecked()) )
    {
      saveAccountingYear.prepare("SELECT openAccountingYearPeriod(:period_id) AS result;");
      saveAccountingYear.bindValue(":period_id", _periodid);
      saveAccountingYear.exec();
      if (saveAccountingYear.first())
      {
	int result = saveAccountingYear.value("result").toInt();
	if (result < 0)
    {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Opening Fiscal Year"),
                             storedProcErrorLookup("openAccountingYearPeriod", result),
                             __FILE__, __LINE__);
        return;
    }
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Opening Fiscal Year"),
                                    saveAccountingYear, __FILE__, __LINE__))
      {
          return;
      }
    }

    if ( (_cachedStartDate != _startDate->date()) ||
         (_cachedEndDate != _endDate->date()) )
    {
      saveAccountingYear.prepare("SELECT changeAccountingYearPeriodDates(:period_id, :startDate, :endDate) AS result;");
      saveAccountingYear.bindValue(":period_id", _periodid);
      saveAccountingYear.bindValue(":startDate", _startDate->date());
      saveAccountingYear.bindValue(":endDate", _endDate->date());
      saveAccountingYear.exec();
      if (saveAccountingYear.first())
      {
	int result = saveAccountingYear.value("result").toInt();
	if (result < 0)
    {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Changing FY Dates"),
                             storedProcErrorLookup("changeAccountingYearPeriodDates", result),
                             __FILE__, __LINE__);
        return;
    }
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Changing FY Dates"),
                                    saveAccountingYear, __FILE__, __LINE__))
      {
          return;
      }
    }

    if ( (!_cachedClosed) && (_closed->isChecked()) )
    {
      saveAccountingYear.prepare("SELECT closeAccountingYearPeriod(:period_id) AS result;");
      saveAccountingYear.bindValue(":period_id", _periodid);
      saveAccountingYear.exec();
      if (saveAccountingYear.first())
      {
	int result = saveAccountingYear.value("result").toInt();
	if (result < 0)
    {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Closing FY Period"),
                             storedProcErrorLookup("closeAccountingYearPeriod", _periodid),
                             __FILE__, __LINE__);
        return;
    }
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Closing FY Period"),
                                    saveAccountingYear, __FILE__, __LINE__))
      {
          return;
      }
    }
  }

  done(_periodid);
}

void accountingYearPeriod::populate()
{
  XSqlQuery populateAccountingYear;
  populateAccountingYear.exec( "SELECT FIRST(yearperiod_id) AS first_yearperiod_id, "
            "  LAST(yearperiod_id) AS last_yearperiod_id "
            "FROM (SELECT yearperiod_id FROM yearperiod "
            "      ORDER BY yearperiod_start) AS data; ");
  if (populateAccountingYear.first())
  {
    if (populateAccountingYear.value("first_yearperiod_id").toInt() != _periodid)
      _startDate->setEnabled(false);
    if (populateAccountingYear.value("last_yearperiod_id").toInt() != _periodid)
      _endDate->setEnabled(false);
  }

  populateAccountingYear.prepare( "SELECT yearperiod_start, yearperiod_end, yearperiod_closed "
             "FROM yearperiod "
             "WHERE (yearperiod_id=:period_id);" );
  populateAccountingYear.bindValue(":period_id", _periodid);
  populateAccountingYear.exec();
  if (populateAccountingYear.first())
  {
    _cachedStartDate = populateAccountingYear.value("yearperiod_start").toDate();
    _cachedEndDate = populateAccountingYear.value("yearperiod_end").toDate();
    _cachedClosed = populateAccountingYear.value("yearperiod_closed").toBool();

    _startDate->setDate(populateAccountingYear.value("yearperiod_start").toDate());
    _endDate->setDate(populateAccountingYear.value("yearperiod_end").toDate());
    _closed->setChecked(populateAccountingYear.value("yearperiod_closed").toBool());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Populating FY Periods"),
                                populateAccountingYear, __FILE__, __LINE__))
  {
      return;
  }
}
