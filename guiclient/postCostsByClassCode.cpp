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

#include "postCostsByClassCode.h"

#include <QSqlError>
#include <QVariant>
#include "submitAction.h"

/*
 *  Constructs a postCostsByClassCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postCostsByClassCode::postCostsByClassCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_selectAll, SIGNAL(clicked()), this, SLOT(sSelectAll()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  
  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();

  _classCode->setType(ClassCode);

  if (!_metrics->boolean("Routings"))
  {
    _directLabor->hide();
    _lowerDirectLabor->hide();
    _overhead->hide();
    _lowerOverhead->hide();
    _machOverhead->hide();
    _lowerMachOverhead->hide();
  }
  else if (_metrics->value("TrackMachineOverhead") != "M")
  {
    _machOverhead->setEnabled(FALSE);
    _machOverhead->setChecked(TRUE);
    _lowerMachOverhead->setEnabled(FALSE);
    _lowerMachOverhead->setChecked(TRUE);
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
postCostsByClassCode::~postCostsByClassCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postCostsByClassCode::languageChange()
{
    retranslateUi(this);
}

void postCostsByClassCode::sSelectAll()
{
  _material->setChecked(TRUE);
  _lowerMaterial->setChecked(TRUE);
  _user->setChecked(TRUE);
  _lowerUser->setChecked(TRUE);
  if (_metrics->boolean("Routings"))
  {
    _directLabor->setChecked(TRUE);
    _lowerDirectLabor->setChecked(TRUE);
    _overhead->setChecked(TRUE);
    _lowerOverhead->setChecked(TRUE);
    if (_metrics->value("TrackMachineOverhead") == "M")
    {
      _machOverhead->setChecked(TRUE);
      _lowerMachOverhead->setChecked(TRUE);
    }
  }
}

void postCostsByClassCode::sPost()
{
    QString sql = "SELECT doPostCosts(item_id, TRUE, :material, :lowMaterial, "
	    "	:directLabor, :lowDirectLabor, :overhead, :lowOverhead, "
	    "	:machOverhead, :lowMachOverhead, :user, :lowUser, :rollUp) "
	    "FROM item, classcode "
	    "WHERE (item_classcode_id=classcode_id)";

    if (_classCode->isSelected())
        sql += " AND (item_classcode_id=:classcode_id);";
    else if (_classCode->isPattern())
        sql += " AND (classcode_code ~ :classcode_pattern);";
    else
	sql = "SELECT doPostCosts(:material, :lowMaterial, "
	    "	:directLabor, :lowDirectLabor, :overhead, :lowOverhead, "
	    "	:machOverhead, :lowMachOverhead, :user, :lowUser, :rollUp);";

    q.prepare(sql);
    q.bindValue(":material", _material->isChecked()		? "t" : "f");
    q.bindValue(":lowMaterial", _lowerMaterial->isChecked()	? "t" : "f");
    q.bindValue(":directLabor", _directLabor->isChecked()	? "t" : "f");
    q.bindValue(":lowDirectLabor", _lowerDirectLabor->isChecked() ? "t" : "f");
    q.bindValue(":overhead", _overhead->isChecked()		? "t" : "f");
    q.bindValue(":lowOverhead", _lowerOverhead->isChecked()	? "t" : "f");
    q.bindValue(":machOverhead", _machOverhead->isChecked()	? "t" : "f");
    q.bindValue(":lowMachOverhead", _lowerMachOverhead->isChecked() ? "t" : "f");
    q.bindValue(":user", _user->isChecked()		? "t" : "f");
    q.bindValue(":lowUser", _lowerUser->isChecked()	? "t" : "f");
    q.bindValue(":rollUp", _rollUp->isChecked()		? "t" : "f");

    _classCode->bindValue(q);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
    }

  accept();
}

void postCostsByClassCode::sSubmit()
{
  ParameterList params;

  params.append("action_name", "PostActualCost");

  if (_material->isChecked())
    params.append("Material");

  if (_lowerMaterial->isChecked())
    params.append("LowerMaterial");

  if (_directLabor->isChecked())
    params.append("DirectLabor");

  if (_lowerDirectLabor->isChecked())
    params.append("LowerDirectLabor");

  if (_overhead->isChecked())
    params.append("Overhead");

  if (_lowerOverhead->isChecked())
    params.append("LowerOverhead");

  if (_machOverhead->isChecked())
    params.append("MachineOverhead");

  if (_lowerMachOverhead->isChecked())
    params.append("LowerMachineOverhead");

  if (_user->isChecked())
    params.append("User");

  if (_lowerUser->isChecked())
    params.append("LowerUser");

  if (_rollUp->isChecked())
    params.append("RollUp");

  _classCode->appendValue(params);

  submitAction newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() == XDialog::Accepted)
    accept();
}

