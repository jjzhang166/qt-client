/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "allocateReservations.h"

#include <QVariant>
#include <QMessageBox>

allocateReservations::allocateReservations(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sAllocate()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sCustomerSelected()));

  _customerTypes->setType(XComboBox::CustomerTypes);
}

allocateReservations::~allocateReservations()
{
  // no need to delete child widgets, Qt does it all for us
}

void allocateReservations::languageChange()
{
  retranslateUi(this);
}

void allocateReservations::sAllocate()
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"), tr("You must specify a valid date range."));
    return;
  }

  XSqlQuery allocateRes;
  allocateRes.prepare("SELECT reserveAllSo(:addpackinglist, :partialreservations, :startDate, :endDate,"
                      "                    :cust_id, :shipto_id, :custtype_id, :custtype_pattern) AS result;");
  allocateRes.bindValue(":addpackinglist", _addPackingList->isChecked());
  allocateRes.bindValue(":partialreservations", _partialReservations->isChecked());
  _dates->bindValue(allocateRes);
  if (_selectedCustomer->isChecked())
    allocateRes.bindValue(":cust_id", _cust->id());
  if (_selectedCustomerShipto->isChecked())
    allocateRes.bindValue(":shipto_id", _customerShipto->id());
  if (_selectedCustomerType->isChecked())
    allocateRes.bindValue(":custtype_id", _customerTypes->id());
  if (_customerTypePattern->isChecked())
    allocateRes.bindValue(":custtype_pattern", _customerType->text());

  allocateRes.exec();

  accept();
}

void allocateReservations::sCustomerSelected()
{
  XSqlQuery selectedRes;
  _customerShipto->clear();
  selectedRes.prepare("SELECT shipto_id, shipto_num"
            "  FROM shiptoinfo"
            " WHERE (shipto_cust_id=:cust_id); ");
  selectedRes.bindValue(":cust_id", _cust->id());
  selectedRes.exec();
  _customerShipto->populate(selectedRes);
}

