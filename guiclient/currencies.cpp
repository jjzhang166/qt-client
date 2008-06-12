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

#include "currencies.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <parameter.h>
#include <QWorkspace>
#include "currency.h"

/*
 *  Constructs a currencies as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
currencies::currencies(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_curr, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_curr, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
currencies::~currencies()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void currencies::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>
#include <QSqlError>

void currencies::init()
{
  statusBar()->hide();

  if (_privileges->check("MaintainCurrencies"))
  {
    connect(_curr, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_curr, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_curr, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_curr, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
    
  _curr->addColumn( tr("Base"),		_ynColumn,       Qt::AlignCenter, true, "flag");
  _curr->addColumn( tr("Name"),		-1,              Qt::AlignLeft,   true, "curr_name");
  _curr->addColumn( tr("Symbol"),	_currencyColumn, Qt::AlignCenter, true, "curr_symbol");
  _curr->addColumn( tr("Abbreviation"),	_currencyColumn, Qt::AlignLeft,   true, "curr_abbr");
    
  sFillList();
}

void currencies::sNew()
{
  bool single = omfgThis->singleCurrency();

  ParameterList params;
  params.append("mode", "new");
    
  currency *newdlg = new currency(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
  sFillList();

  if(single && !omfgThis->singleCurrency())
  {
    // Check for the gain/loss and discrep accounts
    q.prepare("SELECT COALESCE((SELECT TRUE"
              "                   FROM accnt, metric"
              "                  WHERE ((accnt_id=metric_value)"
              "                    AND  (metric_name='CurrencyGainLossAccount'))), FALSE)"
              "   AND COALESCE((SELECT TRUE"
              "                   FROM accnt, metric"
              "                  WHERE ((accnt_id=metric_value)"
              "                    AND  (metric_name='GLSeriesDiscrepancyAccount'))), FALSE) AS result; ");
    q.exec();
    if(q.first() && q.value("result").toBool() != true)
      QMessageBox::warning( this, tr("Additional Configuration Required"),
        tr("Your system is configured to use multiple Currencies, but the\n"
           "Currency Gain/Loss Account and/or the G/L Series Discrepancy Account\n"
           "does not appear to be configured correctly. You should define these\n"
           "Accounts in 'System | Configure Modules | Configure G/L...' before\n"
           "posting any transactions in the system.") );
  }
}

void currencies::sEdit()
{
  ParameterList params;
  params.append("curr_id", _curr->id());
  params.append("mode", "edit");
    
  currency *newdlg = new currency(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
  sFillList();
}

void currencies::sView()
{
  ParameterList params;
  params.append("curr_id", _curr->id());
  params.append("mode", "view");
    
  currency *newdlg = new currency(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void currencies::sDelete()
{
    q.prepare("SELECT curr_base FROM curr_symbol "
    		"WHERE curr_id = :curr_id");
    q.bindValue(":curr_id", _curr->id());
    q.exec();
    if (q.first() && q.value("curr_base").toBool())
    {
	QMessageBox::critical(this,
			      tr("Cannot delete base currency"),
			      tr("You cannot delete the base currency."));
	return;
    }
    
    q.prepare("DELETE FROM curr_symbol WHERE curr_id = :curr_id");
    q.bindValue(":curr_id", _curr->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    
    sFillList();
}

void currencies::sFillList()
{
  _curr->clear();
  q.prepare( "SELECT *, "
             "	CASE WHEN curr_base = TRUE THEN 'Y' "
             "	ELSE '' END AS flag "
             "FROM curr_symbol "
             "ORDER BY flag DESC, curr_name;" );
  q.exec();
  _curr->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void currencies::sPopulateMenu(QMenu* pMenu)
{
    int menuItem;
    
    pMenu->insertItem("View...", this, SLOT(sView()), 0);
    
    menuItem = pMenu->insertItem("Edit...", this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainCurrencies"))
	pMenu->setItemEnabled(menuItem, FALSE);
    
    menuItem = pMenu->insertItem("Delete...", this, SLOT(sDelete()), 0);
    if (!_privileges->check("MaintainCurrencies"))
	pMenu->setItemEnabled(menuItem, FALSE);
}

