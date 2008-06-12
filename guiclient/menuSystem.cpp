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

#include <QAction>
#include <QApplication>
#include <QAssistantClient>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>
#include <QPixmap>
#include <QWorkspace>
#include <QSettings>

#include "guiclient.h"

#include <parameter.h>
#include <openreports.h>
#include <../common/batchManager.h>

#include "version.h"

#include "menuSystem.h"

#include "systemMessage.h"
#include "eventManager.h"
#include "users.h"
#include "submitAction.h"
#include "userPreferences.h"
#include "errorLog.h"

#include "databaseInformation.h"
#include "configureBackup.h"
#include "configureAccountingSystem.h"

#include "accountNumbers.h"
#include "calendars.h"
#include "commentTypes.h"
#include "countries.h"
#include "currencies.h"
#include "currencyConversions.h"
#include "customCommands.h"
#include "departments.h"
#include "ediProfiles.h"
#include "employee.h"
#include "employees.h"
#include "empGroup.h"
#include "empGroups.h"
#include "searchForEmp.h"
#include "forms.h"
#include "groups.h"
#include "images.h"
#include "labelForms.h"
#include "locales.h"
#include "reports.h"
#include "scripts.h"
#include "shifts.h"
#include "uiforms.h"

#include "fixSerial.h"
#include "importXML.h"

#include "configureIE.h"
#include "configureIM.h"
#include "configurePD.h"
#include "configureMS.h"
#include "configureWO.h"
#include "configureSO.h"
#include "configureSR.h"
#include "configurePO.h"
#include "configureAP.h"
#include "configureAR.h"
#include "configureGL.h"
#include "configureCC.h"
#include "configurePM.h"
#include "configureCRM.h"

#include "registration.h"

extern QString __path;

menuSystem::menuSystem(GUIClient *Pparent) :
 QObject(Pparent, "sysModule")
{
  QString _appname = _metrics->value("Application");
  parent = Pparent;

  errorLogListener::initialize();

  cascade = tile = closeActive = closeAll = _rememberPos = _rememberSize = 0;
  _lastActive = 0;
  geometryMenu = 0;

  systemMenu		= new QMenu(parent);
  configModulesMenu	= new QMenu(parent);
  masterInfoMenu	= new QMenu(parent);
  sysUtilsMenu		= new QMenu(parent);
  windowMenu		= new QMenu(parent);
  helpMenu		= new QMenu(parent);
  designMenu            = new QMenu(parent);
  employeeMenu          = new QMenu(parent);

  systemMenu->setObjectName("menu.sys");
  configModulesMenu->setObjectName("menu.sys.configmodules");
  masterInfoMenu->setObjectName("menu.sys.masterinfo");
  sysUtilsMenu->setObjectName("menu.sys.utilities");
  windowMenu->setObjectName("menu.window");
  helpMenu->setObjectName("menu.help");
  designMenu->setObjectName("menu.sys.design");

//  Window
  windowMenu->setCheckable(TRUE);

  cascade = new Action( parent, "window.cascade", tr("&Cascade"), parent->workspace(), SLOT(cascade()), windowMenu, true);
  parent->actions.append( cascade );

  tile = new Action( parent, "window.tile", tr("&Tile"), parent->workspace(), SLOT(tile()), windowMenu, true);
  parent->actions.append( tile );

  closeActive = new Action( parent, "window.closeActiveWindow", tr("Close &Active Window"), this, SLOT(sCloseActive()), windowMenu, true);
  parent->actions.append( closeActive );

  closeAll = new Action( parent, "window.closeAllWindows", tr("Close A&ll Windows"), this, SLOT(sCloseAll()), windowMenu, true);
  parent->actions.append( closeAll );

  _rememberPos = new Action( parent, "window.rememberPositionToggle", tr("Remember Position"), this, SLOT(sRememberPositionToggle()), windowMenu, true);
  _rememberPos->setCheckable(true);

  _rememberSize = new Action( parent, "window.rememberSizeToggle", tr("Remember Size"), this, SLOT(sRememberSizeToggle()), windowMenu, true);
  _rememberSize->setCheckable(true);

  parent->menuBar()->insertItem(tr("&Window"),  windowMenu );
  connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(sPrepareWindowMenu()));
  connect(windowMenu, SIGNAL(aboutToHide()), this, SLOT(sHideWindowMenu()));
  

  actionProperties acts[] = {

    { "sys.scheduleSystemMessage",    tr("Schedule S&ystem Message..."),    SLOT(sScheduleSystemMessage()),    systemMenu, _privileges->check("IssueSystemMessages"), NULL, NULL, _metrics->boolean("EnableBatchManager") },
    { "separator",                    NULL,                                 NULL,                              systemMenu, true,                                      NULL, NULL, _metrics->boolean("EnableBatchManager") },
    { "sys.eventManager",             tr("E&vent Manager..."),              SLOT(sEventManager()),             systemMenu, TRUE,                                      NULL, NULL, true },
    { "sys.batchManager",             tr("&Batch Manager..."),              SLOT(sBatchManager()),             systemMenu, TRUE,                                      NULL, NULL, _metrics->boolean("EnableBatchManager") },
    { "sys.viewDatabaseLog",          tr("View Database &Log..."),          SLOT(sErrorLog()),                 systemMenu, TRUE,                                      NULL, NULL, true },
    { "separator",                    NULL,                                 NULL,                              systemMenu, true,                                      NULL, NULL, true },
    { "sys.preferences",              tr("P&references..."),                SLOT(sPreferences()),              systemMenu, (_privileges->check("MaintainPreferencesSelf") || _privileges->check("MaintainPreferencesOthers")),  NULL,   NULL,   true },
    { "sys.rescanPrivileges",         tr("Rescan &Privileges"),             SLOT(sRescanPrivileges()),         systemMenu, TRUE,                                      NULL, NULL, true },
    { "separator",                    NULL,                                 NULL,                              systemMenu, true,                                      NULL, NULL, true },
    { "sys.maintainUsers",            tr("Maintain &Users..."),             SLOT(sMaintainUsers()),            systemMenu, _privileges->check("MaintainUsers"),       NULL, NULL, true },
    { "sys.maintainGroups",           tr("Maintain &Groups..."),            SLOT(sMaintainGroups()),           systemMenu, _privileges->check("MaintainGroups"),      NULL, NULL, true },

    { "menu",                         tr("&Employees"),                     (char*)employeeMenu,               systemMenu, true,                                      NULL, NULL, true },
    { "sys.employee",                 tr("&New..."),               SLOT(sNewEmployee()),            employeeMenu, employee::userHasPriv(cNew),               NULL, NULL, true },
    { "sys.listEmployees",            tr("&List..."),             SLOT(sListEmployees()),          employeeMenu, employee::userHasPriv(),                   NULL, NULL, true },
    { "sys.searchEmployees",          tr("&Search..."),       SLOT(sSearchEmployees()),        employeeMenu, searchForEmp::userHasPriv(),               NULL, NULL, true },
    { "separator",                    NULL,                                 NULL,                            employeeMenu, true,                                      NULL, NULL, true },
    { "sys.employeeGroups",           tr("Employee &Groups..."),            SLOT(sEmployeeGroups()),         employeeMenu, empGroup::userHasPriv(),                   NULL, NULL, true },

    { "separator",                    NULL,                                 NULL,                              systemMenu, true,                                      NULL, NULL, true },
    { "sys.scheduleServerMaintenance",tr("Schedule Server Mai&ntenance..."),SLOT(sScheduleServerMaintenance()),systemMenu, _privileges->check("MaintainServer"),      NULL, NULL, _metrics->boolean("EnableBatchManager") },
    { "sys.scheduleServerBackup",     tr("Schedule Server Bac&kup..."),     SLOT(sScheduleServerBackup()),     systemMenu, _privileges->check("BackupServer"),        NULL, NULL, _metrics->boolean("EnableBatchManager") },
    { "separator",                    NULL,                                 NULL,                              systemMenu, true,                                      NULL, NULL, _metrics->boolean("EnableBatchManager") },

  //  System | Configure Modules
    { "menu",			tr("&Configure Modules"),(char*)configModulesMenu,systemMenu,		true,					NULL,	NULL,	true	},
    { "sys.configurePD",	tr("&Products..."),	SLOT(sConfigurePD()),	configModulesMenu,	_privileges->check("ConfigurePD"),	NULL,	NULL,	true	},
    { "sys.configureIM",	tr("&Inventory..."),	SLOT(sConfigureIM()),	configModulesMenu,	_privileges->check("ConfigureIM"),	NULL,	NULL,	true	},
    { "sys.configurePO",	tr("P&urchase..."),	SLOT(sConfigurePO()),	configModulesMenu,	_privileges->check("ConfigurePO"),	NULL,	NULL,	true	},
    { "sys.configureMS",	tr("Sch&edule..."),	SLOT(sConfigureMS()),	configModulesMenu,	_privileges->check("ConfigureMS"),	NULL,	NULL,	 (_metrics->value("Application") == "OpenMFG")	},
    { "sys.configureWO",	tr("&Manufacture..."),	SLOT(sConfigureWO()),	configModulesMenu,	_privileges->check("ConfigureWO"),	NULL,	NULL,	true	},
    { "sys.configureCRM",	tr("&CRM..."),	SLOT(sConfigureCRM()),	configModulesMenu,	_privileges->check("ConfigureCRM"),	NULL,	NULL,	true	},
    { "sys.configureSO",	tr("&Sales..."),	SLOT(sConfigureSO()),	configModulesMenu,	_privileges->check("ConfigureSO"),	NULL,	NULL,	true	},
    { "sys.configureGL",	tr("&Accounting..."),	SLOT(sConfigureGL()),	configModulesMenu,	_privileges->check("ConfigureGL"),	NULL,	NULL,	true	},

  //  Master Information
    { "menu",				tr("&Master Information"),	(char*)masterInfoMenu,		systemMenu,	true,						NULL,	NULL,	true	},
    { "sys.databaseInformation",	tr("&Database Information..."),	SLOT(sDatabaseInformation()),	masterInfoMenu,	_privileges->check("ConfigDatabaseInfo"),	NULL,	NULL,	true	},
    { "sys.configureBackup",		tr("Configure &Backup..."),	SLOT(sConfigureBackup()),	masterInfoMenu,	_privileges->check("ConfigureBackupServer"),	NULL,	NULL,	_metrics->boolean("EnableBatchManager")	},
    { "sys.configureAccountingSystemInterface",tr("Configure Accounting System Interface..."), SLOT(sConfigureAccountingSystemInterface()), masterInfoMenu, _privileges->check("ConfigAccountingInterface"),	NULL,	NULL, _metrics->boolean("EnableExternalAccountingInterface") },
    { "separator",		NULL,			NULL,			masterInfoMenu,	true,					NULL,	NULL,	true	},
    { "sys.images",		tr("&Images..."),	SLOT(sImages()),	masterInfoMenu,	_privileges->check("MaintainImages"),	NULL,	NULL,	true	},
    { "sys.forms",		tr("&Forms..."),	SLOT(sForms()),		masterInfoMenu,	_privileges->check("MaintainForms"),	NULL,	NULL,	true	},
    { "sys.labelForms",		tr("&Label Forms..."),	SLOT(sLabelForms()),	masterInfoMenu,	_privileges->check("MaintainForms"),	NULL,	NULL,	true	},
    { "sys.calendars",		tr("C&alendars..."),	SLOT(sCalendars()),	masterInfoMenu,	_privileges->check("MaintainCalendars"),	NULL,	NULL,	true	},
    { "sys.currencies",		tr("Curre&ncies..."),	SLOT(sCurrencies()),	masterInfoMenu,	_privileges->check("CreateNewCurrency"),	NULL,	NULL,	true	},
    { "sys.exchangeRates",	tr("&Exchange Rates..."),SLOT(sExchangeRates()),masterInfoMenu,	_privileges->check("MaintainCurrencyRates") || _privileges->check("ViewCurrencyRates"),
																	NULL,	NULL,	true	},
    { "sys.configureCC",	tr("&Credit Cards..."),	SLOT(sConfigureCC()),	masterInfoMenu,	_privileges->check("ConfigureCC"),	NULL,	NULL,	true	},
    { "sys.countries",		tr("Co&untries..."),	SLOT(sCountries()),	masterInfoMenu,	_privileges->check("MaintainCountries"),	NULL,	NULL,	true	},
    { "sys.locales",		tr("L&ocales..."),	SLOT(sLocales()),	masterInfoMenu,	_privileges->check("MaintainLocales"),	NULL,	NULL,	true	},
    { "sys.commentTypes",	tr("Comment &Types..."),SLOT(sCommentTypes()),	masterInfoMenu,	_privileges->check("MaintainCommentTypes"), NULL, NULL,	true	},
    { "sys.ediProfiles",	tr("EDI &Profiles..."),	SLOT(sEDIProfiles()),	masterInfoMenu,	_privileges->check("MaintainEDIProfiles"), NULL,	NULL,	 _metrics->boolean("EnableBatchManager")	},
    { "sys.departments",	tr("Depart&ments..."),	SLOT(sDepartments()),	masterInfoMenu,	_privileges->check("ViewDepartments") || _privileges->check("MaintainDepartments"),	NULL,	NULL,	true	},
    { "sys.shifts",		tr("S&hifts..."),	SLOT(sShifts()),	masterInfoMenu,	(_privileges->check("ViewShifts") || _privileges->check("MaintainShifts")) ,	NULL,	NULL, _metrics->boolean("Routings")	},
    { "sys.configureIE",	tr("Configure Data Import and E&xport..."),	SLOT(sConfigureIE()),	 masterInfoMenu, configureIE::userHasPriv(),	NULL, NULL, true },

  //  Design
    { "menu",				tr("&Design"),	(char*)designMenu,		systemMenu,	true,						NULL,	NULL,	true	},
    { "sys.reports",		tr("&Reports..."),	SLOT(sReports()),	designMenu,	_privileges->check("MaintainReports"),	NULL,	NULL,	true	},
    { "separator",		NULL,			NULL,			designMenu,	true,					NULL,	NULL,	true	},
    { "sys.uiforms",    	tr("Screens..."),         SLOT(sUIForms()),       designMenu, _privileges->check("MaintainScreens"),	NULL, NULL, true },
    { "sys.scripts",    	tr("Scripts..."),	SLOT(sScripts()),       designMenu, _privileges->check("MaintainScripts"),	NULL, NULL, true },
    { "separator",		NULL,			NULL,			designMenu,	true,					NULL,	NULL,	true	},
    { "sys.customCommands",	tr("Custom Command&s..."),	SLOT(sCustomCommands()), designMenu, _privileges->check("MaintainCustomCommands"),	NULL, NULL, true },

  // Utilities
    { "menu",			tr("&System Utilities"),	(char*)sysUtilsMenu,	systemMenu,	true, NULL, NULL, true	},
    { "sys.fixSerial",		tr("&Serial Columns"),	SLOT(sFixSerial()),	sysUtilsMenu,	_privileges->check("FixSerial"), NULL, NULL, true	},
    { "sys.importXML",		tr("&Import XML"),	SLOT(sImportXML()),	sysUtilsMenu,	importXML::userHasPriv(),	NULL, NULL, true	},

    { "separator",		NULL,				NULL,				systemMenu,	true,	NULL,	NULL,	true	},
    { "sys.printAlignmentPage",	tr("Print &Alignment Page..."),	SLOT(sPrintAlignment()),	systemMenu,	TRUE,	NULL,	NULL,	true	},
    { "separator",		NULL,				NULL,				systemMenu,	true,	NULL,	NULL,	true	},
    { "sys.exitOpenMFG",	tr("E&xit " + _appname + "..."), SLOT(sExit()),			systemMenu,	TRUE,	NULL,	NULL,	true	},

  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));
  parent->populateCustomMenu(systemMenu, "System");
  parent->menuBar()->insertItem(tr("S&ystem"), systemMenu);

  // Community
  communityMenu = new QMenu();
  actionProperties community[] = {
    { "community.home",        tr("xTuple.org &Home"),          SLOT(sCommunityHome()),        communityMenu, true, NULL, NULL, true },
    { "separator",	       NULL,				NULL,		communityMenu, true,	NULL, NULL, true	},
    { "community.register",    tr("&Register"),      SLOT(sRegister()),             communityMenu, true, new QPixmap(":images/dspRegister.png"), NULL, _metrics->value("Application") != "OpenMFG" },
    { "community.newAccount",  tr("&New Account"),   SLOT(sCommunityNewAccount()),  communityMenu, true, NULL, NULL, true },
    { "community.editAccount", tr("&Edit Account"),  SLOT(sCommunityEditAccount()), communityMenu, true, NULL, NULL, true },
    { "separator",	       NULL,				NULL,		communityMenu, true,	NULL, NULL, true	},
    { "community.forums",      tr("Discussion &Forums"),  SLOT(sCommunityForums()),      communityMenu, true, NULL, NULL, true },
    { "community.blogs",       tr("Bl&ogs"),              SLOT(sCommunityBlogs()),       communityMenu, true, NULL, NULL, true },
    { "community.issues",      tr("&Bugs and Feature Requests"), SLOT(sCommunityIssues()),      communityMenu, true, NULL, NULL, true },
    { "separator",	       NULL,				NULL,		communityMenu, true,	NULL, NULL, true	},
    { "community.downloads",   tr("&Downloads"),     SLOT(sCommunityDownloads()),   communityMenu, true, NULL, NULL, true },
  };
  addActionsToMenu(community, sizeof(community) / sizeof(community[0]));
  parent->menuBar()->insertItem(tr("C&ommunity"), communityMenu);

  //  Help
  helpMenu = new QMenu();
  actionProperties help[] = {
    { "help.about",		tr("&About..."),			SLOT(sAbout()),	helpMenu, true,	NULL, NULL, true	},
#ifndef Q_WS_MACX
    { "separator",		NULL,				NULL,		helpMenu, true,	NULL, NULL, true	},
#endif
    { "help.tableOfContents",	tr("Table of &Contents..."),	SLOT(sTOC()),	helpMenu, true, NULL, NULL, true	}
  };
  addActionsToMenu(help, sizeof(help) / sizeof(help[0]));

  parent->menuBar()->insertItem(tr("&Help"),   helpMenu   );
}

void menuSystem::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  for (unsigned int i = 0; i < numElems; i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == QString("menu"))
    {
      acts[i].menu->insertItem(acts[i].actionTitle, (QMenu*)(acts[i].slot));
    }
    else if (acts[i].actionName == QString("separator"))
    {
      acts[i].menu->addSeparator();
    }
    else if (acts[i].toolBar != NULL)
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv,
					  *(acts[i].pixmap),
					  acts[i].toolBar) );
    }
    else
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv ) );
    }
  }
}

void menuSystem::sPrepareWindowMenu()
{
  windowMenu->clear();

  if(!omfgThis->showTopLevel())
  {
    cascade->addTo(windowMenu);
    tile->addTo(windowMenu);
  }
  closeActive->addTo(windowMenu);
  closeAll->addTo(windowMenu);

  QWidgetList windows = omfgThis->windowList();

  bool b = !windows.isEmpty();
  cascade->setEnabled(b);
  tile->setEnabled(b);
  closeActive->setEnabled(b);
  closeAll->setEnabled(b);

  windowMenu->insertSeparator();

  QWidget * activeWindow = parent->workspace()->activeWindow();
  if(omfgThis->showTopLevel())
  {
    //activeWindow = qApp->activeWindow();
    activeWindow = omfgThis->myActiveWindow();
  }
  _lastActive = activeWindow;

  if(activeWindow)
  {
    if(!geometryMenu)
      geometryMenu = new QMenu();

    geometryMenu->clear();
    geometryMenu->setTitle(activeWindow->caption());

    QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
    QString objName = activeWindow->objectName();
    
    _rememberPos->setChecked(settings.value(objName + "/geometry/rememberPos", true).toBool());
    _rememberPos->addTo(geometryMenu);
    _rememberSize->setChecked(settings.value(objName + "/geometry/rememberSize", true).toBool());
    _rememberSize->addTo(geometryMenu);

    windowMenu->addMenu(geometryMenu);
    windowMenu->insertSeparator();
  }

  for (int cursor = 0; cursor < windows.count(); cursor++)
  {
    int menuId = windowMenu->insertItem(windows.at(cursor)->caption(), this, SLOT(sActivateWindow(int)));
    windowMenu->setItemParameter(menuId, cursor);
    windowMenu->setItemChecked(menuId, (activeWindow == windows.at(cursor)));
  }
}

void menuSystem::sHideWindowMenu()
{
  cascade->setEnabled(true);
  tile->setEnabled(true);
  closeActive->setEnabled(true);
  closeAll->setEnabled(true);
}

void menuSystem::sActivateWindow(int intWindowid)
{
  QWidgetList windows = parent->workspace()->windowList();
  if(omfgThis->showTopLevel())
    windows = omfgThis->windowList();
  QWidget *window = windows.at(intWindowid);
  if (window)
    if(omfgThis->showTopLevel())
      window->activateWindow();
    else
      window->setFocus();
}

void menuSystem::sRememberPositionToggle()
{
  if(!_lastActive)
    return;

  QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
  QString objName = _lastActive->objectName();
  settings.setValue(objName + "/geometry/rememberPos", _rememberPos->isChecked());
}

void menuSystem::sRememberSizeToggle()
{
  if(!_lastActive)
    return;

  QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
  QString objName = _lastActive->objectName();
  settings.setValue(objName + "/geometry/rememberSize", _rememberSize->isChecked());
}

void menuSystem::sCloseAll()
{
  if(omfgThis->showTopLevel())
  {
    foreach(QWidget * w, omfgThis->windowList())
    {
      w->close();
    }
  }
  else
    parent->workspace()->closeAllWindows();
}

void menuSystem::sCloseActive()
{
  if(omfgThis->showTopLevel())
  {
    if(omfgThis->windowList().contains(qApp->activeWindow()))
      qApp->activeWindow()->close();
  }
  else
    parent->workspace()->closeActiveWindow();
}

void menuSystem::sScheduleSystemMessage()
{
  ParameterList params;
  params.append("mode", "new");

  systemMessage newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuSystem::sEventManager()
{
  omfgThis->handleNewWindow(new eventManager());
}

void menuSystem::sBatchManager()
{
  batchManager *newdlg = new batchManager();
  newdlg->setViewOtherEvents(_privileges->check("ViewOtherEvents"));
  omfgThis->handleNewWindow(newdlg);
}

void menuSystem::sPreferences()
{
  userPreferences(parent, "", TRUE).exec();
}

void menuSystem::sRescanPrivileges()
{
  _privileges->load();
  omfgThis->saveToolbarPositions();
  _preferences->load();
  omfgThis->initMenuBar();
}

void menuSystem::sDatabaseInformation()
{
  databaseInformation(parent, "", TRUE).exec();
}

void menuSystem::sConfigureBackup()
{
  configureBackup(parent, "", TRUE).exec();
}

void menuSystem::sImages()
{
  omfgThis->handleNewWindow(new images());
}

void menuSystem::sReports()
{
  omfgThis->handleNewWindow(new reports());
}

void menuSystem::sForms()
{
  omfgThis->handleNewWindow(new forms());
}

void menuSystem::sLabelForms()
{
  omfgThis->handleNewWindow(new labelForms());
}

void menuSystem::sCalendars()
{
  omfgThis->handleNewWindow(new calendars());
}

void menuSystem::sCurrencies()
{
  omfgThis->handleNewWindow(new currencies());
}

void menuSystem::sExchangeRates()
{
  omfgThis->handleNewWindow(new currencyConversions());
}

void menuSystem::sCountries()
{
  omfgThis->handleNewWindow(new countries());
}

void menuSystem::sLocales()
{
  omfgThis->handleNewWindow(new locales());
}

void menuSystem::sCommentTypes()
{
  omfgThis->handleNewWindow(new commentTypes());
}

void menuSystem::sAccountNumbers()
{
  omfgThis->handleNewWindow(new accountNumbers());
}

void menuSystem::sEDIProfiles()
{
  omfgThis->handleNewWindow(new ediProfiles());
}

void menuSystem::sDepartments()
{
  omfgThis->handleNewWindow(new departments());
}

void menuSystem::sShifts()
{
  omfgThis->handleNewWindow(new shifts());
}

void menuSystem::sConfigureIE()
{
  configureIE(parent, "", TRUE).exec();
}

void menuSystem::sCustomCommands()
{
  omfgThis->handleNewWindow(new customCommands());
}

void menuSystem::sScripts()
{
  omfgThis->handleNewWindow(new scripts());
}

void menuSystem::sUIForms()
{
  omfgThis->handleNewWindow(new uiforms());
}

void menuSystem::sConfigureIM()
{
  configureIM(parent, "", TRUE).exec();
}

void menuSystem::sConfigurePD()
{
  configurePD(parent, "", TRUE).exec();
}

void menuSystem::sConfigureMS()
{
  configureMS(parent, "", TRUE).exec();
}

void menuSystem::sConfigureWO()
{
  configureWO(parent, "", TRUE).exec();
}

void menuSystem::sConfigurePO()
{
  configurePO(parent, "", TRUE).exec();
}

void menuSystem::sConfigureSO()
{
  configureSO(parent, "", TRUE).exec();
}

void menuSystem::sConfigureSR()
{
  configureSR(parent, "", TRUE).exec();
}

void menuSystem::sConfigureAP()
{
  configureAP(parent, "", TRUE).exec();
}

void menuSystem::sConfigureAR()
{
  configureAR(parent, "", TRUE).exec();
}

void menuSystem::sConfigureGL()
{
  configureGL(parent, "", TRUE).exec();
}

void menuSystem::sConfigureCC()
{
  configureCC(parent, "", TRUE).exec();
}

void menuSystem::sConfigurePM()
{
  configurePM(parent, "", TRUE).exec();
}

void menuSystem::sConfigureCRM()
{
  configureCRM(parent, "", TRUE).exec();
}

void menuSystem::sMaintainUsers()
{
  omfgThis->handleNewWindow(new users());
}

void menuSystem::sMaintainGroups()
{
  omfgThis->handleNewWindow(new groups());
}

void menuSystem::sNewEmployee()
{
  ParameterList params;
  employee newdlg(parent);
  params.append("mode", "new");
  newdlg.set(params);
  newdlg.exec();
}

void menuSystem::sListEmployees()
{
  omfgThis->handleNewWindow(new employees());
}

void menuSystem::sSearchEmployees()
{
  omfgThis->handleNewWindow(new searchForEmp());
}

void menuSystem::sEmployeeGroups()
{
  omfgThis->handleNewWindow(new empGroups());
}

void menuSystem::sScheduleServerMaintenance()
{
  ParameterList params;
  params.append("action_name", "ServerMaintenance");

  submitAction newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuSystem::sScheduleServerBackup()
{
  ParameterList params;
  params.append("action_name", "ServerBackup");

  submitAction newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuSystem::sErrorLog()
{
  omfgThis->handleNewWindow(new errorLog());
}

void menuSystem::sPrintAlignment()
{
  orReport report("Alignment");
  if (report.isValid())
    report.print();
  else
    report.reportError(omfgThis);
}

void menuSystem::sExit()
{
  parent->close();
}

void menuSystem::sAbout()
{
  QMessageBox::about( parent, tr("About..."),
                      tr( "%1\nVersion %2\n%3")
                          .arg(_Name)
                          .arg(_Version)
                          .arg(_Copyright) );
}

void menuSystem::sTOC()
{
  parent->_assClient->openAssistant();
}

// START_RW
void menuSystem::sConfigureAccountingSystemInterface()
{
  configureAccountingSystem(parent, "", TRUE).exec();
}
// END_RW

void menuSystem::sFixSerial()
{
  omfgThis->handleNewWindow(new fixSerial());
}

void menuSystem::sImportXML()
{
  omfgThis->handleNewWindow(new importXML());
}

void menuSystem::sCommunityHome()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/");
}

void menuSystem::sRegister()
{
  registration newdlg(parent);
  newdlg.exec();
}

void menuSystem::sCommunityNewAccount()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/index.php?option=com_registration&task=register");
}

void menuSystem::sCommunityEditAccount()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/index.php?option=com_user&task=UserDetails&Itemid=21");
}

void menuSystem::sCommunityForums()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/phpBB2/");
}

void menuSystem::sCommunityBlogs()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/index.php?option=com_content&task=blogsection&id=0&Itemid=9");
}

void menuSystem::sCommunityIssues()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/index.php?option=com_mantis&Itemid=26");
}

void menuSystem::sCommunityDownloads()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/index.php?option=com_docman&Itemid=50");
}

