/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendor.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "addresscluster.h"
#include "comment.h"
#include "crmaccount.h"
#include "dspAPApplications.h"
#include "dspCheckRegister.h"
#include "dspPOsByVendor.h"
#include "dspPoItemReceivingsByVendor.h"
#include "dspVendorAPHistory.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "selectPayments.h"
#include "storedProcErrorLookup.h"
#include "taxRegistration.h"
#include "unappliedAPCreditMemos.h"
#include "vendorAddress.h"
#include "xcombobox.h"

#define DEBUG false

vendor::vendor(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _number->setShowInactive(true);
  
  QWidget *hideme = 0;
  
  if (_privileges->check("ViewPurchaseOrders"))
  {
    _po = new dspPOsByVendor(this, "dspPOsByVendor", Qt::Widget);
    _purchaseOrdersPage->layout()->addWidget(_po);
    _po->setCloseVisible(false);
    hideme = _po->findChild<QWidget*>("_vendGroup");
    hideme->hide();
    VendorGroup *povend = _po->findChild<VendorGroup*>("_vend");
    if (povend)
    {
      povend->setState(VendorGroup::Selected);
      connect(povend,  SIGNAL(newVendId(int)), _po,    SLOT(sFillList()));
      connect(_number, SIGNAL(newId(int)),     povend, SLOT(setVendId(int)));
    }
    _po->show();

    _receipts = new dspPoItemReceivingsByVendor(this, "dspPoItemReceivingsByVendor", Qt::Widget);
    _receiptsReturnsPage->layout()->addWidget(_receipts);
    _receipts->setCloseVisible(false);
    hideme = _receipts->findChild<QWidget*>("_vendorGroup");
    hideme->hide();
    QWidget *rcptvend = _receipts->findChild<QWidget*>("_vendor");
    rcptvend->hide();
    connect(rcptvend, SIGNAL(newId(int)), _receipts,     SLOT(sFillList()));
    connect(_number,  SIGNAL(newId(int)), rcptvend,      SLOT(setId(int)));
  }
  else
    _tabs->setTabEnabled(_tabs->indexOf(_ordersTab), false);
  
  if (_privileges->check("MaintainPayments"))
  {
    _payables = new selectPayments(this, "selectPayments", Qt::Widget, false);
    _payablesPage->layout()->addWidget(_payables);
    hideme = _payables->findChild<QWidget*>("_close");
    hideme->hide();
    VendorGroup *payvend = _payables->findChild<VendorGroup*>("_vendorgroup");
    payvend->setState(VendorGroup::Selected);
    payvend->hide();
    connect(payvend, SIGNAL(newVendId(int)), _payables,     SLOT(sFillList()));
    connect(_number, SIGNAL(newId(int)),     payvend,       SLOT(setVendId(int)));
  }
  else
    _payablesButton->setEnabled(false);
  
  if (_privileges->check("MaintainAPMemos") ||
      _privileges->check("ViewAPMemos"))
  {
    _credits = new unappliedAPCreditMemos(this, "unappliedAPCreditMemos", Qt::Widget);
    _creditMemosPage->layout()->addWidget(_credits);
    hideme = _credits->findChild<QWidget*>("_close");
    hideme->hide();
    VendorGroup *cmvend = _credits->findChild<VendorGroup*>("_vendorgroup");
    cmvend->setState(VendorGroup::Selected);
    cmvend->hide();
    connect(cmvend,  SIGNAL(newVendId(int)), _credits,      SLOT(sFillList()));
    connect(_number, SIGNAL(newId(int)),     cmvend,        SLOT(setVendId(int)));
  }
  else
    _creditMemosButton->setEnabled(false);
  
  if (_privileges->check("MaintainPayments"))
  {
    _checks = new dspCheckRegister(this, "dspCheckRegister", Qt::Widget);
    _apChecksPage->layout()->addWidget(_checks);
    _checks->findChild<QWidget*>("_close")->hide();
    _checks->findChild<QGroupBox*>("_recipGroup")->setChecked(true);
    _checks->findChild<QGroupBox*>("_recipGroup")->hide();
    _checks->findChild<DateCluster*>("_dates")->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
    _checks->findChild<DateCluster*>("_dates")->setEndNull(tr("Latest"),	  omfgThis->endOfTime(),   true);
    VendorCluster *checkvend = _checks->findChild<VendorCluster*>("_vend");
    connect(checkvend, SIGNAL(newId(int)), _checks,       SLOT(sFillList()));
    connect(_number,   SIGNAL(newId(int)), checkvend,     SLOT(setId(int)));
  }
  else
    _apChecksButton->setEnabled(false);
  
  if (_privileges->check("ViewAPOpenItems"))
  {
    _history = new dspVendorAPHistory(this, "dspVendorAPHistory", Qt::Widget);
    _apHistoryPage->layout()->addWidget(_history);
    _history->setCloseVisible(false);
    _history->findChild<QWidget*>("_vendGroup")->hide();
    _history->findChild<DateCluster*>("_dates")->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
    _history->findChild<DateCluster*>("_dates")->setEndNull(tr("Latest"),	  omfgThis->endOfTime(),   true);
    VendorCluster *histvend = _history->findChild<VendorCluster*>("_vend");
    connect(histvend, SIGNAL(newId(int)), _history,      SLOT(sFillList()));
    connect(_number,  SIGNAL(newId(int)), histvend,      SLOT(setId(int)));
  }
  else
    _apHistoryButton->setEnabled(false);
  
  connect(_crmacct,               SIGNAL(clicked()),                       this,         SLOT(sCrmAccount()));
  connect(_save,                  SIGNAL(clicked()),                       this,         SLOT(sSaveClicked()));
  connect(_printAddresses,        SIGNAL(clicked()),                       this,         SLOT(sPrintAddresses()));
  connect(_newAddress,            SIGNAL(clicked()),                       this,         SLOT(sNewAddress()));
  connect(_editAddress,           SIGNAL(clicked()),                       this,         SLOT(sEditAddress()));
  connect(_viewAddress,           SIGNAL(clicked()),                       this,         SLOT(sViewAddress()));
  connect(_deleteAddress,         SIGNAL(clicked()),                       this,         SLOT(sDeleteAddress()));
  connect(_deleteTaxreg,          SIGNAL(clicked()),                       this,         SLOT(sDeleteTaxreg()));
  connect(_editTaxreg,            SIGNAL(clicked()),                       this,         SLOT(sEditTaxreg()));
  connect(_newTaxreg,             SIGNAL(clicked()),                       this,         SLOT(sNewTaxreg()));
  connect(_viewTaxreg,            SIGNAL(clicked()),                       this,         SLOT(sViewTaxreg()));
  connect(_next,                  SIGNAL(clicked()),                       this,         SLOT(sNext()));
  connect(_previous,              SIGNAL(clicked()),                       this,         SLOT(sPrevious()));
  connect(_generalButton,         SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_taxButton,             SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_mainButton,            SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_altButton,             SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_summaryButton,         SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_purchaseOrdersButton,  SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_receiptsReturnsButton, SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_payablesButton,        SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_creditMemosButton,     SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_apChecksButton,        SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_apHistoryButton,       SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_notesButton,           SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_commentsButton,        SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_checksButton,          SIGNAL(clicked()),                       this,         SLOT(sHandleButtons()));
  connect(_number,                SIGNAL(newId(int)),                      this,         SLOT(setId(int)));
  connect(_number,                SIGNAL(editingFinished()),               this,         SLOT(sNumberEdited()));
  connect(_number,                SIGNAL(editable(bool)),                  this,         SLOT(sNumberEditable(bool)));
  connect(_number,                SIGNAL(editingFinished()),               this,         SLOT(sCheckRequired()));

  connect(_address, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _contact2, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  connect(_address, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _contact1, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

// Have to override comment *New* button and possibly force Vendor save first
  QWidget* _newComment = _comments->findChild<QWidget*>("_newComment");
  disconnect(_newComment, SIGNAL(clicked()), _comments, SLOT(sNew()));
  connect(_newComment,    SIGNAL(clicked()),      this, SLOT(sSaveAndNewComment()));

  _defaultCurr->setLabel(_defaultCurrLit);

  QRegExp tmpregex = QRegExp(_metrics->value("EFTAccountRegex"));
  _accountValidator = new QRegExpValidator (tmpregex, this);
  tmpregex = QRegExp(_metrics->value("EFTRoutingRegex"));
  _routingValidator = new QRegExpValidator(tmpregex, this);

  _routingNumber->setValidator(_routingValidator);
  _achAccountNumber->setValidator(_accountValidator);

  _vendaddr->addColumn(tr("Number"),        70,              Qt::AlignLeft,   true,  "vendaddr_code");
  _vendaddr->addColumn(tr("Name"),          50,              Qt::AlignLeft,   true,  "vendaddr_name");
  _vendaddr->addColumn(tr("City"),          -1,              Qt::AlignLeft,   true,  "addr_city");
  _vendaddr->addColumn(tr("State"),         -1,              Qt::AlignLeft,   true,  "addr_state");
  _vendaddr->addColumn(tr("Country"),       -1,              Qt::AlignLeft,   true,  "addr_country");
  _vendaddr->addColumn(tr("Postal Code"),   -1,              Qt::AlignLeft,   true,  "addr_postalcode");

  _taxreg->addColumn(tr("Tax Authority"),   100,             Qt::AlignLeft,   true,  "taxauth_code");
  _taxreg->addColumn(tr("Tax Zone"),        100,             Qt::AlignLeft,   true,  "taxzone_code");
  _taxreg->addColumn(tr("Registration #"),  -1,              Qt::AlignLeft,   true,  "taxreg_number");

  _transmitStack->setCurrentIndex(0);
  if (_metrics->boolean("EnableBatchManager") &&
      ! (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled")))
  {
    _checksButton->hide();
  }
  else if (! _metrics->boolean("EnableBatchManager") &&
           (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled")))
  {
    _checksButton->hide();
    _transmitStack->setCurrentIndex(1);
  }
  else if (! _metrics->boolean("EnableBatchManager") &&
           ! (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled")))
    ediTab->setVisible(false);
  // else defaults are OK

  if (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled") && omfgThis->_key.isEmpty())
    _checksButton->setEnabled(false);


  _charass->setType("V");
  
  _accountType->append(0, "Checking", "K");
  _accountType->append(1, "Savings",  "C");
  _account->setType(GLCluster::cRevenue | GLCluster::cExpense |
                    GLCluster::cAsset | GLCluster::cLiability);

  _vendid      = -1;
  _crmacctid   = -1;
  _captive     = false;
  _NumberGen   = -1;
}

vendor::~vendor()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendor::languageChange()
{
  retranslateUi(this);
}

SetResponse vendor::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
  {
    _number->setEditMode(true);
//    setId(param.toInt());
    _number->setId(param.toInt());
    _captive=true;
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      emit newMode(_mode);
      
      clear();

      if (_privileges->check("MaintainVendorAddresses"))
      {
        connect(_vendaddr, SIGNAL(valid(bool)), _editAddress, SLOT(setEnabled(bool)));
        connect(_vendaddr, SIGNAL(valid(bool)), _deleteAddress, SLOT(setEnabled(bool)));
        connect(_vendaddr, SIGNAL(itemSelected(int)), _editAddress, SLOT(animateClick()));
      }
      else
      {
        _newAddress->setEnabled(false);
        connect(_vendaddr, SIGNAL(itemSelected(int)), _viewAddress, SLOT(animateClick()));
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      emit newMode(_mode);

      if (_privileges->check("MaintainVendorAddresses"))
      {
        connect(_vendaddr, SIGNAL(valid(bool)), _editAddress, SLOT(setEnabled(bool)));
        connect(_vendaddr, SIGNAL(valid(bool)), _deleteAddress, SLOT(setEnabled(bool)));
        connect(_vendaddr, SIGNAL(itemSelected(int)), _editAddress, SLOT(animateClick()));
      }
      else
      {
        _newAddress->setEnabled(false);
        connect(_vendaddr, SIGNAL(itemSelected(int)), _viewAddress, SLOT(animateClick()));
      }
    }
    else if (param.toString() == "view")
    {
      setViewMode();
    }
    _tempMode = _mode;
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _number->setEditMode(true);
    sLoadCrmAcct(param.toInt());
    _captive=true;
  }
  
  if(_metrics->value("CRMAccountNumberGeneration") == "A")
    _number->setEnabled(false);

  if(cNew == _mode || !pParams.inList("showNextPrev"))
  {
    _next->hide();
    _previous->hide();
  }

  if (_mode == cEdit && _vendid > 0)
  {
    if (!_lock.acquire("vendinfo", _vendid, AppLock::Interactive))
    {
      setViewMode();
    }
  }

  return NoError;
}

void vendor::setId(int p)
{
  if (_vendid==p)
    return;
  
  if (! _lock.release())
    ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                         _lock.lastError(), __FILE__, __LINE__);
  
  if (_mode == cEdit && !_lock.acquire("vendinfo", p, AppLock::Interactive))
    setViewMode();
  
  _vendid=p;
  sPopulate();
  emit newId(_vendid);
}

int vendor::id() const
{
  return _vendid;
}

/** \return one of cNew, cEdit, cView, ...
 \todo   change possible modes to an enum in guiclient.h (and add cUnknown?)
 */
int vendor::mode() const
{
  return _mode;
}

void vendor::setViewMode()
{
  _mode = cView;
  emit newMode(_mode);

  _number->setEnabled(false);
  _vendtype->setEnabled(false);
  _active->setEnabled(false);
  _name->setEnabled(false);
  _accountNumber->setEnabled(false);
  _defaultTerms->setEnabled(false);
  _defaultShipVia->setEnabled(false);
  _defaultCurr->setEnabled(false);
  _contact1->setEnabled(false);
  _contact2->setEnabled(false);
  _address->setEnabled(false);
  _notes->setReadOnly(true);
  _poComments->setReadOnly(true);
  _poItems->setEnabled(false);
  _restrictToItemSource->setEnabled(false);
  _receives1099->setEnabled(false);
  _qualified->setEnabled(false);
  _newAddress->setEnabled(false);
  _defaultFOBGroup->setEnabled(false);
  _taxzone->setEnabled(false);
  _match->setEnabled(false);
  _newTaxreg->setEnabled(false);
  _comments->setReadOnly(true);
  _documents->setReadOnly(true);
  _charass->setReadOnly(true);

  _achGroup->setEnabled(false);
  _routingNumber->setEnabled(false);
  _achAccountNumber->setEnabled(false);
  _individualId->setEnabled(false);
  _individualName->setEnabled(false);
  _accountType->setEnabled(false);
  _distribGroup->setEnabled(false);

  _save->hide();
  _close->setText(tr("&Close"));

  disconnect(_taxreg, SIGNAL(valid(bool)), _deleteTaxreg, SLOT(setEnabled(bool)));
  disconnect(_taxreg, SIGNAL(valid(bool)), _editTaxreg, SLOT(setEnabled(bool)));
  disconnect(_taxreg, SIGNAL(itemSelected(int)), _editTaxreg, SLOT(animateClick()));
  connect(_taxreg, SIGNAL(itemSelected(int)), _viewTaxreg, SLOT(animateClick()));

}

bool vendor::sSave()
{
  XSqlQuery vendorSave;
  QList<GuiErrorCheck> errors;
  errors
         << GuiErrorCheck(_number->number().trimmed().isEmpty(), _number,
                          tr("Please enter a Number for this new Vendor."))
         << GuiErrorCheck(_name->text().trimmed().isEmpty(), _name,
                          tr("Please enter a Name for this new Vendor."))
         << GuiErrorCheck(_defaultTerms->id() == -1, _defaultTerms,
                          tr("You must select a Terms code for this Vendor."))
         << GuiErrorCheck(_vendtype->id() == -1, _vendtype,
                          tr("You must select a Vendor Type for this Vendor."))
//         << GuiErrorCheck(_accountSelected->isChecked() &&
//                          !_account->isValid(),
//                          _account
//                          tr("You must select a Default Distribution Account for this Vendor."))
//         << GuiErrorCheck(_expcatSelected->isChecked() &&
//                          !_expcat->isValid(),
//                          _expcat
//                          tr("You must select a Default Distribution Expense Category for this Vendor."))
//         << GuiErrorCheck(_taxSelected->isChecked() &&
//                          !_taxCode->isValid(),
//                          _taxCode
//                          tr("You must select a Default Distribution Tax Code for this Vendor."))
         << GuiErrorCheck(_achGroup->isChecked() &&
                          ! _routingNumber->hasAcceptableInput() &&
                          !omfgThis->_key.isEmpty(),
                          _routingNumber,
                          tr("The Routing Number is not valid."))
         << GuiErrorCheck(_achGroup->isChecked() &&
                          ! _achAccountNumber->hasAcceptableInput() &&
                          !omfgThis->_key.isEmpty(), _achAccountNumber,
                          tr("The Account Number is not valid."))
         << GuiErrorCheck(_achGroup->isChecked() &&
                          _useACHSpecial->isChecked() &&
                          _individualName->text().trimmed().isEmpty() &&
                          !omfgThis->_key.isEmpty(),
                          _individualName,
                          tr("Please enter an Individual Name if EFT Check "
                             "Printing is enabled and '%1' is checked.")
                            .arg(_useACHSpecial->title()))
    ;

  if (_number->number().trimmed().toUpper() != _cachedNumber.toUpper())
  {
    XSqlQuery dupq;
    dupq.prepare("SELECT vend_name "
                 "FROM vendinfo "
                 "WHERE (UPPER(vend_number)=UPPER(:vend_number)) "
                 "  AND (vend_id<>:vend_id);" );
    dupq.bindValue(":vend_number", _number->number().trimmed());
    dupq.bindValue(":vend_id", _vendid);
    dupq.exec();
    if (dupq.first())
      GuiErrorCheck(true, _number,
                    tr("<p>The newly entered Vendor Number cannot be "
                       "used as it is already used by the Vendor '%1'. "
                       "Please correct or enter a new Vendor Number." )
                    .arg(vendorSave.value("vend_name").toString()) );
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Vendor"), errors))
    return false;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery begin("BEGIN;");
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                           begin, __FILE__, __LINE__))
    return false;

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("<p>There are multiple uses of this Vendor's "
		       "Address. What would you like to do?"),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
    if (0 == answer)
      saveResult = _address->save(AddressCluster::CHANGEONE);
    else if (1 == answer)
      saveResult = _address->save(AddressCluster::CHANGEALL);
  }
  if (saveResult < 0)	// not else-if: this is error check for CHANGE{ONE,ALL}
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Address"),
                         tr("<p>There was an error saving this address (%1). "
			 "Check the database server log for errors.")
                         .arg(saveResult), __FILE__, __LINE__);
    _address->setFocus();
    return false;
  }

  QString sql;
  if (_mode == cEdit || _tempMode == cEdit)
  {
    sql = "UPDATE vendinfo "
          "SET vend_number=<? value(\"vend_number\") ?>,"
          "    vend_accntnum=<? value(\"vend_accntnum\") ?>,"
          "    vend_active=<? value(\"vend_active\") ?>,"
          "    vend_vendtype_id=<? value(\"vend_vendtype_id\") ?>,"
          "    vend_name=<? value(\"vend_name\") ?>,"
          "    vend_cntct1_id=<? value(\"vend_cntct1_id\") ?>,"
          "    vend_cntct2_id=<? value(\"vend_cntct2_id\") ?>,"
	  "    vend_addr_id=<? value(\"vend_addr_id\") ?>,"
          "    vend_po=<? value(\"vend_po\") ?>,"
          "    vend_restrictpurch=<? value(\"vend_restrictpurch\") ?>,"
          "    vend_1099=<? value(\"vend_1099\") ?>,"
          "    vend_qualified=<? value(\"vend_qualified\") ?>,"
          "    vend_comments=<? value(\"vend_comments\") ?>,"
          "    vend_pocomments=<? value(\"vend_pocomments\") ?>,"
          "    vend_fobsource=<? value(\"vend_fobsource\") ?>,"
          "    vend_fob=<? value(\"vend_fob\") ?>,"
          "    vend_terms_id=<? value(\"vend_terms_id\") ?>,"
          "    vend_shipvia=<? value(\"vend_shipvia\") ?>,"
	  "    vend_curr_id=<? value(\"vend_curr_id\") ?>,"
          "    vend_taxzone_id=<? value(\"vend_taxzone_id\") ?>,"
          "    vend_match=<? value(\"vend_match\") ?>,"
          "    vend_ach_enabled=<? value(\"vend_ach_enabled\") ?>,"
          "<? if exists(\"key\") ?>"
          "    vend_ach_routingnumber=encrypt(setbytea(<? value(\"vend_ach_routingnumber\") ?>),"
          "                             setbytea(<? value(\"key\") ?>), 'bf'),"
          "    vend_ach_accntnumber=encrypt(setbytea(<? value(\"vend_ach_accntnumber\") ?>),"
          "                           setbytea(<? value(\"key\") ?>), 'bf'),"
          "<? endif ?>"
          "    vend_ach_use_vendinfo=<? value(\"vend_ach_use_vendinfo\") ?>,"
          "    vend_ach_accnttype=<? value(\"vend_ach_accnttype\") ?>,"
          "    vend_ach_indiv_number=<? value(\"vend_ach_indiv_number\") ?>,"
          "    vend_ach_indiv_name=<? value(\"vend_ach_indiv_name\") ?>,"
          "    vend_accnt_id=<? value(\"vend_accnt_id\") ?>,"
          "    vend_expcat_id=<? value(\"vend_expcat_id\") ?>,"
          "    vend_tax_id=<? value(\"vend_tax_id\") ?> "
          "WHERE (vend_id=<? value(\"vend_id\") ?>);" ;
  }
  else if (_mode == cNew)
    sql = "INSERT INTO vendinfo "
          "( vend_id, vend_number, vend_accntnum,"
          "  vend_active, vend_vendtype_id, vend_name,"
          "  vend_cntct1_id, vend_cntct2_id, vend_addr_id,"
          "  vend_po, vend_restrictpurch,"
          "  vend_1099, vend_qualified,"
          "  vend_comments, vend_pocomments,"
          "  vend_fobsource, vend_fob,"
          "  vend_terms_id, vend_shipvia, vend_curr_id,"
          "  vend_taxzone_id, vend_match, vend_ach_enabled,"
          "  vend_ach_routingnumber, vend_ach_accntnumber,"
          "  vend_ach_use_vendinfo,"
          "  vend_ach_accnttype, vend_ach_indiv_number,"
          "  vend_ach_indiv_name,"
          "  vend_accnt_id, vend_expcat_id, vend_tax_id) "
          "VALUES "
          "( <? value(\"vend_id\") ?>,"
          "  <? value(\"vend_number\") ?>,"
          "  <? value(\"vend_accntnum\") ?>,"
          "  <? value(\"vend_active\") ?>,"
          "  <? value(\"vend_vendtype_id\") ?>,"
          "  <? value(\"vend_name\") ?>,"
          "  <? value(\"vend_cntct1_id\") ?>,"
          "  <? value(\"vend_cntct2_id\") ?>,"
          "  <? value(\"vend_addr_id\") ?>,"
          "  <? value(\"vend_po\") ?>,"
          "  <? value(\"vend_restrictpurch\") ?>,"
          "  <? value(\"vend_1099\") ?>,"
          "  <? value(\"vend_qualified\") ?>,"
          "  <? value(\"vend_comments\") ?>,"
          "  <? value(\"vend_pocomments\") ?>,"
          "  <? value(\"vend_fobsource\") ?>,"
          "  <? value(\"vend_fob\") ?>,"
          "  <? value(\"vend_terms_id\") ?>,"
          "  <? value(\"vend_shipvia\") ?>,"
          "  <? value(\"vend_curr_id\") ?>, "
          "  <? value(\"vend_taxzone_id\") ?>,"
          "  <? value(\"vend_match\") ?>,"
          "  <? value(\"vend_ach_enabled\") ?>,"
          "<? if exists(\"key\") ?>"
          "  encrypt(setbytea(<? value(\"vend_ach_routingnumber\") ?>),"
          "          setbytea(<? value(\"key\") ?>), 'bf'),"
          "  encrypt(setbytea(<? value(\"vend_ach_accntnumber\") ?>),"
          "          setbytea(<? value(\"key\") ?>), 'bf'),"
          "<? else ?>"
          "  '',"
          "  '',"
          "<? endif ?>"
          "  <? value(\"vend_ach_use_vendinfo\") ?>,"
          "  <? value(\"vend_ach_accnttype\") ?>,"
          "  <? value(\"vend_ach_indiv_number\") ?>,"
          "  <? value(\"vend_ach_indiv_name\") ?>,"
          "  <? value(\"vend_accnt_id\") ?>,"
          "  <? value(\"vend_expcat_id\") ?>,"
          "  <? value(\"vend_tax_id\") ?> "
          "   );"  ;

  ParameterList params;
  params.append("vend_id", _vendid);
  params.append("vend_vendtype_id", _vendtype->id());
  params.append("vend_terms_id", _defaultTerms->id());
  params.append("vend_curr_id", _defaultCurr->id());

  params.append("vend_number",   _number->number().trimmed().toUpper());
  params.append("vend_accntnum", _accountNumber->text().trimmed());
  params.append("vend_name",     _name->text().trimmed());

  if (_contact1->id() > 0)
    params.append("vend_cntct1_id", _contact1->id());		// else NULL
  if (_contact2->id() > 0)
    params.append("vend_cntct2_id", _contact2->id());		// else NULL
  if (_address->id() > 0)
    params.append("vend_addr_id", _address->id());		// else NULL

  params.append("vend_comments",   _notes->toPlainText());
  params.append("vend_pocomments", _poComments->toPlainText());
  params.append("vend_shipvia",    _defaultShipVia->text());

  params.append("vend_active",        QVariant(_active->isChecked()));
  params.append("vend_po",            QVariant(_poItems->isChecked()));
  params.append("vend_restrictpurch", QVariant(_restrictToItemSource->isChecked()));
  params.append("vend_1099",          QVariant(_receives1099->isChecked()));
  params.append("vend_qualified",     QVariant(_qualified->isChecked()));
  params.append("vend_match",         QVariant(_match->isChecked()));

  if (!omfgThis->_key.isEmpty())
    params.append("key",                   omfgThis->_key);
  params.append("vend_ach_enabled",      QVariant(_achGroup->isChecked()));
  params.append("vend_ach_routingnumber",_routingNumber->text().trimmed());
  params.append("vend_ach_accntnumber",  _achAccountNumber->text().trimmed());
  params.append("vend_ach_use_vendinfo", QVariant(! _useACHSpecial->isChecked()));
  params.append("vend_ach_indiv_number", _individualId->text().trimmed());
  params.append("vend_ach_indiv_name",   _individualName->text().trimmed());

  params.append("vend_ach_accnttype",  _accountType->code());

  if(_taxzone->isValid())
    params.append("vend_taxzone_id", _taxzone->id());

  if (_useWarehouseFOB->isChecked())
  {
    params.append("vend_fobsource", "W");
    params.append("vend_fob", "");
  }
  else if (_useVendorFOB)
  {
    params.append("vend_fobsource", "V");
    params.append("vend_fob", _vendorFOB->text().trimmed());
  }

  if(_accountSelected->isChecked() && _account->isValid())
  {
    params.append("vend_accnt_id", _account->id());
    params.append("vend_expcat_id", -1);
    params.append("vend_tax_id", -1);
  }
  else if (_expcatSelected->isChecked() && _expcat->isValid())
  {
    params.append("vend_accnt_id", -1);
    params.append("vend_expcat_id", _expcat->id());
    params.append("vend_tax_id", -1);
  }
  else if (_taxSelected->isChecked() && _taxCode->isValid())
  {
    params.append("vend_accnt_id", -1);
    params.append("vend_expcat_id", -1);
    params.append("vend_tax_id", _taxCode->id());
  }
  else
  {
    params.append("vend_accnt_id", -1);
    params.append("vend_expcat_id", -1);
    params.append("vend_tax_id", -1);
  }

  MetaSQLQuery mql(sql);
  XSqlQuery upsq = mql.toQuery(params);
  if (upsq.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Vendor"),
                         vendorSave, __FILE__, __LINE__);
    return false;
  }

  XSqlQuery commit("COMMIT;");

  // We also need to find out the CRMAcct in case of adding Comments
  if (_mode == cNew && _crmacctid == -1)
  {
    sql = "SELECT crmacct_id FROM crmacct WHERE (crmacct_vend_id = <? value('vend_id') ?>);";
    MetaSQLQuery mql(sql);
    XSqlQuery crma = mql.toQuery(params);
    if (crma.first())
    {
      _crmacctid = crma.value("crmacct_id").toInt();
    }
    else if (crma.lastError().type() != QSqlError::NoError)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Vendor CRM Account"),
                         crma, __FILE__, __LINE__);
      return false;
    }
  }
  _tempMode = cEdit;

  return true;
}

void vendor::sSaveClicked()
{
  _save->setFocus();
  
  if (!sSave())
    return;
  
//  _autoSaved=false;
  _NumberGen = -1;
  omfgThis->sVendorsUpdated();
  emit saved(_vendid);
  if (_captive || isModal())
    close();
  else
    clear();
}

void vendor::sCheck()
{
  _number->setNumber(_number->number().trimmed().toUpper());
  if (_number->number().length() && _cachedNumber != _number->number())
  {
    if(cNew == _mode && -1 != _NumberGen && _number->number().toInt() != _NumberGen)
    {
      XSqlQuery query;
      query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
      query.bindValue(":Number", _NumberGen);
      query.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Releasing Number"),
                           query, __FILE__, __LINE__);
      _NumberGen = -1;
    }

    XSqlQuery dupq;
    dupq.prepare("SELECT vend_id, 1 AS type"
                 "  FROM vendinfo "
                 " WHERE (vend_number=:vend_number)"
                 " UNION "
                 "SELECT crmacct_id, 2 AS type "
                 "  FROM crmacct "
                 " WHERE (crmacct_number=:vend_number)"
                 " ORDER BY type;");
    dupq.bindValue(":vend_number", _number->number());
    dupq.exec();
    if (dupq.first())
    {
      if ((dupq.value("type").toInt() == 1) && (_notice))
      {
        if (QMessageBox::question(this, tr("Vendor Exists"),
                                  tr("<p>This number is currently "
                                     "used by an existing Vendor. "
                                     "Do you want to edit "
                                     "that Vendor?"),
                         QMessageBox::Yes,
                         QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
        {
          _number->setNumber(_cachedNumber);
          _number->setFocus();
          return;
        }

        if (! _lock.release())
          ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                               _lock.lastError(), __FILE__, __LINE__);

        _vendid = dupq.value("vend_id").toInt();

        if (_mode == cEdit && !_lock.acquire("vendinfo", _vendid, AppLock::Interactive))
        {
          setViewMode();
        }

        _mode = cEdit;
        emit newMode(_mode);
        sPopulate();
        _name->setFocus();
      }
      else if ( (_mode == cEdit) &&
                ((dupq.value("type").toInt() == 2) ) &&
                (_notice))
      {
        QMessageBox::critical(this, tr("Invalid Number"),
                              tr("<p>This number is currently "
                                 "assigned to another Account."));
        _number->setNumber(_cachedNumber);
        _number->setFocus();
        _notice = false;
        return;
      }
      else if ((dupq.value("type").toInt() == 2) && (_notice))
      {
        if (QMessageBox::question(this, tr("Convert"),
                                  tr("<p>This number is currently assigned to Account. "
                                     "Do you want to convert the Account to a Vendor?"),
                         QMessageBox::Yes,
                         QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
        {
          _number->setId(-1);
          _number->setFocus();
          return;
        }
        sLoadCrmAcct(dupq.value("vend_id").toInt());
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor"),
                                  dupq, __FILE__, __LINE__))
      return;
  }
}

bool vendor::sCheckRequired()
{
  if ( ( _number->number().trimmed().length() == 0) ||
       (_name->text().trimmed().length() == 0) ||
       (_vendid == -1) )
  {
    return false;
  }
  return true;
}

void vendor::sLoadCrmAcct(int crmacctId)
{
  _notice = false;
  _crmacctid = crmacctId;

  XSqlQuery getq;
  getq.prepare("SELECT * FROM crmacct WHERE (crmacct_id=:crmacct_id);");
  getq.bindValue(":crmacct_id", crmacctId);
  getq.exec();
  if (getq.first())
  {
    _crmowner = getq.value("crmacct_owner_username").toString();
    _number->setCanEdit(true);
    _number->setEditMode(true);
    _number->setNumber(getq.value("crmacct_number").toString());
    _cachedNumber=_number->number().trimmed().toUpper();
    _name->setText(getq.value("crmacct_name").toString());
    _active->setChecked(getq.value("crmacct_active").toBool());

    _contact1->setId(getq.value("crmacct_cntct_id_1").toInt());
    _contact1->setSearchAcct(_crmacctid);
    _contact2->setId(getq.value("crmacct_cntct_id_2").toInt());
    _contact2->setSearchAcct(_crmacctid);

    if (getq.value("crmacct_cntct_id_1").toInt() != 0)
    {
      XSqlQuery contactQry;
      contactQry.prepare("SELECT cntct_addr_id FROM cntct WHERE (cntct_id=:cntct_id);");
      contactQry.bindValue(":cntct_id", _contact1->id());
      contactQry.exec();
      if (contactQry.first())
      {
        _address->setId(contactQry.value("cntct_addr_id").toInt());
        _address->setSearchAcct(_crmacctid);
      }
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Account"),
                           getq, __FILE__, __LINE__))
    return;

  _crmacct->setEnabled(_crmacctid > 0 &&
                       (_privileges->check("MaintainAllCRMAccounts") ||
                        _privileges->check("ViewAllCRMAccounts") ||
                        (omfgThis->username() == _crmowner && _privileges->check("MaintainPersonalCRMAccounts")) ||
                        (omfgThis->username() == _crmowner && _privileges->check("ViewPersonalCRMAccounts"))));

  _name->setFocus();
}

bool vendor::sPopulate()
{
  if (DEBUG)
    qDebug("vendor::sPopulate() entered with _vendid %d and _crmacctid %d",
           _vendid, _crmacctid);

  XSqlQuery vendorPopulate;
  ParameterList params;

  MetaSQLQuery mql(
            "<? if exists('vend_id') ?>"
            "SELECT vendinfo.*, crmacct_id, crmacct_owner_username, "
            "<? if exists('key') ?>"
            "       CASE WHEN LENGTH(vend_ach_routingnumber) > 0 THEN"
            "       formatbytea(decrypt(setbytea(vend_ach_routingnumber),"
            "                           setbytea(<? value('key') ?>), 'bf'))"
            "            ELSE '' END AS routingnum,"
            "       CASE WHEN LENGTH(vend_ach_accntnumber) > 0 THEN"
            "       formatbytea(decrypt(setbytea(vend_ach_accntnumber),"
            "                           setbytea(<? value('key') ?>), 'bf'))"
            "            ELSE '' END AS accntnum "
            "<? else ?>"
            "       <? value('na') ?> AS routingnum,"
            "       <? value('na') ?> AS accntnum "
            "<? endif ?>"
            "FROM vendinfo "
            "  JOIN crmacct ON (vend_id=crmacct_vend_id) "
            "WHERE (vend_id=<? value('vend_id') ?>);"
            "<? elseif exists('crmacct_id') ?>"
            "SELECT crmacct_number AS vend_number, crmacct_name   AS vend_name,"
            "       crmacct_active AS vend_active,"
            "       crmacct_cntct_id_1 AS vend_cntct1_id,"
            "       crmacct_cntct_id_2 AS vend_cntct2_id,"
            "       fetchMetricText('DefaultPOShipVia') AS vend_shipvia,"
            "       NULL AS vend_accntnum,    NULL AS vend_vendtype_id,"
            "       NULL AS vend_name,        NULL AS vend_addr_id,"
            "       fetchMetricValue('DefaultTerms') AS vend_terms_id,"
            "       NULL  AS vend_curr_id,"
            "       false AS vend_po,         false AS vend_restrictpurch,"
            "       false AS vend_1099,       NULL AS vend_match,"
            "       false  AS vend_qualified, NULL AS vend_comments,"
            "       NULL AS vend_pocomments,  NULL AS vend_taxzone_id,"
            "       -1 AS vend_expcat_id, -1 AS vend_tax_id,"
            "       'W'  AS vend_fobsource,   NULL AS vend_fob,"
            "       NULL AS vend_ach_enabled, NULL AS routingnum,"
            "       NULL AS accntnum,         NULL AS vend_ach_use_vendinfo,"
            "       NULL AS vend_ach_indiv_number, NULL AS vend_ach_indiv_name,"
            "       NULL AS vend_ach_accnttype,"
            "       crmacct_id, crmacct_owner_username"
            "  FROM crmacct"
            " WHERE crmacct_id=<? value('crmacct_id') ?>;"
            "<? endif ?>");

  if (_vendid > 0)
    params.append("vend_id", _vendid);
  else if (_crmacctid > 0)
    params.append("crmacct_id", _crmacctid);

  params.append("key",     omfgThis->_key);
  params.append("na",      tr("N/A"));
  vendorPopulate = mql.toQuery(params);
  if (vendorPopulate.first())
  {
    if (_mode == cNew)
    {
      _mode = cEdit;
      emit newMode(_mode);
    }
    
    _notice = false;
    _cachedNumber = vendorPopulate.value("vend_number").toString();

    _crmacctid = vendorPopulate.value("crmacct_id").toInt();
    _crmowner = vendorPopulate.value("crmacct_owner_username").toString();
    _number->setNumber(vendorPopulate.value("vend_number").toString());
    _accountNumber->setText(vendorPopulate.value("vend_accntnum"));
    _vendtype->setId(vendorPopulate.value("vend_vendtype_id").toInt());
    _active->setChecked(vendorPopulate.value("vend_active").toBool());
    _name->setText(vendorPopulate.value("vend_name"));
    _contact1->setId(vendorPopulate.value("vend_cntct1_id").toInt());
    _contact1->setSearchAcct(_crmacctid);
    _contact2->setId(vendorPopulate.value("vend_cntct2_id").toInt());
    _contact2->setSearchAcct(_crmacctid);
    _address->setId(vendorPopulate.value("vend_addr_id").toInt());
    _defaultTerms->setId(vendorPopulate.value("vend_terms_id").toInt());
    _defaultShipVia->setText(vendorPopulate.value("vend_shipvia").toString());
    _defaultCurr->setId(vendorPopulate.value("vend_curr_id").toInt());
    _poItems->setChecked(vendorPopulate.value("vend_po").toBool());
    _restrictToItemSource->setChecked(vendorPopulate.value("vend_restrictpurch").toBool());
    _receives1099->setChecked(vendorPopulate.value("vend_1099").toBool());
    _match->setChecked(vendorPopulate.value("vend_match").toBool());
    _qualified->setChecked(vendorPopulate.value("vend_qualified").toBool());
    _notes->setText(vendorPopulate.value("vend_comments").toString());
    _poComments->setText(vendorPopulate.value("vend_pocomments").toString());

    _taxzone->setId(vendorPopulate.value("vend_taxzone_id").toInt());

    if (vendorPopulate.value("vend_fobsource").toString() == "V")
    {
      _useVendorFOB->setChecked(true);
      _vendorFOB->setText(vendorPopulate.value("vend_fob"));
    }
    else
      _useWarehouseFOB->setChecked(true);

    _achGroup->setChecked(vendorPopulate.value("vend_ach_enabled").toBool());
    _routingNumber->setText(vendorPopulate.value("routingnum").toString());
    _achAccountNumber->setText(vendorPopulate.value("accntnum").toString());
    _useACHSpecial->setChecked(! vendorPopulate.value("vend_ach_use_vendinfo").toBool());
    _individualId->setText(vendorPopulate.value("vend_ach_indiv_number").toString());
    _individualName->setText(vendorPopulate.value("vend_ach_indiv_name").toString());

    _accountType->setCode(vendorPopulate.value("vend_ach_accnttype").toString());

    _account->setId(vendorPopulate.value("vend_accnt_id").toInt());
    if(vendorPopulate.value("vend_expcat_id").toInt() != -1)
    {
      _expcatSelected->setChecked(true);
      _expcat->setId(vendorPopulate.value("vend_expcat_id").toInt());
    }
    if(vendorPopulate.value("vend_tax_id").toInt() != -1)
    {
      _taxSelected->setChecked(true);
      _taxCode->setId(vendorPopulate.value("vend_tax_id").toInt());
    }

    sFillAddressList();
    sFillTaxregList();

    _comments->setId(_crmacctid);
    _documents->setId(_vendid);
    _address->setSearchAcct(_crmacctid);
    _charass->setId(_vendid);

    emit newId(_vendid);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor"),
                                vendorPopulate, __FILE__, __LINE__))
    return false;
  else
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor"),
                         tr("Could not find the Vendor information. Perhaps "
                            "the Vendor and Account have been disconnected."),
                __FILE__, __LINE__);
    return false;
  }

  _crmacct->setEnabled(_crmacctid > 0 &&
                       (_privileges->check("MaintainAllCRMAccounts") ||
                        _privileges->check("ViewAllCRMAccounts") ||
                        (omfgThis->username() == _crmowner && _privileges->check("MaintainPersonalCRMAccounts")) ||
                        (omfgThis->username() == _crmowner && _privileges->check("ViewPersonalCRMAccounts"))));

  MetaSQLQuery pos("SELECT MIN(pohead_orderdate) AS minpodate, "
                   "       MAX(pohead_orderdate) AS maxpodate, "
                   "       SUM(currToBase(pohead_curr_id,"
                   "           (poitem_qty_ordered - poitem_qty_received) * poitem_unitprice,"
                   "           CURRENT_DATE)) AS backlog "
                   "FROM vendinfo"
                   "     LEFT OUTER JOIN pohead ON (pohead_vend_id=vend_id)"
                   "     LEFT OUTER JOIN poitem ON (poitem_pohead_id=pohead_id"
                   "                            AND poitem_status='O')"
                   "WHERE (vend_id=<? value(\"vend_id\") ?>);");
  
  vendorPopulate = pos.toQuery(params);
  if (vendorPopulate.first())
  {
    _firstPurchase->setDate(vendorPopulate.value("minpodate").toDate());
    _lastPurchase->setDate(vendorPopulate.value("maxpodate").toDate());
    _backlog->setDouble(vendorPopulate.value("backlog").toDouble());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Vendor Information"),
                                vendorPopulate, __FILE__, __LINE__))
  {
    return false;
  }
  
  MetaSQLQuery purchbydate("SELECT SUM(currToBase(vohead_curr_id,"
                           "             vohead_amount,"
                           "             vohead_gldistdate)) AS purchases "
                           "FROM vohead JOIN apopen ON (apopen_doctype='V' AND"
                           "                            apopen_docnumber=vohead_number AND"
                           "                            NOT apopen_void) "
                           "WHERE (vohead_posted"
                           "  AND (vohead_gldistdate "
                           "       BETWEEN (<? literal(\"older\") ?>)"
                           "           AND (<? literal(\"younger\") ?>))"
                           "  AND (vohead_vend_id=<? value(\"vend_id\") ?>));");
  params.append("older",   "DATE_TRUNC('year', CURRENT_DATE) - INTERVAL '1 year'");
  params.append("younger", "DATE_TRUNC('year', CURRENT_DATE) - INTERVAL '1 day'");
  vendorPopulate = purchbydate.toQuery(params);
  if (vendorPopulate.first())
    _lastYearsPurchases->setDouble(vendorPopulate.value("purchases").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Vendor Information"),
                                vendorPopulate, __FILE__, __LINE__))
  {
    return false;
  }
  
  ParameterList ytdparams;
  ytdparams.append("vend_id", _number->id());
  ytdparams.append("older",   "DATE_TRUNC('year', CURRENT_DATE)");
  ytdparams.append("younger", "CURRENT_DATE");
  vendorPopulate = purchbydate.toQuery(ytdparams);
  if (vendorPopulate.first())
    _ytdPurchases->setDouble(vendorPopulate.value("purchases").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Vendor Information"),
                                vendorPopulate, __FILE__, __LINE__))
  {
    return false;
  }
  
  MetaSQLQuery balm("SELECT COALESCE(SUM((apopen_amount-apopen_paid) / apopen_curr_rate * "
                    "  CASE WHEN (apopen_doctype IN ('D','V')) THEN 1 ELSE -1 END), 0.0) AS balance "
                    "FROM apopen "
                    "WHERE ((apopen_open)"
                    "   AND (apopen_vend_id=<? value(\"vend_id\") ?>));");
  vendorPopulate = balm.toQuery(params);
  if (vendorPopulate.first())
    _openBalance->setDouble(vendorPopulate.value("balance").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retreiving Vendor Information"),
                                vendorPopulate, __FILE__, __LINE__))
  {
    return false;
  }

  emit populated();
  return true;
}

void vendor::sPrintAddresses()
{
  ParameterList params;
  params.append("vend_id", _vendid);

  orReport report("VendorAddressList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void vendor::sNewAddress()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("vend_id", _vendid);

  vendorAddress newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillAddressList();
}

void vendor::sEditAddress()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vendaddr_id", _vendaddr->id());

  vendorAddress newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillAddressList();
}

void vendor::sViewAddress()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("vendaddr_id", _vendaddr->id());

  vendorAddress newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void vendor::sDeleteAddress()
{
  XSqlQuery delq;
  delq.prepare( "DELETE FROM vendaddrinfo "
             "WHERE (vendaddr_id=:vendaddr_id);" );
  delq.bindValue(":vendaddr_id", _vendaddr->id());
  delq.exec();
  sFillAddressList();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor Addresses"),
                           delq, __FILE__, __LINE__))
    return;
}

void vendor::sFillAddressList()
{
  XSqlQuery addrq;
  addrq.prepare("SELECT vendaddr_id, vendaddr_code, vendaddr_name,"
                "       addr_city,   addr_state,    addr_country,"
                "       addr_postalcode"
                " FROM vendaddrinfo"
                " LEFT OUTER JOIN addr ON (vendaddr_addr_id=addr_id)"
                " WHERE (vendaddr_vend_id=:vend_id)"
                " ORDER BY vendaddr_code;" );
  addrq.bindValue(":vend_id", _vendid);
  addrq.exec();
  _vendaddr->populate(addrq);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor Addresses"),
                           addrq, __FILE__, __LINE__))
    return;
}

void vendor::sFillTaxregList()
{
  XSqlQuery taxreg;
  taxreg.prepare("SELECT taxreg_id, taxreg_taxauth_id,"
                 "       taxauth_code, taxzone_code, taxreg_number"
                 "  FROM taxreg"
                 "  JOIN taxauth ON (taxreg_taxauth_id=taxauth_id)"
                 "  LEFT OUTER JOIN taxzone ON (taxreg_taxzone_id=taxzone_id)"
                 " WHERE ((taxreg_rel_type='V') "
                 "    AND (taxreg_rel_id=:vend_id));");
  taxreg.bindValue(":vend_id", _vendid);
  taxreg.exec();
  _taxreg->clear();
  _taxreg->populate(taxreg, true);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Tax Registrations"),
                           taxreg, __FILE__, __LINE__))
    return;
}

void vendor::sNewTaxreg()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("taxreg_rel_id", _vendid);
  params.append("taxreg_rel_type", "V");

  taxRegistration newdlg(this, "", true);
  if (newdlg.set(params) == NoError && newdlg.exec() != XDialog::Rejected)
    sFillTaxregList();
}

void vendor::sEditTaxreg()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("taxreg_id", _taxreg->id());

  taxRegistration newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.set(params) == NoError && newdlg.exec() != XDialog::Rejected)
    sFillTaxregList();
}

void vendor::sViewTaxreg()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxreg_id", _taxreg->id());

  taxRegistration newdlg(this, "", true);
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void vendor::sDeleteTaxreg()
{
  XSqlQuery delq;
  delq.prepare("DELETE FROM taxreg WHERE (taxreg_id=:taxreg_id);");
  delq.bindValue(":taxreg_id", _taxreg->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Tax Registration"),
                           delq, __FILE__, __LINE__))
    return;

  sFillTaxregList();
}

void vendor::sNext()
{
  XSqlQuery vendorNext;
  vendorNext.prepare("SELECT vend_id "
            "  FROM vendinfo"
            " WHERE (:number < vend_number)"
            " ORDER BY vend_number"
            " LIMIT 1;");
  vendorNext.bindValue(":number", _number->number());
  vendorNext.exec();
  if(!vendorNext.first())
  {
    QMessageBox::information(this, tr("At Last Record"),
       tr("You are already on the last record.") );
    return;
  }
  int newid = vendorNext.value("vend_id").toInt();

  if(!sCheckSave())
    return;

  clear();

  _vendid = newid;
  _mode = cEdit;
  emit newMode(_mode);
  sPopulate();
}

void vendor::sPrevious()
{
  XSqlQuery nextq;
  nextq.prepare("SELECT vend_id "
            "  FROM vendinfo"
            " WHERE (:number > vend_number)"
            " ORDER BY vend_number DESC"
            " LIMIT 1;");
  nextq.bindValue(":number", _number->number());
  nextq.exec();
  if (!nextq.first() && nextq.lastError().type() == QSqlError::NoError)
  {
    QMessageBox::information(this, tr("At First Record"),
                             tr("You are already on the first record.") );
    return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor"),
                                nextq, __FILE__, __LINE__))
    return;
  int newid = nextq.value("vend_id").toInt();

  if(!sCheckSave())
    return;

  clear();

  _vendid = newid;
  _mode = cEdit;
  emit newMode(_mode);
  sPopulate();
}

bool vendor::sCheckSave()
{
  if(cEdit == _mode)
  {
    switch(QMessageBox::question(this, tr("Save Changes?"),
         tr("Would you like to save any changes before continuing?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
    {
      case QMessageBox::Yes:
        return sSave();
        break;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
      default:
        return false;
    };
  }
  return true;
}

void vendor::clear()
{
  _cachedNumber = QString::null;
  _crmacctid = -1;
  _vendid = -1;

  _active->setChecked(true);
  _poItems->setChecked(false);
  _restrictToItemSource->setChecked(false);
  _qualified->setChecked(false);
  _match->setChecked(false);
  _receives1099->setChecked(false);

  _vendtype->setId(-1);
  _defaultTerms->setId(-1);
  _defaultCurr->setCurrentIndex(0);
  _taxzone->setId(-1);

  _useWarehouseFOB->setChecked(true);

  disconnect(_number, SIGNAL(newId(int)), this, SLOT(setId(int)));
  _number->clear();
  connect(_number, SIGNAL(newId(int)), this, SLOT(setId(int)));

  _name->clear();
  _accountNumber->clear();
  _defaultShipVia->clear();
  _vendorFOB->clear();
  _notes->clear();
  _poComments->clear();

  _address->clear();
  _contact1->clear();
  _contact2->clear();

  _vendaddr->clear();
  _taxreg->clear();

  _achGroup->setChecked(false);
  _routingNumber->clear();
  _achAccountNumber->clear();
  _individualId->clear();
  _individualName->clear();
  _accountType->setCurrentIndex(0);

  _comments->setId(-1);
  _documents->setId(-1);
  _charass->setId(-1);
  _tabs->setCurrentIndex(0);
  
  if (_number->editMode() || _mode == cNew)
    sPrepare();
}

void vendor::sNumberEditable(bool p)
{
  if (p && _number->id() == -1)
    clear();
}

void vendor::sPrepare()
{
  if (_mode == cEdit)
  {
    _mode = cNew;
    emit newMode(_mode);
  }
  
  XSqlQuery idq;
  idq.exec("SELECT NEXTVAL('vend_vend_id_seq') AS vend_id");
  if (idq.first())
  {
    _vendid = idq.value("vend_id").toInt();
    emit newId(_vendid);
    
    _charass->setId(_vendid);
    _documents->setId(_vendid);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting new id"),
                                idq, __FILE__, __LINE__))
    return;
  
  disconnect(_number, SIGNAL(editable(bool)), this, SLOT(sNumberEditable(bool)));
  _number->clear();
  _number->setCanEdit(true);
  _number->setEditMode(true);
  connect(_number, SIGNAL(editable(bool)), this, SLOT(sNumberEditable(bool)));
  
  // Handle Auto numbering
  if(((_x_metrics &&
       _x_metrics->value("CRMAccountNumberGeneration") == "A") ||
      (_x_metrics->value("CRMAccountNumberGeneration") == "O"))
     && _number->number().isEmpty() )
  {
    XSqlQuery num;
    num.exec("SELECT fetchCRMAccountNumber() AS number;");
    if (num.first())
      _number->setNumber(num.value("number").toString());
  }
  
  _NumberGen = _number->number().toInt();
}

void vendor::close()
{
  if (cNew == _mode && _tempMode == cEdit && _crmacctid == -1)
  {
    XSqlQuery delq;
    delq.prepare("DELETE FROM vendinfo WHERE (vend_id=:vend_id);");
    delq.bindValue(":vend_id", _vendid);
    delq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting"),
                           delq, __FILE__, __LINE__))
      return;
    omfgThis->sVendorsUpdated();
    // Clean out CRM Acct but only if not used elsewhere
    delq.prepare("DELETE FROM crmacct WHERE ((crmacct_number=:vend_number) "
                      "                 AND ((crmacct_cust_id IS NULL)  "
                      "                  AND (crmacct_competitor_id IS NULL) "
                      "                  AND (crmacct_partner_id IS NULL)    "
                      "                  AND (crmacct_prospect_id IS NULL)   "
                      "                  AND (crmacct_taxauth_id IS NULL))); ");
    delq.bindValue(":vend_number", _number->number().trimmed().toUpper());
    delq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting"),
                           delq, __FILE__, __LINE__))
      return;
  }
  XWidget::close();
}

void vendor::closeEvent(QCloseEvent *pEvent)
{
  if(cNew == _mode && -1 != _NumberGen)
  {
    XSqlQuery query;
    query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
    query.bindValue(":Number", _NumberGen);
    query.exec();
    _NumberGen = -1;
  }
  XWidget::closeEvent(pEvent);
}

void vendor::sHandleButtons()
{
  if (_taxButton->isChecked())
    _settingsStack->setCurrentWidget(_taxPage);
  else
    _settingsStack->setCurrentWidget(_generalPage);

  if (_mainButton->isChecked())
    _addressStack->setCurrentWidget(_mainPage);
  else
    _addressStack->setCurrentWidget(_alternatesPage);
  
  if (_notesButton->isChecked())
    _remarksStack->setCurrentWidget(_notesPage);
  else
    _remarksStack->setCurrentWidget(_commentsPage);
  
  if (_purchaseOrdersButton->isChecked())
    _ordersStack->setCurrentWidget(_purchaseOrdersPage);
  else if (_receiptsReturnsButton->isChecked())
    _ordersStack->setCurrentWidget(_receiptsReturnsPage);
  else
    _ordersStack->setCurrentWidget(_summaryPage);
  
  if (_payablesButton->isChecked())
    _accountingStack->setCurrentWidget(_payablesPage);
  else if (_creditMemosButton->isChecked())
    _accountingStack->setCurrentWidget(_creditMemosPage);
  else if (_apHistoryButton->isChecked())
    _accountingStack->setCurrentWidget(_apHistoryPage);
  else
    _accountingStack->setCurrentWidget(_apChecksPage);
  
  if (_checksButton->isChecked())
    _transmitStack->setCurrentWidget(_eftPage);
  else
    _transmitStack->setCurrentWidget(_emptyPage);
}

void vendor::sNumberEdited()
{
  _notice = true;
  sCheck();
}

void vendor::sCrmAccount()
{
  ParameterList params;
  params.append("crmacct_id", _crmacctid);
  if ((cView == _mode && _privileges->check("ViewAllCRMAccounts")) ||
      (cView == _mode && _privileges->check("ViewPersonalCRMAccounts")
                      && omfgThis->username() == _crmowner) ||
      (cEdit == _mode && _privileges->check("ViewAllCRMAccounts")
                      && ! _privileges->check("MaintainAllCRMAccounts")) ||
      (cEdit == _mode && _privileges->check("ViewPersonalCRMAccounts")
                      && ! _privileges->check("MaintainPersonalCRMAccounts")
                      && omfgThis->username() == _crmowner))
    params.append("mode", "view");
  else if ((cEdit == _mode && _privileges->check("MaintainAllCRMAccounts")) ||
           (cEdit == _mode && _privileges->check("MaintainPersonalCRMAccounts")
                           && omfgThis->username() == _crmowner))
    params.append("mode", "edit");
  else if ((cNew == _mode && _privileges->check("MaintainAllCRMAccounts")) ||
           (cNew == _mode && _privileges->check("MaintainPersonalCRMAccounts")
                          && omfgThis->username() == _crmowner))
    params.append("mode", "edit");
  else
  {
    qWarning("tried to open Account window without privilege");
    return;
  }

  crmaccount *newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendor::sSaveAndNewComment()
{
  if (_mode == cNew && _crmacctid == -1)
  { 
    if (!sSave())
      return;
    _comments->setId(_crmacctid);
  }
  _comments->sNew();
}
