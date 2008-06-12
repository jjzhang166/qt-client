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

#include "printStatementsByCustomerType.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

printStatementsByCustomerType::printStatementsByCustomerType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _captive = FALSE;

  _customerTypes->setType(CustomerType);

  if (_preferences->boolean("XCheckBox/forgetful"))
    _dueonly->setChecked(true);
}

printStatementsByCustomerType::~printStatementsByCustomerType()
{
  // no need to delete child widgets, Qt does it all for us
}

void printStatementsByCustomerType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printStatementsByCustomerType::set(const ParameterList &pParams)
{
  QVariant param;

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void printStatementsByCustomerType::sPrint()
{
  QString sql( "SELECT cust_id, (cust_number || '-' || cust_name) AS customer,"
               "       findCustomerForm(cust_id, 'S') AS reportname "
               "FROM cust, custtype, aropen "
               "WHERE ( (cust_custtype_id=custtype_id)"
               " AND (aropen_cust_id=cust_id)"
               " AND (aropen_open)" );

  if (_dueonly->isChecked())
    sql += " AND (aropen_duedate < (CURRENT_DATE - :graceDays))";
  if (_customerTypes->isSelected())
    sql += " AND (custtype_id=:custtype_id)";
  else if (_customerTypes->isPattern())
    sql += " AND (custtype_code ~ :custtype_pattern)";

  sql += ") "
         "GROUP BY cust_id, cust_number, cust_name "
         "HAVING (SUM((aropen_amount - aropen_paid) * CASE WHEN (aropen_doctype IN ('C', 'R')) THEN -1 ELSE 1 END) > 0) "
         "ORDER BY cust_number;";

  q.prepare(sql);
  _customerTypes->bindValue(q);
  q.bindValue(":graceDays", _graceDays->value());
  q.exec();
  if (q.first())
  {
    QPrinter printer(QPrinter::HighResolution);
    bool userCanceled = false;
    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }
    bool doSetup = true;
    do
    {
      message( tr("Printing Statement for Customer %1.")
               .arg(q.value("customer").toString()) );

      ParameterList params;
      params.append("cust_id", q.value("cust_id").toInt());

      orReport report(q.value("reportname").toString(), params);
      if (! (report.isValid() && report.print(&printer, doSetup)) )
      {
        report.reportError(this);
	orReport::endMultiPrint(&printer);
        reject();
      }
      doSetup = false;
    }
    while (q.next());
    orReport::endMultiPrint(&printer);

    message("");
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    QMessageBox::information( this, tr("No Statement to Print"),
                              tr("There are no Customers whose accounts are past due for which Statements should be printed.") );

  if (_captive)
    accept();
  else
    _close->setText(tr("&Close"));
}
