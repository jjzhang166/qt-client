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


#include <qlayout.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <xsqlquery.h>
#include <parameter.h>

#include "xcombobox.h"

#include "configuregroup.h"

ConfigureGroup::ConfigureGroup(QWidget *pParent, const char *pName) :
  QButtonGroup(pParent, pName)
{
  _type = None;
  _itemid = -1;

  setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed, 0, 0, sizePolicy().hasHeightForWidth()));
  setColumnLayout(0, Qt::Vertical );
  layout()->setSpacing(0);
  layout()->setMargin(5);

  _configGroup = new QHBoxLayout(0, 0, 7, "configGroup"); 
  _configGroup->setEnabled(FALSE);

  QVBoxLayout *mainGroup = new QVBoxLayout(layout());
  QHBoxLayout *optionGroup = new QHBoxLayout(0, 0, 5, "optionGroup"); 
  QVBoxLayout *unspacedFlagsGroup = new QVBoxLayout(0, 0, 0, "unspacedFlagsGroup"); 
  QHBoxLayout *attributeGroup = new QHBoxLayout( 0, 0, 5, "attributeGroup"); 
  QHBoxLayout *flagsGroup = new QHBoxLayout(0, 0, 0, "flagsGroup"); 

  _everyConfig = new QRadioButton(tr("Used in Every Configuration"), this, "_everyConfig");
  _everyConfig->setChecked(TRUE);
  mainGroup->addWidget(_everyConfig);

  _exclusiveUse = new QRadioButton(tr("Used only in Option:"), this, "_exclusiveUse");
  optionGroup->addWidget(_exclusiveUse);

  _option = new XComboBox(FALSE, this, "_option" );
  _option->setEnabled(FALSE);
  optionGroup->addWidget(_option);

  _configGroup->addLayout(optionGroup);

  _adjustQuantity = new QCheckBox(tr("Adjust Qty. Per per Config. Qty."), this, "_adjustQuantity");
  _adjustQuantity->setEnabled(FALSE);
  unspacedFlagsGroup->addWidget(_adjustQuantity);

  _attributeLit = new QLabel(tr("Attribute:"), this, "_attributeLit");
  _attributeLit->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));
  attributeGroup->addWidget(_attributeLit);

  _attribute = new XComboBox(FALSE, this, "_attribute");
  _attribute->setEnabled(FALSE);
  attributeGroup->addWidget(_attribute);

  unspacedFlagsGroup->addLayout(attributeGroup);

  flagsGroup->addLayout(unspacedFlagsGroup);
  QSpacerItem *flagsSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  flagsGroup->addItem(flagsSpacer);

  _configGroup->addLayout(flagsGroup);
  mainGroup->addLayout(_configGroup);

  _adjustQuantity->hide();

  connect(this, SIGNAL(clicked(int)), this, SLOT(internal()));
  connect(_option, SIGNAL(newID(int)), this, SLOT(internal()));
  connect(_exclusiveUse, SIGNAL(toggled(bool)), _option, SLOT(setEnabled(bool)));

  setFocusProxy(_everyConfig);
}

QString ConfigureGroup::stype()
{
  if (_type == Option)
    return "O";
  if (_type == Attribute)
    return "A";
  if (_type == Quantity)
    return "Q";
  return "N";
}

ConfigureGroupType ConfigureGroup::type()
{
  return _type;
}

int ConfigureGroup::id()
{
  if ( (_type == Option) || (_type == Quantity) )
    return _option->id();
  else if (_type == Attribute)
    return _attribute->id();
  else
    return -1;
}

bool ConfigureGroup::flag()
{
  if (_type == Quantity)
    return _adjustQuantity->isChecked();
  else
    return FALSE;
}

void ConfigureGroup::setItemid(int pItemid)
{
  if (pItemid != -1)
  {
    XSqlQuery query;
    query.prepare( "SELECT item_config "
                   "FROM item "
                   "WHERE (item_id=:item_id);" );
    query.bindValue(":item_id", pItemid);
    query.exec();
    query.first();
    if (query.value("item_config").toBool())
    {
      _configGroup->setEnabled(TRUE);
 
      query.prepare( "SELECT itemopn_id, char_name "
                     "FROM itemopn, char "
                     "WHERE ( (itemopn_char_id=char_id)"
                     " AND (itemopn_item_id=:item_id) ) "
                     "ORDER BY char_name;" );
      query.bindValue(":item_id", pItemid);
      query.exec();
      _option->populate(query);

      return;
    }
  }

  _configGroup->setEnabled(FALSE);
  _option->clear();
  _attribute->clear();
}

void ConfigureGroup::setType(QString pType)
{
  if (pType == "O")
    setType(Option);
  else if (pType == "A")
    setType(Attribute);
  else if (pType == "Q")
    setType(Quantity);
  else
    setType(None);
}

void ConfigureGroup::setType(ConfigureGroupType pType)
{
  _type = pType;

  if (_type == None)
    _everyConfig->setChecked(TRUE);
  else
  {
    _exclusiveUse->setChecked(TRUE);

    if (_type == Option)
    {
      _adjustQuantity->hide();
      _attributeLit->hide();
      _attribute->hide();
    }
    else if (_type == Attribute)
    {
      _adjustQuantity->hide();
      _attributeLit->show();
      _attribute->show();
    }
    else if (_type == Quantity)
    {
      _adjustQuantity->show();
      _attributeLit->hide();
      _attribute->hide();
    }
    else
    {
      _type = None;
      _everyConfig->setChecked(TRUE);
    }
  }
}

void ConfigureGroup::setId(int pId)
{
  if ( (_type == Option) || (_type == Quantity) )
    _option->setId(pId);
  else if (_type == Attribute)
  {
    XSqlQuery query;
    query.prepare( "SELECT itematr_itemopn_id "
                   "FROM itematr "
                   "WHERE (itematr_id=:itematr_id);" );
    query.bindValue(":itematr_id", pId);
    query.exec();
    if (query.first())
    {
      _option->setId(query.value("itematr_itemopn_id").toInt());
      _attribute->setId(pId);
    }
  }
}

void ConfigureGroup::setFlag(bool pFlag)
{
  if (_type == Quantity)
    _adjustQuantity->setChecked(pFlag);
}

void ConfigureGroup::internal()
{
  if (_everyConfig->isChecked())
  {
    _type = None;
    _attribute->setEnabled(FALSE);
    _adjustQuantity->setEnabled(FALSE);
  }
  else
  {
    XSqlQuery query;
    query.prepare( "SELECT itemopn_type "
                   "FROM itemopn "
                   "WHERE (itemopn_id=:itemopn_id);" );
    query.bindValue(":itemopn_id", _option->id());
    query.exec();
    if (query.first())
    {
      QString type = query.value("itemopn_type").toString();
      if (type == "A")
      {
        _type = Attribute;
        _attribute->setEnabled(TRUE);
        _adjustQuantity->hide();
        _attributeLit->show();
        _attribute->show();
      
        query.prepare( "SELECT itematr_id, char_name "
                       "FROM itematr, char "
                       "WHERE ( (itematr_char_id=char_id)"
                       " AND (itematr_itemopn_id=:itemopn_id) ) "
                       "ORDER BY itematr_default DESC, char_name;" );
        query.bindValue(":itemopn_id", _option->id());
        query.exec();
        _attribute->populate(query);
        _attribute->setEnabled(query.count());
      }
      else if (type == "Q")
      {
        _type = Quantity;
        _adjustQuantity->setEnabled(TRUE);
        _adjustQuantity->show();
        _attributeLit->hide();
        _attribute->hide();
      }
      else if (type == "O")
      {
        _type = Option;
        _adjustQuantity->hide();
        _attributeLit->hide();
        _attribute->hide();
      }
//  ToDo
    }
//  ToDo
  }
}

