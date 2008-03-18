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

#include "printWoTraveler.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include "inputManager.h"

/*
 *  Constructs a printWoTraveler as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printWoTraveler::printWoTraveler(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_wo, SIGNAL(valid(bool)), this, SLOT(sHandlePrintButton()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_wo, SIGNAL(newId(int)), this, SLOT(sHandleOptions(int)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_pickList, SIGNAL(toggled(bool)), this, SLOT(sHandlePrintButton()));
    connect(_routing, SIGNAL(toggled(bool)), this, SLOT(sHandlePrintButton()));
    connect(_packingList, SIGNAL(toggled(bool)), this, SLOT(sHandlePrintButton()));
    connect(_woLabel, SIGNAL(toggled(bool)), this, SLOT(sHandlePrintButton()));

    _captive = FALSE;

    omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

    Preferences _pref = Preferences(omfgThis->username());
    if (_pref.boolean("XCheckBox/forgetful"))
	{
      _pickList->setChecked(true);
	  _routing->setChecked(true);
	}

    _wo->setType(cWoExploded | cWoReleased | cWoIssued);

    if (!_privleges->check("ReleaseWorkOrders"))
      _releaseWo->setEnabled(FALSE);
      
    if (!_metrics->boolean("Routings"))
    {
      _routing->setChecked(FALSE);
      _routing->hide();
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
printWoTraveler::~printWoTraveler()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printWoTraveler::languageChange()
{
    retranslateUi(this);
}

enum SetResponse printWoTraveler::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _releaseWo->setFocus();
  }

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void printWoTraveler::sHandleOptions(int pWoid)
{
  if (pWoid != -1)
  {
    XSqlQuery check;

    check.prepare( "SELECT womatl_id "
                   "FROM womatl "
                   "WHERE (womatl_wo_id=:wo_id) "
                   "LIMIT 1;" );
    check.bindValue(":wo_id", pWoid);
    check.exec();
    if (check.lastError().type() != QSqlError::None)
    {
      systemError(this, check.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _pickList->setEnabled(check.first());
    _pickList->setChecked(check.first());
  
    if (_metrics->boolean("Routings"))
    {
      check.prepare( "SELECT wooper_id "
                     "FROM wooper "
                     "WHERE (wooper_wo_id=:wo_id) "
                     "LIMIT 1;" );
      check.bindValue(":wo_id", pWoid);
      check.exec();
      if (check.lastError().type() != QSqlError::None)
      {
        systemError(this, check.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      _routing->setEnabled(check.first());
      _routing->setChecked(check.first());
    }

    check.prepare( "SELECT wo_id "
                   "FROM wo "
                   "WHERE ( (wo_ordtype='S')"
                   " AND (wo_id=:wo_id) );" );
    check.bindValue(":wo_id", pWoid);
    check.exec();
    if (check.lastError().type() != QSqlError::None)
    {
      systemError(this, check.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _packingList->setEnabled(check.first());
    _packingList->setChecked(check.first()); 
    
  }
}

void printWoTraveler::sPrint()
{
  QPrinter  printer(QPrinter::HighResolution);
  bool      setupPrinter = TRUE;
  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("Could not initialize printing system for multiple reports."));
    return;
  }

  if (_pickList->isChecked())
  {
    ParameterList params;
    params.append("wo_id", _wo->id());

    orReport report("PickList", params);

    if (report.isValid() && report.print(&printer, setupPrinter))
      setupPrinter = FALSE;
    else
    {
      report.reportError(this);
      orReport::endMultiPrint(&printer);
      return;
    }
  }

  if (_routing->isChecked())
  {
    ParameterList params;
    params.append("wo_id", _wo->id());

    orReport report("Routing", params);

    if (report.isValid() && report.print(&printer, setupPrinter))
      setupPrinter = FALSE;
    else
    {
      report.reportError(this);
      orReport::endMultiPrint(&printer);
      return;
    }
  }

  if (_woLabel->isChecked())
  {
    XSqlQuery query;
    query.prepare( "SELECT wo_id, CAST(wo_qtyord AS INTEGER) AS wo_qtyord_int "
                   "FROM wo "
                   " WHERE (wo_id=:wo_id);" );
    query.bindValue(":wo_id", _wo->id());
    query.exec();
    if (query.first())
    {
      ParameterList params;
      params.append("wo_id", query.value("wo_id"));
      params.append("labelTo", query.value("wo_qtyord_int"));

      orReport report("WOLabel", params);
      if (report.isValid() && report.print(&printer, setupPrinter))
	setupPrinter = FALSE;
      else
      {
	report.reportError(this);
	orReport::endMultiPrint(&printer);
	return;
      }
    }
    else if (query.lastError().type() != QSqlError::None)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_packingList->isChecked())
  {
    XSqlQuery query;
    query.prepare( "SELECT cohead_id, findCustomerForm(cohead_cust_id, 'L') AS reportname "
                   "FROM cohead, coitem, wo "
                   "WHERE ( (coitem_cohead_id=cohead_id)"
                   " AND (wo_ordid=coitem_id)"
                   " AND (wo_ordtype='S')"
                   " AND (wo_id=:wo_id) );" );
    query.bindValue(":wo_id", _wo->id());
    query.exec();
    if (query.first())
    {
      ParameterList params;
      params.append("sohead_id", query.value("cohead_id"));
      params.append("head_id",  query.value("cohead_id"));
      params.append("head_type",  "SO");
      if (_metrics->boolean("MultiWhs"))
	params.append("MultiWhs");

      orReport report(query.value("reportname").toString(), params);
      if (report.isValid() && report.print(&printer, setupPrinter))
	setupPrinter = FALSE;
      else
      {
	report.reportError(this);
	orReport::endMultiPrint(&printer);
	return;
      }
    }
    else if (query.lastError().type() != QSqlError::None)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  orReport::endMultiPrint(&printer);

  if (_releaseWo->isChecked())
  {
    XSqlQuery release;
    release.prepare("SELECT releaseWo(:wo_id, FALSE);");
    release.bindValue(":wo_id", _wo->id());
    release.exec();
    if (release.lastError().type() != QSqlError::None)
    {
      systemError(this, release.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
  }

  if (_captive)
    close();
  else
  {
    _wo->setId(-1);
    _wo->setFocus();
  }
}

void printWoTraveler::sHandlePrintButton()
{
  _print->setEnabled( (_wo->isValid()) &&
                      ( (_pickList->isChecked()) || (_routing->isChecked()) || (_packingList->isChecked())  || (_woLabel->isChecked())) );
}
