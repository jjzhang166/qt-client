/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef GUICLIENT_H
#define GUICLIENT_H

#include <QAction>
#include <QDate>
#include <QList>
#include <QMainWindow>
#include <QTimer>

#include <xsqlquery.h>

#include "../common/format.h"
#include "../hunspell/hunspell.hxx"

class QCheckBox;
class QCloseEvent;
class QDockWidget;
class QDoubleValidator;
class QFileSystemWatcher;
class QIntValidator;
class QMdiArea;
class QMenu;
class QMenuBar;
class QPushButton;
class QScriptEngine;
class QShowEvent;
class QSplashScreen;
class QToolbar;

class menuProducts;
class menuInventory;
class menuSchedule;
class menuPurchase;
class menuManufacture;
class menuCRM;
class menuSales;
class menuAccounting;
class menuWindow;
class menuSystem;

class TimeoutHandler;
class InputManager;
class ReportHandler;

class XMainWindow;
class XWidget;


#define cNew                  1
#define cEdit                 2
#define cView                 3
#define cCopy                 4
#define cRelease              5
#define cPost                 6
#define cReplace              7

#define cPostedCounts         0x01
#define cUnpostedCounts       0x02
#define cAllCounts            0x04

//  Possible Location/Itemloc Sources
#define cLocation             0x01
#define cItemloc              0x02

//  Possible Transactions Groups
#define cTransAll             0xFF
#define cTransReceipts        0x01
#define cTransIssues          0x02
#define cTransShipments       0x04
#define cTransAdjCounts       0x08
#define cTransTransfers       0x10
#define cTransScraps          0x20

// Possible return values from submitReport
#define cNoReportDefinition   -2

int  systemError(QWidget *, const QString &);
int  systemError(QWidget *, const QString &, const QString &, const int);
void message(const QString &, int = 0);
void resetMessage();
void audioAccept();
void audioReject();
QString translationFile(const QString localestr, const QString component);
QString translationFile(const QString localestr, const QString component, QString &version);

extern bool _evaluation;

extern QSplashScreen *_splash;

#include "../common/metrics.h"
extern Metrics     *_metrics;
extern Preferences *_preferences;
extern Privileges  *_privileges;
#include "../common/metricsenc.h"
extern Metricsenc  *_metricsenc;


enum SetResponse
{
  NoError, NoError_Cancel, NoError_Run, NoError_Print, NoError_Submit,
  Error_NoSetup, UndefinedError
};


class Action : public QAction
{
  public:
    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool );

    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool,
            const QPixmap *, QWidget *);
            
    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool,
            const QPixmap *, QWidget *,
            const QString &);

    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, const QString & );

    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, const QString &,
            const QPixmap *, QWidget *);
            
    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, const QString &,
            const QPixmap *, QWidget *,
            const QString &); 

  private:
    void init( QWidget *pParent, const char *pName, const QString &pDisplayName,
               QObject *pTarget, const char *pActivateSlot,
               QWidget *pAddTo,  const QString &pEnabled);
};

class GUIClient : public QMainWindow
{
  friend class XWidget;
  friend class XMainWindow;
  friend class XDialog;
  friend class xTupleGuiClientInterface;

  Q_OBJECT

  Q_PROPERTY(QString key READ key)
  
  public:
    enum WindowSystem {
      Unknown, X11, WIN, MAC, QWS, WINCE, S60
    };

    GUIClient(const QString &, const QString &);
    virtual ~GUIClient();

    Q_INVOKABLE void setUpListener(const QString &);

    Q_INVOKABLE void setCaption();
    Q_INVOKABLE void saveToolbarPositions();

    Q_INVOKABLE inline QMdiArea *workspace()         { return _workspace;    }
    Q_INVOKABLE inline InputManager *inputManager()    { return _inputManager; }
    Q_INVOKABLE inline QString databaseURL()           { return _databaseURL;  }
    Q_INVOKABLE inline QString username()              { return _username;     }

    Q_INVOKABLE inline const QDate startOfTime()       { return _startOfTime;  }
    Q_INVOKABLE inline const QDate endOfTime()         { return _endOfTime;    }
    Q_INVOKABLE inline const QDate dbDate()            { return _dbDate;       }

    Q_INVOKABLE inline QDoubleValidator *qtyVal()      { return _qtyVal;       }
    Q_INVOKABLE inline QDoubleValidator *transQtyVal() { return _transQtyVal;  }
    Q_INVOKABLE inline QDoubleValidator *qtyPerVal()   { return _qtyPerVal;    }
    Q_INVOKABLE inline QDoubleValidator *scrapVal()    { return _scrapVal;     } 
    Q_INVOKABLE inline QDoubleValidator *percentVal()  { return _percentVal;   }
    Q_INVOKABLE inline QDoubleValidator *negPercentVal()  { return _negPercentVal;   }
    Q_INVOKABLE inline QDoubleValidator *moneyVal()    { return _moneyVal;     }
    Q_INVOKABLE inline QDoubleValidator *negMoneyVal() { return _negMoneyVal;  }
    Q_INVOKABLE inline QDoubleValidator *priceVal()    { return _priceVal;     }
    Q_INVOKABLE inline QDoubleValidator *costVal()     { return _costVal;      }
    Q_INVOKABLE inline QDoubleValidator *ratioVal()    { return _ratioVal;     }
    Q_INVOKABLE inline QDoubleValidator *weightVal()   { return _weightVal;    }
    Q_INVOKABLE inline QDoubleValidator *runTimeVal()  { return _runTimeVal;   }
    Q_INVOKABLE inline QIntValidator *orderVal()       { return _orderVal;     }
    Q_INVOKABLE inline QIntValidator *dayVal()         { return _dayVal;       }

    Q_INVOKABLE inline QFont systemFont()              { return *_systemFont;  }
    Q_INVOKABLE inline QFont fixedFont()               { return *_fixedFont;   }
    Q_INVOKABLE GUIClient::WindowSystem getWindowSystem();

    Q_INVOKABLE bool singleCurrency();
    Q_INVOKABLE bool showTopLevel() const { return _showTopLevel; }
    Q_INVOKABLE QWidgetList windowList();
    Q_INVOKABLE void populateCustomMenu(QMenu*, const QString &);

    Q_INVOKABLE void handleNewWindow(QWidget *, Qt::WindowModality = Qt::NonModal, bool forceFloat = false);
    Q_INVOKABLE QMenuBar *menuBar();

    // Used by scripting
    Q_INVOKABLE void addDockWidget ( Qt::DockWidgetArea area, QDockWidget * dockwidget );
    Q_INVOKABLE void addToolBar ( QToolBar * toolbar );
    Q_INVOKABLE void addToolBar ( Qt::ToolBarArea area, QToolBar * toolbar );
    Q_INVOKABLE void addToolBarBreak ( Qt::ToolBarArea area = Qt::TopToolBarArea );
    Q_INVOKABLE void tabifyDockWidget ( QDockWidget * first, QDockWidget * second );
    Q_INVOKABLE void setCentralWidget(QWidget * widget);

	TimeoutHandler   *_timeoutHandler;
    ReportHandler    *_reportHandler;

    QMap<const QObject*,int> _customCommands;

    QString _key;
    Q_INVOKABLE QString key() { return _key; }

    QString _singleWindow;

    Q_INVOKABLE        void  launchBrowser(QWidget*, const QString &);
    Q_INVOKABLE     QWidget *myActiveWindow();
    Q_INVOKABLE inline bool  shuttingDown() { return _shuttingDown; }

    void loadScriptGlobals(QScriptEngine * engine);

    #ifdef Q_OS_MAC
    		void updateMacDockMenu(QWidget *w);
    		void removeFromMacDockMenu(QWidget *w);
    	#endif

    //check hunspell is ready
    Q_INVOKABLE bool hunspell_ready();
    //spellcheck word, returns 1 if word ok otherwise 0
    Q_INVOKABLE int hunspell_check(const QString word);
    //suggest words for word, returns number of words in slst
    Q_INVOKABLE const QStringList hunspell_suggest(const QString word);
    //add word to dict (word is valid until spell object is not destroyed)
    Q_INVOKABLE int hunspell_add(const QString word);
    //add word to dict (word is valid until spell object is not destroyed)
    Q_INVOKABLE int hunspell_ignore(const QString word);

  public slots:
    void sReportError(const QString &);
    void sTick();

    void sAssortmentsUpdated(int, bool);
    void sBBOMsUpdated(int, bool);
    void sBOMsUpdated(int, bool);
    void sBOOsUpdated(int, bool);
    void sBankAccountsUpdated();
    void sBankAdjustmentsUpdated(int, bool);
    void sBillingSelectionUpdated(int, int);
    void sBudgetsUpdated(int, bool);
    void sCashReceiptsUpdated(int, bool);
    void sChecksUpdated(int, int, bool);
    void sConfigureGLUpdated();
    void sContractsUpdated(int, bool);
    void sCreditMemosUpdated();
    void sCrmAccountsUpdated(int);
    void sCustomCommand();
    void sCustomersUpdated(int, bool);
    void sEmitNotifyHeard(const QString &note);
    void sEmitSignal(QString, QString);
    void sEmitSignal(QString, int);
    void sEmitSignal(QString, bool);
    void sEmployeeUpdated(int);
    void sGlSeriesUpdated();
    void initMenuBar();
    void sInvoicesUpdated(int, bool);
    void sItemGroupsUpdated(int, bool);
    void sItemsUpdated(int, bool);
    void sItemsitesUpdated();
    void sPaymentsUpdated(int, int, bool);
    void sProjectsUpdated(int);
    void sProspectsUpdated();
    void sPurchaseOrderReceiptsUpdated();
    void sPurchaseOrdersUpdated(int, bool);
    void sPurchaseRequestsUpdated();
    void sQOHChanged(int, bool);
    void sQuotesUpdated(int);
    void sReportsChanged(int, bool);
    void sReturnAuthorizationsUpdated();
    void sSalesOrdersUpdated(int);
    void sSalesRepUpdated(int);
    void sStandardPeriodsUpdated();
    void sSystemMessageAdded();
    void sTaxAuthsUpdated(int);
    void sTransferOrdersUpdated(int);
    void sUserUpdated(QString);
    void sVendorsUpdated();
    void sVouchersUpdated();
    void sWarehousesUpdated();
    void sWorkCentersUpdated();
    void sWorkOrderMaterialsUpdated(int, int, bool);
    void sWorkOrderOperationsUpdated(int, int, bool);
    void sWorkOrdersUpdated(int, bool);

    void sIdleTimeout();

    void sFocusChanged(QWidget* old, QWidget* now);

    void sClearErrorMessages();
    void sNewErrorMessage();
    void setWindowTitle();

  signals:
    void tick();

    void messageNotify();

    /** @name Data Update Signals
     
       Application windows can connect to the GUIClient to listen for specific
       data update events. There is a signal for each corresponding slot.

       For example, the opportunity window listens for updates to sales
       orders to keep its list current:
       @code{.cpp}
       connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillSalesList()));
       @endcode

       @{
     */
    void assortmentsUpdated(int pItemid, bool pLocal);
    void bankAccountsUpdated();
    void bankAdjustmentsUpdated(int pBankadjid, bool pLocal);
    void bbomsUpdated(int pItemid, bool pLocal);
    void billingSelectionUpdated(int pCoheadid, int pCoitemid);
    void bomsUpdated(int pItemid, bool pLocal);
    void boosUpdated(int pItemid, bool pLocal);
    void budgetsUpdated(int pItemid, bool pLocal);
    void cashReceiptsUpdated(int pCashrcptid, bool pLocal);
    void checksUpdated(int pBankaccntid, int pCheckid, bool pLocal);
    void configureGLUpdated();
    void contractsUpdated(int pContrctid, bool pLocal);
    void creditMemosUpdated();
    void crmAccountsUpdated(int crmacctid);
    void customersUpdated(int pCustid, bool pLocal);
    void employeeUpdated(int id);
    void glSeriesUpdated();
    void invoicesUpdated(int pInvcheadid, bool pLocal);
    void itemGroupsUpdated(int pItemgrpid, bool pLocal);
    void itemsUpdated(int pItemid, bool pLocal);
    void itemsitesUpdated();
    void paymentsUpdated(int pBankaccntid, int pApselectid, bool pLocal);
    void projectsUpdated(int prjid);
    void prospectsUpdated();
    void purchaseOrderReceiptsUpdated();
    void purchaseOrdersUpdated(int pPoheadid, bool pLocal);
    void purchaseRequestsUpdated();
    void qohChanged(int pItemsiteid, bool pLocal);
    void quotesUpdated(int pQuheadid, bool pLocal);
    void reportsChanged(int pReportid, bool pLocal);
    void returnAuthorizationsUpdated();
    void salesOrdersUpdated(int pSoheadid, bool pLocal);
    void salesRepUpdated(int id);
    void standardPeriodsUpdated();
    void systemMessageAdded();
    void taxAuthsUpdated(int taxauthid);
    void transferOrdersUpdated(int id);
    void userUpdated(QString username);
    void vendorsUpdated();
    void vouchersUpdated();
    void warehousesUpdated();
    void workCentersUpdated();
    void workOrderMaterialsUpdated(int pWoid, int pWomatlid, bool pLocal);
    void workOrderOperationsUpdated(int pWoid, int pWooperid, bool pLocal);
    void workOrdersUpdated(int pWoid, bool pLocal);

    /** @} */

    void emitSignal(QString, QString);
    void emitSignal(QString, int);
    void emitSignal(QString, bool);

  protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);

    void addDocumentWatch(QString path, int id);
    bool removeDocumentWatch(QString path);

  protected slots:
    void windowDestroyed(QObject*);

  private slots:
    void handleDocument(QString path);
    void hunspell_initialize();
    void hunspell_uninitialize();

  private:
    QMdiArea   *_workspace;
    QTimer       _tick;
    QPushButton  *_eventButton;
    QPushButton  *_registerButton;
    QPushButton  *_errorButton;
    QString      _databaseURL;
    QString      _username;
    bool         _showTopLevel;
    QWidgetList  _windowList;
    QMenuBar	*_menuBar;
    QWidget     *_activeWindow;

    InputManager   *_inputManager;

    menuProducts    *productsMenu;
    menuInventory   *inventoryMenu;
    menuSchedule    *scheduleMenu;
    menuPurchase    *purchaseMenu;
    menuManufacture *manufactureMenu;
    menuCRM         *crmMenu;
    menuSales       *salesMenu;
    menuAccounting  *accountingMenu;
    menuWindow      *windowMenu;
    menuSystem      *systemMenu;

    QDate _startOfTime;
    QDate _endOfTime;
    QDate _dbDate;

    QDoubleValidator *_qtyVal;
    QDoubleValidator *_transQtyVal;
    QDoubleValidator *_qtyPerVal;
    QDoubleValidator *_scrapVal;
    QDoubleValidator *_percentVal;
    QDoubleValidator *_negPercentVal;
    QDoubleValidator *_moneyVal;
    QDoubleValidator *_negMoneyVal;
    QDoubleValidator *_priceVal;
    QDoubleValidator *_costVal;
    QDoubleValidator *_ratioVal;
    QDoubleValidator *_weightVal;
    QDoubleValidator *_runTimeVal;
    QIntValidator    *_orderVal;
    QIntValidator    *_dayVal;

    QFont *_systemFont;
    QFont *_fixedFont;

    bool _shown;
    bool _shuttingDown;

    QFileSystemWatcher* _fileWatcher;
    QMap<QString, int> _fileMap;
    QTextCodec * _spellCodec;
    Hunspell * _spellChecker;
    bool _spellReady;
    QStringList _spellAddWords;

    QMenu *_menu;
};
extern GUIClient *omfgThis;

#endif

