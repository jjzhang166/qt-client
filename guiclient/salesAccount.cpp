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
#include "salesAccount.h"

#include <metasql.h>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

salesAccount::salesAccount(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));

  ParameterList params;
  params.append("any", tr("Any"));

  MetaSQLQuery warehousemql("SELECT -1, <? value('any') ?> AS warehous_code, 0 AS sort "
                            "UNION SELECT warehous_id, warehous_code, 1 AS sort "
                            "FROM whsinfo "
                            "ORDER BY sort, warehous_code");
  _warehouse->populate(warehousemql.toQuery(params));

  MetaSQLQuery shippingzonemql("SELECT -1, <? value('any') ?> AS shipzone_name, 0 AS sort "
                               "UNION SELECT shipzone_id, shipzone_name, 1 AS sort "
                               "FROM shipzone "
                               "ORDER BY sort, shipzone_name");
  _shippingZones->populate(shippingzonemql.toQuery(params));

  MetaSQLQuery saletypemql("SELECT -1, <? value('any') ?> AS saletype_code, 0 AS sort "
                           "UNION SELECT saletype_id, saletype_code, 1 AS sort "
                           "FROM saletype "
                           "WHERE (saletype_active) "
                           "ORDER BY sort, saletype_code");
  _saleTypes->populate(saletypemql.toQuery(params));

  _customerTypes->setType(ParameterGroup::CustomerType);
  _productCategories->setType(ParameterGroup::ProductCategory);

  if (! _metrics->boolean("EnableReturnAuth"))
  {
    _corLit->setVisible(false);
    _cor->setVisible(false);
    _returnsLit->setVisible(false);
    _returns->setVisible(false);
    _cowLit->setVisible(false);
    _cow->setVisible(false);
  }

  _sales->setType(GLCluster::cRevenue);
  _credit->setType(GLCluster::cRevenue);
  _cos->setType(GLCluster::cExpense);
  _returns->setType(GLCluster::cRevenue);
  _cor->setType(GLCluster::cExpense);
  _cow->setType(GLCluster::cExpense);
}

salesAccount::~salesAccount()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesAccount::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesAccount::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("salesaccnt_id", &valid);
  if (valid)
  {
    _salesaccntid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _warehouse->setEnabled(false);
      _shippingZones->setEnabled(false);
      _saleTypes->setEnabled(false);
      _customerTypes->setEnabled(false);
      _productCategories->setEnabled(false);
      _sales->setReadOnly(true);
      _credit->setReadOnly(true);
      _cos->setReadOnly(true);
      _returns->setReadOnly(true);
      _cor->setReadOnly(true);
      _cow->setReadOnly(true);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void salesAccount::sSave()
{
  XSqlQuery salesSave;
  QList<GuiErrorCheck> errors;

  if (_metrics->boolean("InterfaceARToGL"))
  {
    errors << GuiErrorCheck(!_sales->isValid(), _sales,
                            tr("<p>You must select a Sales Account for this Assignment."))
           << GuiErrorCheck(!_credit->isValid(), _credit,
                            tr("<p>You must select a Return Account for this Assignment."))
           << GuiErrorCheck(!_cos->isValid(), _cos,
                            tr("<p>You must select a Cost of Sales Account for this Assignment."))
           << GuiErrorCheck(_metrics->boolean("EnableReturnAuth") && !_returns->isValid(), _returns,
                            tr("<p>You must select a Returns Account for this Assignment."))
           << GuiErrorCheck(_metrics->boolean("EnableReturnAuth") && !_cor->isValid(), _cor,
                            tr("<p>You must select a Cost of Returns Account for this Assignment."))
           << GuiErrorCheck(_metrics->boolean("EnableReturnAuth") && !_cow->isValid(), _cow,
                            tr("<p>You must select a Cost of Warranty Account for this Assignment."))
           ;
  }

  salesSave.prepare("SELECT salesaccnt_id"
            "  FROM salesaccnt"
            " WHERE((salesaccnt_warehous_id=:salesaccnt_warehous_id)"
            "   AND (salesaccnt_shipzone_id=:salesaccnt_shipzone_id)"
            "   AND (salesaccnt_saletype_id=:salesaccnt_saletype_id)"
            "   AND (salesaccnt_custtype=:salesaccnt_custtype)"
            "   AND (salesaccnt_custtype_id=:salesaccnt_custtype_id)"
            "   AND (salesaccnt_prodcat=:salesaccnt_prodcat)"
            "   AND (salesaccnt_prodcat_id=:salesaccnt_prodcat_id)"
            "   AND (salesaccnt_id != :salesaccnt_id))");
  salesSave.bindValue(":salesaccnt_id", _salesaccntid);
  salesSave.bindValue(":salesaccnt_warehous_id", _warehouse->id());
  salesSave.bindValue(":salesaccnt_shipzone_id", _shippingZones->id());
  salesSave.bindValue(":salesaccnt_saletype_id", _saleTypes->id());

  if (_customerTypes->isAll())
  {
    salesSave.bindValue(":salesaccnt_custtype", ".*");
    salesSave.bindValue(":salesaccnt_custtype_id", -1);
  }
  else if (_customerTypes->isSelected())
  {
    salesSave.bindValue(":salesaccnt_custtype", "[^a-zA-Z0-9_]");
    salesSave.bindValue(":salesaccnt_custtype_id", _customerTypes->id());
  }
  else
  {
    XSqlQuery regexCheck;
    regexCheck.prepare("SELECT custtype_id "
                       "FROM custtype "
                       "WHERE (custtype_code ~ :pattern);");
    regexCheck.bindValue(":pattern", _customerTypes->pattern());
    regexCheck.exec();
    if (regexCheck.lastError().type() != QSqlError::NoError)
    {
      systemError(this, regexCheck.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    salesSave.bindValue(":salesaccnt_custtype", _customerTypes->pattern());
    salesSave.bindValue(":salesaccnt_custtype_id", -1);
  }

  if (_productCategories->isAll())
  {
    salesSave.bindValue(":salesaccnt_prodcat", ".*");
    salesSave.bindValue(":salesaccnt_prodcat_id", -1);
  }
  else if (_productCategories->isSelected())
  {
    salesSave.bindValue(":salesaccnt_prodcat", "[^a-zA-Z0-9_]");
    salesSave.bindValue(":salesaccnt_prodcat_id", _productCategories->id());
  }
  else
  {
    XSqlQuery regexCheck;
    regexCheck.prepare("SELECT prodcat_id "
                       "FROM prodcat "
                       "WHERE (prodcat_code ~ :pattern);");
    regexCheck.bindValue(":pattern", _productCategories->pattern());
    regexCheck.exec();
    if (regexCheck.lastError().type() != QSqlError::NoError)
    {
      systemError(this, regexCheck.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    salesSave.bindValue(":salesaccnt_prodcat", _productCategories->pattern());
    salesSave.bindValue(":salesaccnt_prodcat_id", -1);
  }
  salesSave.exec();
  if(salesSave.first())
  {
    errors << GuiErrorCheck(true, _warehouse,
                           tr("<p>You cannot specify a duplicate Warehouse/Customer Type/Product Category for the Sales Account Assignment."));
  }
  else if (salesSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Sales Account Assignment"), errors))
    return;

  if (_mode == cNew)
  {
    salesSave.prepare( "INSERT INTO salesaccnt "
               "( salesaccnt_warehous_id,"
               "  salesaccnt_shipzone_id, salesaccnt_saletype_id,"
               "  salesaccnt_custtype, salesaccnt_custtype_id,"
               "  salesaccnt_prodcat, salesaccnt_prodcat_id,"
               "  salesaccnt_sales_accnt_id,"
               "  salesaccnt_credit_accnt_id,"
               "  salesaccnt_cos_accnt_id,"
	       "  salesaccnt_returns_accnt_id,"
	       "  salesaccnt_cor_accnt_id,"
	       "  salesaccnt_cow_accnt_id) "
               "VALUES "
               "( :salesaccnt_warehous_id,"
               "  :salesaccnt_shipzone_id, :salesaccnt_saletype_id,"
               "  :salesaccnt_custtype, :salesaccnt_custtype_id,"
               "  :salesaccnt_prodcat, :salesaccnt_prodcat_id,"
               "  :salesaccnt_sales_accnt_id,"
               "  :salesaccnt_credit_accnt_id,"
               "  :salesaccnt_cos_accnt_id,"
	       "  :salesaccnt_returns_accnt_id,"
	       "  :salesaccnt_cor_accnt_id,"
               "  :salesaccnt_cow_accnt_id ) "
               "RETURNING salesaccnt_id;" );
  }
  else if (_mode == cEdit)
    salesSave.prepare( "UPDATE salesaccnt "
               "SET salesaccnt_warehous_id=:salesaccnt_warehous_id,"
               "    salesaccnt_shipzone_id=:salesaccnt_shipzone_id,"
               "    salesaccnt_saletype_id=:salesaccnt_saletype_id,"
               "    salesaccnt_custtype=:salesaccnt_custtype,"
               "    salesaccnt_custtype_id=:salesaccnt_custtype_id,"
               "    salesaccnt_prodcat=:salesaccnt_prodcat,"
               "    salesaccnt_prodcat_id=:salesaccnt_prodcat_id,"
               "    salesaccnt_sales_accnt_id=:salesaccnt_sales_accnt_id,"
               "    salesaccnt_credit_accnt_id=:salesaccnt_credit_accnt_id,"
               "    salesaccnt_cos_accnt_id=:salesaccnt_cos_accnt_id,"
               "    salesaccnt_returns_accnt_id=:salesaccnt_returns_accnt_id,"
               "    salesaccnt_cor_accnt_id=:salesaccnt_cor_accnt_id,"
               "    salesaccnt_cow_accnt_id=:salesaccnt_cow_accnt_id "
               "WHERE (salesaccnt_id=:salesaccnt_id);" );

  salesSave.bindValue(":salesaccnt_id", _salesaccntid);
  salesSave.bindValue(":salesaccnt_sales_accnt_id", _sales->id());
  salesSave.bindValue(":salesaccnt_credit_accnt_id", _credit->id());
  salesSave.bindValue(":salesaccnt_cos_accnt_id", _cos->id());
  if (_returns->isValid())
    salesSave.bindValue(":salesaccnt_returns_accnt_id",	_returns->id());
  if (_cor->isValid())
    salesSave.bindValue(":salesaccnt_cor_accnt_id",	_cor->id());
  if (_cow->isValid())
    salesSave.bindValue(":salesaccnt_cow_accnt_id",	_cow->id());

  salesSave.bindValue(":salesaccnt_warehous_id", _warehouse->id());
  salesSave.bindValue(":salesaccnt_shipzone_id", _shippingZones->id());
  salesSave.bindValue(":salesaccnt_saletype_id", _saleTypes->id());

  if (_customerTypes->isAll())
  {
    salesSave.bindValue(":salesaccnt_custtype", ".*");
    salesSave.bindValue(":salesaccnt_custtype_id", -1);
  }
  else if (_customerTypes->isSelected())
  {
    salesSave.bindValue(":salesaccnt_custtype", "[^a-zA-Z0-9_]");
    salesSave.bindValue(":salesaccnt_custtype_id", _customerTypes->id());
  }
  else
  {
    salesSave.bindValue(":salesaccnt_custtype", _customerTypes->pattern());
    salesSave.bindValue(":salesaccnt_custtype_id", -1);
  }

  if (_productCategories->isAll())
  {
    salesSave.bindValue(":salesaccnt_prodcat", ".*");
    salesSave.bindValue(":salesaccnt_prodcat_id", -1);
  }
  else if (_productCategories->isSelected())
  {
    salesSave.bindValue(":salesaccnt_prodcat", "[^a-zA-Z0-9_]");
    salesSave.bindValue(":salesaccnt_prodcat_id", _productCategories->id());
  }
  else
  {
    salesSave.bindValue(":salesaccnt_prodcat", _productCategories->pattern());
    salesSave.bindValue(":salesaccnt_prodcat_id", -1);
  }

  salesSave.exec();
  if (_mode == cNew && salesSave.first())
    _salesaccntid = salesSave.value("salesaccnt_id").toInt();
  else if (salesSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, salesSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_salesaccntid);
}

void salesAccount::populate()
{
  XSqlQuery salespopulate;
  salespopulate.prepare( "SELECT * "
             "FROM salesaccnt "
             "WHERE (salesaccnt_id=:salesaccnt_id);" );
  salespopulate.bindValue(":salesaccnt_id", _salesaccntid);
  salespopulate.exec();
  if (salespopulate.first())
  {
    _warehouse->setId(salespopulate.value("salesaccnt_warehous_id").toInt());
    _shippingZones->setId(salespopulate.value("salesaccnt_shipzone_id").toInt());
    _saleTypes->setId(salespopulate.value("salesaccnt_saletype_id").toInt());

    if (salespopulate.value("salesaccnt_custtype_id").toInt() != -1)
      _customerTypes->setId(salespopulate.value("salesaccnt_custtype_id").toInt());
    else if (salespopulate.value("salesaccnt_custtype").toString() != ".*")
      _customerTypes->setPattern(salespopulate.value("salesaccnt_custtype").toString());
  
    if (salespopulate.value("salesaccnt_prodcat_id").toInt() != -1)
      _productCategories->setId(salespopulate.value("salesaccnt_prodcat_id").toInt());
    else if (salespopulate.value("salesaccnt_prodcat").toString() != ".*")
      _productCategories->setPattern(salespopulate.value("salesaccnt_prodcat").toString());
  
    _sales->setId(salespopulate.value("salesaccnt_sales_accnt_id").toInt());
    _credit->setId(salespopulate.value("salesaccnt_credit_accnt_id").toInt());
    _cos->setId(salespopulate.value("salesaccnt_cos_accnt_id").toInt());

    if (!salespopulate.value("salesaccnt_returns_accnt_id").isNull())
      _returns->setId(salespopulate.value("salesaccnt_returns_accnt_id").toInt());
    if (!salespopulate.value("salesaccnt_cor_accnt_id").isNull())
      _cor->setId(salespopulate.value("salesaccnt_cor_accnt_id").toInt());
    if (!salespopulate.value("salesaccnt_cow_accnt_id").isNull())
      _cow->setId(salespopulate.value("salesaccnt_cow_accnt_id").toInt());
  }
  else if (salespopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, salespopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
