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

#include "prospect.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QCloseEvent>

#include <openreports.h>

#include "salesOrder.h"
#include "storedProcErrorLookup.h"

prospect::prospect(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_deleteQuote,SIGNAL(clicked()),	this,	SLOT(sDeleteQuote()));
  connect(_editQuote,	SIGNAL(clicked()),	this,	SLOT(sEditQuote()));
  connect(_newQuote,	SIGNAL(clicked()),	this,	SLOT(sNewQuote()));
  connect(_number,	SIGNAL(lostFocus()),	this,	SLOT(sCheckNumber()));
  connect(_printQuote,SIGNAL(clicked()),	this,	SLOT(sPrintQuote()));
  connect(_quotes,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),	this,	SLOT(sPopulateQuotesMenu(QMenu*)));
  connect(_save,	SIGNAL(clicked()),	this,	SLOT(sSave()));
  connect(_viewQuote,	SIGNAL(clicked()),	this,	SLOT(sViewQuote()));
  connect(omfgThis,	SIGNAL(quotesUpdated(int, bool)), this, SLOT(sFillQuotesList()));

  if (_privileges->check("MaintainProspectMasters"))
    connect(_quotes, SIGNAL(itemSelected(int)), _editQuote, SLOT(animateClick()));
  else
    connect(_quotes, SIGNAL(itemSelected(int)), _viewQuote, SLOT(animateClick()));

  _prospectid = -1;
  _crmacct->setId(-1);

  _taxauth->setAllowNull(true);
  _taxauth->setType(XComboBox::TaxAuths);

  _quotes->addColumn(tr("Quote #"),          _orderColumn, Qt::AlignLeft );
  _quotes->addColumn(tr("Quote Date"),       _dateColumn,  Qt::AlignLeft );

  _NumberGen = -1;
}

prospect::~prospect()
{
  // no need to delete child widgets, Qt does it all for us
}

void prospect::languageChange()
{
  retranslateUi(this);
}

enum SetResponse prospect::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_name", &valid);
  if (valid)
    _name->setText(param.toString());

  param = pParams.value("crmacct_number", &valid);
  if (valid)
    _number->setText(param.toString());

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
  }

  param = pParams.value("prospect_id", &valid);
  if (valid)
  {
    _prospectid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      // prospects and customers share an id sequence
      q.exec("SELECT NEXTVAL('cust_cust_id_seq') AS prospect_id;");
      if (q.first())
      {
        _prospectid = q.value("prospect_id").toInt();
	_number->setFocus();
      }
      else
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      if(((_metrics->value("CRMAccountNumberGeneration") == "A") ||
          (_metrics->value("CRMAccountNumberGeneration") == "O"))
       && _number->text().isEmpty() )
      {
        q.exec("SELECT fetchCRMAccountNumber() AS number;");
        if (q.first())
        {
          _number->setText(q.value("number"));
          _NumberGen = q.value("number").toInt();
        }


      }

    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _contact->setEnabled(FALSE);
      _taxauth->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _newQuote->setEnabled(FALSE);
      _save->hide();

      disconnect(_quotes, SIGNAL(itemSelected(int)), _editQuote, SLOT(animateClick(QMenu*)));
      disconnect(_quotes, SIGNAL(valid(bool)),       _editQuote, SLOT(setEnabled(bool)));
      disconnect(_quotes, SIGNAL(valid(bool)),       _editQuote, SLOT(setEnabled(bool)));
      disconnect(_quotes, SIGNAL(valid(bool)),     _deleteQuote, SLOT(setEnabled(bool)));

      connect(_quotes, SIGNAL(itemSelected(int)), _viewQuote, SLOT(animateClick(QMenu*)));

      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  if(_metrics->value("CRMAccountNumberGeneration") == "A")
    _number->setEnabled(FALSE);
  
  return NoError;
}

// similar code in address, customer, shipto, vendor, vendorAddress, ...
int prospect::saveContact(ContactCluster* pContact)
{
  pContact->setAccount(_crmacct->id());

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
		    tr("There are multiple Contacts sharing this address (%1).\n"
		       "What would you like to do?")
		    .arg(pContact->label()),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
  else if (-10 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving %1").arg(pContact->label()),
		    tr("Would you like to update the existing Contact or "
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

void prospect::sSave()
{
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _number->text().stripWhiteSpace().length() == 0,
      tr("You must enter a number for this Prospect before saving"),
      _number
    },

    { true, "", NULL }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Prospect"),
			  QString("<p>") + error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  if (_number->text().stripWhiteSpace() != _cachedNumber)
  {
    // in anticipation of converting prospects to customers, disallow overlap of prospect and customer numbers
    q.prepare( "SELECT prospect_name AS name "
	       "FROM prospect "
	       "WHERE (UPPER(prospect_number)=UPPER(:prospect_number)) "
	       "  AND (prospect_id<>:prospect_id) "
	       "UNION "
	       "SELECT cust_name AS name "
	       "FROM cust "
	       "WHERE (UPPER(cust_number)=UPPER(:prospect_number));" );
    q.bindValue(":prospect_name", _number->text().stripWhiteSpace());
    q.bindValue(":prospect_id", _prospectid);
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Prospect Number Used"),
			     tr("<p>The newly entered Prospect Number cannot "
				"be used as it is currently in use by '%1'. "
				"Please enter a different Prospect Number." )
			     .arg(q.value("name").toString()) );
      _number->setFocus();
      return;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (! q.exec("BEGIN;"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (saveContact(_contact) < 0)
  {
    rollback.exec();
    _contact->setFocus();
    return;
  }

  if (_mode == cEdit)
  {
    q.prepare( "UPDATE prospect SET "
               "       prospect_number=:prospect_number,"
	       "       prospect_name=:prospect_name,"
               "       prospect_cntct_id=:prospect_cntct_id,"
               "       prospect_comments=:prospect_comments,"
               "       prospect_taxauth_id=:prospect_taxauth_id,"
               "       prospect_salesrep_id=:prospect_salesrep_id,"
               "       prospect_warehous_id=:prospect_warehous_id,"
	       "       prospect_active=:prospect_active "
               "WHERE (prospect_id=:prospect_id);" );
  }
  else
  {
    q.prepare( "INSERT INTO prospect "
               "( prospect_id,	      prospect_number,	    prospect_name,"
	       "  prospect_cntct_id,  prospect_taxauth_id,  prospect_comments,"
	       "  prospect_salesrep_id, prospect_warehous_id, prospect_active) "
	       " VALUES "
               "( :prospect_id,	      :prospect_number,	    :prospect_name,"
	       "  :prospect_cntct_id, :prospect_taxauth_id, :prospect_comments,"
	       "  :prospect_salesrep_id, :prospect_warehous_id, :prospect_active);");
  }

  q.bindValue(":prospect_id",		_prospectid);
  q.bindValue(":prospect_number",	_number->text().stripWhiteSpace());
  q.bindValue(":prospect_name",		_name->text().stripWhiteSpace());
  if (_contact->id() > 0)
    q.bindValue(":prospect_cntct_id",	_contact->id());	// else NULL
  if (_taxauth->id() > 0)
    q.bindValue(":prospect_taxauth_id",	_taxauth->id());	// else NULL
  if (_salesrep->id() > 0)
    q.bindValue(":prospect_salesrep_id", _salesrep->id());      // else NULL
  if (_site->id() > 0)
    q.bindValue(":prospect_warehous_id", _site->id());          // else NULL
  q.bindValue(":prospect_comments",	_notes->text());
  q.bindValue(":prospect_active",	QVariant(_active->isChecked(), 0));

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_crmacct->id() > 0)
  {
    q.prepare("UPDATE crmacct SET crmacct_prospect_id = :prospect_id "
	      "WHERE (crmacct_id=:crmacct_id);");
    q.bindValue(":prospect_id",	_prospectid);
    q.bindValue(":crmacct_id",	_crmacct->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
	rollback.exec();
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
    }
  }
  else
  {
/*
// This is now done in a trigger
    q.prepare( "SELECT createCrmAcct(:number, :name, :active, :type, NULL, "
	       "      NULL, NULL, :prospect_id, NULL, NULL, :cntct, NULL) AS crmacctid;");
    q.bindValue(":number",	_number->text().stripWhiteSpace());
    q.bindValue(":name",	_name->text().stripWhiteSpace());
    q.bindValue(":active",	QVariant(true, 0));
    q.bindValue(":type",	"O");	// TODO - when will this be "I"?
    q.bindValue(":prospect_id",	_prospectid);
    if (_contact->id() > 0)
      q.bindValue(":cntct",	_contact->id());
    q.exec();
    if (q.first())
    {
      int crmacctid = q.value("crmacctid").toInt();
      if (crmacctid <= 0)
      {
	rollback.exec();
	systemError(this, storedProcErrorLookup("createCrmAcct", crmacctid),
		    __FILE__, __LINE__);
	return;
      }
      _crmacct->setId(crmacctid);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    // need to save contacts again with updated CRM Account
    if (saveContact(_contact) < 0)
    {
      rollback.exec();
      _contact->setFocus();
      return;
    }
 */
  }

  q.exec("COMMIT;");
  _NumberGen = -1;
  omfgThis->sProspectsUpdated();
  if (_mode == cNew)
    omfgThis->sCrmAccountsUpdated(_crmacct->id());

  close();
}

void prospect::sCheckNumber()
{
  _number->setText(_number->text().stripWhiteSpace().upper());

  if (_mode == cNew)
  {
    if(cNew == _mode && -1 != _NumberGen && _number->text().toInt() != _NumberGen)
    {
      XSqlQuery query;
      query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
      query.bindValue(":Number", _NumberGen);
      query.exec();
      _NumberGen = -1;
    }

    q.prepare( "SELECT prospect_id "
               "FROM prospect "
               "WHERE (prospect_number=:prospect);" );
    q.bindValue(":prospect", _number->text());
    q.exec();
    if (q.first())
    {
      _prospectid = q.value("prospect_id").toInt();
      _mode = cEdit;
      populate();
      _name->setFocus();
    }
  }
}

void prospect::sPrintQuote()
{
  QString reportname;

  q.prepare("SELECT findCustomerForm(:prospect_id, 'Q') AS reportname;");
  q.bindValue(":prospect_id", _prospectid);
  q.exec();
  if (q.first())
    reportname = q.value("reportname").toString();
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("quhead_id", _quotes->id());

  orReport report(reportname, params);
  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
    return;
  }
}

void prospect::sNewQuote()
{
  ParameterList params;
  params.append("mode", "newQuote");
  params.append("cust_id", _prospectid);

  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospect::sEditQuote()
{
  ParameterList params;
  params.append("mode", "editQuote");
  params.append("quhead_id", _quotes->id());
    
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospect::sViewQuote()
{
  ParameterList params;
  params.append("mode", "viewQuote");
  params.append("quhead_id", _quotes->id());
    
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospect::sDeleteQuote()
{
  if ( QMessageBox::warning( this, tr("Delete Selected Quote"),
                             tr("Are you sure that you want to delete the selected Quote?" ),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    q.prepare("SELECT deleteQuote(:quhead_id) AS result;");
    q.bindValue(":quhead_id", _quotes->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteQuote", result),
		    __FILE__, __LINE__);
	return;
      }
      else
	omfgThis->sQuotesUpdated(-1);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void prospect::sFillQuotesList()
{
  q.prepare("SELECT DISTINCT quhead_id, quhead_number, "
	    "                formatDate(quhead_quotedate) "
	    "FROM quhead "
	    "WHERE (quhead_cust_id=:prospect_id) "
	    "ORDER BY quhead_number;");
  q.bindValue(":prospect_id", _prospectid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _quotes->populate(q);
}

void prospect::sPopulateQuotesMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("Edit..."),   this, SLOT(sEditQuote()),   0 );
  menuThis->insertItem(tr("View..."),   this, SLOT(sViewQuote()),   0 );
  menuThis->insertItem(tr("Delete..."), this, SLOT(sDeleteQuote()), 0 );
  menuThis->insertItem(tr("Print..."),  this, SLOT(sPrintQuote()),  0 );
}

void prospect::populate()
{
  XSqlQuery prospect;
  prospect.prepare("SELECT prospect.*, crmacct_id "
                "FROM prospect, crmacct "
                "WHERE ((prospect_id=:prospect_id)"
		"  AND  (prospect_id=crmacct_prospect_id));" );
  prospect.bindValue(":prospect_id", _prospectid);
  prospect.exec();
  if (prospect.first())
  {
    _crmacct->setId(prospect.value("crmacct_id").toInt());

    _number->setText(prospect.value("prospect_number"));
    _cachedNumber = prospect.value("prospect_number").toString();
    _name->setText(prospect.value("prospect_name").toString());
    _contact->setId(prospect.value("prospect_cntct_id").toInt());
    _taxauth->setId(prospect.value("prospect_taxauth_id").toInt());
    _salesrep->setId(prospect.value("prospect_salesrep_id").toInt());
    _site->setId(prospect.value("prospect_warehous_id").toInt());
    _notes->setText(prospect.value("prospect_comments").toString());
    _active->setChecked(prospect.value("prospect_active").toBool());
  }
  else if (prospect.lastError().type() != QSqlError::None)
  {
    systemError(this, prospect.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillQuotesList();
}

void prospect::closeEvent(QCloseEvent *pEvent)
{
  if(cNew == _mode && -1 != _NumberGen)
  {
    XSqlQuery query;
    query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
    query.bindValue(":Number", _NumberGen);
    query.exec();
    _NumberGen = -1;
  }
  XMainWindow::closeEvent(pEvent);
}

