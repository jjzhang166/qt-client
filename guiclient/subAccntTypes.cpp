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

#include "subAccntTypes.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "subAccntType.h"
#include <openreports.h>
#include <qstatusbar.h>

/*
 *  Constructs a subAccntTypes as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
subAccntTypes::subAccntTypes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_subaccnttypes, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_subaccnttypes, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_subaccnttypes, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
subAccntTypes::~subAccntTypes()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void subAccntTypes::languageChange()
{
    retranslateUi(this);
}


void subAccntTypes::init()
{
  statusBar()->hide();

  _subaccnttypes->addColumn(tr("Code"),        70, Qt::AlignLeft );
  _subaccnttypes->addColumn(tr("Type"),        50, Qt::AlignLeft );
  _subaccnttypes->addColumn(tr("Description"), -1, Qt::AlignLeft );
  
  sFillList();
}

void subAccntTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  subAccntType newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void subAccntTypes::sPrint()
{
  orReport report("SubAccountTypeMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void subAccntTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("subaccnttype_id", _subaccnttypes->id());

  subAccntType newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void subAccntTypes::sDelete()
{
  q.prepare("SELECT deleteSubAccountType(:subaccnttype_id) AS result;");
  q.bindValue(":subaccnttype_id", _subaccnttypes->id());
  q.exec();
  if(q.first())
  {
    switch(q.value("result").toInt())
    {
    case -1:
      QMessageBox::critical( this, tr("Cannot Delete G/L Subaccount Type"),
          tr( "The selected G/L Subaccount Type cannot be deleted as it is currently used in one or more G/L Accounts.\n"
              "You must reassign these G/L Accounts before you may delete the selected G/L Subaccount Type." ) );
        break;
    default:
      sFillList();
    }
  }
}

void subAccntTypes::sFillList()
{
  q.prepare(
      "SELECT subaccnttype_id, subaccnttype_code,"
      "       CASE WHEN(subaccnttype_accnt_type='A') THEN :asset"
      "            WHEN(subaccnttype_accnt_type='L') THEN :liability"
      "            WHEN(subaccnttype_accnt_type='E') THEN :expense"
      "            WHEN(subaccnttype_accnt_type='R') THEN :revenue"
      "            WHEN(subaccnttype_accnt_type='Q') THEN :equity"
      "            ELSE :error"
      "       END, subaccnttype_descrip "
      "FROM subaccnttype "
      "ORDER BY subaccnttype_code; " );
  q.bindValue(":asset", tr("Asset"));
  q.bindValue(":liability", tr("Liability"));
  q.bindValue(":expense", tr("Expense"));
  q.bindValue(":revenue", tr("Revenue"));
  q.bindValue(":equity", tr("Equity"));
  q.bindValue(":error", tr("ERROR"));
  q.exec();
  _subaccnttypes->populate(q);
}

