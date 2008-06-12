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

#include "searchForContact.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>

#include "contact.h"

searchForContact::searchForContact(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_edit,	 SIGNAL(clicked()),	this, SLOT(sEdit()));
  connect(_search,	 SIGNAL(lostFocus()),	this, SLOT(sFillList()));
  connect(_searchStreet, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchCity,   SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchState,  SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchPostalCode,SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchCountry,SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchContact,SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchName,	 SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchNumber, SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchPhone,	 SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_showInactive, SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_cntct,	 SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu *)));
  connect(_view,	 SIGNAL(clicked()),	this, SLOT(sView()));

  _cntct->addColumn(tr("First"),       50, Qt::AlignLeft   );
  _cntct->addColumn(tr("Last"),        -1, Qt::AlignLeft   );
  _cntct->addColumn(tr("Account Number"),      80, Qt::AlignCenter );
  _cntct->addColumn(tr("Account Name"),        -1, Qt::AlignLeft   );
  _cntct->addColumn(tr("Phone"),      100, Qt::AlignLeft   );
  _cntct->addColumn(tr("Address"),     -1, Qt::AlignLeft   );
  _cntct->addColumn(tr("City"),        75, Qt::AlignLeft   );
  _cntct->addColumn(tr("State"),       50, Qt::AlignLeft   );
  _cntct->addColumn(tr("Country"),    100, Qt::AlignLeft   );
  _cntct->addColumn(tr("Postal Code"), 75, Qt::AlignLeft   );
  
  _editpriv = _privileges->check("MaintainContacts");
  _viewpriv = _privileges->check("ViewContacts");
  if (_editpriv)
  {
    connect(_cntct, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_cntct, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else if (_viewpriv)
    connect(_cntct, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    
}

searchForContact::~searchForContact()
{
  // no need to delete child widgets, Qt does it all for us
}

void searchForContact::languageChange()
{
  retranslateUi(this);
}

void searchForContact::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  pMenu->setItemEnabled(menuItem, _editpriv);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  pMenu->setItemEnabled(menuItem, _viewpriv);
}

void searchForContact::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cntct_id", _cntct->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void searchForContact::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cntct_id", _cntct->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void searchForContact::sFillList()
{
  if (_search->text().stripWhiteSpace().length() == 0)
  {
    _cntct->clear();
    return;
  }

  QString sql;

  sql = "SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       crmacct_id AS id, crmacct_number AS number, crmacct_name AS name,"
	"       cntct.* "
    "    FROM cntct LEFT OUTER JOIN"
	"         crmacct ON (cntct_crmacct_id=crmacct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> cntct_active AND <? endif ?>"
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(crmacct_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(crmacct_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? if exists(\"searchContactName\") ?>"
	"   OR (UPPER(cntct_first_name || ' ' || cntct_last_name) "
	"             ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchPhone\") ?>"
	"   OR (UPPER(cntct_phone || ' ' || cntct_phone2 || ' ' || "
	"             cntct_fax) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchStreetAddr\") ?>"
	"   OR (UPPER(addr_line1 || ' ' || addr_line2 || ' ' || "
	"             addr_line3) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchCity\") ?>"
	"   OR (UPPER(addr_city) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchState\") ?>"
	"   OR (UPPER(addr_state) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchPostalCode\") ?>"
	"   OR (UPPER(addr_postalcode) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchCountry\") ?>"
	"   OR (UPPER(addr_country) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"  )"
	"ORDER BY number;"  ;

  MetaSQLQuery mql(sql);
  ParameterList params;
  params.append("searchString", _search->text().stripWhiteSpace().upper());

  if (! _showInactive->isChecked())
    params.append("activeOnly");

  if (_searchNumber->isChecked())
    params.append("searchNumber");

  if (_searchName->isChecked())
    params.append("searchName");
    
  if (_searchContact->isChecked())
    params.append("searchContactName");

  if (_searchPhone->isChecked())
    params.append("searchPhone");

  if (_searchStreet->isChecked())
    params.append("searchStreetAddr");

  if (_searchCity->isChecked())
    params.append("searchCity");

  if (_searchState->isChecked())
    params.append("searchState");

  if (_searchPostalCode->isChecked())
    params.append("searchPostalCode");

  if (_searchCountry->isChecked())
    params.append("searchCountry");

  XSqlQuery fillq = mql.toQuery(params);
  XTreeWidgetItem* last = 0;
  _cntct->clear();

  while (fillq.next())
  {
    last = new XTreeWidgetItem(_cntct, last,
			     fillq.value("cntct_id").toInt(),
			     fillq.value("cntct_id").toInt(),
			     fillq.value("cntct_first_name"),
			     fillq.value("cntct_last_name"),
			     fillq.value("number"),
			     fillq.value("name"),
			     fillq.value("cntct_phone"),
			     fillq.value("street"),
			     fillq.value("addr_city"),
			     fillq.value("addr_state"),
			     fillq.value("addr_country"),
			     fillq.value("addr_postalcode"));
  }
  if (fillq.lastError().type() != QSqlError::None)
  {
    systemError(this, fillq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _cntct->setCurrentItem(_cntct->topLevelItem(0));
}
