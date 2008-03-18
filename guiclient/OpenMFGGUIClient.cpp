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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

//  OpenMFGGUIClient.cpp
//  Created 12/07/1999 JSL
//  Copyright (c) 1999-2007, OpenMFG, LLC

#include <QTimer>
#include <QAction>
#include <Q3VBox>
#include <QStatusBar>
#include <QWorkspace>
#include <QDateTime>
#include <QPushButton>
#include <QCheckBox>
#include <QValidator>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QSqlDatabase>
#include <QImage>
#include <QSplashScreen>
#include <QMessageBox>
#include <QApplication>
#include <QCursor>
#include <QDir>
#include <QAssistantClient>
#include <Q3Process>
#include <QSqlError>
#include <QPixmap>
#include <Q3Frame>
#include <QTextStream>
#include <QCloseEvent>
#include <QMainWindow>
#include <QSettings>
#include <QDesktopWidget>

#include <parameter.h>
#include <dbtools.h>
#include <quuencode.h>

#include "OpenMFGGUIClient.h"
#include "version.h"

#include "systemMessage.h"
#include "menuProducts.h"
#include "menuInventory.h"
#include "menuSchedule.h"
#include "menuManufacture.h"
#include "menuPurchase.h"
#include "menuCRM.h"
#include "menuSales.h"
#include "menuAccounting.h"
#include "menuSystem.h"

#include "moduleIM.h"
#include "moduleMS.h"
#include "moduleCP.h"
#include "modulePD.h"
#include "moduleWO.h"
#include "modulePO.h"
#include "moduleSO.h"
#include "moduleSR.h"
#include "moduleSA.h"
#include "modulePM.h"
#include "moduleAR.h"
#include "moduleAP.h"
#include "moduleGL.h"
#include "moduleSys.h"
#include "moduleCRM.h"

#include "timeoutHandler.h"
#include "idleShutdown.h"
#include "inputManager.h"

#include "custcluster.h"
#include "crmacctcluster.h"
#include "crmaccount.h"
#include "dspCustomerInformation.h"

#include "splashconst.h"

#if defined(Q_OS_WIN32)
#define NOCRYPT
#include <windows.h>
#else
#if defined(Q_OS_MACX)
#include <stdlib.h>
#endif
#endif

class Metrics;
class Preferences;
class Privleges;
class Metricsenc;

#ifdef Q_WS_MACX
void qt_mac_set_native_menubar(bool);
#endif

QSplashScreen *_splash;

Metrics       *_metrics=0;
Preferences   *_preferences=0;
Privleges     *_privleges=0;
Metricsenc    *_metricsenc=0;
QList<QString> _hotkeyList;

bool _evaluation;

#include <SaveSizePositionEventFilter.h>
static SaveSizePositionEventFilter * __saveSizePositionEventFilter = 0;

Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, bool pEnabled ) :
 QAction(pDisplayName, pParent)
{
  _name = pName;
  _displayName = pDisplayName;

  QString hotkey;
  hotkey = _preferences->parent(pName);
  if (!hotkey.isNull() && !_hotkeyList.contains(hotkey))
  {
    _hotkeyList << hotkey;
    setShortcutContext(Qt::ApplicationShortcut);
    if (hotkey.left(1) == "C")
      setShortcut(QString("Ctrl+%1").arg(hotkey.right(1)));

    else if (hotkey.left(1) == "F")
      setShortcut(hotkey);
  }

  connect(this, SIGNAL(activated()), pTarget, pActivateSlot);
  setEnabled(pEnabled);
  pAddTo->addAction(this);
}

Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, bool pEnabled,
                const QPixmap &pIcon, QWidget *pToolBar ) :
 QAction(pDisplayName, pParent)
{
  _name = pName;
  _displayName = pDisplayName;

  QString hotkey = _preferences->parent(pName);
  if (!hotkey.isNull() && !_hotkeyList.contains(hotkey))
  {
    _hotkeyList << hotkey;
    setShortcutContext(Qt::ApplicationShortcut);
    if (hotkey.left(1) == "C")
      setShortcut(QString("Ctrl+%1").arg(hotkey.right(1)));

    else if (hotkey.left(1) == "F")
      setShortcut(hotkey);
  }

  connect(this, SIGNAL(activated()), pTarget, pActivateSlot);
  setEnabled(pEnabled);
  pAddTo->addAction(this);
  setIconSet(QIcon(pIcon));
  addTo(pToolBar);
}

Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, bool pEnabled,
                const QPixmap &pIcon, QWidget *pToolBar,
                const QString &pToolTip ) :
 QAction(pDisplayName, pParent)
{
  _name = pName;
  _displayName = pDisplayName;
  _toolTip = pToolTip;

  QString hotkey = _preferences->parent(pName);
  if (!hotkey.isNull() && !_hotkeyList.contains(hotkey))
  {
    _hotkeyList << hotkey;
    setShortcutContext(Qt::ApplicationShortcut);
    if (hotkey.left(1) == "C")
      setShortcut(QString("Ctrl+%1").arg(hotkey.right(1)));

    else if (hotkey.left(1) == "F")
      setShortcut(hotkey);
  }

  connect(this, SIGNAL(activated()), pTarget, pActivateSlot);
  setEnabled(pEnabled);
  pAddTo->addAction(this);
  setIconSet(QIcon(pIcon));
  addTo(pToolBar);
  setToolTip(_toolTip);
}

class OpenMFGCustInfoAction : public CustInfoAction
{
  public:
    void customerInformation(QWidget* parent, int pCustid)
    {
      ParameterList params;
      params.append("cust_id", pCustid);

      QWidget * w = parent;
      while(w && !w->isWindow())
        w = w->parentWidget();
      if(w && w->isModal())
        dspCustomerInformation::doDialog(w, params);
      else
      {
        dspCustomerInformation * newdlg = new dspCustomerInformation();
        newdlg->set(params);
        omfgThis->handleNewWindow(newdlg);
      }
    }
};

class OpenMFGCRMAcctInfoAction : public CRMAcctInfoAction
{
  public:
    void crmacctInformation(QWidget* parent, int pid)
    {
      ParameterList params;
      params.append("crmacct_id", pid);
      if (_privleges->check("MaintainCRMAccounts"))
	params.append("mode", "edit");
      else if (_privleges->check("ViewCRMAccounts"))
	params.append("mode", "view");
      else
	return;

      QWidget *w = parent;
      while (w && !w->isWindow())
	w = w->parentWidget();
      if (w && w->isModal())
      {
	crmaccount::doDialog(w, params);
      }
      else
      {
	crmaccount* newdlg = new crmaccount();
	newdlg->set(params);
	omfgThis->handleNewWindow(newdlg);
      }
    }
};

OpenMFGGUIClient *omfgThis;
OpenMFGGUIClient::OpenMFGGUIClient(const QString &pDatabaseURL, const QString &pUsername)
{
  _menuBar = 0;
  _activeWindow = 0;

  _databaseURL = pDatabaseURL;
  _username = pUsername;
  __saveSizePositionEventFilter = new SaveSizePositionEventFilter(this);

  _splash->showMessage(tr("Initializing Internal Data"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();

  _showTopLevel = false;
  if(_preferences->value("InterfaceWindowOption") == "TopLevel")
    _showTopLevel = true;

  __itemListSerial = 0;
  __custListSerial = 0;

  _q.exec("SELECT startOfTime() AS sot, endOfTime() AS eot;");
  if (_q.first())
  {
    _startOfTime = _q.value("sot").toDate();
    _endOfTime = _q.value("eot").toDate();
  }
  else
    systemError( this, tr( "A Critical Error occurred at %1::%2.\n"
                           "Please immediately log out and contact your Systems Adminitrator." )
                       .arg(__FILE__)
                       .arg(__LINE__) );

//  Create the application validators
  _qtyVal      = new QDoubleValidator(0, 99999999.0, 2, this);
  _transQtyVal = new QDoubleValidator(-99999999.0, 99999999.0, 2, this);
  _qtyPerVal   = new QDoubleValidator(0, 99999999.0, 4, this);
  _scrapVal    = new QDoubleValidator(0, 9999.0, 2, this);
  _percentVal  = new QDoubleValidator(0, 9999.0, 2, this);
  _moneyVal    = new QDoubleValidator(0, 9999999999.0, 2, this);
  _negMoneyVal = new QDoubleValidator(-9999999999.0, 9999999999.0, 2, this);
  _priceVal    = new QDoubleValidator(0, 9999999.0, 4, this);
  _costVal     = new QDoubleValidator(0, 9999999.0, 4, this);
  _ratioVal    = new QDoubleValidator(0, 9999999999.0, 5, this);
  _weightVal   = new QDoubleValidator(0, 99999999.0, 2, this);
  _runTimeVal  = new QDoubleValidator(0, 99999999.0, 1, this);
  _orderVal    = new QIntValidator(0, 999999, this);
  _dayVal      = new QIntValidator(0, 9999, this);

#ifdef Q_WS_MACX
  _assClient = new QAssistantClient((qApp->applicationDirPath() + "/../Resources"), this);
#else
  _assClient = new QAssistantClient(qApp->applicationDirPath(), this);
#endif

  connect(_assClient, SIGNAL(error(const QString &)), this, SLOT(sReportError(const QString &)));

  QStringList commands;
  commands //<< "-hideSidebar"
           << "-profile"
#ifdef Q_WS_MACX
           << qApp->applicationDirPath() + QString("/../Resources/helpXTupleGUIClient/XTupleGUIClient.adp");
#else
           << qApp->applicationDirPath() + QString("/helpXTupleGUIClient/XTupleGUIClient.adp");
#endif

  _assClient->setArguments(commands);

  //QFont f(qApp->font());
  //f.setPointSize(8);
  //qApp->setFont(f, TRUE);
  _fixedFont = new QFont("courier", 8);
  _systemFont = new QFont(qApp->font());

  Q3VBox *boxThis = new Q3VBox(this);
  boxThis->setFrameStyle(Q3Frame::StyledPanel | Q3Frame::Sunken);
  setCentralWidget(boxThis);

  _workspace = new QWorkspace(boxThis);

//  Install the InputManager
  _inputManager = new InputManager();
  qApp->installEventFilter(_inputManager);

#ifndef Q_WS_MACX
  setIcon(QPixmap(":/images/icon.xpm"));
#endif
  setCaption();

//  Populate the menu bar
#ifdef Q_WS_MACX
//  qt_mac_set_native_menubar(false);
#endif
  XSqlQuery window;
  window.prepare("SELECT usr_window "
		 "FROM usr "
		 "WHERE (usr_username=CURRENT_USER);");
  window.exec();
  // keep synchronized with user.ui.h
  _singleWindow = "";
  if (window.first())
    _singleWindow = window.value("usr_window").toString();
  if (_singleWindow.isEmpty())
    initMenuBar();

//  Load the user indicated background image
  _splash->showMessage(tr("Loading the Background Image"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();

  if (_preferences->value("BackgroundImageid").toInt() > 0)
  {
    _q.prepare( "SELECT image_data "
                "FROM image "
                "WHERE (image_id=:image_id);" );
    _q.bindValue(":image_id", _preferences->value("BackgroundImageid").toInt());
    _q.exec();
    if (_q.first())
    {
      QImage background;

      background.loadFromData(QUUDecode(_q.value("image_data").toString()));
      _workspace->setPaletteBackgroundPixmap(QPixmap::fromImage(background));
      _workspace->setBackgroundMode(Qt::FixedPixmap);
    }
  }

  _splash->showMessage(tr("Initializing Internal Timers"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();

  _eventButton = NULL;
  _registerButton = NULL;
  sTick();

  _timeoutHandler = new TimeoutHandler(this);
  connect(_timeoutHandler, SIGNAL(timeout()), this, SLOT(sIdleTimeout()));
  _timeoutHandler->setIdleMinutes(_preferences->value("IdleTimeout").toInt());
  _reportHandler = 0;

  OpenMFGCustInfoAction* ciAction = new OpenMFGCustInfoAction();
  CustInfo::_custInfoAction = ciAction;

  CRMAcctLineEdit::_crmacctInfoAction = new OpenMFGCRMAcctInfoAction();

  _splash->showMessage(tr("Completing Initialzation"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();
  _splash->finish(this);

  connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(sFocusChanged(QWidget*,QWidget*)));
}

bool OpenMFGGUIClient::singleCurrency()
{
    bool retValue = true;

    XSqlQuery currCount;
    currCount.exec("SELECT count(*) AS count FROM curr_symbol;");
    if (currCount.first())
	retValue = (currCount.value("count").toInt() <= 1);
    else
	systemError(this, currCount.lastError().databaseText(), __FILE__, __LINE__);
    return retValue;
}

void OpenMFGGUIClient::setCaption()
{
  QString name;

  _splash->showMessage(tr("Loading Database Information"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();

  _q.exec( "SELECT metric_value, CURRENT_USER AS username "
           "FROM metric "
           "WHERE (metric_name='DatabaseName')" );
  if (_q.first())
  {
    if (_q.value("metric_value").toString().isEmpty())
      name = tr("Unnamed Database");
    else
      name = _q.value("metric_value").toString();

    QString server;
    QString protocol;
    QString database;
    QString port;
    parseDatabaseURL(_databaseURL, protocol, server, database, port);

    if (_evaluation)
      QMainWindow::setCaption( tr("%1 Evaluation %2 - Logged on as %3")
                               .arg(_Name)
                               .arg(_Version)
                               .arg(_q.value("username").toString()) );
    else
      QMainWindow::setCaption( tr("%1 %2 - %3 on %4/%5 AS %6")
                               .arg(_Name)
                               .arg(_Version)
                               .arg(name)
                               .arg(server)
                               .arg(database)
                               .arg(_q.value("username").toString()) );
  }
  else
    QMainWindow::setCaption(_Name);
}

void OpenMFGGUIClient::initMenuBar()
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  menuBar()->clear();
  _hotkeyList.clear();

  QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(this);
  while(!toolbars.isEmpty())
    delete toolbars.takeFirst();

  if (!_preferences->boolean("UseOldMenu"))
  {
      if (_preferences->boolean("ShowPDMenu"))
      {
          _splash->showMessage(tr("Initializing the Products Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          productsMenu = new menuProducts(this);
      }
      
      if (_preferences->boolean("ShowIMMenu"))
      {
          _splash->showMessage(tr("Initializing the Inventory Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          inventoryMenu = new menuInventory(this);
      }
      
      if (_metrics->value("Application") == "OpenMFG")
      {
          if (_preferences->boolean("ShowMSMenu"))
          {
              _splash->showMessage(tr("Initializing the Scheduling Module"), SplashTextAlignment, SplashTextColor);
              qApp->processEvents();
              scheduleMenu = new menuSchedule(this);
          }
      }
      
      if (_preferences->boolean("ShowPOMenu"))
      {
          _splash->showMessage(tr("Initializing the Purchase Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          purchaseMenu = new menuPurchase(this);
      }
      
      if (_preferences->boolean("ShowWOMenu"))
      {
          _splash->showMessage(tr("Initializing the Manufacture Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          manufactureMenu = new menuManufacture(this);
      }
      
      if (_preferences->boolean("ShowCRMMenu"))
      {
          _splash->showMessage(tr("Initializing the CRM Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          crmMenu = new menuCRM(this);
      }
      
      if (_preferences->boolean("ShowSOMenu"))
      {
          _splash->showMessage(tr("Initializing the Sales Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          salesMenu = new menuSales(this);
      }
      
      if (_preferences->boolean("ShowGLMenu"))
      {
          _splash->showMessage(tr("Initializing the Accounting Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          accountingMenu = new menuAccounting(this);
      }
      
      _splash->showMessage(tr("Initializing the System Module"), SplashTextAlignment, SplashTextColor);
      qApp->processEvents();
      systemMenu = new menuSystem(this);
  }
  else
  {
      if (_preferences->boolean("ShowIMMenu"))
      {
          _splash->showMessage(tr("Initializing the I/M Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          imModule = new moduleIM(this);
      }
      
      if (_preferences->boolean("ShowPDMenu"))
      {
          _splash->showMessage(tr("Initializing the P/D Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          pdModule = new modulePD(this);
      }
      
      if (_metrics->value("Application") == "OpenMFG")
      {
          if (_preferences->boolean("ShowMSMenu"))
          {
              _splash->showMessage(tr("Initializing the M/S Module"), SplashTextAlignment, SplashTextColor);
              qApp->processEvents();
              msModule = new moduleMS(this);
          }
          
          if (_preferences->boolean("ShowCPMenu"))
          {
              _splash->showMessage(tr("Initializing the C/P Module"), SplashTextAlignment, SplashTextColor);
              qApp->processEvents();
              cpModule = new moduleCP(this);
          }
      }
      
      if (_preferences->boolean("ShowPOMenu"))
      {
          _splash->showMessage(tr("Initializing the P/O Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          poModule = new modulePO(this);
      }
      
      if (_preferences->boolean("ShowWOMenu"))
      {
          _splash->showMessage(tr("Initializing the W/O Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          woModule = new moduleWO(this);
      }
      
      if (_preferences->boolean("ShowCRMMenu"))
      {
          _splash->showMessage(tr("Initializing the CRM Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          crmModule = new moduleCRM(this);
      }
      
      if (_preferences->boolean("ShowSOMenu"))
      {
          _splash->showMessage(tr("Initializing the S/O Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          soModule = new moduleSO(this);
      }
      
      if (_preferences->boolean("ShowSRMenu"))
      {
          _splash->showMessage(tr("Initializing the S/R Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          srModule = new moduleSR(this);
      }
      
      if (_preferences->boolean("ShowSAMenu"))
      {
          _splash->showMessage(tr("Initializing the S/A Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          saModule = new moduleSA(this);
      }
      
      if (_preferences->boolean("ShowPMMenu"))
      {
          _splash->showMessage(tr("Initializing the P/M Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          pmModule = new modulePM(this);
      }
      
      if (_preferences->boolean("ShowAPMenu"))
      {
          _splash->showMessage(tr("Initializing the A/P Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          apModule = new moduleAP(this);
      }
      
      if (_preferences->boolean("ShowARMenu"))
      {
          _splash->showMessage(tr("Initializing the A/R Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          arModule = new moduleAR(this);
      }
      
      if (_preferences->boolean("ShowGLMenu"))
      {
          _splash->showMessage(tr("Initializing the G/L Module"), SplashTextAlignment, SplashTextColor);
          qApp->processEvents();
          glModule = new moduleGL(this);
      }
 
     _splash->showMessage(tr("Initializing the System Module"), SplashTextAlignment, SplashTextColor);
     qApp->processEvents();
     sysModule = new moduleSys(this);
  }
  // QSettings config("OpenMFG", "OpenMFG");
  // restoreState(config.value("MainWindowState", QByteArray()).toByteArray(), 1);

  qApp->restoreOverrideCursor();
}

void OpenMFGGUIClient::saveToolbarPositions()
{
  // QSettings config("OpenMFG", "OpenMFG");
  // config.setValue("MainWindowState", saveState(1));
}

void OpenMFGGUIClient::closeEvent(QCloseEvent *event)
{
  saveToolbarPositions();

//  Close the database connection
  QSqlDatabase::database().close();

  event->accept();
}

void OpenMFGGUIClient::sReportError(const QString &pError)
{
  qDebug(pError);
}

void OpenMFGGUIClient::sTick()
{
//  Check the database
  XSqlQuery tickle;
  tickle.exec( "SELECT CURRENT_DATE AS dbdate,"
               "       hasMessages() AS messages,"
               "       hasEvents() AS events;" );
  if (tickle.first())
  {
    _dbDate = tickle.value("dbdate").toDate();

    if (isVisible())
    {
      if (tickle.value("messages").toBool())
      {
//  Grab any new System Messages
        XSqlQuery msg;
        msg.exec( "SELECT msguser_id "
                  "FROM msg, msguser "
                  "WHERE ( (msguser_username=CURRENT_USER)"
                  " AND (msguser_msg_id=msg_id)"
                  " AND (CURRENT_TIMESTAMP BETWEEN msg_scheduled AND msg_expires)"
                  " AND (msguser_viewed IS NULL) );" );
        if (msg.first())
        {
          ParameterList params;
          params.append("mode", "acknowledge");

          systemMessage newdlg(this, "", TRUE);
          newdlg.set(params);

          do
          {
            ParameterList params;
            params.append("msguser_id", msg.value("msguser_id").toInt());

            newdlg.set(params);
            newdlg.exec();
          }
          while (msg.next());
        }
      }

//  Handle any un-dispatched Events
      if (tickle.value("events").toBool())
      {
        if (_eventButton)
        {
          if (!_eventButton->isVisible())
            _eventButton->show();
        }
        else
        {
          _eventButton = new QPushButton(QIcon(":/images/dspEvents.png"), "", statusBar());
          _eventButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
          _eventButton->setMinimumSize(QSize(32, 32));
          _eventButton->setMaximumSize(QSize(32, 32));
          statusBar()->setMinimumHeight(36);
          statusBar()->addWidget(_eventButton);

          if(!_preferences->boolean("UseOldMenu"))
            connect(_eventButton, SIGNAL(clicked()), systemMenu, SLOT(sEventManager()));
          else
            connect(_eventButton, SIGNAL(clicked()), sysModule, SLOT(sEventManager()));
        }
      }
      else if ( (_eventButton) && (_eventButton->isVisible()) )
        _eventButton->hide();

      if (_metrics->value("Application") != "OpenMFG" &&
          !_preferences->boolean("UseOldMenu"))
      {
        if (_registerButton)
          _registerButton->setVisible(_metrics->value("Registered") != "Yes");
        else
        {
          _registerButton = new QPushButton(QIcon(":/images/dspRegister.png"), "", statusBar());
          _registerButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
          _registerButton->setMinimumSize(QSize(32, 32));
          _registerButton->setMaximumSize(QSize(32, 32));
          statusBar()->setMinimumHeight(36);
          statusBar()->addWidget(_registerButton);

          connect(_registerButton, SIGNAL(clicked()), systemMenu, SLOT(sRegister()));
        }
      }
    }

    emit(tick());

    _tick.singleShot(60000, this, SLOT(sTick()));
  }
  else
    systemError(this, tr("<p>Your application was probably left idle too long "
                          "and has been disconnected from the database server."
                          "Try exiting the application and starting it again."
                          "<br><pre>%1</pre>" )
                      .arg(tickle.lastError().databaseText()));
}

//  Global notification slots
void OpenMFGGUIClient::sItemsUpdated(int intPItemid, bool boolPLocalUpdate)
{
  emit itemsUpdated(intPItemid, boolPLocalUpdate);
}

void OpenMFGGUIClient::sItemsitesUpdated()
{
  emit itemsitesUpdated();
}

void OpenMFGGUIClient::sWarehousesUpdated()
{
  emit warehousesUpdated();
}

void OpenMFGGUIClient::sCustomersUpdated(int pCustid, bool pLocal)
{
  emit customersUpdated(pCustid, pLocal);
}

void OpenMFGGUIClient::sGlSeriesUpdated()
{
  emit glSeriesUpdated();
}

void OpenMFGGUIClient::sVendorsUpdated()
{
  emit vendorsUpdated();
}

void OpenMFGGUIClient::sProspectsUpdated()
{
  emit prospectsUpdated();
}

void OpenMFGGUIClient::sReturnAuthorizationsUpdated()
{
  emit returnAuthorizationsUpdated();
}

void OpenMFGGUIClient::sStandardPeriodsUpdated()
{
  emit standardPeriodsUpdated();
}

void OpenMFGGUIClient::sSalesOrdersUpdated(int pSoheadid)
{
  emit salesOrdersUpdated(pSoheadid, TRUE);
}

void OpenMFGGUIClient::sCreditMemosUpdated()
{
  emit creditMemosUpdated();
}

void OpenMFGGUIClient::sQuotesUpdated(int pQuheadid)
{
  emit quotesUpdated(pQuheadid, TRUE);
}

void OpenMFGGUIClient::sWorkOrderMaterialsUpdated(int pWoid, int pWomatlid, bool pLocalUpdate)
{
  emit workOrderMaterialsUpdated(pWoid, pWomatlid, pLocalUpdate);
}

void OpenMFGGUIClient::sWorkOrderOperationsUpdated(int pWoid, int pWooperid, bool pLocalUpdate)
{
  emit workOrderOperationsUpdated(pWoid, pWooperid, pLocalUpdate);
}

void OpenMFGGUIClient::sWorkOrdersUpdated(int pWoid, bool pLocalUpdate)
{
  emit workOrdersUpdated(pWoid, pLocalUpdate);
}

void OpenMFGGUIClient::sPurchaseOrdersUpdated(int pPoheadid, bool pLocalUpdate)
{
  emit purchaseOrdersUpdated(pPoheadid, pLocalUpdate);
}

void OpenMFGGUIClient::sPurchaseOrderReceiptsUpdated()
{
  emit purchaseOrderReceiptsUpdated();
}

void OpenMFGGUIClient::sPurchaseRequestsUpdated()
{
  emit purchaseRequestsUpdated();
}

void OpenMFGGUIClient::sVouchersUpdated()
{
  emit vouchersUpdated();
}

void OpenMFGGUIClient::sBOMsUpdated(int intPItemid, bool boolPLocalUpdate)
{
  emit bomsUpdated(intPItemid, boolPLocalUpdate);
}

void OpenMFGGUIClient::sBBOMsUpdated(int intPItemid, bool boolPLocalUpdate)
{
  emit bbomsUpdated(intPItemid, boolPLocalUpdate);
}

void OpenMFGGUIClient::sBOOsUpdated(int intPItemid, bool boolPLocalUpdate)
{
  emit boosUpdated(intPItemid, boolPLocalUpdate);
}

void OpenMFGGUIClient::sBudgetsUpdated(int intPItemid, bool boolPLocalUpdate)
{
  emit budgetsUpdated(intPItemid, boolPLocalUpdate);
}

void OpenMFGGUIClient::sAssortmentsUpdated(int pItemid, bool pLocalUpdate)
{
  emit assortmentsUpdated(pItemid, pLocalUpdate);
}

void OpenMFGGUIClient::sWorkCentersUpdated()
{
  emit workCentersUpdated();
}

void OpenMFGGUIClient::sBillingSelectionUpdated(int pCoheadid, int pCoitemid)
{
  emit billingSelectionUpdated(pCoheadid, pCoitemid);
}

void OpenMFGGUIClient::sInvoicesUpdated(int pInvcheadid, bool pLocal)
{
  emit invoicesUpdated(pInvcheadid, pLocal);
}

void OpenMFGGUIClient::sItemGroupsUpdated(int pItemgrpid, bool pLocal)
{
  emit itemGroupsUpdated(pItemgrpid, pLocal);
}

void OpenMFGGUIClient::sCashReceiptsUpdated(int pCashrcptid, bool pLocal)
{
  emit cashReceiptsUpdated(pCashrcptid, pLocal);
}

void OpenMFGGUIClient::sBankAdjustmentsUpdated(int pBankadjid, bool pLocal)
{
  emit bankAdjustmentsUpdated(pBankadjid, pLocal);
}

void OpenMFGGUIClient::sQOHChanged(int pItemsiteid, bool pLocal)
{
  emit qohChanged(pItemsiteid, pLocal);
}

void OpenMFGGUIClient::sReportsChanged(int pReportid, bool pLocal)
{
  emit reportsChanged(pReportid, pLocal);
}

void OpenMFGGUIClient::sChecksUpdated(int pBankaccntid, int pCheckid, bool pLocal)
{
  emit checksUpdated(pBankaccntid, pCheckid, pLocal);
  emit paymentsUpdated(pBankaccntid, -1, pLocal);
}

void OpenMFGGUIClient::sPaymentsUpdated(int pBankaccntid, int pApselectid, bool pLocal)
{
  emit paymentsUpdated(pBankaccntid, pApselectid, pLocal);
}

void OpenMFGGUIClient::sConfigureGLUpdated()
{
  emit configureGLUpdated();
}

void OpenMFGGUIClient::sProjectsUpdated(int prjid)
{
  emit projectsUpdated(prjid);
}

void OpenMFGGUIClient::sCrmAccountsUpdated(int crmacctid)
{
  emit crmAccountsUpdated(crmacctid);
}

void OpenMFGGUIClient::sTaxAuthsUpdated(int taxauthid)
{
  emit taxAuthsUpdated(taxauthid);
}

void OpenMFGGUIClient::sTransferOrdersUpdated(int id)
{
  emit transferOrdersUpdated(id);
}

void OpenMFGGUIClient::sIdleTimeout()
{
 // so we don't accidentally get called again waiting
  _timeoutHandler->reset();

  ParameterList params;
  params.append("minutes", _timeoutHandler->idleMinutes());
  
  idleShutdown newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() == QDialog::Accepted)
    qApp->quit();
}

int systemError(QWidget *pParent, const QString &pMessage)
{
  int result = QMessageBox::critical( pParent, QObject::tr("System Message"),
                                      pMessage + QObject::tr("\nReport this to your Systems Administrator.") );
  return result;
}

int systemError(QWidget *pParent, const QString &pMessage, const QString &pFileName, const int lineNumber)
{
  int result = QMessageBox::critical( pParent,
				      QObject::tr("System Message (%1 at %2)")
				      .arg(pFileName)
				      .arg(lineNumber),
                                      pMessage + QObject::tr("\nReport this to your Systems Administrator.") );
  return result;
}

void message(const QString &pMessage, int pTimeout)
{
  if (pTimeout == 0)
    omfgThis->statusBar()->message(pMessage);
  else
    omfgThis->statusBar()->message(pMessage, pTimeout);

  qApp->processEvents();
}

void resetMessage()
{
  omfgThis->statusBar()->message(QObject::tr("Ready..."));
  qApp->processEvents();
}

void audioAccept()
{
  qApp->beep();
}

void audioReject()
{
  qApp->beep();
}


void OpenMFGGUIClient::populateCustomMenu(QMenu * menu, const QString & module)
{
  QMenu *customMenu = 0;
  XSqlQuery qry;
  qry.prepare("SELECT cmd_id, cmd_title, cmd_privname"
              "  FROM cmd"
              " WHERE (cmd_module=:module)"
              " ORDER BY cmd_title, cmd_id; ");
  qry.bindValue(":module", module);
  qry.exec();
  while(qry.next())
  {
    if(customMenu == 0)
      customMenu = menu->addMenu(tr("Custom"));

    bool allowed = true;
    QString privname = qry.value("cmd_privname").toString();
    if(!privname.isEmpty())
      allowed = _privleges->check("Custom"+privname);

    Action * action = new Action( this, QString("custom.")+qry.value("cmd_title").toString(), qry.value("cmd_title").toString(),
      this, SLOT(sCustomCommand()), customMenu, allowed);

    _customCommands.insert(action, qry.value("cmd_id").toInt());
    actions.append(action);
  }
}

void OpenMFGGUIClient::sCustomCommand()
{
  const QObject * obj = sender();
  QMap<const QObject*,int>::const_iterator it;
  it = _customCommands.find(obj);
  if(it != _customCommands.end())
  {
    Q3Process *proc = new Q3Process(this);
    connect(proc, SIGNAL(processExited()), proc, SLOT(deleteLater()));
    q.prepare("SELECT 1 AS base,"
              "       0 AS ord,"
              "       cmd_executable AS argument"
              "  FROM cmd"
              " WHERE (cmd_id=:cmd_id)"
              " UNION "
              "SELECT 2 AS base,"
              "       cmdarg_order AS ord,"
              "       cmdarg_arg AS argument"
              "  FROM cmdarg"
              " WHERE (cmdarg_cmd_id=:cmd_id)"
              " ORDER BY base, ord; ");
    q.bindValue(":cmd_id", it.data());
    q.exec();
    while(q.next())
      proc->addArgument(q.value("argument").toString());
    proc->start();
  }
}

void OpenMFGGUIClient::launchBrowser(QWidget * w, const QString & url)
{
#if defined(Q_OS_WIN32)
  // Windows - let the OS do the work
  QT_WA( {
      ShellExecute(w->winId(), 0, (TCHAR*)url.ucs2(), 0, 0, SW_SHOWNORMAL );
    } , {
      ShellExecuteA( w->winId(), 0, url.local8Bit(), 0, 0, SW_SHOWNORMAL );
    } );
#else
  const char *b = getenv("BROWSER");
  QStringList browser;
  if(b) {
    browser = QStringList::split(':', b);
  }
#if defined(Q_OS_MACX)
  browser.append("/usr/bin/open");
#else
  // append this on linux just as a good guess
  browser.append("/usr/bin/firefox");
  browser.append("/usr/bin/mozilla");
#endif
  for(QStringList::const_iterator cit=browser.begin(); cit!=browser.end(); ++cit) {
    QString app = *cit;
    if(app.contains("%s")) {
      app.replace("%s", url);
    } else {
      app += " " + url;
    }
    app.replace("%%", "%");
    Q3Process *proc = new Q3Process(w);
    connect(proc, SIGNAL(processExited()), proc, SLOT(deleteLater()));
    proc->setArguments(QStringList::split(QRegExp(" +"), app));
    if(proc->start())
      return;
  }

  // There was an error. Offer the user a chance to look at the online help to
  // tell them about the BROWSER variable
  if(1==QMessageBox::warning(w, tr("Failed to open URL"), url, tr("OK"), tr("Help"))) {
    //launchHelp("browser.html");
    QMessageBox::information( w, tr("Quick Help"),
      tr("Before you can run a browser you must set the environment variable BROWSER to\n"
         "point to the browser executable.") );
  }
#endif
}

QWidgetList OpenMFGGUIClient::windowList()
{
  if(_showTopLevel)
    return _windowList;
  else
    return _workspace->windowList();
}

void OpenMFGGUIClient::windowDestroyed(QObject * o)
{
  QWidget * w = qobject_cast<QWidget *>(o);
  if(w)
  {
    if(w == _activeWindow)
      _activeWindow = 0;

    _windowList.removeAll(w);
  }
}

bool SaveSizePositionEventFilter::eventFilter(QObject *obj, QEvent *event)
{
  if(event->type() == QEvent::Close)
  {
    QWidget * w = qobject_cast<QWidget *>(obj);
    if(w)
    {
      QString objName = w->objectName();
      QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
      settings.setValue(objName + "/geometry/size", w->size());
      if(omfgThis->showTopLevel())
        settings.setValue(objName + "/geometry/pos", w->pos());
      else
        settings.setValue(objName + "/geometry/pos", w->parentWidget()->pos());
    }
  }
  return QObject::eventFilter(obj, event);
}

void OpenMFGGUIClient::handleNewWindow(QWidget * w, Qt::WindowModality m)
{
  w->setWindowModality(m);

  connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(windowDestroyed(QObject*)));

  QRect availableGeometry = QApplication::desktop()->availableGeometry();
  if(!_showTopLevel)
    availableGeometry = _workspace->geometry();

  QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
  QString objName = w->objectName();
  QPoint pos = settings.value(objName + "/geometry/pos").toPoint();
  QSize size = settings.value(objName + "/geometry/size").toSize();

  if(size.isValid() && settings.value(objName + "/geometry/rememberSize", true).toBool())
    w->resize(size);

  if(_showTopLevel)
  {
    _windowList.append(w);
    w->setWindowFlags(Qt::WDestructiveClose);
    QMainWindow *mw = qobject_cast<QMainWindow*>(w);
    if (mw)
      mw->statusBar()->show();
	QRect r(pos, w->size());
    if(!pos.isNull() && availableGeometry.contains(r) && settings.value(objName + "/geometry/rememberPos", true).toBool())
      w->move(pos);
    w->show();
  }
  else
  {
    QWidget * fw = w->focusWidget();
    w->setAttribute(Qt::WA_DeleteOnClose);
    _workspace->addWindow(w);
    QRect r(pos, w->size());
    if(!pos.isNull() && availableGeometry.contains(r) && settings.value(objName + "/geometry/rememberPos", true).toBool())
      w->move(pos);
    w->show();
    if(fw)
      fw->setFocus();
  }

  w->installEventFilter(__saveSizePositionEventFilter);
}

QMenuBar *OpenMFGGUIClient::menuBar()
{
#ifdef Q_WS_MACX
  if (_menuBar == 0)
    _menuBar = new QMenuBar();

  return _menuBar;
#else
  return QMainWindow::menuBar();
#endif
}

void OpenMFGGUIClient::sFocusChanged(QWidget * /*old*/, QWidget * /*now*/)
{
  QWidget * thisActive = workspace()->activeWindow();
  if(omfgThis->showTopLevel())
    thisActive = qApp->activeWindow();
  if(thisActive == this)
    return;
  _activeWindow = thisActive;
}

QWidget * OpenMFGGUIClient::myActiveWindow()
{
  return _activeWindow;
}
