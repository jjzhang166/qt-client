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

#include "crmaccount.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "characteristicAssignment.h"
#include "contact.h"
#include "customer.h"
#include "incident.h"
#include "prospect.h"
#include "taxAuthority.h"
#include "todoItem.h"
#include "vendor.h"
#include "storedProcErrorLookup.h"
#include "dspCustomerInformation.h"
#include "opportunity.h"
#include "mqlutil.h"

crmaccount::crmaccount(QWidget* parent, Qt::WFlags fl)
    : QWidget(parent, fl)
{
  setupUi(this);

  q.prepare("SELECT usr_id "
	    "FROM usr "
	    "WHERE (usr_username=CURRENT_USER);");
  q.exec();
  if (q.first())
    _myUsrId = q.value("usr_id").toInt();
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    close();
  }

  connect(_activeTodoIncdt, SIGNAL(toggled(bool)), this, SLOT(sPopulateTodo()));
  connect(_attach,		SIGNAL(clicked()), this, SLOT(sAttach()));
  connect(_autoUpdateTodo,  SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate()));
  connect(_close,		SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_competitor,		SIGNAL(clicked()), this, SLOT(sCompetitor()));
//  connect(_competitorButton,	SIGNAL(clicked()), this, SLOT(sCompetitor()));
  connect(_completedTodoIncdt, SIGNAL(toggled(bool)), this, SLOT(sPopulateTodo()));
  connect(_contacts, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_customerButton,	SIGNAL(clicked()), this, SLOT(sCustomer()));
  connect(_deleteCharacteristic,SIGNAL(clicked()), this, SLOT(sDeleteCharacteristic()));
  connect(_deleteTodoIncdt,	SIGNAL(clicked()), this, SLOT(sDeleteTodoIncdt()));
  connect(_detach,		SIGNAL(clicked()), this, SLOT(sDetach()));
  connect(_edit,		SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_editCharacteristic,	SIGNAL(clicked()), this, SLOT(sEditCharacteristic()));
  connect(_editTodoIncdt,	SIGNAL(clicked()), this, SLOT(sEditTodoIncdt()));
  connect(_new,			SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_newCharacteristic,	SIGNAL(clicked()), this, SLOT(sNewCharacteristic()));
  connect(_newIncdt,		SIGNAL(clicked()), this, SLOT(sNewIncdt()));
  connect(_newTodo,		SIGNAL(clicked()), this, SLOT(sNewTodo()));
  connect(_partner,		SIGNAL(clicked()), this, SLOT(sPartner()));
//  connect(_partnerButton,	SIGNAL(clicked()), this, SLOT(sPartner()));
  connect(_prospectButton,	SIGNAL(clicked()), this, SLOT(sProspect()));
  connect(_save,		SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_showTodo,	    SIGNAL(toggled(bool)), this, SLOT(sPopulateTodo()));
  connect(_showIncdt,	    SIGNAL(toggled(bool)), this, SLOT(sPopulateTodo()));
  connect(_taxauthButton,	SIGNAL(clicked()), this, SLOT(sTaxAuth()));
  connect(_todo, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(sHandleTodoPrivs()));
  connect(_todo, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)), this, SLOT(sPopulateTodoMenu(QMenu*)));
  connect(_vendorButton,	SIGNAL(clicked()), this, SLOT(sVendor()));
  connect(_viewTodoIncdt,	SIGNAL(clicked()), this, SLOT(sViewTodoIncdt()));
  connect(omfgThis, SIGNAL(customersUpdated(int, bool)), this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(prospectsUpdated()),  this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(taxAuthsUpdated(int)),this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(vendorsUpdated()),    this, SLOT(sUpdateRelationships()));
  connect(_custInfoButton, SIGNAL(clicked()), this, SLOT(sCustomerInfo()));
  connect(_primaryButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_secondaryButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_allButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_customer, SIGNAL(toggled(bool)), this, SLOT(sCustomerToggled()));
  connect(_prospect, SIGNAL(toggled(bool)), this, SLOT(sProspectToggled()));
  connect(_oplist, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this, SLOT(sPopulateOplistMenu(QMenu*)));

  _contacts->addColumn(tr("First Name"),	 50, Qt::AlignLeft );
  _contacts->addColumn(tr("Last Name"),		 -1, Qt::AlignLeft );
  _contacts->addColumn(tr("Phone"),		100, Qt::AlignLeft );
  _contacts->addColumn(tr("Alternate"),		100, Qt::AlignLeft );
  _contacts->addColumn(tr("Fax"),		100, Qt::AlignLeft );
  _contacts->addColumn(tr("E-Mail"),		100, Qt::AlignLeft );
  _contacts->addColumn(tr("Web Address"),	100, Qt::AlignLeft );

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft );
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft );

  _todo->addColumn(tr("Type"),    _statusColumn, Qt::AlignCenter );	
  _todo->addColumn(tr("Seq"),	     _seqColumn, Qt::AlignRight);
  _todo->addColumn(tr("User"),      _userColumn, Qt::AlignLeft );
  _todo->addColumn(tr("Name"),		    100, Qt::AlignLeft );
  _todo->addColumn(tr("Description"),        -1, Qt::AlignLeft );
  _todo->addColumn(tr("Status"),  _statusColumn, Qt::AlignLeft );
  _todo->addColumn(tr("Due Date"),  _dateColumn, Qt::AlignLeft );
  _todo->addColumn(tr("Incident"), _orderColumn, Qt::AlignLeft );

  _oplist->addColumn(tr("Name"),        _itemColumn,     Qt::AlignLeft );
  _oplist->addColumn(tr("CRM Acct."),   _userColumn,     Qt::AlignLeft );
  _oplist->addColumn(tr("Owner"),       _userColumn,     Qt::AlignLeft );
  _oplist->addColumn(tr("Stage"),       _orderColumn,    Qt::AlignLeft );
  _oplist->addColumn(tr("Source"),      _orderColumn,    Qt::AlignLeft,   false );
  _oplist->addColumn(tr("Type"),        _orderColumn,    Qt::AlignLeft,   false );
  _oplist->addColumn(tr("Prob.%"),      _prcntColumn,    Qt::AlignCenter, false );
  _oplist->addColumn(tr("Amount"),      _moneyColumn,    Qt::AlignRight,  false );
  _oplist->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   false );
  _oplist->addColumn(tr("Target Date"), _dateColumn,     Qt::AlignLeft );
  _oplist->addColumn(tr("Actual Date"), _dateColumn,     Qt::AlignLeft,   false );

  Preferences _pref = Preferences(omfgThis->username());
  if (_pref.boolean("XCheckBox/forgetful"))
  {
    _active->setChecked(true);
    _activeTodoIncdt->setChecked(true);
    _showIncdt->setChecked(true);
    _showTodo->setChecked(true);
  }

  _mode		= cNew;
  _crmacctId    = -1;
  _competitorId	= -1;
  _custId       = -1;
  _partnerId    = -1;
  _prospectId   = -1;
  _taxauthId    = -1;
  _vendId       = -1;
  _comments->setId(-1);

  sHandleTodoPrivs();
  
  _primary->setMinimalLayout(FALSE);
  _primary->setAccountVisible(FALSE);
  _primary->setActiveVisible(FALSE);
  _secondary->setMinimalLayout(FALSE);
  _secondary->setAccountVisible(FALSE);
  _secondary->setActiveVisible(FALSE);
  
  sHandleAutoUpdate();
}

crmaccount::~crmaccount()
{
    // no need to delete child widgets, Qt does it all for us
}

void crmaccount::languageChange()
{
    retranslateUi(this);
}

enum SetResponse crmaccount::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  // if _modal then disable any widgets that lead to opening QMainWindows
  if (_modal)
  {
    _customerButton->setEnabled(false);
    _custInfoButton->setEnabled(false);
    _taxauthButton->setEnabled(false);
    _vendorButton->setEnabled(false);
    _prospectButton->setEnabled(false);
  }

  if (_mode == cView)
  {
    _customer->setEnabled(false);
    _prospect->setEnabled(false);
    _taxauth->setEnabled(false);
    _vendor->setEnabled(false);
    _partner->setEnabled(false);
    _competitor->setEnabled(false);
  }
  else
  {
    _customer->setEnabled(_privleges->check("MaintainCustomerMasters") && !_modal);
    _prospect->setEnabled(_privleges->check("MaintainProspects") && !_modal);
    _taxauth->setEnabled(_privleges->check("MaintainTaxAuthorities") && !_modal);
    _vendor->setEnabled(_privleges->check("MaintainVendors") && !_modal);
    _partner->setEnabled(_privleges->check("MaintainPartners"));
    _competitor->setEnabled(_privleges->check("MaintainCompetitorMasters"));
  }

  if (! _privleges->check("MaintainContacts") || _mode == cView)
    _edit->setText("View");
  _edit->setEnabled(_privleges->check("MaintainContacts"));
  _attach->setEnabled(_privleges->check("MaintainContacts"));
  _detach->setEnabled(_privleges->check("MaintainContacts"));

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacctId = param.toInt();
    sPopulate();
  }

  _modal = pParams.inList("modal");

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _number->setFocus();

      q.prepare("SELECT createCrmAcct('TEMPORARY' || (last_value + 1), '',"
                "		      false, 'O', NULL, NULL, NULL, NULL,"
                "		      NULL, NULL, NULL, NULL) AS result "
                "FROM crmacct_crmacct_id_seq;");
      q.bindValue(":crmacct_id", _crmacctId);
      q.exec();
      if (q.first())
      {
        _crmacctId = q.value("result").toInt();
        if (_crmacctId < 0)
        {
          QMessageBox::critical(this, tr("Error creating Initial Account"),
                storedProcErrorLookup("createCrmAcct", _crmacctId));
          _crmacctId = -1;
          return UndefinedError;
        }
        _comments->setId(_crmacctId);
      }
      else if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      _number->clear();

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _typeBox->setEnabled(FALSE);
      _primary->setEnabled(FALSE);
      _secondary->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _parentCrmacct->setEnabled(FALSE);
      _newCharacteristic->setEnabled(FALSE);
      _editCharacteristic->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _newIncdt->setEnabled(FALSE);
      _newTodo->setEnabled(FALSE);
      _deleteTodoIncdt->setEnabled(FALSE);
      _editTodoIncdt->setEnabled(FALSE);

      _attach->hide();
      _detach->hide();
      _save->hide();

      _close->setFocus();
    }
  }
  return NoError;
}

// similar code in address, customer, shipto, vendor, vendorAddress
int crmaccount::saveContact(ContactCluster* pContact)
{
  pContact->setAccount(_crmacctId);

  int answer = 2;	// Cancel
  int saveResult = pContact->save(AddressCluster::CHECK);

  if (-1 == saveResult)
    systemError(this, tr("There was an error saving a Contact (%1, %2).\n"
			 "Check the database server log for errors.")
		      .arg(pContact->label()).arg(saveResult),
		__FILE__, __LINE__);
  else if (-2 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("<p>There are multiple Contacts sharing this address (%1)."
		       " What would you like to do?")
		    .arg(pContact->label()),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
  else if (-10 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving %1").arg(pContact->label()),
		    tr("<p>Would you like to update the existing Contact or "
		       "create a new one?"),
		    tr("Create New"),
		    tr("Change Existing"),
		    tr("Cancel"),
		    2, 2);
  if (0 == answer)
    return pContact->save(AddressCluster::CHANGEONE);
  else if (1 == answer)
    return pContact->save(AddressCluster::CHANGEALL);

  return saveResult;
}

void crmaccount::sClose()
{
  if (_mode == cNew)
  {
    if (QMessageBox::question(this, tr("Close without saving?"),
			      tr("<p>Are you sure you want to close this window "
				 "without saving the new CRM Account?"),
			      QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
      return;

    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    struct {
      QString spName;
      int     spArg;
    } toDelete[] = {
      {	"deleteCustomer",	_custId },
      { "deleteProspect",	_prospectId },
      { "deleteVendor",		_vendId },
      { "deleteTaxAuthority",	_taxauthId },
      /* any others? */
      { "deleteCRMAccount",	_crmacctId }
    };

    q.exec("BEGIN;");

    q.prepare("SELECT MIN(detachContact(cntct_id, :crmacct_id)) AS returnVal "
	      "FROM cntct "
	      "WHERE (cntct_crmacct_id=:crmacct_id);");
    q.bindValue(":crmacct_id", _crmacctId);
    q.exec();
    if (q.first())
    {
      int returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
	rollback.exec();
	systemError(this, tr("Error detaching Contact from CRM Account (%1).")
			  .arg(returnVal), __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    for (unsigned int i = 0; i < sizeof(toDelete) / sizeof(toDelete[0]); i++)
    {
      if (toDelete[i].spArg <= 0)
	continue;

      q.prepare(QString("SELECT %1(%2) AS returnVal;")
			.arg(toDelete[i].spName)
			.arg(toDelete[i].spArg));
      q.exec();
      if (q.first())
      {
	int returnVal = q.value("returnVal").toInt();
	if (returnVal < 0)
	{
	  rollback.exec();
	  systemError(this, storedProcErrorLookup(toDelete[i].spName, returnVal),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	rollback.exec();
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    } // for each thing to delete

    q.exec("COMMIT;");
    omfgThis->sCrmAccountsUpdated(_crmacctId);
  } // if cNew

  close();
  if (_modal)
    parentWidget()->close();
}

void crmaccount::sSave()
{
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  QString incomplete = tr("<p>The %1 relationship is selected but no "
			  "appropriate data have been created. Either click the"
			  " %2 button to enter the data or unselect %1.");
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _number->text().stripWhiteSpace().length() == 0,
      tr("<p>You must enter a number for this CRM Account before saving it."),
      _number
    },
    { _name->text().stripWhiteSpace().length() == 0,
      tr("<p>You must enter a name for this CRM Account before saving it."),
      _name
    },
    { _customer->isChecked() && _custId <= 0,
      incomplete.arg(_customer->text()).arg(_customerButton->text()),
      _customerButton
    },
    { _prospect->isChecked() && _prospectId <= 0,
      incomplete.arg(_prospect->text()).arg(_prospectButton->text()),
      _prospectButton
    },
    { _vendor->isChecked() && _vendId <= 0,
      incomplete.arg(_vendor->text()).arg(_vendorButton->text()),
      _vendorButton
    },
    /*	nothing necessary for now
    { _partner->isChecked() && _partnerId <= 0,
      incomplete.arg(_partner->text()).arg(_partnerButton->text()),
      _partnerButton
    },
    { _competitor->isChecked() && _competitorId <= 0,
      incomplete.arg(_competitor->text()).arg(_competitorButton->text()),
      _competitorButton
    },
    */
    { _taxauth->isChecked() && _taxauthId <= 0,
      incomplete.arg(_taxauth->text()).arg(_taxauthButton->text()),
      _taxauthButton
    },
    { true, "", NULL }
  }; // error[]

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save CRM Account"),
			  error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  q.prepare( "SELECT crmacct_number "
	     "FROM crmacct "
	     "WHERE ((crmacct_number=:number)"
	     " AND   (crmacct_id<>:crmacct_id) );" );

  q.bindValue(":crmacct_id", _crmacctId);
  q.bindValue(":number", _number->text());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical(this, tr("Cannot Save CRM Account"),
			  storedProcErrorLookup("createCrmAcct", -1));
    _number->setFocus();
    return;
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  QString spName = "";
  int	  spArg = 0;
  int	  answer = QMessageBox::No;

  if (_prospectId > 0 && !_prospect->isChecked())
  {
    answer = QMessageBox::question(this, tr("Delete Prospect?"),
		    tr("<p>Are you sure you want to delete %1 as a Prospect?")
		      .arg(_number->text()),
		    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (QMessageBox::No == answer)
    {
      _prospect->setChecked(true);
      return;
    }
    spName = "deleteProspect";
    spArg = _prospectId;
  }

  if (_custId > 0 && !_customer->isChecked())
  {
    answer = QMessageBox::question(this, tr("Delete Customer?"),
		    tr("<p>Are you sure you want to delete %1 as a Customer?")
		      .arg(_number->text()),
		    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (QMessageBox::No == answer)
    {
      _customer->setChecked(true);
      return;
    }
    spName = "deleteCustomer";
    spArg = _custId;
  }

  if (_vendId > 0 && ! _vendor->isChecked())
  {
    answer = QMessageBox::question(this, tr("Delete Vendor?"),
		    tr("<p>Are you sure you want to delete %1 as a Vendor?")
		      .arg(_number->text()),
		    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (QMessageBox::No == answer)
    {
      _vendor->setChecked(true);
      return;
    }
  }

  if (_taxauthId > 0 && ! _taxauth->isChecked())
  {
    answer = QMessageBox::question(this, tr("Delete Tax Authority?"),
		    tr("<p>Are you sure you want to delete %1 as a Tax Authority?")
		      .arg(_number->text()),
		    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (QMessageBox::No == answer)
    {
      _taxauth->setChecked(true);
      return;
    }
  }
  
  q.exec("BEGIN;");
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  int returnVal = 0;

  returnVal = saveContact(_primary);
  if (returnVal < 0)
  {
    rollback.exec();
    return;
  }

  returnVal = saveContact(_secondary);
  if (returnVal < 0)
  {
    rollback.exec();
    return;
  }

  if (! spName.isEmpty())
  {
    q.exec(QString("SELECT %1(%2) AS returnVal;").arg(spName).arg(spArg));
    if (q.first())
    {
      returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup(spName, returnVal), __FILE__, __LINE__);
        return;
      }
    } 
    else if (q.lastError().type() != QSqlError::None)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    /*
    if (_neither->isChecked())
    {
      if (_prospectId > 0)
	_prospectId = -1;
      else if (_custId > 0)
	_custId = -1;
    }
    */
  }

  if (! _taxauth->isChecked() && _taxauthId > 0)
  {
    q.prepare("SELECT deleteTaxAuthority(:taxauthid) AS returnVal;");
    q.bindValue(":taxauthid", _taxauthId);
    q.exec();
    if (q.first())
    {
      returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("deleteTaxAuthority", returnVal), __FILE__, __LINE__);
        return;
      }
    } 
    else if (q.lastError().type() != QSqlError::None)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _taxauthId = -1;
  }

  if (! _vendor->isChecked() && _vendId > 0)
  {
    q.prepare("SELECT deleteVendor(:vendid) AS returnVal;");
    q.bindValue(":vendid", _vendId);
    q.exec();
    if (q.first())
    {
      returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("deleteVendor", returnVal), __FILE__, __LINE__);
        return;
      }
    } 
    else if (q.lastError().type() != QSqlError::None)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _vendId = -1;
  }

  if (saveNoErrorCheck() < 0)	// do the heavy lifting
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sCrmAccountsUpdated(_crmacctId);
  omfgThis->sCustomersUpdated(-1, TRUE);
  omfgThis->sProspectsUpdated();
  omfgThis->sVendorsUpdated();
  omfgThis->sTaxAuthsUpdated(-1);
  close();
  if (_modal)
    parentWidget()->close();
}

int crmaccount::saveNoErrorCheck()
{
  q.prepare("UPDATE crmacct "
	    "SET crmacct_id=:crmacct_id,"
	    "    crmacct_number=UPPER(:crmacct_number),"
	    "    crmacct_name=:crmacct_name,"
	    "    crmacct_parent_id=:crmacct_parent_id,"
	    "    crmacct_active=:crmacct_active,"
	    "    crmacct_type=:crmacct_type,"
	    "    crmacct_cust_id=:crmacct_cust_id,"
	    "    crmacct_competitor_id=:crmacct_competitor_id,"
	    "    crmacct_partner_id=:crmacct_partner_id,"
	    "    crmacct_prospect_id=:crmacct_prospect_id,"
	    "    crmacct_taxauth_id=:crmacct_taxauth_id,"
	    "    crmacct_vend_id=:crmacct_vend_id,"
	    "    crmacct_cntct_id_1=:crmacct_cntct_id_1,"
	    "    crmacct_cntct_id_2 =:crmacct_cntct_id_2,"
	    "    crmacct_notes=:crmacct_notes "
	    "WHERE (crmacct_id=:crmacct_id);" );
  
  q.bindValue(":crmacct_id",		_crmacctId);
  q.bindValue(":crmacct_number",	_number->text());
  q.bindValue(":crmacct_name",		_name->text());
  q.bindValue(":crmacct_active",	QVariant(_active->isChecked(), 0));
  q.bindValue(":crmacct_type",		_organization->isChecked() ? "O" : "I");
  q.bindValue(":crmacct_notes",		_notes->text());
  if (_custId > 0)	 q.bindValue(":crmacct_cust_id",	_custId);
  if (_competitorId > 0) q.bindValue(":crmacct_competitor_id",  _competitorId);
  if (_partnerId > 0)	 q.bindValue(":crmacct_partner_id",	_partnerId);
  if (_prospectId > 0)	 q.bindValue(":crmacct_prospect_id",    _prospectId);
  if (_taxauthId > 0)	 q.bindValue(":crmacct_taxauth_id",	_taxauthId);
  if (_vendor->isChecked())   q.bindValue(":crmacct_vend_id",	_vendId);
  if (_primary->id() > 0)   q.bindValue(":crmacct_cntct_id_1",	_primary->id());
  if (_secondary->id() > 0) q.bindValue(":crmacct_cntct_id_2",	_secondary->id());
  if (_parentCrmacct->id() > 0)
			q.bindValue(":crmacct_parent_id", _parentCrmacct->id());

  if (! q.exec())
  {
    return -1;
  }

  return 0;
}

void crmaccount::sNewCharacteristic()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("crmacct_id", _crmacctId);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sGetCharacteristics();
}

void crmaccount::sEditCharacteristic()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sGetCharacteristics();
}

void crmaccount::sDeleteCharacteristic()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();

  sGetCharacteristics();
}

void crmaccount::sGetCharacteristics()
{
  q.prepare( "SELECT charass_id, char_name, charass_value "
             "FROM charass, char "
             "WHERE ( (charass_target_type='CRMACCT')"
             " AND (charass_char_id=char_id)"
             " AND (charass_target_id=:crmacct_id) ) "
             "ORDER BY char_name;" );
  q.bindValue(":crmacct_id", _crmacctId);
  q.exec();
  _charass->populate(q);
}

void crmaccount::sPopulate()
{
  q.prepare( "SELECT * "
             "FROM crmacct "
             "WHERE (crmacct_id=:crmacct_id);" );
  q.bindValue(":crmacct_id", _crmacctId);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("crmacct_number").toString());
    _name->setText(q.value("crmacct_name").toString());
    _active->setChecked(q.value("crmacct_active").toBool());

    QString acctType = q.value("crmacct_type").toString();
    if (acctType == "O")
      _organization->setChecked(true);
    else if (acctType == "I")
      _individual->setChecked(true);
    else
      qWarning("crmaccount::sPopulate() - acctType '%s' incorrect", acctType.toLatin1().data());

    _custId	= q.value("crmacct_cust_id").toInt();
    _competitorId = q.value("crmacct_competitor_id").toInt();
    _partnerId	= q.value("crmacct_partner_id").toInt();
    _prospectId	= q.value("crmacct_prospect_id").toInt();
    _vendId	= q.value("crmacct_vend_id").toInt();
    _taxauthId	= q.value("crmacct_taxauth_id").toInt();
    _primary->setId(q.value("crmacct_cntct_id_1").toInt());
    _secondary->setId(q.value("crmacct_cntct_id_2").toInt());
    _notes->setText(q.value("crmacct_notes").toString());
    _parentCrmacct->setId(q.value("crmacct_parent_id").toInt());
    _comments->setId(_crmacctId);

    _customer->setChecked(_custId > 0);
    _customer->setDisabled(_customer->isChecked());
    _prospect->setChecked(_prospectId > 0);
    _prospect->setDisabled(_customer->isChecked());
    _taxauth->setChecked(_taxauthId > 0);
    _vendor->setChecked(_vendId > 0);
    _partner->setChecked(_partnerId > 0);
    _competitor->setChecked(_competitorId > 0);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sPopulateContacts();
  sGetCharacteristics();
  sPopulateTodo();
  sPopulateOplist();
}

void crmaccount::sPopulateContacts()
{
  q.prepare("SELECT cntct_id, cntct_first_name, "
	    "       cntct_last_name, cntct_phone, "
	    "       cntct_phone2, cntct_fax, cntct_email, cntct_webaddr "
	    "FROM cntct "
	    "WHERE (cntct_crmacct_id=:crmacct_id) "
	    "ORDER BY cntct_last_name, cntct_first_name;");
  q.bindValue(":crmacct_id", _crmacctId);
  q.exec();
  _contacts->clear();
  _contacts->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void crmaccount::sPopulateTodo()
{
  _todo->clear();

  if (! _showTodo->isChecked() && ! _showIncdt->isChecked())
    return;

  QString sql = "<? if exists(\"showTodo\") ?>"
		"SELECT todoitem_id AS id, todoitem_usr_id AS altId, "
		"       'T' AS type, todoitem_seq AS seq, "
		"       todoitem_name AS name, "
		"       firstLine(todoitem_description) AS descrip, "
		"       todoitem_status AS status, todoitem_due_date AS due, "
		"       usr_username AS usr, incdt_number AS incdt "
		"FROM usr, todoitem LEFT OUTER JOIN "
		"     incdt ON (incdt_id=todoitem_incdt_id) "
		"WHERE ( (todoitem_usr_id=usr_id)"
		"  AND   (todoitem_crmacct_id=<? value(\"crmacct_id\") ?>)"
		"  <? if not exists(\"completed\") ?>"
		"    AND   (todoitem_status != 'C')"
		"  <? endif ?>"
		"  <? if exists(\"active\") ?>AND (todoitem_active) <? endif ?>"
		"  <? if exists(\"usr_id\") ?>"
		"    AND (todoitem_usr_id=<? value(\"usr_id\") ?>)"
		"  <? endif ?>"
		"       ) "
		"  <? if exists(\"showIncidents\")?>"
		"  UNION "
		"  <? endif ?>"
		"<? endif ?>"
		"<? if exists(\"showIncidents\")?>"
		"SELECT incdt_id AS id, usr_id AS altId, "
		"       'I' AS type, NULL AS seq, "
		"       incdt_summary AS name, "
		"       firstLine(incdt_descrip) AS descrip, "
		"       incdt_status AS status,  NULL AS due, "
		"       incdt_assigned_username AS usr, incdt_number AS incdt "
		"FROM incdt LEFT OUTER JOIN usr ON (usr_username=incdt_assigned_username)"
		"WHERE ((incdt_crmacct_id=<? value(\"crmacct_id\") ?>)"
		"  <? if not exists(\"completed\") ?> "
		"   AND (incdt_status != 'L')"
		"  <? endif ?>"
		"       ) "
		"<? endif ?>"
		"ORDER BY incdt, seq ASC, usr;" ;

  ParameterList params;
  params.append("crmacct_id", _crmacctId);
  if (_showTodo->isChecked())
    params.append("showTodo");
  if (_showIncdt->isChecked())
    params.append("showIncidents");
  if (_activeTodoIncdt->isChecked())
    params.append("active");
  if (_completedTodoIncdt->isChecked())
    params.append("completed");
  if ((_privleges->check("MaintainPersonalTodoList") ||
       _privleges->check("ViewPersonalTodoList")) &&
      ! (_privleges->check("MaintainOtherTodoLists") ||
	 _privleges->check("ViewOtherTodoLists")) )
    params.append("usr_id", _myUsrId);

  MetaSQLQuery mql(sql);
  XSqlQuery itemQ = mql.toQuery(params);

  if (itemQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XTreeWidgetItem *last = 0;
  while (itemQ.next())
  {
    last = new XTreeWidgetItem(_todo, last,
			     itemQ.value("id").toInt(),
			     itemQ.value("altId").toInt(),
			     itemQ.value("type").toString(),
			     itemQ.value("seq").isNull() ? "" :
				    itemQ.value("seq").toString(),
			     itemQ.value("usr").toString(),
			     itemQ.value("name").toString(),
			     itemQ.value("descrip").toString(),
			     itemQ.value("status").toString(),
			     itemQ.value("due").isNull() ? "" :
				    itemQ.value("due").toString(),
			     itemQ.value("incdt").isNull() ? "" :
				    itemQ.value("incdt").toString());
    if (itemQ.value("status") != "C")
    {
      if (itemQ.value("due").toDate() < QDate::currentDate())
	last->setTextColor("red");
      else if (itemQ.value("due").toDate() > QDate::currentDate())
	last->setTextColor("green");
    }
  }
  sHandleTodoPrivs();
}

void crmaccount::sHandleTodoPrivs()
{
  bool newTodoPriv = ((cEdit == _mode || cNew == _mode) &&
		      (_privleges->check("MaintainPersonalTodoList") ||
		       _privleges->check("MaintainOtherTodoLists")) );

  bool editTodoPriv = (cEdit == _mode || cNew == _mode) && (
    (_myUsrId == _todo->altId() && _privleges->check("MaintainPersonalTodoList")) ||
    (_privleges->check("MaintainOtherTodoLists")) );

  bool viewTodoPriv =
    (_myUsrId == _todo->altId() && _privleges->check("ViewPersonalTodoList")) ||
    (_privleges->check("ViewOtherTodoLists"));

  _newTodo->setEnabled(newTodoPriv);
  _newIncdt->setEnabled((cEdit == _mode || cNew == _mode) &&
			_privleges->check("MaintainIncidents"));

  if (_todo->currentItem())
  {
    if (_todo->currentItem()->text(0) == "T")
      _editTodoIncdt->setEnabled(editTodoPriv);
    else
      _editTodoIncdt->setEnabled((cEdit == _mode || cNew == _mode) &&
				  _privleges->check("MaintainIncidents"));

    if (_todo->currentItem()->text(0) == "T")
      _viewTodoIncdt->setEnabled(viewTodoPriv);
    else
      _viewTodoIncdt->setEnabled(_privleges->check("ViewIncidents"));

    if (_todo->currentItem()->text(0) == "T")
      _deleteTodoIncdt->setEnabled(editTodoPriv);
    else
      _deleteTodoIncdt->setEnabled((cEdit == _mode || cNew == _mode) &&
				    _privleges->check("MaintainIncidents"));
  }
  else
  {
    _editTodoIncdt->setEnabled(false);
    _viewTodoIncdt->setEnabled(false);
    _deleteTodoIncdt->setEnabled(false);
  }
}

void crmaccount::sPopulateTodoMenu(QMenu *pMenu)
{
  int menuItem;

  bool newTodoPriv = ((cEdit == _mode || cNew == _mode) &&
		      (_privleges->check("MaintainPersonalTodoList") ||
		       _privleges->check("MaintainOtherTodoLists")) );

  bool editTodoPriv = (cEdit == _mode || cNew == _mode) && (
      (_myUsrId == _todo->altId() && _privleges->check("MaintainPersonalTodoList")) ||
      _privleges->check("MaintainOtherTodoLists"));

  bool viewTodoPriv =
      (_myUsrId == _todo->altId() && _privleges->check("ViewPersonalTodoList")) ||
      _privleges->check("ViewOtherTodoLists");

  menuItem = pMenu->insertItem(tr("New To-Do Item..."), this, SLOT(sNewTodo()));
  pMenu->setItemEnabled(menuItem, newTodoPriv);

  menuItem = pMenu->insertItem(tr("New Incident..."), this, SLOT(sNewIncdt()));
  pMenu->setItemEnabled(menuItem, (cEdit == _mode || cNew == _mode) &&
				  _privleges->check("MaintainIncidents"));

  if (_todo->currentItem())
  {
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEditTodoIncdt()));
    pMenu->setItemEnabled(menuItem,
			  _todo->currentItem()->text(0) == "T" ? editTodoPriv :
			       (cEdit == _mode || cNew == _mode) &&
				_privleges->check("MaintainIncidents") );

    menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sViewTodoIncdt()));
    pMenu->setItemEnabled(menuItem,
			  _todo->currentItem()->text(0) == "T" ? viewTodoPriv :
				_privleges->check("ViewIncidents"));

    menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDeleteTodoIncdt()));
    pMenu->setItemEnabled(menuItem,
			  _todo->currentItem()->text(0) == "T" ? editTodoPriv :
			      (cEdit == _mode || cNew == _mode) &&
				_privleges->check("MaintainIncidents"));
  }
}

void crmaccount::sCompetitor()
{
  _competitorId = (_competitor->isChecked() ? 1 : 0);
}

void crmaccount::sCustomer()
{
  ParameterList params;
  
  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (! _customer->isChecked())	// don't know how we got here
    return;

  QString confirm;

  if (_prospectId > 0)
  {
	if (QMessageBox::question(this, tr("Convert"),
		tr("<p>Are you sure you want to convert this "
		       "Prospect to a Customer?"),
		QMessageBox::Yes,
		QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
	return;
	
	params.append("prospect_id",	  _prospectId);
	params.append("mode","new");
	
  }
  else if (_custId <= 0)
  {
    params.append("crmacct_id",	  _crmacctId);
    params.append("mode","new");
  } 	
  else
  {
    params.append("cust_id",_custId);
    params.append("mode",		  (_mode == cView) ? "view" : "edit");
  }

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  
}

void crmaccount::sPartner()
{
  _partnerId = (_partner->isChecked() ? 1 : 0);
}

void crmaccount::sProspect()
{
  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (! _prospect->isChecked())	// don't know how we got here
    return;

  QString spName = "";
  int	  spArg = 0;

  if (_custId > 0)
  {
    spName = "convertCustomerToProspect";
    spArg = _custId;
    if (QMessageBox::question(this, tr("Convert"),
		    tr("<p>Are you sure you want to convert this Customer to "
		       "a Prospect and delete its Customer information?"),
		    QMessageBox::Yes,
		    QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;
  }
  else if (_prospectId <= 0)
  {
    spName = "createProspect";
    spArg = _crmacctId;
  }

  if (! spName.isEmpty())
  {
    q.exec(QString("SELECT %1(%2) AS returnVal;").arg(spName).arg(spArg));
    if (q.first())
    {
      int returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
	systemError(this, storedProcErrorLookup(spName, returnVal), __FILE__, __LINE__);
	return;
      }
      _custId = -1;
      _prospectId = returnVal;
    } 
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sProspectsUpdated();
    omfgThis->sCustomersUpdated(_custId, true);
  }

  ParameterList params;
  params.append("crmacct_id",	  _crmacctId);
  params.append("crmacct_number", _number->text());
  params.append("crmacct_name",	  _name->text());
  params.append("prospect_id",	  _prospectId);
  params.append("mode",		  (_mode == cView) ? "view" : "edit");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sTaxAuth()
{
  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("crmacct_id", _crmacctId);
  params.append("crmacct_number", _number->text());
  params.append("crmacct_name", _name->text());
  if (_taxauthId <= 0 && _privleges->check("MaintainTaxAuthorities") &&
      (_mode == cEdit || _mode == cNew))
    params.append("mode", "new");
  else if (_taxauthId <= 0)
    systemError(this, tr("No existing Tax Authorities to View"));
  else if (_taxauthId > 0 && _privleges->check("MaintainTaxAuthorities") &&
	    (_mode == cEdit || _mode == cNew))
  {
    params.append("taxauth_id", _taxauthId);
    params.append("mode", "edit");
  }
  else if (_taxauthId > 0 && _mode == cView)
  {
    params.append("taxauth_id", _taxauthId);
    params.append("mode", "view");
  }

  taxAuthority *newdlg = new taxAuthority();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sVendor()
{
  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("crmacct_id", _crmacctId);
  params.append("crmacct_number", _number->text());
  params.append("crmacct_name", _name->text());
  if (_vendId <= 0 && _privleges->check("MaintainVendors") &&
      (_mode == cEdit || _mode == cNew))
    params.append("mode", "new");
  else if (_vendId <= 0)
    systemError(this, tr("No existing Vendor to View"));
  else if (_vendId > 0 && _privleges->check("MaintainVendors") &&
	    (_mode == cEdit || _mode == cNew))
  {
    params.append("vend_id", _vendId);
    params.append("mode", "edit");
  }
  else if (_vendId > 0 && _mode == cView)
  {
    params.append("vend_id", _vendId);
    params.append("mode", "view");
  }

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sNew()
{
  ParameterList params;
  params.append("crmacct_id", _crmacctId);
  params.append("mode", "new");

  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  contact *newdlg = new contact();
  newdlg->set(params);
  if (newdlg->exec() != QDialog::Rejected)
    sPopulate();
}

void crmaccount::sEdit()
{
  ParameterList params;
  params.append("cntct_id", _contacts->id());
  params.append("crmacct_id", _crmacctId);

  if (_mode == cView && _contacts->id() > 0)
    params.append("mode", "view");
  else if (_privleges->check("MaintainContacts") && _contacts->id() > 0)
  {
    params.append("mode", "edit");
    params.append("cntct_id", _contacts->id());
  }
  else  if (_privleges->check("MaintainContacts") && _contacts->id() <= 0)
    params.append("mode", "new");

  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  contact *newdlg = new contact();
  newdlg->set(params);
  if (newdlg->exec() != QDialog::Rejected)
    sPopulate();
}

void crmaccount::sView()
{
  ParameterList params;
  params.append("cntct_id", _contacts->id());

  params.append("mode", "view");

  contact *newdlg = new contact();
  newdlg->set(params);
  if (newdlg->exec() != QDialog::Rejected)
    sPopulate();
}

void crmaccount::sAttach()
{
  ContactCluster attached(this, "attached");
  attached.sEllipses();
  if (attached.id() > 0)
  {
    int answer = QMessageBox::Yes;

    if (attached.crmAcctId() > 0 && attached.crmAcctId() != _crmacctId)
      answer = QMessageBox::question(this, tr("Detach Contact?"),
			    tr("<p>This Contact is currently attached to a "
			       "different CRM Account. Are you sure you want "
			       "to change the CRM Account for this person?"),
			    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (answer == QMessageBox::Yes)
    {
      q.prepare("SELECT attachContact(:cntct_id, :crmacct_id) AS returnVal;");
      q.bindValue(":cntct_id", attached.id());
      q.bindValue(":crmacct_id", _crmacctId);
      q.exec();
      if (q.first())
      {
	int returnVal = q.value("returnVal").toInt();
	if (returnVal < 0)
	{
	  systemError(this, storedProcErrorLookup("attachContact", returnVal),
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
    sPopulate();
  }
}

void crmaccount::sDetach()
{
  int answer = QMessageBox::question(this, tr("Detach Contact?"),
			tr("<p>Are you sure you want to detach this Contact "
			   "from this CRM Account?"),
			QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
  if (answer == QMessageBox::Yes)
  {
    q.prepare("SELECT detachContact(:cntct_id, :crmacct_id) AS returnVal;");
    q.bindValue(":cntct_id", _contacts->id());
    q.bindValue(":crmacct_id", _crmacctId);
    q.exec();
    if (q.first())
    {
      int returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
	systemError(this, tr("Error detaching Contact from CRM Account (%1).")
			  .arg(returnVal), __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sPopulate();
  }
}

void crmaccount::sNewTodo()
{
  ParameterList params;
  params.append("crmacct_id", _crmacctId);
  params.append("mode", "new");

  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  todoItem newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sPopulateTodo();
}

void crmaccount::sNewIncdt()
{
  ParameterList params;
  params.append("crmacct_id", _crmacctId);
  params.append("mode", "new");

  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  incident newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sPopulateTodo();
}

void crmaccount::sEditTodoIncdt()
{
  if (_todo->currentItem()->text(0) == "T")
    sEditTodo();
  else
    sEditIncdt();
}

void crmaccount::sEditTodo()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", _todo->id());

  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  todoItem newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sPopulateTodo();
}

int crmaccount::getIncidentId()
{
  int returnVal = -1;

  if (_todo->currentItem()->text(0) == "I")
    returnVal = _todo->id();
  else if (! _todo->currentItem()->text(7).isEmpty())
  {
    XSqlQuery incdt;
    incdt.prepare("SELECT incdt_id FROM incdt WHERE (incdt_number=:number);");
    incdt.bindValue(":number", _todo->currentItem()->text(7).toInt());
    if (incdt.exec() && incdt.first())
     returnVal = incdt.value("incdt_id").toInt();
    else if (incdt.lastError().type() != QSqlError::None)
      systemError(this, incdt.lastError().databaseText(), __FILE__, __LINE__);
  }

  return returnVal;
}

void crmaccount::sEditIncdt()
{
  ParameterList params;
  params.append("mode",     "edit");
  params.append("incdt_id", getIncidentId());

  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  incident newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sPopulateTodo();
}

void crmaccount::sDeleteTodoIncdt()
{
  if (_todo->currentItem()->text(0) == "T")
    sDeleteTodo();
  else
    sDeleteIncdt();
}

void crmaccount::sDeleteTodo()
{
  q.prepare("SELECT deleteTodoItem(:todoitem_id) AS result;");
  q.bindValue(":todoitem_id", _todo->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteTodoItem", result));
      return;
    }
    sPopulateTodo();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void crmaccount::sDeleteIncdt()
{
  q.prepare("SELECT deleteIncident(:incdt_id) AS result;");
  q.bindValue(":incdt_id", getIncidentId());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteIncident", result));
      return;
    }
    sPopulateTodo();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void crmaccount::sViewTodoIncdt()
{
  if (_todo->currentItem()->text(0) == "T")
    sViewTodo();
  else
    sViewIncdt();
}

void crmaccount::sViewTodo()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", _todo->id());

  todoItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void crmaccount::sViewIncdt()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", getIncidentId());

  incident newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void crmaccount::sUpdateRelationships()
{
  q.prepare( "SELECT * "
             "FROM crmacct "
             "WHERE (crmacct_id=:crmacct_id);" );
  q.bindValue(":crmacct_id", _crmacctId);
  q.exec();
  if (q.first())
  {
    _custId	= q.value("crmacct_cust_id").toInt();
    _competitorId = q.value("crmacct_competitor_id").toInt();
    _partnerId	= q.value("crmacct_partner_id").toInt();
    _prospectId	= q.value("crmacct_prospect_id").toInt();
    _taxauthId	= q.value("crmacct_taxauth_id").toInt();
    _vendId	= q.value("crmacct_vend_id").toInt();

    _customer->setChecked(_custId > 0);
    _customer->setDisabled(_customer->isChecked());
    _prospect->setChecked(_prospectId > 0);
    _prospect->setDisabled(_customer->isChecked());
    _taxauth->setChecked(_taxauthId > 0);
    _vendor->setChecked(_vendId > 0);
    _partner->setChecked(_partnerId > 0);
    _competitor->setChecked(_competitorId > 0);
    _custInfoButton->setEnabled(_custId > 0 && _customer->isChecked());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  

  sPopulateContacts();
}

void crmaccount::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("New..."), this, SLOT(sNew()), 0);
  if (!_privleges->check("MaintainContacts") || _mode == cView)
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privleges->check("MaintainContacts") || _mode == cView)
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  if (!_privleges->check("ViewContacts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Attach..."), this, SLOT(sAttach()), 0);
  if (!_privleges->check("MaintainContacts") || _mode == cView)
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Detach"), this, SLOT(sDetach()), 0);
  if (!_privleges->check("MaintainContacts") || _mode == cView)
    pMenu->setItemEnabled(menuItem, FALSE);
}

void crmaccount::sPopulateOplistMenu(QMenu *pMenu)
{
  int menuItem;

  bool editPriv = _privleges->check("MaintainOpportunities");
  bool viewPriv = _privleges->check("VeiwOpportunities") || editPriv;

  //menuItem = pMenu->insertItem(tr("New..."), this, SLOT(sOplistNew()), 0);
  //pMenu->setItemEnabled(menuItem, editPriv);

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sOplistEdit()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sOplistView()), 0);
  pMenu->setItemEnabled(menuItem, viewPriv);

  //menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sOplistDelete()), 0);
  //pMenu->setItemEnabled(menuItem, editPriv);
}

void crmaccount::doDialog(QWidget *parent, const ParameterList & pParams)
{
  QDialog newdlg(parent);
  QVBoxLayout * vbox = new QVBoxLayout(&newdlg);
  crmaccount * ci = new crmaccount(&newdlg);
  vbox->addWidget(ci);
  newdlg.setWindowTitle(ci->windowTitle());
  ParameterList params;
  params = pParams;
  params.append("modal");
  ci->set(params);
  newdlg.exec();
}

void crmaccount::sHandleAutoUpdate()
{
  if (_autoUpdateTodo)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sPopulateTodo()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sPopulateTodo()));
}

void crmaccount::sCustomerInfo()
{
  ParameterList params;
  params.append("cust_id", _custId);

  dspCustomerInformation *newdlg = new dspCustomerInformation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sHandleButtons()
{
  if (_primaryButton->isChecked())
    _widgetStack->setCurrentIndex(0);
  else if (_secondaryButton->isChecked())
    _widgetStack->setCurrentIndex(1);
  else
    _widgetStack->setCurrentIndex(2);
}

void crmaccount::sCustomerToggled()
{
  if (_customer->isChecked())
    _prospect->setChecked(FALSE);

  _custInfoButton->setEnabled(_custId > 0 && _customer->isChecked());
}

void crmaccount::sProspectToggled()
{
  if (_prospect->isChecked())
    _customer->setChecked(FALSE);
}

void crmaccount::sOplistView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("ophead_id", _oplist->id());

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void crmaccount::sOplistEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ophead_id", _oplist->id());

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sPopulateOplist();
}

void crmaccount::sPopulateOplist()
{
  MetaSQLQuery mql = mqlLoad(":/crm/account/FillOpListDetail.mql");

  ParameterList params;
  params.append("crmacct_id", _crmacctId);

  XSqlQuery itemQ = mql.toQuery(params);

  if (itemQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _oplist->populate(itemQ);
}
