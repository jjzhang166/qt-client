/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include <QGridLayout>
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
  _selectedOnly = false;
  if (_x_preferences)
    if (_x_preferences->boolean("selectedSites"))
      _selectedOnly=true;
  
  if(pName)
    setObjectName(pName);

  _fixed = true;
  QButtonGroup *buttonGroup = new QButtonGroup(this);
  
  _all = new QRadioButton(tr("All Sites"), this);
  _all->setObjectName("_all");
  _site = new QLabel(tr("Site:"),this);
  _site->setObjectName("_site");
  _selected = new QRadioButton(tr("Selected:"), this);
  _selected->setObjectName("_selected");
	 
  if (!_selectedOnly)
  {
    _all->setChecked(true);
    buttonGroup->addButton(_all);
    buttonGroup->addButton(_selected);
  }
  
  _warehouses = new WComboBox(this, "_warehouses");

  _all->setVisible(! _selectedOnly);
  _selected->setVisible(! _selectedOnly);
  _site->setVisible(_selectedOnly);

  setFixedSize(! _selectedOnly);

  connect(_selected, SIGNAL(toggled(bool)), _warehouses, SLOT(setEnabled(bool)));
  connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(updated()));
  connect(_warehouses, SIGNAL(newID(int)), this, SIGNAL(updated()));

  if (((_x_preferences) ? _x_preferences->value("PreferredWarehouse").toInt() : -1) != -1)
    _selected->setChecked(true);

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
  _all->setChecked(true);
}

void WarehouseGroup::setFixedSize(bool pFixed)
{
  qDebug("WarehouseGroup::setFixedSize(%d) entered", pFixed);
  if (layout())
    delete layout();
    
  QGridLayout *gridLayout = new QGridLayout();
  gridLayout->setMargin(0);
  gridLayout->setSpacing(5);
  gridLayout->addWidget(_all,        0, 0);
  gridLayout->addWidget(_selected,   1, 0);
  gridLayout->addWidget(_warehouses, 1, 1);
  gridLayout->setRowStretch(2, 1);
  gridLayout->setColumnStretch(2, 1);
  if (pFixed)
  {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  }
  else
  {
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  }

  setLayout(gridLayout);
  _warehouses->setEnabled(_selected->isChecked());
  
  _fixed = pFixed;
}

void WarehouseGroup::setId(int pId)
{
  _selected->setChecked(true);
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

