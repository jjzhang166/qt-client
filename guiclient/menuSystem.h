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

//  menuSystem.h
//  Created 12/11/2000 JSL
//  Copyright (c) 2000-2008, OpenMFG, LLC

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
    void sConfigureSR();
    void sConfigureSO();
    void sConfigurePO();
    void sConfigureAP();
    void sConfigureAR();
    void sConfigureGL();
    void sConfigureCC();
    void sConfigurePM();
    void sConfigureCRM();

    void sScheduleSystemMessage();
    void sEventManager();
    void sBatchManager();
    void sPreferences();
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

    void sFixSerial();
    void sImportXML();

    void sPrintAlignment();

    void sExit();

    void sAbout();
    void sTOC();

    void sCommunityHome();
    void sCommunityNewAccount();
    void sCommunityEditAccount();
    void sCommunityForums();
    void sCommunityBlogs();
    void sCommunityIssues();
    void sCommunityDownloads();
    void sRegister();

// START_RW
    void sConfigureAccountingSystemInterface();
// END_RW

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
