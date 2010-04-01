/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QTimer>
#include <QAction>
#include <QVBoxLayout>
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
#include <QProcess>
#include <QSqlError>
#include <QPixmap>
#include <QFrame>
#include <QTextStream>
#include <QCloseEvent>
#include <QMainWindow>
#include <QDesktopWidget>
#include <QDebug>
#include <QScriptEngine>
#include <QScriptValue>
#include <QBuffer>
#include <QDesktopServices>
#include <QScriptEngineDebugger>

#include <parameter.h>
#include <dbtools.h>
#include <quuencode.h>
#include <xvariant.h>

#include "xtsettings.h"
#include "xuiloader.h"
#include "guiclient.h"
#include "version.h"
#include "xmainwindow.h"
#include "xdialog.h"
#include "errorLog.h"

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

#include "timeoutHandler.h"
#include "idleShutdown.h"
#include "inputManager.h"
#include "xdoublevalidator.h"

#include "custcluster.h"
#include "crmacctcluster.h"
#include "crmaccount.h"
#include "customer.h"
#include "distributeInventory.h"
#include "splashconst.h"
#include "scripttoolbox.h"

#include "setupscriptapi.h"

#define CHECK_REGISTERED 0

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
class Privileges;
class Metricsenc;

#ifdef Q_WS_MACX
void qt_mac_set_native_menubar(bool);
#endif

QSplashScreen *_splash;

Metrics       *_metrics=0;
Preferences   *_preferences=0;
Privileges    *_privileges=0;
Metricsenc    *_metricsenc=0;
QList<QString> _hotkeyList;

bool _evaluation;

#include <SaveSizePositionEventFilter.h>
static SaveSizePositionEventFilter * __saveSizePositionEventFilter = 0;

static int __interval = 0;
static int __intervalCount = 0;

void collectMetrics();

// #name is a special check like calling a function
// @name:mode is a class call to static method userHasPriv(mode)
//     where mode is one of new edit view
static bool __privCheck(const QString & privname)
{
  if(privname == "#superuser")
  {
    XSqlQuery su;
    su.exec("SELECT rolsuper FROM pg_roles WHERE (rolname=CURRENT_USER);");
    if (su.first())
      return su.value("rolsuper").toBool();
    else
      return false;
  }
  return _privileges->check(privname);
}

static void __menuEvaluate(QAction * act)
{
  if(!act) return;
  QString privs = act->data().toString();
  if(privs == "true")
    act->setEnabled(true);
  else if(privs == "false")
    act->setEnabled(false);
  else if(!privs.isEmpty())
  {
    bool enable = false;
    QStringList privlist = privs.split(' ', QString::SkipEmptyParts);
    for (int i = 0; i < privlist.size(); ++i)
    {
      bool tb = true;
      QStringList privandlist = privlist.at(i).split('+', QString::SkipEmptyParts);
      if(privandlist.size() > 1)
      {
        for(int ii = 0; ii < privandlist.size(); ++ii)
          tb = tb && __privCheck(privandlist.at(ii));
      }
      else
        tb = enable || __privCheck(privlist.at(i));
      enable = enable || tb;
    }
    act->setEnabled(enable);
  }
}

static QScriptValue settingsValue(QScriptContext *context, QScriptEngine * /*engine*/)
{
  if (context->argumentCount() == 2)
    return xtsettingsValue(context->argument(0).toString(), context->argument(1).toVariant()).toString();
  else
    return xtsettingsValue(context->argument(0).toString()).toString();
}

static QScriptValue settingsSetValue(QScriptContext *context, QScriptEngine *engine)
{
  xtsettingsSetValue(context->argument(0).toString(), context->argument(1).toVariant());
  return settingsValue(context, engine);
}

Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, bool pEnabled ) :
 QAction(pDisplayName, pParent)
{
  init(pParent, pName, pDisplayName, pTarget, pActivateSlot, pAddTo, (pEnabled?"true":"false"));
}

Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, bool pEnabled,
                const QPixmap &pIcon, QWidget *pToolBar ) :
 QAction(pDisplayName, pParent)
{
  init(pParent, pName, pDisplayName, pTarget, pActivateSlot, pAddTo, (pEnabled?"true":"false"));

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
  init(pParent, pName, pDisplayName, pTarget, pActivateSlot, pAddTo, (pEnabled?"true":"false"));

  setIconSet(QIcon(pIcon));
  addTo(pToolBar);
  setToolTip(pToolTip);
}

Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, const QString & pEnabled ) :
 QAction(pDisplayName, pParent)
{
  init(pParent, pName, pDisplayName, pTarget, pActivateSlot, pAddTo, pEnabled);
}

Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, const QString & pEnabled,
                const QPixmap &pIcon, QWidget *pToolBar ) :
 QAction(pDisplayName, pParent)
{
  init(pParent, pName, pDisplayName, pTarget, pActivateSlot, pAddTo, pEnabled);

  setIconSet(QIcon(pIcon));
  addTo(pToolBar);
}

Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, const QString & pEnabled,
                const QPixmap &pIcon, QWidget *pToolBar,
                const QString &pToolTip ) :
 QAction(pDisplayName, pParent)
{
  init(pParent, pName, pDisplayName, pTarget, pActivateSlot, pAddTo, pEnabled);

  setIconSet(QIcon(pIcon));
  addTo(pToolBar);
  setToolTip(pToolTip);
}

void Action::init( QWidget *, const char *pName, const QString &/*pDisplayName*/,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, const QString & pEnabled )
{
  setObjectName(pName);

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
  //setEnabled(pEnabled);
  pAddTo->addAction(this);

  if(!pEnabled.isEmpty())
    setData(pEnabled);
  __menuEvaluate(this);
}

class xTupleGuiClientInterface : public GuiClientInterface
{
  public:
  QWidget* openWindow(const QString pname, ParameterList pparams, QWidget *parent = 0, Qt::WindowModality modality = Qt::NonModal, Qt::WindowFlags flags = 0)
  {
    ScriptToolbox toolbox(0);
		QWidget* w = toolbox.openWindow(pname, parent, modality, flags);
		
    if (w)
    {
      if (w->inherits("QDialog"))
      {
        XDialog* xdlg = (XDialog*)w;
        xdlg->set(pparams);
        w = (QWidget*)xdlg;
        return w;
      }
      else if (w->inherits("XMainWindow"))
      {
        XMainWindow* xwind = (XMainWindow*)w;
        xwind->set(pparams);
        w = (QWidget*)xwind;
        w->show();
        return w;
      }
      else if (w->inherits("XWidget"))
      {
        XWidget* xwind = (XWidget*)w;
        xwind->set(pparams);
        w = (QWidget*)xwind;
        w->show();
        return w;
      }
    }
    return 0;
  }
};

class xTupleCustInfoAction : public CustInfoAction
{
  public:
    void customerInformation(QWidget* parent, int pCustid)
    {
      ParameterList params;
      params.append("cust_id", pCustid);
      if (_privileges->check("ViewCustomerMasters"))
        params.append("mode","edit");
      else
        params.append("mode","view");

      QWidget * w = parent;
      while(w && !w->isWindow())
        w = w->parentWidget();
      if(w && w->isModal())
      {
        params.append("modal");
        customer * newdlg = new customer(w, 0, Qt::Window);
        newdlg->set(params);
        omfgThis->handleNewWindow(newdlg);
      }
      else
      {
        customer * newdlg = new customer();
        newdlg->set(params);
        omfgThis->handleNewWindow(newdlg);
      }
    }
};

GUIClient *omfgThis;
GUIClient::GUIClient(const QString &pDatabaseURL, const QString &pUsername)
{
  _menuBar = 0;
  _activeWindow = 0;
  _shown = false;
  _shuttingDown = false;

  _databaseURL = pDatabaseURL;
  _username = pUsername;
  __saveSizePositionEventFilter = new SaveSizePositionEventFilter(this);
  _useCloud = false;

  _splash->showMessage(tr("Initializing Internal Data"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();

  _showTopLevel = true;
  if(_preferences->value("InterfaceWindowOption") == "Workspace")
    _showTopLevel = false;

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

  /*  TODO: either separate validators for extprice, purchprice, and salesprice
            or replace every field that uses _moneyVal, _negMoneyVal, _priceVal, and _costVal
               with CurrCluster or CurrDisplay
  */
  _qtyVal         = new XDoubleValidator(0,              99999999.0, decimalPlaces("qty"),     this);
  _transQtyVal    = new XDoubleValidator(-99999999.0,    99999999.0, decimalPlaces("qty"),     this);
  _qtyPerVal      = new XDoubleValidator(0,              99999999.0, decimalPlaces("qtyper"),  this);
  _scrapVal       = new XDoubleValidator(0,                  9999.0, decimalPlaces("percent"), this);
  _percentVal     = new XDoubleValidator(0,                  9999.0, decimalPlaces("percent"), this);
  _negPercentVal  = new XDoubleValidator(-9999.0,            9999.0, decimalPlaces("percent"), this);
  _moneyVal       = new XDoubleValidator(0,            9999999999.0, decimalPlaces("curr"),    this);
  _negMoneyVal    = new XDoubleValidator(-9999999999.0,9999999999.0, decimalPlaces("curr"),    this);
  _priceVal       = new XDoubleValidator(0,               9999999.0, decimalPlaces("purchprice"), this);
  _costVal        = new XDoubleValidator(0,               9999999.0, decimalPlaces("cost"), this);
  _ratioVal       = new XDoubleValidator(0,            9999999999.0, decimalPlaces("uomratio"), this);
  _weightVal      = new XDoubleValidator(0,              99999999.0, decimalPlaces("weight"), this);
  _runTimeVal     = new XDoubleValidator(0,              99999999.0, 2, this);
  _orderVal       = new QIntValidator(0, 999999, this);
  _dayVal         = new QIntValidator(0, 9999, this);

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

  _fixedFont = new QFont("courier", 8);
  _systemFont = new QFont(qApp->font());

  // TODO: replace QWorkspace with QMdiArea
  _workspace = new QWorkspace();
  setCentralWidget(_workspace);
  _workspace->setScrollBarsEnabled(true);
  //_workspace->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

  _workspace->setContentsMargins(0, 0, 0, 0);

//  Install the InputManager
  _inputManager = new InputManager();
  qApp->installEventFilter(_inputManager);

  setWindowTitle();

//  Populate the menu bar
#ifdef Q_WS_MACX
//  qt_mac_set_native_menubar(false);
#endif
  XSqlQuery window;
  window.prepare("SELECT usr_window "
                 "  FROM usr "
                 " WHERE (usr_username=CURRENT_USER);");
  window.exec();
  // keep synchronized with user.ui.h
  _singleWindow = "";
  if (window.first())
    _singleWindow = window.value("usr_window").toString();
  if (_singleWindow.isEmpty())
    initMenuBar();
  else
    _showTopLevel = true; // if we are in single level mode we want to run toplevel always

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
  _errorButton = NULL;
  __interval = _metrics->value("updateTickInterval").toInt();
  if(__interval < 1)
    __interval = 1;
  __intervalCount = 0;
  sTick();

  _timeoutHandler = new TimeoutHandler(this);
  connect(_timeoutHandler, SIGNAL(timeout()), this, SLOT(sIdleTimeout()));
  _timeoutHandler->setIdleMinutes(_preferences->value("IdleTimeout").toInt());
  _reportHandler = 0;

  //ContactCluster::_guiClientInterface = new xTupleGuiClientInterface();
  VirtualClusterLineEdit::_guiClientInterface = new xTupleGuiClientInterface();
	Documents::_guiClientInterface = VirtualClusterLineEdit::_guiClientInterface;

  xTupleCustInfoAction* ciAction = new xTupleCustInfoAction();
  CustInfo::_custInfoAction = ciAction;

  _splash->showMessage(tr("Completing Initialzation"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();
  _splash->finish(this);

  connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(sFocusChanged(QWidget*,QWidget*)));

  //Restore Window Size Saved on Close                                                                       
  QRect availableGeometry = QApplication::desktop()->availableGeometry();                                    
  QPoint pos = xtsettingsValue("GUIClient/geometry/pos", QPoint()).toPoint();                           
  QSize size = xtsettingsValue("GUIClient/geometry/size", QSize()).toSize();                          
  int mainXMax = availableGeometry.bottomRight().x();                                                        
  int mainYMax = availableGeometry.bottomRight().y();                                                        
                                                                                                             
  //if the window size is bigger than the                                                                    
  //screen make it the screen size                                                                           
  if(size.isValid())
  {
    if(size.width() > mainXMax)                                                                                
       size.setWidth(mainXMax);                                                                                
    if(size.height() > mainYMax)                                                                               
      size.setHeight(mainYMax);                                                                                
    resize(size);                                                                                              
  }
  //put the main window back on screen at top                                                                
  //left if it is off screen in part or full                                                                 
  if(!pos.isNull())
  {
    if(pos.x() < 0 || (pos.x() + size.width()) > mainXMax)                                                     
      pos.setX(0);                                                                                             
    if(pos.y() < 0 || (pos.y() + size.height()) > mainYMax)                                                    
      pos.setY(0);                                                                                             
    move(pos);                                                                                                 
  }

  collectMetrics();
}

GUIClient::~GUIClient()
{
  QApplication::closeAllWindows();

  errorLogListener::destroy();
  //omfgThis = 0;

  // Close the database connection
  QSqlDatabase::database().close();
}

bool GUIClient::singleCurrency()
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

void GUIClient::setWindowTitle()
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
      QMainWindow::setWindowTitle( tr("%1 Evaluation %2 - Logged on as %3")
                               .arg(_Name)
                               .arg(_Version)
                               .arg(_q.value("username").toString()) );
    else
      QMainWindow::setWindowTitle( tr("%1 %2 - %3 on %4/%5 AS %6")
                               .arg(_Name)
                               .arg(_Version)
                               .arg(name)
                               .arg(server)
                               .arg(database)
                               .arg(_q.value("username").toString()) );
  }
  else
    QMainWindow::setWindowTitle(_Name);
}

void GUIClient::setCaption()
{
}

void GUIClient::initMenuBar()
{
  static bool firstRun = true;

  qApp->setOverrideCursor(Qt::WaitCursor);

  if(!firstRun)
  {
    QList<QMenu*> menulist = findChildren<QMenu*>();
    for(int m = 0; m < menulist.size(); ++m)
    {
      QList<QAction*> actionlist = menulist.at(m)->actions();
      for(int i = 0; i < actionlist.size(); ++i)
        __menuEvaluate(actionlist.at(i));
    }
  }
  else
  {
    menuBar()->clear();
    _hotkeyList.clear();

    QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(this);
    while(!toolbars.isEmpty())
      delete toolbars.takeFirst();
  
    _splash->showMessage(tr("Initializing the Products Module"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
    productsMenu = new menuProducts(this);
        
    _splash->showMessage(tr("Initializing the Inventory Module"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
    inventoryMenu = new menuInventory(this);
        
    if(_metrics->value("Application") != "PostBooks")
    {
      _splash->showMessage(tr("Initializing the Scheduling Module"), SplashTextAlignment, SplashTextColor);
      qApp->processEvents();
      scheduleMenu = new menuSchedule(this);
    }
    
    _splash->showMessage(tr("Initializing the Purchase Module"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
    purchaseMenu = new menuPurchase(this);
    
    _splash->showMessage(tr("Initializing the Manufacture Module"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
    manufactureMenu = new menuManufacture(this);
    
    _splash->showMessage(tr("Initializing the CRM Module"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
    crmMenu = new menuCRM(this);
    
    _splash->showMessage(tr("Initializing the Sales Module"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
    salesMenu = new menuSales(this);
        
    _splash->showMessage(tr("Initializing the Accounting Module"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
    accountingMenu = new menuAccounting(this);
    
    _splash->showMessage(tr("Initializing the System Module"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
    systemMenu = new menuSystem(this);
  
    restoreState(xtsettingsValue("MainWindowState", QByteArray()).toByteArray(), 1);
  
    toolbars = qFindChildren<QToolBar *>(this);
    for (int i = 0; i < toolbars.size(); ++i)
    {
      toolbars.at(i)->show();
    }
  }

  findChild<QMenu*>("menu.prod")->menuAction()->setVisible(_preferences->boolean("ShowPDMenu"));
  findChild<QMenu*>("menu.im")->menuAction()->setVisible(_preferences->boolean("ShowIMMenu"));
  if(_metrics->value("Application") != "PostBooks")
  {
    findChild<QMenu*>("menu.sched")->menuAction()->setVisible(_preferences->boolean("ShowMSMenu"));
  }
  findChild<QMenu*>("menu.purch")->menuAction()->setVisible(_preferences->boolean("ShowPOMenu"));
  findChild<QMenu*>("menu.manu")->menuAction()->setVisible(_preferences->boolean("ShowWOMenu"));
  findChild<QMenu*>("menu.crm")->menuAction()->setVisible(_preferences->boolean("ShowCRMMenu"));
  findChild<QMenu*>("menu.sales")->menuAction()->setVisible(_preferences->boolean("ShowSOMenu"));
  findChild<QMenu*>("menu.accnt")->menuAction()->setVisible(_preferences->boolean("ShowGLMenu"));

  firstRun = false;
  qApp->restoreOverrideCursor();
}

void GUIClient::saveToolbarPositions()
{
  xtsettingsSetValue("MainWindowState", saveState(1));
}

void GUIClient::closeEvent(QCloseEvent *event)
{
  _shuttingDown = true;

  saveToolbarPositions();

  // save main window size for next login
  xtsettingsSetValue("GUIClient/geometry/pos", pos());
  xtsettingsSetValue("GUIClient/geometry/size", size());

  event->accept();
}

void GUIClient::showEvent(QShowEvent *event)
{
  if(!_shown)
  {
    _shown = true;
    // We only want the scripting to work on the NEO menu
    // START script code
      XSqlQuery sq;
      sq.prepare("SELECT script_source, script_order"
              "  FROM script"
              " WHERE((script_name=:script_name)"
              "   AND (script_enabled))"
              " ORDER BY script_order;");
      sq.bindValue(":script_name", "initMenu");
      sq.exec();
      QScriptEngine * engine = 0;
      QScriptEngineDebugger * debugger = 0;
      bool found_one = false;
      while(sq.next())
      {
        found_one = true;
        QString script = sq.value("script_source").toString();
        if(!engine)
        {
          engine = new QScriptEngine(this);
          if (_preferences->boolean("EnableScriptDebug"))
          {
            debugger = new QScriptEngineDebugger(this);
            debugger->attachTo(engine);
          }
          loadScriptGlobals(engine);
        }
  
        QScriptValue result = engine->evaluate(script, "initMenu");
        if (engine->hasUncaughtException())
        {
          int line = engine->uncaughtExceptionLineNumber();
          qDebug() << "uncaught exception at line" << line << ":" << result.toString();
        }
      }
      if(found_one)
      {
        QList<QMenu*> menulist = findChildren<QMenu*>();
        for(int m = 0; m < menulist.size(); ++m)
        {
          QList<QAction*> actionlist = menulist.at(m)->actions();
          for(int i = 0; i < actionlist.size(); ++i)
          {
            QAction* act = actionlist.at(i);
            if(!act->objectName().isEmpty())
            {
              QString hotkey;
              hotkey = _preferences->parent(act->objectName());
              if (!hotkey.isNull() && !_hotkeyList.contains(hotkey))
              {
                _hotkeyList << hotkey;
                act->setShortcutContext(Qt::ApplicationShortcut);
                if (hotkey.left(1) == "C")
                  act->setShortcut(QString("Ctrl+%1").arg(hotkey.right(1)));

                else if (hotkey.left(1) == "F")
                  act->setShortcut(hotkey);
              }
            }
          }
        }
      }
    // END script code
  }
  QMainWindow::showEvent(event);
}

void GUIClient::sReportError(const QString &pError)
{
  qDebug("%s", qPrintable(pError));
}

void GUIClient::sTick()
{
//  Check the database
  XSqlQuery tickle;
  tickle.exec( "SELECT CURRENT_DATE AS dbdate,"
               "       hasAlarms() AS alarms,"
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

          connect(_eventButton, SIGNAL(clicked()), systemMenu, SLOT(sEventManager()));
        }
      }
      else if ( (_eventButton) && (_eventButton->isVisible()) )
        _eventButton->hide();

#if CHECK_REGISTERED
      if (_metrics->value("Application") == "PostBooks")
      {
        if(_metrics->value("Registered") != "Yes" && xtsettingsValue("/xTuple/Registered").toString() != "Yes")
        {
          if (_registerButton)
            _registerButton->setVisible(true);
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
#endif
    }

    __intervalCount++;
    if(__intervalCount >= __interval)
    {
      emit(tick());
      __intervalCount = 0;
    }

    _tick.singleShot(60000, this, SLOT(sTick()));
  }
  else
    systemError(this, tr("<p>You have been disconnected from the database server.  "
                          "This is usually caused by an interruption in your "
                          "network.  Please exit the application and restart."
                          "<br><pre>%1</pre>" )
                      .arg(tickle.lastError().databaseText()));
}

void GUIClient::sNewErrorMessage()
{
  if (_errorButton)
    _errorButton->setVisible(_metrics->value("Registered") != "Yes" && xtsettingsValue("/xTuple/Registered").toString() != "Yes");
  else
  {
    _errorButton = new QPushButton(QIcon(":/images/dspError.png"), "", statusBar());
    _errorButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _errorButton->setMinimumSize(QSize(32, 32));
    _errorButton->setMaximumSize(QSize(32, 32));
    statusBar()->setMinimumHeight(36);
    statusBar()->addWidget(_errorButton);

    connect(_errorButton, SIGNAL(clicked()), systemMenu, SLOT(sErrorLog()));
  }
}

void GUIClient::sClearErrorMessages()
{
  if (_errorButton)
    _errorButton->setVisible(false);
}

//  Global notification slots
void GUIClient::sItemsUpdated(int intPItemid, bool boolPLocalUpdate)
{
  emit itemsUpdated(intPItemid, boolPLocalUpdate);
}

void GUIClient::sItemsitesUpdated()
{
  emit itemsitesUpdated();
}

void GUIClient::sWarehousesUpdated()
{
  emit warehousesUpdated();
}

void GUIClient::sCustomersUpdated(int pCustid, bool pLocal)
{
  emit customersUpdated(pCustid, pLocal);
}

void GUIClient::sGlSeriesUpdated()
{
  emit glSeriesUpdated();
}

void GUIClient::sVendorsUpdated()
{
  emit vendorsUpdated();
}

void GUIClient::sProspectsUpdated()
{
  emit prospectsUpdated();
}

void GUIClient::sReturnAuthorizationsUpdated()
{
  emit returnAuthorizationsUpdated();
}

void GUIClient::sStandardPeriodsUpdated()
{
  emit standardPeriodsUpdated();
}

void GUIClient::sSalesOrdersUpdated(int pSoheadid)
{
  emit salesOrdersUpdated(pSoheadid, TRUE);
}

void GUIClient::sCreditMemosUpdated()
{
  emit creditMemosUpdated();
}

void GUIClient::sQuotesUpdated(int pQuheadid)
{
  emit quotesUpdated(pQuheadid, TRUE);
}

void GUIClient::sWorkOrderMaterialsUpdated(int pWoid, int pWomatlid, bool pLocalUpdate)
{
  emit workOrderMaterialsUpdated(pWoid, pWomatlid, pLocalUpdate);
}

void GUIClient::sWorkOrderOperationsUpdated(int pWoid, int pWooperid, bool pLocalUpdate)
{
  emit workOrderOperationsUpdated(pWoid, pWooperid, pLocalUpdate);
}

void GUIClient::sWorkOrdersUpdated(int pWoid, bool pLocalUpdate)
{
  emit workOrdersUpdated(pWoid, pLocalUpdate);
}

void GUIClient::sPurchaseOrdersUpdated(int pPoheadid, bool pLocalUpdate)
{
  emit purchaseOrdersUpdated(pPoheadid, pLocalUpdate);
}

void GUIClient::sPurchaseOrderReceiptsUpdated()
{
  emit purchaseOrderReceiptsUpdated();
}

void GUIClient::sPurchaseRequestsUpdated()
{
  emit purchaseRequestsUpdated();
}

void GUIClient::sVouchersUpdated()
{
  emit vouchersUpdated();
}

void GUIClient::sBOMsUpdated(int intPItemid, bool boolPLocalUpdate)
{
  emit bomsUpdated(intPItemid, boolPLocalUpdate);
}

void GUIClient::sBBOMsUpdated(int intPItemid, bool boolPLocalUpdate)
{
  emit bbomsUpdated(intPItemid, boolPLocalUpdate);
}

void GUIClient::sBOOsUpdated(int intPItemid, bool boolPLocalUpdate)
{
  emit boosUpdated(intPItemid, boolPLocalUpdate);
}

void GUIClient::sBudgetsUpdated(int intPItemid, bool boolPLocalUpdate)
{
  emit budgetsUpdated(intPItemid, boolPLocalUpdate);
}

void GUIClient::sAssortmentsUpdated(int pItemid, bool pLocalUpdate)
{
  emit assortmentsUpdated(pItemid, pLocalUpdate);
}

void GUIClient::sWorkCentersUpdated()
{
  emit workCentersUpdated();
}

void GUIClient::sBillingSelectionUpdated(int pCoheadid, int pCoitemid)
{
  emit billingSelectionUpdated(pCoheadid, pCoitemid);
}

void GUIClient::sInvoicesUpdated(int pInvcheadid, bool pLocal)
{
  emit invoicesUpdated(pInvcheadid, pLocal);
}

void GUIClient::sItemGroupsUpdated(int pItemgrpid, bool pLocal)
{
  emit itemGroupsUpdated(pItemgrpid, pLocal);
}

void GUIClient::sCashReceiptsUpdated(int pCashrcptid, bool pLocal)
{
  emit cashReceiptsUpdated(pCashrcptid, pLocal);
}

void GUIClient::sBankAdjustmentsUpdated(int pBankadjid, bool pLocal)
{
  emit bankAdjustmentsUpdated(pBankadjid, pLocal);
}

void GUIClient::sQOHChanged(int pItemsiteid, bool pLocal)
{
  emit qohChanged(pItemsiteid, pLocal);
}

void GUIClient::sReportsChanged(int pReportid, bool pLocal)
{
  emit reportsChanged(pReportid, pLocal);
}

void GUIClient::sChecksUpdated(int pBankaccntid, int pCheckid, bool pLocal)
{
  emit checksUpdated(pBankaccntid, pCheckid, pLocal);
  emit paymentsUpdated(pBankaccntid, -1, pLocal);
}

void GUIClient::sPaymentsUpdated(int pBankaccntid, int pApselectid, bool pLocal)
{
  emit paymentsUpdated(pBankaccntid, pApselectid, pLocal);
}

void GUIClient::sConfigureGLUpdated()
{
  emit configureGLUpdated();
}

void GUIClient::sProjectsUpdated(int prjid)
{
  emit projectsUpdated(prjid);
}

void GUIClient::sCrmAccountsUpdated(int crmacctid)
{
  emit crmAccountsUpdated(crmacctid);
}

void GUIClient::sTaxAuthsUpdated(int taxauthid)
{
  emit taxAuthsUpdated(taxauthid);
}

void GUIClient::sTransferOrdersUpdated(int id)
{
  emit transferOrdersUpdated(id);
}

void GUIClient::sIdleTimeout()
{
 // so we don't accidentally get called again waiting
  _timeoutHandler->reset();

  ParameterList params;
  params.append("minutes", _timeoutHandler->idleMinutes());
  
  idleShutdown newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() == XDialog::Accepted)
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
    omfgThis->statusBar()->showMessage(pMessage);
  else
    omfgThis->statusBar()->showMessage(pMessage, pTimeout);

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


void GUIClient::populateCustomMenu(QMenu * menu, const QString & module)
{
  QMenu *customMenu = 0;
  XSqlQuery qry;
  qry.prepare("SELECT cmd_id, cmd_title, cmd_privname,"
              "       COALESCE(cmd_name, cmd_title) AS cmd_name"
              "  FROM cmd"
              " WHERE (cmd_module=:module)"
              " ORDER BY cmd_title, cmd_id; ");
  qry.bindValue(":module", module);
  qry.exec();
  while(qry.next())
  {
    if(customMenu == 0)
    {
      customMenu = menu->addMenu(tr("Custom"));
      customMenu->setObjectName("menu." + module.toLower() + ".custom");
    }

    QString allowed = "true";
    QString privname = qry.value("cmd_privname").toString();
    if(!privname.isEmpty())
      allowed = "Custom"+privname;

    Action * action = new Action( this, QString("custom.")+qry.value("cmd_name").toString(), qry.value("cmd_title").toString(),
      this, SLOT(sCustomCommand()), customMenu, allowed);

    _customCommands.insert(action, qry.value("cmd_id").toInt());
    //actions.append(action);
  }
}

void GUIClient::sCustomCommand()
{
  const QObject * obj = sender();
  QMap<const QObject*,int>::const_iterator it;
  it = _customCommands.find(obj);
  if(it != _customCommands.end())
  {
    q.prepare("SELECT cmd_executable"
              "  FROM cmd"
              " WHERE(cmd_id=:cmd_id);");
    q.bindValue(":cmd_id", it.value());
    q.exec();
    q.first();
    QString cmd = q.value("cmd_executable").toString();
    if(cmd.toLower() == "!customuiform")
    {
      bool haveParams = false;
      ParameterList params;
      bool asDialog = false;
      QString asName;
      q.prepare("SELECT cmdarg_arg AS argument"
                "  FROM cmdarg"
                " WHERE (cmdarg_cmd_id=:cmd_id)"
                " ORDER BY cmdarg_order; ");
      q.bindValue(":cmd_id", it.value());
      q.exec();
      while(q.next())
      {
        cmd = q.value("argument").toString();
        if(cmd.startsWith("uiformtype=", Qt::CaseInsensitive))
          asDialog = (cmd.right(cmd.length() - 11).toLower() == "dialog");
        else if(cmd.startsWith("uiform=", Qt::CaseInsensitive))
          asName = cmd.right(cmd.length() - 7);
        else if (cmd.startsWith("-param=", Qt::CaseInsensitive))
        {
// Taken from OpenRPT/renderapp with slight modifications
          QString str = cmd.right(cmd.length() - 7);
          bool active = true;
          QString name;
          QString type;
          QString value;
          QVariant var;
          int sep = str.indexOf('=');
          if(sep == -1)
            name = str;
          else
          {
            name = str.left(sep);
            value = str.right(str.length() - (sep + 1));
          }
          str = name;
          sep = str.indexOf(':');
          if(sep != -1)
          {
            name = str.left(sep);
            type = str.right(str.length() - (sep + 1));
          }
          if(name.startsWith("-"))
          {
            name = name.right(name.length() - 1);
            active = false;
          }
          else if(name.startsWith("+"))
            name = name.right(name.length() - 1);
          if(!value.isEmpty())
            var = XVariant::decode(type, value);
          if(active)
          {
            haveParams = true;
            params.append(name, var);
          }
// end copied code from OpenRPT/renderapp
        }
      }
      if(asName.isEmpty())
        return;
      q.prepare("SELECT *"
                "  FROM uiform"
                " WHERE((uiform_name=:uiform_name)"
                "   AND (uiform_enabled))"
                " ORDER BY uiform_order DESC"
                " LIMIT 1;");
      q.bindValue(":uiform_name", asName);
      q.exec();
      if(!q.first())
      {
        QMessageBox::critical(this, tr("Could Not Create Form"),
          tr("Could not create the required form. Either an error occurred or the specified form does not exist.") );
        return;
      }

      XUiLoader loader;
      QByteArray ba = q.value("uiform_source").toByteArray();
      QBuffer uiFile(&ba);
      if(!uiFile.open(QIODevice::ReadOnly))
      {
        QMessageBox::critical(this, tr("Could not load file"),
            tr("There was an error loading the UI Form from the database."));
        return;
      }
      QWidget *ui = loader.load(&uiFile);
      uiFile.close();

      if(asDialog)
      {
        XDialog dlg(this);
        dlg.setObjectName(q.value("uiform_name").toString());
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(ui);
        dlg.setLayout(layout);
        dlg.setWindowTitle(ui->windowTitle());
        dlg.exec();
      }
      else
      {
        XMainWindow * wnd = new XMainWindow();
        wnd->setObjectName(q.value("uiform_name").toString());
        wnd->setCentralWidget(ui);
        wnd->setWindowTitle(ui->windowTitle());
        handleNewWindow(wnd);
      }
    }
    else if(cmd.toLower().startsWith("!openurl"))
    {
      //allow a file or Url to be opened from a custom command directly
      // backwards compatibility with original patch - deprecated usage
      QString urltext = cmd.mid(8, cmd.length()).trimmed();
      if (! urltext.isEmpty())
      {
        qWarning("Deprecated usage of !openurl. Use cmdarg.");
        //If url scheme is missing, we'll assume it is "file" for now.
        QUrl url(urltext);
        if (url.scheme().isEmpty())
          url.setScheme("file");
        QDesktopServices::openUrl(url);
      }
      // end of deprecated usage
      q.prepare("SELECT cmdarg_arg "
                "FROM cmdarg "
                "WHERE (cmdarg_cmd_id=:cmd_id) "
                "ORDER BY cmdarg_order;");
      q.bindValue(":cmd_id", it.value());
      q.exec();
      while (q.next())
      {
        QUrl url(q.value("cmdarg_arg").toString());
        if (url.scheme().isEmpty())
          url.setScheme("file");
        QDesktopServices::openUrl(url);
      }
    }
    else
    {
      QProcess *proc = new QProcess(this);
      QString command;
      QStringList args;
      connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), proc, SLOT(deleteLater()));
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
      q.bindValue(":cmd_id", it.value());
      q.exec();
      if (q.first())
      {
        command = q.value("argument").toString();
        while(q.next())
          args.append(q.value("argument").toString());
        proc->start(command, args);
      }
    }
  }
}

void GUIClient::launchBrowser(QWidget * w, const QString & url)
{
#if defined(Q_OS_WIN32)
  // Windows - let the OS do the work
  QT_WA( {
      ShellExecute(w->winId(), 0, (TCHAR*)url.utf16(), 0, 0, SW_SHOWNORMAL );
    } , {
      ShellExecuteA( w->winId(), 0, url.toLocal8Bit(), 0, 0, SW_SHOWNORMAL );
    } );
#else
  const char *b = getenv("BROWSER");
  QStringList browser;
  if(b) {
    QString t(b);
    browser = t.split(':', QString::SkipEmptyParts);
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
    QProcess *proc = new QProcess(w);
    connect(proc, SIGNAL(processExited()), proc, SLOT(deleteLater()));
    QStringList args = app.split(QRegExp(" +"));
    QString cmd = args.first();
    args.removeFirst();
    proc->start(cmd, args);
    if (proc->waitForStarted())
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

QWidgetList GUIClient::windowList()
{
  if(_showTopLevel)
    return _windowList;
  else
    return _workspace->windowList();
}

void GUIClient::windowDestroyed(QObject * o)
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
      xtsettingsSetValue(objName + "/geometry/size", w->size());
      if(omfgThis->showTopLevel())
        xtsettingsSetValue(objName + "/geometry/pos", w->pos());
      else
        xtsettingsSetValue(objName + "/geometry/pos", w->parentWidget()->pos());
    }
  }
  return QObject::eventFilter(obj, event);
}

void GUIClient::handleNewWindow(QWidget * w, Qt::WindowModality m)
{
  // TO DO:  This function should be replaced by a centralized openWindow function
  // used by toolbox, guiclient interface, and core windows
  if(!w->isModal())
  {
    if (w->parentWidget())
    {
      if (w->parentWidget()->window()->isModal())
        w->setWindowModality(Qt::ApplicationModal);
      else
      {
        w->setWindowModality(m);
        // Remove the parent because this is not a behavior we want unless
        // window is modal.  Does, however, completely eliminate ability
        // to set a parent on non-modal window with this implementation
        w->setParent(0);
      }
    }
    else
      w->setWindowModality(m);
  }

  connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(windowDestroyed(QObject*)));

  if(w->inherits("XMainWindow") || w->inherits("XWidget") || w->inherits("XDialog"))
  {
    if(w->inherits("XDialog"))
      qDebug() << "warning: " << w->objectName() << " inherts XDialog and was passed to handleNewWindow()";
    w->show();
    return;
  }

  qDebug() << "GUIClient::handleNewWindow() called on object that doesn't inherit XMainWindow: " << w->objectName();

  QRect availableGeometry = QApplication::desktop()->availableGeometry();
  if(!_showTopLevel && !w->isModal())
    availableGeometry = _workspace->geometry();

  QString objName = w->objectName();
  QPoint pos = xtsettingsValue(objName + "/geometry/pos").toPoint();
  QSize size = xtsettingsValue(objName + "/geometry/size").toSize();

  if(size.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool())
    w->resize(size);

  bool wIsModal = w->isModal();
  if(_showTopLevel || wIsModal)
  {
    _windowList.append(w);
    w->setAttribute(Qt::WA_DeleteOnClose);
    QMainWindow *mw = qobject_cast<QMainWindow*>(w);
    if (mw)
      mw->statusBar()->show();
    QRect r(pos, w->size());
    if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool())
      w->move(pos);
    w->show();
  }
  else
  {
    QWidget * fw = w->focusWidget();
    w->setAttribute(Qt::WA_DeleteOnClose);
    _workspace->addWindow(w);
    QRect r(pos, w->size());
    if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool())
      w->move(pos);
    w->show();
    if(fw)
      fw->setFocus();
  }

  if(!wIsModal)
    w->installEventFilter(__saveSizePositionEventFilter);
}

QMenuBar *GUIClient::menuBar()
{
#ifdef Q_WS_MACX
  if (_menuBar == 0)
    _menuBar = new QMenuBar();

  return _menuBar;
#else
  return QMainWindow::menuBar();
#endif
}

void GUIClient::sFocusChanged(QWidget * /*old*/, QWidget * /*now*/)
{
  QWidget * thisActive = workspace()->activeWindow();
  if(omfgThis->showTopLevel())
    thisActive = qApp->activeWindow();
  if(thisActive == this)
    return;
  _activeWindow = thisActive;
}

QWidget * GUIClient::myActiveWindow()
{
  return _activeWindow;
}

// TODO: when std edition is extracted, replace this with a script include()
QScriptValue distributeInventorySeriesAdjust(QScriptContext *context,
                                             QScriptEngine  *engine)
{
  int result = -1;

  if (context->argumentCount() == 1 &&
      context->argument(0).isNumber())
    result = distributeInventory::SeriesAdjust(context->argument(0).toInt32(),
                                               0);

  else if (context->argumentCount() == 2 &&
           context->argument(0).isNumber() &&
           qscriptvalue_cast<QWidget*>(context->argument(1)))
    result = distributeInventory::SeriesAdjust(context->argument(0).toInt32(),
           qscriptvalue_cast<QWidget*>(context->argument(1)));

  else if (context->argumentCount() == 3 &&
           context->argument(0).isNumber() &&
           qscriptvalue_cast<QWidget*>(context->argument(1)))
    result = distributeInventory::SeriesAdjust(context->argument(0).toInt32(),
                             qscriptvalue_cast<QWidget*>(context->argument(1)),
                             context->argument(2).toString());

  else if (context->argumentCount() == 4 &&
           context->argument(0).isNumber() &&
           qscriptvalue_cast<QWidget*>(context->argument(1)) &&
           context->argument(3).isDate())
    result = distributeInventory::SeriesAdjust(context->argument(0).toInt32(),
                             qscriptvalue_cast<QWidget*>(context->argument(1)),
                             context->argument(2).toString(),
                             context->argument(3).toDateTime().date());

  else if (context->argumentCount() == 5 &&
           context->argument(0).isNumber() &&
           qscriptvalue_cast<QWidget*>(context->argument(1)) &&
           context->argument(3).isDate() &&
           context->argument(4).isDate())
    result = distributeInventory::SeriesAdjust(context->argument(0).toInt32(),
                             qscriptvalue_cast<QWidget*>(context->argument(1)),
                             context->argument(2).toString(),
                             context->argument(3).toDateTime().date(),
                             context->argument(4).toDateTime().date());

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate SeriesAdjust method");

  return engine->toScriptValue(result);
}

void GUIClient::loadScriptGlobals(QScriptEngine * engine)
{
  if(!engine)
    return;

#if QT_VERSION >= 0x040500
  engine->installTranslatorFunctions();
#endif

  qScriptRegisterMetaType(engine, SetResponsetoScriptValue, SetResponsefromScriptValue);
  qScriptRegisterMetaType(engine, ParameterGroupTypestoScriptValue, ParameterGroupTypesfromScriptValue);
  qScriptRegisterMetaType(engine, ParameterGroupStatestoScriptValue, ParameterGroupStatesfromScriptValue);
  qScriptRegisterMetaType(engine, QtWindowModalitytoScriptValue, QtWindowModalityfromScriptValue);
  qScriptRegisterMetaType(engine, SaveFlagstoScriptValue, SaveFlagsfromScriptValue);

  ScriptToolbox * tb = new ScriptToolbox(engine);
  QScriptValue toolbox = engine->newQObject(tb);
  engine->globalObject().setProperty("toolbox", toolbox);

  QScriptValue mainwindowval = engine->newQObject(this);
  engine->globalObject().setProperty("mainwindow", mainwindowval);
  mainwindowval.setProperty("NoError",  QScriptValue(engine, NoError),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("NoError_Cancel",  QScriptValue(engine, NoError_Cancel),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("NoError_Run",  QScriptValue(engine, NoError_Run),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("NoError_Print",  QScriptValue(engine, NoError_Print),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("NoError_Submit",  QScriptValue(engine, NoError_Submit),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("Error_NoSetup",  QScriptValue(engine, Error_NoSetup),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("UndefinedError",  QScriptValue(engine, UndefinedError),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);


  QScriptValue metricsval = engine->newQObject(_metrics);
  engine->globalObject().setProperty("metrics", metricsval);

  QScriptValue metricsencval = engine->newQObject(_metricsenc);
  engine->globalObject().setProperty("metricsenc", metricsencval);

  QScriptValue preferencesval = engine->newQObject(_preferences);
  engine->globalObject().setProperty("preferences", preferencesval);

  QScriptValue privilegesval = engine->newQObject(_privileges);
  engine->globalObject().setProperty("privileges", privilegesval);

  QScriptValue settingsval = engine->newFunction(settingsValue, 2);
  engine->globalObject().setProperty("settingsValue", settingsval);

  QScriptValue settingssetval = engine->newFunction(settingsSetValue, 2);
  engine->globalObject().setProperty("settingsSetValue", settingssetval);

  engine->globalObject().setProperty("startOfTime", engine->newDate(QDateTime(_startOfTime)));
  engine->globalObject().setProperty("endOfTime", engine->newDate(QDateTime(_endOfTime)));

  // TODO: when std edition is extracted, replace this with a script include()
  QScriptValue distribInvObj = engine->newFunction(distributeInventorySeriesAdjust);
  distribInvObj.setProperty("SeriesAdjust", distribInvObj,
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  engine->globalObject().setProperty("DistributeInventory", distribInvObj);

  mainwindowval.setProperty("UndefinedError",  QScriptValue(engine, UndefinedError),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // #defines from guiclient.h
  mainwindowval.setProperty("cNew", QScriptValue(engine, cNew),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cEdit", QScriptValue(engine, cEdit),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cView", QScriptValue(engine, cView),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cCopy", QScriptValue(engine, cCopy),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cRelease", QScriptValue(engine, cRelease),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cPost", QScriptValue(engine, cPost),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cReplace", QScriptValue(engine, cReplace),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cPostedCounts", QScriptValue(engine, cPostedCounts),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cUnpostedCounts", QScriptValue(engine, cUnpostedCounts),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cAllCounts", QScriptValue(engine, cAllCounts),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cLocation", QScriptValue(engine, cLocation),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cItemloc", QScriptValue(engine, cItemloc),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cTransAll", QScriptValue(engine, cTransAll),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cTransReceipts", QScriptValue(engine, cTransReceipts),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cTransIssues", QScriptValue(engine, cTransIssues),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cTransShipments", QScriptValue(engine, cTransShipments),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cTransAdjCounts", QScriptValue(engine, cTransAdjCounts),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cTransTransfers", QScriptValue(engine, cTransTransfers),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cTransScraps", QScriptValue(engine, cTransScraps),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("SalesJournal", QScriptValue(engine, SalesJournal),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("CreditMemoJournal", QScriptValue(engine, CreditMemoJournal),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("CashReceiptsJournal", QScriptValue(engine, CashReceiptsJournal),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("PayablesJournal", QScriptValue(engine, PayablesJournal),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("CheckJournal", QScriptValue(engine, CheckJournal),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("cNoReportDefinition", QScriptValue(engine, cNoReportDefinition),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);

  QScriptValue inputmanagerval = engine->newQObject(_inputManager);
  engine->globalObject().setProperty("InputManager", inputmanagerval);

  // #defines from inputmanager.h
  inputmanagerval.setProperty("cBCWorkOrder", QScriptValue(engine, cBCWorkOrder),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCWorkOrderMaterial", QScriptValue(engine, cBCWorkOrderMaterial),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCWorkOrderOperation", QScriptValue(engine, cBCWorkOrderOperation),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCSalesOrder", QScriptValue(engine, cBCSalesOrder),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCSalesOrderLineItem", QScriptValue(engine, cBCSalesOrderLineItem),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCItemSite", QScriptValue(engine, cBCItemSite),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCItem", QScriptValue(engine, cBCItem),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCUPCCode", QScriptValue(engine, cBCUPCCode),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCEANCode", QScriptValue(engine, cBCEANCode),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCCountTag", QScriptValue(engine, cBCCountTag),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCLocation", QScriptValue(engine, cBCLocation),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCLocationIssue", QScriptValue(engine, cBCLocationIssue),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCLocationContents", QScriptValue(engine, cBCLocationContents),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCUser", QScriptValue(engine, cBCUser),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCTransferOrder", QScriptValue(engine, cBCTransferOrder),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCTransferOrderLineItem", QScriptValue(engine, cBCTransferOrderLineItem),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  inputmanagerval.setProperty("cBCLotSerialNumber", QScriptValue(engine, cBCLotSerialNumber),
                        QScriptValue::ReadOnly | QScriptValue::Undeletable);

  setupScriptApi(engine);
}
