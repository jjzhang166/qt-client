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

#include "costCategories.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <QWorkspace>
#include <openreports.h>
#include "costCategory.h"
#include "dspItemSitesByParameterList.h"

/*
 *  Constructs a costCategories as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
costCategories::costCategories(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
    connect(_costcat, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_costcat, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
costCategories::~costCategories()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void costCategories::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void costCategories::init()
{
  statusBar()->hide();
  
  _costcat->addColumn(tr("Category"),    _itemColumn, Qt::AlignCenter );
  _costcat->addColumn(tr("Description"), -1,          Qt::AlignLeft   );

  if (_privileges->check("MaintainCostCategories"))
  {
    connect(_costcat, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_costcat, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_costcat, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_costcat, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_costcat, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

void costCategories::sPrint()
{
  orReport report("CostCategoriesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void costCategories::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  costCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void costCategories::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("costcat_id", _costcat->id());

  costCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void costCategories::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("costcat_id", _costcat->id());

  costCategory newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void costCategories::sCopy()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("costcat_id", _costcat->id());

  costCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void costCategories::sDelete()
{
  if ( QMessageBox::information( this, tr("Delete Cost Category"),
                                 tr("Are you sure that you want to delete the selected Cost Category?"),
                                 tr("&Delete"), tr("&Cancel"), 0, 0, 1 ) == 0 )
  {
    q.prepare( "SELECT itemsite_id "
               "FROM itemsite "
               "WHERE (itemsite_costcat_id=:costcat_id) "
               "LIMIT 1;" );
    q.bindValue(":costcat_id", _costcat->id());
    q.exec();
    if (q.first())
      QMessageBox::information( this, tr("Cost Category in Use"),
                                tr( "The selected Cost Category cannot be deleted as it still contains Items.\n"
                                    "You must reassign these Items before deleting this Cost Category.") );

    else
    {
      q.prepare( "DELETE FROM costcat "
                 "WHERE (costcat_id=:costcat_id);" );
      q.bindValue(":costcat_id", _costcat->id());
      q.exec();
      sFillList();
    }
  }
}

void costCategories::sPopulateMenu(QMenu *menu)
{
  int menuItem;

  menuItem = menu->insertItem(tr("Edit Inventory Cost Cateogry..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainCostCategories"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("View Inventory Cost Category..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainCostCategories")) && (!_privileges->check("ViewCostCategories")))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Delete Inventory Cost Category..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainCostCategories"))
    menu->setItemEnabled(menuItem, FALSE);

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("List Items in this Inventory Cost Category..."), this, SLOT(sListItemSites()), 0);
  if (!_privileges->check("ViewItemSites"))
    menu->setItemEnabled(menuItem, FALSE);
}

void costCategories::sListItemSites()
{
  ParameterList params;
  params.append("run");
  params.append("costcat_id", _costcat->id());

  dspItemSitesByParameterList *newdlg = new dspItemSitesByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void costCategories::sFillList()
{
  _costcat->populate( "SELECT costcat_id, costcat_code, costcat_descrip "
                      "FROM costcat "
                      "ORDER BY costcat_code" );
}
