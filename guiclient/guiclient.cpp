/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QVBoxLayout>
#include <QStatusBar>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QDateTime>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QImage>
#include <QSplashScreen>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QPixmap>
#include <QTextStream>
#include <QCloseEvent>
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
#include "errorReporter.h"
#include "login2.h"
#include "storedProcErrorLookup.h"

#include "systemMessage.h"
#include "menuProducts.h"
#include "menuInventory.h"
#include "menuSchedule.h"
#include "menuManufacture.h"
#include "menuPurchase.h"
#include "menuCRM.h"
#include "menuSales.h"
#include "menuAccounting.h"
#include "menuWindow.h"
#include "menuSystem.h"

#include "timeoutHandler.h"
#include "idleShutdown.h"
#include "inputManager.h"
#include "xdoublevalidator.h"

#include "distributeInventory.h"
#include "documents.h"
#include "splashconst.h"
#include "scripttoolbox.h"
#include "menubutton.h"

#include "setup.h"
#include "setupscriptapi.h"

#if defined(Q_OS_WIN)
#define NOCRYPT
#include <windows.h>
#else
#if defined(Q_OS_MAC)
#include <stdlib.h>
#endif
#endif

class Metrics;
class Preferences;
class Privileges;
class Metricsenc;

#ifdef Q_OS_MAC
  extern void qt_mac_set_dock_menu(QMenu *menu);
#endif
/** @addtogroup globals Global Variables and Functions

    @brief

    The xTuple ERP desktop client has a main single main application window,
    an instance of the GUIClient class. This is exposed to the rest of the
    application in the global @c omfgThis variable.
    There is also a handful of global variables.
    @{
    */

/** @brief The splash screen shown at application startup.

    This shows the application edition and progress as the application reads
    basic information from the database.

    @todo Make this static, hence internal to guiclient.cpp ?
  */
QSplashScreen *_splash;

/** @brief A cache of the @c metrics table. */
Metrics       *_metrics=0;

/** @brief A cache of user preferences.  */
Preferences   *_preferences=0;

/** @brief A cache of user privileges.

    The cache is refreshed when the user starts the application or
    selects @b "Rescan Privileges" from the @b System menu.
  */
Privileges    *_privileges=0;

/** @brief A cache of the @c metricsenc table for encrypted metrics. */
Metricsenc    *_metricsenc=0;

/** @brief A list of the hot keys currently mapped for use.

    Values beginning with @c C are mapped to control keys while those
    that start with @c F are function keys.

    @todo Make this static or a private member of GUIClient?
  */
QList<QString> _hotkeyList;

/** @brief A global variable indicating whether this is an evaluation copy
           of the application.
    @todo Determine if this is still useful and possibly remove.
  */
bool _evaluation;

#include <SaveSizePositionEventFilter.h>
static SaveSizePositionEventFilter * __saveSizePositionEventFilter = 0;

static int __interval = 0;
static int __intervalCount = 0;

/** @brief Check if the current user has privileges to use the given Action.
    @sa    Action
  */
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
    act->setEnabled(_privileges->check(privs));
  }
}

static QScriptValue settingsValue(QScriptContext *context, QScriptEngine *engine)
{
  Q_UNUSED(engine);
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

/** @} */

/** @class Action

    @brief A convenience subclass of QAction for simplifying the association of menu items with icons, toolbar buttons, and hotkeys.

    Core application menu items are built using the Action class.
    The Action constructors - actually Action::init - attach QString values
    describing the user's permissions as QAction::data. These strings
    are interpreted in the following fashion:

    @arg @c true           - the user can use this QAction
    @arg @c false          - the user can not use this QAction
    @arg @c Priv           - the user must have @c SimplePriv
    @arg @c \#superuser    - the user must be a DBA
    @arg @c PrivA @c PrivB - the user can have either @c PrivA or @c PrivB
    @arg @c PrivA+PrivB    - the user must have both @c PrivA and @c PrivB

    @sa Privileges::isDba()

    @todo Add support for @c \@name:mode - call @c name::userHasPriv(mode)

 */
Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, bool pEnabled ) :
 QAction(pDisplayName, pParent)
{
  init(pParent, pName, pDisplayName, pTarget, pActivateSlot, pAddTo, (pEnabled?"true":"false"));
}

/** @overload @param pEnabled */
Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, const QString & pEnabled ) :
 QAction(pDisplayName, pParent)
{
  init(pParent, pName, pDisplayName, pTarget, pActivateSlot, pAddTo, pEnabled);
}

/** @overload @param pEnabled */
Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, const QString & pEnabled,
                const QPixmap *pIcon, QWidget *pToolBar ) :
 QAction(pDisplayName, pParent)
{
  init(pParent, pName, pDisplayName, pTarget, pActivateSlot, pAddTo, pEnabled);

  setIcon(QIcon(*pIcon));
  pToolBar->addAction(this);
}

/** @overload @param pEnabled */
Action::Action( QWidget *pParent, const char *pName, const QString &pDisplayName,
                QObject *pTarget, const char *pActivateSlot,
                QWidget *pAddTo, const QString & pEnabled,
                const QPixmap *pIcon, QWidget *pToolBar,
                const QString &pToolTip ) :
 QAction(pDisplayName, pParent)
{
  init(pParent, pName, pDisplayName, pTarget, pActivateSlot, pAddTo, pEnabled);

  setIcon(QIcon(*pIcon));
  pToolBar->addAction(this);
  setToolTip(pToolTip);
}

void Action::init(QWidget *pParent, const char *pName, const QString &pDisplayName,
                  QObject *pTarget, const char *pActivateSlot,
                  QWidget *pAddTo,  const QString & pEnabled)
{
  Q_UNUSED(pParent);
  Q_UNUSED(pDisplayName);
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

  connect(this, SIGNAL(triggered()), pTarget, pActivateSlot);
  //setEnabled(pEnabled);
  pAddTo->addAction(this);

  if(!pEnabled.isEmpty())
    setData(pEnabled);
  __menuEvaluate(this);
}

/** @class xTupleGuiClientInterface
    @brief A concrete implementation of the GuiClientInterface allowing widgets
           to request services from the main application.

    The primary use of this class/interface is for individual widgets to open
    application-level windows.
  */
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

  QAction* findAction(const QString pname)
  {
   return omfgThis->findChild<QAction*>(pname);
  }

  void addDocumentWatch(QString path, int id)
  {
    omfgThis->addDocumentWatch(path, id);
  }

  void removeDocumentWatch(QString path)
  {
    omfgThis->removeDocumentWatch(path);
  }

  bool hunspell_ready()
  {
      return omfgThis->hunspell_ready();
  }

  int hunspell_check(const QString word)
  {
      return omfgThis->hunspell_check(word);
  }

  const QStringList hunspell_suggest(const QString word)
  {
      return omfgThis->hunspell_suggest(word);
  }

  int hunspell_add(const QString word)
  {
      return omfgThis->hunspell_add(word);
  }

  int hunspell_ignore(const QString word)
  {
      return omfgThis->hunspell_ignore(word);
  }
};

/** @class GUIClient

    @brief The GUIClient is the main xTuple ERP desktop client window.

    This should be a singleton object. It provides many utility functions and
    coordinates a number of
    global features and behaviors. For example, the GUIClient::inputManager()
    coordinates barcode scanning for the entire application instance.
  */

/** @brief This is a global object that can be used to get access to the
           utility methods and shared functionality.
  */
GUIClient *omfgThis;

/** @brief Create a new xTuple ERP main application window, set up menus,
           and otherwise initialize the application.

    Do not call this more than once or bad things will happen.
  */
GUIClient::GUIClient(const QString &pDatabaseURL, const QString &pUsername)
{
  XSqlQuery _GGUIClient;
  _menuBar = 0;
  _activeWindow = 0;
  _shown = false;
  _shuttingDown = false;

  _databaseURL = pDatabaseURL;
  _username = pUsername;
  __saveSizePositionEventFilter = new SaveSizePositionEventFilter(this);

  _splash->showMessage(tr("Initializing Internal Data"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();

  _showTopLevel = true;
  if(_preferences->value("InterfaceWindowOption") == "Workspace")
    _showTopLevel = false;

  _GGUIClient.exec("SELECT startOfTime() AS sot, endOfTime() AS eot;");
  if (_GGUIClient.first())
  {
    _startOfTime = _GGUIClient.value("sot").toDate();
    _endOfTime = _GGUIClient.value("eot").toDate();
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
  _orderVal       = new QIntValidator(0, 9999999, this);
  _dayVal         = new QIntValidator(0, 9999, this);

  _fixedFont = new QFont("courier", 8);
  _systemFont = new QFont(qApp->font());

  _workspace = new QMdiArea();

  _workspace->setWindowIcon(QIcon(":/images/clear.png"));
  _workspace->setActivationOrder(QMdiArea::ActivationHistoryOrder);

  setCentralWidget(_workspace);
  _workspace->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  _workspace->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  _workspace->setContentsMargins(0, 0, 0, 0);

//  Install the InputManager
  _inputManager = new InputManager();
  qApp->installEventFilter(_inputManager);

  setWindowTitle();

  // load plugins before building the menus
  // TODO? add a step later to add to the menus from the plugins?
  QStringList checkForPlugins;
  checkForPlugins << QApplication::applicationDirPath()
                  << QString("/usr/lib/postbooks");
  foreach (QString dirname, checkForPlugins)
  {
    QDir pluginsDir(dirname);
    while (! pluginsDir.exists("plugins") && pluginsDir.cdUp())
      ;
    if (pluginsDir.cd("plugins"))
    {
      foreach (QString fileName, pluginsDir.entryList(QDir::Files))
        new QPluginLoader(pluginsDir.absoluteFilePath(fileName), this);
    }
  }

//  Populate the menu bar
  XSqlQuery window;
  window.prepare("SELECT usr_window "
                 "  FROM usr "
                 " WHERE (usr_username=getEffectiveXtUser());");
  window.exec();
  // keep synchronized with user.ui.h
  _singleWindow = "";
  if (window.first())
    _singleWindow = window.value("usr_window").toString();
  if (_singleWindow.isEmpty())
    initMenuBar();
  else
    _showTopLevel = true; // if we are in single level mode we want to run toplevel always

  _splash->showMessage(tr("Loading the Background Image"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();

  if (_preferences->value("BackgroundImageid").toInt() > 0)
  {
    _GGUIClient.prepare( "SELECT image_data "
                "FROM image "
                "WHERE (image_id=:image_id);" );
    _GGUIClient.bindValue(":image_id", _preferences->value("BackgroundImageid").toInt());
    _GGUIClient.exec();
    if (_GGUIClient.first())
    {
      QImage background;

      background.loadFromData(QUUDecode(_GGUIClient.value("image_data").toString()));
      _workspace->setBackground(QBrush(QPixmap::fromImage(background)));
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

  connect(_privileges, SIGNAL(loaded()), this, SLOT(initMenuBar()));

  VirtualClusterLineEdit::_guiClientInterface = new xTupleGuiClientInterface();
  Documents::_guiClientInterface = VirtualClusterLineEdit::_guiClientInterface;
  MenuButton::_guiClientInterface =  VirtualClusterLineEdit::_guiClientInterface;
  XTreeWidget::_guiClientInterface = VirtualClusterLineEdit::_guiClientInterface;
  XComboBox::_guiClientInterface = VirtualClusterLineEdit::_guiClientInterface;
  XTextEdit::_guiClientInterface = VirtualClusterLineEdit::_guiClientInterface;
  XTextEditHighlighter::_guiClientInterface = VirtualClusterLineEdit::_guiClientInterface;

  _splash->showMessage(tr("Completing Initialization"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();
  _splash->finish(this);

  connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(sFocusChanged(QWidget*,QWidget*))); // Need this?

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
  //put the main window back on screen if it is off screen in part or full
  if(!pos.isNull())
  {
    if (pos.x() < availableGeometry.left())
      pos.setX(availableGeometry.left());
    else if (pos.x() + size.width() > mainXMax)
      pos.setX(qMax(availableGeometry.right() - size.width(),
                    availableGeometry.left()));

    if (pos.y() < availableGeometry.top())
      pos.setY(availableGeometry.top());
    else if (pos.y() + size.height() > mainYMax)
      pos.setY(qMax(availableGeometry.height() - size.height(),
                    availableGeometry.top()));
    move(pos);
  }
  #ifdef Q_OS_MAC
    _menu = new QMenu(this);
    updateMacDockMenu(this);
  #endif

  setDocumentMode(true);

  // Set up document file watcher
  _fileWatcher = new QFileSystemWatcher();
  connect(_fileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(handleDocument(QString)));

  hunspell_initialize();

}

GUIClient::~GUIClient()
{
  QApplication::closeAllWindows();

  errorLogListener::destroy();
  //omfgThis = 0;

  // Close the database connection
  XSqlQuery qlc("SELECT logout();");
  QSqlDatabase::database().close();
}

/** @brief Return an enumerated value indicating the windowing system for the
           current run-time environment.
 */
GUIClient::WindowSystem GUIClient::getWindowSystem()
{
#ifdef Q_OS_LINUX
  return X11;
#elif defined Q_OS_WIN
  return WIN;
#elif defined Q_OS_MAC
  return MAC;
#elif defined Q_WS_QWS
  return QWS;
#elif defined Q_WS_WINCE
  return WINCE;
#elif defined Q_WS_S60
  return S60;
#else
  return Unknown;
#endif
}

/** @brief Return whether the database has only one currency or if
           more than one currency is defined.
  */
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

/** @brief Build and set the title for the main application window.

    This title helps distinguish between copies of the application using
    different databases, database server hosts, users, and xTuple ERP
    versions.
  */
void GUIClient::setWindowTitle()
{
  XSqlQuery _GetWindowTitle;
  QString name;

  _splash->showMessage(tr("Loading Database Information"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();

  _GetWindowTitle.exec( "SELECT metric_value, getEffectiveXtUser() AS username "
           "FROM metric "
           "WHERE (metric_name='DatabaseName')" );
  if (_GetWindowTitle.first())
  {
    if (_GetWindowTitle.value("metric_value").toString().isEmpty())
      name = tr("Unnamed Database");
    else
      name = _GetWindowTitle.value("metric_value").toString();

    QString server;
    QString protocol;
    QString database;
    QString port;
    parseDatabaseURL(_databaseURL, protocol, server, database, port);

    if (_evaluation)
      QMainWindow::setWindowTitle( tr("%1 Evaluation %2 - Logged on as %3")
                               .arg(_Name)
                               .arg(_Version)
                               .arg(_GetWindowTitle.value("username").toString()) );
    else
      QMainWindow::setWindowTitle( tr("%1 %2 - %3 on %4:%5/%6 AS %7")
                               .arg(_Name)
                               .arg(_Version)
                               .arg(name)
                               .arg(server)
                               .arg(port)
                               .arg(database)
                               .arg(_GetWindowTitle.value("username").toString()) );
  }
  else
    QMainWindow::setWindowTitle(_Name);
}

/** @deprecated */
void GUIClient::setCaption()
{
}

/** @brief Build the application menus and toolbars based on
           the current user's preferences for menu and toolbar visibility.
 */
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

    QList<QToolBar *> toolbars = this->findChildren<QToolBar *>();
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

    windowMenu = new menuWindow(this);

    _splash->showMessage(tr("Initializing the System Module"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
    systemMenu = new menuSystem(this);

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

  // Restore toolbar positions from local machine
  restoreState(xtsettingsValue("MainWindowState", QByteArray()).toByteArray(), 1);

  // Set visibility of toolbars based on preferences stored in the database
  findChild<QToolBar*>("Products Tools")->setVisible(_preferences->boolean("ShowPDToolbar"));
  findChild<QToolBar*>("Inventory Tools")->setVisible(_preferences->boolean("ShowIMToolbar"));
  if(_metrics->value("Application") != "PostBooks")
  {
    findChild<QToolBar*>("Schedule Tools")->setVisible(_preferences->boolean("ShowMSToolbar"));
  }
  findChild<QToolBar*>("Purchase Tools")->setVisible(_preferences->boolean("ShowPOToolbar"));
  findChild<QToolBar*>("Manufacture Tools")->setVisible(_preferences->boolean("ShowWOToolbar"));
  findChild<QToolBar*>("CRM Tools")->setVisible(_preferences->boolean("ShowCRMToolbar"));
  findChild<QToolBar*>("Sales Tools")->setVisible(_preferences->boolean("ShowSOToolbar"));
  findChild<QToolBar*>("Accounting Tools")->setVisible(_preferences->boolean("ShowGLToolbar"));

  firstRun = false;
  qApp->restoreOverrideCursor();
}

/** @brief Save the position and visibility of application toolbars in
           user preferences.
  */
void GUIClient::saveToolbarPositions()
{
  xtsettingsSetValue("MainWindowState", saveState(1));

  // Set preferences base on visibility of toolbars
  _preferences->set("ShowPDToolbar", findChild<QToolBar*>("Products Tools")->isVisible());
  _preferences->set("ShowIMToolbar", findChild<QToolBar*>("Inventory Tools")->isVisible());
  if(_metrics->value("Application") != "PostBooks")
    _preferences->set("ShowMSToolbar", findChild<QToolBar*>("Schedule Tools")->isVisible());
  _preferences->set("ShowPOToolbar", findChild<QToolBar*>("Purchase Tools")->isVisible());
  _preferences->set("ShowWOToolbar", findChild<QToolBar*>("Manufacture Tools")->isVisible());
  _preferences->set("ShowCRMToolbar", findChild<QToolBar*>("CRM Tools")->isVisible());
  _preferences->set("ShowSOToolbar", findChild<QToolBar*>("Sales Tools")->isVisible());
  _preferences->set("ShowGLToolbar", findChild<QToolBar*>("Accounting Tools")->isVisible());
}

/** @brief Save information about the current state of the application
           when the main window closes and perform final cleanup.

    Examples include the size and position of application windows and
    toolbar visibility.
  */
void GUIClient::closeEvent(QCloseEvent *event)
{
  if (_showTopLevel || _workspace->viewMode() == QMdiArea::SubWindowView)
  {
    foreach (QWidget *child, QApplication::topLevelWidgets())
    {
      if (child != this && ! child->close())
      {
        event->ignore();
        return;
      }
    }
  }
  else
  {
    foreach (QMdiSubWindow *child, _workspace->subWindowList())
    {
      if (! child->close())
      {
        event->ignore();
        return;
      }
    }
  }

  hunspell_uninitialize();

  _shuttingDown = true;

  // Remove any temporary document files being watched
  QMapIterator<QString, int> i(_fileMap);
  while (i.hasNext())
  {
    i.next();
    removeDocumentWatch(i.key());
  }

  saveToolbarPositions();

  // save main window info for next login
  xtsettingsSetValue("GUIClient/geometry/pos",  pos());
  xtsettingsSetValue("GUIClient/geometry/size", size());

  // save state of any main window children
  QList<QMainWindow*> windows = findChildren<QMainWindow*>();
  for (int i = 0; i < windows.size(); i++)
  {
    QByteArray state = windows.at(i)->saveState();
    xtsettingsSetValue(windows.at(i)->objectName() + "/WindowState", state);
  }

  event->accept();
}

/** @brief Perform extra application initialization.

    Primarily this involves running application extension @c initMenu
    scripts and setting up the script engine debugger if necessary.
  */
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
        QString script = scriptHandleIncludes(sq.value("script_source").toString());
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

/** @brief Write a message to the debugging log.

    This will write to the DatabaseLog window if the debugging
    checkbox has been checked.

    On Linux and Mac workstations this will also write to the
    Terminal window (stdout) if the application was started from
    the command line. It will otherwise write to the Console
    application on Macs. On Windows machines you can attach to the
    application with a debugger to catch this.

    @param pError The message to write
  */
void GUIClient::sReportError(const QString &pError)
{
  qDebug("%s", qPrintable(pError));
}

/** @brief This method is called approximately once per minute.

    It checks the database to see if there are any new
    events for the current user and updates the status bar accordingly.
    If there is an error retrieving this information then the function
    warns the user that the database connection as been lost.

    Every few minutes, as determined by the @c updateTickInterval metric,
    this method emits the @c tick signal. This allows individual windows
    to track the passage of time or update themselves if desired without
    setting their own timers.

    @todo Handle aborted transactions more intelligently.
    @todo Make the check for lost database connections more intelligent.
    @todo If the database connection really is gone, try to reconnect.
    */
void GUIClient::sTick()
{
  //  Check the database. TODO: why do we ignore alarms and messages?
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
    }

    __intervalCount++;
    if(__intervalCount >= __interval)
    {
      emit(tick());
      __intervalCount = 0;
    }
  }
  else
  {
    // Check to make sure we are not in the middle of an aborted transaction
    // before we go doing something rash.
    if (!QSqlDatabase::database().isOpen())
    {
      if  (QMessageBox::question(this, tr("Database disconnected"),
                                tr("It appears that the you've been disconnected from the"
                                   "database. Select Yes to try to reconnect or "
                                   "No to terminate the application."),
                                   QMessageBox::Yes,
                                   QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
        qApp->quit();
      else
      {
        if (QSqlDatabase::database().open())
        {
          QString loginqry ="SELECT login() AS result, CURRENT_USER AS user;";
          XSqlQuery login( loginqry );
          if (login.first())
          {
            int result = login.value("result").toInt();
            if (result < 0)
            {
              QMessageBox::critical(this, tr("Error Relogging to the Database"),
                                    storedProcErrorLookup("login", result));
              return;
            }
          }
          else if (login.lastError().type() != QSqlError::NoError)
            QMessageBox::critical(this, tr("System Error"),
                                  tr("A System Error occurred at %1::%2:\n%3")
                                    .arg(__FILE__).arg(__LINE__)
                                    .arg(login.lastError().databaseText()));
        }
      }
    }
  }
  _tick.singleShot(30000, this, SLOT(sTick()));
}

/** @brief Make the error button in the main window's status bar visible.

    This is typically called if there are new messages in the
    Database Log window. This should only be called internally.
 */
void GUIClient::sNewErrorMessage()
{
  if (QApplication::closingDown())
    return;

  if (_errorButton)
    _errorButton->setVisible(true);
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

/** @brief Hide the error button in the main window's status bar.

    This is typically called when clearing the Database Log window.
  */
void GUIClient::sClearErrorMessages()
{
  if (_errorButton)
    _errorButton->setVisible(false);
}

/** @brief Tell the current user if s/he has new Social Desktop messages. */
void GUIClient::sSystemMessageAdded()
{
  emit systemMessageAdded();

  //  Grab any new System Messages
          XSqlQuery msg;
          msg.exec( "SELECT msguser_id "
                    "FROM msg, msguser "
                    "WHERE ( (msguser_username=getEffectiveXtUser())"
                    " AND (msguser_msg_id=msg_id)"
                    " AND (CURRENT_TIMESTAMP BETWEEN msg_scheduled AND msg_expires)"
                    " AND (msguser_viewed IS NULL) );" );
          if (msg.first())
          {
            ParameterList params;
            params.append("mode", "acknowledge");

            systemMessage *newdlg = new systemMessage();
            newdlg->set(params);

            do
            {
              ParameterList params;
              params.append("msguser_id", msg.value("msguser_id").toInt());

              newdlg->set(params);
              omfgThis->handleNewWindow(newdlg);
            }
            while (msg.next());
          }
}

/** @name Data Update Slots

    The GUIClient provides a number of slots that application windows
    call for the purpose of keeping @em other windows informed of
    data changes. Each slot typically emits one corresponding signal
    (see Data Update Signals below).  To be a good citizen, each
    window that makes a change to an object should call the appropriate
    GUIClient slot.  To keep informed of changes made by other
    windows, interested windows should connect to one of the Data
    Update Signals.  For example: the salesOrder window tells other
    windows that the current Sales Order was modified by calling:

    @code
    // in salesOrder::save()
    omfgThis->sSalesOrdersUpdated(_soheadid);

    // in openSalesOrders::openSalesOrders()
    connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList()));
    @endcode

    @note These slots can only be used to notify other windows in the
    same application instance. They do not notify other running programs
    on the same or other workstations.

    @todo Replace the explicit C++ slot connections with Postgres notifications?

    @{
*/

/** @brief This slot tells other open windows the definition or status of one or more Items has changed.
    @param pItemid the internal id of the Item or -1 for multiple or unspecified items
    @param pLocal unknown purpose
  */
void GUIClient::sItemsUpdated(int pItemid, bool pLocal)
{
  emit itemsUpdated(pItemid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Itemsites has changed. */
void GUIClient::sItemsitesUpdated()
{
  emit itemsitesUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Sites or Warehouses has changed. */
void GUIClient::sWarehousesUpdated()
{
  emit warehousesUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Contracts has changed.
    @param pContrctid the internal id of the Contract or -1 for multiple or unspecified Contracts
    @param pLocal unknown purpose
  */
void GUIClient::sContractsUpdated(int pContrctid, bool pLocal)
{
  emit contractsUpdated(pContrctid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Customers has changed.
    @param pCustid the internal id of the Customer or -1 for multiple or unspecified Customers
    @param pLocal unknown purpose
  */
void GUIClient::sCustomersUpdated(int pCustid, bool pLocal)
{
  emit customersUpdated(pCustid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Employees has changed.
    @param id the internal id of the Employee or -1 for multiple or unspecified Employees
  */
void GUIClient::sEmployeeUpdated(int id)
{
  emit employeeUpdated(id);
}

/** @brief This slot tells other open windows the definition or status of one or more G/L Series has changed. */
void GUIClient::sGlSeriesUpdated()
{
  emit glSeriesUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Vendors has changed. */
void GUIClient::sVendorsUpdated()
{
  emit vendorsUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Prospects has changed. */
void GUIClient::sProspectsUpdated()
{
  emit prospectsUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Return Authorizations has changed. */
void GUIClient::sReturnAuthorizationsUpdated()
{
  emit returnAuthorizationsUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Standard Periods has changed. */
void GUIClient::sStandardPeriodsUpdated()
{
  emit standardPeriodsUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Sales Orders has changed.
    @param pSoheadid the internal id of the Sales Order that changed or -1 for multiple or unspecified Sales Orders
  */
void GUIClient::sSalesOrdersUpdated(int pSoheadid)
{
  emit salesOrdersUpdated(pSoheadid, true);
}

/** @brief This slot tells other open windows the definition or status of one or more Sales Representatives has changed.
    @param id the internal id of the Sales Rep that changed or -1 for multiple or unspecified Sales Reps
  */
void GUIClient::sSalesRepUpdated(int id)
{
  emit salesRepUpdated(id);
}

/** @brief This slot tells other open windows the definition or status of one or more Credit Memos has changed. */
void GUIClient::sCreditMemosUpdated()
{
  emit creditMemosUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Quotes has changed.
    @param pQuheadid the internal id of the Quote that changed or -1 for multiple or unspecified Quotes */
void GUIClient::sQuotesUpdated(int pQuheadid)
{
  emit quotesUpdated(pQuheadid, true);
}

/** @brief This slot tells other open windows the definition or status of one or more Work Order Materials records has changed.
    @param pWoid the internal id of the Work Order that changed
    @param pWomatlid the internal id of the W/O Materials record that changed
    @param pLocal unknown purpose
  */
void GUIClient::sWorkOrderMaterialsUpdated(int pWoid, int pWomatlid, bool pLocal)
{
  emit workOrderMaterialsUpdated(pWoid, pWomatlid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Work Order Operations records has changed.

    @param pWoid the internal id of the Work Order that changed
    @param pWooperid the internal id of the Work Order Operation that changed
    @param pLocal unknown purpose
  */
void GUIClient::sWorkOrderOperationsUpdated(int pWoid, int pWooperid, bool pLocal)
{
  emit workOrderOperationsUpdated(pWoid, pWooperid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Work Orders has changed.
    @param pWoid the internal id of the Work Order that changed or -1 for multiple or unspecified work orders
    @param pLocal unknown purpose
  */
void GUIClient::sWorkOrdersUpdated(int pWoid, bool pLocal)
{
  emit workOrdersUpdated(pWoid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Purchase Orders has changed.
    @param pPoheadid the internal id of the Purchase Order that changed or -1 for multiple or unspecified Purchase Orders
    @param pLocal unknown purpose
  */
void GUIClient::sPurchaseOrdersUpdated(int pPoheadid, bool pLocal)
{
  emit purchaseOrdersUpdated(pPoheadid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Receipts has changed.

    The name @c sPurchaseORderReceiptsUpdated is no longer accurate, as this
    applies to any receipt.
  */
void GUIClient::sPurchaseOrderReceiptsUpdated()
{
  emit purchaseOrderReceiptsUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Purchase Requests has changed. */
void GUIClient::sPurchaseRequestsUpdated()
{
  emit purchaseRequestsUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Vouchers has changed. */
void GUIClient::sVouchersUpdated()
{
  emit vouchersUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Bills of Materials has changed.
    @param pItemid the internal id of the Item for which the Bill of Materials changed or -1 for multiple or unspecified Items
    @param pLocal unknown purpose
  */
void GUIClient::sBOMsUpdated(int pItemid, bool pLocal)
{
  emit bomsUpdated(pItemid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Breeder Bills of Materials has changed.
    @param pItemid the internal id of the Item for which the Breeder Bill of Materials changed or -1 for multiple or unspecified Items
    @param pLocal unknown purpose
  */
void GUIClient::sBBOMsUpdated(int pItemid, bool pLocal)
{
  emit bbomsUpdated(pItemid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Bills of Operations has changed.
    @param pItemid the internal id of the Item for which the Bill of Operations changed or -1 for multiple or unspecified Items
    @param pLocal unknown purpose
  */
void GUIClient::sBOOsUpdated(int pItemid, bool pLocal)
{
  emit boosUpdated(pItemid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Budgets has changed.
    @param pItemid the internal id of the Budget which changed or -1 for multiple or unspecified Budgets
    @param pLocal unknown purpose
 */
void GUIClient::sBudgetsUpdated(int pItemid, bool pLocal)
{
  emit budgetsUpdated(pItemid, pLocal);
}

void GUIClient::sAssortmentsUpdated(int pItemid, bool pLocal)
{
  emit assortmentsUpdated(pItemid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Work Centers has changed. */
void GUIClient::sWorkCentersUpdated()
{
  emit workCentersUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Billing Selections has changed.
    @param pCoheadid the internal id of the Sales Order for which a Billing Selection changed or -1 for multiple or unspecified Sales Orders
    @param pCoitemid the internal id of the Sales Order line item for which a Billing Selection changed or -1 for multiple or unspecified Sales Order lines
  */
void GUIClient::sBillingSelectionUpdated(int pCoheadid, int pCoitemid)
{
  emit billingSelectionUpdated(pCoheadid, pCoitemid);
}

/** @brief This slot tells other open windows the definition or status of one or more Invoices has changed.
    @param pInvcheadid the internal id of the Invoice which changed or -1 for multiple or unspecified Invoices
    @param pLocal unknown purpose
  */
void GUIClient::sInvoicesUpdated(int pInvcheadid, bool pLocal)
{
  emit invoicesUpdated(pInvcheadid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Item Groups has changed.
    @param pItemgrpid the Item Group which changed or -1 for multiple or unspecified Item Groups
    @param pLocal unknown purpose
 */
void GUIClient::sItemGroupsUpdated(int pItemgrpid, bool pLocal)
{
  emit itemGroupsUpdated(pItemgrpid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Cash Receipts has changed.
    @param pCashrcptid the Cash Receipt which changed or -1 for multiple or unspecified Cash Receipts
    @param pLocal unknown purpose
 */
void GUIClient::sCashReceiptsUpdated(int pCashrcptid, bool pLocal)
{
  emit cashReceiptsUpdated(pCashrcptid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Bank Accounts has changed. */
void GUIClient::sBankAccountsUpdated()
{
  emit bankAccountsUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Bank Adjustments has changed.
    This is the actual adjustments, not bank adjustment types.
  */
void GUIClient::sBankAdjustmentsUpdated(int pBankadjid, bool pLocal)
{
  emit bankAdjustmentsUpdated(pBankadjid, pLocal);
}

/** @brief This slot tells other open windows the Quantity on Hand of one or more Item Sites has changed.
    @param pItemsiteid the Item Site for which the QOH changed or -1 for multiple or unspecified Item Sites
    @param pLocal unknown purpose
  */
void GUIClient::sQOHChanged(int pItemsiteid, bool pLocal)
{
  emit qohChanged(pItemsiteid, pLocal);
}

/** @brief This slot tells other open windows one or more Report definitions has changed.
    @param pReportid the internal id of the report that changed
    @param pLocal unknown purpose
  */
void GUIClient::sReportsChanged(int pReportid, bool pLocal)
{
  emit reportsChanged(pReportid, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Bank Checks or Payments has changed.
    @param pBankaccntid the internal id of the Bank Account from which a Payment was made or modified
    @param pCheckid the internal id of the Check which changed or -1 for non-check payments, multiple Checks and Payments, or unspecified Checks and Payments
    @param pLocal unknown purpose
  */
void GUIClient::sChecksUpdated(int pBankaccntid, int pCheckid, bool pLocal)
{
  emit checksUpdated(pBankaccntid, pCheckid, pLocal);
  emit paymentsUpdated(pBankaccntid, -1, pLocal);
}

/** @brief This slot tells other open windows the definition or status of one or more Bank Payments has changed.
    @param pBankaccntid the internal id of the Bank Account from which a Payment was made or modified
    @param pApselectid the internal id of the Payment Selection which changed or -1 for multiple or unspecified Payment Selections
    @param pLocal unknown purpose
  */
void GUIClient::sPaymentsUpdated(int pBankaccntid, int pApselectid, bool pLocal)
{
  emit paymentsUpdated(pBankaccntid, pApselectid, pLocal);
}

/** @brief This slot tells other open windows the G/L Configuration has changed. */
void GUIClient::sConfigureGLUpdated()
{
  emit configureGLUpdated();
}

/** @brief This slot tells other open windows the definition or status of one or more Projects has changed.
    @param prjid the internal id of the Project which changed or -1 for multiple or unspecified Projects
  */
void GUIClient::sProjectsUpdated(int prjid)
{
  emit projectsUpdated(prjid);
}

/** @brief This slot tells other open windows the definition or status of one or more CRM Accounts has changed.
    @param crmacctid the internal id of the CRM Account which changed or -1 for multiple or unspecified CRM Accounts
 */
void GUIClient::sCrmAccountsUpdated(int crmacctid)
{
  emit crmAccountsUpdated(crmacctid);
}

/** @brief This slot tells other open windows the definition or status of one or more Tax Authorities has changed.
    @param taxauthid the internal id of the Tax Authority which changed or -1 for multiple or unspecified Tax AUthorities
  */
void GUIClient::sTaxAuthsUpdated(int taxauthid)
{
  emit taxAuthsUpdated(taxauthid);
}

/** @brief This slot tells other open windows the definition or status of one or more Transfer Orders has changed.
    @param id the internal id of the Transfer Order which changed or -1 for multiple or unspecified Transfer Orders
  */
void GUIClient::sTransferOrdersUpdated(int id)
{
  emit transferOrdersUpdated(id);
}

/** @brief This slot tells other open windows the definition or status of a User has changed.
    @param username the username of the User which changed
  */
void GUIClient::sUserUpdated(QString username)
{
  emit userUpdated(username);
}
/** @} */

/** @brief A generic slot for extension scripts to emit their own signals.

    @param source  the script-defined source of this signal
    @param message a QString payload sent with the signal

   Example Usage:
   @code
   // connection and signal handler
   mainwindow["emitSignal(QString, QString)"].connect(emitTest);

   function emitTest(source, message) {
     if (source == "myCustomScript") {
       QMessageBox.information(mywindow, source, message);
     }
   }

   // sample function for emitting the signal via script
   function sendTestSignal() {
     mainwindow.sEmitSignal("myCustomScript", "my custom message");
   }
   @endcode

 */
void GUIClient::sEmitSignal(QString source, QString message)
{
  emit emitSignal(source, message);
}

/** @brief A generic slot for extension scripts to emit their own signals.

    @param source the script-defined source of this signal
    @param id     an integer payload sent with the signal

    @overload
 */
void GUIClient::sEmitSignal(QString source, int id)
{
  emit emitSignal(source, id);
}

/** @brief A generic slot for extension scripts to emit their own signals.

    @param source the script-defined source of this signal
    @param value  a boolean payload sent with the signal

    @overload
 */
void GUIClient::sEmitSignal(QString source, bool value)
{
  emit emitSignal(source, value);
}

/** @brief A slot to ask the user if s/he wants to log out.

    This should only be called internally after @c IdleTimeout minutes
    have passed without user activity. This value is a user preference.
  */
void GUIClient::sIdleTimeout()
{
 // so we don't accidentally get called again waiting
  _timeoutHandler->reset();

  ParameterList params;
  params.append("minutes", _timeoutHandler->idleMinutes());

  idleShutdown newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() == XDialog::Accepted)
    qApp->quit();
}

/** @deprecated systemError should be replaced with ErrorReporter::error */
int systemError(QWidget *pParent, const QString &pMessage)
{
  ErrorReporter::error(QtCriticalMsg, pParent, QObject::tr("System Message"),
                       pMessage);
  return QMessageBox::Ok;
}

/** @deprecated systemError should be replaced with ErrorReporter::error
    @overload
 */
int systemError(QWidget *pParent, const QString &pMessage, const QString &pFileName, const int lineNumber)
{
  ErrorReporter::error(QtCriticalMsg, pParent,
                       QObject::tr("System Message (%1 at %2)")
                                   .arg(pFileName).arg(lineNumber),
                       pMessage, pFileName, lineNumber);
  return QMessageBox::Ok;
}

/** @brief Write a message to the main window's status bar.

    As a side effect, this also allows the application to process queued
    events.

    @param pMessage the message to write
    @param pTimeout the number of milliseconds to show the message or
                    0 to leave it there until another message is shown
  */
void message(const QString &pMessage, int pTimeout)
{
  if (pTimeout == 0)
    omfgThis->statusBar()->showMessage(pMessage);
  else
    omfgThis->statusBar()->showMessage(pMessage, pTimeout);

  qApp->processEvents();
}

/** @brief Write a generic "Ready" message to the main window's status bar.

    As a side effect, this also allows the application to process queued
    events.
  */
void resetMessage()
{
  omfgThis->statusBar()->showMessage(QObject::tr("Ready..."));
  qApp->processEvents();
}

/** @brief Give an audio indicator that an event has been accepted. */
void audioAccept()
{
  qApp->beep();
}

/** @brief Give an audio indicator that an event has been rejected. */
void audioReject()
{
  qApp->beep();
}

/** @brief Find the translation file for a given locale.

    Looks for the translation %file for a particular locale in all of the
    standard places xTuple ERP knows to look. The first %file found is returned
    even if it isn't the most complete, specific, or up-to-date.

    @param localestr The locale to look for, in standard format.
    @param component The application component for which to find a
                     translation file (empty string means core)

    @return The path to the translation file (may be relative or absolute)
 */
QString translationFile(QString localestr, const QString component)
{
  QString version = QString::null;
  return translationFile(localestr, component, version);
}

/** @brief Find the translation file for a given locale.

    This overload should be used primarily by the Update Manager.

    Looks for the translation %file for a particular locale in all
    of the standard places xTuple ERP knows to look. The first %file
    found is returned even if it isn't the most complete, specific,
    or up-to-date.  If translation %file is found for the locale,
    this overload of translationFile(QString) tries to extract a
    version number from the translation %file and pass it back to
    the caller.

    The base translation file is expected to have a @c Version
    string in the component context. Translations from that base
    translation %file are expected to translate the @c Version
    string to something meaningful that can be put in the compatibility
    matrix. One suggestion is @c major.minor.percent-complete ,
    where @c major and @c minor are component release numbers and
    percent-complete indicates how much of the base translation
    file has been completed.

    @param[in]  localestr The locale to look for, in standard format.
    @param[in]  component The application component for which to find a
                          translation file (empty string means core)
    @param[out] version   The version string found in the translation file or
                          an empty string if none was found.

    @return The path to the translation file (may be relative or absolute)
 */
QString translationFile(QString localestr, const QString component, QString &version)
{
  QStringList paths;
#if QT_VERSION >= 0x050000
  paths << QStandardPaths::standardLocations(QStandardPaths::DataLocation);
#else
  //qDebug() << QDesktopServices::storageLocation(QDesktopServices::DataLocation);
  paths << QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
  paths << "/usr/lib/postbooks/dict";
  paths << "dict";
  paths << "";
  paths << "../dict";
  paths << QApplication::applicationDirPath() + "/dict";
  paths << QApplication::applicationDirPath();
  paths << QApplication::applicationDirPath() + "/../dict";
#if defined Q_OS_MAC
  paths << QApplication::applicationDirPath() + "/../../../dict";
  paths << QApplication::applicationDirPath() + "/../../..";
#endif

  QTranslator translator;
  for (QStringList::Iterator pit = paths.begin(); pit != paths.end(); pit++)
  {
    QString filename = *pit + "/" + component + "." + localestr;
    if (translator.load(filename))
    {
      if (! version.isNull())
        version = translator.translate(component.toLatin1().data(), "Version");

      return filename;
    }
  }

  return QString::null;
}

/** @brief Build a Custom submenu from the @c cmd table.

    There are two ways to add items to the xTuple ERP menu system: write
    an extension script called @c initMenu and install it as part of an
    extension package, or create Custom Commands using the @c cmd and
    @c cmdarg tables, either manually or through an extension package.
    GUIClient::populateCustomMenu() reads the @c cmd table, creates
    a submenu called @b Custom for the named module if necessary, and adds
    the appropriate custom commands to the menu.

    @param menu   the application menu to extend
    @param module the value of the @c cmd_module column to filter on to find
                  commands to add to the given menu
  */
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

    QString cmdname = QString("custom." + qry.value("cmd_name").toString());
    Action *action = new Action(this, cmdname.toLatin1().data(), qry.value("cmd_title").toString(),
                                this, SLOT(sCustomCommand()), customMenu, allowed);

    _customCommands.insert(action, qry.value("cmd_id").toInt());
    //actions.append(action);
  }
}

/** @brief The slot called whenever a custom command is invoked from the
           menu system.

    This should never be called directly.
  */
void GUIClient::sCustomCommand()
{
  XSqlQuery GCustomCommand;
  const QObject * obj = sender();
  QMap<const QObject*,int>::const_iterator it;
  it = _customCommands.find(obj);
  if(it != _customCommands.end())
  {
    GCustomCommand.prepare("SELECT cmd_executable"
              "  FROM cmd"
              " WHERE(cmd_id=:cmd_id);");
    GCustomCommand.bindValue(":cmd_id", it.value());
    GCustomCommand.exec();
    GCustomCommand.first();
    QString cmd = GCustomCommand.value("cmd_executable").toString();
    if(cmd.toLower() == "!customuiform")
    {
      bool haveParams = false;
      ParameterList params;
      bool asDialog = false;
      QString asName;
      GCustomCommand.prepare("SELECT cmdarg_arg AS argument"
                "  FROM cmdarg"
                " WHERE (cmdarg_cmd_id=:cmd_id)"
                " ORDER BY cmdarg_order; ");
      GCustomCommand.bindValue(":cmd_id", it.value());
      GCustomCommand.exec();
      while(GCustomCommand.next())
      {
        cmd = GCustomCommand.value("argument").toString();
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
      GCustomCommand.prepare("SELECT *"
                "  FROM uiform"
                " WHERE((uiform_name=:uiform_name)"
                "   AND (uiform_enabled))"
                " ORDER BY uiform_order DESC"
                " LIMIT 1;");
      GCustomCommand.bindValue(":uiform_name", asName);
      GCustomCommand.exec();
      if(!GCustomCommand.first())
      {
        QMessageBox::critical(this, tr("Could Not Create Form"),
                              tr("<p>Could not create the '%1' form. Either an "
                                 "error occurred or the specified form does "
                                 "not exist.").arg(asName) );
        return;
      }

      XUiLoader loader;
      QByteArray ba = GCustomCommand.value("uiform_source").toString().toUtf8();
      QBuffer uiFile(&ba);
      if(!uiFile.open(QIODevice::ReadOnly))
      {
        QMessageBox::critical(this, tr("Could not load file"),
            tr("There was an error loading the UI Form from the database."));
        return;
      }
      QWidget *ui = loader.load(&uiFile);
      QSize size = ui->size();
      uiFile.close();

      if(asDialog)
      {
        XDialog dlg(this);
        dlg.setObjectName(GCustomCommand.value("uiform_name").toString());
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(ui);
        dlg.setLayout(layout);
        dlg.setWindowTitle(ui->windowTitle());
        dlg.resize(size);
        dlg.exec();
      }
      else
      {
        XMainWindow * wnd = new XMainWindow();
        wnd->setObjectName(GCustomCommand.value("uiform_name").toString());
        wnd->setCentralWidget(ui);
        wnd->setWindowTitle(ui->windowTitle());
        wnd->resize(size);
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
      GCustomCommand.prepare("SELECT cmdarg_arg "
                "FROM cmdarg "
                "WHERE (cmdarg_cmd_id=:cmd_id) "
                "ORDER BY cmdarg_order;");
      GCustomCommand.bindValue(":cmd_id", it.value());
      GCustomCommand.exec();
      while (GCustomCommand.next())
      {
        QUrl url(GCustomCommand.value("cmdarg_arg").toString());
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
      GCustomCommand.prepare("SELECT 1 AS base,"
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
      GCustomCommand.bindValue(":cmd_id", it.value());
      GCustomCommand.exec();
      if (GCustomCommand.first())
      {
        command = GCustomCommand.value("argument").toString();
        while(GCustomCommand.next())
          args.append(GCustomCommand.value("argument").toString());
        proc->start(command, args);
      }
    }
  }
}

/** @brief Open a web browser to a given URL.

    @arg MS Windows - the operating system does all of the work
    @arg Mac OS X   - start a separate process using
                      the @c BROWSER environment variable
                      then Apple's @c open if that doesn't work
    @arg Linux      - start a separate process using
                      @c BROWSER, followed by @c /usr/bin/firefox
                      and @c /usr/bin/mozilla if that doesn't work
  */
void GUIClient::launchBrowser(QWidget * w, const QString & url)
{
#if defined(Q_OS_WIN) && QT_VERSION < 0x050000
  // Windows - let the OS do the work , needs qt5 replacement
  QT_WA( {
      ShellExecute(w->winId(), 0, (TCHAR*)url.utf16(), 0, 0, SW_SHOWNORMAL );
    } , {
      ShellExecuteA(w->winId(), 0, url.toLocal8Bit(), 0, 0, SW_SHOWNORMAL );
    } );
#else
  const char *b = getenv("BROWSER");
  QStringList browser;
  if(b) {
    QString t(b);
    browser = t.split(':', QString::SkipEmptyParts);
  }
#if defined(Q_OS_MAC)
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
    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), proc, SLOT(deleteLater()));
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
                             tr("<p>Before you can run a browser you must set "
                                "the environment variable BROWSER to point "
                                "to the browser executable.") );
  }
#endif
}

/** @brief Return the list of windows opened by GUIClient::handleNewWindow().
 */
QWidgetList GUIClient::windowList()
{
  return _windowList;
}

/** @brief Slot called when a window opened by GUIClient::handleNewWindow()
           emits destroyed().

    This should not be called directly.
  */
void GUIClient::windowDestroyed(QObject * o)
{
  QWidget * w = qobject_cast<QWidget *>(o);
  if(w)
    _windowList.removeAll(w);
}

/** @brief Save the size and position of windows when they close. */
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
      else if (w->parentWidget() != 0)
        xtsettingsSetValue(objName + "/geometry/pos", w->parentWidget()->pos());
    }
  }
  return QObject::eventFilter(obj, event);
}

/** @brief One way to open application windows from the C++ core.

    This method takes care of setting window position and size; opening
    properly for free-floating, workspace, and tabbed modes; and
    preparing the window to clean up properly when it closes.

    @todo Document all of the ways to open windows and when to use them.
  */
void GUIClient::handleNewWindow(QWidget *w, Qt::WindowModality m, bool forceFloat)
{
  // TODO:  replace this function with a centralized openWindow function
  // used by toolbox, guiclient interface, and core windows

  #ifdef Q_OS_MAC
    	updateMacDockMenu(w);
  #endif
  
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
        // Get the focusWidget and then reset it as the setParent changes it.
        QWidget * fw = w->focusWidget();
        w->setParent(0);
        if(fw)
          fw->setFocus();
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
  if(!_showTopLevel && !w->isModal() && !forceFloat)
    availableGeometry = _workspace->geometry();

  QString objName = w->objectName();
  QPoint pos = xtsettingsValue(objName + "/geometry/pos").toPoint();
  QSize size = xtsettingsValue(objName + "/geometry/size").toSize();

  if(size.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool() && (metaObject()->className() != QString("xTupleDesigner")))
    w->resize(size);

  bool wIsModal = w->isModal();
  if(_showTopLevel || wIsModal || forceFloat)
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
    QMdiSubWindow *subwin = _workspace->addSubWindow(w);
    _workspace->setActiveSubWindow(subwin);
    connect(w, SIGNAL(destroyed(QObject*)), subwin, SLOT(close()));
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
#ifdef Q_OS_MAC
  if (_menuBar == 0)
    _menuBar = new QMenuBar();

  return _menuBar;
#else
  return QMainWindow::menuBar();
#endif
}

/** @brief Adds the given @a dockwidget to the specified @a area.
   @sa addToolBarBreak()
  */
void GUIClient::addDockWidget ( Qt::DockWidgetArea area, QDockWidget * dockwidget )
{
  QMainWindow::addDockWidget( area, dockwidget);
}

/** @brief Add the given @a toolbar to the top toolbar dock.  */
void GUIClient::addToolBar ( QToolBar * toolbar )
{
  QMainWindow::addToolBar(Qt::TopToolBarArea, toolbar);
}

/** @brief Add the toolbar to the specified area in this main window.

   The toolbar is placed at the end of the current toolbar block (i.e. line).
   If the main window already manages the toolbar then it will
   only move the toolbar to area.

   @overload
   @sa addToolBarBreak()
*/
void GUIClient::addToolBar ( Qt::ToolBarArea area, QToolBar * toolbar )
{
  QMainWindow::addToolBar(area, toolbar);
}

/** @brief Add a toolbar break to the given area after all the other objects
           that are present.
*/
void GUIClient::addToolBarBreak ( Qt::ToolBarArea area )
{
  QMainWindow::addToolBarBreak(area);
}

/** @brief Moves @a second dock widget on top of @a first dock widget,
           creating a tabbed docked area in the main window.
*/
void GUIClient::tabifyDockWidget ( QDockWidget * first, QDockWidget * second )
{
  QMainWindow::tabifyDockWidget( first, second );
}

/** @brief Sets the given @a widget to be the main window's central widget.

    Note: GUIClient takes ownership of the @a widget pointer and
    deletes it at the appropriate time.
*/
void GUIClient::setCentralWidget(QWidget * widget)
{
  QMainWindow::setCentralWidget(widget);
}

/** @brief A slot called when the user's focus has changed from one window
           to another.

    This slot should not be used except internally by GUIClient.
  */
void GUIClient::sFocusChanged(QWidget *old, QWidget *now)
{
  Q_UNUSED(old);
  Q_UNUSED(now);
  QWidget * thisActive = workspace()->activeSubWindow();
  if(omfgThis->showTopLevel())
    thisActive = qApp->activeWindow();
  if(thisActive == this)
    return;
  if(thisActive && thisActive->inherits("QMessageBox"))
    return;
  _activeWindow = thisActive;
}

/** @brief Return the currently active window. */
QWidget * GUIClient::myActiveWindow()
{
  return _activeWindow;
}

/** @brief Create a window from extension scripts to adjust inventory .

    @sa distributeInventory::SeriesAdjust()
  */
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

/** @brief Load JavaScript global variables into a QScriptEngine.

    This method loads global variables and objects such as the script
    @c toolbox , @c mainwindow , @c metrics , various constants, etc.

    This should never be called directly.
 */
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
  qScriptRegisterMetaType(engine, WindowSystemtoScriptValue, WindowSystemfromScriptValue);

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

  mainwindowval.setProperty("X11",  QScriptValue(engine, X11),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("WIN",  QScriptValue(engine, WIN),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("MAC",  QScriptValue(engine, MAC),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("QWS",  QScriptValue(engine, QWS),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("WINCE",  QScriptValue(engine, WINCE),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("S60",  QScriptValue(engine, S60),
                            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  mainwindowval.setProperty("Unknown",  QScriptValue(engine, Unknown),
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
  setupSetupApi(engine);
}

void GUIClient::addDocumentWatch(QString path, int id)
{
  if (_fileWatcher->files().contains(path))
    _fileWatcher->removePath(path);
  _fileWatcher->addPath(path);
  _fileMap.insert(path,id);
}

bool GUIClient::removeDocumentWatch(QString path)
{
  bool result;
  if (_fileWatcher->files().contains(path))
    _fileWatcher->removePath(path);
  _fileMap.remove(path);
  QFileInfo fi = QFileInfo(path);
  QFile().remove(path);
  result = QDir().rmdir(fi.path());
  return result;
}

void GUIClient::handleDocument(QString path)
{
  QByteArray  bytarr;
  QFile sourceFile(path);
  bool opened = false;

  QTimer retry(this);
  retry.setInterval(5000);
  retry.setSingleShot(true);
  retry.start();
  while (retry.isActive() && !opened)
  {
    qApp->processEvents();
    // Retry for 5 sec. so OS can finish any housekeeping.
    // In particular, Microsoft Office deletes and copies
    // files on every save which can take a bit of time to
    // complete.
    opened = sourceFile.open(QIODevice::ReadOnly);
  }

  if (!opened)
  {
    qWarning("File %s could not be opened. Changes will not be saved to the database.",
       qPrintable(path));
    return;
  }

  int id = _fileMap.value(path);

  bytarr = sourceFile.readAll();
  XSqlQuery qry;
  qry.prepare( "UPDATE url SET url_stream = :stream "
               "WHERE (url_id = :id);" );
  qry.bindValue(":id", id);
  qry.bindValue(":stream", bytarr);
  qry.exec();
  addDocumentWatch(path, id);
}

void GUIClient::hunspell_initialize()
{
    _spellReady = false;
    QString langName = QLocale::languageToString(QLocale().language());
    QString appPath("/usr/lib/postbooks");
    if (! QFile::exists(appPath))
      appPath = QApplication::applicationDirPath();
    QString fullPathWithoutExt = appPath + "/" + langName;
    QFile affFile(fullPathWithoutExt + tr(".aff"));
    QFile dicFile(fullPathWithoutExt + tr(".dic"));
    // If we don't have files for the first name lets try a more common naming convention
    if(!(affFile.exists() && dicFile.exists()))
    {
      langName = QLocale().name().toLower(); // retruns lang_cntry format en_us for example
      fullPathWithoutExt = appPath + "/" + langName;
      affFile.setFileName(fullPathWithoutExt + tr(".aff"));
      dicFile.setFileName(fullPathWithoutExt + tr(".dic"));
    }
    if(affFile.exists() && dicFile.exists())
    {
      _spellReady = true;
    }
    _spellChecker = new Hunspell(QString(fullPathWithoutExt+tr(".aff")).toLatin1(),
                                 QString(fullPathWithoutExt+tr(".dic")).toLatin1());

    QString spell_encoding = QString(_spellChecker->get_dic_encoding());
    _spellCodec = QTextCodec::codecForName(spell_encoding.toLocal8Bit());

    QString homePath = QDir::homePath().toLatin1();
    if(_spellReady)
    {
        QFile file(homePath + tr("/xTuple/user.dic"));
        if(file.exists(homePath + tr("/xTuple/user.dic")))
        {
           //open user dictionary if exists
           _spellChecker->add_dic(QString(homePath + tr("/xTuple/user.dic")).toLatin1());
        }
    }
}

void GUIClient::hunspell_uninitialize()
{
    delete (Hunspell *)(_spellChecker);
    QString homePath = QDir::homePath().toLatin1();
    QFile file(homePath + tr("/xTuple/user.dic"));

    if(_spellReady && !_spellAddWords.isEmpty())
    {
      //if user directory missing create it
      QString homePath = QDir::homePath().toLatin1();
      QDir dir(homePath);
      if(!dir.exists(homePath + tr("/xTuple")))
      {
            dir.mkpath(tr("xTuple"));
      }
      //get old words from file
      if (file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
           QTextStream in(&file);
           //skip word count line
           in.readLine();
           while (!in.atEnd())
           {
             //add old words to stringlist
             QString line = in.readLine();
             if(!_spellAddWords.contains(line))
                 _spellAddWords.append(line);
           }
           file.close();
      }
      //store new words added and old words
      if (file.open(QIODevice::WriteOnly | QIODevice::Text))
      {
         QTextStream out(&file);
         //add word count to file first line
         out << _spellAddWords.count() << "\n";
         //add all new words in sort order
         _spellAddWords.sort();
         foreach(QString word, _spellAddWords)
         {
           QByteArray encodedString = _spellCodec->fromUnicode(word);
           out << encodedString.data() << "\n";
         }
         file.close();
      }
    }
}

bool GUIClient::hunspell_ready()
{
       return _spellReady;
}

int GUIClient::hunspell_check(const QString word)
{
      QByteArray encodedString = _spellCodec->fromUnicode(word);
      return _spellChecker->spell(encodedString.data());
}

const QStringList GUIClient::hunspell_suggest(const QString word)
{
    char **wlst;
    QStringList wordList;
    QByteArray encodedString = _spellCodec->fromUnicode(word);
    if(_spellChecker->spell(encodedString.data()) < 1)
    {
      int suggestNum = _spellChecker->suggest(&wlst, encodedString.data());
      if (suggestNum > 0)
      {
         for (int i=0; i < suggestNum; i++)
             wordList.append(_spellCodec->toUnicode(wlst[i]));
      }
      _spellChecker->free_list(&wlst, suggestNum);
    }
    return wordList;
}

int GUIClient::hunspell_add(const QString word)
{
    QByteArray encodedString = _spellCodec->fromUnicode(word);
    //check if word has been added before
    if(!_spellAddWords.contains(encodedString.data()))
        _spellAddWords.append(encodedString.data());
    return _spellChecker->add(encodedString.data());
}

int GUIClient::hunspell_ignore(const QString word)
{
    QByteArray encodedString = _spellCodec->fromUnicode(word);
    return _spellChecker->add(encodedString.data());
}


/** @brief Subscribe to the named Postgres notification.
    @param note the name of the notification to listen for.
    @sa GUIClient:sEmitNotifyHeard()
    @sa GUIClient:messageNotify()
  */
void GUIClient::setUpListener(const QString &note)
{
    if(QSqlDatabase::database().isOpen())
    {
        QSqlDatabase::database().driver()->subscribeToNotification(note);
        QObject::connect(QSqlDatabase::database().driver(), SIGNAL(notification(const QString&)),
                this, SLOT(sEmitNotifyHeard(const QString &)));
    }
}

/** @brief A slot used by setUpListener() for responding to Postgres notifications.

    This should not be called directly.
    @todo make this emit @c messageNotify when notifications other than
          @c testNote and @c messagePosted occur
 */
void GUIClient::sEmitNotifyHeard(const QString &note)
{
    if(note == "testNote")
        QMessageBox::information(this, "asdf", "test note received");
    else if(note == "messagePosted")
        emit messageNotify();
}
#ifdef Q_OS_MAC
    void GUIClient::updateMacDockMenu(QWidget *w)
    {
        QAction *action = new QAction(w);
        action->setText(w->windowTitle());

        _menu->addAction(action);

        qt_mac_set_dock_menu(_menu);

        connect(action, SIGNAL(triggered()), w, SLOT(hide()));
        connect(action, SIGNAL(triggered()), w, SLOT(show()));
    }

    void GUIClient::removeFromMacDockMenu(QWidget *w)
    {
        foreach (QAction *action, _menu->actions())
        {
            if(action->text().compare(w->windowTitle()) == 0)
            {
                _menu->removeAction(action);
            }
        }

        qt_mac_set_dock_menu(_menu);
    }
#endif
