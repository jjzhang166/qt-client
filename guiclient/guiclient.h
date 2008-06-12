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

//  guiclient.h
//  Created 03/22/2003 JSL
//  Copyright (c) 2003-2008, OpenMFG, LLC
//
#ifndef GUICLIENT_H
#define GUICLIENT_H

#include <QDateTime>
#include <QMainWindow>
#include <QTimer>
#include <QAction>
#include <QCloseEvent>
#include <Q3ValueList>
#include <QPixmap>
#include <QMenu>
#include <QMenuBar>

#include <xsqlquery.h>

#include "../common/format.h"

class QSplashScreen;
class QWorkspace;
class QPushButton;
class QIntValidator;
class QDoubleValidator;
class QCheckBox;
class QAssistantClient;
class QScriptEngine;

class menuProducts;
class menuInventory;
class menuSchedule;
class menuPurchase;
class menuManufacture;
class menuCRM;
class menuSales;
class menuAccounting;
class menuSystem;

class TimeoutHandler;
class InputManager;
class ReportHandler;

class XMainWindow;
class QMainWindow;

#define q omfgThis->_q

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

//  Possible Journal Types
#define SalesJournal          0x01
#define CreditMemoJournal     0x02
#define CashReceiptsJournal   0x03
#define PayablesJournal       0x04
#define CheckJournal          0x05

// Possible return values from submitReport
#define cNoReportDefinition   -2

int  systemError(QWidget *, const QString &);
int  systemError(QWidget *, const QString &, const QString &, const int);
void message(const QString &, int = 0);
void resetMessage();
void audioAccept();
void audioReject();

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


class ActionSet;

class Action : public QAction
{
  friend class ActionSet;

  public:
    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool );

    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool,
            const QPixmap &, QWidget *);  
            
    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool,
            const QPixmap &, QWidget *,
            const QString &); 

    inline const QString name()        { return _name;        };
    inline const QString displayName() { return _displayName; };
    inline const QString toolTip() { return _toolTip; };

  protected:
    QString _name;
    QString _displayName;
    QString _toolTip;
};

class ActionSet : public Q3ValueList<Action *>
{
};

class GUIClient : public QMainWindow
{
  friend class XMainWindow;
  friend class XDialog;

  Q_OBJECT
  
  public:
    GUIClient(const QString &, const QString &);

    void setCaption();
    void initMenuBar();
    void saveToolbarPositions();

    inline QWorkspace *workspace()         { return _workspace;    }
    inline InputManager *inputManager()    { return _inputManager; }
    inline QString databaseURL()           { return _databaseURL;  }
    inline QString username()              { return _username;     }

    inline const QDate &startOfTime()      { return _startOfTime;  }
    inline const QDate &endOfTime()        { return _endOfTime;    }
    inline const QDate &dbDate()           { return _dbDate;       }

    inline QDoubleValidator *qtyVal()      { return _qtyVal;       }
    inline QDoubleValidator *transQtyVal() { return _transQtyVal;  }
    inline QDoubleValidator *qtyPerVal()   { return _qtyPerVal;    }
    inline QDoubleValidator *scrapVal()    { return _scrapVal;     } 
    inline QDoubleValidator *percentVal()  { return _percentVal;   }
    inline QDoubleValidator *moneyVal()    { return _moneyVal;     }
    inline QDoubleValidator *negMoneyVal() { return _negMoneyVal;  }
    inline QDoubleValidator *priceVal()    { return _priceVal;     }
    inline QDoubleValidator *costVal()     { return _costVal;      }
    inline QDoubleValidator *ratioVal()    { return _ratioVal;     }
    inline QDoubleValidator *weightVal()   { return _weightVal;    }
    inline QDoubleValidator *runTimeVal()  { return _runTimeVal;   }
    inline QIntValidator *orderVal()       { return _orderVal;     }
    inline QIntValidator *dayVal()         { return _dayVal;       }

    inline QFont systemFont()              { return *_systemFont;  }
    inline QFont fixedFont()               { return *_fixedFont;   }

    bool singleCurrency();
    bool showTopLevel() const { return _showTopLevel; }
    QWidgetList windowList();
    void populateCustomMenu(QMenu*, const QString &);

    void handleNewWindow(QWidget *, Qt::WindowModality = Qt::NonModal);
    QMenuBar *menuBar();

    XSqlQuery        _q;
    ActionSet        actions;
    XSqlQuery        __item;
    int              __itemListSerial;
    XSqlQuery        __cust;
    int              __custListSerial;
    TimeoutHandler   *_timeoutHandler;
    ReportHandler    *_reportHandler;
    QAssistantClient *_assClient;

    QMap<const QObject*,int> _customCommands;

    QString _key;
    QString _singleWindow;

    void launchBrowser(QWidget*, const QString &);
    QWidget * myActiveWindow();

    void loadScriptGlobals(QScriptEngine * engine);

  public slots:
    void sReportError(const QString &);
    void sTick();
    void sChecksUpdated(int, int, bool);
    void sAssortmentsUpdated(int, bool);
    void sBBOMsUpdated(int, bool);
    void sBOMsUpdated(int, bool);
    void sBOOsUpdated(int, bool);
    void sBudgetsUpdated(int, bool);
    void sBankAdjustmentsUpdated(int, bool);
    void sBillingSelectionUpdated(int, int);
    void sCashReceiptsUpdated(int, bool);
    void sConfigureGLUpdated();
    void sCreditMemosUpdated();
    void sCrmAccountsUpdated(int);
    void sCustomCommand();
    void sCustomersUpdated(int, bool);
    void sGlSeriesUpdated();
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
    void sStandardPeriodsUpdated();
    void sTaxAuthsUpdated(int);
    void sTransferOrdersUpdated(int);
    void sVendorsUpdated();
    void sVouchersUpdated();
    void sWarehousesUpdated();
    void sWorkCentersUpdated();
    void sWorkOrderMaterialsUpdated(int, int, bool);
    void sWorkOrderOperationsUpdated(int, int, bool);
    void sWorkOrdersUpdated(int, bool);

    void sIdleTimeout();

    void sFocusChanged(QWidget* old, QWidget* now);

  signals:
    void tick();

    void checksUpdated(int, int, bool);
    void assortmentsUpdated(int, bool);
    void bankAdjustmentsUpdated(int, bool);
    void bbomsUpdated(int, bool);
    void billingSelectionUpdated(int, int);
    void bomsUpdated(int, bool);
    void boosUpdated(int, bool);
    void budgetsUpdated(int, bool);
    void cashReceiptsUpdated(int, bool);
    void configureGLUpdated();
    void creditMemosUpdated();
    void crmAccountsUpdated(int);
    void customersUpdated(int, bool);
    void glSeriesUpdated();
    void invoicesUpdated(int, bool);
    void itemGroupsUpdated(int, bool);
    void itemsUpdated(int, bool);
    void itemsitesUpdated();
    void paymentsUpdated(int, int, bool);
    void projectsUpdated(int);
    void prospectsUpdated();
    void purchaseOrderReceiptsUpdated();
    void purchaseOrdersUpdated(int, bool);
    void purchaseRequestsUpdated();
    void qohChanged(int, bool);
    void quotesUpdated(int, bool);
    void reportsChanged(int, bool);
    void returnAuthorizationsUpdated();
    void salesOrdersUpdated(int, bool);
    void standardPeriodsUpdated();
    void taxAuthsUpdated(int);
    void transferOrdersUpdated(int);
    void vendorsUpdated();
    void vouchersUpdated();
    void warehousesUpdated();
    void workCentersUpdated();
    void workOrderMaterialsUpdated(int, int, bool);
    void workOrderOperationsUpdated(int, int, bool);
    void workOrdersUpdated(int, bool);

  protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);

  protected slots:
    void windowDestroyed(QObject*);

  private:
    QWorkspace   *_workspace;
    QTimer       _tick;
    QPushButton  *_eventButton;
    QPushButton  *_registerButton;
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
    menuSystem      *systemMenu;

    QDate _startOfTime;
    QDate _endOfTime;
    QDate _dbDate;

    QDoubleValidator *_qtyVal;
    QDoubleValidator *_transQtyVal;
    QDoubleValidator *_qtyPerVal;
    QDoubleValidator *_scrapVal;
    QDoubleValidator *_percentVal;
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
};
extern GUIClient *omfgThis;

#endif

