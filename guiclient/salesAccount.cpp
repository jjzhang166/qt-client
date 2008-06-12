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

#include "salesAccount.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

salesAccount::salesAccount(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _warehouse->populate( "SELECT -1, 'Any'::text AS warehous_code, 0 AS sort "
			"UNION SELECT warehous_id, warehous_code, 1 AS sort "
			"FROM warehous "
			"ORDER BY sort, warehous_code" );

  _customerTypes->setType(CustomerType);
  _productCategories->setType(ProductCategory);

  if (! _metrics->boolean("EnableReturnAuth"))
  {
    _corLit->setVisible(false);
    _cor->setVisible(false);
    _returnsLit->setVisible(false);
    _returns->setVisible(false);
    _cowLit->setVisible(false);
    _cow->setVisible(false);
  }
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

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _warehouse->setEnabled(FALSE);
      _customerTypes->setEnabled(FALSE);
      _productCategories->setEnabled(FALSE);
      _sales->setReadOnly(TRUE);
      _credit->setReadOnly(TRUE);
      _cos->setReadOnly(TRUE);
      _returns->setReadOnly(TRUE);
      _cor->setReadOnly(TRUE);
      _cow->setReadOnly(TRUE);
      _save->hide();
      _close->setText(tr("&Close"));

      _close->setFocus();
    }
  }

  return NoError;
}

void salesAccount::sSave()
{
  if (!_sales->isValid())
  {
    QMessageBox::warning( this, tr("Select Sales Account"),
                          tr("You must select a Sales Account for this Assignment.") );
    _sales->setFocus();
    return;
  }

  if (!_credit->isValid())
  {
    QMessageBox::warning( this, tr("Select Credit Memo Account"),
                          tr("You must select a Credit Memo Account for this Assignment.") );
    _credit->setFocus();
    return;
  }

  if (!_cos->isValid())
  {
    QMessageBox::warning( this, tr("Select Cost of Sales Account"), 
                          tr("You must select a Cost of Sales Account for this Assignment.") );
    _cos->setFocus();
    return;
  }

  if (_metrics->value("Application") == "OpenMFG" && _metrics->boolean("EnableReturnAuth"))
  {

    if (!_returns->isValid())
    {
      QMessageBox::warning( this, tr("Select Returns Account"), 
			    tr("You must select a Returns Account for this Assignment.") );
      _returns->setFocus();
      return;
    }

    if (!_cor->isValid())
    {
      QMessageBox::warning( this, tr("Select Cost of Returns Account"), 
			    tr("You must select a Cost of Returns Account for this Assignment.") );
      _cor->setFocus();
      return;
    }

    if (!_cow->isValid())
    {
      QMessageBox::warning( this, tr("Select Cost of Warranty Account"), 
			    tr("You must select a Cost of Warranty Account for this Assignment.") );
      _cow->setFocus();
      return;
    }
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('salesaccnt_salesaccnt_id_seq') AS salesaccnt_id;");
    if (q.first())
      _salesaccntid = q.value("salesaccnt_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
 
    q.prepare( "INSERT INTO salesaccnt "
               "( salesaccnt_warehous_id,"
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
               "  :salesaccnt_custtype, :salesaccnt_custtype_id,"
               "  :salesaccnt_prodcat, :salesaccnt_prodcat_id,"
               "  :salesaccnt_sales_accnt_id,"
               "  :salesaccnt_credit_accnt_id,"
               "  :salesaccnt_cos_accnt_id,"
	       "  :salesaccnt_returns_accnt_id,"
	       "  :salesaccnt_cor_accnt_id,"
	       "  :salesaccnt_cow_accnt_id);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE salesaccnt "
               "SET salesaccnt_warehous_id=:salesaccnt_warehous_id,"
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

  q.bindValue(":salesaccnt_id", _salesaccntid);
  q.bindValue(":salesaccnt_sales_accnt_id", _sales->id());
  q.bindValue(":salesaccnt_credit_accnt_id", _credit->id());
  q.bindValue(":salesaccnt_cos_accnt_id", _cos->id());
  if (_returns->isValid())
    q.bindValue(":salesaccnt_returns_accnt_id",	_returns->id());
  if (_cor->isValid())
    q.bindValue(":salesaccnt_cor_accnt_id",	_cor->id());
  if (_cow->isValid())
    q.bindValue(":salesaccnt_cow_accnt_id",	_cow->id());

  q.bindValue(":salesaccnt_warehous_id", _warehouse->id());

  if (_customerTypes->isAll())
  {
    q.bindValue(":salesaccnt_custtype", ".*");
    q.bindValue(":salesaccnt_custtype_id", -1);
  }
  else if (_customerTypes->isSelected())
  {
    q.bindValue(":salesaccnt_custtype", "[^a-zA-Z0-9_]");
    q.bindValue(":salesaccnt_custtype_id", _customerTypes->id());
  }
  else
  {
    q.bindValue(":salesaccnt_custtype", _customerTypes->pattern());
    q.bindValue(":salesaccnt_custtype_id", -1);
  }

  if (_productCategories->isAll())
  {
    q.bindValue(":salesaccnt_prodcat", ".*");
    q.bindValue(":salesaccnt_prodcat_id", -1);
  }
  else if (_productCategories->isSelected())
  {
    q.bindValue(":salesaccnt_prodcat", "[^a-zA-Z0-9_]");
    q.bindValue(":salesaccnt_prodcat_id", _productCategories->id());
  }
  else
  {
    q.bindValue(":salesaccnt_prodcat", _productCategories->pattern());
    q.bindValue(":salesaccnt_prodcat_id", -1);
  }

  q.exec();

  done(_salesaccntid);
}

void salesAccount::populate()
{
  q.prepare( "SELECT salesaccnt_warehous_id,"
             "       salesaccnt_custtype, salesaccnt_custtype_id,"
             "       salesaccnt_prodcat, salesaccnt_prodcat_id,"
             "       salesaccnt_sales_accnt_id, salesaccnt_credit_accnt_id,"
             "       salesaccnt_cos_accnt_id,"
             "       salesaccnt_returns_accnt_id,"
             "       salesaccnt_cor_accnt_id,"
             "       salesaccnt_cow_accnt_id "
             "FROM salesaccnt "
             "WHERE (salesaccnt_id=:salesaccnt_id);" );
  q.bindValue(":salesaccnt_id", _salesaccntid);
  q.exec();
  if (q.first())
  {
    _warehouse->setId(q.value("salesaccnt_warehous_id").toInt());

    if (q.value("salesaccnt_custtype_id").toInt() != -1)
      _customerTypes->setId(q.value("salesaccnt_custtype_id").toInt());
    else if (q.value("salesaccnt_custtype").toString() != ".*")
      _customerTypes->setPattern(q.value("salesaccnt_custtype").toString());
  
    if (q.value("salesaccnt_prodcat_id").toInt() != -1)
      _productCategories->setId(q.value("salesaccnt_prodcat_id").toInt());
    else if (q.value("salesaccnt_prodcat").toString() != ".*")
      _productCategories->setPattern(q.value("salesaccnt_prodcat").toString());
  
    _sales->setId(q.value("salesaccnt_sales_accnt_id").toInt());
    _credit->setId(q.value("salesaccnt_credit_accnt_id").toInt());
    _cos->setId(q.value("salesaccnt_cos_accnt_id").toInt());

    if (!q.value("salesaccnt_returns_accnt_id").isNull())
      _returns->setId(q.value("salesaccnt_returns_accnt_id").toInt());
    if (!q.value("salesaccnt_cor_accnt_id").isNull())
      _cor->setId(q.value("salesaccnt_cor_accnt_id").toInt());
    if (!q.value("salesaccnt_cow_accnt_id").isNull())
      _cow->setId(q.value("salesaccnt_cow_accnt_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
