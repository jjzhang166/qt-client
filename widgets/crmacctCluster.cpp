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

// crmacctCluster.cpp
// Created 09/11/2006 GJM
// Copyright (c) 2006-2008, OpenMFG, LLC

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>

#include "crmacctcluster.h"
#include "custcluster.h"
#include "xcombobox.h"

CRMAcctCluster::CRMAcctCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new CRMAcctLineEdit(this, pName));
    setNameVisible(true);
    setSubtype(CRMAcctLineEdit::Crmacct);
}

void CRMAcctCluster::setSubtype(CRMAcctLineEdit::CRMAcctSubtype subtype)
{
  // TODO: make this do something useful
  if (_number->inherits("CRMAcctLineEdit"))
    (qobject_cast<CRMAcctLineEdit*>(_number))->setSubtype(subtype);
}

///////////////////////////////////////////////////////////////////////

CRMAcctLineEdit::CRMAcctSubtype CRMAcctCluster::subtype() const
{
  if (_number->inherits("CRMAcctLineEdit"))
    return (qobject_cast<CRMAcctLineEdit*>(_number))->subtype();
  return CRMAcctLineEdit::Crmacct;
}

CRMAcctLineEdit::CRMAcctLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "crmacct", "crmacct_id", "crmacct_number", "crmacct_name", 0, 0, pName)
{
    setTitles(tr("CRM Account"), tr("CRM Accounts"));
    setSubtype(Crmacct);
}

VirtualList* CRMAcctLineEdit::listFactory()
{
  return new CRMAcctList(this);
}

VirtualSearch* CRMAcctLineEdit::searchFactory()
{
  return new CRMAcctSearch(this);
}

///////////////////////////////////////////////////////////////////////

CRMAcctInfoAction* CRMAcctLineEdit::_crmacctInfoAction = 0;

void CRMAcctLineEdit::sInfo()
{
  if (_crmacctInfoAction)
    _crmacctInfoAction->crmacctInformation(this, id());
}

void CRMAcctLineEdit::setSubtype(CRMAcctSubtype subtype)
{
  _subtype = subtype;
  //TODO: refigure everything about this line edit, including find the id for the current text
}

CRMAcctLineEdit::CRMAcctSubtype CRMAcctLineEdit::subtype() const
{
  return _subtype;
}

///////////////////////////////////////////////////////////////////////

CRMAcctList::CRMAcctList(QWidget* pParent, const char* pName, bool, Qt::WFlags pFlags) :
  VirtualList(pParent, pFlags)
{
  _parent = pParent;

  if (!pName)
    setName("CRMAcctList");

  _listTab->setColumnCount(0);

  _listTab->addColumn(tr("Number"),      80, Qt::AlignCenter );
  _listTab->addColumn(tr("Name"),        75, Qt::AlignLeft   );
  _listTab->addColumn(tr("First"),      100, Qt::AlignLeft   );
  _listTab->addColumn(tr("Last"),       100, Qt::AlignLeft   );
  _listTab->addColumn(tr("Phone"),      100, Qt::AlignLeft   );
  _listTab->addColumn(tr("Address"),    100, Qt::AlignLeft|Qt::AlignTop );
  _listTab->addColumn(tr("City"),        75, Qt::AlignLeft   );
  _listTab->addColumn(tr("State"),       50, Qt::AlignLeft   );
  _listTab->addColumn(tr("Country"),    100, Qt::AlignLeft   );
  _listTab->addColumn(tr("Postal Code"), 75, Qt::AlignLeft   );

  _showInactive = false;	// must be before inherits() checks

  if (_parent->inherits("CRMAcctCluster")) // handles Crmacct, Competitor, Partner, Prospect, Taxauth
    setSubtype((qobject_cast<CRMAcctCluster*>(_parent))->subtype());
  else if (_parent->inherits("CRMAcctLineEdit"))
    setSubtype((qobject_cast<CRMAcctLineEdit*>(_parent))->subtype());
  else if (_parent->inherits("CLineEdit") || _parent->inherits("CustCluster"))
  {
    CLineEdit::CLineEditTypes type = _parent->inherits("CLineEdit") ?
				  (qobject_cast<CLineEdit*>(_parent))->type() :
				  (qobject_cast<CustCluster*>(_parent))->type();
    switch (type)
    {
      case CLineEdit::AllCustomers:
	setSubtype(CRMAcctLineEdit::Cust);
	_showInactive = true;
	break;

      case CLineEdit::ActiveCustomers:
	setSubtype(CRMAcctLineEdit::Cust);
	_showInactive = false;
	break;

      case CLineEdit::AllProspects:
	setSubtype(CRMAcctLineEdit::Prospect);
	_showInactive = true;
	break;

      case CLineEdit::ActiveProspects:
	setSubtype(CRMAcctLineEdit::Prospect);
	_showInactive = false;
	break;

      case CLineEdit::AllCustomersAndProspects:
	setSubtype(CRMAcctLineEdit::CustAndProspect);
	_showInactive = true;
	break;

      case CLineEdit::ActiveCustomersAndProspects:
	setSubtype(CRMAcctLineEdit::CustAndProspect);
	_showInactive = false;
	break;

    }
  }
  else if (_parent->inherits("VendorLineEdit") || _parent->inherits("VendorCluster"))
    setSubtype(CRMAcctLineEdit::Vend);
  else
    setSubtype(CRMAcctLineEdit::Crmacct);

  resize(800, size().height());
}

void CRMAcctList::setId(const int id)
{
  _id = id;
  _listTab->setId(id);
}

void CRMAcctList::setShowInactive(const bool show)
{
  if (_showInactive != show)
  {
    _showInactive = show;
    sFillList();
  }
}

void CRMAcctList::setSubtype(const CRMAcctLineEdit::CRMAcctSubtype subtype)
{
  _subtype = subtype;

  switch (subtype)
  {
  case CRMAcctLineEdit::Cust:
    setCaption(tr("Search For Customer"));
    _query =
	"SELECT addr.*, "
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cust_id AS id, cust_number AS number, cust_name AS name,"
	"       cntct.* "
	"    FROM custinfo LEFT OUTER JOIN"
	"         cntct ON (cust_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE cust_active "
	"<? endif ?>"
	"ORDER BY number;"  ;
    break;

  case CRMAcctLineEdit::Prospect:
    setCaption(tr("Search For Prospect"));
    _query =
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       prospect_id AS id, prospect_number AS number, prospect_name AS name,"
	"       cntct.* "
	"    FROM prospect LEFT OUTER JOIN"
	"         cntct ON (prospect_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE prospect_active "
	"<? endif ?>"
	"ORDER BY number;"  ;
    break;

  case CRMAcctLineEdit::Taxauth:
    setCaption(tr("Search For Tax Authority"));
    _listTab->hideColumn(2);
    _listTab->hideColumn(3);
    _listTab->hideColumn(4);
    _query =
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       taxauth_id AS id, taxauth_code AS number, taxauth_name AS name,"
	"       '' AS cntct_first_name, '' AS cntct_last_name, "
	"       '' AS cntct_phone "
	"    FROM taxauth LEFT OUTER JOIN"
	"         addr ON (taxauth_addr_id=addr_id)"
	/*
	"<? if exists(\"activeOnly\") ?>"
	"WHERE taxauth_active "
	"<? endif ?>"
	*/
	"ORDER BY number;"  ;
    break;

  case CRMAcctLineEdit::Vend:
    _listTab->addColumn("Vend. Type", _itemColumn, Qt::AlignLeft);
    setCaption(tr("Search For Vendor"));
    _query =
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       vend_id AS id, vend_number AS number, vend_name AS name, vendtype_code AS type,"
	"       cntct.* "
	"    FROM vendtype JOIN vendinfo"
        "           ON (vend_vendtype_id=vendtype_id) LEFT OUTER JOIN"
	"         cntct ON (vend_cntct1_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (vend_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE vend_active "
	"<? endif ?>"
	"ORDER BY number;"  ;
    break;

  case CRMAcctLineEdit::CustAndProspect:
    setCaption(tr("Search For Customer or Prospect"));
    _query =
	"SELECT addr.*, "
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cust_id AS id, cust_number AS number, cust_name AS name,"
	"       cntct.* "
	"    FROM custinfo LEFT OUTER JOIN"
	"         cntct ON (cust_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE cust_active "
	"<? endif ?>"
	"UNION "
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       prospect_id AS id, prospect_number AS number, prospect_name AS name,"
	"       cntct.* "
	"    FROM prospect LEFT OUTER JOIN"
	"         cntct ON (prospect_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE prospect_active "
	"<? endif ?>"
	"ORDER BY number;"  ;
    break;

  case CRMAcctLineEdit::Crmacct:
  case CRMAcctLineEdit::Competitor:
  case CRMAcctLineEdit::Partner:
  default:
    setCaption(tr("Search For CRM Account"));
    _query =
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       crmacct_id AS id, crmacct_number AS number, crmacct_name AS name,"
	"       cntct.* "
	"    FROM crmacct LEFT OUTER JOIN"
	"         cntct ON (crmacct_cntct_id_1=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE crmacct_active "
	"<? endif ?>"
	"ORDER BY number;"  ;
    break;
  }

  sFillList();
}

void CRMAcctList::sFillList()
{
  MetaSQLQuery mql(_query);
  ParameterList params;
  if (! _showInactive)
    params.append("activeOnly");

  XSqlQuery fillq = mql.toQuery(params);

  XTreeWidgetItem* last = 0;
  _listTab->clear();

  while (fillq.next())
  {
    last = new XTreeWidgetItem(_listTab, last,
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
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2")
				.arg(__FILE__).arg(__LINE__),
			  fillq.lastError().databaseText());
    return;
  }
}

void CRMAcctList::sSearch(const QString& pTarget)
{
  QTreeWidgetItem *target = 0;

  for (int i = 0; i < _listTab->topLevelItemCount(); i++)
  {
    target = _listTab->topLevelItem(i);
    if (target == NULL ||
	target->text(0).startsWith(pTarget, Qt::CaseInsensitive) ||
	target->text(1).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }

  if (target)
  {
    _listTab->setCurrentItem(target);
    _listTab->scrollToItem(target);
  }
}

///////////////////////////////////////////////////////////////////////

CRMAcctSearch::CRMAcctSearch(QWidget* pParent, Qt::WindowFlags pFlags) :
  VirtualSearch(pParent, pFlags)
{
  // remove the stuff we won't use
  disconnect(_searchDescrip,	SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  selectorsLyt->removeWidget(_searchDescrip);
  delete _searchDescrip;

  _listTab->setColumnCount(0);

  _addressLit	    = new QLabel(tr("Primary Contact Address:"),this, "_addressLit");
  _searchStreet	    = new XCheckBox(tr("Street Address"), this);
  _searchCity	    = new XCheckBox(tr("City"),this);
  _searchState	    = new XCheckBox(tr("State"),this);
  _searchPostalCode = new XCheckBox(tr("Postal Code"),this);
  _searchCountry    = new XCheckBox(tr("Country"),this);
  _searchContact    = new XCheckBox(tr("Contact Name"),this);
  _searchPhone	    = new XCheckBox(tr("Contact Phone #"),this);
  _showInactive	    = new QCheckBox(tr("Show Inactive"),this, "_showInactive");
  _searchCombo      = new XCheckBox(tr("Search Combo"),this);
  _comboCombo       = new XComboBox(this, "_comboCombo");
  
  _searchStreet->setObjectName("_searchStreet");
  _searchCity->setObjectName("_searchCity");
  _searchState->setObjectName("_searchState");
  _searchPostalCode->setObjectName("_searchPostalCode");
  _searchCountry->setObjectName("_searchCountry");
  _searchContact->setObjectName("_searchContact");
  _searchPhone->setObjectName("_searchPhone");
  _searchCombo->setObjectName("_searchCombo");

  selectorsLyt->removeWidget(_searchName);
  selectorsLyt->removeWidget(_searchNumber);
  selectorsLyt->addWidget(new QLabel(tr("Search through:"), this, "_searchLit"),
						0, 0);
  selectorsLyt->addWidget(_searchNumber,	1, 0);
  selectorsLyt->addWidget(_searchName,		2, 0);
  selectorsLyt->addWidget(_searchContact,	1, 1);
  selectorsLyt->addWidget(_searchPhone,		2, 1);
  selectorsLyt->addWidget(_addressLit,		0, 2);
  selectorsLyt->addWidget(_searchStreet,	1, 2);
  selectorsLyt->addWidget(_searchCity,		2, 2);
  selectorsLyt->addWidget(_searchState,		3, 2);
  selectorsLyt->addWidget(_searchPostalCode,	4, 2);
  selectorsLyt->addWidget(_searchCombo,         5, 0);
  selectorsLyt->addWidget(_comboCombo,          5, 1);
  selectorsLyt->addWidget(_searchCountry,	5, 2);
  selectorsLyt->addWidget(_showInactive,	5, 3);

  _listTab->addColumn(tr("Number"),      80, Qt::AlignCenter );
  _listTab->addColumn(tr("Name"),        75, Qt::AlignLeft   );
  _listTab->addColumn(tr("First"),      100, Qt::AlignLeft   );
  _listTab->addColumn(tr("Last"),       100, Qt::AlignLeft   );
  _listTab->addColumn(tr("Phone"),      100, Qt::AlignLeft   );
  _listTab->addColumn(tr("Address"),    100, Qt::AlignLeft|Qt::AlignTop );
  _listTab->addColumn(tr("City"),        75, Qt::AlignLeft   );
  _listTab->addColumn(tr("State"),       50, Qt::AlignLeft   );
  _listTab->addColumn(tr("Country"),    100, Qt::AlignLeft   );
  _listTab->addColumn(tr("Postal Code"), 75, Qt::AlignLeft   );

  setTabOrder(_search,		_searchNumber);
  setTabOrder(_searchNumber,	_searchName);
  setTabOrder(_searchName,	_searchContact);
  setTabOrder(_searchContact,	_searchPhone);
  setTabOrder(_searchPhone,	_searchStreet);
  setTabOrder(_searchStreet,	_searchCity);
  setTabOrder(_searchCity,	_searchState);
  setTabOrder(_searchState,	_searchPostalCode);
  setTabOrder(_searchPostalCode,_searchCountry);
  setTabOrder(_searchCountry,	_searchCombo);
  setTabOrder(_searchCombo,     _comboCombo);
  setTabOrder(_comboCombo,      _showInactive);
  setTabOrder(_showInactive,	_listTab);
  setTabOrder(_listTab,		_select);
  setTabOrder(_select,		_close);
  setTabOrder(_close,		_search);

  resize(800, size().height());
  
  _parent = pParent;
  setObjectName("crmacctSearch");
  if (_parent->inherits("CRMAcctCluster")) // handles Crmacct, Competitor, Partner, Prospect, Taxauth
    setSubtype((qobject_cast<CRMAcctCluster*>(_parent))->subtype());
  else if (_parent->inherits("CRMAcctLineEdit"))
    setSubtype((qobject_cast<CRMAcctLineEdit*>(_parent))->subtype());
  else if (_parent->inherits("CLineEdit") || _parent->inherits("CustCluster"))
  {
    CLineEdit::CLineEditTypes type = _parent->inherits("CLineEdit") ?
				  (qobject_cast<CLineEdit*>(_parent))->type() :
				  (qobject_cast<CustCluster*>(_parent))->type();
    switch (type)
    {
      case CLineEdit::AllCustomers:
	setSubtype(CRMAcctLineEdit::Cust);
	_showInactive->setChecked(true);
	break;

      case CLineEdit::ActiveCustomers:
	setSubtype(CRMAcctLineEdit::Cust);
	_showInactive->setChecked(false);
	break;

      case CLineEdit::AllProspects:
	setSubtype(CRMAcctLineEdit::Prospect);
	_showInactive->setChecked(true);
	break;

      case CLineEdit::ActiveProspects:
	setSubtype(CRMAcctLineEdit::Prospect);
	_showInactive->setChecked(false);
	break;

      case CLineEdit::AllCustomersAndProspects:
	setSubtype(CRMAcctLineEdit::CustAndProspect);
	_showInactive->setChecked(true);
	break;

      case CLineEdit::ActiveCustomersAndProspects:
	setSubtype(CRMAcctLineEdit::CustAndProspect);
	_showInactive->setChecked(false);
	break;

    }
  }
  else if (_parent->inherits("VendorCluster")	||
	   _parent->inherits("VendorInfo")	||
	   _parent->inherits("VendorLineEdit"))
  {
    setSubtype(CRMAcctLineEdit::Vend);
  }
  else
    setSubtype(CRMAcctLineEdit::Crmacct);

  // do this late so the constructor can set defaults without triggering queries
  connect(_search,	 SIGNAL(lostFocus()),	this, SLOT(sFillList()));
  connect(_searchStreet, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchCity,   SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchState,  SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchPostalCode,SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchCountry,SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchContact,SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchPhone,	 SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchCombo,  SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_comboCombo,   SIGNAL(newID(int)),    this, SLOT(sFillList()));

  _search->setFocus();
}

void CRMAcctSearch::setId(const int id)
{
  _id = id;
  _listTab->setId(id);
}

void CRMAcctSearch::setShowInactive(const bool show)
{
  if (show != _showInactive->isChecked())
    _showInactive->setChecked(show);
}

void CRMAcctSearch::setSubtype(const CRMAcctLineEdit::CRMAcctSubtype subtype)
{
  _subtype = subtype;

  if(subtype != CRMAcctLineEdit::Vend)
  {
    _searchCombo->hide();
    _comboCombo->hide();
  }
 

  switch (subtype)
  {
  case CRMAcctLineEdit::Cust:
    setCaption(tr("Search For Customer"));
    _searchNumber->setText(tr("Customer Number"));
    _searchName->setText(tr("Customer Name"));
    _searchContact->setText(tr("Billing Contact Name"));
    _searchPhone->setText(tr("Billing Contact Phone #"));
    _addressLit->setText(tr("Billing Contact Address:"));
    break;

  case CRMAcctLineEdit::Prospect:
    setCaption(tr("Search For Prospect"));
    _searchNumber->setText(tr("Prospect Number"));
    _searchName->setText(tr("Prospect Name"));
    break;

  case CRMAcctLineEdit::Taxauth:
    setCaption(tr("Search For Tax Authority"));
    _searchNumber->setText(tr("Tax Authority Code"));
    _searchName->setText(tr("Tax Authority Name"));
    _searchContact->setVisible(false);
    _searchPhone->setVisible(false);
    _addressLit->setText(tr("Tax Authority Address:"));
    _listTab->hideColumn(2);
    _listTab->hideColumn(3);
    _listTab->hideColumn(4);
    _showInactive->setVisible(false);
    break;

  case CRMAcctLineEdit::Vend:
    setCaption(tr("Search For Vendor"));
    _searchCombo->setText(tr("Vendor Type:"));
    _comboCombo->setType(XComboBox::VendorTypes);
    _searchNumber->setText(tr("Vendor Number"));
    _searchName->setText(tr("Vendor Name"));
    _addressLit->setText(tr("Main Address:"));
    _listTab->addColumn("Vend. Type", _itemColumn, Qt::AlignLeft);
    break;

  case CRMAcctLineEdit::CustAndProspect:
    setCaption(tr("Search For Customer or Prospect"));
    _searchNumber->setText(tr("Number"));
    _searchName->setText(tr("Name"));
    _searchContact->setText(tr("Billing or Primary Contact Name"));
    _searchPhone->setText(tr("Billing or Primary Contact Phone #"));
    _addressLit->setText(tr("Billing or Primary Contact Address:"));
    break;

  case CRMAcctLineEdit::Crmacct:
  case CRMAcctLineEdit::Competitor:
  case CRMAcctLineEdit::Partner:
  default:
    setCaption(tr("Search For CRM Account"));
    _searchNumber->setText(tr("CRM Account Number"));
    _searchName->setText(tr("CRM Account Name"));
    _searchContact->setText(tr("Primary Contact Name"));
    _searchPhone->setText(tr("Primary Contact Phone #"));
    _addressLit->setText(tr("Primary Contact Address:"));
    break;
  }

  sFillList();
}

void CRMAcctSearch::sFillList()
{
  _listTab->clear();
  if (_search->text().stripWhiteSpace().length() == 0)
    return;

  QString sql;

  sql = "<? if exists(\"custAndProspect\") ?>"
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
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
	"    <? if exists(\"searchContactName\") ?>"
	"       OR (UPPER(cntct_first_name || ' ' || cntct_last_name) "
	"                 ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchPhone\") ?>"
	"       OR (UPPER(cntct_phone || ' ' || cntct_phone2 || ' ' || "
	"                 cntct_fax) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchStreetAddr\") ?>"
	"       OR (UPPER(addr_line1 || ' ' || addr_line2 || ' ' || "
	"                 addr_line3) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchCity\") ?>"
	"       OR (UPPER(addr_city) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchState\") ?>"
	"       OR (UPPER(addr_state) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchPostalCode\") ?>"
	"       OR (UPPER(addr_postalcode) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchCountry\") ?>"
	"       OR (UPPER(addr_country) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	") "
	"UNION "
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
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
	"    <? if exists(\"searchContactName\") ?>"
	"       OR (UPPER(cntct_first_name || ' ' || cntct_last_name) "
	"                 ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchPhone\") ?>"
	"       OR (UPPER(cntct_phone || ' ' || cntct_phone2 || ' ' || "
	"                 cntct_fax) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchStreetAddr\") ?>"
	"       OR (UPPER(addr_line1 || ' ' || addr_line2 || ' ' || "
	"                 addr_line3) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchCity\") ?>"
	"       OR (UPPER(addr_city) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchState\") ?>"
	"       OR (UPPER(addr_state) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchPostalCode\") ?>"
	"       OR (UPPER(addr_postalcode) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchCountry\") ?>"
	"       OR (UPPER(addr_country) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? elseif exists(\"crmacct\") ?>"
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
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
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
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
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
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
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
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
	"SELECT addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       vend_id AS id, vend_number AS number, vend_name AS name, vendtype_code AS type,"
	"       cntct.* "
	"    FROM vendtype, vendinfo LEFT OUTER JOIN"
	"         cntct ON (vend_cntct1_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (vend_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> vend_active AND <? endif ?>"
	"      (vend_vendtype_id=vendtype_id) "
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

    case CRMAcctLineEdit::CustAndProspect:
      params.append("custAndProspect");
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
  XTreeWidgetItem *last = 0;

  while (fillq.next())
  {
    last = new XTreeWidgetItem(_listTab, last,
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
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2")
				.arg(__FILE__).arg(__LINE__),
			  fillq.lastError().databaseText());
    return;
  }
}
