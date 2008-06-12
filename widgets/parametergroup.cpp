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


#include <QRadioButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>

#include <xsqlquery.h>
#include <parameter.h>

#include "xcombobox.h"

#include "parametergroup.h"

ParameterGroup::ParameterGroup(QWidget *pParent, const char *pName) :
  QGroupBox(pParent)
{
  if(pName)
    setObjectName(pName);

  _type = AdhocGroup;

  setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

  QWidget *selectedGroup = new QWidget(this);
  QWidget *patternGroup = new QWidget(this);

  _all= new QRadioButton(QObject::tr("All"), this, "_all");
  _all->setChecked(TRUE);

  _selected = new QRadioButton(tr("Selected:"), selectedGroup, "_selected");

  _items = new XComboBox(FALSE, selectedGroup, "_items");
  _items->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  _items->setEnabled(FALSE);

  QHBoxLayout * hLayout = new QHBoxLayout(selectedGroup);
  hLayout->setSpacing(5);
  hLayout->setMargin(0);
  hLayout->addWidget(_selected);
  hLayout->addWidget(_items);
  selectedGroup->setLayout(hLayout);
  
  _usePattern = new QRadioButton(tr("Pattern:"), patternGroup, "_usePattern");

  _pattern = new QLineEdit(patternGroup, "_pattern");
  _pattern->setEnabled(FALSE);

  hLayout = new QHBoxLayout(patternGroup);
  hLayout->setSpacing(5);
  hLayout->setMargin(0);
  hLayout->addWidget(_usePattern);
  hLayout->addWidget(_pattern);
  patternGroup->setLayout(hLayout);

  QVBoxLayout * vLayout = new QVBoxLayout(this);
  vLayout->setSpacing(0);
  vLayout->setMargin(5);
  vLayout->addWidget(_all);
  vLayout->addWidget(selectedGroup);
  vLayout->addWidget(patternGroup);
  setLayout(vLayout);
  
  QButtonGroup * buttonGroup = new QButtonGroup(this);
  buttonGroup->addButton(_all);
  buttonGroup->addButton(_selected);
  buttonGroup->addButton(_usePattern);

  connect(_selected, SIGNAL(toggled(bool)), _items, SLOT(setEnabled(bool)));
  connect(_usePattern, SIGNAL(toggled(bool)), _pattern, SLOT(setEnabled(bool)));
  connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(updated()));
  connect(_items, SIGNAL(newID(int)), this, SIGNAL(updated()));
  connect(_pattern, SIGNAL(lostFocus()), this, SIGNAL(updated()));

  setFocusProxy(_all);
}

void ParameterGroup::setType(enum ParameterGroupTypes pType)
{
  if (_type == pType)
    return;

  _type = pType;

  switch (_type)
  {
    case AdhocGroup:
      _all->setText(QObject::tr("All"));
      break;

    case ClassCode:
      _all->setText(QObject::tr("All Class Codes"));
      _items->setType(XComboBox::ClassCodes);
      break;

    case PlannerCode:
      _all->setText(QObject::tr("All Planner Codes"));
      _items->setType(XComboBox::PlannerCodes);
      break;

    case ProductCategory:
      _all->setText(QObject::tr("All Product Categories"));
      _items->setType(XComboBox::ProductCategories);
      break;

    case ItemGroup:
      _all->setText(QObject::tr("All Item Groups"));
      _items->setType(XComboBox::ItemGroups);
      break;

    case CostCategory:
      _all->setText(QObject::tr("All Cost Categories"));
      _items->setType(XComboBox::CostCategories);
      break;

    case CustomerType:
      _all->setText(QObject::tr("All Customer Types"));
      _items->setType(XComboBox::CustomerTypes);
      break;

    case CustomerGroup:
      _all->setText(QObject::tr("All Customer Groups"));
      _items->setType(XComboBox::CustomerGroups);
      break;

    case CurrencyNotBase:
      _all->setText(QObject::tr("All Foreign Currencies"));
      _items->setType(XComboBox::CurrenciesNotBase);
      if (_items->isHidden()) // if there's only one currency defined
        hide();
      break;

    case Currency:
      _all->setText(QObject::tr("All Currencies"));
      _items->setType(XComboBox::Currencies);
      if (_items->isHidden()) // if there's only one currency defined
        hide();
      break;

    case WorkCenter:
      _all->setText(QObject::tr("All Work Centers"));
      _items->setType(XComboBox::WorkCenters);
      break;

    case User:
      _all->setText(QObject::tr("All Users"));
      _items->setType(XComboBox::Users);
  }
}

void ParameterGroup::setId(int pId)
{
  _selected->setChecked(TRUE);
  _items->setId(pId);
}

void ParameterGroup::setPattern(const QString &pPattern)
{
  _usePattern->setChecked(TRUE);
  _pattern->setText(pPattern);
}

void ParameterGroup::setState(enum ParameterGroupStates pState)
{
  switch(pState)
  {
    case Selected:
      _selected->setChecked(true);
      break;
    case Pattern:
      _usePattern->setChecked(true);
      break;
    case All:
    default:
      _all->setChecked(true);
  }
}

void ParameterGroup::appendValue(ParameterList &pParams)
{
  if (_selected->isChecked())
  {
    if (_type == ClassCode)
      pParams.append("classcode_id", _items->id());
    else if (_type == PlannerCode)
      pParams.append("plancode_id", _items->id());
    else if (_type == ProductCategory)
      pParams.append("prodcat_id", _items->id());
    else if (_type == ItemGroup)
      pParams.append("itemgrp_id", _items->id());
    else if (_type == CostCategory)
      pParams.append("costcat_id", _items->id());
    else if (_type == CustomerType)
      pParams.append("custtype_id", _items->id());
    else if (_type == CustomerGroup)
      pParams.append("custgrp_id", _items->id());
    else if (_type == CurrencyNotBase || _type == Currency)
      pParams.append("curr_id", _items->id());
    else if (_type == WorkCenter)
      pParams.append("wrkcnt_id", _items->id());
    else if (_type == User)
      pParams.append("usr_id", _items->id());
  }
  else if (_usePattern->isChecked())
  {
    if (_type == ClassCode)
      pParams.append("classcode_pattern", _pattern->text());
    else if (_type == PlannerCode)
      pParams.append("plancode_pattern", _pattern->text());
    else if (_type == ProductCategory)
      pParams.append("prodcat_pattern", _pattern->text());
    else if (_type == ItemGroup)
      pParams.append("itemgrp_pattern", _pattern->text());
    else if (_type == CostCategory)
      pParams.append("costcat_pattern", _pattern->text());
    else if (_type == CustomerType)
      pParams.append("custtype_pattern", _pattern->text());
    else if (_type == CustomerGroup)
      pParams.append("custgrp_pattern", _pattern->text());
    else if (_type == CurrencyNotBase || _type == Currency)
      pParams.append("currConcat_pattern", _pattern->text());
    else if (_type == WorkCenter)
      pParams.append("wrkcnt_pattern", _pattern->text());
    else if (_type == User)
      pParams.append("usr_pattern", _pattern->text());
  }
}

void ParameterGroup::repopulateSelected()
{
    _items->populate();
}

void ParameterGroup::bindValue(XSqlQuery &pQuery)
{
  if (_selected->isChecked())
  {
    QString name;

    switch (_type)
    {
      case ClassCode:
        name = ":classcode_id";   break;

      case PlannerCode:
        name = ":plancode_id";    break;

      case ProductCategory:
        name = ":prodcat_id";     break;

      case ItemGroup:
        name = ":itemgrp_id";     break;

      case CostCategory:
        name = ":costcat_id";     break;

      case CustomerType:
        name = ":custtype_id";    break;

      case CustomerGroup:
        name = ":custgrp_id";     break;

      case CurrencyNotBase:
      case Currency:
        name = ":curr_id";        break;

      case WorkCenter:
        name = ":wrkcnt_id";      break;

      case User:
        name = ":usr_id";         break;

      case AdhocGroup:
        name = ":id";             break;

      default:
        break;
    }

    pQuery.bindValue(name, _items->id());
  }
  else if (_usePattern->isChecked())
  {
    QString name;

    switch (_type)
    {
      case ClassCode:
        name = ":classcode_pattern";   break;

      case PlannerCode:
        name = ":plancode_pattern";    break;

      case ProductCategory:
        name = ":prodcat_pattern";     break;

      case ItemGroup:
        name = ":itemgrp_pattern";     break;

      case CostCategory:
        name = ":costcat_pattern";     break;

      case CustomerType:
        name = ":custtype_pattern";    break;

      case CustomerGroup:
        name = ":custgrp_pattern";     break;

      case CurrencyNotBase:
      case Currency:
        name = ":currConcat_pattern";  break;

      case WorkCenter:
        name = ":wrkcnt_pattern";      break;

      case User:
        name = ":usr_pattern";         break;

      case AdhocGroup:
        name = ":pattern";             break;

      default:
        break;
    }

    pQuery.bindValue(name, _pattern->text());
  }
}

enum ParameterGroupStates ParameterGroup::state()
{
  if (_all->isChecked())
    return All;
  else if (_selected->isChecked())
    return Selected;
  else
    return Pattern;
}

int ParameterGroup::id()
{
  if (_selected->isChecked())
    return _items->id();
  else
    return -1;
}

QString ParameterGroup::pattern()
{
  if (_usePattern->isChecked())
    return _pattern->text();
  else
    return QString::null;
}

