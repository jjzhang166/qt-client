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

#include "vendors.h"

#include <QMessageBox>
#include <QMenu>

#include <openreports.h>
#include "vendor.h"
#include "storedProcErrorLookup.h"

vendors::vendors(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,  SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_vendor, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view,   SIGNAL(clicked()), this, SLOT(sView()));

  _vendor->addColumn(tr("Type"),   _itemColumn, Qt::AlignCenter, true, "vendtype_code");
  _vendor->addColumn(tr("Number"), _itemColumn, Qt::AlignLeft,   true, "vend_number");
  _vendor->addColumn(tr("Name"),   -1,          Qt::AlignLeft,   true, "vend_name");

  connect(omfgThis, SIGNAL(vendorsUpdated()), SLOT(sFillList()));

  if (_privileges->check("MaintainVendors"))
  {
    connect(_vendor, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_vendor, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_vendor, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_vendor, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

vendors::~vendors()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendors::languageChange()
{
  retranslateUi(this);
}

void vendors::sPrint()
{
  orReport report("VendorMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void vendors::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendors::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vend_id", _vendor->id());
  params.append("showNextPrev");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendors::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("vend_id", _vendor->id());
  params.append("showNextPrev");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendors::sCopy()
{
}

void vendors::sDelete()
{
  q.prepare("SELECT deleteVendor(:vend_id) AS result;");
  q.bindValue(":vend_id", _vendor->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      QMessageBox::critical( this, tr("Cannot Delete Vendor"),
			     storedProcErrorLookup("deleteVendor", result));
      return;
    }
    omfgThis->sVendorsUpdated();
    sFillList();
  }
}

void vendors::sPopulateMenu(QMenu *menuThis)
{
  int intMenuItem;

  intMenuItem = menuThis->insertItem(tr("Edit Vendor..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainVendors"))
    menuThis->setItemEnabled(intMenuItem, FALSE);

  intMenuItem = menuThis->insertItem(tr("View Vendor..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainVendors")) && (!_privileges->check("ViewVendors")))
    menuThis->setItemEnabled(intMenuItem, FALSE);

  intMenuItem = menuThis->insertItem(tr("Delete Vendor..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainVendors"))
    menuThis->setItemEnabled(intMenuItem, FALSE);
}

void vendors::sFillList()
{
  _vendor->populate( "SELECT vend_id, * "
                     "FROM vend, vendtype "
                     "WHERE (vend_vendtype_id=vendtype_id) "
                     "ORDER BY vend_number;" );
}
