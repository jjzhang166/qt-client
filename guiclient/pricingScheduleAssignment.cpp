/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "pricingScheduleAssignment.h"

#include <QVariant>
#include <QMessageBox>

pricingScheduleAssignment::pricingScheduleAssignment(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox,    SIGNAL(accepted()), this, SLOT(sAssign()));
  connect(_shiptoIdCust, SIGNAL(newId(int)), this, SLOT(sCustomerSelected()));

  _customerTypes->setType(XComboBox::CustomerTypes);

  _ipshead->setAllowNull(true);
  _ipshead->populate( "SELECT ipshead_id, (ipshead_name || ' - ' || ipshead_descrip) "
                      "FROM ipshead "
                      "WHERE (CURRENT_DATE < ipshead_expires) "
                      "ORDER BY ipshead_name;" );
}

pricingScheduleAssignment::~pricingScheduleAssignment()
{
  // no need to delete child widgets, Qt does it all for us
}

void pricingScheduleAssignment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse pricingScheduleAssignment::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("ipsass_id", &valid);
  if (valid)
  {
    _ipsassid = param.toInt();
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

      _customerGroup->setEnabled(false);
      _ipshead->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void pricingScheduleAssignment::sAssign()
{
  XSqlQuery pricingAssign;
  if (!_ipshead->isValid())
  {
    QMessageBox::critical(this, tr("Cannot Save Pricing Schedule Assignment"),
                          tr("<p>You must select a Pricing Schedule."));
    return;
  }

  pricingAssign.prepare("SELECT ipsass_id "
                        "FROM ipsass "
                        "WHERE ( (ipsass_ipshead_id=:ipsass_ipshead_id)"
                        "  AND   (ipsass_cust_id=:ipsass_cust_id)"
                        "  AND   (ipsass_shipto_id=:ipsass_shipto_id)"
                        "  AND   (ipsass_shipto_pattern=:ipsass_shipto_pattern)"
                        "  AND   (ipsass_custtype_id=:ipsass_custtype_id)"
                        "  AND   (ipsass_custtype_pattern=:ipsass_custtype_pattern) );" );

  pricingAssign.bindValue(":ipsass_ipshead_id", _ipshead->id());

  if (_selectedShiptoPattern->isChecked())
  {
    pricingAssign.bindValue(":ipsass_cust_id", _shiptoPatternCust->id());
    pricingAssign.bindValue(":ipsass_shipto_pattern", _shiptoPattern->text());
  }
  else
    pricingAssign.bindValue(":ipsass_shipto_pattern", "");
  
  if (_selectedCustomer->isChecked())
    pricingAssign.bindValue(":ipsass_cust_id", _cust->id());

  if (!_selectedShiptoPattern->isChecked() && !_selectedCustomer->isChecked())
    pricingAssign.bindValue(":ipsass_cust_id", -1);

  if (_selectedCustomerShipto->isChecked())
    pricingAssign.bindValue(":ipsass_shipto_id", _shiptoId->id());
  else
    pricingAssign.bindValue(":ipsass_shipto_id", -1);

  if (_selectedCustomerType->isChecked())
    pricingAssign.bindValue(":ipsass_custtype_id", _customerTypes->id());
  else
    pricingAssign.bindValue(":ipsass_custtype_id", -1);

  if (_customerTypePattern->isChecked())
    pricingAssign.bindValue(":ipsass_custtype_pattern", _customerType->text());
  else
    pricingAssign.bindValue(":ipsass_custtype_pattern", "");

  pricingAssign.exec();
  if (pricingAssign.first())
  {
    QMessageBox::critical(this, tr("Cannot Save Pricing Schedule Assignment"),
                          tr("<p>This Pricing Schedule Assignment already exists."));
    return;
  }

  if (_mode == cNew)
  {
    pricingAssign.exec("SELECT NEXTVAL('ipsass_ipsass_id_seq') AS ipsass_id;");
    if (pricingAssign.first())
      _ipsassid = pricingAssign.value("ipsass_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    pricingAssign.prepare( "INSERT INTO ipsass "
               "( ipsass_id, ipsass_ipshead_id,"
               "  ipsass_cust_id, ipsass_shipto_id, ipsass_shipto_pattern,"
               "  ipsass_custtype_id, ipsass_custtype_pattern ) "
               "VALUES "
               "( :ipsass_id, :ipsass_ipshead_id,"
               "  :ipsass_cust_id, :ipsass_shipto_id, :ipsass_shipto_pattern,"
               "  :ipsass_custtype_id, :ipsass_custtype_pattern );" );
  }
  else
    pricingAssign.prepare( "UPDATE ipsass "
               "SET ipsass_id=:ipsass_id, ipsass_ipshead_id=:ipsass_ipshead_id,"
               "    ipsass_cust_id=:ipsass_cust_id,"
               "    ipsass_shipto_id=:ipsass_shipto_id,"
               "    ipsass_shipto_pattern=:ipsass_shipto_pattern,"
               "    ipsass_custtype_id=:ipsass_custtype_id,"
               "    ipsass_custtype_pattern=:ipsass_custtype_pattern "
               "WHERE (ipsass_id=:ipsass_id);" );

  pricingAssign.bindValue(":ipsass_id", _ipsassid);
  pricingAssign.bindValue(":ipsass_ipshead_id", _ipshead->id());

  if (_selectedShiptoPattern->isChecked())
  {
    pricingAssign.bindValue(":ipsass_cust_id", _shiptoPatternCust->id());
    pricingAssign.bindValue(":ipsass_shipto_pattern", _shiptoPattern->text());
  }
  else
    pricingAssign.bindValue(":ipsass_shipto_pattern", "");
  
  if (_selectedCustomer->isChecked())
    pricingAssign.bindValue(":ipsass_cust_id", _cust->id());
  
  if (!_selectedShiptoPattern->isChecked() && !_selectedCustomer->isChecked())
    pricingAssign.bindValue(":ipsass_cust_id", -1);

  if (_selectedCustomerShipto->isChecked())
    pricingAssign.bindValue(":ipsass_shipto_id", _shiptoId->id());
  else
    pricingAssign.bindValue(":ipsass_shipto_id", -1);

  if (_selectedCustomerType->isChecked())
    pricingAssign.bindValue(":ipsass_custtype_id", _customerTypes->id());
  else
    pricingAssign.bindValue(":ipsass_custtype_id", -1);

  if (_customerTypePattern->isChecked())
    pricingAssign.bindValue(":ipsass_custtype_pattern", _customerType->text());
  else
    pricingAssign.bindValue(":ipsass_custtype_pattern", "");

  pricingAssign.exec();

  done(_ipsassid);
}

void pricingScheduleAssignment::populate()
{
  XSqlQuery pricingpopulate;
  pricingpopulate.prepare("SELECT ipsass.*, shipto_cust_id "
                          "FROM ipsass LEFT OUTER JOIN shiptoinfo ON (ipsass_shipto_id=shipto_id) "
                          "WHERE (ipsass_id=:ipsass_id);" );
  pricingpopulate.bindValue(":ipsass_id", _ipsassid);
  pricingpopulate.exec();
  if (pricingpopulate.first())
  {
    _ipshead->setId(pricingpopulate.value("ipsass_ipshead_id").toInt());

    if (!pricingpopulate.value("ipsass_shipto_pattern").toString().isEmpty())
    {
      _selectedShiptoPattern->setChecked(true);
      _shiptoPattern->setText(pricingpopulate.value("ipsass_shipto_pattern").toString());
      _shiptoPatternCust->setId(pricingpopulate.value("ipsass_cust_id").toInt());
    }
    else if (pricingpopulate.value("ipsass_cust_id").toInt() != -1)
    {
      _selectedCustomer->setChecked(true);
      _cust->setId(pricingpopulate.value("ipsass_cust_id").toInt());
    }
    else if (pricingpopulate.value("ipsass_shipto_id").toInt() != -1)
    {
      _selectedCustomerShipto->setChecked(true);
      _shiptoIdCust->setId(pricingpopulate.value("shipto_cust_id").toInt());
      _shiptoId->setId(pricingpopulate.value("ipsass_shipto_id").toInt());
    }
    else if (pricingpopulate.value("ipsass_custtype_id").toInt() != -1)
    {
      _selectedCustomerType->setChecked(true);
      _customerTypes->setId(pricingpopulate.value("ipsass_custtype_id").toInt());
    }
    else
    {
      _customerTypePattern->setChecked(true);
      _customerType->setText(pricingpopulate.value("ipsass_custtype_pattern").toString());
    }
  }
}

void pricingScheduleAssignment::sCustomerSelected()
{
  XSqlQuery pricingCustomerSelected;
  _shiptoId->clear();
  pricingCustomerSelected.prepare("SELECT shipto_id, shipto_num"
            "  FROM shiptoinfo"
            " WHERE (shipto_cust_id=:cust_id); ");
  pricingCustomerSelected.bindValue(":cust_id", _shiptoIdCust->id());
  pricingCustomerSelected.exec();
  _shiptoId->populate(pricingCustomerSelected);
}

