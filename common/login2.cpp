/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "login2.h"

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSplashScreen>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStringList>
#include <QDesktopServices>
#include <QUrl>
#include <QVariant>

#include "dbtools.h"
#include "errorReporter.h"
#include "login2Options.h"
#include "qmd5.h"
#include "storedProcErrorLookup.h"
#include "xsqlquery.h"
#include "xtsettings.h"

#include "splashconst.h"

/* TODO: rename _nonxTupleDB to _isxTupleDB internally and
         set it based on db contents, not command line parameter input
 */
login2::login2(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, modal ? (fl | Qt::Dialog) : fl)
{
  setObjectName(name);
  Q_INIT_RESOURCE(xTupleCommon);
  setupUi(this);

  //_options = _buttonBox->addButton(tr("Options..."), QDialogButtonBox::ActionRole);
  _recent = _buttonBox->addButton(tr("Recent"), QDialogButtonBox::ActionRole);
  //_options->setEnabled(false);
  _recent->setEnabled(false);
  _buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Login"));

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sLogin()));
  connect(_buttonBox, SIGNAL(helpRequested()), this, SLOT(sOpenHelp()));
  //connect(_options, SIGNAL(clicked()), this, SLOT(sOptions()));
  connect(_server, SIGNAL(editingFinished()), this, SLOT(sChangeURL()));
  connect(_database, SIGNAL(editTextChanged(QString)), this, SLOT(sChangeURL()));
  connect(_port, SIGNAL(textChanged(QString)), this, SLOT(sChangeURL()));
  //connect(_otherOption, SIGNAL(toggled(bool)), _options, SLOT(setEnabled(bool)));
  //connect(_otherOption, SIGNAL(toggled(bool)), _recent, SLOT(setEnabled(bool)));
  //connect(_otherOption, SIGNAL(toggled(bool)), this, SLOT(sHandleButton()));

  _splash = 0;

  _captive = false; _nonxTupleDB = false;
  _multipleConnections = false;
  _setSearchPath = false;
  _cloudDatabaseURL= "pgsql://%1.xtuplecloud.com:5432/%1_%2";

  _password->setEchoMode(QLineEdit::Password);

  //updateRecentOptionsActions();
  _databaseURL = xtsettingsValue("/xTuple/_databaseURL", "pgsql://:5432/demo").toString();
  /*
  if(xtsettingsValue("/xTuple/_demoOption", false).toBool())
    _demoOption->setChecked(true);
  else
    _prodOption->setChecked(true);

  if(xtsettingsValue("/xTuple/_cloudOption", false).toBool())
    _cloudOption->setChecked(true);
  else
    _otherOption->setChecked(true);
  _company->setText(xtsettingsValue("/xTuple/cloud_company", "").toString());
  */

  adjustSize();
}

login2::~login2()
{
  // no need to delete child widgets, Qt does it all for us
}

void login2::languageChange()
{
  retranslateUi(this);
}

int login2::set(const ParameterList &pParams) { return set(pParams, 0); }

int login2::set(const ParameterList &pParams, QSplashScreen *pSplash)
{
  _splash = pSplash;
  
  QVariant param;
  bool     valid;

  param = pParams.value("username", &valid);
  if (valid)
  {
    _username->setText(param.toString());
    _password->setFocus();
    _captive = true;
  }
  else
  {
    _username->setFocus();
    _captive = false;
  }

  param = pParams.value("password", &valid);
  if (valid)
  {
    _password->setText(param.toString());
    _captive = true;
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

  /*
  param = pParams.value("cloud", &valid);
  if (valid)
    _cloudOption->setChecked(true);
  */

  /*
  param = pParams.value("company", &valid);
  if (valid)
    _company->setText(param.toString());
  */

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

  param = pParams.value("setSearchPath", &valid);
  if (valid)
    _setSearchPath = true;

  param = pParams.value("applicationName", &valid);
  if (valid) {
    _connAppName = param.toString().trimmed();
  } else {
    _connAppName = "xTuple ERP (unknown)";
  }

  if(pParams.inList("login"))
    sLogin();

  sChangeURL();
  updateRecentOptions();
  updateRecentOptionsActions();

  return 0;
}

void login2::sChangeURL()
{
  buildDatabaseURL(_databaseURL, "psql", _server->text(), _database->lineEdit()->text(), _port->text());

  _databaseURL.replace("https://", "");
  _databaseURL.replace("http://", "");
  populateDatabaseInfo();
}

void login2::sHandleButton()
{
  /*
  if (_otherOption->isChecked())
    _loginModeStack->setCurrentWidget(_serverPage);
  else
    _loginModeStack->setCurrentWidget(_cloudPage);
  */
}

void login2::sOpenHelp()
{
    QString helpurl = "http://www.xtuple.com/how-do-I-login-to-xTuple-PostBooks";
    QUrl url(helpurl);
    QDesktopServices::openUrl(url);
}

void login2::sLogin()
{
  QSqlDatabase db;

  QString databaseURL;
  databaseURL = _databaseURL;
  /*
  if(_cloudOption->isChecked())
  {
    if(_demoOption->isChecked())
      databaseURL = _cloudDatabaseURL.arg(_company->text().trimmed(), "demo");
    else
      databaseURL = _cloudDatabaseURL.arg(_company->text().trimmed(), "quickstart");
  }
  */

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
  //_cCompany  = _company->text().trimmed();

  setCursor(QCursor(Qt::WaitCursor));
  qApp->processEvents();

  if (_splash)
  {
    _splash->showMessage(tr("Connecting to the Database"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
  }

  /*
  if(_cloudOption->isChecked())
  {
    if(_cCompany.isEmpty())
    {
      QMessageBox::warning(this, tr("Incomplete Connection Options"),
        tr("<p>You must specify the Cloud ID name to connect to the cloud."));
      return;
    }
  }
  */

  db.setUserName(_cUsername);

  QRegExp xtuplecloud(".*\\.xtuplecloud\\.com");
  QRegExp xtuple(".*\\.xtuple\\.com");

  bool isCloud = xtuplecloud.exactMatch(hostName);
  bool isXtuple = xtuple.exactMatch(hostName);
  QString salt;

  if(isCloud || isXtuple)
  {
    salt = "j3H44uadEI#8#kSmkh#H%JSLAKDOHImklhdfsn3#432?%^kjasdjla3uy989apa3uipoweurw-03235##+=-lhkhdNOHA?%@mxncvbwoiwerNKLJHwe278NH28shNeGc";
  }
  else
  {
    salt = "xTuple";
  }

  // try connecting to the database in each of the following ways in this order
  QList<QPair<QString, QString> > method;
  method << QPair<QString, QString>("requiressl=1", QMd5(QString(_cPassword + salt  + _cUsername)))
         << QPair<QString, QString>("requiressl=1", _cPassword)
         << QPair<QString, QString>("requiressl=1", QMd5(QString(_cPassword + "OpenMFG" + _cUsername)))
         << QPair<QString, QString>("",             QMd5(QString(_cPassword + salt  + _cUsername)))
         << QPair<QString, QString>("",             _cPassword)
         << QPair<QString, QString>("",             QMd5(QString(_cPassword + "OpenMFG" + _cUsername)))
      ;
  int methodidx; // not declared in for () because we'll need it later
  for (methodidx = 0; methodidx < method.size(); methodidx++)
  {
    db.setConnectOptions(QString("application_name='%1';%2").arg(_connAppName).arg(method.at(methodidx).first));
    db.setPassword(method.at(methodidx).second);
    if (db.open())
      break;  // break instead of for-loop condition to preserve methodidx
  }

  if (db.isOpen())
  {
    QString earliest = "9.1.0",
            latest   = "9.5.0";
    XSqlQuery checkVersion;   // include earliest in the range but exclude latest
    checkVersion.prepare("SELECT compareVersion(:earliest) <= 0"
                         "   AND compareVersion(:latest)   > 0 AS ok,"
                         "       version() AS version;");
    checkVersion.bindValue(":earliest", earliest);
    checkVersion.bindValue(":latest",   latest);
    checkVersion.exec();
    if (checkVersion.first() && ! checkVersion.value("ok").toBool() &&
        QMessageBox::question(this, tr("Unsupported Database Server Version"),
                              tr("<p>The database server is at version %1 but "
                                 "xTuple ERP only supports from %2 up to but "
                                 "not including %3.</p><p>Continue anyway?</p>")
                                .arg(checkVersion.value("version").toString(),
                                     earliest, latest),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::No) {
      if (_splash) {
        _splash->hide();
      }
      setCursor(QCursor(Qt::ArrowCursor));
      db.close();
      if (! _captive) {
        _username->setText("");
        _username->setFocus();
      } else
        _password->setFocus();

      _password->setText("");
      return;
    } else if (ErrorReporter::error(QtCriticalMsg, this,
                                    tr("Cannot Connect to xTuple ERP Server"),
                                    checkVersion, __FILE__, __LINE__)) {
      if (_splash)
        _splash->hide();
      db.close();
      if (! _captive) {
        _username->setText("");
        _username->setFocus();
      } else
        _password->setFocus();

      _password->setText("");
      return;
    }
  }

   // if connected using OpenMFG enhanced auth, remangle the password

  if (db.isOpen() && (methodidx == 2 || methodidx == 5))
      XSqlQuery chgpass(QString("ALTER USER \"%1\" WITH PASSWORD '%2'")
                      .arg(_cUsername, QMd5(QString(_cPassword + salt + _cUsername))));
  else if (db.isOpen() && method.at(methodidx).first.isEmpty())
  {
    XSqlQuery sslq("SHOW ssl;");
    //TODO: Add SSL to installer and require it by default for all xTuple users
    /*if (sslq.first() && sslq.value("ssl").toString() != "on")
      QMessageBox::warning(this, tr("Could Not Enforce Security"),
                           tr("The connection to the xTuple ERP Server is not "
                              "secure. Please ask your administrator to set "
                              "the 'ssl' configuration option to 'on'."));*/
  }

  if (! db.isOpen())
  {
    if (_splash)
      _splash->hide();
    
    setCursor(QCursor(Qt::ArrowCursor));

    QMessageBox::critical(this, tr("Cannot Connect to xTuple ERP Server"),
                          tr("<p>Sorry, can't connect to the specified xTuple ERP server. "
                             "<p>This may be due to a problem with your user name, password, or server connection information. "
                             "<p>Below is more detail on the connection problem: "
                             "<p>%1" ).arg(db.lastError().text()));

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

  /*
  xtsettingsSetValue("/xTuple/_demoOption", (bool)_demoOption->isChecked());
  xtsettingsSetValue("/xTuple/_cloudOption", (bool)_cloudOption->isChecked());
  xtsettingsSetValue("/xTuple/cloud_company", _company->text());
  */

  if (_splash)
  {
    _splash->showMessage(tr("Logging into the Database"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
  }
  
  if(!_nonxTupleDB)
  {
    // check that the connected database at least looks like a valid xTuple database.
    XSqlQuery checkSchema(
                "SELECT EXISTS ("
                "   SELECT 1"
                "   FROM pg_catalog.pg_class AS c"
                "   JOIN  pg_catalog.pg_namespace AS n ON n.oid = c.relnamespace"
                "   WHERE n.nspname = 'public'"
                "   AND c.relname = 'metric'"
                ") AS has_metric"
              );

    if (!(checkSchema.first() ? checkSchema.value("has_metric").toBool() : false))
    {
        setCursor(QCursor(Qt::ArrowCursor));

        if (_splash) {
          _splash->hide();
        }

        QString msg = QObject::tr(
           "<p>The database \"%1\" does not appear to be a valid xTuple database.</p>"
           "<p>Please check the database name and try again.</p>"
        ).arg(dbName);

        QMessageBox::critical(this, tr("Invalid Database"), msg, QMessageBox::Ok | QMessageBox::Escape | QMessageBox::Default);

        return;
    }


    QString loginqry = "";
    if (_setSearchPath)
      loginqry="SELECT login(true) AS result, CURRENT_USER AS user;";
    else
      loginqry="SELECT login() AS result, CURRENT_USER AS user;";
    XSqlQuery login( loginqry );
    setCursor(QCursor(Qt::ArrowCursor));
    if (login.first())
    {
      int result = login.value("result").toInt();
      if (result < 0)
      {
        if (_splash)
          _splash->hide();
        QMessageBox::critical(this, tr("Error Logging In"),
                              storedProcErrorLookup("login", result));
        return;
      }
      _user = login.value("user").toString();
      _databaseURL = databaseURL;
      updateRecentOptions();

      if (login.exec("SELECT getEffectiveXtUser() AS user;") &&
          login.first() &&
          ! login.value("user").toString().isEmpty())
        _user = login.value("user").toString();
      // no error check - older databases don't have getEffectiveXtUser

      accept();
    }
    else if (login.lastError().type() != QSqlError::NoError)
    {
      if (_splash)
        _splash->hide();
      QMessageBox::critical(this, tr("System Error"),
                            tr("A System Error occurred at %1::%2:\n%3")
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
    setCursor(QCursor(Qt::ArrowCursor));
    _databaseURL = databaseURL;
    updateRecentOptions();
    accept();
  }
}

void login2::sOptions()
{
  ParameterList params;
  params.append("databaseURL", _databaseURL);

  if (_multipleConnections)
    params.append("dontSaveSettings");

  login2Options newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
  {
    updateRecentOptions();
    _databaseURL = newdlg._databaseURL;
    populateDatabaseInfo();
    updateRecentOptions();
    updateRecentOptionsActions();
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
  _database->lineEdit()->setText(dbName);
  _port->setText(port);
}

QString login2::username()
{
  return _cUsername;
}

QString login2::password()
{
  return _cPassword;
}

QString login2::company()
{
  return _cCompany;
}

bool login2::useCloud() const
{
  //return _cloudOption->isChecked();
  return false;
}

void login2::setLogo(const QImage & img)
{
  if(img.isNull())
    _logo->setPixmap(QPixmap(":/login/images/splashXTuple.png"));
  else
    _logo->setPixmap(QPixmap::fromImage(img));
}

void login2::updateRecentOptions()
{
  //if (_cloudOption->isChecked())
  //  return;
  QStringList list = xtsettingsValue("/xTuple/_recentOptionsList").toStringList();
  _recent->setEnabled(list.size());
  list.removeAll(_databaseURL);
  list.prepend(_databaseURL);
      
  xtsettingsSetValue("/xTuple/_recentOptionsList", list);
  xtsettingsSetValue("/xTuple/_databaseURL", _databaseURL);
}

void login2::updateRecentOptionsActions()
{ 
  QMenu * recentMenu = new QMenu;
  QStringList list = xtsettingsValue("/xTuple/_recentOptionsList").toStringList();
  if (list.size())
  {
    list.takeFirst();
    int size = list.size();
    if (size > 10)
      size = 10;

    QString protocol;
    QString hostName;
    QString dbName;
    QString port;
    int alreadyExists;
  
    if (size)
    {
      //if (_otherOption->isChecked())
      _recent->setEnabled(true);
      QAction *act;
      for (int i = 0; i < size; ++i) 
      {
        act = new QAction(list.value(i).remove("psql://"),this);
        connect(act, SIGNAL(triggered()), this, SLOT(selectRecentOptions()));
        recentMenu->addAction(act);
        parseDatabaseURL(list.value(i), protocol, hostName, dbName, port);
        alreadyExists = _database->findText(dbName);
        if (alreadyExists == -1)
          _database->addItem(dbName);
      }
  
      recentMenu->addSeparator();

      act = new QAction(tr("Clear &Menu"), this);
      act->setObjectName(QLatin1String("__xt_action_clear_menu_"));
      connect(act, SIGNAL(triggered()), this, SLOT(clearRecentOptions()));
      recentMenu->addAction(act);
    }
    else
      _recent->setEnabled(false);
  }
  else
    _recent->setEnabled(false);
  
  _recent->setMenu(recentMenu);
}

void login2::selectRecentOptions()
{
  if (const QAction *action = qobject_cast<const QAction *>(sender()))
  {
    _databaseURL = action->iconText().prepend("psql://");
    populateDatabaseInfo();
    updateRecentOptions();
    updateRecentOptionsActions();
  }
}

void login2::clearRecentOptions()
{
    QStringList list;
    xtsettingsSetValue("/xTuple/_recentOptionsList", list);
    updateRecentOptionsActions();
}
