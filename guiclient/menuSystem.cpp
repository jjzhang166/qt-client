/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QAction>
#include <QApplication>
#include <QLibraryInfo>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QPluginLoader>
#include <QToolBar>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QDesktopServices>
#include <QUrl>

#include "xtsettings.h"
#include "guiclient.h"
#include "helpDownload.h"

#include <parameter.h>
#include <openreports.h>

#include <csvimpplugininterface.h>

#include "importhelper.h"

#include "version.h"

#include "menuSystem.h"

#include "eventManager.h"
#include "users.h"
#include "dspUserPrivileges.h"
#include "userPreferences.h"
#include "hotkeys.h"
#include "errorLog.h"

#include "customCommands.h"
#include "employee.h"
#include "employees.h"
#include "empGroup.h"
#include "empGroups.h"
#include "searchForEmp.h"
#include "groups.h"
#include "images.h"
#include "metasqls.h"
#include "package.h"
#include "packages.h"
#include "reports.h"
#include "scripts.h"
#include "helpView.h"
#include "uiforms.h"

#include "fixACL.h"
#include "fixSerial.h"
#include "exportData.h"
#include "importData.h"


#include "setup.h"

extern QString __path;

bool menuSystem::loadCSVPlugin()
{
  return ImportHelper::getCSVImpPlugin(parent);
}

menuSystem::menuSystem(GUIClient *Pparent) :
 QObject(Pparent)
{
  setObjectName("sysModule");
  parent = Pparent;

  QList<QToolBar *> toolbars = parent->findChildren<QToolBar *>();

  errorLogListener::initialize();

  systemMenu		= new QMenu(parent);
  masterInfoMenu	= new QMenu(parent);
  sysUtilsMenu		= new QMenu(parent);
  helpMenu		= new QMenu(parent);
  designMenu            = new QMenu(parent);
  employeeMenu          = new QMenu(parent);

  systemMenu->setObjectName("menu.sys");
  masterInfoMenu->setObjectName("menu.sys.masterinfo");
  sysUtilsMenu->setObjectName("menu.sys.utilities");
  helpMenu->setObjectName("menu.help");
  designMenu->setObjectName("menu.sys.design");
  employeeMenu->setObjectName("menu.sys.employee");

  actionProperties acts[] = {

    { "sys.eventManager",             tr("E&vent Manager..."),              SLOT(sEventManager()),             systemMenu, "true",                                      NULL, NULL, true },
    { "sys.viewDatabaseLog",          tr("View Database &Log..."),          SLOT(sErrorLog()),                 systemMenu, "true",                                      NULL, NULL, true },
    { "separator",                    NULL,                                 NULL,                              systemMenu, "true",                                      NULL, NULL, true },
    { "sys.preferences",              tr("&Preferences..."),                SLOT(sPreferences()),              systemMenu, "MaintainPreferencesSelf MaintainPreferencesOthers",  NULL,   NULL,   true },
    { "sys.hotkeys",                  tr("&Hot Keys..."),                   SLOT(sHotKeys()),                  systemMenu, "true",  NULL,   NULL,   !(_privileges->check("MaintainPreferencesSelf") || _privileges->check("MaintainPreferencesOthers")) },
    { "sys.rescanPrivileges",         tr("Rescan &Privileges"),             SLOT(sRescanPrivileges()),         systemMenu, "true",                                      NULL, NULL, true },
    { "separator",                    NULL,                                 NULL,                              systemMenu, "true",                                      NULL, NULL, true },
    { "sys.maintainUsers",            tr("Maintain &User Accounts..."),     SLOT(sMaintainUsers()),            systemMenu, "MaintainUsers",                             NULL, NULL, true },
    { "sys.maintainGroups",           tr("Maintain &Roles..."),             SLOT(sMaintainGroups()),           systemMenu, "MaintainGroups",                            NULL, NULL, true },
    { "sys.userPrivileges",           tr("User Privilege Audit..."),        SLOT(sUserPrivileges()),           systemMenu, "MaintainUsers MaintainGroups",              NULL, NULL, true },

    { "menu",                         tr("&Employees"),                     (char*)employeeMenu,               systemMenu, "true",                                      NULL, NULL, true },
    { "sys.employee",                 tr("&New..."),               	    SLOT(sNewEmployee()),            employeeMenu, "MaintainEmployees",                         NULL, NULL, true },
    { "sys.listEmployees",            tr("&List..."),             	    SLOT(sListEmployees()),          employeeMenu, "ViewEmployees MaintainEmployees",           NULL, NULL, true },
    { "sys.searchEmployees",          tr("&Search..."),       		    SLOT(sSearchEmployees()),        employeeMenu, "ViewEmployees MaintainEmployees",           NULL, NULL, true },
    { "separator",                    NULL,                                 NULL,                            employeeMenu, "true",                                      NULL, NULL, true },
    { "sys.employeeGroups",           tr("Employee &Groups..."),            SLOT(sEmployeeGroups()),         employeeMenu, "ViewEmployeeGroups MaintainEmployeeGroups", NULL, NULL, true },

    { "separator",                    NULL,                                 NULL,                              systemMenu, "true",                                      NULL, NULL, true },

  //  Design
    { "menu",           tr("&Design"),                (char*)designMenu,      systemMenu, "true",                        NULL, NULL, true },
    { "sys.reports",    tr("&Reports..."),            SLOT(sReports()),       designMenu, "MaintainReports",             NULL, NULL, true },
    { "sys.metasqls",   tr("&MetaSQL Statements..."), SLOT(sMetasqls()),      designMenu, "MaintainMetaSQL ViewMetaSQL", NULL, NULL, true },
    { "separator",      NULL,                         NULL,                   designMenu, "true",                        NULL, NULL, true },
    { "sys.uiforms",    tr("S&creens..."),            SLOT(sUIForms()),       designMenu, "MaintainScreens",             NULL, NULL, true },
    { "sys.scripts",    tr("Scr&ipts..."),            SLOT(sScripts()),       designMenu, "MaintainScripts",             NULL, NULL, true },
    { "separator",      NULL,                         NULL,                   designMenu, "true",                        NULL, NULL, true },
    { "sys.customCommands",tr("Custom Command&s..."), SLOT(sCustomCommands()),designMenu, "MaintainCustomCommands",      NULL, NULL, true },
    { "separator",      NULL,                         NULL,                   designMenu, "true",                        NULL, NULL, true },
    { "sys.packages",   tr("&Packages..."),           SLOT(sPackages()),      designMenu, "ViewPackages",                NULL, NULL, true },

  // Utilities
    { "menu",              tr("&Utilities"),(char*)sysUtilsMenu, systemMenu,    "true",                            NULL, NULL, true },
    { "sys.fixACL",        tr("&Access Control"),  SLOT(sFixACL()),     sysUtilsMenu,  "fixACL+#superuser",           NULL, NULL, true },
    { "sys.fixSerial",     tr("&Serial Columns"),  SLOT(sFixSerial()),  sysUtilsMenu,  "FixSerial+#superuser", NULL, NULL, true },
    { "separator",      NULL,                         NULL,             sysUtilsMenu, "true",                        NULL, NULL, true },
    { "sys.CSVAtlases",  tr("Maintain CS&V Atlases..."),             SLOT(sCSVAtlases()),  sysUtilsMenu, "ConfigureImportExport", NULL, NULL, loadCSVPlugin() },
    { "sys.importData",    tr("&Import Data"),     SLOT(sImportData()), sysUtilsMenu,  "ImportXML",        NULL, NULL, true },
    { "sys.exportData",    tr("&Export Data"),     SLOT(sExportData()), sysUtilsMenu,  "ExportXML",       NULL, NULL, true },
    { "separator",		NULL,				NULL,				sysUtilsMenu,	"true",	NULL,	NULL,	true	},
    { "sys.printAlignmentPage",	tr("Print &Alignment Page..."),	SLOT(sPrintAlignment()),	sysUtilsMenu,	"true",	NULL,	NULL,	true	},

    // Setup
    { "sys.setup",	tr("&Setup..."),	SLOT(sSetup()),	systemMenu,	"true",	NULL,	NULL,	true	},
    { "separator",		NULL,				NULL,				systemMenu,	"true",	NULL,	NULL,	true	},
    { "sys.exit",	tr("E&xit xTuple ERP..."), SLOT(sExit()),				systemMenu,	"true",	NULL,	NULL,	true	},

  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));
  parent->populateCustomMenu(systemMenu, "System");
  QAction *m = parent->menuBar()->addMenu(systemMenu);
  if(m)
    m->setText(tr("S&ystem"));

  //  Help
  helpMenu = new QMenu();
  actionProperties help[] = {
    { "help.about",          tr("&About..."),                  SLOT(sAbout()),            helpMenu, "true", NULL, NULL, true },
#ifndef Q_OS_MAC
    { "separator",           NULL,                             NULL,                      helpMenu, "true", NULL, NULL, true },
#endif
    { "help.ReferenceGuide", tr("Reference &Guide..."),        SLOT(sReferenceGuide()),   helpMenu, "true", NULL, NULL, true },
    { "separator",           NULL,                             NULL,                      helpMenu, "true", NULL, NULL, true },
    { "community.home",      tr("xTuple.org &Home"),           SLOT(sCommunityHome()),    helpMenu, "true", NULL, NULL, true },
    { "community.xtupleu",   tr("xTupleU Learning Center"),    SLOT(sCommunityxTupleU()), helpMenu, "true", NULL, NULL, true },
    { "community.xchange",   tr("MarketPlace"),                SLOT(sCommunityXchange()), helpMenu, "true", NULL, NULL, true },
    { "community.forums",    tr("Discussion &Forums"),         SLOT(sCommunityForums()),  helpMenu, "true", NULL, NULL, true },
    { "community.issues",    tr("&Bugs and Feature Requests"), SLOT(sCommunityIssues()),  helpMenu, "true", NULL, NULL, true },
    { "community.blogs",     tr("Bl&ogs"),                     SLOT(sCommunityBlogs()),   helpMenu, "true", NULL, NULL, true },
  };
  addActionsToMenu(help, sizeof(help) / sizeof(help[0]));

  m = parent->menuBar()->addMenu(helpMenu);
  if(m)
    m->setText(tr("&Help"));
}

void menuSystem::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  QAction * m = 0;
  for (unsigned int i = 0; i < numElems; i++)
  {
    if (acts[i].actionName == QString("menu"))
    {
      m = acts[i].menu->addMenu((QMenu*)(acts[i].slot));
      if(m)
        m->setText(acts[i].actionTitle);
    }
    else if (acts[i].actionName == QString("separator"))
    {
      m = acts[i].menu->addSeparator();
    }
    else if (acts[i].toolBar != NULL)
    {
      m = new Action( parent,
                  acts[i].actionName,
                  acts[i].actionTitle,
                  this,
                  acts[i].slot,
                  acts[i].menu,
                  acts[i].priv,
                  (acts[i].pixmap),
                  acts[i].toolBar) ;
    }
    else
    {
      m = new Action( parent,
                  acts[i].actionName,
                  acts[i].actionTitle,
                  this,
                  acts[i].slot,
                  acts[i].menu,
                  acts[i].priv ) ;
    }
    if (m) m->setVisible(acts[i].visible);
  }
}

void menuSystem::sEventManager()
{
  omfgThis->handleNewWindow(new eventManager());
}

void menuSystem::sPreferences()
{
  userPreferences newdlg(parent, "", true);
  newdlg.exec();
}

void menuSystem::sHotKeys()
{
  ParameterList params;

  params.append("currentUser");

  hotkeys newdlg(omfgThis, "", true);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Accepted)
    sRescanPrivileges();
}

void menuSystem::sRescanPrivileges()
{
  _privileges->load();
  omfgThis->saveToolbarPositions();
  _preferences->load();
  omfgThis->initMenuBar();
}

void menuSystem::sCustomCommands()
{
  omfgThis->handleNewWindow(new customCommands());
}

void menuSystem::sPackages()
{
  omfgThis->handleNewWindow(new packages());
}

void menuSystem::sReports()
{
  omfgThis->handleNewWindow(new reports());
}

void menuSystem::sMetasqls()
{
  omfgThis->handleNewWindow(new metasqls());
}

void menuSystem::sScripts()
{
  omfgThis->handleNewWindow(new scripts());
}

void menuSystem::sUIForms()
{
  omfgThis->handleNewWindow(new uiforms());
}

void menuSystem::sSetup()
{
  ParameterList params;
  params.append("module", Xt::SystemModule);

  setup newdlg(parent);
  newdlg.set(params);
  newdlg.exec();
}

void menuSystem::sMaintainUsers()
{
  omfgThis->handleNewWindow(new users());
}

void menuSystem::sUserPrivileges()
{
  omfgThis->handleNewWindow(new dspUserPrivileges());
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

void menuSystem::sTOC()
{
  helpView *_help = helpView::getInstance(parent);
  _help->show();
}

void menuSystem::sDownload()
{
  omfgThis->handleNewWindow(new helpDownload());
}

void menuSystem::sReferenceGuide()
{
  QDesktopServices::openUrl(QUrl("http://www.xtuple.org/sites/default/files/refguide/current/index.html"));
}

void menuSystem::sFixACL()
{
  omfgThis->handleNewWindow(new fixACL());
}

void menuSystem::sFixSerial()
{
  omfgThis->handleNewWindow(new fixSerial());
}

void menuSystem::sExportData()
{
  omfgThis->handleNewWindow(new exportData());
}

void menuSystem::sCSVAtlases()
{

#ifdef Q_OS_MAC
  if (_preferences->value("InterfaceWindowOption") == "Workspace")
  {
    QMessageBox::critical( parent, tr("Interface Option is Invalid"),
                          tr("<p>The Maintain CSV Atlases utility "
                             "is only available when user preferences "
                             "are set to show windows as free-floating.") );
    return;
  }
#endif

  omfgThis->handleNewWindow(ImportHelper::getCSVImpPlugin(parent)->getCSVToolWindow(omfgThis, 0));
  omfgThis->handleNewWindow(ImportHelper::getCSVImpPlugin(parent)->getCSVAtlasWindow(omfgThis, 0));
}

void menuSystem::sImportData()
{
  omfgThis->handleNewWindow(new importData());
}

void menuSystem::sCommunityHome()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/");
}

void menuSystem::sCommunityxTupleU()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtupleuniversity.com/");
}

void menuSystem::sCommunityForums()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/forum");
}

void menuSystem::sCommunityBlogs()
{
  omfgThis->launchBrowser(omfgThis, "http://xtuple.com/blog");
}

void menuSystem::sCommunityIssues()
{
  omfgThis->launchBrowser(omfgThis, "http://www.xtuple.org/issuetracker/view_all_bug_page.php");
}

void menuSystem::sCommunityXchange()
{
  omfgThis->launchBrowser(omfgThis, "https://www.xtuple.org/xchange");
}

#include "../hunspell/config.h"
#include "data.h"        // openrpt/common
void menuSystem::sAbout()
{
  QMessageBox::about( parent, tr("About..."),
                      tr( "<p>%1<br/>Version %2<br/>%3</p>"
                          "<p>Built with:</p><ul>"
                          "<li>OpenRPT %4 %5</li>"
                          "<li>Qt %6</li>"
                          "<li>hunspell %7 (modified by xTuple for compiler compatibility)</li>"
                          "</ul>"
                          )
                          .arg(_Name, _Version, _Copyright)
                          .arg( OpenRPT::version, OpenRPT::build)
                          .arg(QT_VERSION_STR)
                          .arg(PACKAGE_VERSION) // hunspell/config.h
                          );
}
