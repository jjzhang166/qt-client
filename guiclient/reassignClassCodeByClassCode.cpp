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

#include "reassignClassCodeByClassCode.h"

#include <qvariant.h>

/*
 *  Constructs a reassignClassCodeByClassCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
reassignClassCodeByClassCode::reassignClassCodeByClassCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_classCodePattern, SIGNAL(toggled(bool)), _classCode, SLOT(setEnabled(bool)));
    connect(_selectedClassCode, SIGNAL(toggled(bool)), _classCodes, SLOT(setEnabled(bool)));
    connect(_reassign, SIGNAL(clicked()), this, SLOT(sReassign()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
reassignClassCodeByClassCode::~reassignClassCodeByClassCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void reassignClassCodeByClassCode::languageChange()
{
    retranslateUi(this);
}


void reassignClassCodeByClassCode::init()
{
  _classCodes->setType(XComboBox::ClassCodes);
  _newClassCode->setType(XComboBox::ClassCodes);
}

void reassignClassCodeByClassCode::sReassign()
{
  QString sql( "UPDATE item "
               "SET item_classcode_id=:new_classcode_id ");

  if (_selectedClassCode->isChecked())
    sql += "WHERE (item_classcode_id=:old_classcode_id);";
  else if (_classCodePattern->isChecked())
    sql += "WHERE (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :old_classcode_code)));";

  q.prepare(sql);
  q.bindValue(":new_classcode_id", _newClassCode->id());
  q.bindValue(":old_classcode_id", _classCodes->id());
  q.bindValue(":old_classcode_code", _classCode->text());
  q.exec();

  accept();
}
