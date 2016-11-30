/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "maintainItemCosts.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "dspItemCostDetail.h"
#include "mqlutil.h"
#include "itemCost.h"
#include "errorReporter.h"

maintainItemCosts::maintainItemCosts(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

    connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_itemcost, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
    connect(_itemcost, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_itemcost, SIGNAL(itemSelectionChanged()), this, SLOT(sSelectionChanged()));
    
    _itemcost->addColumn(tr("Element"),     -1,           Qt::AlignLeft,   true, "costelem_type");
    _itemcost->addColumn(tr("Lower"),       _costColumn,  Qt::AlignCenter, true, "itemcost_lowlevel" );
    _itemcost->addColumn(tr("Std. Cost"),   _costColumn,  Qt::AlignRight,  true, "itemcost_stdcost"  );
    _itemcost->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,!omfgThis->singleCurrency(), "baseCurr" );
    _itemcost->addColumn(tr("Posted"),      _dateColumn,  Qt::AlignCenter, true, "itemcost_posted" );
    _itemcost->addColumn(tr("Act. Cost"),   _costColumn,  Qt::AlignRight,  true, "itemcost_actcost"  );
    _itemcost->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,!omfgThis->singleCurrency(), "costCurr" );
    _itemcost->addColumn(tr("Updated"),     _dateColumn,  Qt::AlignCenter, true, "itemcost_updated" );

    if (_privileges->check("CreateCosts"))
    {
      connect(_item, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
      _new->setEnabled(false);  //initially disabled until item entered
    }
}

maintainItemCosts::~maintainItemCosts()
{
    // no need to delete child widgets, Qt does it all for us
}

void maintainItemCosts::languageChange()
{
    retranslateUi(this);
}

enum SetResponse maintainItemCosts::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(true);
  }

  return NoError;
}

void maintainItemCosts::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if (((XTreeWidgetItem *)pSelected)->id() == -1)
    return;

  QAction *menuItem;

  if (pSelected->text(1) == "No")
  {
    if (pSelected->text(0) == "Direct Labor")
    {
      menuItem = pMenu->addAction(tr("Update Actual Cost..."), this, SLOT(sUpdateDirectLabor()));
      menuItem->setEnabled(_privileges->check("UpdateActualCosts"));
    }

    else if (pSelected->text(0) == "Overhead")
    {
      menuItem = pMenu->addAction(tr("Update Actual Cost..."), this, SLOT(sUpdateOverhead()));
      menuItem->setEnabled(_privileges->check("UpdateActualCosts"));
    }

    else if (pSelected->text(0) == "Machine Overhead")
    {
      menuItem = pMenu->addAction(tr("Update Actual Cost..."), this, SLOT(sUpdateMachineOverhead()));
      menuItem->setEnabled(_privileges->check("UpdateActualCosts"));
    }
  }
  else
  {
    pMenu->addAction(tr("View Costing Detail..."), this, SLOT(sViewDetail()));
    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Update Actual Cost..."), this, SLOT(sUpdateDetail()));
    menuItem->setEnabled(_privileges->check("UpdateActualCosts"));
  }

  if (((XTreeWidgetItem *)pSelected)->altId() == 0)
  {
    menuItem = pMenu->addAction(tr("Delete Cost..."), this, SLOT(sDelete()));
    menuItem->setEnabled(_privileges->check("DeleteCosts"));
  }

  menuItem = pMenu->addAction(tr("Post Actual Cost to Standard..."), this, SLOT(sPost()));
  menuItem->setEnabled(_privileges->check("PostActualCosts"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Edit Actual Cost..."), this, SLOT(sEnterActualCost()));
  menuItem->setEnabled(_privileges->check("EnterActualCosts"));

  menuItem = pMenu->addAction(tr("New Actual Cost..."), this, SLOT(sCreateUserCost()));
  menuItem->setEnabled(_privileges->check("CreateCosts"));
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
  XSqlQuery maintainUpdateDetail;
  QString cost = _itemcost->currentItem()->text(0);

  maintainUpdateDetail.prepare("SELECT updateCost(:item_id, :cost, true, lowerCost(:item_id, :cost));");
  maintainUpdateDetail.bindValue(":item_id", _item->id());
  maintainUpdateDetail.bindValue(":cost", cost);
  maintainUpdateDetail.exec();

  sFillList();
}

void maintainItemCosts::sUpdateDirectLabor()
{
  XSqlQuery maintainUpdateDirectLabor;
  maintainUpdateDirectLabor.prepare("SELECT updateCost(:item_id, 'Direct Labor', false, directLaborCost(:item_id));");
  maintainUpdateDirectLabor.bindValue(":item_id", _item->id());
  maintainUpdateDirectLabor.exec();

  sFillList();
}

void maintainItemCosts::sUpdateOverhead()
{
  XSqlQuery maintainUpdateOverhead;
  maintainUpdateOverhead.prepare("SELECT updateCost(:item_id, 'Overhead', false, overheadCost(:item_id));");
  maintainUpdateOverhead.bindValue(":item_id", _item->id());
  maintainUpdateOverhead.exec();

  sFillList();
}

void maintainItemCosts::sUpdateMachineOverhead()
{
  XSqlQuery maintainUpdateMachineOverhead;
  maintainUpdateMachineOverhead.prepare("SELECT updateCost(:item_id, 'Machine Overhead', false, machineOverheadCost(:item_id));");
  maintainUpdateMachineOverhead.bindValue(":item_id", _item->id());
  maintainUpdateMachineOverhead.exec();

  sFillList();
}

void maintainItemCosts::sPost()
{
  XSqlQuery maintainPost;
  maintainPost.prepare("SELECT postCost(:item_id);");
  maintainPost.bindValue(":item_id", _itemcost->id());
  maintainPost.exec();
  if (maintainPost.lastError().type() != QSqlError::NoError)
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Cost Information"),
                       maintainPost, __FILE__, __LINE__);

  sFillList();
}

void maintainItemCosts::sDelete()
{
  XSqlQuery maintainDelete;
  double stdCost = 0.0;
  maintainDelete.prepare( "SELECT itemcost_stdcost "
	     "FROM itemcost "
	     "WHERE (itemcost_id=:itemcost_id);" );
  maintainDelete.bindValue(":itemcost_id", _itemcost->id());
  maintainDelete.exec();
  if (maintainDelete.first())
    stdCost = maintainDelete.value("itemcost_stdcost").toDouble();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Cost Information"),
                                maintainDelete, __FILE__, __LINE__))
  {
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
    maintainDelete.prepare("SELECT updateCost(:itemcost_id, 0);");
  }
  else
  {
    maintainDelete.prepare( "DELETE FROM itemcost WHERE (itemcost_id=:itemcost_id);" );
  }
  maintainDelete.bindValue(":itemcost_id", _itemcost->id());
  maintainDelete.exec();
  if (maintainDelete.lastError().type() != QSqlError::NoError)
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Cost Information"),
                                maintainDelete, __FILE__, __LINE__);

  sFillList();
}

void maintainItemCosts::sEnterActualCost()
{
  ParameterList params;
  params.append("itemcost_id", _itemcost->id());
  params.append("mode", "edit");

  itemCost newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec())
    sFillList();
}

void maintainItemCosts::sCreateUserCost()
{
  if (_item->isValid()) {
    ParameterList params;
    params.append("item_id", _item->id());
    params.append("mode", "new");

    itemCost newdlg(this, "", true);
    if (newdlg.set(params) == NoError && newdlg.exec())
      sFillList();
  } else {
    QMessageBox::warning(this, tr("Missing or Invalid Item Number"),
                           tr("Item Number Required"));
  }
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

    MetaSQLQuery mql = mqlLoad("itemCost", "list");

    ParameterList params;
    params.append("item_id", _item->id());
    params.append("error", tr("!ERROR!"));
    params.append("never", tr("Never"));

    XSqlQuery qry = mql.toQuery(params);
    _itemcost->populate(qry, true);

    bool multipleCurrencies = false;
    int firstCurrency = 0;
    bool baseKnown = true;
    if (qry.first())
    {
      firstCurrency = qry.value("itemcost_curr_id").toInt();
      do
      {
        standardCost += qry.value("itemcost_stdcost").toDouble();
        if (qry.value("itemcost_actcost").isNull())
	    baseKnown = false;
	else
            actualCostBase += qry.value("actcostBase").toDouble();
        actualCostLocal += qry.value("itemcost_actcost").toDouble();
	if (! multipleCurrencies &&
            qry.value("itemcost_curr_id").toInt() != firstCurrency)
	    multipleCurrencies = true;
      }
      while (qry.next());
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
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cost Information"),
                                convert, __FILE__, __LINE__);

  }
  else {
    _itemcost->clear();
    _new->setEnabled(false);
  }
}

void maintainItemCosts::sSelectionChanged()
{
  bool yes = (_itemcost->id() != -1);

  if (_privileges->check("EnterActualCosts"))
    _edit->setEnabled(yes);

  if (_privileges->check("DeleteCosts"))
    _delete->setEnabled(yes);
}

