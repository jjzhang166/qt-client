/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesHistoryInformation.h"

#include <QVariant>

salesHistoryInformation::salesHistoryInformation(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
  connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));

  _orderNumber->setValidator(omfgThis->orderVal());
  _invoiceNumber->setValidator(omfgThis->orderVal());
  _shipped->setValidator(omfgThis->qtyVal());
  
  _unitPrice->setValidator(omfgThis->priceVal());
  _extendedPrice->setPrecision(omfgThis->moneyVal());

  _unitPriceLit->setVisible(_privileges->check("ViewCustomerPrices"));
  _unitPrice->setVisible(_privileges->check("ViewCustomerPrices"));
  _extPriceLit->setVisible(_privileges->check("ViewCustomerPrices"));
  _extendedPrice->setVisible(_privileges->check("ViewCustomerPrices"));

  _unitCost->setValidator(omfgThis->priceVal());
  _extendedCost->setPrecision(omfgThis->moneyVal());

  _unitCostLit->setVisible(_privileges->check("ViewCosts"));
  _unitCost->setVisible(_privileges->check("ViewCosts"));
  _extCostLit->setVisible(_privileges->check("ViewCosts"));
  _extendedCost->setVisible(_privileges->check("ViewCosts"));

  _commission->setValidator(omfgThis->negMoneyVal());

  _salesrep->setType(XComboBox::SalesReps);

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

salesHistoryInformation::~salesHistoryInformation()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesHistoryInformation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesHistoryInformation::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("sohist_id", &valid);
  if (valid)
  {
    _sohistid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _orderNumber->setEnabled(false);
      _invoiceNumber->setEnabled(false);
      _orderDate->setEnabled(false);
      _invoiceDate->setEnabled(false);
      _billtoName->setEnabled(false);
      _billtoAddress1->setEnabled(false);
      _billtoAddress2->setEnabled(false);
      _billtoAddress3->setEnabled(false);
      _billtoCity->setEnabled(false);
      _billtoState->setEnabled(false);
      _billtoZip->setEnabled(false);
      _shiptoName->setEnabled(false);
      _shiptoAddress1->setEnabled(false);
      _shiptoAddress2->setEnabled(false);
      _shiptoAddress3->setEnabled(false);
      _shiptoCity->setEnabled(false);
      _shiptoState->setEnabled(false);
      _shiptoZip->setEnabled(false);
      _shipped->setEnabled(false);
      _unitPrice->setEnabled(false);
      _unitCost->setEnabled(false);
      _item->setReadOnly(true);
      _warehouse->setEnabled(false);
      _salesrep->setEnabled(false);
      _commission->setEnabled(false);
      _commissionPaid->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void salesHistoryInformation::sSave()
{
  XSqlQuery salesSave;
  salesSave.prepare( "UPDATE cohist "
             "SET cohist_ordernumber=:cohist_ordernumber, cohist_invcnumber=:cohist_invcnumber,"
             "    cohist_orderdate=:cohist_orderdate, cohist_invcdate=:cohist_invcdate,"
             "    cohist_billtoname=:cohist_billtoname, cohist_billtoaddress1=:cohist_billtoaddress1,"
             "    cohist_billtoaddress2=cohist_billtoaddress2, cohist_billtoaddress3=:cohist_billtoaddress3,"
             "    cohist_billtocity=:cohist_billtocity, cohist_billtostate=:cohist_billtostate,"
             "    cohist_billtozip=:cohist_billtozip,"
             "    cohist_shiptoname=:cohist_shiptoname, cohist_shiptoaddress1=:cohist_shiptoaddress1,"
             "    cohist_shiptoaddress2=:cohist_shiptoaddress2, cohist_shiptoaddress3=:cohist_shiptoaddress3,"
             "    cohist_shiptocity=:cohist_shiptocity, cohist_shiptostate=:cohist_shiptostate,"
             "    cohist_shiptozip=:cohist_shiptozip,"
             "    cohist_itemsite_id=itemsite_id,"
             "    cohist_qtyshipped=:cohist_qtyshipped,"
             "    cohist_unitprice=:cohist_unitprice, cohist_unitcost=:cohist_unitcost,"
             "    cohist_salesrep_id=:cohist_salesrep_id, cohist_commission=:cohist_commission,"
             "    cohist_commissionpaid=:cohist_commissionpaid "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id)"
             " AND (cohist_id=:cohist_id) );" );

  salesSave.bindValue(":cohist_id", _sohistid);
  salesSave.bindValue(":cohist_ordernumber", _orderNumber->text().toInt());
  salesSave.bindValue(":cohist_invcnumber", _invoiceNumber->text().toInt());
  salesSave.bindValue(":cohist_orderdate", _orderDate->date());
  salesSave.bindValue(":cohist_invcdate", _invoiceDate->date());
  salesSave.bindValue(":cohist_billtoname", _billtoName->text().trimmed());
  salesSave.bindValue(":cohist_billtoaddress1", _billtoAddress1->text().trimmed());
  salesSave.bindValue(":cohist_billtoaddress2", _billtoAddress2->text().trimmed());
  salesSave.bindValue(":cohist_billtoaddress3", _billtoAddress3->text().trimmed());
  salesSave.bindValue(":cohist_billtocity", _billtoCity->text().trimmed());
  salesSave.bindValue(":cohist_billtostate", _billtoState->text().trimmed());
  salesSave.bindValue(":cohist_billtozip", _billtoZip->text().trimmed());
  salesSave.bindValue(":cohist_shiptoname", _shiptoName->text().trimmed());
  salesSave.bindValue(":cohist_shiptoaddress1", _shiptoAddress1->text().trimmed());
  salesSave.bindValue(":cohist_shiptoaddress2", _shiptoAddress2->text().trimmed());
  salesSave.bindValue(":cohist_shiptoaddress3", _shiptoAddress3->text().trimmed());
  salesSave.bindValue(":cohist_shiptocity", _shiptoCity->text().trimmed());
  salesSave.bindValue(":cohist_shiptostate", _shiptoState->text().trimmed());
  salesSave.bindValue(":cohist_shiptozip", _shiptoZip->text().trimmed());
  salesSave.bindValue(":cohist_qtyshipped", _shipped->toDouble());
  salesSave.bindValue(":cohist_unitprice", _unitPrice->toDouble());
  salesSave.bindValue(":cohist_unitcost", _unitCost->toDouble());
  salesSave.bindValue(":item_id", _item->id());
  salesSave.bindValue(":warehous_id", _warehouse->id());
  salesSave.bindValue(":cohist_salesrep_id", _salesrep->id());
  salesSave.bindValue(":cohist_commission", _commission->toDouble());
  salesSave.bindValue(":cohist_commissionpaid", QVariant(_commissionPaid->isChecked()));
  // _isCcPayment is read-only, so don't save it
  salesSave.exec();

  done(_sohistid);
}

void salesHistoryInformation::populate()
{
  XSqlQuery salespopulate;
  salespopulate.prepare( "SELECT cohist_ordernumber, cohist_invcnumber,"
             "       cohist_orderdate, cohist_invcdate,"
             "       cohist_billtoname, cohist_billtoaddress1,"
             "       cohist_billtoaddress2, cohist_billtoaddress3,"
             "       cohist_billtocity, cohist_billtostate, cohist_billtozip,"
             "       cohist_shiptoname, cohist_shiptoaddress1,"
             "       cohist_shiptoaddress2, cohist_shiptoaddress3,"
             "       cohist_shiptocity, cohist_shiptostate, cohist_shiptozip,"
             "       cohist_itemsite_id, cohist_salesrep_id, cohist_commissionpaid,"
             "       cohist_qtyshipped, cohist_unitprice, cohist_unitcost,"
             "       (cohist_qtyshipped * cohist_unitprice) AS extprice,"
             "       (cohist_qtyshipped * cohist_unitcost) AS extcost,"
             "       cohist_commission, cohist_cohead_ccpay_id "
             "FROM cohist "
             "WHERE (cohist_id=:sohist_id);" );
  salespopulate.bindValue(":sohist_id", _sohistid);
  salespopulate.exec();
  if (salespopulate.first())
  {
    _orderNumber->setText(salespopulate.value("cohist_ordernumber"));
    _invoiceNumber->setText(salespopulate.value("cohist_invcnumber"));
    _orderDate->setDate(salespopulate.value("cohist_orderdate").toDate());
    _invoiceDate->setDate(salespopulate.value("cohist_invcdate").toDate());
    _billtoName->setText(salespopulate.value("cohist_billtoname"));
    _billtoAddress1->setText(salespopulate.value("cohist_billtoaddress1"));
    _billtoAddress2->setText(salespopulate.value("cohist_billtoaddress2"));
    _billtoAddress3->setText(salespopulate.value("cohist_billtoaddress3"));
    _billtoCity->setText(salespopulate.value("cohist_billtocity"));
    _billtoState->setText(salespopulate.value("cohist_billtostate"));
    _billtoZip->setText(salespopulate.value("cohist_billtozip"));
    _shiptoName->setText(salespopulate.value("cohist_shiptoname"));
    _shiptoAddress1->setText(salespopulate.value("cohist_shiptoaddress1"));
    _shiptoAddress2->setText(salespopulate.value("cohist_shiptoaddress2"));
    _shiptoAddress3->setText(salespopulate.value("cohist_shiptoaddress3"));
    _shiptoCity->setText(salespopulate.value("cohist_shiptocity"));
    _shiptoState->setText(salespopulate.value("cohist_shiptostate"));
    _shiptoZip->setText(salespopulate.value("cohist_shiptozip"));

    _item->setItemsiteid(salespopulate.value("cohist_itemsite_id").toInt());
    _shipped->setDouble(salespopulate.value("cohist_qtyshipped").toDouble());
    _unitPrice->setDouble(salespopulate.value("cohist_unitprice").toDouble());
    _unitCost->setDouble(salespopulate.value("cohist_unitcost").toDouble());
    _extendedPrice->setDouble(salespopulate.value("extprice").toDouble());
    _extendedCost->setDouble(salespopulate.value("extcost").toDouble());
    _salesrep->setId(salespopulate.value("cohist_salesrep_id").toInt());
    _commission->setDouble(salespopulate.value("cohist_commission").toDouble());
    _commissionPaid->setChecked(salespopulate.value("cohist_commissionpaid").toBool());
    _isCcPayment->setChecked(! salespopulate.value("cohist_cohead_ccpay_id").isNull());
  }
}

