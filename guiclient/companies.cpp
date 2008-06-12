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

#include "companies.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <parameter.h>
#include <QWorkspace>
#include "company.h"

/*
 *  Constructs a companies as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
companies::companies(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_company, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_company, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
companies::~companies()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void companies::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void companies::init()
{
  statusBar()->hide();
  
  if (_privileges->check("MaintainChartOfAccounts"))
  {
    connect(_company, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_company, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_company, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_company, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _company->addColumn(tr("Number"),      _itemColumn,  Qt::AlignCenter );
  _company->addColumn(tr("Description"), -1,           Qt::AlignLeft   );

  sFillList();
}

void companies::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  company *newdlg = new company(this, "", TRUE);
  newdlg->set(params);
  if (newdlg->exec() != XDialog::Rejected)
    sFillList();
}

void companies::sEdit()
{
  ParameterList params;
  params.append("company_id", _company->id());
  params.append("mode", "edit");

  company *newdlg = new company(this, "", TRUE);
  newdlg->set(params);
  if (newdlg->exec() != XDialog::Rejected)
    sFillList();
}

void companies::sView()
{
  ParameterList params;
  params.append("company_id", _company->id());
  params.append("mode", "view");

  company *newdlg = new company(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void companies::sDelete()
{
  q.prepare( "SELECT accnt_id "
             "FROM accnt "
             "WHERE (accnt_company=:company);" );
  q.bindValue(":company", _company->currentItem()->text(0));
  q.exec();
  if (q.first())
  {
    QMessageBox::warning( this, tr("Cannot Delete Company"),
                          tr( "The selected Company cannot be deleted as it is in use by existing Account.\n"
                              "You must reclass these Accounts before you may delete the selected Company." ) );
    return;
  }

  q.prepare( "DELETE FROM company "
             "WHERE (company_id=:company_id);" );
  q.bindValue(":company_id", _company->id());
  q.exec();

  sFillList();
}

void companies::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  pMenu->insertItem("View...", this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem("Edit...", this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainChartOfAccounts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem("Delete...", this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainChartOfAccounts"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void companies::sFillList()
{
  q.prepare( "SELECT company_id,"
             "       company_number, company_descrip "
             "FROM company "
             "ORDER BY company_number;" );
  q.exec();
  _company->populate(q);
}

