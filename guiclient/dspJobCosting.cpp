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

#include "dspJobCosting.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"

dspJobCosting::dspJobCosting(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _wo->setQuery( "SELECT wo_id, formatWONumber(wo_id) AS wonumber,"
                  "       warehous_code, item_id, item_number, uom_name,"
                  "       item_descrip1, item_descrip2,"
                  "       wo_qtyord, wo_qtyrcv, wo_status,"
                  "       formatDate(wo_duedate) AS duedate,"
                  "       formatDate(wo_startdate) AS startdate,"
                  "       formatQtyPer(wo_qtyord) AS ordered,"
                  "       formatQtyPer(wo_qtyrcv) AS received, "
                  "       formatQtyPer(noNeg(wo_qtyord - wo_qtyrcv)) AS balance,"
                  "       (item_descrip1 || ' ' || item_descrip2) AS descrip,"
                  "       itemsite_warehous_id "
                  "FROM wo, itemsite, item, warehous, uom "
                  "WHERE ((wo_itemsite_id=itemsite_id)"
                  " AND (itemsite_item_id=item_id)"
                  " AND (item_inv_uom_id=uom_id)"
                  " AND (itemsite_warehous_id=warehous_id)"
                  " AND (item_type = 'J')) "
                  "ORDER BY formatWONumber(wo_id) DESC");
  
  QString _codecol;
  if (!_metrics->boolean("Routings"))
  {
    _codecol = tr("Item Number");
    _showsu->hide();
    _showsu->setChecked(FALSE);
    _showrt->hide();
    _showrt->setChecked(FALSE);
    _showmatl->hide();
    _showmatl->setChecked(TRUE);
  }
  else
    _codecol = tr("Work Center/Item");

  _cost->addColumn(tr("Type"), _itemColumn, Qt::AlignLeft,  true, "type");
  _cost->addColumn(_codecol,   _itemColumn, Qt::AlignLeft,  true, "code");
  _cost->addColumn(tr("Description"),   -1, Qt::AlignLeft,  true, "descrip");
  _cost->addColumn(tr("Qty."),  _qtyColumn, Qt::AlignRight, true, "qty");
  _cost->addColumn(tr("UOM"),   _uomColumn, Qt::AlignCenter,true, "uom");
  _cost->addColumn(tr("Cost"),_moneyColumn, Qt::AlignRight, true, "cost");

  _wo->setFocus();
}

dspJobCosting::~dspJobCosting()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspJobCosting::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspJobCosting::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspJobCosting::setParams(ParameterList &params)
{
  if (! _wo->isValid())
  {
   QMessageBox::warning(this, tr("Select Options"),
                        tr("<p>You must select a Work Order."));
    _wo->setFocus();
    return false;
  }
 if (!_showsu->isChecked() && !_showrt->isChecked() && !_showmatl->isChecked())
 {
   QMessageBox::warning(this, tr("Select Options"),
                        tr("<p>You must select one or more of the options to "
                           "show Set Up, Run Time, and/or Materials."));
   _showsu->setFocus();
   return false;
 }

  params.append("wo_id", _wo->id());
  params.append("setup", tr("Setup"));
  params.append("runtime", tr("Run Time"));
  params.append("material", tr("Material"));
  params.append("timeuom", tr("Hours"));

  if (_showsu->isChecked())
    params.append("showsu");

  if (_showrt->isChecked())
    params.append("showrt");

  if (_showmatl->isChecked())
    params.append("showmatl");

  return true;
}

void dspJobCosting::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("JobCosting", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspJobCosting::sFillList()
{
  if (_wo->isValid())
    sFillList(-1, FALSE);
}

void dspJobCosting::sFillList(int, bool)
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("manufacture", "jobcosting");
  q = mql.toQuery(params);
  _cost->populate(q, TRUE);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
