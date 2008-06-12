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

#include "archRestoreSalesHistory.h"

#include <qvariant.h>

/*
 *  Constructs a archRestoreSalesHistory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
archRestoreSalesHistory::archRestoreSalesHistory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
archRestoreSalesHistory::~archRestoreSalesHistory()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void archRestoreSalesHistory::languageChange()
{
    retranslateUi(this);
}


#define cArchive 0x01
#define cRestore  0x02

void archRestoreSalesHistory::init()
{
  _customerType->setType(CustomerType);
  _productCategory->setType(ProductCategory);
}

enum SetResponse archRestoreSalesHistory::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("archieve", &valid);
  if (valid)
  {
    _mode = cArchive;

    setCaption(tr("Archive Sales History"));
    _miscItems->setText(tr("Archive Freight, Sales Tax and Misc. Items"));
  }

  param = pParams.value("restore", &valid);
  if (valid)
  {
    _mode = cRestore;

    setCaption(tr("Restore Sales History"));
    _miscItems->setText(tr("Restore Freight, Sales Tax and Misc. Items"));
  }

  return NoError;
}

void archRestoreSalesHistory::sSelect()
{
  QString sql;

  if (_mode == cArchive)
    sql = "SELECT archiveSalesHistory(cohist_id) "
          "FROM cust, custtype,"
          "     cohist LEFT OUTER JOIN"
          "     ( itemsite JOIN"
          "       ( item JOIN prodcat"
          "         ON (item_prodcat_id=prodcat_id) )"
          "       ON (itemsite_item_id=item_id) )"
          "     ON (cohist_itemsite_id=itemsite_id) "
          "WHERE ( (cohist_cust_id=cust_id)"
          " AND (cust_custtype_id=custtype_id)"
          " AND (cohist_invcdate BETWEEN :startDate AND :endDate)";
  else if (_mode == cRestore)
    sql = "SELECT restoreSalesHistory(asohist_id) "
          "FROM cust, custtype,"
          "     asohist LEFT OUTER JOIN"
          "     ( itemsite JOIN"
          "       ( item JOIN prodcat"
          "         ON (item_prodcat_id=prodcat_id) )"
          "       ON (itemsite_item_id=item_id) )"
          "     ON (asohist_itemsite_id=itemsite_id) "
          "WHERE ( (asohist_cust_id=cust_id)"
          " AND (cust_custtype_id=custtype_id)"
          " AND (asohist_invcdate BETWEEN :startDate AND :endDate)";

  if (_miscItems->isChecked())
  {
    if (_warehouse->isSelected())
      sql += " AND ( (itemsite_id IS NULL) OR (itemsite_warehous_id=:warehous_id) )";

    if (_productCategory->isChecked())
      sql += " AND ( (itemsite_id IS NULL) OR (prodcat_id=:prodcat_id) )";
    else if (_productCategory->isPattern())
      sql += " AND ( (itemsite_id IS NULL) OR (prodcat_code ~ :prodcat_pattern) )";
  }
  else
  {
    sql += " AND (itemsite_id IS NOT NULL)";

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    if (_productCategory->isChecked())
      sql += " AND (prodcat_id=:prodcat_id)";
    else if (_productCategory->isPattern())
      sql += " AND (prodcat_code ~ :prodcat_pattern)";

  }

  if (_customerType->isSelected())
    sql += " AND (custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " AND (custtype_code ~ :custtype_pattern)";

  sql += " );";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _customerType->bindValue(q);
  _productCategory->bindValue(q);
  _dates->bindValue(q);
  q.exec();

  accept();
}

