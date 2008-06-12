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

#include "contacts.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>

#include <openreports.h>
#include <metasql.h>

#include "contact.h"
#include "storedProcErrorLookup.h"

contacts::contacts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    connect(_contacts, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
    connect(_edit,		SIGNAL(clicked()),	this, SLOT(sEdit()));
    connect(_view,		SIGNAL(clicked()),	this, SLOT(sView()));
    connect(_delete,		SIGNAL(clicked()),	this, SLOT(sDelete()));
    connect(_print,		SIGNAL(clicked()),	this, SLOT(sPrint()));
    connect(_close,		SIGNAL(clicked()),	this, SLOT(close()));
    connect(_new,		SIGNAL(clicked()),	this, SLOT(sNew()));
    connect(_activeOnly,	SIGNAL(toggled(bool)),	this, SLOT(sFillList()));

    _activeOnly->setChecked(true);
    
    _contacts->addColumn(tr("First Name"),	 50, Qt::AlignLeft );
    _contacts->addColumn(tr("Last Name"),	100, Qt::AlignLeft );
    _contacts->addColumn(tr("Account #"),	 80, Qt::AlignLeft );
    _contacts->addColumn(tr("Account Name"),	160, Qt::AlignLeft );
    _contacts->addColumn(tr("Phone"),		100, Qt::AlignLeft );
    _contacts->addColumn(tr("Alternate"),		100, Qt::AlignLeft );
    _contacts->addColumn(tr("Fax"),		100, Qt::AlignLeft );
    _contacts->addColumn(tr("E-Mail"),		100, Qt::AlignLeft );
    _contacts->addColumn(tr("Web Address"),	100, Qt::AlignLeft );

    if (_privileges->check("MaintainContacts"))
    {
      connect(_contacts, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_contacts, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_contacts, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
      _new->setEnabled(FALSE);
      connect(_contacts, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }

    sFillList();
}

contacts::~contacts()
{
    // no need to delete child widgets, Qt does it all for us
}

void contacts::languageChange()
{
    retranslateUi(this);
}

void contacts::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainContacts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainContacts"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void contacts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  contact newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contacts::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cntct_id", _contacts->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contacts::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cntct_id", _contacts->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void contacts::sDelete()
{
  q.prepare("SELECT deleteContact(:cntct_id) AS result;");
  q.bindValue(":cntct_id", _contacts->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteContact", result), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void contacts::setParams(ParameterList &params)
{
  if (_activeOnly->isChecked())
    params.append("activeOnly");
}

void contacts::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("ContactsMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void contacts::sFillList()
{
  QString sql("SELECT cntct_id, cntct_first_name, cntct_last_name, "
	      "       crmacct_number, crmacct_name, cntct_phone, "
	      "       cntct_phone2, cntct_fax, cntct_email, cntct_webaddr "
	      "FROM cntct LEFT OUTER JOIN crmacct ON (cntct_crmacct_id=crmacct_id) "
	      "<? if exists(\"activeOnly\") ?> WHERE cntct_active <? endif ?>"
	      "ORDER BY cntct_last_name, cntct_first_name, crmacct_number;");
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _contacts->clear();
  _contacts->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
