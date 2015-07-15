/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "contact.h"

#include <QMessageBox>
#include <QVariant>

#include <metasql.h>
#include <mqlutil.h>

#include "contactcluster.h"
#include "crmaccount.h"
#include "customer.h"
#include "employee.h"
#include "errorReporter.h"
#include "incident.h"
#include "inputManager.h"
#include "lotSerialRegistration.h"
#include "opportunity.h"
#include "prospect.h"
#include "purchaseOrder.h"
#include "salesOrder.h"
#include "shipTo.h"
#include "storedProcErrorLookup.h"
#include "todoItem.h"
#include "transferOrder.h"
#include "vendor.h"
#include "vendorAddress.h"
#include "warehouse.h"
#include "xsqlquery.h"
#include <time.h>

struct privSet {
  bool canEdit;
  bool canView;
  bool canDetach;
};

class contactPrivate
{
  public:
    contactPrivate(contact *parent)
      : _parent(parent)
    {
      _activeCache = false;
      _mode        = cView;
    };

    ~contactPrivate()
    {
    };

    bool     _activeCache;
    int      _mode;
    contact *_parent;

    struct privSet rowPrivs(XTreeWidgetItem *row)
    {
      struct privSet privs = {false, false, false};

      if (! row)
        return privs;

      QString owner = row->rawValue("owner").toString();
      switch (row->altId())
      {
        case 1:
        case 2:
          privs.canEdit = _privileges->check("MaintainAllCRMAccounts") ||
                     (omfgThis->username() == owner &&
                      _privileges->check("MaintainPersonalCRMAccounts"));
          privs.canView = _privileges->check("ViewAllCRMAccounts") ||
                     (omfgThis->username() == owner &&
                      _privileges->check("ViewPersonalCRMAccounts"));
          privs.canDetach = privs.canEdit;
          break;

        case 3:
        case 4:
          privs.canEdit = _privileges->check("MaintainCustomerMasters");
          privs.canView = _privileges->check("ViewCustomerMasters");
          privs.canDetach = privs.canEdit;
          break;

        case 5:
        case 6:
          privs.canEdit = _privileges->check("MaintainVendors");
          privs.canView = _privileges->check("ViewVendors");
          privs.canDetach = privs.canEdit;
          break;

        case 7:
          privs.canEdit = _privileges->check("MaintainProspectMasters");
          privs.canView = _privileges->check("ViewProspectMasters");
          privs.canDetach = privs.canEdit;
          break;

        case 8:
          privs.canEdit = _privileges->check("MaintainShiptos");
          privs.canView = _privileges->check("ViewShiptos");
          privs.canDetach = privs.canEdit;
          break;

        case 9:
          privs.canEdit = _privileges->check("MaintainVendorAddresses");
          privs.canView = _privileges->check("ViewVendorAddresses");
          privs.canDetach = privs.canEdit;
          break;

        case 10:
          privs.canEdit = _privileges->check("MaintainWarehouses");
          privs.canView = _privileges->check("ViewWarehouses");
          privs.canDetach = privs.canEdit;
          break;

        case 11:
          privs.canEdit = _privileges->check("MaintainEmployees");
          privs.canView = _privileges->check("ViewEmployees");
          privs.canDetach = privs.canEdit;
          break;

        case 12:
        case 13:
          privs.canEdit = _privileges->check("MaintainSalesOrders");
          privs.canView = _privileges->check("ViewSalesOrders");
          privs.canDetach = privs.canEdit;
          break;

        case 14:
          privs.canEdit = _privileges->check("MaintainAllIncidents") ||
                     (omfgThis->username() == owner &&
                      _privileges->check("MaintainPersonalIncidents"));
          privs.canView = _privileges->check("ViewAllIncidents") ||
                     (omfgThis->username() == owner &&
                      _privileges->check("ViewPersonalIncidents"));
          privs.canDetach = false;     // cntct is required
          break;

        case 15:
          privs.canEdit = _metrics->boolean("LotSerialControl");
          privs.canView = _metrics->boolean("LotSerialControl");
          privs.canDetach = false;     // cntct is required
          break;

        case 16:
          privs.canEdit = _privileges->check("MaintainAllOpportunities") ||
                     (omfgThis->username() == owner &&
                      _privileges->check("MaintainPersonalOpportunities"));
          privs.canView = _privileges->check("ViewAllOpportunities") ||
                     (omfgThis->username() == owner &&
                      _privileges->check("ViewPersonalOpportunities"));
          privs.canDetach = privs.canEdit;
          break;

        case 17:
        case 18:
          privs.canEdit = _privileges->check("MaintainPurchaseOrders");
          privs.canView = _privileges->check("ViewPurchaseOrders");
          privs.canDetach = privs.canEdit;
          break;

        case 19:
        case 20:
          privs.canEdit = _privileges->check("MaintainQuotes");
          privs.canView = _privileges->check("ViewQuotes");
          privs.canDetach = privs.canEdit;
          break;

        case 21:
          privs.canEdit = _privileges->check("MaintainAllToDoItems") ||
                     (omfgThis->username() == owner &&
                      _privileges->check("MaintainPersonalToDoItems"));
          privs.canView = _privileges->check("ViewAllToDoItems") ||
                     (omfgThis->username() == owner &&
                      _privileges->check("ViewPersonalToDoItems"));
          privs.canDetach = privs.canEdit;
          break;

        case 22:
        case 23:
          privs.canEdit = _privileges->check("MaintainTransferOrders");
          privs.canView = _privileges->check("ViewTransferOrders");
          privs.canDetach = privs.canEdit;
          break;

        default:
          break;
      }

      return privs;
    };
};

contact::contact(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  _data = new contactPrivate(this);

  // Legacy compatibility removed
  // For legacy compatibility
  //_save = _buttonBox->button(QDialogButtonBox::Save);
  //_save->setObjectName("_save");

  connect(_buttonBox,            SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox,            SIGNAL(rejected()), this, SLOT(sClose()));
  connect(_crmAccount,           SIGNAL(newId(int)), _contact, SLOT(setSearchAcct(int)));
  connect(_detachUse,            SIGNAL(clicked()), this, SLOT(sDetachUse()));
  connect(_editUse,              SIGNAL(clicked()), this, SLOT(sEditUse()));
  connect(_showOrders,       SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_uses,               SIGNAL(valid(bool)), this, SLOT(sHandleValidUse(bool)));
  connect(_uses, SIGNAL(populateMenu(QMenu*, XTreeWidgetItem*)), this, SLOT(sPopulateUsesMenu(QMenu*)));
  connect(_viewUse,                       SIGNAL(clicked()), this, SLOT(sViewUse()));
  connect(omfgThis,         SIGNAL(crmAccountsUpdated(int)), this, SLOT(sFillList()));
  connect(omfgThis,      SIGNAL(customersUpdated(int,bool)), this, SLOT(sFillList()));
  connect(omfgThis,            SIGNAL(employeeUpdated(int)), this, SLOT(sFillList()));
  connect(omfgThis,              SIGNAL(prospectsUpdated()), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(purchaseOrdersUpdated(int,bool)), this, SLOT(sFillList()));
  connect(omfgThis,         SIGNAL(quotesUpdated(int,bool)), this, SLOT(sFillList()));
  connect(omfgThis,    SIGNAL(salesOrdersUpdated(int,bool)), this, SLOT(sFillList()));
  connect(omfgThis,      SIGNAL(transferOrdersUpdated(int)), this, SLOT(sFillList()));
  connect(omfgThis,                SIGNAL(vendorsUpdated()), this, SLOT(sFillList()));
  connect(omfgThis,             SIGNAL(warehousesUpdated()), this, SLOT(sFillList()));

  _charass->setType("CNTCT");

  _uses->addColumn(tr("Used by"),         100, Qt::AlignLeft, true, "type");
  _uses->addColumn(tr("Number"), _orderColumn, Qt::AlignLeft, true, "number");
  _uses->addColumn(tr("Name"),             -1, Qt::AlignLeft, true, "name");
  _uses->addColumn(tr("Role"),             -1, Qt::AlignLeft, true, "role");
  _uses->addColumn(tr("Active"),    _ynColumn, Qt::AlignCenter,true, "active");
  _uses->addColumn(tr("Owner"),   _userColumn, Qt::AlignLeft,  false,"owner");

  _contact->setMinimalLayout(false);
  _contact->setAccountVisible(false);
  _contact->setInitialsVisible(false);
  _contact->setActiveVisible(false);
  _contact->setOwnerVisible(false);
  _contact->setListVisible(false);

  _owner->setUsername(omfgThis->username());
  _owner->setType(UsernameLineEdit::UsersActive);
  _owner->setEnabled(_privileges->check("EditOwner"));

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
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cntct_id", &valid);
  if (valid)
  {
    _contact->setId(param.toInt());
    _comments->setId(_contact->id());
    _documents->setId(_contact->id());
    _charass->setId(_contact->id());
    sPopulate();
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
    _crmAccount->setId(param.toInt());

  param = pParams.value("addr_line1", &valid);
  if (valid)
    _contact->addressWidget()->setLine1(param.toString());

  param = pParams.value("addr_line2", &valid);
  if (valid)
    _contact->addressWidget()->setLine2(param.toString());

  param = pParams.value("addr_line3", &valid);
  if (valid)
    _contact->addressWidget()->setLine3(param.toString());

  param = pParams.value("addr_city", &valid);
  if (valid)
    _contact->addressWidget()->setCity(param.toString());

  param = pParams.value("addr_state", &valid);
  if (valid)
    _contact->addressWidget()->setState(param.toString());

  param = pParams.value("addr_postalcode", &valid);
  if (valid)
    _contact->addressWidget()->setPostalCode(param.toString());

  param = pParams.value("addr_country", &valid);
  if (valid)
    _contact->addressWidget()->setCountry(param.toString());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _data->_mode = cNew;
      XSqlQuery getq;
      getq.exec("SELECT fetchNextNumber('ContactNumber') AS result;");
      getq.first();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Number"),
                               getq, __FILE__, __LINE__))
        return UndefinedError;
      _number->setText(getq.value("result").toString());
      _contact->setNumber(_number->text());
      _contact->setFirst("Contact" + QDateTime::currentDateTime().toString());
      int cntctSaveResult = _contact->save(AddressCluster::CHANGEONE);
      if (cntctSaveResult < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Saving Placeholder"),
                             tr("<p>There was an error creating a new contact (%). "
                                "Check the database server log for errors.")
                             .arg(cntctSaveResult), __FILE__, __LINE__);
        return UndefinedError;
      }
      _comments->setId(_contact->id());
      _documents->setId(_contact->id());
      _charass->setId(_contact->id());
      _contact->setFirst("");
      _contact->setOwnerUsername(omfgThis->username());
    }
    else if (param.toString() == "edit")
    {
      _data->_mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _data->_mode = cView;

      _buttonBox->setStandardButtons(QDialogButtonBox::Close);

      _contact->setEnabled(false);
      _notes->setEnabled(false);
      _comments->setReadOnly(true);
      _documents->setReadOnly(true);
      _charass->setReadOnly(true);
    }
  }

  return NoError;
}

void contact::sPopulateUsesMenu(QMenu* pMenu)
{
  QAction *editAction = 0;
  QAction *viewAction = 0;
  QString editStr = tr("Edit...");
  QString viewStr = tr("View...");

  switch (_uses->altId())
  {
    case 1:
    case 2:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditCRMAccount()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewCRMAccount()));
      break;

    case 3:
    case 4:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditCustomer()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewCustomer()));
      break;

    case 5:
    case 6:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditVendor()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewVendor()));
      break;

    case 7:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditProspect()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewProspect()));
      break;

    case 8:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditShipto()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewShipto()));
      break;

    case 9:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditVendorAddress()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewVendorAddress()));
      break;

    case 10:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditWarehouse()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewWarehouse()));
      break;

    case 11:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditEmployee()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewEmployee()));
      break;

    case 12:
    case 13:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditSalesOrder()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewSalesOrder()));
      break;

    case 14:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditIncident()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewIncident()));
      break;

    case 15:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditLSReg()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewLSReg()));
      break;

    case 16:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditOpportunity()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewOpportunity()));
      break;

    case 17:
    case 18:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditPurchaseOrder()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewPurchaseOrder()));
      break;

    case 19:
    case 20:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditQuote()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewQuote()));
      break;

    case 21:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditTodoItem()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewTodoItem()));
      break;

    case 22:
    case 23:
      editAction = pMenu->addAction(editStr, this, SLOT(sEditTransferOrder()));
      viewAction = pMenu->addAction(viewStr, this, SLOT(sViewTransferOrder()));
                                    break;

    default:
      break;
  }

  struct privSet privs = _data->rowPrivs(_uses->currentItem());
  editAction->setEnabled(privs.canEdit && (cView != _data->_mode));
  viewAction->setEnabled(privs.canView);
  QAction *detachAction = pMenu->addAction(tr("Detach"), this, SLOT(sDetachUse()));
  detachAction->setEnabled(privs.canDetach && (cView != _data->_mode));
}

void contact::sClose()
{
  if (_data->_mode == cNew)
  {
    XSqlQuery delq;
    delq.prepare("DELETE FROM cntct WHERE (cntct_id=:cntct_id);"
                 "SELECT releaseNumber('ContactNumber',:number);");
    delq.bindValue(":cntct_id", _contact->id());
    delq.bindValue(":number", _contact->number().toInt());
    delq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Cleaning up Contact"),
                             delq, __FILE__, __LINE__))
      return;
  }

  reject();
}

void contact::sSave()
{
  if (_contact->first().isEmpty() && _contact->last().isEmpty())
  {
    QMessageBox::information(this, tr("Contact Blank"),
                             tr("<p>You must fill in a contact first or last name as a minimum before saving."));
    return;
  }

  if (_data->_activeCache && ! _contact->active())
  {
    QString errmsg;
    bool    ok = false;
    MetaSQLQuery getm = MQLUtil::mqlLoad("contact", "muststayactive",
                                         errmsg, &ok);
    if (! ok)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("In Use"),
                           errmsg, __FILE__, __LINE__);
      return;
    }
    ParameterList getp;
    getp.append("id", _contact->id());
    XSqlQuery getq = getm.toQuery(getp);
    getq.exec();
    if (getq.first() && getq.value("inuse").toBool())
    {
      QMessageBox::information(this, tr("Cannot make Contact inactive"),
                            tr("<p>You may not mark this Contact as not "
                               "Active when this person is a Contact "
                               "for an active Account, Customer, "
                               "Vendor, or Prospect."));
      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("In Use"),
                                  getq, __FILE__, __LINE__))
      return;
  }

  /* save the address first so we can check for multiple uses, then save the
     contact with less error checking because this is contact maintenance
   */
  AddressCluster* addr = _contact->addressWidget();
  int saveResult = addr->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = 2;     // Cancel
    answer = QMessageBox::question(this, tr("Question Saving Address"),
                    tr("<p>There are multiple Contacts sharing this Address. "
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
  if (saveResult < 0)   // check from errors for CHECK and CHANGE* saves
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Saving Address"),
                         tr("<p>There was an error saving this Address (%1). "
                            "Check the database server log for errors.")
                         .arg(saveResult), __FILE__, __LINE__);
    return;
  }

  _contact->setAddress(saveResult);
  _contact->setNotes(_notes->toPlainText());
  _contact->setOwnerUsername(_owner->username());
  _contact->setCrmAcctId(_crmAccount->id());
  _contact->setActive(_active->isChecked());

  saveResult = _contact->save(AddressCluster::CHANGEALL);
  if (saveResult < 0)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Saving Contact"),
                         tr("<p>There was an error saving this Contact (%1). "
                            "Check the database server log for errors.")
                         .arg(saveResult), __FILE__, __LINE__);
    return;
  }

  done(_contact->id());
}

void contact::sPopulate()
{
  _number->setText(_contact->number());
  _active->setChecked(_contact->active());
  _crmAccount->setId(_contact->crmAcctId());
  _owner->setUsername(_contact->owner());
  _notes->setText(_contact->notes());
  _data->_activeCache = _contact->active();
  sFillList();
}

void contact::sFillList()
{
  _charass->setId(_contact->id());

  QString errmsg;
  bool    ok = false;
  MetaSQLQuery getm = MQLUtil::mqlLoad("contact", "uses", errmsg, &ok);
  if (! ok)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Getting Contact Uses"),
                         errmsg, __FILE__, __LINE__);
    return;
  }
  ParameterList getp;
  getp.append("core");
  getp.append("id",         _contact->id());

  getp.append("billing",    tr("Billing Contact"));
  getp.append("cohead",     tr("Sales Order"));
  getp.append("correspond", tr("Correspondence Contact"));
  getp.append("crmacct",    tr("Account"));
  getp.append("cust",       tr("Customer"));
  getp.append("emp",        tr("Employee"));
  getp.append("ophead",     tr("Opportunity"));
  getp.append("pohead",     tr("Purchase Order"));
  getp.append("primary",    tr("Primary Contact"));
  getp.append("prospect",   tr("Prospect"));
  getp.append("quhead",     tr("Quote"));
  getp.append("secondary",  tr("Secondary Contact"));
  getp.append("shipto",     tr("Ship-To Address"));
  getp.append("todo",       tr("To-Do Item"));
  getp.append("vend",       tr("Vendor"));
  getp.append("vendaddr",   tr("Vendor Address"));
  getp.append("vendcntct",  tr("Vendor Contact"));
  getp.append("whs",        tr("Site"));
  getp.append("incdt",      tr("Incident"));

  if (_metrics->boolean("LotSerialControl"))
    getp.append("lsreg",    tr("Lot/Serial Registration"));
  if (_metrics->boolean("MultiWhs"))
  {
    getp.append("tohead",   tr("Transfer Order"));
    getp.append("from",     tr("Source Contact"));
    getp.append("dest",     tr("Destination Contact"));
  }
  if (_showOrders->isChecked())
    getp.append("showOrders");

  XSqlQuery getq = getm.toQuery(getp);
  _uses->populate(getq, true);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Contact Uses"),
                           getq, __FILE__, __LINE__))
    return;
}

void contact::sDetachUse()
{
  QString question;
  XSqlQuery detachq;
  switch (_uses->altId())
  {
    case 1:
      question = tr("Are you sure that you want to remove this Contact as "
                    "the Primary Contact for this Account?");
      detachq.prepare("UPDATE crmacct SET crmacct_cntct_id_1 = NULL "
                      "WHERE (crmacct_id=:id);");
      break;
    case 2:
      question = tr("Are you sure that you want to remove this Contact as "
                    "the Secondary Contact for this Account?");
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
                    "the Contact for this Site?");
      detachq.prepare("UPDATE whsinfo SET warehous_cntct_id = NULL "
                      "WHERE (warehous_id=:id);");
      break;

    case 11:
      question = tr("Are you sure that you want to remove this Contact as "
                    "the Contact for this Employee?");
      detachq.prepare("UPDATE emp SET emp_cntct_id = NULL "
                      "WHERE (emp_id=:id);");
      break;

    case 12:
      question = tr("Are you sure that you want to remove the link between "
                    "this Contact and this Sales Order? The name and address "
                    "will still be kept in the order for future reference.");
      detachq.prepare("UPDATE cohead SET cohead_billto_cntct_id = NULL"
                      " WHERE (cohead_id=:id);");
      break;

    case 13:
      question = tr("Are you sure that you want to remove the link between "
                    "this Contact and this Sales Order? The name and address "
                    "will still be kept in the order for future reference.");
        detachq.prepare("UPDATE cohead SET cohead_shipto_cntct_id = NULL"
                        " WHERE (cohead_id=:id);");
      break;

    case 14:
      question = tr("Are you sure you want to remove this Contact as "
                    "the Contact for this Incident?");
      detachq.prepare("UPDATE incdt SET incdt_cntct_id = NULL"
                      " WHERE (incdt_id=:id);");
      break;

    case 15:
      question = tr("Are you sure you want to remove this Contact as "
                    "the Contact for this Lot/Serial Registration?");
      detachq.prepare("UPDATE lsreg SET lsreg_cntct_id = NULL"
                      " WHERE lsreg_id=:id;");
      break;

    case 16:
      question = tr("Are you sure you want to remove this Contact as "
                    "the Contact for this Opportunity?");
      detachq.prepare("UPDATE ophead SET ophead_cntct_id = NULL"
                      " WHERE ophead_id=:id;");
      break;

    case 17:
      question = tr("Are you sure you want to remove the link between "
                    "this Contact and this Purchase Order? The name and address "
                    "will still be kept in the order for future reference.");
      detachq.prepare("UPDATE pohead SET pohead_vend_cntct_id = NULL"
                      " WHERE pohead_id=:id;");
      break;

    case 18:
      question = tr("Are you sure you want to remove the link between "
                    "this Contact and this Purchase Order? The name and address "
                    "will still be kept in the order for future reference.");
      detachq.prepare("UPDATE pohead SET pohead_shipto_cntct_id = NULL"
                      " WHERE pohead_id=:id;");
      break;

    case 19:
      question = tr("Are you sure you want to remove the link between "
                    "this Contact and this Quote? The name and address "
                    "will still be kept in the quote for future reference.");
      detachq.prepare("UPDATE quhead SET quhead_billto_cntct_id = NULL"
                      " WHERE quhead_id=:id;");
      break;

    case 20:
      question = tr("Are you sure you want to remove the link between "
                    "this Contact and this Quote? The name and address "
                    "will still be kept in the quote for future reference.");
      detachq.prepare("UPDATE quhead SET quhead_shipto_cntct_id = NULL"
                      " WHERE quhead_id=:id;");
      break;

    case 21:
      question = tr("Are you sure you want to remove this Contact as "
                    "the Contact for this To-Do Item?");
      detachq.prepare("UPDATE todoitem SET todoitem_cntct_id = NULL"
                      " WHERE todoitem_id=:id;");
      break;

    case 22:
      question = tr("Are you sure you want to remove the link between "
                    "this Contact and this Transfer Order? The name and address "
                    "will still be kept in the order for future reference.");
      detachq.prepare("UPDATE tohead SET tohead_srccntct_id = NULL"
                      " WHERE tohead_id=:id;");
      break;

    case 23:
      question = tr("Are you sure you want to remove the link between "
                    "this Contact and this Transfer Order? The name and address "
                    "will still be kept in the order for future reference.");
      detachq.prepare("UPDATE tohead SET tohead_destcntct_id = NULL"
                      " WHERE tohead_id=:id;");
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
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Detaching"),
                           detachq, __FILE__, __LINE__))
    return;

  sFillList();
}

void contact::sEditUse()
{
  switch (_uses->altId())
  {
    case  1:
    case  2: sEditCRMAccount();		break;
    case  3:
    case  4: sEditCustomer();		break;
    case  5:
    case  6: sEditVendor();		break;
    case  7: sEditProspect();		break;
    case  8: sEditShipto();		break;
    case  9: sEditVendorAddress();	break;
    case 10: sEditWarehouse();		break;
    case 11: sEditEmployee();		break;
    case 12:
    case 13: sEditSalesOrder();		break;
    case 14: sEditIncident();		break;
    case 15: sEditLSRegistration();	break;
    case 16: sEditOpportunity();	break;
    case 17:
    case 18: sEditPurchaseOrder();	break;
    case 19:
    case 20: sEditQuote();		break;
    case 21: sEditTodoItem();		break;
    case 22:
    case 23: sEditTransferOrder();	break;
    default: break;
  }
}

void contact::sViewUse()
{
  switch (_uses->altId())
  {
    case  1:
    case  2: sViewCRMAccount();		break;
    case  3:
    case  4: sViewCustomer();		break;
    case  5:
    case  6: sViewVendor();		break;
    case  7: sViewProspect();		break;
    case  8: sViewShipto();		break;
    case  9: sViewVendorAddress();	break;
    case 10: sViewWarehouse();		break;
    case 11: sViewEmployee();		break;
    case 12:
    case 13: sViewSalesOrder();		break;
    case 14: sViewIncident();		break;
    case 15: sViewLSRegistration();	break;
    case 16: sViewOpportunity();	break;
    case 17:
    case 18: sViewPurchaseOrder();	break;
    case 19:
    case 20: sViewQuote();	        break;
    case 21: sViewTodoItem();	        break;
    case 22:
    case 23: sViewTransferOrder();	break;
    default: break;
  }
}

void contact::sHandleValidUse(bool valid)
{
  disconnect(_uses, SIGNAL(itemSelected(int)), _editUse, SLOT(animateClick()));
  disconnect(_uses, SIGNAL(itemSelected(int)), _viewUse, SLOT(animateClick()));

  struct privSet privs = _data->rowPrivs(_uses->currentItem());

  _detachUse->setEnabled(valid && privs.canDetach && _data->_mode != cView);
  _editUse->setEnabled(valid   && privs.canEdit   && _data->_mode != cView);
  _viewUse->setEnabled(valid   && privs.canView);

  if (_editUse->isEnabled())
    connect(_uses, SIGNAL(itemSelected(int)), _editUse, SLOT(animateClick()));
  else if (_viewUse->isEnabled())
    connect(_uses, SIGNAL(itemSelected(int)), _viewUse, SLOT(animateClick()));
}

void contact::sEditCRMAccount()
{
  ParameterList params;
  params.append("mode",       "edit");
  params.append("crmacct_id", _uses->id());
  params.append("modal");
  crmaccount *newdlg = new crmaccount(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void contact::sViewCRMAccount()
{
  ParameterList params;
  params.append("mode",       "view");
  params.append("crmacct_id", _uses->id());
  params.append("modal");
  crmaccount *newdlg = new crmaccount(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void contact::sEditCustomer()
{
  ParameterList params;
  params.append("mode",    "edit");
  params.append("cust_id", _uses->id());
  customer *newdlg = new customer(0, "custForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewCustomer()
{
  ParameterList params;
  params.append("mode",    "view");
  params.append("cust_id", _uses->id());
  customer *newdlg = new customer(0, "custForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditEmployee()
{
  ParameterList params;
  params.append("mode",   "edit");
  params.append("emp_id", _uses->id());
  employee newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewEmployee()
{
  ParameterList params;
  params.append("mode",   "view");
  params.append("emp_id", _uses->id());
  employee newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditIncident()
{
  ParameterList params;
  params.append("mode",     "edit");
  params.append("incdt_id", _uses->id());
  incident newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewIncident()
{
  ParameterList params;
  params.append("mode",     "view");
  params.append("incdt_id", _uses->id());
  incident newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditLSRegistration()
{
  ParameterList params;
  params.append("mode",     "edit");
  params.append("lsreg_id", _uses->id());
  lotSerialRegistration newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewLSRegistration()
{
  ParameterList params;
  params.append("mode",     "view");
  params.append("lsreg_id", _uses->id());
  lotSerialRegistration newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditOpportunity()
{
  ParameterList params;
  params.append("mode",      "edit");
  params.append("ophead_id", _uses->id());
  opportunity newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewOpportunity()
{
  ParameterList params;
  params.append("mode",      "view");
  params.append("ophead_id", _uses->id());
  opportunity newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditProspect()
{
  ParameterList params;
  params.append("mode",        "edit");
  params.append("prospect_id", _uses->id());
  prospect *newdlg = new prospect(0, "prospectForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewProspect()
{
  ParameterList params;
  params.append("mode",        "view");
  params.append("prospect_id", _uses->id());
  prospect *newdlg = new prospect(0, "prospectForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditPurchaseOrder()
{
  ParameterList params;
  params.append("mode",      "edit");
  params.append("pohead_id", _uses->id());
  purchaseOrder *newdlg = new purchaseOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewPurchaseOrder()
{
  ParameterList params;
  params.append("mode",      "view");
  params.append("pohead_id", _uses->id());
  purchaseOrder *newdlg = new purchaseOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditQuote()
{
  ParameterList params;
  params.append("mode",      "edit");
  params.append("quhead_id", _uses->id());
  salesOrder *newdlg = new salesOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewQuote()
{
  ParameterList params;
  params.append("mode",      "view");
  params.append("quhead_id", _uses->id());
  salesOrder *newdlg = new salesOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditSalesOrder()
{
  ParameterList params;
  params.append("mode",      "edit");
  params.append("sohead_id", _uses->id());
  salesOrder *newdlg = new salesOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewSalesOrder()
{
  ParameterList params;
  params.append("mode",      "view");
  params.append("sohead_id", _uses->id());
  salesOrder *newdlg = new salesOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditShipto()
{
  ParameterList params;
  params.append("mode",      "edit");
  params.append("shipto_id", _uses->id());
  shipTo newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewShipto()
{
  ParameterList params;
  params.append("mode",      "view");
  params.append("shipto_id", _uses->id());
  shipTo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditTodoItem()
{
  ParameterList params;
  params.append("mode",        "edit");
  params.append("todoitem_id", _uses->id());
  todoItem newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewTodoItem()
{
  ParameterList params;
  params.append("mode",        "view");
  params.append("todoitem_id", _uses->id());
  todoItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditTransferOrder()
{
  ParameterList params;
  params.append("mode",      "edit");
  params.append("tohead_id", _uses->id());
  transferOrder *newdlg = new transferOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewTransferOrder()
{
  ParameterList params;
  params.append("mode",      "view");
  params.append("tohead_id", _uses->id());
  transferOrder *newdlg = new transferOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditVendorAddress()
{
  ParameterList params;
  params.append("mode",        "edit");
  params.append("vendaddr_id", _uses->id());
  vendorAddress newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewVendorAddress()
{
  ParameterList params;
  params.append("mode",        "view");
  params.append("vendaddr_id", _uses->id());
  vendorAddress newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditVendor()
{
  ParameterList params;
  params.append("mode",    "edit");
  params.append("vend_id", _uses->id());
  vendor *newdlg = new vendor(0, "vendorForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewVendor()
{
  ParameterList params;
  params.append("mode",    "view");
  params.append("vend_id", _uses->id());
  vendor *newdlg = new vendor(0, "vendorForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditWarehouse()
{
  ParameterList params;
  params.append("mode",        "edit");
  params.append("warehous_id", _uses->id());
  warehouse newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewWarehouse()
{
  ParameterList params;
  params.append("mode",        "view");
  params.append("warehous_id", _uses->id());
  warehouse newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}
