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

#include "createCycleCountTags.h"

#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

createCycleCountTags::createCycleCountTags(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _codeGroup = new QButtonGroup(this);
    _codeGroup->addButton(_plancode);
    _codeGroup->addButton(_classcode);

    connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
    connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLocations()));
    connect(_codeGroup, SIGNAL(buttonClicked(int)), this, SLOT(sParameterTypeChanged()));

    _parameter->setType(ClassCode);

    _freeze->setEnabled(_privileges->check("CreateReceiptTrans"));
    
    //If not multi-warehouse hide whs control
    if (!_metrics->boolean("MultiWhs"))
    {
      _warehouseLit->hide();
      _warehouse->hide();
    }

    if (_preferences->boolean("XCheckBox/forgetful"))
      _priority->setChecked(true);

    sPopulateLocations();
}

createCycleCountTags::~createCycleCountTags()
{
    // no need to delete child widgets, Qt does it all for us
}

void createCycleCountTags::languageChange()
{
    retranslateUi(this);
}

enum SetResponse createCycleCountTags::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("maxTags", &valid);
  if (valid)
    _maxTags->setValue(param.toInt());

  _priority->setChecked(pParams.inList("priority"));
  _freeze->setChecked(pParams.inList("freeze"));

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("classcode_id", &valid);
    _parameter->setId(param.toInt());

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
    _parameter->setPattern(param.toString());

  param = pParams.value("comments", &valid);
  if (valid)
    _comments->setText(param.toString());

  if (pParams.inList("run"))
    sCreate();

  return NoError;
}

void createCycleCountTags::sCreate()
{
  QString fname;
  if ((_parameter->type() == ClassCode) && _parameter->isSelected())
  {
    q.prepare("SELECT createCycleCountsByWarehouseByClassCode(:warehous_id, :classcode_id, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByClassCode";
  }
  else if ((_parameter->type() == ClassCode) && _parameter->isPattern())
  {
    q.prepare("SELECT createCycleCountsByWarehouseByClassCode(:warehous_id, :classcode_pattern, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByClassCode";
  }
  else if ((_parameter->type() == PlannerCode) && _parameter->isSelected())
  {
    q.prepare("SELECT createCycleCountsByWarehouseByPlannerCode(:warehous_id, :plancode_id, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByPlannerCode";
  }
  else if ((_parameter->type() == PlannerCode) && _parameter->isPattern())
  {
    q.prepare("SELECT createCycleCountsByWarehouseByPlannerCode(:warehous_id, :plancode_pattern, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByPlannerCode";
  }
  else //if (_parameter->isAll())
  {
    q.prepare("SELECT createCycleCountsByWarehouse(:warehous_id, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouse";
  }

  _parameter->bindValue(q);
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":maxTags", _maxTags->value());
  q.bindValue(":comments", _comments->text());
  q.bindValue(":priority", QVariant(_priority->isChecked(), 0));
  q.bindValue(":freeze", QVariant(_freeze->isChecked(), 0));
  q.bindValue(":ignore", QVariant(_ignoreZeroBalance->isChecked(), 0));
  if(_byLocation->isChecked())
    q.bindValue(":location_id", _location->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup(fname, result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void createCycleCountTags::sPopulateLocations()
{
  q.prepare( "SELECT location_id, "
             "       CASE WHEN (LENGTH(location_descrip) > 0) THEN (formatLocationName(location_id) || '-' || location_descrip)"
             "            ELSE formatLocationName(location_id)"
             "       END AS locationname "
             "FROM location "
             "WHERE (location_warehous_id=:warehous_id) "
             "ORDER BY locationname;" );
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  _location->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void createCycleCountTags::sParameterTypeChanged()
{
  if(_plancode->isChecked())
    _parameter->setType(PlannerCode);
  else //if(_classcode->isChecked())
    _parameter->setType(ClassCode);
}
