/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "company.h"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>

#include <dbtools.h>

#include "login2.h"
#include "currcluster.h"
#include "version.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

#define DEBUG false

company::company(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_extDB,     SIGNAL(editingFinished()), this, SLOT(sHandleTest()));
  connect(_extPort,   SIGNAL(editingFinished()), this, SLOT(sHandleTest()));
  connect(_extServer, SIGNAL(editingFinished()), this, SLOT(sHandleTest()));
  connect(_extTest,   SIGNAL(clicked()), this, SLOT(sTest()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(close()));
  connect(_currency, SIGNAL(newID(int)), this, SLOT(sCurrencyChanged()));

  _number->setMaxLength(_metrics->value("GLCompanySize").toInt());
  _cachedNumber = "";
  _cachedCurrid = CurrCluster::baseId();

  _external->setVisible(_metrics->boolean("MultiCompanyFinancialConsolidation"));
  _authGroup->setVisible(_metrics->boolean("MultiCompanyFinancialConsolidation"));
  _currency->setId(CurrCluster::baseId());
  _unrlzgainloss->setType(GLCluster::cRevenue | GLCluster::cExpense);
  _unrlzgainloss->setShowExternal(true);
  _unrlzgainloss->setIgnoreCompany(true);
  _yearend->setShowExternal(true);
  _yearend->setType(GLCluster::cEquity);
  _yearend->setIgnoreCompany(true);
  _gainloss->setType(GLCluster::cExpense);
  _gainloss->setShowExternal(true);
  _gainloss->setIgnoreCompany(true);
  _discrepancy->setType(GLCluster::cExpense);
  _discrepancy->setShowExternal(true);
  _discrepancy->setIgnoreCompany(true);
  _unassigned->setType(GLCluster::cLiability);
  _unassigned->setShowExternal(true);
  _unassigned->setIgnoreCompany(true);
}

company::~company()
{
  // no need to delete child widgets, Qt does it all for us
}

void company::languageChange()
{
  retranslateUi(this);
}

enum SetResponse company::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("company_id", &valid);
  if (valid)
  {
    _companyid = param.toInt();
    populate();
  }
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _number->setEnabled(false);
      _descrip->setEnabled(false);
      _external->setEnabled(false);
      _authGroup->setEnabled(false);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }
  return NoError;
}

void company::sSave()
{
  XSqlQuery companySave;
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_number->text().trimmed().isEmpty(), _number,
                          tr("You must enter a valid Number.") )
         << GuiErrorCheck(_external->isChecked() && _extServer->text().trimmed().isEmpty(),
                          _extServer, 
                          tr("<p>You must enter a Server if this is an external Company.") )
         << GuiErrorCheck(_external->isChecked() && _extPort->value() == 0,
                          _extPort, 
                          tr("<p>You must enter a Port if this is an external Company.") )
         << GuiErrorCheck(_external->isChecked() && _extDB->text().trimmed().isEmpty(),
                          _extDB, 
                          tr("<p>You must enter a Database if this is an external Company.") )
  ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Company"), errors))
    return;

  companySave.prepare("SELECT company_id"
            "  FROM company"
            " WHERE((company_id != :company_id)"
            "   AND (company_number=:company_number))");
  companySave.bindValue(":company_id",       _companyid);
  companySave.bindValue(":company_number",   _number->text());
  companySave.exec();

  errors << GuiErrorCheck(companySave.first(), _number,
                          tr("A Company Number already exists for the one specified.") );

  if (GuiErrorCheck::reportErrors(this, tr("Duplicate Company Number"), errors))
    return;

  errors << GuiErrorCheck(_yearend->isValid() && _companyid != _yearend->companyId(),
                          _yearend,
                          tr("The Retained Earnings Account must belong to this Company.") )
         << GuiErrorCheck(_gainloss->isValid() && _companyid != _gainloss->companyId(),
                          _gainloss,
                          tr("The Currency Gain/Loss Account must belong to this Company.") )
         << GuiErrorCheck(_discrepancy->isValid() && _companyid != _discrepancy->companyId(),
                          _discrepancy,
                          tr("The G/L Discrepancy Account must belong to this Company.") )
         << GuiErrorCheck(_unassigned->isValid() && _companyid != _unassigned->companyId(),
                          _unassigned,
                          tr("The Unassigned G/L Account must belong to this Company.") )
         << GuiErrorCheck(_unrlzgainloss->isValid() && _companyid != _unrlzgainloss->companyId(),
                          _unrlzgainloss,
                          tr("The Unrealized Currency Gain/LossL Account must belong to this Company.") )
  ;

  if (GuiErrorCheck::reportErrors(this, tr("Company Account Mismatch"), errors))
    return;

  if (_mode == cNew)
  {
    companySave.exec("SELECT NEXTVAL('company_company_id_seq') AS company_id;");
    if (companySave.first())
      _companyid = companySave.value("company_id").toInt();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Company"),
                                  companySave, __FILE__, __LINE__))
    {
      return;
    }
    
    companySave.prepare( "INSERT INTO company "
               "( company_id, company_number, company_descrip,"
               "  company_external, company_server, company_port,"
               "  company_database, company_curr_id, company_yearend_accnt_id, "
               "  company_gainloss_accnt_id, company_dscrp_accnt_id, "
               "  company_unrlzgainloss_accnt_id, company_unassigned_accnt_id) "
               "VALUES "
               "( :company_id, :company_number, :company_descrip,"
               "  :company_external, :company_server, :company_port, "
               "  :company_database, :company_curr_id, :company_yearend_accnt_id, "
               "  :company_gainloss_accnt_id, :company_dscrp_accnt_id, "
               "  :company_unrlzgainloss_accnt_id, :company_unassigned_accnt_id);" );
  }
  else if (_mode == cEdit)
  {
    if (_number->text() != _cachedNumber &&
        QMessageBox::question(this, tr("Change All Accounts?"),
                          _external->isChecked() ?
                              tr("<p>The old Company Number %1 might be used "
                                 "by existing Accounts, including Accounts in "
                                 "the %2 child database. Would you like to "
                                 "change all accounts in the current database "
                                 "that use it to Company Number %3?<p>If you "
                                 "answer 'No' then either Cancel or change the "
                                 "Number back to %4 and Save again. If you "
                                 "answer 'Yes' then change the Company Number "
                                 "in the child database as well.")
                                .arg(_cachedNumber)
                                .arg(_extDB->text())
                                .arg(_number->text())
                                .arg(_cachedNumber)
                            :
                              tr("<p>The old Company Number %1 might be used "
                                 "by existing Accounts. Would you like to "
                                 "change all accounts that use it to Company "
                                 "Number %2?<p>If you answer 'No' then either "
                                 "Cancel or change "
                                 "the Number back to %3 and Save again.")
                                .arg(_cachedNumber)
                                .arg(_number->text())
                                .arg(_cachedNumber),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    companySave.prepare( "UPDATE company "
               "SET company_number=:company_number,"
               "    company_descrip=:company_descrip,"
               "    company_external=:company_external,"
               "    company_server=:company_server,"
               "    company_port=:company_port,"
               "    company_database=:company_database, "
               "    company_curr_id=:company_curr_id, "
               "    company_yearend_accnt_id=:company_yearend_accnt_id, "
               "    company_gainloss_accnt_id=:company_gainloss_accnt_id, "
               "    company_dscrp_accnt_id=:company_dscrp_accnt_id, "
               "    company_unrlzgainloss_accnt_id=:company_unrlzgainloss_accnt_id, "
               "    company_unassigned_accnt_id=:company_unassigned_accnt_id "
               "WHERE (company_id=:company_id);" );
  }
  
  companySave.bindValue(":company_id",       _companyid);
  companySave.bindValue(":company_number",   _number->text().trimmed());
  companySave.bindValue(":company_descrip",  _descrip->toPlainText());
  companySave.bindValue(":company_external", _external->isChecked());
  companySave.bindValue(":company_server",   _extServer->text());
  companySave.bindValue(":company_port",     _extPort->cleanText());
  companySave.bindValue(":company_database", _extDB->text());
  if (_gainloss->isValid())
    companySave.bindValue(":company_gainloss_accnt_id", _gainloss->id());
  if (_discrepancy->isValid())
    companySave.bindValue(":company_dscrp_accnt_id", _discrepancy->id());
  if (_unassigned->isValid())
    companySave.bindValue(":company_unassigned_accnt_id", _unassigned->id());
  if (_yearend->isValid())
    companySave.bindValue(":company_yearend_accnt_id", _yearend->id());
  if (_external->isChecked())
  {
    companySave.bindValue(":company_curr_id", _currency->id());
    if (_unrlzgainloss->isValid())
      companySave.bindValue(":company_unrlzgainloss_accnt_id", _unrlzgainloss->id());
  }
  companySave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Company"),
                                companySave, __FILE__, __LINE__))
  {
    return;
  }

  if ((!_yearend->isValid()) ||
     (!_gainloss->isValid()) ||
     (!_discrepancy->isValid()) ||
     (!_unassigned->isValid()) ||
     (_external->isChecked() &&
      _currency->id() != CurrCluster::baseId() &&
      !_unrlzgainloss->isValid()))
  {
    QMessageBox::warning( this, tr("Accounts Required"),
                          tr("You will need to return to this window to set "
                             "required Accounts before you can use Accounts "
                             "for this company in the system.") );
  }
  
  done(_companyid);
}

void company::populate()
{
  XSqlQuery companypopulate;
  companypopulate.prepare( "SELECT * "
             "FROM company "
             "WHERE (company_id=:company_id);" );
  companypopulate.bindValue(":company_id", _companyid);
  companypopulate.exec();
  if (companypopulate.first())
  {
    _number->setText(companypopulate.value("company_number").toString());
    _descrip->setText(companypopulate.value("company_descrip").toString());
    _external->setChecked(companypopulate.value("company_external").toBool());
    _extServer->setText(companypopulate.value("company_server").toString());
    _extPort->setValue(companypopulate.value("company_port").toInt());
    _extDB->setText(companypopulate.value("company_database").toString());
    _yearend->setId(companypopulate.value("company_yearend_accnt_id").toInt());
    _gainloss->setId(companypopulate.value("company_gainloss_accnt_id").toInt());
    _discrepancy->setId(companypopulate.value("company_dscrp_accnt_id").toInt());
    _unassigned->setId(companypopulate.value("company_unassigned_accnt_id").toInt());
    if (_external->isChecked())
    {
      _cachedCurrid = companypopulate.value("company_curr_id").toInt();
      _currency->setId(companypopulate.value("company_curr_id").toInt());
      _unrlzgainloss->setId(companypopulate.value("company_unrlzgainloss_accnt_id").toInt());
    }

    _cachedNumber = companypopulate.value("company_number").toString();

    companypopulate.prepare("SELECT COUNT(*) AS result "
              "FROM accnt "
              "WHERE (accnt_company=:number);");
    companypopulate.bindValue(":number", _cachedNumber);
    companypopulate.exec();
    if (companypopulate.first())
      _external->setEnabled(companypopulate.value("result").toInt() == 0);
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Company Information"),
                                  companypopulate, __FILE__, __LINE__))
    {
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Company Information"),
                                companypopulate, __FILE__, __LINE__))
  {
    return;
  }
  sHandleTest();
}

void company::sHandleTest()
{
  _extTest->setEnabled(! _extServer->text().isEmpty() &&
                       _extPort->value() != 0 &&
                       ! _extDB->text().isEmpty());
}

void company::sTest()
{
  XSqlQuery companyTest;
  if (DEBUG)
    qDebug("company::sTest()");

  QString dbURL;
  QString protocol = "psql";
  QString host = _extServer->text();
  QString db   = _extDB->text();
  QString port = _extPort->cleanText();

  buildDatabaseURL(dbURL, protocol, host, db, port);
  if (DEBUG)
    qDebug("company::sTest() dbURL before login2 = %s", qPrintable(dbURL));

  ParameterList params;
  params.append("databaseURL", dbURL);
  params.append("multipleConnections");
  params.append("applicationName", _ConnAppName);

  login2 newdlg(this, "testLogin", false);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Rejected)
    return;

  dbURL = newdlg._databaseURL;
  if (DEBUG)
    qDebug("company::sTest() dbURL after login2 = %s", qPrintable(dbURL));
  parseDatabaseURL(dbURL, protocol, host, db, port);

  QSqlDatabase testDB = QSqlDatabase::addDatabase("QPSQL7", db);
  testDB.setHostName(host);
  testDB.setDatabaseName(db);
  testDB.setUserName(newdlg.username());
  testDB.setPassword(newdlg.password());
  testDB.setPort(port.toInt());
  if (testDB.open())
  {
    if (DEBUG)
      qDebug("company::sTest() opened testDB!");

    XSqlQuery rmq(testDB);
    rmq.prepare("SELECT fetchMetricText('ServerVersion') AS result;");
    rmq.exec();
    if (rmq.first())
    {
      if (rmq.value("result").toString() != _metrics->value("ServerVersion"))
      {
        QMessageBox::warning(this, tr("Versions Incompatible"),
                             tr("<p>The version of the child database is not "
                                "the same as the version of the parent "
                                "database (%1 vs. %2). The data cannot safely "
                                "be synchronized.")
                             .arg(rmq.value("result").toString())
                             .arg(_metrics->value("ServerVersion")));
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Connection Information"),
                                  rmq, __FILE__, __LINE__))
    {
      return;
    }

    rmq.exec("SELECT * FROM curr_symbol WHERE curr_base;");
    if (_external->isChecked())
    {
      companyTest.prepare("SELECT * FROM curr_symbol WHERE curr_id=:curr_id;");
      companyTest.bindValue(":curr_id", _currency->id());
      companyTest.exec();
    }
    else
      companyTest.exec("SELECT * FROM curr_symbol WHERE curr_base;");

    if (companyTest.first() && rmq.first())
    {
      if (rmq.value("curr_symbol").toString() != companyTest.value("curr_symbol").toString() &&
          rmq.value("curr_abbr").toString() != companyTest.value("curr_abbr").toString())
      {
        QMessageBox::warning(this, tr("Currencies Incompatible"),
                             tr("<p>The currency of the child database does "
                                "not appear to match the selected currency for "
                                "the company (%1 %2 %3 vs. %4 %5 %6). The data may "
                                "not synchronize properly.")
                             .arg(rmq.value("curr_name").toString())
                             .arg(rmq.value("curr_symbol").toString())
                             .arg(rmq.value("curr_abbr").toString())
                             .arg(companyTest.value("curr_name").toString())
                             .arg(companyTest.value("curr_symbol").toString())
                             .arg(companyTest.value("curr_abbr").toString()));
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Connection Information"),
                                  rmq, __FILE__, __LINE__))
    {
      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Connection Information"),
                                  companyTest, __FILE__, __LINE__))
    {
      return;
    }
    else if (!rmq.first())
    {
      QMessageBox::warning(this, tr("No Base Currency"),
                           tr("<p>The child database does not appear to have "
                              "a base currency defined. The data cannot safely "
                              "be synchronized."));
      return;
    }
    else if (!companyTest.first())
    {
      QMessageBox::warning(this, tr("No Base Currency"),
                           tr("<p>The parent database does not appear to have "
                              "a base currency defined. The data cannot safely "
                              "be synchronized."));
      return;
    }

    rmq.prepare("SELECT * FROM company WHERE (company_number=:number);");
    rmq.bindValue(":number", _number->text());
    rmq.exec();
    if (rmq.first())
    {
      QMessageBox::warning(this, windowTitle(), tr("Test Successful."));
      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Connection Information"),
                                  rmq, __FILE__, __LINE__))
    {
      return;
    }
    else
    {
      QMessageBox::warning(this, tr("No Corresponding Company"),
                           tr("<p>The child database does not appear to have "
                              "a Company %1 defined. The data cannot safely "
                              "be synchronized.").arg(_number->text()));
      return;
    }
  }
}

void company::sCurrencyChanged()
{
  if (!_external->isChecked())
    return;

  if (_currency->id() != _cachedCurrid)
  {
    XSqlQuery qry;
    qry.prepare("SELECT count(trialbal_id) "
                "FROM trialbal "
                " JOIN accnt ON (trialbal_accnt_id=accnt_id) "
                "WHERE (accnt_company=:company_number);");
    qry.bindValue(":company_number", _number->text());
    qry.exec();
    qry.first();
    if (qry.value("count").toInt())
    {
      if (QMessageBox::question(this, tr("Delete Imported Data?"),
                                tr("Financial history has already been imported "
                                   "for this company. Changing the currency will delete "
                                   "this data. This action is not reversible.  Are you "
                                   "sure this is what you want to do?"),
                                QMessageBox::Yes,
                                QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
      {
        qry.prepare("DELETE FROM gltranssync "
                    "WHERE (gltranssync_company_id=:company_id); "
                    "DELETE FROM trialbal "
                    "WHERE (trialbal_accnt_id IN ("
                    "  SELECT accnt_id "
                    "  FROM accnt "
                    "    JOIN company ON (accnt_company=company_number) "
                    "  WHERE (company_id=:company_id)));");
        qry.bindValue(":company_id", _companyid);
        qry.exec();
        if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Changing Currency"),
                                 qry, __FILE__, __LINE__))
        {
          return;
        }
      }
      else
        _currency->setId(_cachedCurrid);
    }
  }
  _unrlzgainloss->setEnabled(_currency->id() != CurrCluster::baseId());
}
