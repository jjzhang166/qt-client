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

#include "customer.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QCloseEvent>

#include <comment.h>
#include <metasql.h>
#include <openreports.h>

#include "addresscluster.h"
#include "characteristicAssignment.h"
#include "creditCard.h"
#include "custCharacteristicDelegate.h"
#include "mqlutil.h"
#include "shipTo.h"
#include "storedProcErrorLookup.h"
#include "taxRegistration.h"
#include "xcombobox.h"

/*
 *  Constructs a customer as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
customer::customer(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_number, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_number, SIGNAL(textEdited(const QString&)), this, SLOT(sNumberEdited()));
    connect(_salesrep, SIGNAL(newID(int)), this, SLOT(sPopulateCommission()));
    connect(_newShipto, SIGNAL(clicked()), this, SLOT(sNewShipto()));
    connect(_editShipto, SIGNAL(clicked()), this, SLOT(sEditShipto()));
    connect(_viewShipto, SIGNAL(clicked()), this, SLOT(sViewShipto()));
    connect(_deleteShipto, SIGNAL(clicked()), this, SLOT(sDeleteShipto()));
    connect(_shipto, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateShiptoMenu(QMenu*)));
    connect(_printShipto, SIGNAL(clicked()), this, SLOT(sPrintShipto()));
    connect(_downCC, SIGNAL(clicked()), this, SLOT(sMoveDown()));
    connect(_upCC, SIGNAL(clicked()), this, SLOT(sMoveUp()));
    connect(_viewCC, SIGNAL(clicked()), this, SLOT(sViewCreditCard()));
    connect(_editCC, SIGNAL(clicked()), this, SLOT(sEditCreditCard()));
    connect(_newCC, SIGNAL(clicked()), this, SLOT(sNewCreditCard()));
    connect(_ediProfile, SIGNAL(activated(int)), this, SLOT(sProfileSelected()));
    connect(_deleteCharacteristic, SIGNAL(clicked()), this, SLOT(sDeleteCharacteristic()));
    connect(_editCharacteristic, SIGNAL(clicked()), this, SLOT(sEditCharacteristic()));
    connect(_newCharacteristic, SIGNAL(clicked()), this, SLOT(sNewCharacteristic()));
    connect(_deleteTaxreg, SIGNAL(clicked()), this, SLOT(sDeleteTaxreg()));
    connect(_editTaxreg,   SIGNAL(clicked()), this, SLOT(sEditTaxreg()));
    connect(_newTaxreg,    SIGNAL(clicked()), this, SLOT(sNewTaxreg()));
    connect(_viewTaxreg,   SIGNAL(clicked()), this, SLOT(sViewTaxreg()));
    connect(_soEdiProfile, SIGNAL(activated(int)), this, SLOT(sSoProfileSelected()));
    connect(_custtype, SIGNAL(currentIndexChanged(int)), this, SLOT(sFillCharacteristicList()));
    
    _custid = -1;
    _crmacctid = -1;
    _NumberGen = -1;

    _sellingWarehouse->setId(-1);

    _currency->setLabel(_currencyLit);
    
    _balanceMethod->insertItem(tr("Balance Forward"));
    _balanceMethod->insertItem(tr("Open Items"));

    _taxreg->addColumn(tr("Tax Authority"), 100, Qt::AlignLeft );
    _taxreg->addColumn(tr("Registration #"), -1, Qt::AlignLeft );

    _shipto->addColumn(tr("Default"), _itemColumn, Qt::AlignLeft, true, "shipto_default");
    _shipto->addColumn(tr("Number"),  _itemColumn, Qt::AlignLeft, true, "shipto_num");
    _shipto->addColumn(tr("Name"),            150, Qt::AlignLeft, true, "shipto_name");
    _shipto->addColumn(tr("Address"),         150, Qt::AlignLeft, true, "shipto_address1");
    _shipto->addColumn(tr("City, State, Zip"), -1, Qt::AlignLeft, true, "shipto_csz");

    _cc->addColumn(tr("Sequence"),_itemColumn, Qt::AlignLeft, true, "ccard_seq");
    _cc->addColumn(tr("Type"),    _itemColumn, Qt::AlignLeft, true, "type");
    _cc->addColumn(tr("Number"),          150, Qt::AlignRight,true, "f_number");
    _cc->addColumn(tr("Active"),           -1, Qt::AlignLeft, true, "ccard_active");
    
    _charass->addColumn(tr("Characteristic"), _itemColumn*2, Qt::AlignLeft );
    _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft );

    _defaultCommissionPrcnt->setValidator(omfgThis->percentVal());
    _defaultDiscountPrcnt->setValidator(omfgThis->percentVal());
  
    _custchar = new QStandardItemModel(0, 2, this);
    _custchar->setHeaderData( 0, Qt::Horizontal, tr("Characteristc"), Qt::DisplayRole);
    _custchar->setHeaderData( 1, Qt::Horizontal, tr("Value"), Qt::DisplayRole);
    _chartempl->setModel(_custchar);
    CustCharacteristicDelegate * delegate = new CustCharacteristicDelegate(this);
    _chartempl->setItemDelegate(delegate);

    key = omfgThis->_key;
    if(!_metrics->boolean("CCAccept") || key.length() == 0 || key.isNull() || key.isEmpty())
    {
      _tab->removePage(_tab->page(10));
    }
    
    if (_metrics->boolean("EnableBatchManager"))
    {
      _ediProfile->append(-1, tr("No EDI"));
      _ediProfile->append(0, tr("Custom Email"));
    
      _soEdiProfile->append(-1, tr("No EDI"));
      _soEdiProfile->append(0, tr("Custom Email"));
    
      q.prepare("SELECT ediprofile_id, ediprofile_name"
                "  FROM ediprofile, ediform"
                " WHERE ((ediform_ediprofile_id=ediprofile_id)"
                "   AND  (ediform_type='invoice')) "
                "ORDER BY ediprofile_name; ");
      q.exec();
      while(q.next()) 
      {
        _ediProfile->append(q.value("ediprofile_id").toInt(), q.value("ediprofile_name").toString());
        _soEdiProfile->append(q.value("ediprofile_id").toInt(), q.value("ediprofile_name").toString());
      }
    }
    else
    {
      _tab->removePage(_tab->page(9));
      _tab->removePage(_tab->page(8));
    }
    
    //If not multi-warehouse hide whs control
    if (!_metrics->boolean("MultiWhs"))
    {
      _sellingWarehouseLit->hide();
      _sellingWarehouse->hide();
    }

    if(!_metrics->boolean("AutoCreditWarnLateCustomers"))
      _warnLate->hide();
    else
      _graceDays->setValue(_metrics->value("DefaultAutoCreditWarnGraceDays").toInt());
}

/*
 *  Destroys the object and frees any allocated resources
 */
customer::~customer()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void customer::languageChange()
{
  retranslateUi(this);
}

enum SetResponse customer::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _custid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      if (_custid <= 0 )
      {
        q.exec("SELECT NEXTVAL('cust_cust_id_seq') AS cust_id");
        if (q.first())
          _custid = q.value("cust_id").toInt();
        else
        {
          systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
          return UndefinedError;
        }
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

      _comments->setId(_custid);

      _salesrep->setId(_metrics->value("DefaultSalesRep").toInt());
      _terms->setId(_metrics->value("DefaultTerms").toInt());
      _taxauth->setCurrentItem(-1);
      _shipform->setId(_metrics->value("DefaultShipFormId").toInt());
      _shipvia->setId(_metrics->value("DefaultShipViaId").toInt());
      _custtype->setId(_metrics->value("DefaultCustType").toInt());
      _backorders->setChecked(_metrics->boolean("DefaultBackOrders"));
      _partialShipments->setEnabled(_metrics->boolean("DefaultBackOrders"));
      _partialShipments->setChecked(_metrics->boolean("DefaultPartialShipments"));
      _allowFFShipto->setChecked(_metrics->boolean("DefaultFreeFormShiptos"));
      _creditLimit->setBaseValue(_metrics->value("SOCreditLimit").toDouble());
      _creditRating->setText(_metrics->value("SOCreditRate"));

      if (_metrics->value("DefaultBalanceMethod") == "B")
        _balanceMethod->setCurrentItem(0);
      else if (_metrics->value("DefaultBalanceMethod") == "O")
        _balanceMethod->setCurrentItem(1);

      if(!_privileges->check("MaintainCustomerMastersCustomerType")
         && !_privileges->check("MaintainCustomerMastersCustomerTypeOnCreate")
         && (_custtype->id() != -1))
        _custtype->setEnabled(false);

      connect(_shipto, SIGNAL(valid(bool)), _editShipto, SLOT(setEnabled(bool)));
      connect(_shipto, SIGNAL(valid(bool)), _deleteShipto, SLOT(setEnabled(bool)));
      connect(_shipto, SIGNAL(itemSelected(int)), _editShipto, SLOT(animateClick()));
      connect(_cc, SIGNAL(valid(bool)), _editCC, SLOT(setEnabled(bool)));
      connect(_cc, SIGNAL(itemSelected(int)), _editCC, SLOT(animateClick()));
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      connect(_backorders, SIGNAL(toggled(bool)), _partialShipments, SLOT(setEnabled(bool)));
      connect(_backorders, SIGNAL(toggled(bool)), _partialShipments, SLOT(setChecked(bool)));

      _number->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      if(!_privileges->check("MaintainCustomerMastersCustomerType")
         && (_custtype->id() != -1))
        _custtype->setEnabled(false);

      connect(_shipto, SIGNAL(valid(bool)), _editShipto, SLOT(setEnabled(bool)));
      connect(_shipto, SIGNAL(valid(bool)), _deleteShipto, SLOT(setEnabled(bool)));
      connect(_shipto, SIGNAL(itemSelected(int)), _editShipto, SLOT(animateClick()));
      connect(_cc, SIGNAL(valid(bool)), _editCC, SLOT(setEnabled(bool)));
      connect(_cc, SIGNAL(itemSelected(int)), _editCC, SLOT(animateClick()));
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      connect(_backorders, SIGNAL(toggled(bool)), _partialShipments, SLOT(setEnabled(bool)));
      connect(_backorders, SIGNAL(toggled(bool)), _partialShipments, SLOT(setChecked(bool)));

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _custtype->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _corrCntct->setEnabled(FALSE);
      _billCntct->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _balanceMethod->setEnabled(FALSE);
      _defaultDiscountPrcnt->setEnabled(FALSE);
      _creditLimit->setEnabled(FALSE);
      _creditRating->setEnabled(FALSE);
      _creditStatusGroup->setEnabled(FALSE);
      _autoUpdateStatus->setEnabled(FALSE);
      _autoHoldOrders->setEnabled(FALSE);
      _taxauth->setEnabled(FALSE);
      _sellingWarehouse->setEnabled(FALSE);
      _salesrep->setEnabled(FALSE);
      _defaultCommissionPrcnt->setEnabled(FALSE);
      _shipvia->setEnabled(FALSE);
      _shipform->setEnabled(FALSE);
      _shipchrg->setEnabled(FALSE);
      _backorders->setEnabled(FALSE);
      _usesPOs->setEnabled(FALSE);
      _blanketPos->setEnabled(FALSE);
      _allowFFShipto->setEnabled(FALSE);
      _allowFFBillto->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _comments->setReadOnly(TRUE);
      _ediProfile->setEnabled(FALSE);
      _ediGroup->setEnabled(FALSE);
      _soEdiProfile->setEnabled(FALSE);
      _soEdiGroup->setEnabled(FALSE);
      _newShipto->setEnabled(FALSE);
      _newCharacteristic->setEnabled(FALSE);
      _newTaxreg->setEnabled(FALSE);
      _currency->setEnabled(FALSE);
      _partialShipments->setEnabled(FALSE);
      _save->hide();

      connect(_shipto, SIGNAL(itemSelected(int)), _viewShipto, SLOT(animateClick()));
      connect(_cc, SIGNAL(itemSelected(int)), _viewCC, SLOT(animateClick()));

      disconnect(_taxreg, SIGNAL(valid(bool)), _deleteTaxreg, SLOT(setEnabled(bool)));
      disconnect(_taxreg, SIGNAL(valid(bool)), _editTaxreg, SLOT(setEnabled(bool)));
      disconnect(_taxreg, SIGNAL(itemSelected(int)), _editTaxreg, SLOT(animateClick()));
      connect(_taxreg, SIGNAL(itemSelected(int)), _viewTaxreg, SLOT(animateClick()));

      _close->setFocus();
    }
  }
  
  param = pParams.value("crmacct_id", &valid);
  if (valid)
    sLoadCrmAcct(param.toInt());

  param = pParams.value("prospect_id", &valid);
  if (valid)
    sLoadProspect(param.toInt());

  if(_metrics->value("CRMAccountNumberGeneration") == "A")
    _number->setEnabled(FALSE);

  return NoError;
}

// similar code in address, customer, shipto, vendor, vendorAddress
int customer::saveContact(ContactCluster* pContact)
{
  pContact->setAccount(_crmacctid);

  int answer = 2;    // Cancel
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
                    tr("Change for All"),
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

bool customer::sSave(bool partial)
{
  if (true)
  {
    if (_number->text().stripWhiteSpace().length() == 0)
    {
      QMessageBox::critical( this, tr("Enter Customer Number"),
                             tr("You must enter a number for this Customer before continuing") );
      _number->setFocus();
      return false;
    }
  
    if (_name->text().stripWhiteSpace().length() == 0)
    {
      QMessageBox::critical( this, tr("Enter Customer Name"),
                             tr("You must enter a name for this Customer before continuing") );
      _number->setFocus();
      return false;
    }

    if (_custtype->id() == -1)
    {
      QMessageBox::critical( this, tr("Select Customer Type"),
                             tr("You must select a Customer Type code for this Customer before continuing.") );
      _terms->setFocus();
      return false;
    }

    if (_terms->id() == -1)
    {
      QMessageBox::critical( this, tr("Select Terms"),
                             tr("You must select a Terms code for this Customer before continuing.") );
      _terms->setFocus();
      return false;
    }

    if (_salesrep->id() == -1)
    {
      QMessageBox::critical( this, tr("Select Sales Rep."),
                             tr("You must select a Sales Rep. for this Customer before continuing.") );
      _salesrep->setFocus();
      return false;
    }

    if (_shipform->id() == -1)
    {
      QMessageBox::critical( this, tr("Select Default Shipping Form"),
                             tr("You must select a default Shipping Form for this Customer before continuing.") );
      _shipform->setFocus();
      return false;
    }

    if (_salesrep->currentItem() == -1)
    {
      QMessageBox::warning( this, tr("Select Sales Representative"),
                            tr( "You must select a Sales Representative before adding this Customer." ));
      _salesrep->setFocus();
      return false;
    }

    if (_number->text().stripWhiteSpace() != _cachedNumber)
    {
      q.prepare( "SELECT cust_name "
                 "FROM custinfo "
                 "WHERE (UPPER(cust_number)=UPPER(:cust_number)) "
                 "  AND (cust_id<>:cust_id);" );
      q.bindValue(":cust_name", _number->text().stripWhiteSpace());
      q.bindValue(":cust_id", _custid);
      q.exec();
      if (q.first())
      {
        QMessageBox::critical( this, tr("Customer Number Used"),
                               tr( "The newly entered Customer Number cannot be used as it is currently\n"
                                   "in use by the Customer '%1'.  Please correct or enter a new Customer Number." )
                                  .arg(q.value("cust_name").toString()) );
        _number->setFocus();
        return false;
      }
    }
  }
 
  if (_mode == cEdit)
  {
    q.prepare( "UPDATE custinfo SET "
               "       cust_number=:cust_number, cust_name=:cust_name,"
               "       cust_salesrep_id=:cust_salesrep_id,"
               "       cust_corrcntct_id=:cust_corrcntct_id, cust_cntct_id=:cust_cntct_id,"
               "       cust_custtype_id=:cust_custtype_id, cust_balmethod=:cust_balmethod,"
               "       cust_creditlmt=:cust_creditlmt, cust_creditlmt_curr_id=:cust_creditlmt_curr_id,"
               "       cust_creditrating=:cust_creditrating,"
               "       cust_autoupdatestatus=:cust_autoupdatestatus, cust_autoholdorders=:cust_autoholdorders,"
               "       cust_creditstatus=:cust_creditstatus,"
               "       cust_backorder=:cust_backorder, cust_ffshipto=:cust_ffshipto, cust_ffbillto=:cust_ffbillto,"
               "       cust_commprcnt=:cust_commprcnt,"
               "       cust_partialship=:cust_partialship, cust_shipvia=:cust_shipvia,"
               "       cust_shipchrg_id=:cust_shipchrg_id, cust_shipform_id=:cust_shipform_id,"
               "       cust_terms_id=:cust_terms_id,"
               "       cust_discntprcnt=:cust_discntprcnt,"
               "       cust_taxauth_id=:cust_taxauth_id, "
               "       cust_active=:cust_active, cust_usespos=:cust_usespos,"
               "       cust_blanketpos=:cust_blanketpos, cust_comments=:cust_comments,"
               "       cust_emaildelivery=:cust_emaildelivery, cust_ediemail=:cust_ediemail,"
               "       cust_edisubject=:cust_edisubject, cust_edifilename=:cust_edifilename,"
               "       cust_ediemailbody=:cust_ediemailbody, cust_edicc=:cust_edicc,"
               "       cust_ediprofile_id=:cust_ediprofile_id, "
               "       cust_ediemailhtml=:cust_ediemailhtml,"
               "       cust_soemaildelivery=:cust_soemaildelivery, cust_soediemail=:cust_soediemail,"
               "       cust_soedisubject=:cust_soedisubject, cust_soedifilename=:cust_soedifilename,"
               "       cust_soediemailbody=:cust_soediemailbody, cust_soedicc=:cust_soedicc,"
               "       cust_soediprofile_id=:cust_soediprofile_id, "
               "       cust_soediemailhtml=:cust_soediemailhtml,"
               "       cust_preferred_warehous_id=:cust_preferred_warehous_id, "
               "       cust_gracedays=:cust_gracedays,"
               "       cust_curr_id=:cust_curr_id "
               "WHERE (cust_id=:cust_id);" );
  }
  else
    q.prepare( "INSERT INTO custinfo "
               "( cust_id, cust_number,"
               "  cust_salesrep_id, cust_name,"
               "  cust_corrcntct_id, cust_cntct_id,"
               "  cust_custtype_id, cust_balmethod,"
               "  cust_creditlmt, cust_creditlmt_curr_id,"
               "  cust_creditrating, cust_creditstatus,"
               "  cust_autoupdatestatus, cust_autoholdorders,"
               "  cust_backorder, cust_ffshipto, cust_ffbillto,"
               "  cust_commprcnt, cust_partialship,"
               "  cust_shipvia,"
               "  cust_shipchrg_id, cust_shipform_id, cust_terms_id,"
               "  cust_discntprcnt, cust_taxauth_id, "
               "  cust_active, cust_usespos, cust_blanketpos, cust_comments,"
               "  cust_emaildelivery, cust_ediemail, cust_edisubject,"
               "  cust_edifilename, cust_ediemailbody, cust_edicc, "
               "  cust_ediprofile_id,"
               "  cust_soemaildelivery, cust_soediemail, cust_soedisubject,"
               "  cust_soedifilename, cust_soediemailbody, cust_soedicc, "
               "  cust_soediemailhtml, cust_ediemailhtml,"
               "  cust_soediprofile_id, cust_preferred_warehous_id, "
               "  cust_gracedays, cust_curr_id ) "
               "VALUES "
               "( :cust_id, :cust_number,"
               "  :cust_salesrep_id, :cust_name,"
               "  :cust_corrcntct_id, :cust_cntct_id,"
               "  :cust_custtype_id, :cust_balmethod,"
               "  :cust_creditlmt, :cust_creditlmt_curr_id,"
               "  :cust_creditrating, :cust_creditstatus,"
               "  :cust_autoupdatestatus, :cust_autoholdorders,"
               "  :cust_backorder, :cust_ffshipto, :cust_ffbillto,"
               "  :cust_commprcnt, :cust_partialship,"
               "  :cust_shipvia,"
               "  :cust_shipchrg_id, :cust_shipform_id, :cust_terms_id,"
               "  :cust_discntprcnt, :cust_taxauth_id,"
               "  :cust_active, :cust_usespos, :cust_blanketpos, :cust_comments,"
               "  :cust_emaildelivery, :cust_ediemail, :cust_edisubject,"
               "  :cust_edifilename, :cust_ediemailbody, :cust_edicc,"
               "  :cust_soediprofile_id,"
               "  :cust_soemaildelivery, :cust_soediemail, :cust_soedisubject,"
               "  :cust_soedifilename, :cust_soediemailbody, :cust_soedicc,"
               "  :cust_soediemailhtml, :cust_ediemailhtml,"
               "  :cust_soediprofile_id, :cust_preferred_warehous_id, "
               "  :cust_gracedays, :cust_curr_id ) " );

  q.bindValue(":cust_id", _custid);
  q.bindValue(":cust_number", _number->text().stripWhiteSpace());
  q.bindValue(":cust_name", _name->text().stripWhiteSpace());
  q.bindValue(":cust_salesrep_id", _salesrep->id());
  if (_corrCntct->id() > 0)
    q.bindValue(":cust_corrcntct_id", _corrCntct->id());        // else NULL
  if (_billCntct->id() > 0)
    q.bindValue(":cust_cntct_id", _billCntct->id());            // else NULL
  q.bindValue(":cust_custtype_id", _custtype->id());

  if (_balanceMethod->currentItem() == 0)
    q.bindValue(":cust_balmethod", "B");
  else
    q.bindValue(":cust_balmethod", "O");

  if (_inGoodStanding->isChecked())
    q.bindValue(":cust_creditstatus", "G");
  else if (_onCreditWarning->isChecked())
    q.bindValue(":cust_creditstatus", "W");
  else if (_onCreditHold->isChecked())
    q.bindValue(":cust_creditstatus", "H");
  else
    q.bindValue(":cust_creditstatus", "U");

  q.bindValue(":cust_creditlmt_curr_id", _creditLimit->id());
  q.bindValue(":cust_creditlmt", _creditLimit->localValue());
  q.bindValue(":cust_creditrating", _creditRating->text());
  q.bindValue(":cust_autoupdatestatus", QVariant(_autoUpdateStatus->isChecked(), 0));
  q.bindValue(":cust_autoholdorders", QVariant(_autoHoldOrders->isChecked(), 0));
  q.bindValue(":cust_commprcnt", (_defaultCommissionPrcnt->toDouble() / 100.0));
  q.bindValue(":cust_terms_id", _terms->id());
  q.bindValue(":cust_discntprcnt", (_defaultDiscountPrcnt->toDouble() / 100.0));

  if (_taxauth->isValid())
    q.bindValue(":cust_taxauth_id", _taxauth->id());

  q.bindValue(":cust_shipvia", _shipvia->currentText());
  q.bindValue(":cust_shipchrg_id", _shipchrg->id());
  q.bindValue(":cust_shipform_id", _shipform->id());

  q.bindValue(":cust_active", QVariant(_active->isChecked(), 0));
  q.bindValue(":cust_usespos", QVariant(_usesPOs->isChecked(), 0));
  q.bindValue(":cust_blanketpos", QVariant(_blanketPos->isChecked(), 0));
  q.bindValue(":cust_partialship", QVariant(_partialShipments->isChecked(), 0));
  q.bindValue(":cust_backorder", QVariant(_backorders->isChecked(), 0));
  q.bindValue(":cust_ffshipto", QVariant(_allowFFShipto->isChecked(), 0));
  q.bindValue(":cust_ffbillto", QVariant(_allowFFBillto->isChecked(), 0));

  q.bindValue(":cust_comments", _notes->text());

  q.bindValue(":cust_emaildelivery", QVariant((_ediProfile->id()==0), 0));
  q.bindValue(":cust_ediemail", _ediEmail->text().stripWhiteSpace());
  q.bindValue(":cust_edisubject", _ediSubject->text().stripWhiteSpace());
  q.bindValue(":cust_edifilename", _ediFilename->text().stripWhiteSpace());
  q.bindValue(":cust_ediemailbody", _ediEmailBody->toPlainText().stripWhiteSpace());
  q.bindValue(":cust_edicc", _ediCC->text().stripWhiteSpace());
  q.bindValue(":cust_ediemailhtml", QVariant(_ediEmailHTML->isChecked(), 0));
  
  q.bindValue(":cust_soemaildelivery", QVariant((_soEdiProfile->id()==0), 0));
  q.bindValue(":cust_soediemail", _soEdiEmail->text().stripWhiteSpace());
  q.bindValue(":cust_soedisubject", _soEdiSubject->text().stripWhiteSpace());
  q.bindValue(":cust_soedifilename", _soEdiFilename->text().stripWhiteSpace());
  q.bindValue(":cust_soediemailbody", _soEdiEmailBody->toPlainText().stripWhiteSpace());
  q.bindValue(":cust_soedicc", _soEdiCC->text().stripWhiteSpace());
  q.bindValue(":cust_soediemailhtml", QVariant(_soEdiEmailHTML->isChecked(), 0));

  q.bindValue(":cust_preferred_warehous_id", _sellingWarehouse->id());
  q.bindValue(":cust_curr_id", _currency->id());

  if(_warnLate->isChecked())
    q.bindValue(":cust_gracedays", _graceDays->value());

  if(_ediProfile->id() > 0)
    q.bindValue(":cust_ediprofile_id", _ediProfile->id());

  if(_soEdiProfile->id() > 0)
    q.bindValue(":cust_soediprofile_id", _soEdiProfile->id());

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  
  if (_mode == cNew)
    _mode = cEdit;
  
  return true;
}
   
void customer::sSave()
{
  if (! q.exec("BEGIN"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (_billCntct->sChanged())
  {
    if (saveContact(_billCntct) < 0)
    {
      rollback.exec();
      _billCntct->setFocus();
      return;
    }
  }

  if (_corrCntct->sChanged())
  {
    if (saveContact(_corrCntct) < 0)
    {
      rollback.exec();
      _corrCntct->setFocus();
      return;
    }
  }
  
  if (!sSave(false))
    return;
  
  //Check to see if this is a prospect with quotes
  bool convertQuotes = false;
  
  q.prepare("SELECT * FROM prospect, quhead "
            " WHERE ((prospect_id=quhead_cust_id) "
            " AND (prospect_id=:prospect_id)); ");
  q.bindValue(":prospect_id", _custid);
  q.exec();
  if (q.first())
    if (_privileges->check("ConvertQuotes") &&
        QMessageBox::question(this, tr("Convert"),
                              tr("<p>Do you want to convert all of the Quotes "
                                 "for the Prospect to Sales Orders?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::Yes)
      convertQuotes = true;



  //Save characteristics
  if (_widgetStack->currentIndex() == 1)
  {
    q.prepare("SELECT updateCharAssignment('C', :target_id, :char_id, :char_value);");
  
    QModelIndex idx1, idx2;
    for(int i = 0; i < _custchar->rowCount(); i++)
    {
      idx1 = _custchar->index(i, 0);
      idx2 = _custchar->index(i, 1);
      q.bindValue(":target_id", _custid);
      q.bindValue(":char_id", _custchar->data(idx1, Qt::UserRole));
      q.bindValue(":char_value", _custchar->data(idx2, Qt::DisplayRole));
      q.exec();
    }
  }
  
  if (convertQuotes)
  {
    q.prepare("SELECT MIN(convertQuote(quhead_id)) AS result "
              "FROM quhead "
              "WHERE (quhead_cust_id=:id);");
    q.bindValue(":id", _custid);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("convertQuote", result),
                    __FILE__, __LINE__);
        // not fatal
      }
      omfgThis->sQuotesUpdated(-1);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      // not fatal
    }
  }

  q.exec("COMMIT;");
  _NumberGen = -1;
  omfgThis->sCustomersUpdated(_custid, TRUE);
  close();
}

void customer::sCheck()
{
  _number->setText(_number->text().stripWhiteSpace().upper());
  if(cNew == _mode && -1 != _NumberGen && _number->text().toInt() != _NumberGen)
  {
    XSqlQuery query;
    query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
    query.bindValue(":Number", _NumberGen);
    query.exec();
    _NumberGen = -1;
  }

  q.prepare( "SELECT cust_id, 1 AS type "
             "FROM custinfo "
             "WHERE (cust_number=:cust_number) "
             "UNION "
             "SELECT prospect_id, 2 AS type "
             "FROM prospect "
             "WHERE (prospect_number=:cust_number) "
             "UNION "
             "SELECT crmacct_id, 3 AS type "
             "FROM crmacct "
             "WHERE (crmacct_number=:cust_number) "
             "ORDER BY type; ");
  q.bindValue(":cust_number", _number->text());
  q.exec();
  if (q.first())
  {
    if ((q.value("type").toInt() == 1) && (_notice))
    {
      if (QMessageBox::question(this, tr("Customer Exists"),
              tr("<p>This number is currently "
                   "used by an existing Customer. "
                   "Do you want to edit "
                   "that Customer?"),
              QMessageBox::Yes,
              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
        _number->clear();
        _number->setFocus();
        return;
      }
      else
      {
        _custid = q.value("cust_id").toInt();
        _mode = cEdit;
        populate();
        _name->setFocus();
      }
    }
    else if ( (_mode == cEdit) && 
              ((q.value("type").toInt() == 2) ||
              (q.value("type").toInt() == 3)) && 
              (_notice))
    {
      if (QMessageBox::critical(this, tr("Invalid Number"),
              tr("<p>This number is currently "
                   "assigned to another CRM account.")))
      {
        _number->setText(_cachedNumber);
        _number->setFocus();
        _notice = false;
        return;
      }
    }
    else if ((q.value("type").toInt() == 2) && (_notice))
    {
      if (QMessageBox::question(this, tr("Convert"),
              tr("<p>This number is currently "
                   "assigned to a Prospect. "
                   "Do you want to convert the "
                   "Prospect to a Customer?"),
              QMessageBox::Yes,
              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
        _number->clear();
        _number->setFocus();
        return;
      }
      else
        sLoadProspect(q.value("cust_id").toInt());
    }
    else if ((q.value("type").toInt() == 3) && (_notice))
    {
      if (QMessageBox::question(this, tr("Convert"),
              tr("<p>This number is currently "
                     "assigned to CRM Account. "
                 "Do you want to convert the "
                   "CRM Account to a Customer?"),
              QMessageBox::Yes,
              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
        _number->clear();
        _number->setFocus();
        return;
      }
      else
        sLoadCrmAcct(q.value("cust_id").toInt());
    }
  }
}

void customer::sPrintShipto()
{
  ParameterList params;
  params.append("cust_id", _custid);

  orReport report("ShipToMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void customer::sNewShipto()
{
  if (_mode == cNew)
  {
    if (!sSave(true))
      return;
  }
  
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _custid);

  shipTo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillShiptoList();
}

void customer::sEditShipto()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("shipto_id", _shipto->id());

  shipTo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillShiptoList();
}

void customer::sViewShipto()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("shipto_id", _shipto->id());

  shipTo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void customer::sDeleteShipto()
{
  q.prepare( "SELECT cohead_id "
             "FROM cohead "
             "WHERE (cohead_shipto_id=:shipto_id) "

             "UNION SELECT cmhead_id "
             "FROM cmhead "
             "WHERE (cmhead_shipto_id=:shipto_id) "

             "UNION SELECT cohist_id "
             "FROM cohist "
             "WHERE (cohist_shipto_id=:shipto_id) "

             "LIMIT 1;" );
  q.bindValue(":shipto_id", _shipto->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Ship-to"),
                           tr( "The selected Ship-to cannot be deleted as there has been Sales History recorded for it.\n"
                               "You may Edit the selected Ship-to and set its status to inactive." ) );
    return;
  }

  q.prepare( "DELETE FROM ipsass "
                 "WHERE (ipsass_shipto_id=:shipto_id);" );
  q.bindValue(":shipto_id", _shipto->id());
  q.exec();
  q.prepare( "DELETE FROM shiptoinfo "
                 "WHERE (shipto_id=:shipto_id);" );
  q.bindValue(":shipto_id", _shipto->id());
  q.exec();

  sFillShiptoList();
}

void customer::sNewCharacteristic()
{
  if (_mode == cNew)
  {
    if (!sSave(true))
      return;
  }
  
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _custid);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharacteristicList();
}

void customer::sEditCharacteristic()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharacteristicList();
}

void customer::sDeleteCharacteristic()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();

  sFillCharacteristicList();
}

void customer::sFillCharacteristicList()
{
  
  q.prepare( "SELECT custtype_char "
             "FROM custtype "
             "WHERE (custtype_id=:custtype_id);");
  q.bindValue(":custtype_id",_custtype->id());
  q.exec();

  q.first();
  if (q.value("custtype_char").toBool())
  {
      _widgetStack->setCurrentIndex(1);
      _custchar->removeRows(0, _custchar->rowCount());
      q.prepare( "SELECT DISTINCT char_id, char_name,"
               "       COALESCE(b.charass_value, (SELECT c.charass_value FROM charass c WHERE ((c.charass_target_type='CT') AND (c.charass_target_id=:custtype_id) AND (c.charass_default) AND (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "  FROM charass a, char "
               "    LEFT OUTER JOIN charass b"
               "      ON (b.charass_target_type='C'"
               "      AND b.charass_target_id=:cust_id"
               "      AND b.charass_char_id=char_id) "
               " WHERE ( (a.charass_char_id=char_id)"
               "   AND   (a.charass_target_type='CT')"
               "   AND   (a.charass_target_id=:custtype_id) ) "
               " ORDER BY char_name;" );
    q.bindValue(":custtype_id", _custtype->id());
    q.bindValue(":cust_id", _custid);
    q.exec();
    
    int row = 0;
    QModelIndex idx;
    while(q.next())
    {
      _custchar->insertRow(_custchar->rowCount());
      idx = _custchar->index(row, 0);
      _custchar->setData(idx, q.value("char_name"), Qt::DisplayRole);
      _custchar->setData(idx, q.value("char_id"), Qt::UserRole);
      idx = _custchar->index(row, 1);
      _custchar->setData(idx, q.value("charass_value"), Qt::DisplayRole);
      _custchar->setData(idx, _custtype->id(), Qt::UserRole);
      row++;
    }
  }
  else
  {
    _widgetStack->setCurrentIndex(0);
    _charass->clear();
    q.prepare( "SELECT charass_id, char_name, charass_value "
               "FROM charass, char "
               "WHERE ( (charass_target_type='C')"
               " AND (charass_char_id=char_id)"
               " AND (charass_target_id=:cust_id) ) "
               "ORDER BY char_name;" );
    q.bindValue(":custtype_id", _custtype->id());
    q.bindValue(":cust_id", _custid);
    q.exec();
    _charass->populate(q);
  }
}

void customer::sPopulateShiptoMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("Edit..."),   this, SLOT(sEdit()),   0 );
  menuThis->insertItem(tr("View..."),   this, SLOT(sView()),   0 );
  menuThis->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0 );
}

void customer::sFillShiptoList()
{
  q.prepare( "SELECT shipto_id, shipto_default,"
             "       shipto_num, shipto_name, shipto_address1,"
             "       (shipto_city || ', ' || shipto_state || '  ' || shipto_zipcode) "
             "FROM shipto "
             "WHERE (shipto_cust_id=:cust_id) "
             "ORDER BY shipto_num;" );
  q.bindValue(":cust_id", _custid);
  q.exec();
  _shipto->populate(q);
}

void customer::sNewTaxreg()
{
  if (_mode == cNew)
  {
    if (!sSave(true))
      return;
  }
  
  ParameterList params;
  params.append("mode", "new");
  params.append("taxreg_rel_id", _custid);
  params.append("taxreg_rel_type", "C");

  taxRegistration newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() != XDialog::Rejected)
    sFillTaxregList();
}

void customer::sEditTaxreg()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("taxreg_id", _taxreg->id());

  taxRegistration newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.set(params) == NoError && newdlg.exec() != XDialog::Rejected)
    sFillTaxregList();
}

void customer::sViewTaxreg()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxreg_id", _taxreg->id());

  taxRegistration newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void customer::sDeleteTaxreg()
{
  q.prepare("DELETE FROM taxreg "
            "WHERE (taxreg_id=:taxreg_id);");
  q.bindValue(":taxreg_id", _taxreg->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillTaxregList();
}

void customer::sFillTaxregList()
{
  XSqlQuery taxreg;
  taxreg.prepare("SELECT taxreg_id, taxreg_taxauth_id, "
                 "       taxauth_code, taxreg_number "
                 "FROM taxreg, taxauth "
                 "WHERE ((taxreg_rel_type='C') "
                 "  AND  (taxreg_rel_id=:cust_id) "
                 "  AND  (taxreg_taxauth_id=taxauth_id));");
  taxreg.bindValue(":cust_id", _custid);
  taxreg.exec();
  _taxreg->clear();
  _taxreg->populate(taxreg, true);
  if (taxreg.lastError().type() != QSqlError::None)
  {
    systemError(this, taxreg.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customer::sPopulateCommission()
{
  if (_mode != cView)
  {
    q.prepare( "SELECT salesrep_commission "
               "FROM salesrep "
               "WHERE (salesrep_id=:salesrep_id);" );
    q.bindValue(":salesrep_id", _salesrep->id());
    q.exec();
    if (q.first())
      _defaultCommissionPrcnt->setDouble(q.value("salesrep_commission").toDouble() * 100);
  }
}

void customer::populate()
{
  XSqlQuery cust;
  _notice = FALSE;
  cust.prepare( "SELECT custinfo.*, "
                "       cust_commprcnt, cust_discntprcnt,"
                "       (cust_gracedays IS NOT NULL) AS hasGraceDays,"
                "       crmacct_id "
                "FROM custinfo, crmacct "
                "WHERE ((cust_id=:cust_id) "
                "  AND  (cust_id=crmacct_cust_id));" );
  cust.bindValue(":cust_id", _custid);
  cust.exec();
  if (cust.first())
  {
    _crmacctid = cust.value("crmacct_id").toInt();

    _number->setText(cust.value("cust_number"));
    _cachedNumber = cust.value("cust_number").toString();
    _name->setText(cust.value("cust_name"));
    _corrCntct->setId(cust.value("cust_corrcntct_id").toInt());
    _billCntct->setId(cust.value("cust_cntct_id").toInt());
    _creditLimit->set(cust.value("cust_creditlmt").toDouble(),
                      cust.value("cust_creditlmt_curr_id").toInt(),
                      QDate::currentDate());
    _creditRating->setText(cust.value("cust_creditrating"));
    _autoUpdateStatus->setChecked(cust.value("cust_autoupdatestatus").toBool());
    _autoHoldOrders->setChecked(cust.value("cust_autoholdorders").toBool());
    _defaultDiscountPrcnt->setDouble(cust.value("cust_discntprcnt").toDouble() * 100);

    if(cust.value("hasGraceDays").toBool())
    {
      _warnLate->setChecked(true);
      _graceDays->setValue(cust.value("cust_gracedays").toInt());
    }

    _notes->setText(cust.value("cust_comments").toString());

    _custtype->setId(cust.value("cust_custtype_id").toInt());
    _salesrep->setId(cust.value("cust_salesrep_id").toInt());
    _defaultCommissionPrcnt->setDouble(cust.value("cust_commprcnt").toDouble() * 100);
    _terms->setId(cust.value("cust_terms_id").toInt());
    _taxauth->setId(cust.value("cust_taxauth_id").toInt());
    _shipform->setId(cust.value("cust_shipform_id").toInt());
    _shipchrg->setId(cust.value("cust_shipchrg_id").toInt());
    _shipvia->setText(cust.value("cust_shipvia").toString());

    _sellingWarehouse->setId(cust.value("cust_preferred_warehous_id").toInt());

    if (cust.value("cust_balmethod").toString() == "B")
      _balanceMethod->setCurrentItem(0);
    else if (cust.value("cust_balmethod").toString() == "O")
      _balanceMethod->setCurrentItem(1);

    _active->setChecked(cust.value("cust_active").toBool());
    _backorders->setChecked(cust.value("cust_backorder").toBool());
    _partialShipments->setChecked(cust.value("cust_partialship").toBool());
    _partialShipments->setEnabled(cust.value("cust_backorder").toBool());
    _allowFFShipto->setChecked(cust.value("cust_ffshipto").toBool());
    _allowFFBillto->setChecked(cust.value("cust_ffbillto").toBool());
    _usesPOs->setChecked(cust.value("cust_usespos").toBool());
    _blanketPos->setChecked(cust.value("cust_blanketpos").toBool());
    _currency->setId(cust.value("cust_curr_id").toInt());

    if (cust.value("cust_creditstatus").toString() == "G")
      _inGoodStanding->setChecked(TRUE);
    else if (cust.value("cust_creditstatus").toString() == "W")
      _onCreditWarning->setChecked(TRUE);
    else
      _onCreditHold->setChecked(TRUE);

    if(cust.value("cust_emaildelivery").toBool())
      _ediProfile->setId(0);
    else
    {
      _ediProfile->setId(cust.value("cust_ediprofile_id").toInt());
      if(0 == _ediProfile->id())
        _ediProfile->setId(-1);
    }
    sProfileSelected();
    _ediEmail->setText(cust.value("cust_ediemail"));
    _ediSubject->setText(cust.value("cust_edisubject"));
    _ediFilename->setText(cust.value("cust_edifilename"));
    _ediEmailBody->setPlainText(cust.value("cust_ediemailbody").toString());
    _ediCC->setText(cust.value("cust_edicc").toString());
    _ediEmailHTML->setChecked(cust.value("cust_ediemailhtml").toBool());
    
    if(cust.value("cust_soemaildelivery").toBool())
      _soEdiProfile->setId(0);
    else
    {
      _soEdiProfile->setId(cust.value("cust_soediprofile_id").toInt());
      if(0 == _soEdiProfile->id())
        _soEdiProfile->setId(-1);
    }
    sSoProfileSelected();
    _soEdiEmail->setText(cust.value("cust_soediemail"));
    _soEdiSubject->setText(cust.value("cust_soedisubject"));
    _soEdiFilename->setText(cust.value("cust_soedifilename"));
    _soEdiEmailBody->setPlainText(cust.value("cust_soediemailbody").toString());
    _soEdiCC->setText(cust.value("cust_soedicc").toString());
    _soEdiEmailHTML->setChecked(cust.value("cust_soediemailhtml").toBool());

    _comments->setId(_custid);
    sFillShiptoList();
    sFillTaxregList();
    sFillCharacteristicList();
    sFillCcardList();
  }
  else if (cust.lastError().type() != QSqlError::None)
  {
    systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customer::sProfileSelected()
{
  _ediGroup->setEnabled((_ediProfile->id() == 0));
}

void customer::sSoProfileSelected()
{
  _soEdiGroup->setEnabled((_soEdiProfile->id() == 0));
}

void customer::sNewCreditCard()
{
  if (_mode == cNew)
  {
    if (!sSave(true))
      return;
  }
  
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _custid);

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCcardList();
}

void customer::sEditCreditCard()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", _custid);
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCcardList();
}

void customer::sViewCreditCard()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", _custid);
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void customer::sMoveUp()
{
  q.prepare("SELECT moveCcardUp(:ccard_id) AS result;");
  q.bindValue(":ccard_id", _cc->id());
  q.exec();
  
  sFillCcardList();

}

void customer::sMoveDown()
{
  q.prepare("SELECT moveCcardDown(:ccard_id) AS result;");
  q.bindValue(":ccard_id", _cc->id());
  q.exec();
  
  sFillCcardList();

}

void customer::sFillCcardList()
{
  key = omfgThis->_key;
  
  q.prepare( "SELECT expireCreditCard(:cust_id, setbytea(:key));");
  q.bindValue(":cust_id", _custid);
  q.bindValue(":key", key);
  q.exec(); 
  
  MetaSQLQuery mql = mqlLoad(":/ar/fillcreditcardlist.mql");
  ParameterList params;
  params.append("cust_id",         _custid);
  params.append("masterCard",      tr("MasterCard"));
  params.append("visa",            tr("VISA"));
  params.append("americanExpress", tr("American Express"));
  params.append("discover",        tr("Discover"));
  params.append("other",           tr("Other"));
  params.append("key",             key);
  q = mql.toQuery(params);
  _cc->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customer::sLoadProspect(int prospectId)
{
  _notice = FALSE;
  _custid = prospectId;
  q.prepare("SELECT * FROM prospect WHERE (prospect_id=:prospect_id);");
  q.bindValue(":prospect_id", prospectId);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("prospect_number").toString());
    _name->setText(q.value("prospect_name").toString());
    _active->setChecked(q.value("prospect_active").toBool());
    _taxauth->setId(q.value("prospect_taxauth_id").toInt());
    _notes->setText(q.value("prospect_comments").toString());
    _billCntct->setId(q.value("prospect_cntct_id").toInt());
  }
  _name->setFocus();
}

void customer::sLoadCrmAcct(int crmacctId )
{
  _notice = FALSE;
  _crmacctid = crmacctId;
  q.prepare("SELECT * FROM crmacct WHERE (crmacct_id=:crmacct_id);");
  q.bindValue(":crmacct_id", crmacctId);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("crmacct_number").toString());
    _name->setText(q.value("crmacct_name").toString());
    _active->setChecked(q.value("crmacct_active").toBool());
  }
  _name->setFocus();
}

void customer::sNumberEdited()
{
  _notice = TRUE;
}

void customer::closeEvent(QCloseEvent *pEvent)
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

