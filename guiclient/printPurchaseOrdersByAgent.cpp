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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "printPurchaseOrdersByAgent.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a printPurchaseOrdersByAgent as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printPurchaseOrdersByAgent::printPurchaseOrdersByAgent(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_internalCopy, SIGNAL(toggled(bool)), _numOfCopies, SLOT(setEnabled(bool)));

    _agent->setType(XComboBox::Agent);
    _agent->setText(omfgThis->username());

    _vendorCopy->setChecked(_metrics->boolean("POVendor"));

    if (_metrics->value("POInternal").toInt() > 0)
    {
      _internalCopy->setChecked(TRUE);
      _numOfCopies->setValue(_metrics->value("POInternal").toInt());
    }
    else
    {
      _internalCopy->setChecked(FALSE);
      _numOfCopies->setEnabled(FALSE);
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
printPurchaseOrdersByAgent::~printPurchaseOrdersByAgent()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printPurchaseOrdersByAgent::languageChange()
{
    retranslateUi(this);
}

void printPurchaseOrdersByAgent::sPrint()
{
  XSqlQuery markprinted;
  markprinted.prepare("UPDATE pohead"
                      "   SET pohead_printed=true"
                      " WHERE (pohead_id=:pohead_id);");
  XSqlQuery pohead;
  pohead.prepare( "SELECT pohead_id "
                  "FROM pohead "
                  "WHERE ( (pohead_agent_username=:username)"
                  " AND (pohead_status='O')"
                  " AND (NOT pohead_printed)"
                  " AND (pohead_saved) );" );
  pohead.bindValue(":username", _agent->currentText());
  pohead.exec();
  if (pohead.first())
  {
    QPrinter  *printer = new QPrinter(QPrinter::HighResolution);
    bool      setupPrinter = TRUE;

    bool userCanceled = false;
    if (orReport::beginMultiPrint(printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }

    do
    {
      if (_vendorCopy->isChecked())
      {
        ParameterList params;
        params.append("pohead_id", pohead.value("pohead_id").toInt());
        params.append("title", "Vendor Copy");

        orReport report("PurchaseOrder", params);
        if (report.isValid() && report.print(printer, setupPrinter))
	  setupPrinter = FALSE;
	else
	{
          report.reportError(this);
	  orReport::endMultiPrint(printer);
	  return;
	}
      }

      if (_internalCopy->isChecked())
      {
        for (int counter = _numOfCopies->value(); counter; counter--)
        {
          ParameterList params;
          params.append("pohead_id", pohead.value("pohead_id"));
          params.append("title", QString("Internal Copy #%1").arg(counter));

          orReport report("PurchaseOrder", params);
          if (report.isValid() && report.print(printer, setupPrinter))
	    setupPrinter = FALSE;
	  else
	  {
            report.reportError(this);
	    orReport::endMultiPrint(printer);
	    return;
	  }
        }
      }
      markprinted.bindValue(":pohead_id", pohead.value("pohead_id").toInt());
      markprinted.exec();
      if (markprinted.lastError().type() != QSqlError::None)
      {
	systemError(this, markprinted.lastError().databaseText(), __FILE__, __LINE__);
	orReport::endMultiPrint(printer);
	return;
      }
    }
    while (pohead.next());
    orReport::endMultiPrint(printer);
  }
  else if (pohead.lastError().type() != QSqlError::None)
  {
    systemError(this, pohead.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    QMessageBox::information( this, tr("No Purchase Orders to Print"),
                              tr("There are no unprinted Purchase Orders entered by the selected Purchasing Agent to print.") );

  accept();
}

