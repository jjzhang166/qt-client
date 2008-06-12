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

#include "countries.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>
#include <QWorkspace>

#include <parameter.h>

#include "country.h"

/*
 *  Constructs a countries as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
countries::countries(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    connect(_new,	SIGNAL(clicked()),	this,	SLOT(sNew()));
    connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
    connect(_close,	SIGNAL(clicked()),	this,	SLOT(close()));
    connect(_delete,	SIGNAL(clicked()),	this,	SLOT(sDelete()));
    connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));
    connect(_countries,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),
					  this,	SLOT(sPopulateMenu(QMenu*)));

    statusBar()->hide();

    if (_privileges->check("MaintainCountries"))
    {
      connect(_countries, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_countries, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_countries, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
      _new->setEnabled(FALSE);
      connect(_countries, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }
      
    _countries->addColumn( tr("Abbreviation"),	      50, Qt::AlignCenter );
    _countries->addColumn( tr("Name"),	              -1, Qt::AlignLeft   );
    _countries->addColumn( tr("Currency Abbr"),      100, Qt::AlignCenter );
    _countries->addColumn( tr("Currency Name"),      100, Qt::AlignLeft   );
    _countries->addColumn( tr("Symbol"),              50, Qt::AlignLeft   );
      
    sFillList();
}

countries::~countries()
{
    // no need to delete child widgets, Qt does it all for us
}

void countries::languageChange()
{
    retranslateUi(this);
}

void countries::sNew()
{
  ParameterList params;
  params.append("mode", "new");
    
  country *newdlg = new country(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
  sFillList();
}

void countries::sEdit()
{
  ParameterList params;
  params.append("country_id", _countries->id());
  params.append("mode", "edit");
    
  country *newdlg = new country(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
  sFillList();
}

void countries::sView()
{
  ParameterList params;
  params.append("country_id", _countries->id());
  params.append("mode", "view");
    
  country *newdlg = new country(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void countries::sDelete()
{
  int answer = QMessageBox::question(this,
				     tr("Confirm Delete"),
				     tr("<p>This is a master table used for "
					"selecting country names for Addresses "
					"and default currency symbols for the "
					"Currency window. Are you sure you "
					"want to delete this country and its "
					"associated information?"),
				      QMessageBox::Yes,
				      QMessageBox::No | QMessageBox::Default);
  if (QMessageBox::Yes == answer)
  {
    q.prepare("DELETE FROM country WHERE country_id = :country_id;");
    q.bindValue(":country_id", _countries->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  
    sFillList();
  }
}

void countries::sFillList()
{
    _countries->clear();
    q.prepare( "SELECT country_id, country_abbr, country_name,"
	       "       country_curr_abbr, country_curr_name, "
	       "       country_curr_symbol "
	       "FROM country "
	       "ORDER BY country_name;" );
    q.exec();
    _countries->populate(q);
    /*
    while (q.next())
    {
	new XTreeWidgetItem(_countries, _countries->lastItem(),
			  q.value("country_id").toInt(),
			  q.value("country_abbr"),
			  q.value("country_name"),
			  q.value("country_curr_abbr"),
			  q.value("country_curr_name"),
			  q.value("country_curr_symbol"));
    }
    */
}

void countries::sPopulateMenu(QMenu* pMenu)
{
    int menuItem;
    
    pMenu->insertItem("View...", this, SLOT(sView()), 0);
    
    menuItem = pMenu->insertItem("Edit...", this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainCountries"))
	pMenu->setItemEnabled(menuItem, FALSE);
    
    menuItem = pMenu->insertItem("Delete...", this, SLOT(sDelete()), 0);
    if (!_privileges->check("MaintainCountries"))
	pMenu->setItemEnabled(menuItem, FALSE);
}
