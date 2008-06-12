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

#include "userPreferences.h"

#include <QSqlError>
#include <QVariant>
#include <QSqlError>

#include <parameter.h>

#include "hotkey.h"
#include "imageList.h"
#include "timeoutHandler.h"

userPreferences::userPreferences(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _pref = _preferences;
  _altPref = 0;

  if(!_privileges->check("MaintainPreferencesOthers"))
    _selectedUser->setEnabled(false);

  connect(_backgroundList,SIGNAL(clicked()),     this, SLOT(sBackgroundList()));
  connect(_close,         SIGNAL(clicked()),     this, SLOT(sClose()));
  connect(_save,          SIGNAL(clicked()),     this, SLOT(sSave()));
  connect(_selectedUser,  SIGNAL(toggled(bool)), this, SLOT(sPopulate()));
  connect(_user,          SIGNAL(newID(int)),    this, SLOT(sPopulate()));
    //hot key signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_hotkey, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_hotkey, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_hotkey, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

  _hotkey->addColumn(tr("Keystroke"), _itemColumn, Qt::AlignLeft );
  _hotkey->addColumn(tr("Action"),    -1,          Qt::AlignLeft );
  _hotkey->addColumn("key",           0,           Qt::AlignLeft );

  connect(_warehouses, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(sWarehouseToggled(QTreeWidgetItem*)));
  connect(_event, SIGNAL(itemSelected(int)), this, SLOT(sAllWarehousesToggled(int)));
  connect(_event, SIGNAL(itemSelectionChanged()), this, SLOT(sFillWarehouseList()));

  _event->addColumn(tr("Module"),      50,   Qt::AlignCenter );
  _event->addColumn(tr("Name"),        150,  Qt::AlignLeft   );
  _event->addColumn(tr("Description"), -1,   Qt::AlignLeft   );
  _event->populate( "SELECT evnttype_id, evnttype_module, evnttype_name, evnttype_descrip "
                    "FROM evnttype "
                    "ORDER BY evnttype_module, evnttype_name" );

  _warehouses->addColumn(tr("Notify"),    50,         Qt::AlignCenter );
  _warehouses->addColumn(tr("Whs."),      _whsColumn, Qt::AlignCenter );
  _warehouses->addColumn(tr("Warehouse"), -1,         Qt::AlignLeft   );
  _warehouses->populate( "SELECT warehous_id, TEXT('-'), warehous_code, warehous_descrip "
                        "FROM warehous "
                        "ORDER BY warehous_code" );

  _dirty = FALSE;

#ifndef Q_WS_MAC
  _backgroundList->setMaximumWidth(25);
#endif

  _user->setType(XComboBox::Users);

  _userGroup->setEnabled(_privileges->check("MaintainUsers"));

  _ellipsesAction->append(1, tr("List"));
  _ellipsesAction->append(2, tr("Search"));

  sPopulate();
}

userPreferences::~userPreferences()
{
  // no need to delete child widgets, Qt does it all for us
  if(_altPref)
    delete _altPref;
}

void userPreferences::languageChange()
{
  retranslateUi(this);
}

void userPreferences::setBackgroundImage(int pImageid)
{
  q.prepare( "SELECT image_id, (image_name || ' - ' || image_descrip) AS description "
             "FROM image "
             "WHERE (image_id=:image_id);" );
  q.bindValue(":image_id", pImageid);
  q.exec();
  if (q.first())
  {
    _backgroundImageid = q.value("image_id").toInt();
    _background->setText(q.value("description").toString());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void userPreferences::sBackgroundList()
{
  ParameterList params;
  params.append("image_id", _backgroundImageid);

  imageList newdlg(this, "", TRUE);
  newdlg.set(params);

  int imageid;
  if ((imageid = newdlg.exec()) != 0)
    setBackgroundImage(imageid);
}

void userPreferences::sPopulate()
{
  if (_currentUser->isChecked())
    _pref = _preferences;
  else
  {
    if(_altPref)
      delete _altPref;
    _altPref = new Preferences(_user->currentText());
    _pref = _altPref;
  }

  if (_pref->value("BackgroundImageid").toInt() > 0)
  {
    _backgroundImage->setChecked(TRUE);
    setBackgroundImage(_pref->value("BackgroundImageid").toInt());
  }
  else
  {
    _noBackgroundImage->setChecked(TRUE);
    _backgroundImageid = -1;
  }

  if (_pref->value("PreferredWarehouse").toInt() == -1)
    _noWarehouse->setChecked(TRUE);
  else
  {
    _selectedWarehouse->setChecked(TRUE);
    _warehouse->setId(_pref->value("PreferredWarehouse").toInt());
  }

  if (_pref->value("InterfaceWindowOption") == "TopLevel")
    _interfaceTopLevel->setChecked(true);
  else
    _interfaceWorkspace->setChecked(true);

  _rememberCheckBoxes->setChecked(! _pref->boolean("XCheckBox/forgetful"));

  _itemCache->setChecked(_pref->boolean("UseItemCache"));
  _customerCache->setChecked(_pref->boolean("UseCustCache"));
  
  _inventoryMenu->setChecked(_pref->boolean("ShowIMMenu"));
  _productsMenu->setChecked(_pref->boolean("ShowPDMenu"));
  _scheduleMenu->setChecked(_pref->boolean("ShowMSMenu"));
  _manufactureMenu->setChecked(_pref->boolean("ShowWOMenu"));
  _crmMenu2->setChecked(_pref->boolean("ShowCRMMenu"));
  _purchaseMenu->setChecked(_pref->boolean("ShowPOMenu"));
  _salesMenu->setChecked(_pref->boolean("ShowSOMenu"));
  _accountingMenu->setChecked(_pref->boolean("ShowGLMenu"));

  _inventoryToolbar->setChecked(_pref->boolean("ShowIMToolbar"));
  _productsToolbar->setChecked(_pref->boolean("ShowPDToolbar"));
  _scheduleToolbar->setChecked(_pref->boolean("ShowMSToolbar"));
  _manufactureToolbar->setChecked(_pref->boolean("ShowWOToolbar"));
  _crmToolbar2->setChecked(_pref->boolean("ShowCRMToolbar"));
  _purchaseToolbar->setChecked(_pref->boolean("ShowPOToolbar"));
  _salesToolbar->setChecked(_pref->boolean("ShowSOToolbar"));
  _accountingToolbar->setChecked(_pref->boolean("ShowGLToolbar"));
  
  _fixedWidthFonts->setChecked(_pref->boolean("UsedFixedWidthFonts"));
  _listNumericItemsFirst->setChecked(_pref->boolean("ListNumericItemNumbersFirst"));
  _showSoitemAvailability->setChecked(_pref->boolean("ShowSOItemAvailability"));
  _ignoreTranslation->setChecked(_pref->boolean("IngoreMissingTranslationFiles"));

  _idleTimeout->setValue(_pref->value("IdleTimeout").toInt());

  if(_pref->value("DefaultEllipsesAction") == "search")
    _ellipsesAction->setId(2);
  else
    _ellipsesAction->setId(1);
    
  //Hide for PostBooks 
  if (_metrics->value("Application") != "OpenMFG")
  {
    _scheduleMenu->hide();
    _scheduleToolbar->hide();
  }
  
  if (!_metrics->boolean("MultiWhs"))
    _warehouseGroup->hide();

  sFillList();
  sFillWarehouseList();
}

void userPreferences::sSave()
{
  if (_backgroundImage->isChecked())
    _pref->set("BackgroundImageid", _backgroundImageid);
  else
    _pref->set("BackgroundImageid", -1);
  
  _pref->set("ShowIMMenu", _inventoryMenu->isChecked());
  _pref->set("ShowPDMenu", _productsMenu->isChecked());
  _pref->set("ShowMSMenu", _scheduleMenu->isChecked());
  _pref->set("ShowWOMenu", _manufactureMenu->isChecked());
  _pref->set("ShowCRMMenu", _crmMenu2->isChecked());
  _pref->set("ShowPOMenu", _purchaseMenu->isChecked());
  _pref->set("ShowSOMenu", _salesMenu->isChecked());
  _pref->set("ShowGLMenu", _accountingMenu->isChecked());
 
  _pref->set("ShowIMToolbar", _inventoryToolbar->isChecked());
  _pref->set("ShowPDToolbar", _productsToolbar->isChecked());
  _pref->set("ShowMSToolbar", _scheduleToolbar->isChecked());
  _pref->set("ShowWOToolbar", _manufactureToolbar->isChecked());
  _pref->set("ShowCRMToolbar", _crmToolbar2->isChecked());
  _pref->set("ShowPOToolbar", _purchaseToolbar->isChecked());
  _pref->set("ShowSOToolbar", _salesToolbar->isChecked());
  _pref->set("ShowGLToolbar", _accountingToolbar->isChecked());
  
  _pref->set("PreferredWarehouse", ((_noWarehouse->isChecked()) ? -1 : _warehouse->id())  );
  _pref->set("XCheckBox/forgetful", !_rememberCheckBoxes->isChecked());
  _pref->set("UseItemCache", _itemCache->isChecked());
  _pref->set("UseCustCache", _customerCache->isChecked());

  _pref->set("UsedFixedWidthFonts", _fixedWidthFonts->isChecked());
  _pref->set("ListNumericItemNumbersFirst", _listNumericItemsFirst->isChecked());
  _pref->set("ShowSOItemAvailability", _showSoitemAvailability->isChecked());
  _pref->set("IngoreMissingTranslationFiles", _ignoreTranslation->isChecked());

  _pref->set("IdleTimeout", _idleTimeout->value());
  omfgThis->_timeoutHandler->setIdleMinutes(_idleTimeout->value());

  if(_ellipsesAction->id() == 2)
    _pref->set("DefaultEllipsesAction", QString("search"));
  else
    _pref->set("DefaultEllipsesAction", QString("list"));

  if(_interfaceTopLevel->isChecked())
    _pref->set("InterfaceWindowOption", QString("TopLevel"));
  else
    _pref->set("InterfaceWindowOption", QString("Workspace"));

  if (_currentUser->isChecked())
  {
    _preferences->load();
        omfgThis->initMenuBar();
  }

  accept();
}

void userPreferences::sClose()
{
  if (_dirty)
    omfgThis->initMenuBar();

  reject();
}

void userPreferences::sFillList()
{
  _hotkey->clear();

  QString hotkey;
  QString action;
  char    key;

  XTreeWidgetItem *last = 0;
  if (_currentUser->isChecked())
  {
    for (key = '1'; key <= '9'; key++)
    {
      hotkey = QString("F%1").arg(key);
      action = _preferences->value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(tr("F%1").arg(key)), action, hotkey);
    }

    for (key = 'A'; key <= 'Z'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = _preferences->value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(QString("Ctrl+%1").arg(key)), action, hotkey);
    }

    for (key = '0'; key <= '9'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = _preferences->value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(QString("Ctrl+%1").arg(key)), action, hotkey);
    }
  }
  else
  {
    Preferences pref(_user->currentText());

    for (key = '1'; key <= '9'; key++)
    {
      hotkey = QString("F%1").arg(key);
      action = pref.value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(tr("F%1").arg(key)), action, hotkey);
    }

    for (key = 'A'; key <= 'Z'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = pref.value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1,QVariant( QString("Ctrl+%1").arg(key)), action, hotkey);
    }

    for (key = '0'; key <= '9'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = pref.value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(QString("Ctrl+%1").arg(key)), action, hotkey);
    }
  }
}

void userPreferences::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  if (_currentUser->isChecked())
    params.append("currentUser");
  else
    params.append("username", _user->currentText());

  hotkey newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
  {
    _dirty = TRUE;
    sFillList();
  }
}

void userPreferences::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("hotkey", _hotkey->currentItem()->text(2));

  if (_currentUser->isChecked())
    params.append("currentUser");
  else
    params.append("username", _user->currentText());

  hotkey newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
  {
    _dirty = TRUE;
    sFillList();
  }
}

void userPreferences::sDelete()
{
  if (_currentUser->isChecked())
  {
    _preferences->remove(_hotkey->currentItem()->text(2));
    _preferences->load();
  }
  else
  {
    q.prepare("SELECT deleteUserPreference(:username, :name) AS _result;");
    if (_currentUser->isChecked())
      q.bindValue(":username", omfgThis->username());
    else
      q.bindValue(":username", _user->currentText());
    q.bindValue(":name", _hotkey->currentItem()->text(2));
    q.exec();
  }

  _dirty = TRUE;
  sFillList();
}

void userPreferences::sAllWarehousesToggled(int pEvnttypeid)
{
  if(!_warehouses->topLevelItemCount() > 0)
    return;

  if (_warehouses->topLevelItem(0)->text(0) == tr("Yes"))
    q.prepare( "DELETE FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_evnttype_id=:evnttype_id) );" );
  else
    q.prepare( "DELETE FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_evnttype_id=:evnttype_id) ); "
               "INSERT INTO evntnot "
               "(evntnot_username, evntnot_evnttype_id, evntnot_warehous_id) "
               "SELECT :username, :evnttype_id, warehous_id "
               "FROM warehous;" );

  if (_currentUser->isChecked())
	q.bindValue(":username", omfgThis->username());
  else
    q.bindValue(":username", _user->currentText());
  q.bindValue(":evnttype_id", pEvnttypeid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillWarehouseList();
}

void userPreferences::sWarehouseToggled(QTreeWidgetItem *selected)
{
  if(!selected)
    return;

  if (selected->text(0) == tr("Yes"))
    q.prepare( "DELETE FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_evnttype_id=:evnttype_id)"
               " AND (evntnot_warehous_id=:warehous_id) );" );
  else
    q.prepare( "INSERT INTO evntnot "
               "(evntnot_username, evntnot_evnttype_id, evntnot_warehous_id) "
               "VALUES "
               "(:username, :evnttype_id, :warehous_id);" );

  if (_currentUser->isChecked())
	q.bindValue(":username", omfgThis->username());
  else
    q.bindValue(":username", _user->currentText());
  q.bindValue(":evnttype_id", _event->id());
  q.bindValue(":warehous_id", ((XTreeWidgetItem *)selected)->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillWarehouseList();
}

void userPreferences::sFillWarehouseList()
{
  for (int i = 0; i < _warehouses->topLevelItemCount(); i++)
  {
    q.prepare( "SELECT evntnot_id "
               "FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_warehous_id=:warehous_id)"
               " AND (evntnot_evnttype_id=:evnttype_id) );" );
    if (_currentUser->isChecked())
	  q.bindValue(":username", omfgThis->username());
    else
      q.bindValue(":username", _user->currentText());
    q.bindValue(":warehous_id", ((XTreeWidgetItem *)(_warehouses->topLevelItem(i)))->id());
    q.bindValue(":evnttype_id", _event->id());
    q.exec();
    if (q.first())
      _warehouses->topLevelItem(i)->setText(0, tr("Yes"));
    else
      _warehouses->topLevelItem(i)->setText(0, tr("No"));
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
