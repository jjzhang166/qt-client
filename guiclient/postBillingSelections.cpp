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

#include "postBillingSelections.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include "rwInterface.h"

/*
 *  Constructs a postBillingSelections as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postBillingSelections::postBillingSelections(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _customerType->setType(CustomerType);
  _post->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
postBillingSelections::~postBillingSelections()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postBillingSelections::languageChange()
{
  retranslateUi(this);
}

void postBillingSelections::sPost()
{
  QString sql("SELECT postBillingSelections(custtype_id, :consolidate) AS result"
              " FROM custtype" );
  if (_customerType->isSelected())
    sql += " WHERE (custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " WHERE (custtype_code ~ :custtype_pattern)";
  q.prepare(sql);
  _customerType->bindValue(q);
  q.bindValue(":consolidate", _consolidate->isChecked());
  q.exec();
  if (q.first())
  {
    if (q.value("result").toInt() == -5)
      QMessageBox::critical( this, tr("Cannot Post one or more Invoices"),
                             tr( "The G/L Account Assignments for one or more of the Billing Selections that you are trying to post\n"
                                 "are not configured correctly.  Because of this, G/L Transactions cannot be posted for these.\n"
                                 "You must contact your Systems Administrator to have this corrected before you may\n"
                                 "post these Billing Selections." ) );
    else if (q.value("result").toInt() < 0)
      systemError( this, tr("A System Error occurred at postBillingSelections::%1, Error #%2.")
                         .arg(__LINE__)
                         .arg(q.value("result").toInt()) );

    omfgThis->sInvoicesUpdated(-1, TRUE);
    omfgThis->sBillingSelectionUpdated(-1, TRUE);
    omfgThis->sSalesOrdersUpdated(-1);
  }
  else
    systemError( this, tr("A System Error occurred at postBillingSelections::%1.")
                       .arg(__LINE__) );

  accept();
}

