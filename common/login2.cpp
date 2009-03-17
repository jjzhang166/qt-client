/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "login2.h"

#include <QVariant>
#include <QMessageBox>
#include <QCursor>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSettings>
#include <QApplication>
#include <QSplashScreen>
#include <QCheckBox>

#include "dbtools.h"
#include "xsqlquery.h"
#include "login2Options.h"
#include "login2.h"
#include "qmd5.h"
#include "storedProcErrorLookup.h"

#include "splashconst.h"

login2::login2(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  Q_INIT_RESOURCE(xTupleCommon);
  setupUi(this);

  connect(_login, SIGNAL(clicked()), this, SLOT(sLogin()));
  connect(_options, SIGNAL(clicked()), this, SLOT(sOptions()));

  _splash = 0;

  _captive = false; _nonxTupleDB = false;
  _multipleConnections = false;
  _evalDatabaseURL = "pgsql://demo.xtuple.com:5434/%1";

  _userid = -1;

  _password->setEchoMode(QLineEdit::Password);

  QSettings ini(QSettings::IniFormat, QSettings::UserScope, "xTuple.com", "xTuple");
  QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
  _databaseURL = ini.readEntry("/xTuple/_databaseURL", settings.readEntry("/OpenMFG/_databaseURL", "pgsql://:5432/"));
  _enhancedAuth = ini.readBoolEntry("/xTuple/_enhancedAuthentication",settings.readBoolEntry("/OpenMFG/_enhancedAuthentication", false));
  _requireSSL = ini.readBoolEntry("/xTuple/_requireSSL",settings.readBoolEntry("/OpenMFG/_requireSSL", false));
  if(ini.readBoolEntry("/xTuple/_demoOption",settings.readBoolEntry("/OpenMFG/_demoOption", false)))
    _demoOption->setChecked(true);
}

login2::~login2()
{
  // no need to delete child widgets, Qt does it all for us
}

void login2::languageChange()
{
  retranslateUi(this);
}

int login2::set(ParameterList &pParams) { return set(pParams, 0); }

int login2::set(ParameterList &pParams, QSplashScreen *pSplash)
{
  _splash = pSplash;
  
  QVariant param;
  bool     valid;

  param = pParams.value("username", &valid);
  if (valid)
  {
    _username->setText(param.toString());
    _password->setFocus();
    _captive = TRUE;
  }
  else
  {
    _username->setFocus();
    _captive = FALSE;
  }

  param = pParams.value("copyright", &valid);
  if (valid)
    _copyrightLit->setText(param.toString());

  param = pParams.value("version", &valid);
  if (valid)
    _versionLit->setText(tr("Version ") + param.toString());

  param = pParams.value("build", &valid);
  if (valid)
    _build->setText(param.toString());

  param = pParams.value("evaluation", &valid);
  if (valid)
  {
/*
    _serverLit->hide();
    _server->hide();
    _databaseLit->hide();
    _database->hide();
    _options->setEnabled(false);
*/
    _demoOption->setChecked(TRUE);
  }

  param = pParams.value("name", &valid);
  if (valid)
    _nameLit->setText(param.toString());

  param = pParams.value("databaseURL", &valid);
  if (valid)
    _databaseURL = param.toString();

  populateDatabaseInfo();

  param = pParams.value("nonxTupleDB", &valid);
  if (valid)
    _nonxTupleDB = true;

  param = pParams.value("multipleConnections", &valid);
  if (valid)
    _multipleConnections = true;

  return 0;
}

void login2::sLogin()
{
  QSqlDatabase db;

  QString databaseURL;
  databaseURL = _databaseURL;
  if (_demoOption->isChecked())
    databaseURL = _evalDatabaseURL.arg(_username->text().trimmed());

  QString protocol;
  QString hostName;
  QString dbName;
  QString port;
  parseDatabaseURL(databaseURL, protocol, hostName, dbName, port);

  if (_splash)
  {
    _splash->show();
    _splash->raise();
    _splash->showMessage(tr("Initializing the Database Connector"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
  }

  // Open the Database Driver
  if (_multipleConnections)
    db = QSqlDatabase::addDatabase("QPSQL7", dbName);
  else
    db = QSqlDatabase::addDatabase("QPSQL7");
  if (!db.isValid())
  {
    if (_splash)
      _splash->hide();
    
    QMessageBox::warning( this, tr("No Database Driver"),
                          tr("<p>A connection could not be established with "
                             "the specified Database as the Proper Database "
                             "Drivers have not been installed. Contact your "
                             "Systems Administator."));
    
    return;
  }

  if(hostName.isEmpty() || dbName.isEmpty())
  {
    if (_splash)
      _splash->hide();
    
    QMessageBox::warning(this, tr("Incomplete Connection Options"),
                         tr("<p>One or more connection options are missing. "
                            "Please check that you have specified the host "
                            "name, database name, and any other required "
                            "options.") );

    return;
  }

  db.setDatabaseName(dbName);
  db.setHostName(hostName);
  db.setPort(port.toInt());

  _cUsername = _username->text().trimmed();
  _cPassword = _password->text().trimmed();

  db.setUserName(_cUsername);
  if(_demoOption->isChecked())
  {
    QString passwd = QMd5(QString(_cPassword + "private" + _cUsername)); 
    db.setPassword(passwd);
  }
  else
  {
    if(_enhancedAuth)
    {
      QString passwd = QMd5(QString(_cPassword + "OpenMFG" + _cUsername));
      db.setPassword(passwd);
    }
    else
      db.setPassword(_cPassword);

    if(_requireSSL)
      db.setConnectOptions("requiressl=1");
  }

  setCursor(QCursor(Qt::waitCursor));

  if (_splash)
  {
    _splash->showMessage(tr("Connecting to the Database"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
  }
  
  //  Try to connect to the Database
  bool result = db.open();

  if (!result)
  {
    if(_requireSSL)
      db.setConnectOptions();

    if (_splash)
      _splash->hide();
    
    setCursor(QCursor(Qt::arrowCursor));

    QMessageBox::critical(this, tr("Cannot Connect to xTuple ERP Server"),
                          tr("<p>A connection to the specified xTuple ERP "
                             "Server cannot be made.  This may be due to an "
                             "incorrect Username and/or Password or the "
                             "server in question cannot support any more "
                             "connections.<p>Please verify your Username and "
                             "Password and try again or wait until the "
                             "specified xTuple ERP Server is less busy.<br>"
                             "System Error<pre>%1" )
                            .arg(db.lastError().driverText() ));
    if (!_captive)
    {
      _username->setText("");
      _username->setFocus();
    }
    else
      _password->setFocus();

    _password->setText("");
    return;
  }

  QSettings ini(QSettings::IniFormat, QSettings::UserScope, "xTuple.com", "xTuple");
  QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
  ini.writeEntry("/xTuple/_demoOption", (bool)_demoOption->isChecked());
  settings.writeEntry("/OpenMFG/_demoOption", (bool)_demoOption->isChecked());

  if (_splash)
  {
    _splash->showMessage(tr("Logging into the Database"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
  }
  
  if(!_nonxTupleDB)
  {
    XSqlQuery login( "SELECT login() AS usr_id,"
                     "       CURRENT_USER AS user;" );
    setCursor(QCursor(Qt::arrowCursor));
    if (login.first())
    {
      int result = login.value("usr_id").toInt();
      if (result < 0)
      {
        if (_splash)
          _splash->hide();
        QMessageBox::critical(this, tr("Error Logging In"),
                              storedProcErrorLookup("login", result));
        return;
      }
      _user = login.value("user").toString();
      _userid = login.value("usr_id").toInt();
      _databaseURL = databaseURL;
      accept();
    }
    else if (login.lastError().type() != QSqlError::NoError)
    {
      if (_splash)
        _splash->hide();
      QMessageBox::critical(this, tr("System Error"),
                            tr("A System Error occurred at %1::%2:\n%1")
                              .arg(__FILE__).arg(__LINE__)
                              .arg(login.lastError().databaseText()));
    }
    else
    {
      if (_splash)
        _splash->hide();
      
      QMessageBox::critical(this, tr("System Error"),
                            tr("<p>An unknown error occurred at %1::%2. You may"
                               " not log in to the specified xTuple ERP Server "
                               "at this time.")
                              .arg(__FILE__).arg(__LINE__));
    }
  }
  else
  {
    setCursor(QCursor(Qt::arrowCursor));
    _databaseURL = databaseURL;
    accept();
  }
}

void login2::sOptions()
{
  ParameterList params;
  params.append("databaseURL", _databaseURL);

  if (_multipleConnections)
    params.append("dontSaveSettings");

  if(_enhancedAuth)
    params.append("useEnhancedAuthentication");

  if(_requireSSL)
    params.append("requireSSL");

  login2Options newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
  {
    _databaseURL = newdlg._databaseURL;
    _enhancedAuth = newdlg._enhancedAuth->isChecked();
    _requireSSL = newdlg._requireSSL->isChecked();
    populateDatabaseInfo();
    _username->setFocus();
  }
}

void login2::populateDatabaseInfo()
{
  QString protocol;
  QString hostName;
  QString dbName;
  QString port;

  parseDatabaseURL(_databaseURL, protocol, hostName, dbName, port);
  _server->setText(hostName);
  _database->setText(dbName);
}

QString login2::username()
{
  return _cUsername;
}

QString login2::password()
{
  return _cPassword;
}

void login2::setLogo(const QImage & img)
{
  if(img.isNull())
    _logo->setPixmap(QPixmap(":/login/images/splashXTuple.png"));
  else
    _logo->setPixmap(QPixmap::fromImage(img));
}
