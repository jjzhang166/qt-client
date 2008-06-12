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

#include "department.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a department as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
department::department(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
department::~department()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void department::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QSqlError>
void department::init()
{
    //statusBar()->hide();
}

enum SetResponse department::set(ParameterList& pParams)
{
    QVariant	param;
    bool	valid;

    param = pParams.value("dept_id", &valid);
    if (valid)
    {
	_deptid = param.toInt();
	populate();
    }

    param = pParams.value("mode", &valid);
    if (valid)
    {
	if (param.toString() == "new")
	{
	    _mode = cNew;
	    _number->setFocus();
	}
	else if (param.toString() == "edit")
	{
	    _mode = cEdit;
	    _name->setFocus();
	}
	else if (param.toString() == "view")
	{
	    _mode = cView;
	    _close->setText(tr("&Close"));
	    _number->setEnabled(false);
	    _name->setEnabled(false);
	    _save->hide();
	    _close->setFocus();
	}
    }

    return NoError;
}

void department::sSave()
{
    QString number = _number->text().stripWhiteSpace().upper();

    if (number.isEmpty())
    {
	QMessageBox::critical(this, tr("Cannot Save Department"),
			      tr("You must enter a Department Number"));
	_number->setFocus();
	return;
    }
    if (_name->text().stripWhiteSpace().isEmpty())
    {
	QMessageBox::critical(this, tr("Cannot Save Department"),
			      tr("You must enter a Department Name"));
	_name->setFocus();
	return;
    }
    
    if (_mode == cNew)
    {
	q.exec("SELECT NEXTVAL('dept_dept_id_seq') AS dept_id;");
	if (q.first())
	    _deptid =  q.value("dept_id").toInt();
	else
	{
	    systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
				.arg(__FILE__)
				.arg(__LINE__)
				.arg(q.lastError().databaseText()));
	    return;
	}
	q.prepare("INSERT INTO dept ( dept_id,  dept_number,  dept_name ) "
		  "       VALUES    (:dept_id, :dept_number, :dept_name );");
    }
    else
	if (_mode == cEdit)
	    q.prepare("UPDATE dept "
		      "SET dept_id=:dept_id, "
		      "    dept_number=:dept_number, "
		      "    dept_name=:dept_name "
		      "WHERE (dept_id=:dept_id);");
    q.bindValue(":dept_id",	_deptid);
    q.bindValue(":dept_number",	number);
    q.bindValue(":dept_name",	_name->text().stripWhiteSpace());

    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
	systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
			    .arg(__FILE__)
			    .arg(__LINE__)
			    .arg(q.lastError().databaseText()));
	return;
    }

    close();
}

void department::sClose()
{
    close();
}


void department::populate()
{
    q.prepare("SELECT dept_number, dept_name "
	      "FROM dept "
	      "WHERE (dept_id=:dept_id);");
    q.bindValue(":dept_id", _deptid);
    q.exec();
    if (q.first())
    {
	_number->setText(q.value("dept_number"));
	_name->setText(q.value("dept_name"));
    }
    else
	systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
			    .arg(__FILE__)
			    .arg(__LINE__)
			    .arg(q.lastError().databaseText()));
}
