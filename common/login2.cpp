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

#include "splashconst.h"

/*
 *  Constructs a login2 as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
login2::login2(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  Q_INIT_RESOURCE(OpenMFGCommon);
  setupUi(this);

  // signals and slots connections
  connect(_login, SIGNAL(clicked()), this, SLOT(sLogin()));
  connect(_options, SIGNAL(clicked()), this, SLOT(sOptions()));

  _splash = 0;

  _captive = false; _nonOpenMFGDB = false;
  _evalDatabaseURL = "pgsql://demo.openmfg.com:5434/%1";

  _userid = -1;

  _password->setEchoMode(QLineEdit::Password);

  QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
  _databaseURL = settings.readEntry("/OpenMFG/_databaseURL", "pgsql://:5432/");
  _enhancedAuth = settings.readBoolEntry("/OpenMFG/_enhancedAuthentication", false);
  _requireSSL = settings.readBoolEntry("/OpenMFG/_requireSSL", false);
  if(settings.readBoolEntry("/OpenMFG/_demoOption", false))
    _demoOption->setChecked(true);
}

/*
 *  Destroys the object and frees any allocated resources
 */
login2::~login2()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
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

  param = pParams.value("nonOpenMFGDB", &valid);
  if (valid)
    _nonOpenMFGDB = true;

  return 0;
}

void login2::sLogin()
{
  QSqlDatabase db;

// Open the Database Driver
  if (_splash)
  {
    _splash->show();
    _splash->raise();
    _splash->showMessage(tr("Initializing the Database Connector"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
  }
  db = QSqlDatabase::addDatabase("QPSQL7");
  if (!db.isValid())
  {
    if (_splash)
      _splash->hide();
    
    QMessageBox::warning( this, tr("No Database Driver"),
                          tr( "A connection could not be established with the specified\n"
                              "Database as the Proper Database Drivers have not been installed.\n"
                                 "Contact your Systems Administator.\n"  ));
    
    return;
  }

  QString databaseURL;
  databaseURL = _databaseURL;
  if (_demoOption->isChecked())
    databaseURL = _evalDatabaseURL.arg(_username->text().stripWhiteSpace());

//  Try to connect to the Database
  QString protocol;
  QString hostName;
  QString dbName;
  QString port;
  parseDatabaseURL(databaseURL, protocol, hostName, dbName, port);

  if(hostName.isEmpty() || dbName.isEmpty())
  {
    if (_splash)
      _splash->hide();
    
    QMessageBox::warning(this, tr("Incomplete Connection Options"),
      tr("One or more connection options are missing. Please check that you have specified"
         " the host name, database name, and any other required options.") );

    return;
  }

  db.setDatabaseName(dbName);
  db.setHostName(hostName);
  db.setPort(port.toInt());

  _cUsername = _username->text().stripWhiteSpace();
  _cPassword = _password->text().stripWhiteSpace();

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
  
  bool result = db.open();

  if (!result)
  {
    if(_requireSSL)
      db.setConnectOptions();

    if (_splash)
      _splash->hide();
    
    setCursor(QCursor(Qt::arrowCursor));

    QMessageBox::critical( this, tr("Cannot Connect to OpenMFG Server"),
                           tr( "A connection to the specified OpenMFG Server cannot be made.  This may be due to an\n"
                               "incorrect Username and/or Password or that the OpenMFG Server in question cannot\n"
                               "support anymore connections.\n\n"
                               "Please verify your Username and Password and try again or wait until the specified\n"
                               "OpenMFG Server is less busy.\n\n"
                               "System Error '%1'" ).arg(db.lastError().driverText() ));
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

  QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
  settings.writeEntry("/OpenMFG/_demoOption", (bool)_demoOption->isChecked());

  if (_splash)
  {
    _splash->showMessage(tr("Logging into the Database"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
  }
  
  if(!_nonOpenMFGDB)
  {
    XSqlQuery login( "SELECT login() AS usr_id,"
                     "       CURRENT_USER AS user;" );
    setCursor(QCursor(Qt::arrowCursor));
    if (login.first())
    {
      switch (login.value("usr_id").toInt())
      {
        case -1:
          if (_splash)
            _splash->hide();
  
          QMessageBox::critical( this, tr("User does not Exist"),
                                 tr( "The specified Username does not exist in the specified Database.\n"
                                     "Contact your Systems Administrator to report this issue." ) );
          break;
  
        case -2:
          if (_splash)
            _splash->hide();
  
          QMessageBox::critical( this, tr("User is Inactive"),
                                 tr( "The specified Username does exists in the specified Database but is not Active.\n"
                                     "Contact your Systems Administrator to report this issue." ) );
          break;
  
        default:
          _user = login.value("user").toString();
          _userid = login.value("usr_id").toInt();
          _databaseURL = databaseURL;
          accept();
          break;
      }
    }
    else
    {
      if (_splash)
        _splash->hide();
      
      QMessageBox::critical( this, tr("System Error"),
                             tr( "A System Error occurred at login2::%1.\n"
                                 "You may not log into the specified OpenMFG Server at this time.\n"
                                 "Report this to your Systems Administrator." )
                             .arg(__LINE__) );
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

