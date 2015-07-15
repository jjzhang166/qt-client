/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unpostedPurchaseOrders.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>

#include "purchaseOrder.h"
#include "printPurchaseOrder.h"
#include "guiclient.h"
#include "storedProcErrorLookup.h"
#include "parameterwidget.h"

unpostedPurchaseOrders::unpostedPurchaseOrders(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "unpostedPurchaseOrders", fl)
{
  setupUi(optionsWidget());
  setUseAltId(true);
  setWindowTitle(tr("Open Purchase Orders"));
  setMetaSQLOptions("openpurchaseorders", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setQueryOnStartEnabled(true);
  setAutoUpdateEnabled(true);
  setSearchVisible(true);

  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);
  parameterWidget()->append(tr("Vendor"), "vend_id", ParameterWidget::Vendor);
  parameterWidget()->appendComboBox(tr("Vendor Type"), "vendtype_id", XComboBox::VendorTypes);
  parameterWidget()->append(tr("Vendor Type Pattern"), "vendtype_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Purchase Agent"), "pohead_agent_usr_id", XComboBox::Agent);

  connect(omfgThis,	SIGNAL(purchaseOrdersUpdated(int, bool)),
                                              this,	SLOT(sFillList()));

  list()->addColumn(tr("P/O #"),         _orderColumn, Qt::AlignLeft,   true, "pohead_number" );
  list()->addColumn(tr("Vendor #"),      _orderColumn, Qt::AlignLeft,   true, "vend_number"   );
  list()->addColumn(tr("Vendor"),        _orderColumn, Qt::AlignLeft,   true, "vend_name"   );
  list()->addColumn(tr("Order Date"),    _dateColumn,  Qt::AlignCenter, true, "pohead_orderdate" );
  list()->addColumn(tr("Release Date"),  _dateColumn,  Qt::AlignCenter, true, "pohead_released" );
  list()->addColumn(tr("Due Date"),      _dateColumn,  Qt::AlignCenter, true, "min_duedate" );
  list()->addColumn(tr("Status"),        _ynColumn,    Qt::AlignCenter, true, "pohead_status" );
  list()->addColumn(tr("Printed"),       _ynColumn,    Qt::AlignCenter, true, "pohead_printed");
  list()->addColumn(tr("Total Amount"),  _moneyColumn, Qt::AlignRight,  true, "order_total" );
  list()->addColumn(tr("Vend. Type"),    _orderColumn, Qt::AlignLeft,   false,"vendtype_code" );
  list()->addColumn(tr("Agent"),         _orderColumn, Qt::AlignLeft,   true, "pohead_agent_username");

  setupCharacteristics("PO");
  list()->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _showUnreleased->setChecked(false);

  if (_privileges->check("MaintainPurchaseOrders"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }
}

void unpostedPurchaseOrders::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedPurchaseOrders::sEdit()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  bool done = false;
  for (int i = 0; i < selected.size(); i++)
  {
    if (((selected[i]->rawValue("pohead_status").toString() == "U" && _privileges->check("MaintainPurchaseOrders")) ||
        (selected[i]->rawValue("pohead_status").toString() == "O" && _privileges->check("MaintainPostedPurchaseOrders"))) &&
        (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id())))
    {
      ParameterList params;
      params.append("mode", "edit");
      params.append("pohead_id", ((XTreeWidgetItem*)(selected[i]))->id());

      purchaseOrder *newdlg = new purchaseOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      done = true;
      break;
    }
  }
  if (done)
    omfgThis->sPurchaseOrdersUpdated(-1, true);
  else
    QMessageBox::information(this, tr("Nothing To Edit"),
			     tr("<p>There were no selected Purchase Orders "
				"that you could edit."),
			     QMessageBox::Ok|QMessageBox::Default);
}

void unpostedPurchaseOrders::sView()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {

      ParameterList params;
      params.append("mode", "view");
      params.append("pohead_id", ((XTreeWidgetItem*)(selected[i]))->id());

      purchaseOrder *newdlg = new purchaseOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      break;
    }
  }
}

void unpostedPurchaseOrders::sDelete()
{
  XSqlQuery unpostedDelete;
  if ( QMessageBox::question(this, tr("Delete Selected Purchase Orders"),
                             tr("<p>Are you sure that you want to delete the "
			        "selected Purchase Orders?" ),
                             QMessageBox::Yes,
                             QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    unpostedDelete.prepare("SELECT deletePo(:pohead_id) AS result;");

    QList<XTreeWidgetItem*> selected = list()->selectedItems();
    bool done = false;
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        if (selected[i]->rawValue("pohead_status").toString() == "U")
        {
          if (selected[i]->altId() != -1)
		  {
            QString question = tr("<p>The Purchase Order that you selected to delete was created "
            "to satisfy Sales Order demand. If you delete the selected "
            "Purchase Order then the Sales Order demand will remain but "
            "the Purchase Order to relieve that demand will not. Are you "
            "sure that you want to delete the selected Purchase Order?" );
            if (QMessageBox::question(this, tr("Delete Purchase Order?"),
                                      question,
                                      QMessageBox::Yes,
                                      QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
			  continue;
		  }
          unpostedDelete.bindValue(":pohead_id", ((XTreeWidgetItem*)(selected[i]))->id());
          unpostedDelete.exec();
          if (unpostedDelete.first() && ! unpostedDelete.value("result").toBool())
              systemError(this, tr("<p>Only Unposted Purchase Orders may be "
                                   "deleted. Check the status of Purchase Order "
                                   "%1. If it is 'U' then contact your system "
                                   "Administrator.").arg(selected[i]->text(0)),
                          __FILE__, __LINE__);
          else if (unpostedDelete.lastError().type() != QSqlError::NoError)
            systemError(this, unpostedDelete.lastError().databaseText(), __FILE__, __LINE__);
          else
            done = true;
        }
      }
    }
    if (done)
      omfgThis->sPurchaseOrdersUpdated(-1, true);
    else
      QMessageBox::information(this, tr("Nothing To Delete"),
			       tr("<p>There were no selected Purchase Orders "
				  "that could be deleted."),
			       QMessageBox::Ok|QMessageBox::Default);
  }
}

void unpostedPurchaseOrders::sPrint()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      ParameterList params;
      params.append("pohead_id", ((XTreeWidgetItem*)(selected[i]))->id());

      printPurchaseOrder newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
      break;
    }
  }
  sFillList();
}

void unpostedPurchaseOrders::sRelease()
{
  XSqlQuery unpostedRelease;
  unpostedRelease.prepare("SELECT releasePurchaseOrder(:pohead_id) AS result;");

  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  bool done = false;
  for (int i = 0; i < selected.size(); i++)
  {
    if ((selected[i]->rawValue("pohead_status").toString() == "U")
      && (_privileges->check("ReleasePurchaseOrders"))
      && (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id())))
      {
      unpostedRelease.bindValue(":pohead_id", ((XTreeWidgetItem*)(selected[i]))->id());
      unpostedRelease.exec();
      if (unpostedRelease.first())
      {
        int result = unpostedRelease.value("result").toInt();
        if (result < 0)
          systemError(this, storedProcErrorLookup("releasePurchaseOrder", result),
                      __FILE__, __LINE__);
        else
          done = true;
      }
      else if (unpostedRelease.lastError().type() != QSqlError::NoError)
        systemError(this, unpostedRelease.lastError().databaseText(), __FILE__, __LINE__);
    }
  }
  if (done)
    omfgThis->sPurchaseOrdersUpdated(-1, true);
  else
    QMessageBox::information(this, tr("Nothing To Release"),
                             tr("<p>There were no selected Purchase Orders "
                                "to be released."),
                             QMessageBox::Ok|QMessageBox::Default);
}

void unpostedPurchaseOrders::sUnrelease()
{
  XSqlQuery unRelease;
  unRelease.prepare("SELECT unreleasePurchaseOrder(:pohead_id) AS result;");
  
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  bool done = false;
  for (int i = 0; i < selected.size(); i++)
  {
    if ((selected[i]->rawValue("pohead_status").toString() == "O")
        && (_privileges->check("UnreleasePurchaseOrders"))
        && (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id())))
    {
      unRelease.bindValue(":pohead_id", ((XTreeWidgetItem*)(selected[i]))->id());
      unRelease.exec();
      if (unRelease.first())
      {
        int result = unRelease.value("result").toInt();
        if (result < 0)
          systemError(this, storedProcErrorLookup("unreleasePurchaseOrder", result),
                      __FILE__, __LINE__);
        else
          done = true;
      }
      else if (unRelease.lastError().type() != QSqlError::NoError)
        systemError(this, unRelease.lastError().databaseText(), __FILE__, __LINE__);
    }
  }
  if (done)
    omfgThis->sPurchaseOrdersUpdated(-1, true);
  else
    QMessageBox::information(this, tr("Nothing To Unrelease"),
                             tr("<p>There were no selected Purchase Orders "
                                "to be unreleased."),
                             QMessageBox::Ok|QMessageBox::Default);
}

void unpostedPurchaseOrders::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem, int)
{
  QAction *menuItem;

  XTreeWidgetItem* item = (XTreeWidgetItem*)pItem;
  bool canMaintain = (item->rawValue("pohead_status").toString() == "U" && _privileges->check("MaintainPurchaseOrders")) ||
                     (item->rawValue("pohead_status").toString() == "O" && _privileges->check("MaintainPurchaseOrders") && _privileges->check("MaintainPostedPurchaseOrders"));


  menuItem = pMenu->addAction(tr("Print..."), this, SLOT(sPrint()));
  menuItem->setEnabled(_privileges->check("PrintPurchaseOrders"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(canMaintain);

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(canMaintain || _privileges->check("ViewPurchaseOrders"));

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled((item->rawValue("pohead_status").toString() == "U" && _privileges->check("MaintainPurchaseOrders")));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Release..."), this, SLOT(sRelease()));
  menuItem->setEnabled(_privileges->check("ReleasePurchaseOrders") &&
                       item->rawValue("pohead_status").toString() == "U");
  menuItem = pMenu->addAction(tr("Unrelease..."), this, SLOT(sUnrelease()));
  menuItem->setEnabled(_privileges->check("UnreleasePurchaseOrders") &&
                       item->rawValue("pohead_status").toString() == "O");
}

bool unpostedPurchaseOrders::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  if (_showUnreleased->isChecked() && _showOpen->isChecked() )
    params.append("showBoth");
  else if (_showUnreleased->isChecked())
    params.append("showUnreleased");
  else if (_showOpen->isChecked())
    params.append("showOpen");
  else
    params.append("shownothing");
  params.append("closed", tr("Closed"));
  params.append("unposted", tr("Unreleased"));
  params.append("open", tr("Open"));

  return true;
}

bool unpostedPurchaseOrders::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkPOSitePrivs(:poheadid) AS result;");
    check.bindValue(":poheadid", orderid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("<p>You may not view or edit this Purchase "
                                 "Order as it references a Site for which you "
                                 "have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
