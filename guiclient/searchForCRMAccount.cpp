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

#include "searchForCRMAccount.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>

//#include "competitor.h"
#include "crmaccount.h"
#include "customer.h"
//#include "partner.h"
#include "prospect.h"
#include "taxAuthority.h"
#include "vendor.h"

searchForCRMAccount::searchForCRMAccount(QWidget* parent, const char* name, Qt::WFlags fl)
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
  connect(_searchCombo,  SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_comboCombo,   SIGNAL(newID(int)),    this, SLOT(sFillList()));
  connect(_crmacct,	 SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu *)));
  connect(_view,	 SIGNAL(clicked()),	this, SLOT(sView()));

  _crmacct->addColumn(tr("Number"),      80, Qt::AlignCenter );
  _crmacct->addColumn(tr("Name"),        -1, Qt::AlignLeft   );
  _crmacct->addColumn(tr("First"),       50, Qt::AlignLeft   );
  _crmacct->addColumn(tr("Last"),        -1, Qt::AlignLeft   );
  _crmacct->addColumn(tr("Phone"),      100, Qt::AlignLeft   );
  _crmacct->addColumn(tr("Address"),     -1, Qt::AlignLeft   );
  _crmacct->addColumn(tr("City"),        75, Qt::AlignLeft   );
  _crmacct->addColumn(tr("State"),       50, Qt::AlignLeft   );
  _crmacct->addColumn(tr("Country"),    100, Qt::AlignLeft   );
  _crmacct->addColumn(tr("Postal Code"), 75, Qt::AlignLeft   );

  connect(omfgThis, SIGNAL(crmAccountsUpdated(int)), this, SLOT(sFillList()));
  _editpriv = _privileges->check("MaintainCRMAccounts");
  _viewpriv = _privileges->check("ViewCRMAccounts");
  if (_editpriv)
  {
    connect(_crmacct, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_crmacct, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else if (_viewpriv)
    connect(_crmacct, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  _subtype = CRMAcctLineEdit::Crmacct;
  _searchCombo->setVisible(FALSE);
  _comboCombo->setVisible(FALSE);
}

searchForCRMAccount::~searchForCRMAccount()
{
  // no need to delete child widgets, Qt does it all for us
}

void searchForCRMAccount::languageChange()
{
  retranslateUi(this);
}

SetResponse searchForCRMAccount::set(const ParameterList& pParams)
{
  QVariant	param;
  bool		valid;

  param = pParams.value("crmaccnt_subtype", &valid);
  if (valid)
  {
    if (param == "crmacct")
    {
      _subtype = CRMAcctLineEdit::Crmacct;
    }
    else if (param == "competitor")	// for now just use crmacct
    {
      _subtype = CRMAcctLineEdit::Competitor;
      setWindowTitle(tr("Search For Competitor CRM Accoun"));
    }
    else if (param == "partner")	// for now just use crmacct
    {
      _subtype = CRMAcctLineEdit::Partner;
      setWindowTitle(tr("Search For Partner CRM Accoun"));
    }
    else
    {
      disconnect(omfgThis, SIGNAL(crmAccountsUpdated(int)), this, SLOT(sFillList()));
      disconnect(_crmacct, SIGNAL(valid(bool)),	        _edit, SLOT(setEnabled(bool)));
      disconnect(_crmacct, SIGNAL(itemSelected(int)),   _edit, SLOT(animateClick()));
      disconnect(_crmacct, SIGNAL(itemSelected(int)),   _view, SLOT(animateClick()));
      const char* updateSignal = 0;

      if (param == "cust")
      {
	_subtype = CRMAcctLineEdit::Cust;
	_editpriv = _privileges->check("MaintainCustomerMasters");
	_viewpriv = _privileges->check("ViewCustomerMasters");
	setWindowTitle(tr("Search For Customer"));
	_searchNumber->setText(tr("Customer Number"));
	_searchName->setText(tr("Customer Name"));
	_searchContact->setText(tr("Billing Contact Name"));
	_searchPhone->setText(tr("Billing Contact Phone #"));
	_addressLit->setText(tr("Billing Contact Address:"));
	_showInactive->setText(tr("Show Inactive Customers"));
	updateSignal = SIGNAL(customersUpdated(int, bool));
      }
      else if (param == "prospect")
      {
	_subtype = CRMAcctLineEdit::Prospect;
	_editpriv = _privileges->check("MaintainProspects");
	_viewpriv = _privileges->check("ViewProspects");
	setWindowTitle(tr("Search For Prospect"));
	_searchNumber->setText(tr("Prospect Number"));
	_searchName->setText(tr("Prospect Name"));
	_showInactive->setText(tr("Show Inactive Prospects"));
	updateSignal = SIGNAL(prospectsUpdated());
      }
      else if (param == "taxauth")
      {
	_subtype = CRMAcctLineEdit::Taxauth;
	_editpriv = _privileges->check("MaintainTaxAuthorities");
	_viewpriv = _privileges->check("ViewTaxAuthorities");
	setWindowTitle(tr("Search For Tax Authority"));
	_searchNumber->setText(tr("Tax Authority Code"));
	_searchName->setText(tr("Tax Authority Name"));
	_searchContact->setVisible(false);
	_searchPhone->setVisible(false);
	_addressLit->setText(tr("Tax Authority Address:"));
	_showInactive->setText(tr("Show Inactive Tax Authorities"));
	_crmacct->hideColumn(2);
	_crmacct->hideColumn(3);
	_crmacct->hideColumn(4);
	_showInactive->setVisible(false);
	updateSignal = SIGNAL(taxAuthsUpdated(int));
      }
      else if (param == "vend")
      {
	_subtype = CRMAcctLineEdit::Vend;
	_editpriv = _privileges->check("MaintainVendors");
	_viewpriv = _privileges->check("ViewVendors");
	setWindowTitle(tr("Search For Vendor"));
	_searchNumber->setText(tr("Vendor Number"));
	_searchName->setText(tr("Vendor Name"));
	_addressLit->setText(tr("Main Address:"));
	_showInactive->setText(tr("Show Inactive Vendors"));
	updateSignal = SIGNAL(vendorsUpdated());
        _crmacct->addColumn(tr("Vend. Type"), _itemColumn, Qt::AlignLeft);
        _searchCombo->setVisible(TRUE);
        _comboCombo->setVisible(TRUE);
      }
      else
	return UndefinedError;
	// and if nobody checks for errors then we'll default to crmaccount

      if (updateSignal)
	connect(omfgThis, updateSignal, this, SLOT(sFillList()));
      if (_editpriv)
      {
	connect(_crmacct, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
	connect(_crmacct, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      }
      else if (_viewpriv)
	connect(_crmacct, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }
  }

  return NoError;
}

void searchForCRMAccount::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  pMenu->setItemEnabled(menuItem, _editpriv);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  pMenu->setItemEnabled(menuItem, _viewpriv);
}

void searchForCRMAccount::openSubwindow(const QString& mode)
{
  ParameterList params;
  params.append("mode", mode);

  switch (_subtype)
  {
    case CRMAcctLineEdit::Crmacct:
      {
      params.append("crmacct_id", _crmacct->id());
      crmaccount *newdlg = new crmaccount();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      }
      break;

    case CRMAcctLineEdit::Cust:
      {
      params.append("cust_id", _crmacct->id());
      customer *newdlg = new customer();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      }
      break;

    case CRMAcctLineEdit::Prospect:
      {
      params.append("prospect_id", _crmacct->id());
      prospect *newdlg = new prospect();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      }
      break;

    case CRMAcctLineEdit::Taxauth:
      {
      params.append("taxauth_id", _crmacct->id());
      taxAuthority newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
      }
      break;

    case CRMAcctLineEdit::Vend:
      {
      params.append("vend_id", _crmacct->id());
      vendor *newdlg = new vendor();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      }
      break;

    case CRMAcctLineEdit::Competitor:
    case CRMAcctLineEdit::Partner:
    default:
      break;
  }

}

void searchForCRMAccount::sEdit()
{
  openSubwindow("edit");
}

void searchForCRMAccount::sView()
{
  openSubwindow("view");
}

void searchForCRMAccount::sFillList()
{
  if (_search->text().stripWhiteSpace().length() == 0)
  {
    _crmacct->clear();
    return;
  }

  QString sql;

  sql = "SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"<? if exists(\"crmacct\") ?>"
	"       crmacct_id AS id, crmacct_number AS number, crmacct_name AS name,"
	"       cntct.* "
	"    FROM crmacct LEFT OUTER JOIN"
	"         cntct ON (crmacct_cntct_id_1=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> crmacct_active AND <? endif ?>"
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(crmacct_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(crmacct_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? elseif exists(\"cust\") ?>"
	"       cust_id AS id, cust_number AS number, cust_name AS name,"
	"       cntct.* "
	"    FROM custinfo LEFT OUTER JOIN"
	"         cntct ON (cust_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> cust_active AND <? endif ?>"
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(cust_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(cust_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? elseif exists(\"prospect\") ?>"
	"       prospect_id AS id, prospect_number AS number, prospect_name AS name,"
	"       cntct.* "
	"    FROM prospect LEFT OUTER JOIN"
	"         cntct ON (prospect_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> prospect_active AND <? endif ?>"
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(prospect_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(prospect_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? elseif exists(\"taxauth\") ?>"
	"       taxauth_id AS id, taxauth_code AS number, taxauth_name AS name,"
	"       '' AS cntct_first_name, '' AS cntct_last_name, "
	"       '' AS cntct_phone "
	"    FROM taxauth LEFT OUTER JOIN"
	"         addr ON (taxauth_addr_id=addr_id) "
	"    WHERE "
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(taxauth_code) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(taxauth_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? elseif exists(\"vend\") ?>"
	"       vend_id AS id, vend_number AS number, vend_name AS name, vendtype_code AS type,"
	"       cntct.* "
	"    FROM vendtype, vendinfo LEFT OUTER JOIN"
	"         cntct ON (vend_cntct1_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (vend_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> vend_active AND <? endif ?>"
        "      (vend_vendtype_id=vendtype_id)"
        "    <? if exists(\"combo_id\") ?>"
        "      AND (vendtype_id=<? value(\"combo_id\") ?>)"
        "    <? endif ?>"
	"      AND (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(vend_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(vend_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? endif ?>"
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

  switch (_subtype)
  {
    case CRMAcctLineEdit::Crmacct:
      params.append("crmacct");
      break;

    case CRMAcctLineEdit::Cust:
      params.append("cust");
      break;

    case CRMAcctLineEdit::Prospect:
      params.append("prospect");
      break;

    case CRMAcctLineEdit::Taxauth:
      params.append("taxauth");
      break;

    case CRMAcctLineEdit::Vend:
      params.append("vend");
      break;

    case CRMAcctLineEdit::Competitor:
    case CRMAcctLineEdit::Partner:
    default:
      return;
  }

  if (! _showInactive->isChecked())
    params.append("activeOnly");

  if (_searchNumber->isChecked())
    params.append("searchNumber");

  if (_searchName->isChecked())
    params.append("searchName");

  if (_subtype != CRMAcctLineEdit::Taxauth)	// taxauth doesn't have contacts (yet?)
  {
    if (_searchContact->isChecked())
      params.append("searchContactName");

    if (_searchPhone->isChecked())
      params.append("searchPhone");
  }

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

  if (_searchCombo->isChecked())
    params.append("combo_id", _comboCombo->id());

  XSqlQuery fillq = mql.toQuery(params);
  XTreeWidgetItem* last = 0;
  _crmacct->clear();

  while (fillq.next())
  {
    last = new XTreeWidgetItem(_crmacct, last,
			     fillq.value("id").toInt(),
			     fillq.value("id").toInt(),
			     fillq.value("number"),
			     fillq.value("name"),
			     fillq.value("cntct_first_name"),
			     fillq.value("cntct_last_name"),
			     fillq.value("cntct_phone"),
			     fillq.value("street"),
			     fillq.value("addr_city"),
			     fillq.value("addr_state"),
			     fillq.value("addr_country"),
			     fillq.value("addr_postalcode"));
    if(_subtype == CRMAcctLineEdit::Vend)
      last->setText(10, fillq.value("type"));
  }
  if (fillq.lastError().type() != QSqlError::None)
  {
    systemError(this, fillq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _crmacct->setCurrentItem(_crmacct->topLevelItem(0));
}
