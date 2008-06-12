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

#include "groups.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>

#include <parameter.h>
#include <openreports.h>
#include "group.h"
#include "guiclient.h"

/*
 *  Constructs a groups as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
groups::groups(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  statusBar()->hide();
  _print->hide();
  
  
  _list->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft );
  _list->addColumn(tr("Description"), -1,          Qt::AlignLeft );

  if (_privileges->check("MaintainGroups"))
  {
    connect(_list, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_list, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_list, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_list, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    _new->setEnabled(FALSE);
  }

   sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
groups::~groups()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void groups::languageChange()
{
  retranslateUi(this);
}

void groups::sDelete()
{
  q.prepare( "SELECT usrgrp_id "
             "FROM usrgrp "
             "WHERE (usrgrp_grp_id=:grp_id);" );
  q.bindValue(":grp_id", _list->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Group"),
                           tr( "The selected Group cannot be deleted as there are one or more Users currently assigned to it.\n"
                               "You must reassign these Users before you may delete the selected Group." ) );
    return;
  }

  q.prepare( "DELETE FROM grppriv"
             " WHERE (grppriv_grp_id=:grp_id);"
             "DELETE FROM grp "
             " WHERE (grp_id=:grp_id);" );
  q.bindValue(":grp_id", _list->id());
  q.exec();

  sFillList();
}

void groups::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  group newdlg(this, "", TRUE);
  newdlg.set(params);
  
  newdlg.exec();
  sFillList();
}

void groups::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("grp_id", _list->id());

  group newdlg(this, "", TRUE);
  newdlg.set(params);
  
  newdlg.exec();
  sFillList();
}

void groups::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("grp_id", _list->id());

  group newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void groups::sFillList()
{
  _list->populate( "SELECT grp_id, grp_name, grp_descrip "
                   "  FROM grp "
                   " ORDER BY grp_name;" );
}

void groups::sPrint()
{
  orReport report("GroupMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


