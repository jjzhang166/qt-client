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

#include "dspCustomerInformationExport.h"

#include <qvariant.h>

/*
 *  Constructs a dspCustomerInformationExport as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCustomerInformationExport::dspCustomerInformationExport(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sQuery()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCustomerInformationExport::~dspCustomerInformationExport()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCustomerInformationExport::languageChange()
{
    retranslateUi(this);
}


void dspCustomerInformationExport::init()
{
  _customerType->setType(CustomerType);
}

void dspCustomerInformationExport::sQuery()
{
  _cust->clear();
  _cust->setColumnCount(0);

  QString sql("SELECT cust_id ");

  if (_number->isChecked())
  {
    _cust->addColumn(tr("Customer Number"), 100, Qt::AlignLeft);
    sql += ", cust_number ";
  }

  if (_name->isChecked())
  {
    _cust->addColumn(tr("Customer Name"), 100, Qt::AlignLeft);
    sql += ", cust_name ";
  }

  if (_billAddress1->isChecked())
  {
    _cust->addColumn(tr("Billing Address 1"), 100, Qt::AlignLeft);
    sql += ", cust_address1 ";
  }

  if (_billAddress2->isChecked())
  {
    _cust->addColumn(tr("Billing Address 2"), 100, Qt::AlignLeft);
    sql += ", cust_address2 ";
  }

  if (_billAddress3->isChecked())
  {
    _cust->addColumn(tr("Billing Address 3"), 100, Qt::AlignLeft);
    sql += ", cust_address3 ";
  }

  if (_billCity->isChecked())
  {
    _cust->addColumn(tr("Billing City"), 100, Qt::AlignLeft);
    sql += ", cust_city ";
  }

  if (_billState->isChecked())
  {
    _cust->addColumn(tr("Billing State"), 100, Qt::AlignLeft);
    sql += ", cust_state ";
  }

  if (_billZipCode->isChecked())
  {
    _cust->addColumn(tr("Billing Zip Code"), 100, Qt::AlignLeft);
    sql += ", cust_zipcode ";
  }

  if (_billCountry->isChecked())
  {
    _cust->addColumn(tr("Billing Country"), 100, Qt::AlignLeft);
    sql += ", cust_country ";
  }

  if (_billContact->isChecked())
  {
    _cust->addColumn(tr("Billing Contact"), 100, Qt::AlignLeft);
    sql += ", cust_contact ";
  }

  if (_billPhone->isChecked())
  {
    _cust->addColumn(tr("Billing Phone"), 100, Qt::AlignLeft);
    sql += ", cust_phone ";
  }

  if (_billFAX->isChecked())
  {
    _cust->addColumn(tr("Billing FAX"), 100, Qt::AlignLeft);
    sql += ", cust_fax ";
  }

  if (_billEmail->isChecked())
  {
    _cust->addColumn(tr("Billing Email"), 100, Qt::AlignLeft);
    sql += ", cust_email ";
  }

  if (_corrAddress1->isChecked())
  {
    _cust->addColumn(tr("Corr. Address 1"), 100, Qt::AlignLeft);
    sql += ", cust_corraddress1 ";
  }

  if (_corrAddress2->isChecked())
  {
    _cust->addColumn(tr("Corr. Address 2"), 100, Qt::AlignLeft);
    sql += ", cust_corraddress2 ";
  }

  if (_corrAddress3->isChecked())
  {
    _cust->addColumn(tr("Corr. Address 3"), 100, Qt::AlignLeft);
    sql += ", cust_corraddress3 ";
  }

  if (_corrCity->isChecked())
  {
    _cust->addColumn(tr("Corr. City"), 100, Qt::AlignLeft);
    sql += ", cust_corrcity ";
  }

  if (_corrState->isChecked())
  {
    _cust->addColumn(tr("Corr. State"), 100, Qt::AlignLeft);
    sql += ", cust_corrstate ";
  }

  if (_corrZipCode->isChecked())
  {
    _cust->addColumn(tr("Corr. Zip Code"), 100, Qt::AlignLeft);
    sql += ", cust_corrzipcode ";
  }

  if (_corrCountry->isChecked())
  {
    _cust->addColumn(tr("Corr. Country"), 100, Qt::AlignLeft);
    sql += ", cust_corrcountry ";
  }

  if (_corrContact->isChecked())
  {
    _cust->addColumn(tr("Corr. Contact"), 100, Qt::AlignLeft);
    sql += ", cust_corrcontact ";
  }

  if (_corrPhone->isChecked())
  {
    _cust->addColumn(tr("Corr. Phone"), 100, Qt::AlignLeft);
    sql += ", cust_corrphone ";
  }

  if (_corrFAX->isChecked())
  {
    _cust->addColumn(tr("Corr. FAX"), 100, Qt::AlignLeft);
    sql += ", cust_corrfax ";
  }

  if (_corrEmail->isChecked())
  {
    _cust->addColumn(tr("Corr. Email"), 100, Qt::AlignLeft);
    sql += ", cust_corremail ";
  }

  sql += "FROM cust, custtype "
         "WHERE ( (cust_custtype_id=custtype_id)";

  if (!_showInactive->isChecked())
    sql += " AND (cust_active)";

  if (_customerType->isSelected())
    sql += " AND (custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " AND (custtype_code ~ :custtype_pattern)";
  
  sql += " ) "
         "ORDER BY cust_number;";

  q.prepare(sql);
  _customerType->bindValue(q);
  q.exec();
  _cust->populate(q);
}

