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

#include "calendars.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <parameter.h>
#include "calendar.h"

/*
 *  Constructs a calendars as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
calendars::calendars(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_calhead, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_calhead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_calhead, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
calendars::~calendars()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void calendars::languageChange()
{
    retranslateUi(this);
}


void calendars::init()
{
  statusBar()->hide();
  
  _calhead->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft );
  _calhead->addColumn(tr("Description"), -1,          Qt::AlignLeft );

  sFillList();
}

void calendars::sNew()
{
  int calType = QMessageBox::information( this, tr("New Calendar Type?"),
                                          tr("What type of Calendar do you want to create?"),
                                          tr("Cancel"), tr("Absolute"), tr("Relative"), 1, 0 );

  if (calType != 0)
  {
    ParameterList params;
    params.append("mode", "new");

    if (calType == 1)
      params.append("type", "absolute");
    else if (calType == 2)
      params.append("type", "relative");

    calendar newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void calendars::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("calhead_id", _calhead->id());

  calendar newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void calendars::sDelete()
{
  QString sql( "DELETE FROM calhead "
               "WHERE (calhead_id=:calhead_id);" );

  if (_calhead->altId() == 1)
    sql += "DELETE FROM acalitem "
           "WHERE (acalitem_calhead_id=:calhead_id);";
  else if (_calhead->altId() == 2)
    sql += "DELETE FROM rcalitem "
           "WHERE (rcalitem_calhead_id=:calhead_id);";

  q.prepare(sql);
  q.bindValue(":calhead_id", _calhead->id());
  q.exec();

  sFillList();
}

void calendars::sFillList()
{
  _calhead->populate( "SELECT calhead_id,"
                      "       CASE WHEN (calhead_type='A') THEN 1"
                      "            WHEN (calhead_type='R') THEN 2"
                      "            ELSE 3"
                      "       END,"
                      "       calhead_name, calhead_descrip "
                      "FROM calhead "
                      "ORDER BY calhead_name;", TRUE );
}

