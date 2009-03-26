/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuSystem_h
#define menuSystem_h

#include <QObject>

class QMenu;
class QPixmap;
class QToolBar;
class GUIClient;
class Action;

class menuSystem : public QObject
{
  Q_OBJECT

  struct actionProperties {
    const char*		actionName;
    const QString	actionTitle;
    const char*		slot;
    QMenu*		menu;
    bool		priv;
    QPixmap*		pixmap;
    QToolBar*		toolBar;
    bool		visible;
  };

  public:
    menuSystem(GUIClient *);

  public slots:
    void sPrepareWindowMenu();
    void sHideWindowMenu();
    void sActivateWindow(int);
    void sCloseAll();
    void sCloseActive();
    void sRememberPositionToggle();
    void sRememberSizeToggle();

    void sConfigureIE();
    void sConfigureIM();
    void sConfigurePD();
    void sConfigureMS();
    void sConfigureWO();
    void sConfigureSO();
    void sConfigurePO();
    void sConfigureGL();
    void sConfigureEncryption();
    void sConfigureCC();
    void sConfigureCRM();

    void sScheduleSystemMessage();
    void sEventManager();
    void sBatchManager();
    void sPreferences();
    void sHotKeys();
    void sRescanPrivileges();
    void sMaintainUsers();
    void sMaintainGroups();
    void sNewEmployee();
    void sListEmployees();
    void sSearchEmployees();
    void sEmployeeGroups();
    void sScheduleServerMaintenance();
    void sScheduleServerBackup();
    void sErrorLog();

    void sDatabaseInformation();
    void sConfigureBackup();

    void sImages();
    void sReports();
    void sForms();
    void sLabelForms();
    void sCalendars();
    void sCurrencies();
    void sExchangeRates();
    void sCountries();
    void sLocales();
    void sCommentTypes();
    void sAccountNumbers();
    void sEDIProfiles();
    void sDepartments();
    void sShifts();
    void sCustomCommands();
    void sScripts();
    void sUIForms();
    void sPackages();

    void sFixACL();
    void sFixSerial();
    void sImportXML();

    void sPrintAlignment();

    void sExit();

    void sAbout();
    void sTOC();

    void sCommunityHome();
    //void sCommunityNewAccount();
    void sCommunityEditAccount();
    void sCommunityForums();
    void sCommunityBlogs();
    void sCommunityIssues();
    void sCommunityWiki();
    void sCommunityDownloads();
    void sRegister();
    void sCommunitySupport();
    void sCommunityTranslation();

  private:
    GUIClient *parent;

    QToolBar *toolBar;

    QMenu *communityMenu;
    QMenu *configModulesMenu;
    QMenu *designMenu;
    QMenu *employeeMenu;
    QMenu *helpMenu;
    QMenu *masterInfoMenu;
    QMenu *sysUtilsMenu;
    QMenu *systemMenu;
    QMenu *windowMenu;

    QMenu *geometryMenu;

    Action *cascade;
    Action *tile;
    Action *closeActive;
    Action *closeAll;
    Action *_rememberPos;
    Action *_rememberSize;

    QWidget *_lastActive;

    void	addActionsToMenu(actionProperties [], unsigned int);

};

#endif
