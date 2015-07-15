/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesOrderInformation.h"

#include <QSqlError>
#include <QVariant>

salesOrderInformation::salesOrderInformation(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _soitemid = 0;
}

salesOrderInformation::~salesOrderInformation()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesOrderInformation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesOrderInformation::set(const ParameterList &pParams)
{
  XSqlQuery saleset;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _soitemid = param.toInt();
    saleset.prepare( "SELECT coitem_cohead_id "
               "FROM coitem "
               "WHERE (coitem_id=:soitem_id);" );
    saleset.bindValue(":soitem_id", _soitemid);
    saleset.exec();
    if (saleset.first())
    {
      _soheadid = saleset.value("coitem_cohead_id").toInt();
      populate();
    }
    else if (saleset.lastError().type() != QSqlError::NoError)
    {
      systemError(this, saleset.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _soheadid = param.toInt();
    populate();
  }

  return NoError;
}

void salesOrderInformation::populate()
{
  XSqlQuery salespopulate;
  salespopulate.prepare( "SELECT cohead_number, warehous_code,"
             "       formatDate(cohead_orderdate) AS f_orderdate,"
             "       formatDate(MIN(coitem_scheddate)) AS f_shipdate,"
             "       formatDate(cohead_packdate) AS f_packdate,"
             "       CASE WHEN (cohead_holdtype='N') THEN :none"
             "            WHEN (cohead_holdtype='C') THEN :credit"
             "            WHEN (cohead_holdtype='S') THEN :ship"
             "            WHEN (cohead_holdtype='P') THEN :pack"
             "            WHEN (cohead_holdtype='R') THEN :return"
             "            ELSE :other"
             "       END AS f_holdtype,"
             "       cohead_shipvia, cohead_billtoname,"
             "       cohead_billtoaddress1, cohead_billtoaddress2, cohead_billtoaddress3,"
             "       cohead_billtocity, cohead_billtostate, cohead_billtozipcode,"
             "       cohead_shiptoname,"
             "       cohead_shiptoaddress1, cohead_shiptoaddress2, cohead_shiptoaddress3,"
             "       cohead_shiptocity, cohead_shiptostate, cohead_shiptozipcode "
             "FROM cohead, coitem, itemsite LEFT OUTER JOIN "
         "     whsinfo ON (itemsite_warehous_id = warehous_id) "
             "WHERE ( (coitem_cohead_id=cohead_id)"
	     " AND (coitem_itemsite_id=itemsite_id) "
             " AND (coitem_status <> 'X')"
             " AND (coitem_id=:soitem_id) "
             " AND (cohead_id=:sohead_id) ) "
             "GROUP BY cohead_number, warehous_code, cohead_orderdate, cohead_packdate,"
             "         cohead_holdtype, cohead_shipvia, cohead_billtoname,"
             "         cohead_billtoaddress1, cohead_billtoaddress2, cohead_billtoaddress3,"
             "         cohead_billtocity, cohead_billtostate, cohead_billtozipcode,"
             "         cohead_shiptoname,"
             "         cohead_shiptoaddress1, cohead_shiptoaddress2, cohead_shiptoaddress3,"
             "         cohead_shiptocity, cohead_shiptostate, cohead_shiptozipcode;" );
  salespopulate.bindValue(":none",   tr("None"));
  salespopulate.bindValue(":credit", tr("Credit"));
  salespopulate.bindValue(":ship",   tr("Ship"));
  salespopulate.bindValue(":pack",   tr("Pack"));
  salespopulate.bindValue(":return", tr("Return"));
  salespopulate.bindValue(":other",  tr("Other"));
  salespopulate.bindValue(":sohead_id", _soheadid);
  salespopulate.bindValue(":soitem_id", _soitemid);
  salespopulate.exec();
  if (salespopulate.first())
  {
    _orderNumber->setText(salespopulate.value("cohead_number").toString());
    _warehouse->setText(salespopulate.value("warehous_code").toString());
    _orderDate->setText(salespopulate.value("f_orderdate").toString());
    _shipDate->setText(salespopulate.value("f_shipdate").toString());
    _packDate->setText(salespopulate.value("f_packdate").toString());
    _shipVia->setText(salespopulate.value("cohead_shipvia").toString());
    _holdType->setText(salespopulate.value("f_holdtype").toString());

    _billtoName->setText(salespopulate.value("cohead_billtoname").toString());
    _billtoAddress1->setText(salespopulate.value("cohead_billtoaddress1").toString());
    _billtoAddress2->setText(salespopulate.value("cohead_billtoaddress2").toString());
    _billtoAddress3->setText(salespopulate.value("cohead_billtoaddress3").toString());
    _billtoCity->setText(salespopulate.value("cohead_billtocity").toString());
    _billtoState->setText(salespopulate.value("cohead_billtostate").toString());
    _billtoZipCode->setText(salespopulate.value("cohead_billtozipcode").toString());

    _shiptoName->setText(salespopulate.value("cohead_shiptoname").toString());
    _shiptoAddress1->setText(salespopulate.value("cohead_shiptoaddress1").toString());
    _shiptoAddress2->setText(salespopulate.value("cohead_shiptoaddress2").toString());
    _shiptoAddress3->setText(salespopulate.value("cohead_shiptoaddress3").toString());
    _shiptoCity->setText(salespopulate.value("cohead_shiptocity").toString());
    _shiptoState->setText(salespopulate.value("cohead_shiptostate").toString());
    _shiptoZipCode->setText(salespopulate.value("cohead_shiptozipcode").toString());
  }
}
