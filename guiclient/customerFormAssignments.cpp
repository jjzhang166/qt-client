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

#include "customerFormAssignments.h"

#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>
#include "customerFormAssignment.h"

/*
 *  Constructs a customerFormAssignments as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
customerFormAssignments::customerFormAssignments(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_custform, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
customerFormAssignments::~customerFormAssignments()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void customerFormAssignments::languageChange()
{
    retranslateUi(this);
}


void customerFormAssignments::init()
{
  statusBar()->hide();
  
  _custform->addColumn(tr("Customer Type"), -1,  Qt::AlignCenter );
  _custform->addColumn(tr("Invoice"),       100, Qt::AlignCenter );
  _custform->addColumn(tr("Credit Memo"),   100, Qt::AlignCenter );
  _custform->addColumn(tr("Statement"),     100, Qt::AlignCenter );
  _custform->addColumn(tr("Quote"),         100, Qt::AlignCenter );
  _custform->addColumn(tr("Packing List"),  100, Qt::AlignCenter );
  _custform->addColumn(tr("S/O Pick List"), 100, Qt::AlignCenter );

  if (_privileges->check("MaintainSalesAccount"))
  {
    connect(_custform, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_custform, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_custform, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_custform, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

void customerFormAssignments::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  
  customerFormAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void customerFormAssignments::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("custform_id", _custform->id());
  
  customerFormAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void customerFormAssignments::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("custform_id", _custform->id());
  
  customerFormAssignment newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void customerFormAssignments::sDelete()
{
  q.prepare( "DELETE FROM custform "
             "WHERE (custform_id=:custform_id);" );
  q.bindValue(":custform_id", _custform->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void customerFormAssignments::sFillList()
{
  q.prepare( "SELECT custform_id,"
             "       CASE WHEN custform_custtype_id=-1 THEN custform_custtype"
             "            ELSE (SELECT custtype_code FROM custtype WHERE (custtype_id=custform_custtype_id))"
             "       END AS custtypecode,"
             "       CASE WHEN (custform_invoice_report_id=-1) THEN :default"
             "            ELSE (SELECT report_name FROM report WHERE (report_id=custform_invoice_report_id))"
             "       END,"
             "       CASE WHEN (custform_creditmemo_report_id=-1) THEN :default"
             "            ELSE (SELECT report_name FROM report WHERE (report_id=custform_creditmemo_report_id))"
             "       END,"
             "       CASE WHEN (custform_statement_report_id=-1) THEN :default"
             "            ELSE (SELECT report_name FROM report WHERE (report_id=custform_statement_report_id))"
             "       END,"
             "       CASE WHEN (custform_quote_report_id=-1) THEN :default"
             "            ELSE (SELECT report_name FROM report WHERE (report_id=custform_quote_report_id))"
             "       END,"
             "       CASE WHEN (custform_packinglist_report_id=-1) THEN :default"
             "            ELSE (SELECT report_name FROM report WHERE (report_id=custform_packinglist_report_id))"
             "       END,"
             "       CASE WHEN (custform_sopicklist_report_id=-1) THEN :default"
             "            ELSE (SELECT report_name FROM report WHERE (report_id=custform_sopicklist_report_id))"
             "       END "
             "FROM custform "
             "ORDER BY custtypecode;" );
  q.bindValue(":default", tr("Default"));
  q.exec();
  _custform->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
