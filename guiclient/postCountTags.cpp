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

#include "postCountTags.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a postCountTags as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postCountTags::postCountTags(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _codeGroup = new QButtonGroup(this);
    _codeGroup->addButton(_plancode);
    _codeGroup->addButton(_classcode);

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    connect(_codeGroup, SIGNAL(buttonClicked(int)), this, SLOT(sParameterTypeChanged()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
postCountTags::~postCountTags()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postCountTags::languageChange()
{
    retranslateUi(this);
}


void postCountTags::init()
{
  _parameter->setType(ClassCode);
  
  if(!_privileges->check("ThawInventory"))
  {
    _thaw->setChecked(FALSE);
    _thaw->setEnabled(FALSE);
  }
}

void postCountTags::sPost()
{
  QString sql( "SELECT MIN(postCountTag(invcnt_id, :thaw)) AS result "
               "FROM invcnt, itemsite, item "
               "WHERE ( (invcnt_itemsite_id=itemsite_id)"
               " AND (invcnt_qoh_after IS NOT NULL)"
               " AND (NOT invcnt_posted)"
               " AND (itemsite_item_id=item_id)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";
  
    if ((_parameter->type() == ClassCode) && _parameter->isSelected())
      sql += " AND (item_classcode_id=:classcode_id)";
    else if ((_parameter->type() == ClassCode) && _parameter->isPattern())
      sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";
    else if ((_parameter->type() == PlannerCode) && _parameter->isSelected())
      sql += " AND (itemsite_plancode_id=:plancode_id)";
    else if ((_parameter->type() == PlannerCode) && _parameter->isPattern())
      sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";
  
  sql += ");";
  
  q.prepare(sql);
  q.bindValue(":thaw", QVariant(_thaw->isChecked(), 0));
  _warehouse->bindValue(q);
  _parameter->bindValue(q);
  q.exec();

  if (q.first())
  {
    if (q.value("result").toInt() < 0)
      QMessageBox::information( this, tr("One or More Posts Failed"),
        tr("One or more of your Count Tags could not be posted. Review\n"
           "the Count Tag Edit list to ensure your Count Tags and\n"
           "Count Slips have been entered properly. Or contact your\n"
           "Systems Administrator for more information. The function\n"
           "postCountTag returned the error #%1." )
           .arg(q.value("result").toInt()) );
  }
  else
    systemError( this, tr("A System Error occurred at postCountTags::%1.")
                       .arg(__LINE__) );

  accept();
}

void postCountTags::sParameterTypeChanged()
{
  if(_plancode->isChecked())
    _parameter->setType(PlannerCode);
  else
    _parameter->setType(ClassCode);

}
