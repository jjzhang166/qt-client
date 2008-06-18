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

#include "contact.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include "characteristicAssignment.h"
#include "contactcluster.h"
#include "crmaccount.h"
#include "customer.h"
#include "employee.h"
#include "inputManager.h"
#include "prospect.h"
#include "shipTo.h"
#include "storedProcErrorLookup.h"
#include "vendor.h"
#include "vendorAddress.h"
#include "warehouse.h"

contact::contact(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close,		  SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_deleteCharacteristic,  SIGNAL(clicked()), this, SLOT(sDeleteCharass()));
  connect(_detachUse,		  SIGNAL(clicked()), this, SLOT(sDetachUse()));
  connect(_editCharacteristic,    SIGNAL(clicked()), this, SLOT(sEditCharass()));
  connect(_editUse,  		  SIGNAL(clicked()), this, SLOT(sEditUse()));
  connect(_newCharacteristic,     SIGNAL(clicked()), this, SLOT(sNewCharass()));
  connect(_save,	          SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_uses, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateUsesMenu(QMenu*)));
  connect(_uses, SIGNAL(valid(bool)), this, SLOT(sHandleValidUse(bool)));
  connect(_viewUse,	          SIGNAL(clicked()), this, SLOT(sViewUse()));
  connect(omfgThis,        SIGNAL(vendorsUpdated()), this, SLOT(sFillList()));
  connect(omfgThis,      SIGNAL(prospectsUpdated()), this, SLOT(sFillList()));
  connect(omfgThis,     SIGNAL(warehousesUpdated()), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(crmAccountsUpdated(int)), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(customersUpdated(int,bool)), this, SLOT(sFillList()));

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft );
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft );

  _uses->addColumn(tr("Used by"),         100, Qt::AlignLeft  );
  _uses->addColumn(tr("Number"), _orderColumn, Qt::AlignLeft  );
  _uses->addColumn(tr("Name"),	           -1, Qt::AlignLeft  );
  _uses->addColumn(tr("Role"),	           -1, Qt::AlignLeft  );
  _uses->addColumn(tr("Active"),    _ynColumn, Qt::AlignCenter);

  _activeCache = false;
}

contact::~contact()
{
    // no need to delete child widgets, Qt does it all for us
}

void contact::languageChange()
{
    retranslateUi(this);
}

enum SetResponse contact::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cntct_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _contact->setId(param.toInt());
    _comments->setId(_contact->id());
    sPopulate();
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _contact->setAccount(param.toInt());
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _contact->setFirst("Contact" + QDateTime::currentDateTime().toString());
      int cntctSaveResult = _contact->save(AddressCluster::CHANGEONE);
      if (cntctSaveResult < 0)
      {
	systemError(this, tr("There was an error creating a new contact (%).\n"
			     "Check the database server log for errors.")
			  .arg(cntctSaveResult),
		    __FILE__, __LINE__);
	return UndefinedError;
      }
      _comments->setId(_contact->id());
      _contact->setFirst("");

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));

      _contact->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _save->hide();
      _close->setText(tr("&Close"));

      _contact->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _newCharacteristic->setEnabled(FALSE);
      _editCharacteristic->setEnabled(FALSE);
      _deleteCharacteristic->setEnabled(FALSE);

      _close->setFocus();
    }
  }

  return NoError;
}

void contact::sPopulateUsesMenu(QMenu* pMenu)
{
  int menuItem;
  QString editStr = tr("Edit...");
  QString viewStr = tr("View...");
  QString detachStr = tr("Detach");

  switch (_uses->altId())
  {
    case 1:
    case 2:
      menuItem = pMenu->insertItem(editStr, this, SLOT(sEditCRMAccount()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCRMAccounts"));
      menuItem = pMenu->insertItem(viewStr, this, SLOT(sViewCRMAccount()));
      pMenu->setItemEnabled(menuItem, _privileges->check("ViewCRMAccounts"));
      menuItem = pMenu->insertItem(detachStr, this, SLOT(sDetachUse()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCRMAccounts"));
      break;

    case 3:
    case 4:
      menuItem = pMenu->insertItem(editStr, this, SLOT(sEditCustomer()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCustomerMasters"));
      menuItem = pMenu->insertItem(viewStr, this, SLOT(sViewCustomer()));
      pMenu->setItemEnabled(menuItem, _privileges->check("ViewCustomerMasters"));
      menuItem = pMenu->insertItem(detachStr, this, SLOT(sDetachUse()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCustomerMasters"));
      break;

    case 5:
    case 6:
      menuItem = pMenu->insertItem(editStr, this, SLOT(sEditVendor()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainVendors"));
      menuItem = pMenu->insertItem(viewStr, this, SLOT(sViewVendor()));
      pMenu->setItemEnabled(menuItem, _privileges->check("ViewVendors"));
      menuItem = pMenu->insertItem(detachStr, this, SLOT(sDetachUse()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainVendors"));
      break;

    case 7:
      menuItem = pMenu->insertItem(editStr, this, SLOT(sEditProspect()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainProspects"));
      menuItem = pMenu->insertItem(viewStr, this, SLOT(sViewProspect()));
      pMenu->setItemEnabled(menuItem, _privileges->check("ViewProspects"));
      menuItem = pMenu->insertItem(detachStr, this, SLOT(sDetachUse()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainProspects"));
      break;

    case 8:
      menuItem = pMenu->insertItem(editStr, this, SLOT(sEditShipto()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainShiptos"));
      menuItem = pMenu->insertItem(viewStr, this, SLOT(sViewShipto()));
      pMenu->setItemEnabled(menuItem, _privileges->check("ViewShiptos"));
      menuItem = pMenu->insertItem(detachStr, this, SLOT(sDetachUse()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainShiptos"));
      break;

    case 9:
      menuItem = pMenu->insertItem(editStr, this, SLOT(sEditVendorAddress()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainVendorAddresses"));
      menuItem = pMenu->insertItem(viewStr, this, SLOT(sViewVendorAddress()));
      pMenu->setItemEnabled(menuItem, _privileges->check("ViewVendorAddresses"));
      menuItem = pMenu->insertItem(detachStr, this, SLOT(sDetachUse()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainVendorAddresses"));
      break;

    case 10:
      menuItem = pMenu->insertItem(editStr, this, SLOT(sEditWarehouse()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainWarehouses"));
      menuItem = pMenu->insertItem(viewStr, this, SLOT(sViewWarehouse()));
      pMenu->setItemEnabled(menuItem, _privileges->check("ViewWarehouses"));
      menuItem = pMenu->insertItem(detachStr, this, SLOT(sDetachUse()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainWarehouses"));
      break;

    case 11:
      menuItem = pMenu->insertItem(editStr, this, SLOT(sEditEmployee()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainEmployees"));
      menuItem = pMenu->insertItem(viewStr, this, SLOT(sViewEmployee()));
      pMenu->setItemEnabled(menuItem, _privileges->check("ViewEmployees"));
      menuItem = pMenu->insertItem(detachStr, this, SLOT(sDetachUse()));
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainEmployees"));

    default:
      break;
  }
}

void contact::sClose()
{
  if (_mode == cNew)
  {
    q.prepare("SELECT deleteContact(:cntct_id) AS returnVal;");
    q.bindValue(":cntct_id", _contact->id());
    q.exec();
    if (q.first())
    {
      int returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
	systemError(this, storedProcErrorLookup("deleteContact", returnVal),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  reject();
}

void contact::sSave()
{
  if (_activeCache && ! _contact->active())
  {
    q.prepare("SELECT EXISTS(SELECT 1 FROM crmacct WHERE(crmacct_active"
	      "                                   AND (crmacct_cntct_id_1=:id)"
	      "                                    OR (crmacct_cntct_id_2=:id))"
	      "        UNION SELECT 2 FROM custinfo WHERE(cust_active"
	      "                                   AND (cust_cntct_id=:id)"
	      "                                    OR (cust_corrcntct_id=:id))"
	      "        UNION SELECT 3 FROM vendinfo WHERE(vend_active"
	      "                                   AND (vend_cntct1_id=:id)"
	      "                                    OR (vend_cntct2_id=:id))"
	      "        UNION SELECT 4 FROM prospect WHERE (prospect_active"
	      "                                   AND (prospect_cntct_id=:id))"
	      "        UNION SELECT 5 FROM shiptoinfo WHERE (shipto_active"
	      "                                   AND (shipto_cntct_id=:id))"
	      "        UNION SELECT 6 FROM vendaddrinfo WHERE"
	      "                                   (vendaddr_cntct_id=:id)"
	      "        UNION SELECT 7 FROM whsinfo WHERE (warehous_active"
	      "                                   AND (warehous_cntct_id=:id))"
	      " ) AS inuse;");
    q.bindValue(":id", _contact->id());
    q.exec();
    if (q.first() && q.value("inuse").toBool())
    {
      QMessageBox::information(this, tr("Cannot make Contact inactive"),
			    tr("<p>You may not mark this Contact as not "
			       "Active when this person is a Contact "
			       "for an active CRM Account, Customer, "
			       "Vendor, or Prospect."));
      return;
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  /* save the address first so we can check for multiple uses, then save the
     contact with less error checking because this is contact maintenance
   */
  AddressCluster* addr = _contact->addressWidget();
  int saveResult = addr->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = 2;	// Cancel
    answer = QMessageBox::question(this, tr("Question Saving Address"),
		    tr("There are multiple Contacts sharing this Address.\n"
		       "What would you like to do?"),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);

    if (0 == answer)
      saveResult = addr->save(AddressCluster::CHANGEONE);
    else if (1 == answer)
      saveResult = addr->save(AddressCluster::CHANGEALL);
    else
      return;
  }
  if (saveResult < 0)	// check from errors for CHECK and CHANGE* saves
  {
    systemError(this, tr("There was an error saving this Address (%1).\n"
			 "Check the database server log for errors.")
		      .arg(saveResult),
		__FILE__, __LINE__);
    return;
  }

  _contact->setAddress(saveResult);
  _contact->setNotes(_notes->text());

  saveResult = _contact->save(AddressCluster::CHANGEALL);
  if (saveResult < 0)
  {
    systemError(this, tr("There was an error saving this Contact (%1).\n"
			 "Check the database server log for errors.")
		      .arg(saveResult),
		__FILE__, __LINE__);
    return;
  }

  done(_contact->id());
}

void contact::sPopulate()
{
  _notes->setText(_contact->notes());
  _activeCache = _contact->active();
  sFillList();
}

void contact::sNewCharass()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cntct_id", _contact->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contact::sEditCharass()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contact::sDeleteCharass()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void contact::sFillList()
{
  q.prepare( "SELECT charass_id, char_name, charass_value "
             "FROM charass, char "
             "WHERE ((charass_target_type='CNTCT')"
             " AND   (charass_char_id=char_id)"
             " AND   (charass_target_id=:cntct_id) ) "
             "ORDER BY char_name;" );
  q.bindValue(":cntct_id", _contact->id());
  q.exec();
  _charass->clear();
  _charass->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT crmacct_id AS id, 1 AS altId, :crmacct AS type,"
	    "       crmacct_number AS number,"
	    "       crmacct_name AS name, :primary AS role,"
	    "       formatBoolYN(crmacct_active) AS active"
	    "  FROM crmacct WHERE (crmacct_cntct_id_1=:id)"
            "UNION "
	    "SELECT crmacct_id AS id, 2 AS altId, :crmacct AS type,"
	    "       crmacct_number AS number,"
	    "       crmacct_name AS name, :secondary AS role,"
	    "       formatBoolYN(crmacct_active) AS active"
	    "  FROM crmacct WHERE (crmacct_cntct_id_2=:id)"
	    "UNION "
	    "SELECT cust_id AS id, 3 AS altId, :cust AS type,"
	    "       cust_number AS number,"
	    "       cust_name AS name, :billing AS role,"
	    "       formatBoolYN(cust_active) AS active"
	    "  FROM custinfo WHERE (cust_cntct_id=:id)"
	    "UNION "
	    "SELECT cust_id AS id, 4 AS altId, :cust AS type,"
	    "       cust_number AS number,"
	    "       cust_name AS name, :correspond AS role,"
	    "       formatBoolYN(cust_active) AS active"
	    "  FROM custinfo WHERE (cust_corrcntct_id=:id)"
	    "UNION "
	    "SELECT vend_id AS id, 5 AS altId, :vend AS type,"
	    "       vend_number AS number,"
	    "       vend_name AS name, :primary AS role,"
	    "       formatBoolYN(vend_active) AS active"
	    "  FROM vendinfo WHERE (vend_cntct1_id=:id)"
	    "UNION "
	    "SELECT vend_id AS id, 6 AS altId, :vend AS type,"
	    "       vend_number AS number,"
	    "       vend_name AS name, :secondary AS role,"
	    "       formatBoolYN(vend_active) AS active"
	    "  FROM vendinfo WHERE (vend_cntct2_id=:id)"
	    "UNION "
	    "SELECT prospect_id AS id, 7 AS altId, :prospect AS type,"
	    "       prospect_number AS number,"
	    "       prospect_name AS name, '' AS role,"
	    "       formatBoolYN(prospect_active) AS active"
	    "  FROM prospect WHERE (prospect_cntct_id=:id)"
	    "UNION "
	    "SELECT shipto_id AS id, 8 AS altId, :shipto AS type,"
	    "       shipto_num AS number,"
	    "       shipto_name AS name, '' AS role,"
	    "       formatBoolYN(shipto_active) AS active"
	    "  FROM shiptoinfo WHERE (shipto_cntct_id=:id)"
	    "UNION "
	    "SELECT vendaddr_id AS id, 9 AS altId, :vendaddr AS type,"
	    "       vendaddr_code AS number,"
	    "       vendaddr_name AS name, '' AS role,"
	    "       formatBoolYN(true) AS active"
	    "  FROM vendaddrinfo WHERE (vendaddr_cntct_id=:id)"
	    "UNION SELECT warehous_id AS id, 10 AS altId, :whs AS type,"
	    "       warehous_code AS number,"
	    "       warehous_descrip AS name, '' AS role,"
	    "       formatBoolYN(warehous_active) AS active"
	    "  FROM whsinfo WHERE (warehous_cntct_id=:id)"
	    "UNION SELECT emp_id AS id, 11 AS altId, :emp AS type,"
	    "       emp_code AS number,"
	    "       emp_number AS name, '' AS role,"
	    "       formatBoolYN(emp_active) AS active"
	    "  FROM emp WHERE (emp_cntct_id=:id)"
	    "ORDER BY type, number;");
  q.bindValue(":id",		_contact->id());
  q.bindValue(":primary",	tr("Primary Contact"));
  q.bindValue(":secondary",	tr("Secondary Contact"));
  q.bindValue(":billing",	tr("Billing Contact"));
  q.bindValue(":correspond",	tr("Correspondence Contact"));
  q.bindValue(":crmacct",	tr("CRM Account"));
  q.bindValue(":cust",		tr("Customer"));
  q.bindValue(":vend",		tr("Vendor"));
  q.bindValue(":prospect",	tr("Prospect"));
  q.bindValue(":shipto",	tr("Ship-To Address"));
  q.bindValue(":vendaddr",	tr("Vendor Address"));
  q.bindValue(":whs",		tr("Warehouse"));
  q.bindValue(":emp",		tr("Employee"));
  q.exec();
  _uses->populate(q, true);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void contact::sDetachUse()
{
  QString question;
  XSqlQuery detachq;
  switch (_uses->altId())
  {
    case 1:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Primary Contact for this CRM Account?");
      detachq.prepare("UPDATE crmacct SET crmacct_cntct_id_1 = NULL "
		      "WHERE (crmacct_id=:id);");
      break;
    case 2:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Secondary Contact for this CRM Account?");
      detachq.prepare("UPDATE crmacct SET crmacct_cntct_id_2 = NULL "
		      "WHERE (crmacct_id=:id);");
      break;

    case 3:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Billing Contact for this Customer?");
      detachq.prepare("UPDATE custinfo SET cust_cntct_id = NULL "
		      "WHERE (cust_id=:id);");
      break;
    case 4:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Correspondence Contact for this Customer?");
      detachq.prepare("UPDATE custinfo SET cust_corrcntct_id = NULL "
		      "WHERE (cust_id=:id);");
      break;

    case 5:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Primary Contact for this Vendor?");
      detachq.prepare("UPDATE vendinfo SET vend_cntct1_id = NULL "
		      "WHERE (vend_id=:id);");
      break;

    case 6:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Secondary Contact for this Vendor?");
      detachq.prepare("UPDATE vendinfo SET vend_cntct2_id = NULL "
		      "WHERE (vend_id=:id);");
      break;

    case 7:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Contact for this Prospect?");
      detachq.prepare("UPDATE prospect SET prospect_cntct_id = NULL "
		      "WHERE (prospect_id=:id);");
      break;

    case 8:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Contact for this Ship-To Address?");
      detachq.prepare("UPDATE shiptoinfo SET shipto_cntct_id = NULL "
		      "WHERE (shipto_id=:id);");
      break;

    case 9:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Contact for this Vendor Address?");
      detachq.prepare("UPDATE vendaddrinfo SET vendaddr_cntct_id = NULL "
		      "WHERE (vendaddr_id=:id);");
      break;

    case 10:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Contact for this Warehouse?");
      detachq.prepare("UPDATE whsinfo SET warehous_cntct_id = NULL "
		      "WHERE (warehous_id=:id);");
      break;

    case 11:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Contact for this Employee?");
      detachq.prepare("UPDATE emp SET emp_cntct_id = NULL "
		      "WHERE (emp_id=:id);");
      break;

    default:
      break;
  }

  if (! question.isEmpty() &&
      QMessageBox::question(this, tr("Detach Contact?"), question,
		    QMessageBox::Yes,
		    QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  detachq.bindValue(":id", _uses->id());
  detachq.exec();
  if (detachq.lastError().type() != QSqlError::None)
  {
    systemError(this, detachq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void contact::sEditUse()
{
  switch (_uses->altId())
  {
    case 1:
    case 2:
      sEditCRMAccount();
      break;

    case 3:
    case 4:
      sEditCustomer();
      break;

    case 5:
    case 6:
      sEditVendor();
      break;

    case 7:
      sEditProspect();
      break;

    case 8:
      sEditShipto();
      break;

    case 9:
      sEditVendorAddress();
      break;

    case 10:
      sEditWarehouse();
      break;

    case 11:
      sEditEmployee();
      break;

    default:
      break;
  }
}

void contact::sViewUse()
{
  switch (_uses->altId())
  {
    case 1:
    case 2:
      sViewCRMAccount();
      break;

    case 3:
    case 4:
      sViewCustomer();
      break;

    case 5:
    case 6:
      sViewVendor();
      break;

    case 7:
      sViewProspect();
      break;

    case 8:
      sViewShipto();
      break;

    case 9:
      sViewVendorAddress();
      break;

    case 10:
      sViewWarehouse();
      break;

    case 11:
      sViewEmployee();
      break;

    default:
      break;
  }
}

void contact::sHandleValidUse(bool valid)
{
  bool editPriv = (
		  (_uses->altId() == 1 && _privileges->check("MaintainCRMAccounts")) ||
		  (_uses->altId() == 2 && _privileges->check("MaintainCRMAccounts")) ||
		  (_uses->altId() == 3 && _privileges->check("MaintainCustomerMasters")) ||
		  (_uses->altId() == 4 && _privileges->check("MaintainCustomerMasters")) ||
		  (_uses->altId() == 5 && _privileges->check("MaintainVendors")) ||
		  (_uses->altId() == 6 && _privileges->check("MaintainVendors")) ||
		  (_uses->altId() == 7 && _privileges->check("MaintainProspects")) ||
		  (_uses->altId() == 8 && _privileges->check("MaintainShiptos")) ||
		  (_uses->altId() == 9 && _privileges->check("MaintainVendorAddresses")) ||
		  (_uses->altId() ==10 && _privileges->check("MaintainWarehouses")) ||
		  (_uses->altId() ==11 && _privileges->check("MaintainEmployees")) 
  );
  bool viewPriv = (
		  (_uses->altId() == 1 && _privileges->check("ViewCRMAccounts")) ||
		  (_uses->altId() == 2 && _privileges->check("ViewCRMAccounts")) ||
		  (_uses->altId() == 3 && _privileges->check("ViewCustomerMasters")) ||
		  (_uses->altId() == 4 && _privileges->check("ViewCustomerMasters")) ||
		  (_uses->altId() == 5 && _privileges->check("ViewVendors")) ||
		  (_uses->altId() == 6 && _privileges->check("ViewVendors")) ||
		  (_uses->altId() == 7 && _privileges->check("ViewProspects")) ||
		  (_uses->altId() == 8 && _privileges->check("ViewShiptos")) ||
		  (_uses->altId() == 9 && _privileges->check("ViewVendorAddresses")) ||
		  (_uses->altId() ==10 && _privileges->check("ViewWarehouses"))  ||
		  (_uses->altId() ==11 && _privileges->check("ViewEmployees")) 
  );

  disconnect(_uses, SIGNAL(itemSelected(int)), _editUse, SLOT(animateClick()));
  disconnect(_uses, SIGNAL(itemSelected(int)), _viewUse, SLOT(animateClick()));
  if (editPriv)
  {
    _detachUse->setEnabled(valid);
    _editUse->setEnabled(valid);
    _viewUse->setEnabled(valid);
    connect(_uses, SIGNAL(itemSelected(int)), (_mode == cEdit) ? _editUse : _viewUse, SLOT(animateClick()));
  }
  else if (viewPriv)
  {
    _detachUse->setEnabled(false);
    _editUse->setEnabled(false);
    _viewUse->setEnabled(true);
    connect(_uses, SIGNAL(itemSelected(int)), _viewUse, SLOT(animateClick()));
  }
  else
  {
    _detachUse->setEnabled(false);
    _editUse->setEnabled(false);
    _viewUse->setEnabled(false);
  }
}

void contact::sEditCRMAccount()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("crmacct_id",	_uses->id());
  crmaccount::doDialog(this, params);
}

void contact::sViewCRMAccount()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("crmacct_id",	_uses->id());
  crmaccount::doDialog(this, params);
}

void contact::sEditCustomer()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id",	_uses->id());
  customer *newdlg = new customer(0, "custForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewCustomer()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id",	_uses->id());
  customer *newdlg = new customer(0, "custForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditEmployee()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("emp_id",	_uses->id());
  employee newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewEmployee()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("emp_id",	_uses->id());
  employee newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditProspect()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("prospect_id",	_uses->id());
  prospect *newdlg = new prospect(0, "prospectForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewProspect()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prospect_id",	_uses->id());
  prospect *newdlg = new prospect(0, "prospectForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditShipto()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("shipto_id",	_uses->id());
  shipTo newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewShipto()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("shipto_id",	_uses->id());
  shipTo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditVendorAddress()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vendaddr_id",	_uses->id());
  vendorAddress newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewVendorAddress()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("vendaddr_id",	_uses->id());
  vendorAddress newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditVendor()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vend_id",	_uses->id());
  vendor *newdlg = new vendor(0, "vendorForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewVendor()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("vend_id",	_uses->id());
  vendor *newdlg = new vendor(0, "vendorForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditWarehouse()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("warehous_id",	_uses->id());
  warehouse newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewWarehouse()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("warehous_id",	_uses->id());
  warehouse newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}
