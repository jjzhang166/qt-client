/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "address.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

#include <metasql.h>
#include <parameter.h>

#include "addresscluster.h"
#include "contact.h"
#include "mqlutil.h"
#include "shipTo.h"
#include "vendor.h"
#include "vendorAddress.h"
#include "warehouse.h"
#include "errorReporter.h"

address::address(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_editAddrUse, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_viewAddrUse, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
    connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(_uses, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));

    _uses->addColumn(tr("Used by"),	 50, Qt::AlignLeft, true, "type");
    _uses->addColumn(tr("First Name\nor Number"),50, Qt::AlignLeft, true, "cntct_first_name");
    _uses->addColumn(tr("Last Name\nor Name"),	 -1, Qt::AlignLeft, true, "cntct_last_name");
    _uses->addColumn(tr("Account"),	 80, Qt::AlignLeft, true, "crmacct_number");
    _uses->addColumn(tr("Phone"),	100, Qt::AlignLeft, true, "cntct_phone");
    _uses->addColumn(tr("Alternate"),	100, Qt::AlignLeft, true, "cntct_phone2");
    _uses->addColumn(tr("Fax"),		100, Qt::AlignLeft, true, "cntct_fax");
    _uses->addColumn(tr("E-Mail"),	100, Qt::AlignLeft, true, "cntct_email");
    _uses->addColumn(tr("Web Address"),	100, Qt::AlignLeft, true, "cntct_webaddr");

    _charass->setType("ADDR");
}

address::~address()
{
  // no need to delete child widgets, Qt does it all for us
}

void address::languageChange()
{
  retranslateUi(this);
}

enum SetResponse address::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("addr_id", &valid);
  if (valid)
  {
    _captive = true;
    _addr->setId(param.toInt());
    sPopulate();
    _charass->setId(_addr->id());
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _addr->setLine1("Address" + QDateTime::currentDateTime().toString());
      int addrSaveResult = _addr->save(AddressCluster::CHANGEONE);
      if (addrSaveResult < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Address"),
                  tr("There was an error creating a new address (%1).\n"
                     "Check the database server log for errors.") .arg(addrSaveResult), __FILE__, __LINE__);
        return UndefinedError;
      }
      _comments->setId(_addr->id());
      _charass->setId(_addr->id());
      _addr->setLine1("");
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _editAddrUse->hide();
      disconnect(_uses, SIGNAL(itemSelected(int)), _editAddrUse, SLOT(animateClick()));
      connect(_uses, SIGNAL(itemSelected(int)), _viewAddrUse, SLOT(animateClick()));

      _addr->setEnabled(false);
      _notes->setEnabled(false);
      _comments->setReadOnly(true);
      _editAddrUse->setEnabled(false);
      _charass->setReadOnly(true);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void address::sSave()
{
   internalSave();
   done(_addr->id());
}

void address::internalSave(AddressCluster::SaveFlags flag)
{
  _addr->setNotes(_notes->toPlainText());

  int saveResult = _addr->save(flag);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Saving Shared Address"),
		    tr("There are multiple Contacts sharing this Address.\n"
		       "If you save this Address, the Address for all "
		       "of these Contacts will be changed. Would you like to "
		       "save this Address?"),
		    QMessageBox::No | QMessageBox::Default, QMessageBox::Yes);
    if (QMessageBox::No == answer)
      return;
    saveResult = _addr->save(AddressCluster::CHANGEALL);
  }
  if (0 > saveResult)	// NOT else if
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Address"),
              tr("There was an error saving this address (%1).\n"
                 "Check the database server log for errors.") .arg(saveResult), __FILE__, __LINE__);
  }
}

void address::reject()
{
  XSqlQuery rejectAddress;
  if (cNew == _mode)
  {
    rejectAddress.prepare("SELECT deleteAddress(:addr_id) AS result;"
              "SELECT releaseNumber('AddressNumber', :number); ");
    rejectAddress.bindValue(":addr_id", _addr->id());
    rejectAddress.bindValue(":number", _addr->number().toInt());
    rejectAddress.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Rejecting Address"),
                                  rejectAddress, __FILE__, __LINE__))
    {
        return;
    }
  }

  XDialog::reject();
}

void address::sPopulate()
{
  _notes->setText(_addr->notes());
  _comments->setId(_addr->id());
  _charass->setId(_addr->id());

  MetaSQLQuery mql = mqlLoad("address", "uses");
  
  ParameterList params;
  params.append("addr_id",  _addr->id());
  params.append("contact",  tr("Contact"));
  params.append("shipto",   tr("Ship-To"));
  params.append("vendor",   tr("Vendor"));
  params.append("vendaddr", tr("Vendor Address"));
  params.append("whs",      tr("Site"));
  XSqlQuery usesQ = mql.toQuery(params);
  _uses->populate(usesQ, true);	// true => use alt id (to distinguish types)
}

void address::sPopulateMenu(QMenu *pMenu)
{
  QString editStr = tr("Edit...");
  QString viewStr = tr("View...");

  switch (_uses->altId())
  {
    case 1:
      if (_privileges->check("MaintainAllContacts") &&
	  (cNew == _mode || cEdit == _mode))
    (void)pMenu->addAction(editStr, this, SLOT(sEditContact()));
      else if (_privileges->check("ViewAllContacts"))
    (void)pMenu->addAction(viewStr, this, SLOT(sViewContact()));

      break;

    case 2:	// ship-to
      if (_privileges->check("MaintainShiptos") &&
	  (cNew == _mode || cEdit == _mode))
    (void)pMenu->addAction(editStr, this, SLOT(sEditShipto()));
      else if (_privileges->check("ViewShiptos"))
    (void)pMenu->addAction(viewStr, this, SLOT(sViewShipto()));

      break;

    case 3:	// vendor
      /* comment out until we make vendor a XDialog or address a XMainWindow
      if (_privileges->check("MaintainVendors") &&
	  (cNew == _mode || cEdit == _mode))
	menuItem = pMenu->addAction(editStr, this, SLOT(sEditVendor()));
      else if (_privileges->check("ViewVendors"))
	menuItem = pMenu->addAction(viewStr, this, SLOT(sViewVendor()));
      */

      break;

    case 4:	// vendaddr
      if (_privileges->check("MaintainVendorAddresses") &&
	  (cNew == _mode || cEdit == _mode))
    (void)pMenu->addAction(editStr, this, SLOT(sEditVendorAddress()));
      else if (_privileges->check("ViewVendorAddresses"))
    (void)pMenu->addAction(viewStr, this, SLOT(sViewVendorAddress()));

      break;

    case 5:	// warehouse
      if (_privileges->check("MaintainWarehouses") &&
	  (cNew == _mode || cEdit == _mode))
    (void)pMenu->addAction(editStr, this, SLOT(sEditWarehouse()));
      else if (_privileges->check("ViewWarehouses"))
    (void)pMenu->addAction(viewStr, this, SLOT(sViewWarehouse()));

      break;

    default:
      break;
  }
}

void address::sEdit()
{
  internalSave();
  switch (_uses->altId())
  {
    case 1:
      sEditContact();
      break;

    case 2:
      sEditShipto();
      break;

    case 3:
      sEditVendor();
      break;

    case 4:
      sEditVendorAddress();
      break;

    case 5:
      sEditWarehouse();
      break;

    default:
      break;
  }

  // force AddressCluster to reload its data
  int tmpAddrId = _addr->id();
  _addr->setId(-1);
  _addr->setId(tmpAddrId);
  sPopulate();
}

void address::sView()
{
  switch (_uses->altId())
  {
    case 1:
      sViewContact();
      break;

    case 2:
      sViewShipto();
      break;

    case 3:
      sViewVendor();
      break;

    case 4:
      sViewVendorAddress();
      break;

    case 5:
      sViewWarehouse();
      break;

    default:
      break;
  }
}

void address::sEditContact()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cntct_id", _uses->id());
  contact newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void address::sViewContact()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cntct_id", _uses->id());
  contact newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void address::sEditShipto()
{
  ParameterList params;
  shipTo newdlg(this, "", true);
  params.append("mode", "edit");
  params.append("shipto_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}

void address::sViewShipto()
{
  ParameterList params;
  shipTo newdlg(this, "", true);
  params.append("mode", "view");
  params.append("shipto_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}

void address::sEditVendor()
{
  ParameterList params;
  vendor * newdlg = new vendor(this);
  params.append("mode", "edit");
  params.append("vend_id", _uses->id());
  newdlg->set(params);
  newdlg->show();
}

void address::sViewVendor()
{
  ParameterList params;
  vendor * newdlg = new vendor(this);
  params.append("mode", "view");
  params.append("vend_id", _uses->id());
  newdlg->set(params);
  newdlg->show();
}

void address::sEditVendorAddress()
{
  ParameterList params;
  vendorAddress newdlg(this, "", true);
  params.append("mode", "edit");
  params.append("vendaddr_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}

void address::sViewVendorAddress()
{
  ParameterList params;
  vendorAddress newdlg(this, "", true);
  params.append("mode", "view");
  params.append("vendaddr_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}

void address::sEditWarehouse()
{
  ParameterList params;
  warehouse newdlg(this, "", true);
  params.append("mode", "edit");
  params.append("warehous_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}

void address::sViewWarehouse()
{
  ParameterList params;
  warehouse newdlg(this, "", true);
  params.append("mode", "view");
  params.append("warehous_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}
