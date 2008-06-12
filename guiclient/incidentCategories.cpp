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

#include "incidentCategories.h"

#include <QMenu>
#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <openreports.h>
#include "incidentCategory.h"

/*
 *  Constructs an incidentCategories as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
incidentCategories::incidentCategories(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_close,  SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_incidentCategories, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_incidentCategories, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_new,   SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,  SIGNAL(clicked()), this, SLOT(sView()));

  statusBar()->hide();
  
  _incidentCategories->addColumn(tr("Order"),  _seqColumn, Qt::AlignRight);
  _incidentCategories->addColumn(tr("Category"),      100, Qt::AlignLeft );
  _incidentCategories->addColumn(tr("Description"),    -1, Qt::AlignLeft );

  if (_privileges->check("MaintainIncidentCategories"))
  {
    connect(_incidentCategories, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_incidentCategories, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_incidentCategories, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_incidentCategories, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

incidentCategories::~incidentCategories()
{
    // no need to delete child widgets, Qt does it all for us
}

void incidentCategories::languageChange()
{
    retranslateUi(this);
}

void incidentCategories::sFillList()
{
  q.prepare( "SELECT incdtcat_id, incdtcat_order, "
	     "       incdtcat_name, firstLine(incdtcat_descrip) "
             "FROM incdtcat "
             "ORDER BY incdtcat_order, incdtcat_name;" );
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _incidentCategories->populate(q);
}

void incidentCategories::sDelete()
{
  q.prepare( "DELETE FROM incdtcat "
             "WHERE (incdtcat_id=:incdtcat_id);" );
  q.bindValue(":incdtcat_id", _incidentCategories->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void incidentCategories::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  incidentCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentCategories::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdtcat_id", _incidentCategories->id());

  incidentCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentCategories::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdtcat_id", _incidentCategories->id());

  incidentCategory newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void incidentCategories::sPopulateMenu( QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainIncidentCategories"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainIncidentCategories"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void incidentCategories::sPrint()
{
  orReport report("IncidentCategoriesList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
