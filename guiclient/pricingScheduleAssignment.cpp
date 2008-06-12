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

#include "pricingScheduleAssignment.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a pricingScheduleAssignment as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
pricingScheduleAssignment::pricingScheduleAssignment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_selectedCustomerType, SIGNAL(toggled(bool)), _customerTypes, SLOT(setEnabled(bool)));
  connect(_customerTypePattern, SIGNAL(toggled(bool)), _customerType, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sAssign()));
  connect(_selectedCustomerShipto, SIGNAL(toggled(bool)), _customerShipto, SLOT(setEnabled(bool)));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sCustomerSelected()));

  _customerTypes->setType(XComboBox::CustomerTypes);

  _ipshead->populate( "SELECT ipshead_id, (ipshead_name || ' - ' || ipshead_descrip) "
                      "FROM ipshead "
                      "WHERE (CURRENT_DATE BETWEEN ipshead_effective AND ipshead_expires) "
                      "ORDER BY ipshead_name;" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
pricingScheduleAssignment::~pricingScheduleAssignment()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void pricingScheduleAssignment::languageChange()
{
    retranslateUi(this);
}

enum SetResponse pricingScheduleAssignment::set(const ParameterList &pParams)
{
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

      _cust->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _customerGroup->setEnabled(FALSE);
      _ipshead->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void pricingScheduleAssignment::sAssign()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('ipsass_ipsass_id_seq') AS ipsass_id;");
    if (q.first())
      _ipsassid = q.value("ipsass_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO ipsass "
               "( ipsass_id, ipsass_ipshead_id,"
               "  ipsass_cust_id, ipsass_shipto_id, ipsass_shipto_pattern,"
               "  ipsass_custtype_id, ipsass_custtype_pattern ) "
               "VALUES "
               "( :ipsass_id, :ipsass_ipshead_id,"
               "  :ipsass_cust_id, :ipsass_shipto_id, :ipsass_shipto_pattern,"
               "  :ipsass_custtype_id, :ipsass_custtype_pattern );" );
  }
  else
    q.prepare( "UPDATE ipsass "
               "SET ipsass_id=:ipsass_id, ipsass_ipshead_id=:ipsass_ipshead_id,"
               "    ipsass_cust_id=:ipsass_cust_id,"
               "    ipsass_shipto_id=:ipsass_shipto_id,"
               "    ipsass_shipto_pattern=:ipsass_shipto_pattern,"
               "    ipsass_custtype_id=:ipsass_custtype_id,"
               "    ipsass_custtype_pattern=:ipsass_custtype_pattern "
               "WHERE (ipsass_id=:ipsass_id);" );

  q.bindValue(":ipsass_id", _ipsassid);
  q.bindValue(":ipsass_ipshead_id", _ipshead->id());

  if (_selectedCustomer->isChecked() || _selectedShiptoPattern->isChecked())
    q.bindValue(":ipsass_cust_id", _cust->id());
  else
    q.bindValue(":ipsass_cust_id", -1);

  if (_selectedCustomerShipto->isChecked())
    q.bindValue(":ipsass_shipto_id", _customerShipto->id());
  else
    q.bindValue(":ipsass_shipto_id", -1);
  
  if (_selectedCustomerType->isChecked())
    q.bindValue(":ipsass_custtype_id", _customerTypes->id());
  else
    q.bindValue(":ipsass_custtype_id", -1);

  if (_customerTypePattern->isChecked())
    q.bindValue(":ipsass_custtype_pattern", _customerType->text());
  else
    q.bindValue(":ipsass_custtype_pattern", "");

  if (_selectedShiptoPattern->isChecked())
    q.bindValue(":ipsass_shipto_pattern", _shiptoPattern->text());
  else
    q.bindValue(":ipsass_shipto_pattern", "");

  q.exec();

  done(_ipsassid);
}

void pricingScheduleAssignment::populate()
{
  q.prepare( "SELECT ipsass_ipshead_id, ipsass_cust_id,"
             "       ipsass_custtype_id, ipsass_custtype_pattern,"
             "       ipsass_shipto_pattern,"
             "       ipsass_shipto_id, shipto_cust_id "
             "FROM ipsass LEFT OUTER JOIN shipto ON (ipsass_shipto_id=shipto_id) "
             "WHERE (ipsass_id=:ipsass_id);" );
  q.bindValue(":ipsass_id", _ipsassid);
  q.exec();
  if (q.first())
  {
    _ipshead->setId(q.value("ipsass_ipshead_id").toInt());

    if (!q.value("ipsass_shipto_pattern").toString().isEmpty())
    {
      _selectedShiptoPattern->setChecked(TRUE);
      _shiptoPattern->setText(q.value("ipsass_shipto_pattern").toString());
      _cust->setId(q.value("ipsass_cust_id").toInt());
    }
    else if (q.value("ipsass_cust_id").toInt() != -1)
    {
      _selectedCustomer->setChecked(TRUE);
      _cust->setId(q.value("ipsass_cust_id").toInt());
    }
    else if (q.value("ipsass_shipto_id").toInt() != -1)
    {
      int shiptoid = q.value("ipsass_shipto_id").toInt();
      _selectedCustomerShipto->setChecked(TRUE);
      _cust->setId(q.value("shipto_cust_id").toInt());
      _customerShipto->setId(shiptoid);
    }
    else if (q.value("ipsass_custtype_id").toInt() != -1)
    {
      _selectedCustomerType->setChecked(TRUE);
      _customerTypes->setId(q.value("ipsass_custtype_id").toInt());
    }
    else
    {
      _customerTypePattern->setChecked(TRUE);
      _customerType->setText(q.value("ipsass_custtype_pattern").toString());
    }
  }
}

void pricingScheduleAssignment::sCustomerSelected()
{
  _customerShipto->clear();
  q.prepare("SELECT shipto_id, shipto_num"
            "  FROM shipto"
            " WHERE (shipto_cust_id=:cust_id); ");
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  _customerShipto->populate(q);
}

