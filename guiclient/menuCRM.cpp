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

//  menuCRM.cpp
//  Created 09/05/2006 GJM
//  Copyright (c) 2006-2008, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include "guiclient.h"

#include "project.h"
#include "projects.h"
#include "dspOrderActivityByProject.h"

#include "contact.h"
#include "contacts.h"
#include "searchForContact.h"
#include "address.h"
#include "addresses.h"
#include "crmaccount.h"
#include "crmaccounts.h"
#include "searchForCRMAccount.h"
#include "dspIncidentsByCRMAccount.h"
#include "dspTodoByUserAndIncident.h"
#include "incidentWorkbench.h"
#include "incident.h"
#include "todoList.h"
#include "todoItem.h"

#include "honorifics.h"
#include "incidentCategories.h"
#include "incidentPriorities.h"
#include "incidentSeverities.h"
#include "incidentResolutions.h"
#include "characteristics.h"

#include "opportunity.h"
#include "opportunityList.h"
#include "opportunitySources.h"
#include "opportunityStages.h"
#include "opportunityTypes.h"

#include "menuCRM.h"

menuCRM::menuCRM(GUIClient *Pparent) :
  QObject(Pparent, "crmModule")
{
  parent = Pparent;
  
  toolBar = new QToolBar(tr("CRM Tools"));
  toolBar->setObjectName("CRM Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowCRMToolbar"))
    parent->addToolBar(toolBar);

  // Menus
  crmMenu           = new QMenu(parent);
  projectsMenu      = new QMenu(parent);
  incidentMenu      = new QMenu(parent);
  todoMenu          = new QMenu(parent);
  reportsMenu       = new QMenu(parent);
  accountsMenu      = new QMenu(parent);
  contactsMenu      = new QMenu(parent);
  addressMenu       = new QMenu(parent);
  utilitiesMenu     = new QMenu(parent);
  masterMenu        = new QMenu(parent);
  masterIncdMenu    = new QMenu(parent);
  opportunityMenu   = new QMenu(parent);
  masterOppMenu     = new QMenu(parent);

  crmMenu->setObjectName("menu.crm");
  projectsMenu->setObjectName("menu.crm.projects");
  incidentMenu->setObjectName("menu.crm.incident");
  todoMenu->setObjectName("menu.crm.todo");
  reportsMenu->setObjectName("menu.crm.reports");
  accountsMenu->setObjectName("menu.crm.accounts");
  contactsMenu->setObjectName("menu.crm.contacts");
  addressMenu->setObjectName("menu.crm.address");
  utilitiesMenu->setObjectName("menu.crm.utilities");
  masterMenu->setObjectName("menu.crm.master");
  masterIncdMenu->setObjectName("menu.crm.masterincd");
  opportunityMenu->setObjectName("menu.crm.opportunity");
  masterOppMenu->setObjectName("menu.crm.masteropp");

  actionProperties acts[] = {
    // CRM | Incident
    { "menu",			tr("&Incident"),	(char*)incidentMenu,		crmMenu,	true, NULL, NULL, true	, NULL },
    { "crm.incident",		tr("&New..."),		SLOT(sIncident()),		incidentMenu,	_privileges->check("AddIncidents") || _privileges->check("MaintainIncidents"), NULL, NULL, true , NULL },
    { "separator",		NULL,				NULL,			incidentMenu,	true, NULL, NULL, true	, NULL },
    { "crm.incidentWorkbench",	tr("&Workbench..."),	SLOT(sIncidentWorkbench()),	incidentMenu,	_privileges->check("ViewIncidents") || _privileges->check("MaintainIncidents"), new QPixmap(":/images/incidents.png"), toolBar, true , "Incident Workbench" },

    // CRM / To Do
    { "menu",			tr("&To-Do"),	(char*)todoMenu,	crmMenu,	true, NULL, NULL, true	, NULL },
    { "crm.todoItem",		tr("&New..."),	SLOT(sTodoItem()),	todoMenu,	_privileges->check("MaintainPersonalTodoList"), NULL, NULL, true	, NULL },
    { "crm.todoList",		tr("&List..."),		SLOT(sTodoList()),	todoMenu,	_privileges->check("MaintainPersonalTodoList") || _privileges->check("ViewPersonalTodoList"),new QPixmap(":/images/toDoList.png"), toolBar, true	, "To-Do List"},

    //  Project
    { "menu", tr("Pro&ject"), (char*)projectsMenu, crmMenu,true, NULL, NULL, true	, NULL },
    { "pm.newProject", tr("&New..."), SLOT(sNewProject()), projectsMenu, _privileges->check("MaintainProjects"), NULL, NULL, true , NULL },
    { "pm.projects", tr("&List..."), SLOT(sProjects()), projectsMenu, _privileges->check("ViewProjects"), new QPixmap(":/images/projects.png"), toolBar, true , "List Projects" },
    
    // Opportunity
    { "menu",		tr("&Opportunity"),	(char*)opportunityMenu,	crmMenu,		true, NULL, NULL, true	, NULL },
    { "crm.newOpportunity", tr("&New..."), SLOT(sNewOpportunity()), opportunityMenu, _privileges->check("MaintainOpportunities"), NULL, NULL, true , NULL },
    { "crm.listOpportunity", tr("&List..."), SLOT(sOpportunities()), opportunityMenu, (_privileges->check("MaintainOpportunities") || _privileges->check("ViewOpportunities")), NULL, NULL, true , NULL },

    { "separator",		NULL,				NULL,			crmMenu,	true, NULL, NULL, true	, NULL },

    // Reports
    { "menu",				tr("&Reports"),		(char*)reportsMenu,			crmMenu,	true, NULL, NULL, true	, NULL },

    { "pm.dspOrderActivityByProject", tr("Order &Activity by Project..."), SLOT(sDspOrderActivityByProject()), reportsMenu, _privileges->check("ViewProjects"), NULL, NULL, true , NULL },
    { "separator",		NULL,				NULL,			reportsMenu,	true, NULL, NULL, true	, NULL },
    { "crm.dspIncidentsByCRMAccount",		tr("&Incidents by CRM Account..."),		SLOT(sDspIncidentsByCRMAccount()),		reportsMenu,	_privileges->check("ViewCRMAccounts") && _privileges->check("ViewIncidents") && _privileges->check("ViewOtherTodoLists"), NULL, NULL, true	, NULL },
    { "crm.dspTodoByUserAndIncident",		tr("&To-Do List Items by User and Incident..."),		SLOT(sDspTodoByUserAndIncident()),		reportsMenu,	_privileges->check("MaintainOtherTodoLists") || _privileges->check("ViewOtherTodoLists"), NULL, NULL, true	, NULL },
    { "separator",		NULL,				NULL,			crmMenu,	true, NULL, NULL, true	, NULL },
    
    // CRM | Account
    { "menu",		tr("&Account"),		(char*)accountsMenu,	crmMenu,		true, NULL, NULL, true	, NULL },
    { "crm.crmaccount",		tr("&New..."),	SLOT(sCRMAccount()),	accountsMenu,	_privileges->check("MaintainCRMAccounts"), NULL, NULL, true , NULL },
    { "crm.crmaccounts",	tr("&List..."),	SLOT(sCRMAccounts()),	accountsMenu,	_privileges->check("MaintainCRMAccounts") || _privileges->check("ViewCRMAccounts"),new QPixmap(":/images/accounts.png"), toolBar, true , "List Accounts" },
    { "crm.crmaccountsearch",	tr("&Search..."),SLOT(sSearchForCRMAccount()),accountsMenu,	_privileges->check("MaintainCRMAccounts") || _privileges->check("ViewCRMAccounts"), NULL, NULL, true , NULL },
      
    // CRM | Contact
    { "menu",		tr("&Contact"),		(char*)contactsMenu,	crmMenu,		true, NULL, NULL, true	, NULL },
    { "crm.contact",	tr("&New..."),		SLOT(sContact()),	contactsMenu,	_privileges->check("MaintainContacts"), NULL, NULL, true	, NULL },
    { "crm.contacts",	tr("&List..."),		SLOT(sContacts()),	contactsMenu,	_privileges->check("MaintainContacts") || _privileges->check("ViewContacts"),new QPixmap(":/images/contacts.png"), toolBar, true , "List Contacts" },
    { "crm.contactsearch",	tr("&Search..."),		SLOT(sSearchForContact()),	contactsMenu,	_privileges->check("MaintainContacts") || _privileges->check("ViewContacts"), NULL, NULL, true	, NULL },
    
    // CRM | Address
    { "menu",		tr("A&ddress"),		(char*)addressMenu,	crmMenu,		true, NULL, NULL, true	, NULL },
    { "crm.address",	tr("&New..."),		SLOT(sAddress()),	addressMenu,	_privileges->check("MaintainAddresses"), NULL, NULL, true	, NULL },
    { "crm.addresses",	tr("&List..."),	SLOT(sAddresses()),	addressMenu,	_privileges->check("MaintainAddresses") || _privileges->check("ViewAddresses"), NULL, NULL, true , NULL },

    { "separator",		NULL,				NULL,			crmMenu,	true, NULL, NULL, true	, NULL },

    // Master Information
    { "menu",			tr("&Master Information"),		(char*)masterMenu,		crmMenu,	true, NULL, NULL, true	, NULL },
    { "crm.honorifics",		tr("&Titles..."),			SLOT(sHonorifics()),		masterMenu,	_privileges->check("MaintainTitles") || _privileges->check("ViewTitles"), NULL, NULL, true	, NULL },
   
    { "menu",			tr("&Incident"),		(char*)masterIncdMenu,		masterMenu,	true, NULL, NULL, true	, NULL },
    { "crm.incidentCategories",	tr("&Categories..."),		SLOT(sIncidentCategories()),	masterIncdMenu,	_privileges->check("MaintainIncidentCategories"), NULL, NULL, true , NULL },
    { "crm.incidentPriorities",	tr("&Priorities..."),		SLOT(sIncidentPriorities()),	masterIncdMenu,	_privileges->check("MaintainIncidentPriorities"), NULL, NULL, true , NULL },
    { "crm.incidentSeverities",	tr("&Severities..."),		SLOT(sIncidentSeverities()),	masterIncdMenu,	_privileges->check("MaintainIncidentSeverities"), NULL, NULL, true , NULL },
    { "crm.incidentResolutions", tr("&Resolutions..."),		SLOT(sIncidentResolutions()),	masterIncdMenu,	_privileges->check("MaintainIncidentResolutions"), NULL, NULL, true , NULL },

    { "menu",			tr("&Opportunity"),		(char*)masterOppMenu,		masterMenu,	true, NULL, NULL, true	, NULL },
    { "crm.opportunitySources",	tr("&Sources..."),		SLOT(sOpportunitySources()),	masterOppMenu,	_privileges->check("MaintainOpportunitySources"), NULL, NULL, true , NULL },
    { "crm.opportunityStages",	tr("St&ages..."),		SLOT(sOpportunityStages()),	masterOppMenu,	_privileges->check("MaintainOpportunityStages"), NULL, NULL, true , NULL },
    { "crm.opportunityTypes",	tr("&Types..."),		SLOT(sOpportunityTypes()),	masterOppMenu,	_privileges->check("MaintainOpportunityTypes"), NULL, NULL, true , NULL },

    { "crm.characteristics",	tr("C&haracteristics..."),		SLOT(sCharacteristics()),	masterMenu,	_privileges->check("MaintainCharacteristics") ||
															_privileges->check("ViewCharacteristics"), NULL, NULL, true , NULL },
  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));

  parent->populateCustomMenu(crmMenu, "CRM");
  parent->menuBar()->insertItem(tr("&CRM"), crmMenu);
}
  
void menuCRM::addActionsToMenu(actionProperties acts[], unsigned int numElems)
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
    else if ((acts[i].toolBar != NULL) && (acts[i].toolBar != NULL))
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv,
					  *(acts[i].pixmap),
					  acts[i].toolBar,
                      acts[i].toolTip) );
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
					  acts[i].toolBar,
                      acts[i].actionTitle) );
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

void menuCRM::sNewProject()
{
  ParameterList params;
  params.append("mode", "new");

  project newdlg(omfgThis, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuCRM::sProjects()
{
  omfgThis->handleNewWindow(new projects());
}

void menuCRM::sDspOrderActivityByProject()
{
  omfgThis->handleNewWindow(new dspOrderActivityByProject());
}

void menuCRM::sCRMAccount()
{
  ParameterList params;
  params.append("mode", "new");
  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sCRMAccounts()
{
  omfgThis->handleNewWindow(new crmaccounts());
}

void menuCRM::sSearchForCRMAccount()
{
  omfgThis->handleNewWindow(new searchForCRMAccount());
}

void menuCRM::sContact()
{
  ParameterList params;
  params.append("mode", "new");
  contact* newdlg = new contact();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sContacts()
{
  omfgThis->handleNewWindow(new contacts());
}

void menuCRM::sSearchForContact()
{
  omfgThis->handleNewWindow(new searchForContact());
}

void menuCRM::sAddress()
{
  ParameterList params;
  params.append("mode", "new");
  address* newdlg = new address();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sAddresses()
{
  omfgThis->handleNewWindow(new addresses());
}

void menuCRM::sIncidentWorkbench()
{
  omfgThis->handleNewWindow(new incidentWorkbench());
}

void menuCRM::sIncident()
{
  ParameterList params;
  params.append("mode", "new");
  incident* newdlg = new incident();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sTodoList()
{
  omfgThis->handleNewWindow(new todoList());
}

void menuCRM::sTodoItem()
{
  ParameterList params;
  params.append("mode", "new");
  todoItem* newdlg = new todoItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sHonorifics()
{
  omfgThis->handleNewWindow(new honorifics());
}

void menuCRM::sIncidentCategories()
{
  omfgThis->handleNewWindow(new incidentCategories());
}

void menuCRM::sIncidentPriorities()
{
  omfgThis->handleNewWindow(new incidentPriorities());
}

void menuCRM::sIncidentSeverities()
{
  omfgThis->handleNewWindow(new incidentSeverities());
}

void menuCRM::sIncidentResolutions()
{
  omfgThis->handleNewWindow(new incidentResolutions());
}

void menuCRM::sCharacteristics()
{
  omfgThis->handleNewWindow(new characteristics());
}

void menuCRM::sDspIncidentsByCRMAccount()
{
  omfgThis->handleNewWindow(new dspIncidentsByCRMAccount());
}

void menuCRM::sDspTodoByUserAndIncident()
{
  omfgThis->handleNewWindow(new dspTodoByUserAndIncident());
}

void menuCRM::sOpportunitySources()
{
  omfgThis->handleNewWindow(new opportunitySources());
}

void menuCRM::sOpportunityStages()
{
  omfgThis->handleNewWindow(new opportunityStages());
}

void menuCRM::sOpportunityTypes()
{
  omfgThis->handleNewWindow(new opportunityTypes());
}

void menuCRM::sNewOpportunity()
{
  ParameterList params;
  params.append("mode", "new");

  opportunity newdlg(omfgThis, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void menuCRM::sOpportunities()
{
  omfgThis->handleNewWindow(new opportunityList());
}
