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

#include "maintainItemCosts.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include "dspItemCostDetail.h"
#include "itemCost.h"

/*
 *  Constructs a maintainItemCosts as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
maintainItemCosts::maintainItemCosts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_itemcost, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
    connect(_itemcost, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_itemcost, SIGNAL(itemSelectionChanged()), this, SLOT(sSelectionChanged()));

    statusBar()->hide();
    
    _itemcost->addColumn(tr("Element"),     -1,           Qt::AlignLeft   );
    _itemcost->addColumn(tr("Lower"),       _costColumn,  Qt::AlignCenter );
    _itemcost->addColumn(tr("Std. Cost"),   _costColumn,  Qt::AlignRight  );
    _itemcost->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft );
    _itemcost->addColumn(tr("Posted"),      _dateColumn,  Qt::AlignCenter );
    _itemcost->addColumn(tr("Act. Cost"),   _costColumn,  Qt::AlignRight  );
    _itemcost->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft );
    _itemcost->addColumn(tr("Updated"),     _dateColumn,  Qt::AlignCenter );

    if (omfgThis->singleCurrency())
    {
	_itemcost->hideColumn(3);
	_itemcost->hideColumn(6);
    }

    if (_privileges->check("CreateCosts"))
      connect(_item, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
maintainItemCosts::~maintainItemCosts()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void maintainItemCosts::languageChange()
{
    retranslateUi(this);
}

enum SetResponse maintainItemCosts::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  return NoError;
}

void maintainItemCosts::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if (((XTreeWidgetItem *)pSelected)->id() == -1)
    return;

  int menuItem;

  if (pSelected->text(1) == "No")
  {
    if (pSelected->text(0) == "Direct Labor")
    {
      menuItem = pMenu->insertItem(tr("Update Actual Cost..."), this, SLOT(sUpdateDirectLabor()), 0);
      if (!_privileges->check("UpdateActualCosts"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }

    else if (pSelected->text(0) == "Overhead")
    {
      menuItem = pMenu->insertItem(tr("Update Actual Cost..."), this, SLOT(sUpdateOverhead()), 0);
      if (!_privileges->check("UpdateActualCosts"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }

    else if (pSelected->text(0) == "Machine Overhead")
    {
      menuItem = pMenu->insertItem(tr("Update Actual Cost..."), this, SLOT(sUpdateMachineOverhead()), 0);
      if (!_privileges->check("UpdateActualCosts"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }
  }
  else
  {
    pMenu->insertItem(tr("View Costing Detail..."), this, SLOT(sViewDetail()), 0);
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Update Actual Cost..."), this, SLOT(sUpdateDetail()), 0);
    if (!_privileges->check("UpdateActualCosts"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if (((XTreeWidgetItem *)pSelected)->altId() == 0)
  {
    menuItem = pMenu->insertItem(tr("Delete Cost..."), this, SLOT(sDelete()), 0);
    if (!_privileges->check("DeleteCosts"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("Post Actual Cost to Standard..."), this, SLOT(sPost()), 0);
  if (!_privileges->check("PostActualCosts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Actual Cost..."), this, SLOT(sEnterActualCost()), 0);
  if (!_privileges->check("EnterActualCosts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("New Actual Cost..."), this, SLOT(sCreateUserCost()), 0);
  if (!_privileges->check("CreateCosts"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void maintainItemCosts::sViewDetail()
{
  ParameterList params;
  params.append("itemcost_id", _itemcost->id());
  params.append("run");

  dspItemCostDetail *newdlg = new dspItemCostDetail();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void maintainItemCosts::sUpdateDetail()
{
  QString cost = _itemcost->currentItem()->text(0);

  q.prepare("SELECT updateCost(:item_id, :cost, TRUE, lowerCost(:item_id, :cost));");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":cost", cost);
  q.exec();

  sFillList();
}

void maintainItemCosts::sUpdateDirectLabor()
{
  q.prepare("SELECT updateCost(:item_id, 'Direct Labor', FALSE, directLaborCost(:item_id));");
  q.bindValue(":item_id", _item->id());
  q.exec();

  sFillList();
}

void maintainItemCosts::sUpdateOverhead()
{
  q.prepare("SELECT updateCost(:item_id, 'Overhead', FALSE, overheadCost(:item_id));");
  q.bindValue(":item_id", _item->id());
  q.exec();

  sFillList();
}

void maintainItemCosts::sUpdateMachineOverhead()
{
  q.prepare("SELECT updateCost(:item_id, 'Machine Overhead', FALSE, machineOverheadCost(:item_id));");
  q.bindValue(":item_id", _item->id());
  q.exec();

  sFillList();
}

void maintainItemCosts::sPost()
{
  q.prepare("SELECT postCost(:item_id);");
  q.bindValue(":item_id", _itemcost->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  sFillList();
}

void maintainItemCosts::sDelete()
{
  double stdCost = 0.0;
  q.prepare( "SELECT itemcost_stdcost "
	     "FROM itemcost "
	     "WHERE (itemcost_id=:itemcost_id);" );
  q.bindValue(":itemcost_id", _itemcost->id());
  q.exec();
  if (q.first())
    stdCost = q.value("itemcost_stdcost").toDouble();
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (stdCost > 0)
  {
    if (QMessageBox::warning(this, tr("Deletion of Costing Elements"),
			     tr("<p>Before a Costing Element is deleted, the "
				"system will set the Actual Cost value to 0. "
				"This helps ensure Inventory is valued "
				"correctly. Once the 0 value Actual Cost is "
				"posted to Standard, the Costing Element will "
				"be removed."),
			     QMessageBox::Ok | QMessageBox::Default,
			     QMessageBox::Cancel) == QMessageBox::Cancel)
      return;
    q.prepare("SELECT updateCost(:itemcost_id, 0);");
  }
  else
  {
    q.prepare( "DELETE FROM itemcost WHERE (itemcost_id=:itemcost_id);" );
  }
  q.bindValue(":itemcost_id", _itemcost->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  sFillList();
}

void maintainItemCosts::sEnterActualCost()
{
  ParameterList params;
  params.append("itemcost_id", _itemcost->id());
  params.append("mode", "edit");

  itemCost newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec())
    sFillList();
}

void maintainItemCosts::sCreateUserCost()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("mode", "new");

  itemCost newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec())
    sFillList();
}

void maintainItemCosts::sNew()
{
  sCreateUserCost();
}

void maintainItemCosts::sEdit()
{
  sEnterActualCost();
}

void maintainItemCosts::sFillList()
{
  if (_item->isValid())
  {
    double standardCost = 0.0;
    double actualCostBase = 0.0;
    double actualCostLocal = 0.0;

    q.prepare( "SELECT itemcost_id,"
               "       CASE WHEN (costelem_sys) THEN 1"
               "            ELSE 0"
               "       END,"
               "       CASE WHEN itemcost_costelem_id = -1 THEN :error "
	       "	    ELSE costelem_type "
	       "	END AS costelem_type, formatBoolYN(itemcost_lowlevel),"
               "       formatCost(itemcost_stdcost), currConcat(baseCurrID()), "
	       "       formatDate(itemcost_posted, 'Never'),"
               "       formatCost(itemcost_actcost), currConcat(itemcost_curr_id) AS currConcat, "
	       "       formatDate(itemcost_updated, 'Never'),"
               "       itemcost_stdcost AS stdcost, "
	       "       itemcost_actcost AS actcostLocal, "
	       "       currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE) AS actcostBase, "
	       "       itemcost_curr_id "
               "FROM itemcost LEFT OUTER JOIN costelem ON "
               "	(itemcost_costelem_id=costelem_id)"
               "WHERE (itemcost_item_id=:item_id) "
               "ORDER BY itemcost_lowlevel, costelem_type;" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":error", tr("!ERROR!"));
    q.exec();
    _itemcost->populate(q, TRUE);

    bool multipleCurrencies = false;
    int firstCurrency = 0;
    bool baseKnown = true;
    if (q.first())
    {
      firstCurrency = q.value("itemcost_curr_id").toInt();
      do
      {
        standardCost += q.value("stdcost").toDouble();
	if (q.value("actcostBase").isNull())
	    baseKnown = false;
	else
	    actualCostBase += q.value("actcostBase").toDouble();
        actualCostLocal += q.value("actcostLocal").toDouble();
	if (! multipleCurrencies &&
	    q.value("itemcost_curr_id").toInt() != firstCurrency)
	    multipleCurrencies = true;
      }
      while (q.next());
    }

    XSqlQuery convert;
    double actualCost = 0;
    if (multipleCurrencies)
    {
	actualCost = actualCostBase;
	convert.prepare("SELECT currConcat(baseCurrId()) AS baseConcat, "
			"       currConcat(baseCurrId()) AS currConcat;");
    }
    else
    {
	actualCost = actualCostLocal;
	baseKnown = true; // not necessarily but we can trust actualCost
	convert.prepare("SELECT currConcat(baseCurrId()) AS baseConcat, "
			"	currConcat(:firstCurrency) AS currConcat;" );
	convert.bindValue(":firstCurrency", firstCurrency);
    }
    convert.exec();
    if (convert.first())
	new XTreeWidgetItem(_itemcost,
			    _itemcost->topLevelItem(_itemcost->topLevelItemCount() - 1), -1,
			    QVariant(tr("Totals")),
			    "",
			    formatCost(standardCost),
			    convert.value("baseConcat"),
			    "",
			    baseKnown ? formatCost(actualCost) : tr("?????"),
			    convert.value("currConcat"));
    else if (convert.lastError().type() != QSqlError::NoError)
	systemError(this, convert.lastError().databaseText(), __FILE__, __LINE__);

  }
  else
    _itemcost->clear();
}

void maintainItemCosts::sSelectionChanged()
{
  bool yes = (_itemcost->id() != -1);

  if (_privileges->check("EnterActualCosts"))
    _edit->setEnabled(yes);

  if (_privileges->check("DeleteCosts"))
    _delete->setEnabled(yes);
}

