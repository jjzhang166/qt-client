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

#include "whseWeek.h"

#include <QMessageBox>
#include <QVariant>

/*
 *  Constructs a whseWeek as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
whseWeek::whseWeek(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sPopulate()));
  connect(_sunday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_monday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_tuesday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_wednesday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_thursday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_friday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_saturday, SIGNAL(toggled(bool)), this, SLOT(sChange()));

  _warehousid = -1;
  _dirty = false;

  sPopulate();
}

/*
 *  Destroys the object and frees any allocated resources
 */
whseWeek::~whseWeek()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void whseWeek::languageChange()
{
    retranslateUi(this);
}

void whseWeek::sPopulate()
{
  if(!checkAndSave())
  {
    if(-1 == _warehousid)
      _warehouse->setAll();
    else
      _warehouse->setId(_warehousid);
    return;
  }

  _warehousid = _warehouse->id();

  q.prepare("SELECT TRUE AS result"
            "  FROM whsewk"
            " WHERE ((whsewk_warehous_id=:warehous_id)"
            "   AND  (whsewk_weekday=:wkday));");

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 0);
  q.exec();
  bool result = false;
  if(q.first())
    result = q.value("result").toBool();
  _sunday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 1);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _monday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 2);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _tuesday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 3);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _wednesday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 4);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _thursday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 5);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _friday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 6);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _saturday->setChecked(result);

  _dirty = false;
}

void whseWeek::sSave()
{
  if(save(_warehouse->id()))
    accept();
}

bool whseWeek::save(int whsid)
{
  q.prepare("SELECT setWhseWkday(:whsid, 0, :sunday),"
            "       setWhseWkday(:whsid, 1, :monday),"
            "       setWhseWkday(:whsid, 2, :tuesday),"
            "       setWhseWkday(:whsid, 3, :wednesday),"
            "       setWhseWkday(:whsid, 4, :thursday),"
            "       setWhseWkday(:whsid, 5, :friday),"
            "       setWhseWkday(:whsid, 6, :saturday);");
  q.bindValue(":whsid", whsid);
  q.bindValue(":sunday", QVariant(_sunday->isChecked(), 0));
  q.bindValue(":monday", QVariant(_monday->isChecked(), 0));
  q.bindValue(":tuesday", QVariant(_tuesday->isChecked(), 0));
  q.bindValue(":wednesday", QVariant(_wednesday->isChecked(), 0));
  q.bindValue(":thursday", QVariant(_thursday->isChecked(), 0));
  q.bindValue(":friday", QVariant(_friday->isChecked(), 0));
  q.bindValue(":saturday", QVariant(_saturday->isChecked(), 0));

  if(!q.exec())
  {
    QMessageBox::critical(this, tr("Error Saving"),
      tr("There was an error saving the information.") );
    return false;
  }

  if(whsid == -1)
  {
    q.prepare("SELECT setWhseWkday(warehous_id, 0, :sunday),"
              "       setWhseWkday(warehous_id, 1, :monday),"
              "       setWhseWkday(warehous_id, 2, :tuesday),"
              "       setWhseWkday(warehous_id, 3, :wednesday),"
              "       setWhseWkday(warehous_id, 4, :thursday),"
              "       setWhseWkday(warehous_id, 5, :friday),"
              "       setWhseWkday(warehous_id, 6, :saturday)"
              "  FROM warehous;");
    q.bindValue(":sunday", QVariant(_sunday->isChecked(), 0));
    q.bindValue(":monday", QVariant(_monday->isChecked(), 0));
    q.bindValue(":tuesday", QVariant(_tuesday->isChecked(), 0));
    q.bindValue(":wednesday", QVariant(_wednesday->isChecked(), 0));
    q.bindValue(":thursday", QVariant(_thursday->isChecked(), 0));
    q.bindValue(":friday", QVariant(_friday->isChecked(), 0));
    q.bindValue(":saturday", QVariant(_saturday->isChecked(), 0));
    q.exec();
  }

  _dirty = false;
  return true;
}

void whseWeek::sClose()
{
  if(checkAndSave())
    reject();
}

bool whseWeek::checkAndSave()
{
  if (_dirty)
  {
    switch(QMessageBox::question(this, tr("Unsaved Changes"),
             tr("There are unsaved changes. Would you like to save before continuing?"),
             QMessageBox::Yes | QMessageBox::Default,
             QMessageBox::No,
             QMessageBox::Cancel | QMessageBox::Escape))
    {
      case QMessageBox::Cancel:
        return false;
      case QMessageBox::Yes:
        return save(_warehousid);
    }
  }

  return true;
}

void whseWeek::sChange()
{
  _dirty = true;
}

