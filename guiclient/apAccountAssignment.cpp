/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "apAccountAssignment.h"

#include <QVariant>

apAccountAssignment::apAccountAssignment(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_selectedVendorType, SIGNAL(toggled(bool)), _vendorTypes, SLOT(setEnabled(bool)));
  connect(_vendorTypePattern, SIGNAL(toggled(bool)), _vendorType, SLOT(setEnabled(bool)));

  _vendorTypes->setType(XComboBox::VendorTypes);

  _ap->setType(GLCluster::cLiability);
  _prepaid->setType(GLCluster::cAsset);
  _discount->setType(GLCluster::cRevenue | GLCluster::cExpense);
}

apAccountAssignment::~apAccountAssignment()
{
  // no need to delete child widgets, Qt does it all for us
}

void apAccountAssignment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse apAccountAssignment::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("apaccnt_id", &valid);
  if (valid)
  {
    _apaccntid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _vendorTypeGroup->setEnabled(false);
      _ap->setReadOnly(true);
      _prepaid->setReadOnly(true);
      _discount->setReadOnly(true);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void apAccountAssignment::sSave()
{
  QList<GuiErrorCheck> errors;

  if (_metrics->boolean("InterfaceAPToGL"))
  {
    errors << GuiErrorCheck(!_ap->isValid(), _ap,
                            tr("<p>You must select an A/P Account before saving this A/P Account Assignment."))
           << GuiErrorCheck(!_prepaid->isValid(), _prepaid,
                            tr("<p>You must select a Prepaid Account before saving this A/P Account Assignment."))
           << GuiErrorCheck(!_discount->isValid(), _discount,
                            tr("<p>You must select a Discount Account before saving this A/P Account Assignment."))
           ;
  }

  XSqlQuery saveAssign;
  if (_mode == cNew)
  {
    saveAssign.prepare( "SELECT apaccnt_id "
               "FROM apaccnt "
               "WHERE ( (apaccnt_vendtype_id=:apaccnt_vendtype_id)"
               " AND (apaccnt_vendtype=:apaccnt_vendtype) );" );

    if (_allVendorTypes->isChecked())
    {
      saveAssign.bindValue(":apaccnt_vendtype_id", -1);
      saveAssign.bindValue(":apaccnt_vendtype", ".*");
    }
    else if (_selectedVendorType->isChecked())
    {
      saveAssign.bindValue(":apaccnt_vendtype_id", _vendorTypes->id());
      saveAssign.bindValue(":apaccnt_vendtype", "");
    }
    else if (_vendorTypePattern->isChecked())
    {
      saveAssign.bindValue(":apaccnt_vendtype_id", -1);
      saveAssign.bindValue(":apaccnt_vendtype", _vendorType->text().trimmed());
    }

    saveAssign.exec();
    if (saveAssign.first())
    {
      errors << GuiErrorCheck(true, _allVendorTypes,
                             tr("<p>You may not save this A/P Account Assignment as it already exists."));
    }
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save A/P Account Assignment"), errors))
    return;

  if (_mode == cNew)
  {
    saveAssign.exec("SELECT NEXTVAL('apaccnt_apaccnt_id_seq') AS _apaccnt_id;");
    if (saveAssign.first())
      _apaccntid = saveAssign.value("_apaccnt_id").toInt();
//  ToDo

    saveAssign.prepare( "INSERT INTO apaccnt "
               "( apaccnt_id, apaccnt_vendtype_id, apaccnt_vendtype,"
               "  apaccnt_ap_accnt_id, apaccnt_prepaid_accnt_id,"
               "  apaccnt_discount_accnt_id ) "
               "VALUES "
               "( :apaccnt_id, :apaccnt_vendtype_id, :apaccnt_vendtype,"
               "  :apaccnt_ap_accnt_id, :apaccnt_prepaid_accnt_id,"
               "  :apaccnt_discount_accnt_id ) " );
  }
  else if (_mode == cEdit)
    saveAssign.prepare( "UPDATE apaccnt "
               "SET apaccnt_vendtype_id=:apaccnt_vendtype_id,"
               "    apaccnt_vendtype=:apaccnt_vendtype,"
               "    apaccnt_ap_accnt_id=:apaccnt_ap_accnt_id,"
               "    apaccnt_prepaid_accnt_id=:apaccnt_prepaid_accnt_id,"
               "    apaccnt_discount_accnt_id=:apaccnt_discount_accnt_id "
               "WHERE (apaccnt_id=:apaccnt_id);" );

  saveAssign.bindValue(":apaccnt_id", _apaccntid);
  saveAssign.bindValue(":apaccnt_ap_accnt_id", _ap->id());
  saveAssign.bindValue(":apaccnt_prepaid_accnt_id", _prepaid->id());
  saveAssign.bindValue(":apaccnt_discount_accnt_id", _discount->id());

  if (_allVendorTypes->isChecked())
  {
    saveAssign.bindValue(":apaccnt_vendtype_id", -1);
    saveAssign.bindValue(":apaccnt_vendtype", ".*");
  }
  else if (_selectedVendorType->isChecked())
  {
    saveAssign.bindValue(":apaccnt_vendtype_id", _vendorTypes->id());
    saveAssign.bindValue(":apaccnt_vendtype", "");
  }
  else if (_vendorTypePattern->isChecked())
  {
    saveAssign.bindValue(":apaccnt_vendtype_id", -1);
    saveAssign.bindValue(":apaccnt_vendtype", _vendorType->text().trimmed());
  }

  saveAssign.exec();

  done(_apaccntid);
}

void apAccountAssignment::populate()
{
  XSqlQuery populateAssign;
  populateAssign.prepare( "SELECT apaccnt_vendtype_id, apaccnt_vendtype,"
             "       apaccnt_ap_accnt_id, apaccnt_prepaid_accnt_id,"
             "       apaccnt_discount_accnt_id "
             "FROM apaccnt "
             "WHERE (apaccnt_id=:apaccnt_id);" );
  populateAssign.bindValue(":apaccnt_id", _apaccntid);
  populateAssign.exec();
  if (populateAssign.first())
  {
    if (populateAssign.value("apaccnt_vendtype_id").toInt() == -1)
    {
      if (populateAssign.value("apaccnt_vendtype").toString() == ".*")
        _allVendorTypes->setChecked(true);
      else
      {
        _vendorTypePattern->setChecked(true);
        _vendorType->setText(populateAssign.value("apaccnt_vendtype").toString());
      }
    }
    else
    {
      _selectedVendorType->setChecked(true);
      _vendorTypes->setId(populateAssign.value("apaccnt_vendtype_id").toInt());
    }

    _ap->setId(populateAssign.value("apaccnt_ap_accnt_id").toInt());
    _prepaid->setId(populateAssign.value("apaccnt_prepaid_accnt_id").toInt());
    _discount->setId(populateAssign.value("apaccnt_discount_accnt_id").toInt());
  }
}


