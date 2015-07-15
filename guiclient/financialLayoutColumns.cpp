/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "financialLayoutColumns.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
//#include <QWorkspace>

#include <openreports.h>
#include <reporthandler.h>

financialLayoutColumns::financialLayoutColumns(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_month, SIGNAL(toggled(bool)), this, SLOT(sToggleMonth()));
  connect(_quarter, SIGNAL(toggled(bool)), this, SLOT(sToggleQuarter()));
  connect(_year, SIGNAL(toggled(bool)), this, SLOT(sToggleYear()));
  connect(_fullmonth, SIGNAL(toggled(bool)), this, SLOT(sTogglePrior()));
  connect(_fullquarter, SIGNAL(toggled(bool)), this, SLOT(sTogglePrior()));
  connect(_fullyear, SIGNAL(toggled(bool)), this, SLOT(sTogglePrior()));
  connect(_yeartodate, SIGNAL(toggled(bool)), this, SLOT(sTogglePrior()));
  connect(_fullyear, SIGNAL(toggled(bool)), this, SLOT(sToggleYearToDate()));
  connect(_yeartodate, SIGNAL(toggled(bool)), this, SLOT(sToggleFullYear()));

  languageChange();
}

financialLayoutColumns::~financialLayoutColumns()
{
  // no need to delete child widgets, Qt does it all for us
}

void financialLayoutColumns::languageChange()
{
  retranslateUi(this);
}

enum SetResponse financialLayoutColumns::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("flhead_id", &valid);
  if (valid)
  {
    _flheadid = param.toInt();
  }

  param = pParams.value("flcol_id", &valid);
  if (valid)
  {
    _flcolid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      populate();
    }
    else if (param.toString() == "view")
    {
      _name->setEnabled(false);
      _descrip->setEnabled(false);
      _report->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _selected->setEnabled(false);
      _prior->setEnabled(false);
      _budget->setEnabled(false);
      _showdb->setEnabled(false);
    }
  }
  
  param = pParams.value("type", &valid);
  if (valid)
  {
    if (param.toString() == "balance")
    {
      _month->setChecked(true);
      _month->setEnabled(false);
      _fullmonth->setEnabled(true);
      _fullquarter->setEnabled(true);
      _fullyear->setEnabled(true);
      _yeartodate->setEnabled(true);
      _quarter->setHidden(true);
      _year->setHidden(true);
      _yeartodate->setHidden(true);
      _fullmonth->setText(tr("Month End"));
      _fullquarter->setText(tr("Quarter End"));
      _fullyear->setText(tr("Year End"));
    }
    if (param.toString() != "cash")
      _showdb->setHidden(true);
  }
  
  return NoError;
}

void financialLayoutColumns::sSave()
{ 
  XSqlQuery financialSave;
  QString sql;

  if ((!_month->isChecked()) && (!_quarter->isChecked()) && (!_year->isChecked()))
  {
    QMessageBox::critical( this, tr("Cannot Save settings"),
              tr("<p>At least one of Month, Date or Year must be selected.") );
    return;
  }
      
  if (_mode == cNew)
  {
    financialSave.exec("SELECT NEXTVAL('flcol_flcol_id_seq') AS flcol_id;");
    if (financialSave.first())
      _flcolid = financialSave.value("flcol_id").toInt();
    else if (financialSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, financialSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
      
    sql = ( "INSERT INTO flcol "
        "(flcol_id,flcol_flhead_id,flcol_name,flcol_descrip,"
        "flcol_report_id,flcol_month,flcol_quarter,flcol_year,"
        "flcol_showdb,flcol_prcnt,flcol_priortype,flcol_priormonth,"
        "flcol_priorquarter,flcol_prioryear,flcol_priorprcnt,"
        "flcol_priordiff,flcol_priordiffprcnt,flcol_budget,"
        "flcol_budgetprcnt,flcol_budgetdiff,flcol_budgetdiffprcnt) "
        "VALUES "
         "(:flcol_id,:flcol_flhead_id,:flcol_name,:flcol_descrip,"
        ":flcol_report_id,:flcol_month,:flcol_quarter,:flcol_year,"
        ":flcol_showdb,:flcol_prcnt,:flcol_priortype,:flcol_priormonth,"
        ":flcol_priorquarter,:flcol_prioryear,:flcol_priorprcnt,"
        ":flcol_priordiff,:flcol_priordiffprcnt,:flcol_budget,"
        ":flcol_budgetprcnt,:flcol_budgetdiff,:flcol_budgetdiffprcnt);" );
  }
  else if (_mode == cEdit)
  {
    sql =  ("UPDATE flcol "
        "SET flcol_name=:flcol_name,flcol_descrip=:flcol_descrip,"
        "flcol_report_id=:flcol_report_id,flcol_month=:flcol_month,"
        "flcol_quarter=:flcol_quarter,flcol_year=:flcol_year,"
        "flcol_showdb=:flcol_showdb,flcol_prcnt=:flcol_prcnt,"
        "flcol_priortype=:flcol_priortype,flcol_priormonth=:flcol_priormonth,"
        "flcol_priorquarter=:flcol_priorquarter,"
        "flcol_prioryear=:flcol_prioryear,flcol_priorprcnt=:flcol_priorprcnt,"
        "flcol_priordiff=:flcol_priordiff,flcol_priordiffprcnt=:flcol_priordiffprcnt,"
        "flcol_budget=:flcol_budget,flcol_budgetprcnt=:flcol_budgetprcnt,"
        "flcol_budgetdiff=:flcol_budgetdiff,flcol_budgetdiffprcnt=:flcol_budgetdiffprcnt"
        " WHERE (flcol_id=:flcol_id);");
  }

  financialSave.prepare(sql);
     
  financialSave.bindValue(":flcol_id", _flcolid);
  financialSave.bindValue(":flcol_flhead_id", _flheadid);
  financialSave.bindValue(":flcol_name", _name->text());
  financialSave.bindValue(":flcol_descrip", _descrip->text());
  financialSave.bindValue(":flcol_report_id", _report->id());
  financialSave.bindValue(":flcol_month", _month->isChecked());
  financialSave.bindValue(":flcol_quarter", _quarter->isChecked());
  financialSave.bindValue(":flcol_year", _year->isChecked());
  financialSave.bindValue(":flcol_showdb", _showdb->isChecked());
  financialSave.bindValue(":flcol_prcnt", _prcnt->isChecked());
  if (_priorperiod->isChecked())
    financialSave.bindValue(":flcol_priortype", "P");
  else
    financialSave.bindValue(":flcol_priortype", "Y");
  financialSave.bindValue(":flcol_priormonth", _fullmonth->isChecked());
  financialSave.bindValue(":flcol_priorquarter", _fullquarter->isChecked());
  if (_fullyear->isChecked())
    financialSave.bindValue(":flcol_prioryear", "F");
  else if (_yeartodate->isChecked())
    financialSave.bindValue(":flcol_prioryear", "D");
  else
    financialSave.bindValue(":flcol_prioryear", "N");
  financialSave.bindValue(":flcol_priorprcnt", _priorprcnt->isChecked());
  financialSave.bindValue(":flcol_priordiff", _priordiff->isChecked());
  financialSave.bindValue(":flcol_priordiffprcnt", _priordiffprcnt->isChecked());
  financialSave.bindValue(":flcol_budget", _budget->isChecked());
  if (_budget->isChecked())
  {
    financialSave.bindValue(":flcol_budgetprcnt", _budgetprcnt->isChecked());
    financialSave.bindValue(":flcol_budgetdiff", _budgetdiff->isChecked());
    financialSave.bindValue(":flcol_budgetdiffprcnt", _budgetdiffprcnt->isChecked());
  }
  else
  {
      financialSave.bindValue(":flcol_budgetprcnt", false);
    financialSave.bindValue(":flcol_budgetdiff", false);
    financialSave.bindValue(":flcol_budgetdiffprcnt", false);
  }
    
  financialSave.exec();
  if (financialSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, financialSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_flcolid);
}

void financialLayoutColumns::populate()
{
  XSqlQuery financialpopulate;
  financialpopulate.prepare( "SELECT * "
             "FROM flcol "
             "WHERE (flcol_id=:flcol_id);" );
  financialpopulate.bindValue(":flcol_id", _flcolid);
  financialpopulate.exec();
  if (financialpopulate.first())
  {
    _name->setText(financialpopulate.value("flcol_name"));
    _descrip->setText(financialpopulate.value("flcol_descrip"));
    _report->setId(financialpopulate.value("flcol_report_id").toInt());
    _month->setChecked(financialpopulate.value("flcol_month").toBool());
    _quarter->setChecked(financialpopulate.value("flcol_quarter").toBool());
    _year->setChecked(financialpopulate.value("flcol_year").toBool());
    _showdb->setChecked(financialpopulate.value("flcol_showdb").toBool());
    _prcnt->setChecked(financialpopulate.value("flcol_prcnt").toBool());
  if (financialpopulate.value("flcol_priortype").toString() == "P")
        _priorperiod->setChecked(true);
  else
        _prioryear->setChecked(true);
    _fullmonth->setChecked(financialpopulate.value("flcol_priormonth").toBool());
    _fullquarter->setChecked(financialpopulate.value("flcol_priorquarter").toBool());
    if (financialpopulate.value("flcol_prioryear").toString() == "F")
    {
        _fullyear->setChecked(true);
        _yeartodate->setChecked(false);
    }
    else if (financialpopulate.value("flcol_prioryear").toString() == "D")
    {
        _yeartodate->setChecked(true);
        _fullyear->setChecked(false);
    }
    else
    {
        _fullyear->setChecked(false);
        _yeartodate->setChecked(false);
    }
    _priorprcnt->setChecked(financialpopulate.value("flcol_priorprcnt").toBool());
    _priordiff->setChecked(financialpopulate.value("flcol_priordiff").toBool());
    _priordiffprcnt->setChecked(financialpopulate.value("flcol_priordiffprcnt").toBool());
    _budget->setChecked(financialpopulate.value("flcol_budget").toBool());
    _budgetprcnt->setChecked(financialpopulate.value("flcol_budgetprcnt").toBool());  
    _budgetdiff->setChecked(financialpopulate.value("flcol_budgetdiff").toBool());
    _budgetdiffprcnt->setChecked(financialpopulate.value("flcol_budgetdiffprcnt").toBool());  
  }
  else if (financialpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, financialpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void financialLayoutColumns::sToggleMonth()
{
  if (!_month->isChecked())
  {
    _fullmonth->setChecked(false);
    _fullmonth->setEnabled(false);
  }
  else
    _fullmonth->setEnabled(true);
}

void financialLayoutColumns::sToggleQuarter()
{
  if (!_quarter->isChecked())
  {
    _fullquarter->setChecked(false);
    _fullquarter->setEnabled(false);
  }
  else
    _fullquarter->setEnabled(true);
}

void financialLayoutColumns::sToggleYear()
{
  if (!_year->isChecked())
  {
    _fullyear->setChecked(false);
    _yeartodate->setChecked(false);
    _fullyear->setEnabled(false);
    _yeartodate->setEnabled(false);
  }
  else
  {
    _fullyear->setEnabled(true);
    _yeartodate->setEnabled(true);
  }
}

void financialLayoutColumns::sTogglePrior()
{
  if ((_fullmonth->isChecked()) || (_fullquarter->isChecked()) || (_fullyear->isChecked()) || (_yeartodate->isChecked()))
  {
    _priorprcnt->setEnabled(true);
    _priordiff->setEnabled(true);
    _priordiffprcnt->setEnabled(true);
  }
  else
  {
    _priorprcnt->setChecked(false);
    _priordiff->setChecked(false);
    _priordiffprcnt->setChecked(false);  
    _priorprcnt->setEnabled(false);
    _priordiff->setEnabled(false);
    _priordiffprcnt->setEnabled(false);    
  }
}

void financialLayoutColumns::sToggleYearToDate()
{
  if (_fullyear->isChecked())
    _yeartodate->setChecked(false);
}

void financialLayoutColumns::sToggleFullYear()
{
  if (_yeartodate->isChecked())
    _fullyear->setChecked(false);
}

