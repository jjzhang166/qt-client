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

#include "shifts.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <openreports.h>
#include "shift.h"

/*
 *  Constructs a shifts as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
shifts::shifts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_shiftList, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
shifts::~shifts()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void shifts::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>
#include <QSqlError>
void shifts::init()
{
    statusBar()->hide();

    _shiftList->addColumn(tr("Shift Number"),	_userColumn,	Qt::AlignLeft );
    _shiftList->addColumn(tr("Shift Name"),	-1,		Qt::AlignLeft );

    if (_privileges->check("MaintainShifts"))
    {
	connect(_shiftList, SIGNAL(valid(bool)), _edit,	SLOT(setEnabled(bool)));
	connect(_shiftList, SIGNAL(valid(bool)), _delete,SLOT(setEnabled(bool)));
	connect(_shiftList, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
	_new->setEnabled(false);
	_edit->setEnabled(false);
	_delete->setEnabled(false);
	connect(_shiftList, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }
    connect(_shiftList, SIGNAL(valid(bool)), _view,	SLOT(setEnabled(bool)));

    sFillList();
}

void shifts::sClose()
{
    close();
}


void shifts::sPrint()
{
    orReport report("ShiftsMasterList");
    if (report.isValid())
	report.print();
    else
	report.reportError(this);
}

void shifts::sNew()
{
    ParameterList params;
    params.append("mode", "new");

    shift newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();	//if (newdlg.exec() != XDialog::Rejected)
	sFillList();
}

void shifts::sEdit()
{
    ParameterList params;
    params.append("mode", "edit");
    params.append("shift_id", _shiftList->id());

    shift newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();	//if (newdlg.exec() != XDialog::Rejected)
	sFillList();
}

void shifts::sView()
{
    ParameterList params;
    params.append("mode", "view");
    params.append("shift_id", _shiftList->id());

    shift *newdlg = new shift(this, "", TRUE);
    newdlg->set(params);
    newdlg->show();
}

void shifts::sDelete()
{
    q.prepare("DELETE FROM shift WHERE shift_id = :shift_id;");
    q.bindValue(":shift_id", _shiftList->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
	systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
			    .arg(__FILE__)
			    .arg(__LINE__)
			    .arg(q.lastError().databaseText()));
    sFillList();
}

void shifts::sFillList()
{
    _shiftList->populate("SELECT shift_id, shift_number, shift_name "
			"FROM shift "
			"ORDER BY shift_number;");
}

void shifts::sPopulateMenu(QMenu *pMenu )
{
    int menuItem;

    menuItem = pMenu->insertItem(tr("Edit"), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainShifts"));

    menuItem = pMenu->insertItem(tr("View"), this, SLOT(sView()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewShifts") ||
				    _privileges->check("MaintainShifts"));

    menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainShifts"));
}
