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

#include "currencyConversions.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include "currencyConversion.h"
#include "openreports.h"
#include "currency.h"
#include "datecluster.h"
#include "xcombobox.h"

/*
 *  Constructs a currencyConversions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
currencyConversions::currencyConversions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_conversionRates, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_newCurrency, SIGNAL(clicked()), this, SLOT(sNewCurrency()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
currencyConversions::~currencyConversions()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void currencyConversions::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>
#include <QSqlError>

void currencyConversions::init()
{
    statusBar()->hide();

    setBaseCurrency();
    _conversionRates->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignLeft);
    _conversionRates->addColumn(tr("Exchange Rate"), -1,           Qt::AlignRight);
    _conversionRates->addColumn(tr("Effective"),    _dateColumn,   Qt::AlignCenter);
    _conversionRates->addColumn(tr("Expires"),      _dateColumn,   Qt::AlignCenter);
    
    _newCurrency->setEnabled(_privileges->check("CreateNewCurrency"));
    
    bool maintainPriv = _privileges->check("MaintainCurrencyRates");
    _new->setEnabled(maintainPriv);
    if (maintainPriv)
    {
	connect(_conversionRates, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
	connect(_conversionRates, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
	connect(_conversionRates, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    }
    else
    {
	connect(_conversionRates, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }
    
    connect(_conversionRates, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

    _dateCluster->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
    _dateCluster->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
    
    _queryParameters->setType(CurrencyNotBase);

    switch (_metrics->value("CurrencyExchangeSense").toInt())
    {
	case 0:
	    _exchSenseLit->setText(tr("Base x Exchange Rate = Foreign"));
	    break;
	case 1:
	    _exchSenseLit->setText(tr("Foreign x Exchange Rate = Base"));
	    break;
	case -1:
	default:
	    QMessageBox::warning(this, tr("No Exchange Rate Direction Defined"),
				   tr("No selection has yet been made for "
				      "whether exchange rates convert from\n"
				      "the base currency to foreign currencies "
				      "or from foreign to base. Go to\n"
				      "System | Configure Modules | Configure "
				      "G/L\nand make your selection."));
	    close();
    }


    sFillList();
}

void currencyConversions::sNew()
{
    if (omfgThis->singleCurrency())
	QMessageBox::warning(this, tr("No Foreign Currencies Defined"),
				   tr("There is only a base currency defined.  "
				      "You must add more currencies before you "
				      "can create an exchange rate.  Click the "
				      "NEW CURRENCY button to add another "
				      "currency."));
    else
    {
	ParameterList params;
	params.append("mode", "new");
	if (_queryParameters->isSelected())
	    params.append("curr_id", _queryParameters->id());
	
	currencyConversion newdlg(this, "", TRUE);
	newdlg.set(params);
	newdlg.exec();
	sFillList();
    }
}

void currencyConversions::sEdit()
{
    ParameterList params;
    params.append("mode", "edit");
    params.append("curr_rate_id", _conversionRates->id());
    
    currencyConversion newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
    sFillList();
}

void currencyConversions::sView()
{
    ParameterList params;
    params.append("mode", "view");
    params.append("curr_rate_id", _conversionRates->id());
    
    currencyConversion newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
}

void currencyConversions::sPrint()
{
    ParameterList params;
    
    orReport report("CurrencyConversionList", params);
    if (report.isValid())
	report.print();
    else
	report.reportError(this);
}

void currencyConversions::sClose()
{
    close();
}

void currencyConversions::sDelete()
{
    q.prepare("DELETE FROM curr_rate "
	      "WHERE curr_rate_id = :curr_rate_id");
    q.bindValue(":curr_rate_id", _conversionRates->id());
    q.exec();
    sFillList();
}

void currencyConversions::sNewCurrency()
{
    ParameterList params;
    params.append("mode", "new");
    
    currency newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
    setBaseCurrency();
    _queryParameters->repopulateSelected();
}

void currencyConversions::sFillList()
{
    QString inverter("");

    if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
	inverter = "1 / ";

    // the N in round(%1 curr_rate, N) has to be SCALE - 1 of curr_rate's
    // NUMERIC(p, SCALE) type definition
    QString sql = QString("SELECT curr_rate_id, currConcat(curr_id) AS f_curr, "
			  "    round(%1 curr_rate, 5), curr_effective, "
			  "    curr_expires "
			  "FROM curr_symbol NATURAL JOIN curr_rate "
			  "WHERE curr_base = FALSE "
			  "  AND curr_expires >= :startDate "
			  "  AND curr_effective <= :endDate ").arg(inverter);

    if (_queryParameters->isSelected())
    {
	QString intString;
	sql+= " AND curr_id = :curr_id ";
    }
    else if (_queryParameters->isPattern())
    {
	sql += " AND currConcat(curr_id) ~ :currConcat_pattern ";
    }

    sql += " ORDER BY f_curr, curr_effective, curr_expires ";
    
    q.prepare(sql);

    _queryParameters->bindValue(q);
    _dateCluster->bindValue(q);

    q.exec();
    _conversionRates->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
	QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
			      .arg(__FILE__)
			      .arg(__LINE__),
			      q.lastError().databaseText());
    }
}

void currencyConversions::sPopulateMenu( QMenu* pMenu)
{
    int menuItem;
    
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainCurrencyRates"))
	pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

    menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
    if (!_privileges->check("MaintainCurrencyRates"))
	pMenu->setItemEnabled(menuItem, FALSE);
}

void currencyConversions::setBaseCurrency()
{
    q.prepare("SELECT currConcat(curr_abbr, curr_symbol) AS baseCurrency "
	      "FROM curr_symbol "
	      "WHERE curr_base = TRUE");
    q.exec();
    if (q.first())
    {
	_baseCurrency->setText(q.value("baseCurrency").toString());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
	QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
			      .arg(__FILE__)
			      .arg(__LINE__),
			      q.lastError().databaseText());
    }
}
