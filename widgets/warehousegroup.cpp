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


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>

#include <xsqlquery.h>
#include <parameter.h>

#include "warehouseCluster.h"
#include "warehousegroup.h"

WarehouseGroup::WarehouseGroup(QWidget *pParent, const char *pName) :
  QGroupBox(pParent)
{
  if(pName)
    setObjectName(pName);

  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QWidget * selectedGroup = new QWidget(this);
  QButtonGroup * buttonGroup = new QButtonGroup(this);

  _all = new QRadioButton(tr("All Warehouses"), this, "_all");
  _all->setChecked(TRUE);
  buttonGroup->addButton(_all);

  _selected = new QRadioButton(tr("Selected:"), selectedGroup, "_selected");
  buttonGroup->addButton(_selected);

  _warehouses = new WComboBox(selectedGroup, "_warehouses");
  _warehouses->setEnabled(FALSE);

  QHBoxLayout *hLayout = new QHBoxLayout(selectedGroup);
  hLayout->setMargin(0);
  hLayout->setSpacing(5);
  hLayout->addWidget(_selected);
  hLayout->addWidget(_warehouses);
  selectedGroup->setLayout(hLayout);

  QVBoxLayout *vLayout = new QVBoxLayout(this);
  vLayout->setMargin(5);
  vLayout->setSpacing(0);
  vLayout->addWidget(_all);
  vLayout->addWidget(selectedGroup);
  setLayout(vLayout);

  connect(_selected, SIGNAL(toggled(bool)), _warehouses, SLOT(setEnabled(bool)));
  connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(updated()));
  connect(_warehouses, SIGNAL(newID(int)), this, SIGNAL(updated()));

  if (((_x_preferences) ? _x_preferences->value("PreferredWarehouse").toInt() : -1) != -1)
    _selected->setChecked(TRUE);

  setTabOrder(_all, _selected);
  setTabOrder(_selected, _warehouses);
  setTabOrder(_warehouses, _all);
  setFocusProxy(_all);
  
  if (_x_metrics)
  {
    if (!_x_metrics->boolean("MultiWhs"))
    {
      this->hide();
      setAll();
    }
  }
}

void WarehouseGroup::setAll()
{
  _all->setChecked(TRUE);
}

void WarehouseGroup::setId(int pId)
{
  _selected->setChecked(TRUE);
  _warehouses->setId(pId);
}

void WarehouseGroup::appendValue(ParameterList &pParams)
{
  if (_selected->isChecked())
    pParams.append("warehous_id", _warehouses->id());
}

void WarehouseGroup::bindValue(XSqlQuery &pQuery)
{
  if (_selected->isChecked())
    pQuery.bindValue(":warehous_id", _warehouses->id());
}

void WarehouseGroup::findItemSites(int pItemid)
{
  _warehouses->findItemsites(pItemid);
}

int WarehouseGroup::id()
{
  if (_selected->isChecked())
    return _warehouses->id();
  else
    return -1;
}

bool WarehouseGroup::isAll() const
{
  return _all->isChecked();
}

bool WarehouseGroup::isSelected() const
{
  return _selected->isChecked();
}

