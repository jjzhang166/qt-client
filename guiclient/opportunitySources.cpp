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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "opportunitySources.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>

#include <parameter.h>
#include <openreports.h>
#include "opportunitySource.h"
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a opportunitySources as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
opportunitySources::opportunitySources(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
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
  
  
  _oplist->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft );
  _oplist->addColumn(tr("Description"), -1,          Qt::AlignLeft );

  if (_privleges->check("MaintainOpportunitySources"))
  {
    connect(_oplist, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_oplist, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_oplist, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_oplist, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    _new->setEnabled(FALSE);
  }

   sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
opportunitySources::~opportunitySources()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void opportunitySources::languageChange()
{
  retranslateUi(this);
}

void opportunitySources::sDelete()
{
  q.prepare( "SELECT ophead_id "
             "FROM ophead "
             "WHERE (ophead_opsource_id=:opsource_id);" );
  q.bindValue(":opsource_id", _oplist->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Opportunity Source"),
                           tr( "The selected Opportunity Source cannot be deleted as there are one or more Opportunities currently assigned to it.\n"
                               "You must reassign these Opportunities before you may delete the selected Opportunity Source." ) );
    return;
  }

  q.prepare( "DELETE FROM opsource "
             "WHERE (opsource_id=:opsource_id);" );
  q.bindValue(":opsource_id", _oplist->id());
  q.exec();

  sFillList();
}

void opportunitySources::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("opsource_id", _oplist->id());

  opportunitySource newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void opportunitySources::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("opsource_id", _oplist->id());

  opportunitySource newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void opportunitySources::sFillList()
{
  _oplist->populate( "SELECT opsource_id, opsource_name, opsource_descrip "
                     "  FROM opsource "
                     " ORDER BY opsource_name;" );
}

void opportunitySources::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  opportunitySource newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void opportunitySources::sPrint()
{
  orReport report("OpportunitySourceMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


