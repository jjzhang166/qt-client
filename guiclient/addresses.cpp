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

#include "addresses.h"

#include <QMenu>
#include <QSqlError>
#include <QMessageBox>
#include <QVariant>
#include <QStatusBar>
#include <openreports.h>
#include <metasql.h>
#include "addresses.h"
#include "address.h"

#include "storedProcErrorLookup.h"

/*
 *  Constructs a addresses as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
addresses::addresses(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_address, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*, int)));
    connect(_edit,		SIGNAL(clicked()),	this, SLOT(sEdit()));
    connect(_view,		SIGNAL(clicked()),	this, SLOT(sView()));
    connect(_delete,		SIGNAL(clicked()),	this, SLOT(sDelete()));
    connect(_print,		SIGNAL(clicked()),	this, SLOT(sPrint()));
    connect(_close,		SIGNAL(clicked()),	this, SLOT(close()));
    connect(_new,		SIGNAL(clicked()),	this, SLOT(sNew()));
    connect(_activeOnly,	SIGNAL(toggled(bool)),	this, SLOT(sFillList()));

    //statusBar()->hide();
    _activeOnly->setChecked(true);
    
    _address->addColumn(tr("Line 1"),	 -1, Qt::AlignLeft );
    _address->addColumn(tr("Line 2"),	 75, Qt::AlignLeft );
    _address->addColumn(tr("Line 3"),	 75, Qt::AlignLeft );
    _address->addColumn(tr("City"),	 75, Qt::AlignLeft );
    _address->addColumn(tr("State"),	 50, Qt::AlignLeft );
    _address->addColumn(tr("Country"),	 50, Qt::AlignLeft );
    _address->addColumn(tr("Postal Code"),50,Qt::AlignLeft );

    if (_privileges->check("MaintainAddresses"))
    {
      connect(_address, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_address, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_address, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
      _new->setEnabled(FALSE);
      connect(_address, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }

    sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
addresses::~addresses()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void addresses::languageChange()
{
    retranslateUi(this);
}


void addresses::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainAddresses"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainAddresses"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void addresses::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  address newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void addresses::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("addr_id", _address->id());

  address newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void addresses::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("addr_id", _address->id());

  address newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void addresses::sDelete()
{
  q.prepare("SELECT deleteAddress(:addr_id) AS result;");
  q.bindValue(":addr_id", _address->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      QMessageBox::warning(this, tr("Cannot Delete Selected Address"),
			   storedProcErrorLookup("deleteAddress", result));
      return;
    }
    else
      sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void addresses::sPrint()
{
  ParameterList params;
  if (_activeOnly->isChecked())
    params.append("activeOnly");

  orReport report("AddressesMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void addresses::sFillList()
{
  QString sql("SELECT addr_id, addr_line1, addr_line2, addr_line3, "
	      "       addr_city, addr_state, addr_country, addr_postalcode "
              "FROM addr "
	      "<? if exists(\"activeOnly\") ?> WHERE addr_active <? endif ?>"
              "ORDER BY addr_country, addr_state, addr_city, addr_line1;");
  ParameterList params;
  if (_activeOnly->isChecked())
    params.append("activeOnly");
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _address->populate(q);
}
