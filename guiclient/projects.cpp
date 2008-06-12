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

#include "projects.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <parameter.h>
#include <QWorkspace>
#include "project.h"

/*
 *  Constructs a projects as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
projects::projects(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_prj, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_prj, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
projects::~projects()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void projects::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void projects::init()
{
  statusBar()->hide();
  
  if (_privileges->check("MaintainProjects"))
  {
    connect(_prj, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_prj, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_prj, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_prj, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _prj->addColumn(tr("Number"), -1,           Qt::AlignCenter );
  _prj->addColumn(tr("Name"),   _orderColumn, Qt::AlignLeft   );
  _prj->addColumn(tr("Status"), _itemColumn,  Qt::AlignCenter );

  connect(omfgThis, SIGNAL(projectsUpdated(int)), this, SLOT(sFillList()));

  sFillList();
}

void projects::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void projects::sEdit()
{
  ParameterList params;
  params.append("prj_id", _prj->id());
  params.append("mode", "edit");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void projects::sView()
{
  ParameterList params;
  params.append("prj_id", _prj->id());
  params.append("mode", "view");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void projects::sDelete()
{
  q.prepare("SELECT deleteProject(:prj_id) AS result");
  q.bindValue(":prj_id", _prj->id());
  q.exec();
  if(q.first())
  {
    int result = q.value("result").toInt();
    if(result < 0)
    {
      QString errmsg;
      switch(result)
      {
        case -1:
          errmsg = tr("One or more Quote's refer to this project.");
          break;
        case -2:
          errmsg = tr("One or more Sales Orders refer to this project.");
          break;
        case -3:
          errmsg = tr("One or more Work Orders refer to this project.");
          break;
        case -4:
          errmsg = tr("One or more Purchase Requests refer to this project.");
          break;
        case -5:
          errmsg = tr("One or more Purchase order Items refer to this project.");
          break;
        case -6:
          errmsg = tr("One or more Invoices refer to this project.");
          break;
        default:
          errmsg = tr("Error #%1 encountered while trying to delete project.").arg(result);
      }
      QMessageBox::critical( this, tr("Cannot Delete Project"),
        tr("Could not delete the project for one or more reasons.\n") + errmsg);
      return;
    }
  }
  sFillList();
}

void projects::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  pMenu->insertItem("View...", this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem("Edit...", this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainProjects"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem("Delete...", this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainProjects"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void projects::sFillList()
{
  q.prepare( "SELECT prj_id,"
             "       prj_number, prj_name,"
             "       CASE WHEN(prj_status='P') THEN :planning"
             "            WHEN(prj_status='O') THEN :open"
             "            WHEN(prj_status='C') THEN :complete"
             "            ELSE :undefined"
             "       END "
             "FROM prj "
             "ORDER BY prj_number;" );
  q.bindValue(":planning", tr("Concept"));
  q.bindValue(":open", tr("In-Process"));
  q.bindValue(":complete", tr("Complete"));
  q.bindValue(":undefined", tr("Undefined"));
  q.exec();
  _prj->populate(q);
}

