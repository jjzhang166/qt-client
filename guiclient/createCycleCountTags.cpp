/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createCycleCountTags.h"

#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

createCycleCountTags::createCycleCountTags(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _codeGroup = new QButtonGroup(this);
    _codeGroup->addButton(_plancode);
    _codeGroup->addButton(_classcode);

    connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
    connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLocations()));
    connect(_codeGroup, SIGNAL(buttonClicked(int)), this, SLOT(sParameterTypeChanged()));

    _parameter->setType(ParameterGroup::ClassCode);

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
  XDialog::set(pParams);
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
  XSqlQuery createCreate;
  QString fname;
  if ((_parameter->type() == ParameterGroup::ClassCode) && _parameter->isSelected())
  {
    createCreate.prepare("SELECT createCycleCountsByWarehouseByClassCode(:warehous_id, :classcode_id, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByClassCode";
  }
  else if ((_parameter->type() == ParameterGroup::ClassCode) && _parameter->isPattern())
  {
    createCreate.prepare("SELECT createCycleCountsByWarehouseByClassCode(:warehous_id, :classcode_pattern, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByClassCode";
  }
  else if ((_parameter->type() == ParameterGroup::PlannerCode) && _parameter->isSelected())
  {
    createCreate.prepare("SELECT createCycleCountsByWarehouseByPlannerCode(:warehous_id, :plancode_id, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByPlannerCode";
  }
  else if ((_parameter->type() == ParameterGroup::PlannerCode) && _parameter->isPattern())
  {
    createCreate.prepare("SELECT createCycleCountsByWarehouseByPlannerCode(:warehous_id, :plancode_pattern, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByPlannerCode";
  }
  else //if (_parameter->isAll())
  {
    createCreate.prepare("SELECT createCycleCountsByWarehouse(:warehous_id, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouse";
  }

  _parameter->bindValue(createCreate);
  createCreate.bindValue(":warehous_id", _warehouse->id());
  createCreate.bindValue(":maxTags", _maxTags->value());
  createCreate.bindValue(":comments", _comments->toPlainText());
  createCreate.bindValue(":priority", QVariant(_priority->isChecked()));
  createCreate.bindValue(":freeze",   QVariant(_freeze->isChecked()));
  createCreate.bindValue(":ignore",   QVariant(_ignoreZeroBalance->isChecked()));
  if(_byLocation->isChecked())
    createCreate.bindValue(":location_id", _location->id());
  createCreate.exec();
  if (createCreate.first())
  {
    int result = createCreate.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup(fname, result), __FILE__, __LINE__);
      return;
    }
  }
  else if (createCreate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, createCreate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void createCycleCountTags::sPopulateLocations()
{
  XSqlQuery createPopulateLocations;
  createPopulateLocations.prepare( "SELECT location_id, "
             "       CASE WHEN (LENGTH(location_descrip) > 0) THEN (formatLocationName(location_id) || '-' || location_descrip)"
             "            ELSE formatLocationName(location_id)"
             "       END AS locationname "
             "FROM location "
             "WHERE (location_warehous_id=:warehous_id) "
             "ORDER BY locationname;" );
  createPopulateLocations.bindValue(":warehous_id", _warehouse->id());
  createPopulateLocations.exec();
  _location->populate(createPopulateLocations);
  if (createPopulateLocations.lastError().type() != QSqlError::NoError)
  {
    systemError(this, createPopulateLocations.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void createCycleCountTags::sParameterTypeChanged()
{
  if(_plancode->isChecked())
    _parameter->setType(ParameterGroup::PlannerCode);
  else //if(_classcode->isChecked())
    _parameter->setType(ParameterGroup::ClassCode);
}
